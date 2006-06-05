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
namespace ksys {
//---------------------------------------------------------------------------
extern const char __monthDays[2][12]  = {
  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }, { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};
//---------------------------------------------------------------------------
extern size_t sizeOf_timeval_tv_sec = sizeof(((struct timeval *) NULL)->tv_sec);
//---------------------------------------------------------------------------
utf8::String getTimeString(int64_t t)
{
  struct timeval tv = time2Timeval(t);
  struct tm tma = time2tm(timeval2Time(tv));
  utf8::String s;
  s.resize(3 + 3 + 5 + 3 + 3 + 3 + 7);
#if HAVE_SNPRINTF
  int a = snprintf(
#elif HAVE__SNPRINTF
  int a = _snprintf(
#endif
    s.c_str(),
    3 + 3 + 5 + 3 + 3 + 3 + 7,
    "%02u.%02u.%04u %02u:%02u:%02u.%06u",
    tma.tm_mday,
    tma.tm_mon + 1,
    tma.tm_year + 1900,
    tma.tm_hour,
    tma.tm_min,
    tma.tm_sec,
    (unsigned int) tv.tv_usec
  );
  if( a == -1 ){
    int32_t err = errno;
    throw ksys::ExceptionSP(new ksys::Exception(err,__PRETTY_FUNCTION__));
  }
  return s;
}
//---------------------------------------------------------------------------
utf8::String getTimeCode(int64_t t)
{
  struct timeval tv = time2Timeval(t);
  struct tm tma = time2tm(timeval2Time(tv));
  utf8::String s;
  s.resize(4 + 2 + 2 + 2 + 2 + 2 + 7);
#if HAVE_SNPRINTF
  int a = snprintf(
#elif HAVE__SNPRINTF
  int a = _snprintf(
#endif
    s.c_str(),
    4 + 2 + 2 + 2 + 2 + 2 + 7,
    "%04u%02u%02u%02u%02u%02u%06u",
    tma.tm_year + 1900,
    tma.tm_mon + 1,
    tma.tm_mday,
    tma.tm_hour,
    tma.tm_min,
    tma.tm_sec,
    (unsigned int) tv.tv_usec
  );
  if( a == -1 ){
    int32_t err = errno;
    throw ksys::ExceptionSP(new ksys::Exception(err,__PRETTY_FUNCTION__));
  }
  return s;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#if !HAVE_GETTIMEOFDAY && (defined(__WIN32__) || defined(__WIN64__))
int gettimeofday(struct timeval * tvp, struct timezone * tzp)
{
  union {
    FILETIME st;
    ULARGE_INTEGER sti;
  };
/*  union {
    FILETIME st2;
    ULARGE_INTEGER st2i;
  };*/
  GetSystemTimeAsFileTime(&st);
//  FileTimeToLocalFileTime(&st,&st2);

  /*time_t tt, tt2;
  tt2 = time(&tt);
  ULARGE_INTEGER v;
  v.QuadPart = sti.QuadPart / 10000000u - tt;*/

  sti.QuadPart -= UINT64_C(11644473600) * 10000000u; // January 1, 1970 (UTC) - January 1, 1601 (UTC)
//  st2i.QuadPart -= UINT64_C(11644473600) * 10000000u; // January 1, 1970 (UTC) - January 1, 1601 (UTC)
  if( ksys::sizeOf_timeval_tv_sec < 8 ){
    *(uint32_t *) &tvp->tv_sec = (uint32_t) (sti.QuadPart / 10000000u);
  }
  else {
    *(uint64_t *) &tvp->tv_sec = (uint64_t) (sti.QuadPart / 10000000u);
  }
  tvp->tv_usec = long((sti.QuadPart % 10000000u) / 10u);
  if( tzp != NULL ){
    struct _timeb tb;
    _ftime(&tb);
    tzp->tz_minuteswest = tb.timezone;
    tzp->tz_dsttime = tb.dstflag;
//    tzp->tz_minuteswest = -(int) (((st2i.QuadPart - sti.QuadPart) / 10000000u) / 60u);
//    tzp->tz_dsttime = 0;
  }
  return 0;
}
#endif
//---------------------------------------------------------------------------

