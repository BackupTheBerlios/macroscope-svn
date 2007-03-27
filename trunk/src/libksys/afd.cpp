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
  close();
#if defined(__WIN32__) || defined(__WIN64__)
  if( eventObject_ != NULL ){
    CloseHandle(eventObject_);
    eventObject_ = NULL;
  }
#endif
  Requester::requester().detachDescriptor(*this);
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
  atty_(false),
  random_(false),
  direct_(false),
  nocache_(false),
  createPath_(true)
{
  Requester::requester().attachDescriptor(*this);
#if defined(__WIN32__) || defined(__WIN64__)
  alignment_ = 1;
  eventObject_ = CreateEventA(NULL,TRUE,FALSE,NULL);
  if( eventObject_ == NULL ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
  }
#endif
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::close()
{
  bool closed = false;
  {
    AutoLock<InterlockedMutex> lock(mutex());
    if( descriptor_ != INVALID_HANDLE_VALUE ){
      if( !std_ ){
#if defined(__WIN32__) || defined(__WIN64__)
        CloseHandle(descriptor_);
#else
        ::close(descriptor_);
#endif
      }
      descriptor_ = INVALID_HANDLE_VALUE;
      closed = true;
    }
  }
  if( closed ){
    if( removeAfterClose_ ){
      try {
        remove(fileName_);
      }
      catch( ... ){}
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
file_t AsyncFile::openHelper(bool async)
{
  async = true;
  file_t handle = INVALID_HANDLE_VALUE;
  int32_t err;
#if defined(__WIN32__) || defined(__WIN64__)
  SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
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
    else if( handle == INVALID_HANDLE_VALUE )
      handle = CreateFileA(
        ansiFileName,
	GENERIC_READ,
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
  else {
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
    else if( handle == INVALID_HANDLE_VALUE )
      handle = CreateFileW(
        unicodeFileName,
	GENERIC_READ,
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
    err = GetLastError();
    if( err == ERROR_PATH_NOT_FOUND && createPath_ && !readOnly_ ){
      createDirectory(getPathFromPathName(fileName_));
      return openHelper(async);
    }
    newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
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
      O_RDWR |
      (createIfNotExist_ ? O_CREAT : 0) |
      (exclusive_ ? O_EXLOCK : 0) | (direct_ ? O_DIRECT : 0),
      um | S_IRUSR | S_IWUSR
    );
  else if( handle < 0 )
    handle = ::open(
      ansiFileName,
      O_RDONLY |
      (createIfNotExist_ ? O_CREAT : 0) |
      (exclusive_ ? O_EXLOCK : 0) | (direct_ ? O_DIRECT : 0),
      um | S_IRUSR | S_IWUSR
    );
  if( handle <= 0 ){
    err = errno;
    if( err == ENOENT && createPath_ && !readOnly_ && !stat(getPathFromPathName(fileName_)) )
      err = ENOTDIR;
    if( err == ENOTDIR && createPath_ && !readOnly_ ){
      createDirectory(getPathFromPathName(fileName_));
      return openHelper(async);
    }
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
  }
  if( async ){
    int flags = fcntl(handle,F_GETFL,0);
    if( flags == -1 || fcntl(handle,F_SETFL,flags | O_NONBLOCK) == -1 ){
      err = errno;
      ::close(handle);
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
    }
  }
#endif
  return handle;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::open()
{
  if( !isOpen() && !redirectByName() ){
    Fiber * fiber = currentFiber();
    if( fiber != NULL ){
      fiber->event_.timeout_ = ~uint64_t(0);
      fiber->event_.string0_ = fileName_;
      fiber->event_.createIfNotExist_ = createIfNotExist_;
      fiber->event_.exclusive_ = exclusive_;
      fiber->event_.readOnly_ = readOnly_;
      fiber->event_.file_ = this;
      fiber->event_.type_ = etOpenFile;
      fiber->thread()->postRequest(this);
      fiber->switchFiber(fiber->mainFiber());
      assert( fiber->event_.type_ == etOpenFile );
      if( fiber->event_.errno_ != 0 )
        newObjectV1C2<Exception>(fiber->event_.errno_,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
      descriptor_ = fiber->event_.fileDescriptor_;
    }
    else {
      descriptor_ = openHelper(true);
    }
#if defined(__WIN32__) || defined(__WIN64__)
    if( fileName_.strncasecmp("COM",3) == 0 ){
      utf8::String::Iterator i(fileName_);
      i += 3;
      while( i.isDigit() ) i.next();
      seekable_ = i.getChar() != ':' || !(i + 1).eof();
    }
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
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
      }
      alignment_ = bytesPerSector;
    }
#else
#if HAVE_ISATTY
    atty_ = isatty(descriptor_); 
#elif HAVE__ISATTY
    atty_ = _isatty(descriptor_); 
#endif
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
  SetLastError(ERROR_SUCCESS);
  i.lo = GetFileSize(descriptor_, &i.hi);
  if( GetLastError() != ERROR_SUCCESS ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
  }
  return i.a;
#else
  struct stat st;
  if( fstat(descriptor_,&st) != 0 ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
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
/*!
 * \todo check for SetFileValidData working (may be faster then SetEndOfFile)
 */
    if( SetEndOfFile(descriptor_) == 0 ){
      err = GetLastError() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
    }
#elif HAVE_FTRUNCATE
    if( ftruncate(descriptor_,nSize) == -1 ){
      err = errno;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
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
int64_t AsyncFile::read(uint64_t pos,void * buf,uint64_t size)
{
  Fiber * fiber = currentFiber();
  if( fiber != NULL ){
    int64_t r = 0;
    while( size > 0 ){
      fiber->event_.timeout_ = ~uint64_t(0);
      fiber->event_.position_ = pos;
      fiber->event_.cbuffer_ = buf;
      fiber->event_.length_ = size;
      fiber->event_.type_ = etRead;
      fiber->thread()->postRequest(this);
      fiber->switchFiber(fiber->mainFiber());
      if( fiber->event_.errno_ != 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
        SetLastError(fiber->event_.errno_);
        if( fiber->event_.errno_ == ERROR_HANDLE_EOF ) break;
#else
        if( fiber->event_.count_ == 0 ) break;
#endif
        if( r == 0 ) r = -1;
        break;
      }
      if( fiber->event_.count_ == 0 ) break;
      buf = (uint8_t *) buf + (size_t) fiber->event_.count_;
      size -= fiber->event_.count_;
      if( seekable_ ) pos += fiber->event_.count_;
      r += fiber->event_.count_;
    }
    return r;
  }
#if defined(__WIN32__) || defined(__WIN64__)
  OVERLAPPED overlapped;
  memset(&overlapped,0,sizeof(overlapped));
  overlapped.hEvent = eventObject_;
  int64_t r = 0;
  while( size > 0 ){
    overlapped.Offset = (DWORD) pos;
    overlapped.OffsetHigh = (DWORD) (pos >> 32);
    DWORD a = size > 0x40000000 ? 0x40000000 : (DWORD) size, rr;
    BOOL status;
    for(;;){
      if( ResetEvent(overlapped.hEvent) == 0 ){
        int32_t err = errno;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
      }
      SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
      status = ReadFile(descriptor_,buf,a,&rr,std_ || !seekable_ ? NULL : &overlapped);
      if( status != 0 || GetLastError() != ERROR_NO_SYSTEM_RESOURCES ) break;
      a >>= 1;
    }
    if( status == 0 && GetLastError() == ERROR_IO_PENDING ){
      status = ::GetOverlappedResult(descriptor_,&overlapped,&rr,TRUE);
      if( status != 0 ) SetLastError(ERROR_SUCCESS);
    }
    if( GetLastError() == ERROR_HANDLE_EOF ) break;
    if( GetLastError() != ERROR_SUCCESS ){
      if( r == 0 ) r = -1;
      break;
    }
    if( rr == 0 ) break;
    buf = (uint8_t *) buf + rr;
    size -= rr;
    if( seekable_ ) pos += rr;
    r += rr;
  }
  return r;
#elif HAVE_PREAD
  return seekable_ ? pread(descriptor_,buf,size,pos) : ::read(descriptor_,buf,size);
#else
  intptr_t r = -1;
  uint64_t OldPos = seekable_ ? tell() : 0;
  int err;
  if( seekable_ ) seek(pos);
  r = ::read(descriptor_,buf,size);
  err = errno;
  if( seekable_ ) seek(OldPos);
  errno = err;
  return r;
#endif
}
//---------------------------------------------------------------------------
int64_t AsyncFile::write(uint64_t pos,const void * buf,uint64_t size)
{
  Fiber * fiber = currentFiber();
  if( fiber != NULL ){
    int64_t w = 0;
    while( size > 0 ){
      fiber->event_.timeout_ = ~uint64_t(0);
      fiber->event_.position_ = pos;
      fiber->event_.cbuffer_ = buf;
      fiber->event_.length_ = size;
      fiber->event_.type_ = etWrite;
      fiber->thread()->postRequest(this);
      fiber->switchFiber(fiber->mainFiber());
      if( fiber->event_.errno_ != 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
        SetLastError(fiber->event_.errno_);
#else
        errno = fiber->event_.errno_;
#endif
        if( w == 0 ) w = -1;
        break;
      }
      buf = (const uint8_t *) buf + (size_t) fiber->event_.count_;
      size -= fiber->event_.count_;
      if( seekable_ ) pos += fiber->event_.count_;
      w += fiber->event_.count_;
    }
    return w;
  }
#if defined(__WIN32__) || defined(__WIN64__)
  OVERLAPPED overlapped;
  memset(&overlapped,0,sizeof(overlapped));
  overlapped.hEvent = eventObject_;
  int64_t w = 0;
  while( size > 0 ){
    overlapped.Offset = (DWORD) pos;
    overlapped.OffsetHigh = (DWORD) (pos >> 32);
    DWORD a = size > 0x40000000 ? 0x40000000 : (DWORD) size, ww;
    BOOL status;
    for(;;){
      if( ResetEvent(overlapped.hEvent) == 0 ){
        int32_t err = errno;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
      }
      SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
      status = WriteFile(descriptor_,buf,a,&ww,std_ || !seekable_ ? NULL : &overlapped);
      if( status != 0 || GetLastError() != ERROR_NO_SYSTEM_RESOURCES ) break;
      a >>= 1;
    }
    if( status == 0 && GetLastError() == ERROR_IO_PENDING ){
      status = ::GetOverlappedResult(descriptor_,&overlapped,&ww,TRUE);
      if( status != 0 ) SetLastError(ERROR_SUCCESS);
    }
    if( GetLastError() != ERROR_SUCCESS ){
      if( w == 0 ) w = -1;
      break;
    }
    if( ww == 0 ) break;
    buf = (const uint8_t *) buf + ww;
    size -= ww;
    if( seekable_ ) pos += ww;
    w += ww;
  }
  return w;
#elif HAVE_PWRITE
  return seekable_ ? pwrite(descriptor_,buf,size,pos) : ::write(descriptor_,buf,size);
#else
  intptr_t r = -1;
  uint64_t OldPos = seekable_ ? tell() : 0;
  int err;
  if( seekable_ ) seek(pos);
  r = ::write(descriptor_,buf,size);
  err = errno;
  if( seekable_ ) seek(OldPos);
  errno = err;
  return r;
#endif
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::readBuffer(void * buf,uint64_t size)
{
  if( size > 0 ){
    int64_t r = read(buf,size);
    if( r < 0 ){
      int32_t err = oserror() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
    }
    if( r == 0 || (uint64_t) r != size )
      newObjectV1C2<EFileEOF>(EIO,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
  }
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::writeBuffer(const void * buf,uint64_t size)
{
  if( size > 0 ){
    int64_t w = write(buf,size);
    if( w < 0 ){
      int32_t err = oserror() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
    }
    if( (uint64_t) w != size )
      newObjectV1C2<Exception>(EIO,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
  }
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::readBuffer(uint64_t pos,void * buf,uint64_t size)
{
  if( size > 0 ){
    int64_t r = read(pos,buf,size);
    if( r < 0 ){
      int32_t err = oserror() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
    }
    if( r == 0 || (uint64_t) r != size )
      newObjectV1C2<EFileEOF>(EIO,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
  }
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::writeBuffer(uint64_t pos,const void * buf,uint64_t size)
{
  if( size > 0 ){
    int64_t w = write(pos,buf,size);
    if( w < 0 ){
      int32_t err = oserror() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
    }
    if( (uint64_t) w != size )
      newObjectV1C2<Exception>(EIO,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
  }
  return *this;
}
//---------------------------------------------------------------------------
bool AsyncFile::tryRDLock(uint64_t pos,uint64_t size)
{
  if( isRunInFiber() ){
#if defined(__WIN32__) || defined(__WIN64__)
    Fiber * fiber = currentFiber();
    fiber->event_.errno_ = 0;
    if( !exclusive_ ){
      fiber->event_.timeout_ = ~uint64_t(0);
      fiber->event_.type_ = etLockFile;
      fiber->event_.lockType_ = AsyncEvent::tryRDLock;
      fiber->event_.position_ = pos;
      fiber->event_.length_ = size;
      fiber->thread()->postRequest(this);
      fiber->switchFiber(fiber->mainFiber());
      if( fiber->event_.errno_ != 0 && fiber->event_.errno_ != ERROR_LOCK_VIOLATION )
        newObjectV1C2<Exception>(fiber->event_.errno_ + errorOffset,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
    }
    return fiber->event_.errno_ == 0;
#else
    if( !exclusive_ ){
      struct flock fl;
      fl.l_start = pos;
      fl.l_len = size;
      fl.l_type = F_RDLCK;
      fl.l_whence = SEEK_SET;
      if( fcntl(descriptor_,F_SETLKW,&fl) == 0 ) return true;
      if( errno != EAGAIN ){
        int32_t err = errno;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
      }
      return false;
    }
    return true;
#endif
  }
  if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( size == 0 ) size = ~UINT64_C(0);
    OVERLAPPED overlapped;
    memset(&overlapped,0,sizeof(overlapped));
    overlapped.Offset = reinterpret_cast<uint64 *>(&pos)->lo;
    overlapped.OffsetHigh = reinterpret_cast<uint64 *>(&pos)->hi;
    overlapped.hEvent = eventObject_;
    if( ResetEvent(overlapped.hEvent) == 0 ){
      int32_t err = errno;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
    }
    SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
    BOOL status = LockFileEx(
      descriptor_,
      LOCKFILE_FAIL_IMMEDIATELY,
      0,
      reinterpret_cast<uint64 *>(&size)->lo,
      reinterpret_cast<uint64 *>(&size)->hi,
      &overlapped
    );
    DWORD err;
    if( status == 0 && GetLastError() == ERROR_IO_PENDING ){
      status = ::GetOverlappedResult(descriptor_,&overlapped,&err,TRUE);
      if( status != 0 ) return GetLastError() != ERROR_LOCK_VIOLATION;
    }
    err = GetLastError();
    switch( err ){
      case ERROR_LOCK_VIOLATION : return false;
      default                   :
        newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
    }
#else
    struct flock fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(descriptor_,F_SETLKW,&fl) == 0 ) return true;
    if( errno != EAGAIN ){
      int32_t err = errno;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
    }
    return false;
#endif
  }
  return true;
}
//---------------------------------------------------------------------------
bool AsyncFile::tryWRLock(uint64_t pos,uint64_t size)
{
  if( isRunInFiber() ){
#if defined(__WIN32__) || defined(__WIN64__)
    Fiber * fiber = currentFiber();
    fiber->event_.errno_ = 0;
    if( !exclusive_ ){
      fiber->event_.timeout_ = ~uint64_t(0);
      fiber->event_.type_ = etLockFile;
      fiber->event_.lockType_ = AsyncEvent::tryWRLock;
      fiber->event_.position_ = pos;
      fiber->event_.length_ = size;
      fiber->thread()->postRequest(this);
      fiber->switchFiber(fiber->mainFiber());
      if( fiber->event_.errno_ != 0 && fiber->event_.errno_ != ERROR_LOCK_VIOLATION )
        newObjectV1C2<Exception>(fiber->event_.errno_ + errorOffset,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
    }
    return fiber->event_.errno_ == 0;
#else
    if( !exclusive_ ){
      struct flock fl;
      fl.l_start = pos;
      fl.l_len = size;
      fl.l_type = F_WRLCK;
      fl.l_whence = SEEK_SET;
      if( fcntl(descriptor_,F_SETLK,&fl) == 0 ) return true;
      if( errno != EAGAIN ){
        int32_t err = errno;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
      }
      return false;
    }
    return true;
#endif
  }
  if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( size == 0 ) size = ~UINT64_C(0);
    OVERLAPPED overlapped;
    memset(&overlapped,0,sizeof(overlapped));
    overlapped.Offset = reinterpret_cast<uint64 *>(&pos)->lo;
    overlapped.OffsetHigh = reinterpret_cast<uint64 *>(&pos)->hi;
    overlapped.hEvent = eventObject_;
    if( ResetEvent(overlapped.hEvent) == 0 ){
      int32_t err = errno;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
    }
    SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
    BOOL status = LockFileEx(
      descriptor_,
      LOCKFILE_FAIL_IMMEDIATELY | LOCKFILE_EXCLUSIVE_LOCK,
      0,
      reinterpret_cast<uint64 *>(&size)->lo,
      reinterpret_cast<uint64 *>(&size)->hi,
      &overlapped
    );
    DWORD err;
    if( status == 0 && GetLastError() == ERROR_IO_PENDING ){
      status = ::GetOverlappedResult(descriptor_,&overlapped,&err,TRUE);
      if( status != 0 ) return GetLastError() != ERROR_LOCK_VIOLATION;
    }
    err = GetLastError();
    switch( err ){
      case ERROR_LOCK_VIOLATION : return false;
      default                   :
        newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
    }
#else
    struct flock fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(descriptor_,F_SETLK,&fl) == 0 ) return true;
    if( errno != EAGAIN ){
      int32_t err = errno;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
    }
    return false;
#endif
  }
  return true;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::rdLock(uint64_t pos,uint64_t size)
{
  if( isRunInFiber() ){
    if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
      Fiber * fiber = currentFiber();
      fiber->event_.timeout_ = ~uint64_t(0);
      fiber->event_.type_ = etLockFile;
      fiber->event_.lockType_ = AsyncEvent::rdLock;
      fiber->event_.position_ = pos;
      fiber->event_.length_ = size;
      fiber->thread()->postRequest(this);
      fiber->switchFiber(fiber->mainFiber());
      if( fiber->event_.errno_ != 0 )
        newObjectV1C2<Exception>(fiber->event_.errno_ + errorOffset,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
#else
      struct flock fl;
      fl.l_start = pos;
      fl.l_len = size;
      fl.l_type = F_RDLCK;
      fl.l_whence = SEEK_SET;
      if( fcntl(descriptor_,F_SETLKW,&fl) != 0 ){
        int32_t err = errno;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
      }
#endif
    }
  }
  else {
    if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
      if( size == 0 ) size = ~UINT64_C(0);
      OVERLAPPED overlapped;
      memset(&overlapped,0,sizeof(overlapped));
      overlapped.Offset = reinterpret_cast<uint64 *>(&pos)->lo;
      overlapped.OffsetHigh = reinterpret_cast<uint64 *>(&pos)->hi;
      overlapped.hEvent = eventObject_;
      if( ResetEvent(overlapped.hEvent) == 0 ){
        int32_t err = errno;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
      }
      SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
      BOOL status = LockFileEx(
        descriptor_,
        0,
        0,
        reinterpret_cast<uint64 *>(&size)->lo,
        reinterpret_cast<uint64 *>(&size)->hi,
        &overlapped
      );
      DWORD err;
      if( status == 0 && GetLastError() == ERROR_IO_PENDING ){
        status = ::GetOverlappedResult(descriptor_,&overlapped,&err,TRUE);
        if( status != 0 ) SetLastError(ERROR_SUCCESS);
      }
      err = GetLastError();
      switch( err ){
        case ERROR_SUCCESS        : break;
        default                   :
          newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
      }
#else
      struct flock  fl;
      fl.l_start = pos;
      fl.l_len = size;
      fl.l_type = F_RDLCK;
      fl.l_whence = SEEK_SET;
      if( fcntl(descriptor_, F_SETLKW, &fl) != 0 ){
        int32_t err = errno;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
      }
#endif
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::wrLock(uint64_t pos,uint64_t size)
{
  if( isRunInFiber() ){
    if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
      Fiber * fiber = currentFiber();
      fiber->event_.timeout_ = ~uint64_t(0);
      fiber->event_.type_ = etLockFile;
      fiber->event_.lockType_ = AsyncEvent::wrLock;
      fiber->event_.position_ = pos;
      fiber->event_.length_ = size;
      fiber->thread()->postRequest(this);
      fiber->switchFiber(fiber->mainFiber());
      if( fiber->event_.errno_ != 0 )
        newObjectV1C2<Exception>(fiber->event_.errno_ + errorOffset,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
#else
      struct flock  fl;
      fl.l_start = pos;
      fl.l_len = size;
      fl.l_type = F_WRLCK;
      fl.l_whence = SEEK_SET;
      if( fcntl(descriptor_, F_SETLKW, &fl) != 0 ){
        int32_t err = errno;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
      }
#endif
    }
  }
  else {
    if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
      if( size == 0 ) size = ~UINT64_C(0);
      OVERLAPPED overlapped;
      memset(&overlapped,0,sizeof(overlapped));
      overlapped.Offset = reinterpret_cast<uint64 *>(&pos)->lo;
      overlapped.OffsetHigh = reinterpret_cast<uint64 *>(&pos)->hi;
      overlapped.hEvent = eventObject_;
      if( ResetEvent(overlapped.hEvent) == 0 ){
        int32_t err = errno;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
      }
      SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
      BOOL status = LockFileEx(
        descriptor_,
        LOCKFILE_EXCLUSIVE_LOCK,
        0,
        reinterpret_cast<uint64 *>(&size)->lo,
        reinterpret_cast<uint64 *>(&size)->hi,
        &overlapped
      );
      DWORD err;
      if( status == 0 && GetLastError() == ERROR_IO_PENDING ){
        status = ::GetOverlappedResult(descriptor_,&overlapped,&err,TRUE);
        if( status != 0 ) SetLastError(ERROR_SUCCESS);
      }
      err = GetLastError();
      switch( err ){
        case ERROR_SUCCESS        : break;
        default                   :
          newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
      }
#else
      struct flock  fl;
      fl.l_start = pos;
      fl.l_len = size;
      fl.l_type = F_WRLCK;
      fl.l_whence = SEEK_SET;
      if( fcntl(descriptor_,F_SETLKW,&fl) != 0 ){
        int32_t err = errno;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
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
    OVERLAPPED overlapped;
    memset(&overlapped,0,sizeof(overlapped));
    overlapped.Offset = reinterpret_cast<uint64 *>(&pos)->lo;
    overlapped.OffsetHigh = reinterpret_cast<uint64 *>(&pos)->hi;
    BOOL status = UnlockFileEx(
      descriptor_,
      0,
      reinterpret_cast<uint64 *>(&size)->lo,
      reinterpret_cast<uint64 *>(&size)->hi,
      &overlapped
    );
    if( status == 0 ){
      int32_t err = GetLastError() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
    }
#else
    struct flock fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(descriptor_,F_SETLK,&fl) != 0 ){
      int32_t err = errno;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
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
  SetLastError(ERROR_SUCCESS);
  i.lo = SetFilePointer(descriptor_,0,&i.hi,FILE_CURRENT);
  if( GetLastError() != ERROR_SUCCESS ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
  }
  return i.a;
#else
  int64_t pos = lseek(descriptor_,0,SEEK_CUR);
  if( pos < 0 ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
  }
  return pos;
#endif
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::seek(uint64_t pos)
{
#if defined(__WIN32__) || defined(__WIN64__)
  SetLastError(ERROR_SUCCESS);
  reinterpret_cast<uint64 *>(&pos)->lo = SetFilePointer(descriptor_,reinterpret_cast<uint64 *>(&pos)->lo,(PLONG) &reinterpret_cast<uint64 *>(&pos)->hi,FILE_BEGIN);
  if( GetLastError() != ERROR_SUCCESS ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
  }
#else
  int64_t lp = lseek(descriptor_,pos,SEEK_SET);
  if( lp < 0 || (uint64_t) lp != pos ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + fileName_)->throwSP();
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
    if( buffer->size_ == 0 ) buffer->size_ = getpagesize() * 16;
    if( buffer->buffer_ == (const uint8_t *) NULL ) buffer->buffer_.alloc(buffer->size_);
    uintptr_t i, ss = 0;
    for(;;){
      if( buffer->pos_ >= buffer->len_ ){
        if( buffer->len_ == 0 ){
          buffer->bufferFilePos_ = tell();
          r = read(buffer->buffer_,buffer->size_);
        }
        else {
      	  if( seekable_ )
            r = read(buffer->bufferFilePos_ + buffer->len_,buffer->buffer_,buffer->size_);
	        else
            r = read(buffer->buffer_,buffer->size_);
          buffer->bufferFilePos_ += buffer->pos_;
          buffer->pos_ = buffer->len_ = 0;
        }
        if( r <= 0 ){
          if( ss == 0 ) eof = true;
          break;
        }
        if( seekable_ ) seek(buffer->bufferFilePos_);
        buffer->len_ = (uintptr_t) r;
        buffer->pos_ = 0;
      }
      if( buffer->detectUnicodeFFFE_ && buffer->bufferFilePos_ == 0 && buffer->pos_ == 0 && buffer->len_ >= 2 &&
          buffer->buffer_[0] == 0xFF && buffer->buffer_[1] == 0xFE ){
        buffer->pos_ += 2;
	      buffer->codePage_ = CP_UNICODE;
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
      if( seekable_ ) seek(buffer->bufferFilePos_ + i);
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
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
      AutoPtr<char> s2;
      s2.alloc(sl + 1);
      utf8::mbcs2utf8s(buffer->codePage_,s2,sl + 1,s,~uintptr_t(0) >> 1);
      s2.xchg(s);
    }
    utf8::String::Container * container = newObjectV1V2<utf8::String::Container>(0,s.ptr());
    s.ptr(NULL);
    str = container;
  }
  return eof;
}
//---------------------------------------------------------------------------
uint64_t AsyncFile::LineGetBuffer::tell()
{
  return bufferFilePos_ + pos_;
}
//---------------------------------------------------------------------------
AsyncFile::LineGetBuffer & AsyncFile::LineGetBuffer::seek(uint64_t pos)
{
  if( pos < bufferFilePos_ || pos >= bufferFilePos_ + len_ ){
    file_->seek(pos);
    bufferFilePos_ = pos;
    pos_ = 0;
    len_ = 0;
  }
  else {
    file_->seek(pos);
    pos_ = uintptr_t(pos - bufferFilePos_);
  }
  return *this;
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
  return ReadFile(descriptor_, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}
//---------------------------------------------------------------------------
BOOL AsyncFile::Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
  return WriteFile(descriptor_, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}
//---------------------------------------------------------------------------
BOOL AsyncFile::GetOverlappedResult(LPOVERLAPPED lpOverlapped, LPDWORD lpNumberOfBytesTransferred, BOOL bWait, LPDWORD /*lpdwFlags*/)
{
  return ::GetOverlappedResult(descriptor_, lpOverlapped, lpNumberOfBytesTransferred, bWait);
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
  close();
#if defined(__WIN32__) || defined(__WIN64__)
  HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
  if( handle == INVALID_HANDLE_VALUE ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( handle != NULL ){
    fileName("stdin");
    descriptor_ = handle;
    std_ = true;
    seekable_ = false;
  }
  else {
    std_ = false;
    seekable_ = true;
  }
#else
  descriptor_ = STDIN_FILENO;
  std_ = true;
  seekable_ = false;
///  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
#endif
}
//------------------------------------------------------------------------------
void AsyncFile::redirectToStdout()
{
  close();
#if defined(__WIN32__) || defined(__WIN64__)
  HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
  if( handle == INVALID_HANDLE_VALUE ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( handle != NULL ){
    fileName("stdout");
    descriptor_ = handle;
    std_ = true;
    seekable_ = false;
  }
  else {
    std_ = false;
    seekable_ = true;
  }
#else
  descriptor_ = STDOUT_FILENO;
  std_ = true;
  seekable_ = false;
#endif
}
//---------------------------------------------------------------------------
void AsyncFile::redirectToStderr()
{
  close();
#if defined(__WIN32__) || defined(__WIN64__)
  HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
  if( handle == INVALID_HANDLE_VALUE ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( handle != NULL ){
    fileName("stderr");
    descriptor_ = handle;
    std_ = true;
    seekable_ = false;
  }
  else {
    std_ = false;
    seekable_ = true;
  }
#else
  descriptor_ = STDERR_FILENO;
  std_ = true;
  seekable_ = false;
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
  if( isRunInFiber() ){
    Fiber * fiber = currentFiber();
    fiber->event_.timeout_ = ~uint64_t(0);
    fiber->event_.type_ = etWaitCommEvent;
    fiber->event_.position_ = 0;
    fiber->thread()->postRequest(this);
    fiber->switchFiber(fiber->mainFiber());
    if( fiber->event_.errno_ != 0 )
      newObjectV1C2<Exception>(fiber->event_.errno_ + errorOffset,__PRETTY_FUNCTION__)->throwSP();
    evtMask = fiber->event_.evtMask_;
  }
  else {
    if( WaitCommEvent(descriptor_,&evtMask,NULL) == 0 ){
      int32_t err = GetLastError() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
  }
  return evtMask;
}
#else
uint32_t AsyncFile::waitCommEvent()
{
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
  return 0;
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
