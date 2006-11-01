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
#define _VERSION_C_AS_HEADER_
#include "version.c"
#undef _VERSION_C_AS_HEADER_
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class KFTPClient : public ksock::ClientFiber {
  public:
    virtual ~KFTPClient();
    KFTPClient(const ksys::ConfigSP & config,const utf8::String & section,int & errorCode);
  protected:
    void main();
  private:
    ksys::ConfigSP config_;
    utf8::String section_;
    utf8::String host_;
    ksock::SockAddr remoteAddress_;
    int & errorCode_;
    ksys::LogFile * log_;
    ksys::LogFile logFile_;

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
KFTPClient::KFTPClient(const ksys::ConfigSP & config,const utf8::String & section,int & errorCode) :
  config_(config), section_(section), errorCode_(errorCode), log_(&ksys::stdErr)
{
  host_ = config_->section(section_).text();
}
//------------------------------------------------------------------------------
KFTPClient & KFTPClient::checkCode(int32_t code,int32_t noThrowCode)
{
  if( code != eOK && code != noThrowCode )
    newObject<ksys::Exception>(code,__PRETTY_FUNCTION__)->throwSP();
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
  crc = config_->section(section_).section("compression").value("crc",crc);
  uintptr_t compressionLevel = config_->section("compression").value("level",3);
  compressionLevel = config_->section(section_).section("compression").value("level",compressionLevel);
  bool optimize = config_->section("compression").value("optimize",false);
  optimize = config_->section(section_).section("compression").value("optimize",optimize);
  uintptr_t bufferSize = config_->section("compression").value("buffer_size",getpagesize());
  bufferSize = config_->section(section_).section("compression").value("buffer_size",bufferSize);
  bool noAuth = config_->value("noauth",false);
  noAuth = config_->section(section_).value("noauth",noAuth);

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
      bufferSize,
      noAuth
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
  uint64_t partialBlockSize = config_->value("partial_block_size",getpagesize());
  partialBlockSize = config_->section(section_).section("put").value("partial_block_size",partialBlockSize);
  if( config_->section(section_).section("put").isValue("log_file") ){
    logFile_.codePage(config_->section(section_).section("put").value("log_file_codepage",utf8::getCodePage(CP_ACP)));
    logFile_.fileName(config_->section(section_).section("put").text("log_file"));
    log_ = &logFile_;
  }

  ksys::Vector<utf8::String> list;
  getDirList(list,local,exclude,recursive,false,true);
  utf8::String mode(config_->text("mode","auto"));
  mode = config_->section(section_).section("put").text("mode",mode);
  utf8::String remotePath(
    ksys::includeTrailingPathDelimiter(
      ksys::unScreenString(config_->section(section_).section("put").text("remote"))
    )
  );
  int64_t all = 0, ptime = getlocaltimeofday(), atime = 0, ttime;
  for( intptr_t i = list.count() - 1; i >= 0; i-- ){
    uint64_t l, ll, lp, rl;
    utf8::String rfile(
      remotePath + utf8::String(utf8::String::Iterator(list[i]) + localPath.strlen())
    );
    try {
      int8_t cmd = int8_t(cmPutFile);
      uint64_t bs = (uint64_t) config_->value("buffer_size",getpagesize());
      bs = (uint64_t) config_->section(section_).section("put").value("buffer_size",bs);
      if( bs == 0 ) bs = getpagesize();
      ksys::AsyncFile file(list[i]);
      MSFTPStat rmst, lmst;
      if( lmst.stat(file.fileName()) ){
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
        else if( mode.strcasecmp("partial") == 0 ){
          *this << int8_t(cmStat) << rfile;
          read(&rmst,sizeof(rmst));
          getCode(eBadPathName);
          if( rmst.st_size_ > lmst.st_size_ ){
            ll = file.size();
            *this << int8_t(cmResize) << rfile << ll;
            getCode();
          }
          ll = file.size();
          cmd = cmPutFilePartial;
        }
        if( bs > ll ) bs = ll;
        if( cmd == cmPutFilePartial ) bs = partialBlockSize;
        *this << cmd << rfile << file.tell() << ll << mtime << bs;
        if( cmd == cmPutFilePartial ) *this << partialBlockSize;
        getCode();
        ksys::AutoPtr<uint8_t> b;
        b.alloc((size_t) bs);
        ttime = getlocaltimeofday();
        ksys::SHA256 lhash, rhash;
        memset(lhash.sha256(),0,lhash.size());
        memset(rhash.sha256(),1,rhash.size());
        for( rl = l = 0; l < ll; l += lp ){
          file.readBuffer(b,lp = ll - l > bs ? bs : ll - l);
          if( cmd == cmPutFilePartial ){
            lhash.make(b,(uintptr_t) lp);
            *this << l << lp;
            writeBuffer(lhash.sha256(),lhash.size());
            readBuffer(rhash.sha256(),rhash.size());
          }
          if( memcmp(lhash.sha256(),rhash.sha256(),rhash.size()) != 0 ){
            write(b.ptr(),lp);
            getCode();
            rl += lp;
          }
          if( terminated_ ) return;
        }
        if( cmd == cmPutFilePartial && ll > 0 ) *this << uint64_t(0) << uint64_t(0);
        getCode();
        ttime = getlocaltimeofday() - ttime;
        atime += ttime;
        if( ttime == 0 ) ttime = 1;
        l = rl;
        log_->debug(l > 0 ? 1 : 2,
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
      e->writeStdError(log_);
      log_->debug(3,
        utf8::String::Stream() << section_ << " " << host_ << " failed: " <<
        ksys::strError(e->code()) << ", " << list[i] << "\n"
      );
    }
    list.resize(list.count() - 1);
  }
  if( (ptime = getlocaltimeofday() - ptime) == 0 ) ptime = 1;
  if( atime == 0 ) atime = 1;
  log_->debug(0,
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
  uint64_t partialBlockSize = config_->value("partial_block_size",getpagesize());
  partialBlockSize = config_->section(section_).section("get").value("partial_block_size",partialBlockSize);
  if( config_->section(section_).section("put").isValue("log_file") ){
    logFile_.codePage(config_->section(section_).section("get").value("log_file_codepage",utf8::getCodePage(CP_ACP)));
    logFile_.fileName(config_->section(section_).section("get").text("log_file"));
    log_ = &logFile_;
  }

  uint64_t all = 0, ptime = getlocaltimeofday(), atime = 0, ttime;
  uint64_t l, ll, lp, r, wl;
  ksys::Vector<utf8::String> list;
  *this << int8_t(cmList) << remote << exclude << uint8_t(recursive);
  getCode();
  *this >> ll;
  while( ll-- > 0 ) list.add(readString());
  for( intptr_t i = list.count() - 1; i >= 0; i-- ){
    try {
      ksys::AsyncFile file(localPath + list[i]);
      file.createIfNotExist(true);
      if( !file.tryOpen() ){
        ksys::createDirectory(
          ksys::excludeTrailingPathDelimiter(
            ksys::getPathFromPathName(file.fileName())
          )
        );
        file.open();
      }
      MSFTPStat rmst, lmst;
      lmst.stat(file.fileName());
      ll = 0;
      *this << int8_t(cmStat) << remotel + list[i];
      read(&rmst,sizeof(rmst));
      getCode();
      int8_t cmd = cmGetFile;
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
      else if( mode.strcasecmp("partial") == 0 ){
        if( lmst.st_size_ > rmst.st_size_ ) file.resize(rmst.st_size_);
        cmd = cmGetFilePartial;
        ll = rmst.st_size_;
      }
      if( bs > ll ) bs = ll;
      if( cmd == cmGetFilePartial ) bs = partialBlockSize;
      *this << cmd << remotel + list[i] << file.tell() << ll;
      if( cmd == cmGetFilePartial ) *this << partialBlockSize;
      getCode();
      ksys::AutoPtr<uint8_t> b;
      b.alloc((size_t) bs);
      ttime = getlocaltimeofday();
      ksys::SHA256 lhash, rhash;
      memset(lhash.sha256(),0,lhash.size());
      memset(rhash.sha256(),1,rhash.size());
      for( wl = l = 0; l < ll; l += lp ){
        lp = ll - l > bs ? bs : ll - l;
        if( cmd == cmGetFilePartial ){
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
        if( memcmp(lhash.sha256(),rhash.sha256(),rhash.size()) != 0 ){
          read(b.ptr(),lp);
          if( file.size() < l ) file.resize(l);
          file.writeBuffer(l,b,lp);
          wl += lp;
        }
        if( terminated_ ) return;
      }
      ttime = getlocaltimeofday() - ttime;
      atime += ttime;
      if( ttime == 0 ) ttime = 1;
      l = wl;
      log_->debug(l > 0 ? 1 : 2,
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
      e->writeStdError(log_);
      log_->debug(3,
        utf8::String::Stream() << section_ << " " << host_ << " failed: " <<
        ksys::strError(e->code()) << ", " << list[i] << "\n"
      );
    }
    list.resize(list.count() - 1);
  }
  if( (ptime = getlocaltimeofday() - ptime) == 0 ) ptime = 1;
  if( atime == 0 ) atime = 1;
  log_->debug(0,
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
  try {
    if( config_->section(section_).isValue("log_file") ){
      logFile_.codePage(config_->section(section_).value("log_file_codepage",utf8::getCodePage(CP_ACP)));
      logFile_.fileName(config_->section(section_).text("log_file"));
      log_ = &logFile_;
    }
    remoteAddress_.resolve(host_,MSFTPDefaultPort);
/*    remoteAddress_.addr4_.sin_len = sizeof(remoteAddress_.addr4_);
    remoteAddress_.addr4_.sin_family = PF_INET;
    remoteAddress_.addr4_.sin_addr.s_addr = inet_addr("192.168.201.200");
    remoteAddress_.addr4_.sin_port = htons(2121);*/
    connect(remoteAddress_);

    auth();
    put();
    get();
    *this << int8_t(cmQuit);
    getCode();
  }
  catch( ksys::ExceptionSP & e ){
    log_->debug(8,utf8::String::Stream() << section_ << " " << host_ << " incomplete.\n");
    if( errorCode_ == 0 ) errorCode_ = e->code();
    log_->close();
    throw;
  }
  catch( ... ){
    if( errorCode_ == 0 ) errorCode_ = -1;
    log_->close();
    throw;
  }
  log_->close();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class KFTPShell : public ksock::Client {
  public:
    ~KFTPShell();
    KFTPShell(int & errorCode);

    void open();
  protected:
  private:
    ksys::ConfigSP config_;
    int & errorCode_;
};
//------------------------------------------------------------------------------
KFTPShell::~KFTPShell()
{
  close();
}
//------------------------------------------------------------------------------
KFTPShell::KFTPShell(int & errorCode) :
  config_(
    newObject<ksys::InterlockedConfig<ksys::FiberInterlockedMutex> >()
  ),
  errorCode_(errorCode)
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
  ksys::stdErr.fileName(
    config_->value("log_file",ksys::stdErr.fileName())
  );
  ksys::stdErr.setRedirect(
    config_->value("log_redirect",utf8::String())
  );
  for( i = config_->sectionCount() - 1; i >= 0; i-- ){
    utf8::String sectionName(config_->section(i).name());
    if( sectionName.strncasecmp("job",3) == 0 )
      attachFiber(newObjectV<KFTPClient>(config_,sectionName,errorCode_));
  }
}
//------------------------------------------------------------------------------
struct rv {
  void * a0;
  void * a1;
  void * a2;
};
struct rv dummy(
  void * param1,
  void * param2,
  void * param3,
  void * param4,
  void * param5,
  void * param6,
  void * param7,
  void * param8,
  void * param9,
  void * param10,
  void * param11,
  void * param12)
{
  return rv();
}
//------------------------------------------------------------------------------
int main(int argc,char * argv[])
{
  struct rv a;
  a = dummy(
    (void *) 0x1, (void *) 0x2, (void *) 0x3, (void *) 0x4,
    (void *) 0x5, (void *) 0x6, (void *) 0x7, (void *) 0x8,
    (void *) 0x9,(void *) 0x10,(void *) 0x11,(void *) 0x12
  );
  
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
      if( ksys::argv()[u].strcmp("--version") == 0 ){
        ksys::stdErr.debug(9,utf8::String::Stream() << msftp_version.tex_ << "\n");
        fprintf(stdout,"%s\n",msftp_version.tex_);
        dispatch = false;
        continue;
      }
      if( ksys::argv()[u].strcmp("-c") == 0 && u + 1 < ksys::argv().count() ){
        ksys::Config::defaultFileName(ksys::argv()[u + 1]);
      }
      else if( ksys::argv()[u].strcmp("--log") == 0 && u + 1 < ksys::argv().count() ){
        ksys::stdErr.fileName(ksys::argv()[u + 1]);
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
      KFTPShell shell(errcode);
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
