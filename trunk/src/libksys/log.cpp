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
}
//---------------------------------------------------------------------------
void LogFile::cleanup()
{
  stdErr.~LogFile();
}
//---------------------------------------------------------------------------
LogFile::~LogFile()
{
}
//---------------------------------------------------------------------------
LogFile::LogFile() : filter_(lmFATAL)
{
}
//---------------------------------------------------------------------------
LogFile::LogFile(const utf8::String & fileName) : filter_(lmFATAL)
{
  file_.fileName(fileName);
}
//---------------------------------------------------------------------------
LogFile & LogFile::open()
{
  AutoLock<InterlockedMutex> lock(mutex_);
  if( this == &stdErr )
    file_.fileName(
      changeFileExt(getExecutableName(),utf8::string("log"))
    );
  file_.open();
  return *this;
}
//---------------------------------------------------------------------------
LogFile & LogFile::close()
{
  file_.close();
  return *this;
}
//---------------------------------------------------------------------------
const char * const LogFile::priNicks_[] = {
  "INFO", "NOTIFY", "WARNING", "ERROR", "FATAL", "PROFILER", "DIRECT"
};
//---------------------------------------------------------------------------
LogFile & LogFile::log(LogMessagePriority pri,const char * fmt, ... )
{
  try {
    if( pri <= filter_ || pri > lmFATAL ){
      utf8::String s;
      struct timeval tv = time2Timeval(gettimeofday());
      struct tm t = time2tm(timeval2Time(tv));
      int a, b;
      va_list ap;
#if !defined(__BCPLUSPLUS__) && !defined(__FreeBSD__) && !defined(__linux__)
#if HAVE_SNPRINTF
      a = snprintf(
#elif HAVE__SNPRINTF
      a = _snprintf(
#endif
        s.c_str(),0,
        "%02u.%02u.%04u %02u:%02u:%02u.%06ld %s(%u): ",
        t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min, t.tm_sec, tv.tv_usec,
        priNicks_[pri],
        getpid()
      );
      va_start(ap,fmt);
#if HAVE_VSNPRINTF
      b = vsnprintf(
#elif HAVE__VSNPRINTF
      b = _vsnprintf(
#endif
        s.c_str(),0,fmt,ap);
      va_end(ap);
      if( a <= 0 || b <= 0 ){ // hack for ugly msvcrt.dll versions
#endif
        s.resize(65535);
        a = sprintf(s.c_str(),
          "%02u.%02u.%04u %02u:%02u:%02u.%06ld %s(%u): ",
          t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min, t.tm_sec, tv.tv_usec,
          priNicks_[pri],
          getpid()
        );
        va_start(ap,fmt);
        b = vsprintf(s.c_str() + a,fmt,ap);
        va_end(ap);
#if !defined(__BCPLUSPLUS__) && !defined(__FreeBSD__) && !defined(__linux__)
      }
      else {
        s.resize(a + b);
#if HAVE_SNPRINTF
        snprintf(
#elif HAVE__SNPRINTF
       _snprintf(
#endif
          s.c_str(),s.size(),
          "%02u.%02u.%04u %02u:%02u:%02u.%06ld %s(%u): ",
          t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min, t.tm_sec, tv.tv_usec,
          priNicks_[pri],
          getpid()
        );
        va_start(ap,fmt);
#if HAVE_VSNPRINTF
        vsnprintf(
#elif HAVE__VSNPRINTF
       _vsnprintf(
#endif
          s.c_str() + a,s.size(),fmt,ap);
        va_end(ap);
      }
#endif
      open();
      AutoLock<InterlockedMutex> lock(mutex_);
      int64_t sz = file_.size();
      file_.wrLock(sz,0);
      if( pri == lmDIRECT ){
        file_.writeBuffer(file_.size(),s.c_str() + a,b * sizeof(char));
      }
      else {
        file_.writeBuffer(file_.size(),s.c_str(),(a + b) * sizeof(char));
      }
      file_.unLock(sz,0);
    }
  }
  catch( ... ){}
  if( pri == lmFATAL ) exit(-1);
  return *this;
}
//---------------------------------------------------------------------------
LogFile & LogFile::sysLog(LogMessagePriority pri,const char * fmt, ... )
{
  try {
    if( pri <= filter_ || pri > lmFATAL ){
      utf8::String s;
      struct timeval tv = time2Timeval(gettimeofday());
      struct tm t = time2tm(timeval2Time(tv));
      int a, b;
      va_list ap;
#if !defined(__BCPLUSPLUS__) && !defined(__FreeBSD__) && !defined(__linux__)
#if HAVE_SNPRINTF
      a = snprintf(
#elif HAVE__SNPRINTF
      a = _snprintf(
#endif
        s.c_str(),0,
        "%02u.%02u.%04u %02u:%02u:%02u.%06ld %s(%u): ",
        t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min, t.tm_sec, tv.tv_usec,
        priNicks_[pri],
        getpid()
      );
      va_start(ap,fmt);
#if HAVE_VSNPRINTF
      b = vsnprintf(
#elif HAVE__VSNPRINTF
      b = _vsnprintf(
#endif
        s.c_str(),0,fmt,ap);
      va_end(ap);
      if( a <= 0 || b <= 0 ){ // hack for ugly msvcrt.dll versions
#endif
        s.resize(65535);
        a = sprintf(s.c_str(),
          "%02u.%02u.%04u %02u:%02u:%02u.%06ld %s(%u): ",
          t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min, t.tm_sec, tv.tv_usec,
          priNicks_[pri],
          getpid()
        );
        va_start(ap,fmt);
        b = vsprintf(s.c_str() + a,fmt,ap);
        va_end(ap);
#if !defined(__BCPLUSPLUS__) && !defined(__FreeBSD__) && !defined(__linux__)
      }
      else {
        s.resize(a + b);
#if HAVE_SNPRINTF
        snprintf(
#elif HAVE__SNPRINTF
       _snprintf(
#endif
          s.c_str(),s.strlen(),
          "%02u.%02u.%04u %02u:%02u:%02u.%06ld %s(%u): ",
          t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min, t.tm_sec, tv.tv_usec,
          priNicks_[pri],
          getpid()
        );
        va_start(ap,fmt);
#if HAVE_VSNPRINTF
        vsnprintf(
#elif HAVE__VSNPRINTF
       _vsnprintf(
#endif
         s.c_str() + a,s.strlen() - a,fmt,ap);
        va_end(ap);
      }
#endif
      open();
      AutoLock<InterlockedMutex> lock(mutex_);
      int64_t sz = file_.size();
      file_.wrLock(sz,0);
      if( pri == lmDIRECT ){
        file_.writeBuffer(file_.size(),s.c_str() + a,b * sizeof(char));
      }
      else {
        file_.writeBuffer(file_.size(),s.c_str(),(a + b) * sizeof(char));
      }
      file_.unLock(sz,0);
    }
  }
  catch( ... ){}
  if( pri == lmFATAL ) exit(-1);
  return *this;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
