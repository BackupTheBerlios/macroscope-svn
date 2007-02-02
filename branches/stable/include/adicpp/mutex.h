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
#ifndef _Mutex_H_
#define _Mutex_H_
//---------------------------------------------------------------------------
namespace ksys {
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

