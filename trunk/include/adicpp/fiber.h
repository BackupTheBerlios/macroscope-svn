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
#if defined(__WIN32__) || defined(__WIN64__)
const WM_ASYNCIO = WM_USER + 1;
#endif
//---------------------------------------------------------------------------
enum AsyncEventType {
  etNone,
  etError,
  etRead,
  etWrite,
  etAccept,
  etConnect,
  etClose,
  etQuit,
  etDispatch,
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
    union {
      Fiber * fiber_;
      AsyncDescriptor * descriptor_;
    };
    int32_t errno_;
    union {
      AsyncEventType evt_;
      int32_t event_;
    };
    union {
      int64_t count_;
      intptr_t data_;
    };

    ~AsyncEvent();
    AsyncEvent();

    static EmbeddedListNode<AsyncEvent> & node(const AsyncEvent & object){ return object.node_; }
  protected:
  private:
    mutable EmbeddedListNode<AsyncEvent> node_;
    AsyncEvent(const AsyncEvent &) {}
    AsyncEvent & operator = (const AsyncEvent &){ return *this; }
};
//---------------------------------------------------------------------------
inline AsyncEvent::~AsyncEvent()
{
}
//---------------------------------------------------------------------------
inline AsyncEvent::AsyncEvent() : event_(etNone)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class IoRequest {
  public:
    Thread * ioThread_;
#if defined(__WIN32__) || defined(__WIN64__)
    OVERLAPPED overlapped_;
    intptr_t ehi_;
#elif HAVE_KQUEUE
#if HAVE_STRUCT_AIOCB
    struct aiocb iocb_;
#else
#error async io not implemented because you system headers not have struct aiocb definition
#endif
#endif
    AsyncDescriptor * descriptor_;
    union {
      uint64_t position_;
      union {
        void * buffer_;
        const void * cbuffer_;
      };
      uint64_t length_;
      ksock::SockAddr address_;
    };
    AsyncEventType ioType_;

    ~IoRequest();
    IoRequest();

    static EmbeddedListNode<IoRequest> & node(const IoRequest & object){ return object.node_; }
  protected:
  private:
    mutable EmbeddedListNode<IoRequest> node_;
    IoRequest(const IoRequest &) {}
    IoRequest & operator = (const IoRequest &){ return *this; }
};
//---------------------------------------------------------------------------
inline IoRequest::~IoRequest()
{
}
//---------------------------------------------------------------------------
inline IoRequest::IoRequest() : ioThread_(NULL), descriptor_(NULL), ioType_(etNone)
{
#if HAVE_KQUEUE
  memset(&iocb_,0,sizeof(iocb_));
#endif
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Fiber {
  friend void initialize();
  friend void cleanup();
  friend class AsyncDescriptor;
  friend class AsyncDescriptorsCluster;
  public:
    virtual ~Fiber();
    Fiber();

    const bool & started() const;
    const bool & terminated() const;
    Fiber & terminate();
    const bool & finished() const;

    void switchFiber(Fiber * fiber) GNUG_NOTHROW;
    void switchFiber(Fiber & fiber) GNUG_NOTHROW;

    Fiber & mainFiber() const;

    Fiber & allocateStack(void * ip,void * param,size_t size,Fiber * mainFiber,uintptr_t dummy1 = 0,uintptr_t dummy2 = 0);
#if defined(__WIN32__) || defined(__WIN64__)
    Fiber & createFiber(DWORD dwStackSize);
    Fiber & deleteFiber();
    Fiber & clearFiber();
    Fiber & convertThreadToFiber();
#endif
    const AsyncEvent & event() const;
    const IoRequest & io() const;
  protected:
    bool started_;
    bool terminated_;
    bool finished_;
  private:
    Fiber * mainFiber_;
#if defined(__WIN32__) || defined(__WIN64__)
    static VOID WINAPI start(Fiber * fiber);
    LPVOID fiber_;
    void * ip_;
    void * param_;
#else
    static void start(Fiber * fiber,void * param,void (* ip)(void *)) GNUG_NOTHROW_CDECL;
    AutoPtr<uint8_t> stack_;
    void * stackPointer_;
    void switchFiber2(Fiber * fiber) GNUG_NOTHROW_CDECL;
#endif
  private:
    AsyncEvent event_;
    IoRequest io_;
    
    static void initialize();
    static void cleanup();
    Fiber & attach(AsyncDescriptor & descriptor);
    Fiber & detach(AsyncDescriptor & descriptor);
};
//---------------------------------------------------------------------------
inline Fiber::~Fiber()
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( fiber_ != NULL ) DeleteFiber(fiber_);
#endif
}
//---------------------------------------------------------------------------
inline Fiber::Fiber() :
  started_(false), terminated_(false), finished_(false),
  mainFiber_(NULL),
#if !defined(__WIN32__) && !defined(__WIN64__)
  stackPointer_(NULL)
#else
  fiber_(NULL), ip_(NULL), param_(NULL)
#endif
{
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
inline void Fiber::switchFiber(Fiber & fiber)
{
  switchFiber(&fiber);
}
//---------------------------------------------------------------------------
inline Fiber & Fiber::mainFiber() const
{
  return *mainFiber_;
}
//---------------------------------------------------------------------------
inline const AsyncEvent & Fiber::event() const
{
  return event_;
}
//---------------------------------------------------------------------------
inline const IoRequest & Fiber::io() const
{
  return io_;
}
//---------------------------------------------------------------------------
extern uint8_t currentFiberPlaceHolder[];
inline ThreadLocalVariable<Fiber> & currentFiber()
{
  return *reinterpret_cast<ThreadLocalVariable<Fiber> *>(currentFiberPlaceHolder);
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
inline void Fiber::switchFiber(Fiber * fiber)
{
  currentFiber() = fiber;
  SwitchToFiber(fiber->fiber_);
}
//---------------------------------------------------------------------------
inline Fiber & Fiber::createFiber(DWORD dwStackSize)
{
  assert( fiber_ == NULL );
  fiber_ = CreateFiber(dwStackSize,(LPFIBER_START_ROUTINE) start,this);
  if( fiber_ == NULL ){
    int32_t err = GetLastError() + errorOffset;
    throw ExceptionSP(new Exception(err,utf8::string(__PRETTY_FUNCTION__)));
  }
  return *this;
}
//---------------------------------------------------------------------------
inline Fiber & Fiber::convertThreadToFiber()
{
  assert( fiber_ == NULL );
  if( (fiber_ = ConvertThreadToFiber(NULL)) == NULL ){
    int32_t err = GetLastError() + errorOffset;
    throw ExceptionSP(new Exception(err,utf8::string(__PRETTY_FUNCTION__)));
  }
  return *this;
}
//---------------------------------------------------------------------------
inline Fiber & Fiber::deleteFiber()
{
  DeleteFiber(fiber_);
  fiber_ = NULL;
  finished_ = true;
  return *this;
}
//---------------------------------------------------------------------------
inline Fiber & Fiber::clearFiber()
{
  fiber_ = NULL;
  finished_ = true;
  return *this;
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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncDescriptorKey {
  public:
    ~AsyncDescriptorKey();
    AsyncDescriptorKey();
#if defined(__WIN32__) || defined(__WIN64__)
    AsyncDescriptorKey(SOCKET socket,int8_t specification);
    AsyncDescriptorKey(HANDLE file,int8_t specification);
    AsyncDescriptorKey(const AsyncDescriptorKey & key);
    AsyncDescriptorKey & operator = (const AsyncDescriptorKey & key);
    struct {
      union {
        SOCKET socket_;
        HANDLE file_;
        int descriptor_;
      };
      int8_t specification_;
    };
#else
    union {
      int socket_;
      int file_;
      int descriptor_;
    };
    AsyncDescriptorKey(int descriptor);
    AsyncDescriptorKey & operator = (const AsyncDescriptorKey & key);
#endif
    bool hashKeyEqu(const AsyncDescriptorKey & key,bool) const;
    uintptr_t hash(bool) const;
  protected:
  private:
};
//---------------------------------------------------------------------------
inline AsyncDescriptorKey::~AsyncDescriptorKey()
{
}
//---------------------------------------------------------------------------
inline AsyncDescriptorKey::AsyncDescriptorKey()
{
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
inline AsyncDescriptorKey::AsyncDescriptorKey(SOCKET socket,int8_t specification) : socket_(socket), specification_(specification)
{
}
//---------------------------------------------------------------------------
inline AsyncDescriptorKey::AsyncDescriptorKey(HANDLE file,int8_t specification) : file_(file), specification_(specification)
{
}
//---------------------------------------------------------------------------
inline AsyncDescriptorKey::AsyncDescriptorKey(const AsyncDescriptorKey & key)
{
  memcpy(&socket_,&key.socket_,(size_t) ((int8_t *) &specification_ - (int8_t *) &socket_ + 1)); 
}
//---------------------------------------------------------------------------
inline AsyncDescriptorKey & AsyncDescriptorKey::operator = (const AsyncDescriptorKey & key)
{
  memcpy(&socket_,&key.socket_,(size_t) ((int8_t *) &specification_ - (int8_t *) &socket_ + 1)); 
  return *this;
}
//---------------------------------------------------------------------------
inline bool AsyncDescriptorKey::hashKeyEqu(const AsyncDescriptorKey & key,bool) const 
{ 
  return memcmp(&socket_,&key.socket_,(size_t) ((int8_t *) &specification_ - (int8_t *) &socket_ + 1)) == 0; 
}
//---------------------------------------------------------------------------
inline uintptr_t AsyncDescriptorKey::hash(bool) const 
{
  return HF::hash(&socket_,(uintptr_t) ((int8_t *) &specification_ - (int8_t *) &socket_) + 1);
}
//---------------------------------------------------------------------------
#else
//---------------------------------------------------------------------------
inline AsyncDescriptorKey::AsyncDescriptorKey(int descriptor) : descriptor_(descriptor)
{
}
//---------------------------------------------------------------------------
inline AsyncDescriptorKey & AsyncDescriptorKey::operator = (const AsyncDescriptorKey & key)
{
  descriptor_ = key.descriptor_;
  return *this;
}
//---------------------------------------------------------------------------
inline bool AsyncDescriptorKey::hashKeyEqu(const AsyncDescriptorKey & key,bool) const 
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
  friend class AsyncDescriptorsCluster;
  friend class BaseServer;
  public:
    virtual ~AsyncDescriptor();
    AsyncDescriptor();

    AsyncDescriptor & attach(Fiber & fiber,AsyncDescriptorsCluster & cluster);
    AsyncDescriptor & detach();

    Fiber * const & fiber() const;
    AsyncDescriptorsCluster * const & cluster() const;

#if defined(__WIN32__) || defined(__WIN64__)
    virtual int WSAEnumNetworkEvents(WSAEVENT hEventObject,DWORD event) = 0;
    virtual BOOL AcceptEx(
      SOCKET sAcceptSocket,
      PVOID lpOutputBuffer,
      DWORD dwReceiveDataLength,
      DWORD dwLocalAddressLength,
      DWORD dwRemoteAddressLength,
      LPDWORD lpdwBytesReceived,
      LPOVERLAPPED lpOverlapped
    ) = 0;
    virtual BOOL Connect(HANDLE event,IoRequest * request) = 0;
    virtual BOOL Read(
      LPVOID lpBuffer,
      DWORD nNumberOfBytesToRead,
      LPDWORD lpNumberOfBytesRead,
      LPOVERLAPPED lpOverlapped
    ) = 0;
    virtual BOOL Write(
      LPCVOID lpBuffer,
      DWORD nNumberOfBytesToWrite,
      LPDWORD lpNumberOfBytesWritten,
      LPOVERLAPPED lpOverlapped
    ) = 0;
    virtual BOOL GetOverlappedResult(
      LPOVERLAPPED lpOverlapped,
      LPDWORD lpNumberOfBytesTransferred,
      BOOL bWait,
      LPDWORD lpdwFlags = NULL
    ) = 0;
#elif HAVE_KQUEUE
    virtual int accept() = 0;
    virtual void connect(ksys::IoRequest * request) = 0;
#endif
  protected:
    virtual void shutdown2();
    virtual void flush2();
  private:
    Fiber * fiber_;
    AsyncDescriptorsCluster * cluster_;
};
//---------------------------------------------------------------------------
inline AsyncDescriptor::~AsyncDescriptor()
{
}
//---------------------------------------------------------------------------
inline AsyncDescriptor::AsyncDescriptor() : fiber_(NULL), cluster_(NULL)
{
}
//---------------------------------------------------------------------------
inline Fiber * const & AsyncDescriptor::fiber() const
{
  return fiber_;
}
//---------------------------------------------------------------------------
inline AsyncDescriptorsCluster * const & AsyncDescriptor::cluster() const
{
  return cluster_;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncIoSlave : public Thread, public Semaphore, public InterlockedMutex {
  friend class AsyncDescriptorsCluster;
  public:
    ~AsyncIoSlave();
    AsyncIoSlave();

    AsyncIoSlave & transplant(EmbeddedList<IoRequest,IoRequest::node> & requests);
    bool ready();
#if HAVE_KQUEUE
    AsyncIoSlave & cancelEvent(const IoRequest & request);
#endif
  protected:
  private:
    EmbeddedList<IoRequest,IoRequest::node> requests_;
    EmbeddedList<IoRequest,IoRequest::node> newRequests_;
#if defined(__WIN32__) || defined(__WIN64__)
    HANDLE events_[MAXIMUM_WAIT_OBJECTS];
    EmbeddedListNode<IoRequest> * eReqs_[MAXIMUM_WAIT_OBJECTS];
#elif HAVE_KQUEUE
    int kqueue_;
    Array<struct kevent> kevents_;
#else
#error async io not implemented
#endif
    void execute();
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncIoMaster : public Thread {
  friend class AsyncDescriptorsCluster;
  public:
    ~AsyncIoMaster();
    AsyncIoMaster();
  protected:
  private:
    Vector<AsyncIoSlave> slaves_;
    void execute();
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncDescriptorsCluster {
  friend void initialize();
  friend void cleanup();
  friend class AsyncDescriptor;
  friend class AsyncIoMaster;
  public:
    ~AsyncDescriptorsCluster();
    AsyncDescriptorsCluster();
    AsyncDescriptorsCluster & queue();
    AsyncDescriptorsCluster & postEventNB(int32_t event,Fiber * fiber);
    AsyncDescriptorsCluster & postEventNB(AsyncDescriptor * descriptor,int32_t errNo,int32_t event,int64_t count);
    AsyncDescriptorsCluster & postEvent(int32_t event,Fiber * fiber);
    AsyncDescriptorsCluster & postEvent(AsyncDescriptor * descriptor,int32_t errNo,int32_t event,int64_t count);
  protected:
    Semaphore semaphore_;
    InterlockedMutex eventMutex_;
    EmbeddedList<AsyncEvent,AsyncEvent::node> events_;
    HashedObjectList<AsyncDescriptorKey,AsyncDescriptor> descriptors_;
  private:
    bool waitForEvent(AsyncDescriptor * & descriptor,AsyncEvent & ev);

    static uint8_t mutex_[];
    static InterlockedMutex & mutex();
//    static uint8_t descriptorsPH_[];
//    static HashedObjectList<AsyncDescriptorKey,AsyncDescriptor> & descriptors();
#if defined(__WIN32__) || defined(__WIN64__)
    static HWND hWnd_;
    static LRESULT CALLBACK wndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

//    class MessagesController : public Thread {
//      friend class AsyncDescriptorsCluster;
//      public:
//        ~MessagesController();
//        MessagesController();
//      protected:
//      private:
//        bool messageLoop_;
//        void execute();
//    };
//    friend class MessagesController;
//    static uint8_t messagesController_[sizeof(MessagesController)];
//    static MessagesController & messagesController();
#endif
    static uintptr_t hWndRefCount_;

    static uint8_t ioMaster_[];
    static uint8_t ioRequests_[];
    static uint8_t ioRequestsSemaphore_[];
    static uint8_t ioRequestsMutex_[];

    static AsyncIoMaster & ioMaster();
    static EmbeddedList<IoRequest,IoRequest::node> & ioRequests();
    static Semaphore & ioRequestsSemaphore();
    static InterlockedMutex & ioRequestsMutex();

#if defined(__WIN32__) || defined(__WIN64__)
    static VOID CALLBACK fileIOCompletionRoutine(
      DWORD dwErrorCode,
      DWORD dwNumberOfBytesTransfered,
      LPOVERLAPPED lpOverlapped
    );
    static VOID CALLBACK socketIOCompletionRoutine(
      DWORD dwError,
      DWORD cbTransferred,
      LPWSAOVERLAPPED lpOverlapped,
      DWORD dwFlags
    );
#endif
    static void allocateSig();
    static void deallocateSig();
    static void initialize();
    static void cleanup();
    AsyncDescriptorsCluster & attach(AsyncDescriptor & descriptor);
    AsyncDescriptorsCluster & detach(AsyncDescriptor & descriptor);
  public:
#if defined(__WIN32__) || defined(__WIN64__)
    static const HWND & hWnd();
#endif
    AsyncDescriptorsCluster & postIoRequest(
      AsyncDescriptor * descriptor,
      uint64_t position,
      const void * buffer,
      uint64_t length,
      AsyncEventType ioType
    );
    AsyncDescriptorsCluster & postIoRequest(
      AsyncDescriptor * descriptor,const ksock::SockAddr & addr
    );
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline bool AsyncIoSlave::ready()
{
  AutoLock<InterlockedMutex> lock(*this);
  return !tryWait() && requests_.count() == 0 && newRequests_.count() == 0;
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
inline const HWND & AsyncDescriptorsCluster::hWnd()
{
  return hWnd_;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
inline AsyncDescriptorsCluster & AsyncDescriptorsCluster::postIoRequest(
  AsyncDescriptor * descriptor,
  uint64_t position,
  const void * buffer,
  uint64_t length,
  AsyncEventType ioType)
{
  assert( descriptor != NULL && descriptor->fiber_ != NULL );
  bool post;
  AutoLock<InterlockedMutex> lock(ioRequestsMutex());
  post = ioRequests().count() == 0;
  descriptor->fiber_->io_.descriptor_ = descriptor;
  descriptor->fiber_->io_.position_ = position;
  descriptor->fiber_->io_.cbuffer_ = buffer;
  descriptor->fiber_->io_.length_ = length;
  descriptor->fiber_->io_.ioType_ = ioType;
  ioRequests().insToTail(descriptor->fiber_->io_);
  if( post ) ioRequestsSemaphore().post();
  return *this;
}
//---------------------------------------------------------------------------
inline AsyncDescriptorsCluster & AsyncDescriptorsCluster::postIoRequest(
  AsyncDescriptor * descriptor,const ksock::SockAddr & addr)
{
  assert( descriptor != NULL && descriptor->fiber_ != NULL );
  bool post;
  AutoLock<InterlockedMutex> lock(ioRequestsMutex());
  post = ioRequests().count() == 0;
  descriptor->fiber_->io_.descriptor_ = descriptor;
  descriptor->fiber_->io_.ioType_ = etConnect;
  descriptor->fiber_->io_.address_ = addr;
  ioRequests().insToTail(descriptor->fiber_->io_);
  if( post ) ioRequestsSemaphore().post();
  return *this;
}
//---------------------------------------------------------------------------
inline AsyncIoMaster & AsyncDescriptorsCluster::ioMaster()
{
  return *reinterpret_cast<AsyncIoMaster *>(ioMaster_);
}
//---------------------------------------------------------------------------
inline EmbeddedList<IoRequest,IoRequest::node> & AsyncDescriptorsCluster::ioRequests()
{
  return *reinterpret_cast<EmbeddedList<IoRequest,IoRequest::node> *>(ioRequests_);
}
//---------------------------------------------------------------------------
inline Semaphore & AsyncDescriptorsCluster::ioRequestsSemaphore()
{
  return *reinterpret_cast<Semaphore *>(ioRequestsSemaphore_);
}
//---------------------------------------------------------------------------
inline InterlockedMutex & AsyncDescriptorsCluster::ioRequestsMutex()
{
  return *reinterpret_cast<InterlockedMutex *>(ioRequestsMutex_);
}
//---------------------------------------------------------------------------
inline InterlockedMutex & AsyncDescriptorsCluster::mutex()
{
  return *reinterpret_cast<InterlockedMutex *>(mutex_);
}
//---------------------------------------------------------------------------
inline AsyncDescriptorsCluster & AsyncDescriptorsCluster::
  postEventNB(int32_t event,Fiber * fiber)
{
  fiber->event_.event_ = event;
  fiber->event_.fiber_ = fiber;
  events_.insToTail(fiber->event_);
  semaphore_.post();
  return *this;
}
//---------------------------------------------------------------------------
inline AsyncDescriptorsCluster & AsyncDescriptorsCluster::
  postEventNB(AsyncDescriptor * descriptor,int32_t errNo,int32_t event,int64_t count)
{
  descriptor->fiber_->event_.descriptor_ = descriptor;
  descriptor->fiber_->event_.errno_ = errNo;
  descriptor->fiber_->event_.event_ = event;
  descriptor->fiber_->event_.count_ = count;
  events_.insToTail(descriptor->fiber_->event_);
  semaphore_.post();
  return *this;
}
//---------------------------------------------------------------------------

inline AsyncDescriptorsCluster & AsyncDescriptorsCluster::
  postEvent(int32_t event,Fiber * fiber)
{
  AutoLock<InterlockedMutex> lock(eventMutex_);
  fiber->event_.event_ = event;
  fiber->event_.fiber_ = fiber;
  events_.insToTail(fiber->event_);
  semaphore_.post();
  return *this;
}
//---------------------------------------------------------------------------
inline AsyncDescriptorsCluster & AsyncDescriptorsCluster::
  postEvent(AsyncDescriptor * descriptor,int32_t errNo,int32_t event,int64_t count)
{
  AutoLock<InterlockedMutex> lock(eventMutex_);
  descriptor->fiber_->event_.descriptor_ = descriptor;
  descriptor->fiber_->event_.errno_ = errNo;
  descriptor->fiber_->event_.event_ = event;
  descriptor->fiber_->event_.count_ = count;
  events_.insToTail(descriptor->fiber_->event_);
  semaphore_.post();
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class BaseFiber : public Fiber {
  friend class BaseThread;
  friend class BaseServer;
  friend class FiberInterlockedMutex;
  friend class FiberMutex;
  friend class FiberSemaphore;
  public:
    virtual ~BaseFiber();
    BaseFiber();

    BaseThread * const & thread() const;
    BaseFiber & thread(BaseThread * threadL);
  protected:
    virtual void execute() = 0;
  private:
    BaseThread * thread_;

    static void fiber2(BaseFiber * fiber);
    BaseFiber & dead();

    mutable EmbeddedListNode<BaseFiber> ipc_;
    static EmbeddedListNode<BaseFiber> & ipc(const BaseFiber & object){ return object.ipc_; }
    mutable EmbeddedListNode<BaseFiber> dead_;
    static EmbeddedListNode<BaseFiber> & dead(const BaseFiber & object){ return object.dead_; }
};
//---------------------------------------------------------------------------
inline BaseFiber::~BaseFiber()
{
}
//---------------------------------------------------------------------------
inline BaseFiber::BaseFiber() : thread_(NULL)
{
}
//---------------------------------------------------------------------------
inline BaseThread * const & BaseFiber::thread() const
{
  return thread_;
}
//---------------------------------------------------------------------------
inline BaseFiber & BaseFiber::thread(BaseThread * threadL)
{
  thread_ = threadL;
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class BaseThread : public Thread, public AsyncDescriptorsCluster, public Fiber {
  friend class BaseFiber;
  friend class BaseServer;
  public:
    virtual ~BaseThread();
    BaseThread();

    BaseServer * const & server() const;
    BaseThread & server(BaseServer * serverL);
  protected:
    InterlockedMutex mutex_;
    HashedObjectList<HashedObjectListKey<BaseFiber *>,BaseFiber> fibers_;
    EmbeddedList<BaseFiber,BaseFiber::dead> deadFibers_;
  private:
    BaseServer * server_;

    void execute();
};
//---------------------------------------------------------------------------
inline BaseThread::~BaseThread()
{
}
//---------------------------------------------------------------------------
inline BaseThread::BaseThread() : server_(NULL)
{
  descriptors_.ownsObjects(false);
}
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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class BaseServer {
  friend class BaseFiber;
  friend class BaseThread;
  public:
    virtual ~BaseServer();
    BaseServer();

    const uintptr_t & maxFibersPerThread() const;
    BaseServer & maxFibersPerThread(uintptr_t mfpt);
    const uintptr_t & maxThreads() const;
    BaseServer & maxThreads(uintptr_t mt);
    const uintptr_t & fiberStackSize() const;
    BaseServer & fiberStackSize(uintptr_t size);
    BaseServer & fiberTimeout(int64_t fiberTimeout);
    const int64_t & fiberTimeout() const;

    bool active() const;
  protected:
    virtual BaseThread * newThread();
    virtual BaseFiber * newFiber() = 0;
    virtual void attachFiber(BaseFiber & fiber);
    void closeServer();
    void sweepFibers();
  private:
    mutable InterlockedMutex mutex_;
    HashedObjectList<HashedObjectListKey<BaseThread *>,BaseThread> threads_;
    uintptr_t mfpt_;
    uintptr_t mt_;
    uintptr_t fiberStackSize_;
    int64_t fiberTimeout_;

    BaseThread * selectThread();
};
//------------------------------------------------------------------------------
inline BaseServer::~BaseServer()
{
}
//------------------------------------------------------------------------------
inline BaseServer::BaseServer() :
  mfpt_(8),
  mt_(numberOfProcessors()),
  fiberStackSize_(getpagesize()),
  fiberTimeout_(10000000)
{
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
  fiberStackSize_ = size;
  return *this;
}
//------------------------------------------------------------------------------
inline BaseServer & BaseServer::fiberTimeout(int64_t fiberTimeout)
{
  fiberTimeout_ = fiberTimeout;
  return *this;
}
//------------------------------------------------------------------------------
inline const int64_t & BaseServer::fiberTimeout() const
{
  return fiberTimeout_;
}
//------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class FiberInterlockedMutex {
  public:
    ~FiberInterlockedMutex();
    FiberInterlockedMutex();

    void acquire();
    bool tryAcquire();
    void release();
  protected:
  private:
    InterlockedMutex main_;
    InterlockedMutex mutex_;
    EmbeddedList<BaseFiber,BaseFiber::ipc> wait_;
    FiberInterlockedMutex(const FiberInterlockedMutex &) {}
    FiberInterlockedMutex & operator = (const FiberInterlockedMutex &) { return *this; }
};
//---------------------------------------------------------------------------
inline FiberInterlockedMutex::~FiberInterlockedMutex()
{
  assert( wait_.first() == NULL && wait_.last() == NULL );
}
//---------------------------------------------------------------------------
inline FiberInterlockedMutex::FiberInterlockedMutex()
{
}
//---------------------------------------------------------------------------
inline bool FiberInterlockedMutex::tryAcquire()
{
  return main_.tryAcquire();
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class FiberMutex {
  public:
    ~FiberMutex();
    FiberMutex();

    FiberMutex & rdLock();
    bool tryRDLock();
    FiberMutex & wrLock();
    bool tryWRLock();
    FiberMutex & unlock();
  protected:
  private:
    Mutex main_;
    InterlockedMutex mutex_;
    EmbeddedList<BaseFiber,BaseFiber::ipc> wait_;
    FiberMutex(const FiberMutex &) {}
    FiberMutex & operator = (const FiberMutex &) { return *this; }
};
//---------------------------------------------------------------------------
inline FiberMutex::~FiberMutex()
{
  assert( wait_.first() == NULL && wait_.last() == NULL );
}
//---------------------------------------------------------------------------
inline FiberMutex::FiberMutex()
{
}
//---------------------------------------------------------------------------
inline bool FiberMutex::tryRDLock()
{
  return main_.tryRDLock();
}
//---------------------------------------------------------------------------
inline bool FiberMutex::tryWRLock()
{
  return main_.tryWRLock();
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class FiberSemaphore {
  public:
    ~FiberSemaphore();
    FiberSemaphore();

    void post();
    void wait();
    bool tryWait();
  protected:
  private:
    Semaphore semaphore_;
    InterlockedMutex mutex_;
    EmbeddedList<BaseFiber,BaseFiber::ipc> wait_;
    FiberSemaphore(const FiberSemaphore &) {}
    FiberSemaphore & operator = (const FiberSemaphore &) { return *this; }
};
//---------------------------------------------------------------------------
inline FiberSemaphore::~FiberSemaphore()
{
  assert( wait_.first() == NULL && wait_.last() == NULL );
}
//---------------------------------------------------------------------------
inline FiberSemaphore::FiberSemaphore()
{
}
//---------------------------------------------------------------------------
inline bool FiberSemaphore::tryWait()
{
  return semaphore_.tryWait();
}
//---------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
#endif /* _fiber_H_ */
//------------------------------------------------------------------------------
