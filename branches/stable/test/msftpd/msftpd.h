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
//------------------------------------------------------------------------------
#ifndef _msftp_H_
#define _msftp_H_
//------------------------------------------------------------------------------
enum MSFTPCmdType {
  cmQuit,
  cmUser,
  cmPassword,
  cmEncryption,
  cmCompression,
  cmList,
  cmStat,
  cmSetTimes,
  cmResize,
  cmPutFile,
  cmGetFile
};
//------------------------------------------------------------------------------
enum MSFTPError {
  eOK,
  eFail = ksock::AsyncSocket::aeCount, // 2007
  eMkDir,                              // 2008
  eFileOpen,                           // 2009
  eFileWrite,                          // 2010
  eFileStat,                           // 2011
  eSetTimes,                           // 2012
  eBadPathName,                        // 2013
  eResize,                             // 2014
  eDiskFull,                           // 2015
  eCount
};
//------------------------------------------------------------------------------
struct MSFTPStat {
  int16_t st_dev_;
  int16_t st_ino_;
  int16_t st_mode_;
  int16_t st_nlink_;
  int32_t st_uid_;
  int32_t st_gid_;
  int16_t st_rdev_;
  int64_t st_size_;
  int64_t st_atime_;
  int64_t st_mtime_;
  int64_t st_ctime_;

  bool stat(const utf8::String & pathName);
  bool statAsync(const utf8::String & pathName);
};
//---------------------------------------------------------------------------
inline bool MSFTPStat::stat(const utf8::String & pathName)
{
  memset(this,0,sizeof(MSFTPStat));
  ksys::Stat st;
  bool isSt = ksys::stat(pathName,st);
  if( isSt ){
    st_dev_ = (int16_t) st.st_dev;
    st_ino_ = st.st_ino;
    st_mode_ = st.st_mode;
    st_nlink_ = st.st_nlink;
    st_uid_ = st.st_uid;
    st_gid_ = st.st_gid;
    st_rdev_ = (int16_t) st.st_rdev;
    st_size_ = st.st_size;
    st_atime_ = st.st_atime;
    st_mtime_ = st.st_mtime;
    st_ctime_ = st.st_ctime;
  }
  return isSt;
}
//---------------------------------------------------------------------------
inline bool MSFTPStat::statAsync(const utf8::String & pathName)
{
  memset(this,0,sizeof(MSFTPStat));
  ksys::Stat st;
  bool isSt = ksys::stat(pathName,st);
  if( isSt ){
    st_dev_ = (int16_t) st.st_dev;
    st_ino_ = st.st_ino;
    st_mode_ = st.st_mode;
    st_nlink_ = st.st_nlink;
    st_uid_ = st.st_uid;
    st_gid_ = st.st_gid;
    st_rdev_ = (int16_t) st.st_rdev;
    st_size_ = st.st_size;
    st_atime_ = st.st_atime;
    st_mtime_ = st.st_mtime;
    st_ctime_ = st.st_ctime;
  }
  return isSt;
}
//---------------------------------------------------------------------------
const int MSFTPDefaultPort = 2121;
//---------------------------------------------------------------------------
#endif
