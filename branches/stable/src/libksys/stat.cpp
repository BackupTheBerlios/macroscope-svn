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
#define EPOCH_BIAS UINT64_C(116444592000000000)
//---------------------------------------------------------------------------
bool stat(const utf8::String & pathName,struct Stat & st)
{
  if( currentFiber() != NULL ){
    currentFiber()->event_.string0_ = pathName;
    currentFiber()->event_.stat_ = &st;
    currentFiber()->event_.type_ = etStat;
    currentFiber()->thread()->postRequest();
    currentFiber()->switchFiber(currentFiber()->mainFiber());
    assert( currentFiber()->event_.type_ == etStat );
    if( currentFiber()->event_.errno_ != 0 &&
        currentFiber()->event_.errno_ != ERROR_PATH_NOT_FOUND + errorOffset &&
        currentFiber()->event_.errno_ != ERROR_FILE_NOT_FOUND + errorOffset )
      throw ksys::ExceptionSP(
        newObject<EFileError>(currentFiber()->event_.errno_,__PRETTY_FUNCTION__)
      );
    return currentFiber()->event_.rval_;
  }
  int32_t err = 0;
  memset(&st,0,sizeof(st));
#if defined(__WIN32__) || defined(__WIN64__)
  WIN32_FILE_ATTRIBUTE_DATA fData;
  memset(&fData, 0, sizeof(fData));
  bool    fgfaEx  = true;
  HANDLE  hFile   = INVALID_HANDLE_VALUE;
  if( isWin9x() ){
    utf8::AnsiString s(anyPathName2HostPathName(pathName).getANSIString());
    SetLastError(ERROR_SUCCESS);
    GetFileAttributesExA(s,GetFileExInfoStandard,&fData);
    if( GetLastError() == ERROR_CALL_NOT_IMPLEMENTED ){
      SetLastError(ERROR_SUCCESS);
      fData.dwFileAttributes = GetFileAttributesA(s);
      fgfaEx = false;
    }
    if( GetLastError() == ERROR_SUCCESS && (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
      hFile = CreateFileA(s,FILE_READ_ATTRIBUTES,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  }
  else {
    utf8::WideString s(anyPathName2HostPathName(pathName).getUNICODEString());
    SetLastError(ERROR_SUCCESS);
    GetFileAttributesExW(s, GetFileExInfoStandard, &fData);
    if( GetLastError() == ERROR_CALL_NOT_IMPLEMENTED ){
      SetLastError(ERROR_SUCCESS);
      fData.dwFileAttributes = GetFileAttributesW(s);
      fgfaEx = false;
    }
    if( GetLastError() == ERROR_SUCCESS && (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
      hFile = CreateFileW(s,FILE_READ_ATTRIBUTES,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
  }
  if( GetLastError() != ERROR_SUCCESS ){
    err = GetLastError();
    goto done;
  }
  st.st_ino = st.st_uid = st.st_gid = st.st_mode = 0;
  st.st_nlink = 1;
  if( fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
    st.st_mode = S_IFDIR;
  }
  else{
    int isdev = GetFileType(hFile) & ~FILE_TYPE_REMOTE;
    if( isdev != FILE_TYPE_DISK ){
      if( (isdev == FILE_TYPE_CHAR) || (isdev == FILE_TYPE_PIPE) ){
        if( isdev == FILE_TYPE_CHAR )
          st.st_mode = S_IFCHR;
        else
          st.st_mode = _S_IFIFO;
        //        st.st_rdev = st.st_dev = (dev_t) hFile;
        st.st_nlink = 1;
        st.st_uid = st.st_gid = st.st_ino = 0;
        st.st_atime = st.st_mtime = st.st_ctime = 0;
        if( isdev == FILE_TYPE_CHAR ){
          st.st_size = INT64_C(0);
        }
        else{
          unsigned long ulAvail;
          int           rc;
          rc = PeekNamedPipe(hFile, NULL, 0, NULL, &ulAvail, NULL);
          if( rc ){
            st.st_size = (off_t) ulAvail;
          }
          else{
            st.st_size = (off_t) 0;
          }
        }
        goto done;
      }
      else if( isdev == FILE_TYPE_UNKNOWN ){
        errno = EBADF;
        err = -1;
        goto done;
      }
      else{
        err = GetLastError();
        goto done;
      }
    }
    else{
      st.st_mode = S_IFREG;
    }
  }
  if( !fgfaEx ){
    BY_HANDLE_FILE_INFORMATION  fileInformation;
    if( GetFileInformationByHandle(hFile, &fileInformation) == 0 ){
      err = GetLastError();
      goto done;
    }
    fData.ftCreationTime = fileInformation.ftCreationTime;
    fData.ftLastAccessTime = fileInformation.ftLastAccessTime;
    fData.ftLastWriteTime = fileInformation.ftLastWriteTime;
    fData.nFileSizeHigh = fileInformation.nFileSizeHigh;
    fData.nFileSizeLow = fileInformation.nFileSizeLow;
    st.st_nlink = (int16_t) fileInformation.nNumberOfLinks;
  }
  if( fData.dwFileAttributes & FILE_ATTRIBUTE_READONLY )
    st.st_mode |= (S_IREAD + (S_IREAD >> 3) + (S_IREAD >> 6));
  else
    st.st_mode |= ((S_IREAD | S_IWRITE) + ((S_IREAD | S_IWRITE) >> 3) + ((S_IREAD | S_IWRITE) >> 6));
  st.st_size = uint64_t(fData.nFileSizeLow) + (uint64_t(fData.nFileSizeHigh) << 32);
  st.st_ctime = (time_t) ((*((uint64_t *) &fData.ftCreationTime) - EPOCH_BIAS) / UINT64_C(10000000));
  st.st_atime = (time_t) ((*((uint64_t *) &fData.ftLastAccessTime) - EPOCH_BIAS) / UINT64_C(10000000));
  st.st_mtime = (time_t) ((*((uint64_t *) &fData.ftLastWriteTime) - EPOCH_BIAS) / UINT64_C(10000000));
  st.st_rdev = st.st_dev = 0;
done:
  CloseHandle(hFile);
  SetLastError(err);
  if( err != 0 && err != ERROR_PATH_NOT_FOUND && err != ERROR_FILE_NOT_FOUND )
    Exception::throwSP(err + errorOffset, __PRETTY_FUNCTION__);
#else
  if( stat(anyPathName2HostPathName(pathName).getANSIString(), &st) != 0 ){
    err = errno;
    if( err != ENOENT )
      Exception::throwSP(err, __PRETTY_FUNCTION__);
  }
#endif
  return err == 0;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
