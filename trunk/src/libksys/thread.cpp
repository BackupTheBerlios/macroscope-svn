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
#include <adicpp/ksys.h>
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
#if HAVE_SCHED_H
//---------------------------------------------------------------------------
int schedGetPriorityMin()
{
#ifdef __FreeBSD__
  return 0;
#else
  int pri = sched_get_priority_min(sched_getscheduler(0));
  if( pri == -1 ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__);
  }
  return pri;
#endif
}
//---------------------------------------------------------------------------
int schedGetPriorityMax()
{
#ifdef __FreeBSD__
  return 31;
#else
  int pri = sched_get_priority_max(sched_getscheduler(0));
  if( pri == -1 ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__);
  }
  return pri;
#endif
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
uint8_t currentThreadPlaceHolder[sizeof(ThreadLocalVariable<Thread>)];
//------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
void Thread::afterConstruction()
{
  //if( !object_.heap_ )
  //  newObjectV1C2<Exception>(
  //    EINVAL,
  //    __PRETTY_FUNCTION__ + utf8::String(" ") +
  //    "Thread must be allocated dynamicaly only via newObject function(s)."
  //  )->throwSP();
}
//---------------------------------------------------------------------------
#if HAVE_PTHREAD_H
uint8_t Thread::handleNull_[sizeof(pthread_t)];
#endif
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
DWORD WINAPI Thread::threadFunc(LPVOID thread)
#else
void * Thread::threadFunc(void * thread)
#endif
{
#if HAVE_PTHREAD_H
  if( (errno = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL)) != 0 ){
    perror(NULL);
    abort();
  }
  if( (errno = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL)) != 0 ){
    perror(NULL);
    abort();
  }
  /*if( (errno = pthread_mutex_lock(&reinterpret_cast<Thread *>(thread)->mutex_)) != 0 ){
    perror(NULL);
    abort();
  }
  if( (errno = pthread_mutex_unlock(&reinterpret_cast<Thread *>(thread)->mutex_)) != 0 ){
    perror(NULL);
    abort();
  }
  if( (errno = pthread_mutex_destroy(&reinterpret_cast<Thread *>(thread)->mutex_)) != 0 ){
    perror(NULL);
    abort();
  }
  reinterpret_cast<Thread *>(thread)->mutex_ = NULL;*/
#endif
  intptr_t i;
  try {
    for( i = beforeExecuteActions().count() - 1; i >= 0; i-- )
      ((void (*)(void *)) beforeExecuteActions()[i].handler())(beforeExecuteActions()[i].data());
  }
  catch( ... ){
  }
#if defined(__WIN32__) || defined(__WIN64__)
  reinterpret_cast<Thread *>(thread)->exitCode_ = STILL_ACTIVE;
#endif
  reinterpret_cast<Thread *>(thread)->finished_ = false;
  reinterpret_cast<Thread *>(thread)->started_ = true;
  try {
    currentThread() = reinterpret_cast<Thread *>(thread);
    reinterpret_cast<Thread *>(thread)->beforeExecute();
    reinterpret_cast<Thread *>(thread)->threadExecute();
    reinterpret_cast<Thread *>(thread)->afterExecute();
#if defined(__WIN32__) || defined(__WIN64__)
    if( reinterpret_cast<Thread *>(thread)->exitCode_ == (int32_t) STILL_ACTIVE )
      reinterpret_cast<Thread *>(thread)->exitCode_ = 0;
#endif
  }
  catch( ExceptionSP & e ){
    try {
      reinterpret_cast<Thread *>(thread)->afterExecute();
    }
    catch( ExceptionSP & e ){
      e->writeStdError();
    }
    catch( ... ){
#ifndef NDEBUG
    stdErr.debug(9,utf8::String::Stream() << __FILE__ << " " << __LINE__).flush();
#endif
    }
    e->writeStdError();
    reinterpret_cast<Thread *>(thread)->exitCode_ = e->code();
  }
  catch( ... ){
#ifndef NDEBUG
    stdErr.debug(9,utf8::String::Stream() << __FILE__ << " " << __LINE__).flush();
#endif
  }
  for( i = afterExecuteActions().count() - 1; i >= 0; i-- )
    ((void (*)(void *)) afterExecuteActions()[i].handler())(afterExecuteActions()[i].data());
  reinterpret_cast<Thread *>(thread)->finished_ = true;
  currentThread() = NULL;
#if defined(__WIN32__) || defined(__WIN64__)
//  while( PostThreadMessage(mainThreadId,threadFinishMessage,0,0) == 0 ) Sleep(1);
  return (DWORD) reinterpret_cast<Thread *>(thread)->exitCode_;
#elif HAVE_PTHREAD_H
  return (void *) reinterpret_cast<Thread *>(thread)->exitCode_;
//  pthread_exit((void *) reinterpret_cast<Thread *>(thread)->exitCode_);
#endif
}
//---------------------------------------------------------------------------
Thread::Thread() :
//#if __x86_64__
// getnameinfo take more then 8 Kb stack space
#if defined(__WIN32__) || defined(__WIN64__)
  stackSize_(64u * 1024u),
#else
  stackSize_(64u * 1024u/*PTHREAD_STACK_MIN*/),
#endif
//#else
//  stackSize_(PTHREAD_STACK_MIN),
//#endif
#if defined(__WIN32__) || defined(__WIN64__)
  handle_(NULL),
  id_(~DWORD(0)),
#elif HAVE_PTHREAD_H
//  handle_((pthread_t) NULL),// mutex_(NULL),
#endif
  exitCode_(0),
  started_(false), terminated_(false), finished_(false)
{
#if HAVE_PTHREAD_H
  memcpy(&handle_,handleNull_,sizeof(handle_));
#endif
//  stackSize((uintptr_t) getpagesize());
}
//---------------------------------------------------------------------------
Thread & Thread::resume()
{
  int32_t err;
#if defined(__WIN32__) || defined(__WIN64__)
  if( handle_ == NULL ){
    started_ = terminated_ = false;
    handle_ = CreateThread(NULL,stackSize_,threadFunc,this,CREATE_SUSPENDED,&id_);
    if( handle_ == NULL ){
      err = GetLastError() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    if( ResumeThread(handle_) == (DWORD) - 1 ){
      err = GetLastError() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
  }
  else if( finished_ ){
    wait();
    resume();
  }
#elif HAVE_PTHREAD_H
  pthread_attr_t attr;
  memset(&attr,0,sizeof(attr));
  if( memcmp(&handle_,handleNull_,sizeof(handle_)) == 0 ){
    started_ = terminated_ = false;
//    if( (errno = pthread_mutex_init(&mutex_,NULL)) != 0 ) goto l1;
    if( (errno = pthread_attr_init(&attr)) != 0 ) goto l1;
    if( (errno = pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE)) != 0 ) goto l1;
    if( (errno = pthread_attr_setstacksize(&attr,stackSize_)) != 0 ) goto l1;
#if HAVE_PTHREAD_ATTR_SETGUARDSIZE
    if( (errno = pthread_attr_setguardsize(&attr,0)) != 0 ) goto l1;
#endif
    //if( (errno = pthread_mutex_lock(&mutex_)) != 0 ) goto l1;
    if( (errno = pthread_create(&handle_,&attr,threadFunc,this)) != 0 ){
//      pthread_mutex_unlock(&mutex_);
      goto l1;
    }
  }
  else if( finished_ ){
    wait();
    resume();
  }
  /*if( (errno = pthread_mutex_unlock(&mutex_)) != 0 ){
    perror(NULL);
    abort();
  }*/
  return *this;
l1:
  err = errno;
  pthread_attr_destroy(&attr);
  //pthread_mutex_destroy(&mutex_);
  //mutex_ = NULL;
  newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
#endif
  return *this;
}
//---------------------------------------------------------------------------
Thread & Thread::suspend()
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( SuspendThread(handle_) == (DWORD) -1 ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#else
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
#endif
  return *this;
}
//---------------------------------------------------------------------------
Thread & Thread::wait()
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( handle_ != NULL ){
    threadBeforeWait();
    DWORD exitCode;
    BOOL r = GetExitCodeThread(handle_,&exitCode);
    if( r == 0 || exitCode == STILL_ACTIVE ){
      if( WaitForSingleObject(handle_,INFINITE) == WAIT_FAILED ){
        int32_t err = GetLastError() + errorOffset;
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
      GetExitCodeThread(handle_,&exitCode);
    }
    CloseHandle(handle_);
    handle_ = NULL;
#elif HAVE_PTHREAD_H
  if( memcmp(&handle_,handleNull_,sizeof(handle_)) != 0 ){
    threadBeforeWait();
    void * result;
    if( (errno = pthread_join(handle_,&result)) != 0 ){
      perror(NULL);
      abort();
    }
    /*if( mutex_ != NULL ){
      if( (errno = pthread_mutex_destroy(&mutex_)) != 0 ){
        perror(NULL);
        abort();
      }
      mutex_ = NULL;
    }*/
    memcpy(&handle_,handleNull_,sizeof(handle_));
#endif
    threadAfterWait();
  }
  return *this;
}
//---------------------------------------------------------------------------
Thread & Thread::priority(uintptr_t pri,bool noThrow)
{
  if( started_ && !finished_ ){
#if HAVE_SYSCONF
//    if( sysconf(_POSIX_THREAD_PRIORITY_SCHEDULING) <= 0 ) return *this;
#endif
#if HAVE_PTHREAD_GETSCHEDPARAM && HAVE_PTHREAD_SETSCHEDPARAM
    int policy;
    struct sched_param param;
    if( (errno = pthread_getschedparam(handle_,&policy,&param)) != 0 ){
l1:   int32_t err = errno;
      AutoPtr<Exception> e(newObjectV1C2<Exception>(err,utf8::int2Str(pri) + " " + __PRETTY_FUNCTION__));
      if( !noThrow ) e.ptr(NULL)->throwSP();
      e->writeStdError();
      return *this;
    }
    param.sched_priority = pri;
    if( (errno = pthread_setschedparam(handle_,policy,&param)) != 0 ) goto l1;
#elif HAVE_PTHREAD_SETPRIO
    if( (errno = pthread_setprio(handle_,int(pri))) != 0 ){
      int32_t err = errno;
      AutoPtr<Exception> e(newObjectV1C2<Exception>(err,utf8::int2Str(pri) + " " + __PRETTY_FUNCTION__));
      if( !noThrow ) e.ptr(NULL)->throwSP();
      e->writeStdError();
    }
#elif defined(__WIN32__) || defined(__WIN64__)
    if( SetThreadPriority(handle_,(int) pri) == 0 ){
      int32_t err = GetLastError() + errorOffset;
      AutoPtr<Exception> e(newObjectV1C2<Exception>(err,utf8::int2Str(pri) + " " + __PRETTY_FUNCTION__));
      if( !noThrow ) e.ptr(NULL)->throwSP();
      e->writeStdError();
    }
#endif
  }
  return *this;
}
//---------------------------------------------------------------------------
uintptr_t Thread::priority() const
{
  uintptr_t pri = 0;
  if( started_ && !finished_ ){
#if HAVE_SYSCONF
//    if( sysconf(_POSIX_THREAD_PRIORITY_SCHEDULING) <= 0 ) return pri;
#endif
#if HAVE_PTHREAD_GETSCHEDPARAM && HAVE_PTHREAD_SETSCHEDPARAM
    int policy;
    struct sched_param param;
    if( (errno = pthread_getschedparam(handle_,&policy,&param)) != 0 ){
      int32_t err = errno;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    pri = param.sched_priority;
#elif HAVE_PTHREAD_GETPRIO
    int a = pthread_getprio(handle_);
    if( a == -1 ){
      int32_t err = errno;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    pri = a;
#elif defined(__WIN32__) || defined(__WIN64__)
    pri = GetThreadPriority(handle_);
    if( pri == THREAD_PRIORITY_ERROR_RETURN ){
      int32_t err = GetLastError() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
#endif
  }
  return pri;
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
//#pragma comment (linker, "/defaultlib:ntdll.lib")
//---------------------------------------------------------------------------
/*typedef enum _SYSTEM_INFORMATION_CLASS {
  SystemBasicInformation, // 0 Y N
  SystemProcessorInformation, // 1 Y N
  SystemPerformanceInformation, // 2 Y N
  SystemTimeOfDayInformation, // 3 Y N
  SystemNotImplemented1, // 4 Y N
  SystemProcessesAndThreadsInformation, // 5 Y N
  SystemCallCounts, // 6 Y N
  SystemConfigurationInformation, // 7 Y N
  SystemProcessorTimes, // 8 Y N
  SystemGlobalFlag, // 9 Y Y
  SystemNotImplemented2, // 10 Y N
  SystemModuleInformation, // 11 Y N
  SystemLockInformation, // 12 Y N
  SystemNotImplemented3, // 13 Y N
  SystemNotImplemented4, // 14 Y N
  SystemNotImplemented5, // 15 Y N
  SystemHandleInformation, // 16 Y N
  SystemObjectInformation, // 17 Y N
  SystemPagefileInformation, // 18 Y N
  SystemInstructionEmulationCounts, // 19 Y N
  SystemInvalidInfoClass1, // 20
  SystemCacheInformation, // 21 Y Y
  SystemPoolTagInformation, // 22 Y N
  SystemProcessorStatistics, // 23 Y N
  SystemDpcInformation, // 24 Y Y
  SystemNotImplemented6, // 25 Y N
  SystemLoadImage, // 26 N Y
  SystemUnloadImage, // 27 N Y
  SystemTimeAdjustment, // 28 Y Y
  SystemNotImplemented7, // 29 Y N
  SystemNotImplemented8, // 30 Y N
  SystemNotImplemented9, // 31 Y N
  SystemCrashDumpInformation, // 32 Y N
  SystemExceptionInformation, // 33 Y N
  SystemCrashDumpStateInformation, // 34 Y Y/N
  SystemKernelDebuggerInformation, // 35 Y N
  SystemContextSwitchInformation, // 36 Y N
  SystemRegistryQuotaInformation, // 37 Y Y
  SystemLoadAndCallImage, // 38 N Y
  SystemPrioritySeparation, // 39 N Y
  SystemNotImplemented10, // 40 Y N
  SystemNotImplemented11, // 41 Y N
  SystemInvalidInfoClass2, // 42
  SystemInvalidInfoClass3, // 43
  SystemTimeZoneInformation, // 44 Y N
  SystemLookasideInformation, // 45 Y N
  SystemSetTimeSlipEvent, // 46 N Y
  SystemCreateSession, // 47 N Y
  SystemDeleteSession, // 48 N Y
  SystemInvalidInfoClass4, // 49
  SystemRangeStartInformation, // 50 Y N
  SystemVerifierInformation, // 51 Y Y
  SystemAddVerifier, // 52 N Y
  SystemSessionProcessesInformation // 53 Y N
} SYSTEM_INFORMATION_CLASS;
*/
typedef enum {
  StateInitialized,
  StateReady,
  StateRunning,
  StateStandby,
  StateTerminated,
  StateWait,
  StateTransition,
  StateUnknown
} THREAD_STATE;

typedef enum _KWAIT_REASON {
  Executive,
  FreePage,
  PageIn,
  PoolAllocation,
  DelayExecution,
  Suspended,
  UserRequest,
  WrExecutive,
  WrFreePage,
  WrPageIn,
  WrPoolAllocation,
  WrDelayExecution,
  WrSuspended,
  WrUserRequest,
  WrEventPair,
  WrQueue,
  WrLpcReceive,
  WrLpcReply,
  WrVirtualMemory,
  WrPageOut,
  WrRendezvous,
  Spare2,
  Spare3,
  Spare4,
  Spare5,
  Spare6,
  WrKernel
} KWAIT_REASON;

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
#ifdef MIDL_PASS
  [size_is(MaximumLength / 2), length_is((Length) / 2) ] USHORT * Buffer;
#else // MIDL_PASS
  PWSTR  Buffer;
#endif // MIDL_PASS
} UNICODE_STRING;

typedef LONG KPRIORITY;

typedef struct _VM_COUNTERS {
  SIZE_T PeakVirtualSize;
  SIZE_T VirtualSize;
  ULONG PageFaultCount;
  SIZE_T PeakWorkingSetSize;
  SIZE_T WorkingSetSize;
  SIZE_T QuotaPeakPagedPoolUsage;
  SIZE_T QuotaPagedPoolUsage;
  SIZE_T QuotaPeakNonPagedPoolUsage;
  SIZE_T QuotaNonPagedPoolUsage;
  SIZE_T PagefileUsage;
  SIZE_T PeakPagefileUsage;
} VM_COUNTERS;
typedef VM_COUNTERS *PVM_COUNTERS;

typedef struct _CLIENT_ID {
  HANDLE UniqueProcess;
  HANDLE UniqueThread;
} CLIENT_ID;
typedef CLIENT_ID *PCLIENT_ID;

typedef struct _SYSTEM_THREAD {
  FILETIME     ftKernelTime;      // 100 nsec units
  FILETIME     ftUserTime;        // 100 nsec units
  FILETIME     ftCreateTime;      // relative to 01-01-1601
  DWORD        dWaitTime;
  PVOID        pStartAddress;
  CLIENT_ID    Cid;               // process/thread ids
  DWORD        dPriority;
  DWORD        dBasePriority;
  DWORD        dContextSwitches;
  DWORD        dThreadState;      // 2=running, 5=waiting
  KWAIT_REASON WaitReason;
  DWORD        dReserved01;
} SYSTEM_THREAD, * PSYSTEM_THREAD, **PPSYSTEM_THREAD;

typedef struct _SYSTEM_PROCESS {          // common members
  DWORD          dNext;           // relative offset
  DWORD          dThreadCount;
  DWORD          dReserved01;
  DWORD          dReserved02;
  DWORD          dReserved03;
  DWORD          dReserved04;
  DWORD          dReserved05;
  DWORD          dReserved06;
  FILETIME       ftCreateTime;    // relative to 01-01-1601
  FILETIME       ftUserTime;      // 100 nsec units
  FILETIME       ftKernelTime;    // 100 nsec units
  UNICODE_STRING usName;
  KPRIORITY      BasePriority;
  DWORD          dUniqueProcessId;
  DWORD          dInheritedFromUniqueProcessId;
  DWORD          dHandleCount;
  DWORD          dReserved07;
  DWORD          dReserved08;
  VM_COUNTERS    VmCounters;      // see ntddk.h
  DWORD          dCommitCharge;   // bytes
} SYSTEM_PROCESS, * PSYSTEM_PROCESS, **PPSYSTEM_PROCESS;

typedef struct _SYSTEM_PROCESS_NT4 {     // Windows NT 4.0
  SYSTEM_PROCESS Process;         // common members
  SYSTEM_THREAD  aThreads [1];     // thread array
} SYSTEM_PROCESS_NT4, * PSYSTEM_PROCESS_NT4, **PPSYSTEM_PROCESS_NT4;

typedef struct _SYSTEM_PROCESS_NT5 {      // Windows 2000
  SYSTEM_PROCESS Process;         // common members
  IO_COUNTERS    IoCounters;      // see ntddk.h
  SYSTEM_THREAD  aThreads [1];     // thread array
} SYSTEM_PROCESS_NT5, * PSYSTEM_PROCESS_NT5, **PPSYSTEM_PROCESS_NT5;

#ifndef STATUS_INFO_LENGTH_MISMATCH
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L) // ntsubauth
#endif

typedef NTSTATUS (NTAPI * PFNZwQuerySystemInformation)(
  IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
  IN OUT PVOID SystemInformation,
  IN ULONG SystemInformationLength,
  OUT PULONG ReturnLength OPTIONAL
);
typedef NTSTATUS (NTAPI * PFNNtQuerySystemInformation)(
  IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
  OUT PVOID SystemInformation,
  IN ULONG SystemInformationLength,
  OUT PULONG ReturnLength OPTIONAL
);
//---------------------------------------------------------------------------
bool Thread::isSuspended(uintptr_t tid)
{
  bool r = false;
  HINSTANCE hInstLib = LoadLibraryEx("ntdll.dll",NULL,DONT_RESOLVE_DLL_REFERENCES);
  if( hInstLib == NULL ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  PFNNtQuerySystemInformation pNtQuerySystemInformation =
    (PFNNtQuerySystemInformation) GetProcAddress(hInstLib,"NtQuerySystemInformation");
  union {
    PSYSTEM_PROCESS_INFORMATION pProcess;
    PSYSTEM_PROCESS_NT4 pProcessNT4;
    PSYSTEM_PROCESS_NT5 pProcessNT5;
  };
  PSYSTEM_PROCESS_INFORMATION pInfo = NULL;
  ULONG returnLength, length = 0;
  NTSTATUS status = pNtQuerySystemInformation(
    SystemProcessInformation,
    pInfo,
    length,
    &returnLength
  );
  while( status == STATUS_INFO_LENGTH_MISMATCH ){
    length = returnLength;
    if( pInfo == NULL ){
      pInfo = (PSYSTEM_PROCESS_INFORMATION) LocalAlloc(LMEM_FIXED,returnLength);
    }
    else {
      pInfo = (PSYSTEM_PROCESS_INFORMATION) LocalReAlloc(pInfo,returnLength,0);
    }
    status = pNtQuerySystemInformation(
      SystemProcessInformation,
      pInfo,
      length,
      &returnLength
    );
  }
  if( status == STATUS_SUCCESS ){
    pProcess = pInfo;
    uintptr_t pid = GetCurrentProcessId();
    for(;;){
      /*fprintf(stdout,"%S %u\n",
        pProcessNT5->Process.usName.Buffer != NULL ? pProcessNT5->Process.usName.Buffer : L"Idle",
        pProcess->UniqueProcessId
      );*/
      if( (uintptr_t) pProcess->UniqueProcessId == pid ){
        PSYSTEM_THREAD pThread, pThread1;
        if( isWinNT4() ){
          pThread = pProcessNT4->aThreads;
        }
        else {
          pThread = pProcessNT5->aThreads;
        }
        pThread1 = pThread + pProcessNT5->Process.dThreadCount;
        while( pThread < pThread1 ){
          /*fprintf(stdout,"%S %u %u\n",
            pProcessNT5->Process.usName.Buffer != NULL ? pProcessNT5->Process.usName.Buffer : L"Idle",
            pThread->Cid.UniqueProcess,
            pThread->Cid.UniqueThread
          );*/
          if( (uintptr_t) pThread->Cid.UniqueThread == tid ){
            r = pThread->dThreadState == StateStandby || pThread->dThreadState == StateWait;
            pProcess->NextEntryOffset = 0;
            break;
          }
          pThread++;
        }
      }
      if( pProcess->NextEntryOffset == 0 ) break;
      pProcess = (PSYSTEM_PROCESS_INFORMATION) ((PBYTE) pProcess + pProcess->NextEntryOffset);
    }
  }
  else {
    LocalFree(pInfo);
    FreeLibrary(hInstLib);
    int32_t err = status + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  LocalFree(pInfo);
  FreeLibrary(hInstLib);
  return r;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
uintptr_t Thread::waitForSignal(uintptr_t mId)
{
  MSG msg;
  for(;;){
    if( PeekMessage(&msg,NULL,0,0,PM_REMOVE) != 0 ){
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    if( msg.message == mId ) break;
    if( WaitMessage() == 0 ){
      int32_t err = GetLastError() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
  }
  return msg.message;
}
//---------------------------------------------------------------------------
uintptr_t Thread::checkForSignal(uintptr_t mId)
{
  MSG msg;
  if( PeekMessage(&msg,NULL,0,0,PM_REMOVE) != 0 ){
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return msg.message == mId ? msg.message : 0;
}
//---------------------------------------------------------------------------
#elif HAVE_SIGNAL_H
//---------------------------------------------------------------------------
uintptr_t Thread::waitForSignal(uintptr_t sId)
{
  assert( sId < _SIG_MAXSIG );
  for(;;){
    waitForSignalsSemaphore();
    if( sId == 0 ){
      interlockedIncrement(signalsCounters[SIGINT - 1],0);
      if( interlockedIncrement(signalsCounters[SIGINT - 1],0) > 0 ) return SIGINT;
      if( interlockedIncrement(signalsCounters[SIGQUIT - 1],0) > 0 ) return SIGQUIT;
      if( interlockedIncrement(signalsCounters[SIGTERM - 1],0) > 0 ) return SIGTERM;
    }
    else if( interlockedIncrement(signalsCounters[sId - 1],0) > 0 ) return sId;
  }
}
//---------------------------------------------------------------------------
uintptr_t Thread::checkForSignal(uintptr_t sId)
{
  assert( sId < _SIG_MAXSIG );
  if( sId == 0 ){
    interlockedIncrement(signalsCounters[SIGINT - 1],0);
    if( interlockedIncrement(signalsCounters[SIGINT - 1],0) > 0 ) return SIGINT;
    if( interlockedIncrement(signalsCounters[SIGQUIT - 1],0) > 0 ) return SIGQUIT;
    if( interlockedIncrement(signalsCounters[SIGTERM - 1],0) > 0 ) return SIGTERM;
  }
  else if( interlockedIncrement(signalsCounters[sId - 1],0) > 0 ) return sId;
  return 0;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
uint8_t Thread::beforeExecuteActions_[sizeof(Array<Action>)];
uint8_t Thread::afterExecuteActions_[sizeof(Array<Action>)];
//---------------------------------------------------------------------------
void Thread::initialize()
{
  new (beforeExecuteActions_) Array<Action>;
  new (afterExecuteActions_) Array<Action>;
  new (currentThreadPlaceHolder) ThreadLocalVariable<Thread *>;
#if HAVE_PTHREAD_H
  memset(handleNull_,0xFF,sizeof(pthread_t));
#endif
}
//---------------------------------------------------------------------------
void Thread::cleanup()
{
  currentThread().~ThreadLocalVariable<Thread *>();
  afterExecuteActions().~Array<Action>();
  beforeExecuteActions().~Array<Action>();
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
