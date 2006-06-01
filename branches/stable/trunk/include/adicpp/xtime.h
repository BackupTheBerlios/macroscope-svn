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
#ifndef _xtime_H_
#define _xtime_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
inline int64_t timeval2Time(const struct timeval & tv)
{
  return tv.tv_sec * UINT64_C(1000000) + tv.tv_usec;
}
//---------------------------------------------------------------------------
inline struct timeval time2Timeval(int64_t a)
{
  struct timeval  tv;
  tv.tv_sec = long(a / 1000000);
  tv.tv_usec = long(a % 1000000);
  return tv;
}
//---------------------------------------------------------------------------
inline int64_t tm2Time(struct tm t)
{
  return mktime(&t) * UINT64_C(1000000);
}
//---------------------------------------------------------------------------
inline struct timeval tm2Timeval(struct tm t)
{
  struct timeval tv;
  tv.tv_sec = (long) mktime(&t);
  tv.tv_usec = 0;
  return tv;
}
//---------------------------------------------------------------------------
inline struct tm time2tm(int64_t a)
{
  time_t t = (time_t) (a / 1000000u);
  struct tm ta;
#if HAVE_GMTIME_S
  gmtime_s(&ta,&t);
#else
  ta = *gmtime(&t);
#endif
  return ta;
}
//---------------------------------------------------------------------------
inline struct tm timeval2tm(const struct timeval & a)
{
  time_t t = (time_t) a.tv_sec;
  struct tm ta;
#if HAVE_GMTIME_S
  gmtime_s(&ta,&t);
#else
  ta = *gmtime(&t);
#endif
  return ta;
}
//---------------------------------------------------------------------------
inline bool isLeapYear(uintptr_t year)
{
  return (year % 4u == 0) && ((year % 100u != 0) || (year % 400u == 0));
}
//---------------------------------------------------------------------------
inline uintptr_t yearDays(uintptr_t year)
{
  return isLeapYear(year) + 365;
}
//---------------------------------------------------------------------------
extern const char __monthDays[2][12];
//---------------------------------------------------------------------------
inline uintptr_t monthDays(uintptr_t year, uintptr_t mon)
{
  return __monthDays[isLeapYear(year)][mon];
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#if SIZEOF_STRUCT_TIMEVAL == 0 && !HAVE_WINDOWS_H
struct timeval {
    long  tv_sec;         /* seconds since Jan. 1, 1970 */
    long  tv_usec;        /* and microseconds */
};
#endif
//---------------------------------------------------------------------------
#if SIZEOF_STRUCT_TIMEZONE == 0/* && !HAVE_WINDOWS_H*/
struct timezone {
    int tz_minuteswest; /* minutes west of Greenwich */
    int tz_dsttime;     /* type of dst correction */
};
#endif
//---------------------------------------------------------------------------
#if !HAVE_GETTIMEOFDAY
int gettimeofday(struct timeval * tp, struct timezone * tzp);
#endif
//---------------------------------------------------------------------------
inline int64_t gettimeofday()
{
  struct timeval t;
  uint64_t a = ~(~uint64_t(0) >> 1);
  if( gettimeofday(&t, NULL) == 0 ){
    a = t.tv_sec;
    a = a * 1000000u + t.tv_usec;
  }
  return a;
}
//---------------------------------------------------------------------------
inline int64_t getlocaltimeofday()
{
  struct timeval tv;
  struct timezone tz;
  uint64_t a = ~(~uint64_t(0) >> 1);
  if( gettimeofday(&tv,&tz) == 0 ){
    a = tv.tv_sec - tz.tz_minuteswest * 60u + tz.tz_dsttime * 60u * 60u;
    a = a * 1000000u + tv.tv_usec;
  }
  return a;
}
//---------------------------------------------------------------------------
#endif /* _Sysutils_H_ */
//---------------------------------------------------------------------------
