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
//---------------------------------------------------------------------------
#ifndef afdH
#define afdH
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncFile : public AsyncDescriptor {
  friend void initialize(int,char **);
  friend void cleanup();
  public:
    virtual ~AsyncFile();
    AsyncFile(const utf8::String & fileName = utf8::String());

    int64_t readV(void * buf,uint64_t size){ return read(buf,size); }
    int64_t writeV(const void * buf,uint64_t size){ return write(buf,size); }

    bool isOpen() const;
    AsyncFile & open();
    file_t openHelper();
    bool tryOpen();
    AsyncFile & close();
    AsyncFile & seek(uint64_t pos);
    uint64_t size() const;
    AsyncFile & resize(uint64_t nSize);
    uint64_t tell() const;
    int64_t read(void * buf,uint64_t size);
    int64_t write(const void * buf,uint64_t size);
    int64_t read(uint64_t pos, void * buf, uint64_t size);
    int64_t write(uint64_t pos, const void * buf, uint64_t size);
    AsyncFile & readBuffer(void * buf, uint64_t size);
    AsyncFile & writeBuffer(const void * buf, uint64_t size);
    AsyncFile & readBuffer(uint64_t pos, void * buf, uint64_t size);
    AsyncFile & writeBuffer(uint64_t pos, const void * buf, uint64_t size);
#if defined(__WIN32__) || defined(__WIN64__)
    AsyncFile & flush();
#else
    AsyncFile & flush() { return *this; }
#endif

    bool tryRDLock(uint64_t pos, uint64_t size);
    bool tryWRLock(uint64_t pos, uint64_t size);
    AsyncFile & rdLock(uint64_t pos,uint64_t size,uint64_t timeout = ~uint64_t(0));
    AsyncFile & wrLock(uint64_t pos,uint64_t size,uint64_t timeout = ~uint64_t(0));
    AsyncFile & unLock(uint64_t pos,uint64_t size);
    file_t dup() const;
    uint64_t copy(AsyncFile & src,uint64_t size = 0);

    uintptr_t gets(AutoPtr<char> & p,bool * eof = NULL);

    class LineGetBuffer {
      public:
        ~LineGetBuffer() {}
        LineGetBuffer() {}
        LineGetBuffer(AsyncFile & file,uintptr_t size = 0) : 
          file_(&file), bufferFilePos_(0), size_(size), pos_(0), len_(0), codePage_(CP_ACP), removeNewLine_(false), detectUnicodeFFFE_(true) {}

        uint64_t tell();
        LineGetBuffer & seek(uint64_t pos);

        AsyncFile * file_;
        AutoPtr<uint8_t> buffer_;
        uint64_t bufferFilePos_;
        uintptr_t size_;
        uintptr_t pos_;
        uintptr_t len_;
        uintptr_t codePage_;
        bool removeNewLine_;
	      bool detectUnicodeFFFE_;
    };
    bool gets(utf8::String & str,LineGetBuffer * buffer = NULL);

    const utf8::String & fileName() const;
    AsyncFile & fileName(const utf8::String & name);
    bool exclusive() const;
    AsyncFile & exclusive(bool v);
    bool removeAfterClose() const;
    AsyncFile & removeAfterClose(bool v);
    bool readOnly() const;
    AsyncFile & readOnly(bool v);
    bool createIfNotExist() const;
    AsyncFile & createIfNotExist(bool v);
    bool random() const;
    AsyncFile & random(bool v);
    bool direct() const;
    AsyncFile & direct(bool v);
    bool nocache() const;
    AsyncFile & nocache(bool v);
    bool createPath() const;
    AsyncFile & createPath(bool v);
    bool async() const;
    AsyncFile & async(bool v);

    const uintptr_t & codePage() const;
    AsyncFile & codePage(uintptr_t v);

#if defined(__WIN32__) || defined(__WIN64__)
    DWORD waitCommEvent();
    const DWORD & alignment() const;
#else
    uint32_t waitCommEvent();
    uint32_t alignment() const { return 1; }
#endif
    bool std() const;
    bool seekable() const;
    bool atty() const;

    AsyncFile & operator << (const char * s){ writeBuffer(s,::strlen(s)); return *this; }
    AsyncFile & operator << (const utf8::String & s);
  protected:
  private:
    static uint8_t mutex_[];
    static InterlockedMutex & mutex();
#if _MSC_VER
#pragma warning(push,3)
#endif
#if defined(__WIN32__) || defined(__WIN64__)
    HANDLE eventObject_;
    union int64 {
      int64_t a;
      struct {
        ULONG lo;
        LONG  hi;
      };
    };
    union uint64 {
      uint64_t  a;
      struct {
        ULONG lo;
        ULONG hi;
      };
    };
    DWORD alignment_;
#endif
    utf8::String fileName_;
    struct {
      uint8_t exclusive_        : 1;
      uint8_t removeAfterClose_ : 1;
      uint8_t readOnly_         : 1;
      uint8_t createIfNotExist_ : 1;
      uint8_t std_              : 1;
      uint8_t seekable_         : 1;
      uint8_t atty_             : 1;
      uint8_t random_           : 1;
      uint8_t direct_           : 1;
      uint8_t nocache_          : 1;
      uint8_t createPath_       : 1;
      uint8_t async_            : 1;
    };
#if _MSC_VER
#pragma warning(pop)
#endif
    uintptr_t codePage_;

#if defined(__WIN32__) || defined(__WIN64__)
    int WSAEnumNetworkEvents(WSAEVENT hEventObject, DWORD event);
    BOOL AcceptEx(SOCKET sAcceptSocket, PVOID lpOutputBuffer, DWORD dwReceiveDataLength, DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, LPDWORD lpdwBytesReceived, LPOVERLAPPED lpOverlapped);
    BOOL Connect(HANDLE event, ksys::AsyncEvent * request);
    BOOL Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
    BOOL Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
    BOOL GetOverlappedResult(LPOVERLAPPED lpOverlapped, LPDWORD lpNumberOfBytesTransferred, BOOL bWait, LPDWORD lpdwFlags);
#elif HAVE_KQUEUE || __linux__
    int accept();
    void connect(AsyncEvent * request);
#endif
    void shutdown2();
    void flush2();
    void close2();
    bool redirectByName();
    void redirectToStdin();
    void redirectToStdout();
    void redirectToStderr();

    static void initialize();
    static void cleanup();
};
//---------------------------------------------------------------------------
inline bool AsyncFile::isOpen() const
{
  return descriptor_ != INVALID_HANDLE_VALUE;
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
inline const DWORD & AsyncFile::alignment() const
{
  return alignment_;
}
#endif
//---------------------------------------------------------------------------
inline bool AsyncFile::std() const
{
  return std_ != 0;
}
//---------------------------------------------------------------------------
inline bool AsyncFile::seekable() const
{
  return seekable_ != 0;
}
//---------------------------------------------------------------------------
inline bool AsyncFile::atty() const
{
  return atty_ != 0;
}
//---------------------------------------------------------------------------
inline const utf8::String & AsyncFile::fileName() const
{
  return fileName_;
}
//---------------------------------------------------------------------------
inline AsyncFile & AsyncFile::fileName(const utf8::String & name)
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( fileName_.strcasecmp(name) != 0 ){
#else
  if( fileName_.strcmp(name) != 0 ){
#endif
    close();
    fileName_ = name;
  }
  return *this;
}
//---------------------------------------------------------------------------
inline bool AsyncFile::exclusive() const
{
  return exclusive_ != 0;
}
//---------------------------------------------------------------------------
inline AsyncFile & AsyncFile::exclusive(bool v)
{
  exclusive_ = v;
  return *this;
}
//---------------------------------------------------------------------------
inline bool AsyncFile::removeAfterClose() const
{
  return removeAfterClose_ != 0;
}
//---------------------------------------------------------------------------
inline AsyncFile & AsyncFile::removeAfterClose(bool v)
{
  removeAfterClose_ = v;
  return *this;
}
//---------------------------------------------------------------------------
inline bool AsyncFile::readOnly() const
{
  return readOnly_ != 0;
}
//---------------------------------------------------------------------------
inline AsyncFile & AsyncFile::readOnly(bool v)
{
  readOnly_ = v;
  return *this;
}
//---------------------------------------------------------------------------
inline bool AsyncFile::createIfNotExist() const
{
  return createIfNotExist_ != 0;
}
//---------------------------------------------------------------------------
inline AsyncFile & AsyncFile::createIfNotExist(bool v)
{
  createIfNotExist_ = v;
  return *this;
}
//---------------------------------------------------------------------------
inline bool AsyncFile::random() const
{
  return random_ != 0;
}
//---------------------------------------------------------------------------
inline AsyncFile & AsyncFile::random(bool v)
{
  random_ = v;
  return *this;
}
//---------------------------------------------------------------------------
inline bool AsyncFile::direct() const
{
  return direct_ != 0;
}
//---------------------------------------------------------------------------
inline AsyncFile & AsyncFile::direct(bool v)
{
  direct_ = v;
  return *this;
}
//---------------------------------------------------------------------------
inline bool AsyncFile::nocache() const
{
  return nocache_ != 0;
}
//---------------------------------------------------------------------------
inline AsyncFile & AsyncFile::nocache(bool v)
{
  nocache_ = v;
  return *this;
}
//---------------------------------------------------------------------------
inline bool AsyncFile::createPath() const
{
  return createPath_ != 0;
}
//---------------------------------------------------------------------------
inline AsyncFile & AsyncFile::createPath(bool v)
{
  createPath_ = v;
  return *this;
}
//---------------------------------------------------------------------------
inline bool AsyncFile::async() const
{
  return async_ != 0;
}
//---------------------------------------------------------------------------
inline AsyncFile & AsyncFile::async(bool v)
{
  async_ = v;
  return *this;
}
//---------------------------------------------------------------------------
inline const uintptr_t & AsyncFile::codePage() const
{
  return codePage_;
}
//---------------------------------------------------------------------------
inline AsyncFile & AsyncFile::codePage(uintptr_t v)
{
  codePage_ = v;
  return *this;
}
//---------------------------------------------------------------------------
inline InterlockedMutex & AsyncFile::mutex()
{
  return *reinterpret_cast<InterlockedMutex *>(mutex_);
}
//---------------------------------------------------------------------------
inline int64_t AsyncFile::read(void * buf,uint64_t size)
{
  uint64_t pos = seekable_ ? tell() : 0;
  int64_t r = read(pos,buf,size);
  if( r > 0 && seekable_ ) seek(pos + r);
  return r;
}
//---------------------------------------------------------------------------
inline int64_t AsyncFile::write(const void * buf,uint64_t size)
{
  uint64_t pos = seekable_ ? tell() : 0;
  int64_t w = write(pos,buf,size);
  if( w > 0 && seekable_ ) seek(pos + w);
  return w;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T> class AutoFileWRLock {
  public:
    ~AutoFileWRLock();
    AutoFileWRLock();
    AutoFileWRLock(T & file,uint64_t pos = 0,uint64_t size = 0);
    AutoFileWRLock<T> & setLocked(T & file,uint64_t pos = 0,uint64_t size = 0);
  protected:
  private:
    T * file_;
    uint64_t pos_;
    uint64_t size_;

    AutoFileWRLock(const AutoFileWRLock<T> &){}
    void operator =(const AutoFileWRLock<T> &){}
};
//---------------------------------------------------------------------------
template<typename T> inline AutoFileWRLock<T>::~AutoFileWRLock()
{
  if( file_ != NULL ) file_->unLock(pos_,size_);
}
//---------------------------------------------------------------------------
template<typename T> inline
AutoFileWRLock<T>::AutoFileWRLock() : file_(NULL), pos_(0), size_(0)
{
}
//---------------------------------------------------------------------------
template<typename T> inline
AutoFileWRLock<T>::AutoFileWRLock(T & file,uint64_t pos,uint64_t size) :
  file_(&file), pos_(pos), size_(size)
{
  file_->wrLock(pos,size);
}
//---------------------------------------------------------------------------
template<typename T> inline
AutoFileWRLock<T> & AutoFileWRLock<T>::setLocked(T & file,uint64_t pos,uint64_t size)
{
  assert( file_ == NULL );
  file_ = &file;
  pos_ = pos;
  size_ = size;
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AutoFileRemove {
  public:
    ~AutoFileRemove();
    AutoFileRemove(const utf8::String & name);
  protected:
  private:
    utf8::String name_;

    AutoFileRemove(const AutoFileRemove &){}
    void operator =(const AutoFileRemove &){}
};
//---------------------------------------------------------------------------
inline AutoFileRemove::~AutoFileRemove()
{
  remove(name_,true);
}
//---------------------------------------------------------------------------
inline AutoFileRemove::AutoFileRemove(const utf8::String & name) : name_(name)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AutoFileClose {
  public:
    ~AutoFileClose();
    AutoFileClose(AsyncFile & file);
  protected:
  private:
    AsyncFile * file_;

    AutoFileClose(const AutoFileClose &){}
    void operator =(const AutoFileClose &){}
};
//---------------------------------------------------------------------------
inline AutoFileClose::~AutoFileClose()
{
  if( file_ != NULL ) file_->close();
}
//---------------------------------------------------------------------------
inline AutoFileClose::AutoFileClose(AsyncFile & file) : file_(&file)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EFileEOF : public Exception {
  public:
    EFileEOF() {}
    EFileEOF(int32_t code,const char * what);
    EFileEOF(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EFileEOF::EFileEOF(int32_t code,const char * what) : Exception(code,what)
{
}
//---------------------------------------------------------------------------
inline EFileEOF::EFileEOF(int32_t code,const utf8::String & what) : Exception(code,what)
{
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
