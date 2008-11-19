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
#ifndef _ReadWriteLock_H_
#define _ReadWriteLock_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class LiteWriteLock {
  friend void initialize(int,char **);
  friend void cleanup();
  public:
    ~LiteWriteLock();
    LiteWriteLock();

    LiteWriteLock & acquire();
    bool tryAcquire();
    LiteWriteLock & release();

    static void (* acquire_)(volatile ilock_t & ref);
    static void singleAcquire(volatile ilock_t & ref);
    static void multiAcquire(volatile ilock_t & ref);
  protected:
  private:
    volatile ilock_t refCount_;

    LiteWriteLock(const LiteWriteLock &){}
    void operator =(const LiteWriteLock &){}

    static void initialize();
    static void cleanup();
};
//---------------------------------------------------------------------------
inline LiteWriteLock::~LiteWriteLock()
{
  assert( refCount_ == 0 );
}
//---------------------------------------------------------------------------
inline LiteWriteLock::LiteWriteLock() : refCount_(0)
{
}
//---------------------------------------------------------------------------
inline LiteWriteLock & LiteWriteLock::acquire()
{
  acquire_(refCount_);
  return *this;
}
//---------------------------------------------------------------------------
inline bool LiteWriteLock::tryAcquire()
{
  return interlockedCompareExchange(refCount_,-1,0) == 0;
}
//---------------------------------------------------------------------------
inline LiteWriteLock & LiteWriteLock::release()
{
  interlockedIncrement(refCount_,1);
  return *this;
}
//---------------------------------------------------------------------------
class AutoILock {
  public:
    ~AutoILock() { interlockedIncrement(*ref_,1); }
    AutoILock(volatile ilock_t & ref) : ref_(&ref) { LiteWriteLock::acquire_(ref); }
  protected:
  private:
    volatile ilock_t * ref_;

    AutoILock(const AutoILock &);
    void operator = (const AutoILock &);
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class LiteReadWriteLock : protected LiteWriteLock
{
  friend class LiteWriteLock;
  public:
    ~LiteReadWriteLock();
    LiteReadWriteLock();

    LiteReadWriteLock & rdLock();
    bool tryRDLock();
    LiteReadWriteLock & wrLock();
    bool tryWRLock();
    LiteReadWriteLock & unlock();
  protected:
  private:
    volatile ilock_t value_;
    static void (*rdLock_)(LiteReadWriteLock * mutex);
    static void singleRDLock(LiteReadWriteLock * mutex);
    static void multiRDLock(LiteReadWriteLock * mutex);
    static void (*wrLock_)(LiteReadWriteLock * mutex);
    static void singleWRLock(LiteReadWriteLock * mutex);
    static void multiWRLock(LiteReadWriteLock * mutex);

    LiteReadWriteLock(const LiteReadWriteLock &) {}
    void operator =(const LiteReadWriteLock &) {}
};
//---------------------------------------------------------------------------
inline LiteReadWriteLock::~LiteReadWriteLock()
{
  assert( value_ == 0 );
}
//---------------------------------------------------------------------------
inline LiteReadWriteLock::LiteReadWriteLock() : value_(0)
{
}
//---------------------------------------------------------------------------
inline LiteReadWriteLock & LiteReadWriteLock::rdLock()
{
  rdLock_(this);
  return *this;
}
//---------------------------------------------------------------------------
inline bool LiteReadWriteLock::tryRDLock()
{
  acquire();
  bool r = value_ >= 0;
  if( r ) value_++;
  release();
  return r;
}
//---------------------------------------------------------------------------
inline LiteReadWriteLock & LiteReadWriteLock::wrLock()
{
  wrLock_(this);
  return *this;
}
//---------------------------------------------------------------------------
inline bool LiteReadWriteLock::tryWRLock()
{
  acquire();
  bool r = value_ == 0;
  if( r ) value_ = -1;
  release();
  return r;
}
//---------------------------------------------------------------------------
inline LiteReadWriteLock & LiteReadWriteLock::unlock()
{
  acquire();
  assert( value_ != 0 );
  if( value_ > 0 ) value_--; else if( value_ < 0 ) value_++;
  release();
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class WriteLock {
  friend void initialize(int,char **);
  friend void cleanup();
  friend class LiteWriteLock;
  public:
    ~WriteLock();
    WriteLock();

    WriteLock & acquire();
    bool tryAcquire();
    WriteLock & release();
  protected:
  private:
#if defined(__WIN32__) || defined(__WIN64__)
    HANDLE sem_;
#elif HAVE_PTHREAD_H
    pthread_mutex_t mutex_;
    static uint8_t pthreadReadWriteLockNull_[sizeof(pthread_mutex_t)];
#endif
    WriteLock(const WriteLock &){}
    void operator =(const WriteLock &){}
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ReadWriteLock
#if !HAVE_PTHREAD_H
  : protected WriteLock
#endif
{
  friend class LiteWriteLock;
  public:
    ~ReadWriteLock();
    ReadWriteLock();

    ReadWriteLock & rdLock();
    bool tryRDLock();
    ReadWriteLock & wrLock();
    bool tryWRLock();
    ReadWriteLock & unlock();
  protected:
  private:
#if HAVE_PTHREAD_H
    pthread_rwlock_t mutex_;
    static uint8_t pthreadRWLockNull_[sizeof(pthread_rwlock_t)];
#else
    volatile int32_t value_;
    static void (*rdLock_)(ReadWriteLock * mutex);
    static void singleRDLock(ReadWriteLock * mutex);
    static void multiRDLock(ReadWriteLock * mutex);
    static void (*wrLock_)(ReadWriteLock * mutex);
    static void singleWRLock(ReadWriteLock * mutex);
    static void multiWRLock(ReadWriteLock * mutex);
#endif
    ReadWriteLock(const ReadWriteLock &){}
    void operator =(const ReadWriteLock &){}
};
//---------------------------------------------------------------------------
#if !HAVE_PTHREAD_H
//---------------------------------------------------------------------------
inline ReadWriteLock::~ReadWriteLock()
{
  assert( value_ == 0 );
}
//---------------------------------------------------------------------------
inline ReadWriteLock::ReadWriteLock() : value_(0)
{
}
//---------------------------------------------------------------------------
inline ReadWriteLock & ReadWriteLock::rdLock()
{
  rdLock_(this);
  return *this;
}
//---------------------------------------------------------------------------
inline bool ReadWriteLock::tryRDLock()
{
  acquire();
  bool  r = value_ >= 0;
  if( r ) value_++;
  release();
  return r;
}
//---------------------------------------------------------------------------
inline ReadWriteLock & ReadWriteLock::wrLock()
{
  wrLock_(this);
  return *this;
}
//---------------------------------------------------------------------------
inline bool ReadWriteLock::tryWRLock()
{
  acquire();
  bool r = value_ == 0;
  if( r ) value_ = -1;
  release();
  return r;
}
//---------------------------------------------------------------------------
inline ReadWriteLock & ReadWriteLock::unlock()
{
  acquire();
  assert( value_ != 0 );
  if( value_ > 0 ) value_--; else if( value_ < 0 ) value_++;
  release();
  return *this;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T> class AutoReadWriteLockRDLock {
  public:
    ~AutoReadWriteLockRDLock();
    AutoReadWriteLockRDLock(T & mutex);
  protected:
  private:
    T & mutex_;

    AutoReadWriteLockRDLock(const AutoReadWriteLockRDLock<T> &){}
    void operator =(const AutoReadWriteLockRDLock<T> &){}
};
//---------------------------------------------------------------------------
template<typename T> inline AutoReadWriteLockRDLock<T>::~AutoReadWriteLockRDLock()
{
  mutex_.unlock();
}
//---------------------------------------------------------------------------
template<typename T> inline AutoReadWriteLockRDLock<T>::AutoReadWriteLockRDLock(T & mutex) : mutex_(mutex)
{
  mutex_.rdLock();
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T> class AutoReadWriteLockWRLock {
  public:
    ~AutoReadWriteLockWRLock();
    AutoReadWriteLockWRLock(T & mutex);
  protected:
  private:
    T & mutex_;

    AutoReadWriteLockWRLock(const AutoReadWriteLockWRLock<T> &){}
    void operator =(const AutoReadWriteLockWRLock<T> &){}
};
//---------------------------------------------------------------------------
template<typename T> inline AutoReadWriteLockWRLock<T>::~AutoReadWriteLockWRLock()
{
  mutex_.unlock();
}
//---------------------------------------------------------------------------
template<typename T> inline AutoReadWriteLockWRLock<T>::AutoReadWriteLockWRLock(T & mutex) : mutex_(mutex)
{
  mutex_.wrLock();
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
extern uint8_t giantPlaceHolder[];
inline WriteLock & giant()
{
  return *reinterpret_cast<WriteLock *>(giantPlaceHolder);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T> class AutoLock {
  public:
    ~AutoLock();
    AutoLock();
    AutoLock(T & mutex);
    AutoLock & setLocked(T & mutex){ mutex.acquire(); mutex_ = &mutex; return *this; }
    AutoLock & unLock(){ if( mutex_ != NULL ) mutex_->release(); mutex_ = NULL; return *this; }
  protected:
  private:
    T * mutex_;

    AutoLock(const AutoLock<T> &);
    void operator =(const AutoLock<T> &);
};
//---------------------------------------------------------------------------
template <typename T> inline AutoLock<T>::~AutoLock()
{
  if( mutex_ != NULL ) mutex_->release();
}
//---------------------------------------------------------------------------
template <typename T> inline AutoLock<T>::AutoLock() : mutex_(NULL)
{
}
//---------------------------------------------------------------------------
template <typename T> inline AutoLock<T>::AutoLock(T & mutex) : mutex_(&mutex)
{
  mutex_->acquire();
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
#endif /* _ReadWriteLock_H_ */
//---------------------------------------------------------------------------

