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
uint8_t currentThreadPlaceHolder[sizeof(ThreadLocalVariable<Thread>)];
//------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
DWORD WINAPI Thread::threadFunc(LPVOID thread)
#else
void * Thread::threadFunc(void * thread)
#endif
{
#if HAVE_PTHREAD_H
  if( pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL) != 0 ){
    perror(NULL);
    abort();
  }
  if( pthread_mutex_lock(&reinterpret_cast<Thread *>(thread)->mutex_) != 0 ){
    perror(NULL);
    abort();
  }
  if( pthread_mutex_unlock(&reinterpret_cast<Thread *>(thread)->mutex_) != 0 ){
    perror(NULL);
    abort();
  }
  if( pthread_mutex_destroy(&reinterpret_cast<Thread *>(thread)->mutex_) != 0 ){
    perror(NULL);
    abort();
  }
  reinterpret_cast<Thread *>(thread)->mutex_ = NULL;
#endif
  intptr_t i;
  try {
    for( i = beforeExecuteActions().count() - 1; i >= 0; i-- )
      ((void (*)(void *)) beforeExecuteActions()[i].handler())(beforeExecuteActions()[i].data());
  }
  catch( ... ){
  }
#if defined(__WIN32__) || defined(__WIN64__)
  reinterpret_cast<Thread *>(thread)->exitCode_ = STILL_ACTIVE;
#endif
  reinterpret_cast<Thread *>(thread)->started_ = true;
  try {
    currentThread() = reinterpret_cast<Thread *>(thread);
    reinterpret_cast<Thread *>(thread)->threadExecute();
#if defined(__WIN32__) || defined(__WIN64__)
    if( reinterpret_cast<Thread *>(thread)->exitCode_ == (int32_t) STILL_ACTIVE )
      reinterpret_cast<Thread *>(thread)->exitCode_ = 0;
#endif
  }
  catch( ksys::ExceptionSP & e ){
    e->writeStdError();
    reinterpret_cast<Thread *>(thread)->exitCode_ = e->codes()[0];
  }
  catch( ... ){
  }
  try {
    for( i = afterExecuteActions().count() - 1; i >= 0; i-- )
      ((void (*)(void *)) afterExecuteActions()[i].handler())(afterExecuteActions()[i].data());
  }
  catch( ... ){
  }
  reinterpret_cast<Thread *>(thread)->finished_ = true;
  currentThread() = NULL;
#if defined(__WIN32__) || defined(__WIN64__)
//  while( PostThreadMessage(mainThreadId,threadFinishMessage,0,0) == 0 ) Sleep(1);
  return (DWORD) reinterpret_cast<Thread *>(thread)->exitCode_;
#elif HAVE_PTHREAD_H
  return (void *) (intptr_t) reinterpret_cast<Thread *>(thread)->exitCode_;
#endif
}
//---------------------------------------------------------------------------
Thread::Thread() :
  stackSize_(PTHREAD_STACK_MIN),
#if defined(__WIN32__) || defined(__WIN64__)
  handle_(NULL),
  id_(~DWORD(0)),
#elif HAVE_PTHREAD_H
  handle_(NULL), mutex_(NULL),
#endif
  exitCode_(0),
  started_(false), terminated_(false), finished_(false)
{
  stackSize((uintptr_t) getpagesize());
}
//---------------------------------------------------------------------------
Thread & Thread::resume()
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( handle_ == NULL ){
    started_ = terminated_ = finished_ = false;
    handle_ = CreateThread(NULL,stackSize_,threadFunc,this,CREATE_SUSPENDED,&id_);
    if( handle_ == NULL ){
      int32_t err = GetLastError() + errorOffset;
      Exception::throwSP(err,__PRETTY_FUNCTION__);
    }
#elif HAVE_PTHREAD_H
  pthread_attr_t attr = NULL;
  if( handle_ == NULL ){
    started_ = terminated_ = finished_ = false;
    if( pthread_mutex_init(&mutex_,NULL) != 0 ) goto l1;
    if( pthread_attr_init(&attr) != 0 ) goto l1;
    if( pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE) != 0 ) goto l1;
    if( pthread_attr_setstacksize(&attr,stackSize_) != 0 ) goto l1;
#if HAVE_PTHREAD_ATTR_SETGUARDSIZE
    if( pthread_attr_setguardsize(&attr,0) != 0 ) goto l1;
#endif
    if( pthread_mutex_lock(&mutex_) != 0 ) goto l1;
    if( pthread_create(&handle_,&attr,threadFunc,this) != 0 ){
      pthread_mutex_unlock(&mutex_);
      goto l1;
    }
#endif
  }
#if defined(__WIN32__) || defined(__WIN64__)
  ResumeThread(handle_);
  return *this;
#elif HAVE_PTHREAD_H
  if( pthread_mutex_unlock(&mutex_) != 0 ){
    perror(NULL);
    abort();
  }
  return *this;
l1:
  int32_t err = errno;
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&mutex_);
  mutex_ = NULL;
  Exception::throwSP(err,__PRETTY_FUNCTION__);
#endif
}
//---------------------------------------------------------------------------
Thread & Thread::wait()
{
  if( handle_ != NULL ){
#if defined(__WIN32__) || defined(__WIN64__)
    DWORD exitCode;
    BOOL r = GetExitCodeThread(handle_,&exitCode);
    if( r == 0 || exitCode == STILL_ACTIVE ){
      WaitForSingleObject(handle_,INFINITE);
      GetExitCodeThread(handle_,&exitCode);
    }
    CloseHandle(handle_);
    handle_ = NULL;
#elif HAVE_PTHREAD_H
    if( pthread_join(handle_,NULL) != 0 ){
      perror(NULL);
      abort();
    }
    if( mutex_ != NULL ){
      if( pthread_mutex_destroy(&mutex_) != 0 ){
        perror(NULL);
	abort();
      }
      mutex_ = NULL;
    }
    handle_ = NULL;
#endif
  }
  return *this;
}
//---------------------------------------------------------------------------
Thread & Thread::priority(uintptr_t pri)
{
  if( started_ && !finished_ ){
#if HAVE_PTHREAD_SETSCHEDPARAM
    int policy;
    struct sched_param param;
    if( pthread_getschedparam(handle_,&policy,&param) != 0 ){
l1:   int32_t err = errno;
      Exception::throwSP(err,__PRETTY_FUNCTION__);
    }
    param.sched_priority = pri;
    if( pthread_setschedparam(handle_,policy,&param) != 0 ) goto l1;
#elif defined(__WIN32__) || defined(__WIN64__)
    if( SetThreadPriority(handle_,(int) pri) == 0 ){
      int32_t err = GetLastError() + errorOffset;
      Exception::throwSP(err,__PRETTY_FUNCTION__);
    }
#endif
  }
  return *this;
}
//---------------------------------------------------------------------------
uintptr_t Thread::priority() const
{
  uintptr_t pri = 0;
  if( started_ && !finished_ ){
#if HAVE_PTHREAD_GETSCHEDPARAM
    int policy;
    struct sched_param param;
    if( pthread_getschedparam(handle_,&policy,&param) != 0 ){
      int32_t err = errno;
      Exception::throwSP(err,__PRETTY_FUNCTION__);
    }
    pri = param.sched_priority;
#elif defined(__WIN32__) || defined(__WIN64__)
    pri = GetThreadPriority(handle_);
    if( pri == THREAD_PRIORITY_ERROR_RETURN ){
      int32_t err = GetLastError() + errorOffset;
      Exception::throwSP(err,__PRETTY_FUNCTION__);
    }
#endif
  }
  return pri;
}
//---------------------------------------------------------------------------
uint8_t Thread::beforeExecuteActions_[sizeof(Array<Action>)];
uint8_t Thread::afterExecuteActions_[sizeof(Array<Action>)];
//---------------------------------------------------------------------------
void Thread::initialize()
{
  new (beforeExecuteActions_) Array<Action>;
  new (afterExecuteActions_) Array<Action>;
  new (currentThreadPlaceHolder) ThreadLocalVariable<Thread>;
}
//---------------------------------------------------------------------------
void Thread::cleanup()
{
  currentThread().~ThreadLocalVariable<Thread>();
  afterExecuteActions().~Array<Action>();
  beforeExecuteActions().~Array<Action>();
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
