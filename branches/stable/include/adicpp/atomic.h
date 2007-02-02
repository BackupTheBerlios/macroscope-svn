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
#ifndef _atomic_H_
#define _atomic_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
//#if __i386__ || __x86_64__
//#define FAST_MUTEX 1
//#endif
//---------------------------------------------------------------------------
/*#ifndef MPLOCKED
#ifdef SMP
#define __STRING(x) #x
#define __XSTRING(x) __STRING(x)
#else
#define __STRING(x) #x
#define __XSTRING(x) __STRING(x)
#endif
#endif
#define MPLOCKED2 "lock ; "
//__XSTRING(MPLOCKED2)*/
//---------------------------------------------------------------------------
#if __GNUG__ && __i386__
//inline int32_t interlockedIncrement(volatile int32_t * v,int32_t a)
//{
//  asm volatile (
//    " lock ; "
//    "       xaddl   %0, %1 ;        "
//    "# atomic_fetchadd_int"
//    : "+r" (a),                     /* 0 (result) */
//    "=m" (*v)                     /* 1 */
//    : "m" (*v));                    /* 2 */
//  return a;
//}

inline int32_t interlockedIncrement(volatile int32_t & v,int32_t a)
{
  asm volatile ( "lock; xadd %%eax,(%%edx)" : "=a" (a) : "d" (&v), "a" (a));
  return a;
/*  //  register long eax asm("eax");
  //  long r;
  __asm __volatile (//    ".arch i686\n"
  //    ".intel_syntax noprefix\n"
  __XSTRING(MPLOCKED) "xadd %%eax,(%%edx)\n"
  // возвращаемые параметры
  : "=a" (a)
  // принимаемые параметры    
  : "d" (&v), // первый параметр %0 c - edx
  "a" (a)  // второй параметр %1 a - eax
  // регистры, в которые гадим
  //    : "ecx"
  );*/
//  return interlockedIncrement(&v,a);
#if HAVE_MACHINE_ATOMIC_H
//  return atomic_fetchadd_32((volatile u_int *) &v,a);
#endif
}

int64_t interlockedIncrement(volatile int64_t & v,int64_t a);

inline int32_t interlockedCompareExchange(volatile int32_t & v, int32_t exValue, int32_t cmpValue)
{
  asm volatile ("lock; cmpxchg %%ecx,(%%edx)" : "=a" (cmpValue) : "d" (&v), "a" (cmpValue), "c" (exValue));
  return cmpValue;
}

inline int64_t interlockedCompareExchange(volatile int64_t & v,int64_t exValue,int64_t cmpValue)
{
//  asm volatile ("lock; cmpxchg8b (%%ebp)" : "=a" (cmpValue) : "ebp" (&v), "a" (cmpValue), "c" (exValue));
//#if HAVE_MACHINE_ATOMIC_H
//  atomic_cmpset_long(&v,cmpValue,exValue);
//#endif
//  return cmpValue;
  return interlockedCompareExchange(*(int32_t *) &v,int32_t(exValue),int32_t(cmpValue));
}
#elif __GNUG__ && __x86_64__
inline int32_t interlockedIncrement(volatile int32_t & v, int32_t a)
{
  asm volatile ("lock; xadd %%eax,(%%rdx)" : "=a" (a) : "d" (&v), "a" (a));
  return a;
}

inline int64_t interlockedIncrement(volatile int64_t & v,int64_t a)
{
  asm volatile ("lock; xadd %%rax,(%%rdx)" : "=a" (a) : "d" (&v), "a" (a));
  return a;
}

inline int32_t interlockedCompareExchange(volatile int32_t & v, int32_t exValue, int32_t cmpValue)
{
  asm volatile ("lock; cmpxchg %%ecx,(%%rdx)" : "=a" (cmpValue) : "d" (&v), "a" (cmpValue), "c" (exValue));
  return cmpValue;
}

inline int64_t interlockedCompareExchange(volatile int64_t & v,int64_t exValue,int64_t cmpValue)
{
  asm volatile ("lock; cmpxchg %%rcx,(%%rdx)" : "=a" (cmpValue) : "d" (&v), "a" (cmpValue), "c" (exValue));
  return cmpValue;
}
#elif _MSC_VER && _M_IX86
__forceinline int32_t __fastcall interlockedIncrement(volatile int32_t & v,int32_t a)
{
  __asm {
    mov         eax,a
    mov         edx,v
    lock xadd   [edx],eax
  }
}

int64_t interlockedIncrement(volatile int64_t & v,int64_t a);

__forceinline int32_t interlockedCompareExchange(volatile int32_t & v,int32_t exValue,int32_t cmpValue)
{
  __asm {
    mov         eax,cmpValue
    mov         ecx,exValue
    mov         edx,v
    lock cmpxchg [edx],ecx
  }
}

__forceinline int64_t interlockedCompareExchange(volatile int64_t & v,int64_t exValue,int64_t cmpValue)
{
  __asm {
    mov         esi,v
    mov         ebx,dword ptr exValue
    mov         ecx,dword ptr exValue + 4
    mov         eax,dword ptr cmpValue
    mov         edx,dword ptr cmpValue + 4
    lock cmpxchg8b qword ptr [esi]
  }
}
#elif _MSC_VER && _M_X64
__forceinline int32_t interlockedIncrement(volatile int32_t & v,int32_t a)
{
  return InterlockedExchangeAdd((LONG *) &v,a);
}

__forceinline int64_t interlockedIncrement(volatile int64_t & v,int64_t a)
{
  return InterlockedExchangeAdd64((LONGLONG *) &v,a);
}

__forceinline int32_t interlockedCompareExchange(volatile int32_t & v, int32_t exValue, int32_t cmpValue)
{
  return InterlockedCompareExchange((LONG *) &v,exValue,cmpValue);
}

__forceinline int64_t interlockedCompareExchange(volatile int64_t & v,int64_t exValue,int64_t cmpValue)
{
  return InterlockedCompareExchange64((LONGLONG *) &v,exValue,cmpValue);
}
#elif defined(__BCPLUSPLUS__)
int32_t __fastcall  __declspec(nothrow) interlockedIncrement(int32_t & v,int32_t a);
int32_t __fastcall  __declspec(nothrow) interlockedCompareExchange(int32_t & v,int32_t exValue,int32_t cmpValue);
#else
int32_t interlockedIncrement(volatile int32_t & v, int32_t a);
int32_t interlockedCompareExchange(volatile int32_t & v, int32_t exValue, int32_t cmpValue);
#endif
//---------------------------------------------------------------------------
void interlockedCompareExchangeAcquire(volatile int32_t & v,int32_t exValue,int32_t cmpValue);
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _atomic_H_ */
//---------------------------------------------------------------------------
