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
uint8_t AsyncFile::mutex_[sizeof(InterlockedMutex)];
//---------------------------------------------------------------------------
AsyncFile::~AsyncFile()
{
  detachOnClose_ = true;
  close();
}
//---------------------------------------------------------------------------
AsyncFile::AsyncFile(const utf8::String & fileName) :
  fileName_(fileName),
  exclusive_(false),
  removeAfterClose_(false),
  readOnly_(false),
  createIfNotExist_(false),
  std_(false),
  seekable_(true),
  detachOnClose_(true),
  random_(false),
  direct_(false),
  nocache_(false)
{
  file_ = INVALID_HANDLE_VALUE;
  handle_ = INVALID_HANDLE_VALUE;
#if defined(__WIN32__) || defined(__WIN64__)
  specification_ = 1;
  alignment_ = 1;
#endif
}
//---------------------------------------------------------------------------
bool AsyncFile::fileMember() const
{
  return currentFiber() != NULL && std_ == 0;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::close()
{
  if( detachOnClose_ ) detach();
  bool closed = false;
  {
    AutoLock<InterlockedMutex> lock(mutex());
    if( file_ != INVALID_HANDLE_VALUE ){
      if( !std_ ){
#if defined(__WIN32__) || defined(__WIN64__)
        CloseHandle(file_);
#else
        ::close(file_);
#endif
      }
      file_ = INVALID_HANDLE_VALUE;
      closed = true;
    }
    if( handle_ != INVALID_HANDLE_VALUE ){
      if( !std_ ){
#if defined(__WIN32__) || defined(__WIN64__)
        CloseHandle(handle_);
#else
        ::close(handle_);
#endif
      }
      handle_ = INVALID_HANDLE_VALUE;
      closed = true;
    }
  }
  if( closed && removeAfterClose_ ){
    try {
      remove(fileName_);
    }
    catch( ... ){}
  }
  return *this;
}
//---------------------------------------------------------------------------
file_t AsyncFile::openHelper(bool async)
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( fileName_.strncasecmp("COM",3) == 0 ){
    utf8::String::Iterator i(fileName_);
    i += 3;
    while( i.isDigit() ) i.next();
    seekable_ = i.getChar() != ':' || !(i + 1).eof();
  }
#endif
  file_t handle = INVALID_HANDLE_VALUE;
#if defined(__WIN32__) || defined(__WIN64__)
  int32_t err;
  if( isWin9x() ){
    utf8::AnsiString ansiFileName(anyPathName2HostPathName(fileName_).getANSIString());
    if( !readOnly_ )
      handle = CreateFileA(
        ansiFileName,GENERIC_READ | GENERIC_WRITE,
        exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        createIfNotExist_ ? OPEN_ALWAYS : OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE |
          (async ? FILE_FLAG_OVERLAPPED : 0) |
          (random_ ? FILE_FLAG_RANDOM_ACCESS : FILE_FLAG_SEQUENTIAL_SCAN) |
          (direct_ ? FILE_FLAG_WRITE_THROUGH : 0) |
          (nocache_ ? FILE_FLAG_NO_BUFFERING : 0),
        NULL
      );
    if( handle == INVALID_HANDLE_VALUE )
      handle = CreateFileA(
        ansiFileName,GENERIC_READ,
        exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        createIfNotExist_ ? OPEN_ALWAYS : OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE |
          (async ? FILE_FLAG_OVERLAPPED : 0) |
          (random_ ? FILE_FLAG_RANDOM_ACCESS : FILE_FLAG_SEQUENTIAL_SCAN) |
          (direct_ ? FILE_FLAG_WRITE_THROUGH : 0) |
          (nocache_ ? FILE_FLAG_NO_BUFFERING : 0),
        NULL
      );
  }
  else{
    utf8::WideString unicodeFileName(anyPathName2HostPathName(fileName_).getUNICODEString());
    if( !readOnly_ )
      handle = CreateFileW(
        unicodeFileName,
        GENERIC_READ | GENERIC_WRITE,
        exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        createIfNotExist_ ? OPEN_ALWAYS : OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE |
          (async ? FILE_FLAG_OVERLAPPED : 0) |
          (random_ ? FILE_FLAG_RANDOM_ACCESS : FILE_FLAG_SEQUENTIAL_SCAN) |
          (direct_ ? FILE_FLAG_WRITE_THROUGH : 0) |
          (nocache_ ? FILE_FLAG_NO_BUFFERING : 0),
        NULL
      );
    if( handle == INVALID_HANDLE_VALUE )
      handle = CreateFileW(
        unicodeFileName,GENERIC_READ,
        exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        createIfNotExist_ ? OPEN_ALWAYS : OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE |
          (async ? FILE_FLAG_OVERLAPPED : 0) |
          (random_ ? FILE_FLAG_RANDOM_ACCESS : FILE_FLAG_SEQUENTIAL_SCAN) |
          (direct_ ? FILE_FLAG_WRITE_THROUGH : 0) |
          (nocache_ ? FILE_FLAG_NO_BUFFERING : 0),
        NULL
      );
  }
  if( handle == INVALID_HANDLE_VALUE ){
    err = GetLastError() + errorOffset;
/*      case ERROR_INVALID_DRIVE       :
      case ERROR_ACCESS_DENIED       :
      case ERROR_TOO_MANY_OPEN_FILES :
      case ERROR_PATH_NOT_FOUND      :
      case ERROR_FILE_NOT_FOUND      :
      case ERROR_INVALID_NAME        :
      case ERROR_INVALID_FLAGS       :
      case ERROR_INVALID_ADDRESS     :
      case ERROR_INSUFFICIENT_BUFFER :*/
    newObject<EFileError>(err,fileName_)->throwSP();
  }
#else
  utf8::AnsiString ansiFileName(anyPathName2HostPathName(fileName_).getANSIString());
  mode_t um = umask(0);
  umask(um);
#ifndef O_DIRECT
#define O_DIRECT 0
#endif
  if( !readOnly_ )
    handle = ::open(
      ansiFileName,
      O_RDWR | O_CREAT | (exclusive_ ? O_EXLOCK : 0) | (direct_ ? O_DIRECT : 0),
      um | S_IRUSR | S_IWUSR
    );
  if( handle < 0 )
    handle = ::open(
      ansiFileName,
      O_RDONLY | O_CREAT | (exclusive_ ? O_EXLOCK : 0) | (direct_ ? O_DIRECT : 0),
      um | S_IRUSR | S_IWUSR
    );
  if( handle <= 0 ){
    err = errno;
/*      case ENOTDIR      :
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
      case EINVAL       :*/
    newObject<EFileError>(err,fileName_)->throwSP();
  }
  if( async ){
    int flags = fcntl(handle,F_GETFL,0);
    if( flags == -1 || fcntl(handle,F_SETFL,flags | O_NONBLOCK) == -1 ){
      err = errno;
      ::close(handle);
      newObject<EAsyncSocket>(err,__PRETTY_FUNCTION__)->throwSP();
    }
  }
#endif
  return handle;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::open()
{
  if( !isOpen() && !redirectByName() ){
    if( fileMember() ){
      attach();
      fiber()->event_.string0_ = fileName_;
      fiber()->event_.createIfNotExist_ = createIfNotExist_;
      fiber()->event_.exclusive_ = exclusive_;
      fiber()->event_.readOnly_ = readOnly_;
      fiber()->event_.file_ = this;
      fiber()->event_.type_ = etOpenFile;
      fiber()->thread()->postRequest(this);
      fiber()->switchFiber(fiber()->mainFiber());
      assert( fiber()->event_.type_ == etOpenFile );
      file_ = fiber()->event_.fileDescriptor_;
      if( fiber()->event_.errno_ != 0 )
        newObject<EFileError>(fiber()->event_.errno_,fileName_)->throwSP();
    }
    else {
      handle_ = openHelper();
    }
#if defined(__WIN32__) || defined(__WIN64__)
    if( nocache_ ){
      utf8::String s(getRootFromPathName(fileName_));
      BOOL r;
      DWORD sectorsPerCluster, bytesPerSector, numberOfFreeClusters, totalNumberOfClusters;
      if( isWin9x() )
        r = GetDiskFreeSpaceA(
          s.getANSIString(),
          &sectorsPerCluster,
          &bytesPerSector,
          &numberOfFreeClusters,
          &totalNumberOfClusters
        );
      else
        r = GetDiskFreeSpaceW(
          s.getUNICODEString(),
          &sectorsPerCluster,
          &bytesPerSector,
          &numberOfFreeClusters,
          &totalNumberOfClusters
        );
      if( r == 0 ){
        int32_t err = GetLastError() + errorOffset;
        close();
        newObject<EFileError>(err,fileName_)->throwSP();
      }
      alignment_ = bytesPerSector;
    }
#endif
  }
  return *this;
}
//---------------------------------------------------------------------------
bool AsyncFile::tryOpen()
{
  bool r = false;
  try {
    open();
    r = true;
  }
  catch( ExceptionSP & e ){
    oserror(e->code());
  }
  return r;
}
//---------------------------------------------------------------------------
uint64_t AsyncFile::size() const
{
#if defined(__WIN32__) || defined(__WIN64__)
  uint64 i;
  SetLastError(NO_ERROR);
  i.lo = GetFileSize(fileMember() ? file_ : handle_, &i.hi);
  if( GetLastError() != NO_ERROR ){
    int32_t err = GetLastError() + errorOffset;
    newObject<EFileError>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return i.a;
#else
  struct stat st;
  if( fstat(fileMember() ? file_ : handle_,&st) != 0 ){
    int32_t err = errno;
    newObject<EFileError>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return st.st_size;
#endif
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::resize(uint64_t nSize)
{
  uint64_t oldPos = tell();
  try{
    int32_t err;
#if defined(__WIN32__) || defined(__WIN64__)
    seek(nSize);
// TODO: check for SetFileValidData working (may be faster then SetEndOfFile)
    if( SetEndOfFile(fileMember() ? file_ : handle_) == 0 ){
      err = GetLastError() + errorOffset;
      newObject<EDiskFull>(err, __PRETTY_FUNCTION__)->throwSP();
    }
#elif HAVE_FTRUNCATE
    if( ftruncate(fileMember() ? file_ : handle_, nSize) == -1 ){
      err = errno;
      newObject<EDiskFull>(err, __PRETTY_FUNCTION__)->throwSP();
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
  if( fileMember() ){
    int64_t r = 0;
    while( size > 0 ){
      int64_t pos = seekable_ ? tell() : 0;
      fiber()->event_.position_ = pos;
      fiber()->event_.cbuffer_ = buf;
      fiber()->event_.length_ = size;
      fiber()->event_.type_ = etRead;
      fiber()->thread()->postRequest(this);
      fiber()->switchFiber(fiber()->mainFiber());
      if( fiber()->event_.errno_ != 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
        SetLastError(fiber()->event_.errno_);
        if( fiber()->event_.errno_ == ERROR_HANDLE_EOF ) break;
#else
        if( fiber()->event_.count_ == 0 ) break;
#endif
        if( r == 0 ) r = -1;
        break;
      }
      if( fiber()->event_.count_ == 0 ) break;
      buf = (uint8_t *) buf + (size_t) fiber()->event_.count_;
      r += fiber()->event_.count_;
      size -= fiber()->event_.count_;
      if( seekable_ ) seek(pos + fiber()->event_.count_);
    }
    return r;
  }
#if defined(__WIN32__) || defined(__WIN64__)
  int64_t r = 0;
  while( size > 0 ){
    DWORD rr, a = size > 1024 * 1024 * 1024 ? 1024 * 1024 * 1024 : (DWORD) size;
l1: if( ReadFile(handle_,buf,a,&rr,NULL) == 0 ){
      if( GetLastError() == ERROR_NO_SYSTEM_RESOURCES ){
        a >>= 1;
        goto l1;
      }
      if( GetLastError() == ERROR_HANDLE_EOF ) break;
      r = -1;
      break;
    }
    if( rr == 0 ) break;
    size -= rr;
    r += rr;
  }
  return r;
#else
  return ::read(handle_,buf,size);
#endif
}
//---------------------------------------------------------------------------
int64_t AsyncFile::write(const void * buf,uint64_t size)
{
  if( fileMember() ){
    int64_t w = 0;
    while( size > 0 ){
      int64_t pos = seekable_ ? tell() : 0;
      fiber()->event_.position_ = pos;
      fiber()->event_.cbuffer_ = buf;
      fiber()->event_.length_ = size;
      fiber()->event_.type_ = etWrite;
      fiber()->thread()->postRequest(this);
      fiber()->switchFiber(fiber()->mainFiber());
      if( fiber()->event_.errno_ != 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
        SetLastError(fiber()->event_.errno_);
#else
        errno = fiber()->event_.errno_;
#endif
        return -1;
      }
      buf = (uint8_t *) buf + (size_t) fiber()->event_.count_;
      w += fiber()->event_.count_;
      size -= fiber()->event_.count_;
      if( seekable_ ) seek(pos + fiber()->event_.count_);
      if( (uint64_t) w < size ) break;
    }
    return w;
  }
#if defined(__WIN32__) || defined(__WIN64__)
  int64_t w = 0;
  while( size > 0 ){
    DWORD ww, a = size > 1024 * 1024 * 1024 ? 1024 * 1024 * 1024 : (DWORD) size;
l1: if( WriteFile(handle_,buf,a,&ww,NULL) == 0 ){
      if( GetLastError() == ERROR_NO_SYSTEM_RESOURCES ){
        a >>= 1;
        goto l1;
      }
      w = -1;
      break;
    }
    if( ww == 0 ) break;
    size -= ww;
    w += ww;
  }
  return w;
#else
  return ::write(handle_,buf,size);
#endif
}
//---------------------------------------------------------------------------
int64_t AsyncFile::read(uint64_t pos,void * buf,uint64_t size)
{
  if( fileMember() ){
    int64_t r = 0;
    while( size > 0 ){
      fiber()->event_.position_ = pos;
      fiber()->event_.cbuffer_ = buf;
      fiber()->event_.length_ = size;
      fiber()->event_.type_ = etRead;
      fiber()->thread()->postRequest(this);
      fiber()->switchFiber(fiber()->mainFiber());
      if( fiber()->event_.errno_ != 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
        SetLastError(fiber()->event_.errno_);
        if( fiber()->event_.errno_ == ERROR_HANDLE_EOF ) break;
#else
        errno = fiber()->event_.errno_;
#endif
        return -1;
      }
      buf = (uint8_t *) buf + (size_t) fiber()->event_.count_;
      pos += fiber()->event_.count_;
      r += fiber()->event_.count_;
      size -= fiber()->event_.count_;
      if( r == 0 ) break;
    }
    return r;
  }
#if defined(__WIN32__) || defined(__WIN64__)
  uint64_t ps(tell());
  seek(pos);
  int64_t r = 0;
  while( size > 0 ){
    DWORD rr, a = size > 1024 * 1024 * 1024 ? 1024 * 1024 * 1024 : (DWORD) size;
l1: if( ReadFile(handle_,buf,a,&rr,NULL) == 0 ){
      if( GetLastError() == ERROR_NO_SYSTEM_RESOURCES ){
        a >>= 1;
        goto l1;
      }
      if( GetLastError() == ERROR_HANDLE_EOF ) break;
      r = -1;
      break;
    }
    if( rr == 0 ) break;
    size -= rr;
    r += rr;
  }
  seek(ps);
  return r;
#elif HAVE_PREAD
  return pread(handle_,buf,size,pos);
#else
  intptr_t r = -1;
  uint64_t OldPos = tell();
  int err;
  seek(pos);
  r = ::read(handle_,buf,size);
  err = errno;
  seek(OldPos);
  errno = err;
  return r;
#endif
}
//---------------------------------------------------------------------------
int64_t AsyncFile::write(uint64_t pos,const void * buf,uint64_t size)
{
  if( fileMember() ){
    int64_t w = 0;
    while( size > 0 ){
      fiber()->event_.position_ = pos;
      fiber()->event_.cbuffer_ = buf;
      fiber()->event_.length_ = size;
      fiber()->event_.type_ = etWrite;
      fiber()->thread()->postRequest(this);
      fiber()->switchFiber(fiber()->mainFiber());
      if( fiber()->event_.errno_ != 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
        SetLastError(fiber()->event_.errno_);
#else
        errno = fiber()->event_.errno_;
#endif
        return -1;
      }
      buf = (uint8_t *) buf + (size_t) fiber()->event_.count_;
      pos += fiber()->event_.count_;
      w += fiber()->event_.count_;
      size -= fiber()->event_.count_;
      if( w == 0 ) break;
    }
    return w;
  }
#if defined(__WIN32__) || defined(__WIN64__)
  uint64_t ps(tell());
  seek(pos);
  int64_t w = 0;
  while( size > 0 ){
    DWORD ww, a = size > 1024 * 1024 * 1024 ? 1024 * 1024 * 1024 : (DWORD) size;
l1: if( WriteFile(handle_,buf,a,&ww,NULL) == 0 ){
      if( GetLastError() == ERROR_NO_SYSTEM_RESOURCES ){
        a >>= 1;
        goto l1;
      }
      w = -1;
      break;
    }
    if( ww == 0 ) break;
    size -= ww;
    w += ww;
  }
  seek(ps);
  return w;
#elif HAVE_PWRITE
  return pwrite(handle_,buf,size,pos);
#else
  intptr_t r = -1;
  uint64_t OldPos = tell();
  int err;
  seek(pos);
  r = ::write(handle_,buf,size);
  err = errno;
  seek(OldPos);
  errno = err;
  return r;
#endif
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::readBuffer(void * buf,uint64_t size)
{
  int64_t r = read(buf,size);
  if( r == 0 && size > 0 )
    newObject<EFileEOF>(EIO,__PRETTY_FUNCTION__)->throwSP();
  if( r < 0 || (uint64_t) r != size )
    newObject<EFileError>(fiber()->event_.errno_ + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::writeBuffer(const void * buf,uint64_t size)
{
  int64_t w = write(buf,size);
  if( w < 0 && size > 0 )
    newObject<EFileError>(fiber()->event_.errno_ + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  if( (uint64_t) w != size && size > 0 )
    newObject<EFileError>(EIO, __PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::readBuffer(uint64_t pos,void * buf,uint64_t size)
{
  int64_t r = read(pos,buf,size);
  if( r == 0 && size > 0 )
    newObject<EFileEOF>(EIO,__PRETTY_FUNCTION__)->throwSP();
  if( r < 0 || (uint64_t) r != size )
    newObject<EFileError>(fiber()->event_.errno_ + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::writeBuffer(uint64_t pos,const void * buf,uint64_t size)
{
  int64_t w = write(pos, buf, size);
  if( w < 0 && size > 0 )
    newObject<EFileError>(fiber()->event_.errno_ + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  if( (uint64_t) w != size && size > 0 )
    newObject<EFileError>(EIO,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
bool AsyncFile::tryRDLock(uint64_t pos,uint64_t size)
{
  if( fileMember() ){
#if defined(__WIN32__) || defined(__WIN64__)
    fiber()->event_.errno_ = 0;
    if( !exclusive_ ){
      fiber()->event_.type_ = etLockFile;
      fiber()->event_.lockType_ = AsyncEvent::tryRDLock;
      fiber()->event_.position_ = pos;
      fiber()->event_.length_ = size;
      fiber()->thread()->postRequest(this);
      fiber()->switchFiber(fiber()->mainFiber());
      if( fiber()->event_.errno_ != 0 && fiber()->event_.errno_ != ERROR_LOCK_VIOLATION )
        newObject<EFLock>(fiber()->event_.errno_ + errorOffset, __PRETTY_FUNCTION__)->throwSP();
    }
    return fiber()->event_.errno_ == 0;
#else
    if( !exclusive_ ){
      struct flock fl;
      fl.l_start = pos;
      fl.l_len = size;
      fl.l_type = F_RDLCK;
      fl.l_whence = SEEK_SET;
      if( fcntl(file_,F_SETLKW,&fl) == 0 ) return true;
      if( errno != EAGAIN ){
        int32_t err = errno;
        newObject<EFLock>(err, __PRETTY_FUNCTION__)->throwSP();
      }
      return false;
    }
    return true;
#endif
  }
  if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( size == 0 ) size = ~UINT64_C(0);
    OVERLAPPED Overlapped;
    Overlapped.Offset = reinterpret_cast<uint64 *>(&pos)->lo;
    Overlapped.OffsetHigh = reinterpret_cast<uint64 *>(&pos)->hi;
    SetLastError(0);
    LockFileEx(handle_,LOCKFILE_FAIL_IMMEDIATELY,0,reinterpret_cast< uint64 *>(&size)->lo,reinterpret_cast< uint64 *>(&size)->hi,&Overlapped);
    DWORD err = GetLastError();
    switch( err ){
      case 0                    : return true;
      case ERROR_IO_PENDING     :
      case ERROR_LOCK_VIOLATION :
         return false;
      default                   :
        newObject<EFLock>(err + errorOffset, __PRETTY_FUNCTION__)->throwSP();
    }
#else
    struct flock fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(handle_,F_SETLKW,&fl) == 0 ) return true;
    if( errno != EAGAIN ){
      int32_t err = errno;
      newObject<EFLock>(err, __PRETTY_FUNCTION__)->throwSP();
    }
    return false;
#endif
  }
  return true;
}
//---------------------------------------------------------------------------
bool AsyncFile::tryWRLock(uint64_t pos,uint64_t size)
{
  if( fileMember() ){
#if defined(__WIN32__) || defined(__WIN64__)
    fiber()->event_.errno_ = 0;
    if( !exclusive_ ){
      fiber()->event_.type_ = etLockFile;
      fiber()->event_.lockType_ = AsyncEvent::tryWRLock;
      fiber()->event_.position_ = pos;
      fiber()->event_.length_ = size;
      fiber()->thread()->postRequest(this);
      fiber()->switchFiber(fiber()->mainFiber());
      if( fiber()->event_.errno_ != 0 && fiber()->event_.errno_ != ERROR_LOCK_VIOLATION )
        newObject<EFLock>(fiber()->event_.errno_ + errorOffset, __PRETTY_FUNCTION__)->throwSP();
    }
    return fiber()->event_.errno_ == 0;
#else
    if( !exclusive_ ){
      struct flock fl;
      fl.l_start = pos;
      fl.l_len = size;
      fl.l_type = F_WRLCK;
      fl.l_whence = SEEK_SET;
      if( fcntl(file_,F_SETLK,&fl) == 0 ) return true;
      if( errno != EAGAIN ){
        int32_t err = errno;
        newObject<EFLock>(err, __PRETTY_FUNCTION__)->throwSP();
      }
      return false;
    }
    return true;
#endif
  }
  if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( size == 0 ) size = ~UINT64_C(0);
    OVERLAPPED  Overlapped;
    Overlapped.Offset = reinterpret_cast< uint64 *>(&pos)->lo;
    Overlapped.OffsetHigh = reinterpret_cast< uint64 *>(&pos)->hi;
    SetLastError(0);
    LockFileEx(handle_, LOCKFILE_FAIL_IMMEDIATELY | LOCKFILE_EXCLUSIVE_LOCK, 0, reinterpret_cast< uint64 *>(&size)->lo, reinterpret_cast< uint64 *>(&size)->hi, &Overlapped);
    DWORD err = GetLastError();
    switch( err ){
      case 0                    : return true;
      case ERROR_IO_PENDING     :
      case ERROR_LOCK_VIOLATION :
        return false;
      default                   :
        newObject<EFLock>(err + errorOffset, __PRETTY_FUNCTION__)->throwSP();
    }
#else
    struct flock fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(handle_,F_SETLK,&fl) == 0 ) return true;
    if( errno != EAGAIN ){
      int32_t err = errno;
      newObject<EFLock>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    return false;
#endif
  }
  return true;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::rdLock(uint64_t pos,uint64_t size)
{
  if( fileMember() ){
    if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
      fiber()->event_.type_ = etLockFile;
      fiber()->event_.lockType_ = AsyncEvent::rdLock;
      fiber()->event_.position_ = pos;
      fiber()->event_.length_ = size;
      fiber()->thread()->postRequest(this);
      fiber()->switchFiber(fiber()->mainFiber());
      if( fiber()->event_.errno_ != 0 )
        newObject<EFLock>(fiber()->event_.errno_ + errorOffset, __PRETTY_FUNCTION__)->throwSP();
#else
      struct flock fl;
      fl.l_start = pos;
      fl.l_len = size;
      fl.l_type = F_RDLCK;
      fl.l_whence = SEEK_SET;
      if( fcntl(file_,F_SETLKW,&fl) != 0 ){
        int32_t err = errno;
        newObject<EFLock>(err, __PRETTY_FUNCTION__)->throwSP();
      }
#endif
    }
  }
  else {
    if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
      if( size == 0 ) size = ~UINT64_C(0);
      OVERLAPPED Overlapped;
      Overlapped.Offset = reinterpret_cast<uint64 *>(&pos)->lo;
      Overlapped.OffsetHigh = reinterpret_cast<uint64 *>(&pos)->hi;
      if( LockFileEx(handle_, 0, 0, reinterpret_cast<uint64 *>(&size)->lo,reinterpret_cast< uint64 *>(&size)->hi,&Overlapped) == 0 ){
        int32_t err = GetLastError() + errorOffset;
        newObject<EFileError>(err,__PRETTY_FUNCTION__)->throwSP();
      }
#else
      struct flock  fl;
      fl.l_start = pos;
      fl.l_len = size;
      fl.l_type = F_RDLCK;
      fl.l_whence = SEEK_SET;
      if( fcntl(handle_, F_SETLKW, &fl) != 0 ){
        int32_t err = errno;
        newObject<EFLock>(err, __PRETTY_FUNCTION__)->throwSP();
      }
#endif
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::wrLock(uint64_t pos, uint64_t size)
{
  if( fileMember() ){
    if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
      fiber()->event_.type_ = etLockFile;
      fiber()->event_.lockType_ = AsyncEvent::wrLock;
      fiber()->event_.position_ = pos;
      fiber()->event_.length_ = size;
      fiber()->thread()->postRequest(this);
      fiber()->switchFiber(fiber()->mainFiber());
      if( fiber()->event_.errno_ != 0 )
        newObject<EFLock>(fiber()->event_.errno_ + errorOffset, __PRETTY_FUNCTION__)->throwSP();
#else
      struct flock  fl;
      fl.l_start = pos;
      fl.l_len = size;
      fl.l_type = F_WRLCK;
      fl.l_whence = SEEK_SET;
      if( fcntl(file_, F_SETLKW, &fl) != 0 ){
        int32_t err = errno;
        newObject<EFLock>(err, __PRETTY_FUNCTION__)->throwSP();
      }
#endif
    }
  }
  else {
    if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
      if( size == 0 ) size = ~UINT64_C(0);
      OVERLAPPED Overlapped;
      Overlapped.Offset = reinterpret_cast<uint64 *>(&pos)->lo;
      Overlapped.OffsetHigh = reinterpret_cast<uint64 *>(&pos)->hi;
      if( LockFileEx(handle_,LOCKFILE_EXCLUSIVE_LOCK,0,reinterpret_cast<uint64 *>(&size)->lo,reinterpret_cast<uint64 *>(&size)->hi,&Overlapped) == 0 ){
        int32_t err = GetLastError() + errorOffset;
        newObject<EFileError>(err, __PRETTY_FUNCTION__)->throwSP();
      }
#else
      struct flock  fl;
      fl.l_start = pos;
      fl.l_len = size;
      fl.l_type = F_WRLCK;
      fl.l_whence = SEEK_SET;
      if( fcntl(handle_,F_SETLKW,&fl) != 0 ){
        int32_t err = errno;
        newObject<EFLock>(err, __PRETTY_FUNCTION__)->throwSP();
      }
#endif
    }
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
    Overlapped.Offset = reinterpret_cast< uint64 *>(&pos)->lo;
    Overlapped.OffsetHigh = reinterpret_cast< uint64 *>(&pos)->hi;
    if( UnlockFileEx(fileMember() ? file_ : handle_,0,reinterpret_cast<uint64 *>(&size)->lo,reinterpret_cast<uint64 *>(&size)->hi,&Overlapped) == 0 ){
      int32_t err = GetLastError() + errorOffset;
      newObject<EFileError>(err,__PRETTY_FUNCTION__)->throwSP();
    }
#else
    struct flock fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(fileMember() ? file_ : handle_,F_SETLK,&fl) != 0 ){
      int32_t err = errno;
      newObject<EFLock>(err, __PRETTY_FUNCTION__)->throwSP();
    }
#endif
  }
  return *this;
}
//---------------------------------------------------------------------------
uint64_t AsyncFile::tell() const
{
#if defined(__WIN32__) || defined(__WIN64__)
  int64 i;
  i.hi = 0;
  SetLastError(NO_ERROR);
  i.lo = SetFilePointer(fileMember() ? file_ : handle_,0,&i.hi,FILE_CURRENT);
  if( GetLastError() != NO_ERROR ){
    int32_t err = GetLastError() + errorOffset;
    newObject<EFileError>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return i.a;
#else
  int64_t pos = lseek(fileMember() ? file_ : handle_,0,SEEK_CUR);
  if( pos < 0 ){
    int32_t err = errno;
    newObject<EFileError>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return pos;
#endif
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::seek(uint64_t pos)
{
#if defined(__WIN32__) || defined(__WIN64__)
  SetLastError(NO_ERROR);
  reinterpret_cast<uint64 *>(&pos)->lo = SetFilePointer(fileMember() ? file_ : handle_,reinterpret_cast< uint64 *>(&pos)->lo,(PLONG) &reinterpret_cast<uint64 *>(&pos)->hi,FILE_BEGIN);
  if( GetLastError() != NO_ERROR ){
    int32_t err = GetLastError() + errorOffset;
    newObject<EFileError>(err, __PRETTY_FUNCTION__)->throwSP();
  }
#else
  int64_t lp = lseek(fileMember() ? file_ : handle_,pos,SEEK_SET);
  if( lp < 0 || (uint64_t) lp != pos ){
    int32_t err = errno;
    newObject<EFileError>(err, __PRETTY_FUNCTION__)->throwSP();
  }
#endif
  return *this;
}
//---------------------------------------------------------------------------
uintptr_t AsyncFile::gets(AutoPtr<char> & p,bool * eof)
{
  uint64_t op = tell();
  int64_t r, rr, l = 0;
  char * a, * q;
  if( eof != NULL ) *eof = false;
  for(;;){
    a = p.realloc(size_t(l + 512)).ptr() + l;
    rr = r = read(a,512);
    if( r <= 0 ){
      if( eof != NULL ) *eof = true;
      break;
    }
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
  p.realloc(size_t(l + (l > 0)));
  if( l > 0 ) p[uintptr_t(l)] = '\0';
  return uintptr_t(l);
}
//---------------------------------------------------------------------------
bool AsyncFile::gets(utf8::String & str,LineGetBuffer * buffer)
{
  int64_t r;
  bool eof = false;
  AutoPtr<char> s;
  if( buffer == NULL ){
    uint64_t op = tell();
    int64_t rr, l = 0;
    char * a, * q;
    for(;;){
      a = s.realloc(size_t(l + 512)).ptr() + l;
      rr = r = read(a,512);
      if( r <= 0 ){
        eof = true;
        break;
      }
      for( q = a; r > 0; q++, r-- ){
        if( *q == '\n' ){
          l = intptr_t(q - s.ptr() + (buffer->removeNewLine_ == false));
          goto l1;
        }
      }
      l += rr;
    }
l1:
    seek(op + l);
    s.realloc(size_t(l + (l > 0)));
    if( l > 0 ) s[uintptr_t(l)] = '\0';
  }
  else {
    if( buffer->size_ == 0 ) buffer->size_ = getpagesize() * 32;
    if( buffer->buffer_ == (const uint8_t *) NULL ) buffer->buffer_.alloc(buffer->size_);
    uintptr_t i, ss = 0;
    for(;;){
      if( buffer->pos_ >= buffer->len_ ){
        if( buffer->len_ == 0 ){
          buffer->bufferFilePos_ = tell();
          r = read(buffer->buffer_,buffer->size_);
        }
        else {
          r = read(buffer->bufferFilePos_ + buffer->len_,buffer->buffer_,buffer->size_);
          buffer->bufferFilePos_ += buffer->len_;
        }
        if( r <= 0 ){
          if( ss == 0 ) eof = true;
          break;
        }
        seek(buffer->bufferFilePos_);
        buffer->len_ = (uintptr_t) r;
        buffer->pos_ = 0;
      }
      bool nl = false;
      uintptr_t symSize;
      if( buffer->codePage_ == CP_UNICODE ){
        symSize = sizeof(wchar_t);
        for( i = buffer->pos_; i < buffer->len_; i += symSize )
          if( *(wchar_t *) (buffer->buffer_.ptr() + i) == L'\n' ){ i += symSize; nl = true; break; }
      }
      else {
        symSize = sizeof(char);
        for( i = buffer->pos_; i < buffer->len_; i += symSize )
          if( buffer->buffer_[i] == '\n' ){ i += symSize; nl = true; break; }
      }
      s.realloc(ss + i - buffer->pos_ + symSize - (nl && buffer->removeNewLine_) * symSize);
      memcpy(&s[ss],&buffer->buffer_[buffer->pos_],i - buffer->pos_ - (nl && buffer->removeNewLine_) * symSize);
      ss += i - buffer->pos_ - (nl && buffer->removeNewLine_) * symSize;
      if( buffer->codePage_ == CP_UNICODE ){
        *(wchar_t *) (s.ptr() + ss) = L'\0';
      }
      else {
        s[ss] = '\0';
      }
      buffer->pos_ = i;
      seek(buffer->bufferFilePos_ + i);
      if( nl ) break;
    }
  }
  if( s.ptr() == NULL ){
    str = utf8::String();
  }
  else {
    if( buffer != NULL && buffer->codePage_ != CP_UTF8 ){
      intptr_t sl = utf8::mbcs2utf8s(buffer->codePage_,NULL,0,s,~uintptr_t(0) >> 1);
      if( sl < 0 ){
        int32_t err = errno;
        newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
      AutoPtr<char> s2;
      s2.alloc(sl + 1);
      utf8::mbcs2utf8s(buffer->codePage_,s2,sl + 1,s,~uintptr_t(0) >> 1);
      s2.xchg(s);
    }
    utf8::String::Container * container = newObject<utf8::String::Container>(0,s.ptr());
    s.ptr(NULL);
    str = container;
  }
  return eof;
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
int AsyncFile::WSAEnumNetworkEvents(WSAEVENT /*hEventObject*/, DWORD /*event*/)
{
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return SOCKET_ERROR;
}
//---------------------------------------------------------------------------
BOOL AsyncFile::AcceptEx(SOCKET /*sAcceptSocket*/, PVOID /*lpOutputBuffer*/, DWORD /*dwReceiveDataLength*/, DWORD /*dwLocalAddressLength*/, DWORD /*dwRemoteAddressLength*/, LPDWORD /*lpdwBytesReceived*/, LPOVERLAPPED /*lpOverlapped*/)
{
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}
//---------------------------------------------------------------------------
BOOL AsyncFile::Connect(HANDLE /*event*/, ksys::AsyncEvent * /*request*/)
{
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}
//---------------------------------------------------------------------------
BOOL AsyncFile::Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
  return ReadFile(file_, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}
//---------------------------------------------------------------------------
BOOL AsyncFile::Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
  return WriteFile(file_, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}
//---------------------------------------------------------------------------
BOOL AsyncFile::GetOverlappedResult(LPOVERLAPPED lpOverlapped, LPDWORD lpNumberOfBytesTransferred, BOOL bWait, LPDWORD /*lpdwFlags*/)
{
  return ::GetOverlappedResult(file_, lpOverlapped, lpNumberOfBytesTransferred, bWait);
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
void AsyncFile::connect(AsyncEvent * /*request*/)
{
  errno = ENOSYS;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
void AsyncFile::shutdown2()
{
}
//------------------------------------------------------------------------------
void AsyncFile::flush2()
{
}
//------------------------------------------------------------------------------
void AsyncFile::close2()
{
}
//------------------------------------------------------------------------------
void AsyncFile::openAPI()
{
}
//------------------------------------------------------------------------------
void AsyncFile::closeAPI()
{
}
//------------------------------------------------------------------------------
void AsyncFile::redirectToStdin()
{
#if defined(__WIN32__) || defined(__WIN64__)
  HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
  if( handle == INVALID_HANDLE_VALUE ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( handle != NULL ){
    fileName("stdin");
    handle_ = handle;
    file_ = handle;
    std_ = true;
  }
#else
  Exception::throwSP(ENOSYS,__PRETTY_FUNCTION__);
#endif
}
//------------------------------------------------------------------------------
void AsyncFile::redirectToStdout()
{
#if defined(__WIN32__) || defined(__WIN64__)
  HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
  if( handle == INVALID_HANDLE_VALUE ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( handle != NULL ){
    fileName("stdout");
    handle_ = handle;
    file_ = handle;
    std_ = true;
  }
#else
  Exception::throwSP(ENOSYS,__PRETTY_FUNCTION__);
#endif
}
//---------------------------------------------------------------------------
void AsyncFile::redirectToStderr()
{
#if defined(__WIN32__) || defined(__WIN64__)
  HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
  if( handle == INVALID_HANDLE_VALUE ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( handle != NULL ){
    fileName("stderr");
    handle_ = handle;
    file_ = handle;
    std_ = true;
  }
#else
  Exception::throwSP(ENOSYS,__PRETTY_FUNCTION__);
#endif
}
//---------------------------------------------------------------------------
bool AsyncFile::redirectByName()
{
  bool r = true;
#if defined(__WIN32__) || defined(__WIN64__)
  if( fileName_.strcasecmp("stdin") == 0 ) redirectToStdin();
  else
  if( fileName_.strcasecmp("stdout") == 0 ) redirectToStdout();
  else
  if( fileName_.strcasecmp("stderr") == 0 ) redirectToStderr();
#else
  if( fileName_.strcmp("stdin") == 0 ) redirectToStdin();
  else
  if( fileName_.strcmp("stdout") == 0 ) redirectToStdout();
  else
  if( fileName_.strcmp("stderr") == 0 ) redirectToStderr();
#endif
  else
    r = false;
  return r;
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
DWORD AsyncFile::waitCommEvent()
{
  DWORD evtMask;
  if( fileMember() ){
    fiber()->event_.type_ = etWaitCommEvent;
    fiber()->event_.position_ = 0;
    fiber()->thread()->postRequest(this);
    fiber()->switchFiber(fiber()->mainFiber());
    if( fiber()->event_.errno_ != 0 )
      newObject<Exception>(fiber()->event_.errno_ + errorOffset,__PRETTY_FUNCTION__)->throwSP();
    evtMask = fiber()->event_.evtMask_;
  }
  else {
    if( WaitCommEvent(handle_,&evtMask,NULL) == 0 ){
      int32_t err = GetLastError() + errorOffset;
      newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
  }
  return evtMask;
}
#else
uint32_t AsyncFile::waitCommEvent()
{
  newObject<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
}
#endif
//---------------------------------------------------------------------------
void AsyncFile::initialize()
{
  new (mutex_) InterlockedMutex;
}
//---------------------------------------------------------------------------
void AsyncFile::cleanup()
{
  mutex().~InterlockedMutex();
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
