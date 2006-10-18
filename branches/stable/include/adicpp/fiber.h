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
#ifndef _fiber_H_
#define _fiber_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Fiber {
  friend void initialize();
  friend void cleanup();
  friend class BaseThread;
  friend class BaseServer;
  friend class Requester;
  friend class AsyncDescriptor;
  public:
    virtual ~Fiber();
    Fiber();

    const bool & started() const;
    const bool & terminated() const;
    Fiber & terminate();
    const bool & finished() const;

    void switchFiber(Fiber * fiber) GNUG_NOTHROW;

    Fiber * const mainFiber() const;

    BaseThread * const & thread() const;

    void attachDescriptor(AsyncDescriptor & descriptor);
    void detachDescriptor(AsyncDescriptor & descriptor);

    AsyncEvent event_;
  protected:
    bool started_;
    bool terminated_;
    bool finished_;

    virtual void fiberExecute() = 0;
  private:
#if defined(__WIN32__) || defined(__WIN64__)
    static VOID WINAPI start(Fiber * fiber);
    LPVOID fiber_;
#else
    static void start(Fiber * fiber, void * param, void (*ip) (void *)) GNUG_NOTHROW_CDECL;
    AutoPtr<uint8_t> stack_;
    void * stackPointer_;
    void switchFiber2(Fiber * fiber) GNUG_NOTHROW_CDECL;
#endif
    BaseThread * thread_;
    EmbeddedList<
      AsyncDescriptor,
      AsyncDescriptor::fiberListNode,
      AsyncDescriptor::fiberListNodeObject
    > descriptorsList_;

    Fiber & allocateStack(void * ip,void * param,size_t size,Fiber * mainFiber,uintptr_t dummy1 = 0,uintptr_t dummy2 = 0);
#if defined(__WIN32__) || defined(__WIN64__)
    Fiber & createFiber(uintptr_t dwStackSize);
    Fiber & deleteFiber();
    Fiber & clearFiber();
    Fiber & convertThreadToFiber();
#else
    static void fiber2(Fiber * fiber);
#endif

    void detachDescriptors();

    mutable EmbeddedListNode<Fiber> node_;
    static EmbeddedListNode<Fiber> & node(const Fiber & object){
      return object.node_;
    }
    static Fiber & nodeObject(const EmbeddedListNode<Fiber> & node,Fiber * p = NULL){
      return node.object(p->node_);
    }

    static uint8_t currentFiberPlaceHolder[];
    friend Fiber * currentFiber();
    static void initialize();
    static void cleanup();
};
//---------------------------------------------------------------------------
inline BaseThread * const & Fiber::thread() const
{
  return thread_;
}
//---------------------------------------------------------------------------
inline const bool & Fiber::started() const
{
  return started_;
}
//---------------------------------------------------------------------------
inline const bool & Fiber::terminated() const
{
  return terminated_;
}
//---------------------------------------------------------------------------
inline Fiber & Fiber::terminate()
{
  terminated_ = true;
  return *this;
}
//---------------------------------------------------------------------------
inline const bool & Fiber::finished() const
{
  return finished_;
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
inline Fiber & Fiber::createFiber(uintptr_t dwStackSize)
{
  assert( fiber_ == NULL );
#ifdef __WIN64__
  fiber_ = CreateFiber(dwStackSize, (LPFIBER_START_ROUTINE) start, this);
#else
  fiber_ = CreateFiber((DWORD) dwStackSize,(LPFIBER_START_ROUTINE) start,this);
#endif
  if( fiber_ == NULL ){
    int32_t err = GetLastError() + errorOffset;
    Exception::throwSP(err,__PRETTY_FUNCTION__);
  }
  return *this;
}
//---------------------------------------------------------------------------
inline Fiber & Fiber::convertThreadToFiber()
{
  assert( fiber_ == NULL );
  if( (fiber_ = ConvertThreadToFiber(this)) == NULL ){
    int32_t err = GetLastError() + errorOffset;
    Exception::throwSP(err,__PRETTY_FUNCTION__);
  }
  return *this;
}
//---------------------------------------------------------------------------
inline Fiber & Fiber::deleteFiber()
{
  DeleteFiber(fiber_);
  clearFiber();
  return *this;
}
//---------------------------------------------------------------------------
inline Fiber & Fiber::clearFiber()
{
  fiber_ = NULL;
  return *this;
}
//---------------------------------------------------------------------------
inline void Fiber::switchFiber(Fiber * fiber)
{
  *reinterpret_cast<ThreadLocalVariable<Fiber> *>(currentFiberPlaceHolder) = fiber;
  SwitchToFiber(fiber->fiber_);
}
//---------------------------------------------------------------------------
#else
//---------------------------------------------------------------------------
inline void Fiber::switchFiber(Fiber * fiber)
{
  *reinterpret_cast<ThreadLocalVariable<Fiber> *>(currentFiberPlaceHolder) = fiber;
  switchFiber2(fiber);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
inline Fiber * currentFiber()
{
  return *reinterpret_cast<ThreadLocalVariable<Fiber> *>(Fiber::currentFiberPlaceHolder);
}
//---------------------------------------------------------------------------
inline bool isRunInFiber()
{
  return currentFiber() != NULL;
}
//---------------------------------------------------------------------------
inline AsyncDescriptor & AsyncDescriptor::attach()
{
  Fiber * fiber = currentFiber();
  if( fiber != NULL ) fiber->attachDescriptor(*this);
  return *this;
}
//------------------------------------------------------------------------------
inline AsyncDescriptor & AsyncDescriptor::detach()
{
  if( fiber_ != NULL ) fiber_->detachDescriptor(*this);
  return *this;
}
//------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
typedef EmbeddedList<AsyncEvent,AsyncEvent::node,AsyncEvent::nodeObject> Events;
typedef EmbeddedListNode<AsyncEvent> EventsNode;
//---------------------------------------------------------------------------
class AsyncIoSlave : public Thread, public Semaphore, public InterlockedMutex {
  public:
    virtual ~AsyncIoSlave();
    AsyncIoSlave();

    bool transplant(AsyncEvent & requests);
#if HAVE_KQUEUE
    void cancelEvent(const AsyncEvent & request);
#endif
    bool abortNotification(DirectoryChangeNotification * dcn = NULL);
  protected:
  private:
    AsyncIoSlave(const AsyncIoSlave &){}
    void operator = (const AsyncIoSlave &){}

    Events requests_;
    Events newRequests_;
#if defined(__WIN32__) || defined(__WIN64__)
    HANDLE events_[MAXIMUM_WAIT_OBJECTS];
    AsyncEvent * eReqs_[MAXIMUM_WAIT_OBJECTS];
#elif HAVE_KQUEUE
    int                   kqueue_;
    Array<struct kevent>  kevents_;
#else
#error async io not implemented
#endif
    void openAPI(AsyncEvent * object);
    void closeAPI(AsyncEvent * object);
    void threadExecute();
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncOpenFileSlave : public Thread, public Semaphore, public InterlockedMutex {
  public:
    virtual ~AsyncOpenFileSlave();
    AsyncOpenFileSlave();

    bool transplant(AsyncEvent & requests);
  protected:
  private:
    AsyncOpenFileSlave(const AsyncOpenFileSlave &){}
    void operator = (const AsyncOpenFileSlave &){}

    Events requests_;

    void threadExecute();
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncTimerSlave : public Thread, public Semaphore, public InterlockedMutex {
  public:
    virtual ~AsyncTimerSlave();
    AsyncTimerSlave();

    void transplant(AsyncEvent & requests);
    void abortTimer();
  protected:
  private:
    AsyncTimerSlave(const AsyncTimerSlave &){}
    void operator = (const AsyncTimerSlave &){}

    Events requests_;

    void threadExecute();
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncAcquireSlave : public Thread, public Semaphore, public InterlockedMutex {
  public:
    virtual ~AsyncAcquireSlave();
    AsyncAcquireSlave();

    bool transplant(AsyncEvent & requests);
  protected:
  private:
    AsyncAcquireSlave(const AsyncAcquireSlave &){}
    void operator = (const AsyncAcquireSlave &){}

    Events requests_;
#if defined(__WIN32__) || defined(__WIN64__)
    Events newRequests_;
    HANDLE sems_[MAXIMUM_WAIT_OBJECTS];
    AsyncEvent * eSems_[MAXIMUM_WAIT_OBJECTS];
    intptr_t sp_;
#endif

    void threadExecute();
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
class AsyncWin9xDirectoryChangeNotificationSlave : public Thread, public Semaphore, public InterlockedMutex {
  public:
    virtual ~AsyncWin9xDirectoryChangeNotificationSlave();
    AsyncWin9xDirectoryChangeNotificationSlave();

    bool transplant(AsyncEvent & requests);
    bool abortNotification(DirectoryChangeNotification * dcn = NULL);
  protected:
  private:
    AsyncWin9xDirectoryChangeNotificationSlave(const AsyncAcquireSlave &){}
    void operator = (const AsyncWin9xDirectoryChangeNotificationSlave &){}

    Events requests_;
#if defined(__WIN32__) || defined(__WIN64__)
    Events newRequests_;
    HANDLE sems_[MAXIMUM_WAIT_OBJECTS];
    AsyncEvent * eSems_[MAXIMUM_WAIT_OBJECTS];
    intptr_t sp_;
#endif

    void threadExecute();
};
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Requester {
  public:
    ~Requester();
    Requester();

    void abort();
    bool abortNotification(DirectoryChangeNotification * dcn = NULL);
    void postRequest(AsyncDescriptor * descriptor);
  protected:
  private:
    InterlockedMutex ioRequestsMutex_;
    Vector<AsyncIoSlave> ioSlaves_;
    int64_t ioSlavesSweepTime_;

    InterlockedMutex ofRequestsMutex_;
    Vector<AsyncOpenFileSlave> ofSlaves_;
    int64_t ofSlavesSweepTime_;

    InterlockedMutex timerRequestsMutex_;
    AutoPtr<AsyncTimerSlave> timerSlave_;

    InterlockedMutex acquireRequestsMutex_;
    Vector<AsyncAcquireSlave> acquireSlaves_;
    int64_t acquireSlavesSweepTime_;

#if defined(__WIN32__) || defined(__WIN64__)
    InterlockedMutex wdcnRequestsMutex_;
    Vector<AsyncWin9xDirectoryChangeNotificationSlave> wdcnSlaves_;
    int64_t wdcnSlavesSweepTime_;
#endif

    Requester(const Requester &){}
    void operator = (const Requester &){}
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class BaseThread : public Thread, public Fiber {
  friend void initialize();
  friend void cleanup();
  friend class Fiber;
  friend class BaseServer;
  public:
    virtual ~BaseThread();
    BaseThread();

    BaseServer * const &  server() const;
    BaseThread & server(BaseServer * serverL);

    void postRequest(AsyncDescriptor * descriptor = NULL);
    void postEvent(AsyncEvent * event);
    const uintptr_t & maxFibersPerThread() const;
    BaseThread & maxFibersPerThread(uintptr_t mfpt);
  protected:
  private:
    BaseServer * server_;
    Semaphore semaphore_;
    InterlockedMutex mutex_;
    Events events_;
    EmbeddedList<
      Fiber,
      Fiber::node,
      Fiber::nodeObject
    > fibers_;
    EmbeddedList<
      AsyncDescriptor,
      AsyncDescriptor::clusterListNode,
      AsyncDescriptor::clusterListNodeObject
    > descriptorsList_;
//    uintptr_t maxStackSize_;
    uintptr_t mfpt_;

    static uint8_t requester_[];
    static Requester & requester();

    void queue();
    void sweepFiber(Fiber * fiber);
    void detectMaxFiberStackSize();

    void attachDescriptor(AsyncDescriptor & descriptor,Fiber & toFiber);
    void detachDescriptor(AsyncDescriptor & descriptor);

    void threadExecute();
    void fiberExecute() {}

    mutable EmbeddedListNode<BaseThread> serverListNode_;
    static EmbeddedListNode<BaseThread> & serverListNode(const BaseThread & object){
      return object.serverListNode_;
    }
    static BaseThread & serverListNodeObject(const EmbeddedListNode<BaseThread> & node,BaseThread * p = NULL){
      return node.object(p->serverListNode_);
    }

    static void initialize();
    static void cleanup();
};
//---------------------------------------------------------------------------
inline BaseServer * const & BaseThread::server() const
{
  return server_;
}
//---------------------------------------------------------------------------
inline BaseThread & BaseThread::server(BaseServer * serverL)
{
  server_ = serverL;
  return *this;
}
//---------------------------------------------------------------------------
inline void BaseThread::postRequest(AsyncDescriptor * descriptor)
{
  requester().postRequest(descriptor);
}
//---------------------------------------------------------------------------
inline Requester & BaseThread::requester()
{
  return *reinterpret_cast<Requester *>(requester_);
}
//------------------------------------------------------------------------------
inline void Fiber::attachDescriptor(AsyncDescriptor & descriptor)
{
  thread_->attachDescriptor(descriptor,*this);
}
//------------------------------------------------------------------------------
inline void Fiber::detachDescriptor(AsyncDescriptor & descriptor)
{
  thread_->detachDescriptor(descriptor);
}
//------------------------------------------------------------------------------
inline Fiber * const Fiber::mainFiber() const
{
  return thread_;
}
//---------------------------------------------------------------------------
inline const uintptr_t & BaseThread::maxFibersPerThread() const
{
  return mfpt_;
}
//------------------------------------------------------------------------------
inline BaseThread & BaseThread::maxFibersPerThread(uintptr_t mfpt)
{
  mfpt_ = mfpt;
  return *this;
}
//------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class BaseServer {
  friend class Fiber;
  friend class BaseThread;
  public:
    virtual ~BaseServer();
    BaseServer();

    const uintptr_t & maxThreads() const;
    BaseServer &      maxThreads(uintptr_t mt);
    const uintptr_t & fiberStackSize() const;
    BaseServer &      fiberStackSize(uintptr_t size);
    BaseServer &      fiberTimeout(uint64_t fiberTimeout);
    const uint64_t &  fiberTimeout() const;
    void              DispatchWindowMessages(); // real work do it only under Windows, otherwise stub
    bool              active() const;

    enum HowCloseServer { 
      csWait = 1,
      csTerminate = 2,
      csShutdown = 4,
      csDWM = 8,
      csAbort = 16
    };

    const HowCloseServer & howCloseServer() const;
    BaseServer & howCloseServer(uintptr_t how);
    void closeServer();
  protected:
    virtual BaseThread * newThread();
    virtual Fiber * newFiber() = 0;
    virtual void attachFiber(const AutoPtr<Fiber> & fiber);
    virtual void maintainFiber(Fiber * fiber);
    void maintainFibers();
    void abortNotification(DirectoryChangeNotification * dcn = NULL);
    void sweepThreads();
    bool shutdown_;
  private:
    mutable InterlockedMutex mutex_;
    EmbeddedList<
      BaseThread,
      BaseThread::serverListNode,
      BaseThread::serverListNodeObject
    > threads_;
    uintptr_t mt_;
    uintptr_t fiberStackSize_;
    uint64_t fiberTimeout_;
    HowCloseServer howCloseServer_;

    BaseThread * selectThread();
};
//------------------------------------------------------------------------------
inline const BaseServer::HowCloseServer & BaseServer::howCloseServer() const
{
  return howCloseServer_;
}
//------------------------------------------------------------------------------
inline BaseServer & BaseServer::howCloseServer(uintptr_t how)
{
  howCloseServer_ = (HowCloseServer) how;
  return *this;
}
//------------------------------------------------------------------------------
inline const uintptr_t & BaseServer::maxThreads() const
{
  return mt_;
}
//------------------------------------------------------------------------------
inline BaseServer & BaseServer::maxThreads(uintptr_t mt)
{
  mt_ = mt;
  return *this;
}
//------------------------------------------------------------------------------
inline const uintptr_t & BaseServer::fiberStackSize() const
{
  return fiberStackSize_;
}
//------------------------------------------------------------------------------
inline BaseServer & BaseServer::fiberStackSize(uintptr_t size)
{
  fiberStackSize_ = size + (-intptr_t(size) & (sizeof(uintptr_t) - 1));
  return *this;
}
//------------------------------------------------------------------------------
inline BaseServer & BaseServer::fiberTimeout(uint64_t fiberTimeout)
{
  fiberTimeout_ = fiberTimeout + (fiberTimeout == 0);
  return *this;
}
//------------------------------------------------------------------------------
inline const uint64_t & BaseServer::fiberTimeout() const
{
  return fiberTimeout_;
}
//------------------------------------------------------------------------------
inline void BaseServer::abortNotification(DirectoryChangeNotification * dcn)
{
  BaseThread::requester().abortNotification(dcn);
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
#endif /* _fiber_H_ */
//------------------------------------------------------------------------------
