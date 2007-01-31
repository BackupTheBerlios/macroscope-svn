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
#include "../msftpd/msftpd.h"
//------------------------------------------------------------------------------
#define _VERSION_C_AS_HEADER_
#include "version.c"
#undef _VERSION_C_AS_HEADER_
//------------------------------------------------------------------------------
using namespace ksys;
//------------------------------------------------------------------------------
class KFTPShell;
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class KFTPClient : public ksock::ClientFiber {
  public:
    virtual ~KFTPClient();
    KFTPClient() {}
    KFTPClient(KFTPShell & shell,const utf8::String & section);
  protected:
    void main();
  private:
    KFTPShell * shell_;
    utf8::String section_;
    utf8::String host_;
    ksock::SockAddr remoteAddress_;
    LogFile * log_;
    LogFile logFile_;

    KFTPClient & checkCode(int32_t code,int32_t noThrowCode = eOK);
    KFTPClient & getCode(int32_t noThrowCode = eOK);
    void auth();
    void put();
    void get();
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class KFTPShell : public ksock::Client {
  friend class KFTPClient;
  public:
    ~KFTPShell();
    KFTPShell(int & errorCode);

    void open();
  protected:
  private:
    ConfigSP config_;
    int & errorCode_;
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
KFTPClient::~KFTPClient()
{
}
//------------------------------------------------------------------------------
KFTPClient::KFTPClient(KFTPShell & shell,const utf8::String & section) :
  shell_(&shell), section_(section), log_(&stdErr)
{
  host_ = shell_->config_->section(section_).text();
}
//------------------------------------------------------------------------------
KFTPClient & KFTPClient::checkCode(int32_t code,int32_t noThrowCode)
{
  if( code != eOK && code != noThrowCode )
    newObjectV1C2<Exception>(code,__PRETTY_FUNCTION__)->throwSP();
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
  AuthParams ap;
  ap.maxRecvSize_ = shell_->config_->value("max_recv_size",-1);
  ap.maxRecvSize_ = shell_->config_->section(section_).value("max_recv_size",ap.maxRecvSize_);
  ap.maxSendSize_ = shell_->config_->value("max_send_size",-1);
  ap.maxSendSize_ = shell_->config_->section(section_).value("max_send_size",ap.maxSendSize_);
  ap.recvTimeout_ = shell_->config_->value("recv_timeout",-1);
  ap.recvTimeout_ = shell_->config_->section(section_).value("recv_timeout",ap.recvTimeout_);
  if( ap.recvTimeout_ != ~uint64_t(0) ) ap.recvTimeout_ *= 1000000u;
  ap.sendTimeout_ = shell_->config_->value("send_timeout",-1);
  ap.sendTimeout_ = shell_->config_->section(section_).value("send_timeout",ap.sendTimeout_);
  if( ap.sendTimeout_ != ~uint64_t(0) ) ap.sendTimeout_ *= 1000000u;
  ap.user_ = shell_->config_->text("user");
  ap.user_ = shell_->config_->section(section_).text("user",ap.user_);
  ap.password_ = shell_->config_->text("password");
  ap.password_ = shell_->config_->section(section_).text("password",ap.password_);
  ap.encryption_ = shell_->config_->section("encryption").text(utf8::String(),"default");
  ap.threshold_ = shell_->config_->section("encryption").value("threshold",1024 * 1024);
  ap.encryption_ = shell_->config_->section(section_).section("encryption").text(utf8::String(),ap.encryption_);
  ap.threshold_ = shell_->config_->section(section_).section("encryption").value("threshold",ap.threshold_);
  ap.compression_ = shell_->config_->section("compression").text(utf8::String(),"default");
  ap.compression_ = shell_->config_->section(section_).section("compression").text(utf8::String(),ap.compression_);
  ap.compressionType_ = shell_->config_->section("compression").value("type","default");
  ap.compressionType_ = shell_->config_->section(section_).section("compression").value("type",ap.compressionType_);
  ap.crc_ = shell_->config_->section("compression").value("crc","default");
  ap.crc_ = shell_->config_->section(section_).section("compression").value("crc",ap.crc_);
  ap.level_ = shell_->config_->section("compression").value("level",9);
  ap.level_ = shell_->config_->section(section_).section("compression").value("level",ap.level_);
  ap.optimize_ = shell_->config_->section("compression").value("optimize",true);
  ap.optimize_ = shell_->config_->section(section_).section("compression").value("optimize",ap.optimize_);
  ap.bufferSize_ = shell_->config_->section("compression").value("buffer_size",getpagesize() * 16);
  ap.bufferSize_ = shell_->config_->section(section_).section("compression").value("buffer_size",ap.bufferSize_);
  ap.noAuth_ = shell_->config_->value("noauth",false);
  ap.noAuth_ = shell_->config_->section(section_).value("noauth",ap.noAuth_);
  checkCode(clientAuth(ap));
}
//------------------------------------------------------------------------------
void KFTPClient::put()
{
  if( !shell_->config_->section(section_).isSection("put") ) return;
  utf8::String local(shell_->config_->section(section_).section("put").text("local"));
  utf8::String localPath(
    includeTrailingPathDelimiter(getPathFromPathName(local))
  );
  utf8::String exclude(shell_->config_->section(section_).section("put").text("exclude"));
  bool recursive = shell_->config_->value("recursive",true);
  recursive = shell_->config_->section(section_).section("put").value("recursive",recursive);
  uint64_t partialBlockSize = shell_->config_->value("partial_block_size",getpagesize());
  partialBlockSize = shell_->config_->section(section_).section("put").value("partial_block_size",partialBlockSize);
  if( shell_->config_->section(section_).section("put").isValue("log_file") ){
    logFile_.codePage(shell_->config_->section(section_).section("put").value("log_file_codepage",utf8::getCodePage(CP_ACP)));
    logFile_.fileName(shell_->config_->section(section_).section("put").text("log_file"));
    log_ = &logFile_;
  }

  Vector<utf8::String> list;
  getDirList(list,local,exclude,recursive,false,true);
  utf8::String mode(shell_->config_->text("mode","auto"));
  mode = shell_->config_->section(section_).section("put").text("mode",mode);
  utf8::String remotePath(
    includeTrailingPathDelimiter(
      shell_->config_->section(section_).section("put").text("remote")
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
      uint64_t bs = (uint64_t) shell_->config_->value("buffer_size",getpagesize());
      bs = (uint64_t) shell_->config_->section(section_).section("put").value("buffer_size",bs);
      if( bs == 0 ) bs = getpagesize();
      AsyncFile file(list[i]);
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
        AutoPtr<uint8_t> b;
        b.alloc((size_t) bs);
        ttime = getlocaltimeofday();
        SHA256 lhash, rhash;
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
        utf8::String::Stream stream;
        stream <<
          section_ << " " << host_ << " elapsed: " << utf8::elapsedTime2Str(ttime) <<
          ", avg speed " << (l * 1000000u / ttime) / 1024u << "." <<
          utf8::String::Stream::Format(unsigned((l * 1000000u / ttime) % 1024u) * 100u / 1024u,"%02") <<
          " kbps, " << l / 1024u << "." <<
          utf8::String::Stream::Format(unsigned((l % 1024u) * 100u / 1024u),"%02") <<
          " kb transfered\n  " << list[i] << " to " << rfile << "\n"
        ;
        log_->debug(l > 0 ? 1 : 2,stream);
        if( log_ != &stdErr ) stdErr.debug(l > 0 ? 1 : 2,stream);
        all += l;
      }
    }
    catch( ExceptionSP & e ){
      switch( e->code() ){
        case 0 :
#if defined(__WIN32__) || defined(__WIN64__)
        case ERROR_SHARING_VIOLATION + errorOffset :
        case ERROR_LOCK_VIOLATION + errorOffset :
        case ERROR_INVALID_ACCESS + errorOffset :
        case ERROR_FILE_NOT_FOUND + errorOffset :
        case ERROR_PATH_NOT_FOUND + errorOffset :
        case ERROR_ACCESS_DENIED + errorOffset :
        case ERROR_INVALID_DRIVE + errorOffset :
        case ERROR_SEEK + errorOffset :
        case ERROR_SECTOR_NOT_FOUND + errorOffset :
#else
        case EPERM : case ENOENT : case EACCES : case ENOTDIR :
#endif
          break;
        default :
          if( e->code() < ksock::AsyncSocket::aeMagic || e->code() >= eCount ) throw;
      }
      e->writeStdError(log_);
      utf8::String::Stream stream;
      stream << section_ << " " << host_ << " failed: " <<
        strError(e->code()) << ", " << list[i] << "\n"
      ;
      log_->debug(3,stream);
      if( log_ != &stdErr ) stdErr.debug(3,stream);
    }
    list.resize(list.count() - 1);
  }
  if( (ptime = getlocaltimeofday() - ptime) == 0 ) ptime = 1;
  if( atime == 0 ) atime = 1;
  utf8::String::Stream stream;
  stream <<
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
  ;
  log_->debug(0,stream);
  if( log_ != &stdErr ) stdErr.debug(0,stream);
}
//------------------------------------------------------------------------------
void KFTPClient::get()
{
  if( !shell_->config_->section(section_).isSection("get") ) return;
  utf8::String localPath(
    includeTrailingPathDelimiter(
      shell_->config_->section(section_).section("get").text("local")
    )
  );
  utf8::String exclude(shell_->config_->section(section_).section("get").text("exclude"));
  bool recursive = shell_->config_->value("recursive",true);
  recursive = shell_->config_->section(section_).section("get").value("recursive",recursive);
  utf8::String mode(shell_->config_->text("mode","auto"));
  mode = shell_->config_->section(section_).section("get").text("mode",mode);
  utf8::String remote(
    shell_->config_->section(section_).section("get").text("remote")
  );
  utf8::String remotel(includeTrailingPathDelimiter(getPathFromPathName(remote)));
  uint64_t bs = (uint64_t) shell_->config_->value("buffer_size",getpagesize());
  bs = (uint64_t) shell_->config_->section(section_).section("get").value("buffer_size",bs);
  if( bs == 0 ) bs = getpagesize();
  uint64_t partialBlockSize = shell_->config_->value("partial_block_size",getpagesize());
  partialBlockSize = shell_->config_->section(section_).section("get").value("partial_block_size",partialBlockSize);
  if( shell_->config_->section(section_).section("get").isValue("log_file") ){
    logFile_.codePage(shell_->config_->section(section_).section("get").value("log_file_codepage",utf8::getCodePage(CP_ACP)));
    logFile_.fileName(shell_->config_->section(section_).section("get").text("log_file"));
    log_ = &logFile_;
  }

  uint64_t all = 0, ptime = getlocaltimeofday(), atime = 0, ttime;
  uint64_t l, ll, lp, r, wl;
  Vector<utf8::String> list;
  *this << int8_t(cmList) << remote << exclude << uint8_t(recursive);
  getCode();
  *this >> ll;
  while( ll-- > 0 ) list.add(readString());
  for( intptr_t i = list.count() - 1; i >= 0; i-- ){
    try {
      AsyncFile file(localPath + list[i]);
      file.createIfNotExist(true).open();
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
      AutoPtr<uint8_t> b;
      b.alloc((size_t) bs);
      ttime = getlocaltimeofday();
      SHA256 lhash, rhash;
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
    catch( ExceptionSP & e ){
      switch( e->code() ){
        case 0 :
#if defined(__WIN32__) || defined(__WIN64__)
        case ERROR_SHARING_VIOLATION + errorOffset :
        case ERROR_LOCK_VIOLATION + errorOffset :
        case ERROR_INVALID_ACCESS + errorOffset :
        case ERROR_FILE_NOT_FOUND + errorOffset :
        case ERROR_PATH_NOT_FOUND + errorOffset :
        case ERROR_ACCESS_DENIED + errorOffset :
        case ERROR_INVALID_DRIVE + errorOffset :
        case ERROR_SEEK + errorOffset :
        case ERROR_SECTOR_NOT_FOUND + errorOffset :
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
        strError(e->code()) << ", " << list[i] << "\n"
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
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//------------------------------------------------------------------------------
class ZebexPDL {
  public:
    ~ZebexPDL();
    ZebexPDL();

    void fiberExecute();
  protected:
  private:
    enum PCC { // Protocol Control Code
      DLE = 0,
      SOH = 1,  //	A	START OF HEADING
      STX = 2,  // B START OF TEXT
      ETX = 3,  // C END OF TEXT
      EOT = 4,  // D END OF TRANSMISSION
      ENQ = 5,  // E ENQUIRY
      ACK = 6,  // F ACK ACKNOWLEDGE
      NAK = 21, // U NEGATIVE ACKNOWLEDGE
      RAR = 0x41, // Remove all records
      GPR = 0xa5, // Get parameters
    };
    enum PEC { // Protocol Error Code
      E_OK = 0,
      E_CRC_ERROR = 0x30,
      E_EOF = 0x31,
      E_ILLEGAL_VALUE = 0x32
    };
    uintptr_t serialPortNumber_;
    AsyncFile * serial_;
    uint16_t crc_;

    ZebexPDL & operator << (const PCC & pcc);
    ZebexPDL & operator >> (PCC & pcc);
    ZebexPDL & operator << (const uint8_t & a);
    ZebexPDL & operator >> (uint8_t & a);
    ZebexPDL & operator << (const uint16_t & a);
    ZebexPDL & operator >> (uint16_t & a);
    
    void clearTerminalHelper(uint8_t a);
    void clearTerminal();
};
//------------------------------------------------------------------------------
ZebexPDL::~ZebexPDL()
{
}
//------------------------------------------------------------------------------
ZebexPDL::ZebexPDL() : serialPortNumber_(0), serial_(NULL)
{
}
//------------------------------------------------------------------------------
ZebexPDL & ZebexPDL::operator << (const PCC & pcc)
{
  serial_->writeBuffer(&pcc,1);
  crc_ += pcc;
  return *this;
}
//------------------------------------------------------------------------------
ZebexPDL & ZebexPDL::operator >> (PCC & pcc)
{
  serial_->readBuffer(&pcc,1);
  crc_ += pcc;
  return *this;
}
//------------------------------------------------------------------------------
ZebexPDL & ZebexPDL::operator << (const uint8_t & a)
{
  serial_->writeBuffer(&a,sizeof(a));
  crc_ += a;
  return *this;
}
//------------------------------------------------------------------------------
ZebexPDL & ZebexPDL::operator >> (uint8_t & a)
{
  serial_->readBuffer(&a,sizeof(a));
  crc_ += a;
  return *this;
}
//------------------------------------------------------------------------------
ZebexPDL & ZebexPDL::operator << (const uint16_t & a)
{
  serial_->writeBuffer(&a,sizeof(a));
  for( uintptr_t i = 0; i < sizeof(a); i++ ) crc_ += ((const uint8_t *) &a)[i];
  return *this;
}
//------------------------------------------------------------------------------
ZebexPDL & ZebexPDL::operator >> (uint16_t & a)
{
  serial_->readBuffer(&a,sizeof(a));
  for( uintptr_t i = 0; i < sizeof(a); i++ ) crc_ += ((const uint8_t *) &a)[i];
  return *this;
}
//------------------------------------------------------------------------------
void ZebexPDL::clearTerminalHelper(uint8_t a)
{
  using namespace ksys;
  uint8_t e;
// Handshaking ?
  *this << uint8_t(0x05);
  serial_->waitCommEvent();
  *this >> e;
  if( e == 0x06 ){
//    newObject<Exception>(ERROR_INVALID_DATA,__PRETTY_FUNCTION__)->throwSP();
  }
  *this << uint8_t(0x55);
  serial_->waitCommEvent();
  *this >> e;
  if( e == 0x07 ){
//    newObject<Exception>(ERROR_INVALID_DATA,__PRETTY_FUNCTION__)->throwSP();
  }
  *this << uint8_t(0x54);
  serial_->waitCommEvent();
  *this >> e;
  if( e == 0x06 ){
//    newObject<Exception>(ERROR_INVALID_DATA,__PRETTY_FUNCTION__)->throwSP();
  }
// Command on clear terminal ?
  uint8_t clr[] = { 0x02, 0x43, 0x45, a, 0x03 };
  serial_->writeBuffer(clr,sizeof(clr));
//  *this << uint8_t(0x02) << uint8_t(0x43) << uint8_t(0x45) << a << uint8_t(0x03);
  serial_->waitCommEvent();
  *this >> e;
  if( e == 0x06 ){
//    newObject<Exception>(ERROR_INVALID_DATA,__PRETTY_FUNCTION__)->throwSP();
  }
// May be end of exchage marker code ?
  *this << uint8_t(0x1b);
}
//------------------------------------------------------------------------------
void ZebexPDL::clearTerminal()
{
  using namespace ksys;
  AsyncFile serial;
  serial_ = &serial;
  serial.exclusive(true).fileName(
    "COM" + utf8::int2Str(serialPortNumber_) + ":"
  ).open();
#if defined(__WIN32__) || defined(__WIN64__)
  if( SetupComm(serial.descriptor(),1600,1600) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  COMMTIMEOUTS cto;
  memset(&cto,0,sizeof(cto));
  cto.ReadIntervalTimeout = MAXDWORD;
  if( SetCommTimeouts(serial.descriptor(),&cto) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( SetCommMask(serial.descriptor(),EV_RXCHAR) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( PurgeComm(serial.descriptor(),PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  /*if( EscapeCommFunction(serial.descriptor(),CLRRTS) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( EscapeCommFunction(serial.descriptor(),CLRDTR) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( EscapeCommFunction(serial.descriptor(),CLRBREAK) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( EscapeCommFunction(serial.descriptor(),SETDTR) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }*/
#endif
  clearTerminalHelper(0x31);
  ksleep(200000);
  clearTerminalHelper(0x32);
  ksleep(200000);
  clearTerminalHelper(0x33);
#if defined(__WIN32__) || defined(__WIN64__)
  if( EscapeCommFunction(serial.descriptor(),CLRDTR) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( PurgeComm(serial.descriptor(),PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#endif
}
//------------------------------------------------------------------------------
void ZebexPDL::fiberExecute()
{
  using namespace ksys;
/*  HMODULE pdx = LoadLibraryEx(L"C:\\Korvin\\1C\\Эталон\\PDXDLL.dll",NULL,0);

  union {
    void (* clearTable)(int,int);
    void * p;
  };

  p = GetProcAddress(pdx,"?PDXClearTable@@YA_NH@Z");

  clearTable(3,1);

  FreeLibrary(pdx);*/

  serialPortNumber_ = 3;

  clearTerminal();

  AsyncFile serial;
  serial_ = &serial;
  serial.exclusive(true).fileName(
    "COM" + utf8::int2Str(serialPortNumber_) + ":"
  ).open();
#if defined(__WIN32__) || defined(__WIN64__)
  /*if( SetupComm(serial.descriptor(),1600,1600) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }*/
  COMMTIMEOUTS cto;
  memset(&cto,0,sizeof(cto));
  cto.ReadIntervalTimeout = 100;//MAXDWORD;
  if( SetCommTimeouts(serial.descriptor(),&cto) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( SetCommMask(serial.descriptor(),EV_RXCHAR) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( PurgeComm(serial.descriptor(),PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }

#define FILE_DEVICE_SERIAL_PORT 0x0000001b
#define METHOD_BUFFERED 0
#define FILE_ANY_ACCESS 0
#define IOCTL_SERIAL_SET_RTS CTL_CODE(FILE_DEVICE_SERIAL_PORT,12,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_CLR_RTS CTL_CODE(FILE_DEVICE_SERIAL_PORT,13,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_SET_DTR CTL_CODE(FILE_DEVICE_SERIAL_PORT, 9,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_CLR_DTR CTL_CODE(FILE_DEVICE_SERIAL_PORT,10,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_CLR_DTR CTL_CODE(FILE_DEVICE_SERIAL_PORT,10,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_SERIAL_GET_COMMSTATUS CTL_CODE(FILE_DEVICE_SERIAL_PORT,27,METHOD_BUFFERED,FILE_ANY_ACCESS)
//  DWORD bytesReturned;
/*  if( DeviceIoControl(serial.descriptor(),IOCTL_SERIAL_CLR_RTS,NULL,0,NULL,0,&bytesReturned,NULL) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( DeviceIoControl(serial.descriptor(),IOCTL_SERIAL_CLR_DTR,NULL,0,NULL,0,&bytesReturned,NULL) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( DeviceIoControl(serial.descriptor(),IOCTL_SERIAL_SET_DTR,NULL,0,NULL,0,&bytesReturned,NULL) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }*/
  DCB dcb;
  if( GetCommState(serial.descriptor(),&dcb) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  dcb.BaudRate = CBR_9600;
  dcb.StopBits = ONESTOPBIT;
  if( SetCommState(serial.descriptor(),&dcb) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }

  if( EscapeCommFunction(serial.descriptor(),CLRRTS) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( EscapeCommFunction(serial.descriptor(),CLRDTR) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( EscapeCommFunction(serial.descriptor(),CLRBREAK) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( EscapeCommFunction(serial.descriptor(),SETDTR) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#endif
  uint8_t err;
  /*uint8_t head, tail;
  uint16_t crc;
  crc_ = 0;*/
//  *this << STX << GPR << ETX << crc_;
  /*typedef struct _COMSTAT {
    DWORD fCtsHold :1;
    DWORD fDsrHold :1;
    DWORD fRlsdHold :1;
    DWORD fXoffHold :1;
    DWORD fXoffSent :1;
    DWORD fEof :1;
    DWORD fTxim :1;
    DWORD fReserved :25;
    DWORD cbInQue;
    DWORD cbOutQue;
  } COMSTAT, *LPCOMSTAT;*/
  typedef struct _SERIAL_DEV_STATUS {
    DWORD Errors;
    COMSTAT ComStat;
  } SERIAL_DEV_STATUS, *PSERIAL_DEV_STATUS;

//  SERIAL_DEV_STATUS sds;
  /*if( DeviceIoControl(serial.descriptor(),IOCTL_SERIAL_GET_COMMSTATUS,NULL,0,&sds,sizeof(sds),&bytesReturned,NULL) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }*/

  /*if( ClearCommError(serial.descriptor(),&sds.Errors,&sds.ComStat) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }*/

// Handshaking ?
  *this << uint8_t(0x05);
  *this >> err;
//  assert( err == 0x06 );
  *this << uint8_t(0x55);
  *this >> err;
//  assert( err == 0x07 );
  *this << uint8_t(0x54);
  *this >> err;
//  assert( err == 0x06 );
// Command on clear terminal ?
  *this << uint8_t(0x02) << uint8_t(0x43) << uint8_t(0x45) << uint8_t(0x31) << uint8_t(0x03);
  *this >> err;
//  assert( err == 0x06 );
// May be end of exchage code ?
  *this << uint8_t(0x1b);

#if defined(__WIN32__) || defined(__WIN64__)
  if( EscapeCommFunction(serial.descriptor(),CLRDTR) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
/*  if( DeviceIoControl(serial.descriptor(),IOCTL_SERIAL_CLR_DTR,NULL,0,NULL,0,&bytesReturned,NULL) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }*/
  if( PurgeComm(serial.descriptor(),PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    serial.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#endif
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
void KFTPClient::main()
{
  try {
//    ZebexPDL pdl;
//    pdl.fiberExecute();
  
    if( shell_->config_->section(section_).isValue("log_file") ){
      logFile_.codePage(shell_->config_->section(section_).value("log_file_codepage",utf8::getCodePage(CP_ACP)));
      logFile_.fileName(shell_->config_->section(section_).text("log_file"));
      log_ = &logFile_;
    }
    remoteAddress_.resolve(host_,MSFTPDefaultPort);
    connect(remoteAddress_);

    auth();
    put();
    get();
    *this << int8_t(cmQuit);
    getCode();
  }
  catch( ExceptionSP & e ){
    log_->debug(8,utf8::String::Stream() << section_ << " " << host_ << " incomplete.\n");
    if( shell_->errorCode_ == 0 ) shell_->errorCode_ = e->code();
    log_->close();
    throw;
  }
  catch( ... ){
    if( shell_->errorCode_ == 0 ) shell_->errorCode_ = -1;
    log_->close();
    throw;
  }
  log_->close();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
KFTPShell::~KFTPShell()
{
  close();
}
//------------------------------------------------------------------------------
KFTPShell::KFTPShell(int & errorCode) :
  config_(
    newObject<InterlockedConfig<FiberInterlockedMutex> >()
  ),
  errorCode_(errorCode)
{
}
//------------------------------------------------------------------------------
void KFTPShell::open()
{
  intptr_t i;
  config_->parse().override();
  stdErr.rotationThreshold(
    config_->value("debug_file_rotate_threshold",1024 * 1024)
  );
  stdErr.rotatedFileCount(
    config_->value("debug_file_rotate_count",10)
  );
  stdErr.setDebugLevels(
    config_->value("debug_levels","+0,+1,+2,+3")
  );
  stdErr.fileName(
    config_->value("log_file",stdErr.fileName())
  );
  for( i = config_->sectionCount() - 1; i >= 0; i-- ){
    utf8::String sectionName(config_->section(i).name());
    if( sectionName.strncasecmp("job",3) == 0 )
      attachFiber(newObjectR1C2<KFTPClient>(*this,sectionName));
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
int main(int _argc,char * _argv[])
{
  struct rv a;
  a = dummy(
    (void *) 0x1, (void *) 0x2, (void *) 0x3, (void *) 0x4,
    (void *) 0x5, (void *) 0x6, (void *) 0x7, (void *) 0x8,
    (void *) 0x9,(void *) 0x10,(void *) 0x11,(void *) 0x12
  );
  
  int errcode = 0;
  adicpp::AutoInitializer autoInitializer(_argc,_argv);
  autoInitializer = autoInitializer;

  try {
    union {
      intptr_t i;
      uintptr_t u;
    };
    stdErr.fileName(SYSLOG_DIR("msftp/") + "msftp.conf");
    Config::defaultFileName(SYSCONF_DIR("") + "msftp.conf");
#ifndef NDEBUG
    fprintf(stderr,"%s\n",(const char *) getCurrentDir().getOEMString());
#endif
    bool dispatch = true;
    for( u = 1; u < argv().count(); u++ ){
      if( argv()[u].strcmp("--version") == 0 ){
        stdErr.debug(9,utf8::String::Stream() << msftp_version.tex_ << "\n");
        fprintf(stdout,"%s\n",msftp_version.tex_);
        dispatch = false;
        continue;
      }
      if( argv()[u].strcmp("-c") == 0 && u + 1 < argv().count() ){
        Config::defaultFileName(argv()[u + 1]);
      }
      else if( argv()[u].strcmp("--log") == 0 && u + 1 < argv().count() ){
        stdErr.fileName(argv()[u + 1]);
      }
      else if( argv()[u].strcmp("--sha256") == 0 && u + 1 < argv().count() ){
        SHA256 passwordSHA256;
        passwordSHA256.make(argv()[u + 1].c_str(),argv()[u + 1].size());
        utf8::String b64(base64Encode(passwordSHA256.sha256(),32));
        fprintf(stdout,"%s\n",b64.c_str());
        copyStrToClipboard(b64);
        dispatch = false;
      }
    }
    if( dispatch ){
      KFTPShell shell(errcode);
      shell.open();
    }
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    errcode = e->code();
  }
  return errcode;
}
//------------------------------------------------------------------------------
