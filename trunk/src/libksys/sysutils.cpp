/*-
 * Copyright 2006-2008 Guram Dukashvili
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
bool stackBackTrace = true;
//---------------------------------------------------------------------------
utf8::String getBackTrace(/*intptr_t flags,*/intptr_t skipCount,Thread * thread)
{
#if __GNUG__
  thread = thread;
  utf8::String s;
  AutoPtr<char *,AutoPtrMemoryDestructor> strings(backtrace());
  strings.ptr(backtrace_symbols(strings));
  for( char ** p = strings; p != NULL && *p != NULL; p++ ){
    if( --skipCount < 0 ){
      s += *p;
      s += "\n";
    }
  }
  return s;
#elif !defined(NDEBUG) && (defined(__WIN32__) || defined(__WIN64__))
  WriteLock mutex;
  if( currentFiber() != NULL ){
    if( thread == NULL ){
      thread = currentThread();
      currentFiber()->event_.mutex0_ = &mutex;
      mutex.acquire();
    }
    else {
      currentFiber()->event_.mutex0_ = NULL;
    }
    currentFiber()->event_.timeout_ = ~uint64_t(0);
    currentFiber()->event_.data1_ = skipCount;
    currentFiber()->event_.thread_ = thread;
    currentFiber()->event_.type_ = etStackBackTrace;
    currentFiber()->thread()->postRequest();
    if( thread != currentThread() ){
      currentFiber()->switchFiber(currentFiber()->mainFiber());
    }
    else {
      mutex.acquire();
      mutex.release();
    }
    assert( currentFiber()->event_.type_ == etStackBackTrace );
    assert( currentFiber()->event_.errno_ == 0 );
    return currentFiber()->event_.string0_;
  }
  AsyncEvent event;
  event.mutex0_ = &mutex;
  event.data1_ = skipCount;
  event.tid_ = (uintptr_t) GetCurrentThreadId();
  event.type_ = etStackBackTraceZero;
  event.timeout_ = ~uint64_t(0);
  mutex.acquire();
  Requester::requester().postRequest(&event);
  mutex.acquire();
  mutex.release();
  assert( event.type_ == etStackBackTraceZero );
  assert( event.errno_ == 0 );
  return event.string0_;
#else
  skipCount = skipCount;
  thread = thread;
  return utf8::String();
#endif
}
//---------------------------------------------------------------------------
Mutant getProcessPriority()
{
  Mutant m;
#if defined(__WIN32__) || defined(__WIN64__)
  switch( GetPriorityClass(GetCurrentProcess()) ){
    case IDLE_PRIORITY_CLASS :
      m = "IDLE_PRIORITY_CLASS";
      break;
    case BELOW_NORMAL_PRIORITY_CLASS :
      m = "BELOW_NORMAL_PRIORITY_CLASS";
      break;
    case NORMAL_PRIORITY_CLASS :
      m = "NORMAL_PRIORITY_CLASS";
      break;
    case ABOVE_NORMAL_PRIORITY_CLASS :
      m = "ABOVE_NORMAL_PRIORITY_CLASS";
      break;
    case HIGH_PRIORITY_CLASS :
      m = "HIGH_PRIORITY_CLASS";
      break;
    case REALTIME_PRIORITY_CLASS :
      m = "REALTIME_PRIORITY_CLASS";
      break;
    default :
      m = GetPriorityClass(GetCurrentProcess());
  }
#elif HAVE_GETPRIORITY || HAVE_RTPRIO
  int32_t err;
  switch( getpriority(PRIO_PROCESS,0) ){
    case -1 :
      err = errno;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      break;
    case IDLE_PRIORITY_CLASS :
      m = "IDLE_PRIORITY_CLASS";
      break;
    case BELOW_NORMAL_PRIORITY_CLASS :
      m = "BELOW_NORMAL_PRIORITY_CLASS";
      break;
    case NORMAL_PRIORITY_CLASS :
      m = "NORMAL_PRIORITY_CLASS";
      break;
    case ABOVE_NORMAL_PRIORITY_CLASS :
      m = "ABOVE_NORMAL_PRIORITY_CLASS";
      break;
    case HIGH_PRIORITY_CLASS :
      m = "HIGH_PRIORITY_CLASS";
      break;
    case REALTIME_PRIORITY_CLASS :
      m = "REALTIME_PRIORITY_CLASS";
      break;
    default :
      m = getpriority(PRIO_PROCESS,0);
  }
#endif
  return m;
}
//---------------------------------------------------------------------------
void setProcessPriority(const Mutant & m,bool noThrow)
{
#if defined(__WIN32__) || defined(__WIN64__)
  int32_t err = 0;
  BOOL r;
  try {
    r = SetPriorityClass(GetCurrentProcess(),m);
    if( r == 0 ) err = GetLastError();
  }
  catch( ExceptionSP & ){
    r = 0;
  }
  if( r == 0 && err == 0 ){
    utf8::String s(m);
    if( s.casecompare("IDLE_PRIORITY_CLASS") == 0 )
      r = SetPriorityClass(GetCurrentProcess(),IDLE_PRIORITY_CLASS);
    else
    if( s.casecompare("BELOW_NORMAL_PRIORITY_CLASS") == 0 )
      r = SetPriorityClass(GetCurrentProcess(),BELOW_NORMAL_PRIORITY_CLASS);
    else
    if( s.casecompare("NORMAL_PRIORITY_CLASS") == 0 )
      r = SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
    else
    if( s.casecompare("ABOVE_NORMAL_PRIORITY_CLASS") == 0 )
      r = SetPriorityClass(GetCurrentProcess(),ABOVE_NORMAL_PRIORITY_CLASS);
    else
    if( s.casecompare("HIGH_PRIORITY_CLASS") == 0 )
      r = SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
    else
    if( s.casecompare("REALTIME_PRIORITY_CLASS") == 0 )
      r = SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);
    err = GetLastError();
  }
  if( r == 0 && err != 0 ){
    AutoPtr<Exception> e(newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__));
    if( noThrow ) e->writeStdError(); else e.ptr(NULL)->throwSP();
  }
#elif HAVE_SETPRIORITY || HAVE_RTPRIO
  int32_t err = 0;
  int r;
  try {
    r = setpriority(PRIO_PROCESS,0,m);
    if( r != 0 ) err = errno;
  }
  catch( ExceptionSP & ){
    r = -1;
  }
  if( r != 0 && err == 0 ){
    utf8::String s(m);
    if( s.casecompare("IDLE_PRIORITY_CLASS") == 0 ){
#if HAVE_RTPRIO
      struct rtprio rtp;
      rtp.type = RTP_PRIO_IDLE;
      rtp.prio = 0;
      r = rtprio(RTP_SET,0,&rtp);
#elif HAVE_SETPRIORITY
      r = setpriority(PRIO_PROCESS,0,IDLE_PRIORITY_CLASS);
#endif
    }
    else
    if( s.casecompare("BELOW_NORMAL_PRIORITY_CLASS") == 0 )
      r = setpriority(PRIO_PROCESS,0,BELOW_NORMAL_PRIORITY_CLASS);
    else
    if( s.casecompare("NORMAL_PRIORITY_CLASS") == 0 )
      r = setpriority(PRIO_PROCESS,0,NORMAL_PRIORITY_CLASS);
    else
    if( s.casecompare("ABOVE_NORMAL_PRIORITY_CLASS") == 0 )
      r = setpriority(PRIO_PROCESS,0,ABOVE_NORMAL_PRIORITY_CLASS);
    else
    if( s.casecompare("HIGH_PRIORITY_CLASS") == 0 )
      r = setpriority(PRIO_PROCESS,0,HIGH_PRIORITY_CLASS);
    else
    if( s.casecompare("REALTIME_PRIORITY_CLASS") == 0 ){
#if HAVE_RTPRIO
      struct rtprio rtp;
      rtp.type = RTP_PRIO_REALTIME;
      rtp.prio = RTP_PRIO_MAX;
      r = rtprio(RTP_SET,0,&rtp);
#elif HAVE_SETPRIORITY
      r = setpriority(PRIO_PROCESS,0,REALTIME_PRIORITY_CLASS);
#endif
    }
    err = errno;
  }
  if( r != 0 && err != 0 ){
    AutoPtr<Exception> e(newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__));
    if( noThrow ) e->writeStdError(); else e.ptr(NULL)->throwSP();
  }
#else
  if( !noThrow )
    newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
#endif
}
//---------------------------------------------------------------------------
bool isEnv(const utf8::String & name)
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( isWin9x() ){
    utf8::AnsiString s(name.getANSIString());
    GetEnvironmentVariableA(s,NULL,0);
    int32_t err = GetLastError();
    if( err == ERROR_ENVVAR_NOT_FOUND ) return false;
    if( err != ERROR_SUCCESS )
      newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__)->throwSP();
    return true;
  }
  AutoPtr<wchar_t,AutoPtrMemoryDestructor> b;
  utf8::WideString s(name.getUNICODEString());
  GetEnvironmentVariableW(s,NULL,0);
  int32_t err = GetLastError();
  if( err == ERROR_ENVVAR_NOT_FOUND ) return false;
  if( err != ERROR_SUCCESS )
    newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  return true;
#else
  return getenv(name.getANSIString()) != NULL;
#endif
}
//---------------------------------------------------------------------------
utf8::String getEnv(const utf8::String & name,const utf8::String & defValue)
{
#if defined(__WIN32__) || defined(__WIN64__)
  DWORD sz;
  if( isWin9x() ){
    AutoPtr<char,AutoPtrMemoryDestructor> b;
    utf8::AnsiString s(name.getANSIString());
    SetLastError(ERROR_SUCCESS);
    sz = GetEnvironmentVariableA(s,b,0);
    if( sz != 0 ){
      b.alloc(sz);
      SetLastError(ERROR_SUCCESS);
      sz = GetEnvironmentVariableA(s,b,sz) + 1;
    }
    int32_t err = GetLastError();
    if( sz == 0 || err == ERROR_ENVVAR_NOT_FOUND ) return defValue;
    if( err != ERROR_SUCCESS )
      newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__)->throwSP();
    if( sz <= 1 ) return utf8::String();
    return b.ptr();
  }
  AutoPtr<wchar_t,AutoPtrMemoryDestructor> b;
  utf8::WideString s(name.getUNICODEString());
  SetLastError(ERROR_SUCCESS);
  sz = GetEnvironmentVariableW(s,b,0);
  if( sz != 0 ){
    b.alloc(sz * sizeof(wchar_t));
    SetLastError(ERROR_SUCCESS);
    sz = GetEnvironmentVariableW(s,b,sz) + 1;
  }
  int32_t err = GetLastError();
  if( sz == 0 || err == ERROR_ENVVAR_NOT_FOUND ) return defValue;
  if( err != ERROR_SUCCESS )
    newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  if( sz <= 1 ) return utf8::String();
  return b.ptr();
#else
  char * env = getenv(name.getANSIString());
  if( env == NULL && errno != 0 && errno != ENOENT ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return env == NULL ? defValue : env;
#endif
}
//---------------------------------------------------------------------------
void setEnv(const utf8::String & name,const utf8::String & value,bool overwrite)
{
#if defined(__WIN32__) || defined(__WIN64__)
  BOOL r;
  if( isWin9x() ){
    r = SetEnvironmentVariableA(name.getANSIString(),value.getANSIString());
  }
  else {
    r = SetEnvironmentVariableW(name.getUNICODEString(),value.getUNICODEString());
  }
  if( r == 0 && GetLastError() != ERROR_ENVVAR_NOT_FOUND ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  overwrite = overwrite;
#else
  if( setenv(name.getANSIString(),value.getANSIString(),overwrite) != 0 ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#endif
}
//---------------------------------------------------------------------------
void putEnv(const utf8::String & string)
{
#if defined(__WIN32__) || defined(__WIN64__)
  utf8::String::Iterator i(string), j(string.strstr("="));
  setEnv(utf8::String(i,j),j + 1);
#else
  if( putenv(string.getANSIString()) != 0 ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#endif
}
//---------------------------------------------------------------------------
void unSetEnv(const utf8::String & name)
{
#if defined(__WIN32__) || defined(__WIN64__)
  BOOL r;
  if( isWin9x() ){
    r = SetEnvironmentVariableA(name.getANSIString(),NULL);
  }
  else {
    r = SetEnvironmentVariableW(name.getUNICODEString(),NULL);
  }
  if( r == 0 && GetLastError() != ERROR_ENVVAR_NOT_FOUND ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#else
  unsetenv(name.getANSIString());
#endif
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DirectoryChangeNotification::~DirectoryChangeNotification()
{
  cancel();
  stop();
}
//---------------------------------------------------------------------------
DirectoryChangeNotification::DirectoryChangeNotification() :
#if defined(__WIN32__) || defined(__WIN64__)
  hFFCNotification_(INVALID_HANDLE_VALUE),
  hDirectory_(INVALID_HANDLE_VALUE),
  bufferSize_(0),
#elif HAVE_FAM_H
  monitorStarted_(false),
#endif
  createPath_(true)
{
#if HAVE_FAM_H
  FAMCONNECTION_GETFD(&famConnection_) = -1;
  if( FAMOpen(&famConnection_) != 0 )
    newObjectV1C2<Exception>(EINVAL,FamErrlist[FAMErrno] + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
#elif !defined(__WIN32__) && !defined(__WIN64__)
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
#endif
}
//---------------------------------------------------------------------------
void DirectoryChangeNotification::monitor(const utf8::String & pathName,uint64_t timeout,bool noThrow)
{
#if defined(__WIN32__) || defined(__WIN64__)
  assert( 
    (hFFCNotification_ == INVALID_HANDLE_VALUE && hDirectory_ == INVALID_HANDLE_VALUE) || 
    ((hFFCNotification_ != INVALID_HANDLE_VALUE) ^ (hDirectory_ != INVALID_HANDLE_VALUE)) 
  );
  if( isWin9x() ){
    if( hFFCNotification_ == INVALID_HANDLE_VALUE ){
      hFFCNotification_ = FindFirstChangeNotificationA(
        anyPathName2HostPathName(pathName).getOEMString(),
        FALSE,
        FILE_NOTIFY_CHANGE_FILE_NAME
      );
      if( hFFCNotification_ == INVALID_HANDLE_VALUE ){
        int32_t err = GetLastError();
        if( (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND) && createPath_ ){
          createDirectory(pathName);
          return monitor(pathName,timeout);
        }
        if( noThrow ) return;
        newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__)->throwSP();
      }
    }
    else if( FindNextChangeNotification(hFFCNotification_) == 0 ){
      int32_t err = GetLastError() + errorOffset;
      stop();
      SetLastError(err);
      if( noThrow ) return;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
  }
  else {
    if( buffer_.count() == 0 ){
      bufferSize_ = getpagesize() * 16u / sizeof(FILE_NOTIFY_INFORMATION);
      buffer_.resize(bufferSize_);
      bufferSize_ *= sizeof(FILE_NOTIFY_INFORMATION);
    }
    if( hDirectory_ == INVALID_HANDLE_VALUE ){
      hDirectory_ = CreateFileW(
          anyPathName2HostPathName(pathName).getUNICODEString(),
          GENERIC_READ,
          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
          NULL,
          OPEN_EXISTING,
          FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
          NULL
      );
      if( hDirectory_ == INVALID_HANDLE_VALUE ){
        int32_t err = GetLastError();
        if( (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND) && createPath_ ){
          createDirectory(pathName);
          return monitor(pathName,timeout);
        }
        if( noThrow ) return;
        newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__)->throwSP();
      }
    }
  }
  Fiber * fiber = currentFiber();
  assert( fiber != NULL );
  fiber->event_.abort_ = false;
  fiber->event_.position_ = 0;
  fiber->event_.directoryChangeNotification_ = this;
  fiber->event_.timeout_ = timeout;
  fiber->event_.type_ = etDirectoryChangeNotification;
  fiber->thread()->postRequest();
  fiber->switchFiber(fiber->mainFiber());
  assert( fiber->event_.type_ == etDirectoryChangeNotification );
  if( fiber->event_.errno_ == ERROR_REQUEST_ABORTED ) stop();
  if( fiber->event_.errno_ != 0 && fiber->event_.errno_ != ERROR_NOTIFY_ENUM_DIR ){
    if( noThrow ) return;
    newObjectV1C2<Exception>(fiber->event_.errno_ + errorOffset,pathName + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
  }
  SetLastError(fiber->event_.errno_);
#elif HAVE_FAM_H
  if( !monitorStarted_ ){
    utf8::String name(anyPathName2HostPathName(pathName));
    fname_ = includeTrailingPathDelimiter(name) + getTempFileName("flag");
    Stat st;
    if( !stat(name,st) ){
      errno = ENOENT;
      if( noThrow ) return;
      newObjectV1C2<Exception>(ENOENT,pathName + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
    }
    if( (st.st_mode & S_IFDIR) == 0 ){
      if( noThrow ) return;
      newObjectV1C2<Exception>(ENOTDIR,pathName + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
    }
    utf8::AnsiString aname(name.getANSIString());
    if( access(aname,R_OK | X_OK) != 0 ){
      int32_t err = errno;
      if( noThrow ) return;
      newObjectV1C2<Exception>(err,pathName + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
    }    
    if( FAMMonitorDirectory(&famConnection_,aname,&famRequest_,NULL) != 0 ){
      if( noThrow ) return;
      newObjectV1C2<Exception>(EINVAL,pathName + utf8::String(" ") + FamErrlist[FAMErrno] + " " + __PRETTY_FUNCTION__)->throwSP();
    }
    monitorStarted_ = true;
    name_ = name;
  }
  if( monitorStarted_ ){
    for(;;){
      Fiber * fiber = currentFiber();
      assert( fiber != NULL );
      fiber->event_.abort_ = false;
      fiber->event_.position_ = 0;
      fiber->event_.directoryChangeNotification_ = this;
      fiber->event_.timeout_ = timeout;
      fiber->event_.type_ = etDirectoryChangeNotification;
      fiber->thread()->postRequest();
      fiber->switchFiber(fiber->mainFiber());
      assert( fiber->event_.type_ == etDirectoryChangeNotification );
      errno = fiber->event_.errno_;
      FAMErrno = fiber->event_.errno_;
      if( fiber->event_.errno_ != 0 ){
        if( noThrow ) return;
        newObjectV1C2<Exception>(EINVAL,pathName + utf8::String(" ") + FamErrlist[fiber->event_.errno_] + " " + __PRETTY_FUNCTION__)->throwSP();
      }
      else if( famEvent_.code == FAMDeleted || famEvent_.code == FAMCreated ){
        if( stat(fname_) ){
          //for( uintptr_t i = 0; i < 100; i++ ){
          //  if( remove(fname_,true) ) break;
	  //  ksleep(10000);
	  //}
          newObjectV1C2<Exception>(EINTR,pathName + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
        }
        break;
      }
    }
  }
#else
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
#endif
}
//---------------------------------------------------------------------------
void DirectoryChangeNotification::stop()
{
  cancel();
#if defined(__WIN32__) || defined(__WIN64__)
  if( hFFCNotification_ != INVALID_HANDLE_VALUE ){
    FindCloseChangeNotification(hFFCNotification_);
    hFFCNotification_ = INVALID_HANDLE_VALUE;
  }
  if( hDirectory_ != INVALID_HANDLE_VALUE ){
    CloseHandle(hDirectory_);
    hDirectory_ = INVALID_HANDLE_VALUE;
  }
  buffer_.clear();
  bufferSize_ = 0;
#elif HAVE_FAM_H
  if( FAMCONNECTION_GETFD(&famConnection_) != -1 ){
    if( FAMCancelMonitor(&famConnection_,&famRequest_) != 0 )
      newObjectV1C2<Exception>(EINVAL,FamErrlist[FAMErrno] + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
    if( FAMClose(&famConnection_) != 0 )
      newObjectV1C2<Exception>(EINVAL,FamErrlist[FAMErrno] + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
    monitorStarted_ = false;
    FAMCONNECTION_GETFD(&famConnection_) = -1;
    remove(fname_,true);
  }
#endif
}
//---------------------------------------------------------------------------
void DirectoryChangeNotification::cancel()
{
  Requester::requester().abortNotification(this);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
uint8_t argvPlaceHolder[sizeof(Array<utf8::String>)];
#if defined(__WIN32__) || defined(__WIN64__)
char pathDelimiter = '\\';
char pathDelimiterStr[2] = "\\";
wchar_t pathDelimiterW = L'\\';
wchar_t pathDelimiterStrW[2] = L"\\";
#else
char pathDelimiter = '/';
char pathDelimiterStr[2] = "/";
wchar_t pathDelimiterW = L'/';
wchar_t pathDelimiterStrW[2] = L"/";
#endif
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
DWORD mainThreadId;
UINT threadFinishMessage;
UINT fiberFinishMessage;
//---------------------------------------------------------------------------
#else
pthread_t mainThread;
#endif
//---------------------------------------------------------------------------
uintmax_t fibonacci(uintmax_t n)
{
  if( n == 0 ) return 0;
  uintmax_t prev = 0, curr = 1, next;
  while( --n > 0 ){
    next = curr + prev;
    prev = curr;
    curr = next;
  }
  return curr;
}
//---------------------------------------------------------------------------
void createGUID(guid_t & uuid)
{
#if defined(__WIN32__) || defined(__WIN64__)
  typedef RPC_STATUS (RPC_ENTRY * FpUuidCreate)(UUID __RPC_FAR * Uuid);
  static FpUuidCreate pUuidCreate = NULL;
  if( pUuidCreate == NULL ){
    HMODULE handle = LoadLibraryA("Rpcrt4.dll");
    if( handle == NULL ) goto er;
    pUuidCreate = (FpUuidCreate) GetProcAddress(handle,"UuidCreate");
    if( pUuidCreate == NULL ) goto er;
  }
  RPC_STATUS status = pUuidCreate((UUID *) &uuid);
  if( status != RPC_S_OK ){
//  if( FAILED(CoCreateGuid(&uuid)) ){
er: int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#elif HAVE_UUIDGEN
  if( uuidgen(&uuid,1) != 0 ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#elif HAVE_UUID_CREATE
  uuid_t * u;
  uint32_t status;
  uuid_create(&u,&status);
  if( status != uuid_s_ok ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  uuid = *u;
  free(u);
#else
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
#endif
}
//---------------------------------------------------------------------------
guid_t stringToGUID(const char * s)
{
  guid_t uuid;
  memset(&uuid,0,sizeof(uuid));
#if defined(__WIN32__) || defined(__WIN64__)
  typedef RPC_STATUS (RPC_ENTRY * FpUuidFromString)(
    unsigned char __RPC_FAR* StringUuid,
    UUID __RPC_FAR* Uuid
  );
  RPC_STATUS status = 0;
  static FpUuidFromString pUuidFromString = NULL;
  if( pUuidFromString == NULL ){
    HMODULE handle = LoadLibraryA("Rpcrt4.dll");
    if( handle == NULL ) goto er;
    pUuidFromString = (FpUuidFromString) GetProcAddress(handle,"UuidFromString");
    if( pUuidFromString == NULL ) goto er;
  }
  status = pUuidFromString((unsigned char *) s,&uuid);
  if( status != RPC_S_OK ){
er: int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#elif HAVE_UUID_FROM_STRING
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
#else
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
#endif
  return uuid;
}
//---------------------------------------------------------------------------
utf8::String createGUIDAsBase32String(bool reverse)
{
  guid_t uuid;
  createGUID(uuid);
  if( reverse ){
    uint8_t * p1 = (uint8_t *) &uuid, * p2 = p1 + sizeof(uuid) - 1;
    while( p1 < p2 ){
      xchg(*p1,*p2);
      p1++;
      p2--;
    }
  }
  return base32Encode(&uuid,sizeof(uuid));
}
//---------------------------------------------------------------------------
utf8::String screenChar(const utf8::String::Iterator & ii)
{
  char b[12];
  const char * p;
  switch( ii.getChar() ){
    case '\a' :
      p = "\\a";
      break;
    case '\b' :
      p = "\\b";
      break;
    case '\t' :
      p = "\\t";
      break;
    case '\v' :
      p = "\\v";
      break;
    case '\f' :
      p = "\\f";
      break;
    case '\r' :
      p = "\\r";
      break;
    case '\n' :
      p = "\\n";
      break;
    case '\\' :
      p = "\\\\";
      break;
    case ',' :
      p = "\\,";
      break;
    case ';' :
      p = "\\;";
      break;
    case '\"' :
      p = "\\\"";
      break;
    case '=' :
      p = "\\=";
      break;
    default  :
      if( ii.isCntrl() || ii.isSpace() )
        return "\\0x" +  utf8::int2HexStr(ii.getChar(),SIZEOF_WCHAR_T * 2);
      memset(b,0,sizeof(b));
      memcpy(b,ii.c_str(),ii.seqLen());
      p = b;
  }
  return utf8::plane(p);
}
//---------------------------------------------------------------------------
/*utf8::String screenString(const utf8::String & s)
{
  utf8::String a;
  utf8::String::Iterator i(s);
  while( !i.eof() ){
    a += screenChar(i);
    i.next();
  }
  return a;
}*/
//---------------------------------------------------------------------------
static inline void screenChar(const char * seq,uintptr_t & seql,char * b,uintptr_t & l)
{
  uintptr_t c;
  switch( c = utf8::utf82ucs(seq,seql) ){
    case '\a' :
      if( b != NULL ) memcpy(b,"\\a",2);
      l = 2;
      break;
    case '\b' :
      if( b != NULL ) memcpy(b,"\\b",2);
      l = 2;
      break;
    case '\t' :
      if( b != NULL ) memcpy(b,"\\t",2);
      l = 2;
      break;
    case '\v' :
      if( b != NULL ) memcpy(b,"\\v",2);
      l = 2;
      break;
    case '\f' :
      if( b != NULL ) memcpy(b,"\\f",2);
      l = 2;
      break;
    case '\r' :
      if( b != NULL ) memcpy(b,"\\r",2);
      l = 2;
      break;
    case '\n' :
      if( b != NULL ) memcpy(b,"\\n",2);
      l = 2;
      break;
    case '\\' :
      if( b != NULL ) memcpy(b,"\\\\",2);
      l = 2;
      break;
    case ',' :
      if( b != NULL ) memcpy(b,"\\,",2);
      l = 2;
      break;
    case ';' :
      if( b != NULL ) memcpy(b,"\\;",2);
      l = 2;
      break;
    case '\"' :
      if( b != NULL ) memcpy(b,"\\\"",2);
      l = 2;
      break;
    case '=' :
      if( b != NULL ) memcpy(b,"\\=",2);
      l = 2;
      break;
    default  :
      if( (utf8::getC1Type(c) & (C1_CNTRL | C1_SPACE)) != 0 ){
        if( b != NULL ) memcpy(b,"\\0x",3);
        utf8::String s(utf8::int2HexStr(c,SIZEOF_WCHAR_T * 2));
        l = s.size();
        if( b != NULL ) memcpy(b + 3,s.c_str(),l);
        l += 3;
      }
      else {
        l = seql;
        if( b != NULL ) memcpy(b,seq,l);
      }
  }
}
//---------------------------------------------------------------------------
utf8::String screenString(const utf8::String & s)
{
  const char * p;
  uintptr_t seql, l, len = 0;
  AutoPtr<char,AutoPtrMemoryDestructor> a;
  for( p = s.c_str(); *p != '\0'; p += seql, len += l ) screenChar(p,seql,NULL,l);
  if( len == 0 ) return utf8::String();
  a.alloc(len + 1);
  char * q = a;
  for( p = s.c_str(); *p != '\0'; p += seql, q += l ) screenChar(p,seql,q,l);
  a[len] = '\0';
  utf8::String::Container * container = newObjectV1V2<utf8::String::Container,int,char *,AutoPtrNonVirtualClassDestructor>(0,a.ptr());
  a.ptr(NULL);
  return container;
}
//---------------------------------------------------------------------------
utf8::String unScreenChar(const utf8::String::Iterator & ii,uintptr_t & screenLen)
{
  utf8::String s;
  screenLen = 1;
  switch( ii.getChar() ){
    case '0' :
      if( ii[screenLen].getUpperChar() == 'X' ){
        screenLen++;
        while( screenLen < SIZEOF_WCHAR_T * 2 + 2 && ii[screenLen].isXdigit() ) screenLen++;
        if( screenLen == SIZEOF_WCHAR_T * 2 + 2 ){
          intmax_t a;
          if( utf8::tryStr2Int(utf8::String(ii + 2,ii + SIZEOF_WCHAR_T * 2 + 2),a,16) ){
            char b[5];
            b[utf8::ucs2utf8seq(b,(uintptr_t) a)] = '\0';
            s = utf8::plane(b);
            break;
          }
        }
      }
      else {
        while( screenLen < SIZEOF_WCHAR_T * 2 + 3 && ii[screenLen].isDigit() ) screenLen++;
        if( screenLen == SIZEOF_WCHAR_T * 2 + 3 ){
          intmax_t a;
          if( utf8::tryStr2Int(utf8::String(ii + 1,ii + SIZEOF_WCHAR_T * 2 + 3),a,10) ){
            char b[5];
            b[utf8::ucs2utf8seq(b,(uintptr_t) a)] = '\0';
            s = utf8::plane(b);
            break;
          }
        }
      }
      screenLen = 1;
      goto l1;
    case 'a' :
      s = "\a";
      break;
    case 'b' :
      s = "\b";
      break;
    case 't' :
      s = "\t";
      break;
    case 'v' :
      s = "\v";
      break;
    case 'f' :
      s = "\f";
      break;
    case 'r' :
      s = "\r";
      break;
    case 'n' :
      s = "\n";
      break;
    case '\\' :
      s = "\\";
      break;
    default :
l1:   s = utf8::String(ii,ii + 1);
  }
  return s;
}
//---------------------------------------------------------------------------
static inline void unScreenChar(uintptr_t c,const char * seq,uintptr_t & seql,char * b,uintptr_t & l)
{
  uintptr_t i;
  switch( c ){
    case '0' :
      if( utf8::utf8c2UpperUCS(seq + seql) == 'X' ){
        for( i = 0; i < SIZEOF_WCHAR_T * 2; i++ ){
          if( (utf8::getC1Type(utf8::utf82ucs(seq + seql + i + 1)) & (C1_XDIGIT | C1_DIGIT)) == 0 ) break;
        }
        if( i == SIZEOF_WCHAR_T * 2 ){
          intmax_t a;
          if( utf8::tryStr2Int(utf8::String(seq + seql + 1,SIZEOF_WCHAR_T * 2),a,16) ){
            char buf[6];
            buf[l = utf8::ucs2utf8seq(buf,(uintptr_t) a)] = '\0';
            if( b != NULL ) memcpy(b,buf,l);
            seql = SIZEOF_WCHAR_T * 2 + 2;
            break;
          }
        }
      }
      goto l1;
    case 'a' :
      if( b != NULL ) *b = '\a';
      l = 1;
      break;
    case 'b' :
      if( b != NULL ) *b = '\b';
      l = 1;
      break;
    case 't' :
      if( b != NULL ) *b = '\t';
      l = 1;
      break;
    case 'v' :
      if( b != NULL ) *b = '\v';
      l = 1;
      break;
    case 'f' :
      if( b != NULL ) *b = '\f';
      l = 1;
      break;
    case 'r' :
      if( b != NULL ) *b = '\r';
      l = 1;
      break;
    case 'n' :
      if( b != NULL ) *b = '\n';
      l = 1;
      break;
    case '\\' :
      if( b != NULL ) *b = '\\';
      l = 1;
      break;
    default :
l1:   if( b != NULL ) memcpy(b,seq,seql);
      l = seql;
  }
}
//---------------------------------------------------------------------------
utf8::String unScreenString(const utf8::String & s)
{
  const char * p;
  uintptr_t seql, l, len = 0, c;
  AutoPtr<char,AutoPtrMemoryDestructor> a;
  for( p = s.c_str(); *p != '\0'; p += seql, len += l ){
    c = utf8::utf82ucs(p,seql);
    l = seql;
    if( c == '\\' && p[1] != '\0' ){
      p += seql;
      c = utf8::utf82ucs(p,seql);
      unScreenChar(c,p,seql,NULL,l);
    }
  }
  if( len == 0 ) return utf8::String();
  a.alloc(len + 1);
  char * q = a;
  for( p = s.c_str(); *p != '\0'; p += seql, q += l ){
    c = utf8::utf82ucs(p,seql);
    l = seql;
    if( c == '\\' && p[1] != '\0' ){
      p += seql;
      c = utf8::utf82ucs(p,seql);
      unScreenChar(c,p,seql,q,l);
    }
    else {
      memcpy(q,p,seql);
    }
  }
  a[len] = '\0';
  utf8::String::Container * container = newObjectV1V2<utf8::String::Container,int,char *,AutoPtrNonVirtualClassDestructor>(0,a.ptr());
  a.ptr(NULL);
  return container;
}
//---------------------------------------------------------------------------
static intptr_t stringPartByNoHelper(const utf8::String & s,uintptr_t n,const char * delim,utf8::String * pRetValue,bool unscreen)
{
  bool inQuotationMarks = false;
  uintptr_t l, k = 0, c, prev = 0;
  utf8::String v;
  utf8::String::Iterator i(s), q(s);
  if( !i.eos() ) for(;;){
    c = i.getChar();
    if( inQuotationMarks && c == '\"' && prev != '\\' ){
      if( pRetValue != NULL ) v += unscreen ? unScreenString(utf8::String(q,i)) : utf8::String(q,i);
      q = i + 1;
      inQuotationMarks = false;
    }
    else if( !inQuotationMarks && c == '\"' ){
      q = i + 1;
      inQuotationMarks = true;
    }
    else if( !inQuotationMarks ){
      bool eos;
      const char * d = delim;
      for(;;){
        eos = utf8::utf82ucs(d,l) == c;
        if( eos ){
          if( pRetValue != NULL ) v += utf8::String(q,i).trim();
          q = i + 1;
          break;
        }
        if( *d == '\0' ) break;
        d += l;
      }
      if( eos ){
        if( k == n ) break;
        k++;
        v = utf8::String();
      }
    }
    prev = c;
    if( i.eos() ) break;
    i.next();
  }
  if( pRetValue != NULL ) *pRetValue = v;
  return k;
}
//---------------------------------------------------------------------------
uintptr_t enumStringParts(const utf8::String & s,const char * delim,bool unscreen)
{
  return stringPartByNoHelper(s,~uintptr_t(0),delim,NULL,unscreen);
}
//---------------------------------------------------------------------------
utf8::String stringPartByNo(const utf8::String & s,uintptr_t n,const char * delim,bool unscreen)
{
  utf8::String v;
  stringPartByNoHelper(s,n,delim,&v,unscreen);
  return v;
}
//---------------------------------------------------------------------------
intptr_t findStringPart(const utf8::String & s,const utf8::String & part,bool caseSensitive,const char * delim,bool unscreen)
{
  intptr_t i;
  for( i = enumStringParts(s) - 1; i >= 0; i-- ){
    if( caseSensitive ){
      if( stringPartByNo(s,i,delim,unscreen).compare(part) == 0 ) break;
    }
    else {
      if( stringPartByNo(s,i,delim,unscreen).casecompare(part) == 0 ) break;
    }
  }
  return i;
}
//---------------------------------------------------------------------------
utf8::String splitString(const utf8::String & s,utf8::String & s0,utf8::String & s1,const utf8::String & separator)
{
  utf8::String::Iterator i(s.strcasestr(separator));
  if( i.eos() ) s0 = s; else s0 = utf8::String(s,i);
  s1 = i + separator.length();
  return s;
}
//---------------------------------------------------------------------------
utf8::String formatByteLength(uintmax_t len,uintmax_t all,const char * fmt)
{
  if( len == 0 ) return "-";
  uintmax_t q, b, c, t1, t2, t3;
  const char * suffix = "";
  bool suffixes = false, percent = false;
  while( *fmt != '\0' ) switch( *fmt++ ){
    case 'S' : case 's' : suffixes = true; break;
    case 'P' : case 'p' : percent = true; break;
  }
  if( percent ){
    q = len * 1000000u / (all != 0 ? all : 1);
    b = q / 10000u;
    c = q % 10000u;
  }
  else {
    b = c = 0;
  }
  t2 = 1;
  if( suffixes ){
    if( len >= uintmax_t(1024u) * 1024u * 1024u * 1024u ){
      t2 = 1024u;
      t2 *= 1024u * 1024u * 1024u;
      suffix = "T";
    }
    if( len >= 1024u * 1024u * 1024u ){
      t2 = 1024u * 1024u * 1024u;
      suffix = "G";
    }
    else if( len >= 1024u * 1024u ){
      t2 = 1024u * 1024u;
      suffix = "M";
    }
    else if( len >= 1024u ){
      t2 = 1024u;
      suffix = "K";
    }
    else {
      suffixes = false;
    }
  }
  t1 = len / t2;
  t3 = len % t2;
  if( suffixes ){
    t2 = t3 / (t2 / 1024u);
    if( t2 == 0 )
      return utf8::String::print(
        percent ? "%"PRIuMAX"%s(%"PRIuMAX".%04"PRIuMAX"%%)" : "%"PRIuMAX"%s",
        t1,
        suffix,
        b,
        c
      );
    return utf8::String::print(
      percent ? "%"PRIuMAX".%04"PRIuMAX"%s(%"PRIuMAX".%04"PRIuMAX"%%)" : "%"PRIuMAX".%04"PRIuMAX"%s",
      t1,
      t2,
      suffix,
      b,
      c
    );
  }
  return utf8::String::print(
    percent ? "%"PRIuMAX"(%"PRIuMAX".%04"PRIuMAX"%%)" : "%"PRIuMAX,
    t1,
    b,
    c
  );
}
//------------------------------------------------------------------------------
utf8::String getHostName(bool noThrow,const utf8::String & def)
{
#if HAVE_UNAME
  struct utsname un;
  if( uname(&un) != 0 ){
    if( noThrow ) return def;
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#else
  struct {
    utf8::String nodename;
  } un;
  ksock::APIAutoInitializer ksockAPIAutoInitializer;
  un.nodename = ksock::SockAddr::gethostname(noThrow,def);
#endif
  return un.nodename;
}
//------------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
HMODULE getModuleHandleByAddr(void * addr)
{
  MODULEENTRY32 me;
  HANDLE hSnapshot;
  HMODULE h = NULL;
  if( addr == NULL ) addr = (void *) getModuleHandleByAddr;
  if( (hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,0)) != INVALID_HANDLE_VALUE ){
    me.dwSize = sizeof(me);
    if( Module32First(hSnapshot,&me) == TRUE ) do {
      if( (uint8_t *) me.modBaseAddr                  <= (uint8_t *) addr &&
          (uint8_t *) me.modBaseAddr + me.modBaseSize >  (uint8_t *) addr ){
        h = me.hModule;
        break;
      }
    } while( Module32Next(hSnapshot,&me) == TRUE );
    CloseHandle(hSnapshot);
  }
  return h;
}
#endif
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
utf8::String getModuleFileNameByHandle(HMODULE h)
{
  DWORD a = 1, b;
  if( isWin9x() ){
    AutoPtr<char,AutoPtrMemoryDestructor> moduleFileName;
    for(;;){
      moduleFileName.realloc(a * sizeof(char));
      if( (b = GetModuleFileNameA(h,moduleFileName,a)) < a ) break;
      a <<= 1;
    }
    moduleFileName.realloc((b + 1) * sizeof(char));
    return moduleFileName.ptr();
  }
  AutoPtr<wchar_t,AutoPtrMemoryDestructor> moduleFileName;
  for(;;){
    moduleFileName.realloc(a * sizeof(wchar_t));
    if( (b = GetModuleFileNameW(h,moduleFileName,a)) < a ) break;
    a <<= 1;
  }
  moduleFileName.realloc((b + 1) * sizeof(wchar_t));
  return moduleFileName.ptr();
}
#endif
//---------------------------------------------------------------------------
utf8::String getCurrentDir()
{
#if defined(__WIN32__) || defined(__WIN64__)
  DWORD a;
  if( isWin9x() ){
    AutoPtr<char,AutoPtrMemoryDestructor> dirName;
    dirName.realloc((a = GetCurrentDirectoryA(0,NULL) + 1) * sizeof(char));
    GetCurrentDirectoryA(a,dirName);
    strcat(dirName,pathDelimiterStr);
    return dirName.ptr();
  }
  AutoPtr<wchar_t,AutoPtrMemoryDestructor> dirName;
  dirName.realloc((a = GetCurrentDirectoryW(0,NULL) + 1) * sizeof(wchar_t));
  GetCurrentDirectoryW(a,dirName);
  wcscat(dirName,pathDelimiterStrW);
  return dirName.ptr();
#else
  AutoPtr<char,AutoPtrMemoryDestructor> dirName((char *) kmalloc(MAXPATHLEN + 1 + 1));
  if( getcwd(dirName,MAXPATHLEN + 1) == NULL ) strcpy(dirName,".");
  strcat(dirName,pathDelimiterStr);
  return dirName.ptr();
#endif
}
//---------------------------------------------------------------------------
void changeCurrentDir(const utf8::String & name)
{
  int32_t err = 0;
#if defined(__WIN32__) || defined(__WIN64__)
  BOOL r;
  if( isWin9x() ){
    r = SetCurrentDirectoryA((const char *) anyPathName2HostPathName(name).getANSIString());
  }
  else {
    r = SetCurrentDirectoryW((const wchar_t *) anyPathName2HostPathName(name).getUNICODEString());
  }
  if( r == 0 ) err = GetLastError();
#else
  if( chdir((const char *) anyPathName2HostPathName(name).getANSIString()) != 0 ) err = errno;
#endif
  if( err != 0 )
    newObjectV1C2<Exception>(err + errorOffset,name + " " + __PRETTY_FUNCTION__)->throwSP();
}
//---------------------------------------------------------------------------
utf8::String getTempFileName(const utf8::String & ext)
{
  return createGUIDAsBase32String() + (ext.isNull() ? utf8::String() : "." + ext);
}
//---------------------------------------------------------------------------
utf8::String getTempPath()
{
#if defined(__WIN32__) || defined(__WIN64__)
  DWORD a;
  if( isWin9x() ){
    AutoPtr<char,AutoPtrMemoryDestructor> dirName;
    dirName.realloc((a = GetTempPathA(0,NULL) + 2) * sizeof(char));
    GetTempPathA(a,dirName);
    if( dirName[strlen(dirName) - 1] != '\\' ) strcat(dirName,"\\");
    return dirName.ptr();
  }
  AutoPtr<wchar_t,AutoPtrMemoryDestructor> dirName;
  dirName.realloc((a = GetTempPathW(0,NULL) + 2) * sizeof(wchar_t));
  GetTempPathW(a,dirName);
  if( dirName[lstrlenW(dirName) - 1] != '\\' ) lstrcatW(dirName,L"\\");
  return dirName.ptr();
#elif HAVE_PATHS_H
  return _PATH_TMP;
#else
#error you system not have paths.h
#endif
}
//---------------------------------------------------------------------------
utf8::String changeFilePath(const utf8::String & filePathName,const utf8::String & path,const utf8::String & newPath)
{
  utf8::String::Iterator i(filePathName);
  uintptr_t l = includeTrailingPathDelimiter(path).length();
  return includeTrailingPathDelimiter(newPath) + utf8::String(i + l);
}
//---------------------------------------------------------------------------
utf8::String changeFileExt(const utf8::String & fileName,const utf8::String & extension)
{
  utf8::String::Iterator i(fileName);
  i.last();
  while( !i.bos() ){
    uintptr_t c = i.getChar();
    if( c == '\\' || c == '/' || c == ':' ) break;
    if( c == '.' ) return utf8::String(utf8::String::Iterator(fileName),i) + extension;
    i.prev();
  }
  return fileName + extension;
}
//---------------------------------------------------------------------------
utf8::String getFileExt(const utf8::String & fileName)
{
  utf8::String::Iterator i(fileName);
  i.last();
  while( !i.bos() ){
    uintptr_t c = i.getChar();
    if( c == '\\' || c == '/' || c == ':' ) break;
    if( c == '.' ) return utf8::String(i);
    i.prev();
  }
  return utf8::String();
}
//---------------------------------------------------------------------------
utf8::String getExecutableName()
{
#if defined(__WIN32__) || defined(__WIN64__)
  HMODULE h = getModuleHandleByAddr();
  if( h == NULL ) h = GetModuleHandle(NULL);
  return getModuleFileNameByHandle(h);
#else
  return argv().count() > 0 ? argv()[0] : utf8::String();
#endif
}
//---------------------------------------------------------------------------
utf8::String getExecutablePath()
{
  utf8::String name(getExecutableName());
  utf8::String::Iterator i(name);
  i.last();
  while( !i.bos() ){
    uintptr_t c = i.getChar();
    if( c == '\\' || c == '/' ) break;
    i.prev();
  }
  return utf8::String(utf8::String::Iterator(name),i + 1);
}
//---------------------------------------------------------------------------
utf8::String excludeTrailingPathDelimiter(const utf8::String & path)
{
  utf8::String s(path);
  utf8::String::Iterator i(s);
  i.last().prev();
  switch( i.getChar() ){
    case '\\' : case '/' :
      s = s.unique().resize(s.size() - i.seqLen());
    default:;
  }
  return s;
}
//---------------------------------------------------------------------------
utf8::String includeTrailingPathDelimiter(const utf8::String & path)
{
  utf8::String s(path);
  if( !s.isNull() ){
    utf8::String::Iterator i(s);
    i.last().prev();
    switch( i.getChar() ){
      case '\\' : case '/' :
        break;
      default:
        char b[2];
        b[0] = pathDelimiter;
        b[1] = '\0';
        return s + b;
    }
  }
  return s;
}
//---------------------------------------------------------------------------
bool isPathNameRelative(const utf8::String & pathName)
{
  utf8::String::Iterator i(pathName);
  uintptr_t c = 0, c2, c3;
  while( !i.eos() ){
    if( !i.isSpace() ){
      switch( c2 = i.getChar() ){
        case '\\' : case '/' : return false;
	      default: ;
      }
      if( c != 0 && c2 == ':' ) return false;
      if( c == 0 && ((c3 = i.getUpperChar()) < 'A' || c3 > 'Z') ) break;
      c = i.getChar();
    }
    i.next();
  }
  return true;
}
//---------------------------------------------------------------------------
utf8::String absolutePathNameFromWorkDir(
  const utf8::String & workDir,
  const utf8::String & pathName)
{
  if( isPathNameRelative(pathName) ) return includeTrailingPathDelimiter(workDir) + pathName;
  return pathName;
}
//---------------------------------------------------------------------------
bool createDirectory(const utf8::String & name)
{
//  fprintf(stderr,"%s %d %s\n",__FILE__,__LINE__,name.c_str());
  if( currentFiber() != NULL ){
    currentFiber()->event_.timeout_ = ~uint64_t(0);
    currentFiber()->event_.string0_ = name;
    currentFiber()->event_.type_ = etCreateDir;
    currentFiber()->thread()->postRequest();
    currentFiber()->switchFiber(currentFiber()->mainFiber());
    assert( currentFiber()->event_.type_ == etCreateDir );
#if defined(__WIN32__) || defined(__WIN64__)
    if( currentFiber()->event_.errno_ - errorOffset == ERROR_ALREADY_EXISTS ) return false;
#else
    if( currentFiber()->event_.errno_ == EEXIST ) return false;
#endif
    if( currentFiber()->event_.errno_ != 0 )
      newObjectV1C2<Exception>(currentFiber()->event_.errno_,name + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
    return currentFiber()->event_.rval_;
  }
  int32_t err = 0;
#if defined(__WIN32__) || defined(__WIN64__)
  if( isWin9x() ){
    if( CreateDirectoryA(anyPathName2HostPathName(name).getANSIString(),NULL) == 0 )
      err = GetLastError();
  }
  else {
    if( CreateDirectoryW(anyPathName2HostPathName(name).getUNICODEString(),NULL) == 0 )
      err = GetLastError();
  }
  if( err == ERROR_PATH_NOT_FOUND ){
#else
  mode_t um = umask(0);
  umask(um);
  if( mkdir(anyPathName2HostPathName(name).getANSIString(),um | S_IRUSR | S_IWUSR | S_IXUSR) != 0 )
    err = errno;
//  fprintf(stderr,"%s %d err = %d %s\n",__FILE__,__LINE__,err,name.c_str());
  if( err == ENOTDIR ){
#endif
    utf8::String parentDir(getPathFromPathName(name));
    if( !parentDir.isNull() ){
      if( createDirectory(parentDir) ) return createDirectory(name);
      err = oserror();
    }
  }
#if defined(__WIN32__) || defined(__WIN64__)
  if( err != 0 && err != ERROR_ALREADY_EXISTS )
#else
  if( err != 0 && err != EEXIST )
#endif
    newObjectV1C2<Exception>(err + errorOffset,name + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
  oserror(err);
  return err == 0;
}
//---------------------------------------------------------------------------
static int32_t removeDirectoryHelper(const utf8::String & name)
{
  int32_t err = 0;
#if defined(__WIN32__) || defined(__WIN64__)
  if( isWin9x() ){
    if( RemoveDirectoryA(anyPathName2HostPathName(name).getANSIString()) == 0 )
      err = GetLastError();
  }
  else {
    if( RemoveDirectoryW(anyPathName2HostPathName(name).getUNICODEString()) == 0 )
      err = GetLastError();
  }
#else
  if( rmdir(anyPathName2HostPathName(name).getANSIString()) != 0 ) err = errno;
#endif
  oserror(err);
  return err;
}
//---------------------------------------------------------------------------
bool removeDirectory(const utf8::String & name,bool recursive,bool noThrow)
{
  Fiber * fiber = currentFiber();
  if( fiber != NULL ){
    fiber->event_.timeout_ = ~uint64_t(0);
    fiber->event_.string0_ = name;
    fiber->event_.recursive_ = recursive;
    fiber->event_.type_ = etRemoveDir;
    fiber->thread()->postRequest();
    fiber->switchFiber(fiber->mainFiber());
    assert( fiber->event_.type_ == etRemoveDir );
#if defined(__WIN32__) || defined(__WIN64__)
    if( fiber->event_.errno_ - errorOffset == ERROR_PATH_NOT_FOUND ||
        fiber->event_.errno_ - errorOffset == ERROR_FILE_NOT_FOUND ) return false;
#else
    if( fiber->event_.errno_ == ENOTDIR ) return false;
#endif
    if( fiber->event_.errno_ != 0 && !noThrow )
      newObjectV1C2<Exception>(fiber->event_.errno_,name + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
    return fiber->event_.rval_ && fiber->event_.errno_ == 0;
  }
  int32_t err = removeDirectoryHelper(name);
#if defined(__WIN32__) || defined(__WIN64__)
  if( err == ERROR_DIR_NOT_EMPTY ){
#else
  if( err == ENOTEMPTY ){
#endif
    Vector<utf8::String> list;
    getDirList(list,includeTrailingPathDelimiter(name) + "*",utf8::String(),recursive);
    err = 0;
    bool rval = true;
    for( intptr_t i = list.count() - 1; i >= 0; i-- ){
      struct Stat st;
      stat(list[i],st);
      if( (st.st_mode & S_IFDIR) != 0 && recursive ){
        if( !removeDirectory(list[i],true) ){
          if( rval ){
            err = oserror();
            rval = false;
          }
        }
      }
      else {
        if( !remove(list[i]) ){
          if( rval ){
            err = oserror();
            rval = false;
          }
        }
      }
    }
    if( rval ){
      err = removeDirectoryHelper(name);
    }
    else {
      oserror(err);
      return rval;
    }
  }
#if defined(__WIN32__) || defined(__WIN64__)
  if( err == ERROR_PATH_NOT_FOUND || err == ERROR_FILE_NOT_FOUND ) return false;
#else
  if( err == ENOTDIR ) return false;
#endif
  oserror(err);
  if( err != 0 && !noThrow )
    newObjectV1C2<Exception>(err + errorOffset,name + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
  return err == 0;
}
//---------------------------------------------------------------------------
bool remove(const utf8::String & name,bool noThrow)
{
  Fiber * fiber = currentFiber();
  if( fiber != NULL ){
    fiber->event_.timeout_ = ~uint64_t(0);
    fiber->event_.string0_ = name;
    fiber->event_.type_ = etRemoveFile;
    fiber->thread()->postRequest();
    fiber->switchFiber(fiber->mainFiber());
    assert( fiber->event_.type_ == etRemoveFile );
#if defined(__WIN32__) || defined(__WIN64__)
    if( fiber->event_.errno_ - errorOffset == ERROR_PATH_NOT_FOUND ||
        fiber->event_.errno_ - errorOffset == ERROR_FILE_NOT_FOUND ) return false;
#else
    if( fiber->event_.errno_ == ENOENT ) return false;
#endif
    if( fiber->event_.errno_ != 0 && !noThrow )
      newObjectV1C2<Exception>(fiber->event_.errno_,name + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
    return fiber->event_.rval_ && fiber->event_.errno_ == 0;
  }
  int32_t err = 0;
  oserror(0);
#if defined(__WIN32__) || defined(__WIN64__)
  if( isWin9x() ){
    if( DeleteFileA(anyPathName2HostPathName(name).getANSIString()) == 0 )
      err = GetLastError();
  }
  else {
    if( DeleteFileW(anyPathName2HostPathName(name).getUNICODEString()) == 0 )
      err = GetLastError();
  }
  if( err == ERROR_PATH_NOT_FOUND || err == ERROR_FILE_NOT_FOUND ) return false;
#else
  if( unlink(anyPathName2HostPathName(name).getANSIString()) != 0 ) err = errno;
  if( err == ENOENT ) return false;
#endif
  if( err != 0 ){
    struct Stat st;
    stat(name,st);
    if( (st.st_mode & S_IFDIR) != 0 ){
      try {
        if( removeDirectory(name,true) ) return true;
        err = oserror();
      }
      catch( ExceptionSP & e){
        err = e->code() >= errorOffset ? e->code() - errorOffset : e->code();
      }
    }
    if( !noThrow )
      newObjectV1C2<Exception>(err + errorOffset,name + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
    oserror(err);
  }
  return err == 0;
}
//---------------------------------------------------------------------------
#if !defined(__WIN32__) && !defined(__WIN64__)
void chModOwn(
  const utf8::String & pathName,
  const Mutant & mode,const Mutant & user,const Mutant & group)
{
  int32_t err;
  union {
    mode_t m;
    intmax_t m2;
  };
  m2 = umask(0);
  umask(m);
  if( mode.type() == mtInt ){
    m2 = mode;
  }
  else {
    utf8::tryStr2Int(mode,m2,8);
  }
  if( chmod(anyPathName2HostPathName(pathName).getANSIString(),(mode_t) m2) != 0 ){
    err = errno;
    newObjectV1C2<Exception>(err,pathName + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
  }
  const struct passwd * u = getpwnam(utf8::String(user).getANSIString());
  uid_t userID(u != NULL ? u->pw_uid : (uid_t) user);
  const struct group * g = getgrnam(utf8::String(group).getANSIString());
  gid_t groupID(g != NULL ? g->gr_gid : (gid_t) group);
  if( chown(anyPathName2HostPathName(pathName).getANSIString(),userID,groupID) != 0 ){
    err = errno;
    newObjectV1C2<Exception>(err,pathName + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
  }
}
#else
void chModOwn(const utf8::String &,const Mutant &,const Mutant &,const Mutant &)
{
}
#endif
//---------------------------------------------------------------------------
utf8::String getRootFromPathName(const utf8::String & pathName)
{
#if defined(__WIN32__) || defined(__WIN64__)
  uintptr_t offset = pathName.ncompare("\\\\?\\",4) == 0 || pathName.ncompare("//?/",4) == 0 ? 4 : 0;
  utf8::String s(utf8::String::Iterator(pathName) + offset);
  if( s.ncompare("\\\\",2) == 0 || s.ncompare("//",2) == 0 ){ // windows network path
    utf8::String::Iterator i(utf8::String::Iterator(s) + 2);
    while( !i.bos() ){
      uintptr_t c = i.getChar();
      if( c == '/' || c == '\\' ){ // windows network server name
        while( !i.bos() ){
          c = i.getChar();
          if( c == '/' || c == '\\' ) break; // windows network server resource name
          i.next();
        }
        return utf8::String(s,i) + pathDelimiterStr;
      }
      i.next();
    }
    return utf8::String(s,i) + pathDelimiterStr;
  }
  utf8::String::Iterator i(s);
  while( !i.bos() ){
    uintptr_t c = i.getChar();
    if( c == '/' || c == '\\' ) break;
    if( c == ':' ){
      return utf8::String(s,i + 1) + pathDelimiterStr;
    }
    i.next();
  }
  return getRootFromPathName(getCurrentDir());
#else
// FIX ME
  return pathDelimiterStr;
#endif
}
//---------------------------------------------------------------------------
utf8::String getPathFromPathName(const utf8::String & pathName)
{
  utf8::String::Iterator i(pathName);
  i.last();
  if( (i - 1).getChar() == ':' ) return utf8::String();
  while( !i.bos() ){
    switch( i.getChar() ){
      case ':' :
        return utf8::String(utf8::String::Iterator(pathName),i + 1);
      case '\\' : case '/' :
        return utf8::String(utf8::String::Iterator(pathName),i);
      default :;
    }
    i.prev();
  }
  return ".";
}
//---------------------------------------------------------------------------
utf8::String getNameFromPathName(const utf8::String & pathName)
{
  utf8::String::Iterator i(pathName);
  i.last();
  while( !i.bos() ){
    switch( i.getChar() ){
      case '\\' : case '/' :
        return utf8::String(i + 1,utf8::String::Iterator(pathName).last());
      default :;
    }
    i.prev();
  }
  return i;
}
//---------------------------------------------------------------------------
utf8::String anyPathName2HostPathName(const utf8::String & pathName)
{
  if( pathDelimiter == '\\' && !pathName.strstr("/").eos() )
    return pathName.replaceAll("/",pathDelimiterStr);
  if( pathDelimiter == '/' && !pathName.strstr("\\").eos() )
    return pathName.replaceAll("\\",pathDelimiterStr);
  return pathName;
}
//---------------------------------------------------------------------------
bool nameFitMask(const utf8::String & name,const utf8::String & mask)
{
  uintptr_t c;
  utf8::String::Iterator ni(name), mi(mask);
  while( !ni.eos() && !mi.eos() ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( (c = mi.getUpperChar()) == '?' ){
#else
    if( (c = mi.getChar()) == '?' ){
#endif
      ni.next();
      mi.next();
    }
    else if( c == '*' ){
      mi.next();
      if( mi.eos() ){ ni.last(); break; }
#if defined(__WIN32__) || defined(__WIN64__)
      while( ni.getUpperChar() != mi.getUpperChar() ){
        ni.next();
        if( ni.eos() ) break;
      }
      while( ni.getUpperChar() == (c = mi.getUpperChar()) && c != '*' && c != '?' && c != '[' ){
        ni.next();
        mi.next();
        if( ni.eos() ) break;
      }
#else
      while( !mi.eos() && !ni.eos() && ni.getChar() != mi.getChar() ){
        ni.next();
        if( ni.eos() ) break;
      }
      while( ni.getChar() == (c = mi.getChar()) && c != '*' && c != '?' && c != '[' ){
        ni.next();
        mi.next();
        if( ni.eos() ) break;
      }
#endif
    }
    else if( c == '[' ){
//      mi.next();
      newObjectV1C2<Exception>(ENOSYS,utf8::String(__PRETTY_FUNCTION__) + " " + utf8::int2Str(__LINE__) + " FIXME")->throwSP();
    }
    else {
#if defined(__WIN32__) || defined(__WIN64__)
      if( ni.getUpperChar() != c ) break;
#else
      if( ni.getChar() != c ) break;
#endif
      ni.next();
      mi.next();
    }
  }
  return ni.eos() && mi.eos();
}
//---------------------------------------------------------------------------
bool rename(const utf8::String & oldPathName,const utf8::String & newPathName,bool createPathIfNotExist,bool noThrow)
{
  oserror(0);
  Fiber * fiber = currentFiber();
  if( fiber != NULL ){
    fiber->event_.timeout_ = ~uint64_t(0);
    fiber->event_.string0_ = oldPathName;
    fiber->event_.string1_ = newPathName;
    fiber->event_.createIfNotExist_ = createPathIfNotExist;
    fiber->event_.type_ = etRename;
    fiber->thread()->postRequest();
    fiber->switchFiber(currentFiber()->mainFiber());
    assert( fiber->event_.type_ == etRename );
    if( fiber->event_.errno_ != 0 && !noThrow )
      newObjectV1C2<Exception>(fiber->event_.errno_,__PRETTY_FUNCTION__ + utf8::String(" ") + oldPathName)->throwSP();
    oserror(fiber->event_.errno_);
  }
  else {
#if defined(__WIN32__) || defined(__WIN64__)
    BOOL r;
    if( isWin9x() ){
      r = MoveFileA(
        anyPathName2HostPathName(oldPathName).getANSIString(),
        anyPathName2HostPathName(newPathName).getANSIString()
      );
    }
    else {
      r = MoveFileExW(
        anyPathName2HostPathName(oldPathName).getUNICODEString(),
        anyPathName2HostPathName(newPathName).getUNICODEString(),
        MOVEFILE_COPY_ALLOWED
      );
    }
    if( r == 0 ){
      int32_t err = GetLastError();
      if( err == ERROR_PATH_NOT_FOUND && createPathIfNotExist ){
        createDirectory(getPathFromPathName(newPathName));
        return rename(oldPathName,newPathName,false);
      }
#else
    if( ::rename(anyPathName2HostPathName(oldPathName).getANSIString(),anyPathName2HostPathName(newPathName).getANSIString()) != 0 ){
      int32_t err = errno;
      if( err == ENOTDIR && createPathIfNotExist ){
        createDirectory(getPathFromPathName(newPathName));
        return rename(oldPathName,newPathName,false);
      }
#endif
      if( !noThrow )
        newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__ + utf8::String(" ") + oldPathName)->throwSP();
      oserror(err);
    }
  }
  return oserror() == 0;
}
//---------------------------------------------------------------------------
void copy(const utf8::String & dstPathName,const utf8::String & srcPathName,uintptr_t bufferSize)
{
  if( currentFiber() != NULL ){
    currentFiber()->event_.timeout_ = ~uint64_t(0);
    currentFiber()->event_.string0_ = dstPathName;
    currentFiber()->event_.string1_ = srcPathName;
    currentFiber()->event_.count_ = bufferSize;
    currentFiber()->event_.type_ = etCopy;
    currentFiber()->thread()->postRequest();
    currentFiber()->switchFiber(currentFiber()->mainFiber());
    assert( currentFiber()->event_.type_ == etRename );
    if( currentFiber()->event_.errno_ != 0 )
      newObjectV1C2<Exception>(currentFiber()->event_.errno_,__PRETTY_FUNCTION__ + utf8::String(" ") + srcPathName)->throwSP();
  }
  else {
    if( bufferSize == 0 ) bufferSize = getpagesize() * 4u;
#if defined(__WIN32__) || defined(__WIN64__)
    BOOL r;
    if( isWin9x() ){
      r = CopyFileA(
        anyPathName2HostPathName(srcPathName).getANSIString(),
        anyPathName2HostPathName(dstPathName).getANSIString(),
        FALSE
      );
    }
    else {
      BOOL bCancel = FALSE;
      r = CopyFileExW(
        anyPathName2HostPathName(srcPathName).getUNICODEString(),
        anyPathName2HostPathName(dstPathName).getUNICODEString(),
        NULL,
        NULL,
        &bCancel,
        0
      );
    }
    if( r == 0 ){
      int32_t err = oserror() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + srcPathName)->throwSP();
    }
#else
    AsyncFile dstFile, srcFile;
    dstFile.fileName(dstPathName).createIfNotExist(true).exclusive(true);
    srcFile.fileName(srcPathName).readOnly(true);
    dstFile.open();
    srcFile.open();
    AutoPtrBuffer buffer;
    buffer.alloc(bufferSize);
    dstFile.resize(0);
    for( uint64_t ll, l = srcFile.size(); l > 0; l -= ll ){
      ll = bufferSize > l ? l : bufferSize;
      srcFile.readBuffer(buffer,ll);
      dstFile.writeBuffer(buffer,ll);
    }
#endif
  }
}
//---------------------------------------------------------------------------
void ksleep(uint64_t timeout)
{
  if( currentFiber() != NULL ){
    currentFiber()->event_.abort_ = false;
    currentFiber()->event_.timerStartTime_ = gettimeofday();
    currentFiber()->event_.timeout_ = timeout;
    currentFiber()->event_.type_ = etTimer;
    currentFiber()->thread()->postRequest();
    currentFiber()->switchFiber(currentFiber()->mainFiber());
    assert( currentFiber()->event_.type_ == etTimer );
    if( currentFiber()->event_.errno_ != 0 )
      newObjectV1C2<Exception>(currentFiber()->event_.errno_ + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  }
  else {
#if HAVE_NANOSLEEP
    struct timespec rqtp;
    rqtp.tv_sec = timeout / 1000000u;
    rqtp.tv_nsec = timeout % 1000000u;
    nanosleep(&rqtp, NULL);
#elif defined(__WIN32__) || defined(__WIN64__)
    Sleep((DWORD) (timeout / 1000u + (timeout < 1000000u)));
#elif HAVE_USLEEP
    usleep(timeout / 1000u);
#elif HAVE_SLEEP
    sleep(timeout / 1000000u);
#endif
  }
}
//---------------------------------------------------------------------------
static bool getDirListHelper(
  const utf8::String & name,
  const utf8::String & mask,
  const utf8::String & exMask,
  bool exMaskAsList)
{
  bool add = false;
  if( nameFitMask(name,mask) ){
    if( exMaskAsList ){
      add = true;
      for( intptr_t i = enumStringParts(exMask) - 1; i >= 0; i-- )
        if( nameFitMask(name,stringPartByNo(exMask,i)) ){
          add = false;
          break;
        }
    }
    else {
      add = !nameFitMask(name,exMask);
    }
  }
  return add;
}
//---------------------------------------------------------------------------
void getDirList(
  Vector<utf8::String> & list,
  const utf8::String & dirAndMask,
  const utf8::String & exMask,
  bool recursive,
  bool includeDirs,
  bool exMaskAsList)
{
  Fiber * fiber = currentFiber();
  if( fiber != NULL ){
    fiber->event_.timeout_ = ~uint64_t(0);
    fiber->event_.dirList_ = &list;
    fiber->event_.string0_ = dirAndMask;
    fiber->event_.string1_ = exMask;
    fiber->event_.recursive_ = recursive;
    fiber->event_.includeDirs_ = includeDirs;
    fiber->event_.exMaskAsList_ = exMaskAsList;
    fiber->event_.type_ = etDirList;
    fiber->thread()->postRequest();
    fiber->switchFiber(fiber->mainFiber());
    assert( fiber->event_.type_ == etDirList );
    if( fiber->event_.errno_ != 0 )
      newObjectV1C2<Exception>(currentFiber()->event_.errno_,__PRETTY_FUNCTION__ + utf8::String(" ") + dirAndMask)->throwSP();
    return;
  }
  int32_t err;
  utf8::String path(getPathFromPathName(dirAndMask));
  utf8::String mask(getNameFromPathName(dirAndMask));

#if defined(__WIN32__) || defined(__WIN64__)
  union {
    WIN32_FIND_DATAA fda;
    WIN32_FIND_DATAW fdw;
  };
  HANDLE handle = NULL;
  if( isWin9x() ){
    handle = FindFirstFileA(anyPathName2HostPathName(path + pathDelimiterStr + "*").getANSIString(),&fda);
    if( handle == INVALID_HANDLE_VALUE ){
      err = GetLastError();
      if( err == ERROR_PATH_NOT_FOUND ) return;
      newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__)->throwSP();
    }
    try {
      do {
        if( strcmp(fda.cFileName,".") == 0 || strcmp(fda.cFileName,"..") == 0 ) continue;
        const char * d_name = fda.cFileName;
        if( (fda.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ){
          bool fit = getDirListHelper(d_name,mask,exMask,exMaskAsList);
          if( includeDirs && fit )
            list.add(path + pathDelimiterStr + d_name);
          if( recursive && fit )
#else
    DIR * dir = opendir(anyPathName2HostPathName(path).getANSIString());
    if( dir == NULL ){
      err = errno;
      if( err == ENOTDIR ) return;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + dirAndMask)->throwSP();
    }
    try {
      struct dirent * ent;
#if HAVE_READDIR_R
      struct dirent * result, res;
      ent = &res;
      for(;;){
        if( readdir_r(dir,ent,&result) != 0 ){
          err = errno;
          newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + dirAndMask)->throwSP();
        }
        if( result == NULL ) break;
#else
        while( (ent = readdir(dir)) != NULL ){
#endif
        if( strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0 ) continue;
        const char * d_name = ent->d_name;
#ifdef DT_DIR
        if( ent->d_type == DT_DIR ){
          bool fit = getDirListHelper(d_name,mask,exMask,exMaskAsList);
          if( includeDirs && fit )
            list.add(path + pathDelimiterStr + d_name);
          if( recursive && fit )
#else
        struct Stat st;
        stat(path + pathDelimiterStr + ent->d_name,st);
        if( (st.st_mode & S_IFDIR) != 0 ){
          if( recursive )
#endif
#endif
            getDirList(
              list,
              path + pathDelimiterStr + d_name + pathDelimiterStr + mask,
              exMask,
              recursive,
              includeDirs,
              exMaskAsList
            );
        }
        else if( getDirListHelper(d_name,mask,exMask,exMaskAsList) ){
          list.add(path + pathDelimiterStr + d_name);
        }
#if defined(__WIN32__) || defined(__WIN64__)
      } while( FindNextFileA(handle,&fda) != 0 );
      if( GetLastError() != ERROR_NO_MORE_FILES ){
        err = GetLastError() + errorOffset;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + dirAndMask)->throwSP();
      }
    }
    catch( ... ){
      FindClose(handle);
      throw;
    }
    FindClose(handle);
  }
  else {
    handle = FindFirstFileW(anyPathName2HostPathName(path + pathDelimiterStr + "*").getUNICODEString(),&fdw);
    if( handle == INVALID_HANDLE_VALUE ){
      err = GetLastError();
      if( err == ERROR_PATH_NOT_FOUND ) return;
      newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__ + utf8::String(" ") + dirAndMask)->throwSP();
    }
    try {
      do {
        if( lstrcmpW(fdw.cFileName,L".") == 0 || lstrcmpW(fdw.cFileName,L"..") == 0 ) continue;
        if( (fdw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ){
          bool fit = getDirListHelper(fdw.cFileName,mask,exMask,exMaskAsList);
          if( includeDirs && fit )
            list.add(path + pathDelimiterStr + fdw.cFileName);
          if( recursive && fit )
            getDirList(
              list,
              path + pathDelimiterStr + fdw.cFileName + pathDelimiterStr + mask,
              exMask,
              recursive,
              includeDirs,
              exMaskAsList
            );
        }
        else if( getDirListHelper(fdw.cFileName,mask,exMask,exMaskAsList) ){
          list.add(path + pathDelimiterStr + fdw.cFileName);
        }
      } while( FindNextFileW(handle,&fdw) != 0 );
      if( GetLastError() != ERROR_NO_MORE_FILES ){
        err = GetLastError() + errorOffset;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + dirAndMask)->throwSP();
      }
    }
    catch( ... ){
      FindClose(handle);
      throw;
    }
    FindClose(handle);
  }
#else
      }
    }
    catch( ... ){
      closedir(dir);
      throw;
    }
    closedir(dir);
#endif
}
//---------------------------------------------------------------------------
// переключение раскладки клавиатуры
// LoadKeyboardLayout("00000419", KLF_ACTIVATE); //русский
//---------------------------------------------------------------------------
void copyStrToClipboard(const utf8::String & s)
{
#if defined(__WIN32__) || defined(__WIN64__)
  int32_t err;
  if( OpenClipboard(NULL) == 0 ){
    err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( EmptyClipboard() == 0 ){
    err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( isWin9x() ){
    utf8::AnsiString pass(s.getANSIString());
    HGLOBAL w = GlobalAlloc(
      GMEM_MOVEABLE | GMEM_DDESHARE,
      (strlen(pass) + 1) * sizeof(char)
    );
    if( w == NULL ){
      err = GetLastError() + errorOffset;
      CloseClipboard();
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    LPVOID ww = GlobalLock(w);
    if( ww == NULL ){
      err = GetLastError() + errorOffset;
      CloseClipboard();
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    memcpy(ww,(const char *) pass,(strlen(pass) + 1) * sizeof(char));
    GlobalUnlock(ww);
    if( SetClipboardData(CF_TEXT,w) == NULL ){
      err = GetLastError() + errorOffset;
      GlobalFree(w);
      CloseClipboard();
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
  }
  else {
    utf8::WideString pass(s.getUNICODEString());
    HGLOBAL w = GlobalAlloc(
      GMEM_MOVEABLE | GMEM_DDESHARE,
      (lstrlenW(pass) + 1) * sizeof(wchar_t)
    );
    if( w == NULL ){
      err = GetLastError() + errorOffset;
      CloseClipboard();
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    LPVOID ww = GlobalLock(w);
    if( ww == NULL ){
      err = GetLastError() + errorOffset;
      CloseClipboard();
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    memcpy(ww,(const wchar_t *) pass,(lstrlenW(pass) + 1) * sizeof(wchar_t));
    GlobalUnlock(ww);
    if( SetClipboardData(CF_UNICODETEXT,w) == NULL ){
      err = GetLastError() + errorOffset;
      GlobalFree(w);
      CloseClipboard();
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
  }
  CloseClipboard();
#endif
}
//---------------------------------------------------------------------------
pid_t execute(const utf8::String & name,const Array<utf8::String> & args,const Array<utf8::String> * env,bool wait,bool usePathEnv,bool noThrow)
{
  utf8::String s;
  for( uintptr_t i = 0; i < args.count(); i++ ){
    if( i > 0 ) s += " ";
    s += args[i];
  }
  return execute(name,s,env,wait,usePathEnv,noThrow);
}
//---------------------------------------------------------------------------
pid_t execute(const utf8::String & name,const utf8::String & args,const Array<utf8::String> * env,bool wait,bool usePathEnv,bool noThrow)
{
  ExecuteProcessParameters params;
  params.name_ = name;
  params.args_ = args;
  if( env != NULL ) params.env_ = *env;
  params.wait_ = wait;
  params.usePathEnv_ = usePathEnv;
  params.noThrow_ = noThrow;
  return execute(params);
}
//---------------------------------------------------------------------------
pid_t execute(const ExecuteProcessParameters & params)
{
  Fiber * fiber = currentFiber();
  if( fiber != NULL ){
    fiber->event_.timeout_ = ~uint64_t(0);
    fiber->event_.executeParameters_ = &params;
    fiber->event_.type_ = etExec;
    fiber->thread()->postRequest();
    fiber->switchFiber(fiber->mainFiber());
    assert( fiber->event_.type_ == etExec );
    if( fiber->event_.errno_ != 0 ){
      if( !params.noThrow_ )
        newObjectV1C2<Exception>(fiber->event_.errno_,__PRETTY_FUNCTION__ + utf8::String(" ") + params.name_)->throwSP();
      oserror(fiber->event_.errno_);
      return -1;
    }
    return (pid_t) fiber->event_.data_;
  }
#if defined(__WIN32__) || defined(__WIN64__)
  BOOL r;
  uintptr_t i;
  DWORD rr = 0;
  union {
    STARTUPINFOA sui;
    STARTUPINFOW suiW;
  };
  //SECURITY_ATTRIBUTES sa;
  //sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  //sa.bInheritHandle = TRUE;
  //sa.lpSecurityDescriptor = NULL;
  memset(&suiW,0,sizeof(suiW));
  PROCESS_INFORMATION pi;
  memset(&pi,0,sizeof(pi));
  Array<utf8::String> path;
  int32_t err;
  if( isWin9x() ){
    AutoPtr<char,AutoPtrMemoryDestructor> e;
    uintptr_t eCount = 0;
    if( params.env_.count() > 0 ){
      for( i = 0; i < params.env_.count(); i++ ){
        if( params.usePathEnv_ && params.env_[i].ncasecompare("PATH=",5) == 0 ){
          utf8::String a(utf8::String::Iterator(params.env_[i]) + 5);
          for( intptr_t k = enumStringParts(a,";") - 1; k >= 0; k-- ) path.add(stringPartByNo(a,k,";"));
        }
        utf8::AnsiString s(params.env_[i].getANSIString());
        uintptr_t l = strlen(s) + 1;
        e.reallocT(eCount + l + 1);
        strcpy(e.ptr() + eCount,s);
        eCount += l;
        e[eCount] = '\0';
      }
    }
    sui.cb = sizeof(sui);
    sui.hStdInput = params.stdio_;
    sui.hStdOutput = params.stdout_;
    sui.hStdError = params.stderr_;
    if( sui.hStdInput != INVALID_HANDLE_VALUE || sui.hStdOutput != INVALID_HANDLE_VALUE || sui.hStdError != INVALID_HANDLE_VALUE ){
      sui.dwFlags |= STARTF_USESTDHANDLES;
      if( sui.hStdInput != INVALID_HANDLE_VALUE && SetHandleInformation(sui.hStdInput,HANDLE_FLAG_INHERIT,1) == 0 ){
        err = GetLastError() + errorOffset;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
      if( sui.hStdOutput != INVALID_HANDLE_VALUE && SetHandleInformation(sui.hStdOutput,HANDLE_FLAG_INHERIT,1) == 0 ){
        err = GetLastError() + errorOffset;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
      if( sui.hStdError != INVALID_HANDLE_VALUE && SetHandleInformation(sui.hStdError,HANDLE_FLAG_INHERIT,1) == 0 ){
        err = GetLastError() + errorOffset;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
    }
    utf8::AnsiString name((anyPathName2HostPathName("\"" + params.name_ + "\" ") + params.args_).getANSIString());
    r = CreateProcessA(
      NULL,
      name,
      NULL,
      NULL,
      sui.dwFlags & STARTF_USESTDHANDLES ? TRUE : FALSE,
      /*DETACHED_PROCESS |*/ CREATE_NO_WINDOW | CREATE_SUSPENDED,
      e,
      NULL,
      &sui,
      &pi
    );
  }
  else {
    AutoPtr<wchar_t,AutoPtrMemoryDestructor> e;
    uintptr_t eCount = 0;
    if( params.env_.count() > 0 ){
      for( i = 0; i < params.env_.count(); i++ ){
        if( params.usePathEnv_ && params.env_[i].ncasecompare("PATH=",5) == 0 ){
          utf8::String a(utf8::String::Iterator(params.env_[i]) + 5);
          for( intptr_t k2 = enumStringParts(a,";"), k = 0; k < k2; k++ ) path.add(stringPartByNo(a,k,";"));
        }
        uintptr_t l = params.env_[i].length() + 1;
        e.reallocT(eCount + l + 1);
        utf8::utf8s2ucs(params.env_[i].c_str(),l,e.ptr() + eCount);
        eCount += l;
        e[eCount] = L'\0';
      }
    }
    suiW.cb = sizeof(suiW);
    suiW.hStdInput = params.stdio_;
    suiW.hStdOutput = params.stdout_;
    suiW.hStdError = params.stderr_;
    if( suiW.hStdInput != INVALID_HANDLE_VALUE || suiW.hStdOutput != INVALID_HANDLE_VALUE || suiW.hStdError != INVALID_HANDLE_VALUE ){
      suiW.dwFlags |= STARTF_USESTDHANDLES;
      if( suiW.hStdInput != INVALID_HANDLE_VALUE && SetHandleInformation(suiW.hStdInput,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT) == 0 ){
        err = GetLastError() + errorOffset;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
      if( suiW.hStdOutput != INVALID_HANDLE_VALUE && SetHandleInformation(suiW.hStdOutput,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT) == 0 ){
        err = GetLastError() + errorOffset;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
      if( suiW.hStdError != INVALID_HANDLE_VALUE && SetHandleInformation(suiW.hStdError,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT) == 0 ){
        err = GetLastError() + errorOffset;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
    }

    utf8::WideString name((anyPathName2HostPathName("\"" + params.name_ + "\" ") + params.args_).getUNICODEString());
    r = CreateProcessW(
      NULL,
      name,
      NULL,
      NULL,
      suiW.dwFlags & STARTF_USESTDHANDLES ? TRUE : FALSE,
      /*DETACHED_PROCESS |*/ CREATE_NO_WINDOW | CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT,
      e,
      NULL,
      &suiW,
      &pi
    );
  }
  if( r == 0 ){
    err = GetLastError();
    if( params.usePathEnv_ && (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND || err == ERROR_INVALID_NAME) ){
      utf8::String a(getEnv("PATH"));
      for( intptr_t k2 = enumStringParts(a,";"), k = 0; k < k2; k++ ) path.add(stringPartByNo(a,k,";"));
      for( i = 0; i < path.count(); i++ ){
        ExecuteProcessParameters params2(params);
        params2.name_ = includeTrailingPathDelimiter(path[i]) + params.name_;
        params2.usePathEnv_ = false;
        params2.noThrow_ = true;
        pid_t exitCode = execute(params2);
        if( exitCode >= 0 ){
          params.pathName_ = params2.name_;
          return exitCode;
        }
        params2.name_ += ".exe";
        exitCode = execute(params2);
        if( exitCode >= 0 ){
          params.pathName_ = params2.name_;
          return exitCode;
        }
        err = GetLastError();
      }
    }
    if( !params.noThrow_ ) newObjectV1C2<Exception>(err + errorOffset,params.name_ + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
    SetLastError(err);
    return -1;
  }
  else {
    params.pathName_ = params.name_;
  }
  if( ResumeThread(pi.hThread) == DWORD(-1) ) goto err;
  if( params.wait_ ){
    rr = WaitForSingleObject(pi.hProcess,INFINITE);
    if( rr == WAIT_FAILED ){
err:  err = GetLastError() + errorOffset;
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      if( params.noThrow_ ) return pi.dwProcessId;
      newObjectV1C2<Exception>(err,params.name_ + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
    }
    if( rr != WAIT_OBJECT_0 ){
      if( params.noThrow_ ) return pi.dwProcessId;
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      if( params.noThrow_ ) return pi.dwProcessId;
      newObjectV1C2<Exception>(ERROR_INVALID_DATA,params.name_ + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
    }
    DWORD exitCode = 0;
    if( GetExitCodeProcess(pi.hProcess,&exitCode) == 0 ){
      err = GetLastError() + errorOffset;
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      if( params.noThrow_ ) return pi.dwProcessId;
      newObjectV1C2<Exception>(err,params.name_ + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
    }
    pi.dwProcessId = exitCode;
  }
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return pi.dwProcessId;
#else
  //utf8::AnsiString name((anyPathName2HostPathName("\"" + params.name_ + "\" ") + params.args_).getANSIString());
  Array<utf8::AnsiString> aargs;
  utf8::String::Iterator sep(params.args_);
  while( !sep.eos() ){
    while( sep.isSpace() && sep.next() );
    utf8::String::Iterator sep2(sep + 1);
    if( sep.getChar() == '\"' ){
      while( (sep2.getChar() != '\"' || (sep2 - 1).getChar() == '\\') && sep2.next() );
    }
    else {
      while( (!sep2.isSpace() || (sep2 - 1).getChar() == '\\') && sep2.next() );
    }
    aargs.add(utf8::String(sep,sep2).getANSIString());
    sep = sep2;
  }
  utf8::AnsiString nameHolder(params.name_.getANSIString());
  const char * name = nameHolder;
  AutoPtr<char *,AutoPtrMemoryDestructor> argsPtrs;
  argsPtrs.reallocT(aargs.count() + 2);
  argsPtrs[0] = nameHolder;
  for( uintptr_t i = 0; i < aargs.count(); i++ ) argsPtrs[i + 1] = aargs[i];
  argsPtrs[aargs.count() + 1] = NULL;
  pid_t pid = fork();
  if( pid == -1 ){
    if( params.noThrow_ ) return -1;
    int32_t err = errno;
    newObjectV1C2<Exception>(err,params.name_ + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
  }
  if( pid == 0 ){
    for( uintptr_t i = 0; i < params.env_.count(); i++ ){
#ifndef NDEBUG
      fprintf(stderr,"%s, %d, %s\n",__FILE__,__LINE__,params.env_[i].c_str());
#endif
      utf8::String::Iterator sep(params.env_[i].strstr("="));
      if( sep.eos() ) continue;
      setEnv(utf8::String(params.env_[i],sep),sep + 1);
    }
    if( params.stdio_ != INVALID_HANDLE_VALUE && dup2(STDIN_FILENO,params.stdio_) == - 1 ) exit(errno);
    if( params.stdout_ != INVALID_HANDLE_VALUE && dup2(STDOUT_FILENO,params.stdout_) == - 1 ) exit(errno);
    if( params.stderr_ != INVALID_HANDLE_VALUE && dup2(STDERR_FILENO,params.stderr_) == - 1 ) exit(errno);
    if( execvp(name,argsPtrs) == -1 ) exit(errno);
    exit(0);
  }
  else if( params.wait_ ){
    int status;
    pid_t pid2 = waitpid(pid,&status,0);
    pid = WEXITSTATUS(status);
    if( pid2 == -1 || WEXITSTATUS(status) != 0 ){
      if( params.noThrow_ ) return pid;
      int32_t err = pid2 == -1 ? errno : WEXITSTATUS(status);
      newObjectV1C2<Exception>(err,params.name_ + utf8::String(" ") + __PRETTY_FUNCTION__)->throwSP();
    }    
  }
  return pid;
#endif
}
//---------------------------------------------------------------------------
int32_t waitForProcess(pid_t pid)
{
  Fiber * fiber = currentFiber();
  if( fiber != NULL ){
    fiber->event_.timeout_ = ~uint64_t(0);
    fiber->event_.pid_ = pid;
    fiber->event_.type_ = etWaitForProcess;
    fiber->thread()->postRequest();
    fiber->switchFiber(fiber->mainFiber());
    assert( fiber->event_.type_ == etWaitForProcess );
    if( fiber->event_.errno_ != 0 )
      newObjectV1C2<Exception>(fiber->event_.errno_,__PRETTY_FUNCTION__)->throwSP();
    return (int32_t) fiber->event_.data_;
  }
#if defined(__WIN32__) || defined(__WIN64__)
  int32_t err;
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE,FALSE,pid);
  if( hProcess == NULL ){
    err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  DWORD r = WaitForSingleObject(hProcess,INFINITE);
  if( r == WAIT_FAILED ){
    err = GetLastError() + errorOffset;
    CloseHandle(hProcess);
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  if( r != WAIT_OBJECT_0 ){
    CloseHandle(hProcess);
    newObjectV1C2<Exception>(ERROR_INVALID_DATA,__PRETTY_FUNCTION__)->throwSP();
  }
  DWORD exitCode;
  if( GetExitCodeProcess(hProcess,&exitCode) == 0 ){
    err = GetLastError() + errorOffset;
    CloseHandle(hProcess);
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  CloseHandle(hProcess);
  return exitCode;
#else
  int status;
  pid_t pid2 = waitpid(pid,&status,0);
  if( pid2 == -1 ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return WEXITSTATUS(status);
#endif
}
//---------------------------------------------------------------------------
#if !defined(__WIN32__) && !defined(__WIN64__)
static uint64_t processStartTime;
#endif
//---------------------------------------------------------------------------
int64_t getProcessStartTime(bool toLocalTime)
{
#if defined(__WIN32__) || defined(__WIN64__)
  int32_t err = 0;
  SetLastError(ERROR_SUCCESS);
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,GetCurrentProcessId());
  union {
    FILETIME ft;
    ULARGE_INTEGER sti;
  } creationTime, exitTime, kernelTime, userTime;
  creationTime.sti.QuadPart = 0;
  if( hProcess != NULL )
    if( GetProcessTimes(hProcess,&creationTime.ft,&exitTime.ft,&kernelTime.ft,&userTime.ft) != 0 )
      if( toLocalTime ) FileTimeToLocalFileTime(&creationTime.ft,&creationTime.ft);
  err = GetLastError();
  if( hProcess != NULL ) CloseHandle(hProcess);
  if( err != ERROR_SUCCESS ) newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__)->throwSP();
#ifdef __BORLANDC__
  return (creationTime.sti.QuadPart - 11644473600ui64 * 10000000u) / 10u;
#else
  return (creationTime.sti.QuadPart - UINT64_C(11644473600) * 10000000u) / 10u;
#endif
#else
  return toLocalTime ? processStartTime + getgmtoffset() : processStartTime;
#endif
}
//---------------------------------------------------------------------------
intptr_t strToMonth(const utf8::String & month)
{
  if( month.casecompare("Jan") == 0 ) return 0;
  if( month.casecompare("Feb") == 0 ) return 1;
  if( month.casecompare("Mar") == 0 ) return 2;
  if( month.casecompare("Apr") == 0 ) return 3;
  if( month.casecompare("Mai") == 0 ) return 4;
  if( month.casecompare("May") == 0 ) return 4;
  if( month.casecompare("Jun") == 0 ) return 5;
  if( month.casecompare("Jul") == 0 ) return 6;
  if( month.casecompare("Aug") == 0 ) return 7;
  if( month.casecompare("Sep") == 0 ) return 8;
  if( month.casecompare("Okt") == 0 ) return 9;
  if( month.casecompare("Oct") == 0 ) return 9;
  if( month.casecompare("Nov") == 0 ) return 10;
  if( month.casecompare("Dez") == 0 ) return 11;
  if( month.casecompare("Dec") == 0 ) return 11;
  return -1;
}
//------------------------------------------------------------------------------
utf8::String printTraffic(intmax_t traf,bool zero)
{
  uintmax_t t = traf >= 0 ? traf : -traf, t3;
  intmax_t t1, t2;
  const char * postfix;

  if( t >= uintmax_t(1024u) * 1024u * 1024u * 1024u ){
    t2 = uintmax_t(1024u) * 1024u * 1024u * 1024u;
    postfix = "T";
  }
  else if( t >= 1024u * 1024u * 1024u ){
    t2 = 1024u * 1024u * 1024u;
    postfix = "G";
  }
  else if( t >= 1024u * 1024u ){
    t2 = 1024u * 1024u;
    postfix = "M";
  }
  else if( t >= 1024u ){
    t2 = 1024u;
    postfix = "K";
  }
  else {
    return utf8::String::print(traf != 0 || zero ? "%"PRIdMAX : "-",traf);
  }
  t1 = traf / t2;
  t3 = t % t2;
  return utf8::String::print(traf != 0 || zero ? "%"PRIdMAX".%04"PRIuMAX"%s" : "-",t1,uintmax_t(t3 / (t2 / 1024u)),postfix);
}
//------------------------------------------------------------------------------
utf8::String printTraffic(uintmax_t traf,bool zero)
{
  uintmax_t t1, t2, t3;
  const char * postfix;

  if( traf >= uintmax_t(1024u) * 1024u * 1024u * 1024u ){
    t2 = uintmax_t(1024u) * 1024u * 1024u * 1024u;
    postfix = "T";
  }
  else if( traf >= 1024u * 1024u * 1024u ){
    t2 = 1024u * 1024u * 1024u;
    postfix = "G";
  }
  else if( traf >= 1024u * 1024u ){
    t2 = 1024u * 1024u;
    postfix = "M";
  }
  else if( traf >= 1024u ){
    t2 = 1024u;
    postfix = "K";
  }
  else {
    return utf8::String::print(traf != 0 || zero ? "%"PRIuMAX : "-",traf);
  }
  t1 = traf / t2;
  t3 = traf % t2;
  return utf8::String::print(traf != 0 || zero ? "%"PRIuMAX".%04"PRIuMAX"%s" : "-",t1,uintmax_t(t3 / (t2 / 1024u)),postfix);
}
//------------------------------------------------------------------------------
utf8::String printTraffic(uintmax_t traf,uintmax_t allTraf,bool html)
{
  uintmax_t q = traf * 10000u / allTraf, b = q / 100u, c = q % 100u;
  if( traf == 0 ) return printTraffic(traf);
  return printTraffic(traf) + utf8::String::print(html ? "<FONT SIZE=0>(%"PRIuMAX".%02"PRIuMAX"%%)</FONT>" : "(%"PRIuMAX".%02"PRIuMAX"%%)",b,c);
}
//------------------------------------------------------------------------------
utf8::String printCount(uintmax_t count,uintmax_t allCount,bool html)
{
  uintmax_t q = count * 10000u / allCount, b = q / 100u, c = q % 100u;
  if( count == 0 ) return "-";
  return utf8::int2Str(count) + utf8::String::print(html ? "<FONT SIZE=0>(%"PRIuMAX".%02"PRIuMAX"%%)</FONT>" : "(%"PRIuMAX".%02"PRIuMAX"%%)",b,c);
}
//------------------------------------------------------------------------------
utf8::String getTimestamp(const utf8::String & date,const utf8::String & time)
{
  utf8::String::Iterator i(date);
  return 
    utf8::String(i + 4,i + 4 + 2).replaceAll(" ","0") + "." +
    utf8::int2Str0(strToMonth(utf8::String(i,i + 3)),2) + "." +
    utf8::String(i + 7,i + 7 + 4) + " " +
    time/*.replaceAll(":","")*/
  ;
}
//---------------------------------------------------------------------------
// base64 routines
//---------------------------------------------------------------------------
static const char base64Table[64] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '+', '/'
};
//---------------------------------------------------------------------------
uintptr_t rfcBase64Encode(
  const uint8_t * inStr,
  size_t inLen, 
  uint8_t * outStr,
  size_t outLen)
{
  size_t currOutLen = 0, i = 0;
  while( i < inLen ){
    uint8_t a = inStr[i];
    uint8_t b = uint8_t(i + 1 >= inLen ? 0 : inStr[i + 1]);
    uint8_t c = uint8_t(i + 2 >= inLen ? 0 : inStr[i + 2]);
    if( i + 2 < inLen ){
      if( currOutLen + 4 <= outLen ){
        outStr[currOutLen++] = base64Table[(a >> 2) & 0x3F];
        outStr[currOutLen++] = base64Table[((a << 4) & 0x30) + ((b >> 4) & 0xf)];
        outStr[currOutLen++] = base64Table[((b << 2) & 0x3c) + ((c >> 6) & 0x3)];
        outStr[currOutLen++] = base64Table[c & 0x3F];
      }
      else {
        currOutLen += 4;
      }
    }
    else if( i + 1 < inLen ){
      if( currOutLen + 4 <= outLen ){
        outStr[currOutLen++] = base64Table[(a >> 2) & 0x3F];
        outStr[currOutLen++] = base64Table[((a << 4) & 0x30) + ((b >> 4) & 0xf)];
        outStr[currOutLen++] = base64Table[((b << 2) & 0x3c) + ((c >> 6) & 0x3)];
        outStr[currOutLen++] = '=';
      }
      else {
        currOutLen += 4;
      }
    }
    else {
      if( currOutLen + 4 <= outLen ){
        outStr[currOutLen++] = base64Table[(a >> 2) & 0x3F];
        outStr[currOutLen++] = base64Table[((a << 4) & 0x30) + ((b >> 4) & 0xf)];
        outStr[currOutLen++] = '=';
        outStr[currOutLen++] = '=';
      }
      else {
        currOutLen += 4;
      }
    }
    i += 3;
  }
  if( currOutLen < outLen ) outStr[currOutLen] = '\0';
  return currOutLen;
}
//---------------------------------------------------------------------------
static uint8_t base64DecodeTable[256];
//---------------------------------------------------------------------------
static inline uint8_t base64Encode6(const uint8_t * p,uintptr_t i)
{
  return (uint8_t) (((*(uint16_t *) (p + (i >> 3))) >> (i & 7)) & 63u);
}
//---------------------------------------------------------------------------
utf8::String base64Encode(const void * p,uintptr_t l)
{
  utf8::String s;
  if( l > 0 ){
    uintptr_t i, k;
    s.resize(((l << 3) / 6) + ((l << 3) % 6 != 0));
    char * sp = s.c_str();
    k = (l << 3) - 8;
    for( i = 0; i < k; i += 6 ){
      *sp++ = base64Table[base64Encode6((uint8_t *) p,i)];
    }
    union {
      uint8_t c[2];
      uint32_t c32;
    };
    c32 = *((uint8_t *) p + (i >> 3));
    while( i < k + 8 ){
      *sp++ = base64Table[base64Encode6(c,i & 7)];
      i += 6;
    }
  }
  return s;
}
//---------------------------------------------------------------------------
uintptr_t base64Decode(const utf8::String & s,void * p,uintptr_t size)
{
  uintptr_t i = 0;
  utf8::String::Iterator sp(s);
  size <<= 3;
  while( !sp.eos() ){
    uintptr_t c = sp.getChar();
    if( c < 1 || c >= 256 ) newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    c = base64DecodeTable[c];
    if( c >= 64 ) newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    if( i < size ){
      if( size - i <= 8 ){
        *(uint8_t *) ((uint8_t *) p + (i >> 3)) &= (uint8_t) (~(63u << (i & 7)));
        *(uint8_t *) ((uint8_t *) p + (i >> 3)) |= (uint8_t) (c << (i & 7));
      }
      else {
        *(uint16_t *) ((uint8_t *) p + (i >> 3)) &= (uint16_t) (~(63u << (i & 7)));
        *(uint16_t *) ((uint8_t *) p + (i >> 3)) |= (uint16_t) (c << (i & 7));
      }
    }
    i += 6;
    sp++;
  }
  return i >> 3;
}
//---------------------------------------------------------------------------
// base32 routines
//---------------------------------------------------------------------------
static const char base32Table[32] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
  'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z',
  '0', '1', '2', '3', '4', '5'
};
//---------------------------------------------------------------------------
static uint8_t base32DecodeTable[256];
//---------------------------------------------------------------------------
static inline uint8_t base32Encode5(const uint8_t * p,uintptr_t i)
{
  return (uint8_t) (((*(uint16_t *) (p + (i >> 3))) >> (i & 7)) & 31u);
}
//---------------------------------------------------------------------------
utf8::String base32Encode(const void * p,uintptr_t l)
{
  utf8::String s;
  if( l > 0 ){
    uintptr_t i, k;
    s.resize(((l << 3) / 5) + ((l << 3) % 5 != 0));
    char * sp = s.c_str();
    k = (l << 3) - 8;
    for( i = 0; i < k; i += 5 ){
      *sp++ = base32Table[base32Encode5((uint8_t *) p,i)];
    }
    union {
      uint8_t c[2];
      uint32_t c32;
    };
    c32 = *((uint8_t *) p + (i >> 3));
    while( i < k + 8 ){
      *sp++ = base32Table[base32Encode5(c,i & 7)];
      i += 5;
    }
  }
  return s;
}
//---------------------------------------------------------------------------
uintptr_t base32Decode(const utf8::String & s,void * p,uintptr_t size)
{
  uintptr_t i = 0;
  utf8::String::Iterator sp(s);
  size <<= 3;
  while( !sp.eos() ){
    uintptr_t c = sp.getChar();
    if( c < 1 || c >= 256 ) newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    c = base32DecodeTable[c];
    if( c >= 32 ) newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    if( i < size ){
      if( size - i <= 8 ){
        *(uint8_t *) ((uint8_t *) p + (i >> 3)) &= (uint8_t) (~(31u << (i & 7)));
        *(uint8_t *) ((uint8_t *) p + (i >> 3)) |= (uint8_t) (c << (i & 7));
      }
      else {
        *(uint16_t *) ((uint8_t *) p + (i >> 3)) &= (uint16_t) (~(31u << (i & 7)));
        *(uint16_t *) ((uint8_t *) p + (i >> 3)) |= (uint16_t) (c << (i & 7));
      }
    }
    i += 5;
    sp++;
  }
  return i >> 3;
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
pid_t getpid()
{
  return GetCurrentProcessId();
}
//---------------------------------------------------------------------------
uid_t getuid()
{
/*!
 * \todo rewrite for windows
 */
  return 0;
}
//---------------------------------------------------------------------------
gid_t getgid()
{
/*!
 * \todo rewrite for windows
 */
  return 0;
}
//---------------------------------------------------------------------------
#else
//---------------------------------------------------------------------------
pid_t getpid()
{
  return ::getpid();
}
//---------------------------------------------------------------------------
uid_t getuid()
{
  return ::getuid();
}
//---------------------------------------------------------------------------
gid_t getgid()
{
  return ::getgid();
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
bool isWow64()
{
  typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
  LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
    GetModuleHandle("kernel32"),
    "IsWow64Process"
  );
  BOOL bIsWow64 = FALSE;
  if( fnIsWow64Process != NULL && fnIsWow64Process(GetCurrentProcess(),&bIsWow64) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return bIsWow64 == FALSE ? false : true;
}
#endif
//---------------------------------------------------------------------------
#ifndef DOXYGEN_SHOULD_SKIP_THIS
//---------------------------------------------------------------------------
utf8::String getMachineUniqueKey()
{
#if PRIVATE_RELEASE
#if (defined(__WIN32__) || defined(__WIN64__)) && !defined(__MINGW32__)
//#pragma comment(lib,"wbemuuid.lib")
  utf8::String s;

  VARIANT vtName;
  VariantInit(&vtName);
  VARIANT vtDeviceId;
  VariantInit(&vtDeviceId);
  VARIANT vtDescription;
  VariantInit(&vtDescription);
  VARIANT vtProcessorId;
  VariantInit(&vtProcessorId);
  VARIANT vtMACAddress;
  VariantInit(&vtMACAddress);
  VARIANT vtAdapterTypeId;
  VariantInit(&vtAdapterTypeId);
  HRESULT CoInit_hres;
  try {
    HRESULT hres;
    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------
    CoInit_hres = CoInitializeEx(0,COINIT_MULTITHREADED);
    if( FAILED(CoInit_hres) )
      newObjectV1C2<Exception>(HRESULT_CODE(CoInit_hres) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
    try {
      // Step 2: --------------------------------------------------
      // Set general COM security levels --------------------------
      // Note: If you are using Windows 2000, you need to specify -
      // the default authentication credentials for a user by using
      // a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
      // parameter of CoInitializeSecurity ------------------------
      if( CoInit_hres == S_OK ){
        hres = CoInitializeSecurity(
          NULL, 
          -1,                          // COM authentication
          NULL,                        // Authentication services
          NULL,                        // Reserved
          RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
          RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
          NULL,                        // Authentication info
          EOAC_NONE,                   // Additional capabilities 
          NULL                         // Reserved
        );
        if( FAILED(hres) && hres != RPC_E_TOO_LATE )
          newObjectV1C2<Exception>(HRESULT_CODE(hres) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
      }
      // Step 3: ---------------------------------------------------
      // Obtain the initial locator to WMI -------------------------
      IWbemLocator *pLoc = NULL;
      hres = CoCreateInstance(
        CLSID_WbemLocator,             
        0, 
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator, (LPVOID *) &pLoc
      );
      if( FAILED(hres) )
        newObjectV1C2<Exception>(HRESULT_CODE(hres) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
      try {
        // Step 4: -----------------------------------------------------
        // Connect to WMI through the IWbemLocator::ConnectServer method
        IWbemServices *pSvc = NULL;
        // Connect to the root\cimv2 namespace with
        // the current user and obtain pointer pSvc
        // to make IWbemServices calls.
        hres = pLoc->ConnectServer(
          L"ROOT\\CIMV2", // Object path of WMI namespace
          NULL,                    // User name. NULL = current user
          NULL,                    // User password. NULL = current
          0,                       // Locale. NULL indicates current
          NULL,                    // Security flags.
          0,                       // Authority (e.g. Kerberos)
          0,                       // Context object 
          &pSvc                    // pointer to IWbemServices proxy
        );
        if( FAILED(hres) )
          newObjectV1C2<Exception>(HRESULT_CODE(hres) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
        try {
          // Step 5: --------------------------------------------------
          // Set security levels on the proxy -------------------------
          hres = CoSetProxyBlanket(
             pSvc,                        // Indicates the proxy to set
             RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
             RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
             NULL,                        // Server principal name 
             RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
             RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
             NULL,                        // client identity
             EOAC_NONE                    // proxy capabilities 
          );
          if( FAILED(hres) )
            newObjectV1C2<Exception>(HRESULT_CODE(hres) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
          // Step 6: --------------------------------------------------
          // Use the IWbemServices pointer to make requests of WMI ----
          IEnumWbemClassObject * pEnumerator = NULL;
          hres = pSvc->ExecQuery(
            SysAllocString(L"WQL"),
            SysAllocString(L"SELECT * FROM Win32_Processor"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
            NULL,
            &pEnumerator
          );
          if( FAILED(hres) )
            newObjectV1C2<Exception>(HRESULT_CODE(hres) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
          try {
            // Step 7: -------------------------------------------------
            // Get the data from the query in step 6 -------------------
            IWbemClassObject * pclsObj;
            ULONG uReturn = 0;
            while( pEnumerator != NULL ){
              HRESULT hr = pEnumerator->Next(int(WBEM_INFINITE),1,&pclsObj,&uReturn);
              if( uReturn == 0 ) break;
              try {
                hr = pclsObj->Get(L"Name", 0, &vtName, 0, 0);
                if( FAILED(hr) )
                  newObjectV1C2<Exception>(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
                if( V_VT(&vtName) != VT_BSTR ){
                  hr = VariantChangeTypeEx(&vtName,&vtName,0,0,VT_BSTR);
                  newObjectV1C2<Exception>(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
                }
                hr = pclsObj->Get(L"DeviceId", 0, &vtDeviceId, 0, 0);
                if( FAILED(hr) )
                  newObjectV1C2<Exception>(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
                if( V_VT(&vtDeviceId) != VT_BSTR ){
                  hr = VariantChangeTypeEx(&vtDeviceId,&vtDeviceId,0,0,VT_BSTR);
                  newObjectV1C2<Exception>(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
                }
                hr = pclsObj->Get(L"Description", 0, &vtDescription, 0, 0);
                if( FAILED(hr) )
                  newObjectV1C2<Exception>(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
                if( V_VT(&vtDescription) != VT_BSTR ){
                  hr = VariantChangeTypeEx(&vtDescription,&vtDescription,0,0,VT_BSTR);
                  newObjectV1C2<Exception>(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
                }
                hr = pclsObj->Get(L"ProcessorId", 0, &vtProcessorId, 0, 0);
                if( FAILED(hr) )
                  newObjectV1C2<Exception>(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
                if( V_VT(&vtProcessorId) != VT_BSTR ){
                  hr = VariantChangeTypeEx(&vtProcessorId,&vtProcessorId,0,0,VT_BSTR);
                  newObjectV1C2<Exception>(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
                }
                if( !s.isNull() ) s += "\n";
                s += utf8::String(V_BSTR(&vtDeviceId)) + " " +
                  V_BSTR(&vtName) + " " +
                  V_BSTR(&vtDescription) + " " +
                  V_BSTR(&vtProcessorId)
                ;
              }
              catch( ... ){
                pclsObj->Release();
                throw;
              }
              pclsObj->Release();
            }
          }
          catch( ... ){
            pEnumerator->Release();
            throw;
          }
          pEnumerator->Release();

          hres = pSvc->ExecQuery(
            SysAllocString(L"WQL"),
            SysAllocString(L"SELECT * FROM Win32_NetworkAdapter"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
            NULL,
            &pEnumerator
          );
          if( FAILED(hres) )
            newObjectV1C2<Exception>(HRESULT_CODE(hres) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
          try {
            // Step 7: -------------------------------------------------
            // Get the data from the query in step 6 -------------------
            IWbemClassObject * pclsObj;
            ULONG uReturn = 0;
            while( pEnumerator != NULL ){
              HRESULT hr = pEnumerator->Next(int(WBEM_INFINITE),1,&pclsObj,&uReturn);
              if( uReturn == 0 ) break;
              try {
                hr = pclsObj->Get(L"Name", 0, &vtName, 0, 0);
                if( FAILED(hr) )
                  newObjectV1C2<Exception>(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
                if( V_VT(&vtName) != VT_BSTR ){
                  hr = VariantChangeTypeEx(&vtName,&vtName,0,0,VT_BSTR);
                  newObjectV1C2<Exception>(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
                }
                hr = pclsObj->Get(L"AdapterTypeId", 0, &vtAdapterTypeId, 0, 0);
                if( FAILED(hr) ){
                  HRESULT hr2 = pclsObj->Get(L"AdapterType", 0, &vtAdapterTypeId, 0, 0);
                  if( FAILED(hr2) ){
                    hr = hr2;
                    newObjectV1C2<Exception>(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
                  }
                  if( V_VT(&vtAdapterTypeId) != VT_NULL ){
                    if( V_VT(&vtAdapterTypeId) != VT_BSTR ){
                      hr = VariantChangeTypeEx(&vtAdapterTypeId,&vtAdapterTypeId,0,0,VT_BSTR);
                      newObjectV1C2<Exception>(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
                    }
                  }
                }
                if( V_VT(&vtAdapterTypeId) != VT_NULL ){
                  if( V_VT(&vtAdapterTypeId) == VT_BSTR ){
                    if( utf8::String("Ethernet").ncasecompare(V_BSTR(&vtAdapterTypeId),8) == 0 ){
                      VariantClear(&vtAdapterTypeId);
                      V_VT(&vtAdapterTypeId) = VT_I4;
                      V_I4(&vtAdapterTypeId) = 0;
                    }
                    else {
                      VariantClear(&vtAdapterTypeId);
                      V_VT(&vtAdapterTypeId) = VT_I4;
                      V_I4(&vtAdapterTypeId) = 3;
                    }
                  }
                  if( V_VT(&vtAdapterTypeId) != VT_I4 ){
                    hr = VariantChangeTypeEx(&vtAdapterTypeId,&vtAdapterTypeId,0,0,VT_I4);
                    newObjectV1C2<Exception>(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
                  }
                  if( V_I4(&vtAdapterTypeId) >= 0 && V_I4(&vtAdapterTypeId) != 3 ){
                    hr = pclsObj->Get(L"MACAddress", 0, &vtMACAddress, 0, 0);
                    if( FAILED(hr) )
                      newObjectV1C2<Exception>(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
                    if( V_VT(&vtMACAddress) != VT_NULL ){
                      if( V_VT(&vtMACAddress) != VT_BSTR ){
                        hr = VariantChangeTypeEx(&vtMACAddress,&vtMACAddress,0,0,VT_BSTR);
                        newObjectV1C2<Exception>(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
                      }
                      if( !s.isNull() ) s += "\n";
                      s += utf8::String(V_BSTR(&vtName)) + " " + V_BSTR(&vtMACAddress);
                    }
                  }
                }
              }
              catch( ... ){
                pclsObj->Release();
                throw;
              }
              pclsObj->Release();
            }
          }
          catch( ... ){
            pEnumerator->Release();
            throw;
          }
          pEnumerator->Release();
        }
        catch( ... ){
          pSvc->Release();
          throw;
        }
        pSvc->Release();
      }
      catch( ... ){
        pLoc->Release();
        throw;
      }
      pLoc->Release();
    }
    catch( ... ){
      if( CoInit_hres == S_OK ) CoUninitialize();
      throw;
    }
    if( CoInit_hres == S_OK ) CoUninitialize();
  }
  catch( ... ){
    VariantClear(&vtProcessorId);
    VariantClear(&vtDescription);
    VariantClear(&vtDeviceId);
    VariantClear(&vtName);
    VariantClear(&vtMACAddress);
    throw;
  }
  VariantClear(&vtProcessorId);
  VariantClear(&vtDescription);
  VariantClear(&vtDeviceId);
  VariantClear(&vtName);
  VariantClear(&vtMACAddress);
  intptr_t size = s.size(), i;
  union {
    uint8_t * a;
    uint16_t * b;
    uint32_t * c;
  };
  a = (uint8_t *) s.c_str();
  for( i = size - 1; i >= 0; i-- )
    xchg(a[a[i] % size],a[a[size - i - 1] % size]);
  b = (uint16_t *) s.c_str();
  for( size /= 2, i = size - 1; i >= 0; i-- )
    xchg(b[b[i] % size],b[b[size - i - 1] % size]);
  c = (uint32_t *) s.c_str();
  for( size /= 2, i = size - 1; i >= 0; i-- )
    xchg(c[c[i] % size],c[c[size - i - 1] % size]);
  return s;
#elif __MINGW32__
  return utf8::String();
#elif (HAVE_SYS_IOCTL_H && HAVE_SYS_SYSCTL_H && HAVE_NET_IF_H && HAVE_NET_IF_TYPES_H) || HAVE_DO_CPUID
  utf8::String key;
#if HAVE_DO_CPUID
  u_int regs[4];
  do_cpuid(1,regs);
  u_int cpu_id = regs[0]; // eax
// regs[2]; // ecx
  u_int cpu_feature = regs[3]; // edx
  if( cpu_feature & CPUID_PSN ){
    do_cpuid(3,regs);
    char cpuSerial[32];
    snprintf(cpuSerial,
      sizeof(cpuSerial),
      "%04x-%04x-%04x-%04x-%04x-%04x",
      (cpu_id >> 16), (cpu_id & 0xffff),
      (regs[3] >> 16), (regs[3] & 0xffff),
      (regs[2] >> 16), (regs[2] & 0xffff)
    );
    key += cpuSerial;
  }
#endif
#if HAVE_SYS_IOCTL_H && HAVE_SYS_SYSCTL_H && HAVE_NET_IF_H && HAVE_NET_IF_TYPES_H
  int mib[6];
  mib[0] = CTL_NET;
  mib[1] = PF_ROUTE;
  mib[2] = 0;
  mib[3] = 0;                     /* address family */
  mib[4] = NET_RT_IFLIST;
  mib[5] = 0;
  size_t needed;
  AutoPtrBuffer buf;

  for(;;){
    if( sysctl(mib,6,NULL,&needed,NULL,0) != 0 ){
      int32_t err = errno;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    buf.realloc(needed);
    if( sysctl(mib,6,buf,&needed,NULL,0) == 0 ) break;
    if( errno != ENOMEM ){
      int32_t err = errno;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    // Routing table grew, retrying
  }
  struct ifreq ifr;
  struct if_msghdr * ifm, * ifm2 = NULL, * nextifm;
  struct ifa_msghdr * ifam;
  struct sockaddr_dl * sdl = NULL, * sdl2 = NULL;
  const uint8_t * next = buf, * lim = next + needed;
  while( next < lim ){
    ifm = (struct if_msghdr *) next;
    if( ifm->ifm_type == RTM_IFINFO ){
      if( ifm->ifm_data.ifi_datalen == 0 )
        ifm->ifm_data.ifi_datalen = sizeof(struct if_data);
      sdl = (struct sockaddr_dl *)((char *)ifm + sizeof(struct if_msghdr) - sizeof(struct if_data) + ifm->ifm_data.ifi_datalen);
    }
//    else {
//      newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__ + utf8::String(" out of sync parsing NET_RT_IFLIST"))->throwSP();
//    }
//      if( ifm->ifm_flags & IFF_UP );
//      if( sdl->sdl_type == IFT_ETHER );
    if( ifm->ifm_type == RTM_IFINFO && (ifm->ifm_flags & IFF_LOOPBACK) == 0 && sdl->sdl_type == IFT_ETHER ){
      if( ifm2 == NULL ) ifm2 = ifm;
      if( sdl2 == NULL ) sdl2 = sdl;
      char name[IFNAMSIZ];
      memcpy(name,sdl->sdl_data,sizeof(name) < sdl->sdl_nlen ? sizeof(name)-1 : sdl->sdl_nlen);
      name[sizeof(name) < sdl->sdl_nlen ? sizeof(name)-1 : sdl->sdl_nlen] = '\0';
      ifr.ifr_addr.sa_family = AF_INET;
      strncpy(ifr.ifr_name,name,sizeof(ifr.ifr_name));
      ksock::APIAutoInitializer ksockAPIAutoInitializer;
      int s = socket(ifr.ifr_addr.sa_family,SOCK_DGRAM,0);
      if( s == INVALID_SOCKET ){
        int32_t err = errno;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
      if( ioctl(s,SIOCGIFCAP,(caddr_t) &ifr) != 0 ){
        int32_t err = errno;
        close(s);
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
      try {
        struct rt_addrinfo info;
        info.rti_info[RTAX_IFA] = (struct sockaddr *) sdl;
        if( sdl != NULL && sdl->sdl_alen > 0 ){
          if( sdl->sdl_type == IFT_ETHER && sdl->sdl_alen == ETHER_ADDR_LEN ){
#if HAVE_NET_ETHERNET_H
#ifndef NDEBUG
            fprintf(stderr,"name %s, ether %s\n",name,ether_ntoa((const struct ether_addr *)LLADDR(sdl)));
#endif
  	    key += ether_ntoa((const struct ether_addr *)LLADDR(sdl));
#else
#endif
          }
          else {
            int n = sdl->sdl_nlen > 0 ? sdl->sdl_nlen + 1 : 0;
#if HAVE_NET_IF_DL_H
#ifndef NDEBUG
            fprintf(stderr,"name %s, lladdr %s\n",name,link_ntoa(sdl) + n);
#endif
	    key += link_ntoa(sdl) + n;
#else
#endif
          }
        }
      }
      catch( ExceptionSP & ){
        close(s);
        throw;
      }
      close(s);
    }
    next += ifm->ifm_msglen;
    ifam = NULL;
    while( next < lim ){
      nextifm = (struct if_msghdr *) next;
      if( nextifm->ifm_type != RTM_NEWADDR ) break;
      if( ifam == NULL ) ifam = (struct ifa_msghdr *) nextifm;
      next += nextifm->ifm_msglen;
    }
  }
#endif
#ifndef NDEBUG
  fprintf(stderr,"machine key: %s\n",(const char *) key.getOEMString());
#endif
  return key;
#else
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
  return utf8::String();
#endif
#else
  return utf8::String();
#endif
}
//---------------------------------------------------------------------------
utf8::String getMachineCleanUniqueKey()
{
  utf8::String key(getMachineUniqueKey());
  return base64Encode(key.c_str(),key.size());
}
//---------------------------------------------------------------------------
static uint8_t machineCryptKey[sizeof(SHA256_CTX) + 1];
//---------------------------------------------------------------------------
utf8::String getMachineCryptedUniqueKey(const utf8::String & text,const utf8::String & info,SHA256Cryptor * decryptor)
{
  SHA256Cryptor crypt;
  SHA256Cryptor & cryptor = decryptor != NULL ? *decryptor : crypt;
  cryptor.threshold(~(uintptr_t) 0);
  cryptor.init(text.c_str(),text.size());
  if( machineCryptKey[sizeof(SHA256_CTX)] == 0 ){
    memcpy(machineCryptKey,cryptor.sha256(),cryptor.size());
    machineCryptKey[sizeof(SHA256_CTX)] = 1;
  }
  SHA256 sha;
  sha.make(cryptor.sha256(),cryptor.size());
  utf8::String key(base64Encode(sha.sha256(),sha.size()));
  AutoPtrBuffer info2;
  uintptr_t size = info.size();
  info2.alloc(size + sizeof(uint32_t) + sizeof(uint32_t));
  uint32_t checksum = crc32(0,NULL,0);
  checksum = crc32(checksum,info.c_str(),size);
  *(uint32_t *)(info2.ptr() + size) = checksum;
  cryptor.crypt(info2,info.c_str(),size);
  cryptor.crypt(info2.ptr() + size,sizeof(uint32_t));
  checksum = crc32(0,NULL,0);
  checksum = crc32(checksum,info2,size + sizeof(uint32_t));
  *(uint32_t *)(info2.ptr() + size + sizeof(uint32_t)) = checksum;
  cryptor.init(text.c_str(),text.size());
  return key + base64Encode(info2,size + sizeof(uint32_t) + sizeof(uint32_t));
}
//---------------------------------------------------------------------------
#if PRIVATE_RELEASE
//---------------------------------------------------------------------------
bool checkMachineBinding(const utf8::String & key,bool abortProgram)
{
  struct tm lt = time2tm(getlocaltimeofday());
  if( lt.tm_year < 2009 - 1900 || (lt.tm_year == 2009 - 1900 && lt.tm_mon < 7) ) return false;
  bool pirate = true, mkey = true, expire = true;
  try {
    SHA256Cryptor decryptor;
    if( machineUniqueCryptedKey().isNull() ){
      machineUniqueCryptedKey() = getMachineCryptedUniqueKey(getMachineCleanUniqueKey(),utf8::String(),&decryptor);
    }
    else {
      memcpy(decryptor.sha256(),machineCryptKey,decryptor.size());
    }
    pirate = mkey = machineUniqueCryptedKey().left(43).compare(key.left(43)) != 0;
    if( key.length() > 43 ){ // check expiration date
      utf8::String info(key.right(key.size() - 43));
      AutoPtrBuffer info1;
      uintptr_t size, size2;
      info1.alloc(size = base64Decode(info,NULL,0));
      if( size > sizeof(uint32_t) * 2 ){
        base64Decode(info,info1,size);
        size2 = size - sizeof(uint32_t) - sizeof(uint32_t);
        uint32_t checksum = crc32(0,NULL,0);
        checksum = crc32(checksum,info1,size - sizeof(uint32_t));
        pirate = *(uint32_t *)(info1.ptr() + size - sizeof(uint32_t)) != checksum || pirate;
        utf8::String info2;
        info2.resize(size2);
        decryptor.crypt(info1,size - sizeof(uint32_t));
        memcpy(info2.c_str(),info1,size2);
        checksum = crc32(0,NULL,0);
        checksum = crc32(checksum,info1,size2);
        pirate = *(uint32_t *)(info1.ptr() + size2) != checksum || pirate;
// now this is expiration date text from keymaker command line as plain text in info2
        uint64_t ld = gettimeofday();
        uint64_t ed = timeFromTimeString(info2,false);
        expire = ed > 0 && ld > ed;
        pirate = expire && pirate;
      }
      else {
        pirate = true;
      }
    }
    else {
      pirate = true;
    }
  }
  catch( ... ){
    pirate = true;
  }
  if( pirate ){
    if( abortProgram ){
      if( !mkey && expire ){
//        stdErr.debug(9,utf8::String::Stream() << "Expiration date reached. Aborted...\n");
        stdErr.debug(9,utf8::String::Stream() << "nonsystem error 1253. Aborted...\n");
      }
      else {
//        stdErr.debug(9,utf8::String::Stream() << "Pirate copy detected. Aborted...\n");
        stdErr.debug(9,utf8::String::Stream() << "nonsystem error 4298. Aborted...\n");
      }
      exit(EINVAL);
    }
//    newObjectV1C2<Exception>(EINVAL,"Pirate copy detected")->throwSP();
    newObjectV1C2<Exception>(EINVAL,"nonsystem error 1726")->throwSP();
  }
  return pirate;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
uint8_t machineUniqueCryptedKeyHolder[sizeof(utf8::String)];
//---------------------------------------------------------------------------
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
//---------------------------------------------------------------------------
void readProtectedMemory(const void * memory,void * data,uintptr_t count)
{
#if defined(__WIN32__) || defined(__WIN64__)
  SIZE_T rb;
  BOOL r = ReadProcessMemory(
    GetCurrentProcess(),
    memory,
    data,
    count,
    &rb
  );
  if( r == 0 || rb != count ){
    int32_t err = GetLastError();
    if( rb != count ) err = ERROR_INVALID_DATA;
    newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  }
#else
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
#endif
}
//---------------------------------------------------------------------------
void writeProtectedMemory(void * memory,const void * data,uintptr_t count)
{
#if defined(__WIN32__) || defined(__WIN64__)
  DWORD old;
  BOOL r = VirtualProtect(
    memory,
    count,
    PAGE_EXECUTE_READWRITE,
    &old
  );
  if( r == 0 ){
    int32_t err = GetLastError();
    newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  }
  SIZE_T w;
  r = WriteProcessMemory(
    GetCurrentProcess(),
    memory,
    data,
    count,
    &w
  );
  if( r == 0 || w != count ){
    int32_t err = GetLastError();
    VirtualProtect(memory,count,old,&old);
    if( w != count ) err = ERROR_INVALID_DATA;
    newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  }
  VirtualProtect(memory,count,old,&old);
#else
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
#endif
}
//---------------------------------------------------------------------------
void * findProcImportedEntryAddress(const utf8::String & dllName,const utf8::String & importedDllName,const utf8::String & funcName,bool noThrow)
{
#if (defined(__WIN32__) || defined(__WIN64__)) && !defined(__MINGW32__)
  union {
    HMODULE hModule;
    uintptr_t basePointer;
  };
  if( isWin9x() ){
    hModule = GetModuleHandleA(dllName.getANSIString());
  }
  else {
    hModule = GetModuleHandleW(dllName.getUNICODEString());
  }
  if( hModule == NULL ){
    if( noThrow ) return NULL;
    int32_t err = GetLastError();
    newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  }
  ULONG size;
  PVOID importTableOffset = ImageDirectoryEntryToData(hModule,TRUE,IMAGE_DIRECTORY_ENTRY_IMPORT,&size);
  if( importTableOffset == NULL ){
    if( noThrow ) return NULL;
    int32_t err = GetLastError();
    newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  }
  uintptr_t rva = uintptr_t(importTableOffset) - basePointer;
  for(;;){
    uintptr_t dllNameOffset = *(uintptr_t *)(rva + 12 + basePointer);
    if( dllNameOffset == 0 ){
      if( noThrow ) return NULL;
      newObjectV1C2<Exception>(ERROR_INVALID_DATA + errorOffset,__PRETTY_FUNCTION__)->throwSP();
    }
    if( utf8::String((const char *)(basePointer + dllNameOffset)).casecompare(importedDllName) != 0 ){
      rva += 20;
      continue;
    }
    uintptr_t funcNamesOffset = *(uintptr_t *)(rva + basePointer), nFuncName = 0;
    for(;;){
      uintptr_t funcNameOffset = *(uintptr_t *)(funcNamesOffset + basePointer);
      if( funcNameOffset == 0 ){
        if( noThrow ) return NULL;
        newObjectV1C2<Exception>(ERROR_INVALID_DATA + errorOffset,__PRETTY_FUNCTION__)->throwSP();
      }
      nFuncName++;
      utf8::String name((const char *)(basePointer + funcNameOffset) + 2);
      if( name.compare(funcName) != 0 ){
        funcNamesOffset += sizeof(uintptr_t);
        nFuncName++;
        continue;
      }
      uintptr_t funcAddrsOffset = *(uintptr_t *)(rva + 16 + basePointer), nFuncAddr = 0;
      for(;;){
        uintptr_t funcAddrOffset = *(uintptr_t *)(funcAddrsOffset + basePointer);
        if( funcAddrOffset == 0 ){
          if( noThrow ) return NULL;
          newObjectV1C2<Exception>(ERROR_INVALID_DATA + errorOffset,__PRETTY_FUNCTION__)->throwSP();
        }
        nFuncAddr++;
        if( nFuncAddr != nFuncName ){
          funcAddrsOffset += sizeof(uintptr_t);
          nFuncAddr++;
          continue;
        }
        return (void *) (funcAddrsOffset + basePointer);
      }
    }
  }
#else
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
  return NULL;
#endif
}
//---------------------------------------------------------------------------
bool isDaemonCommandLineOption()
{
  utf8::String opt("--daemon");
  for( uintptr_t i = 1; i < argv().count(); i++ )
    if( argv()[i].compare(opt) == 0 ) return true;
  return false;
}
//---------------------------------------------------------------------------
void daemonize()
{
#if !defined(__WIN32__) && !defined(__WIN64__)
  struct sigaction osa, sa;
  pid_t newgrp;
  int oerrno;
  int osa_ok;
// A SIGHUP may be thrown when the parent exits below.
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  osa_ok = sigaction(SIGHUP,&sa,&osa);
  if( osa_ok != 0 ){
    perror(NULL);
    abort();
  }
  switch( fork() ){
    case -1 :
      perror(NULL);
      abort();
    case 0 :
      break;
    default :
      exit(0);
  }																					
  newgrp = setsid();
  oerrno = errno;
  sigaction(SIGHUP,&osa,NULL);
  if( newgrp == -1 ){
    errno = oerrno;
    perror(NULL);
    abort();
  }
#endif
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// System initialization and finalization ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
void initializeArguments(int argc,char ** argv)
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( isWin9x() ){
#endif
    ksys::argv().resize(argc);
    while( --argc >= 0 ) ksys::argv()[argc] = argv[argc];
#if defined(__WIN32__) || defined(__WIN64__)
  }
  else {
    LPWSTR * wStr = CommandLineToArgvW(GetCommandLineW(),&argc);
    try {
      ksys::argv().resize(argc);
      if( wStr == NULL ){
        while( --argc >= 0 ) ksys::argv()[argc] = argv[argc];
      }
      else {
        for( int i = 0; i < argc; i++ ) ksys::argv()[i] = wStr[i];
        LocalFree(wStr);
      }
    }
    catch( ... ){}
  }
#endif
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
static BOOL WINAPI consoleCtrlHandler(DWORD dwCtrlType)
{
  switch( dwCtrlType ){
    default                  :
    case CTRL_C_EVENT        :
    case CTRL_BREAK_EVENT    :
    case CTRL_CLOSE_EVENT    :
    case CTRL_LOGOFF_EVENT   :
    case CTRL_SHUTDOWN_EVENT :
      while( PostThreadMessage(mainThreadId,WM_QUIT,0,0) == 0 ) ksleep1();
  }
  return TRUE;
}
#elif HAVE_SIGNAL_H
static uint8_t signalsCountersSemPlaceHolder[sizeof(Semaphore)];
static inline Semaphore & signalsCountersSem()
{
  return *reinterpret_cast<Semaphore *>(signalsCountersSemPlaceHolder);
}

void waitForSignalsSemaphore()
{
  signalsCountersSem().wait();
}

volatile uilock_t signalsCounters[_SIG_MAXSIG];
static void sigHandler(int sig,siginfo_t * /*siginfo*/,ucontext_t * /*uap*/)
{
  pthread_t tid = pthread_self();
  if( pthread_equal(mainThread,tid) || sig != SIGINT ){
    interlockedIncrement(signalsCounters[sig - 1],1);
    signalsCountersSem().post();
  }
}

static void sigSYSHandler(int sig,siginfo_t * /*siginfo*/,ucontext_t * /*uap*/)
{
}
#endif
//---------------------------------------------------------------------------
void initialize(int argc,char ** argv)
{
// runtime checks for system or compiler incompatibilities
  volatile size_t sz;
  sz = sizeof(char);
  assert( sz == 1 );
  if( sz != 1 ){
    fprintf(stderr,"sizeof(char) != 1\n");
    abort();
  }
  sz = false;
  assert( sz == 0 );
  if( sz != 0 ){
    fprintf(stderr,"(unsigned int) false != 0\n");
    abort();
  }
  sz = !1;
  assert( sz == 0 );
  if( sz != 0 ){
    fprintf(stderr,"(unsigned int) !1 != 0\n");
    abort();
  }
  sz = !0;
  assert( sz == 1 );
  if( sz != 1 ){
    fprintf(stderr,"(unsigned int) !0 != 1\n");
    abort();
  }
#if HAVE__TZSET
  _tzset();
#elif HAVE_TZSET
  tzset();
#endif
#if defined(__WIN32__) || defined(__WIN64__)
// force the system to create the message queue for main thread (if not already created)
  MSG msg;
  SetLastError(0);
  PeekMessage(&msg,NULL,WM_USER,WM_USER,PM_NOREMOVE);
  if( GetLastError() != 0 ){
    perror(NULL);
    abort();
  }
  mainThreadId = GetCurrentThreadId();
  threadFinishMessage = RegisterWindowMessageA("F07E92E6-81C9-441D-98D6-4D9802D81956");
  fiberFinishMessage = RegisterWindowMessageA("A08C6FDA-1417-43C5-A17D-D146500B5886");
  SetProcessShutdownParameters(0x400,SHUTDOWN_NORETRY);
  SetConsoleCtrlHandler(consoleCtrlHandler,TRUE);
#else
  if( processStartTime == 0 ) processStartTime = gettimeofday();
  mainThread = pthread_self();
#if HAVE_SIGNAL_H
  new (signalsCountersSemPlaceHolder) Semaphore;
  struct sigaction act;
  memset(&act,0,sizeof(act));
  act.sa_flags = SA_SIGINFO;
  act.sa_handler = (void (*)(int)) sigSYSHandler;
  sigset_t ss;
  if( sigemptyset(&ss) != 0 ||
      sigaddset(&ss,SIGSYS) != 0 ||
      sigprocmask(SIG_BLOCK,&ss,NULL) != 0 ||
      sigaction(SIGSYS,&act,NULL) != 0 ){
    perror(NULL);
    abort();
  }
  memset(&act,0,sizeof(act));
  act.sa_flags = SA_SIGINFO;
  act.sa_handler = (void (*)(int)) sigHandler;
  if( sigaction(SIGHUP,&act,NULL) != 0 ||
      sigaction(SIGINT,&act,NULL) != 0 ||
      sigaction(SIGQUIT,&act,NULL) != 0 ||
      sigaction(SIGTERM,&act,NULL) != 0 ||
      sigaction(SIGXCPU,&act,NULL) != 0 ||
      sigaction(SIGXFSZ,&act,NULL) != 0 ){
    perror(NULL);
    abort();
  }
#endif
#endif
  Object::initialize();
  LZO1X::initialize();
//---------------------------------------------------------------------------
  {
    intptr_t i;
    memset(base64DecodeTable,~0u,sizeof(base64DecodeTable));
    for( i = sizeof(base64Table) / sizeof(base64Table[0]) - 1; i >= 0; i-- )
      base64DecodeTable[uintptr_t(base64Table[i])] = (uint8_t) i;
    memset(base32DecodeTable,~0u,sizeof(base32DecodeTable));
    for( i = sizeof(base32Table) / sizeof(base32Table[0]) - 1; i >= 0; i-- )
      base32DecodeTable[uintptr_t(base32Table[i])] = (uint8_t) i;
  }
//---------------------------------------------------------------------------
  LiteWriteLock::initialize();
  MemoryManager::initialize();
  Thread::initialize();
  MemoryStream::initialize();
  Mutant::initialize();
  utf8::String::initialize();
  new (argvPlaceHolder) Array<utf8::String>;
  initializeArguments(argc,argv);
  strErrorInitialize();
  Fiber::initialize();
  ksock::api.initialize();
  ksock::SockAddr::initialize();
  BaseThread::initialize();
  AsyncFile::initialize();
  LogFile::initialize();
  Config::initialize();
#ifdef NETMAIL_ENABLE_PROFILER
  TProfiler::initialize();
#endif
  try {
    utf8::String cwd(getCurrentDir());
    utf8::String::Iterator i(cwd);
    while( !i.eos() ){
      uintptr_t c = i.getChar();
      if( c == '\\' || c == '/' ){
        pathDelimiter = char(c);
        pathDelimiterStr[0] = char(c);
        pathDelimiterStr[1] = '\0';
        break;
      }
      i.next();
    }
  }
  catch( ... ){
  }
  new (machineUniqueCryptedKeyHolder) utf8::String();
#if HAVE_CLOCK_GETRES
//  struct timespec ts;
//  clock_getres(CLOCK_REALTIME,&ts);
//  fprintf(stderr,"timer resolution: %ld %ld\n",ts.tv_sec,ts.tv_nsec);
//  uint64_t t2 = gettimeofday(), t3 = 0;
//  while( gettimeofday() - t2 < 2000000u ){
//    clock_gettime(CLOCK_REALTIME,&ts);
//    if( t3 < (uint64_t) ts.tv_nsec ) t3 = ts.tv_nsec;
//  }
//  fprintf(stderr,"max tv_nsec: %"PRIu64"\n",t3);
//  Semaphore sem;
//  uint64_t t0 = gettimeofday(), t1 = t0;
//  sem.timedWait(1000000u);
//  t0 = gettimeofday() - t0;
//  struct timespec ts0, ts1;
//  clock_gettime(CLOCK_REALTIME,&ts0);
//  ksleep(1000000u);
//  clock_gettime(CLOCK_REALTIME,&ts1);
//  fprintf(stderr,"sem_timedwait: %"PRIu64", start %ld %ld, stop %ld %ld, tod %"PRIu64"\n",t0,ts0.tv_sec,ts0.tv_nsec,ts1.tv_sec,ts1.tv_nsec,t1 / 1000000u);
//  fprintf(stderr,"stop - start %ld %ld\n",ts1.tv_sec - ts1.tv_sec,ts1.tv_nsec - ts0.tv_nsec);
#endif
}
//---------------------------------------------------------------------------
void cleanup()
{
  stdErr.close();
  using namespace utf8;
  machineUniqueCryptedKey().~String();
#ifdef NETMAIL_ENABLE_PROFILER
  TProfiler::cleanup();
#endif
  Config::cleanup();
  LogFile::cleanup();
  AsyncFile::cleanup();
  BaseThread::cleanup();
  ksock::SockAddr::cleanup();
  ksock::api.cleanup();
  Fiber::cleanup();
  strErrorCleanup();
  argv().~Array<utf8::String>();
  utf8::String::cleanup();
  Mutant::cleanup();
  MemoryStream::cleanup();
  Thread::cleanup();
  MemoryManager::cleanup();
  LiteWriteLock::cleanup();
  Object::cleanup();
#if defined(__WIN32__) || defined(__WIN64__)
  SetConsoleCtrlHandler(consoleCtrlHandler,FALSE);
#elif HAVE_SIGNAL_H
  signalsCountersSem().~Semaphore();
#endif
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
intptr_t memncmp(const void * m1,const void * m2,uintptr_t n)
{
  intptr_t c = 0;
  
  while( n-- > 0 ){
    c = intptr_t(*(const uint8_t *) m1) - intptr_t(*(const uint8_t *) m2);
    if( c != 0 ) break;
    m1 = (const uint8_t *) m1 + 1;
    m2 = (const uint8_t *) m2 + 1;
  }
  return c;
}
//---------------------------------------------------------------------------
intptr_t memncmp(const void * m1,uintptr_t n1,const void * m2,uintptr_t n2)
{
  intptr_t c = n1 - n2;
  
  for(;;){
    if( c != 0 || n1 == 0 ) break;
    c = intptr_t(*(const uint8_t *) m1) - intptr_t(*(const uint8_t *) m2);
    m1 = (const uint8_t *) m1 + 1;
    m2 = (const uint8_t *) m2 + 1;
    n1--;
  }
  return c;
}
//---------------------------------------------------------------------------
void memxchg(void * m1,void * m2,uintptr_t n)
{
  while( n >= sizeof(uintptr_t) ){
    register uintptr_t v = *(uintptr_t *) m1;
    *(uintptr_t *) m1 = *(uintptr_t *) m2;
    *(uintptr_t *) m2 = v;
    m1 = (uintptr_t *) m1 + 1;
    m2 = (uintptr_t *) m2 + 1;
    n -= sizeof(uintptr_t);
  }
  while( n > 0 ){
    register uint8_t v = *(uint8_t *) m1;
    *(uint8_t *) m1 = *(uint8_t *) m2;
    *(uint8_t *) m2 = v;
    m1 = (uint8_t *) m1 + 1;
    m2 = (uint8_t *) m2 + 1;
    n--;
  }
}
//---------------------------------------------------------------------------
