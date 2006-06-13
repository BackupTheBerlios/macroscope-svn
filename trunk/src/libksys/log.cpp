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
char      stdErr_[sizeof(LogFile) / sizeof(char)];
LogFile & stdErr  = *reinterpret_cast< LogFile *>(stdErr_);
//---------------------------------------------------------------------------
void LogFile::initialize()
{
  new (&stdErr) LogFile;
}
//---------------------------------------------------------------------------
void LogFile::cleanup()
{
  stdErr.~LogFile();
}
//---------------------------------------------------------------------------
LogFile::~LogFile()
{
  close();
}
//---------------------------------------------------------------------------
LogFile::LogFile() : 
  enabledLevels_(1 + 2 + 4 + 8),
  rotationThreshold_(1024 * 1024),
  rotatedFileCount_(10)
{
  afile_ = new AsyncFile;
  fmutex_ = new FiberInterlockedMutex;
}
//---------------------------------------------------------------------------
LogFile & LogFile::open()
{
  return *this;
}
//---------------------------------------------------------------------------
LogFile & LogFile::close()
{
  afile_->close();
  file_.close();
  return *this;
}
//---------------------------------------------------------------------------
LogFile & LogFile::fileName(const utf8::String & name)
{
  {
    AutoLock<FiberInterlockedMutex> lock(fmutex_);
    afile_->close();
    afile_->fileName(name);
  }
  AutoLock<InterlockedMutex> lock(mutex_);
  file_.close();
  file_.fileName(name);
  return *this;
}
//---------------------------------------------------------------------------
const char * const  LogFile::priNicks_[]  = {
  "INFO",
  "DEBUG",
  "NOTIFY",
  "WARNING",
  "ERROR",
  "FATAL",
  "PROFILER",
  "DIRECT"
};
//---------------------------------------------------------------------------
void LogFile::rotate(uint64_t size)
{
  if( rotatedFileCount_ > 0 && size <= rotationThreshold_) return;
  if( isRunInFiber() ){
    AsyncFile flock(afile_->fileName() + ".lck");
    flock.removeAfterClose(true);
    flock.open();
    if( flock.tryWRLock(0,0) ){
      utf8::String fileExt(getFileExt(afile_->fileName()));
      Stat st;
      intptr_t i = 0, j;
      while( statAsync(changeFileExt(afile_->fileName(),utf8::int2Str(i)) + fileExt,st) ) i++;
      for( j = --i; j >= (intptr_t) rotatedFileCount_ - 1; j-- )
        removeAsync(changeFileExt(afile_->fileName(),utf8::int2Str(j)) + fileExt);
      while( i > 0 ){
        renameAsync(
          changeFileExt(afile_->fileName(),utf8::int2Str(i - 1)) + fileExt,
          changeFileExt(afile_->fileName(),utf8::int2Str(i)) + fileExt
        );
        i--;
      }
      afile_->close();
      for(;;){
        {
          AutoLock<InterlockedMutex> lock(mutex_);
          file_.close();
        }
        bool renamed = false;
        try {
          renameAsync(afile_->fileName(),changeFileExt(afile_->fileName(),"0") + fileExt);
          renamed = true;
        }
        catch( ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
          if( e->code() != ERROR_SHARING_VIOLATION + errorOffset &&
              e->code() != ERROR_LOCK_VIOLATION + errorOffset ) throw;
#else
          if( e->code() != EAGAIN && e->code() != EDEADLK ) throw;
#endif
        }
        if( renamed ) break;
      }
      flock.unLock(0,0);
    }
    else {
      afile_->close();
    }
  }
  else {
    FileHandleContainer flock(file_.fileName() + ".lck");
    flock.removeAfterClose(true);
    flock.open();
    if( flock.tryWRLock(0,0) ){
      utf8::String fileExt(getFileExt(file_.fileName()));
      Stat st;
      intptr_t i = 0, j;
      while( stat(changeFileExt(file_.fileName(),utf8::int2Str(i)) + fileExt,st) ) i++;
      for( j = --i; j >= (intptr_t) rotatedFileCount_ - 1; j-- )
        remove(changeFileExt(file_.fileName(),utf8::int2Str(j)) + fileExt);
      while( i > 0 ){
        rename(
          changeFileExt(file_.fileName(),utf8::int2Str(i - 1)) + fileExt,
          changeFileExt(file_.fileName(),utf8::int2Str(i)) + fileExt
        );
        i--;
      }
      file_.close();
      for(;;){
        {
          AutoLock<FiberInterlockedMutex> lock(fmutex_);
          afile_->close();
        }
        bool renamed = false;
        try {
          rename(file_.fileName(),changeFileExt(file_.fileName(),"0") + fileExt);
          renamed = true;
        }
        catch( ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
          if( e->code() != ERROR_SHARING_VIOLATION + errorOffset &&
              e->code() != ERROR_LOCK_VIOLATION + errorOffset ) throw;
#else
          if( e->code() != EAGAIN && e->code() != EDEADLK ) throw;
#endif
        }
        if( renamed ) break;
      }
      flock.unLock(0,0);
    }
    else {
      file_.close();
    }
  }
}
//---------------------------------------------------------------------------
LogFile & LogFile::internalLog(LogMessagePriority pri,uintptr_t level,const utf8::String::Stream & stream)
{
  assert( level <= 9 || level == ~uintptr_t(0) );
  if( level <= 9 && (enabledLevels_ & (1u << level)) == 0 ) return *this;

  struct timeval tv = time2Timeval(getlocaltimeofday());
  struct tm t = time2tm(timeval2Time(tv));
  int a;

#if HAVE_SNPRINTF
#define SNPRINTF snprintf
#elif HAVE__SNPRINTF
#define SNPRINTF _snprintf
#endif
  if( pri == lmDEBUG ){
    a = SNPRINTF(
      NULL,
      0,
      "%02u.%02u.%04u %02u:%02u:%02u.%06ld %s(%u,%u): ",
      t.tm_mday,
      t.tm_mon + 1,
      t.tm_year + 1900,
      t.tm_hour,
      t.tm_min,
      t.tm_sec,
      tv.tv_usec,
      priNicks_[pri],
      getpid(),
      (unsigned int) level
    );
  }
  else {
    a = SNPRINTF(
      NULL,
      0,
      "%02u.%02u.%04u %02u:%02u:%02u.%06ld %s(%u): ",
      t.tm_mday,
      t.tm_mon + 1,
      t.tm_year + 1900,
      t.tm_hour,
      t.tm_min,
      t.tm_sec,
      tv.tv_usec,
      priNicks_[pri],
      getpid()
    );
  }
  if( a == -1 ){
    int32_t err = errno;
    throw ksys::ExceptionSP(new ksys::Exception(err,__PRETTY_FUNCTION__));
  }
  AutoPtr<char> buf;
  buf.alloc(a);
  if( pri == lmDEBUG ){
    a = SNPRINTF(
      buf.ptr(),
      a,
      "%02u.%02u.%04u %02u:%02u:%02u.%06ld %s(%u,%u): ",
      t.tm_mday,
      t.tm_mon + 1,
      t.tm_year + 1900,
      t.tm_hour,
      t.tm_min,
      t.tm_sec,
      tv.tv_usec,
      priNicks_[pri],
      getpid(),
      (unsigned int) level
    );
  }
  else {
    a = SNPRINTF(
      buf.ptr(),
      a,
      "%02u.%02u.%04u %02u:%02u:%02u.%06ld %s(%u): ",
      t.tm_mday,
      t.tm_mon + 1,
      t.tm_year + 1900,
      t.tm_hour,
      t.tm_min,
      t.tm_sec,
      tv.tv_usec,
      priNicks_[pri],
      getpid()
    );
  }
#undef SNPRINTF
  if( a == -1 ){
    int32_t err = errno;
    throw ksys::ExceptionSP(new ksys::Exception(err,__PRETTY_FUNCTION__));
  }
  intptr_t l = utf8::utf8s2mbcs(CP_OEMCP,NULL,0,stream.plane(),stream.count());
  if( l < 0 ){
    int32_t err = errno;
    throw ksys::ExceptionSP(new ksys::Exception(err,__PRETTY_FUNCTION__));
  }
  buf.realloc(a + l);
  utf8::utf8s2mbcs(CP_OEMCP,buf.ptr() + a,l,stream.plane(),stream.count());
  if( isRunInFiber() ){
    AutoLock<FiberInterlockedMutex> lock(fmutex_);
    if( !afile_->isOpen() ){
      if( afile_->fileName().strlen() == 0 )
        afile_->fileName(changeFileExt(getExecutableName(),"log"));
      afile_->open();
    }
    else {
      afile_->detach();
      afile_->attach();
    }
    uint64_t sz = afile_->size();
    try {
      AutoFileWRLock<AsyncFile> flock(afile_,sz,0);
      if( pri == lmDIRECT ){
        afile_->writeBuffer(afile_->size(),buf.ptr() + a,l * sizeof(char));
      }
      else {
        afile_->writeBuffer(afile_->size(),buf.ptr(),(a + l) * sizeof(char));
      }
      sz = afile_->size();
    }
    catch( ... ){
      afile_->detach();
      afile_->close();
      throw;
    }
    rotate(sz);
    afile_->detach();
  }
  else {
    AutoLock<InterlockedMutex> lock(mutex_);
    if( !file_.isOpen() ){
      if( file_.fileName().strlen() == 0 )
        file_.fileName(changeFileExt(getExecutableName(),"log"));
      file_.open();
    }
    uint64_t sz = file_.size();
    {
      AutoFileWRLock<FileHandleContainer> flock(file_,sz,0);
      if( pri == lmDIRECT ){
        file_.writeBuffer(file_.size(),buf.ptr() + a,l * sizeof(char));
      }
      else {
        file_.writeBuffer(file_.size(),buf.ptr(),(a + l) * sizeof(char));
      }
      sz = file_.size();
    }
    rotate(sz);
  }
  return *this;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
