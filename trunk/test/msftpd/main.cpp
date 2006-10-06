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
    int8_t cmd_;

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
    MSFTPServerFiber & getFileHash();
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
  bool isSt = mst.stat(file);
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
  file.createIfNotExist(true);
  uint64_t l, lp, ll, pos, r;
  int64_t mtime;
  uint64_t bs;
  *this >> pos >> ll >> mtime >> bs;
  if( cmd_ == cmPutFilePartial ) *this >> bs;
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
    e->writeStdError();
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
  if( cmd_ != cmPutFilePartial ){
    if( bs > ll ) bs = (uint32_t) ll; else if( bs == 0 ) bs = (uint32_t) ll;
    if( bs > 0x40000000 ) bs = 0x40000000;
  }
  b.alloc((size_t) bs);
  ksys::SHA256 lhash, rhash;
  memset(lhash.sha256(),0,lhash.size());
  memset(rhash.sha256(),1,rhash.size());
  for( l = 0; l < ll || (ll > 0 && cmd_ == cmPutFilePartial); l += lp ){
    lp = ll - l > bs ? bs : ll - l;
    if( cmd_ == cmPutFilePartial ){
      *this >> l >> lp;
      if( l == 0 && lp == 0 ) break;
      r = file.size();
      if( l < r ){
        r = l + lp > r ? r - l : lp;
        file.readBuffer(l,b.ptr(),r);
      }
      else {
        r = 0;
      }
      if( r < bs ) memset(b.ptr() + r,0,(size_t) (bs - r));
      lhash.make(b,(uintptr_t) lp);
      readBuffer(rhash.sha256(),rhash.size());
      writeBuffer(lhash.sha256(),lhash.size());
    }
    err = eOK;
    if( memcmp(lhash.sha256(),rhash.sha256(),rhash.size()) != 0 ){
      read(b.ptr(),lp);
      try {
        if( file.size() < l ) file.resize(l);
        file.writeBuffer(l,b.ptr(),lp);
      }
      catch( ksys::ExceptionSP & e ){
        e->writeStdError();
#if defined(__WIN32__) || defined(__WIN64__)
        if( e->code() != ERROR_DISK_FULL + ksys::errorOffset ) throw;
#else
        if( e->code() != ENOSPC ) throw;
#endif
        err = eDiskFull;
      }
      putCode(err);
    }
    if( err != eOK || terminated() ) return *this;
  }
  if( cmd_ != cmPutFilePartial && l != ll ) return putCode(eFileWrite);
  file.close();
  MSFTPStat mst;
  if( !mst.stat(file.fileName()) ) return putCode(eFileStat);
  struct utimbuf ut;
  ut.actime = (time_t) mst.st_atime_;
  ut.modtime = (time_t) mtime;
  return putCode(ksys::utime(file.fileName(),ut) ? eOK : eSetTimes);
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::get()
{
  utf8::String name;
  uint64_t l, ll, lp, bs = config_->value("buffer_size",getpagesize());
  *this >> name >> l >> ll;
  if( cmd_ == cmGetFilePartial ) *this >> bs;
  ksys::AsyncFile file(name);
  file.readOnly(true);
  try {
    file.open();
  }
  catch( ksys::ExceptionSP & e ){
    e->writeStdError();
  }
  putCode(file.isOpen() ? eOK : eFileOpen);
  if( file.isOpen() ){
    file.seek(l);
    ksys::AutoPtr<uint8_t> b;
    b.alloc((size_t) bs);
    ksys::SHA256 lhash, rhash;
    memset(lhash.sha256(),0,lhash.size());
    memset(rhash.sha256(),1,rhash.size());
    for( l = 0; l < ll; l += lp ){
      file.readBuffer(b,lp = ll - l > bs ? bs : ll - l);
      if( cmd_ == cmGetFilePartial ){
        lhash.make(b,(uintptr_t) lp);
        *this << l << lp;
        writeBuffer(lhash.sha256(),lhash.size());
        readBuffer(rhash.sha256(),rhash.size());
      }
      if( memcmp(lhash.sha256(),rhash.sha256(),rhash.size()) != 0 ) write(b.ptr(),lp);
    }
    if( cmd_ == cmPutFilePartial && ll > 0 ) *this << uint64_t(0) << uint64_t(0);
  }
  flush();
  return *this;
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::list()
{
  utf8::String localPath, exclude;
  uint8_t recursive;
  *this >> localPath >> exclude >> recursive;
  ksys::Vector<utf8::String> list;
  getDirList(list,localPath,exclude,recursive != 0,false,true);
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
MSFTPServerFiber & MSFTPServerFiber::getFileHash()
{
  utf8::String name;
  uint64_t l, ll, lp, bs, partialBlockSize;
  *this >> name >> partialBlockSize;
  if( partialBlockSize < getpagesize() ) partialBlockSize = getpagesize();
  if( partialBlockSize > 4u * 1024u * 1024u ) partialBlockSize = 4u * 1024u * 1024u;
  ksys::AsyncFile file(name);
  if( file.tryOpen() ){
    lp = file.size();
    ll = partialBlockSize /*partialBlockSize(lp)*/;
    bs = ll;
    l = lp / bs;
    l += lp > 0 && lp % bs != 0;
  }
  else {
    l = ll = 0;
  }
  *this << ll << l;
  if( file.isOpen() ){
    ksys::SHA256 hash;
    ksys::AutoPtr<uint8_t> b;
    b.alloc((size_t) bs);
    ll = lp;
    for( l = 0; l < ll; l += lp ){
      file.readBuffer(b,lp = ll - l > bs ? bs : ll - l);
      hash.make(b,(uintptr_t) lp);
      write(hash.sha256(),hash.size());
    }
  }
  putCode(eOK);
  return *this;
}
//------------------------------------------------------------------------------
void MSFTPServerFiber::main()
{
  config_->parse().override();
  ksys::stdErr.rotationThreshold(
    config_->value("debug_file_rotate_threshold",1024 * 1024)
  );
  ksys::stdErr.rotatedFileCount(
    config_->value("debug_file_rotate_count",10)
  );
  ksys::stdErr.setDebugLevels(
    config_->value("debug_levels","+0,+1,+2,+3")
  );
  ksys::stdErr.fileName(
    config_->value("log_file",ksys::stdErr.fileName())
  );
  ksys::stdErr.setRedirect(
    config_->value("log_redirect",utf8::String())
  );

  auth();
  while( !terminated() ){
    *this >> cmd_;
    if( cmd_ == cmQuit ){
      putCode(eOK);
      break;
    }
    else if( cmd_ == cmList ){
      list();
    }
    else if( cmd_ == cmStat ){
      stat();
    }
    else if( cmd_ == cmSetTimes ){
      setTimes();
    }
    else if( cmd_ == cmResize ){
      resize();
    }
    else if( cmd_ == cmPutFile || cmd_ == cmPutFilePartial ){
      put();
    }
    else if( cmd_ == cmGetFile || cmd_ == cmGetFilePartial ){
      get();
    }
    else if( cmd_ == cmGetFileHash ){
      getFileHash();
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
  msftpConfig_->parse().override();
  serviceName_ = msftpConfig_->value("service_name","msftp");
  displayName_ = msftpConfig_->value("service_display_name","Macroscope FTP Service");
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
  msftpConfig_->parse().override();
  ksys::Array<ksock::SockAddr> addrs;
  ksock::SockAddr::resolve(
    msftpConfig_->text("bind"),
    addrs,
    MSFTPDefaultPort
  );
  for( intptr_t i = addrs.count() - 1; i >= 0; i-- ) msftp_.addBind(addrs[i]);
  msftp_.open();
  ksys::stdErr.debug(0,
    utf8::String::Stream() << msftpd_version.gnu_ << " started\n"
  );
}
//------------------------------------------------------------------------------
void MSFTPService::stop()
{
  msftp_.close();
  ksys::stdErr.debug(0,
    utf8::String::Stream() << msftpd_version.gnu_ << " stopped\n"
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
    ksys::Services services(msftpd_version.gnu_);
    services.add(newObject<MSFTPService>());
#if defined(__WIN32__) || defined(__WIN64__)
    bool dispatch = true;
#else
    bool dispatch = false;
#endif
    for( u = 1; u < ksys::argv().count(); u++ ){
      if( ksys::argv()[u].strcmp("--version") == 0 ){
        ksys::stdErr.debug(9,utf8::String::Stream() << msftpd_version.tex_ << "\n");
        fprintf(stdout,"%s\n",msftpd_version.tex_);
        dispatch = false;
        continue;
      }
      if( ksys::argv()[u].strcmp("-c") == 0 && u + 1 < ksys::argv().count() ){
        ksys::Config::defaultFileName(ksys::argv()[u + 1]);
      }
      else if( ksys::argv()[u].strcmp("--log") == 0 && u + 1 < ksys::argv().count() ){
        ksys::stdErr.fileName(ksys::argv()[u + 1]);
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

