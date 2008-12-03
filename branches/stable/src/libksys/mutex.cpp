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
#include <adicpp/ksys.h>
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
uint8_t giantPlaceHolder[sizeof(WriteLock)];
//---------------------------------------------------------------------------
#if !defined(__GNUG__) && !defined(__i386__) && !defined(_MSC_VER) && !defined(__BCPLUSPLUS__)
//---------------------------------------------------------------------------
int32_t interlockedIncrement(volatile int32_t & v,int32_t a)
{
  giant().acquire();
  int32_t ov = v;
  v += a;
  giant().release();
  return ov;
}
//---------------------------------------------------------------------------
int64_t interlockedIncrement(volatile int64_t & v,int64_t a)
{
  giant().acquire();
  int64_t ov = v;
  v += a;
  giant().release();
  return ov;
}
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
#elif defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
int64_t interlockedIncrement(volatile int64_t & v,int64_t a)
{
  return InterlockedExchangeAdd64((LONGLONG *) &v,a);
}
//---------------------------------------------------------------------------
int64_t interlockedCompareExchange(volatile int64_t & v,int64_t exValue,int64_t cmpValue)
{
  return InterlockedCompareExchange64((LONGLONG *) &v,exValue,cmpValue);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !defined(__WIN32__) && !defined(__WIN64__) && __i386__ && !__x86_64__
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
void (*LiteWriteLock::acquire_)(volatile ilock_t & ref) = LiteWriteLock::singleAcquire;
//---------------------------------------------------------------------------
void LiteWriteLock::multiAcquire(volatile ilock_t & ref)
{
  uintptr_t i;
  for(;;){
    i = 4096;
    do {
      if( interlockedCompareExchange(ref,-1,0) == 0 ) goto l1;
    } while( --i > 0 );
    ksleep1();
  }
l1:;
}
//---------------------------------------------------------------------------
void LiteWriteLock::singleAcquire(volatile ilock_t & ref)
{
  for(;;){
    if( interlockedCompareExchange(ref,-1,0) == 0 ) break;
    ksleep1();
  }
}
//---------------------------------------------------------------------------
void LiteWriteLock::initialize()
{
  if( numberOfProcessors() > 1 ){
    LiteWriteLock::acquire_ = LiteWriteLock::multiAcquire;
    LiteReadWriteLock::rdLock_ = LiteReadWriteLock::multiRDLock;
    LiteReadWriteLock::wrLock_ = LiteReadWriteLock::multiWRLock;
  }
  else {
    LiteWriteLock::acquire_ = LiteWriteLock::singleAcquire;
    LiteReadWriteLock::rdLock_ = LiteReadWriteLock::singleRDLock;
    LiteReadWriteLock::wrLock_ = LiteReadWriteLock::singleWRLock;
  }
#if HAVE_PTHREAD_H
  memset(WriteLock::pthreadReadWriteLockNull_,0xFF,sizeof(pthread_mutex_t));
  memset(ReadWriteLock::pthreadRWLockNull_,0xFF,sizeof(pthread_rwlock_t));
#endif
  new (giantPlaceHolder) WriteLock;
}
//---------------------------------------------------------------------------
void LiteWriteLock::cleanup()
{
  giant().~WriteLock();
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
void (*LiteReadWriteLock::rdLock_)(LiteReadWriteLock * mutex) = LiteReadWriteLock::singleRDLock;
//---------------------------------------------------------------------------
void LiteReadWriteLock::multiRDLock(LiteReadWriteLock * mutex)
{
  uintptr_t st = 1, i;
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
void LiteReadWriteLock::singleRDLock(LiteReadWriteLock * mutex)
{
  uintptr_t st = 1;
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
void (*LiteReadWriteLock::wrLock_)(LiteReadWriteLock * mutex) = LiteReadWriteLock::singleWRLock;
//---------------------------------------------------------------------------
void LiteReadWriteLock::multiWRLock(LiteReadWriteLock * mutex)
{
  uintptr_t st = 1, i;
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
void LiteReadWriteLock::singleWRLock(LiteReadWriteLock * mutex)
{
  uintptr_t st = 1;
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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if HAVE_PTHREAD_H
uint8_t WriteLock::pthreadReadWriteLockNull_[sizeof(pthread_mutex_t)];
#endif
//---------------------------------------------------------------------------
WriteLock::~WriteLock()
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( sem_ != NULL ){
    CloseHandle(sem_);
    sem_ = NULL;
  }
#elif HAVE_PTHREAD_H
  if( memcmp(&mutex_,pthreadReadWriteLockNull_,sizeof(mutex_)) != 0 ){
    int r = pthread_mutex_destroy(&mutex_);
    if( r != 0 )
      newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  }
#endif
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
WriteLock::WriteLock()
{
  sem_ = CreateSemaphoreA(NULL,1,~(ULONG) 0 >> 1, NULL);
  if( sem_ == NULL ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
}
//---------------------------------------------------------------------------
WriteLock & WriteLock::acquire()
{
  DWORD r = WaitForSingleObject(sem_,INFINITE);
  if( r == WAIT_FAILED || (r != WAIT_OBJECT_0 && r != WAIT_ABANDONED) ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return *this;
}
//---------------------------------------------------------------------------
bool WriteLock::tryAcquire()
{
  DWORD r = WaitForSingleObject(sem_,0);
  if( r == WAIT_FAILED ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return r == WAIT_OBJECT_0 || r == WAIT_ABANDONED;
}
//---------------------------------------------------------------------------
WriteLock & WriteLock::release()
{
  if( ReleaseSemaphore(sem_,1,NULL) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return *this;
}
//---------------------------------------------------------------------------
#elif HAVE_PTHREAD_H
//---------------------------------------------------------------------------
WriteLock::WriteLock()
{
  memcpy(&mutex_,pthreadReadWriteLockNull_,sizeof(mutex_));
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
    memcpy(&mutex_,pthreadReadWriteLockNull_,sizeof(mutex_));
    //pthread_mutexattr_destroy(&attr);
    newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  }
}
//---------------------------------------------------------------------------
WriteLock & WriteLock::acquire()
{
  int r = pthread_mutex_lock(&mutex_);
  if( r != 0 )
    newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
bool WriteLock::tryAcquire()
{
  int r = pthread_mutex_trylock(&mutex_);
  if( r != 0 && r != EBUSY ) newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return r == 0;
}
//---------------------------------------------------------------------------
WriteLock & WriteLock::release()
{
  int r = pthread_mutex_unlock(&mutex_);
  if( r != 0 )
    newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if HAVE_PTHREAD_H
//---------------------------------------------------------------------------
uint8_t ReadWriteLock::pthreadRWLockNull_[sizeof(pthread_rwlock_t)];
//---------------------------------------------------------------------------
ReadWriteLock::~ReadWriteLock()
{
  if( memcmp(&mutex_,pthreadRWLockNull_,sizeof(mutex_)) != 0 ){
    int r = pthread_rwlock_destroy(&mutex_);
    assert( r == 0 );
    if( r != 0 ) newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  }
}
//---------------------------------------------------------------------------
ReadWriteLock::ReadWriteLock()
{
  memcpy(&mutex_,pthreadRWLockNull_,sizeof(mutex_));
  int r = pthread_rwlock_init(&mutex_,NULL);
  if( r != 0 ){
    memcpy(&mutex_,pthreadRWLockNull_,sizeof(mutex_));
    newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  }
}
//---------------------------------------------------------------------------
ReadWriteLock & ReadWriteLock::rdLock()
{
  int r = pthread_rwlock_rdlock(&mutex_);
  if( r != 0 ) newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
bool ReadWriteLock::tryRDLock()
{
  int r = pthread_rwlock_tryrdlock(&mutex_);
  if( r != 0 && r != EBUSY ) newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return r == 0;
}
//---------------------------------------------------------------------------
ReadWriteLock & ReadWriteLock::wrLock()
{
  int r = pthread_rwlock_wrlock(&mutex_);
  if( r != 0 ) newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
bool ReadWriteLock::tryWRLock()
{
  int r = pthread_rwlock_trywrlock(&mutex_);
  if( r != 0 && r != EBUSY ) newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return r == 0;
}
//---------------------------------------------------------------------------
ReadWriteLock & ReadWriteLock::unlock()
{
  int r = pthread_rwlock_unlock(&mutex_);
  if( r != 0 ) newObjectV1C2<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
