/*-
 * Copyright 2006 Guram Dukashvili
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
#ifndef asyncH
#define asyncH
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
  etCopy,
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
  etStackBackTrace,
  etStackBackTraceZero,
  etCount
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Fiber;
class AsyncDescriptor;
class FiberInterlockedMutex;
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
    enum LockFileType { rdLock, wrLock, tryRDLock, tryWRLock };
#if defined(__WIN32__) || defined(__WIN64__)
    OVERLAPPED overlapped_;
#elif SIZEOF_AIOCB
    struct aiocb iocb_;
    Thread * ioSlave_;
#endif
#if _MSC_VER
#pragma warning(push,3)
#endif
    union {
      struct {
        union {
          uint64_t position_;
          uint64_t timerStartTime_;
          uintptr_t data0_;
          intptr_t data1_;
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
          InterlockedMutex * mutex0_;
          FiberInterlockedMutex * mutex_;
          DirectoryChangeNotification * directoryChangeNotification_;
          struct Stat * stat_;
          struct {
            bool readOnly_;
            bool createIfNotExist_;
            bool exclusive_;
            bool recursive_;
            bool includeDirs_;
            bool exMaskAsList_;
            bool abort_;
            bool rval_;
          };
        };
        union {
          uint64_t length_;
          uint64_t count_;
          uintptr_t data_;
          Thread * thread_;
          uintptr_t tid_;
          Vector<utf8::String> * dirList_;
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
    uint64_t timeout_;
    int32_t errno_;
    AsyncEventType type_;
  private:
    AsyncEvent(const AsyncEvent &){}
    void operator = (const AsyncEvent &){}
};
//---------------------------------------------------------------------------
inline AsyncEvent::~AsyncEvent()
{
}
//---------------------------------------------------------------------------
inline AsyncEvent::AsyncEvent() : position_(0), buffer_(NULL), length_(0),
  fiber_(NULL), descriptor_(NULL), timeout_(~uint64_t(0)), errno_(0), type_(etNone)
{
#if defined(__WIN32__) || defined(__WIN64__)
  memset(&overlapped_,0,sizeof(overlapped_));
#endif
#if SIZEOF_AIOCB
  memset(&iocb_, 0, sizeof(iocb_));
  ioSlave_ = NULL;
#endif
}
//------------------------------------------------------------------------------
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
    bool hashKeyEqu(const AsyncDescriptorKey & key) const;
    uintptr_t hash() const;
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
inline AsyncDescriptorKey::AsyncDescriptorKey(int descriptor) : descriptor_(descriptor)
{
}
//---------------------------------------------------------------------------
inline bool AsyncDescriptorKey::hashKeyEqu(const AsyncDescriptorKey & key) const
{
  return descriptor_ == key.descriptor_;
}
//---------------------------------------------------------------------------
inline uintptr_t AsyncDescriptorKey::hash() const
{
  return HF::hash(&descriptor_,sizeof(descriptor_));
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
    virtual void connect(AsyncEvent * request) = 0;
    virtual int64_t read2(void * buf, uint64_t len);
    virtual int64_t write2(const void * buf, uint64_t len);
#endif
    virtual void shutdown2();
    virtual void flush2();
    virtual void close2();
    virtual void openAPI();
    virtual void closeAPI();
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
} // namespace ksys
//------------------------------------------------------------------------------
#endif // asyncH
//------------------------------------------------------------------------------
