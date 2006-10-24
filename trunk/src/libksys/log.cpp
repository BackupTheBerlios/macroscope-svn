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
  enabledLevels_(1 + 2 + 4 + 8 + 16 + 32 + 64 + 128 + 256 + 512),
  rotationThreshold_(1024 * 1024),
  rotatedFileCount_(10),
  codePage_(CP_ACP)
{
  file_.createIfNotExist(true);
  lockFile_.createIfNotExist(true).removeAfterClose(true);
}
//---------------------------------------------------------------------------
LogFile & LogFile::open()
{
  return *this;
}
//---------------------------------------------------------------------------
LogFile & LogFile::close()
{
  file_.close();
  lockFile_.close();
  return *this;
}
//---------------------------------------------------------------------------
LogFile & LogFile::fileName(const utf8::String & name)
{
  AutoLock<FiberInterlockedMutex> lock(mutex_);
  try {
    file_.fileName(name);
    lockFile_.fileName(name + ".lck");
  }
  catch( ... ){}
  return *this;
}
//---------------------------------------------------------------------------
const char * const  LogFile::priNicks_[] = {
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
  if( (rotatedFileCount_ > 0 && size <= rotationThreshold_) || file_.std() ) return;
  file_.close();
  utf8::String name(file_.fileName() + ".rename");
  rename(file_.fileName(),name);
  try {
    utf8::String fileExt(getFileExt(file_.fileName()));
    Stat st;
    intptr_t i = 0;
    while( stat(changeFileExt(file_.fileName(),"." + utf8::int2Str(i)) + fileExt,st) ) i++;
    while( i > 0 ){
      if( i >= (intptr_t) rotatedFileCount_ ){
        remove(changeFileExt(file_.fileName(),"." + utf8::int2Str(i - 1)) + fileExt);
      }
      else {
        rename(
          changeFileExt(file_.fileName(),"." + utf8::int2Str(i - 1)) + fileExt,
          changeFileExt(file_.fileName(),"." + utf8::int2Str(i)) + fileExt
        );
      }
      i--;
    }
    rename(name,changeFileExt(file_.fileName(),".0") + fileExt);
  }
  catch( ... ){
    rename(name,file_.fileName());
    throw;
  }
}
//---------------------------------------------------------------------------
LogFile & LogFile::internalLog(LogMessagePriority pri,uintptr_t level,const utf8::String::Stream & stream)
{
  assert( level <= 9 || level == ~uintptr_t(0) );
  if( level <= 9 && (enabledLevels_ & (uintptr_t(1) << level)) == 0 ) return *this;
  try {
    struct timeval tv = time2Timeval(getlocaltimeofday());
    struct tm t = time2tm(timeval2Time(tv));
    intptr_t a;

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
      Exception::throwSP(err,__PRETTY_FUNCTION__);
    }
    AutoPtr<char> buf;
    buf.alloc(a + 1);
    if( pri == lmDEBUG ){
      a = SNPRINTF(
        buf.ptr(),
        a + 1,
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
        a + 1,
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
      Exception::throwSP(err,__PRETTY_FUNCTION__);
    }
    union {
      char buf2[128];
      wchar_t buf2w[128];
    };
    a = utf8::utf8s2mbcs(codePage_,NULL,0,buf,sizeof(buf2));
    intptr_t l = utf8::utf8s2mbcs(codePage_,NULL,0,stream.plane(),stream.count());
    if( a < 0 || l < 0 ){
      int32_t err = errno;
      Exception::throwSP(err,__PRETTY_FUNCTION__);
    }
    utf8::utf8s2mbcs(codePage_,buf2,sizeof(buf2w),buf,sizeof(buf2));
    buf.realloc(a + l);
    memcpy(buf,buf2,a);
    utf8::utf8s2mbcs(codePage_,buf.ptr() + a,l,stream.plane(),stream.count());
    uint64_t sz = 0;
    AutoLock<FiberInterlockedMutex> lock(mutex_);
    if( file_.fileName().strlen() == 0 ){
      file_.fileName(changeFileExt(getExecutableName(),".log"));
      lockFile_.fileName(file_.fileName() + ".lck");
    }
    if( !file_.std() ){
      lockFile_.open();
      lockFile_.detach();
      lockFile_.attach();
    }
    AutoFileWRLock<AsyncFile> flock;
    if( !file_.std() ){
      if( !lockFile_.tryWRLock(0,0) ){
        file_.close();
        lockFile_.wrLock(0,0);
      }
      flock.setLocked(lockFile_);
    }
    file_.open();
    file_.detach();
    file_.attach();
    try {
      if( pri == lmDIRECT ){
        if( file_.std() )
          file_.writeBuffer(buf.ptr() + a,l * sizeof(char));
        else
          file_.writeBuffer(file_.size(),buf.ptr() + a,l * sizeof(char));
      }
      else {
        if( file_.std() )
          file_.writeBuffer(buf.ptr(),(a + l) * sizeof(char));
        else
          file_.writeBuffer(file_.size(),buf.ptr(),(a + l) * sizeof(char));
      }
      sz = file_.size();
    }
    catch( ... ){
      file_.close();
      throw;
    }
    file_.detach();
    rotate(sz);
    lockFile_.detach();
  }
/*  catch( ExceptionSP & e ){
    e->code();
  }*/
  catch( ... ){
  }
  return *this;
}
//---------------------------------------------------------------------------
LogFile & LogFile::setDebugLevels(const utf8::String & levels)
{
  for( intptr_t i = enumStringParts(levels) - 1; i >= 0; i-- ){
    Mutant level(stringPartByNo(levels,i));
    try {
      level.changeType(mtInt);
    }
    catch( ExceptionSP & e ){
      if( dynamic_cast<utf8::EStr2Scalar *>(e.ptr()) == NULL ) throw;
    }
    if( (intptr_t) level >= 0 )
      enableDebugLevel((intptr_t) level);
    else
      disableDebugLevel(-(intptr_t) level);
  }
  return *this;
}
//---------------------------------------------------------------------------
LogFile & LogFile::redirectToStdout()
{
#if defined(__WIN32__) || defined(__WIN64__)
  AutoLock<FiberInterlockedMutex> lock(mutex_);
  try {
    lockFile_.close();
    file_.close().redirectToStdout();
  }
  catch( ... ){}
#else
  throw ExceptionSP(newObject<Exception>(ENOSYS,__PRETTY_FUNCTION__));
#endif
  return *this;
}
//---------------------------------------------------------------------------
LogFile & LogFile::redirectToStderr()
{
#if defined(__WIN32__) || defined(__WIN64__)
  AutoLock<FiberInterlockedMutex> lock(mutex_);
  try {
    lockFile_.close();
    file_.close().redirectToStderr();
  }
  catch( ... ){}
#else
  throw ExceptionSP(newObject<Exception>(ENOSYS,__PRETTY_FUNCTION__));
#endif
  return *this;
}
//---------------------------------------------------------------------------
LogFile & LogFile::setRedirect(const utf8::String & redirect)
{
  if( redirect.strcasecmp("stdout") == 0 ) redirectToStdout();
  else
  if( redirect.strcasecmp("stderr") == 0 ) redirectToStderr();
  return *this;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
