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
uint8_t currentThreadPlaceHolder[sizeof(ThreadLocalVariable<Thread>)];
//------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
DWORD WINAPI Thread::threadFunc(LPVOID thread)
#else
void * Thread::threadFunc(void * thread)
#endif
{
#if HAVE_PTHREAD_H
  if( (errno = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL)) != 0 ){
    perror(NULL);
    abort();
  }
  if( (errno = pthread_mutex_lock(&reinterpret_cast<Thread *>(thread)->mutex_)) != 0 ){
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
  reinterpret_cast<Thread *>(thread)->mutex_ = NULL;
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
  reinterpret_cast<Thread *>(thread)->started_ = true;
  try {
    currentThread() = reinterpret_cast<Thread *>(thread);
    reinterpret_cast<Thread *>(thread)->threadExecute();
#if defined(__WIN32__) || defined(__WIN64__)
    if( reinterpret_cast<Thread *>(thread)->exitCode_ == (int32_t) STILL_ACTIVE )
      reinterpret_cast<Thread *>(thread)->exitCode_ = 0;
#endif
  }
  catch( ksys::ExceptionSP & e ){
    e->writeStdError();
    reinterpret_cast<Thread *>(thread)->exitCode_ = e->codes()[0];
  }
  catch( ... ){
  }
  try {
    for( i = afterExecuteActions().count() - 1; i >= 0; i-- )
      ((void (*)(void *)) afterExecuteActions()[i].handler())(afterExecuteActions()[i].data());
  }
  catch( ... ){
  }
  reinterpret_cast<Thread *>(thread)->finished_ = true;
  currentThread() = NULL;
#if defined(__WIN32__) || defined(__WIN64__)
//  while( PostThreadMessage(mainThreadId,threadFinishMessage,0,0) == 0 ) Sleep(1);
  return (DWORD) reinterpret_cast<Thread *>(thread)->exitCode_;
#elif HAVE_PTHREAD_H
  return (void *) (intptr_t) reinterpret_cast<Thread *>(thread)->exitCode_;
#endif
}
//---------------------------------------------------------------------------
Thread::Thread() :
#if __x86_64__
// getnameinfo take more then 8 Kb stack space
  stackSize_(PTHREAD_STACK_MIN * 8),
#else
  stackSize_(PTHREAD_STACK_MIN),
#endif
#if defined(__WIN32__) || defined(__WIN64__)
  handle_(NULL),
  id_(~DWORD(0)),
#elif HAVE_PTHREAD_H
  handle_(NULL), mutex_(NULL),
#endif
  exitCode_(0),
  started_(false), terminated_(false), finished_(false)
{
//  stackSize((uintptr_t) getpagesize());
}
//---------------------------------------------------------------------------
Thread & Thread::resume()
{
  int32_t err;
#if defined(__WIN32__) || defined(__WIN64__)
  if( handle_ == NULL ){
    started_ = terminated_ = finished_ = false;
    handle_ = CreateThread(NULL,stackSize_,threadFunc,this,CREATE_SUSPENDED,&id_);
    if( handle_ == NULL ){
      err = GetLastError() + errorOffset;
      newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
#elif HAVE_PTHREAD_H
  pthread_attr_t attr = NULL;
  if( handle_ == NULL ){
    started_ = terminated_ = finished_ = false;
    if( (errno = pthread_mutex_init(&mutex_,NULL)) != 0 ) goto l1;
    if( (errno = pthread_attr_init(&attr)) != 0 ) goto l1;
    if( (errno = pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE)) != 0 ) goto l1;
    if( (errno = pthread_attr_setstacksize(&attr,stackSize_)) != 0 ) goto l1;
#if HAVE_PTHREAD_ATTR_SETGUARDSIZE
    if( (errno = pthread_attr_setguardsize(&attr,0)) != 0 ) goto l1;
#endif
    if( (errno = pthread_mutex_lock(&mutex_)) != 0 ) goto l1;
    if( (errno = pthread_create(&handle_,&attr,threadFunc,this)) != 0 ){
      pthread_mutex_unlock(&mutex_);
      goto l1;
    }
#endif
  }
#if defined(__WIN32__) || defined(__WIN64__)
  if( ResumeThread(handle_) == (DWORD) - 1 ){
    err = GetLastError() + errorOffset;
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return *this;
#elif HAVE_PTHREAD_H
  if( (errno = pthread_mutex_unlock(&mutex_)) != 0 ){
    perror(NULL);
    abort();
  }
  return *this;
l1:
  err = errno;
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&mutex_);
  thread->mutex_ = NULL;
  newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  return *this;
#endif
}
//---------------------------------------------------------------------------
Thread & Thread::suspend()
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( SuspendThread(handle_) == (DWORD) -1 ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#else
  newObject<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
#endif
  return *this;
}
//---------------------------------------------------------------------------
Thread & Thread::wait()
{
  if( handle_ != NULL ){
#if defined(__WIN32__) || defined(__WIN64__)
    DWORD exitCode;
    BOOL r = GetExitCodeThread(handle_,&exitCode);
    if( r == 0 || exitCode == STILL_ACTIVE ){
      WaitForSingleObject(handle_,INFINITE);
      GetExitCodeThread(handle_,&exitCode);
    }
    CloseHandle(handle_);
    handle_ = NULL;
#elif HAVE_PTHREAD_H
    if( (errno = pthread_join(handle_,NULL)) != 0 ){
      perror(NULL);
      abort();
    }
    if( mutex_ != NULL ){
      if( (errno = pthread_mutex_destroy(&mutex_)) != 0 ){
        perror(NULL);
	      abort();
      }
      mutex_ = NULL;
    }
    handle_ = NULL;
#endif
  }
  return *this;
}
//---------------------------------------------------------------------------
Thread & Thread::priority(uintptr_t pri)
{
  if( started_ && !finished_ ){
#if HAVE_PTHREAD_SETSCHEDPARAM
    int policy;
    struct sched_param param;
    if( (errno = pthread_getschedparam(handle_,&policy,&param)) != 0 ){
l1:   int32_t err = errno;
      newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    param.sched_priority = pri;
    if( (errno = pthread_setschedparam(handle_,policy,&param)) != 0 ) goto l1;
#elif defined(__WIN32__) || defined(__WIN64__)
    if( SetThreadPriority(handle_,(int) pri) == 0 ){
      int32_t err = GetLastError() + errorOffset;
      newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
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
#if HAVE_PTHREAD_GETSCHEDPARAM
    int policy;
    struct sched_param param;
    if( (errno = pthread_getschedparam(handle_,&policy,&param)) != 0 ){
      int32_t err = errno;
      newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    pri = param.sched_priority;
#elif defined(__WIN32__) || defined(__WIN64__)
    pri = GetThreadPriority(handle_);
    if( pri == THREAD_PRIORITY_ERROR_RETURN ){
      int32_t err = GetLastError() + errorOffset;
      newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
#endif
  }
  return pri;
}
//---------------------------------------------------------------------------
bool Thread::isSuspended(uintptr_t handle)
{
  typedef enum _SYSTEM_INFORMATION_CLASS {
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

  typedef struct _SYSTEM_PROCESSES { // Information Class 5
    ULONG NextEntryDelta;
    ULONG ThreadCount;
    ULONG Reserved1[6];
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ProcessName;
    KPRIORITY BasePriority;
    ULONG ProcessId;
    ULONG InheritedFromProcessId;
    ULONG HandleCount;
    ULONG Reserved2[2];
    VM_COUNTERS VmCounters;
    IO_COUNTERS IoCounters; // Windows 2000 only
    SYSTEM_THREADS Threads[1];
  } SYSTEM_PROCESSES, *PSYSTEM_PROCESSES;

  typedef struct _SYSTEM_THREADS {
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG WaitTime;
    PVOID StartAddress;
    CLIENT_ID ClientId;
    KPRIORITY Priority;
    KPRIORITY BasePriority;
    ULONG ContextSwitchCount;
    THREAD_STATE State;
    KWAIT_REASON WaitReason;
  } SYSTEM_THREADS, *PSYSTEM_THREADS;

  typedef struct {
      FILETIME ProcessorTime;
      ULONG unk2, unk3;
      FILETIME CreateTime;
      ULONG unk6;
      ULONG StartAddress;
      ULONG ProcessID;
      ULONG ThreadID;
      ULONG CurrentPriority;
      ULONG BasePriority;
      ULONG ContextSwitchesPerSec;
      ULONG ThreadState;
      ULONG ThreadWaitReason;
      ULONG unk15;
  } SYSTEM_THREAD_INFORMATION;

  typedef struct {
      ULONG NextOffest; // ñëåäóþùèé ïðîöåññ
      ULONG ThreadCount;
      ULONG unk2, unk3, unk4, unk5, unk6, unk7;
      FILETIME CreateTime;
      FILETIME UserTime;
      FILETIME KernelTime;
      ULONG unk14;
      WCHAR *pModuleName; /* unicode */
      ULONG BasePriority;
      ULONG ProcessID;
      ULONG InheritedFromUniqueProcessID;
      ULONG HandleCount;
      ULONG unk20, unk21;
      ULONG PeekVirtualSize;
      ULONG VirtualSize;
      ULONG PageFaultCountPerSec;
      ULONG PeakWorkingSetSize;
      ULONG WorkingSetSize;
      ULONG PeekPagedPoolUsage;
      ULONG PagedPoolUsage;
      ULONG PeekNonPagedPoolUsage;
      ULONG NonPagedPoolUsage;
      ULONG unk31; /* PagefileUsage ? */
      ULONG PeakPagefileUsage;
      ULONG unk33; /* PrivateBytes ? */
      SYSTEM_THREAD_INFORMATION ThreadInfos[1]; // ïîòîêè äàííîãî ïðîöåññà
  } SYSTEM_PROCESS_INFORMATION;

  typedef struct _VM_COUNTERS {
    ULONG PeakVirtualSize;
    ULONG VirtualSize;
    ULONG PageFaultCount;
    ULONG PeakWorkingSetSize;
    ULONG WorkingSetSize;
    ULONG QuotaPeakPagedPoolUsage;
    ULONG QuotaPagedPoolUsage;
    ULONG QuotaPeakNonPagedPoolUsage;
    ULONG QuotaNonPagedPoolUsage;
    ULONG PagefileUsage;
    ULONG PeakPagefileUsage;
  } VM_COUNTERS, *PVM_COUNTERS;

  typedef struct _IO_COUNTERS {
    LARGE_INTEGER ReadOperationCount;
    LARGE_INTEGER WriteOperationCount;
    LARGE_INTEGER OtherOperationCount;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
  } IO_COUNTERS, *PIO_COUNTERS;

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

  HINSTANCE hInstLib = LoadLibrary("ntdll.dll");
  if( hInstLib != NULL ){
    typedef NTSYSAPI NTSTATUS NTAPI (* PFNZwQuerySystemInformation)(
      IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
      IN OUT PVOID SystemInformation,
      IN ULONG SystemInformationLength,
      OUT PULONG ReturnLength OPTIONAL
    );
/*
  Parameters
  SystemInformationClass
  The type of system information to be queried.The permitted values are a subset of
  the enumeration SYSTEM_INFORMATION_CLASS, described in the following section.

  SystemInformation
    Points to a caller-allocated buffer or variable that receives the requested system
    information.
  SystemInformationLength
    The size in bytes of SystemInformation, which the caller should set according to the
    given SystemInformationClass.

  ReturnLength
    Optionally points to a variable that receives the number of bytes actually returned to
    SystemInformation; if SystemInformationLength is too small to contain the available
    information, the variable is normally set to zero except for two information classes
    (6 and 11) when it is set to the number of bytes required for the available information.
    If this information is not needed, ReturnLength may be a null pointer.

  Return Value
    Returns STATUS_SUCCESS or an error status, such as STATUS_INVALID_INFO_CLASS,
    STATUS_NOT_IMPLEMENTED or STATUS_INFO_LENGTH_MISMATCH.
  Related Win32 Functions
    GetSystemInfo, GetTimeZoneInformation, GetSystemTimeAdjustment, PSAPI functions,
    and performance counters.

  Remarks
    ZwQuerySystemInformation is the source of much of the information displayed by
    “Performance Monitor” for the classes Cache, Memory, Objects, Paging File, Process,
    Processor, System, and Thread. It is also frequently used by resource kit utilities that
    display information about the system.
      The ReturnLength information is not always valid (depending on the information
    class), even when the routine returns STATUS_SUCCESS.When the return value indicates
    STATUS_INFO_LENGTH_MISMATCH, only some of the information classes return an estimate
    of the required length.
      Some information classes are implemented only in the “checked” version of the
    kernel. Some, such as SystemCallCounts, return useful information only in “checked”
    versions of the kernel.
      Some information classes require certain flags to have been set in NtGlobalFlags at
    boot time. For example, SystemObjectInformation requires that
    FLG_MAINTAIN_OBJECT_TYPELIST be set at boot time.
      Information class SystemNotImplemented1 (4) would return STATUS_NOT_IMPLEMENTED
    if it were not for the fact that it uses DbgPrint to print the text “EX:
    SystemPathInformation now available via SharedUserData.” and then calls
    DbgBreakPoint.The breakpoint exception is caught by a frame based exception handler
    (in the absence of intervention by a debugger) and causes ZwQuerySystemInformation
    to return with STATUS_BREAKPOINT.
 */
    typedef NTSTATUS (* PFNNtQuerySystemInformation)(
      IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
      OUT PVOID SystemInformation,
      IN ULONG SystemInformationLength,
      OUT PULONG ReturnLength OPTIONAL
    );
    PFNNtQuerySystemInformation pNtQuerySystemInformation =
      (PFNNtQuerySystemInformation) GetProcAddress(hInstLib,"NtQuerySystemInformation");
    PFNZwQuerySystemInformation pZwQuerySystemInformation =
      (PFNZwQuerySystemInformation) GetProcAddress(hInstLib,"ZwQuerySystemInformation");

    SYSTEM_PROCESSES_INFORMATION processes;
    ULONG returnLength;
    NTSTATUS status = pZwQuerySystemInformation(
      SystemProcessesAndThreadsInformation,
      &processes,
      sizeof(processes),
      &returnLength
    );
    if( status == STATUS_INFO_LENGTH_MISMATCH ){
    }
    if( status == STATUS_SUCCESS ){
    }
    FreeLibrary(hInstLib);
  }
}
//---------------------------------------------------------------------------
uint8_t Thread::beforeExecuteActions_[sizeof(Array<Action>)];
uint8_t Thread::afterExecuteActions_[sizeof(Array<Action>)];
//---------------------------------------------------------------------------
void Thread::initialize()
{
  new (beforeExecuteActions_) Array<Action>;
  new (afterExecuteActions_) Array<Action>;
  new (currentThreadPlaceHolder) ThreadLocalVariable<Thread>;
}
//---------------------------------------------------------------------------
void Thread::cleanup()
{
  currentThread().~ThreadLocalVariable<Thread>();
  afterExecuteActions().~Array<Action>();
  beforeExecuteActions().~Array<Action>();
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
