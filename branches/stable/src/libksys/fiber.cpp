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
//------------------------------------------------------------------------------
#include <adicpp/ksys.h>
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
uint8_t Fiber::currentFiberPlaceHolder[sizeof(ThreadLocalVariable<Fiber>)];
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
AsyncEvent::~AsyncEvent()
{
}
//---------------------------------------------------------------------------
AsyncEvent::AsyncEvent() : position_(0), buffer_(NULL), length_(0),
  fiber_(NULL), descriptor_(NULL), errno_(0), type_(etNone)
{
#if defined(__WIN32__) || defined(__WIN64__)
  memset(&overlapped_,0,sizeof(overlapped_));
#endif
#if HAVE_KQUEUE
  memset(&iocb_, 0, sizeof(iocb_));
#endif
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void Fiber::initialize()
{
#if !defined(__WIN32__) && !defined(__WIN64__)
  new (currentFiberPlaceHolder) ThreadLocalVariable<Fiber>;
#endif
}
//------------------------------------------------------------------------------
void Fiber::cleanup()
{
#if !defined(__WIN32__) && !defined(__WIN64__)
  reinterpret_cast<ThreadLocalVariable<Fiber> *>(currentFiberPlaceHolder)->~ThreadLocalVariable<Fiber>();
#endif
}
//------------------------------------------------------------------------------
Fiber::~Fiber()
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( fiber_ != NULL ) DeleteFiber(fiber_);
#endif
}
//---------------------------------------------------------------------------
Fiber::Fiber() : started_(false), terminated_(false), finished_(false),
#if !defined(__WIN32__) && !defined(__WIN64__)
  stackPointer_(NULL),
#else
  fiber_(NULL),
#endif
  thread_(NULL)
{
  event_.fiber_ = this;
}
//---------------------------------------------------------------------------
Fiber & Fiber::allocateStack(
#if !defined(__WIN32__) && !defined(__WIN64__)
  void * ip,void * param,size_t size,Fiber * mainFiber,uintptr_t dummy1,uintptr_t dummy2)
#else
  void *,void *,size_t,Fiber *,uintptr_t,uintptr_t)
#endif
{
#if !defined(__WIN32__) && !defined(__WIN64__)
  size += -intptr_t(size) & (sizeof(uintptr_t) - 1);
  stack_.realloc(size);
  stackPointer_ = stack_.ptr();
  memset(stackPointer_,0xAA,size);
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
#endif
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
#if defined(__WIN32__) || defined(__WIN64__)
    fiber->fiberExecute();
#else
    ip(param);
#endif
  }
  catch( ksys::ExceptionSP & e ){
    e->writeStdError();
  }
  catch( ... ){
  }
  fiber->detachDescriptors();
  fiber->finished_ = true;
  fiber->switchFiber(fiber->thread_);
}
//------------------------------------------------------------------------------
#if !defined(__WIN32__) && !defined(__WIN64__)
void Fiber::fiber2(Fiber * fiber)
{
  try {
    fiber->execute();
  }
  catch( ... ){
    fiber->detachDescriptors();
    throw;
  }
  fiber->detachDescriptors();
}
#endif
//------------------------------------------------------------------------------
void Fiber::detachDescriptors()
{
  EmbeddedListNode<AsyncDescriptor> * adp;
  for( adp = descriptorsList_.first(); adp != NULL; adp = adp->next() )
    AsyncDescriptor::fiberListNodeObject(*adp).detach();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
BaseThread::~BaseThread()
{
  assert( fibers_.count() == 0 );
}
//---------------------------------------------------------------------------
BaseThread::BaseThread() : server_(NULL)//, maxStackSize_(0)
{
}
//---------------------------------------------------------------------------
void BaseThread::threadExecute()
{
#if defined(__WIN32__) || defined(__WIN64__)
  convertThreadToFiber();
  try {
#endif
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
      detectMaxFiberStackSize();
      sweepFiber(ev->fiber_);
    }
  }
}
//------------------------------------------------------------------------------
void BaseThread::sweepFiber(Fiber * fiber)
{
  {
    AutoLock<InterlockedMutex> lock(mutex_);
    fibers_.remove(*fiber);
  }
  delete fiber;
#if defined(__WIN32__) || defined(__WIN64__)
  if( (server_->howCloseServer_ & server_->csDWM) != 0 && fibers_.count() == 0 )
    while( PostThreadMessage(mainThreadId,fiberFinishMessage,NULL,NULL) == 0 ) Sleep(1);
#endif
}
//------------------------------------------------------------------------------
void BaseThread::detectMaxFiberStackSize()
{
}
//------------------------------------------------------------------------------
void BaseThread::postEvent(AsyncEvent * event)
{
  AutoLock<InterlockedMutex> lock(mutex_);
  events_.insToTail(*event);
  if( events_.count() < 2 ) semaphore_.post();
}
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
BaseServer::~BaseServer()
{
  assert( !active() );
}
//------------------------------------------------------------------------------
BaseServer::BaseServer() :
  mfpt_(numberOfProcessors() * 4),
  mt_(numberOfProcessors() * 4),
  fiberStackSize_(PTHREAD_STACK_MIN),
  fiberTimeout_(10000000),
  howCloseServer_(HowCloseServer(csWait | csTerminate | csShutdown | csAbort))
{
}
//------------------------------------------------------------------------------
BaseThread * BaseServer::newThread()
{
  return new BaseThread;
}
//------------------------------------------------------------------------------
void BaseServer::sweepThreads()
{
  AutoLock<InterlockedMutex> lock(mutex_);
  EmbeddedListNode<BaseThread> * btp = threads_.first();
  while( btp != NULL ){
    BaseThread * thread = &BaseThread::serverListNodeObject(*btp);
    thread->mutex_.acquire();
    if( thread->fibers_.count() == 0 ){
      thread->mutex_.release();
      thread->Thread::terminate();
      thread->semaphore_.post();
      thread->wait();
      assert( thread->Thread::finished() );
      btp = btp->next();
      threads_.remove(*thread);
      delete thread;
    }
    else {
      thread->mutex_.release();
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
  EmbeddedListNode<AsyncDescriptor> * adp;
  BaseThread * thread;
  if( how & csTerminate ){
    AutoLock<InterlockedMutex> lock(mutex_);
    for( btp = threads_.first(); btp != NULL; btp = btp->next() ){
      thread = &BaseThread::serverListNodeObject(*btp);
      AutoLock<InterlockedMutex> lock2(thread->mutex_);
      for( bfp = thread->fibers_.first(); bfp != NULL; bfp = bfp->next() )
        Fiber::nodeObject(*bfp).terminate();
    }
  }
  for(;;){
    btp = NULL;
    uint64_t fbtm, fbtmd = 100000;
    for( fbtm = fiberTimeout_; fbtm > 0; fbtm -= fbtmd ){
      {
        AutoLock<InterlockedMutex> lock(mutex_);
        for( btp = threads_.first(); btp != NULL; btp = btp->next() ){
          thread = &BaseThread::serverListNodeObject(*btp);
          AutoLock<InterlockedMutex> lock2(thread->mutex_);
          if( thread->fibers_.count() > 0 ) break;
        }
      }
      if( how & csAbort ) BaseThread::requester().abort();
      if( btp == NULL ) break;
      sweepThreads();
      fbtmd = fbtm > fbtmd ? fbtmd : fbtm;
      if( how & csWait ){
#if defined(__WIN32__) || defined(__WIN64__)
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
            int32_t err = GetLastError() + ksys::errorOffset;
            throw ksys::ExceptionSP(new ksys::Exception(err,__PRETTY_FUNCTION__));
          }
        }
        else
#endif
        ksys::sleep(fbtmd);
      }
    }
    if( btp == NULL ) break;
    if( how & csShutdown ){
      AutoLock<InterlockedMutex> lock(mutex_);
      for( btp = threads_.first(); btp != NULL; btp = btp->next() ){
        thread = &BaseThread::serverListNodeObject(*btp);
        AutoLock<InterlockedMutex> lock2(thread->mutex_);
        for( adp = thread->descriptorsList_.first(); adp != NULL; adp = adp->next() )
          AsyncDescriptor::clusterListNodeObject(*adp).shutdown2();
      }
      BaseThread::requester().abort();
    }
  }
  sweepThreads();
  assert( threads_.count() == 0 );
}
//------------------------------------------------------------------------------
BaseThread * BaseServer::selectThread()
{
  BaseThread * thread = NULL;
  uintptr_t msc = ~uintptr_t(0);
  if( threads_.count() >= mt_ ){
    EmbeddedListNode<BaseThread> * btp;
    for( btp = threads_.first(); btp != NULL; btp = btp->next() ){
      BaseThread * athread = &BaseThread::serverListNodeObject(*btp);
      AutoLock<InterlockedMutex> lock2(athread->mutex_);
      if( athread->fibers_.count() < msc ){
        msc = athread->fibers_.count();
        thread = athread;
      }
    }
  }
  if( thread == NULL ){
    AutoPtr<BaseThread> p(thread = newThread());
    thread->resume();
    threads_.insToTail(*p.ptr(NULL));
    thread->server(this);
  }
  assert( thread != NULL );
  return thread;
}
//------------------------------------------------------------------------------
void BaseServer::attachFiber(const AutoPtr<Fiber> & fiber)
{
  AutoLock<InterlockedMutex> lock(mutex_);
  BaseThread * thread = selectThread();
  fiber->allocateStack(Fiber::start,fiber,fiberStackSize_,thread);
#if defined(__WIN32__) || defined(__WIN64__)
  fiber->createFiber(fiberStackSize_);
#endif
  Fiber * fib = fiber;
  {
    AutoLock<InterlockedMutex> lock2(thread->mutex_);
    thread->fibers_.insToTail(*fiber.ptr(NULL));
    fib->thread_ = thread;
  }
  fib->event_.type_ = etDispatch;
  thread->postEvent(&fib->event_);
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
      int32_t err = GetLastError() + ksys::errorOffset;
      throw ksys::ExceptionSP(
        new ksys::Exception(err,__PRETTY_FUNCTION__)
      );
    }
  }
#endif
}
//------------------------------------------------------------------------------
bool BaseServer::active() const
{
  AutoLock<InterlockedMutex> lock(mutex_);
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
FiberInterlockedMutex::~FiberInterlockedMutex()
{
  if( sem_ != NULL ) CloseHandle(sem_);
}
//---------------------------------------------------------------------------
FiberInterlockedMutex::FiberInterlockedMutex()
{
  sem_ = CreateSemaphoreA(NULL,1,~(ULONG) 0 >> 1, NULL);
  if( sem_ == NULL ){
    int32_t err = GetLastError() + errorOffset;
    throw ExceptionSP(new Exception(err,__PRETTY_FUNCTION__));
  }
}
//---------------------------------------------------------------------------
bool FiberInterlockedMutex::tryAcquireHelper()
{
  DWORD r = WaitForSingleObject(sem_,0);
  if( r == WAIT_FAILED ){
    int32_t err = GetLastError() + errorOffset;
    throw ExceptionSP(new Exception(err,__PRETTY_FUNCTION__));
  }
  return r == WAIT_OBJECT_0 || r == WAIT_ABANDONED;
}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
bool FiberInterlockedMutex::internalAcquire(bool wait)
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
      currentFiber()->event_.type_ = etAcquire;
      currentFiber()->thread()->postRequest();
      currentFiber()->switchFiber(currentFiber()->mainFiber());
      assert( currentFiber()->event_.type_ == etAcquire );
      if( currentFiber()->event_.errno_ != 0 )
        throw ksys::ExceptionSP(
          new Exception(currentFiber()->event_.errno_,__PRETTY_FUNCTION__)
        );
      return true;
    }
  }
  else {
    if( wait ){
#if defined(__WIN32__) || defined(__WIN64__)
      DWORD r = WaitForSingleObject(sem_,INFINITE);
      if( r == WAIT_FAILED || (r != WAIT_OBJECT_0 && r != WAIT_ABANDONED) ){
        int32_t err = GetLastError() + errorOffset;
        throw ExceptionSP(new Exception(err,__PRETTY_FUNCTION__));
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
void FiberInterlockedMutex::release()
{
  if( ReleaseSemaphore(sem_,1,NULL) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    throw ExceptionSP(new Exception(err,__PRETTY_FUNCTION__));
  }
}
#endif
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
/*FiberMutex & FiberMutex::rdLock()
{
  Fiber * fib = currentFiber();
  if( fib == NULL ){
    for(;;){
      AutoLock<FiberInterlockedMutex> lock(mutex_);
      if( counter_ >= 0 ){
        counter_++;
        break;
      }
      sleep1();
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
FiberMutex & FiberMutex::wrLock()
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
FiberMutex & FiberMutex::unlock()
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
} // namespace ksys
//------------------------------------------------------------------------------