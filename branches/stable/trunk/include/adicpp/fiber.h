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
  etLockFile,
  etDirList,
  etCreateDir,
  etRemoveDir,
  etRemoveFile,
  etRename,
  etResolveName,
  etResolveAddress,
  etStat,
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
#else
          int fileDescriptor_;
#endif
          FiberInterlockedMutex * mutex_;
          struct Stat * stat_;
          struct {
            bool readOnly_;
            bool createIfNotExist_;
            bool exclusive_;
            bool recursive_;
            bool includeDirs_;
            bool abortTimer_;
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
  friend class AsyncDescriptorsCluster;
  friend class AsyncIoSlave;
  friend class AsyncOpenFileSlave;
  friend class Requester;
  friend class BaseServer;
  friend class BaseThread;
  friend class BaseFiber;
  public:
    virtual ~AsyncDescriptor();
    AsyncDescriptor();

    AsyncDescriptor & attach();
    AsyncDescriptor & detach();

    Fiber * const & fiber() const;
    AsyncDescriptor & fiber(Fiber * fiber);
    AsyncDescriptorsCluster * const & cluster() const;
    AsyncDescriptor & cluster(AsyncDescriptorsCluster * cluster);

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
    AsyncDescriptorsCluster * cluster_;

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
  assert( fiber_ == NULL && cluster_ == NULL );
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
inline AsyncDescriptor & AsyncDescriptor::fiber(Fiber * fiber)
{
  fiber_ = fiber;
  return *this;
}
//---------------------------------------------------------------------------
inline AsyncDescriptorsCluster * const & AsyncDescriptor::cluster() const
{
  return cluster_;
}
//---------------------------------------------------------------------------
inline AsyncDescriptor & AsyncDescriptor::cluster(AsyncDescriptorsCluster * cluster)
{
  cluster_ = cluster;
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Fiber {
  friend void initialize();
  friend void cleanup();
  friend class Requester;
  friend class AsyncDescriptor;
  friend class AsyncDescriptorsCluster;
  friend class BaseFiber;
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
    Fiber & createFiber(uintptr_t dwStackSize);
    Fiber & deleteFiber();
    Fiber & clearFiber();
    Fiber & convertThreadToFiber();
    LPVOID fiber() const;
#endif
    AsyncEvent & event() const;
    virtual void attachDescriptor(AsyncDescriptor & descriptor);
    virtual void detachDescriptor(AsyncDescriptor & descriptor);
  protected:
    bool started_;
    bool terminated_;
    bool finished_;
  private:
    Fiber * mainFiber_;
#if defined(__WIN32__) || defined(__WIN64__)
    static VOID WINAPI start(Fiber * fiber);
    LPVOID              fiber_;
    void *              ip_;
    void *              param_;
#else
    static void start(Fiber * fiber, void * param, void (*ip) (void *)) GNUG_NOTHROW_CDECL;
    AutoPtr<uint8_t> stack_;
    void * stackPointer_;
    void switchFiber2(Fiber * fiber) GNUG_NOTHROW_CDECL;
#endif
    EmbeddedList<
      AsyncDescriptor,
      AsyncDescriptor::fiberListNode,
      AsyncDescriptor::fiberListNodeObject
    > descriptorsList_;
    mutable AsyncEvent event_;

    static void initialize();
    static void cleanup();
};
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
inline AsyncEvent & Fiber::event() const
{
  return event_;
}
//---------------------------------------------------------------------------
extern uint8_t currentFiberPlaceHolder[];
inline ThreadLocalVariable<Fiber> & currentFiber()
{
  return *reinterpret_cast<ThreadLocalVariable< Fiber> *>(currentFiberPlaceHolder);
}
//---------------------------------------------------------------------------
inline bool isRunInFiber()
{
  return currentFiber() != NULL;
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
inline Fiber & Fiber::createFiber(uintptr_t dwStackSize)
{
  assert( fiber_ == NULL );
#ifdef __WIN64__
  fiber_ = CreateFiber(dwStackSize, (LPFIBER_START_ROUTINE) start, this);
#else
  fiber_ = CreateFiber((DWORD) dwStackSize, (LPFIBER_START_ROUTINE) start, this);
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
  if( (fiber_ = ConvertThreadToFiber(NULL)) == NULL ){
    int32_t err = GetLastError() + errorOffset;
    throw ExceptionSP(new Exception(err, __PRETTY_FUNCTION__));
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
inline LPVOID Fiber::fiber() const
{
  return fiber_;
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
typedef EmbeddedList<AsyncEvent,AsyncEvent::node,AsyncEvent::nodeObject> Events;
typedef EmbeddedListNode<AsyncEvent> EventsNode;
//---------------------------------------------------------------------------
class AsyncIoSlave : public Thread, public Semaphore, public InterlockedMutex {
  public:
    virtual ~AsyncIoSlave();
    AsyncIoSlave();

    void transplant(Events & requests);
#if HAVE_KQUEUE
    void cancelEvent(const Events & request);
#endif
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
    void            execute();
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncOpenFileSlave : public Thread, public Semaphore, public InterlockedMutex {
  public:
    virtual ~AsyncOpenFileSlave();
    AsyncOpenFileSlave();

    void transplant(Events & requests);
  protected:
  private:
    AsyncOpenFileSlave(const AsyncOpenFileSlave &){}
    void operator = (const AsyncOpenFileSlave &){}

    Events requests_;

    void execute();
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncTimerSlave : public Thread, public Semaphore, public InterlockedMutex {
  public:
    virtual ~AsyncTimerSlave();
    AsyncTimerSlave();

    void transplant(Events & requests);
    void abortTimer();
  protected:
  private:
    AsyncTimerSlave(const AsyncTimerSlave &){}
    void operator = (const AsyncTimerSlave &){}

    Events requests_;

    void execute();
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncAcquireSlave : public Thread, public Semaphore, public InterlockedMutex {
  public:
    virtual ~AsyncAcquireSlave();
    AsyncAcquireSlave();

    void transplant(Events & requests);
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

    void execute();
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Requester : public Thread, public Semaphore, public InterlockedMutex {
  friend class AsyncDescriptorsCluster;
  public:
    virtual ~Requester();
    Requester();

    void abortTimer();
  protected:
  private:
    Events ioRequests_;
    Vector<AsyncIoSlave> slaves_;
    Events ofRequests_;
    Vector<AsyncOpenFileSlave> ofSlaves_;
    Events timerRequests_;
    AutoPtr<AsyncTimerSlave> timerSlave_;
    Events acquireRequests_;
    Vector<AsyncAcquireSlave> acquireSlaves_;

    void execute();
    void postRequest(AsyncDescriptor * descriptor,uint64_t position,const void * buffer,uint64_t length,AsyncEventType ioType);
    void postRequest(AsyncDescriptor * descriptor,const ksock::SockAddr & addr);
    void postRequest(AsyncDescriptor * descriptor,const utf8::String & fileName,bool createIfNotExist,bool exclusive,bool readOnly);
    void postRequest(AsyncDescriptor * descriptor,uint64_t position,uint64_t length,AsyncEvent::LockFileType lockType);
    void postRequest(Vector<utf8::String> * dirList,const utf8::String & dirAndMask,const utf8::String & exMask,bool recursive,bool includeDirs);
    void postRequest(AsyncEventType event,const utf8::String & name,bool recursive);
    void postRequest(const utf8::String & name,uintptr_t defPort);
    void postRequest(const utf8::String & name,struct Stat & stat);
    void postRequest(const ksock::SockAddr & address);
    void postRequest(const utf8::String & oldName,const utf8::String & newName);
    void postRequest(uint64_t timeout);
    void postRequest(FiberInterlockedMutex * mutex);

    Requester(const Requester &){}
    void operator = (const Requester &){}
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncDescriptorsCluster {
  friend class BaseFiber;
  friend class BaseThread;
  friend class BaseServer;
  friend class AsyncIoSlave;
  friend class AsyncOpenFileSlave;
  friend class AsyncTimerSlave;
  friend class AsyncAcquireSlave;
  friend class FiberInterlockedMutex;
  friend class AsyncDescriptor;
  friend void initialize();
  friend void cleanup();
  public:
    virtual ~AsyncDescriptorsCluster();
    AsyncDescriptorsCluster();

    void postRequest(AsyncDescriptor * descriptor,uint64_t position,const void * buffer,uint64_t length,AsyncEventType ioType);
    void postRequest(AsyncDescriptor * descriptor,const ksock::SockAddr & addr);
    void postRequest(AsyncDescriptor * descriptor,const utf8::String & fileName,bool createIfNotExist,bool exclusive,bool readOnly);
    void postRequest(AsyncDescriptor * descriptor,uint64_t position,uint64_t length,AsyncEvent::LockFileType lockType);
    void postRequest(Vector<utf8::String> * dirList,const utf8::String & dirAndMask,const utf8::String & exMask,bool recursive,bool includeDirs);
    void postRequest(AsyncEventType event,const utf8::String & name,bool recursive);
    void postRequest(const utf8::String & name,uintptr_t defPort);
    void postRequest(const utf8::String & name,struct Stat & stat);
    void postRequest(const ksock::SockAddr & address);
    void postRequest(const utf8::String & oldName,const utf8::String & newName);
    void postRequest(uint64_t timeout);
    void postRequest(FiberInterlockedMutex * mutex);
  protected:
    Semaphore semaphore_;
    InterlockedMutex mutex_;
    virtual void queue() = 0;
    virtual void sweepFiber(Fiber * fiber) = 0;
  private:
    Events events_;
    EmbeddedList<
      AsyncDescriptor,
      AsyncDescriptor::clusterListNode,
      AsyncDescriptor::clusterListNodeObject
    > descriptorsList_;

    static uintptr_t refCount_;
    static uint8_t requester_[];
    static Requester & requester();

#if defined(__WIN32__) || defined(__WIN64__)
    static VOID CALLBACK fileIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);
    static VOID CALLBACK socketIOCompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
#endif
    static void allocateSig();
    static void deallocateSig();
    static void initialize();
    static void cleanup();

    void attachDescriptor(AsyncDescriptor & descriptor,Fiber & toFiber);
    void detachDescriptor(AsyncDescriptor & descriptor);

    void postEvent(AsyncEventType event,Fiber * fiber);
    void postEvent(AsyncDescriptor * descriptor,int32_t errNo,AsyncEventType event,uint64_t count);
    void postEvent(AsyncDescriptor * descriptor,int32_t errNo,AsyncEventType event,const AsyncDescriptorKey & file);
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline void AsyncDescriptorsCluster::postRequest(AsyncDescriptor * descriptor, uint64_t position, const void * buffer, uint64_t length, AsyncEventType ioType)
{
  requester().postRequest(descriptor,position,buffer,length,ioType);
}
//---------------------------------------------------------------------------
inline void AsyncDescriptorsCluster::postRequest(AsyncDescriptor * descriptor, const ksock::SockAddr & addr)
{
  requester().postRequest(descriptor,addr);
}
//---------------------------------------------------------------------------
inline void AsyncDescriptorsCluster::postRequest(AsyncDescriptor * descriptor,const utf8::String & fileName,bool createIfNotExist,bool exclusive,bool readOnly)
{
  requester().postRequest(descriptor,fileName,createIfNotExist,exclusive,readOnly);
}
//---------------------------------------------------------------------------
inline void AsyncDescriptorsCluster::postRequest(AsyncDescriptor * descriptor,uint64_t position,uint64_t length,AsyncEvent::LockFileType lockType)
{
  requester().postRequest(descriptor,position,length,lockType);
}
//---------------------------------------------------------------------------
inline void AsyncDescriptorsCluster::postRequest(Vector<utf8::String> * dirList,const utf8::String & dirAndMask,const utf8::String & exMask,bool recursive,bool notIncludeDirs)
{
  requester().postRequest(dirList,dirAndMask,exMask,recursive,notIncludeDirs);
}
//---------------------------------------------------------------------------
inline void AsyncDescriptorsCluster::postRequest(AsyncEventType event,const utf8::String & name,bool recursive)
{
  requester().postRequest(event,name,recursive);
}
//---------------------------------------------------------------------------
inline void AsyncDescriptorsCluster::postRequest(const utf8::String & name,uintptr_t defPort)
{
  requester().postRequest(name,defPort);
}
//---------------------------------------------------------------------------
inline void AsyncDescriptorsCluster::postRequest(const utf8::String & name,struct Stat & stat)
{
  requester().postRequest(name,stat);
}
//---------------------------------------------------------------------------
inline void AsyncDescriptorsCluster::postRequest(const ksock::SockAddr & address)
{
  requester().postRequest(address);
}
//---------------------------------------------------------------------------
inline void AsyncDescriptorsCluster::postRequest(const utf8::String & oldName,const utf8::String & newName)
{
  requester().postRequest(oldName,newName);
}
//---------------------------------------------------------------------------
inline void AsyncDescriptorsCluster::postRequest(uint64_t timeout)
{
  requester().postRequest(timeout);
}
//---------------------------------------------------------------------------
inline void AsyncDescriptorsCluster::postRequest(FiberInterlockedMutex * mutex)
{
  requester().postRequest(mutex);
}
//---------------------------------------------------------------------------
inline Requester & AsyncDescriptorsCluster::requester()
{
  return *reinterpret_cast<Requester *>(requester_);
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
    void attachDescriptor(AsyncDescriptor & descriptor);
    void detachDescriptor(AsyncDescriptor & descriptor);
  protected:
    virtual void execute() = 0;
  private:
    BaseThread * thread_;

    static void fiber2(BaseFiber * fiber);
    void detachDescriptors();

    mutable EmbeddedListNode<BaseFiber> node_;
    static EmbeddedListNode<BaseFiber> & node(const BaseFiber & object){
      return object.node_;
    }
    static BaseFiber & nodeObject(const EmbeddedListNode<BaseFiber> & node,BaseFiber * p = NULL){
      return node.object(p->node_);
    }

    //FiberInterlockedMutex::LockNode lockNode_;
};
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

    BaseServer * const &  server() const;
    BaseThread & server(BaseServer * serverL);
  protected:
    EmbeddedList<
      BaseFiber,
      BaseFiber::node,
      BaseFiber::nodeObject
    > fibers_;
    void queue();
    void sweepFiber(Fiber * fiber);
    void detectMaxFiberStackSize();
  private:
    BaseServer * server_;
    uintptr_t maxStackSize_;

    void execute();

    mutable EmbeddedListNode<BaseThread> serverListNode_;
    static EmbeddedListNode<BaseThread> & serverListNode(const BaseThread & object){
      return object.serverListNode_;
    }
    static BaseThread & serverListNodeObject(const EmbeddedListNode<BaseThread> & node,BaseThread * p = NULL){
      return node.object(p->serverListNode_);
    }
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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class BaseServer {
  friend class BaseFiber;
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
      csAbortTimer = 16
    };

    const HowCloseServer & howCloseServer() const;
    BaseServer & howCloseServer(uintptr_t how);
    void closeServer();
  protected:
    virtual BaseThread * newThread();
    virtual BaseFiber * newFiber() = 0;
    virtual void attachFiber(BaseFiber & fiber);
    void sweepThreads();
  private:
    mutable InterlockedMutex                                          mutex_;
    EmbeddedList<
      BaseThread,
      BaseThread::serverListNode,
      BaseThread::serverListNodeObject
    > threads_;
    uintptr_t                                                         mfpt_;
    uintptr_t                                                         mt_;
    uintptr_t                                                         fiberStackSize_;
    uint64_t                                                          fiberTimeout_;
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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
/* TODO: rewrite
class FiberMutex {
  public:
    ~FiberMutex();
    FiberMutex();

    FiberMutex &  rdLock();
    bool          tryRDLock();
    FiberMutex &  wrLock();
    bool          tryWRLock();
    FiberMutex &  unlock();
  protected:
  private:
    FiberInterlockedMutex mutex_;
    intptr_t counter_;

    FiberMutex(const FiberMutex &){}
    void operator =(const FiberMutex &){}
};
//---------------------------------------------------------------------------
inline FiberMutex::~FiberMutex()
{
  assert( counter_ == 0 && wait_.first() == NULL && wait_.last() == NULL && wait_.count() == 0 );
}
//---------------------------------------------------------------------------
inline FiberMutex::FiberMutex() : counter_(0)
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
}*/
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
/*TODO: rewrite
class FiberSemaphore {
  public:
    ~FiberSemaphore();
    FiberSemaphore();

    void  post();
    void  wait();
    bool  tryWait();
  protected:
  private:
    Semaphore                                                     semaphore_;
    InterlockedMutex                                              mutex_;
    EmbeddedList< BaseFiber,BaseFiber::ipc,BaseFiber::ipcObject>  wait_;
    FiberSemaphore(const FiberSemaphore &){}
    void operator =(const FiberSemaphore &){}
};
//---------------------------------------------------------------------------
inline FiberSemaphore::~FiberSemaphore()
{
  assert(wait_.first() == NULL && wait_.last() == NULL);
}
//---------------------------------------------------------------------------
inline FiberSemaphore::FiberSemaphore()
{
}*/
//---------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
#endif /* _fiber_H_ */
//------------------------------------------------------------------------------
