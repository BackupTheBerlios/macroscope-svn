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
#ifndef _Sysutils_H_
#define _Sysutils_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
extern bool stackBackTrace;
utf8::String getBackTrace(/*intptr_t flags = pdbutils::DbgFrameGetAll,*/intptr_t skipCount = 0,Thread * thread = NULL);
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DirectoryChangeNotification {
  public:
    ~DirectoryChangeNotification();
    DirectoryChangeNotification();

#if defined(__WIN32__) || defined(__WIN64__)
    const HANDLE & hFFCNotification() const;
    const HANDLE & hDirectory() const;
    FILE_NOTIFY_INFORMATION * const buffer() const;
    const uintptr_t & bufferSize() const; 
#endif
    void monitor(const utf8::String & pathName,uint64_t timeout = ~uint64_t(0));
    void stop();
  protected:
  private:
#if defined(__WIN32__) || defined(__WIN64__)
    HANDLE hFFCNotification_; // from FindFirstChangeNotification for FindNextChangeNotification
    HANDLE hDirectory_; // for ReadDirectoryChangesW
    AutoPtr<FILE_NOTIFY_INFORMATION> buffer_; // for ReadDirectoryChangesW
    uintptr_t bufferSize_;
#endif
    union {
      uint8_t createPath_       : 1;
    };    
};
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
inline const HANDLE & DirectoryChangeNotification::hFFCNotification() const
{
  return hFFCNotification_;
}
//---------------------------------------------------------------------------
inline const HANDLE & DirectoryChangeNotification::hDirectory() const
{
  return hDirectory_;
}
//---------------------------------------------------------------------------
inline FILE_NOTIFY_INFORMATION * const DirectoryChangeNotification::buffer() const
{
  return buffer_;
}
//---------------------------------------------------------------------------
inline const uintptr_t & DirectoryChangeNotification::bufferSize() const
{
  return bufferSize_;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
uintptr_t rfcBase64Encode(
  const uint8_t * inStr,
  size_t inLen, 
  uint8_t * outStr,
  size_t outLen
);
utf8::String base64Encode(const void * p,uintptr_t l);
uintptr_t base64Decode(const utf8::String & s,void * p,uintptr_t size);
utf8::String base32Encode(const void * p,uintptr_t l);
uintptr_t base32Decode(const utf8::String & s,void * p,uintptr_t size);
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
extern uint8_t argvPlaceHolder[sizeof(Array< utf8::String>)];
inline Array<utf8::String> & argv()
{
  return *reinterpret_cast<Array<utf8::String> *>(argvPlaceHolder);
}
extern char pathDelimiter;
extern char pathDelimiterStr[2];
//---------------------------------------------------------------------------
void  initializeArguments(int argc,char ** argv);
void  initialize(int argc,char ** argv);
void  cleanup();
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
extern DWORD  mainThreadId;
extern UINT   threadFinishMessage;
extern UINT   fiberFinishMessage;
inline bool isWin9x()
{
  DWORD dwVersion = GetVersion();
  return (dwVersion & (DWORD(1u) << 31)) != 0 && (dwVersion & 0xFF) == 4;
}
inline bool isWinNT4()
{
  DWORD dwVersion = GetVersion();
  return (dwVersion & (DWORD(1u) << 31)) == 0 && (dwVersion & 0xFF) == 4;
}
inline bool isWinNT5()
{
  DWORD dwVersion = GetVersion();
  return (dwVersion & (DWORD(1u) << 31)) == 0 && (dwVersion & 0xFF) == 5;
}
inline bool isWinXPorLater()
{
  DWORD dwVersion = GetVersion();
// Get the Windows version.
  DWORD dwWindowsMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
  DWORD dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));
// Get the build number.
/*  DWORD dwBuild;
  if( dwVersion < 0x80000000 )
    dwBuild = (DWORD)(HIWORD(dwVersion));
  else // Windows Me/98/95
    dwBuild = 0;*/
  return dwWindowsMajorVersion > 5 || (dwWindowsMajorVersion == 5 && dwWindowsMinorVersion >= 1);
}

bool isWow64();

#else
inline bool isWin9x()
{
  return false;
}
inline bool isWinNT4()
{
  return false;
}
inline bool isWinNT5()
{
  return false;
}
inline bool isWinXPorLater()
{
  return false;
}
inline bool isWow64(){
  return false;
}
#endif
//---------------------------------------------------------------------------
#ifndef DOXYGEN_SHOULD_SKIP_THIS
utf8::String getMachineUniqueKey();
utf8::String getMachineCleanUniqueKey();
class SHA256Cryptor;
utf8::String getMachineCryptedUniqueKey(const utf8::String & text,const utf8::String & info = utf8::String(),SHA256Cryptor * decryptor = NULL);
extern uint8_t machineUniqueCryptedKeyHolder[];
inline utf8::String & machineUniqueCryptedKey()
{
  return *reinterpret_cast<utf8::String *>(machineUniqueCryptedKeyHolder);
}
#if PRIVATE_RELEASE
bool checkMachineBinding(const utf8::String & key,bool abortProgram = false);
#else
inline bool checkMachineBinding(const utf8::String &,bool abortProgram = false){ return false; }
#endif
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
//---------------------------------------------------------------------------
class guid_t
#if SIZEOF_UUID_T
 : public uuid_t
#elif SIZEOF_UUID
 : public uuid
#elif SIZEOF_GUID
 : public GUID
#endif
{
};
//---------------------------------------------------------------------------
inline utf8::String guidToBase32(const guid_t & u)
{
  return base32Encode(&u,sizeof(u));
}
//---------------------------------------------------------------------------
inline guid_t base32ToGUID(const utf8::String & s)
{
  guid_t uuid;
  base32Decode(s,&uuid,sizeof(uuid));
  return uuid;
}
//---------------------------------------------------------------------------
inline utf8::String guidToBase64(const guid_t & u)
{
  return base64Encode(&u,sizeof(u));
}
//---------------------------------------------------------------------------
inline guid_t base64ToGUID(const utf8::String & s)
{
  guid_t uuid;
  base64Decode(s,&uuid,sizeof(uuid));
  return uuid;
}
//---------------------------------------------------------------------------
uintmax_t fibonacci(uintmax_t n);
void createGUID(guid_t & uuid);
utf8::String createGUIDAsBase32String();
inline guid_t createGUID(){ guid_t uuid; createGUID(uuid); return uuid; }
guid_t stringToGUID(const char * s);
inline guid_t stringToGUID(const utf8::String & s){ return stringToGUID(s.c_str()); }
void copyStrToClipboard(const utf8::String & s);
int64_t getProcessStartTime(bool toLocalTime = false);
intptr_t strToMonth(const utf8::String & month);
utf8::String getTimestamp(const utf8::String & date,const utf8::String & time);
pid_t execute(const utf8::String & name,const utf8::String & args,const Array<utf8::String> * env = NULL,bool wait = false);
pid_t execute(const utf8::String & name,const Array<utf8::String> & args,const Array<utf8::String> * env = NULL,bool wait = false);
int32_t waitForProcess(pid_t pid);
//---------------------------------------------------------------------------
inline uintptr_t strlen(const char * s)
{
  return (uintptr_t) ::strlen(s);
}
//---------------------------------------------------------------------------
void reverseByteArray(void * array,uintptr_t size);
void reverseByteArray(void * dst,const void * src,uintptr_t size);
//---------------------------------------------------------------------------
utf8::String  screenChar(const utf8::String::Iterator & ii);
utf8::String  screenString(const utf8::String & s);
utf8::String  unScreenChar(const utf8::String::Iterator & ii, uintptr_t & screenLen);
utf8::String  unScreenString(const utf8::String & s);
uintptr_t     enumStringParts(const utf8::String & s, const char * delim = ",;");
utf8::String  stringPartByNo(const utf8::String & s, uintptr_t n, const char * delim = ",;");
utf8::String splitString(const utf8::String & s,utf8::String & s0,utf8::String & s1,const utf8::String & separator);
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
HMODULE       getModuleHandleByAddr(void * addr = NULL);
utf8::String  getModuleFileNameByHandle(HMODULE h);
#endif
utf8::String  getCurrentDir();
void          changeCurrentDir(const utf8::String & name);
utf8::String  getTempPath();
utf8::String  changeFileExt(const utf8::String & fileName, const utf8::String & extension);
utf8::String  getFileExt(const utf8::String & fileName);
utf8::String  getExecutableName();
utf8::String  getExecutablePath();
utf8::String  excludeTrailingPathDelimiter(const utf8::String & path);
utf8::String  includeTrailingPathDelimiter(const utf8::String & path);
void chModOwn(const utf8::String & pathName, const Mutant & mode, const Mutant & user, const Mutant & group);
utf8::String getRootFromPathName(const utf8::String & pathName);
utf8::String  getPathFromPathName(const utf8::String & pathName);
utf8::String  getNameFromPathName(const utf8::String & pathName);
utf8::String  anyPathName2HostPathName(const utf8::String & pathName);
bool isPathNameRelative(const utf8::String & pathName);
utf8::String  absolutePathNameFromWorkDir(const utf8::String & workDir, const utf8::String & pathName);
bool utime(const utf8::String & pathName,uint64_t atime,uint64_t mtime);
bool createDirectory(const utf8::String & name);
bool removeDirectory(const utf8::String & name,bool recursive = true,bool noThrow = false);
bool rename(const utf8::String & oldPathName,const utf8::String & newPathName,bool createPathIfNotExist = true,bool noThrow = false);
void copy(const utf8::String & dstPathName,const utf8::String & srcPathName,uintptr_t bufferSize = 0);
bool remove(const utf8::String & name,bool noThrow = false);
void getDirList(
  Vector<utf8::String> & list,
  const utf8::String & dirAndMask,
  const utf8::String & exMask = utf8::String(),
  bool recursive = true,
  bool includeDirs = false,
  bool exMaskAsList = false);
//---------------------------------------------------------------------------
void ksleep(uint64_t timeout);
inline void ksleep1()
{
#if HAVE_NANOSLEEP
  struct timespec rqtp;
  rqtp.tv_sec = 0;
  rqtp.tv_nsec = 1;
  nanosleep(&rqtp, NULL);
#elif defined(__WIN32__) || defined(__WIN64__)
  Sleep(1);
#elif HAVE_USLEEP
  usleep(1);
#elif HAVE_SLEEP
  sleep(1);
#endif
}
//---------------------------------------------------------------------------
struct Stat : public
#if HAVE_STAT64
stat64
#elif HAVE__STAT64
_stat64
#else
stat
#endif
{
};
bool stat(const utf8::String & pathName,Stat * st = NULL);
bool stat(const utf8::String & pathName,struct Stat & st);
//---------------------------------------------------------------------------
typedef utf8::String (* StrErrorHandler)(int32_t);
typedef Array< StrErrorHandler> StrErrorHandlers;
extern uint8_t                  strErrorHandlersHolder[];
inline StrErrorHandlers & strErrorHandlers()
{
  return *reinterpret_cast< StrErrorHandlers *>(strErrorHandlersHolder);
}
void          addStrErrorHandler(StrErrorHandler strErrorHandler);
utf8::String  strError(int32_t err);
void          strErrorInitialize();
void          strErrorCleanup();
//---------------------------------------------------------------------------
pid_t getpid();
uid_t getuid();
gid_t getgid();
//---------------------------------------------------------------------------
/*struct PACKED MFA {
  void * func;
  void * obj;
  MFA();

  bool IsNull() const;
};

inline MFA::MFA() : func(NULL), obj(NULL)
{
}  

inline bool MFA::IsNull() const
{
  return func == NULL || obj == NULL;
}  

template <class T,class A>
struct PACKED MFA_ {
  union PACKED {
    T ptr;
    MFA a;
  };
  MFA_(const T & aptr,A * aobj);
};

template <class T,class A> inline
MFA_<T,A>::MFA_(const T & aptr,A * aobj)
{ 
  ptr = aptr;
  a.obj = aobj;
}

template <class T,class A> inline
MFA GetMFA(const T & ptr,A * obj)
{
  return MFA_<T,A>(ptr,obj).a;
} */
//---------------------------------------------------------------------------
int64_t timeFromTimeString(const utf8::String & s,bool local = true);
utf8::String getTimeString(int64_t t);
int64_t timeFromTimeCodeString(const utf8::String & s,bool local = true);
utf8::String getTimeCode(int64_t t);
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _Sysutils_H_ */
//---------------------------------------------------------------------------
