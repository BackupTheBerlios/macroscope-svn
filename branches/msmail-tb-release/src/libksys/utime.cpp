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
#define EPOCH_BIAS  UINT64_C(116444592000000000)
//---------------------------------------------------------------------------
bool utime(const utf8::String & path, const struct utimbuf & times)
{
  long    err = 0;
#if defined(__WIN32__) || defined(__WIN64__)
  HANDLE  hFile;
  if( isWin9x() ){
    hFile = CreateFileA(anyPathName2HostPathName(path).getANSIString(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
  }
  else{
    hFile = CreateFileW(anyPathName2HostPathName(path).getUNICODEString(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
  }
  if( hFile == INVALID_HANDLE_VALUE ){
    err = GetLastError() + errorOffset;
    throw ExceptionSP(new Exception(err, __PRETTY_FUNCTION__));
  }
  union {
      LARGE_INTEGER lat;
      FILETIME      lastAccessTime;
  };
  union {
      LARGE_INTEGER lwt;
      FILETIME      lastWriteTime;
  };
  lat.QuadPart = times.actime * UINT64_C(10000000) + EPOCH_BIAS;
  lwt.QuadPart = times.modtime * UINT64_C(10000000) + EPOCH_BIAS;
  if( SetFileTime(hFile, NULL, &lastAccessTime, &lastWriteTime) == 0 ){
    err = GetLastError() + errorOffset;
    CloseHandle(hFile);
    throw ExceptionSP(new Exception(err, __PRETTY_FUNCTION__));
  }
  CloseHandle(hFile);
#else
  if( utime(anyPathName2HostPathName(path).getANSIString(), &times) != 0 ){
    err = errno;
    throw ExceptionSP(new Exception(err, __PRETTY_FUNCTION__));
  }
#endif
  return err == 0;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------

