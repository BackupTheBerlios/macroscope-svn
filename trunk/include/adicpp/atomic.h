/*-
 * Copyright 2005-2008 Guram Dukashvili
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
#if __GNUG__ && __i386__
inline int32_t interlockedIncrement(volatile int32_t & v,int32_t a)
{
  asm volatile ( "lock; xadd %%eax,(%%edx)" : "=a" (a) : "d" (&v), "a" (a));
  return a;
}

int64_t interlockedIncrement(volatile int64_t & v,int64_t a);

inline int32_t interlockedCompareExchange(volatile int32_t & v, int32_t exValue, int32_t cmpValue)
{
  asm volatile ("lock; cmpxchg %%ecx,(%%edx)" : "=a" (cmpValue) : "d" (&v), "a" (cmpValue), "c" (exValue));
  return cmpValue;
}

inline int64_t interlockedCompareExchange(volatile int64_t & v,int64_t exValue,int64_t cmpValue)
{
  asm volatile (
    "lock; cmpxchg8b (%%ebp)"
    :
    : "ebp" (&v),
      "b" ((uint32_t) exValue),
      "c" (*((uint32_t *) &exValue + 1)),
      "a" ((uint32_t) cmpValue),
      "d" (*((uint32_t *) &cmpValue + 1)),
  );
  return cmpValue;
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
#elif defined(__WIN32__) || defined(__WIN64__)
inline int32_t interlockedIncrement(volatile int32_t & v,int32_t a)
{
  return InterlockedExchangeAdd((LONG *) &v,a);
}

int64_t interlockedIncrement(volatile int64_t & v,int64_t a);

inline int32_t interlockedCompareExchange(volatile int32_t & v, int32_t exValue, int32_t cmpValue)
{
  return InterlockedCompareExchange((LONG *) &v,exValue,cmpValue);
}

int64_t interlockedCompareExchange(volatile int64_t & v,int64_t exValue,int64_t cmpValue);
#else
int32_t interlockedIncrement(volatile int32_t & v,int32_t a);
int64_t interlockedIncrement(volatile int64_t & v,int64_t a);
int32_t interlockedCompareExchange(volatile int32_t & v,int32_t exValue,int32_t cmpValue);
int64_t interlockedCompareExchange(volatile int64_t & v,int64_t exValue,int64_t cmpValue);
#endif
//---------------------------------------------------------------------------
void interlockedCompareExchangeAcquire(volatile int32_t & v,int32_t exValue,int32_t cmpValue);
void interlockedCompareExchangeAcquire(volatile int64_t & v,int64_t exValue,int64_t cmpValue);
inline uint32_t interlockedIncrement(volatile uint32_t & v,uint32_t a){ return interlockedIncrement(*(volatile int32_t *) &v,(int32_t) a); }
inline uint64_t interlockedIncrement(volatile uint64_t & v,uint64_t a){ return interlockedIncrement(*(volatile int64_t *) &v,(int64_t) a); }
#if SIZEOF_VOID_P <= 4
#if !HAVE_INTPTR_T_AS_INT32_T && !HAVE_INT32_T_AS_INTPTR_T
inline uintptr_t interlockedIncrement(volatile intptr_t & v,intptr_t a){ return interlockedIncrement(*(volatile int32_t *) &v,(int32_t) a); }
inline uintptr_t interlockedIncrement(volatile uintptr_t & v,uintptr_t a){ return interlockedIncrement(*(volatile int32_t *) &v,(int32_t) a); }
#endif
typedef int32_t ilock_t;
typedef uint32_t uilock_t;
#elif SIZEOF_VOID_P <= 8
#if !HAVE_INTPTR_T_AS_INT64_T && !HAVE_INT64_T_AS_INTPTR_T
inline uintptr_t interlockedIncrement(volatile intptr_t & v,intptr_t a){ return interlockedIncrement(*(volatile int64_t *) &v,(int64_t) a); }
inline uintptr_t interlockedIncrement(volatile uintptr_t & v,uintptr_t a){ return interlockedIncrement(*(volatile int64_t *) &v,(int64_t) a); }
#endif
typedef int32_t ilock_t;
typedef uint32_t uilock_t;
#endif
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _atomic_H_ */
//---------------------------------------------------------------------------
