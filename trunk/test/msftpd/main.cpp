/*-
 * Copyright 2006 Guram Dukashvili
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
#include "msftpd.h"
//------------------------------------------------------------------------------
#define _VERSION_C_AS_HEADER_
#include "version.c"
#undef _VERSION_C_AS_HEADER_
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class MSFTPServerFiber : public ksock::ServerFiber {
  public:
    virtual ~MSFTPServerFiber();
    MSFTPServerFiber(const ksys::ConfigSP config);
  protected:
    void main();
  private:
    ksys::ConfigSP config_;
    utf8::String workDir_;
    utf8::String user_;

    bool isValidUser(const utf8::String & user);
    utf8::String getUserPassword(const utf8::String & user);

    MSFTPServerFiber & putCode(int32_t code);
    MSFTPServerFiber & auth();
    MSFTPServerFiber & stat();
    MSFTPServerFiber & setTimes();
    MSFTPServerFiber & resize();
    MSFTPServerFiber & put();
    MSFTPServerFiber & get();
    MSFTPServerFiber & list();
};
//------------------------------------------------------------------------------
MSFTPServerFiber::~MSFTPServerFiber()
{
}
//------------------------------------------------------------------------------
MSFTPServerFiber::MSFTPServerFiber(const ksys::ConfigSP config) :
  config_(config),
  workDir_(ksys::includeTrailingPathDelimiter(ksys::getCurrentDir()))
{
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::putCode(int32_t code)
{
  *this << int32_t(code);
  return *this;
}
//------------------------------------------------------------------------------
bool MSFTPServerFiber::isValidUser(const utf8::String & user)
{
  return config_->parse().override().section("users").isSection(user);
}
//------------------------------------------------------------------------------
utf8::String MSFTPServerFiber::getUserPassword(const utf8::String & user)
{
  return config_->section("users").section(user).text("password");
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::auth()
{
  maxSendSize(config_->value("max_send_size",-1));
  utf8::String encryption(config_->section("encryption").text(utf8::String(),"default"));
  uintptr_t encryptionThreshold = config_->section("encryption").value("threshold",1024 * 1024);
  utf8::String compression(config_->section("compression").text(utf8::String(),"default"));
  utf8::String compressionType(config_->section("compression").text("type","default"));
  utf8::String crc(config_->section("compression").text("crc","default"));
  uintptr_t compressionLevel = config_->section("compression").value("max_level",3);
  bool optimize = config_->section("compression").value("optimize",false);
  uintptr_t bufferSize = config_->section("compression").value("buffer_size",getpagesize());
  MSFTPError e = (MSFTPError) serverAuth(
    encryption,
    encryptionThreshold,
    compression,
    compressionType,
    crc,
    compressionLevel,
    optimize,
    bufferSize
  );
  if( e != eOK )
    ksys::Exception::throwSP(e,__PRETTY_FUNCTION__);
  return *this;
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::stat()
{
  MSFTPStat mst;
  utf8::String file(ksys::absolutePathNameFromWorkDir(workDir_,readString()));
  bool isSt = mst.statAsync(file);
  write(&mst,sizeof(mst));
  return putCode(isSt ? eOK : eBadPathName);
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::setTimes()
{
  utf8::String file(ksys::absolutePathNameFromWorkDir(workDir_,readString()));
  MSFTPStat mst;
  read(&mst,sizeof(mst));
  struct utimbuf ut;
  ut.actime = (time_t) mst.st_atime_;
  ut.modtime = (time_t) mst.st_mtime_;
  return putCode(ksys::utime(file,ut) ? eOK : eSetTimes);
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::resize()
{
  ksys::AsyncFile file(ksys::absolutePathNameFromWorkDir(workDir_,readString()));
  int64_t l;
  *this >> l;
  if( file.tryOpen() ){
    file.resize(l);
  }
  return putCode(file.isOpen() ? eOK : eResize);
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::put()
{
  ksys::AsyncFile file(ksys::absolutePathNameFromWorkDir(workDir_,readString()));
  uint64_t l, lp, ll, pos;
  int64_t mtime;
  uint64_t bs;
  *this >> pos >> ll >> mtime >> bs;
  MSFTPError err = eOK;
  try {
    if( !file.tryOpen() ){
      try {
        ksys::createDirectory(
          ksys::excludeTrailingPathDelimiter(
            ksys::getPathFromPathName(file.fileName())
          )
        );
        try {
          file.open();
        }
        catch( ... ){
          err = eFileOpen;
        }
      }
      catch( ... ){
        err = eMkDir;
      }
    }
  }
  catch( ksys::ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( e->code() != ERROR_DISK_FULL + ksys::errorOffset ) throw;
#else
    if( e->code() != ENOSPC ) throw;
#endif
    err = eDiskFull;
  }
  putCode(err);
  if( err != eOK ) return *this;
  if( file.size() < pos ) file.resize(pos);
  file.seek(pos);
  ksys::AutoPtr<uint8_t> b;
  if( bs > ll ) bs = (uint32_t) ll; else if( bs == 0 ) bs = (uint32_t) ll;
  if( bs > 0x40000000 ) bs = 0x40000000;
  b.alloc((size_t) bs);
  for( l = 0; l < ll; l += lp ){
    read(b.ptr(),lp = ll - l > bs ? bs : ll - l);
    err = eOK;
    try {
      file.writeBuffer(b.ptr(),lp);
    }
    catch( ksys::ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
      if( e->code() != ERROR_DISK_FULL + ksys::errorOffset ) throw;
#else
      if( e->code() != ENOSPC ) throw;
#endif
      err = eDiskFull;
    }
    putCode(err);
    if( err != eOK || terminated() ) return *this;
  }
  if( l != ll ) return putCode(eFileWrite);
  file.close();
  MSFTPStat mst;
  if( !mst.statAsync(file.fileName()) ) return putCode(eFileStat);
  struct utimbuf ut;
  ut.actime = (time_t) mst.st_atime_;
  ut.modtime = (time_t) mtime;
  return putCode(ksys::utime(file.fileName(),ut) ? eOK : eSetTimes);
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::get()
{
  utf8::String name;
  uint64_t l, ll, lp;
  *this >> name >> l >> ll;
  ksys::AsyncFile file(name);
  putCode(file.tryOpen() ? eOK : eFileOpen);
  if( file.isOpen() ){
    file.seek(l);
    uint64_t bs = (uint64_t) config_->value("buffer_size",getpagesize());
    ksys::AutoPtr<uint8_t> b;
    b.alloc((size_t) bs);
    for( l = 0; l < ll; l += lp ){
      file.readBuffer(b,lp = ll - l > bs ? bs : ll - l);
      write(b.ptr(),lp);
    }
  }
  return *this;
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::list()
{
  utf8::String localPath, exclude;
  uint8_t recursive;
  *this >> localPath >> exclude >> recursive;
  ksys::Vector<utf8::String> list;
  getDirList(list,localPath,exclude,recursive != 0);
  putCode(eOK);
  *this << (uint64_t) list.count();
  uintptr_t l = ksys::includeTrailingPathDelimiter(ksys::getPathFromPathName(localPath)).strlen();
  for( intptr_t i = list.count() - 1; i >= 0; i-- ){
    *this << utf8::String::Iterator(list[i]) + l;
    list.resize(list.count() - 1);
  }
  return *this;
}
//------------------------------------------------------------------------------
void MSFTPServerFiber::main()
{
  auth();
  while( !terminated() ){
    int8_t cmd;
    *this >> cmd;
    if( cmd == cmQuit ){
      putCode(eOK);
      break;
    }
    else if( cmd == cmList ){
      list();
    }
    else if( cmd == cmStat ){
      stat();
    }
    else if( cmd == cmSetTimes ){
      setTimes();
    }
    else if( cmd == cmResize ){
      resize();
    }
    else if( cmd == cmPutFile ){
      put();
    }
    else if( cmd == cmGetFile ){
      get();
    }
    else { // unrecognized or unsupported command, terminate
      break;
    }
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class MSFTPServer : public ksock::Server {
  public:
    virtual ~MSFTPServer();
    MSFTPServer(const ksys::ConfigSP config);
  protected:
    ksys::ConfigSP config_;
    ksys::Fiber * newFiber();
  private:
};
//------------------------------------------------------------------------------
MSFTPServer::~MSFTPServer()
{
}
//------------------------------------------------------------------------------
MSFTPServer::MSFTPServer(const ksys::ConfigSP config) : config_(config)
{
}
//------------------------------------------------------------------------------
ksys::Fiber * MSFTPServer::newFiber()
{
  return newObject<MSFTPServerFiber>(config_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class MSFTPService : public ksys::Service {
  public:
    MSFTPService();
  protected:
  private:
    ksys::ConfigSP msftpConfig_;
    MSFTPServer msftp_;

    void start();
    void stop();
    bool active();
};
//------------------------------------------------------------------------------
MSFTPService::MSFTPService() :
  msftpConfig_(newObject<ksys::InterlockedConfig<ksys::FiberInterlockedMutex> >()),
  msftp_(msftpConfig_)
{
  serviceName_ = "msftp";
  displayName_ = "Macroscope FTP Service";
#if defined(__WIN32__) || defined(__WIN64__)
  serviceType_ = SERVICE_WIN32_OWN_PROCESS;
  startType_ = SERVICE_AUTO_START;
  errorControl_ = SERVICE_ERROR_IGNORE;
  binaryPathName_ = ksys::getExecutableName();
  serviceStatus_.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  serviceStatus_.dwWaitHint = 60000; // give me 60 seconds for start or stop 
#endif
}
//------------------------------------------------------------------------------
void MSFTPService::start()
{
#if defined(__WIN32__) || defined(__WIN64__)
#ifndef NDEBUG
//  Sleep(20000);
//  ksys::Config::defaultFileName(
//    ksys::includeTrailingPathDelimiter(
//      ksys::getPathFromPathName(
//        ksys::getExecutableName())) + "..\\test\\msftpd\\msftpd.conf"
//  );
#endif
  msftpConfig_->parse().override(argv_);
#else
  msftpConfig_->parse().override();
#endif
  ksys::Array<ksock::SockAddr> addrs;
  ksock::SockAddr::resolve(
    msftpConfig_->text("bind"),
    addrs,
    MSFTPDefaultPort
  );
  for( intptr_t i = addrs.count() - 1; i >= 0; i-- ) msftp_.addBind(addrs[i]);
  msftp_.open();
  ksys::stdErr.log(
    ksys::lmINFO,
    utf8::String::Stream() << msftpd_version.v_gnu << " started\n"
  );
}
//------------------------------------------------------------------------------
void MSFTPService::stop()
{
  msftp_.close();
  ksys::stdErr.log(
    ksys::lmINFO,
    utf8::String::Stream() << msftpd_version.v_gnu << " stopped\n"
  );
}
//------------------------------------------------------------------------------
bool MSFTPService::active()
{
  return msftp_.active();
}
//------------------------------------------------------------------------------
int main(int argc,char * argv[])
{
  int errcode = 0;
  adicpp::AutoInitializer autoInitializer;
  autoInitializer = autoInitializer;
  try {
    union {
      intptr_t i;
      uintptr_t u;
    };
    ksys::initializeArguments(argc,argv);
    ksys::Config::defaultFileName(SYSCONF_DIR + "msftpd.conf");
    ksys::Services services(msftpd_version.v_gnu);
    services.add(newObject<MSFTPService>());
#if defined(__WIN32__) || defined(__WIN64__)
    bool dispatch = true;
#else
    bool dispatch = false;
#endif
    for( u = 1; u < ksys::argv().count(); u++ ){
      if( argv()[u].strcmp("--version") == 0 ){
        stdErr.debug(9,utf8::String::Stream() << msftpd_version.tex_ << "\n");
        fprintf(stdout,"%s\n",msftpd_version.tex_);
        dispatch = false;
        continue;
      }
      if( ksys::argv()[u].strcmp("-c") == 0 && u + 1 < ksys::argv().count() ){
        ksys::Config::defaultFileName(ksys::argv()[u + 1]);
      }
      else if( ksys::argv()[u].strcmp("--install") == 0 ){
        services.install();
        dispatch = false;
      }
      else if( ksys::argv()[u].strcmp("--uninstall") == 0 ){
        services.uninstall();
        dispatch = false;
      }
      else if( ksys::argv()[u].strcmp("--start") == 0 && u + 1 < ksys::argv().count() ){
        services.start(ksys::argv()[u + 1]);
        dispatch = false;
      }
      else if( ksys::argv()[u].strcmp("--stop") == 0 && u + 1 < ksys::argv().count() ){
        services.stop(ksys::argv()[u + 1]);
        dispatch = false;
      }
      else if( ksys::argv()[u].strcmp("--suspend") == 0 && u + 1 < ksys::argv().count() ){
        services.suspend(ksys::argv()[u + 1]);
        dispatch = false;
      }
      else if( ksys::argv()[u].strcmp("--resume") == 0 && u + 1 < ksys::argv().count() ){
        services.resume(ksys::argv()[u + 1]);
        dispatch = false;
      }
      else if( ksys::argv()[u].strcmp("--query") == 0 && u + 1 < ksys::argv().count() ){
        services.query(ksys::argv()[u + 1]);
        dispatch = false;
      }
      else if( ksys::argv()[u].strcmp("--start-disp") == 0 ){
        dispatch = true;
      }
      else if( ksys::argv()[u].strcmp("--stop-disp") == 0 ){
        services.stopServiceCtrlDispatcher();
        dispatch = false;
      }
      else if( ksys::argv()[u].strcmp("--sha256") == 0 && u + 1 < ksys::argv().count() ){
        ksys::SHA256 passwordSHA256;
        passwordSHA256.make(ksys::argv()[u + 1].c_str(),ksys::argv()[u + 1].size());
        utf8::String b64(ksys::base64Encode(passwordSHA256.sha256(),32));
        fprintf(stdout,"%s\n",b64.c_str());
        ksys::copyStrToClipboard(b64);
        dispatch = false;
      }
    }
    if( dispatch ){
      bool daemon;
      {
        ksys::ConfigSP config(newObject<ksys::InterlockedConfig<ksys::FiberInterlockedMutex> >());
        daemon = config->value("daemon",false);
      }
      services.startServiceCtrlDispatcher(daemon);
    }
  }
  catch( ksys::ExceptionSP & e ){
    e->writeStdError();
    errcode = e->code();
  }
  catch( ... ){
  }
  return errcode;
}
//------------------------------------------------------------------------------
/*#if defined(__WIN32__) || defined(__WIN64__)
//------------------------------------------------------------------------------
extern "C" int WINAPI WinMain(HINSTANCE hCurInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  int numArgs;
  LPSTR * lpArgv = CommandLineToArgv(lpCmdLine,&numArgs);
  if( lpArgv == NULL ) return GetLastError();
  int r = main(numArgs,lpArgv);
  LocalFree(lpArgv);
  return r;
}
//------------------------------------------------------------------------------
extern "C" int WINAPI wWinMain(HINSTANCE hCurInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
  int numArgs;
  LPWSTR * lpArgv = CommandLineToArgvW(lpCmdLine,&numArgs);
  if( lpArgv == NULL ) return GetLastError();
  int r = main(numArgs,lpArgv);
  LocalFree(lpArgv);
  return r;
}
//------------------------------------------------------------------------------
#endif*/
//------------------------------------------------------------------------------

