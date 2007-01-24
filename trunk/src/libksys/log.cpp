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
#ifdef __BCPLUSPLUS__
using namespace std;
#endif
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
char stdErr_[sizeof(LogFile) / sizeof(char)];
LogFile & stdErr = *reinterpret_cast<LogFile *>(stdErr_);
//---------------------------------------------------------------------------
void LogFile::initialize()
{
  new (&stdErr) LogFile;
  stdErr.fileName(changeFileExt(getExecutableName(),".log"));
}
//---------------------------------------------------------------------------
void LogFile::cleanup()
{
  bool stackBackTrace0 = stackBackTrace;
  stackBackTrace = false;
  stdErr.~LogFile();
  stackBackTrace = stackBackTrace0;
}
//---------------------------------------------------------------------------
LogFile::~LogFile()
{
  close();
}
//---------------------------------------------------------------------------
LogFile::LogFile() : 
  bufferPos_(0),
  bufferSize_(0),
  bufferDataTTA_(60 * 1000000u), // 60 seconds
  lastFlushTime_(gettimeofday()),
  rotationThreshold_(1024 * 1024),
  rotatedFileCount_(10),
  codePage_(CP_OEMCP)
{
//  enabledLevels_(1 + 2 + 4 + 8 + 16 + 32 + 64 + 128 + 256 + 512),
  memset(enabledLevels_,0,sizeof(enabledLevels_));
  debugLevel(0,1);
  debugLevel(1,1);
  debugLevel(2,1);
  debugLevel(3,1);
  debugLevel(4,1);
  debugLevel(5,1);
  debugLevel(6,1);
  debugLevel(7,1);
  debugLevel(8,1);
  debugLevel(9,1);
  debugLevel(128,1);
}
//---------------------------------------------------------------------------
LogFile & LogFile::open()
{
  return *this;
}
//---------------------------------------------------------------------------
LogFile & LogFile::close()
{
  wait();
  threadFile_.close();
  return *this;
}
//---------------------------------------------------------------------------
void LogFile::threadBeforeWait()
{
//  for(;;){
    terminate();
    bufferSemaphore_.post().post();
//    fprintf(stderr,"%s %d handle_ = %p, started_ = %u, finished_ = %u\n",__FILE__,__LINE__,handle_,started_,finished_);
//    if( !active() ) break;
//    ksleep1();
//  }
}
//---------------------------------------------------------------------------
LogFile & LogFile::fileName(const utf8::String & name)
{
  file_ = name;
  close();
  return *this;
}
//---------------------------------------------------------------------------
LogFile & LogFile::internalLog(uintptr_t level,const utf8::String::Stream & stream)
{
  if( debugLevel(level) ){
    struct timeval tv = time2Timeval(getlocaltimeofday());
    struct tm t = time2tm(timeval2Time(tv));
    AutoPtr<char> buf;
    bool post = false;
    try {
      char buf[128];
      union {
        char buf2[128];
        wchar_t buf2w[128];
      };
      intptr_t a, l;
#if HAVE_SNPRINTF
#define SNPRINTF snprintf
#elif HAVE__SNPRINTF
#define SNPRINTF _snprintf
#endif
      if( currentFiber() != NULL )
        a = SNPRINTF(
          buf,
          sizeof(buf),
          "%02u.%02u.%04u %02u:%02u:%02u.%06ld (%u.%p,%u): ",
          t.tm_mday,
          t.tm_mon + 1,
          t.tm_year + 1900,
          t.tm_hour,
          t.tm_min,
          t.tm_sec,
          tv.tv_usec,
          getpid(),
          currentFiber(),
          (unsigned int) level
        );
      else
        a = SNPRINTF(
          buf,
          sizeof(buf),
          "%02u.%02u.%04u %02u:%02u:%02u.%06ld (%u,%u): ",
          t.tm_mday,
          t.tm_mon + 1,
          t.tm_year + 1900,
          t.tm_hour,
          t.tm_min,
          t.tm_sec,
          tv.tv_usec,
          getpid(),
          (unsigned int) level
        );
#undef SNPRINTF
      if( a == -1 ){
        int32_t err = errno;
        newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
      utf8::utf8s2mbcs(codePage_,buf2,sizeof(buf2),buf,sizeof(buf));
      l = utf8::utf8s2mbcs(codePage_,NULL,0,stream.plane(),stream.count());
      if( a < 0 || l < 0 ){
        int32_t err = errno;
        newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
      AutoLock<FiberInterlockedMutex> lock(mutex_);
      resume();
      AutoLock<InterlockedMutex> lock2(threadMutex_);
      uintptr_t bufferSize = bufferSize_;
      while( bufferSize < bufferPos_ + a + l ) bufferSize = (bufferSize << 1) + (bufferSize == 0);
      buffer_.realloc(bufferSize);
      bufferSize_ = bufferSize;
      memcpy(buffer_.ptr() + bufferPos_,buf2,a);
      utf8::utf8s2mbcs(codePage_,buffer_.ptr() + bufferPos_ + a,l,stream.plane(),stream.count());
      bufferPos_ += a + l;
      post = bufferPos_ - a - l == 0 || bufferPos_ >= getpagesize() * 16u || gettimeofday() - lastFlushTime_ >= bufferDataTTA_;
    }
    catch( ... ){
    }
    if( post ) bufferSemaphore_.post();
  }
  return *this;
}
//---------------------------------------------------------------------------
void LogFile::rotate(AsyncFile & file)
{
  if( rotatedFileCount_ == 0 || file.std() || file.size() <= rotationThreshold_ ) return;
  file.close();
  file.exclusive(true).open();
  utf8::String fileExt(getFileExt(file.fileName()));
  uintptr_t i = rotatedFileCount_;
  while( i >= 1 ){
    if( i == rotatedFileCount_ ){
      remove(changeFileExt(file.fileName(),"." + utf8::int2Str(i - 1)) + fileExt,true);
    }
    else {
      rename(
        changeFileExt(file.fileName(),"." + utf8::int2Str(i - 1)) + fileExt,
        changeFileExt(file.fileName(),"." + utf8::int2Str(i)) + fileExt,
	true,
	true
      );
    }
    i--;
  }
  file.close();
  rename(file.fileName(),changeFileExt(file.fileName(),".0") + fileExt,true,true);
}
//---------------------------------------------------------------------------
void LogFile::threadExecute()
{
//  fprintf(stderr,"%s %d\n",__FILE__,__LINE__);
  priority(THREAD_PRIORITY_LOWEST);
  threadFile_.fileName(file_).createIfNotExist(true);
  AsyncFile lck;
  lck.fileName(threadFile_.fileName() + ".lck").createIfNotExist(true).removeAfterClose(true);
  bool exception = false;
  for(;;){
    AutoPtr<char> buffer;
    uintptr_t bufferPos;
    {
      AutoLock<InterlockedMutex> lock(threadMutex_);
      buffer.xchg(buffer_);
      bufferPos = bufferPos_;
      bufferPos_ = 0;
      bufferSize_ = 0;
    }
    if( bufferPos == 0 && !terminated_ ){
      bufferSemaphore_.wait();
      continue;
    }
    if( terminated_ && (bufferPos == 0 || exception) ) break;
    if( bufferPos > 0 ){
//      fprintf(stderr,"%s %d bufferPos = %"PRIuPTR", terminated_ = %d, exception = %d\n",__FILE__,__LINE__,bufferPos,terminated_,exception);
      exception = false;
      AutoFileWRLock<AsyncFile> lock;
      try {
        lck.open();
        lck.wrLock(0,0);
        lock.setLocked(lck);
        threadFile_.exclusive(false).open();
        threadFile_.seek(threadFile_.size()).writeBuffer(buffer,bufferPos);
        {
          AutoLock<InterlockedMutex> lock(threadMutex_);
          lastFlushTime_ = gettimeofday();
	}
        rotate(threadFile_);
      }
      catch( ExceptionSP & e ){
        exception = true;
        try {
          utf8::OemString os(getNameFromPathName(getExecutableName()).getOEMString());
          utf8::OemString er((getBackTrace(1) + e->stdError()).getOEMString());
#if HAVE_SYSLOG_H
          openlog(os,LOG_PID,LOG_DAEMON);
          syslog(LOG_ERR,er);
          closelog();
#endif
          fprintf(stderr,"%s: %s",(const char * ) os,(const char * ) er);
	}
        catch( ... ){
	}
      }
    }
    if( exception ){
      threadFile_.close();
      lck.close();
    }
  }
//  fprintf(stderr,"%s %d\n",__FILE__,__LINE__);
}
//---------------------------------------------------------------------------
LogFile & LogFile::setDebugLevels(const utf8::String & levels)
{
  utf8::String minus("-");
  for( intptr_t i = enumStringParts(levels) - 1; i >= 0; i-- ){
    utf8::String s(stringPartByNo(levels,i));
    Mutant level(s);
    try {
      level.changeType(mtInt);
    }
    catch( ExceptionSP & e ){
      if( dynamic_cast<utf8::EStr2Scalar *>(e.ptr()) == NULL ) throw;
    }
    if( s.strstr(minus).eof() ){
      debugLevel(level,1);
    }
    else {
      debugLevel(-(intptr_t) level,0);
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
LogFile & LogFile::setAllDebugLevels(intptr_t value)
{
  for( intptr_t i = sizeof(enabledLevels_) * 8 - 1; i >= 0; i-- )
    debugLevel(i,value);
  return *this;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
