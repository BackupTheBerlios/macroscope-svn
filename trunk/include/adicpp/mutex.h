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
inline void sleep(uint64_t milisec)
{
#if HAVE_NANOSLEEP
  struct timespec rqtp;
  rqtp.tv_sec = milisec / 1000000u;
  rqtp.tv_nsec = milisec % 1000000u;
  nanosleep(&rqtp,NULL);
#elif defined(__WIN32__) || defined(__WIN64__)
  Sleep((DWORD) (milisec / 1000u));
#elif HAVE_USLEEP
  usleep(milisec / 1000u);
#elif HAVE_SLEEP
  sleep(milisec / 1000000u);
#endif
}
//---------------------------------------------------------------------------
inline void sleep1()
{
#if HAVE_NANOSLEEP
  struct timespec rqtp;
  rqtp.tv_sec = 0;
  rqtp.tv_nsec = 1;
  nanosleep(&rqtp,NULL);
#elif defined(__WIN32__) || defined(__WIN64__)
  Sleep(1);
#elif HAVE_USLEEP
  usleep(1);
#elif HAVE_SLEEP
  sleep(1);
#endif
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if __i386__ || __x86_64__
#define FAST_MUTEX 1
#endif
//---------------------------------------------------------------------------
#if __GNUG__ && __i386__
inline int32_t interlockedIncrement(int32_t * v,int32_t a)
{
//  register long eax asm("eax");
//  long r;
  asm volatile (
//    ".arch i686\n"
//    ".intel_syntax noprefix\n"
      "lock; xadd %%eax,(%%edx)\n"
// возвращаемые параметры
    : "=a" (a)
// принимаемые параметры    
    : "d" (v), // первый параметр %0 c - edx
      "a" (a)  // второй параметр %1 a - eax
// регистры, в которые гадим
//    : "ecx"
  );
  return a;
}

inline int32_t interlockedCompareExchange(int32_t * v,int32_t exValue,int32_t cmpValue)
{
  asm volatile (
    "lock; cmpxchg %%ecx,(%%edx)" : "=a" (cmpValue) : "d" (v), "a" (cmpValue), "c" (exValue)
  );
  return cmpValue;
}
#elif __GNUG__ && __x86_64__
inline int32_t interlockedIncrement(int32_t * v,int32_t a)
{
  asm volatile (
    "lock; xadd %%eax,(%%rdx)" : "=a" (a) : "d" (v), "a" (a)
  );
  return a;
}

inline int32_t interlockedCompareExchange(int32_t * v,int32_t exValue,int32_t cmpValue)
{
  asm volatile (
    "lock; cmpxchg %%ecx,(%%rdx)" : "=a" (cmpValue) : "d" (v), "a" (cmpValue), "c" (exValue)
  );
  return cmpValue;
}
#elif defined(_MSC_VER)
inline int32_t __fastcall interlockedIncrement(int32_t * v,int32_t a)
{
  __asm {
	  mov         eax,a
	  mov         edx,v
	  lock xadd   [edx],eax
  }
}

inline int32_t interlockedCompareExchange(int32_t * v,int32_t exValue,int32_t cmpValue)
{
  __asm {
	  mov         eax,cmpValue
	  mov         ecx,exValue
	  mov         edx,v
	  lock cmpxchg [edx],ecx
  }
}
#elif defined(__BCPLUSPLUS__)
int32_t __fastcall __declspec(nothrow) interlockedIncrement(int32_t * v,int32_t a);
int32_t __fastcall __declspec(nothrow) interlockedCompareExchange(int32_t * v,int32_t exValue,int32_t cmpValue);
#else
int32_t interlockedIncrement(int32_t * v,int32_t a);
int32_t interlockedCompareExchange(int32_t * v,int32_t exValue,int32_t cmpValue);
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class InterlockedMutex {
  friend void initialize();
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
    CRITICAL_SECTION cs_;
#elif HAVE_PTHREAD_H
    pthread_mutex_t mutex_;
#endif
    static void initialize();
    static void cleanup();
    InterlockedMutex(const InterlockedMutex &){}
    InterlockedMutex & operator = (const InterlockedMutex &){ return *this; }
};
//---------------------------------------------------------------------------
inline InterlockedMutex::~InterlockedMutex()
{
#if FAST_MUTEX
#elif defined(__WIN32__) || defined(__WIN64__)
  DeleteCriticalSection(&cs_);
#elif HAVE_PTHREAD_H
  if( mutex_ != NULL ){
#ifndef NDEBUG
    int r =
#endif
    pthread_mutex_destroy(&mutex_);
#ifndef NDEBUG
    assert( r == 0 );
#endif
  }
#endif
}
//---------------------------------------------------------------------------
#if FAST_MUTEX
inline InterlockedMutex::InterlockedMutex() : refCount_(0)
{
}
#endif
//---------------------------------------------------------------------------
inline void InterlockedMutex::acquire()
{
#if FAST_MUTEX
  acquire_(this);
#elif defined(__WIN32__) || defined(__WIN64__)
  EnterCriticalSection(&cs_);
#elif HAVE_PTHREAD_H
#ifndef NDEBUG
  int r =
#endif
  pthread_mutex_lock(&mutex_);
#ifndef NDEBUG
  assert( r == 0 );
#endif
#endif
}
//---------------------------------------------------------------------------
inline bool InterlockedMutex::tryAcquire()
{
#if FAST_MUTEX
  return interlockedCompareExchange(&refCount_,-1,0) == 0;
#elif defined(__WIN32__) || defined(__WIN64__)
  return TryEnterCriticalSection(&cs_) != 0;
#elif HAVE_PTHREAD_H
#ifndef NDEBUG
  int r =
#else
  return
#endif
  pthread_mutex_trylock(&mutex_)
#ifndef NDEBUG
  ;
  assert( r == 0 || errno == EBUSY );
  return r == 0;
#else
  == 0;
#endif
#endif
}
//---------------------------------------------------------------------------
inline void InterlockedMutex::release()
{
#if FAST_MUTEX
  interlockedIncrement(&refCount_,1);
#elif defined(__WIN32__) || defined(__WIN64__)
  LeaveCriticalSection(&cs_);
#elif HAVE_PTHREAD_H
#ifndef NDEBUG
  int r =
#endif
  pthread_mutex_unlock(&mutex_);
#ifndef NDEBUG
  assert( r == 0 );
#endif
#endif
}
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

    Mutex & rdLock();
    bool tryRDLock();
    Mutex & wrLock();
    bool tryWRLock();
    Mutex & unlock();
  protected:
  private:
#if FAST_MUTEX || !HAVE_PTHREAD_RWLOCK_INIT
    int32_t value_;
    static void (*rdLock_)(Mutex * mutex);
    static void singleRDLock(Mutex * mutex);
    static void multiRDLock(Mutex * mutex);
    static void (*wrLock_)(Mutex * mutex);
    static void singleWRLock(Mutex * mutex);
    static void multiWRLock(Mutex * mutex);
#elif HAVE_PTHREAD_RWLOCK_INIT
    pthread_rwlock_t mutex_;
#else
#error pthread_rwlock not implemented
#endif
    Mutex(const Mutex &){}
    Mutex & operator = (const Mutex &){ return *this; }
};
//---------------------------------------------------------------------------
inline Mutex::~Mutex()
{
#if FAST_MUTEX || !HAVE_PTHREAD_RWLOCK_INIT
#elif HAVE_PTHREAD_RWLOCK_INIT
#ifndef NDEBUG
  int r =
#endif
  pthread_rwlock_destroy(&mutex_);
  assert( r == 0 );
#endif
}
//---------------------------------------------------------------------------
inline Mutex & Mutex::rdLock()
{
#if FAST_MUTEX || !HAVE_PTHREAD_RWLOCK_INIT
  rdLock_(this);
#elif HAVE_PTHREAD_RWLOCK_INIT
#ifndef NDEBUG
  int r =
#endif
  pthread_rwlock_rdlock(&mutex_);
  assert( r == 0 );
#endif
  return *this;
}
//---------------------------------------------------------------------------
inline bool Mutex::tryRDLock()
{
#if FAST_MUTEX || !HAVE_PTHREAD_RWLOCK_INIT
  acquire();
  bool r = value_ >= 0;
  if( r ) value_++;
  release();
  return r;
#elif HAVE_PTHREAD_RWLOCK_INIT
  int r =
  pthread_rwlock_tryrdlock(&mutex_);
  assert( r == 0 || errno == EBUSY );
  return r == 0;
#endif
}
//---------------------------------------------------------------------------
inline Mutex & Mutex::wrLock()
{
#if FAST_MUTEX || !HAVE_PTHREAD_RWLOCK_INIT
  wrLock_(this);
#elif HAVE_PTHREAD_RWLOCK_INIT
#ifndef NDEBUG
  int r =
#endif
  pthread_rwlock_wrlock(&mutex_);
  assert( r == 0 );
#endif
  return *this;
}
//---------------------------------------------------------------------------
inline bool Mutex::tryWRLock()
{
#if FAST_MUTEX || !HAVE_PTHREAD_RWLOCK_INIT
  acquire();
  bool r = value_ == 0;
  if( r ) value_ = -1;
  release();
  return r;
#elif HAVE_PTHREAD_RWLOCK_INIT
  int r =
  pthread_rwlock_trywrlock(&mutex_);
  assert( r == 0 || errno == EBUSY );
  return r == 0;
#endif
}
//---------------------------------------------------------------------------
inline Mutex & Mutex::unlock()
{
#if FAST_MUTEX || !HAVE_PTHREAD_RWLOCK_INIT
  acquire();
  assert( value_ != 0 );
  if( value_ > 0 ) value_--; else if( value_ < 0 ) value_++;
  release();
#elif HAVE_PTHREAD_RWLOCK_INIT
#ifndef NDEBUG
  int r =
#endif
  pthread_rwlock_unlock(&mutex_);
  assert( r == 0 );
#endif
  return *this;
}
//---------------------------------------------------------------------------
extern uint8_t giantPlaceHolder[];
inline InterlockedMutex & giant()
{
  return *reinterpret_cast<InterlockedMutex *>(giantPlaceHolder);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T>
class AutoLock {
  public:
    ~AutoLock();
    AutoLock(T & mutex);
  protected:
  private:
    T & mutex_;
    AutoLock(const AutoLock<T> &) {}
    AutoLock<T> & operator = (const AutoLock<T> &) { return *this; }
};
//---------------------------------------------------------------------------
template <typename T> inline
AutoLock<T>::~AutoLock()
{
  mutex_.release();
}
//---------------------------------------------------------------------------
template <typename T> inline
AutoLock<T>::AutoLock(T & mutex) : mutex_(mutex)
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
    pthread_cond_t handle_;
    pthread_mutex_t mutex_;
#elif defined(__WIN32__) || defined(__WIN64__)
    HANDLE handle_;
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
  pthread_cond_wait(&handle_,&mutex_);
#ifndef NDEBUG
  assert( r == 0 );
#endif
#elif defined(__WIN32__) || defined(__WIN64__)
  WaitForSingleObject(handle_,INFINITE);
#endif
  return *this;
}
//---------------------------------------------------------------------------
inline bool Event::tryWait()
{
#if HAVE_PTHREAD_H
  struct timeval now;
  struct timespec timeout;
  gettimeofday(&now,NULL);
  timeout.tv_sec = now.tv_sec;
  timeout.tv_nsec = now.tv_usec * 1000;
  int err = pthread_cond_timedwait(&handle_,&mutex_,&timeout);
  assert( err == 0 || err == ETIMEDOUT );
  return err == 0;
#elif defined(__WIN32__) || defined(__WIN64__)
  return WaitForSingleObject(handle_,0) == WAIT_OBJECT_0;
#endif
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline uintptr_t numberOfProcessors()
{
  uintptr_t n = 1;
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
