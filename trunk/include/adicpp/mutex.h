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
#ifndef _Mutex_H_
#define _Mutex_H_
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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class InterlockedMutex {
  friend void initialize(int,char **);
  friend void cleanup();
  friend class Mutex;
  public:
    ~InterlockedMutex();
    InterlockedMutex();

    void acquire();
    bool tryAcquire();
    void release();
  protected:
  private:
#if FAST_MUTEX
    int32_t refCount_;

    static void (* acquire_)(InterlockedMutex * mutex);
    static void singleAcquire(InterlockedMutex * mutex);
    static void multiAcquire(InterlockedMutex * mutex);
#elif defined(__WIN32__) || defined(__WIN64__)
//    static CRITICAL_SECTION staticCS_;
//    CRITICAL_SECTION cs_;
    HANDLE sem_;
#elif HAVE_PTHREAD_H
    pthread_mutex_t mutex_;
#endif
    static void initialize();
    static void cleanup();
    InterlockedMutex(const InterlockedMutex &){}
    void operator =(const InterlockedMutex &){}
};
//---------------------------------------------------------------------------
#if FAST_MUTEX
inline InterlockedMutex::InterlockedMutex() : refCount_(0)
{
}
#endif
//---------------------------------------------------------------------------
#if FAST_MUTEX
//---------------------------------------------------------------------------
inline void InterlockedMutex::acquire()
{
  acquire_(this);
}
//---------------------------------------------------------------------------
inline bool InterlockedMutex::tryAcquire()
{
  return interlockedCompareExchange(refCount_, -1, 0) == 0;
}
//---------------------------------------------------------------------------
inline void InterlockedMutex::release()
{
  interlockedIncrement(refCount_, 1);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Mutex
#if FAST_MUTEX || !HAVE_PTHREAD_RWLOCK_INIT
 : public InterlockedMutex
#endif
{
  friend class InterlockedMutex;
  public:
    ~Mutex();
    Mutex();

    Mutex &     rdLock();
    bool        tryRDLock();
    Mutex &     wrLock();
    bool        tryWRLock();
    Mutex &     unlock();
  protected:
  private:
#if FAST_MUTEX || !HAVE_PTHREAD_RWLOCK_INIT
    int32_t           value_;
    static void (*rdLock_)(Mutex * mutex);
    static void singleRDLock(Mutex * mutex);
    static void multiRDLock(Mutex * mutex);
    static void (*wrLock_)(Mutex * mutex);
    static void singleWRLock(Mutex * mutex);
    static void multiWRLock(Mutex * mutex);
#elif HAVE_PTHREAD_RWLOCK_INIT
    pthread_rwlock_t  mutex_;
#else
#error pthread_rwlock not implemented
#endif
    Mutex(const Mutex &){}
    void operator =(const Mutex &){}
};
//---------------------------------------------------------------------------
#if FAST_MUTEX || !HAVE_PTHREAD_RWLOCK_INIT
//---------------------------------------------------------------------------
inline Mutex::~Mutex()
{
}
//---------------------------------------------------------------------------
inline Mutex::Mutex()
{
  value_ = 0;
}
//---------------------------------------------------------------------------
inline Mutex & Mutex::rdLock()
{
  rdLock_(this);
  return *this;
}
//---------------------------------------------------------------------------
inline bool Mutex::tryRDLock()
{
  acquire();
  bool  r = value_ >= 0;
  if( r ) value_++;
  release();
  return r;
}
//---------------------------------------------------------------------------
inline Mutex & Mutex::wrLock()
{
  wrLock_(this);
  return *this;
}
//---------------------------------------------------------------------------
inline bool Mutex::tryWRLock()
{
  acquire();
  bool r = value_ == 0;
  if( r ) value_ = -1;
  release();
  return r;
}
//---------------------------------------------------------------------------
inline Mutex & Mutex::unlock()
{
  acquire();
  assert(value_ != 0);
  if( value_ > 0 ) value_--; else if( value_ < 0 ) value_++;
  release();
  return *this;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T> class AutoMutexRDLock {
  public:
    ~AutoMutexRDLock();
    AutoMutexRDLock(T & mutex);
  protected:
  private:
    T & mutex_;

    AutoMutexRDLock(const AutoMutexRDLock<T> &){}
    void operator =(const AutoMutexRDLock<T> &){}
};
//---------------------------------------------------------------------------
template<typename T> inline AutoMutexRDLock<T>::~AutoMutexRDLock()
{
  mutex_.unlock();
}
//---------------------------------------------------------------------------
template<typename T> inline AutoMutexRDLock<T>::AutoMutexRDLock(T & mutex) : mutex_(mutex)
{
  mutex_.rdLock();
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T> class AutoMutexWRLock {
  public:
    ~AutoMutexWRLock();
    AutoMutexWRLock(T & mutex);
  protected:
  private:
    T & mutex_;

    AutoMutexWRLock(const AutoMutexWRLock<T> &){}
    void operator =(const AutoMutexWRLock<T> &){}
};
//---------------------------------------------------------------------------
template<typename T> inline AutoMutexWRLock<T>::~AutoMutexWRLock()
{
  mutex_.unlock();
}
//---------------------------------------------------------------------------
template<typename T> inline AutoMutexWRLock<T>::AutoMutexWRLock(T & mutex) : mutex_(mutex)
{
  mutex_.wrLock();
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
extern uint8_t giantPlaceHolder[];
inline InterlockedMutex & giant()
{
  return *reinterpret_cast<InterlockedMutex *>(giantPlaceHolder);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T> class AutoLock {
  public:
    ~AutoLock();
    AutoLock(T & mutex);
  protected:
  private:
    T & mutex_;

    AutoLock(const AutoLock<T> &){}
    void operator =(const AutoLock<T> &){}
};
//---------------------------------------------------------------------------
template <typename T> inline AutoLock<T>::~AutoLock()
{
  mutex_.release();
}
//---------------------------------------------------------------------------
template <typename T> inline AutoLock<T>::AutoLock(T & mutex) : mutex_(mutex)
{
  mutex_.acquire();
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Event {
  public:
    ~Event();
    Event();
#if HAVE_PTHREAD_H
    pthread_cond_t  handle_;
    pthread_mutex_t mutex_;
#elif defined(__WIN32__) || defined(__WIN64__)
    HANDLE          handle_;
#endif
    Event & post();
    Event & wait();
    bool tryWait();
  protected:
  private:
};
//---------------------------------------------------------------------------
inline Event & Event::post()
{
#if HAVE_PTHREAD_H
  pthread_cond_broadcast(&handle_);
#elif defined(__WIN32__) || defined(__WIN64__)
  PulseEvent(handle_);
#endif
  return *this;
}
//---------------------------------------------------------------------------
inline Event & Event::wait()
{
#if HAVE_PTHREAD_H
#ifndef NDEBUG
  int r =
#endif
  pthread_cond_wait(&handle_, &mutex_);
#ifndef NDEBUG
  assert(r == 0);
#endif
#elif defined(__WIN32__) || defined(__WIN64__)
  WaitForSingleObject(handle_, INFINITE);
#endif
  return *this;
}
//---------------------------------------------------------------------------
inline bool Event::tryWait()
{
#if HAVE_PTHREAD_H
  struct timeval  now;
  struct timespec timeout;
  gettimeofday(&now, NULL);
  timeout.tv_sec = now.tv_sec;
  timeout.tv_nsec = now.tv_usec * 1000;
  int err = pthread_cond_timedwait(&handle_, &mutex_, &timeout);
  assert(err == 0 || err == ETIMEDOUT);
  return err == 0;
#elif defined(__WIN32__) || defined(__WIN64__)
  return WaitForSingleObject(handle_, 0) == WAIT_OBJECT_0;
#endif
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline uintptr_t numberOfProcessors()
{
  uintptr_t   n = 1;
#if defined(__WIN32__) || defined(__WIN64__)
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  n = si.dwNumberOfProcessors;
#elif HAVE_SYSCONF && _SC_NPROCESSORS_ONLN
  n = sysconf(_SC_NPROCESSORS_ONLN);
#endif
  return n;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#if !defined(__WIN32__) && !defined(__WIN64__)
inline int32_t SwitchToThread()
{
  return 0;
}
#endif
//---------------------------------------------------------------------------
#endif /* _Mutex_H_ */
//---------------------------------------------------------------------------

