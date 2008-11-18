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
//------------------------------------------------------------------------------
#include <adicpp/ksys.h>
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
uint8_t Fiber::currentFiberPlaceHolder[sizeof(ThreadLocalVariable<Fiber *>)];
//------------------------------------------------------------------------------
void Fiber::initialize()
{
  new (currentFiberPlaceHolder) ThreadLocalVariable<Fiber *>;
}
//------------------------------------------------------------------------------
void Fiber::cleanup()
{
  reinterpret_cast<ThreadLocalVariable<Fiber *> *>(currentFiberPlaceHolder)->~ThreadLocalVariable<Fiber *>();
}
//------------------------------------------------------------------------------
Fiber::~Fiber()
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( fiber_ != NULL ) DeleteFiber(fiber_);
#endif
#if HAVE_UCONTEXT_H
  kfree(context_.uc_stack.ss_sp);
#endif
}
//---------------------------------------------------------------------------
Fiber::Fiber() :
  started_(false), terminated_(false), finished_(false), destroy_(true),
#if defined(__WIN32__) || defined(__WIN64__)
  fiber_(NULL),
#elif HAVE_UCONTEXT_H
#else
  stackPointer_(NULL),
#endif
  thread_(NULL)
{
  event_.fiber_ = this;
#if HAVE_UCONTEXT_H
  context_.uc_stack.ss_sp = NULL;
  context_.uc_mcontext.mc_len = 0;
  context_.uc_flags = 0;
#endif
}
//---------------------------------------------------------------------------
Fiber & Fiber::allocateStack(
#if !defined(__WIN32__) && !defined(__WIN64__)
  size_t size,Fiber * mainFiber,void *,void *,void *,uintptr_t dummy1,uintptr_t dummy2)
#else
  size_t size,Fiber *,void *,void *,void *,uintptr_t,uintptr_t)
#endif
{
#if defined(__WIN32__) || defined(__WIN64__)
  createFiber(size);
#elif HAVE_UCONTEXT_H
/*!
 * \todo rewrite fiber context switching on ucontext functions
 */
  size += -intptr_t(size) & (MINSIGSTKSZ - 1);
  context_.uc_mcontext = mainFiber->context_.uc_mcontext;
  context_.uc_stack.ss_sp = (char *) krealloc(context_.uc_stack.ss_sp,size);
  context_.uc_stack.ss_size = size;
  //context_.uc_mcontext.mc_len = sizeof(mcontext_t);
  context_.uc_flags = 0;
  context_.uc_link = NULL;
  makecontext(&context_,(void (*)()) start,1,this);
#else
  if( &dummy1 < &dummy2 ){
    stackPointer_ = stack_.ptr() + size;
#if __GNUG__ && __x86_64__
// push return address for 'void Fiber::switchFiber2
    ((void **) stackPointer_)[-1] = (void *) start;
 // for epilogue (push ebp)
    ((void **) stackPointer_)[-2] = NULL;
    stackPointer_ = (void **) stackPointer_ - 2;
#else
// push parameters and return address for 'void Fiber::start'
    ((void **) stackPointer_)[-1] = this;
    ((void **) stackPointer_)[-2] = NULL;
// push dummy parameters and return address for 'void Fiber::switchFiber2
    ((void **) stackPointer_)[-3] = NULL;
    ((void **) stackPointer_)[-4] = NULL;
    ((void **) stackPointer_)[-5] = NULL;
    ((void **) stackPointer_)[-6] = (void *) start;
 // for epilogue (push ebp)
    ((void **) stackPointer_)[-7] = NULL;
    stackPointer_ = (void **) stackPointer_ - 7;
#endif
  }
  else {
    stackPointer_ = stack_.ptr();
// push parameters and return address for 'void Fiber::start'
    ((void **) stackPointer_)[0] = this;
    ((void **) stackPointer_)[1] = NULL;
// push dummy parameters and return address for 'void Fiber::switchFiber2(Fiber *)
    ((void **) stackPointer_)[2] = (void *) start;
 // for epilogue (push ebp)
    ((void **) stackPointer_)[5] = NULL;
    stackPointer_ = (void **) stackPointer_ - 5;
  }
  started_ = terminated_ = finished_ = false;
#endif
  return *this;
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
#elif HAVE_UCONTEXT_H
#elif __i386__ && __GNUG__
/*!
 * \todo rewrite fiber context switching on ucontext functions
 */
void Fiber::switchFiber2(void ** currentFiberSP,void ** switchToFiberSP,Fiber *)
{
  asm volatile (
    "mov   %%esp,(%%eax)\n"
    "mov   (%%edx),%%esp\n"
    "mov   %%esp,%%ebp\n"
    :
    : "d" (switchToFiberSP), // edx
      "a" (currentFiberSP)  // eax
  );
}
#elif __GNUG__ && __x86_64__
/*!
 * \todo rewrite fiber context switching on ucontext functions
 */
/*
  Function calling conventions for x86_64 architecture:
  param1 --> RDI
  param2 --> RSI
  param3 --> RDX
  param4 --> RCX
  param5 --> R8
  param6 --> R9
  param7 and more placed on stack
  return value returned in RAX or in RAX:RDX pair or
  if sizeof(return value) > sizeof(RAX:RDX) then
  param1 --> RSI
  param2 --> RDX
  param3 --> RCX
  param4 --> R8
  param5 --> R9
  param6 and more placed on stack
  param1 --> RDI pointer to place for return value
 */
void Fiber::switchFiber2(void ** currentFiberSP,void ** switchToFiberSP,Fiber * fiber)
{
  asm volatile (
    ".intel_syntax\n"
    "mov   [%%rdi],%%rsp\n"
    "mov   %%rsp,[%%rsi]\n"
    "mov   %%rbp,%%rsp\n"
    "mov   %%rdi,%%rdx\n"
    ".att_syntax\n"
    :
    :
  );
}
#else
void Fiber::switchFiber2(Fiber * fiber)
{
  error = ENOSYS;
  perror(NULL);
  exit(ENOSYS);
}
#endif
//------------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
VOID WINAPI Fiber::start(Fiber * fiber)
#else
void Fiber::start(Fiber * fiber)
#endif
{
  *reinterpret_cast<ThreadLocalVariable<Fiber *> *>(currentFiberPlaceHolder) = fiber;
  fiber->started_ = true;
  fiber->terminated_ = false;
  interlockedIncrement(fiber->thread_->server_->fibersCount_,1);
  try {
    fiber->fiberExecute();
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
  }
  interlockedIncrement(fiber->thread_->server_->fibersCount_,uilock_t(ilock_t(-1)));
  fiber->finished_ = true;
  fiber->switchFiber(fiber->thread_);
  exit(ENOSYS);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
BaseThread::~BaseThread()
{
  assert( fibers_.count() == 0 );
}
//---------------------------------------------------------------------------
BaseThread::BaseThread() : server_(NULL)/*, maxStackSize_(0) */, mfpt_(~uintptr_t(0))
{
}
//---------------------------------------------------------------------------
void BaseThread::threadExecute()
{
  interlockedIncrement(server_->threadsCount_,1);
#if defined(__WIN32__) || defined(__WIN64__)
  convertThreadToFiber();
  try {
#endif
#if HAVE_UCONTEXT_H
    getcontext(&context_);
    context_.uc_mcontext.mc_len = sizeof(mcontext_t);
#endif
    while( !Thread::terminated_ ) queue();
#if defined(__WIN32__) || defined(__WIN64__)
  }
  catch( ExceptionSP & ){
    interlockedIncrement(server_->threadsCount_,-1);
    clearFiber();
    throw;
  }
  interlockedIncrement(server_->threadsCount_,-1);
  clearFiber();
#endif
}
//------------------------------------------------------------------------------
void BaseThread::queue()
{
  AsyncEvent * ev;
  mutex_.acquire();
  if( events_.count() > 0 ) ev = &events_.remove(*events_.first()); else ev = NULL;
  mutex_.release();
  if( ev == NULL ){
    semaphore_.wait();
  }
  else {
    assert( !ev->fiber_->finished() );
    Fiber::switchFiber(ev->fiber_);
    if( ev->fiber_->finished() ){
      //detectMaxFiberStackSize();
      currentFiber(NULL);
      sweepFiber(ev->fiber_);
    }
  }
}
//------------------------------------------------------------------------------
void BaseThread::sweepFiber(Fiber * fiber)
{
  {
    AutoLock<LiteWriteLock> lock(fibersReadWriteLock_);
    fibers_.remove(*fiber);
  }
  if( fiber->destroy_ ) deleteObject(fiber);
/*#if defined(__WIN32__) || defined(__WIN64__)
  if( (server_->howCloseServer_ & server_->csDWM) != 0 && fibers_.count() == 0 )
    while( PostThreadMessage(mainThreadId,fiberFinishMessage,NULL,NULL) == 0 ) Sleep(1);
#endif*/
}
//------------------------------------------------------------------------------
void BaseThread::detectMaxFiberStackSize()
{
}
//------------------------------------------------------------------------------
void BaseThread::postEvent(AsyncEvent * event)
{
  AutoLock<LiteWriteLock> lock(mutex_);
  bool postSem = events_.count() == 0;
  events_.insToTail(*event);
  if( postSem ) semaphore_.post();
}
//---------------------------------------------------------------------------
void BaseThread::threadBeforeWait()
{
  Thread::terminate();
  semaphore_.post().post();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
BaseServer::~BaseServer()
{
  assert( !active() );
}
//------------------------------------------------------------------------------
BaseServer::BaseServer() :
  shutdown_(false),
  mt_(numberOfProcessors() * 4),
  fiberStackSize_(PTHREAD_STACK_MIN),
  threadsCount_(0),
  fibersCount_(0),
  fiberTimeout_(10000000),
  howCloseServer_(HowCloseServer(csWait | csTerminate | csShutdown | csAbort))
{
}
//------------------------------------------------------------------------------
BaseThread * BaseServer::newThread()
{
  return newObject<BaseThread>();
}
//------------------------------------------------------------------------------
void BaseServer::sweepThreads()
{
  AutoLock<LiteWriteLock> lock(mutex_);
  EmbeddedListNode<BaseThread> * btp = threads_.first();
  while( btp != NULL ){
    BaseThread * thread = &BaseThread::serverListNodeObject(*btp);
    thread->fibersReadWriteLock_.acquire();
    if( thread->fibers_.count() == 0 ){
      thread->wait();
      btp = btp->next();
      threads_.drop(*thread);
    }
    else {
      thread->fibersReadWriteLock_.release();
      btp = btp->next();
    }
  }
}
//------------------------------------------------------------------------------
void BaseServer::closeServer()
{
  assert( currentFiber() == NULL );
  uintptr_t how = howCloseServer_;
  EmbeddedListNode<BaseThread> * btp;
  EmbeddedListNode<Fiber> * bfp;
  BaseThread * thread;
  shutdown_ = true;
  {
    AutoLock<LiteWriteLock> lock(mutex_);
    for( btp = threads_.first(); btp != NULL; btp = btp->next() ){
      thread = &BaseThread::serverListNodeObject(*btp);
      AutoLock<LiteWriteLock> lock2(thread->fibersReadWriteLock_);
      for( bfp = thread->fibers_.first(); bfp != NULL; bfp = bfp->next() ){
        Fiber * fiber = &Fiber::nodeObject(*bfp);
        if( how & csTerminate ) fiber->terminate();
        fiber->fiberBreakExecution();
      }
    }
  }
  for(;;){
    btp = NULL;
    uint64_t fbtm, fbtmd = 100000;
    for( fbtm = fiberTimeout_; fbtm > 0; fbtm -= fbtmd ){
      {
        AutoLock<LiteWriteLock> lock(mutex_);
        for( btp = threads_.first(); btp != NULL; btp = btp->next() ){
          thread = &BaseThread::serverListNodeObject(*btp);
          AutoLock<LiteWriteLock> lock2(thread->fibersReadWriteLock_);
          if( thread->fibers_.count() > 0 ) break;
        }
      }
      if( how & csAbort ) BaseThread::requester().abort();
      if( btp == NULL ) break;
      sweepThreads();
      fbtmd = fbtm > fbtmd ? fbtmd : fbtm;
      if( how & csWait ){
/*#if defined(__WIN32__) || defined(__WIN64__)
        if( how & csDWM ){
          MSG msg;
          if( PeekMessage(&msg,NULL,0,0,PM_REMOVE) != 0 ){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if( msg.message == fiberFinishMessage ){
              sweepThreads();
            }
            else if( msg.message == WM_QUIT ){
              how |= csShutdown;
              fbtmd = fbtm;
            }
          }
          else if( WaitMessage() == 0 ){
            int32_t err = GetLastError() + errorOffset;
            newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
          }
        }
        else
#endif*/
        ksleep(fbtmd);
      }
    }
    if( btp == NULL ) break;
    if( how & csShutdown ){
      BaseThread::requester().shutdownDescriptors();
      BaseThread::requester().abort();
    }
  }
  sweepThreads();
  assert( threads_.count() == 0 );
  shutdown_ = false;
}
//------------------------------------------------------------------------------
BaseThread * BaseServer::selectThread()
{
  BaseThread * thread = NULL;
  uintptr_t msc = ~uintptr_t(0);
  if( threads_.count() >= mt_ ){
    EmbeddedListNode<BaseThread> * btp;
    for( btp = threads_.first(); btp != NULL; ){
      BaseThread * athread = &BaseThread::serverListNodeObject(*btp);
      athread->fibersReadWriteLock_.acquire();
      if( threads_.count() > mt_ && athread->fibers_.count() == 0 ){
        thread->wait();
        btp = btp->next();
        threads_.drop(*thread);
      }
      else {
        if( athread->fibers_.count() < athread->mfpt_ && athread->fibers_.count() < msc ){
          msc = athread->fibers_.count();
          thread = athread;
        }
        btp = btp->next();
        athread->fibersReadWriteLock_.release();
      }
    }
  }
  if( thread == NULL ){
    AutoPtr<BaseThread> p(thread = newThread());
    thread->resume();
    threads_.insToTail(*p.ptr(NULL));
    thread->server(this);
#if HAVE_UCONTEXT_H
    volatile uintptr_t l;
    while( (l = thread->context_.uc_mcontext.mc_len) == 0 ) ksleep1();
#endif
  }
  assert( thread != NULL );
  return thread;
}
//------------------------------------------------------------------------------
void BaseServer::attachFiber(Fiber & fiber)
{
 {
    AutoLock<LiteWriteLock> lock(mutex_);
    BaseThread * thread = selectThread();
    fiber.allocateStack(fiberStackSize_,thread);
    {
      AutoLock<LiteWriteLock> lock2(thread->fibersReadWriteLock_);
      thread->fibers_.insToTail(fiber);
      fiber.thread_ = thread;
    }
    fiber.event_.type_ = etDispatch;
    thread->postEvent(&fiber.event_);
  }
  if( stdErr.debugLevel(90) )
    stdErr.debug(90,utf8::String::Stream() <<
      "fibers: " << interlockedIncrement(fibersCount_,0) << ", "
      "threads: " << interlockedIncrement(threadsCount_,0) << "\n"
    );
}
//------------------------------------------------------------------------------
void BaseServer::attachFiber(const AutoPtr<Fiber> & fiber)
{
  {
    AutoLock<LiteWriteLock> lock(mutex_);
    BaseThread * thread = selectThread();
    fiber->allocateStack(fiberStackSize_,thread);
    Fiber * fib = fiber;
    {
      AutoLock<LiteWriteLock> lock2(thread->fibersReadWriteLock_);
      thread->fibers_.insToTail(*fiber.ptr(NULL));
      fib->thread_ = thread;
    }
    fib->event_.type_ = etDispatch;
    thread->postEvent(&fib->event_);
  }
  if( stdErr.debugLevel(90) )
    stdErr.debug(90,utf8::String::Stream() <<
      "fibers: " << interlockedIncrement(fibersCount_,0) << ", "
      "threads: " << interlockedIncrement(threadsCount_,0) << "\n"
    );
}
//------------------------------------------------------------------------------
void BaseServer::maintainFibers()
{
  EmbeddedListNode<BaseThread> * btp;
  EmbeddedListNode<Fiber> * bfp;
  AutoLock<LiteWriteLock> lock(mutex_);
  for( btp = threads_.first(); btp != NULL; btp = btp->next() ){
    BaseThread * thread = &BaseThread::serverListNodeObject(*btp);
    AutoLock<LiteWriteLock> lock2(thread->fibersReadWriteLock_);
    for( bfp = thread->fibers_.first(); bfp != NULL; bfp = bfp->next() )
      maintainFiber(&Fiber::nodeObject(*bfp));
  }
}
//------------------------------------------------------------------------------
void BaseServer::maintainFiber(Fiber *)
{
}
//------------------------------------------------------------------------------
void BaseServer::DispatchWindowMessages()
{
#if defined(__WIN32__) || defined(__WIN64__)
  for(;;){
    MSG msg;
    if( PeekMessage(&msg,NULL,0,0,PM_REMOVE) != 0 ){
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    if( msg.message == fiberFinishMessage ) sweepThreads();
    if( msg.message == WM_QUIT || !active() ) break;
    if( WaitMessage() == 0 ){
      int32_t err = GetLastError() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
  }
#endif
}
//------------------------------------------------------------------------------
bool BaseServer::active() const
{
  AutoLock<LiteWriteLock> lock(mutex_);
  EmbeddedListNode<BaseThread> * btp;
  for( btp = threads_.first(); btp != NULL; btp = btp->next() ){
    BaseThread * thread = &BaseThread::serverListNodeObject(*btp);
    if( thread->active() && thread->fibers_.count() > 0 ) break;
  }
  return btp != NULL;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//------------------------------------------------------------------------------
FiberWriteLock::~FiberWriteLock()
{
  if( sem_ != NULL ) CloseHandle(sem_);
}
//---------------------------------------------------------------------------
FiberWriteLock::FiberWriteLock()
{
  sem_ = CreateSemaphoreA(NULL,1,~(ULONG) 0 >> 1, NULL);
  if( sem_ == NULL ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
}
//---------------------------------------------------------------------------
bool FiberWriteLock::tryAcquireHelper()
{
  DWORD r = WaitForSingleObject(sem_,0);
  if( r == WAIT_FAILED ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return r == WAIT_OBJECT_0 || r == WAIT_ABANDONED;
}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
bool FiberWriteLock::internalAcquire(bool wait)
{
  if( isRunInFiber() ){
    if( wait ){
#if defined(__WIN32__) || defined(__WIN64__)
      if( tryAcquireHelper() ) return true;
#else
      if( mutex_.tryAcquire() ) return true;
#endif
      assert( currentFiber() != NULL );
      currentFiber()->event_.mutex_ = this;
      currentFiber()->event_.timeout_ = ~uint64_t(0);
      currentFiber()->event_.type_ = etAcquireReadWriteLock;
      currentFiber()->thread()->postRequest();
      currentFiber()->switchFiber(currentFiber()->mainFiber());
      assert( currentFiber()->event_.type_ == etAcquireReadWriteLock );
      if( currentFiber()->event_.errno_ != 0 )
        newObjectV1C2<Exception>(currentFiber()->event_.errno_,__PRETTY_FUNCTION__)->throwSP();
      return true;
    }
  }
  else {
    if( wait ){
#if defined(__WIN32__) || defined(__WIN64__)
      DWORD r = WaitForSingleObject(sem_,INFINITE);
      if( r == WAIT_FAILED || (r != WAIT_OBJECT_0 && r != WAIT_ABANDONED) ){
        int32_t err = GetLastError() + errorOffset;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
#else
      mutex_.acquire();
#endif
      return true;
    }
  }
#if defined(__WIN32__) || defined(__WIN64__)
  return tryAcquireHelper();
#else
  return mutex_.tryAcquire();
#endif
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
void FiberWriteLock::release()
{
  if( ReleaseSemaphore(sem_,1,NULL) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
}
#endif
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
FiberReadWriteLock & FiberReadWriteLock::rdLock()
{
  bool r;
  for(;;){
    acquire();
    r = value_ >= 0;
    if( r ){
      value_++;
      break;
    }
    queue_++;
    release();
    waitQueue_.acquire();
  }
  release();
  return *this;
}
//---------------------------------------------------------------------------
FiberReadWriteLock & FiberReadWriteLock::wrLock()
{
  bool r;
  for(;;){
    acquire();
    r = value_ == 0;
    if( r ){
      value_ = -1;
      break;
    }
    queue_++;
    release();
    waitQueue_.acquire();
  }
  release();
  return *this;
}
//---------------------------------------------------------------------------
FiberReadWriteLock & FiberReadWriteLock::unlock()
{
  acquire();
  assert( value_ != 0 );
  if( value_ > 0 ) value_--; else if( value_ < 0 ) value_++;
  if( queue_ > 0 ) waitQueue_.release();
  release();
  return *this;
}
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
/*FiberReadWriteLock & FiberReadWriteLock::rdLock()
{
  Fiber * fib = currentFiber();
  if( fib == NULL ){
    for(;;){
      AutoLock<FiberWriteLock> lock(mutex_);
      if( counter_ >= 0 ){
        counter_++;
        break;
      }
      ksleep1();
    }
    stdErr.log(
      lmNOTIFY,
      utf8::String::Stream() << __PRETTY_FUNCTION__ << " called from non fiber enviroment\n"
    );
  }
  else {
    BaseFiber * fiber = dynamic_cast<BaseFiber *>(fib);
    assert( fiber != NULL );
    mutex_.acquire();
    if( counter_ < 0 ){
      wait_.insToTail(*fiber);
      mutex_.release();
      fiber->switchFiber(fiber->mainFiber());
    }
    else {
      acquired_ = true;
      mutex_.release();
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
FiberReadWriteLock & FiberReadWriteLock::wrLock()
{
  Fiber * fib = currentFiber();
  if( fib == NULL ){
    mutex_.acquire();
    main_.wrLock();
    mutex_.release();
    stdErr.log(
      lmNOTIFY,
      utf8::String::Stream() << __PRETTY_FUNCTION__ << " called from non fiber enviroment\n"
    );
  }
  else {
    BaseFiber * fiber = dynamic_cast<BaseFiber *>(fib);
    assert( fiber != NULL && fiber->ipc_.prev() == NULL && fiber->ipc_.next() == NULL );
    mutex_.acquire();
    bool a = main_.tryWRLock();
    if( !a ) wait_.insToTail(*fiber);
    mutex_.release();
    if( !a ){
      assert( fiber != &fiber->mainFiber() );
      fiber->switchFiber(fiber->mainFiber());
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
FiberReadWriteLock & FiberReadWriteLock::unlock()
{
  if( currentFiber() == NULL )
    stdErr.log(
      lmNOTIFY,
      utf8::String::Stream() << __PRETTY_FUNCTION__ << " called from non fiber enviroment\n"
    );
  BaseFiber * fiber;
  mutex_.acquire();
  if( wait_.count() > 0 ){
    fiber = &BaseFiber::ipcObject(*wait_.first());
    wait_.remove(*wait_.first());
    fiber->thread()->postEvent(etDispatch,fiber);
  }
  else {
    main_.unlock();
  }
  mutex_.release();
  return *this;
}*/
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
/*void FiberSemaphore::wait()
{
  Fiber * fib = currentFiber();
  if( fib == NULL ){
    mutex_.acquire();
    semaphore_.wait();
    mutex_.release();
    stdErr.log(
      lmNOTIFY,
      utf8::String::Stream() << __PRETTY_FUNCTION__ << " called from non fiber enviroment\n"
    );
  }
  else {
    BaseFiber * fiber = dynamic_cast<BaseFiber *>(fib);
    assert( fiber != NULL && fiber->ipc_.prev() == NULL && fiber->ipc_.next() == NULL );
    mutex_.acquire();
    bool a = semaphore_.tryWait();
    if( !a ) wait_.insToTail(*fiber);
    mutex_.release();
    if( !a ){
      assert( fiber != &fiber->mainFiber() );
      fiber->switchFiber(fiber->mainFiber());
    }
  }
}
//---------------------------------------------------------------------------
bool FiberSemaphore::tryWait()
{
  mutex_.acquire();
  bool a = semaphore_.tryWait();
  mutex_.release();
  return a;
}
//---------------------------------------------------------------------------
void FiberSemaphore::post()
{
  if( currentFiber() == NULL )
    stdErr.log(
      lmNOTIFY,
      utf8::String::Stream() << __PRETTY_FUNCTION__ << " called from non fiber enviroment\n"
    );
  BaseFiber * fiber;
  mutex_.acquire();
  if( wait_.count() > 0 ){
    fiber = &BaseFiber::ipcObject(*wait_.first());
    wait_.remove(*wait_.first());
    fiber->thread()->postEvent(etDispatch,fiber);
  }
  else {
    semaphore_.post();
  }
  mutex_.release();
}*/
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
bool FiberSemaphore::timedWait(uint64_t timeout)
{
  if( isRunInFiber() ){
    currentFiber()->event_.semaphore_ = this;
    currentFiber()->event_.timeout_ = timeout;
    currentFiber()->event_.type_ = etAcquireSemaphore;
    currentFiber()->thread()->postRequest();
    currentFiber()->switchFiber(currentFiber()->mainFiber());
    assert( currentFiber()->event_.type_ == etAcquireSemaphore );
#if defined(__WIN32__) || defined(__WIN64__)
    if( currentFiber()->event_.errno_ != 0 && currentFiber()->event_.errno_ != WAIT_TIMEOUT )
      newObjectV1C2<Exception>(currentFiber()->event_.errno_,__PRETTY_FUNCTION__)->throwSP();
    return currentFiber()->event_.errno_ != WAIT_TIMEOUT;
#else
    if( currentFiber()->event_.errno_ != 0 && currentFiber()->event_.errno_ != ETIMEDOUT )
      newObjectV1C2<Exception>(currentFiber()->event_.errno_,__PRETTY_FUNCTION__)->throwSP();
    return currentFiber()->event_.errno_ != ETIMEDOUT;
#endif
  }
  return Semaphore::timedWait(timeout);
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
