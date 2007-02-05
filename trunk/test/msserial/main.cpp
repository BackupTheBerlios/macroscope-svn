/*-
 * Copyright 2006-2007 Guram Dukashvili
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
//------------------------------------------------------------------------------
#include <adicpp/adicpp.h>
#include "msserial.h"
//------------------------------------------------------------------------------
#define _VERSION_C_AS_HEADER_
#include "version.c"
#undef _VERSION_C_AS_HEADER_
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
SerialPortControl::~SerialPortControl()
{
  if( reader_ != NULL && !reader_->started() ) deleteObject(reader_);
  if( writer_ != NULL && !writer_->started() ) deleteObject(writer_);
}
//------------------------------------------------------------------------------
SerialPortControl::SerialPortControl() :
  control_(NULL),
  reader_(NULL),
  writer_(NULL)
{
}
//------------------------------------------------------------------------------
void SerialPortControl::open(const utf8::String & device)
{
  device_.exclusive(true).fileName(device).open();
#if defined(__WIN32__) || defined(__WIN64__)
  int32_t err;
  if( SetupComm(device_.descriptor(),1600,1600) == 0 ){
    err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  COMMTIMEOUTS cto;
  memset(&cto,0,sizeof(cto));
  cto.ReadIntervalTimeout = MAXDWORD;
  /*cto.ReadTotalTimeoutMultiplier = 0;
  cto.ReadTotalTimeoutConstant = 0;
  cto.WriteTotalTimeoutMultiplier = MAXDWORD;
  cto.WriteTotalTimeoutConstant = MAXDWORD - 1;*/
  if( SetCommTimeouts(device_.descriptor(),&cto) == 0 ){
    err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( SetCommMask(device_.descriptor(),EV_RXCHAR) == 0 ){
    err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( PurgeComm(device_.descriptor(),PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) == 0 ){
    err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#endif
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
SerialPortFiber::~SerialPortFiber()
{
}
//------------------------------------------------------------------------------
SerialPortFiber::SerialPortFiber(MSSerialService & service,SerialPortControl * control) :
  service_(&service), control_(control)
{
}
//------------------------------------------------------------------------------
void SerialPortFiber::removeControl()
{
  if( control_ != NULL ){
    if( control_->control_ == this ){
      AutoLock<FiberInterlockedMutex> lock(service_->serialPortsMutex_);
      for( intptr_t i = service_->serialPorts_.count() - 1; i >= 0; i-- )
        if( &service_->serialPorts_[i] == control_ ){
          service_->serialPorts_.remove(i);
          control_ = NULL;
          break;
        }
    }
    else {
      control_->semaphore_.post();
    }
    control_ = NULL;
  }
}
//------------------------------------------------------------------------------
void SerialPortFiber::main()
{
  utf8::String device(readString()), mode(readString());
  if( control_ == NULL ){
    AutoLock<FiberInterlockedMutex> lock(service_->serialPortsMutex_);
    for( intptr_t i = service_->serialPorts_.count() - 1; i >= 0; i-- ){
      bool isRD = false, isWR = false;
#if defined(__WIN32__) || defined(__WIN64__)
      if( service_->serialPorts_[i].device_.fileName().strcasecmp(device) == 0 ){
#else
      if( service_->serialPorts_[i].device_.fileName().strcmp(device) == 0 ){
#endif
        isRD = mode.strcasecmp("READ")  == 0;
        isWR = mode.strcasecmp("WRITE") == 0;
        if( (isRD && service_->serialPorts_[i].reader_ != NULL) ||
            (isWR && service_->serialPorts_[i].writer_ != NULL) )
          newObjectV1C2<Exception>(EACCES,__PRETTY_FUNCTION__)->throwSP();
        if( isRD || isWR ){
          control_ = &service_->serialPorts_[i];
          break;
        }
        newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
      }
    }
    if( control_ == NULL ){
      control_ = &service_->serialPorts_.add(newObject<SerialPortControl>());
      control_->open(device);
    }
    if( control_->reader_ == NULL ){
      control_->reader_ = newObjectR1V2<SerialPortFiber>(*service_,control_);
      control_->reader_->socket_ = socket_;
      socket_ = INVALID_SOCKET;
      thread()->server()->attachFiber(control_->reader_);
    }
    else if( control_->control_ != NULL ){
      control_->writer_ = this;
    }
    control_->control_ = this;
  }
  try {
    Array<uint8_t> b;
    while( !terminated_ && (control_->reader_ != NULL || control_->writer_ != NULL) ){
      if( control_->reader_ == this ){
        if( b.count() == 0 ) b.resize(getpagesize());
#if (defined(__WIN32__) || defined(__WIN64__)) && !defined(NDEBUG)
        DWORD evtMask =
#elif !defined(NDEBUG)
        uint32_t evtMask =
#endif
        control_->device_.waitCommEvent();
#if (defined(__WIN32__) || defined(__WIN64__)) && !defined(NDEBUG)
        assert( evtMask & EV_RXCHAR );
#elif !defined(NDEBUG)
        assert( evtMask != 0 );
#endif
        int64_t r = control_->device_.read(b,b.count());
        if( r < 0 ) break;
        if( r != 0 ) write(b,r);
      }
      else if( control_->writer_ == this ){
        if( b.count() == 0 ) b.resize(getpagesize());
        int64_t r = recv(b,b.count());
        control_->device_.writeBuffer(b,r);
      }
      else { // control fiber
        control_->semaphore_.wait();
      }
    }
  }
  catch( ... ){
    removeControl();
    throw;
  }
  removeControl();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
MSSerialService::MSSerialService() :
  config_(newObject<InterlockedConfig<FiberInterlockedMutex> >())
{
  serviceName_ = "msserial";
  displayName_ = "Macroscope Serial Port Service";
#if defined(__WIN32__) || defined(__WIN64__)
  serviceType_ = SERVICE_WIN32_OWN_PROCESS;
  startType_ = SERVICE_AUTO_START;
  errorControl_ = SERVICE_ERROR_IGNORE;
  binaryPathName_ = getExecutableName();
  serviceStatus_.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  serviceStatus_.dwWaitHint = 60000; // give me 60 seconds for start or stop 
#endif
}
//------------------------------------------------------------------------------
Fiber * MSSerialService::newFiber()
{
  return newObjectR1<SerialPortFiber>(*this);
}
//------------------------------------------------------------------------------
void MSSerialService::start()
{
  stdErr.debug(0,utf8::String::Stream() << msserial_version.gnu_ << " started\n");
}
//------------------------------------------------------------------------------
void MSSerialService::stop()
{
  closeServer();
  stdErr.debug(0,utf8::String::Stream() << msserial_version.gnu_ << " stopped\n");
}
//------------------------------------------------------------------------------
bool MSSerialService::active()
{
  return active();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
int main(int ac,char * av[])
{
  int errcode = 0;
  adicpp::AutoInitializer autoInitializer(ac,av);
  autoInitializer = autoInitializer;
  try {
    union {
      intptr_t i;
      uintptr_t u;
    };
    stdErr.fileName(SYSLOG_DIR("msserial/") + "msserial.log");
    Config::defaultFileName(SYSCONF_DIR("") + "msserial.conf");
    Services services(msserial_version.gnu_);
    services.add(newObject<MSSerialService>());
#if defined(__WIN32__) || defined(__WIN64__)
    bool dispatch = true;
#else
    bool dispatch = false;
#endif
    for( u = 1; u < argv().count(); u++ ){
      if( argv()[u].strcmp("--version") == 0 ){
        stdErr.debug(9,utf8::String::Stream() << msserial_version.tex_ << "\n");
        fprintf(stdout,"%s\n",msserial_version.tex_);
        dispatch = false;
        continue;
      }
      if( argv()[u].strcmp("-c") == 0 && u + 1 < argv().count() ){
        Config::defaultFileName(argv()[u + 1]);
      }
      else if( argv()[u].strcmp("--log") == 0 && u + 1 < argv().count() ){
        stdErr.fileName(argv()[u + 1]);
      }
      else if( argv()[u].strcmp("--install") == 0 ){
        services.install();
        dispatch = false;
      }
      else if( argv()[u].strcmp("--uninstall") == 0 ){
        services.uninstall();
        dispatch = false;
      }
      else if( argv()[u].strcmp("--start") == 0 && u + 1 < argv().count() ){
        services.start(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].strcmp("--stop") == 0 && u + 1 < argv().count() ){
        services.stop(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].strcmp("--suspend") == 0 && u + 1 < argv().count() ){
        services.suspend(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].strcmp("--resume") == 0 && u + 1 < argv().count() ){
        services.resume(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].strcmp("--query") == 0 && u + 1 < argv().count() ){
        services.query(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].strcmp("--start-disp") == 0 ){
        dispatch = true;
      }
      else if( argv()[u].strcmp("--stop-disp") == 0 ){
        services.stopServiceCtrlDispatcher();
        dispatch = false;
      }
      else if( argv()[u].strcmp("--sha256") == 0 && u + 1 < argv().count() ){
        SHA256 passwordSHA256;
        passwordSHA256.make(argv()[u + 1].c_str(),argv()[u + 1].size());
        utf8::String b64(base64Encode(passwordSHA256.sha256(),passwordSHA256.size()));
        fprintf(stdout,"%s\n",b64.c_str());
        copyStrToClipboard(b64);
        dispatch = false;
      }
    }
    if( dispatch ){
      bool daemon;
      {
        ConfigSP config(newObject<InterlockedConfig<FiberInterlockedMutex> >());
        daemon = config->value("daemon",false);
      }
      services.startServiceCtrlDispatcher(daemon);
    }
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    errcode = e->code() >= errorOffset ? e->code() - errorOffset : e->code();
  }
  catch( ... ){
  }
  return errcode;
}
//------------------------------------------------------------------------------

