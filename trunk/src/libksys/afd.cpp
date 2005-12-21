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
#include <adicpp/ksys.h>
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
AsyncFile::~AsyncFile()
{
  detach();
  close();
}
//---------------------------------------------------------------------------
AsyncFile::AsyncFile(const utf8::String & fileName,bool exclusive) :
  fileName_(fileName), exclusive_(exclusive), removeAfterClose_(false)
{
  file_ = INVALID_HANDLE_VALUE;
#if defined(__WIN32__) || defined(__WIN64__)
  specification_ = 1;
#endif
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::close()
{
  if( file_ != INVALID_HANDLE_VALUE ){
    detach();
#if defined(__WIN32__) || defined(__WIN64__)
    CloseHandle(file_);
#else
    ::close(file_);
#endif
    file_ = INVALID_HANDLE_VALUE;
  }
  if( removeAfterClose_ ){
    try {
      remove(fileName_);
    }
    catch( ... ){
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::open()
{
  if( file_ == INVALID_HANDLE_VALUE ){
    for(;;){
#if defined(__WIN32__) || defined(__WIN64__)
      if( isWin9x() ){
        utf8::AnsiString ansiFileName(anyPathName2HostPathName(fileName_).getANSIString());
        file_ = CreateFileA(
          ansiFileName,
          GENERIC_READ | GENERIC_WRITE,
          exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
          NULL,
          OPEN_ALWAYS,
          FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
          NULL
        );
        if( file_ == INVALID_HANDLE_VALUE )
          file_ = CreateFileA(
            ansiFileName,
            GENERIC_READ,
            exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
            NULL
          );
      }
      else {
        utf8::WideString unicodeFileName(anyPathName2HostPathName(fileName_).getUNICODEString());
        file_ = CreateFileW(
          unicodeFileName,
          GENERIC_READ | GENERIC_WRITE,
          exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
          NULL,
          OPEN_ALWAYS,
          FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
          NULL
        );
        if( file_ == INVALID_HANDLE_VALUE )
          file_ = CreateFileW(
            unicodeFileName,
            GENERIC_READ,
            exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_ALWAYS,
            FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
            NULL
          );
      }
      if( file_ != INVALID_HANDLE_VALUE ) break;
      int32_t err;
      switch( err = GetLastError() ){
        case ERROR_INVALID_DRIVE       :
        case ERROR_ACCESS_DENIED       :
        case ERROR_TOO_MANY_OPEN_FILES :
        case ERROR_PATH_NOT_FOUND      :
        case ERROR_FILE_NOT_FOUND      :
        case ERROR_INVALID_NAME        :
        case ERROR_INVALID_FLAGS       :
        case ERROR_INVALID_ADDRESS     :
        case ERROR_INSUFFICIENT_BUFFER :
          throw ksys::ExceptionSP(new EFileError(err + errorOffset,fileName_));
      }
#else
      utf8::AnsiString ansiFileName(anyPathName2HostPathName(fileName_).getANSIString());
      mode_t um = umask(0);
      umask(um);
      file_ = ::open(
        ansiFileName,
        O_RDWR | O_CREAT | (exclusive_ ? O_EXLOCK : 0),
        um | S_IRUSR | S_IWUSR
      );
      if( file_ < 0 ) file_ = ::open(
        ansiFileName,
        O_RDONLY | O_CREAT | (exclusive_ ? O_EXLOCK : 0),
        um | S_IRUSR | S_IWUSR
      );
      int32_t err;
      if( file_ >= 0 ){
        if( fcntl(file_,F_SETFL,fcntl(file_,F_GETFL,0) | O_NONBLOCK) != 0 ){
	  err = errno;
	  ::close(file_);
          file_ = INVALID_HANDLE_VALUE;
          throw ksys::ExceptionSP(new EFileError(err,fileName_));
        }
        break;
      }
      switch( err = errno ){
        case ENOTDIR      :
        case ENOENT       :
        case ENAMETOOLONG :
        case EACCES       :
        case ELOOP        :
        case EISDIR       :
        case EROFS        :
        case EMFILE       :
        case ENFILE       :
        case EINTR        :
        case ENOSPC       :
        case EDQUOT       :
        case EIO          :
        case ETXTBSY      :
        case EFAULT       :
        case EOPNOTSUPP   :
        case EINVAL       :
          throw ksys::ExceptionSP(new EFileError(err,fileName_));
      }
#endif
      ksys::sleep1();
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
bool AsyncFile::tryOpen(bool createIfNotExist)
{
  if( file_ == INVALID_HANDLE_VALUE ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( isWin9x() ){
      utf8::AnsiString ansiFileName(anyPathName2HostPathName(fileName_).getANSIString());
      file_ = CreateFileA(
        ansiFileName,
        GENERIC_READ | GENERIC_WRITE,
        exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        createIfNotExist ? OPEN_ALWAYS : OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
        NULL
      );
      if( file_ == INVALID_HANDLE_VALUE ) file_ = CreateFileA(
        ansiFileName,
        GENERIC_READ,
        exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        createIfNotExist ? OPEN_ALWAYS : OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
        NULL
      );
    }
    else {
      utf8::WideString unicodeFileName(anyPathName2HostPathName(fileName_).getUNICODEString());
      file_ = CreateFileW(
        unicodeFileName,
        GENERIC_READ | GENERIC_WRITE,
        exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        createIfNotExist ? OPEN_ALWAYS : OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
        NULL
      );
      if( file_ == INVALID_HANDLE_VALUE ) file_ = CreateFileW(
        unicodeFileName,
        GENERIC_READ,
        exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        createIfNotExist ? OPEN_ALWAYS : OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
        NULL
      );
    }
#else
    utf8::AnsiString ansiFileName(anyPathName2HostPathName(fileName_).getANSIString());
    mode_t um = umask(0);
    umask(um);
    file_ = ::open(
      ansiFileName,
      O_RDWR |
      (createIfNotExist ? O_CREAT : 0) |
      (exclusive_ ? O_EXLOCK : 0),
      um | S_IRUSR | S_IWUSR
    );
    if( file_ < 0 ) file_ = ::open(
      ansiFileName,
      O_RDONLY |
      (createIfNotExist ? O_CREAT : 0) |
      (exclusive_ ? O_EXLOCK : 0),
      um | S_IRUSR | S_IWUSR
    );
#endif
  }
  return isOpen();
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::resize(uint64_t nSize)
{
  uint64_t oldPos = tell();
  try {
    int32_t err;
#if defined(__WIN32__) || defined(__WIN64__)
    seek(nSize);
    if( SetEndOfFile(file_) == 0 ){
      err = GetLastError() + errorOffset;
      throw ksys::ExceptionSP(new EDiskFull(err,utf8::string(__PRETTY_FUNCTION__)));
    }
#elif HAVE_FTRUNCATE
    if( ftruncate(file_,nSize) == -1 ){
      err = errno;
      throw ksys::ExceptionSP(new EDiskFull(err,utf8::string(__PRETTY_FUNCTION__)));
    }
#else
#error resize not implemented
#endif
  }
  catch( ExceptionSP & ){
    seek(oldPos);
    throw;
  }
  if( oldPos > nSize ) oldPos = nSize;
  seek(oldPos);
  return *this;
}
//---------------------------------------------------------------------------
int64_t AsyncFile::read(void * buf,uint64_t size)
{
  int64_t r = 0;
  while( size > 0 ){
    int64_t pos = tell();
    cluster()->postIoRequest(this,pos,buf,size,etRead);
    fiber()->switchFiber(fiber()->mainFiber());
    if( fiber()->event().errno_ != 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
      SetLastError(fiber()->event().errno_);
#else
      errno = fiber()->event().errno_;
#endif
      return -1;
    }
    if( r == 0 ) break;
    buf = (uint8_t *) buf + (size_t) fiber()->event().count_;
    r += fiber()->event().count_;
    size -= fiber()->event().count_;
    seek(pos + fiber()->event().count_);
  }
  return r;
}
//---------------------------------------------------------------------------
int64_t AsyncFile::write(const void * buf,uint64_t size)
{
  int64_t w = 0;
  while( size > 0 ){
    int64_t pos = tell();
    cluster()->postIoRequest(this,pos,(void *) buf,size,etWrite);
    fiber()->switchFiber(fiber()->mainFiber());
    if( fiber()->event().errno_ != 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
      SetLastError(fiber()->event().errno_);
#else
      errno = fiber()->event().errno_;
#endif
      return -1;
    }
    if( w == 0 ) break;
    buf = (uint8_t *) buf + (size_t) fiber()->event().count_;
    w += fiber()->event().count_;
    size -= fiber()->event().count_;
    seek(pos + fiber()->event().count_);
  }
  return w;
}
//---------------------------------------------------------------------------
int64_t AsyncFile::read(uint64_t pos,void * buf,uint64_t size)
{
  int64_t r = 0;
  while( size > 0 ){
    cluster()->postIoRequest(this,pos,buf,size,etRead);
    fiber()->switchFiber(fiber()->mainFiber());
    if( fiber()->event().errno_ != 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
      SetLastError(fiber()->event().errno_);
#else
      errno = fiber()->event().errno_;
#endif
      return -1;
    }
    if( r == 0 ) break;
    buf = (uint8_t *) buf + (size_t) fiber()->event().count_;
    r += fiber()->event().count_;
    size -= fiber()->event().count_;
  }
  return r;
}
//---------------------------------------------------------------------------
int64_t AsyncFile::write(uint64_t pos,const void * buf,uint64_t size)
{
  int64_t w = 0;
  while( size > 0 ){
    cluster()->postIoRequest(this,pos,(void *) buf,size,etWrite);
    fiber()->switchFiber(fiber()->mainFiber());
    if( fiber()->event().errno_ != 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
      SetLastError(fiber()->event().errno_);
#else
      errno = fiber()->event().errno_;
#endif
      return -1;
    }
    if( w == 0 ) break;
    buf = (uint8_t *) buf + (size_t) fiber()->event().count_;
    w += fiber()->event().count_;
    size -= fiber()->event().count_;
  }
  return w;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::readBuffer(void * buf,uint64_t size)
{
  int64_t r = read(buf,size);
  if( r == 0 && size > 0 )
    throw ksys::ExceptionSP(new EFileEOF(EIO,utf8::string(__PRETTY_FUNCTION__)));
  if( r < 0 || (uint64_t) r != size )
    throw ksys::ExceptionSP(new EFileError(
      fiber()->event().errno_ + errorOffset,utf8::string(__PRETTY_FUNCTION__))
    );
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::writeBuffer(const void * buf,uint64_t size)
{
  int64_t w = write(buf,size);
  if( w < 0 && size > 0 )
    throw ksys::ExceptionSP(new EFileError(
      fiber()->event().errno_ + errorOffset,utf8::string(__PRETTY_FUNCTION__))
    );
  if( (uint64_t) w != size && size > 0 )
    throw ksys::ExceptionSP(new EFileError(
      EIO,utf8::string(__PRETTY_FUNCTION__))
    );
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::readBuffer(uint64_t pos,void * buf,uint64_t size)
{
  int64_t r = read(pos,buf,size);
  if( r == 0 && size > 0 )
    throw ksys::ExceptionSP(new EFileEOF(EIO,utf8::string(__PRETTY_FUNCTION__)));
  if( r < 0 || (uint64_t) r != size )
    throw ksys::ExceptionSP(new EFileError(
      fiber()->event().errno_ + errorOffset,utf8::string(__PRETTY_FUNCTION__))
    );
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::writeBuffer(uint64_t pos,const void * buf,uint64_t size)
{
  int64_t w = write(pos,buf,size);
  if( w < 0 && size > 0 )
    throw ksys::ExceptionSP(new EFileError(
      fiber()->event().errno_ + errorOffset,utf8::string(__PRETTY_FUNCTION__))
    );
  if( (uint64_t) w != size && size > 0 )
    throw ksys::ExceptionSP(new EFileError(
      EIO,utf8::string(__PRETTY_FUNCTION__))
    );
  return *this;
}
//---------------------------------------------------------------------------
bool AsyncFile::tryRDLock(uint64_t pos,uint64_t size)
{
  if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( size == 0 ) size = ~UINT64_C(0);
    OVERLAPPED Overlapped;
    Overlapped.Offset = reinterpret_cast<uint64 *>(&pos)->lo;
    Overlapped.OffsetHigh = reinterpret_cast<uint64 *>(&pos)->hi;
    return LockFileEx(
      file_,
      LOCKFILE_FAIL_IMMEDIATELY,
      0,
      reinterpret_cast<uint64 *>(&size)->lo,
      reinterpret_cast<uint64 *>(&size)->hi,
      &Overlapped
    ) != 0;
#else
    struct flock fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(file_,F_SETLKW,&fl) == 0 ) return true;
    if( errno != EAGAIN ){
      int32_t err = errno;
      throw ksys::ExceptionSP(new EFLock(
        err,utf8::string(__PRETTY_FUNCTION__))
      );
    }
#endif
  }
  return false;
}
//---------------------------------------------------------------------------
bool AsyncFile::tryWRLock(uint64_t pos,uint64_t size)
{
  if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( size == 0 ) size = ~UINT64_C(0);
    OVERLAPPED Overlapped;
    Overlapped.Offset = reinterpret_cast<uint64 *>(&pos)->lo;
    Overlapped.OffsetHigh = reinterpret_cast<uint64 *>(&pos)->hi;
    return LockFileEx(
      file_,
      LOCKFILE_FAIL_IMMEDIATELY | LOCKFILE_EXCLUSIVE_LOCK,
      0,
      reinterpret_cast<uint64 *>(&size)->lo,
      reinterpret_cast<uint64 *>(&size)->hi,
      &Overlapped
    ) != 0;
#else
    struct flock fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(file_,F_SETLK,&fl) == 0 ) return true;
    if( errno != EAGAIN ){
      int32_t err = errno;
      throw ksys::ExceptionSP(new EFLock(err,utf8::string(__PRETTY_FUNCTION__)));
    }
#endif
  }
  return true;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::rdLock(uint64_t pos,uint64_t size)
{
  if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( size == 0 ) size = ~UINT64_C(0);
    OVERLAPPED Overlapped;
    Overlapped.Offset = reinterpret_cast<uint64 *>(&pos)->lo;
    Overlapped.OffsetHigh = reinterpret_cast<uint64 *>(&pos)->hi;
    if( LockFileEx(
      file_,
      0,
      0,
      reinterpret_cast<uint64 *>(&size)->lo,
      reinterpret_cast<uint64 *>(&size)->hi,
      &Overlapped
    ) == 0 ){
      int32_t err = GetLastError() + errorOffset;
      throw ksys::ExceptionSP(new EFileError(
        err,utf8::string(__PRETTY_FUNCTION__))
      );
    }
#else
    struct flock fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(file_,F_SETLKW,&fl) != 0 ){
      int32_t err = errno;
      throw ksys::ExceptionSP(new EFLock(err,utf8::string(__PRETTY_FUNCTION__)));
    }
#endif
  }
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::wrLock(uint64_t pos,uint64_t size)
{
  if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( size == 0 ) size = ~UINT64_C(0);
    OVERLAPPED Overlapped;
    Overlapped.Offset = reinterpret_cast<uint64 *>(&pos)->lo;
    Overlapped.OffsetHigh = reinterpret_cast<uint64 *>(&pos)->hi;
    if( LockFileEx(
      file_,
      LOCKFILE_EXCLUSIVE_LOCK,
      0,
      reinterpret_cast<uint64 *>(&size)->lo,
      reinterpret_cast<uint64 *>(&size)->hi,
      &Overlapped
    ) == 0 ){
      int32_t err = GetLastError() + errorOffset;
      throw ksys::ExceptionSP(
        new EFileError(err,utf8::string(__PRETTY_FUNCTION__))
      );
    }
#else
    struct flock fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(file_,F_SETLKW,&fl) != 0 ){
      int32_t err = errno;
      throw ksys::ExceptionSP(new EFLock(err,utf8::string(__PRETTY_FUNCTION__)));
    }
#endif
  }
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::unLock(uint64_t pos,uint64_t size)
{
  if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( size == 0 ) size = ~UINT64_C(0);
    OVERLAPPED Overlapped;
    Overlapped.Offset = reinterpret_cast<uint64 *>(&pos)->lo;
    Overlapped.OffsetHigh = reinterpret_cast<uint64 *>(&pos)->hi;
    if( UnlockFileEx(
      file_,
      0,
      reinterpret_cast<uint64 *>(&size)->lo,
      reinterpret_cast<uint64 *>(&size)->hi,
      &Overlapped
    ) == 0 ){
      int32_t err = GetLastError() + errorOffset;
      throw ksys::ExceptionSP(new EFileError(err,utf8::string(__PRETTY_FUNCTION__)));
    }
#else
    struct flock fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(file_,F_SETLK,&fl) != 0 ){
      int32_t err = errno;
      throw ksys::ExceptionSP(new EFLock(err,utf8::string(__PRETTY_FUNCTION__)));
    }
#endif
  }
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::seek(uint64_t pos)
{
#if defined(__WIN32__) || defined(__WIN64__)
  reinterpret_cast<uint64 *>(&pos)->lo = SetFilePointer(
    file_,
    reinterpret_cast<uint64 *>(&pos)->lo,
    (PLONG) &reinterpret_cast<uint64 *>(&pos)->hi,
    FILE_BEGIN
  );
  if( reinterpret_cast<uint64 *>(&pos)->lo == 0xFFFFFFFF && GetLastError() != NO_ERROR ){
    int32_t err = GetLastError() + errorOffset;
    throw ksys::ExceptionSP(new EFileError(err,utf8::string(__PRETTY_FUNCTION__)));
  }
#else
  int64_t lp = lseek(file_,pos,SEEK_SET);
  if( lp < 0 || (uint64_t) lp != pos ){
    int32_t err = errno;
    throw ksys::ExceptionSP(new EFileError(err,utf8::string(__PRETTY_FUNCTION__)));
  }
#endif
  return *this;
}
//---------------------------------------------------------------------------
uintptr_t AsyncFile::gets(AutoPtr<char> & p)
{
  uint64_t op = tell();
  intptr_t r, rr, l = 0;
  char * a, * q;
  for(;;){
    a = p.realloc(l + 256).ptr() + l;
    rr = r = (intptr_t) read(a,256);
    if( r <= 0 ) break;
    for( q = a; r > 0; q++, r-- ){
      if( *q == '\n' ){
        l = intptr_t(q - p.ptr() + 1);
        goto l1;
      }
    }
    l += rr;
  }
l1:
  seek(op + l);
  p.realloc(l + (l > 0));
  if( l > 0 ) p[l] = '\0';
  return l;
}
//---------------------------------------------------------------------------
utf8::String AsyncFile::gets()
{
  uint64_t op = tell();
  if( op >= size() ) throw ksys::ExceptionSP(new EFileEOF(-1,utf8::string(__PRETTY_FUNCTION__)));
  intptr_t r, rr, l = 0;
  char * a, * q;
  AutoPtr<char> p;
  for(;;){
    a = p.realloc(l + 256).ptr() + l;
    rr = r = (intptr_t) read(a,256);
    if( r <= 0 ) break;
    for( q = a; r > 0; q++, r-- ){
      if( *q == '\n' ){
        l = intptr_t(q - p.ptr() + 1);
        goto l1;
      }
    }
    l += rr;
  }
l1:
  seek(op + l);
  p.realloc(l + (l > 0));
  if( l > 0 ) p[l] = '\0';
  utf8::String::Container * container = new utf8::String::Container(0,p.ptr());
  p.ptr(NULL);
  return container;
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
int AsyncFile::WSAEnumNetworkEvents(WSAEVENT /*hEventObject*/,DWORD /*event*/)
{
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return SOCKET_ERROR;
}
//---------------------------------------------------------------------------
BOOL AsyncFile::AcceptEx(
  SOCKET /*sAcceptSocket*/,
  PVOID /*lpOutputBuffer*/,
  DWORD /*dwReceiveDataLength*/,
  DWORD /*dwLocalAddressLength*/,
  DWORD /*dwRemoteAddressLength*/,
  LPDWORD /*lpdwBytesReceived*/,
  LPOVERLAPPED /*lpOverlapped*/)
{
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}
//---------------------------------------------------------------------------
BOOL AsyncFile::Connect(HANDLE /*event*/,ksys::IoRequest * /*request*/)
{
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}
//---------------------------------------------------------------------------
BOOL AsyncFile::Read(
  LPVOID lpBuffer,
  DWORD nNumberOfBytesToRead,
  LPDWORD lpNumberOfBytesRead,
  LPOVERLAPPED lpOverlapped)
{
  return ReadFile(file_,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped);
}
//---------------------------------------------------------------------------
BOOL AsyncFile::Write(
  LPCVOID lpBuffer,
  DWORD nNumberOfBytesToWrite,
  LPDWORD lpNumberOfBytesWritten,
  LPOVERLAPPED lpOverlapped)
{
  return WriteFile(file_,lpBuffer,nNumberOfBytesToWrite,lpNumberOfBytesWritten,lpOverlapped);
}
//---------------------------------------------------------------------------
BOOL AsyncFile::GetOverlappedResult(
  LPOVERLAPPED lpOverlapped,
  LPDWORD lpNumberOfBytesTransferred,
  BOOL bWait,
  LPDWORD /*lpdwFlags*/)
{
  return ::GetOverlappedResult(file_,lpOverlapped,lpNumberOfBytesTransferred,bWait);
}
//---------------------------------------------------------------------------
#elif HAVE_KQUEUE
//---------------------------------------------------------------------------
int AsyncFile::accept()
{
  errno = ENOSYS;
  return -1;
}
//---------------------------------------------------------------------------
void AsyncFile::connect(ksys::IoRequest * /*request*/)
{
  errno = ENOSYS;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
