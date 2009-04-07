/*-
 * Copyright 2006-2008 Guram Dukashvili
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
using namespace ksys;
//------------------------------------------------------------------------------
class MSFTPService;
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class MSFTPWatchdog : public Fiber {
  public:
    virtual ~MSFTPWatchdog() {}
    MSFTPWatchdog(MSFTPService * msftp = NULL,const utf8::String & section = utf8::String()) :
      msftp_(msftp), section_(section) {}
  protected:
    void fiberExecute();
    void fiberBreakExecution() { dcn_.cancel(); }
  private:
    MSFTPService * msftp_;
    utf8::String section_;
    DirectoryChangeNotification dcn_;
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class MSFTPServerFiber : public ksock::ServerFiber {
  public:
    virtual ~MSFTPServerFiber();
    MSFTPServerFiber(MSFTPService * msftp = NULL);
  protected:
    void main();
  private:
    MSFTPService * msftp_;
    utf8::String workDir_;
    utf8::String user_;
    int8_t cmd_;

    bool isValidUser(const utf8::String & user);
    utf8::String getUserPassword(const utf8::String & user,const AuthParams * ap = NULL);

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
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class MSFTPServer : public ksock::Server {
  public:
    virtual ~MSFTPServer();
    MSFTPServer(MSFTPService * msftp = NULL);
  protected:
    MSFTPService * msftp_;
    Fiber * newFiber();
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class MSFTPService : public Service {
  friend class MSFTPWatchdog;
  friend class MSFTPServerFiber;
  friend class MSFTPServer;
  public:
    MSFTPService() {}
    MSFTPService(int);
    const ConfigSP & msftpConfig() const { return msftpConfig_; }

    void initialize();
    void start();
    void stop();
  protected:
  private:
    ConfigSP msftpConfig_;
    AutoPtr<MSFTPServer> msftp_;

    bool active();
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
MSFTPServerFiber::~MSFTPServerFiber()
{
}
//------------------------------------------------------------------------------
MSFTPServerFiber::MSFTPServerFiber(MSFTPService * msftp) :
  msftp_(msftp),
  workDir_(includeTrailingPathDelimiter(getCurrentDir()))
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
  return msftp_->msftpConfig_->parse().override().section("users").isSection(user);
}
//------------------------------------------------------------------------------
utf8::String MSFTPServerFiber::getUserPassword(const utf8::String & user,const AuthParams *)
{
  return msftp_->msftpConfig_->section("users").section(user).text("password");
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::auth()
{
  currentFiber()->checkFiberStackOverflow();
  AuthParams ap;
  ap.maxRecvSize_ = msftp_->msftpConfig_->value("max_recv_size",-1);
  ap.maxSendSize_ = msftp_->msftpConfig_->value("max_send_size",-1);
  ap.recvTimeout_ = msftp_->msftpConfig_->value("recv_timeout",-1);
  if( ap.recvTimeout_ != ~uint64_t(0) ) ap.recvTimeout_ *= 1000000u;
  ap.sendTimeout_ = msftp_->msftpConfig_->value("send_timeout",-1);
  if( ap.sendTimeout_ != ~uint64_t(0) ) ap.sendTimeout_ *= 1000000u;
  ap.encryption_ = msftp_->msftpConfig_->section("encryption").text(utf8::String(),"default");
  ap.threshold_ = msftp_->msftpConfig_->section("encryption").value("threshold",1024 * 1024);
  ap.compression_ = msftp_->msftpConfig_->section("compression").text(utf8::String(),"default");
  ap.compressionType_ = msftp_->msftpConfig_->section("compression").text("type","default");
  ap.crc_ = msftp_->msftpConfig_->section("compression").text("crc","default");
  ap.level_ = msftp_->msftpConfig_->section("compression").value("max_level",9);
  ap.optimize_ = msftp_->msftpConfig_->section("compression").value("optimize",true);
  ap.bufferSize_ = msftp_->msftpConfig_->section("compression").value("buffer_size",getpagesize() * 16);
  ap.noAuth_ = msftp_->msftpConfig_->value("noauth",false);
  MSFTPError e = (MSFTPError) serverAuth(ap);
  if( e != eOK )
    newObjectV1C2<Exception>(e,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::stat()
{
  currentFiber()->checkFiberStackOverflow();
  MSFTPStat mst;
  utf8::String file(absolutePathNameFromWorkDir(workDir_,readString()));
  bool isSt = mst.stat(file);
  write(&mst,sizeof(mst));
  return putCode(isSt ? eOK : eBadPathName);
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::setTimes()
{
  currentFiber()->checkFiberStackOverflow();
  utf8::String file(absolutePathNameFromWorkDir(workDir_,readString()));
  MSFTPStat mst, lmst;
  read(&mst,sizeof(mst));
  if( (cmd_ == cmSetATime || cmd_ == cmSetMTime) && lmst.stat(file) ){
    if( cmd_ == cmSetATime ) mst.st_mtime_ = lmst.st_mtime_;
    else
    if( cmd_ == cmSetMTime ) mst.st_atime_ = lmst.st_atime_;
  }
  return putCode(utime(file,mst.st_atime_,mst.st_mtime_) ? eOK : eSetTimes);
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::resize()
{
  currentFiber()->checkFiberStackOverflow();
  AsyncFile file(absolutePathNameFromWorkDir(workDir_,readString()));
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
  currentFiber()->checkFiberStackOverflow();
  AsyncFile file(absolutePathNameFromWorkDir(workDir_,readString()));
  file.createIfNotExist(true).exclusive(true);
  uint64_t l, lp, ll, pos, r;
  int64_t mtime;
  uint64_t bs;
  *this >> pos >> ll >> mtime >> bs;
  if( cmd_ == cmPutFilePartial ) *this >> bs;
  MSFTPError err = eOK;
  try {
    if( !file.tryOpen() ) err = eFileOpen;
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
#if defined(__WIN32__) || defined(__WIN64__)
    if( e->code() != ERROR_DISK_FULL + errorOffset ) throw;
#else
    if( e->code() != ENOSPC ) throw;
#endif
    err = eDiskFull;
  }
  putCode(err);
  if( err != eOK ) return *this;
  if( file.size() < pos ) file.resize(pos);
  file.seek(pos);
  AutoPtrBuffer b;
  if( cmd_ != cmPutFilePartial ){
    if( bs > ll ) bs = (uint32_t) ll; else if( bs == 0 ) bs = (uint32_t) ll;
    if( bs > 0x40000000 ) bs = 0x40000000;
  }
  b.alloc((size_t) bs);
  SHA256 lhash, rhash;
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
      if( r < bs ) memset(b.ptr() + uintptr_t(r),0,size_t(bs - r));
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
      catch( ExceptionSP & e ){
        e->writeStdError();
#if defined(__WIN32__) || defined(__WIN64__)
        if( e->code() != ERROR_DISK_FULL + errorOffset ) throw;
#else
        if( e->code() != ENOSPC ) throw;
#endif
        err = eDiskFull;
      }
      putCode(err);
    }
    if( err != eOK || terminated() ) return *this;
  }
  if( cmd_ != cmPutFilePartial && l != ll && lp > 0 ) return putCode(eFileWrite);
  file.close();
  MSFTPStat mst;
  if( !mst.stat(file.fileName()) ) return putCode(eFileStat);
  return putCode(utime(file.fileName(),mst.st_atime_,mtime) ? eOK : eSetTimes);
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::get()
{
  currentFiber()->checkFiberStackOverflow();
  utf8::String name;
  uint64_t l, ll, lp, bs = msftp_->msftpConfig_->value("buffer_size",getpagesize());
  *this >> name >> l >> ll;
  if( cmd_ == cmGetFilePartial ) *this >> bs;
  AsyncFile file(name);
  file.readOnly(true);
  try {
    file.open();
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
  }
  putCode(file.isOpen() ? eOK : eFileOpen);
  if( file.isOpen() ){
    file.seek(l);
    AutoPtrBuffer b;
    b.alloc((size_t) bs);
    SHA256 lhash, rhash;
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
  currentFiber()->checkFiberStackOverflow();
  utf8::String localPath, exclude;
  uint8_t recursive;
  *this >> localPath >> exclude >> recursive;
  Vector<utf8::String> list;
  getDirList(list,localPath,exclude,recursive != 0,false,true);
  putCode(eOK);
  *this << (uint64_t) list.count();
  uintptr_t l = includeTrailingPathDelimiter(getPathFromPathName(localPath)).length();
  for( intptr_t i = list.count() - 1; i >= 0; i-- ){
    *this << (utf8::String::Iterator(list[i]) + l);
    list.resize(list.count() - 1);
  }
  return *this;
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::getFileHash()
{
  currentFiber()->checkFiberStackOverflow();
  utf8::String name;
  uint64_t l, ll, lp, bs, partialBlockSize;
  *this >> name >> partialBlockSize;
  if( partialBlockSize < (uintptr_t) getpagesize() ) partialBlockSize = getpagesize();
  if( partialBlockSize > 4u * 1024u * 1024u ) partialBlockSize = 4u * 1024u * 1024u;
  AsyncFile file(name);
  if( file.tryOpen() ){
    lp = file.size();
    ll = partialBlockSize /*partialBlockSize(lp)*/;
    bs = ll;
    l = lp / bs;
    l += lp > 0 && lp % bs != 0;
  }
  else {
    l = ll = lp = bs = 0;
  }
  *this << ll << l;
  if( file.isOpen() ){
    SHA256 hash;
    AutoPtrBuffer b;
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
  currentFiber()->checkFiberStackOverflow();
  msftp_->msftpConfig_->silent(false).parse().override();
  stdErr.rotationThreshold(
    msftp_->msftpConfig_->value("debug_file_rotate_threshold",1024 * 1024)
  );
  stdErr.rotatedFileCount(
    msftp_->msftpConfig_->value("debug_file_rotate_count",10)
  );
  stdErr.setDebugLevels(
    msftp_->msftpConfig_->value("debug_levels","+0,+1,+2,+3")
  );
  stdErr.fileName(
    msftp_->msftpConfig_->value("log_file",stdErr.fileName())
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
    else if( cmd_ == cmSetTimes || cmd_ == cmSetATime || cmd_ == cmSetMTime ){
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
MSFTPServer::~MSFTPServer()
{
}
//------------------------------------------------------------------------------
MSFTPServer::MSFTPServer(MSFTPService * msftp) : msftp_(msftp)
{
  fiberStackSize(8192 * sizeof(void *) / 4);
}
//------------------------------------------------------------------------------
Fiber * MSFTPServer::newFiber()
{
  return newObjectV1<MSFTPServerFiber>(msftp_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void MSFTPWatchdog::fiberExecute()
{
  currentFiber()->checkFiberStackOverflow();
  utf8::String dir(excludeTrailingPathDelimiter(msftp_->msftpConfig_->textByPath(section_ + ".directory")));
  utf8::String exec(msftp_->msftpConfig_->textByPath(section_ + ".exec"));
  utf8::String cmdLine(msftp_->msftpConfig_->textByPath(section_ + ".command_line"));
  uint64_t timeout = msftp_->msftpConfig_->valueByPath(section_ + ".timeout",0);
  uint64_t delay = msftp_->msftpConfig_->valueByPath(section_ + ".delay",30);
  bool execDaemonStartup = msftp_->msftpConfig_->valueByPath(section_ + ".exec_daemon_startup",true);
  bool repeatIfExitCodeNonzero = msftp_->msftpConfig_->valueByPath(section_ + ".repeat_if_exit_code_nonzero",true);
  uint64_t repeatDelay = msftp_->msftpConfig_->valueByPath(section_ + ".repeat_delay",10);
  dcn_.createPath(false);
  intptr_t exitCode = 0;
  while( !terminated_ ){
    try {
      try {
        if( !execDaemonStartup ) dcn_.monitor(dir,timeout == 0 ? ~uint64_t(0) : timeout * 1000000u);
        if( delay > 0 && !execDaemonStartup ) ksleep(delay * 1000000u);
      }
      catch( ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
        if( !e->searchCode(WAIT_TIMEOUT + errorOffset) ) throw;
#else
        if( !e->searchCode(ETIMEDOUT) ) throw;
#endif
      }
      for(;;){
        exitCode = execute(exec,cmdLine,NULL,true);
        if( exitCode == 0 || !repeatIfExitCodeNonzero ) break;
        if( repeatDelay > 0 ) ksleep(repeatDelay * 1000000u);
      }
    }
    catch( ExceptionSP & e ){
      e->writeStdError();
      ksleep(1000000);
    }
    execDaemonStartup = false;
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
MSFTPService::MSFTPService(int) :
  msftpConfig_(newObject<InterlockedConfig<FiberWriteLock> >())  
{
  msftp_ = newObjectV1<MSFTPServer>(this);
}
//------------------------------------------------------------------------------
void MSFTPService::initialize()
{
  msftpConfig_->silent(true).parse().override();
  stdErr.bufferDataTTA(
    (uint64_t) msftpConfig_->value("debug_file_max_collection_time",60) * 1000000u
  );
  stdErr.rotationThreshold(
    msftpConfig_->value("debug_file_rotate_threshold",1024 * 1024)
  );
  stdErr.rotatedFileCount(
    msftpConfig_->value("debug_file_rotate_count",10)
  );
  stdErr.setDebugLevels(
    msftpConfig_->value("debug_levels","+0,+1,+2,+3")
  );
  stdErr.fileName(
    msftpConfig_->value("log_file",stdErr.fileName())
  );
  msftp_->fiberStackSize(msftpConfig_->value("fiber_stack_size",msftp_->fiberStackSize()));
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  checkMachineBinding(msftpConfig_->value("machine_key"),true);
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
  serviceName_ = msftpConfig_->value("service_name","msftp");
  displayName_ = msftpConfig_->value("service_display_name","Macroscope FTP Service");
#if defined(__WIN32__) || defined(__WIN64__)
  serviceType_ = SERVICE_WIN32_OWN_PROCESS;
  startType_ = SERVICE_AUTO_START;
  errorControl_ = SERVICE_ERROR_IGNORE;
  binaryPathName_ = getExecutableName();
  serviceStatus_.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
  serviceStatus_.dwWaitHint = 60000; // give me 60 seconds for start or stop 
#endif
}
//------------------------------------------------------------------------------
void MSFTPService::start()
{
  msftpConfig_->silent(true).parse().override();
  serviceName_ = msftpConfig_->value("service_name","msftp");
  displayName_ = msftpConfig_->value("service_display_name","Macroscope FTP Service");
  Array<ksock::SockAddr> addrs;
  ksock::SockAddr::resolveNameForBind(
    msftpConfig_->text("bind"),
    addrs,
    MSFTPDefaultPort
  );
  intptr_t i;
  for( i = addrs.count() - 1; i >= 0; i-- ) msftp_->addBind(addrs[i]);
  msftp_->open();
  stdErr.debug(0,
    utf8::String::Stream() << msftpd_version.gnu_ << " started (" << serviceName_ << ")\n"
  );
  for( i = msftpConfig_->sectionCount() - 1; i >= 0; i-- ){
    utf8::String sectionName(msftpConfig_->section(i).name());
    if( sectionName.ncasecompare("watchdog",8) == 0 )
      msftp_->attachFiber(newObjectV1C2<MSFTPWatchdog>(this,sectionName));
  }

}
//------------------------------------------------------------------------------
void MSFTPService::stop()
{
  msftp_->close();
  stdErr.debug(0,
    utf8::String::Stream() << msftpd_version.gnu_ << " stopped (" << serviceName_ << ")\n"
  );
}
//------------------------------------------------------------------------------
bool MSFTPService::active()
{
  return msftp_->active();
}
//------------------------------------------------------------------------------
int main(int _argc,char * _argv[])
{
  //Sleep(15000);

  int errcode = 0;
  adicpp::AutoInitializer autoInitializer(_argc,_argv);
  autoInitializer = autoInitializer;
  bool isDaemon = isDaemonCommandLineOption();
  if( isDaemon ) daemonize();
  try {
    uintptr_t u;
    stdErr.fileName(SYSLOG_DIR(utf8::String(msftpd_version.tag_) + pathDelimiterStr) + msftpd_version.tag_ + ".log");
    Config::defaultFileName(SYSCONF_DIR("") + msftpd_version.tag_ + ".conf");
    Services services(msftpd_version.gnu_);
    MSFTPService * service;
    services.add(service = newObjectV1<MSFTPService>(0));
    bool dispatch = true;
    service->msftpConfig()->silent(true);
    utf8::String pidFileName;
    for( u = 1; u < argv().count(); u++ ){
      if( argv()[u].compare("--chdir") == 0 && u + 1 < argv().count() ){
        changeCurrentDir(argv()[u + 1]);
      }
      else if( argv()[u].compare("-c") == 0 && u + 1 < argv().count() ){
        Config::defaultFileName(argv()[u + 1]);
        service->msftpConfig()->fileName(argv()[u + 1]);
      }
      else if( argv()[u].compare("--log") == 0 && u + 1 < argv().count() ){
        stdErr.fileName(argv()[u + 1]);
      }
      else if( argv()[u].compare("--pid") == 0 && u + 1 < argv().count() ){
        pidFileName = argv()[u + 1];
      }
    }
    service->initialize();
    for( u = 1; u < argv().count(); u++ ){
      if( argv()[u].compare("--version") == 0 ){
        stdErr.debug(9,utf8::String::Stream() << msftpd_version.tex_ << "\n");
        fprintf(stdout,"%s\n",msftpd_version.tex_);
        dispatch = false;
        continue;
      }
      if( argv()[u].compare("--install") == 0 ){
        for( uintptr_t j = u + 1; j < argv().count(); j++ )
          if( argv()[j].isSpace() )
            service->args(service->args() + " \"" + argv()[j] + "\"");
          else
            service->args(service->args() + " " + argv()[j]);
        services.install();
        dispatch = false;
      }
      else if( argv()[u].compare("--uninstall") == 0 ){
        services.uninstall();
        dispatch = false;
      }
      /*else if( argv()[u].compare("--start") == 0 && u + 1 < argv().count() ){
        services.start(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].compare("--stop") == 0 && u + 1 < argv().count() ){
        services.stop(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].compare("--suspend") == 0 && u + 1 < argv().count() ){
        services.suspend(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].compare("--resume") == 0 && u + 1 < argv().count() ){
        services.resume(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].compare("--query") == 0 && u + 1 < argv().count() ){
        services.query(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].compare("--start-disp") == 0 ){
        dispatch = true;
      }
      else if( argv()[u].compare("--stop-disp") == 0 ){
        services.stopServiceCtrlDispatcher();
        dispatch = false;
      }*/
      else if( argv()[u].compare("--sha256") == 0 && u + 1 < argv().count() ){
        SHA256 passwordSHA256;
        passwordSHA256.make(argv()[u + 1].c_str(),argv()[u + 1].size());
        utf8::String b64(base64Encode(passwordSHA256.sha256(),32));
        fprintf(stdout,"%s\n",b64.c_str());
        copyStrToClipboard(b64);
        dispatch = false;
      }
    }
    if( dispatch ){
      bool daemon = service->msftpConfig()->parse().override().value("daemon",false);
      if( !daemon && isDaemon ) daemon = true;
      service->msftpConfig()->silent(false);
#if defined(__WIN32__) || defined(__WIN64__)
      if( daemon ){
        services.startServiceCtrlDispatcher();
      }
      else
#endif
      {
        service->start();
        if( !pidFileName.isNull() ){
          AsyncFile pidFile(pidFileName);
  	      pidFile.createIfNotExist(true).open() << utf8::int2Str(ksys::getpid());
	        pidFile.resize(pidFile.size());
        }
        Thread::waitForSignal();
        service->stop();
      }
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

