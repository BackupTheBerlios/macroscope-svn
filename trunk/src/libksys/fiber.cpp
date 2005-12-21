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
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
uint8_t currentFiberPlaceHolder[sizeof(ThreadLocalVariable<Fiber>)];
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void Fiber::initialize()
{
  new (currentFiberPlaceHolder) ThreadLocalVariable<Fiber>;
}
//------------------------------------------------------------------------------
void Fiber::cleanup()
{
  currentFiber().~ThreadLocalVariable<Fiber>();
}
//------------------------------------------------------------------------------
Fiber & Fiber::allocateStack(
#if !defined(__WIN32__) && !defined(__WIN64__)
  void * ip,void * param,size_t size,Fiber * mainFiber,uintptr_t dummy1,uintptr_t dummy2)
#else
  void * ip,void * param,size_t,Fiber * mainFiber,uintptr_t,uintptr_t)
#endif
{
#if !defined(__WIN32__) && !defined(__WIN64__)
  stack_.realloc(size);
  stackPointer_ = stack_.ptr();
  if( &dummy1 < &dummy2 ){
    stackPointer_ = stack_.ptr() + size;
// push parameters and return address for 'void Fiber::start'
    ((void **) stackPointer_)[-1] = ip;
    ((void **) stackPointer_)[-2] = param;
    ((void **) stackPointer_)[-3] = this;
    ((void **) stackPointer_)[-4] = NULL;
// push parameters and return address for 'Fiber & Fiber::switchFiber(Fiber *)
    ((void **) stackPointer_)[-5] = this;
    ((void **) stackPointer_)[-6] = mainFiber;
    ((void **) stackPointer_)[-7] = (void *) start;
 // for epilogue (push ebp)    
    ((void **) stackPointer_)[-8] = NULL;
// push dummy registers for switchFiber2, see below
    ((void **) stackPointer_)[-9] = NULL;
    ((void **) stackPointer_)[-10] = NULL;
    ((void **) stackPointer_)[-11] = NULL;
    ((void **) stackPointer_)[-12] = (void **) stackPointer_ - 8;
    stackPointer_ = (void **) stackPointer_ - 12;
  }
  else {
    stackPointer_ = stack_.ptr();
// push parameters and return address for 'void Fiber::start'
    ((void **) stackPointer_)[0] = ip;
    ((void **) stackPointer_)[1] = param;
    ((void **) stackPointer_)[2] = this;
    ((void **) stackPointer_)[3] = NULL;
// push parameters and return address for 'void Fiber::switchFiber(Fiber *)
    ((void **) stackPointer_)[4] = this;
    ((void **) stackPointer_)[5] = mainFiber;
    ((void **) stackPointer_)[6] = (void *) start;
    ((void **) stackPointer_)[7] = NULL; // for epilogue (push ebp)
// push dummy registers for switchFiber2, see below
    ((void **) stackPointer_)[8] = NULL;
    ((void **) stackPointer_)[9] = NULL;
    ((void **) stackPointer_)[10] = NULL;
    stackPointer_ = (void **) stackPointer_ + 10;
  }
  started_ = terminated_ = finished_ = false;
#else
  ip_ = ip;
  param_ = param;
#endif
  mainFiber_ = mainFiber;
  return *this;
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
#elif __i386__ && __GNUG__
void Fiber::switchFiber2(Fiber * fiber)
{
  asm volatile (
    "push  %%ebx\n"
    "push  %%esi\n"
    "push  %%edi\n"
    "push  %%ebp\n"
    "mov   %%esp,(%%eax)\n"
    "mov   (%%edx),%%esp\n"
    "pop   %%ebp\n"
    "pop   %%edi\n"
    "pop   %%esi\n"
    "pop   %%ebx\n"
    "leave\n"
    "pop   %%eax\n"
    "pop   %%edx\n"
    "pop   %%edx\n"
    "jmp   *%%eax\n"
    :
    : "d" (&fiber->stackPointer_), // edx
      "a" (&stackPointer_)  // eax
  );
}
#elif __GNUG__ && __x86_64__
void Fiber::switchFiber2(Fiber * fiber)
{
  asm volatile (
    "push  %%rbx\n"
    "push  %%rsi\n"
    "push  %%rdi\n"
    "push  %%rbp\n"
    "mov   %%rsp,(%%rax)\n"
    "mov   (%%rdx),%%rsp\n"
    "pop   %%rbp\n"
    "pop   %%rdi\n"
    "pop   %%rsi\n"
    "pop   %%rbx\n"
    "leave\n"
    "pop   %%rax\n"
    "pop   %%rdx\n"
    "pop   %%rdx\n"
    "jmp   %%rax\n"
    :
    : "d" (&fiber->stackPointer_), // rdx
      "a" (&stackPointer_)  // rax
  );
}
#else
#error fibers for this system not implemented
#endif
//------------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
VOID WINAPI Fiber::start(Fiber * fiber)
#else
void Fiber::start(Fiber * fiber,void * param,void (* ip)(void *))
#endif
{
  fiber->started_ = true;
  try {
#if !defined(__WIN32__) && !defined(__WIN64__)
    ip(param);
#else
    (*(void (*)(void *)) fiber->ip_)(fiber->param_);
#endif
  }
  catch( ksys::ExceptionSP & e ){
    e->writeStdError();
  }
  catch( ... ){
  }
  fiber->finished_ = true;
  fiber->switchFiber(fiber->mainFiber_);
}
//------------------------------------------------------------------------------
Fiber & Fiber::attach(AsyncDescriptor & descriptor)
{
  assert( descriptor.fiber_ == NULL );
  descriptor.fiber_ = this;
  return *this;
}
//------------------------------------------------------------------------------
Fiber & Fiber::detach(AsyncDescriptor & descriptor)
{
  assert( descriptor.fiber_ == this );
  descriptor.fiber_ = NULL;
  return *this;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void BaseThread::execute()
{
#if defined(__WIN32__) || defined(__WIN64__)
  convertThreadToFiber();
  try {
#endif
    currentFiber() = this;
    while( !Thread::terminated_ ) queue();
#if defined(__WIN32__) || defined(__WIN64__)
  }
  catch( ... ){
    clearFiber();
    throw;
  }
  clearFiber();
#endif
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
BaseFiber & BaseFiber::dead()
{
  thread()->mutex_.acquire();
  thread()->deadFibers_.insToTail(*this);
  thread()->mutex_.release();
  return *this;
}
//------------------------------------------------------------------------------
void BaseFiber::fiber2(BaseFiber * fiber)
{
  try {
    fiber->execute();
  }
  catch( ... ){
    fiber->dead();
    throw;
  }
  fiber->dead();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
BaseThread * BaseServer::newThread()
{
  return new BaseThread;
}
//------------------------------------------------------------------------------
BaseThread * BaseServer::selectThread()
{
  AutoLock<InterlockedMutex> lock(mutex_);
  BaseThread * thread = NULL;
  uintptr_t msc = ~uintptr_t(0);
  if( threads_.count() >= numberOfProcessors() ){
    for( intptr_t i = threads_.count() - 1; i >= 0; i-- ){
      BaseThread * athread = threads_.objectOfIndex(i);
      if( athread->fibers_.count() < msc ){
        msc = athread->fibers_.count();
        thread = athread;
      }
    }
  }
  if( msc >= mfpt_ || thread == NULL ){
    if( threads_.count() < mt_ ){
      thread = newThread();
      ksys::AutoPtr<BaseThread> safe(thread);
      threads_.add(thread,thread);
      safe.ptr(NULL);
      thread->server(this);
      thread->resume();
    }
  }
  assert( thread != NULL );
  return thread;
}
//------------------------------------------------------------------------------
void BaseServer::sweepFibers()
{
  AutoLock<InterlockedMutex> lock(mutex_);
  for( intptr_t i = threads_.count() - 1; i >= 0; i-- ){
    BaseThread * thread = threads_.objectOfIndex(i);
    while( thread->deadFibers_.count() > 0 ){
      thread->mutex_.acquire();
      BaseFiber * fiber = thread->deadFibers_.first()->object();
      thread->deadFibers_.remove(*fiber);
      thread->fibers_.removeByObject(fiber);
      thread->mutex_.release();
      if( thread->fibers_.count() == 0 ){
        thread->Thread::terminate();
        thread->semaphore_.post();
        thread->wait();
        threads_.removeByObject(thread);
      }
    }
  }
}
//------------------------------------------------------------------------------
void BaseServer::closeServer()
{
  assert( currentFiber() == NULL );
  intptr_t i, k;
  BaseThread * thread;
  mutex_.acquire();
  for( i = threads_.count() - 1; i >= 0; i-- ){
    thread = threads_.objectOfIndex(i);
    thread->mutex_.acquire();
    for( k = thread->fibers_.count() - 1; k >= 0; k-- )
      thread->fibers_.objectOfIndex(k)->terminate();
    thread->mutex_.release();
  }
  mutex_.release();
  for(;;){
    int64_t fbtm;
    for( fbtm = fiberTimeout_; fbtm > 0; fbtm -= 100000 ){
      mutex_.acquire();
      for( i = threads_.count() - 1; i >= 0; i-- ){
        thread = threads_.objectOfIndex(i);
        thread->mutex_.acquire();
        for( k = thread->fibers_.count() - 1; k >= 0; k-- )
          if( !thread->fibers_.objectOfIndex(k)->finished() ) break;
        thread->mutex_.release();
        if( k >= 0 ) break;
      }
      mutex_.release();
      if( i < 0 ) break;
      sweepFibers();
      ksys::sleep(fbtm > 100000 ? 100000 : fbtm);
    }
    if( fbtm > 0 ) break;
    mutex_.acquire();
    for( i = threads_.count() - 1; i >= 0; i-- ){
      thread = threads_.objectOfIndex(i);
      thread->mutex_.acquire();
      for( k = thread->descriptors_.count() - 1; k >= 0; k-- )
        thread->descriptors_.objectOfIndex(k)->shutdown2();
      thread->mutex_.release();
    }
    mutex_.release();
  }
  sweepFibers();
  assert( threads_.count() == 0 );
/*  for( i = threads_.count() - 1; i >= 0; i-- ){
    thread = threads_.objectOfIndex(i);
    thread->Thread::terminate();
    thread->semaphore_.post();
    thread->wait();
    threads_.removeByIndex(i);
  }*/
}
//------------------------------------------------------------------------------
void BaseServer::attachFiber(BaseFiber & fiber)
{
  BaseThread * thread = selectThread();
  fiber.allocateStack((void *) fiber.fiber2,&fiber,fiberStackSize_,thread);
#if defined(__WIN32__) || defined(__WIN64__)
  fiber.createFiber(fiberStackSize_);
#endif
  AutoLock<InterlockedMutex> cover(thread->mutex_);
  thread->fibers_.add(&fiber,&fiber);
  fiber.thread(thread);
  thread->postEvent(etDispatch,&fiber);
}
//------------------------------------------------------------------------------
bool BaseServer::active() const
{
  AutoLock<InterlockedMutex> lock(mutex_);
  intptr_t i, j = -1;
  for( i = threads_.count() - 1; i >= 0; i-- ){
    BaseThread * thread = threads_.objectOfIndex(i);
    if( thread->active() ){
      thread->mutex_.acquire();
      for( j = thread->fibers_.count() - 1; j >= 0; j-- )
        if( !thread->fibers_.objectOfIndex(j)->finished() ) break;
      thread->mutex_.release();
    }
    if( j >= 0 ) break;
  }
  return j >= 0;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void FiberInterlockedMutex::acquire()
{
  Fiber * fib = currentFiber();
  if( fib == NULL ){
    main_.acquire();
    stdErr.log(lmNOTIFY,"FiberInterlockedMutex::acquire() called from non fiber enviroment\n");
  }
  else if( !tryAcquire() ){
    BaseFiber * fiber =
#ifndef NDEBUG
      dynamic_cast
#else
      static_cast
#endif
        <BaseFiber *>(fib);
    assert( fiber != NULL && fiber->ipc_.prev() == NULL && fiber->ipc_.next() == NULL );
    mutex_.acquire();
    wait_.insToTail(*fiber);
    bool a = main_.tryAcquire();
    mutex_.release();
    if( a ){
      wait_.remove(*fiber);
    }
    else {
      assert( fiber != &fiber->mainFiber() );
      fiber->switchFiber(fiber->mainFiber());
    }
  }
}
//---------------------------------------------------------------------------
void FiberInterlockedMutex::release()
{
  if( currentFiber() == NULL )
    stdErr.log(lmNOTIFY,"FiberInterlockedMutex::release() called from non fiber enviroment\n");
  BaseFiber * fiber;
  mutex_.acquire();
  if( wait_.count() > 0 ){
    fiber = wait_.first()->object();
    wait_.remove(*wait_.first()->object());
  }
  else {
    fiber = NULL;
  }
  mutex_.release();
  if( fiber != NULL ) fiber->thread()->postEvent(etDispatch,fiber);
  main_.release();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
FiberMutex & FiberMutex::rdLock()
{
  Fiber * fib = currentFiber();
  if( fib == NULL ){
    main_.rdLock();
    stdErr.log(lmNOTIFY,"FiberMutex::rdLock() called from non fiber enviroment\n");
  }
  else if( !main_.tryRDLock() ){
    BaseFiber * fiber =
#ifndef NDEBUG
      dynamic_cast
#else
      static_cast
#endif
        <BaseFiber *>(fib);
    assert( fiber != NULL && fiber->ipc_.prev() == NULL && fiber->ipc_.next() == NULL );
    mutex_.acquire();
    wait_.insToTail(*fiber);
    bool a = main_.tryRDLock();
    mutex_.release();
    if( a ){
      wait_.remove(*fiber);
    }
    else {
      assert( fiber != &fiber->mainFiber() );
      fiber->switchFiber(fiber->mainFiber());
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
FiberMutex & FiberMutex::wrLock()
{
  Fiber * fib = currentFiber();
  if( fib == NULL ){
    main_.wrLock();
    stdErr.log(lmNOTIFY,"FiberMutex::rdLock() called from non fiber enviroment\n");
  }
  else if( !main_.tryWRLock() ){
    BaseFiber * fiber =
#ifndef NDEBUG
      dynamic_cast
#else
      static_cast
#endif
        <BaseFiber *>(fib);
    assert( fiber != NULL && fiber->ipc_.prev() == NULL && fiber->ipc_.next() == NULL );
    mutex_.acquire();
    wait_.insToTail(*fiber);
    bool a = main_.tryWRLock();
    mutex_.release();
    if( a ){
      wait_.remove(*fiber);
    }
    else {
      assert( fiber != &fiber->mainFiber() );
      fiber->switchFiber(fiber->mainFiber());
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
FiberMutex & FiberMutex::unlock()
{
  if( currentFiber() == NULL )
    stdErr.log(lmNOTIFY,"FiberMutex::unlock() called from non fiber enviroment\n");
  BaseFiber * fiber;
  mutex_.acquire();
  if( wait_.count() > 0 ){
    fiber = wait_.first()->object();
    wait_.remove(*wait_.first()->object());
  }
  else {
    fiber = NULL;
  }
  mutex_.release();
  if( fiber != NULL ) fiber->thread()->postEvent(etDispatch,fiber);
  main_.unlock();
  return *this;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void FiberSemaphore::wait()
{
  Fiber * fib = currentFiber();
  if( fib == NULL ){
    semaphore_.wait();
    stdErr.log(lmNOTIFY,"FiberSemaphore::acquire() called from non fiber enviroment\n");
  }
  else if( !tryWait() ){
    BaseFiber * fiber =
#ifndef NDEBUG
      dynamic_cast
#else
      static_cast
#endif
        <BaseFiber *>(fib);
    assert( fiber != NULL && fiber->ipc_.prev() == NULL && fiber->ipc_.next() == NULL );
    mutex_.acquire();
    wait_.insToTail(*fiber);
    bool a = semaphore_.tryWait();
    mutex_.release();
    if( a ){
      wait_.remove(*fiber);
    }
    else {
      assert( fiber != &fiber->mainFiber() );
      fiber->switchFiber(fiber->mainFiber());
    }
  }
}
//---------------------------------------------------------------------------
void FiberSemaphore::post()
{
  if( currentFiber() == NULL )
    stdErr.log(lmNOTIFY,"FiberSemaphore::release() called from non fiber enviroment\n");
  BaseFiber * fiber;
  mutex_.acquire();
  if( wait_.count() > 0 ){
    fiber = wait_.first()->object();
    wait_.remove(*wait_.first()->object());
  }
  else {
    fiber = NULL;
  }
  mutex_.release();
  if( fiber != NULL ) fiber->thread()->postEvent(etDispatch,fiber);
  semaphore_.post();
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
