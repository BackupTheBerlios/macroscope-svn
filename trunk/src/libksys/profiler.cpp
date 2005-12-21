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
#ifdef __BCPLUSPLUS__
#pragma option push -w-8070
int64_t __fastcall rdtsc()
{
  __asm {
        rdtsc
  }
}
#pragma option pop
#endif
//---------------------------------------------------------------------------
#ifdef NETMAIL_ENABLE_PROFILER
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
longlong TProfiler::ProcessTSC;
char TProfiler::StubMutex[sizeof(TInterlockedMutex) / sizeof(char)];
TInterlockedMutex & TProfiler::Mutex = *reinterpret_cast<TInterlockedMutex *>(&StubMutex);
TProfiler * TProfiler::FirstProfiler_ = NULL;
bool TProfiler::Enabled_ = true;
char TProfiler::StubLastProfiler[sizeof(TThreadLocalVariable<TProfiler::Auto>) / sizeof(char)];
TThreadLocalVariable<TProfiler::Auto> & TProfiler::LastProfiler = *reinterpret_cast<TThreadLocalVariable<TProfiler::Auto> *>(&TProfiler::StubLastProfiler);
//---------------------------------------------------------------------------
HRESULT TProfiler::Initialize()
{
  new (&Mutex) TInterlockedMutex;
  FirstProfiler_ = NULL;
  new (&LastProfiler) TThreadLocalVariable<TProfiler::Auto>;
  SetProcessAffinityMask(GetCurrentProcess(),1);
//  SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);
  ProcessTSC = rdtsc();
//  Sleep(10000);
//  StdErr.SysLog(lmINFO,L"%S %18.18f\n",
//	  __PRETTY_FUNCTION__,double(rdtsc() - ProcessTSC) / (10000.)
//  );
//  SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
  return S_OK;
}
//---------------------------------------------------------------------------
void TProfiler::QSortProfilers(TProfiler ** Ptr,long lb,long ub)
{
  long l, r, m;
  if( lb < ub ){
    if( ub - lb < 12 ){
      for( r = ub; r > lb; r-- ){
        for( m = r, l = m - 1; l >= lb; l-- ) if( Ptr[m]->TSC_ < Ptr[l]->TSC_ ) m = l;
        TProfiler * t = Ptr[m];
        Ptr[m] = Ptr[r];
        Ptr[r] = t;
      }
    }
    else {
      m = lb + ((ub - lb) >> 1);
      l = lb, r = ub;
      while(1){
        while( l < r && Ptr[m]->TSC_ > Ptr[l]->TSC_ ) l++;
        while( r >= l && Ptr[m]->TSC_ < Ptr[r]->TSC_ ) r--;
        if( l >= r ) break;
        TProfiler * t = Ptr[l];
        Ptr[l] = Ptr[r];
        Ptr[r] = t;
        if( r == m ) m = l; else if( l == m ) m = r;
        l++;
        r--;
      }
      QSortProfilers(Ptr, lb, r);
      QSortProfilers(Ptr, r + 1, ub);
    }
  }
}
//---------------------------------------------------------------------------
HRESULT TProfiler::Cleanup()
{
  ProcessTSC = rdtsc() - ProcessTSC;
  Enabled_ = false;
  LastProfiler.~TThreadLocalVariable<TProfiler::Auto>();
  Mutex.~TInterlockedMutex();
  StdErr.SysLog(lmPROFILER,L"Execution time %I64dtps %I64d:%02I64d:%02I64d:%02I64d.%03I64d\n",
    ProcessTSC,
	  ProcessTSC / (CPU_TPS * 60 * 60 * 24 * 1000),                // days
	  ProcessTSC / (CPU_TPS * 60 * 60 * 1000),                     // hours
	  ProcessTSC / (CPU_TPS * 60 * 1000),                          // minutes
	  ProcessTSC / (CPU_TPS * 1000),                		           // seconds
	  ProcessTSC / CPU_TPS - ProcessTSC / (CPU_TPS * 1000) * 1000  // microseconds
	);
	long i, Count = 0;
	TProfiler * Profiler = FirstProfiler_;
	while( Profiler != NULL ){
    Count++;
	  Profiler = Profiler->NextProfiler_;
	}
	TProfiler ** Profilers = (TProfiler **) LocalAlloc(LMEM_FIXED,Count * sizeof(TProfiler *));
	Count = 0;
	Profiler = FirstProfiler_;
	while( Profiler != NULL ){
    Profilers[Count++] = Profiler;
	  Profiler = Profiler->NextProfiler_;
	}
	if( Count > 0 && Profilers == NULL ){
    StdErr.SysLog(lmPROFILER,L"E_OUTOFMEMORY\n");
  }
  else if( ProcessTSC > 0 || Count > 0 ){
    QSortProfilers(Profilers,0,Count - 1);
	  StdErr.SysLog(lmDIRECT,
      L"\n    +%%     -%%              tps    counter  function\n"
    );
    longlong TSC = 0;
  	for( i = Count - 1; i >= 0; i-- ) TSC += Profilers[i]->TSC_;
  	for( i = Count - 1; i >= 0; i-- ){
  	  StdErr.SysLog(lmDIRECT,L"%2I64d.%03I64d %2I64d.%03I64d %16I64d %10I64d  %S %S %d\n",
        Profilers[i]->TSC_ * 100 / ProcessTSC,
        Profilers[i]->TSC_ * 100000 / ProcessTSC % 1000,
        Profilers[i]->TSC_ * 100 / TSC,
        Profilers[i]->TSC_ * 100000 / TSC % 1000,
        Profilers[i]->TSC_,
        Profilers[i]->Count_,
        Profilers[i]->PrettyFunc_,
        Profilers[i]->FuncFile_,
        Profilers[i]->FuncLine_
  	  );
  	}
  	LocalFree(Profilers);
  }
  return S_OK;
}
//---------------------------------------------------------------------------
TProfiler::TProfiler(const char * Func,const char * PrettyFunc,const char * FuncFile,long FuncLine)
{
  if( Enabled_ ){
    NextProfiler_ = FirstProfiler_;
    Func_ = Func;
    PrettyFunc_ = PrettyFunc;
    FuncFile_ = FuncFile;
    FuncLine_ = FuncLine;
    TSC_ = 0;
    Count_ = 0;
    FirstProfiler_ = this;
  }
}
//---------------------------------------------------------------------------
TProfiler::~TProfiler()
{
}
//---------------------------------------------------------------------------
TProfiler::Auto::Auto(TProfiler * Profiler)
{
  if( Enabled_ ){
    Profiler_ = Profiler;
    LastProfiler_ = TProfiler::LastProfiler;
    TSCDelta_ = 0;
    TProfiler::LastProfiler = this;
    TSC_ = rdtsc();
  }
}
//---------------------------------------------------------------------------
TProfiler::Auto::~Auto()
{
  if( Enabled_ ){
    longlong a = rdtsc() - TSC_;
    TSCDelta_ += a;
    Profiler_->Acquire();
    Profiler_->TSC_ += TSCDelta_;
    Profiler_->Count_++;
    Profiler_->Release();
/*    Auto * Profiler = LastProfiler_;
    while( Profiler != NULL ){
      Profiler->Profiler_->Acquire();
      Profiler->Profiler_->TSC_ -= a;
      Profiler->Profiler_->Release();
      Profiler = Profiler->LastProfiler_;
    }*/
    if( LastProfiler_ != NULL ){
      LastProfiler_->Profiler_->Acquire();
      LastProfiler_->Profiler_->TSCDelta_ -= a;
      LastProfiler_->Profiler_->Release();
    }
    TProfiler::LastProfiler = LastProfiler_;
  }
}
//---------------------------------------------------------------------------
void TProfiler::Constructor(const char * Func,const char * PrettyFunc,const char * FuncFile,long FuncLine,bool & Flag)
{
  Mutex.Acquire();
  if( !Flag ){
    new (this) TProfiler(Func,PrettyFunc,FuncFile,FuncLine);
    Flag = true;
  }
  Mutex.Release();
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
