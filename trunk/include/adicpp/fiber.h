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
enum AsyncEventType { 
  etNone,
  etError,
  etOpenFile,
  etDirList,
  etCreateDir,
  etRemoveDir,
  etRemoveFile,
  etRename,
  etResolveName,
  etResolveAddress,
  etStat,
  etDirectoryChangeNotification,
  etLockFile,
  etRead,
  etWrite,
  etAccept,
  etConnect,
  etClose,
  etQuit,
  etDispatch,
  etTimer,
  etAcquire,
  etCount
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Fiber;
class AsyncDescriptor;
//---------------------------------------------------------------------------
class AsyncEvent {
  public:
    ~AsyncEvent();
    AsyncEvent();

    static EmbeddedListNode<AsyncEvent> & node(const AsyncEvent & object){
      return object.node_;
    }
    static AsyncEvent & nodeObject(const EmbeddedListNode<AsyncEvent> & node, AsyncEvent * p = NULL){
      return node.object(p->node_);
    }
  private:
    mutable EmbeddedListNode<AsyncEvent> node_;
  public:
    utf8::String string0_;
    utf8::String string1_;
#if defined(__WIN32__) || defined(__WIN64__)
    enum LockFileType { rdLock, wrLock, tryRDLock, tryWRLock };
    OVERLAPPED overlapped_;
#elif HAVE_KQUEUE && HAVE_STRUCT_AIOCB
    struct aiocb iocb_;
#else
#error async io not implemented because you system headers not have struct aiocb definition
#endif
#if _MSC_VER
#pragma warning(push,3)
#endif
    union {
      struct {
        union {
          uint64_t position_;
          uint64_t timerStartTime_;
        };
        union {
          void * buffer_;
          const void * cbuffer_;
#if defined(__WIN32__) || defined(__WIN64__)
          HANDLE fileDescriptor_;
          SOCKET socket_;
#else
          int fileDescriptor_;
          int socket_;
#endif
          FiberInterlockedMutex * mutex_;
          DirectoryChangeNotification * directoryChangeNotification_;
          struct Stat * stat_;
          struct {
            bool readOnly_;
            bool createIfNotExist_;
            bool exclusive_;
            bool recursive_;
            bool includeDirs_;
            bool abort_;
            bool rval_;
          };
        };
        union {
          uint64_t length_;
          uint64_t count_;
          uintptr_t data_;
          Vector<utf8::String> * dirList_;
          uint64_t timeout_;
        };
        LockFileType lockType_;
      };
      struct {
        ksock::SockAddr address_;
        uintptr_t defPort_;
      };
    };
#if _MSC_VER
#pragma warning(pop)
#endif
    Fiber * fiber_;
    AsyncDescriptor * descriptor_;
    int32_t errno_;
    AsyncEventType type_;
  private:
    AsyncEvent(const AsyncEvent &){}
    void operator = (const AsyncEvent &){}
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncDescriptorKey {
  public:
    ~AsyncDescriptorKey();
    AsyncDescriptorKey();
#if defined(__WIN32__) || defined(__WIN64__)
    AsyncDescriptorKey(SOCKET socket,int8_t specification = 0);
    AsyncDescriptorKey(HANDLE file,int8_t specification = 0);
#if _MSC_VER
#pragma warning(push,3)
#endif
    struct {
      union {
        SOCKET socket_;
        HANDLE file_;
        HANDLE descriptor_;
      };
      int8_t  specification_;
    };
#if _MSC_VER
#pragma warning(pop)
#endif
#else
    union {
      int socket_;
      int file_;
      int descriptor_;
    };
    AsyncDescriptorKey(int descriptor);
#endif
    bool                  hashKeyEqu(const AsyncDescriptorKey & key) const;
    uintptr_t             hash() const;
  protected:
  private:
    AsyncDescriptorKey(const AsyncDescriptorKey &){}
    void operator = (const AsyncDescriptorKey &){}
};
//---------------------------------------------------------------------------
inline AsyncDescriptorKey::~AsyncDescriptorKey()
{
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
inline AsyncDescriptorKey::AsyncDescriptorKey() :
  file_(INVALID_HANDLE_VALUE), specification_(0)
{
}
//---------------------------------------------------------------------------
inline AsyncDescriptorKey::AsyncDescriptorKey(SOCKET socket, int8_t specification)
  : socket_(socket), specification_(specification)
{
}
//---------------------------------------------------------------------------
inline AsyncDescriptorKey::AsyncDescriptorKey(HANDLE file, int8_t specification)
  : file_(file), specification_(specification)
{
}
//---------------------------------------------------------------------------
inline bool AsyncDescriptorKey::hashKeyEqu(const AsyncDescriptorKey & key) const
{
  return memcmp(&socket_, &key.socket_, (size_t) ((int8_t *) &specification_ - (int8_t *) &socket_ + 1)) == 0;
}
//---------------------------------------------------------------------------
inline uintptr_t AsyncDescriptorKey::hash() const
{
  return HF::hash(&socket_, (uintptr_t) ((int8_t *) &specification_ - (int8_t *) &socket_) + 1);
}
//---------------------------------------------------------------------------
#else
//---------------------------------------------------------------------------
inline AsyncDescriptorKey::AsyncDescriptorKey() : descriptor_(-1)
{
}
//---------------------------------------------------------------------------
inline AsyncDescriptorKey::AsyncDescriptorKey(int descriptor)
  : descriptor_(descriptor)
{
}
//---------------------------------------------------------------------------
inline bool AsyncDescriptorKey::hashKeyEqu(const AsyncDescriptorKey & key, bool) const
{
  return descriptor_ == key.descriptor_;
}
//---------------------------------------------------------------------------
inline uintptr_t AsyncDescriptorKey::hash(bool) const
{
  return HF::hash(descriptor_);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncDescriptor : public AsyncDescriptorKey {
  friend class Fiber;
  friend class AsyncIoSlave;
  friend class AsyncOpenFileSlave;
  friend class Requester;
  friend class BaseServer;
  friend class BaseThread;
  public:
    virtual ~AsyncDescriptor();
    AsyncDescriptor();

    AsyncDescriptor & attach();
    AsyncDescriptor & detach();

    Fiber * const & fiber() const;

    virtual bool isSocket() const;

#if defined(__WIN32__) || defined(__WIN64__)
    virtual int WSAEnumNetworkEvents(WSAEVENT hEventObject, DWORD event) = 0;
    virtual BOOL AcceptEx(SOCKET sAcceptSocket, PVOID lpOutputBuffer, DWORD dwReceiveDataLength, DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, LPDWORD lpdwBytesReceived, LPOVERLAPPED lpOverlapped) = 0;
    virtual BOOL Connect(HANDLE event, AsyncEvent * request) = 0;
    virtual BOOL Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) = 0;
    virtual BOOL Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) = 0;
    virtual BOOL GetOverlappedResult(LPOVERLAPPED lpOverlapped, LPDWORD lpNumberOfBytesTransferred, BOOL bWait, LPDWORD lpdwFlags = NULL) = 0;
#endif
  protected:
#if HAVE_KQUEUE
    virtual int accept() = 0;
    virtual void connect(ksys::IoRequest * request) = 0;
    virtual int64_t read2(void * buf, uint64_t len);
    virtual int64_t write2(const void * buf, uint64_t len);
#endif
    virtual void shutdown2();
    virtual void flush2();
  private:
    Fiber * fiber_;

    mutable EmbeddedListNode<AsyncDescriptor> fiberListNode_;
    static EmbeddedListNode<AsyncDescriptor> & fiberListNode(const AsyncDescriptor & object){
      return object.fiberListNode_;
    }
    static AsyncDescriptor & fiberListNodeObject(const EmbeddedListNode<AsyncDescriptor> & node,AsyncDescriptor * p = NULL){
      return node.object(p->fiberListNode_);
    }

    mutable EmbeddedListNode<AsyncDescriptor> clusterListNode_;
    static EmbeddedListNode<AsyncDescriptor> & clusterListNode(const AsyncDescriptor & object){
      return object.clusterListNode_;
    }
    static AsyncDescriptor & clusterListNodeObject(const EmbeddedListNode<AsyncDescriptor> & node,AsyncDescriptor * p = NULL){
      return node.object(p->clusterListNode_);
    }
};
//---------------------------------------------------------------------------
inline AsyncDescriptor::~AsyncDescriptor()
{
  assert( fiber_ == NULL );
}
//---------------------------------------------------------------------------
inline AsyncDescriptor::AsyncDescriptor() : fiber_(NULL)
{
}
//---------------------------------------------------------------------------
inline Fiber * const & AsyncDescriptor::fiber() const
{
  return fiber_;
}
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
    throw ExceptionSP(new Exception(err, __PRETTY_FUNCTION__));
  }
  return *this;
}
//---------------------------------------------------------------------------
inline Fiber & Fiber::convertThreadToFiber()
{
  assert( fiber_ == NULL );
  if( (fiber_ = ConvertThreadToFiber(this)) == NULL ){
    int32_t err = GetLastError() + errorOffset;
    throw ExceptionSP(new Exception(err, __PRETTY_FUNCTION__));
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
  currentFiber() = fiber;
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
  currentFiber()->attachDescriptor(*this);
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
class FiberInterlockedMutex {
  friend class AsyncAcquireSlave;
  public:
    ~FiberInterlockedMutex();
    FiberInterlockedMutex();

    void acquire();
    bool tryAcquire();
    void release();
  protected:
  private:
#if defined(__WIN32__) || defined(__WIN64__)
    HANDLE sem_;
    bool tryAcquireHelper();
#else
    InterlockedMutex mutex_;
#endif

    FiberInterlockedMutex(const FiberInterlockedMutex &){}
    void operator =(const FiberInterlockedMutex &){}

    bool internalAcquire(bool wait);
};
//---------------------------------------------------------------------------
#if !defined(__WIN32__) && !defined(__WIN64__)
inline FiberInterlockedMutex::~FiberInterlockedMutex()
{
}
#endif
//---------------------------------------------------------------------------
#if !defined(__WIN32__) && !defined(__WIN64__)
inline FiberInterlockedMutex::FiberInterlockedMutex()
{
}
#endif
//---------------------------------------------------------------------------
inline void FiberInterlockedMutex::acquire()
{
  internalAcquire(true);
}
//---------------------------------------------------------------------------
inline bool FiberInterlockedMutex::tryAcquire()
{
  return internalAcquire(false);
}
//---------------------------------------------------------------------------
#if !defined(__WIN32__) && !defined(__WIN64__)
inline void FiberInterlockedMutex::release()
{
  mutex_.release();
}
#endif
//------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class FiberMutex : protected FiberInterlockedMutex {
  public:
    ~FiberMutex();
    FiberMutex();

    FiberMutex & rdLock();
    bool tryRDLock();
    FiberMutex & wrLock();
    bool tryWRLock();
    FiberMutex & unlock();
  protected:
    FiberInterlockedMutex waitQueue_;
    int32_t value_;
    int32_t queue_;

    FiberMutex(const FiberMutex &){}
    void operator =(const FiberMutex &){}
  private:
};
//---------------------------------------------------------------------------
inline FiberMutex::~FiberMutex()
{
}
//---------------------------------------------------------------------------
inline FiberMutex::FiberMutex() : value_(0), queue_(0)
{
}
//---------------------------------------------------------------------------
inline bool FiberMutex::tryRDLock()
{
  acquire();
  bool r = value_ >= 0;
  if( r ) value_++;
  release();
  return r;
}
//---------------------------------------------------------------------------
inline bool FiberMutex::tryWRLock()
{
  acquire();
  bool r = value_ == 0;
  if( r ) value_ = -1;
  release();
  return r;
}
//---------------------------------------------------------------------------
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
    void cancelEvent(const Events & request);
#endif
    void abortNotification();
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
    void abortNotification();
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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class BaseServer {
  friend class Fiber;
  friend class BaseThread;
  public:
    virtual ~BaseServer();
    BaseServer();

    const uintptr_t & maxFibersPerThread() const;
    BaseServer &      maxFibersPerThread(uintptr_t mfpt);
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
    void sweepThreads();
  private:
    mutable InterlockedMutex mutex_;
    EmbeddedList<
      BaseThread,
      BaseThread::serverListNode,
      BaseThread::serverListNodeObject
    > threads_;
    uintptr_t mfpt_;
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
inline const uintptr_t & BaseServer::maxFibersPerThread() const
{
  return mfpt_;
}
//------------------------------------------------------------------------------
inline BaseServer & BaseServer::maxFibersPerThread(uintptr_t mfpt)
{
  mfpt_ = mfpt;
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
} // namespace ksys
//------------------------------------------------------------------------------
#endif /* _fiber_H_ */
//------------------------------------------------------------------------------
