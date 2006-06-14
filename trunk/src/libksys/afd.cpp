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
  close(true);
}
//---------------------------------------------------------------------------
AsyncFile::AsyncFile(const utf8::String & fileName) :
  fileName_(fileName),
  exclusive_(false),
  removeAfterClose_(false),
  readOnly_(false),
  createIfNotExist_(false)
{
  file_ = INVALID_HANDLE_VALUE;
#if defined(__WIN32__) || defined(__WIN64__)
  specification_ = 1;
#endif
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::close(bool calledFromDestructor)
{
  if( file_ != INVALID_HANDLE_VALUE ){
#if defined(__WIN32__) || defined(__WIN64__)
    CloseHandle(file_);
#else
    ::close(file_);
#endif
    file_ = INVALID_HANDLE_VALUE;
  }
  if( removeAfterClose_ ){
    try {
      removeAsync(fileName_);
    }
    catch( ... ){
      if( !calledFromDestructor ) throw;
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::open()
{
  if( file_ == INVALID_HANDLE_VALUE ){
    assert( currentFiber() != NULL );
    attach();
    fiber()->event_.string0_ = fileName_;
    fiber()->event_.createIfNotExist_ = true;
    fiber()->event_.exclusive_ = exclusive_;
    fiber()->event_.readOnly_ = readOnly_;
    fiber()->event_.type_ = etOpenFile;
    fiber()->thread()->postRequest(this);
    fiber()->switchFiber(fiber()->mainFiber());
    assert( fiber()->event_.type_ == etOpenFile );
    descriptor_ = fiber()->event_.fileDescriptor_;
    if( fiber()->event_.errno_ != 0 )
      throw ksys::ExceptionSP(
        new EFileError(fiber()->event_.errno_ + errorOffset, fileName_)
      );
/*#if defined(__WIN32__) || defined(__WIN64__)
    switch( event().errno_ ){
      case ERROR_INVALID_DRIVE       :
      case ERROR_ACCESS_DENIED       :
      case ERROR_TOO_MANY_OPEN_FILES :
      case ERROR_PATH_NOT_FOUND      :
      case ERROR_FILE_NOT_FOUND      :
      case ERROR_INVALID_NAME        :
      case ERROR_INVALID_FLAGS       :
      case ERROR_INVALID_ADDRESS     :
      case ERROR_INSUFFICIENT_BUFFER :
        throw ksys::ExceptionSP(new EFileError(err + errorOffset, fileName_));
    }
#else
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
          throw ksys::ExceptionSP(new EFileError(err, fileName_));
      }
#endif
    }*/
  }
  return *this;
}
//---------------------------------------------------------------------------
bool AsyncFile::tryOpen()
{
  if( file_ == INVALID_HANDLE_VALUE ){
    assert( currentFiber() != NULL );
    attach();
    fiber()->event_.string0_ = fileName_;
    fiber()->event_.createIfNotExist_ = createIfNotExist_;
    fiber()->event_.exclusive_ = exclusive_;
    fiber()->event_.readOnly_ = readOnly_;
    fiber()->event_.type_ = etOpenFile;
    fiber()->thread()->postRequest(this);
    fiber()->switchFiber(fiber()->mainFiber());
    assert( fiber()->event_.type_ == etOpenFile );
    descriptor_ = fiber()->event_.fileDescriptor_;
  }
  return isOpen();
}
//---------------------------------------------------------------------------
uint64_t AsyncFile::size() const
{
  assert( isOpen() );
#if defined(__WIN32__) || defined(__WIN64__)
  uint64  i;
  SetLastError(NO_ERROR);
  i.lo = GetFileSize(file_, &i.hi);
  if( GetLastError() != NO_ERROR ){
    int32_t err = GetLastError() + errorOffset;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
  return i.a;
#else
  struct stat st;
  if( fstat(file_,&st) != 0 ){
    int32_t err = errno;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
  return st.st_size;
#endif
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::resize(uint64_t nSize)
{
  assert( isOpen() );
  uint64_t oldPos = tell();
  try{
    int32_t err;
#if defined(__WIN32__) || defined(__WIN64__)
    seek(nSize);
// TODO: check for SetFileValidData working (may be faster then SetEndOfFile)
    if( SetEndOfFile(file_) == 0 ){
      err = GetLastError() + errorOffset;
      throw ksys::ExceptionSP(new EDiskFull(err, __PRETTY_FUNCTION__));
    }
#elif HAVE_FTRUNCATE
    if( ftruncate(file_, nSize) == -1 ){
      err = errno;
      throw ksys::ExceptionSP(new EDiskFull(err, __PRETTY_FUNCTION__));
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
  assert( isOpen() );
  int64_t r = 0;
  while( size > 0 ){
    int64_t pos = tell();
    fiber()->event_.position_ = pos;
    fiber()->event_.cbuffer_ = buf;
    fiber()->event_.length_ = size;
    fiber()->event_.type_ = etRead;
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
    r += fiber()->event_.count_;
    size -= fiber()->event_.count_;
    seek(pos + fiber()->event_.count_);
    if( (uint64_t) r < size ) break;
  }
  return r;
}
//---------------------------------------------------------------------------
int64_t AsyncFile::write(const void * buf,uint64_t size)
{
  assert( isOpen() );
  int64_t w = 0;
  while( size > 0 ){
    int64_t pos = tell();
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
    seek(pos + fiber()->event_.count_);
    if( (uint64_t) w < size ) break;
  }
  return w;
}
//---------------------------------------------------------------------------
int64_t AsyncFile::read(uint64_t pos,void * buf,uint64_t size)
{
  assert( isOpen() );
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
#else
      errno = fiber()->event_.errno_;
#endif
      return -1;
    }
    buf = (uint8_t *) buf + (size_t) fiber()->event_.count_;
    r += fiber()->event_.count_;
    size -= fiber()->event_.count_;
    if( r == 0 ) break;
  }
  return r;
}
//---------------------------------------------------------------------------
int64_t AsyncFile::write(uint64_t pos,const void * buf,uint64_t size)
{
  assert( isOpen() );
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
    w += fiber()->event_.count_;
    size -= fiber()->event_.count_;
    if( w == 0 ) break;
  }
  return w;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::readBuffer(void * buf, uint64_t size)
{
  assert( isOpen() );
  int64_t r = read(buf, size);
  if( r == 0 && size > 0 )
    throw ksys::ExceptionSP(new EFileEOF(EIO, __PRETTY_FUNCTION__));
  if( r < 0 || (uint64_t) r != size )
    throw ksys::ExceptionSP(new EFileError(
      fiber()->event_.errno_ + errorOffset, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::writeBuffer(const void * buf, uint64_t size)
{
  assert( isOpen() );
  int64_t w = write(buf, size);
  if( w < 0 && size > 0 )
    throw ksys::ExceptionSP(new EFileError(
      fiber()->event_.errno_ + errorOffset, __PRETTY_FUNCTION__));
  if( (uint64_t) w != size && size > 0 )
    throw ksys::ExceptionSP(new EFileError(EIO, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::readBuffer(uint64_t pos, void * buf, uint64_t size)
{
  assert( isOpen() );
  int64_t r = read(pos, buf, size);
  if( r == 0 && size > 0 )
    throw ksys::ExceptionSP(new EFileEOF(EIO, __PRETTY_FUNCTION__));
  if( r < 0 || (uint64_t) r != size )
    throw ksys::ExceptionSP(new EFileError(
      fiber()->event_.errno_ + errorOffset, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::writeBuffer(uint64_t pos, const void * buf, uint64_t size)
{
  assert( isOpen() );
  int64_t w = write(pos, buf, size);
  if( w < 0 && size > 0 )
    throw ksys::ExceptionSP(
      new EFileError(fiber()->event_.errno_ + errorOffset, __PRETTY_FUNCTION__));
  if( (uint64_t) w != size && size > 0 )
    throw ksys::ExceptionSP(new EFileError(EIO, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
bool AsyncFile::tryRDLock(uint64_t pos,uint64_t size)
{
  assert( isOpen() );
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
      throw ksys::ExceptionSP(new EFLock(fiber()->event_.errno_ + errorOffset, __PRETTY_FUNCTION__));
  }
  return fiber()->event_.errno_ == 0;
#else
  if( !exclusive_ ){
    struct flock  fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(file_, F_SETLKW, &fl) == 0 ) return true;
    if( errno != EAGAIN ){
      int32_t err = errno;
      throw ksys::ExceptionSP(new EFLock(err, __PRETTY_FUNCTION__));
    }
  }
  return false;
#endif
}
//---------------------------------------------------------------------------
bool AsyncFile::tryWRLock(uint64_t pos,uint64_t size)
{
  assert( isOpen() );
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
      throw ksys::ExceptionSP(new EFLock(fiber()->event_.errno_ + errorOffset, __PRETTY_FUNCTION__));
  }
  return fiber()->event_.errno_ == 0;
#else
  if( !exclusive_ ){
    struct flock  fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(file_, F_SETLK, &fl) == 0 ) return true;
    if( errno != EAGAIN ){
      int32_t err = errno;
      throw ksys::ExceptionSP(new EFLock(err, __PRETTY_FUNCTION__));
    }
  }
  return true;
#endif
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::rdLock(uint64_t pos, uint64_t size)
{
  assert( isOpen() );
  if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
    fiber()->event_.type_ = etLockFile;
    fiber()->event_.lockType_ = AsyncEvent::rdLock;
    fiber()->event_.position_ = pos;
    fiber()->event_.length_ = size;
    fiber()->thread()->postRequest(this);
    fiber()->switchFiber(fiber()->mainFiber());
    if( fiber()->event_.errno_ != 0 )
      throw ksys::ExceptionSP(new EFLock(fiber()->event_.errno_ + errorOffset, __PRETTY_FUNCTION__));
#else
    struct flock  fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(file_, F_SETLKW, &fl) != 0 ){
      int32_t err = errno;
      throw ksys::ExceptionSP(new EFLock(err, __PRETTY_FUNCTION__));
    }
#endif
  }
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::wrLock(uint64_t pos, uint64_t size)
{
  assert( isOpen() );
  if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
    fiber()->event_.type_ = etLockFile;
    fiber()->event_.lockType_ = AsyncEvent::wrLock;
    fiber()->event_.position_ = pos;
    fiber()->event_.length_ = size;
    fiber()->thread()->postRequest(this);
    fiber()->switchFiber(fiber()->mainFiber());
    if( fiber()->event_.errno_ != 0 )
      throw ksys::ExceptionSP(new EFLock(fiber()->event_.errno_ + errorOffset, __PRETTY_FUNCTION__));
#else
    struct flock  fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(file_, F_SETLKW, &fl) != 0 ){
      int32_t err = errno;
      throw ksys::ExceptionSP(new EFLock(err, __PRETTY_FUNCTION__));
    }
#endif
  }
  return *this;
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::unLock(uint64_t pos, uint64_t size)
{
  assert( isOpen() );
  if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( size == 0 ) size = ~UINT64_C(0);
    OVERLAPPED Overlapped;
    Overlapped.Offset = reinterpret_cast< uint64 *>(&pos)->lo;
    Overlapped.OffsetHigh = reinterpret_cast< uint64 *>(&pos)->hi;
    if( UnlockFileEx(file_,0,reinterpret_cast< uint64 *>(&size)->lo,reinterpret_cast<uint64 *>(&size)->hi,&Overlapped) == 0 ){
      int32_t err = GetLastError() + errorOffset;
      throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
    }
#else
    struct flock  fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(file_, F_SETLK, &fl) != 0 ){
      int32_t err = errno;
      throw ksys::ExceptionSP(new EFLock(err, __PRETTY_FUNCTION__));
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
  i.lo = SetFilePointer(file_, 0, &i.hi, FILE_CURRENT);
  if( GetLastError() != NO_ERROR ){
    int32_t err = GetLastError() + errorOffset;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
  return i.a;
#else
  int64_t pos = lseek(file_,0,SEEK_CUR);
  if( pos < 0 ){
    int32_t err = errno;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
  return pos;
#endif
}
//---------------------------------------------------------------------------
AsyncFile & AsyncFile::seek(uint64_t pos)
{
#if defined(__WIN32__) || defined(__WIN64__)
  SetLastError(NO_ERROR);
  reinterpret_cast<uint64 *>(&pos)->lo = SetFilePointer(file_, reinterpret_cast< uint64 *>(&pos)->lo, (PLONG) &reinterpret_cast< uint64 *>(&pos)->hi, FILE_BEGIN);
  if( GetLastError() != NO_ERROR ){
    int32_t err = GetLastError() + errorOffset;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
#else
  int64_t lp = lseek(file_,pos,SEEK_SET);
  if( lp < 0 || (uint64_t) lp != pos ){
    int32_t err = errno;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
#endif
  return *this;
}
//---------------------------------------------------------------------------
uintptr_t AsyncFile::gets(AutoPtr<char> & p,bool * eof)
{
  uint64_t op = tell();
  intptr_t r, rr, l = 0;
  char * a, * q;
  if( eof != NULL ) *eof = false;
  for(;;){
    a = p.realloc(l + getpagesize()).ptr() + l;
    rr = r = (intptr_t) read(a,getpagesize());
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
  p.realloc(l + (l > 0));
  if( l > 0 ) p[l] = '\0';
  return l;
}
//---------------------------------------------------------------------------
utf8::String AsyncFile::gets(bool * eof)
{
  uint64_t op = tell();
  intptr_t r, rr, l  = 0;
  char * a, * q;
  AutoPtr<char>  p;
  if( eof != NULL ) *eof = false;
  for(;;){
    a = p.realloc(l + getpagesize()).ptr() + l;
    rr = r = (intptr_t) read(a,getpagesize());
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
  p.realloc(l + (l > 0));
  if( l > 0 ) p[l] = '\0';
  utf8::String::Container * container = new utf8::String::Container(0,p.ptr());
  p.ptr(NULL);
  return container;
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
void AsyncFile::connect(ksys::IoRequest * /*request*/)
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
} // namespace ksys
//---------------------------------------------------------------------------
