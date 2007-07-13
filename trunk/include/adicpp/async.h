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
  etExec,
  etWaitForProcess,
  etResolveName,
  etResolveAddress,
  etStat,
  etDirectoryChangeNotification,
  etWaitCommEvent,
  etLockFile,
  etRead,
  etWrite,
  etAccept,
  etConnect,
  etClose,
  etQuit,
  etDispatch,
  etTimer,
  etAcquireMutex,
  etAcquireSemaphore,
  etStackBackTrace,
  etStackBackTraceZero,
  etCount
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Fiber;
class AsyncDescriptor;
class AsyncFile;
class FiberInterlockedMutex;
class FiberSemaphore;
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
typedef HANDLE file_t;
typedef SOCKET sock_t;
#else
typedef int sock_t;
typedef int file_t;
#define INVALID_HANDLE_VALUE -1
#define INVALID_SOCKET -1
#endif
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
          const Array<utf8::String> * env_;
          AsyncFile * file_;
        };
        union {
          void * buffer_;
          const void * cbuffer_;
          file_t fileDescriptor_;
          sock_t socket_;
          InterlockedMutex * mutex0_;
          FiberInterlockedMutex * mutex_;
          FiberSemaphore * semaphore_;
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
            bool wait_;
          };
        };
        union {
          uint64_t length_;
          uint64_t count_;
          uintptr_t data_;
          Thread * thread_;
          pid_t pid_;
          uintptr_t tid_;
          Vector<utf8::String> * dirList_;
          const Array<utf8::String> * args_;
#if defined(__WIN32__) || defined(__WIN64__)
          DWORD evtMask_;
#endif
        };
        LockFileType lockType_;
      };
      struct {
        ksock::SockAddr address_;
        uintptr_t defPort_;
	      intptr_t aiFlag_;
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
    union {
      sock_t socket_;
      file_t descriptor_;
    };
#if defined(__WIN32__) || defined(__WIN64__)
    AsyncDescriptorKey(sock_t socket);
#endif
    AsyncDescriptorKey(file_t descriptor);
    file_t descriptor() const;
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
inline AsyncDescriptorKey::AsyncDescriptorKey() : descriptor_(INVALID_HANDLE_VALUE)
{
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
inline AsyncDescriptorKey::AsyncDescriptorKey(sock_t socket) : socket_(socket)
{
}
#endif
//---------------------------------------------------------------------------
inline AsyncDescriptorKey::AsyncDescriptorKey(file_t descriptor) : descriptor_(descriptor)
{
}
//---------------------------------------------------------------------------
inline file_t AsyncDescriptorKey::descriptor() const
{
  return descriptor_;
}
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

    virtual bool isSocket() const;

    virtual int64_t readV(void * buf,uint64_t size) = 0;
    virtual int64_t writeV(const void * buf,uint64_t size) = 0;

#if defined(__WIN32__) || defined(__WIN64__)
    virtual int WSAEnumNetworkEvents(WSAEVENT hEventObject, DWORD event) = 0;
    virtual BOOL AcceptEx(SOCKET sAcceptSocket, PVOID lpOutputBuffer, DWORD dwReceiveDataLength, DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, LPDWORD lpdwBytesReceived, LPOVERLAPPED lpOverlapped) = 0;
    virtual BOOL Connect(HANDLE event, AsyncEvent * request) = 0;
    virtual BOOL Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) = 0;
    virtual BOOL Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) = 0;
    virtual BOOL GetOverlappedResult(LPOVERLAPPED lpOverlapped, LPDWORD lpNumberOfBytesTransferred, BOOL bWait, LPDWORD lpdwFlags = NULL) = 0;
#endif
  protected:
#if HAVE_KQUEUE || __linux__
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
    mutable EmbeddedListNode<AsyncDescriptor> listNode_;
    static EmbeddedListNode<AsyncDescriptor> & listNode(const AsyncDescriptor & object){
      return object.listNode_;
    }
    static AsyncDescriptor & listNodeObject(const EmbeddedListNode<AsyncDescriptor> & node,AsyncDescriptor * p = NULL){
      return node.object(p->listNode_);
    }
};
//---------------------------------------------------------------------------
inline AsyncDescriptor::~AsyncDescriptor()
{
}
//---------------------------------------------------------------------------
inline AsyncDescriptor::AsyncDescriptor()
{
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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class FiberSemaphore : public Semaphore {
  friend class AsyncAcquireSlave;
  public:
    ~FiberSemaphore();
    FiberSemaphore();

    bool timedWait(uint64_t timeout);
    FiberSemaphore & wait();
  protected:
  private:
    FiberSemaphore(const FiberSemaphore &){}
    void operator =(const FiberSemaphore &){}
};
//---------------------------------------------------------------------------
inline FiberSemaphore::~FiberSemaphore()
{
}
//---------------------------------------------------------------------------
inline FiberSemaphore::FiberSemaphore()
{
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
inline FiberSemaphore & FiberSemaphore::wait()
{
  timedWait(~uint64_t(0));
  return *this;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
#endif // asyncH
//------------------------------------------------------------------------------
