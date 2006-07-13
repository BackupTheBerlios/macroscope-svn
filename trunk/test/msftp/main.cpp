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
#include "../msftpd/msftpd.h"
//------------------------------------------------------------------------------
extern "C" {
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
#if _MSC_VER
#pragma warning(push,3)
#endif
#include "version.h"
#if _MSC_VER
#pragma warning(pop)
#endif
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class KFTPClient : public ksock::ClientFiber {
  public:
    virtual ~KFTPClient();
    KFTPClient(const ksys::ConfigSP & config,const utf8::String & section);
  protected:
    void main();
  private:
    ksys::ConfigSP config_;
    utf8::String section_;
    utf8::String host_;
    ksock::SockAddr remoteAddress_;

    KFTPClient & checkCode(int32_t code,int32_t noThrowCode = eOK);
    KFTPClient & getCode(int32_t noThrowCode = eOK);
    void auth();
    void put();
    void get();
};
//------------------------------------------------------------------------------
KFTPClient::~KFTPClient()
{
}
//------------------------------------------------------------------------------
KFTPClient::KFTPClient(const ksys::ConfigSP & config,const utf8::String & section) :
  config_(config), section_(section)
{
  host_ = config_->section(section_).text();
}
//------------------------------------------------------------------------------
KFTPClient & KFTPClient::checkCode(int32_t code,int32_t noThrowCode)
{
  if( code != eOK && code != noThrowCode )
    throw ksys::ExceptionSP(new ksys::Exception(code,__PRETTY_FUNCTION__));
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
  utf8::String user, password, encryption, compression, compressionType, crc;
  maxSendSize(config_->value("max_send_size",-1));
  maxSendSize(config_->section(section_).value("max_send_size",maxSendSize()));
  user = config_->text("user");
  user = config_->section(section_).text("user",user);
  password = config_->text("password");
  password = config_->section(section_).text("password",password);
  encryption = config_->section("encryption").text(utf8::String(),"default");
  uintptr_t encryptionThreshold = config_->section("encryption").value("threshold",1024 * 1024);
  encryption = config_->section(section_).section("encryption").text(utf8::String(),encryption);
  encryptionThreshold = config_->section(section_).section("encryption").value("threshold",encryptionThreshold);
  compression = config_->section("compression").text(utf8::String(),"default");
  compression = config_->section(section_).section("compression").text(utf8::String(),compression);
  compressionType = config_->section("compression").value("type","default");
  compressionType = config_->section(section_).section("compression").value("type",compressionType);
  crc = config_->section("compression").value("crc","default");
  crc = config_->section(section_).section("compression").value("crc",compressionType);
  uintptr_t compressionLevel = config_->section("compression").value("level",3);
  compressionLevel = config_->section(section_).section("compression").value("level",compressionLevel);
  bool optimize = config_->section("compression").value("optimize",false);
  optimize = config_->section(section_).section("compression").value("optimize",optimize);
  uintptr_t bufferSize = config_->section("compression").value("buffer_size",getpagesize());
  bufferSize = config_->section(section_).section("compression").value("buffer_size",bufferSize);
/*  ksys::stdErr.log(
    ksys::lmINFO,
    utf8::String::Stream() <<
      "encryption --> " << encryption << ", encryptionThreshold: " << encryptionThreshold <<
      ", compression --> " << compression << ", compressionLevel: " << compressionLevel << "\n"
  );*/
  checkCode(
    clientAuth(
      user,
      password,
      encryption,
      encryptionThreshold,
      compression,
      compressionType,
      crc,
      compressionLevel,
      optimize,
      bufferSize
    )
  );
}
//------------------------------------------------------------------------------
void KFTPClient::put()
{
  if( !config_->section(section_).isSection("put") ) return;
  utf8::String local(
    ksys::unScreenString(
      config_->section(section_).section("put").text("local")
    )
  );
  utf8::String localPath(
    ksys::includeTrailingPathDelimiter(ksys::getPathFromPathName(local))
  );
  utf8::String exclude(config_->section(section_).section("put").text("exclude"));
  bool recursive = config_->value("recursive",true);
  recursive = config_->section(section_).section("put").value("recursive",recursive);

  ksys::Vector<utf8::String> list;
  getDirList(list,local,exclude,recursive);
  utf8::String mode(config_->text("mode","auto"));
  mode = config_->section(section_).section("put").text("mode",mode);
  utf8::String remotePath(
    ksys::includeTrailingPathDelimiter(
      ksys::unScreenString(config_->section(section_).section("put").text("remote"))
    )
  );
  int64_t all = 0, ptime = getlocaltimeofday(), atime = 0, ttime;
  for( intptr_t i = list.count() - 1; i >= 0; i-- ){
    uint64_t l, ll, lp;
    utf8::String rfile(
      remotePath + utf8::String(utf8::String::Iterator(list[i]) + localPath.strlen())
    );
    try {
      uint64_t bs = (uint64_t) config_->value("buffer_size",getpagesize());
      bs = (uint64_t) config_->section(section_).section("put").value("buffer_size",bs);
      if( bs == 0 ) bs = getpagesize();
      ksys::AsyncFile file(list[i]);
      MSFTPStat rmst, lmst;
      if( lmst.statAsync(file.fileName()) ){
        file.open();
        ll = 0;
        int64_t mtime = lmst.st_mtime_;
        if( mode.strcasecmp("auto") == 0 ){
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
        else if( mode.strcasecmp("overwrite") == 0 ){
          ll = file.size();
          if( lmst.st_size_ > 0 ){
            *this << int8_t(cmResize) << rfile << int64_t(0);
            getCode(eResize);
          }
        }
        if( bs > ll ) bs = ll;
        *this << int8_t(cmPutFile) << rfile << file.tell() << ll << mtime << bs;
        getCode();
        ksys::AutoPtr<uint8_t> b;
        b.alloc((size_t) bs);
        ttime = getlocaltimeofday();
        for( l = 0; l < ll; l += lp ){
          file.readBuffer(b,lp = ll - l > bs ? bs : ll - l);
          write(b.ptr(),lp);
          getCode();
          if( terminated_ ) return;
        }
        getCode();
        ttime = getlocaltimeofday() - ttime;
        atime += ttime;
        if( ttime == 0 ) ttime = 1;
        ksys::stdErr.log(
          ksys::lmDIRECT,
          utf8::String::Stream() <<
          section_ << " " << host_ << " elapsed: " << utf8::elapsedTime2Str(ttime) <<
          ", avg speed " << (l * 1000000u / ttime) / 1024u << "." <<
          utf8::String::Stream::Format(unsigned((l * 1000000u / ttime) % 1024u) * 100u / 1024u,"%02") <<
          " kbps, " << l / 1024u << "." <<
          utf8::String::Stream::Format(unsigned((l % 1024u) * 100u / 1024u),"%02") <<
          " kb transfered\n  " << list[i] << " to " << rfile << "\n"
        );
        all += l;
      }
    }
    catch( ksys::ExceptionSP & e ){
      switch( e->code() ){
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
          if( e->code() < ksock::AsyncSocket::aeMagic || e->code() >= eCount ) throw;
      }
      ksys::stdErr.log(
        ksys::lmDIRECT,
        utf8::String::Stream() << "job " << host_ << " failed: " <<
        ksys::strError(e->code()) << ", " << list[i] << "\n"
      );
    }
    list.resize(list.count() - 1);
  }
  if( (ptime = getlocaltimeofday() - ptime) == 0 ) ptime = 1;
  if( atime == 0 ) atime = 1;
  ksys::stdErr.log(
    ksys::lmDIRECT,
    utf8::String::Stream() <<
    section_ << " " << host_ << " operation put complete.\n" <<
    "  elapsed: " << utf8::elapsedTime2Str(ptime) << ", avg speed " << 
    (all * 1000000u / atime) / 1024u << "." << 
    utf8::String::Stream::Format(unsigned((all * 1000000u / atime) % 1024u) * 100u / 1024u,"%02") <<
    " kbps, " << all / 1024u << "." <<
    utf8::String::Stream::Format((all % 1024u) * 100u / 1024u,"%02") << " kb transfered\n" <<
    "  compression:\n" <<
    "    receive  difference: " << rcDifference() / 1024u << "." <<
    utf8::String::Stream::Format(unsigned(abs(int(rcDifference() % 1024u)) * 100u / 1024u),"%02") <<
    " kb, ratio: " << 
    utf8::String::Stream::Format(rcRatio() / 100u,"%3") << "." <<
    utf8::String::Stream::Format(rcRatio() % 100u,"%02") << "%\n" <<
    "    transmit difference: " << scDifference() / 1024u << "." <<
    utf8::String::Stream::Format(unsigned(abs(int(scDifference() % 1024u)) * 100u / 1024u),"%02") <<
    " kb, ratio: " << 
    utf8::String::Stream::Format(scRatio() / 100u,"%3") << "." <<
    utf8::String::Stream::Format(scRatio() % 100u,"%02") << "%\n" <<
    "    overall  difference: " << rscDifference() / 1024u << "." <<
    utf8::String::Stream::Format(unsigned(abs(int(rscDifference() % 1024u)) * 100u / 1024u),"%02") <<
    " kb, ratio: " << 
    utf8::String::Stream::Format(rscRatio() / 100u,"%3") << "." <<
    utf8::String::Stream::Format(rscRatio() % 100u,"%02") << "%\n"
  );
}
//------------------------------------------------------------------------------
void KFTPClient::get()
{
  if( !config_->section(section_).isSection("get") ) return;
  utf8::String localPath(
    ksys::includeTrailingPathDelimiter(
      ksys::unScreenString(
        config_->section(section_).section("get").text("local")
      )
    )
  );
  utf8::String exclude(config_->section(section_).section("get").text("exclude"));
  bool recursive = config_->value("recursive",true);
  recursive = config_->section(section_).section("get").value("recursive",recursive);
  utf8::String mode(config_->text("mode","auto"));
  mode = config_->section(section_).section("get").text("mode",mode);
  utf8::String remote(
    ksys::unScreenString(config_->section(section_).section("get").text("remote"))
  );
  utf8::String remotel(ksys::includeTrailingPathDelimiter(ksys::getPathFromPathName(remote)));
  uint64_t bs = (uint64_t) config_->value("buffer_size",getpagesize());
  bs = (uint64_t) config_->section(section_).section("get").value("buffer_size",bs);
  if( bs == 0 ) bs = getpagesize();
  uint64_t all = 0, ptime = getlocaltimeofday(), atime = 0, ttime;
  uint64_t l, ll, lp;
  ksys::Vector<utf8::String> list;
  *this << int8_t(cmList) << remote << exclude << uint8_t(recursive);
  getCode();
  *this >> ll;
  while( ll-- > 0 ) list.add(readString());
  for( intptr_t i = list.count() - 1; i >= 0; i-- ){
    try {
      ksys::AsyncFile file(localPath + list[i]);
      if( !file.tryOpen() ){
        ksys::createDirectory(
          ksys::excludeTrailingPathDelimiter(
            ksys::getPathFromPathName(file.fileName())
          )
        );
        file.open();
      }
      MSFTPStat rmst, lmst;
      lmst.statAsync(file.fileName());
      ll = 0;
      *this << int8_t(cmStat) << remotel + list[i];
      read(&rmst,sizeof(rmst));
      getCode();
      if( mode.strcasecmp("auto") == 0 ){
        if( rmst.st_size_ > lmst.st_size_ ){
          file.seek(rmst.st_size_);
          ll = rmst.st_size_ - lmst.st_size_;
        }
        else if( rmst.st_size_ < lmst.st_size_ || rmst.st_mtime_ != lmst.st_mtime_ ){
          ll = lmst.st_size_;
          file.resize(0);
        }
      }
      else if( mode.strcasecmp("overwrite") == 0 ){
        file.resize(0);
        ll = rmst.st_size_;
      }
      if( bs > ll ) bs = ll;
      *this << int8_t(cmGetFile) << remotel + list[i] << file.tell() << ll;
      getCode();
      ksys::AutoPtr<uint8_t> b;
      b.alloc((size_t) bs);
      ttime = getlocaltimeofday();
      for( l = 0; l < ll; l += lp ){
        read(b.ptr(),lp = ll - l > bs ? bs : ll - l);
        file.writeBuffer(b,lp);
        if( terminated_ ) return;
      }
      ttime = getlocaltimeofday() - ttime;
      atime += ttime;
      if( ttime == 0 ) ttime = 1;
      ksys::stdErr.log(
        ksys::lmDIRECT,
        utf8::String::Stream() <<
        section_ << " " << host_ << " elapsed: " << utf8::elapsedTime2Str(ttime) <<
        ", avg speed " << (l * 1000000u / ttime) / 1024u << "." <<
        utf8::String::Stream::Format(unsigned((l * 1000000u / ttime) % 1024u) * 100u / 1024u,"%02") <<
        " kbps, " << l / 1024u << "." <<
        utf8::String::Stream::Format(unsigned((l % 1024u) * 100u / 1024u),"%02") <<
        " kb transfered\n  " << list[i] << " to " << file.fileName() << "\n"
      );
      all += l;
    }
    catch( ksys::ExceptionSP & e ){
      switch( e->code() ){
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
          if( e->code() < ksock::AsyncSocket::aeMagic || e->code() >= eCount ) throw;
      }
      ksys::stdErr.log(
        ksys::lmDIRECT,
        utf8::String::Stream() << "job " << host_ << " failed: " <<
        ksys::strError(e->code()) << ", " << list[i] << "\n"
      );
    }
    list.resize(list.count() - 1);
  }
  if( (ptime = getlocaltimeofday() - ptime) == 0 ) ptime = 1;
  if( atime == 0 ) atime = 1;
  ksys::stdErr.log(
    ksys::lmDIRECT,
    utf8::String::Stream() <<
    section_ << " " << host_ << " operation get complete.\n" <<
    "  elapsed: " << utf8::elapsedTime2Str(ptime) << ", avg speed " << 
    (all * 1000000u / atime) / 1024u << "." << 
    utf8::String::Stream::Format(unsigned((all * 1000000u / atime) % 1024u) * 100u / 1024u,"%02") <<
    " kbps, " << all / 1024u << "." <<
    utf8::String::Stream::Format((all % 1024u) * 100u / 1024u,"%02") << " kb transfered\n" <<
    "  compression:\n" <<
    "    receive  difference: " << rcDifference() / 1024u << "." <<
    utf8::String::Stream::Format(unsigned(abs(int(rcDifference() % 1024u)) * 100u / 1024u),"%02") <<
    " kb, ratio: " << 
    utf8::String::Stream::Format(rcRatio() / 100u,"%3") << "." <<
    utf8::String::Stream::Format(rcRatio() % 100u,"%02") << "%\n" <<
    "    transmit difference: " << scDifference() / 1024u << "." <<
    utf8::String::Stream::Format(unsigned(abs(int(scDifference() % 1024u)) * 100u / 1024u),"%02") <<
    " kb, ratio: " << 
    utf8::String::Stream::Format(scRatio() / 100u,"%3") << "." <<
    utf8::String::Stream::Format(scRatio() % 100u,"%02") << "%\n" <<
    "    overall  difference: " << rscDifference() / 1024u << "." <<
    utf8::String::Stream::Format(unsigned(abs(int(rscDifference() % 1024u)) * 100u / 1024u),"%02") <<
    " kb, ratio: " << 
    utf8::String::Stream::Format(rscRatio() / 100u,"%3") << "." <<
    utf8::String::Stream::Format(rscRatio() % 100u,"%02") << "%\n"
  );
}
//------------------------------------------------------------------------------
void KFTPClient::main()
{
// test
  ksys::Fetcher fetch;
  fetch.url("http://www.firebirdsql.org/download/prerelease/rlsnotes20rc3_0200_82.zip");
  fetch.proxy("korvin:WFa1PXt-@192.168.201.2:3128");
  fetch.fetch("rlsnotes20rc3_0200_82.zip");



  remoteAddress_.resolve(host_,MSFTPDefaultPort);
  connect(remoteAddress_);

/*  ksys::AsyncFile file("c:\\dump.tmp");
  file.open();
  ksys::AsyncFile file2("c:\\dump.tmp");
  file2.open();
//  file.tryWRLock(0,0);
//  file2.tryRDLock(0,0);
  for( intptr_t i = 1000; i >= 0; i-- ){
    char buf[6];
    file.writeBuffer("abcdef",6);
    file.seek(0);
    file.readBuffer(buf,6);
    file2.writeBuffer("zxcvbn",6);
    file2.seek(0);
    file2.readBuffer(buf,6);
  }
  for( intptr_t i = 100000; i >= 0; i-- ){
    thread()->postEvent(ksys::etDispatch,this);
    switchFiber(mainFiber());
  }*/

  auth();
  put();
  get();
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

    void open();
  protected:
  private:
    ksys::ConfigSP config_;
};
//------------------------------------------------------------------------------
KFTPShell::~KFTPShell()
{
  close();
}
//------------------------------------------------------------------------------
KFTPShell::KFTPShell() : config_(new ksys::InterlockedConfig<ksys::FiberInterlockedMutex>)
{
}
//------------------------------------------------------------------------------
void KFTPShell::open()
{
  intptr_t i;
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
  for( i = config_->sectionCount() - 1; i >= 0; i-- ){
    utf8::String sectionName(config_->section(i).name());
    if( sectionName.strncasecmp("job",3) == 0 )
      attachFiber(new KFTPClient(config_,sectionName));
  }
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
    ksys::Config::defaultFileName(SYSCONF_DIR + "msftp.conf");
#ifndef NDEBUG
    fprintf(stderr,"%s\n",(const char *) ksys::getCurrentDir().getOEMString());
#endif
    bool dispatch = true;
    for( u = 1; u < ksys::argv().count(); u++ ){
      if( ksys::argv()[u].strcmp("-c") == 0 && u + 1 < ksys::argv().count() ){
        ksys::Config::defaultFileName(ksys::argv()[u + 1]);
      }
      else if( ksys::argv()[i].strcmp("--sha256") == 0 && u + 1 < ksys::argv().count() ){
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
      shell.open();
    }
  }
  catch( ksys::ExceptionSP & e ){
    e->writeStdError();
    errcode = e->code();
  }
  return errcode;
}
//------------------------------------------------------------------------------
