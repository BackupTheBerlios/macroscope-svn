#include <adicpp/lconfig.h>
#include <adicpp/pdbutils/pdbutils.h>

#ifndef R_OK
#  define R_OK    4               /* Test for read permission.  */
#  define W_OK    2               /* Test for write permission.  */
#  define X_OK    1               /* Test for execute permission.  */
#  define F_OK    0               /* Test for existence.  */  
#endif

#define NL "\n"

//#define LOCAL_DEBUG

//#if defined(LOCAL_DEBUG)
#include <iostream>
//#endif

#ifdef HAVE_DBGHELP_H

namespace pdbutils {

HANDLE _currentProcess = 0;

#if defined(LOCAL_DEBUG)

# define DOUT(msg) do { std::cout << msg; } while (false)
# define DOUTNL(msg) do { std::cout << msg << std::endl; } while (false)
#else
# define DOUT(msg)
# define DOUTNL(msg) 
#endif

#define ELOGNL(msg)  do { DbgStream out; out << msg << NL; OutputDebugString(out.str().c_str()); } while (false)

namespace {


}

static intptr_t CurrentPointerRecursion = 0;

static
bool 
_init_sys_libs()
{
  static bool _inited = false;
  if (_inited == true)
    return true;
  //SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
  SymSetOptions( SYMOPT_DEFERRED_LOADS );
  if (!SymInitialize(GetCurrentProcess(), 0, TRUE))
    return false;
  _currentProcess = GetCurrentProcess();
  char sympath[4096];
  SymGetSearchPath(_currentProcess, sympath, sizeof(sympath));
  
  char moduleFilename[MAX_PATH];
  memset(moduleFilename, 0, sizeof(moduleFilename));
  HMODULE hm = GetModuleHandle(NULL);
  GetModuleFileName(hm, moduleFilename, MAX_PATH);
  DbgString mfile = moduleFilename;
  intptr_t lidx = DBGSTRING_LASTINDEX(mfile, "\\");
  if (lidx != -1)
  {
    DbgString modulepath = DBGSTRING_SUBSTR2(mfile, 0, lidx);
    DbgString modname = DBGSTRING_SUBSTR1(mfile, lidx + 1);
    modname = DBGSTRING_SUBSTR2(modname, 0, DBGSTRING_SIZE(modname) - 4); // asume .exe
    DbgString str = sympath;
    str = str + ";" + modulepath;
    str = str + ";" + modulepath + "\\" + modname + "_pdbs";
    SymSetSearchPath(_currentProcess, (char*)DBGSTRING2CHARPTR(str));
    char buffer[1024];
    SymGetSearchPath(_currentProcess, buffer, 1024);
    ELOGNL("Set new SymbolPath: " << (char*)buffer);
  }
  else
  {
    DOUTNL("Cannot set new SymbolPath: " << (char*)DBGSTRING2CHARPTR(mfile));
  }
  _inited = true;
  return _inited;
}

bool 
DbgFrame::findElementForAddress(void* addr, DbgType& type) const
{
  intptr_t i = 0;
  for (i = 0; i < params.size(); ++i)
  {
    if (params[i].findElementForAddress(addr, type) == true)
        return true;
  }
  for (i = 0; i < locals.size(); ++i)
  {
    if (locals[i].findElementForAddress(addr, type) == true)
      return true;
  }
  return false;
}


//static 
intptr_t 
getProgramCounters(FrameAddress* frames, intptr_t pccount, void* threadHandle)
{
  if (_init_sys_libs() == false)
    return 0;
  if (threadHandle == 0)
    threadHandle = GetCurrentThread();
  DWORD dwMachineType = 0;
  
  memset(frames, 0, sizeof(FrameAddress) * pccount);
  STACKFRAME64 sf;
  memset(&sf, 0, sizeof(sf));

  CONTEXT context;
  context.ContextFlags = CONTEXT_FULL; 
  HANDLE currentThread = threadHandle;
  HANDLE currentProcess = _currentProcess;
  if (GetThreadContext(currentThread, &context) == 0)
    return 0;
#ifdef _M_IX86
    // Initialize the STACKFRAME structure for the first call.  This is only
    // necessary for Intel CPUs, and isn't mentioned in the documentation.
  sf.AddrPC.Offset       = context.Eip;
  sf.AddrPC.Mode         = AddrModeFlat;
  sf.AddrStack.Offset    = context.Esp;
  sf.AddrStack.Mode      = AddrModeFlat;
  sf.AddrFrame.Offset    = context.Ebp;
  sf.AddrFrame.Mode      = AddrModeFlat;
  dwMachineType = IMAGE_FILE_MACHINE_I386;
#endif
#ifdef _M_X64
  dwMachineType = IMAGE_FILE_MACHINE_AMD64;
#endif
  intptr_t i;
  for (i = 0; i < pccount; ++i)
  {
        // Get the next stack frame
    if ( ! StackWalk64(  dwMachineType,
                            currentProcess,
                            currentThread,
                            &sf,
                            &context,
                            NULL,
                            SymFunctionTableAccess64,
                            SymGetModuleBase64,
                            NULL ) )
      break;
    if( 0 == sf.AddrFrame.Offset ) // Basic sanity check to make sure
      break;                      // the frame is OK.  Bail if not.
    frames[i].address = (intptr_t) sf.AddrPC.Offset;
    frames[i].offset = (intptr_t) sf.AddrFrame.Offset;
  }
  return i;
}

//static 
intptr_t
getProgramCounters(FrameAddress* frames, intptr_t pccount, CONTEXT& context)
{
  if (_init_sys_libs() == false)
    return 0;
  
  DWORD dwMachineType = 0;
  memset(frames, 0, sizeof(FrameAddress) * pccount);
  STACKFRAME sf;
  memset(&sf, 0, sizeof(sf));
  
  HANDLE currentThread = GetCurrentThread();
  //CONTEXT context;
  //context.ContextFlags = CONTEXT_FULL; 
  
 #ifdef _M_IX86
    // Initialize the STACKFRAME structure for the first call.  This is only
    // necessary for Intel CPUs, and isn't mentioned in the documentation.
    sf.AddrPC.Offset       = context.Eip;
    sf.AddrPC.Mode         = AddrModeFlat;
    sf.AddrStack.Offset    = context.Esp;
    sf.AddrStack.Mode      = AddrModeFlat;
    sf.AddrFrame.Offset    = context.Ebp;
    sf.AddrFrame.Mode      = AddrModeFlat;

    dwMachineType = IMAGE_FILE_MACHINE_I386;
#endif
  intptr_t i ;
  for (i = 0; i < pccount; ++i)
  {
        // Get the next stack frame
    if ( ! StackWalk(  dwMachineType,
                            _currentProcess,
                            NULL,
                            &sf,
                            &context,
                            0,
                            SymFunctionTableAccess,
                            SymGetModuleBase,
                            0 ) )
      break;
    if ( 0 == sf.AddrFrame.Offset ) // Basic sanity check to make sure
      break;                      // the frame is OK.  Bail if not.
    frames[i].address = (int)sf.AddrPC.Offset;
    frames[i].offset = (int)sf.AddrFrame.Offset;
  }
  return i;
}

static
bool 
get_logical_address(void * addr, char * szModule, uintptr_t len, uintptr_t & section, uintptr_t & offset)
{
  MEMORY_BASIC_INFORMATION mbi;
  if (VirtualQuery( addr, &mbi, sizeof(mbi)) == FALSE)
    return false;

  uintptr_t hMod = (uintptr_t) mbi.AllocationBase;
  if (GetModuleFileName((HMODULE) hMod, szModule, (DWORD) len) == FALSE)
    return false;

  PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;
  if (pDosHdr == 0)
    return false;

  PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);
  
  PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );
  
  uintptr_t rva = (uintptr_t) addr - hMod;
  
  for (intptr_t i = 0; i < pNtHdr->FileHeader.NumberOfSections; i++, pSection++)
  {
    uintptr_t sectionStart = pSection->VirtualAddress;
    uintptr_t sectionEnd = sectionStart + max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);
    
    if ((rva >= sectionStart) && (rva <= sectionEnd))
    {
      section = DWORD(i + 1);
      offset = rva - sectionStart;
      return true;
    }
  }
  return false;
}

bool querySymbolValue(PSYMBOL_INFO pSym, DbgFrame& frame);

static
BOOL CALLBACK
dbgutils_enumerateSymbolsCallback(PSYMBOL_INFO  pSymInfo, ULONG SymbolSize, PVOID  UserContext)
{
  //char szBuffer[2048];
  DbgFrame *dbframe = (DbgFrame*)UserContext;
  __try
  {
    querySymbolValue( pSymInfo, *((DbgFrame*)UserContext));
  }
  __except( 1 )
  {
    dbframe->functionName = "pdbutils: Unexpected Failure in Parsing PDB-Info";
    DOUTNL("punting on symbol: " << pSymInfo->Name);
  }
  return TRUE;
}

static
void
dbgutils_getFrame(const FrameAddress& frameAddr, DbgFrame& frame)
{
  BYTE symbolBuffer[sizeof(SYMBOL_INFO) + 1024];
  PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)symbolBuffer;
  pSymbol->SizeOfStruct = sizeof(symbolBuffer);
  pSymbol->MaxNameLen = 1024;
  DOUTNL("Frame: " << (void*)frameAddr.offset << ", " << (void*)frameAddr.offset);
  
  if (frame.queryFlags & DbgFrameGetFuncName)
  {
    DWORD64 symDisplacement = 0;
    if (SymFromAddr(_currentProcess, frameAddr.address, &symDisplacement, pSymbol))
    {
      frame.displacement = symDisplacement;
      frame.functionName  =  pSymbol->Name;
      if (frame.functionName.startsWith("pdbutils") == true)
      {
        frame.functionName = frame.functionName + " <internal hidden debuging function>";
        return;
      }
      DOUTNL("Name: " << pSymbol->Name);
    }
  }
  if (frame.queryFlags & DbgFrameGetLibary)
    {
      char szModule[MAX_PATH] = "";
      uintptr_t section = 0, offset = 0;
      if (get_logical_address((void*)frameAddr.address, szModule, sizeof(szModule), section, offset ) == true)
      {
        frame.libraryName = szModule;
      }
      DOUTNL("libname: " << szModule);
    }
  
  if (frame.queryFlags & DbgFrameGetSourcePos)
  {

    // Get the source line for this stack frame entry
    IMAGEHLP_LINE64 lineInfo = { sizeof(IMAGEHLP_LINE) };
    DWORD dwLineDisplacement;
    if ( SymGetLineFromAddr64( _currentProcess, frameAddr.address,
                                &dwLineDisplacement, &lineInfo ) )
    {
      frame.sourceFileName = lineInfo.FileName;
      frame.sourceFileLine = lineInfo.LineNumber;
      //DOUTNL("source: " << lineInfo.FileName << "(" << lineInfo.LineNumber << ")");
    }
  }
  if (frame.queryFlags & DbgFrameGetArguments)
  {
    
    IMAGEHLP_STACK_FRAME imagehlpStackFrame;
    memset(&imagehlpStackFrame, 0, sizeof(imagehlpStackFrame));
    imagehlpStackFrame.InstructionOffset = frameAddr.address;

    SymSetContext(_currentProcess, &imagehlpStackFrame, 0);
    
    frame.frame = frameAddr;
    SymEnumSymbols(_currentProcess, 0, 0, dbgutils_enumerateSymbolsCallback, &frame ); 
  }
  if (frame.queryFlags & DbgFrameGetGlobals)
  {
    SymEnumSymbols(GetCurrentProcess(),(ULONG64)GetModuleHandle(NULL),0,dbgutils_enumerateSymbolsCallback, &frame);
  }
  
}

//static 
void 
getFrame(const FrameAddress& frameAddr, DbgFrame& frame, intptr_t flags)
{
  if (_init_sys_libs() == false)
    return;
  frame.queryFlags = flags;
  MaxPointerRecursion = DbgMaxPointerMask & flags;
  dbgutils_getFrame(frameAddr, frame);

}

void
getGlobalFrame(DbgFrame& frame, intptr_t flags)
{
  FrameAddress fa; fa.offset = 0; fa.address = 0;
  frame.queryFlags = flags | DbgFrameGetGlobals;
  dbgutils_getFrame(fa, frame);
}

DbgString 
getFullBackTrace(intptr_t skipCount, void* threadHandle)
{
  return getBackTrace(DbgFrameGetAll, skipCount == -1 ? -1 : skipCount + 1, threadHandle);
}

void
getBackTrace(DbgStream& os, intptr_t flags, intptr_t skipCount, PCONTEXT pCtx)
{
   if (_init_sys_libs() == false)
    return;
  FrameAddress pcs[128];
  memset(pcs, 0, sizeof(pcs));
  
     
  intptr_t count = getProgramCounters(pcs, 128, *pCtx);
  skipCount = -1;
  //os << "Crash StackFrame PC: " << count << NL;
  for (intptr_t i = skipCount + 1; i < count; ++i)
  {
    DbgFrame frame;
    getFrame(pcs[i], frame, flags);
    frame.toString(os, flags);
  }
}

DbgString 
renderBackTrace(FrameAddress* frameAddresses, intptr_t frameCount, intptr_t flags)
{
  DbgStream ss;
  for (intptr_t i = 0; i < frameCount; ++i)
  {
    DbgFrame frame;
    getFrame(frameAddresses[i], frame, flags);
    frame.toString(ss, flags);
  }
  return ss.str().c_str();
}

DbgString 
getBackTrace(intptr_t flags, intptr_t skipCount, void* threadHandle)
{
   if (_init_sys_libs() == false)
    return "";

  FrameAddress pcs[128];
  memset(pcs, 0, sizeof(pcs));
  DbgStream ss;
  
  intptr_t count = getProgramCounters(pcs, 128, threadHandle);
  for (intptr_t i = skipCount; i < count; ++i)
  {
    DbgFrame frame;
    getFrame(pcs[i], frame, flags);
    frame.toString(ss, flags);
  }
  return ss.str();
}

DbgType 
getTypeFromAddress(void* addr)
{
  DbgType erg;
  if (_init_sys_libs() == false)
    return erg;
  FrameAddress pcs[128];
  memset(pcs, 0, sizeof(pcs));
  intptr_t count = getProgramCounters(pcs, 128);
  for (intptr_t i = 2; i < count; ++i)
  {
    DbgFrame frame;
    getFrame(pcs[i], frame, DbgFrameGetAll + 3);
    if (frame.findElementForAddress(addr, erg) == true)
      return erg;
    
  }
  
  return erg;
}

DbgString
getCrashDumpFileName(const DbgString& suffix)
{
  char moduleFilename[MAX_PATH];
  memset(moduleFilename, 0, sizeof(moduleFilename));
  HMODULE hm = GetModuleHandle(NULL);
  GetModuleFileName(hm, moduleFilename, MAX_PATH);
  DbgString mfile = moduleFilename;
  if (DBGSTRING_SIZE(mfile) <= 5)
  {
    DOUT("dumpfilename too short: " << mfile);
    return mfile;
  }
  mfile = DBGSTRING_SUBSTR2(mfile, 0, DBGSTRING_SIZE(mfile) - 4);

  //intptr_t lbs = DBGSTRING_LASTINDEX(mfile, "\\");
  SYSTEMTIME st;
  GetLocalTime(&st);
  DbgStream ss;
  char buffer[128];
  sprintf(buffer, "_%04u-%02u-%02u_%02u_%02u_%02u", st.wYear, st.wMonth, st.wDay, 
                                                    st.wHour, st.wMinute, st.wSecond);

  DbgString fileBase = mfile + buffer;
  DbgString str = fileBase + suffix;
  if (_access(str, F_OK) == 0)
  {
    for (intptr_t i = 1; i < 1000; ++i)
    {
      DbgStream oss;
      oss << fileBase << "_" << i << suffix;
      if (_access(oss.str().c_str(), F_OK) != 0)
        return oss.str().c_str();
    }
  }
  DOUT("dumpfile: " << str);
  return str;
}

DbgString 
getExceptionString( DWORD dwCode )
{
  #define EXCEPTION( x ) case EXCEPTION_##x: return #x;
  switch ( dwCode )
  {
    EXCEPTION( ACCESS_VIOLATION )
    EXCEPTION( DATATYPE_MISALIGNMENT )
    EXCEPTION( BREAKPOINT )
    EXCEPTION( SINGLE_STEP )
    EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
    EXCEPTION( FLT_DENORMAL_OPERAND )
    EXCEPTION( FLT_DIVIDE_BY_ZERO )
    EXCEPTION( FLT_INEXACT_RESULT )
    EXCEPTION( FLT_INVALID_OPERATION )
    EXCEPTION( FLT_OVERFLOW )
    EXCEPTION( FLT_STACK_CHECK )
    EXCEPTION( FLT_UNDERFLOW )
        EXCEPTION( INT_DIVIDE_BY_ZERO )
        EXCEPTION( INT_OVERFLOW )
        EXCEPTION( PRIV_INSTRUCTION )
        EXCEPTION( IN_PAGE_ERROR )
        EXCEPTION( ILLEGAL_INSTRUCTION )
        EXCEPTION( NONCONTINUABLE_EXCEPTION )
        EXCEPTION( STACK_OVERFLOW )
        EXCEPTION( INVALID_DISPOSITION )
        EXCEPTION( GUARD_PAGE )
        EXCEPTION( INVALID_HANDLE )
    }

    // If not one of the "known" exceptions, try to get the string
    // from NTDLL.DLL's message table.

    char szBuffer[512] = { 0 };

    FormatMessage( FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                   GetModuleHandle( "NTDLL.DLL"),
                   dwCode, 0, szBuffer, sizeof( szBuffer ), 0 );

    return szBuffer;
}

void 
dbgutils_createThreadDump(DbgStream& ss, intptr_t skipCount, intptr_t threadId, HANDLE threadHandle)
{
  //ss << "ScipCount2: " << skipCount << NL;
  ss << "==================================================================" NL
     << "ThreadID: " << threadId << "  ThreadHandle: " << (void*)threadHandle << NL
     ;
  ss << getFullBackTrace(skipCount == -1 ? -1 : skipCount + 1, threadHandle);
}

void dbgutils_suspendThreadsAndAscDump(DbgStream& ss, intptr_t dumpFlags, intptr_t scipCount, PCONTEXT pCtx);


void writeAscCrashDump(const char* filename, const char* content, intptr_t length)
{
  DbgString fname;
  if (filename == 0)
    fname = getCrashDumpFileName(".btc");
  else
    fname = filename;
  
  //fname = fname + ".btc";
  HANDLE hFile = CreateFile ( DBGSTRING2CHARPTR(fname), GENERIC_READ | GENERIC_WRITE    ,
                              0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile != NULL)
  {
    DWORD written = 0;
    WriteFile(hFile, content, (DWORD) length, &written, NULL);
    CloseHandle(hFile);
  }
  else
  {
    DOUTNL("Cannot open file for write: " << fname);
  }
}

void 
dbgutils_createBinaryDump(const char* filename, PEXCEPTION_POINTERS pExceptionInfo)
{
   DbgString fname;
  if (filename == 0)
    fname = getCrashDumpFileName(".dmp");
  else
    fname = filename;
  DbgString rfilename = fname;// + ".dmp";

    HANDLE hFile = CreateFile ( DBGSTRING2CHARPTR(rfilename), GENERIC_READ | GENERIC_WRITE    ,
                                  0                              ,
                                  NULL                           ,
                                  CREATE_ALWAYS                  ,
                                  FILE_ATTRIBUTE_NORMAL          ,
                                  NULL                            );

    MINIDUMP_EXCEPTION_INFORMATION mexi;
    mexi.ThreadId = GetCurrentThreadId();
    mexi.ExceptionPointers = pExceptionInfo;
    mexi.ClientPointers = TRUE;
    MiniDumpWriteDump (GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithDataSegs, pExceptionInfo == NULL ? NULL : &mexi, NULL, NULL);
    CloseHandle(hFile);
}

void
dbgutils_createDump(const char* filename, intptr_t flags, PEXCEPTION_POINTERS pExceptionInfo, intptr_t skipCount)
{
  
 
  
  if (flags & DbgCrashDumpCreateBinaryDump)
  {
    dbgutils_createBinaryDump(filename, pExceptionInfo);
  }
  PCONTEXT pCtx = NULL;
  if ((flags & DbgCrashDumpCreateTextDump) == DbgCrashDumpCreateTextDump)
  {
    
    DbgStream ss;
    
    if (pExceptionInfo != NULL)
    {
      PEXCEPTION_RECORD pExceptionRecord = pExceptionInfo->ExceptionRecord;
      ss << "Exception code: " << (void*)(uintptr_t)pExceptionRecord->ExceptionCode << NL
        << getExceptionString(pExceptionRecord->ExceptionCode) << NL;

    // Now print information about where the fault occured
    TCHAR szFaultingModule[MAX_PATH];
    uintptr_t section, offset;
    get_logical_address(  pExceptionRecord->ExceptionAddress,
                        szFaultingModule,
                        sizeof( szFaultingModule ),
                        section, offset );

    ss << "Fault address:  " << pExceptionRecord->ExceptionAddress << " " 
      << section << " " << offset << " " << szFaultingModule << NL;

    pCtx = pExceptionInfo->ContextRecord;

    // Show the registers
#ifdef _M_IX86  // X86 Only!
    ss << NL"Registers:"NL
        << "EAX: " << pCtx->Eax << NL
        << "EBX: " << pCtx->Ebx << NL
        << "ECX: " << pCtx->Ecx << NL
        << "EDX: " << pCtx->Edx << NL
        << "ESI: " << pCtx->Esi << NL
        << "EDI: " << pCtx->Edi << NL
        << "CS:EIP: " << pCtx->SegCs << ":" << pCtx->Eip << NL
        << "SS:ESP: " << pCtx->Esp << ":" << pCtx->Ebp << NL
        << "DS: " << pCtx->SegDs << NL
        << "ES: " << pCtx->SegEs << NL
        << "FS: " << pCtx->SegFs << NL
        << "GS: " << pCtx->SegGs << NL
        << "Flags: " << pCtx->EFlags << NL
        ;
#endif

    }
    dbgutils_suspendThreadsAndAscDump(ss, flags, skipCount + 1, pCtx);
    writeAscCrashDump(filename, ss.str().c_str(), ss.str().size());
  }
}


typedef WINBASEAPI HANDLE WINAPI OpenThreadFunc(DWORD dwDesiredAccess, BOOL bInheridedHandle, DWORD dwThreadID);
HANDLE openThread(DWORD dwDesiredAccess, BOOL bInheridedHandle, DWORD dwThreadID)
{
  static HINSTANCE kernel32 = NULL;
  static FARPROC openthread = NULL;
  if (kernel32 == NULL)
  {
    kernel32 = LoadLibrary("kernel32.dll");
    if (kernel32 == NULL)
      return NULL;
    openthread = GetProcAddress(kernel32, "OpenThread");
    if (openthread == NULL)
    {
      FreeLibrary(kernel32);
      kernel32 = NULL;
      return NULL;
    }
  }
  return ((OpenThreadFunc*)(void*)openthread)(dwDesiredAccess, bInheridedHandle, dwThreadID);
}

struct DbgThreadInfo
{
    intptr_t threadId;
    HANDLE threadHandle;

    DbgThreadInfo(intptr_t tid, HANDLE th) 
    : threadId(tid)
    , threadHandle(th) 
    {} 
};

void
dbgutils_suspendThreadsAndAscDump(DbgStream& ss, intptr_t dumpFlags, intptr_t skipCount, PCONTEXT pCtx)
{
  DWORD pid = GetCurrentProcessId();
  intptr_t currentThreadId = GetCurrentThreadId();
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);
  if (snapshot == INVALID_HANDLE_VALUE)
  {
    // ### create standard dump of current thread
    ss << "<< Cannot take thread snapshot >> " << NL;
    return;
  }
  
  DbgVector<DbgThreadInfo> threads;

  THREADENTRY32 te = { sizeof(te) };
  BOOL ok = Thread32First(snapshot, &te);
  for ( ; ok == TRUE; ok = Thread32Next(snapshot, &te))
  {
    if (te.th32OwnerProcessID != pid)
      continue;
    //OpenProcess
    HANDLE hThread = openThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT , FALSE, te.th32ThreadID);
    if (hThread == NULL)
    {
      ss << "<< Cannot open Thread " << te.th32ThreadID << NL;
      continue;
    }
    threads.push_back(DbgThreadInfo(te.th32ThreadID, hThread));
    if (currentThreadId != currentThreadId)
    {
      SuspendThread(hThread);
    }
  }
  if (pCtx != NULL)
  {
     ss << "==================================================================" NL
     << "Crashing Thread: " << NL
     ;
    getBackTrace(ss, DbgFrameGetAll, skipCount == -1 ? -1 : skipCount + 1, pCtx);
  }

  intptr_t i;
  for (i = 0; i < threads.size(); ++i)
  {
    intptr_t sc = skipCount + 1;
    
    if (threads[i].threadId != currentThreadId)
      sc = -1;
    if ((DbgCrashDumpAllThreads & dumpFlags) || threads[i].threadId != currentThreadId)
      dbgutils_createThreadDump(ss, sc, threads[i].threadId, threads[i].threadHandle); 
    //std::cout << "ThreadDump: " << te.th32ThreadID << std::endl;
    //std::cout << ss.str() << std::endl;
  }
  if (dumpFlags & DbgFrameGetGlobals)
  {
    ss << "==================================================================" NL
       << "Globals:" NL NL;
    DbgFrame globalframe;
    getGlobalFrame(globalframe, DbgFrameGetAll);
    globalframe.toString(ss, DbgFrameGetAll);
  }
  for (i = 0; i < threads.size(); ++i)
  {
    ResumeThread(threads[i].threadHandle);
    CloseHandle(threads[i].threadHandle);
  }
  CloseHandle(snapshot);
}


//static 
void 
createDump(const char* filename, intptr_t crashdumpflags, intptr_t skipCount)
{
  DbgStream ss;
  if (crashdumpflags & DbgCrashDumpCreateBinaryDump)
  {
    dbgutils_createBinaryDump(filename, NULL);
  }
  if (crashdumpflags & DbgCrashDumpCreateTextDump)
  {
    dbgutils_suspendThreadsAndAscDump(ss, crashdumpflags, skipCount + 1, NULL);
    DOUTNL("Write Asc CrashDump:");
    writeAscCrashDump(filename, ss.str().c_str(), ss.str().size());
  }
}


LPTOP_LEVEL_EXCEPTION_FILTER previousExFilter = NULL;

static intptr_t gCrashDumpFlags = 0;

LONG __stdcall dbgutils_UnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
  dbgutils_createDump(0, gCrashDumpFlags, pExceptionInfo, -1);
  if (previousExFilter != NULL)
    return previousExFilter(pExceptionInfo);
  else
    return EXCEPTION_CONTINUE_SEARCH;
}

//static 
void 
installCrashDumpHook(intptr_t crashdumpflags)
{
  if (previousExFilter != NULL)
    return;
  gCrashDumpFlags = crashdumpflags;
  previousExFilter = SetUnhandledExceptionFilter(dbgutils_UnhandledExceptionFilter);
}

//static
void
uninstallCrashDumpHook()
{
  if (previousExFilter == NULL)
    return;
  SetUnhandledExceptionFilter(previousExFilter);
}

} // namespace pdbutils {

#endif
