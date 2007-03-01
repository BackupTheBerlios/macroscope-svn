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
#ifndef _xtime_H_
#define _xtime_H_
//---------------------------------------------------------------------------
#if SIZEOF_TIMEVAL == 0 && !HAVE_WINDOWS_H
struct timeval {
  long tv_sec;         /* seconds since Jan. 1, 1970 */
  long tv_usec;        /* and microseconds */
};
#endif
//---------------------------------------------------------------------------
#if SIZEOF_TIMEZONE == 0/* && !HAVE_WINDOWS_H*/
struct timezone {
  int tz_minuteswest; /* minutes west of Greenwich */
  int tz_dsttime;     /* type of dst correction */
};
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
struct timeval32 {
  int32_t tv_sec;
  int32_t tv_usec;
  
  timeval32 & operator = (const struct timeval & tv);
  operator const struct timeval () const;
};
//---------------------------------------------------------------------------
inline timeval32 & timeval32::operator = (const struct timeval & tv)
{
  tv_sec = int32_t(tv.tv_sec);
  tv_usec = int32_t(tv.tv_usec);
  return *this;
}
//---------------------------------------------------------------------------
inline timeval32::operator const struct timeval () const
{
  struct timeval tv;
  tv.tv_sec = (long) tv_sec;
  tv.tv_usec = (long) tv_usec;
  return tv;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
struct timeval64 {
  int64_t tv_sec;
  int64_t tv_usec;
  
  timeval64 & operator = (const struct timeval & tv);
  operator const struct timeval () const;
};
//---------------------------------------------------------------------------
inline timeval64 & timeval64::operator = (const struct timeval & tv)
{
  tv_sec = int64_t(tv.tv_sec);
  tv_usec = int64_t(tv.tv_usec);
  return *this;
}
//---------------------------------------------------------------------------
inline timeval64::operator const struct timeval () const
{
  struct timeval tv;
  tv.tv_sec = (long) tv_sec;
  tv.tv_usec = (long) tv_usec;
  return tv;
}
//---------------------------------------------------------------------------
#if !HAVE_GETTIMEOFDAY
int gettimeofday(struct timeval * tp,struct timezone * tzp);
#endif
//---------------------------------------------------------------------------
int64_t gettimeofday();
int64_t getlocaltimeofday();
int64_t getgmtoffset();
//---------------------------------------------------------------------------
#if !HAVE_TIMEGM
inline time_t timegm(struct tm * t)
{
  return mktime(t) - getgmtoffset() / 1000000u;
}
#endif
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
intptr_t str2Month(const char * month);
//---------------------------------------------------------------------------
inline int64_t timeval2Time(const struct timeval & tv)
{
  return tv.tv_sec * UINT64_C(1000000) + tv.tv_usec;
}
//---------------------------------------------------------------------------
inline struct timeval time2Timeval(int64_t a)
{
  struct timeval tv;
  tv.tv_sec = long(a / 1000000);
  tv.tv_usec = long(a % 1000000);
  return tv;
}
//---------------------------------------------------------------------------
inline int64_t tm2Time(struct tm t)
{
  return timegm(&t) * UINT64_C(1000000);
}
//---------------------------------------------------------------------------
inline struct timeval tm2Timeval(struct tm t)
{
  struct timeval tv;
  tv.tv_sec = (long) timegm(&t);
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
#endif /* _Sysutils_H_ */
//---------------------------------------------------------------------------
