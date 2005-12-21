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
#ifndef afdH
#define afdH
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncFile : public AsyncDescriptor {
  public:
    virtual ~AsyncFile();

    AsyncFile(const utf8::String & fileName = utf8::String(),bool exclusive = false);

    bool isOpen() const;
    AsyncFile & open();
    bool tryOpen(bool createIfNotExist = false);
    AsyncFile & close();
    AsyncFile & seek(uint64_t pos);
    uint64_t size() const;
    AsyncFile & resize(uint64_t nSize);
    uint64_t tell() const;
    int64_t read(void * buf,uint64_t size);
    int64_t write(const void * buf,uint64_t size);
    int64_t read(uint64_t pos,void * buf,uint64_t size);
    int64_t write(uint64_t pos,const void * buf,uint64_t size);
    AsyncFile & readBuffer(void * buf,uint64_t size);
    AsyncFile & writeBuffer(const void * buf,uint64_t size);
    AsyncFile & readBuffer(uint64_t pos,void * buf,uint64_t size);
    AsyncFile & writeBuffer(uint64_t pos,const void * buf,uint64_t size);

    bool tryRDLock(uint64_t pos,uint64_t size);
    bool tryWRLock(uint64_t pos,uint64_t size);
    AsyncFile & rdLock(uint64_t pos,uint64_t size);
    AsyncFile & wrLock(uint64_t pos,uint64_t size);
    AsyncFile & unLock(uint64_t pos,uint64_t size);

    uintptr_t gets(AutoPtr<char> & p);
    utf8::String gets();

    const utf8::String & fileName() const;
    AsyncFile & fileName(const utf8::String & name);
    const bool & exclusive() const;
    AsyncFile & exclusive(bool exclusive);
    const bool & removeAfterClose() const;
    AsyncFile & removeAfterClose(bool removeAfterCloseL);
  protected:
  private:
#if defined(__WIN32__) || defined(__WIN64__)
    union int64 {
      int64_t a;
      struct {
        ULONG lo;
        LONG hi;
      };
    };
    union uint64 {
      uint64_t a;
      struct {
        ULONG lo;
        ULONG hi;
      };
    };
#endif
    utf8::String fileName_;
    bool exclusive_;
    bool removeAfterClose_;

#if defined(__WIN32__) || defined(__WIN64__)
    int WSAEnumNetworkEvents(WSAEVENT hEventObject,DWORD event);
    BOOL AcceptEx(
      SOCKET sAcceptSocket,	
      PVOID lpOutputBuffer,	
      DWORD dwReceiveDataLength,	
      DWORD dwLocalAddressLength,	
      DWORD dwRemoteAddressLength,	
      LPDWORD lpdwBytesReceived,	
      LPOVERLAPPED lpOverlapped	
    );
    BOOL Connect(HANDLE event,ksys::IoRequest * request);
    BOOL Read(
      LPVOID lpBuffer,
      DWORD nNumberOfBytesToRead,
      LPDWORD lpNumberOfBytesRead,
      LPOVERLAPPED lpOverlapped
    );
    BOOL Write(
      LPCVOID lpBuffer,
      DWORD nNumberOfBytesToWrite,
      LPDWORD lpNumberOfBytesWritten,
      LPOVERLAPPED lpOverlapped
    );
    BOOL GetOverlappedResult(
      LPOVERLAPPED lpOverlapped,
      LPDWORD lpNumberOfBytesTransferred,
      BOOL bWait,
      LPDWORD lpdwFlags
    );
#elif HAVE_KQUEUE
    int accept();
    void connect(ksys::IoRequest * request);
#endif
};
//---------------------------------------------------------------------------
inline bool AsyncFile::isOpen() const
{
  return file_ != INVALID_HANDLE_VALUE;
}
//---------------------------------------------------------------------------
inline uint64_t AsyncFile::size() const
{
#if defined(__WIN32__) || defined(__WIN64__)
  uint64 i;
  i.lo = GetFileSize(file_,&i.hi);
  return i.a;
#else
  struct stat st;
  fstat(file_,&st);
  return st.st_size;
#endif
}
//---------------------------------------------------------------------------
inline uint64_t AsyncFile::tell() const
{
#if defined(__WIN32__) || defined(__WIN64__)
  int64 i;
  i.hi = 0;
  i.lo = SetFilePointer(file_,0,&i.hi,FILE_CURRENT);
  if( i.lo == 0xFFFFFFFF && GetLastError() != NO_ERROR ) i.a = -1;
  return i.a;
#else
  return lseek(file_,0,SEEK_CUR);
#endif
}
//---------------------------------------------------------------------------
inline const utf8::String & AsyncFile::fileName() const
{
  return fileName_;
}
//---------------------------------------------------------------------------
inline AsyncFile & AsyncFile::fileName(const utf8::String & name)
{
  close();
  fileName_ = name;
  return *this;
}
//---------------------------------------------------------------------------
inline const bool & AsyncFile::exclusive() const
{
  return exclusive_;
}
//---------------------------------------------------------------------------
inline AsyncFile & AsyncFile::exclusive(bool exclusive)
{
  exclusive_ = exclusive;
  return *this;
}
//---------------------------------------------------------------------------
inline const bool & AsyncFile::removeAfterClose() const
{
  return removeAfterClose_;
}
//---------------------------------------------------------------------------
inline AsyncFile & AsyncFile::removeAfterClose(bool removeAfterCloseL)
{
  removeAfterClose_ = removeAfterCloseL;
  return *this;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
