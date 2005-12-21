//------------------------------------------------------------------------------
#include <adicpp/adicpp.h>
#include "msftpd.h"
//------------------------------------------------------------------------------
extern "C" {
#if _MSC_VER || __INTEL_COMPILER
#pragma warning(disable:411)
#endif
#if __BCPLUSPLUS__
typedef struct {
  int   v_hex;
  const char *v_short;
  const char *v_long;
  const char *v_tex;
  const char *v_gnu;
  const char *v_web;
  const char *v_sccs;
  const char *v_rcs;
} msftpd_version_t;
extern msftpd_version_t msftpd_version;
#define _VERSION_H_
#endif
#include "version.h"
#if _MSC_VER || __INTEL_COMPILER
#pragma warning(default:4200)
#endif
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class MSFTPServerFiber : public ksock::ServerFiber {
  public:
    virtual ~MSFTPServerFiber();
    MSFTPServerFiber(const ksys::ConfigSP & config);
  protected:
    void main();
  private:
    const ksys::ConfigSP & config_;
    utf8::String workDir_;
    utf8::String user_;

    MSFTPServerFiber & putCode(int32_t code);
    MSFTPServerFiber & auth();
    MSFTPServerFiber & stat();
    MSFTPServerFiber & setTimes();
    MSFTPServerFiber & resize();
    MSFTPServerFiber & put();
};
//------------------------------------------------------------------------------
MSFTPServerFiber::~MSFTPServerFiber()
{
}
//------------------------------------------------------------------------------
MSFTPServerFiber::MSFTPServerFiber(const ksys::ConfigSP & config) :
  config_(config),
  workDir_(ksys::includeTrailingPathDelimiter(ksys::getCurrentDir()))
{
  config_->parse().override();
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::putCode(int32_t code)
{
  *this << int32_t(code);
  return *this;
}
//------------------------------------------------------------------------------
MSFTPServerFiber & MSFTPServerFiber::auth()
{
  maxSendSize(config_->value(utf8::string("max_send_size"),-1));
  ksys::HashedObjectList<utf8::String,utf8::String> usersDatabase;
  for( intptr_t i = config_->section(utf8::string("users")).sectionCount() - 1; i >= 0; i-- ){
    ksys::AutoPtr<utf8::String> password(
      new utf8::String(config_->section(
        utf8::string("users")).section(i).text(utf8::string("password")))
    );
    usersDatabase.add(
      password,
      config_->section(utf8::string("users")).section(i).name()
    );
    password.ptr(NULL);
  }
  utf8::String encryption(
    config_->section(utf8::string("encryption")).text(utf8::String(),utf8::string("allow"))
  );
  uintptr_t encryptionThreshold =
    config_->section(utf8::string("encryption")).value(utf8::string("threshold"),1024 * 1024);
  utf8::String compression(
    config_->section(
      utf8::string("compression")).text(utf8::String(),utf8::string("allow")
    )
  );
  uintptr_t compressionLevel = config_->section(
    utf8::string("compression")).value(utf8::string("max_level"),3
  );
  uintptr_t bufferSize = config_->section(
    utf8::string("compression")).value(utf8::string("buffer_size"),256 * 1024
  );
  MSFTPError e = (MSFTPError) serverAuth(
    usersDatabase,encryption,encryptionThreshold,
    compression,compressionLevel,bufferSize,user_
  );
  if( e != eOK )
    throw ksys::ExceptionSP(new ksys::Exception(e,utf8::string(__PRETTY_FUNCTION__)));
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
    attachDescriptor(file);
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
  uint32_t bs;
  *this >> pos >> ll >> mtime >> bs;
  MSFTPError err = eOK;
  try {
    if( !file.tryOpen() ){
      ksys::createDirectory(
        ksys::excludeTrailingPathDelimiter(
          ksys::getPathFromPathName(file.fileName())
        )
      );
      if( !file.tryOpen() ) err = eFileOpen;
    }
  }
  catch( ksys::ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( e->codes()[0] != ERROR_DISK_FULL + ksys::errorOffset ) throw;
#else
    if( e->codes()[0] != ENOSPC ) throw;
#endif
    err = eDiskFull;
  }
  putCode(err);
  if( err != eOK ) return *this;
  attachDescriptor(file);
  if( file.size() < pos ) file.resize(pos);
  file.seek(pos);
  ksys::AutoPtr<uint8_t> b;
  if( bs > ll ) bs = (uint32_t) ll; else if( bs == 0 ) bs = (uint32_t) ll;
  if( bs > 0x40000000 ) bs = 0x40000000;
  b.alloc(bs);
  for( l = 0; l < ll; l += lp ){
    read(b.ptr(),lp = ll - l > bs ? bs : ll - l);
    err = eOK;
    try {
      file.writeBuffer(b.ptr(),lp);
    }
    catch( ksys::ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
      if( e->codes()[0] != ERROR_DISK_FULL + ksys::errorOffset ) throw;
#else
      if( e->codes()[0] != ENOSPC ) throw;
#endif
      err = eDiskFull;
    }
    putCode(err);
    if( err != eOK || terminated() ) return *this;
  }
  if( l != ll ) return putCode(eFileWrite);
  file.close();
  MSFTPStat mst;
  if( !mst.stat(file.fileName()) ) return putCode(eFileStat);
  struct utimbuf ut;
  ut.actime = (time_t) mst.st_atime_;
  ut.modtime = (time_t) mtime;
  return putCode(ksys::utime(file.fileName(),ut) ? eOK : eSetTimes);
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
    MSFTPServer(const ksys::ConfigSP & config);
  protected:
    const ksys::ConfigSP & config_;
    ksys::BaseFiber * newFiber();
  private:
};
//------------------------------------------------------------------------------
MSFTPServer::~MSFTPServer()
{
}
//------------------------------------------------------------------------------
MSFTPServer::MSFTPServer(const ksys::ConfigSP & config) : config_(config)
{
}
//------------------------------------------------------------------------------
ksys::BaseFiber * MSFTPServer::newFiber()
{
  return new MSFTPServerFiber(config_);
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
  msftpConfig_(new ksys::InterlockedConfig), msftp_(msftpConfig_)
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
  msftpConfig_->override(argv_).parse();
#else
  msftpConfig_->override().parse();
#endif
  ksys::Array<ksock::SockAddr> addrs;
  ksock::SockAddr::resolve(
    msftpConfig_->text(utf8::string("bind")),
    addrs,
    MSFTPDefaultPort
  );
  for( intptr_t i = addrs.count() - 1; i >= 0; i-- )
    msftp_.addBind(addrs[i]);
  msftp_.open();
  ksys::stdErr.log(ksys::lmINFO,"%s started\n",msftpd_version.v_gnu);
}
//------------------------------------------------------------------------------
void MSFTPService::stop()
{
  msftp_.close();
  ksys::stdErr.log(ksys::lmINFO,"%s stopped\n",msftpd_version.v_gnu);
}
//------------------------------------------------------------------------------
bool MSFTPService::active()
{
  return msftp_.active();
}
//------------------------------------------------------------------------------
int main(int argc,char * argv[])
{
//  ksys::sleep(20000000);
  int errcode = 0;
  adicpp::initialize();
  try {
    union {
      intptr_t i;
      uintptr_t u;
    };
    ksys::initializeArguments(argc,argv);
    ksys::Config::defaultFileName(SYSCONF_DIR + "msftpd.conf");
    ksys::Services services(utf8::string(msftpd_version.v_gnu));
    services.add(new MSFTPService);
    bool dispatch = 
#if defined(__WIN32__) || defined(__WIN64__)
      true;
#else
      false;
#endif
    for( u = 1; u < ksys::argv().count(); u++ ){
      if( ksys::argv()[u].strcmp(utf8::string("-c")) == 0 && u + 1 < ksys::argv().count() ){
        ksys::Config::defaultFileName(ksys::argv()[u + 1]);
      }
      else if( ksys::argv()[u].strcmp(utf8::string("--install")) == 0 ){
        services.install();
        dispatch = false;
      }
      else if( ksys::argv()[u].strcmp(utf8::string("--uninstall")) == 0 ){
        services.uninstall();
        dispatch = false;
      }
      else if( ksys::argv()[u].strcmp(utf8::string("--start")) == 0 && u + 1 < ksys::argv().count() ){
        services.start(ksys::argv()[u + 1]);
        dispatch = false;
      }
      else if( ksys::argv()[u].strcmp(utf8::string("--stop")) == 0 && u + 1 < ksys::argv().count() ){
        services.stop(ksys::argv()[u + 1]);
        dispatch = false;
      }
      else if( ksys::argv()[u].strcmp(utf8::string("--suspend")) == 0 && u + 1 < ksys::argv().count() ){
        services.suspend(ksys::argv()[u + 1]);
        dispatch = false;
      }
      else if( ksys::argv()[u].strcmp(utf8::string("--resume")) == 0 && u + 1 < ksys::argv().count() ){
        services.resume(ksys::argv()[u + 1]);
        dispatch = false;
      }
      else if( ksys::argv()[u].strcmp(utf8::string("--query")) == 0 && u + 1 < ksys::argv().count() ){
        services.query(ksys::argv()[u + 1]);
        dispatch = false;
      }
      else if( ksys::argv()[u].strcmp(utf8::string("--start-disp")) == 0 ){
        dispatch = true;
      }
      else if( ksys::argv()[u].strcmp(utf8::string("--stop-disp")) == 0 ){
        services.stopServiceCtrlDispatcher();
        dispatch = false;
      }
      else if( ksys::argv()[u].strcmp(utf8::string("--sha256")) == 0 && u + 1 < ksys::argv().count() ){
        ksys::SHA256 passwordSHA256;
        passwordSHA256.make(ksys::argv()[u + 1].c_str(),ksys::argv()[u + 1].size());
        utf8::String b64(ksys::base64Encode(passwordSHA256.sha256(),32));
        fprintf(stdout,"%s\n",b64.c_str());
        ksys::copyStrToClipboard(b64);
        dispatch = false;
      }
    }
    if( dispatch ){
      ksys::ConfigSP config(new ksys::InterlockedConfig);
      services.startServiceCtrlDispatcher(config->value(utf8::string("daemon"),false));
    }
  }
  catch( ksys::ExceptionSP & e ){
    e->writeStdError();
    errcode = e->codes()[0];
  }
  catch( ... ){
  }
  adicpp::cleanup();
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

