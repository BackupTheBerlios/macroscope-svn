//------------------------------------------------------------------------------
#include <adicpp/adicpp.h>
#include "../msftpd/msftpd.h"
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
} msftp_version_t;
extern msftp_version_t msftp_version;
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
class KFTPClient : public ksock::ClientFiber, public ksock::AsyncSocket {
  public:
    ~KFTPClient();
    KFTPClient(const ksys::ConfigSP & config,const utf8::String & section);
  protected:
    void main();
  private:
    const ksys::ConfigSP & config_;
    utf8::String section_;
    utf8::String host_;
    ksock::SockAddr remoteAddress_;

    KFTPClient & checkCode(int32_t code,int32_t noThrowCode = eOK);
    KFTPClient & getCode(int32_t noThrowCode = eOK);
    void auth();
    void put();
};
//------------------------------------------------------------------------------
KFTPClient::~KFTPClient()
{
}
//------------------------------------------------------------------------------
KFTPClient::KFTPClient(const ksys::ConfigSP & config,const utf8::String &  section) :
  config_(config), section_(section)
{
  host_ = config_->section(section_).text();
}
//------------------------------------------------------------------------------
KFTPClient & KFTPClient::checkCode(int32_t code,int32_t noThrowCode)
{
  if( code != eOK && code != noThrowCode )
    throw ksys::ExceptionSP(new ksys::Exception(code,utf8::string(__PRETTY_FUNCTION__)));
  return *this;
}
//------------------------------------------------------------------------------
KFTPClient & KFTPClient::getCode(int32_t noThrowCode)
{
  int32_t r;
  *this >> r;
  return checkCode(r,noThrowCode);
}
//------------------------------------------------------------------------------
void KFTPClient::auth()
{
  utf8::String user, password, encryption, compression;
  maxSendSize(config_->value(utf8::string("max_send_size"),-1));
  if( config_->section(section_).isValue(utf8::string("max_send_size")) )
    maxSendSize(config_->section(section_).value(utf8::string("max_send_size"),-1));
  user = config_->text(utf8::string("user"));
  if( config_->section(section_).isValue(utf8::string("user")) )
    user = config_->section(section_).text(utf8::string("user"));
  password = config_->text(utf8::string("password"));
  if( config_->section(section_).isValue(utf8::string("password")) )
    password = config_->section(section_).text(utf8::string("password"));
  encryption =
    config_->section(utf8::string("encryption")).text(utf8::String(),utf8::string("allow"));
  uintptr_t encryptionThreshold =
    config_->section(utf8::string("encryption")).value(utf8::string("threshold"),1024 * 1024);
  if( config_->section(section_).isSection(utf8::string("encryption")) ){
    encryption = config_->section(section_).
      section(utf8::string("encryption")).text(utf8::String(),utf8::string("allow"));
    encryptionThreshold =
      config_->section(section_).section(
        utf8::string("encryption")).value(utf8::string("threshold"),1024 * 1024);
  }
  compression =
    config_->section(
      utf8::string("compression")).text(utf8::String(),
      utf8::string("allow")
    );
  if( config_->section(section_).section(
      utf8::string("compression")).text(utf8::String()).strlen() > 0 )
    compression =
      config_->section(section_).section(
        utf8::string("compression")).text(utf8::String(),
        utf8::string("allow")
      );
  uintptr_t compressionLevel =
    config_->section(
      utf8::string("compression")).value(utf8::string("compression_level"),9
    );
  if( config_->section(section_).section(
        utf8::string("compression")).isValue(utf8::string("compression_level")) )
    compressionLevel =
      config_->section(section_).section(
        utf8::string("compression")).value(utf8::string("compression_level"),9
      );
  uintptr_t bufferSize =
    config_->section(
      utf8::string("compression")).value(utf8::string("buffer_size"),256 * 1024
    );
  if( config_->section(section_).section(
    utf8::string("compression")).isValue(utf8::string("buffer_size")) )
    bufferSize =
      config_->section(section_).section(
        utf8::string("compression")).value(utf8::string("buffer_size"),256 * 1024
      );
  checkCode(
    clientAuth(
      user,password,encryption,encryptionThreshold,compression,compressionLevel,bufferSize
    )
  );
}
//------------------------------------------------------------------------------
void KFTPClient::put()
{
  utf8::String local(
    ksys::unScreenString(
      config_->section(section_).section(
        utf8::string("put")).text(utf8::string("local"))
    )
  );
  utf8::String localPath(
    ksys::includeTrailingPathDelimiter(ksys::getPathFromPathName(local))
  );
  utf8::String exclude(config_->section(section_).
    section(utf8::string("put")).text(utf8::string("exclude"))
  );
  bool recursive = config_->value(utf8::string("recursive"),true);
  if( config_->section(section_).section(utf8::string("put")).isValue(
        utf8::string("recursive")) )
    recursive =
      config_->section(section_).section(utf8::string("put")).value(
        utf8::string("recursive"),true
      );

  ksys::Vector<utf8::String> list;
  getDirList(list,local,exclude,recursive);
  utf8::String mode(config_->text(utf8::string("mode"),utf8::string("auto")));
  if( config_->section(section_).section(utf8::string("put")).
        isValue(utf8::string("mode")) )
    mode =
      config_->section(section_).section(
        utf8::string("put")).text(utf8::string("mode"),
        utf8::string("auto")
      );
  utf8::String remotePath(
    ksys::includeTrailingPathDelimiter(
      ksys::unScreenString(
        config_->section(section_).
          section(utf8::string("put")).text(utf8::string("remote"))
      )
    )
  );
  int64_t all = 0, ptime = gettimeofday(), atime = 0;
  for( intptr_t i = list.count() - 1; i >= 0; i-- ){
    try {
      uint32_t bs = (uint32_t) config_->value(utf8::string("buffer_size"),256 * 1024);
      if( config_->section(section_).
          section(utf8::string("put")).isValue(utf8::string("buffer_size")) )
        bs = (uint32_t)
          config_->section(section_).
            section(utf8::string("put")).value(
              utf8::string("buffer_size"),256 * 1024
            );
      if( bs == 0 ) bs = 256 * 1024;
      utf8::String rfile(
        remotePath + utf8::string(utf8::String::Iterator(list[i]) + localPath.strlen())
      );
      ksys::AsyncFile file(list[i]);
      MSFTPStat rmst, lmst;
      if( lmst.stat(file.fileName()) ){
        file.open();
        attachDescriptor(file);
        uint64_t l, ll = 0, lp;
        int64_t mtime = lmst.st_mtime_;
        if( mode.strcasecmp(utf8::string("auto")) == 0 ){
          *this << int8_t(cmStat) << rfile;
          read(&rmst,sizeof(rmst));
          getCode(eBadPathName);
          if( rmst.st_size_ < lmst.st_size_ ){
            file.seek(rmst.st_size_);
            ll = file.size() - rmst.st_size_;
          }
          else if( rmst.st_size_ > lmst.st_size_ || rmst.st_mtime_ != lmst.st_mtime_ ){
            ll = file.size();
            if( rmst.st_size_ > 0 ){
              *this << int8_t(cmResize) << rfile << int64_t(0);
              getCode();
            }
          }
        }
        else if( mode.strcasecmp(utf8::string("overwrite")) == 0 ){
          ll = file.size();
          if( rmst.st_size_ > 0 ){
            *this << int8_t(cmResize) << rfile << int64_t(0);
            getCode();
          }
        }
        if( bs > ll ) bs = (uint32_t) ll;
        *this << int8_t(cmPutFile) << rfile << file.tell() << ll << mtime << bs;
        getCode();
        ksys::AutoPtr<uint8_t> b;
        b.alloc(bs);
        int64_t ttime = gettimeofday();
        for( l = 0; l < ll; l += lp ){
          file.readBuffer(b,lp = ll - l > bs ? bs : ll - l);
          write(b.ptr(),lp);
          getCode();
          if( terminated_ ) return;
        }
        getCode();
        ttime = gettimeofday() - ttime;
        atime += ttime;
        if( ttime == 0 ) ttime = 1;
        ksys::stdErr.sysLog(
          ksys::lmDIRECT,
          "job %s elapsed: %s, avg speed %s.%02u kbps, %s.%02u kb transfered\n  %s to %s\n",
          (const char *) host_.getANSIString(),
          utf8::elapsedTime2Str(ttime).c_str(),
          utf8::int2Str((l * 1000000u / ttime) / 1024u).c_str(),
          unsigned((l * 1000000u / ttime) % 1024u) * 100u / 1024u,
          utf8::int2Str(l / 1024u).c_str(),
          unsigned((l % 1024u) * 100u / 1024u),
          (const char *) list[i].getANSIString(),
          (const char *) rfile.getANSIString()
        );
        all += l;
      }
    }
    catch( ksys::ExceptionSP & e ){
      switch( e->codes()[0] ){
        case 0 :
#if defined(__WIN32__) || defined(__WIN64__)
        case ERROR_SHARING_VIOLATION + ksys::errorOffset :
        case ERROR_LOCK_VIOLATION + ksys::errorOffset :
        case ERROR_INVALID_ACCESS + ksys::errorOffset :
        case ERROR_FILE_NOT_FOUND + ksys::errorOffset :
        case ERROR_PATH_NOT_FOUND + ksys::errorOffset :
        case ERROR_ACCESS_DENIED + ksys::errorOffset :
        case ERROR_INVALID_DRIVE + ksys::errorOffset :
        case ERROR_SEEK + ksys::errorOffset :
        case ERROR_SECTOR_NOT_FOUND + ksys::errorOffset :
#else
        case EPERM : case ENOENT : case EACCES : case ENOTDIR :
#endif
          break;
        default :
          if( e->codes()[0] < ksock::AsyncSocket::aeMagic ||
              e->codes()[0] >= eCount ) throw;
      }
      ksys::stdErr.sysLog(
        ksys::lmDIRECT,
        "job %s failed: %s, %s\n",
        (const char *) host_.getANSIString(),
        (const char *) ksys::strError(e->codes()[0]).getANSIString(),
        (const char *) list[i].getANSIString()
      );
    }
  }
  if( (ptime = gettimeofday() - ptime) == 0 ) ptime = 1;
  if( atime == 0 ) atime = 1;
  ksys::stdErr.sysLog(
    ksys::lmDIRECT,
    "job %s operation put complete.\n"
    "  elapsed: %s, avg speed %s.%02u kbps, %s.%02u kb transfered\n"
    "  compression:\n"
    "    receive  difference: %s.%02u kb, ratio: %3u.%02u%%\n"
    "    transmit difference: %s.%02u kb, ratio: %3u.%02u%%\n"
    "    overall  difference: %s.%02u kb, ratio: %3u.%02u%%\n",
    (const char *) host_.getANSIString(),
    utf8::elapsedTime2Str(ptime).c_str(),
    utf8::int2Str((all * 1000000u / atime) / 1024u).c_str(),
    unsigned((all * 1000000u / atime) % 1024u) * 100u / 1024u,
    utf8::int2Str(all / 1024u).c_str(),
    unsigned((all % 1024u) * 100u / 1024u),
    utf8::int2Str(rcDifference() / 1024u).c_str(),
    unsigned(abs(int(rcDifference() % 1024u)) * 100u / 1024u),
    unsigned(rcRatio() / 100u),
    unsigned(rcRatio() % 100u),
    utf8::int2Str(scDifference() / 1024u).c_str(),
    unsigned(abs(int(scDifference() % 1024u)) * 100u / 1024u),
    unsigned(scRatio() / 100u),
    unsigned(scRatio() % 100u),
    utf8::int2Str(rscDifference() / 1024u).c_str(),
    unsigned(abs(int(rscDifference() % 1024u)) * 100u / 1024u),
    unsigned(rscRatio() / 100u),
    unsigned(rscRatio() % 100u)
  );
}
//------------------------------------------------------------------------------
void KFTPClient::main()
{
  open().setlinger();
  remoteAddress_.resolve(host_,MSFTPDefaultPort);
  connect(remoteAddress_);
  auth();
  put();
  if( terminated_ ) return;
  *this << int8_t(cmQuit);
  getCode();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class KFTPShell : public ksock::Client {
  public:
    ~KFTPShell();
    KFTPShell();

    void main();
  protected:
  private:
    ksys::ConfigSP config_;
};
//------------------------------------------------------------------------------
KFTPShell::~KFTPShell()
{
}
//------------------------------------------------------------------------------
KFTPShell::KFTPShell() : config_(new ksys::InterlockedConfig)
{
}
//------------------------------------------------------------------------------
void KFTPShell::main()
{
  intptr_t i;
  config_->parse().override();
  for( i = config_->sectionCount() - 1; i >= 0; i-- ){
    utf8::String sectionName(config_->section(i).name());
    if( sectionName.strncasecmp(utf8::string("job"),3) == 0 ){
      ksys::AutoPtr<KFTPClient> fiber(new KFTPClient(config_,sectionName));
      attachFiber(fiber);
      fiber.ptr(NULL);
    }
  }
#if defined(__WIN32__) || defined(__WIN64__)
  for(;;){
    MSG msg;
    if( PeekMessage(&msg,NULL,0,0,PM_REMOVE) != 0 ){
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    if( msg.message == WM_QUIT || finished() ) break;
    if( WaitMessage() == 0 ){
      int32_t err = GetLastError() + ksys::errorOffset;
      throw ksys::ExceptionSP(
        new ksys::Exception(err,utf8::string(__PRETTY_FUNCTION__))
      );
    }
  }
#endif
}
//------------------------------------------------------------------------------
int main(int argc,char * argv[])
{
  int errcode = 0;
  adicpp::initialize();
  try {
    union {
      intptr_t i;
      uintptr_t u;
    };
    ksys::initializeArguments(argc,argv);
    ksys::Config::defaultFileName(ksys::getExecutablePath() + "msftp.conf");
    bool dispatch = true;
    for( u = 1; u < ksys::argv().count(); u++ ){
      if( ksys::argv()[u].strcmp(utf8::string("-c")) == 0 && u + 1 < ksys::argv().count() ){
        ksys::Config::defaultFileName(ksys::argv()[u + 1]);
      }
      else if( ksys::argv()[i].strcmp(utf8::string("--sha256")) == 0 && u + 1 < ksys::argv().count() ){
        ksys::SHA256 passwordSHA256;
        passwordSHA256.make(ksys::argv()[u + 1].c_str(),ksys::argv()[u + 1].size());
        utf8::String b64(ksys::base64Encode(passwordSHA256.sha256(),32));
        fprintf(stdout,"%s\n",b64.c_str());
        ksys::copyStrToClipboard(b64);
        dispatch = false;
      }
    }
    if( dispatch ){
      KFTPShell shell;
      shell.main();
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
