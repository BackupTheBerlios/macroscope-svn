/*-
 * Copyright 2005-2007 Guram Dukashvili
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
  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};
//---------------------------------------------------------------------------
intptr_t str2Month(const char * month)
{
  if( strncmp(month, "Jan", 3) == 0 ) return 0;
  if( strncmp(month, "Feb", 3) == 0 ) return 1;
  if( strncmp(month, "Mar", 3) == 0 ) return 2;
  if( strncmp(month, "Apr", 3) == 0 ) return 3;
  if( strncmp(month, "Mai", 3) == 0 ) return 4;
  if( strncmp(month, "May", 3) == 0 ) return 4;
  if( strncmp(month, "Jun", 3) == 0 ) return 5;
  if( strncmp(month, "Jul", 3) == 0 ) return 6;
  if( strncmp(month, "Aug", 3) == 0 ) return 7;
  if( strncmp(month, "Sep", 3) == 0 ) return 8;
  if( strncmp(month, "Okt", 3) == 0 ) return 9;
  if( strncmp(month, "Oct", 3) == 0 ) return 9;
  if( strncmp(month, "Nov", 3) == 0 ) return 10;
  if( strncmp(month, "Dez", 3) == 0 ) return 11;
  if( strncmp(month, "Dec", 3) == 0 ) return 11;
  return -1;
}
//------------------------------------------------------------------------------
size_t sizeOf_timeval_tv_sec = sizeof(((struct timeval *) NULL)->tv_sec);
//---------------------------------------------------------------------------
int64_t timeFromTimeString(const utf8::String & s,bool local)
{
  struct timeval tv;
  struct tm tma;
  unsigned int usec = 0;
  memset(&tv,0,sizeof(tv));
  memset(&tma,0,sizeof(tma));
#if HAVE_SSCANF
  int a = sscanf(s.c_str(),
#elif HAVE__SSCANF
  int a = _sscanf(s.c_str(),
#elif HAVE_SNSCANF
  int a = snscanf(s.c_str(),s.size() + 1,
#elif HAVE__SNSCANF
  int a = _snscanf(s.c_str(),s.size() + 1,
#endif
    "%02u.%02u.%04u %02u:%02u:%02u.%06u",
    (unsigned int *) &tma.tm_mday,
    (unsigned int *) &tma.tm_mon,
    (unsigned int *) &tma.tm_year,
    (unsigned int *) &tma.tm_hour,
    (unsigned int *) &tma.tm_min,
    (unsigned int *) &tma.tm_sec,
    &usec
  );
  if( a == -1 ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( tma.tm_year == 0 && tma.tm_mon == 0 && tma.tm_mday == 0 &&
      tma.tm_hour == 0 && tma.tm_min == 0 && tma.tm_sec == 0 && usec == 0 ){
    return 0;
  }
  tma.tm_mon -= 1;
  tma.tm_year -= 1900;
  if( ksys::sizeOf_timeval_tv_sec == 4 ){
    *(int32_t *) &tv.tv_sec = (int32_t) mktime(&tma);
  }
  if( ksys::sizeOf_timeval_tv_sec == 8 ){
    *(int64_t *) &tv.tv_sec = (int64_t) mktime(&tma);
  }
  if( tv.tv_sec < 0 ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( local ){
    struct timeval tv2;
    struct timezone tz;
    gettimeofday(&tv2,&tz);
    tv.tv_sec -= tz.tz_minuteswest * 60u/* + tz.tz_dsttime * 60u * 60u*/;
  }
  tv.tv_usec = usec;
  return timeval2Time(tv);
}
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
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return s;
}
//---------------------------------------------------------------------------
int64_t timeFromTimeCodeString(const utf8::String & s,bool local)
{
  struct timeval tv;
  struct tm tma;
  unsigned int usec = 0;
  memset(&tv,0,sizeof(tv));
  memset(&tma,0,sizeof(tma));
#if HAVE_SSCANF
  int a = sscanf(s.c_str(),
#elif HAVE__SSCANF
  int a = _sscanf(s.c_str(),
#elif HAVE_SNSCANF
  int a = snscanf(s.c_str(),s.size() + 1,
#elif HAVE__SNSCANF
  int a = _snscanf(s.c_str(),s.size() + 1,
#endif
    "%04u%02u%02u%02u%02u%02u%06u",
    (unsigned int *) &tma.tm_year,
    (unsigned int *) &tma.tm_mon,
    (unsigned int *) &tma.tm_mday,
    (unsigned int *) &tma.tm_hour,
    (unsigned int *) &tma.tm_min,
    (unsigned int *) &tma.tm_sec,
    &usec
  );
  if( a == -1 ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  tma.tm_mon -= 1;
  tma.tm_year -= 1900;
  if( ksys::sizeOf_timeval_tv_sec == 4 ){
    *(int32_t *) &tv.tv_sec = (int32_t) mktime(&tma);
  }
  if( ksys::sizeOf_timeval_tv_sec == 8 ){
    *(int64_t *) &tv.tv_sec = (int64_t) mktime(&tma);
  }
  if( tv.tv_sec < 0 ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( local ){
    struct timeval tv2;
    struct timezone tz;
    gettimeofday(&tv2,&tz);
    tv.tv_sec -= tz.tz_minuteswest * 60u/* + tz.tz_dsttime * 60u * 60u*/;
  }
  tv.tv_usec = usec;
  return timeval2Time(tv);
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
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
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
  if( ksys::sizeOf_timeval_tv_sec == 4 ){
    *(uint32_t *) &tvp->tv_sec = (uint32_t) (sti.QuadPart / 10000000u);
  }
  if( ksys::sizeOf_timeval_tv_sec == 8 ){
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
int64_t gettimeofday()
{
  struct timeval t;
  uint64_t a = ~(~uint64_t(0) >> 1);
  if( gettimeofday(&t,NULL) == 0 ){
    a = t.tv_sec;
    a = a * 1000000u + t.tv_usec;
  }
  return a;
}
//---------------------------------------------------------------------------
int64_t getlocaltimeofday()
{
  struct timeval tv;
  struct timezone tz;
  uint64_t a = ~(~uint64_t(0) >> 1);
  gettimeofday(&tv,&tz);
  a = tv.tv_sec - tz.tz_minuteswest * int64_t(60) + tz.tz_dsttime * 60u * 60u;
  a = a * 1000000u + tv.tv_usec;
  return a;
}
//---------------------------------------------------------------------------
int64_t getgmtoffset()
{
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv,&tz);
  return (-tz.tz_minuteswest * int64_t(60) + tz.tz_dsttime * 60 * 60) * 1000000;
}
//---------------------------------------------------------------------------
