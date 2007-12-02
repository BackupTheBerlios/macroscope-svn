/*-
 * Copyright (C) 2005-2007 Guram Dukashvili. All rights reserved.
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
// msmail1cImpl.cpp : Implementation of Cmsmail1c
//------------------------------------------------------------------------------
#define _VERSION_C_AS_HEADER_
#include "version.c"
#undef _VERSION_C_AS_HEADER_
//------------------------------------------------------------------------------
#include "stdafx.h"
#include "msmail1cImpl.h"
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Cmsmail1c::~Cmsmail1c() throw()
{
}
//------------------------------------------------------------------------------
Cmsmail1c::Cmsmail1c()
{
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void Cmsmail1c::msmail1c::ServerBusyThread::threadBeforeWait()
{
  terminate();
  sem_.post();
}
//------------------------------------------------------------------------------
BOOL CALLBACK Cmsmail1c::msmail1c::ServerBusyThread::enumWindowsProc(HWND hwnd,LPARAM lParam)
{
  ServerBusyThread * thread = (ServerBusyThread *) lParam;
  try {
    bool match = false;
    int i, l;
    thread->text_ = uint8_t(0);
    if( isWin9x() ){
      l = GetWindowTextLength(hwnd);
      if( uintptr_t(l + 1) > thread->text_.count() ) thread->text_.resize(l + 1);
      SetLastError(ERROR_SUCCESS);
      l = GetWindowTextA(hwnd,(LPSTR) thread->text_.ptr(),l + 1);
      if( l > 0 ){
        const char * text = (const char *) thread->text_.ptr();
        for( i = 0; i <= l && text[i] == thread->textToFindA_[i]; i++ );
        match = text[i] == '\0';
      }
    }
    else {
      l = GetWindowTextLength(hwnd);
      if( (l + 1) * sizeof(WCHAR) > thread->text_.count() ) thread->text_.resize((l + 1) * sizeof(WCHAR));
      SetLastError(ERROR_SUCCESS);
      l = GetWindowTextW(hwnd,(LPWSTR) thread->text_.ptr(),(l + 1) * sizeof(WCHAR));
      if( l > 0 ){
        const wchar_t * text = (const wchar_t *) thread->text_.ptr();
        for( i = 0; i <= l && text[i] == thread->textToFindW_[i]; i++ );
        match = text[i] == L'\0';
      }
    }
    if( match ){
      if( thread->count_ == thread->windows_.count() )
        thread->windows_.add(hwnd); else thread->windows_[thread->count_] = hwnd;
      thread->count_++;
    }
  }
  catch( ... ){}
  return TRUE;
}
//------------------------------------------------------------------------------
void Cmsmail1c::msmail1c::ServerBusyThread::threadExecute()
{
  for(;;){
    sem_.timedWait(3u * 1000000u);
    if( terminated_ ) break;
    count_ = 0;
    if( isWin9x() )
      textToFindA_ = "—ервер зан€т";
    else
      textToFindW_ = L"—ервер зан€т";
    if( EnumWindows((WNDENUMPROC) enumWindowsProc,(LPARAM) this) != FALSE ){
      try {
        Array<HWND> windows(windows_);
        for( intptr_t i = count_ - 1; i >= 0; i-- ){
          if( isWin9x() )
            textToFindA_ = "ѕо&вторить";
          else
            textToFindW_ = L"ѕо&вторить";
          count_ = 0;
          if( EnumChildWindows(windows[i],(WNDENUMPROC) enumWindowsProc,(LPARAM) this) == FALSE ) continue;
          if( count_ == 0 ) continue;
          if( isWin9x() ){
            PostMessageA(windows_[0],WM_SYSKEYDOWN,'в',1 | (1 << 29));
            PostMessageA(windows_[0],WM_SYSCHAR,'в',1 | (1 << 29));
            PostMessageA(windows_[0],WM_SYSKEYUP,'в',1 | (1 << 29));
          }
          else {
            PostMessageW(windows_[0],WM_SYSKEYDOWN,L'в',1 | (1 << 29));
            PostMessageW(windows_[0],WM_SYSCHAR,L'в',1 | (1 << 29));
            PostMessageW(windows_[0],WM_SYSKEYUP,L'в',1 | (1 << 29));
          }
          //PostMessage(windows_[0],WM_SETFOCUS,NULL,NULL);
          //PostMessage(windows_[0],WM_KEYDOWN,VK_RETURN,1);
          //PostMessage(windows_[0],WM_CHAR,VK_RETURN,1);
          //PostMessage(windows_[0],WM_KEYUP,VK_RETURN,1);
        }
      }
      catch( ... ){}
    }
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Cmsmail1c::msmail1c::~msmail1c()
{
  client_.close();
  if( pAsyncEvent_ != NULL ){
    pAsyncEvent_->Release();
    pAsyncEvent_ = NULL;
  }
  if( pBackConnection_ != NULL ) pBackConnection_->Release();
  for( intptr_t i = hashedArrays_.count() - 1; i >= 0; i-- )
    hashedArrays_[i].drop();
  hashedArrays_.clear();
  functions_.drop();
}
//------------------------------------------------------------------------------
Cmsmail1c::msmail1c::msmail1c() :
  pBackConnection_(NULL),
  pAsyncEvent_(NULL),
  active_(false),
  lastError_(0),
  functionsAutoDrop_(functions_)
{
}
//------------------------------------------------------------------------------
// IInitDone Methods
//------------------------------------------------------------------------------
void * Cmsmail1c::oldDBENG32LockFile_;
/*void * Cmsmail1c::oldBKENDGetProcAddress_;
void * Cmsmail1c::oldMSVCRTLockFile_;
void * Cmsmail1c::oldMSVCR71LockFile_;
void * Cmsmail1c::oldMFC42LockFile_;
Cmsmail1c::ImportedEntry Cmsmail1c::oldSEVENGetProcAddress_;*/
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::Init(LPDISPATCH pBackConnection)
{
//  stdErr.enableDebugLevel(9);
  try {
    msmail1c_ = newObject<msmail1c>();
    static const wchar_t * fn[] = {
      L"LockFile", L"Ѕлокировать‘айл",
      L"UnlockFile", L"–азблокировать‘айл",
      L"GetLastError", L"ѕолучить одќшибки",
      L"Sleep", L"—пать",
      L"SleepIn", L"—пать¬",
      L"TextToFile", L"“екст¬‘айл",
      L"NewMessage", L"Ќовое—ообщение",
      L"SetMessageAttribute", L"”становитьјтрибут—ообщени€",
      L"GetMessageAttribute", L"ѕолучитьјтрибут—ообщени€",
      L"SendMessage", L"ѕослать—ообщение",
      L"RemoveMessage", L"”далить—ообщение",
      L"GetDB", L"ѕолучить»Ѕ",
      L"CopyMessage", L"—копировать—ообщение",
      L"RemoveMessageAttribute", L"”далитьјтрибут—ообщени€",
      L"DebugMessage", L"ќтладочное—ообщение",
      L"IsDirectory", L"Ёто аталог",
      L"RemoveDirectory", L"”далить аталог",
      L"MK1100SendBarCodeInfo", L"MK1100ѕослать»нформациюЎтрихкода",
      L"GetDBInGroupList", L"—писок»Ѕ¬√руппе",
      L"File2String", L"‘айл¬—троку",
      L"String2File", L"—троку¬‘айл",
      L"TimeFromTimeString", L"¬рем€»з¬ремени—трокой",
      L"CreateHashedArray", L"—оздать’ешированныйћассив",
      L"RemoveHashedArray", L"”далить’ешированныйћассив",
      L"SetHashedArrayValue", L"”становить«начение’ешированногоћассива",
      L"GetHashedArrayValue", L"ѕолучить«начение’ешированногоћассива",
      L"RemoveAllHashedArrays", L"”далить¬се’ешированныећассивы",
      L"InstallDeviceScanner", L"”становить—канер”стройства",
      L"RemoveDeviceScanner", L"”далить—канер”стройства",
      L"AttachFileToMessage", L"ѕрикрепить‘айл —ообщению",
      L"SaveMessageAttachmentToFile", L"—охранитьѕрикрепление—ообщени€¬‘айл",
      L"ReceiveMessages", L"ѕолучить—ообщени€",
      L"RepairLocking", L"ѕочинитьЅлокировки",
      L"RepairServerBusy", L"ѕочинить—ервер«ан€т"
    };
    msmail1c_->functions_.estimatedChainLength(1);
//    functions_.thresholdNumerator(5);
//    functions_.thresholdDenominator(8);
    for( uintptr_t i = 0; i < sizeof(fn) / sizeof(fn[0]); i++ )
      msmail1c_->functions_.insert(*newObjectC1C2<msmail1c::Function>(fn[i],uint8_t(i / 2)));
  }
  catch( ... ){
    return E_FAIL;
  }
//  uintptr_t max = functions_.maxChainLength();
//  uintptr_t min = functions_.minChainLength();
//  uintptr_t avg = functions_.avgChainLength();
  msmail1c_->pBackConnection_ = pBackConnection;
  msmail1c_->pBackConnection_->AddRef();
  memset(lockFileJmpCodeSafe_,0,sizeof(lockFileJmpCodeSafe_));
  memset(unLockFileJmpCodeSafe_,0,sizeof(unLockFileJmpCodeSafe_));
  memset(flushFileBuffersJmpCodeSafe_,0,sizeof(flushFileBuffersJmpCodeSafe_));
  return S_OK;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::Done()
{
  if( msmail1c_->client_.mk1100TCPServer_ != NULL ){
    msmail1c_->client_.mk1100TCPServer_->close();
    msmail1c_->client_.mk1100TCPServer_ = NULL;
  }
/*  if( oldBKENDGetProcAddress_ != NULL ){
    void * proc = findProcImportedEntryAddress("bkend.dll","KERNEL32.DLL","GetProcAddress");
    writeProtectedMemory(
      proc,
      &oldBKENDGetProcAddress_,
      sizeof(uintptr_t)
    );
    oldBKENDGetProcAddress_ = NULL;
  }*/
  if( oldDBENG32LockFile_ != NULL ){
    void * proc = findProcImportedEntryAddress("dbeng32.dll","KERNEL32.DLL","LockFile");
    writeProtectedMemory(
      proc,
      &oldDBENG32LockFile_,
      sizeof(uintptr_t)
    );
    oldDBENG32LockFile_ = NULL;
  }
/*  if( oldMSVCRTLockFile_ != NULL ){
    void * proc = findProcImportedEntryAddress("msvcrt.dll","KERNEL32.DLL","LockFile");
    writeProtectedMemory(
      proc,
      &oldMSVCRTLockFile_,
      sizeof(uintptr_t)
    );
    oldMSVCRTLockFile_ = NULL;
  }
  if( oldMSVCR71LockFile_ != NULL ){
    void * proc = findProcImportedEntryAddress("msvcr71.dll","KERNEL32.DLL","LockFile");
    writeProtectedMemory(
      proc,
      &oldMSVCR71LockFile_,
      sizeof(uintptr_t)
    );
    oldMSVCR71LockFile_ = NULL;
  }
  if( oldMFC42LockFile_ != NULL ){
    void * proc = findProcImportedEntryAddress("mfc42.dll","KERNEL32.DLL","LockFile");
    writeProtectedMemory(
      proc,
      &oldMFC42LockFile_,
      sizeof(uintptr_t)
    );
    oldMFC42LockFile_ = NULL;
  }
  if( oldSEVENGetProcAddress_.p_ != NULL ){
    void * proc = findProcImportedEntryAddress("seven.dll","KERNEL32.DLL","GetProcAddress");
    writeProtectedMemory(
      proc,
      &oldSEVENGetProcAddress_.p_,
      sizeof(uintptr_t)
    );
    oldSEVENGetProcAddress_.p_ = NULL;
  }*/
  if( lockFileJmpCodeSafe_[0] != 0 ){
    writeProtectedMemory(LockFile,lockFileJmpCodeSafe_,sizeof(lockFileJmpCodeSafe_));
    memset(lockFileJmpCodeSafe_,0,sizeof(lockFileJmpCodeSafe_));
  }
  if( unLockFileJmpCodeSafe_[0] != 0 ){
    writeProtectedMemory(UnlockFile,unLockFileJmpCodeSafe_,sizeof(unLockFileJmpCodeSafe_));
    memset(unLockFileJmpCodeSafe_,0,sizeof(unLockFileJmpCodeSafe_));
  }
  if( flushFileBuffersJmpCodeSafe_[0] != 0 ){
    writeProtectedMemory(FlushFileBuffers,flushFileBuffersJmpCodeSafe_,sizeof(flushFileBuffersJmpCodeSafe_));
    memset(flushFileBuffersJmpCodeSafe_,0,sizeof(flushFileBuffersJmpCodeSafe_));
  }
  msmail1c_ = NULL;
  return S_OK;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::GetInfo(SAFEARRAY ** pInfo)
{
  long lInd = 0;
  VARIANT varVersion;
  V_VT(&varVersion) = VT_I4;
  V_I4(&varVersion) = 2000;
  return SafeArrayPutElement(*pInfo,&lInd,&varVersion);
}
//------------------------------------------------------------------------------
// ILanguageExtender Methods
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::RegisterExtensionAs(BSTR * bstrExtensionName)
{
  return (*bstrExtensionName = SysAllocString(L"msmail1c")) != NULL ? S_OK : E_OUTOFMEMORY;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::GetNProps(long * plProps)
{
  *plProps = 24;
  return S_OK;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::FindProp(BSTR bstrPropName,long * plPropNum)
{
  if( _wcsicoll(bstrPropName,L"Name") == 0 ) *plPropNum = 0;
  else
  if( _wcsicoll(bstrPropName,L"Ќаименование") == 0 ) *plPropNum = 0;
  else
  if( _wcsicoll(bstrPropName,L"Active") == 0 ) *plPropNum = 1;
  else
  if( _wcsicoll(bstrPropName,L"јктивирован") == 0 ) *plPropNum = 1;
  else
  if( _wcsicoll(bstrPropName,L"ProcessPriority") == 0 ) *plPropNum = 2;
  else
  if( _wcsicoll(bstrPropName,L"ѕриоритетѕроцесса") == 0 ) *plPropNum = 2;
  else
  if( _wcsicoll(bstrPropName,L"ErrorCode") == 0 ) *plPropNum = 3;
  else
  if( _wcsicoll(bstrPropName,L" одќшибки") == 0 ) *plPropNum = 3;
  else
  if( _wcsicoll(bstrPropName,L"User") == 0 ) *plPropNum = 4;
  else
  if( _wcsicoll(bstrPropName,L"ѕользователь") == 0 ) *plPropNum = 4;
  else
  if( _wcsicoll(bstrPropName,L"Key") == 0 ) *plPropNum = 5;
  else
  if( _wcsicoll(bstrPropName,L" люч") == 0 ) *plPropNum = 5;
  else
  if( _wcsicoll(bstrPropName,L"Groups") == 0 ) *plPropNum = 6;
  else
  if( _wcsicoll(bstrPropName,L"√руппы") == 0 ) *plPropNum = 6;
  else
  if( _wcsicoll(bstrPropName,L"MailServer") == 0 ) *plPropNum = 7;
  else
  if( _wcsicoll(bstrPropName,L"ѕочтовый—ервер") == 0 ) *plPropNum = 7;
  else
  if( _wcsicoll(bstrPropName,L"ConfigFile") == 0 ) *plPropNum = 8;
  else
  if( _wcsicoll(bstrPropName,L" онфигурационный‘айл") == 0 ) *plPropNum = 8;
  else
  if( _wcsicoll(bstrPropName,L"LogFile") == 0 ) *plPropNum = 9;
  else
  if( _wcsicoll(bstrPropName,L"Ћог‘айл") == 0 ) *plPropNum = 9;
  else
  if( _wcsicoll(bstrPropName,L"LocalTime") == 0 ) *plPropNum = 10;
  else
  if( _wcsicoll(bstrPropName,L"Ћокальное¬рем€") == 0 ) *plPropNum = 10;
  else
  if( _wcsicoll(bstrPropName,L"SystemTime") == 0 ) *plPropNum = 11;
  else
  if( _wcsicoll(bstrPropName,L"—истемное¬рем€") == 0 ) *plPropNum = 11;
  else
  if( _wcsicoll(bstrPropName,L"UUID") == 0 ) *plPropNum = 12;
  else
  if( _wcsicoll(bstrPropName,L"¬”»ƒ") == 0 ) *plPropNum = 12;
  else
  if( _wcsicoll(bstrPropName,L"RandomNumber") == 0 ) *plPropNum = 13;
  else
  if( _wcsicoll(bstrPropName,L"—лучайное„исло") == 0 ) *plPropNum = 13;
  else
  if( _wcsicoll(bstrPropName,L"Version") == 0 ) *plPropNum = 14;
  else
  if( _wcsicoll(bstrPropName,L"¬ерси€") == 0 ) *plPropNum = 14;
  else
  if( _wcsicoll(bstrPropName,L"Connected") == 0 ) *plPropNum = 15;
  else
  if( _wcsicoll(bstrPropName,L"—оединение”становлено") == 0 ) *plPropNum = 15;
  else
  if( _wcsicoll(bstrPropName,L"WorkServer") == 0 ) *plPropNum = 16;
  else
  if( _wcsicoll(bstrPropName,L"–абочий—ервер") == 0 ) *plPropNum = 16;
  else
  if( _wcsicoll(bstrPropName,L"ReceivedMessagesList") == 0 ) *plPropNum = 17;
  else
  if( _wcsicoll(bstrPropName,L"—писокѕолученных—ообщений") == 0 ) *plPropNum = 17;
  else
  if( _wcsicoll(bstrPropName,L"DBList") == 0 ) *plPropNum = 18;
  else
  if( _wcsicoll(bstrPropName,L"—писок»Ѕ") == 0 ) *plPropNum = 18;
  else
  if( _wcsicoll(bstrPropName,L"DBGroupList") == 0 ) *plPropNum = 19;
  else
  if( _wcsicoll(bstrPropName,L"√рупповой—писок»Ѕ") == 0 ) *plPropNum = 19;
  else
  if( _wcsicoll(bstrPropName,L"UserList") == 0 ) *plPropNum = 20;
  else
  if( _wcsicoll(bstrPropName,L"—писокѕользователей") == 0 ) *plPropNum = 20;
  else
  if( _wcsicoll(bstrPropName,L"SendingMessagesList") == 0 ) *plPropNum = 21;
  else
  if( _wcsicoll(bstrPropName,L"—писокЌеотправленных—ообщений") == 0 ) *plPropNum = 21;
  else
  if( _wcsicoll(bstrPropName,L"MK1100Port") == 0 ) *plPropNum = 22;
  else
  if( _wcsicoll(bstrPropName,L"MK1100ѕорт") == 0 ) *plPropNum = 22;
  else
  if( _wcsicoll(bstrPropName,L"AsyncMessagesReceiving") == 0 ) *plPropNum = 23;
  else
  if( _wcsicoll(bstrPropName,L"јсинхронноеѕолучение—ообщений") == 0 ) *plPropNum = 23;
  else 
    return DISP_E_MEMBERNOTFOUND;
  return S_OK;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::GetPropName(long lPropNum,long lPropAlias,BSTR * pbstrPropName)
{
  switch( lPropNum ){
    case 0 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"Name")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"Ќаименование")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 1 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"Active")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"јктивирован")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 2 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"ProcessPriority")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"ѕриоритетѕроцесса")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 3 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"ErrorCode")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L" одќшибки")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 4 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"User")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"ѕользователь")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 5 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"Key")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L" люч")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 6 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"Groups")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"√руппы")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 7 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"MailServer")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"ѕочтовый—ервер")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 8 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"ConfigFile")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L" онфигурационный‘айл")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 9 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"LogFile")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"Ћог‘айл")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 10 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"LocalTime")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"Ћокальное¬рем€")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 11 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"SystemTime")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"—истемное¬рем€")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 12 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"UUID")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"¬”»ƒ")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 13 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"RandomNumber")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"—лучайное„исло")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 14 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"Version")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"¬ерси€")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 15 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"Connected")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"—оединение”становлено")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 16 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"WorkServer")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"–абочий—ервер")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 17 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"ReceivedMessagesList")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"—писокѕолученных—ообщений")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 18 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"DBList")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"—писок»Ѕ")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 19 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"DBGroupList")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"√рупповой—писок»Ѕ")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 20 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"UserList")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"—писокѕользователей")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 21 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"SendingMessagesList")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"—писокЌеотправленных—ообщений")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 22 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"MK1100Port")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"MK1100ѕорт")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 23 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"AsyncMessagesReceiving")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"јсинхронноеѕолучение—ообщений")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
  }
  return E_NOTIMPL;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::GetPropVal(long lPropNum,VARIANT * pvarPropVal)
{
  HRESULT hr = S_OK;
  try {
    switch( lPropNum ){
      case 0 :
        V_BSTR(pvarPropVal) = msmail1c_->name_.getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 1 :
        if( V_VT(pvarPropVal) != VT_I4 ) hr = VariantChangeTypeEx(pvarPropVal,pvarPropVal,0,0,VT_I4);
        if( SUCCEEDED(hr) ) V_I4(pvarPropVal) = msmail1c_->active_ ? 1 : 0;
        break;
      case 2 :
        if( V_VT(pvarPropVal) != VT_I4 ) hr = VariantChangeTypeEx(pvarPropVal,pvarPropVal,0,0,VT_I4);
        if( SUCCEEDED(hr) ){
          V_VT(pvarPropVal) = VT_BSTR;
          switch( GetPriorityClass(GetCurrentProcess()) ){
            case IDLE_PRIORITY_CLASS :
              V_BSTR(pvarPropVal) = SysAllocString(L"IDLE_PRIORITY_CLASS");
              break;
            case BELOW_NORMAL_PRIORITY_CLASS :
              V_BSTR(pvarPropVal) = SysAllocString(L"BELOW_NORMAL_PRIORITY_CLASS");
              break;
            case NORMAL_PRIORITY_CLASS :
              V_BSTR(pvarPropVal) = SysAllocString(L"NORMAL_PRIORITY_CLASS");
              break;
            case ABOVE_NORMAL_PRIORITY_CLASS :
              V_BSTR(pvarPropVal) = SysAllocString(L"ABOVE_NORMAL_PRIORITY_CLASS");
              break;
            case HIGH_PRIORITY_CLASS :
              V_BSTR(pvarPropVal) = SysAllocString(L"HIGH_PRIORITY_CLASS");
              break;
            case REALTIME_PRIORITY_CLASS :
              V_BSTR(pvarPropVal) = SysAllocString(L"REALTIME_PRIORITY_CLASS");
              break;
            default :
              V_VT(pvarPropVal) = VT_I4;
              V_I4(pvarPropVal) = GetPriorityClass(GetCurrentProcess());
          }
          if( V_VT(pvarPropVal) == VT_BSTR && V_BSTR(pvarPropVal) == NULL ) hr = E_OUTOFMEMORY;
        }
        break;
      case 3 :
        if( V_VT(pvarPropVal) != VT_I4 ) hr = VariantChangeTypeEx(pvarPropVal,pvarPropVal,0,0,VT_I4);
        if( SUCCEEDED(hr) ) V_I4(pvarPropVal) = msmail1c_->lastError_;
        break;
      case 4 : // User
        V_BSTR(pvarPropVal) = msmail1c_->user_.getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 5 : // Key
        V_BSTR(pvarPropVal) = msmail1c_->key_.getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 6 : // Groups
        V_BSTR(pvarPropVal) = msmail1c_->groups_.getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 7 : // MailServer
        V_BSTR(pvarPropVal) = msmail1c_->mailServer_.getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 8 : // ConfigFile
        V_BSTR(pvarPropVal) = msmail1c_->configFile_.getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 9 : // LogFile
        V_BSTR(pvarPropVal) = msmail1c_->logFile_.getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 10 : // LocalTime
        V_BSTR(pvarPropVal) = utf8::int2Str(getlocaltimeofday()).getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 11 : // SystemTime
        V_BSTR(pvarPropVal) = utf8::int2Str(gettimeofday()).getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 12 : // UUID
        V_BSTR(pvarPropVal) = createGUIDAsBase32String().getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 13 : // RandomNumber
        V_BSTR(pvarPropVal) = utf8::int2Str(msmail1c_->rnd_.random()).getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 14 : // Version
        V_BSTR(pvarPropVal) = utf8::String(msmail1clib_version.gnu_).getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 15 : // Connected
        if( V_VT(pvarPropVal) != VT_I4 ) hr = VariantChangeTypeEx(pvarPropVal,pvarPropVal,0,0,VT_I4);
        if( SUCCEEDED(hr) ){
          V_I4(pvarPropVal) = 0;
          if( msmail1c_->active_ ){
            AutoLock<FiberInterlockedMutex> lock(msmail1c_->client_.connectedMutex_);
            V_I4(pvarPropVal) = msmail1c_->client_.connected_ ? 1 : 0;
          }
        }
        break;
      case 16 : // WorkServer
        {
          AutoLock<FiberInterlockedMutex> lock(msmail1c_->client_.connectedMutex_);
          if( msmail1c_->active_ && msmail1c_->client_.connected_ ){
            V_BSTR(pvarPropVal) = msmail1c_->client_.connectedToServer_.getOLEString();
            V_VT(pvarPropVal) = VT_BSTR;
          }
          else {
            if( V_VT(pvarPropVal) != VT_BSTR ) hr = VariantChangeTypeEx(pvarPropVal,pvarPropVal,0,0,VT_BSTR);
          }
        }
        break;
      case 17 : // ReceivedMessagesList
        V_BSTR(pvarPropVal) = msmail1c_->client_.getReceivedMessageList().getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 18 : // DBList
        V_BSTR(pvarPropVal) = msmail1c_->client_.getDBList().getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 19 : // DBGroupList
        V_BSTR(pvarPropVal) = msmail1c_->client_.getDBGroupList().getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 20 : // UserList
        V_BSTR(pvarPropVal) = msmail1c_->client_.getUserList().getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 21 : // SendingMessagesList
        V_BSTR(pvarPropVal) = msmail1c_->client_.getSendingMessageList().getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 22 : // MK1100Port
        if( V_VT(pvarPropVal) != VT_I4 ) hr = VariantChangeTypeEx(pvarPropVal,pvarPropVal,0,0,VT_I4);
        if( SUCCEEDED(hr) ) V_I4(pvarPropVal) = msmail1c_->client_.mk1100Port_;
        break;
      case 23 : // AsyncMessagesReceiving
        if( V_VT(pvarPropVal) != VT_I4 ) hr = VariantChangeTypeEx(pvarPropVal,pvarPropVal,0,0,VT_I4);
        if( SUCCEEDED(hr) ) V_I4(pvarPropVal) = msmail1c_->client_.asyncMessagesReceiving_ ? 1 : 0;
        break;
    }
  }
  catch( ExceptionSP & e ){
    hr = HRESULT_FROM_WIN32(e->code() - errorOffset);
    e->writeStdError();
  }
  return hr;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::SetPropVal(long lPropNum,VARIANT * varPropVal)
{
  HRESULT hr = S_OK;
  bool r;
  try {
    switch( lPropNum ){
      case 0 :
        if( V_VT(varPropVal) != VT_BSTR ) hr = VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_BSTR);
        if( SUCCEEDED(hr) ) msmail1c_->name_ = V_BSTR(varPropVal);
        break;
      case 1 :
        if( V_VT(varPropVal) != VT_I4 ) hr = VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_I4);
        if( SUCCEEDED(hr) ){
          if( V_I4(varPropVal) != 0 && msmail1c_->pAsyncEvent_ == NULL ){
            hr = msmail1c_->pBackConnection_->QueryInterface(IID_IAsyncEvent,(void **) &msmail1c_->pAsyncEvent_);
            if( SUCCEEDED(hr) ) hr = msmail1c_->pAsyncEvent_->SetEventBufferDepth(1000000);
          }
          if( SUCCEEDED(hr) ){
            if( V_I4(varPropVal) != 0 ){
              msmail1c_->client_.close();
              msmail1c_->active_ = false;
              msmail1c_->client_.pAsyncEvent_ = msmail1c_->pAsyncEvent_;
              msmail1c_->client_.name_ = msmail1c_->name_;
              msmail1c_->client_.user_ = msmail1c_->user_;
              msmail1c_->client_.key_ = msmail1c_->key_;
              msmail1c_->client_.groups_ = msmail1c_->groups_;
              msmail1c_->client_.mailServer_ = msmail1c_->mailServer_;
              msmail1c_->client_.readConfig(msmail1c_->configFile_,msmail1c_->logFile_);
              msmail1c_->client_.open();
              msmail1c_->active_ = true;
            }
            else {
              msmail1c_->client_.close();
              msmail1c_->active_ = false;
            }
            if( V_I4(varPropVal) == 0 && msmail1c_->pAsyncEvent_ != NULL ){
              msmail1c_->pAsyncEvent_->Release();
              msmail1c_->pAsyncEvent_ = NULL;
            }
          }
        }
        break;
      case 2  :
        r = false;
        if( V_VT(varPropVal) == VT_BSTR ){
          r = true;
          utf8::String s(V_BSTR(varPropVal));
          if( s.strcasecmp("IDLE_PRIORITY_CLASS") == 0 )
            SetPriorityClass(GetCurrentProcess(),IDLE_PRIORITY_CLASS);
          else
          if( s.strcasecmp("BELOW_NORMAL_PRIORITY_CLASS") == 0 )
            SetPriorityClass(GetCurrentProcess(),BELOW_NORMAL_PRIORITY_CLASS);
          else
          if( s.strcasecmp("NORMAL_PRIORITY_CLASS") == 0 )
            SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
          else
          if( s.strcasecmp("ABOVE_NORMAL_PRIORITY_CLASS") == 0 )
            SetPriorityClass(GetCurrentProcess(),ABOVE_NORMAL_PRIORITY_CLASS);
          else
          if( s.strcasecmp("HIGH_PRIORITY_CLASS") == 0 )
            SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
          else
          if( s.strcasecmp("REALTIME_PRIORITY_CLASS") == 0 )
            SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);
          else
            r = false;
        }
        if( !r && (V_VT(varPropVal) == VT_I4 || VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_I4) == S_OK) ){
          SetPriorityClass(GetCurrentProcess(),V_I4(varPropVal));
        }
        break;
      case 3 :
        hr = E_NOTIMPL;
        break;
      case 4 : // User
        if( V_VT(varPropVal) != VT_BSTR ) hr = VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_BSTR);
        if( SUCCEEDED(hr) ) msmail1c_->user_ = V_BSTR(varPropVal);
        break;
      case 5 : // Key
        if( V_VT(varPropVal) != VT_BSTR ) hr = VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_BSTR);
        if( SUCCEEDED(hr) ) msmail1c_->key_ = V_BSTR(varPropVal);
        break;
      case 6 : // Groups
        if( V_VT(varPropVal) != VT_BSTR ) hr = VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_BSTR);
        if( SUCCEEDED(hr) ) msmail1c_->groups_ = V_BSTR(varPropVal);
        break;
      case 7 : // MailServer
        if( V_VT(varPropVal) != VT_BSTR ) hr = VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_BSTR);
        if( SUCCEEDED(hr) ) msmail1c_->mailServer_ = V_BSTR(varPropVal);
        break;
      case 8 : // ConfigFile
        if( V_VT(varPropVal) != VT_BSTR ) hr = VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_BSTR);
        if( SUCCEEDED(hr) ) msmail1c_->configFile_ = V_BSTR(varPropVal);
        break;
      case 9 : // LogFile
        if( V_VT(varPropVal) != VT_BSTR ) hr = VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_BSTR);
        if( SUCCEEDED(hr) ) msmail1c_->logFile_ = V_BSTR(varPropVal);
        break;
      case 10 : // LocalTime
        hr = E_NOTIMPL;
        break;
      case 11 : // SystemTime
        hr = E_NOTIMPL;
        break;
      case 12 : // UUID
        hr = E_NOTIMPL;
        break;
      case 13 : // RandomNumber
        hr = E_NOTIMPL;
        break;
      case 14 : // Version
        hr = E_NOTIMPL;
        break;
      case 15 :
        hr = E_NOTIMPL;
        break;
      case 16 :
        hr = E_NOTIMPL;
        break;
      case 17 :
        hr = E_NOTIMPL;
        break;
      case 18 :
        hr = E_NOTIMPL;
        break;
      case 19 :
        hr = E_NOTIMPL;
        break;
      case 20 :
        hr = E_NOTIMPL;
        break;
      case 21 :
        hr = E_NOTIMPL;
        break;
      case 22 : // MK1100Port
        if( V_VT(varPropVal) != VT_I4 ) hr = VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_I4);
        if( SUCCEEDED(hr) ){
          msmail1c_->client_.mk1100Port_ = u_short(V_I4(varPropVal));
          if( msmail1c_->client_.mk1100TCPServer_ != NULL ){
            msmail1c_->client_.mk1100TCPServer_->close();
            msmail1c_->client_.mk1100TCPServer_ = NULL;
          }
          if( msmail1c_->client_.mk1100Port_ != 0 ){
            msmail1c_->client_.mk1100TCPServer_ = newObjectV1<msmail::MK1100TCPServer>(&msmail1c_->client_);
            msmail1c_->client_.mk1100TCPServer_->open();
          }
        }
        break;
      case 23 : // AsyncMessagesReceiving
        if( V_VT(varPropVal) != VT_I4 ) hr = VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_I4);
        if( SUCCEEDED(hr) ) msmail1c_->client_.asyncMessagesReceiving_ = V_I4(varPropVal) != 0;
        break;
    }
  }
  catch( ExceptionSP & e ){
    hr = HRESULT_FROM_WIN32(e->code() - errorOffset);
    e->writeStdError();
  }
  return hr;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::IsPropReadable(long lPropNum,BOOL * pboolPropRead)
{
  switch( lPropNum ){
    case 0 : *pboolPropRead = TRUE; break;
    case 1 : *pboolPropRead = TRUE; break;
    case 2 : *pboolPropRead = TRUE; break;
    case 3 : *pboolPropRead = TRUE; break;
    case 4 : *pboolPropRead = TRUE; break;
    case 5 : *pboolPropRead = TRUE; break;
    case 6 : *pboolPropRead = TRUE; break;
    case 7 : *pboolPropRead = TRUE; break;
    case 8 : *pboolPropRead = TRUE; break;
    case 9 : *pboolPropRead = TRUE; break;
    case 10 : *pboolPropRead = TRUE; break;
    case 11 : *pboolPropRead = TRUE; break;
    case 12 : *pboolPropRead = TRUE; break;
    case 13 : *pboolPropRead = TRUE; break;
    case 14 : *pboolPropRead = TRUE; break;
    case 15 : *pboolPropRead = TRUE; break;
    case 16 : *pboolPropRead = TRUE; break;
    case 17 : *pboolPropRead = TRUE; break;
    case 18 : *pboolPropRead = TRUE; break;
    case 19 : *pboolPropRead = TRUE; break;
    case 20 : *pboolPropRead = TRUE; break;
    case 21 : *pboolPropRead = TRUE; break;
    case 22 : *pboolPropRead = TRUE; break;
    case 23 : *pboolPropRead = TRUE; break;
    default : return E_NOTIMPL;
  }
  return S_OK;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::IsPropWritable(long lPropNum,BOOL * pboolPropWrite)
{
  switch( lPropNum ){
    case 0 : *pboolPropWrite = TRUE; break;
    case 1 : *pboolPropWrite = TRUE; break;
    case 2 : *pboolPropWrite = TRUE; break;
    case 3 : *pboolPropWrite = FALSE; break;
    case 4 : *pboolPropWrite = TRUE; break;
    case 5 : *pboolPropWrite = TRUE; break;
    case 6 : *pboolPropWrite = TRUE; break;
    case 7 : *pboolPropWrite = TRUE; break;
    case 8 : *pboolPropWrite = TRUE; break;
    case 9 : *pboolPropWrite = TRUE; break;
    case 10 : *pboolPropWrite = FALSE; break;
    case 11 : *pboolPropWrite = FALSE; break;
    case 12 : *pboolPropWrite = FALSE; break;
    case 13 : *pboolPropWrite = FALSE; break;
    case 14 : *pboolPropWrite = FALSE; break;
    case 15 : *pboolPropWrite = FALSE; break;
    case 16 : *pboolPropWrite = FALSE; break;
    case 17 : *pboolPropWrite = FALSE; break;
    case 18 : *pboolPropWrite = FALSE; break;
    case 19 : *pboolPropWrite = FALSE; break;
    case 20 : *pboolPropWrite = FALSE; break;
    case 21 : *pboolPropWrite = FALSE; break;
    case 22 : *pboolPropWrite = TRUE; break;
    case 23 : *pboolPropWrite = TRUE; break;
    default : return E_NOTIMPL;
  }
  return S_OK;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::GetNMethods(long * plMethods)
{
  *plMethods = 32;
  return S_OK;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::FindMethod(BSTR bstrMethodName,long * plMethodNum)
{
  msmail1c::Function * f = msmail1c_->functions_.find(msmail1c::Function(bstrMethodName));
  *plMethodNum = f != NULL ? f->value_ : -1;
  return f != NULL ? S_OK : DISP_E_MEMBERNOTFOUND;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::GetMethodName(long lMethodNum,long lMethodAlias,BSTR * pbstrMethodName)
{
  switch( lMethodNum ){
    case 0 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"LockFile")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"Ѕлокировать‘айл")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 1 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"UnlockFile")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"–азблокировать‘айл")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 2 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"GetLastError")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"ѕолучить одќшибки")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 3 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"Sleep")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"—пать")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 4 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"SleepIn")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"—пать¬")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 5 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"TextToFile")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"“екст¬‘айл")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 6 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"NewMessage")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"Ќовое—ообщение")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 7 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"SetMessageAttribute")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"”становитьјтрибут—ообщени€")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 8 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"GetMessageAttribute")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"ѕолучитьјтрибут—ообщени€")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 9 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"SendMessage")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"ѕослать—ообщение")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 10 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"RemoveMessage")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"”далить—ообщение")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 11 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"GetDB")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"ѕолучить»Ѕ")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 12 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"CopyMessage")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"—копировать—ообщение")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 13 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"RemoveMessageAttribute")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"”далитьјтрибут—ообщени€")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 14 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"DebugMessage")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"ќтладочное—ообщение")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 15 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"IsDirectory")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"Ёто аталог")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 16 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"RemoveDirectory")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"”далить аталог")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 17 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"MK1100SendBarCodeInfo")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"MK1100ѕослать»нформациюЎтрихкода")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 18 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"GetDBInGroupList")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"—писок»Ѕ¬√руппе")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 19 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"File2String")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"‘айл¬—троку")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 20 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"String2File")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"—троку¬‘айл")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 21 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"TimeFromTimeString")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"¬рем€»з¬ремени—трокой")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 22 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"CreateHashedArray")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"—оздать’ешированныйћассив")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 23 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"RemoveHashedArray")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"”далить’ешированныйћассив")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 24 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"SetHashedArrayValue")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"”становить«начение’ешированногоћассива")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 25 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"GetHashedArrayValue")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"ѕолучить«начение’ешированногоћассива")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 26 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"RemoveAllHashedArrays")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"”далить¬се’ешированныећассивы")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 27 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"InstallDeviceScanner")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"”становить—канер”стройства")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 28 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"RemoveDeviceScanner")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"”далить—канер”стройства")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 29 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"AttachFileToMessage")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"ѕрикрепить‘айл —ообщению")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 30 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"SaveMessageAttachmentToFile")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"—охранитьѕрикрепление—ообщени€¬‘айл")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 31 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"ReceiveMessages")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"ѕолучить—ообщени€")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 32 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"RepairLocking")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"ѕочинитьЅлокировки")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 33 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"RepairServerBusy")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"ѕочинить—ервер«ан€т")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
  }
  return E_NOTIMPL;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::GetNParams(long lMethodNum,long * plParams)
{
  switch( lMethodNum ){
    case 0 : *plParams = 3; break;
    case 1 : *plParams = 1; break;
    case 2 : *plParams = 1; break;
    case 3 : *plParams = 1; break;
    case 4 : *plParams = 2; break;
    case 5 : *plParams = 2; break;
    case 6 : *plParams = 0; break;
    case 7 : *plParams = 3; break;
    case 8 : *plParams = 2; break;
    case 9 : *plParams = 2; break;
    case 10 : *plParams = 2; break;
    case 11 : *plParams = 0; break;
    case 12 : *plParams = 1; break;
    case 13 : *plParams = 2; break;
    case 14 : *plParams = 2; break;
    case 15 : *plParams = 1; break;
    case 16 : *plParams = 1; break;
    case 17 : *plParams = 2; break;
    case 18 : *plParams = 1; break;
    case 19 : *plParams = 1; break;
    case 20 : *plParams = 2; break;
    case 21 : *plParams = 1; break;
    case 22 : *plParams = 0; break;
    case 23 : *plParams = 1; break;
    case 24 : *plParams = 3; break;
    case 25 : *plParams = 2; break;
    case 26 : *plParams = 0; break;
    case 27 : *plParams = 1; break;
    case 28 : *plParams = 1; break;
    case 29 : *plParams = 3; break;
    case 30 : *plParams = 3; break;
    case 31 : *plParams = 1; break;
    case 32 : *plParams = 0; break;
    case 33 : *plParams = 0; break;
    default :
      *plParams = -1;
      return E_NOTIMPL;
  }
  return S_OK;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::GetParamDefValue(long lMethodNum,long lParamNum,VARIANT * pvarParamDefValue)
{
  HRESULT hr = S_OK;
  switch( lMethodNum ){
    case 9 : case 10 :
      if( lParamNum == 1 ){
        if( V_VT(pvarParamDefValue) != VT_I4 )
          hr = VariantChangeType(pvarParamDefValue,pvarParamDefValue,0,VT_I4);
        if( SUCCEEDED(hr) ) V_I4(pvarParamDefValue) = 0;
      }
      break;
    case 31 :
      if( lParamNum == 1 ){
        if( V_VT(pvarParamDefValue) != VT_I4 )
          hr = VariantChangeType(pvarParamDefValue,pvarParamDefValue,0,VT_I4);
        if( SUCCEEDED(hr) ) V_I4(pvarParamDefValue) = 1;
      }
      break;
  }
  return hr;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::HasRetVal(long lMethodNum,BOOL * pboolRetValue)
{
  *pboolRetValue = TRUE;
  return S_OK;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::CallAsProc(long lMethodNum,SAFEARRAY ** paParams)
{
  return E_NOTIMPL;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::CallAsFunc(long lMethodNum,VARIANT * pvarRetValue,SAFEARRAY ** paParams)
{
  HRESULT hr;
  VARIANT * pv0, * pv1, * pv2/*, v0, v1, v2*/;
//  VariantInit(&v0);
//  VariantInit(&v1);
//  VariantInit(&v2);
  if( V_VT(pvarRetValue) != VT_I4 ) hr = VariantChangeType(pvarRetValue,pvarRetValue,0,VT_I4);
  if( SUCCEEDED(hr) ){
    LONG lIndex = -1;
    V_I4(pvarRetValue) = 0;
    msmail1c_->lastError_ = ERROR_SUCCESS;
    try {
      switch( lMethodNum ){
        case 0 : // LockFile
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0; // »м€ файла
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              lIndex = 1; // ћинимальное врем€ (в милисекундах)
              hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv1);
              if( SUCCEEDED(hr) ){
                lIndex = 2; // ћаксимальное врем€ (в милисекундах)
                hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv2);
                if( SUCCEEDED(hr) ){
                  if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
                  if( SUCCEEDED(hr) ){
                    if( V_VT(pv1) != VT_I4 ) hr = VariantChangeTypeEx(pv1,pv1,0,0,VT_I4);
                    if( SUCCEEDED(hr) ){
                      if( V_VT(pv2) != VT_I4 ) hr = VariantChangeTypeEx(pv2,pv2,0,0,VT_I4);
                      if( SUCCEEDED(hr) )
                        hr = lockFile(V_BSTR(pv0),V_I4(pv1),V_I4(pv2),&V_I4(pvarRetValue));
                    }
                  }
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 1 : // UnlockFile
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0; // »м€ файла
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
              if( SUCCEEDED(hr) )
                hr = unlockFile(V_BSTR(pv0),&V_I4(pvarRetValue));
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 2 : // GetLastError
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0; // »м€ файла
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
              if( SUCCEEDED(hr) )
                hr = getLastError(V_BSTR(pv0),&V_I4(pvarRetValue));
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 3 : // Sleep
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_I4 ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_I4);
              if( SUCCEEDED(hr) ){
                hr = sleep(V_I4(pv0));
                if( SUCCEEDED(hr) ) V_I4(pvarRetValue) = 1;
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 4 : // SleepIn
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              lIndex = 1;
              hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv1);
              if( SUCCEEDED(hr) ){
                if( V_VT(pv0) != VT_I4 ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_I4);
                if( SUCCEEDED(hr) ){
                  if( V_VT(pv1) != VT_I4 ) hr = VariantChangeTypeEx(pv1,pv1,0,0,VT_I4);
                  if( SUCCEEDED(hr) ){
                    hr = sleepIn(V_I4(pv0),V_I4(pv1));
                    if( SUCCEEDED(hr) ) V_I4(pvarRetValue) = 1;
                  }
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 5 : // TextToFile
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              lIndex = 1;
              hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv1);
              if( SUCCEEDED(hr) ){
                if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
                if( SUCCEEDED(hr) ){
                  if( V_VT(pv1) != VT_BSTR ) hr = VariantChangeTypeEx(pv1,pv1,0,0,VT_BSTR);
                  if( SUCCEEDED(hr) ){
                    hr = textToFile(V_BSTR(pv0),V_BSTR(pv1),&V_I4(pvarRetValue));
                    if( SUCCEEDED(hr) ) V_I4(pvarRetValue) = 1;
                  }
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 6 : // NewMessage
          if( !msmail1c_->active_ ) newObjectV1C2<Exception>(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__)->throwSP();
          V_BSTR(pvarRetValue) = msmail1c_->client_.newMessage().getOLEString();
          V_VT(pvarRetValue) = VT_BSTR;
          break;
        case 7 : // SetMessageAttribute
          if( !msmail1c_->active_ ) newObjectV1C2<Exception>(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__)->throwSP();
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0; // UUID of Message
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              lIndex = 1; // Key of attribute
              hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv1);
              if( SUCCEEDED(hr) ){
                lIndex = 2; // Value of attribute
                hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv2);
                if( SUCCEEDED(hr) ){
                  if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
                  if( SUCCEEDED(hr) ){
                    if( V_VT(pv1) != VT_BSTR ) hr = VariantChangeTypeEx(pv1,pv1,0,0,VT_BSTR);
                    if( SUCCEEDED(hr) ){
                      if( V_VT(pv2) != VT_BSTR ) hr = VariantChangeTypeEx(pv2,pv2,0,0,VT_BSTR);
                      if( SUCCEEDED(hr) ){
                        V_BSTR(pvarRetValue) = msmail1c_->client_.value(V_BSTR(pv0),V_BSTR(pv1),V_BSTR(pv2)).getOLEString();
                        V_VT(pvarRetValue) = VT_BSTR;
                      }
                    }
                  }
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 8 : // GetMessageAttribute
          if( !msmail1c_->active_ ) newObjectV1C2<Exception>(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__)->throwSP();
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0; // UUID of Message
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              lIndex = 1; // Key of attribute
              hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv1);
              if( SUCCEEDED(hr) ){
                if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
                if( SUCCEEDED(hr) ){
                  if( V_VT(pv1) != VT_BSTR ) hr = VariantChangeTypeEx(pv1,pv1,0,0,VT_BSTR);
                  if( SUCCEEDED(hr) )
                    hr = msmail1c_->client_.value(V_BSTR(pv0),V_BSTR(pv1),pvarRetValue);
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 9 : // SendMessage
          if( !msmail1c_->active_ ) newObjectV1C2<Exception>(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__)->throwSP();
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
              if( SUCCEEDED(hr) ){
                lIndex = 1;
                hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv1);
                if( SUCCEEDED(hr) ){
                  if( V_VT(pv1) != VT_I4 ) hr = VariantChangeTypeEx(pv1,pv1,0,0,VT_I4);
                  if( SUCCEEDED(hr) ){
                    V_I4(pvarRetValue) = msmail1c_->client_.sendMessage(V_BSTR(pv0),V_I4(pv1) != 0) ? 1 : 0;
                    msmail1c_->lastError_ = msmail1c_->client_.workFiberLastError_ - (msmail1c_->client_.workFiberLastError_ >= errorOffset ? errorOffset : 0);
                  }
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 10 : // RemoveMessage
          if( !msmail1c_->active_ ) newObjectV1C2<Exception>(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__)->throwSP();
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
              if( SUCCEEDED(hr) ){
                lIndex = 1;
                hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv1);
                if( SUCCEEDED(hr) ){
                  if( V_VT(pv1) != VT_I4 ) hr = VariantChangeTypeEx(pv1,pv1,0,0,VT_I4);
                  if( SUCCEEDED(hr) ){
                    V_I4(pvarRetValue) = msmail1c_->client_.removeMessage(V_BSTR(pv0),V_I4(pv1) != 0);
                    msmail1c_->lastError_ = msmail1c_->client_.workFiberLastError_ - (msmail1c_->client_.workFiberLastError_ >= errorOffset ? errorOffset : 0);
                  }
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 11 : // GetDB
          if( !msmail1c_->active_ ) newObjectV1C2<Exception>(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__)->throwSP();
          msmail1c_->client_.getDB();
          V_I4(pvarRetValue) = 1;
          break;
        case 12 : // CopyMessage
          if( !msmail1c_->active_ ) newObjectV1C2<Exception>(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__)->throwSP();
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
              if( SUCCEEDED(hr) ){
                V_BSTR(pvarRetValue) = msmail1c_->client_.copyMessage(V_BSTR(pv0)).getOLEString();
                V_VT(pvarRetValue) = VT_BSTR;
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 13 : // RemoveMessageAttribute
          if( !msmail1c_->active_ ) newObjectV1C2<Exception>(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__)->throwSP();
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0; // UUID of Message
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              lIndex = 1; // Key of attribute
              hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv1);
              if( SUCCEEDED(hr) ){
                if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
                if( SUCCEEDED(hr) ){
                  if( V_VT(pv1) != VT_BSTR ) hr = VariantChangeTypeEx(pv1,pv1,0,0,VT_BSTR);
                  if( SUCCEEDED(hr) ){
                    V_BSTR(pvarRetValue) = msmail1c_->client_.removeValue(V_BSTR(pv0),V_BSTR(pv1)).getOLEString();
                    V_VT(pvarRetValue) = VT_BSTR;
                  }
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 14 : // DebugMessage
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0; // UUID of Message
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              lIndex = 1; // Key of attribute
              hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv1);
              if( SUCCEEDED(hr) ){
                if( V_VT(pv0) != VT_I4 ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_I4);
                if( SUCCEEDED(hr) ){
                  if( V_VT(pv1) != VT_BSTR ) hr = VariantChangeTypeEx(pv1,pv1,0,0,VT_BSTR);
                  if( SUCCEEDED(hr) ){
                    stdErr.debug(V_I4(pv0),utf8::String::Stream() << utf8::String(V_BSTR(pv1)));
                    V_I4(pvarRetValue) = 1;
                  }
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 15 : // IsDirectory
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0; // »м€ каталога
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
              if( SUCCEEDED(hr) ){
                Stat st;
                if( stat(V_BSTR(pv0),st) ){
                  V_I4(pvarRetValue) = (st.st_mode & S_IFDIR) != 0 ? 1 : 0;
                }
                else {
                  msmail1c_->lastError_ = GetLastError();
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 16 : // RemoveDirectory
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0; // »м€ каталога
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
              if( SUCCEEDED(hr) ) removeDirectory(V_BSTR(pv0),true);
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 17 : // MK1100SendBarCodeInfo
          if( msmail1c_->client_.mk1100TCPServer_ == NULL ) newObjectV1C2<Exception>(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__)->throwSP();
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              lIndex = 1;
              hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv1);
              if( SUCCEEDED(hr) ){
                if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
                if( SUCCEEDED(hr) ){
                  if( V_VT(pv1) != VT_BSTR ) hr = VariantChangeTypeEx(pv1,pv1,0,0,VT_BSTR);
                  if( SUCCEEDED(hr) ){
                    msmail::MK1100ClientFiber * mk1100 = (msmail::MK1100ClientFiber *) utf8::str2Int(V_BSTR(pv0));
                    AutoLock<FiberInterlockedMutex> lock(msmail1c_->client_.mk1100TCPServer_->fibersMutex_);
                    intptr_t i = msmail1c_->client_.mk1100TCPServer_->fibers_.bSearch(mk1100);
                    if( i >= 0 ){
                      mk1100->data_ = V_BSTR(pv1);
                      mk1100->sem_.post();
                    }
                  }
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 18 : // GetDBInGroupList
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
              if( SUCCEEDED(hr) ){
                V_BSTR(pvarRetValue) = msmail1c_->client_.getDBInGroupList(V_BSTR(pv0)).getOLEString();
                V_VT(pvarRetValue) = VT_BSTR;
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 19 : // File2String
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
              if( SUCCEEDED(hr) ){
                AutoPtr<uint8_t> buffer;
                Stat st;
                if( stat(V_BSTR(pv0),st) ){
                  buffer.alloc((size_t) st.st_size);
                  AsyncFile file(V_BSTR(pv0));
                  file.readOnly(true).open().readBuffer(buffer,st.st_size).close();
                  V_BSTR(pvarRetValue) = base64Encode(buffer,(uintptr_t) st.st_size).getOLEString();
                  V_VT(pvarRetValue) = VT_BSTR;
                }
                else {
                  V_BSTR(pvarRetValue) = SysAllocString(L"");
                  V_VT(pvarRetValue) = VT_BSTR;
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 20 : // String2File
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              lIndex = 1;
              hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv1);
              if( SUCCEEDED(hr) ){
                if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
                if( SUCCEEDED(hr) ){
                  if( V_VT(pv1) != VT_BSTR ) hr = VariantChangeTypeEx(pv1,pv1,0,0,VT_BSTR);
                  if( SUCCEEDED(hr) ){
                    AutoPtr<uint8_t> buffer;
                    uintptr_t fsize = base64Decode(V_BSTR(pv0),buffer,0);
                    buffer.alloc(fsize);
                    base64Decode(V_BSTR(pv0),buffer,fsize);
                    AsyncFile file(V_BSTR(pv1));
                    file.createIfNotExist(true).open().writeBuffer(buffer,fsize).resize(fsize).close();
                  }
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 21 : // TimeFromTimeString
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
              if( SUCCEEDED(hr) ){
                V_BSTR(pvarRetValue) = utf8::int2Str(timeFromTimeString(V_BSTR(pv0))).getOLEString();
                V_VT(pvarRetValue) = VT_BSTR;
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 22 : // CreateHashedArray
          msmail1c_->hashedArrays_.add().estimatedChainLength(4);
          V_I4(pvarRetValue) = (LONG) msmail1c_->hashedArrays_.count();
          break;          
        case 23 : // RemoveHashedArray
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_I4 ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_I4);
              if( SUCCEEDED(hr) ){
                if( uintptr_t(V_I4(pv0) - 1) < msmail1c_->hashedArrays_.count() ){
                  msmail1c_->hashedArrays_[uintptr_t(V_I4(pv0) - 1)].drop();
                  msmail1c_->hashedArrays_.remove(V_I4(pv0) - 1);
                  V_I4(pvarRetValue) = 1;
                }
                else {
                  msmail1c_->lastError_ = ERROR_NOT_FOUND;
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;          
        case 24 : // SetHashedArrayValue
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_I4 ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_I4);
              if( SUCCEEDED(hr) ){
                lIndex = 1;
                hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv1);
                if( SUCCEEDED(hr) ){
                  if( V_VT(pv1) != VT_BSTR ) hr = VariantChangeTypeEx(pv1,pv1,0,0,VT_BSTR);
                  if( SUCCEEDED(hr) ){
                    lIndex = 2;
                    hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv2);
                    if( SUCCEEDED(hr) ){
                      if( uintptr_t(V_I4(pv0) - 1) < msmail1c_->hashedArrays_.count() ){
            		        AutoPtr<msmail1c::HashedArrayKey> item(newObjectC1C2<msmail1c::HashedArrayKey>(V_BSTR(pv1),*pv2));
			                  msmail1c::HashedArrayKey * p;
                        msmail1c_->hashedArrays_[uintptr_t(V_I4(pv0) - 1)].insert(item,false,false,&p);
                        if( p != item ) p->value_ = item->value_; else item.ptr(NULL);
                        V_I4(pvarRetValue) = item == NULL ? 1 : 0;
                      }
                      else {
                        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
                      }
                    }
                  }
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;          
        case 25 : // GetHashedArrayValue
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_I4 ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_I4);
              if( SUCCEEDED(hr) ){
                lIndex = 1;
                hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv1);
                if( SUCCEEDED(hr) ){
                  if( V_VT(pv1) != VT_BSTR ) hr = VariantChangeTypeEx(pv1,pv1,0,0,VT_BSTR);
                  if( SUCCEEDED(hr) ){
                    if( uintptr_t(V_I4(pv0) - 1) < msmail1c_->hashedArrays_.count() ){
                      //uintptr_t max = hashedArrays_[V_I4(pv0) - 1].maxChainLength();
                      //uintptr_t min = hashedArrays_[V_I4(pv0) - 1].minChainLength();
                      //uintptr_t avg = hashedArrays_[V_I4(pv0) - 1].avgChainLength();
                      msmail1c::HashedArrayKey * key = msmail1c_->hashedArrays_[uintptr_t(V_I4(pv0) - 1)].find(
                        *newObjectC1<msmail1c::HashedArrayKey>(V_BSTR(pv1))
                      );
                      if( key != NULL ){
                        hr = VariantChangeTypeEx(pvarRetValue,&key->value_,0,0,V_VT(&key->value_));
                      }
                      else {
                        msmail1c_->lastError_ = ERROR_NOT_FOUND;
                        hr = VariantChangeTypeEx(pvarRetValue,pvarRetValue,0,0,VT_EMPTY);
                      }
                    }
                    else {
                      hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
                    }
                  }
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 26 : // RemoveAllHashedArrays
          for( intptr_t i = msmail1c_->hashedArrays_.count() - 1; i >= 0; i-- )
            msmail1c_->hashedArrays_[i].drop();
          msmail1c_->hashedArrays_.clear();
          V_I4(pvarRetValue) = 1;
          break;
        case 27 : // InstallDeviceScanner
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_I4 ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_I4);
              if( SUCCEEDED(hr) ){
                V_I4(pvarRetValue) = msmail1c_->client_.installSerialPortScanner(V_I4(pv0));
                msmail1c_->lastError_ = msmail1c_->client_.workFiberLastError_;
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;          
        case 28 : // RemoveDeviceScanner
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_I4 ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_I4);
              if( SUCCEEDED(hr) ){
                V_I4(pvarRetValue) = msmail1c_->client_.removeSerialPortScanner(V_I4(pv0));
                msmail1c_->lastError_ = msmail1c_->client_.workFiberLastError_;
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 29 : // AttachFileToMessage
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
              if( SUCCEEDED(hr) ){
                lIndex = 1;
                hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv1);
                if( SUCCEEDED(hr) ){
                  if( V_VT(pv1) != VT_BSTR ) hr = VariantChangeTypeEx(pv1,pv1,0,0,VT_BSTR);
                  if( SUCCEEDED(hr) ){
                    lIndex = 2;
                    hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv2);
                    if( SUCCEEDED(hr) ){
                      if( V_VT(pv2) != VT_BSTR ) hr = VariantChangeTypeEx(pv2,pv2,0,0,VT_BSTR);
                      if( SUCCEEDED(hr) ){
                        V_I4(pvarRetValue) = msmail1c_->client_.attachFileToMessage(V_BSTR(pv0),V_BSTR(pv1),V_BSTR(pv2)) ? 1 : 0;
                      }
                    }
                  }
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 30 : // SaveMessageAttachmentToFile
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
              if( SUCCEEDED(hr) ){
                lIndex = 1;
                hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv1);
                if( SUCCEEDED(hr) ){
                  if( V_VT(pv1) != VT_BSTR ) hr = VariantChangeTypeEx(pv1,pv1,0,0,VT_BSTR);
                  if( SUCCEEDED(hr) ){
                    lIndex = 2;
                    hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv2);
                    if( SUCCEEDED(hr) ){
                      if( V_VT(pv2) != VT_BSTR ) hr = VariantChangeTypeEx(pv2,pv2,0,0,VT_BSTR);
                      if( SUCCEEDED(hr) ){
                        V_I4(pvarRetValue) = msmail1c_->client_.saveMessageAttachmentToFile(V_BSTR(pv0),V_BSTR(pv1),V_BSTR(pv2)) ? 1 : 0;
                      }
                    }
                  }
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;        
        case 31 : // ReceiveMessages
          if( !msmail1c_->client_.asyncMessagesReceiving_ ){
            msmail1c_->client_.readConfig(msmail1c_->configFile_,msmail1c_->logFile_);
            hr = SafeArrayLock(*paParams);
            if( SUCCEEDED(hr) ){
              lIndex = 0;
              hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
              if( SUCCEEDED(hr) ){
                if( V_VT(pv0) != VT_I4 ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_I4);
                if( SUCCEEDED(hr) ){
                  V_I4(pvarRetValue) = msmail1c_->client_.receiveMessages(V_I4(pv0) ? true : false) ? 1 : 0;
                }
              }
              SafeArrayUnlock(*paParams);
            }
          }
          break;
        case 32 : // RepairLocking
          msmail1c_->client_.readConfig(msmail1c_->configFile_,msmail1c_->logFile_);
          checkMachineBinding(msmail1c_->client_.config_->value("machine_key"));
          {
            void * proc, * p;
            /*proc = findProcImportedEntryAddress("bkend.dll","KERNEL32.DLL","GetProcAddress");
            readProtectedMemory(proc,&p,sizeof(void *));
            if( oldBKENDGetProcAddress_ == NULL && p != reparedGetProcAddress ){
              void * a = reparedGetProcAddress;
              writeProtectedMemory(proc,&a,sizeof(uintptr_t));
              oldBKENDGetProcAddress_ = p;
              V_I4(pvarRetValue) = 1;
            }
            else {
              msmail1c_->lastError_ = ERROR_ALREADY_ASSIGNED;
            }
            if( V_I4(pvarRetValue) ){*/
              proc = findProcImportedEntryAddress("dbeng32.dll","KERNEL32.DLL","LockFile",true);
              if( proc == NULL )
                proc = findProcImportedEntryAddress("dbeng8.dll","KERNEL32.DLL","LockFile");
              readProtectedMemory(proc,&p,sizeof(void *));
              if( oldDBENG32LockFile_ == NULL && p != repairedLockFile ){
                void * a = repairedLockFile;
                writeProtectedMemory(proc,&a,sizeof(uintptr_t));
                oldDBENG32LockFile_ = p;
              }
              else {
                V_I4(pvarRetValue) = 0;
                msmail1c_->lastError_ = ERROR_ALREADY_ASSIGNED;
              }
            /*}
            if( V_I4(pvarRetValue) ){
              proc = findProcImportedEntryAddress("msvcrt.dll","KERNEL32.DLL","LockFile");
              readProtectedMemory(proc,&p,sizeof(void *));
              if( oldMSVCRTLockFile_ == NULL && p != reparedLockFile ){
                void * a = reparedLockFile;
                writeProtectedMemory(proc,&a,sizeof(uintptr_t));
                oldMSVCRTLockFile_ = p;
              }
              else {
                V_I4(pvarRetValue) = 0;
                msmail1c_->lastError_ = ERROR_ALREADY_ASSIGNED;
              }
            }
            if( V_I4(pvarRetValue) ){
              proc = findProcImportedEntryAddress("mfc42.dll","KERNEL32.DLL","LockFile");
              readProtectedMemory(proc,&p,sizeof(void *));
              if( oldMFC42LockFile_ == NULL && p != reparedLockFile ){
                void * a = reparedLockFile;
                writeProtectedMemory(proc,&a,sizeof(uintptr_t));
                oldMFC42LockFile_ = p;
              }
              else {
                V_I4(pvarRetValue) = 0;
                msmail1c_->lastError_ = ERROR_ALREADY_ASSIGNED;
              }
            }*/
            /*if( V_I4(pvarRetValue) ){
              HMODULE hModule = LoadLibraryA("msvcr71.dll");
              proc = findProcImportedEntryAddress("msvcr71.dll","KERNEL32.DLL","LockFile");
              readProtectedMemory(proc,&p,sizeof(void *));
              if( oldMSVCR71LockFile_ == NULL && p != reparedLockFile ){
                void * a = reparedLockFile;
                writeProtectedMemory(proc,&a,sizeof(uintptr_t));
                oldMSVCR71LockFile_ = p;
              }
              else {
                V_I4(pvarRetValue) = 0;
                msmail1c_->lastError_ = ERROR_ALREADY_ASSIGNED;
              }
            }*//*
            if( V_I4(pvarRetValue) ){
              proc = findProcImportedEntryAddress("seven.dll","KERNEL32.DLL","GetProcAddress");
              readProtectedMemory(proc,&p,sizeof(void *));
              if( oldSEVENGetProcAddress_.p_ == NULL && p != reparedGetProcAddress ){
                void * a = reparedGetProcAddress;
                writeProtectedMemory(proc,&a,sizeof(uintptr_t));
                oldSEVENGetProcAddress_.p_ = p;
                V_I4(pvarRetValue) = 1;
              }
              else {
                V_I4(pvarRetValue) = 0;
                msmail1c_->lastError_ = ERROR_ALREADY_ASSIGNED;
              }
            }*/
          }
          if( flushFileBuffersJmpCodeSafe_[0] == 0 ){
            uint8_t jmpCode[sizeof(lockFileJmpCodeSafe_)] = { 0xB8, 0, 0, 0, 0, 0xFF, 0xE0 };

            /*readProtectedMemory(LockFile,lockFileJmpCodeSafe_,sizeof(lockFileJmpCodeSafe_));
            *(void **) (jmpCode + 1) = repairedLockFile;
            writeProtectedMemory(LockFile,jmpCode,sizeof(jmpCode));

            readProtectedMemory(UnlockFile,unLockFileJmpCodeSafe_,sizeof(unLockFileJmpCodeSafe_));
            *(void **) (jmpCode + 1) = repairedUnlockFile;
            writeProtectedMemory(UnlockFile,jmpCode,sizeof(jmpCode));*/

            readProtectedMemory(FlushFileBuffers,flushFileBuffersJmpCodeSafe_,sizeof(flushFileBuffersJmpCodeSafe_));
            *(void **) (jmpCode + 1) = repairedFlushFileBuffers;
            writeProtectedMemory(FlushFileBuffers,jmpCode,sizeof(jmpCode));

            V_I4(pvarRetValue) = 1;
          }
          else {
            msmail1c_->lastError_ = ERROR_ALREADY_ASSIGNED;
          }
          break;
        case 33 : // RepairServerBusy
          msmail1c_->serverBusyThread_ = newObject<msmail1c::ServerBusyThread>();
          msmail1c_->serverBusyThread_->resume();
          V_I4(pvarRetValue) = 1;
          break;
        default :
          hr = E_NOTIMPL;
      }
    }
    catch( ExceptionSP & e ){
      msmail1c_->lastError_ = e->code() - (e->code() >= errorOffset ? errorOffset : 0);
      hr = HRESULT_FROM_WIN32(msmail1c_->lastError_);
      if( lIndex >= 0 ) SafeArrayUnlock(*paParams);
      e->writeStdError();
    }
  }
//  VariantClear(&v2);
//  VariantClear(&v1);
//  VariantClear(&v0);
  return hr;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Cmsmail1c::msmail1c::LockedFile::~LockedFile()
{
}
//---------------------------------------------------------------------------
Cmsmail1c::msmail1c::LockedFile::LockedFile() : lastError_(0), locked_(false)
{
}
//---------------------------------------------------------------------------
Cmsmail1c::msmail1c::LockedFile * Cmsmail1c::msmail1c::findFileByName(const utf8::String & name)
{
  for( intptr_t i = files_.count() - 1; i >= 0; i-- )
    if( files_[i].file_.fileName().strcasecmp(name) == 0 ) return &files_[i];
  return NULL;
}
//---------------------------------------------------------------------------
Cmsmail1c::msmail1c::LockedFile * Cmsmail1c::msmail1c::addFile(const utf8::String & name)
{
  LockedFile & p = files_.safeAdd(newObject<LockedFile>());
  p.file_.fileName(name).exclusive(false).createIfNotExist(true).removeAfterClose(true);
  return &p;
}
//---------------------------------------------------------------------------
#ifndef DOXYGEN_SHOULD_SKIP_THIS
//---------------------------------------------------------------------------
STDMETHODIMP Cmsmail1c::lockFile(IN BSTR name,IN ULONG minSleepTime,IN ULONG maxSleepTime,OUT LONG * pLastError)
{
  ULONG tm;
  if( maxSleepTime < minSleepTime ){
    tm = maxSleepTime;
    maxSleepTime = minSleepTime;
    minSleepTime = tm;
  }
  msmail1c::LockedFile * file = NULL;
  *pLastError = 0;
  try {
    checkMachineBinding(msmail1c_->client_.config_->value("machine_key"));
    file = msmail1c_->findFileByName(name);
    if( file == NULL ) file = msmail1c_->addFile(name);
    if( !file->file_.isOpen() || !file->locked_ ){
      file->lastError_ = 0;
      file->file_.open();
      file->file_.wrLock(0,0,(msmail1c_->rnd_.random(maxSleepTime - minSleepTime) + minSleepTime) * 1000u);
      file->locked_ = true;
    }
  }
  catch( ExceptionSP & e ){
    *pLastError = e->code() > errorOffset ? e->code() - errorOffset : e->code();
    if( file != NULL ){
      file->lastError_ = *pLastError;
      file->file_.close();
    }
  }
  return S_OK;
}
//---------------------------------------------------------------------------
STDMETHODIMP Cmsmail1c::unlockFile(IN BSTR name,OUT LONG * pLastError)
{
  msmail1c::LockedFile * file = NULL;
  *pLastError = 0;
  try {
    file = msmail1c_->findFileByName(name);
    if( file != NULL ){
      if( file->locked_ ){
        file->file_.unLock(0,0);
        file->locked_ = false;
      }
      file->file_.close();
    }
    else {
      *pLastError = ERROR_FILE_NOT_FOUND;
    }
  }
  catch( ExceptionSP & e ){
    *pLastError = HRESULT_FROM_WIN32(e->code() - errorOffset);
    if( file != NULL ) file->lastError_ = *pLastError;
  }
  return S_OK;
}
//---------------------------------------------------------------------------
STDMETHODIMP Cmsmail1c::getLastError(IN BSTR name,OUT LONG * pLastError)
{
  msmail1c::LockedFile * file = NULL;
  try {
    file = msmail1c_->findFileByName(name);
    if( file != NULL ){
      *pLastError = file->lastError_;
    }
    else {
      *pLastError = ERROR_FILE_NOT_FOUND;
    }
  }
  catch( ExceptionSP & e ){
    *pLastError = e->code() - errorOffset;
    if( file != NULL ) file->lastError_ = *pLastError;
  }
  return S_OK;
}
//---------------------------------------------------------------------------
STDMETHODIMP Cmsmail1c::sleep(IN LONG sleepTime)
{
  Sleep(sleepTime);
  return S_OK;
}
//---------------------------------------------------------------------------
STDMETHODIMP Cmsmail1c::sleepIn(IN ULONG minSleepTime,IN ULONG maxSleepTime)
{
  ULONG tm;
  if( maxSleepTime < minSleepTime ){
    tm = maxSleepTime;
    maxSleepTime = minSleepTime;
    minSleepTime = tm;
  }
  Sleep((DWORD) msmail1c_->rnd_.random(maxSleepTime - minSleepTime) + minSleepTime);
  return S_OK;
}
//---------------------------------------------------------------------------
STDMETHODIMP Cmsmail1c::textToFile(IN BSTR name,IN BSTR text,OUT LONG * pLastError)
{
  HANDLE handle;

  SetLastError(0);
  handle = CreateFileW(
      name,
      GENERIC_READ | GENERIC_WRITE,
      0,
      NULL,
      CREATE_ALWAYS,
      FILE_ATTRIBUTE_NORMAL,
      NULL
  );
  if( handle == INVALID_HANDLE_VALUE ) goto err;
  DWORD NumberOfBytesWritten;
  static const uint8_t unicodeDocMarker[2] = { 0xFE, 0xFF };
  if( WriteFile(handle,unicodeDocMarker,sizeof(unicodeDocMarker),&NumberOfBytesWritten,NULL) == 0 ) goto err;
  if( WriteFile(handle,text,DWORD(wcslen(text) * sizeof(text[0])),&NumberOfBytesWritten,NULL) == 0 ) goto err;
  if( SetEndOfFile(handle) == 0 ) goto err;
  SetLastError(0);
err:
  *pLastError = GetLastError();
  CloseHandle(handle);
  return S_OK;
}
//---------------------------------------------------------------------------
/*FARPROC WINAPI Cmsmail1c::reparedGetProcAddress(HMODULE hModule,LPCSTR lpProcName)
{
  return
    GetModuleHandleA("KERNEL32.DLL") == hModule &&
    strcmp(lpProcName,"LockFile") == 0 ?
      (FARPROC) reparedLockFile :
      oldSEVENGetProcAddress_.f_(hModule,lpProcName)
  ;
}*/
//---------------------------------------------------------------------------
BOOL WINAPI Cmsmail1c::repairedLockFile(
  HANDLE hFile,
  DWORD dwFileOffsetLow,
  DWORD dwFileOffsetHigh,
  DWORD nNumberOfBytesToLockLow,
  DWORD nNumberOfBytesToLockHigh)
{
//  utf8::String name(getFileNameByHandle(hFile));
//  utf8::String ext(getFileNameByHandle(hFile).right(4));
//  DWORD flag = ext.strcasecmp(".lck") != 0 && ext.strcasecmp(".tmp") != 0 ? 0 : LOCKFILE_FAIL_IMMEDIATELY;
  OVERLAPPED overlapped;
  memset(&overlapped,0,sizeof(overlapped));
  overlapped.Offset = dwFileOffsetLow;
  overlapped.OffsetHigh = dwFileOffsetHigh;
  overlapped.hEvent = NULL;//CreateEvent(NULL,TRUE,FALSE,NULL);
  //if( overlapped.hEvent == NULL ) return FALSE;
  BOOL lk = LockFileEx(
    hFile,
    LOCKFILE_EXCLUSIVE_LOCK,// | flag,
    0,
    nNumberOfBytesToLockLow,
    nNumberOfBytesToLockHigh,
    &overlapped
  );
  DWORD err = GetLastError();
  /*if( lk == FALSE && err == ERROR_LOCK_VIOLATION ){
    utf8::String ext(getFileNameByHandle(hFile).right(4));
    if( ext.strcasecmp(".lck") != 0 && ext.strcasecmp(".tmp") != 0 ){
      lk = LockFileEx(
        hFile,
        LOCKFILE_EXCLUSIVE_LOCK,
        0,
        nNumberOfBytesToLockLow,
        nNumberOfBytesToLockHigh,
        &overlapped
      );
      err = GetLastError();
    }
  }*/
  if( lk == FALSE && err == ERROR_IO_PENDING ){
    err = WaitForSingleObject(overlapped.hEvent,1000);
    if( err == WAIT_TIMEOUT ){
      CancelIo(hFile);
      err = WAIT_TIMEOUT;
    }
    else if( err == WAIT_ABANDONED ){
      CancelIo(hFile);
      err = WAIT_TIMEOUT;
    }
    else if( err == WAIT_OBJECT_0 ){
      DWORD NumberOfBytesTransferred;
      GetOverlappedResult(hFile,&overlapped,&NumberOfBytesTransferred,FALSE);
      SetLastError(ERROR_SUCCESS);
      lk = TRUE;
    }
    else if( err == WAIT_FAILED ){
      err = GetLastError();
      CancelIo(hFile);
    }
    else {
      CancelIo(hFile);
      err = ERROR_INVALID_DATA;
    }
  }
//  CloseHandle(overlapped.hEvent);
  SetLastError(err);
  return lk;
}
//------------------------------------------------------------------------------
BOOL WINAPI Cmsmail1c::repairedUnlockFile(
  HANDLE hFile,
  DWORD dwFileOffsetLow,
  DWORD dwFileOffsetHigh,
  DWORD nNumberOfBytesToUnlockLow,
  DWORD nNumberOfBytesToUnlockHigh)
{
  OVERLAPPED overlapped;
  memset(&overlapped,0,sizeof(overlapped));
  overlapped.Offset = dwFileOffsetLow;
  overlapped.OffsetHigh = dwFileOffsetHigh;
  return UnlockFileEx(hFile,0,nNumberOfBytesToUnlockLow,nNumberOfBytesToUnlockHigh,&overlapped);
}
//------------------------------------------------------------------------------
BOOL WINAPI Cmsmail1c::repairedFlushFileBuffers(HANDLE /*hFile*/)
{
//  utf8::String name(getFileNameByHandle(hFile));
//  utf8::String ext(getFileNameByHandle(hFile).right(4));
//  if( ext.strcasecmp(".lck") != 0 && ext.strcasecmp(".tmp") != 0 ){
  return TRUE;
}
//------------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
