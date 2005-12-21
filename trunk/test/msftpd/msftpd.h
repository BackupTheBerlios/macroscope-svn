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
  cmGetFile,
};
//------------------------------------------------------------------------------
enum MSFTPError {
  eOK,
  eFail = ksock::AsyncSocket::aeCount, // 2007
  eFileOpen,                           // 2008
  eFileWrite,                          // 2009
  eFileStat,                           // 2010
  eSetTimes,                           // 2011
  eBadPathName,                        // 2012
  eResize,                             // 2013
  eDiskFull,                           // 2014
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
};
//---------------------------------------------------------------------------
inline bool MSFTPStat::stat(const utf8::String & pathName)
{
  memset(this,0,sizeof(MSFTPStat));
  ksys::Stat st;
  bool isSt = ksys::stat(pathName,st);
  if( isSt ){
    st_dev_ = st.st_dev;
    st_ino_ = st.st_ino;
    st_mode_ = st.st_mode;
    st_nlink_ = st.st_nlink;
    st_uid_ = st.st_uid;
    st_gid_ = st.st_gid;
    st_rdev_ = st.st_rdev;
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
