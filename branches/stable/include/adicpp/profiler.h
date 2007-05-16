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
#ifndef _Profiler_H_
#define _Profiler_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
#if defined(__GNUG__) && defined(__i386__)
inline int64_t rdtsc()
{
  union {
      struct {
          long  low;
          long  high;
      } b;
      int64_t a;
  } a;
  asm volatile ("rdtsc\n" : "=a" (a.b.low), "=d" (a.b.high));
  return a.a;
}
#elif _MSC_VER
int64_t rdtsc();
#elif defined(__BCPLUSPLUS__)
int64_t __fastcall rdtsc();
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#ifdef NETMAIL_ENABLE_PROFILER
class TProfiler : public TInterlockedMutex {
    friend void initialize();
    friend void cleanup();
  private:
    static void initialize();
    static void cleanup();

    static int64_t              ProcessTSC;
    static char                 StubMutex[];
    static TInterlockedMutex &  Mutex;
    static TProfiler *          FirstProfiler_;
    static bool                 Enabled_;

    TProfiler *                 NextProfiler_;
    const char *                Func_;
    const char *                PrettyFunc_;
    const char *                FuncFile_;
    long                        FuncLine_;
    int64_t                     TSC_;
    int64_t                     Count_;

    static void QSortProfilers(TProfiler ** Ptr, long lb, long ub);
  public:
    friend class Auto;
    class Auto {
      private:
        TProfiler * Profiler_;
        Auto *      LastProfiler_;
        int64_t     TSCDelta_;
        int64_t     TSC_;
      public:
        Auto(TProfiler * Profiler) GNUG_NOTHROW;
        ~Auto() GNUG_NOTHROW;
    };
  private:
    static char                           StubLastProfiler[];
    static TThreadLocalVariable< Auto> &  LastProfiler;
  public:
          TProfiler(const char * Func, const char * PrettyFunc, const char * FuncFile, long FuncLine) GNUG_NOTHROW;
          ~TProfiler() GNUG_NOTHROW;

    void  Constructor(const char * Func, const char * PrettyFunc, const char * FuncFile, long FuncLine, bool & Flag) GNUG_NOTHROW;
};
//---------------------------------------------------------------------------
#ifndef NETMAIL_PROFILER_CPU_TPS
#define CPU_TPS (int64_t) 1835726.6002333333333333333333333
// default value for my AMD Athlon XP 2500+
#endif
#define NETMAIL_DEFINE_PROFILER \
static char __netmail_profiler__[sizeof(TProfiler) / sizeof(char)];\
static bool __netmail_profiler_flag__ = false;\
reinterpret_cast<TProfiler *>(__netmail_profiler__)->Constructor(__FUNCTION__,__PRETTY_FUNCTION__,__FILE__,__LINE__,__netmail_profiler_flag__);\
TProfiler::Auto __netmail_profiler_tsc__(reinterpret_cast<TProfiler *>(__netmail_profiler__));
//---------------------------------------------------------------------------
#else
#define NETMAIL_DEFINE_PROFILER
#endif
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
#endif /* _Profiler_H_ */
