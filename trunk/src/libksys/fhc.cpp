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
FileHandleContainer::~FileHandleContainer()
{
  close();
}
//---------------------------------------------------------------------------
FileHandleContainer::FileHandleContainer(const utf8::String & fileName)
  : handle_(INVALID_HANDLE_VALUE),
    fileName_(fileName),
    exclusive_(false),
    removeAfterClose_(false),
    readOnly_(false),
    createIfNotExist_(false)
{
}
//---------------------------------------------------------------------------
FileHandleContainer & FileHandleContainer::close()
{
  if( handle_ != INVALID_HANDLE_VALUE ){
#if defined(__WIN32__) || defined(__WIN64__)
    CloseHandle(handle_);
#else
    ::close(handle_);
#endif
    handle_ = INVALID_HANDLE_VALUE;
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
FileHandleContainer & FileHandleContainer::open()
{
  if( handle_ == INVALID_HANDLE_VALUE ){
    for(;;){
#if defined(__WIN32__) || defined(__WIN64__)
      int32_t err;
      if( isWin9x() ){
        utf8::AnsiString  ansiFileName  (anyPathName2HostPathName(fileName_).getANSIString());
        if( !readOnly_ )
          handle_ = CreateFileA(
            ansiFileName,GENERIC_READ | GENERIC_WRITE,
            exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            createIfNotExist_ ? OPEN_ALWAYS : OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
            NULL
          );
        if( handle_ == INVALID_HANDLE_VALUE )
          handle_ = CreateFileA(
            ansiFileName,GENERIC_READ,
            exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            createIfNotExist_ ? OPEN_ALWAYS : OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
            NULL
          );
      }
      else{
        utf8::WideString unicodeFileName(anyPathName2HostPathName(fileName_).getUNICODEString());
        if( !readOnly_ )
          handle_ = CreateFileW(
            unicodeFileName,
            GENERIC_READ | GENERIC_WRITE,
            exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            createIfNotExist_ ? OPEN_ALWAYS : OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
            NULL
          );
        if( handle_ == INVALID_HANDLE_VALUE )
          handle_ = CreateFileW(
            unicodeFileName,GENERIC_READ,
            exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            createIfNotExist_ ? OPEN_ALWAYS : OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
            NULL
          );
      }
      if( handle_ != INVALID_HANDLE_VALUE ) break;
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
          throw ksys::ExceptionSP(new EFileError(err + errorOffset, fileName_));
      }
#else
      utf8::AnsiString  ansiFileName  (anyPathName2HostPathName(fileName_).getANSIString());
      mode_t            um            = umask(0);
      umask(um);
      if( !readOnly_ )
        handle_ = ::open(ansiFileName, O_RDWR | O_CREAT | (exclusive_ ? O_EXLOCK : 0), um | S_IRUSR | S_IWUSR);
      if( handle_ < 0 )
        handle_ = ::open(ansiFileName, O_RDONLY | O_CREAT | (exclusive_ ? O_EXLOCK : 0), um | S_IRUSR | S_IWUSR);
      if( handle_ >= 0 )
        break;
      switch( errno ){
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
          throw ksys::ExceptionSP(new EFileError(errno, fileName_));
      }
#endif
      ksys::sleep1();
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
bool FileHandleContainer::tryOpen(bool createIfNotExist)
{
  if( handle_ == INVALID_HANDLE_VALUE ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( isWin9x() ){
      utf8::AnsiString  ansiFileName  (anyPathName2HostPathName(fileName_).getANSIString());
      if( !readOnly_ )
        handle_ = CreateFileA(ansiFileName, GENERIC_READ | GENERIC_WRITE, exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, createIfNotExist ? OPEN_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS, NULL);
      if( handle_ == INVALID_HANDLE_VALUE )
        handle_ = CreateFileA(ansiFileName, GENERIC_READ, exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, createIfNotExist ? OPEN_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS, NULL);
    }
    else{
      utf8::WideString  unicodeFileName (anyPathName2HostPathName(fileName_).getUNICODEString());
      if( !readOnly_ )
        handle_ = CreateFileW(unicodeFileName, GENERIC_READ | GENERIC_WRITE, exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, createIfNotExist ? OPEN_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS, NULL);
      if( handle_ == INVALID_HANDLE_VALUE )
        handle_ = CreateFileW(unicodeFileName, GENERIC_READ, exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, createIfNotExist ? OPEN_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS, NULL);
    }
#else
    utf8::AnsiString  ansiFileName  (anyPathName2HostPathName(fileName_).getANSIString());
    mode_t            um            = umask(0);
    umask(um);
    if( !readOnly_ )
      handle_ = ::open(ansiFileName, O_RDWR | (createIfNotExist ? O_CREAT : 0) | (exclusive_ ? O_EXLOCK : 0), um | S_IRUSR | S_IWUSR);
    if( handle_ < 0 )
      handle_ = ::open(ansiFileName, O_RDONLY | (createIfNotExist ? O_CREAT : 0) | (exclusive_ ? O_EXLOCK : 0), um | S_IRUSR | S_IWUSR);
#endif
  }
  return isOpen();
}
//---------------------------------------------------------------------------
uint64_t FileHandleContainer::size() const
{
#if defined(__WIN32__) || defined(__WIN64__)
  uint64  i;
  SetLastError(NO_ERROR);
  i.lo = GetFileSize(handle_,&i.hi);
  if( GetLastError() != NO_ERROR ){
    int32_t err = GetLastError() + errorOffset;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
  return i.a;
#else
  struct stat st;
  if( fstat(handle_,&st) != 0 ){
    int32_t err = errno;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
  return st.st_size;
#endif
}
//---------------------------------------------------------------------------
FileHandleContainer & FileHandleContainer::resize(uint64_t nSize)
{
  uint64_t oldPos = tell();
  try{
    int err;
#if defined(__WIN32__) || defined(__WIN64__)
    seek(nSize);
// TODO: check for SetFileValidData working (may be faster then SetEndOfFile)
    if( SetEndOfFile(handle_) == 0 ){
      err = GetLastError() + errorOffset;
      throw ksys::ExceptionSP(new EDiskFull(err, __PRETTY_FUNCTION__));
    }
#elif HAVE_FTRUNCATE
    if( ftruncate(handle_, nSize) == -1 ){
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
int64_t FileHandleContainer::read(void * buf, uint64_t size)
{
#if defined(__WIN32__) || defined(__WIN64__)
  int64_t rr  = 0;
  while( size > 0 ){
    DWORD r, a  = size > 1024 * 1024 * 1024 ? 1024 * 1024 * 1024 : (DWORD) size;
    if( ReadFile(handle_, buf, a, &r, NULL) == 0 ){
      if( GetLastError() == ERROR_HANDLE_EOF ) break;
      rr = -1;
      break;
    }
    if( r == 0 ) break;
    size -= r;
    rr += r;
  }
  return rr;
#else
  return ::read(handle_, buf, size);
#endif
}
//---------------------------------------------------------------------------
int64_t FileHandleContainer::write(const void * buf, uint64_t size)
{
#if defined(__WIN32__) || defined(__WIN64__)
  int64_t ww  = 0;
  while( size > 0 ){
    DWORD w, a  = size > 1024 * 1024 * 1024 ? 1024 * 1024 * 1024 : (DWORD) size;
    if( WriteFile(handle_, buf, a, &w, NULL) == 0 ){
      ww = -1;
      break;
    }
    if( w == 0 ) break;
    size -= w;
    ww += w;
  }
  return ww;
#else
  return ::write(handle_, buf, size);
#endif
}
//---------------------------------------------------------------------------
int64_t FileHandleContainer::read(uint64_t pos, void * buf, uint64_t size)
{
#if defined(__WIN32__) || defined(__WIN64__)
  uint64_t  ps  (tell());
  seek(pos);
  int64_t rr  = 0;
  while( size > 0 ){
    DWORD r, a  = size > 1024 * 1024 * 1024 ? 1024 * 1024 * 1024 : (DWORD) size;
    if( ReadFile(handle_, buf, a, &r, NULL) == 0 ){
      if( GetLastError() == ERROR_HANDLE_EOF ) break;
      rr = -1;
      break;
    }
    if( r == 0 ) break;
    size -= r;
    rr += r;
  }
  seek(ps);
  return rr;
#elif HAVE_PREAD
  return pread(handle_, buf, size, pos);
#else
  intptr_t  r       = -1;
  uint64_t  OldPos  = tell();
  int       err;
  seek(pos);
  r = ::read(handle_, buf, size);
  err = errno;
  seek(OldPos);
  errno = err;
  return r;
#endif
}
//---------------------------------------------------------------------------
int64_t FileHandleContainer::write(uint64_t pos, const void * buf, uint64_t size)
{
#if defined(__WIN32__) || defined(__WIN64__)
  uint64_t  ps  (tell());
  seek(pos);
  int64_t ww  = 0;
  while( size > 0 ){
    DWORD w, a  = size > 1024 * 1024 * 1024 ? 1024 * 1024 * 1024 : (DWORD) size;
    if( WriteFile(handle_, buf, a, &w, NULL) == 0 ){
      ww = -1;
      break;
    }
    if( w == 0 ) break;
    size -= w;
    ww += w;
  }
  seek(ps);
  return ww;
#elif HAVE_PWRITE
  return pwrite(handle_, buf, size, pos);
#else
  intptr_t  r       = -1;
  uint64_t  OldPos  = tell();
  int       err;
  seek(pos);
  r = ::write(handle_, buf, size);
  err = errno;
  seek(OldPos);
  errno = err;
  return r;
#endif
}
//---------------------------------------------------------------------------
FileHandleContainer & FileHandleContainer::readBuffer(void * buf, uint64_t size)
{
#if defined(__WIN32__) || defined(__WIN64__)
  int64_t r = read(buf, size);
  if( r < 0 ){
    int32_t err = GetLastError() + errorOffset;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
  if( r == 0 )
    throw ksys::ExceptionSP(new EFileEOF(EIO, __PRETTY_FUNCTION__));
  if( (uint64_t) r < size )
    throw ksys::ExceptionSP(new EFileError(EIO, __PRETTY_FUNCTION__));
#else
  ssize_t r;
  if( (r = ::read(handle_, buf, size) < 0) ){
    int32_t err = errno;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
  if( r == 0 )
    throw ksys::ExceptionSP(new EFileEOF(EIO, __PRETTY_FUNCTION__));
  if( (uint64_t) r < size )
    throw ksys::ExceptionSP(new EFileError(EIO, __PRETTY_FUNCTION__));
#endif
  return *this;
}
//---------------------------------------------------------------------------
FileHandleContainer & FileHandleContainer::writeBuffer(const void * buf, uint64_t size)
{
#if defined(__WIN32__) || defined(__WIN64__)
  int64_t w = write(buf, size);
  if( w < 0 ){
    int32_t err = GetLastError() + errorOffset;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
  if( (uint64_t) w < size )
    throw ksys::ExceptionSP(new EFileError(EIO, __PRETTY_FUNCTION__));
#else
  if( (uint64_t) ::write(handle_, buf, size) != size ){
    int32_t err = errno;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
#endif
  return *this;
}
//---------------------------------------------------------------------------
FileHandleContainer & FileHandleContainer::readBuffer(uint64_t pos, void * buf, uint64_t size)
{
#if defined(__WIN32__) || defined(__WIN64__)
  int64_t r = read(pos, buf, size);
  if( r < 0 || (uint64_t) r != size ){
    int32_t err = GetLastError() + errorOffset;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
#else
  ssize_t r;
#if HAVE_PREAD
  r = pread(handle_, buf, size, pos);
#else
  uint64_t  OldPos  = tell();
  r = ::read(handle_, buf, size);
  int err = errno;
  seek(OldPos);
  errno = err;
#endif
  if( r < 0 || (uint64_t) r < size )
    throw ksys::ExceptionSP(new EFileError(errno, __PRETTY_FUNCTION__));
  if( r == 0 )
    throw ksys::ExceptionSP(new EFileEOF(errno, __PRETTY_FUNCTION__));
#endif
  return *this;
}
//---------------------------------------------------------------------------
FileHandleContainer & FileHandleContainer::writeBuffer(uint64_t pos, const void * buf, uint64_t size)
{
#if defined(__WIN32__) || defined(__WIN64__)
  int64_t w = write(pos, buf, size);
  if( w < 0 || (uint64_t) w != size ){
    int32_t err = GetLastError() + errorOffset;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
#else
  ssize_t w;
#if HAVE_PWRITE
  w = pwrite(handle_, buf, size, pos);
#else
  uint64_t  OldPos  = tell();
  w = ::write(handle_, buf, size);
  int err = errno;
  seek(OldPos);
  errno = err;
#endif
  if( w < 0 || (uint64_t) w < size ){
    int err = errno;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
  if( w == 0 ){
    int err = errno;
    throw ksys::ExceptionSP(new EFileEOF(err, __PRETTY_FUNCTION__));
  }
#endif
  return *this;
}
//---------------------------------------------------------------------------
bool FileHandleContainer::tryRDLock(uint64_t pos, uint64_t size)
{
  if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( size == 0 ) size = ~UINT64_C(0);
    OVERLAPPED  Overlapped;
    Overlapped.Offset = reinterpret_cast< uint64 *>(&pos)->lo;
    Overlapped.OffsetHigh = reinterpret_cast< uint64 *>(&pos)->hi;
    SetLastError(0);
    LockFileEx(handle_, LOCKFILE_FAIL_IMMEDIATELY, 0, reinterpret_cast< uint64 *>(&size)->lo, reinterpret_cast< uint64 *>(&size)->hi, &Overlapped);
    DWORD err = GetLastError();
    switch( err ){
      case 0                : return true;
      case ERROR_IO_PENDING : return false;
      default               :
        throw ksys::ExceptionSP(new EFLock(err + errorOffset, __PRETTY_FUNCTION__));
    }
#else
    struct flock  fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(handle_, F_SETLKW, &fl) == 0 ) return true;
    if( errno != EAGAIN ){
      int32_t err = errno;
      throw ksys::ExceptionSP(new EFLock(err, __PRETTY_FUNCTION__));
    }
    return false;
#endif
  }
  return true;
}
//---------------------------------------------------------------------------
bool FileHandleContainer::tryWRLock(uint64_t pos, uint64_t size)
{
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
      case 0                : return true;
      case ERROR_IO_PENDING : return false;
      default               :
        throw ksys::ExceptionSP(new EFLock(err + errorOffset, __PRETTY_FUNCTION__));
    }
#else
    struct flock  fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(handle_, F_SETLK, &fl) == 0 ) return true;
    if( errno != EAGAIN ){
      int32_t err = errno;
      throw ksys::ExceptionSP(new EFLock(err, __PRETTY_FUNCTION__));
    }
    return false;
#endif
  }
  return true;
}
//---------------------------------------------------------------------------
FileHandleContainer & FileHandleContainer::rdLock(uint64_t pos, uint64_t size)
{
  if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( size == 0 ) size = ~UINT64_C(0);
    OVERLAPPED  Overlapped;
    Overlapped.Offset = reinterpret_cast< uint64 *>(&pos)->lo;
    Overlapped.OffsetHigh = reinterpret_cast< uint64 *>(&pos)->hi;
    if( LockFileEx(handle_, 0, 0, reinterpret_cast< uint64 *>(&size)->lo, reinterpret_cast< uint64 *>(&size)->hi, &Overlapped) == 0 ){
      int32_t err = GetLastError() + errorOffset;
      throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
    }
#else
    struct flock  fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(handle_, F_SETLKW, &fl) != 0 ){
      int32_t err = errno;
      throw ksys::ExceptionSP(new EFLock(err, __PRETTY_FUNCTION__));
    }
#endif
  }
  return *this;
}
//---------------------------------------------------------------------------
FileHandleContainer & FileHandleContainer::wrLock(uint64_t pos, uint64_t size)
{
  if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( size == 0 ) size = ~UINT64_C(0);
    OVERLAPPED  Overlapped;
    Overlapped.Offset = reinterpret_cast<uint64 *>(&pos)->lo;
    Overlapped.OffsetHigh = reinterpret_cast<uint64 *>(&pos)->hi;
    if( LockFileEx(handle_, LOCKFILE_EXCLUSIVE_LOCK, 0, reinterpret_cast< uint64 *>(&size)->lo, reinterpret_cast< uint64 *>(&size)->hi, &Overlapped) == 0 ){
      int32_t err = GetLastError() + errorOffset;
      throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
    }
#else
    struct flock  fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(handle_, F_SETLKW, &fl) != 0 ){
      int32_t err = errno;
      throw ksys::ExceptionSP(new EFLock(err, __PRETTY_FUNCTION__));
    }
#endif
  }
  return *this;
}
//---------------------------------------------------------------------------
FileHandleContainer & FileHandleContainer::unLock(uint64_t pos, uint64_t size)
{
  if( !exclusive_ ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( size == 0 ) size = ~UINT64_C(0);
    OVERLAPPED Overlapped;
    Overlapped.Offset = reinterpret_cast< uint64 *>(&pos)->lo;
    Overlapped.OffsetHigh = reinterpret_cast< uint64 *>(&pos)->hi;
    if( UnlockFileEx(handle_, 0, reinterpret_cast< uint64 *>(&size)->lo, reinterpret_cast< uint64 *>(&size)->hi, &Overlapped) == 0 ){
      int32_t err = GetLastError() + errorOffset;
      throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
    }
#else
    struct flock  fl;
    fl.l_start = pos;
    fl.l_len = size;
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    if( fcntl(handle_, F_SETLK, &fl) != 0 ){
      int32_t err = errno;
      throw ksys::ExceptionSP(new EFLock(err, __PRETTY_FUNCTION__));
    }
#endif
  }
  return *this;
}
//---------------------------------------------------------------------------
uint64_t FileHandleContainer::tell() const
{
#if defined(__WIN32__) || defined(__WIN64__)
  int64 i;
  i.hi = 0;
  SetLastError(NO_ERROR);
  i.lo = SetFilePointer(handle_, 0, &i.hi, FILE_CURRENT);
  if( GetLastError() != NO_ERROR ){
    int32_t err = GetLastError() + errorOffset;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
  return i.a;
#else
  int64_t pos = lseek(handle_, 0, SEEK_CUR);
  if( pos < 0 ){
    int32_t err = errno;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
  return pos;
#endif
}
//---------------------------------------------------------------------------
FileHandleContainer & FileHandleContainer::seek(uint64_t pos)
{
#if defined(__WIN32__) || defined(__WIN64__)
  SetLastError(NO_ERROR);
  reinterpret_cast<uint64 *>(&pos)->lo = SetFilePointer(handle_, reinterpret_cast< uint64 *>(&pos)->lo, (PLONG) &reinterpret_cast< uint64 *>(&pos)->hi, FILE_BEGIN);
  if( GetLastError() != NO_ERROR ){
    int32_t err = GetLastError() + errorOffset;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
#else
  int64_t lp = lseek(handle_, pos, SEEK_SET);
  if( lp < 0 || (uint64_t) lp != pos ){
    int32_t err = errno;
    throw ksys::ExceptionSP(new EFileError(err, __PRETTY_FUNCTION__));
  }
#endif
  return *this;
}
//---------------------------------------------------------------------------
uintptr_t FileHandleContainer::gets(AutoPtr<char> & p)
{
  uint64_t op = tell();
  intptr_t r, rr, l = 0;
  char * a, * q;
  for(;;){
    a = p.realloc(l + 256).ptr() + l;
    rr = r = (intptr_t) read(a, 256);
    if( r <= 0 ) break;
    for( q = a; r > 0; q++, r-- ){
      if( *q == '\n' ){
        l = intptr_t(q - p.ptr() + 1);
        goto l1;
      }
    }
    l += rr;
  }
  l1 : seek(op + l);
  p.realloc(l + (l > 0));
  if( l > 0 ) p[l] = '\0';
  return l;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
EFileError::EFileError(int32_t code, const utf8::String & what) : Exception(code, what)
{
}
//---------------------------------------------------------------------------
EFileEOF::EFileEOF(int32_t code, const utf8::String & what) : Exception(code, what)
{
}  
//---------------------------------------------------------------------------
EDiskFull::EDiskFull(int32_t code, const utf8::String & what) : Exception(code, what)
{
}
//---------------------------------------------------------------------------
EFLock::EFLock(int32_t code, const utf8::String & what) : Exception(code, what)
{
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
