/*-
 * Copyright 2005 Guram Dukashvili
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
//---------------------------------------------------------------------------
#include <adicpp/ksys.h>
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
Service::~Service()
{
}
//---------------------------------------------------------------------------
Service::Service()
#if defined(__WIN32__) || defined(__WIN64__)
  : serviceNameW_(utf8::String().getUNICODEString())
#else
  : status_(svcReady)
#endif
{
#if defined(__WIN32__) || defined(__WIN64__)
  statusHandle_ = 0;
#endif
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
Service & Service::serviceMain(uintptr_t n,DWORD dwArgc,LPWSTR * lpszArgv)
{
  uintptr_t i;
  int32_t err;
  try {
    for( i = 0; i < dwArgc; i++ ) argv().add(lpszArgv[i]);
    statusHandle_ = RegisterServiceCtrlHandlerW(
      serviceName_.getUNICODEString(),
      Services::handlers_[n]
    );
    if( statusHandle_ == 0 ){
      err = GetLastError() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    serviceStatus_.dwServiceType = serviceType_;
    serviceStatus_.dwCurrentState = SERVICE_START_PENDING;
    serviceStatus_.dwWin32ExitCode = NO_ERROR;
    serviceStatus_.dwServiceSpecificExitCode = ERROR_SUCCESS;
    serviceStatus_.dwCheckPoint = 0;
    if( SetServiceStatus(statusHandle_,&serviceStatus_) == 0 ){
      err = GetLastError() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    start();
    serviceStatus_.dwCurrentState = SERVICE_RUNNING;
    if( SetServiceStatus(statusHandle_,&serviceStatus_) == 0 ){
      err = GetLastError() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
/*    MSG msg;
    for(;;){
      if( PeekMessage(&msg,NULL,0,0,PM_REMOVE) != 0 ){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      if( msg.message == WM_QUIT ) break;
      if( WaitMessage() == 0 ){
        long err = GetLastError() + errorOffset;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
    }*/
    semaphore_.wait();
    serviceStatus_.dwCurrentState = SERVICE_STOP_PENDING;
    if( SetServiceStatus(statusHandle_,&serviceStatus_) == 0 ){
      err = GetLastError() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    stop();
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    serviceStatus_.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
    serviceStatus_.dwServiceSpecificExitCode = e->code() >= errorOffset ? e->code() - errorOffset : e->code();
  }
  catch( ... ){
    serviceStatus_.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
    serviceStatus_.dwServiceSpecificExitCode = DWORD(-1);
  }
  serviceStatus_.dwCurrentState = SERVICE_STOPPED;
  BOOL r = SetServiceStatus(statusHandle_,&serviceStatus_);
  assert( r != 0 );
  if( r == 0 ) abort();
  return *this;
}
//---------------------------------------------------------------------------
Service & Service::handler(uintptr_t /*n*/,DWORD fdwControl)
{
  if( fdwControl == SERVICE_CONTROL_STOP ) semaphore_.post();
  return *this;
}
//---------------------------------------------------------------------------
Service & Service::install(SC_HANDLE hSCManager)
{
  install();
  utf8::WideString serviceNameW(serviceName_.getUNICODEString());
  utf8::WideString displayNameW(displayName_.getUNICODEString());
  utf8::WideString binaryPathNameW((args_.strlen() > 0 ? binaryPathName_ + args_ : binaryPathName_).getUNICODEString());
  utf8::WideString loadOrderGroupW(loadOrderGroup_.getUNICODEString());
  utf8::WideString dependenciesW(dependencies_.getUNICODEString());
  utf8::WideString serviceStartNameW(serviceStartName_.getUNICODEString());
  utf8::WideString passwordW(password_.getUNICODEString());
  SC_HANDLE handle = CreateServiceW(
    hSCManager,
    serviceNameW,
    displayNameW,
    SERVICE_ALL_ACCESS,
    serviceType_,
    startType_,
    errorControl_,
    binaryPathNameW,
    loadOrderGroup_.strlen() > 0 ? (wchar_t *) loadOrderGroupW : NULL,
    (serviceType_ == SERVICE_KERNEL_DRIVER || serviceType_ == SERVICE_FILE_SYSTEM_DRIVER) &&
    (startType_ == SERVICE_BOOT_START || startType_ == SERVICE_SYSTEM_START) ?
    &tagId_ : NULL,
    dependencies_.strlen() > 0 ? (wchar_t *) dependenciesW : NULL,
    serviceStartName_.strlen() > 0 ? (wchar_t *) serviceStartNameW : NULL,
    password_.strlen() > 0 ? (wchar_t *) passwordW : NULL
  );
  if( handle == NULL ){
    int32_t err = GetLastError();
    if( err == ERROR_SERVICE_EXISTS ){
      SetLastError(err = ERROR_SUCCESS);
      handle = OpenServiceW(hSCManager,serviceNameW,SERVICE_ALL_ACCESS);
      if( handle == NULL ||
          ChangeServiceConfigW(handle,
            serviceType_,
            startType_,
            errorControl_,
            binaryPathNameW,
            loadOrderGroup_.strlen() > 0 ? (wchar_t *) loadOrderGroupW : NULL,
            (serviceType_ == SERVICE_KERNEL_DRIVER || serviceType_ == SERVICE_FILE_SYSTEM_DRIVER) &&
            (startType_ == SERVICE_BOOT_START || startType_ == SERVICE_SYSTEM_START) ?
            &tagId_ : NULL,
            dependencies_.strlen() > 0 ? (wchar_t *) dependenciesW : NULL,
            serviceStartName_.strlen() > 0 ? (wchar_t *) serviceStartNameW : NULL,
            password_.strlen() > 0 ? (wchar_t *) passwordW : NULL,
            displayNameW
      ) == 0 ) err = GetLastError();
    }
    if( err != ERROR_SUCCESS ){
      if( handle != NULL ) CloseServiceHandle(handle);
      newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__)->throwSP();
    }
  }
  CloseServiceHandle(handle);
  return *this;
}
//---------------------------------------------------------------------------
Service & Service::uninstall(SC_HANDLE hSCManager)
{
  uninstall();
  utf8::WideString serviceNameW(serviceName_.getUNICODEString());
  SC_HANDLE handle = OpenServiceW(
    hSCManager,
    serviceNameW,
    SERVICE_ALL_ACCESS
  );
  if( handle == NULL ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( DeleteService(handle) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    CloseServiceHandle(handle);
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  CloseServiceHandle(handle);
  return *this;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
void Service::install()
{
}
//---------------------------------------------------------------------------
void Service::uninstall()
{
}
//---------------------------------------------------------------------------
void Service::start()
{
  newObjectV1C2<Exception>(ENOSYS,"service " + serviceName_ + " not implemented start")->throwSP();
}
//---------------------------------------------------------------------------
void Service::stop()
{
  newObjectV1C2<Exception>(ENOSYS,"service " + serviceName_ + " not implemented stop")->throwSP();
}
//---------------------------------------------------------------------------
void Service::suspend()
{
  newObjectV1C2<Exception>(ENOSYS,"service " + serviceName_ + " not support suspending")->throwSP();
}
//---------------------------------------------------------------------------
void Service::resume()
{
  newObjectV1C2<Exception>(ENOSYS,"service " + serviceName_ + " not support resuming")->throwSP();
}
//---------------------------------------------------------------------------
utf8::String Service::status()
{
#if !defined(__WIN32__) && !defined(__WIN64__)
  switch( status_ ){
    case svcReady      : return "ready for start";
    case svcStarting   : return "starting";
    case svcStarted    : return "started";
    case svcSuspending : return "suspending";
    case svcSuspended  : return "suspended";
    case svcResuming   : return "resuming";
    case svcResumed    : return "resumed";
    case svcStoping    : return "stoping";
    case svcStoped     : return "stoped";
    default            :;
  }
#endif
  newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
  return "";
}
//---------------------------------------------------------------------------
bool Service::active()
{
  return status_ == svcStarted || status_ == svcResumed;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
VOID (WINAPI * const Services::serviceMains_[])(DWORD,LPWSTR *) = {
  serviceMainTemplate<0>,
  serviceMainTemplate<1>,
  serviceMainTemplate<2>,
  serviceMainTemplate<3>
};
VOID (WINAPI * const Services::handlers_[])(DWORD) = {
  handlerTemplate<0>,
  handlerTemplate<1>,
  handlerTemplate<2>,
  handlerTemplate<3>
};
Service * Services::services_[sizeof(serviceMains_) / sizeof(serviceMains_[0])] = {
  NULL, NULL, NULL, NULL
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
Services::~Services()
{
  clear();
#if defined(__WIN32__) || defined(__WIN64__)
  CloseServiceHandle(handle_);
#endif
}
//---------------------------------------------------------------------------
Services::Services(const utf8::String &
#if defined(__WIN32__) || defined(__WIN64__)
 )
#else
 name) : controlName_(name)
#endif
{
#if defined(__WIN32__) || defined(__WIN64__)
  handle_ = NULL;
  serviceStartTable_.resize(sizeof(services_) / sizeof(services_[0]) + 1);
  handle_ = OpenSCManagerW(NULL,NULL,SC_MANAGER_ALL_ACCESS | GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE);
  if( handle_ == NULL ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#endif
}
//---------------------------------------------------------------------------
Services & Services::clear()
{
#if defined(__WIN32__) || defined(__WIN64__)
  for( intptr_t i = sizeof(services_) / sizeof(services_[0]) - 1; i >= 0; i-- ){
/*    try {
      services_[i]->resume();
    }
    catch( ExceptionSP & e ){
      if( e->code() != ENOSYS ) throw;
    }
    services_[i]->stop();*/
    delete services_[i];
    services_[i] = NULL;
  }
#else
  for( intptr_t i = services_.count() - 1; i >= 0; i-- ){
    try {
      services_[i].resume();
    }
    catch( ExceptionSP & e ){
      if( e->code() != ENOSYS ) throw;
    }
    if( services_[i].active() ) services_[i].stop();
  }
  services_.clear();
#endif
  return *this;
}
//---------------------------------------------------------------------------
Services & Services::add(Service * service)
{
#if defined(__WIN32__) || defined(__WIN64__)
  for( uintptr_t i = 0; i < sizeof(services_) / sizeof(services_[0]); i++ )
    if( services_[i] == NULL ){
      services_[i] = service;
      break;
    }
#else
  services_.add(service);
#endif
  return *this;
}
//---------------------------------------------------------------------------
Services & Services::install()
{
#if defined(__WIN32__) || defined(__WIN64__)
  uintptr_t i;
  for( i = 0; i < sizeof(services_) / sizeof(services_[0]); i++ ){
    if( services_[i] == NULL ) continue;
    services_[i]->install(handle_);
  }
#endif
  return *this;
}
//---------------------------------------------------------------------------
Services & Services::uninstall()
{
#if defined(__WIN32__) || defined(__WIN64__)
  uintptr_t i;
  for( i = 0; i < sizeof(services_) / sizeof(services_[0]); i++ ){
    if( services_[i] == NULL ) continue;
    services_[i]->uninstall(handle_);
  }
#endif
  return *this;
}
//---------------------------------------------------------------------------
Service & Services::serviceByName(const utf8::String & serviceName)
{
  intptr_t i =
#if defined(__WIN32__) || defined(__WIN64__)
    sizeof(services_) / sizeof(services_[0]);
  while( --i >= 0 )
    if( services_[i]->serviceName_.strcasecmp(serviceName) == 0 )
      return *services_[i];
#else
    services_.count();
  while( --i >= 0 )
    if( services_[i].serviceName_.strcasecmp(serviceName) == 0 )
      return services_[i];
#endif
  newObjectV1C2<Exception>(EINVAL,"Unknown service")->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
Services & Services::start(const utf8::String &
#if !defined(__WIN32__) && !defined(__WIN64__)
 serviceName
#endif
)
{
#if !defined(__WIN32__) && !defined(__WIN64__)
  control_ = newObjectC1<SharedMemoryQueue>(controlName_);
  if( control_->creator() )
    newObjectV1C2<Exception>(EAGAIN,"dispatcher not started")->throwSP();
  int32_t err;
  utf8::String error;
  control_->swap() << SMQ_WRL << svcStart << serviceName;
  control_->ref() << SMQ_RDL >> err;
  if( err != 0 ) control_->ref() >> error;
  control_->ref() << SMQ_RDU << SMQ_WRU;
  if( err != 0 ){
#ifndef NDEBUG
    fprintf(stderr,"%s\n",(const char *) error.getANSIString());
#endif
    newObjectV1C2<Exception>(err,error)->throwSP();
  }
#ifndef NDEBUG
  fprintf(stderr,"%s\n",__PRETTY_FUNCTION__);
#endif
#endif
  return *this;
}
//---------------------------------------------------------------------------
Services & Services::stop(const utf8::String &
#if !defined(__WIN32__) && !defined(__WIN64__)
 serviceName
#endif
)
{
#if !defined(__WIN32__) && !defined(__WIN64__)
  control_ = newObjectC1<SharedMemoryQueue>(controlName_);
  if( control_->creator() )
    newObjectV1C2<Exception>(EAGAIN,"dispatcher not started")->throwSP();
  int32_t err;
  utf8::String error;
  control_->swap() << SMQ_WRL << svcStop << serviceName;
  control_->ref() << SMQ_RDL >> err;
  if( err != 0 ) control_->ref() >> error;
  control_->ref() << SMQ_RDU << SMQ_WRU;
  if( err != 0 ){
#ifndef NDEBUG
    fprintf(stderr,"%s\n",(const char *) error.getANSIString());
#endif
    newObjectV1C2<Exception>(err,error)->throwSP();
  }
#ifndef NDEBUG
  fprintf(stderr,"%s\n",__PRETTY_FUNCTION__);
#endif
#endif
  return *this;
}
//---------------------------------------------------------------------------
Services & Services::suspend(const utf8::String &
#if !defined(__WIN32__) && !defined(__WIN64__)
 serviceName
#endif
)
{
#if !defined(__WIN32__) && !defined(__WIN64__)
  control_ = newObjectC1<SharedMemoryQueue>(controlName_);
  if( control_->creator() )
    newObjectV1C2<Exception>(EAGAIN,"dispatcher not started")->throwSP();
  int32_t err;
  utf8::String error;
  control_->swap() << SMQ_WRL << svcSuspend << serviceName;
  control_->ref() << SMQ_RDL >> err;
  if( err != 0 ) control_->ref() >> error;
  control_->ref() << SMQ_RDU << SMQ_WRU;
  if( err != 0 ){
#ifndef NDEBUG
    fprintf(stderr,"%s\n",(const char *) error.getANSIString());
#endif
    newObjectV1C2<Exception>(err,error)->throwSP();
  }
#ifndef NDEBUG
  fprintf(stderr,"%s\n",__PRETTY_FUNCTION__);
#endif
#endif
  return *this;
}
//---------------------------------------------------------------------------
Services & Services::resume(const utf8::String &
#if !defined(__WIN32__) && !defined(__WIN64__)
 serviceName
#endif
)
{
#if !defined(__WIN32__) && !defined(__WIN64__)
  control_ = newObjectC1<SharedMemoryQueue>(controlName_);
  if( control_->creator() )
    newObjectV1C2<Exception>(EAGAIN,"dispatcher not started")->throwSP();
  int32_t err;
  utf8::String error;
  control_->swap() << SMQ_WRL << svcResume << serviceName;
  control_->ref() << SMQ_RDL >> err;
  if( err != 0 ) control_->ref() >> error;
  control_->ref() << SMQ_RDU << SMQ_WRU;
  if( err != 0 ){
#ifndef NDEBUG
    fprintf(stderr,"%s\n",(const char *) error.getANSIString());
#endif
    newObjectV1C2<Exception>(err,error)->throwSP();
  }
#ifndef NDEBUG
  fprintf(stderr,"%s\n",__PRETTY_FUNCTION__);
#endif
#endif
  return *this;
}
//---------------------------------------------------------------------------
Services & Services::query(const utf8::String &
#if !defined(__WIN32__) && !defined(__WIN64__)
 serviceName
#endif
)
{
#if !defined(__WIN32__) && !defined(__WIN64__)
  control_ = newObjectC1<SharedMemoryQueue>(controlName_);
  if( control_->creator() )
    newObjectV1C2<Exception>(EAGAIN,"dispatcher not started")->throwSP();
  utf8::String sn(serviceName.strcasecmp("all") == 0 ? utf8::String() : serviceName);
  int32_t err;
  utf8::String error;
  control_->swap() << SMQ_WRL << svcQuery << sn;
  control_->ref() << SMQ_RDL >> err;
  if( err == 0 ){
    uintptr_t count;
    control_->ref() >> count;
    while( count > 0 ){
      utf8::String name, status;
      control_->ref() >> name >> status;
      fprintf(stderr,"%s: %s\n",
        (const char *) name.getANSIString(),
        (const char *) status.getANSIString()
      );
      count--;
    }
  }
  else {
    control_->ref() >> error;
  }
  control_->ref() << SMQ_RDU << SMQ_WRU;
  if( err != 0 ){
#ifndef NDEBUG
    fprintf(stderr,"%s\n",(const char *) error.getANSIString());
#endif
    newObjectV1C2<Exception>(err,error)->throwSP();
  }
#ifndef NDEBUG
  fprintf(stderr,"%s\n",__PRETTY_FUNCTION__);
#endif
#endif
  return *this;
}
//---------------------------------------------------------------------------
Services & Services::stopServiceCtrlDispatcher()
{
#if !defined(__WIN32__) && !defined(__WIN64__)
  control_ = newObjectC1<SharedMemoryQueue>(controlName_);
  if( control_->creator() )
    newObjectV1C2<Exception>(EAGAIN,"dispatcher not started")->throwSP();
  int32_t err;
  utf8::String error;
  control_->swap() << SMQ_WRL << svcStopDispatcher << utf8::String();
  control_->ref() << SMQ_RDL >> err;
  if( err != 0 ) control_->ref() >> error;
  control_->ref() << SMQ_RDU << SMQ_WRU;
  if( err != 0 ){
#ifndef NDEBUG
    fprintf(stderr,"%s\n",(const char *) error.getANSIString());
#endif
    newObjectV1C2<Exception>(err,error)->throwSP();
  }
#ifndef NDEBUG
  fprintf(stderr,"%s\n",__PRETTY_FUNCTION__);
#endif
#endif
  return *this;
}
//---------------------------------------------------------------------------
Services & Services::startServiceCtrlDispatcher(
#if defined(__WIN32__) || defined(__WIN64__)
  bool,bool
#else
  bool daemonize, bool startServices
#endif
)
{
#if defined(__WIN32__) || defined(__WIN64__)
  uintptr_t i;
  for( i = 0; i < sizeof(services_) / sizeof(services_[0]); i++ ){
    if( services_[i] == NULL ) break;
    services_[i]->serviceNameW_ = services_[i]->serviceName_.getUNICODEString();
    serviceStartTable_[i].lpServiceName = services_[i]->serviceNameW_;
    serviceStartTable_[i].lpServiceProc = serviceMains_[i];
  }
  serviceStartTable_[i].lpServiceName = NULL;
  serviceStartTable_[i].lpServiceProc = NULL;
  if( StartServiceCtrlDispatcherW(serviceStartTable_.ptr()) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#elif HAVE_DAEMON
  if( daemonize ){
    int r = daemon(1,1);
    if( r < 0 ){
      int32_t err = errno;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__);
    }
    if( r > 0 ) exit(EX_OK); // if parent then immediate exit
  }
  SharedMemoryQueue::unlink(controlName_);
  control_ = newObjectC1<SharedMemoryQueue>(controlName_);
  if( !control_->creator() )
    newObjectV1C2<Exception>(EAGAIN,__PRETTY_FUNCTION__)->throwSP();
  Thread::resume();
  wait();
#else
#error unsupported daemonizing interface
#endif
  return *this;
}
//---------------------------------------------------------------------------
#if !defined(__WIN32__) && !defined(__WIN64__)
//---------------------------------------------------------------------------
void Services::threadExecute()
{
#ifndef NDEBUG
  fprintf(stderr,"services dispatcher started\n");
#endif
  int32_t err;
  utf8::String error;
  intptr_t i, j;
  for( j = 0, i = services_.count() - 1; i >= 0; i-- ){
    try {
      services_[i].start();
      j++;
    }
    catch( ExceptionSP & e ){
      e->writeStdError();
    }
    catch( ... ){
#ifndef NDEBUG
      fprintf(stderr,"unexpected error while initial starting services\n");
#endif
    }
  }
  if( j == 0 ) terminate();
  while( !terminated_ ){
    ServiceDispatcherCmd cmd;
    utf8::String name;
    control_->ref() << SMQ_RDL >> cmd >> name << SMQ_RDU;
    err = 0;
    try {
      if( cmd == svcStart ){
#ifndef NDEBUG
        fprintf(stderr,"starting service %s\n",(const char *) name.getANSIString());
#endif
        Service & service = serviceByName(name);
	if( service.status_ == svcReady || service.status_ == svcStoped ){
  	  try {
	    service.status_ = svcStarting;
	    service.start();
	  }
	  catch( ... ){
            service.status_ = svcReady;
	    throw;
	  }
          service.status_ = svcStarted;
	}
	else {
	  err = EAGAIN;
	  error = "service is not ready or already started";
	}
      }
      else if( cmd == svcStop ){
#ifndef NDEBUG
        fprintf(stderr,"stopping service %s\n",(const char *) name.getANSIString());
#endif
        Service & service = serviceByName(name);
	if( service.status_ == svcStarted || service.status_ == svcResumed ){
  	  try {
	    service.status_ = svcStoping;
	    service.stop();
	  }
	  catch( ... ){
            service.status_ = svcStarted;
	    throw;
	  }
          service.status_ = svcStoped;
	}
	else {
	  err = EAGAIN;
	  error = "service is not started or resumed";
	}
      }
      else if( cmd == svcSuspend ){
#ifndef NDEBUG
        fprintf(stderr,"suspending service %s\n",(const char *) name.getANSIString());
#endif
        Service & service = serviceByName(name);
	if( service.status_ == svcStarted || service.status_ == svcResumed ){
  	  try {
	    service.status_ = svcSuspending;
	    service.suspend();
	  }
	  catch( ... ){
            service.status_ = svcStarted;
	    throw;
	  }
          service.status_ = svcSuspended;
	}
	else {
	  err = EAGAIN;
	  error = "service is not started or resumed";
	}
      }
      else if( cmd == svcResume ){
#ifndef NDEBUG
        fprintf(stderr,"resuming service %s\n",(const char *) name.getANSIString());
#endif
        Service & service = serviceByName(name);
	if( service.status_ == svcSuspended ){
  	  try {
	    service.status_ = svcResuming;
	    service.suspend();
	  }
	  catch( ... ){
            service.status_ = svcResumed;
	    throw;
	  }
          service.status_ = svcSuspended;
	}
	else {
	  err = EAGAIN;
	  error = "service is not started or suspended";
	}
      }
      else if( cmd == svcQuery ){
        if( name.strlen() > 0 ){
	  Service & service = serviceByName(name);
          control_->ref() << SMQ_WRL << err << uintptr_t(1) << name;
	  try {
	    if( (service.status_ == svcStarted || 
	         service.status_ == svcResumed) && !service.active() ){
	      service.status_ = svcStoped;
	    }
            control_->ref() << service.status();
	  }
	  catch( ... ){
  	    control_->ref() << SMQ_WRU;
	    throw;
	  }
          control_->ref() << SMQ_WRU;
	}
	else {
          control_->ref() << SMQ_WRL << err << services_.count();
	  for( uintptr_t i = 0; services_.count(); i++ ){
	    control_->ref() << services_[i].serviceName_;
	    try {
	      control_->ref() << services_[i].status();
	    }
	    catch( ... ){
	      control_->ref() << SMQ_WRU;
	    }
	  }
	  control_->ref() << SMQ_WRU;
	}
	continue;
      }
      else if( cmd == svcStopDispatcher ){
#ifndef NDEBUG
        fprintf(stderr,"stopping services dispatcher\n");
#endif
        terminate();
      }
      else {
        newObjectV1C2<Exception>(EINVAL,"Unknown command")->throwSP();
      }
    }
    catch( ExceptionSP & e ){
      e->writeStdError();
      err = e->code();
      error = e->what();
    }
    catch( ... ){
      err = EFAULT;
      error = "Unknown error";
    }
    control_->ref() << SMQ_WRL << err;
    if( err != 0 ) control_->ref() << error;
    control_->ref() << SMQ_WRU;
  }
#ifndef NDEBUG
  fprintf(stderr,"services dispatcher stopped\n");
#endif
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
