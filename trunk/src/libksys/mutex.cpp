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
#include <adicpp/ksys.h>
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
uint8_t giantPlaceHolder[sizeof(InterlockedMutex)];
//---------------------------------------------------------------------------
#if !defined(__GNUG__) && !defined(__i386__) && !defined(_MSC_VER) && !defined(__BCPLUSPLUS__)
int32_t interlockedIncrement(volatile int32_t & v,int32_t a)
{
  giant().acquire();
  int32_t ov = v;
  v += a;
  giant().release();
  return ov;
}

int64_t interlockedIncrement(volatile int64_t & v,int64_t a)
{
  giant().acquire();
  int64_t ov = v;
  v += a;
  giant().release();
  return ov;
}


int32_t interlockedCompareExchange(volatile int32_t & v,int32_t exValue,int32_t cmpValue)
{
  giant().acquire();
  int32_t ov;
  if( v == cmpValue ){
    ov = cmpValue;
    v = exValue;
  }
  else {
    ov = exValue;
  }
  giant().release();
  return ov;
}

int64_t interlockedCompareExchange(volatile int64_t & v,int64_t exValue,int64_t cmpValue)
{
  giant().acquire();
  int64_t ov;
  if( v == cmpValue ){
    ov = cmpValue;
    v = exValue;
  }
  else {
    ov = exValue;
  }
  giant().release();
  return ov;
}
#endif
//---------------------------------------------------------------------------
#if !defined(__WIN32__) && !defined(__WIN64__)//(_M_IX86 || __i386__ || defined(BCPLUSPLUS)) && !__x86_64__ && !_M_X64
int64_t interlockedIncrement(volatile int64_t & v,int64_t a)
{
  int64_t old;
  do {
    old = v;
  } while( interlockedCompareExchange(v,old + a,old) != old );
  return old;
}
#endif
//---------------------------------------------------------------------------
void interlockedCompareExchangeAcquire(volatile int32_t & v,int32_t exValue,int32_t cmpValue)
{
  for(;;){
    if( interlockedCompareExchange(v,exValue,cmpValue) == 0 ) break;
    ksleep1();
  }
}
//---------------------------------------------------------------------------
void interlockedCompareExchangeAcquire(volatile int64_t & v,int64_t exValue,int64_t cmpValue)
{
  for(;;){
    if( interlockedCompareExchange(v,exValue,cmpValue) == 0 ) break;
    ksleep1();
  }
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
void InterlockedMutex::initialize()
{
#if FAST_MUTEX
  if( numberOfProcessors() > 1 ){
    InterlockedMutex::acquire_ = InterlockedMutex::multiAcquire;
#if !HAVE_PTHREAD_RWLOCK_INIT
    Mutex::rdLock_ = Mutex::multiRDLock;
    Mutex::wrLock_ = Mutex::multiWRLock;
#endif
  }
  else {
    InterlockedMutex::acquire_ = InterlockedMutex::singleAcquire;
#if !HAVE_PTHREAD_RWLOCK_INIT
    Mutex::rdLock_ = Mutex::singleRDLock;
    Mutex::wrLock_ = Mutex::singleWRLock;
#endif
  }
#elif defined(__WIN32__) || defined(__WIN64__)
  //memset(&staticCS_,0,sizeof(staticCS_));
#elif HAVE_PTHREAD_H
  memset(pthreadMutexNull_,0xFF,sizeof(pthread_mutex_t));
  memset(Mutex::pthreadRWLockNull_,0xFF,sizeof(pthread_rwlock_t));
#endif
  new (giantPlaceHolder) InterlockedMutex;
}
//---------------------------------------------------------------------------
void InterlockedMutex::cleanup()
{
  giant().~InterlockedMutex();
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//CRITICAL_SECTION InterlockedMutex::staticCS_;
#endif
//---------------------------------------------------------------------------
#if !FAST_MUTEX && HAVE_PTHREAD_H
uint8_t InterlockedMutex::pthreadMutexNull_[sizeof(pthread_mutex_t)];
#endif
//---------------------------------------------------------------------------
InterlockedMutex::~InterlockedMutex()
{
#if FAST_MUTEX
  assert( refCount_ == 0 );
#elif defined(__WIN32__) || defined(__WIN64__)
//  if( memcmp(&cs_,&staticCS_,sizeof(cs_)) != 0 ) DeleteCriticalSection(&cs_);
  if( sem_ != NULL ){
    CloseHandle(sem_);
    sem_ = NULL;
  }
#elif HAVE_PTHREAD_H
  if( memcmp(&mutex_,pthreadMutexNull_,sizeof(mutex_)) != 0 ){
    int r = pthread_mutex_destroy(&mutex_);
    if( r != 0 )
      newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  }
#endif
}
//---------------------------------------------------------------------------
#if FAST_MUTEX
//---------------------------------------------------------------------------
void (*InterlockedMutex::acquire_)(InterlockedMutex * mutex) = InterlockedMutex::singleAcquire;
//---------------------------------------------------------------------------
void InterlockedMutex::multiAcquire(InterlockedMutex * mutex)
{
  uintptr_t i;
  for(;;){
    i = 4096;
    do {
      if( interlockedCompareExchange(mutex->refCount_,-1,0) == 0 ) goto l1;
    } while( --i > 0 );
    ksleep1();
  }
l1:;
}
//---------------------------------------------------------------------------
void InterlockedMutex::singleAcquire(InterlockedMutex * mutex)
{
  for(;;){
    if( interlockedCompareExchange(mutex->refCount_,-1,0) == 0 ) break;
    ksleep1();
  }
}
//---------------------------------------------------------------------------
#elif defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
InterlockedMutex::InterlockedMutex()
{
//  memset(&cs_,0,sizeof(cs_));
//  if( InitializeCriticalSectionAndSpinCount(&cs_,4000 | (DWORD(1) << 31)) == 0 ){
  sem_ = CreateSemaphoreA(NULL,1,~(ULONG) 0 >> 1, NULL);
  if( sem_ == NULL ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
}
//---------------------------------------------------------------------------
void InterlockedMutex::acquire()
{
//  EnterCriticalSection(&cs_);
  DWORD r = WaitForSingleObject(sem_,INFINITE);
  if( r == WAIT_FAILED || (r != WAIT_OBJECT_0 && r != WAIT_ABANDONED) ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
}
//---------------------------------------------------------------------------
bool InterlockedMutex::tryAcquire()
{
//  return TryEnterCriticalSection(&cs_) != 0;
  DWORD r = WaitForSingleObject(sem_,0);
  if( r == WAIT_FAILED ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return r == WAIT_OBJECT_0 || r == WAIT_ABANDONED;
}
//---------------------------------------------------------------------------
void InterlockedMutex::release()
{
//  LeaveCriticalSection(&cs_);
  if( ReleaseSemaphore(sem_,1,NULL) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
}
//---------------------------------------------------------------------------
#elif HAVE_PTHREAD_H
//---------------------------------------------------------------------------
InterlockedMutex::InterlockedMutex()
{
  memcpy(&mutex_,pthreadMutexNull_,sizeof(mutex_));
  int r;
  /*pthread_mutexattr_t attr;
  if( pthread_mutexattr_init(&attr) != 0 ){
    r = errno;
    newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  }
#if HAVE_SYSCONF
  if( sysconf(_POSIX_THREAD_PRIO_INHERIT) > 0 ){
    if( pthread_mutexattr_setprotocol(&attr,PTHREAD_PRIO_INHERIT) != 0 ){
      r = errno;
      pthread_mutexattr_destroy(&attr);
      newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
    }
  }
#endif*/
  r = pthread_mutex_init(&mutex_,NULL);
  if( r != 0 ){
    memcpy(&mutex_,pthreadMutexNull_,sizeof(mutex_));
    //pthread_mutexattr_destroy(&attr);
    newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  }
}
//---------------------------------------------------------------------------
void InterlockedMutex::acquire()
{
  int r = pthread_mutex_lock(&mutex_);
  if( r != 0 )
    newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
}
//---------------------------------------------------------------------------
bool InterlockedMutex::tryAcquire()
{
  int r = pthread_mutex_trylock(&mutex_);
  if( r != 0 && r != EBUSY ) newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return r == 0;
}
//---------------------------------------------------------------------------
void InterlockedMutex::release()
{
  int r = pthread_mutex_unlock(&mutex_);
  if( r != 0 )
    newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if FAST_MUTEX || !HAVE_PTHREAD_RWLOCK_INIT
//---------------------------------------------------------------------------
void (*Mutex::rdLock_)(Mutex * mutex) = Mutex::singleRDLock;
//---------------------------------------------------------------------------
void Mutex::multiRDLock(Mutex * mutex)
{
  uint64_t st = 1;
  uintptr_t i;
  for(;;){
    i = 4096;
    do {
      mutex->acquire();
      if( mutex->value_ >= 0 ){ mutex->value_++; goto l1; }
      mutex->release();
    } while( --i > 0 );
    ksleep(st);
    if( st < 100000 ) st <<= 1;
  }
l1:
  mutex->release();
}
//---------------------------------------------------------------------------
void Mutex::singleRDLock(Mutex * mutex)
{
  uint64_t st = 1;
  for(;;){
    mutex->acquire();
    if( mutex->value_ >= 0 ){ mutex->value_++; break; }
    mutex->release();
    ksleep(st);
    if( st < 100000 ) st <<= 1;
  }
  mutex->release();
}
//---------------------------------------------------------------------------
void (*Mutex::wrLock_)(Mutex * mutex) = Mutex::singleWRLock;
//---------------------------------------------------------------------------
void Mutex::multiWRLock(Mutex * mutex)
{
  uint64_t st = 1;
  uintptr_t i;
  for(;;){
    i = 4096;
    do {
      mutex->acquire();
      if( mutex->value_ == 0 ){ mutex->value_--; goto l1; }
      mutex->release();
    } while( --i > 0 );
    ksleep(st);
    if( st < 100000 ) st <<= 1;
  }
l1:
  mutex->release();
}
//---------------------------------------------------------------------------
void Mutex::singleWRLock(Mutex * mutex)
{
  uint64_t st = 1;
  for(;;){
    mutex->acquire();
    if( mutex->value_ == 0 ){ mutex->value_--; break; }
    mutex->release();
    ksleep(st);
    if( st < 100000 ) st <<= 1;
  }
  mutex->release();
}
//---------------------------------------------------------------------------
#else
//---------------------------------------------------------------------------
uint8_t Mutex::pthreadRWLockNull_[sizeof(pthread_rwlock_t)];
//---------------------------------------------------------------------------
Mutex::~Mutex()
{
  if( memcmp(&mutex_,pthreadRWLockNull_,sizeof(mutex_)) != 0 ){
    int r = pthread_rwlock_destroy(&mutex_);
    assert( r == 0 );
    if( r != 0 ) newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  }
}
//---------------------------------------------------------------------------
Mutex::Mutex()
{
  memcpy(&mutex_,pthreadRWLockNull_,sizeof(mutex_));
  int r = pthread_rwlock_init(&mutex_,NULL);
  if( r != 0 ){
    memcpy(&mutex_,pthreadRWLockNull_,sizeof(mutex_));
    newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  }
}
//---------------------------------------------------------------------------
Mutex & Mutex::rdLock()
{
  int r = pthread_rwlock_rdlock(&mutex_);
  if( r != 0 ) newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
bool Mutex::tryRDLock()
{
  int r = pthread_rwlock_tryrdlock(&mutex_);
  if( r != 0 && r != EBUSY ) newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return r == 0;
}
//---------------------------------------------------------------------------
Mutex & Mutex::wrLock()
{
  int r = pthread_rwlock_wrlock(&mutex_);
  if( r != 0 ) newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
bool Mutex::tryWRLock()
{
  int r = pthread_rwlock_trywrlock(&mutex_);
  if( r != 0 && r != EBUSY ) newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return r == 0;
}
//---------------------------------------------------------------------------
Mutex & Mutex::unlock()
{
  int r = pthread_rwlock_unlock(&mutex_);
  if( r != 0 ) newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
Event::~Event()
{
#if HAVE_PTHREAD_H
  static const pthread_mutex_t stm = PTHREAD_MUTEX_INITIALIZER;
  if( memcpy(&mutex_,&stm,sizeof(mutex_)) != 0 ) pthread_mutex_destroy(&mutex_);
  static const pthread_cond_t stc = PTHREAD_COND_INITIALIZER;
  if( memcmp(&handle_,&stc,sizeof(handle_)) != 0 ) pthread_cond_destroy(&handle_);
#elif defined(__WIN32__) || defined(__WIN64__)
  if( handle_ != NULL ) CloseHandle(handle_);
#endif
}
//---------------------------------------------------------------------------
Event::Event()
{
  int32_t err;
#if HAVE_PTHREAD_H
  static const pthread_cond_t stc = PTHREAD_COND_INITIALIZER;
  handle_ = stc;
  static const pthread_mutex_t stm = PTHREAD_MUTEX_INITIALIZER;
  mutex_ = stm;
  if( pthread_cond_init(&handle_,NULL) != 0 || pthread_mutex_init(&mutex_,NULL) != 0 ){
    err = errno;
    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&handle_);
#elif defined(__WIN32__) || defined(__WIN64__)
  handle_ = CreateEvent(NULL,TRUE,FALSE,NULL);
  if( handle_ == NULL ){
    err = GetLastError() + errorOffset;
#endif
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
