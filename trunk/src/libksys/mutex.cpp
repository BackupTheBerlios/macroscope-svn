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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
uint8_t giantPlaceHolder[sizeof(InterlockedMutex)];
//---------------------------------------------------------------------------
#if !defined(__GNUG__) && !defined(__i386__) && !defined(_MSC_VER) && !defined(__BCPLUSPLUS__)
int32_t interlockedIncrement(int32_t & v,int32_t a)
{
  giant().acquire();
  int32_t ov = v;
  v += a;
  giant().release();
  return ov;
}

int32_t interlockedCompareExchange(int32_t & v,int32_t exValue,int32_t cmpValue)
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
#endif
//---------------------------------------------------------------------------
#ifdef __BCPLUSPLUS__
#pragma option push -w-8070 -O2 -6 -y- -r
int32_t __fastcall __declspec(nothrow) interlockedIncrement(int32_t &,int32_t)
{
  __asm {
    lock xadd       [eax],edx
    mov             eax,edx
  }
}

int32_t __fastcall __declspec(nothrow) interlockedCompareExchange(int32_t &,int32_t,int32_t)
{
  __asm {
    xchg            eax,ecx
    lock cmpxchg    [ecx],edx
  }
}
#pragma option pop
#endif
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
#endif
#if defined(__WIN32__) || defined(__WIN64__)
  memset(&staticCS_,0,sizeof(staticCS_));
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
CRITICAL_SECTION InterlockedMutex::staticCS_;
#endif
//---------------------------------------------------------------------------
InterlockedMutex::~InterlockedMutex()
{
#if FAST_MUTEX
#elif defined(__WIN32__) || defined(__WIN64__)
//  if( memcmp(&cs_,&staticCS_,sizeof(cs_)) != 0 ) DeleteCriticalSection(&cs_);
  if( sem_ != NULL ){
    CloseHandle(sem_);
    sem_ = NULL;
  }
#elif HAVE_PTHREAD_H
  if( mutex_ != NULL ){
    int r = pthread_mutex_destroy(&mutex_);
    if( r != 0 ) Exception::throwSP(r,__PRETTY_FUNCTION__);
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
    sleep1();
  }
l1:;
}
//---------------------------------------------------------------------------
void InterlockedMutex::singleAcquire(InterlockedMutex * mutex)
{
  for(;;){
    if( interlockedCompareExchange(mutex->refCount_,-1,0) == 0 ) break;
    sleep1();
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
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
}
//---------------------------------------------------------------------------
void InterlockedMutex::acquire()
{
//  EnterCriticalSection(&cs_);
  DWORD r = WaitForSingleObject(sem_,INFINITE);
  if( r == WAIT_FAILED || (r != WAIT_OBJECT_0 && r != WAIT_ABANDONED) ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
}
//---------------------------------------------------------------------------
bool InterlockedMutex::tryAcquire()
{
//  return TryEnterCriticalSection(&cs_) != 0;
  DWORD r = WaitForSingleObject(sem_,0);
  if( r == WAIT_FAILED ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return r == WAIT_OBJECT_0 || r == WAIT_ABANDONED;
}
//---------------------------------------------------------------------------
void InterlockedMutex::release()
{
//  LeaveCriticalSection(&cs_);
  if( ReleaseSemaphore(sem_,1,NULL) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
}
//---------------------------------------------------------------------------
#elif HAVE_PTHREAD_H
//---------------------------------------------------------------------------
InterlockedMutex::InterlockedMutex() : mutex_(NULL)
{
  int r = pthread_mutex_init(&mutex_,NULL);
  if( r != 0 ) newObject<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
}
//---------------------------------------------------------------------------
void InterlockedMutex::acquire()
{
  int r = pthread_mutex_lock(&mutex_);
  if( r != 0 ) newObject<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
}
//---------------------------------------------------------------------------
bool InterlockedMutex::tryAcquire()
{
  int r = pthread_mutex_trylock(&mutex_);
  if( r != 0 && r != EBUSY ) newObject<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return r == 0;
}
//---------------------------------------------------------------------------
void InterlockedMutex::release()
{
  int r = pthread_mutex_unlock(&mutex_);
  if( r != 0 ) newOBject<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
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
    sleep(st);
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
    sleep(st);
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
    sleep(st);
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
    sleep(st);
    if( st < 100000 ) st <<= 1;
  }
  mutex->release();
}
//---------------------------------------------------------------------------
#else
//---------------------------------------------------------------------------
Mutex::~Mutex()
{
  int r = pthread_rwlock_destroy(&mutex_);
  if( r != 0 ) newObject<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
}
//---------------------------------------------------------------------------
Mutex::Mutex()
{
  mutex_ = NULL;
  int r = pthread_rwlock_init(&mutex_,NULL);
  if( r != 0 ) newObject<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
}
//---------------------------------------------------------------------------
Mutex & Mutex::rdLock()
{
  int r = pthread_rwlock_rdlock(&mutex_);
  if( r != 0 ) newObject<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
bool Mutex::tryRDLock()
{
  int r = pthread_rwlock_tryrdlock(&mutex_);
  if( r != 0 && r != EBUSY ) newObject<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return r == 0;
}
//---------------------------------------------------------------------------
Mutex & Mutex::wrLock()
{
  int r = pthread_rwlock_wrlock(&mutex_);
  if( r != 0 ) newObject<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
bool Mutex::tryWRLock()
{
  int r = pthread_rwlock_trywrlock(&mutex_);
  if( r != 0 && r != EBUSY ) newObject<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  return r == 0;
}
//---------------------------------------------------------------------------
Mutex & Mutex::unlock()
{
  int r = pthread_rwlock_unlock(&mutex_);
  if( r != 0 ) newObject<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
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
  if( mutex_ != NULL ) pthread_mutex_destroy(&mutex_);
  if( handle_ != NULL ) pthread_cond_destroy(&handle_);
#elif defined(__WIN32__) || defined(__WIN64__)
  if( handle_ != NULL ) CloseHandle(handle_);
#endif
}
//---------------------------------------------------------------------------
Event::Event()
{
  int32_t err;
#if HAVE_PTHREAD_H
  handle_ = NULL;
  mutex_ = NULL;
  if( pthread_cond_init(&handle_,NULL) != 0 || pthread_mutex_init(&mutex_,NULL) != 0 ){
    err = errno;
    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&handle_);
#elif defined(__WIN32__) || defined(__WIN64__)
  handle_ = CreateEvent(NULL,TRUE,FALSE,NULL);
  if( handle_ == NULL ){
    err = GetLastError() + errorOffset;
#endif
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
