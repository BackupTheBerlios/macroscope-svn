/*-
 * Copyright 2005-2008 Guram Dukashvili
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
#if !defined(IDLE_PRIORITY_CLASS) && HAVE_SETPRIORITY
#define IDLE_PRIORITY_CLASS PRIO_MAX
#define BELOW_NORMAL_PRIORITY_CLASS (PRIO_MAX / 2)
#define NORMAL_PRIORITY_CLASS ((PRIO_MAX + PRIO_MIN) / 2)
#define ABOVE_NORMAL_PRIORITY_CLASS (PRIO_MIN / 3)
#define HIGH_PRIORITY_CLASS ((PRIO_MIN / 3) * 2)
#define REALTIME_PRIORITY_CLASS PRIO_MIN
#endif										
Mutant getProcessPriority();
void setProcessPriority(const Mutant & m,bool noThrow = false);
//---------------------------------------------------------------------------
bool isEnv(const utf8::String & name);
utf8::String getEnv(const utf8::String & name,const utf8::String & defValue = utf8::String());
void setEnv(const utf8::String & name,const utf8::String & value,bool overwrite = true);
void putEnv(const utf8::String & string);
void unSetEnv(const utf8::String & name);
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
#elif HAVE_SIGNAL_H
extern volatile uilock_t signalsCounters[_SIG_MAXSIG];
void waitForSignalsSemaphore();
#endif
//---------------------------------------------------------------------------
bool isDaemonCommandLineOption();
void daemonize();
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DirectoryChangeNotification {
  friend class AsyncIoSlave;
  friend class AsyncMiscSlave;
#if defined(__WIN32__) || defined(__WIN64__)
  friend class AsyncWin9xDirectoryChangeNotificationSlave;
#endif
  public:
    virtual ~DirectoryChangeNotification();
    DirectoryChangeNotification();

    void monitor(const utf8::String & pathName,uint64_t timeout = ~uint64_t(0),bool noThrow = false);
    void stop();
    void cancel();

    bool createPath() const { return createPath_; }
    DirectoryChangeNotification & createPath(bool v){ createPath_ = v; return *this; }
  protected:
  private:
#if defined(__WIN32__) || defined(__WIN64__)
    HANDLE hFFCNotification_; // from FindFirstChangeNotification for FindNextChangeNotification
    HANDLE hDirectory_; // for ReadDirectoryChangesW
    Array<FILE_NOTIFY_INFORMATION> buffer_; // for ReadDirectoryChangesW
    uintptr_t bufferSize_;
#elif HAVE_FAM_H
    utf8::String name_;
    utf8::String fname_;
    FAMConnection famConnection_;
    FAMRequest famRequest_;
    FAMEvent famEvent_;
    bool monitorStarted_;
#endif
    bool createPath_;
};
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
utf8::String getFileNameByHandle(HANDLE hFile);
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
void readProtectedMemory(const void * memory,void * data,uintptr_t count);
void writeProtectedMemory(void * memory,const void * data,uintptr_t count);
void * findProcImportedEntryAddress(const utf8::String & dllName,const utf8::String & importedDllName,const utf8::String & funcName,bool noThrow = false);
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
extern wchar_t pathDelimiterW;
extern wchar_t pathDelimiterStrW[2];
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
extern pthread_t mainThread;
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
utf8::String createGUIDAsBase32String(bool reverse = false);
inline guid_t createGUID(){ guid_t uuid; createGUID(uuid); return uuid; }
guid_t stringToGUID(const char * s);
inline guid_t stringToGUID(const utf8::String & s){ return stringToGUID(s.c_str()); }
void copyStrToClipboard(const utf8::String & s);
int64_t getProcessStartTime(bool toLocalTime = false);
intptr_t strToMonth(const utf8::String & month);
utf8::String printTraffic(intmax_t traf,bool zero = false);
utf8::String printTraffic(uintmax_t traf,bool zero = false);
utf8::String printTraffic(uintmax_t traf,uintmax_t allTraf,bool html = true);
utf8::String printCount(uintmax_t count,uintmax_t allCount,bool html = true);
utf8::String getTimestamp(const utf8::String & date,const utf8::String & time);
//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option push -w-inl
#endif
class ExecuteProcessParameters {
  public:
    ExecuteProcessParameters() :
      stdio_(INVALID_HANDLE_VALUE),
      stdout_(INVALID_HANDLE_VALUE),
      stderr_(INVALID_HANDLE_VALUE),
      wait_(false), usePathEnv_(true), noThrow_(false) {}

    ExecuteProcessParameters(const ExecuteProcessParameters & params) { operator = (params); }

    ExecuteProcessParameters & operator = (const ExecuteProcessParameters & params)
    {
      name_ = params.name_;
      args_ = params.args_;
      env_ = params.env_;
      stdio_ = params.stdio_;
      stdout_ = params.stdout_;
      stderr_ = params.stderr_;
      wait_ = params.wait_;
      usePathEnv_ = params.usePathEnv_;
      noThrow_ = params.noThrow_;
      return *this;
    }

    utf8::String name_;
    utf8::String args_;
    Array<utf8::String> env_;
    file_t stdio_;
    file_t stdout_;
    file_t stderr_;
    bool wait_;
    bool usePathEnv_;
    bool noThrow_;

  private:
};
#ifdef __BORLANDC__
#pragma option pop
#endif

pid_t execute(const utf8::String & name,const utf8::String & args,const Array<utf8::String> * env = NULL,bool wait = false,bool usePathEnv = true,bool noThrow = false);
pid_t execute(const utf8::String & name,const Array<utf8::String> & args,const Array<utf8::String> * env = NULL,bool wait = false,bool usePathEnv = true,bool noThrow = false);
pid_t execute(const ExecuteProcessParameters & params);
int32_t waitForProcess(pid_t pid);
//---------------------------------------------------------------------------
utf8::String  screenChar(const utf8::String::Iterator & ii);
utf8::String  screenString(const utf8::String & s);
utf8::String  unScreenChar(const utf8::String::Iterator & ii, uintptr_t & screenLen);
utf8::String  unScreenString(const utf8::String & s);
uintptr_t     enumStringParts(const utf8::String & s, const char * delim = ",;",bool unscreen = true);
utf8::String  stringPartByNo(const utf8::String & s, uintptr_t n, const char * delim = ",;",bool unscreen = true);
intptr_t findStringPart(const utf8::String & s,const utf8::String & part,bool caseSensitive = true,const char * delim = ",;",bool unscreen = true);
utf8::String splitString(const utf8::String & s,utf8::String & s0,utf8::String & s1,const utf8::String & separator);
utf8::String formatByteLength(uintmax_t len,uintmax_t all,const char * fmt = "SP");
//---------------------------------------------------------------------------
utf8::String getHostName(bool noThrow = false,const utf8::String & def = utf8::String());
#if defined(__WIN32__) || defined(__WIN64__)
HMODULE       getModuleHandleByAddr(void * addr = NULL);
utf8::String  getModuleFileNameByHandle(HMODULE h);
#endif
utf8::String  getCurrentDir();
void          changeCurrentDir(const utf8::String & name);
utf8::String  getTempFileName(const utf8::String & ext = "tmp");
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
bool nameFitMask(const utf8::String & name,const utf8::String & mask);
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
#if SIZEOF_STAT64 > 0
stat64
#elif SIZEOF_STATI64 > 0
stati64
#elif SIZEOF__STAT64 > 0
_stat64
#else
stat
#endif
{
};
bool stat(const utf8::String & pathName,Stat * st = NULL);
bool stat(const utf8::String & pathName,Stat & st);
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
intptr_t memncmp(const void * m1,const void * m2,uintptr_t n);
intptr_t memncmp(const void * m1,uintptr_t n1,const void * m2,uintptr_t n2);
void memxchg(void * m1,void * m2,uintptr_t n);
//---------------------------------------------------------------------------
#endif /* _Sysutils_H_ */
//---------------------------------------------------------------------------
