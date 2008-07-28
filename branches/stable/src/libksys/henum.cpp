#/*-
 * Copyright 2006-2007 Guram Dukashvili
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
/*#if HAVE_NTDLL_H
#include <ntdll.h>
#else
#include <adicpp/ntddk/ntdll.h>
#endif*/
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
#define NT_SUCCESS(Status)				((LONG)(Status) >= 0)
#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)
static inline PVOID getInfoTable(IN ULONG ATableType,HANDLE hHeap)
{
	ULONG    mSize = 0x8000;
	PVOID    mPtr;
	NTSTATUS status;
	do {
		mPtr = HeapAlloc(hHeap, 0, mSize);
		if( mPtr == NULL ) return NULL;
		memset(mPtr, 0, mSize);
		status = NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS) ATableType, mPtr, mSize, NULL); 
		if( status == STATUS_INFO_LENGTH_MISMATCH ){
			HeapFree(hHeap,0,mPtr);
			mSize = mSize * 2;
		}
	} while( status == STATUS_INFO_LENGTH_MISMATCH );
	if( NT_SUCCESS(status) ) return mPtr;
	HeapFree(hHeap, 0, mPtr);
	return NULL;
}
//---------------------------------------------------------------------------
#pragma warning(disable : 4200)
typedef VOID		*POBJECT;
typedef struct _SYSTEM_HANDLE
{
	ULONG		uIdProcess;
	UCHAR		ObjectType;    // OB_TYPE_* (OB_TYPE_TYPE, etc.)
	UCHAR		Flags;         // HANDLE_FLAG_* (HANDLE_FLAG_INHERIT, etc.)
	USHORT		Handle;
	POBJECT		pObject;
	ACCESS_MASK	GrantedAccess;
} SYSTEM_HANDLE, *PSYSTEM_HANDLE;
//---------------------------------------------------------------------------
typedef struct _SYSTEM_HANDLE_INFORMATION
{
	ULONG			uCount;
	SYSTEM_HANDLE	aSH[];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;
//---------------------------------------------------------------------------
typedef enum _SYSTEMINFOCLASS
{
	SystemBasicInformation,             // 0x002C
	SystemProcessorInformation,         // 0x000C
	SystemPerformanceInformation,       // 0x0138
	SystemTimeInformation,              // 0x0020
	SystemPathInformation,              // not implemented
	SystemProcessInformation,           // 0x00C8+ per process
	SystemCallInformation,              // 0x0018 + (n * 0x0004)
	SystemConfigurationInformation,     // 0x0018
	SystemProcessorCounters,            // 0x0030 per cpu
	SystemGlobalFlag,                   // 0x0004 (fails if size != 4)
	SystemCallTimeInformation,          // not implemented
	SystemModuleInformation,            // 0x0004 + (n * 0x011C)
	SystemLockInformation,              // 0x0004 + (n * 0x0024)
	SystemStackTraceInformation,        // not implemented
	SystemPagedPoolInformation,         // checked build only
	SystemNonPagedPoolInformation,      // checked build only
	SystemHandleInformation,            // 0x0004  + (n * 0x0010)
	SystemObjectTypeInformation,        // 0x0038+ + (n * 0x0030+)
	SystemPageFileInformation,          // 0x0018+ per page file
	SystemVdmInstemulInformation,       // 0x0088
	SystemVdmBopInformation,            // invalid info class
	SystemCacheInformation,             // 0x0024
	SystemPoolTagInformation,           // 0x0004 + (n * 0x001C)
	SystemInterruptInformation,         // 0x0000, or 0x0018 per cpu
	SystemDpcInformation,               // 0x0014
	SystemFullMemoryInformation,        // checked build only
	SystemLoadDriver,                   // 0x0018, set mode only
	SystemUnloadDriver,                 // 0x0004, set mode only
	SystemTimeAdjustmentInformation,    // 0x000C, 0x0008 writeable
	SystemSummaryMemoryInformation,     // checked build only
	SystemNextEventIdInformation,       // checked build only
	SystemEventIdsInformation,          // checked build only
	SystemCrashDumpInformation,         // 0x0004
	SystemExceptionInformation,         // 0x0010
	SystemCrashDumpStateInformation,    // 0x0004
	SystemDebuggerInformation,          // 0x0002
	SystemContextSwitchInformation,     // 0x0030
	SystemRegistryQuotaInformation,     // 0x000C
	SystemAddDriver,                    // 0x0008, set mode only
	SystemPrioritySeparationInformation,// 0x0004, set mode only
	SystemPlugPlayBusInformation,       // not implemented
	SystemDockInformation,              // not implemented
	SystemPowerInfo,             // 0x0060 (XP only!)
	SystemProcessorSpeedInformation,    // 0x000C (XP only!)
	SystemTimeZoneInformation,          // 0x00AC
	SystemLookasideInformation,         // n * 0x0020
	SystemSetTimeSlipEvent,
    SystemCreateSession,				// set mode only
    SystemDeleteSession,				// set mode only
    SystemInvalidInfoClass1,			// invalid info class
    SystemRangeStartInformation,		// 0x0004 (fails if size != 4)
    SystemVerifierInformation,
    SystemAddVerifier,
    SystemSessionProcessesInformation,	// checked build only
	MaxSystemInfoClass
} SYSTEMINFOCLASS, *PSYSTEMINFOCLASS;
//---------------------------------------------------------------------------
static inline UCHAR getFileHandleType(HANDLE hHeap)
{
	HANDLE                     hFile;
	PSYSTEM_HANDLE_INFORMATION Info;
	ULONG                      r;
	UCHAR                      Result = 0;

	hFile = CreateFile("NUL", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
	if( hFile != INVALID_HANDLE_VALUE ){
		Info = (PSYSTEM_HANDLE_INFORMATION) getInfoTable(SystemHandleInformation,hHeap);
    if( Info ){
			for( r = 0; r < Info->uCount; r++ ){
				if( Info->aSH[r].Handle == (USHORT) hFile && Info->aSH[r].uIdProcess == GetCurrentProcessId()){
					Result = Info->aSH[r].ObjectType;
					break;
				}
			}
	    HeapFree(hHeap, 0, Info);
		}
		CloseHandle(hFile);
	}
	return Result;
}
//---------------------------------------------------------------------------
typedef struct _FILE_NAME_INFORMATION
{
    ULONG	FileNameLength;
    WCHAR	FileName[1];
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;
//---------------------------------------------------------------------------
typedef struct _NM_INFO {
	HANDLE  hFile;
	FILE_NAME_INFORMATION Info;
	WCHAR Name[MAX_PATH];
} NM_INFO, *PNM_INFO;
//---------------------------------------------------------------------------
typedef enum _FILE_INFORMATION_CLASS
{
    FileDirectoryInformation       = 1,
    FileFullDirectoryInformation, // 2
    FileBothDirectoryInformation, // 3
    FileBasicInformation,         // 4
    FileStandardInformation,      // 5
    FileInternalInformation,      // 6
    FileEaInformation,            // 7
    FileAccessInformation,        // 8
    FileNameInformation,          // 9
    FileRenameInformation,        // 10
    FileLinkInformation,          // 11
    FileNamesInformation,         // 12
    FileDispositionInformation,   // 13
    FilePositionInformation,      // 14
    FileFullEaInformation,        // 15
    FileModeInformation,          // 16
    FileAlignmentInformation,     // 17
    FileAllInformation,           // 18
    FileAllocationInformation,    // 19
    FileEndOfFileInformation,     // 20
    FileAlternateNameInformation, // 21
    FileStreamInformation,        // 22
    FilePipeInformation,          // 23
    FilePipeLocalInformation,     // 24
    FilePipeRemoteInformation,    // 25
    FileMailslotQueryInformation, // 26
    FileMailslotSetInformation,   // 27
    FileCompressionInformation,   // 28
    FileObjectIdInformation,      // 29
    FileCompletionInformation,    // 30
    FileMoveClusterInformation,   // 31
    FileInformationReserved32,    // 32
    FileInformationReserved33,    // 33
    FileNetworkOpenInformation,   // 34
    FileMaximumInformation
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;
//---------------------------------------------------------------------------
typedef NTSYSAPI NTSTATUS (NTAPI * NtQueryInformationFile)(
    IN HANDLE					FileHandle,
    OUT PIO_STATUS_BLOCK		IoStatusBlock,
    OUT PVOID					FileInformation,
    IN ULONG					Length,
    IN FILE_INFORMATION_CLASS	FileInformationClass
);
static DWORD WINAPI getFileNameThread(PVOID lpParameter)
{
	PNM_INFO        NmInfo = (PNM_INFO) lpParameter;
	IO_STATUS_BLOCK IoStatus;
  NtQueryInformationFile q = (NtQueryInformationFile) GetProcAddress(GetModuleHandle("ntdll.dll"),"NtQueryInformationFile");
	q(NmInfo->hFile, &IoStatus, &NmInfo->Info, sizeof(NM_INFO) - sizeof(HANDLE), FileNameInformation);
	return 0;
}
//---------------------------------------------------------------------------
utf8::String getFileNameByHandle(HANDLE hFile)
{
	HANDLE hHeap = GetProcessHeap();
	PNM_INFO Info = (PNM_INFO) HeapAlloc(hHeap, 0, sizeof(NM_INFO));
	Info->hFile = hFile;
  HANDLE hThread = CreateThread(NULL, 0, getFileNameThread, Info, 0, NULL);
	if( WaitForSingleObject(hThread,INFINITE) == WAIT_TIMEOUT ) TerminateThread(hThread,0);
	CloseHandle(hThread);
  utf8::String name(Info->Info.FileName,Info->Info.FileNameLength >> 1);
	HeapFree(hHeap,0,Info);
  return name;
}
//---------------------------------------------------------------------------
/*void main()
{
	PSYSTEM_HANDLE_INFORMATION Info;
	ULONG                      r;
	CHAR                       Name[MAX_PATH];
	HANDLE                     hProcess, hFile;
	UCHAR                      ObFileType;

	hHeap = GetProcessHeap();

	ObFileType = GetFileHandleType();

	Info = GetInfoTable(SystemHandleInformation);

	if (Info)
	{
		for (r = 0; r < Info->uCount; r++)
		{
			if (Info->aSH[r].ObjectType == ObFileType)
			{
				hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, Info->aSH[r].uIdProcess);

				if (hProcess)
				{
					if (DuplicateHandle(hProcess, (HANDLE)Info->aSH[r].Handle,
						GetCurrentProcess(), &hFile, 0, FALSE, DUPLICATE_SAME_ACCESS))
					{
						GetFileName(hFile, Name);

						printf("%s\n", Name);

						CloseHandle(hFile);
					}
					CloseHandle(hProcess);
				}
			}
		}
		HeapFree(hHeap, 0, Info);
	}
}
*/
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
