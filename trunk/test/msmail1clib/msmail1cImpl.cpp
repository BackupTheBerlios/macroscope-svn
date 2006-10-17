/*-
 * Copyright (C) 2005-2006 Guram Dukashvili. All rights reserved.
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
Cmsmail1c::~Cmsmail1c()
{
}
//------------------------------------------------------------------------------
Cmsmail1c::Cmsmail1c() :
  pBackConnection_(NULL),
  pAsyncEvent_(NULL),
  active_(false),
  lastError_(0)
{
}
//------------------------------------------------------------------------------
// IInitDone Methods
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::Init(LPDISPATCH pBackConnection)
{
//  stdErr.enableDebugLevel(9);
  pBackConnection_ = pBackConnection;
  pBackConnection_->AddRef();
  return S_OK;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::Done()
{
  client_.close();
  if( pAsyncEvent_ != NULL ){
    pAsyncEvent_->Release();
    pAsyncEvent_ = NULL;
  }
  if( pBackConnection_ != NULL ) pBackConnection_->Release();
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
  *plProps = 23;
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
        V_BSTR(pvarPropVal) = name_.getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 1 :
        if( V_VT(pvarPropVal) != VT_I4 ) hr = VariantChangeTypeEx(pvarPropVal,pvarPropVal,0,0,VT_I4);
        if( SUCCEEDED(hr) ) V_I4(pvarPropVal) = active_ ? 1 : 0;
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
        if( SUCCEEDED(hr) ) V_I4(pvarPropVal) = lastError_;
        break;
      case 4 : // User
        V_BSTR(pvarPropVal) = user_.getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 5 : // Key
        V_BSTR(pvarPropVal) = key_.getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 6 : // Groups
        V_BSTR(pvarPropVal) = groups_.getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 7 : // MailServer
        V_BSTR(pvarPropVal) = mailServer_.getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 8 : // ConfigFile
        V_BSTR(pvarPropVal) = configFile_.getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 9 : // LogFile
        V_BSTR(pvarPropVal) = logFile_.getOLEString();
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
        {
          guid_t uuid;
          createGUID(uuid);
          utf8::String suuid(base32Encode(&uuid,sizeof(uuid)));
          V_BSTR(pvarPropVal) = suuid.getOLEString();
          V_VT(pvarPropVal) = VT_BSTR;
        }
        break;
      case 13 : // RandomNumber
        V_BSTR(pvarPropVal) = utf8::int2Str(rnd_.random()).getOLEString();
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
          if( active_ ){
            AutoLock<FiberInterlockedMutex> lock(client_.connectedMutex_);
            V_I4(pvarPropVal) = client_.connected_ ? 1 : 0;
          }
        }
        break;
      case 16 : // WorkServer
        {
          AutoLock<FiberInterlockedMutex> lock(client_.connectedMutex_);
          if( active_ && client_.connected_ ){
            V_BSTR(pvarPropVal) = client_.connectedToServer_.getOLEString();
            V_VT(pvarPropVal) = VT_BSTR;
          }
          else {
            if( V_VT(pvarPropVal) != VT_BSTR ) hr = VariantChangeTypeEx(pvarPropVal,pvarPropVal,0,0,VT_BSTR);
          }
        }
        break;
      case 17 : // ReceivedMessagesList
        V_BSTR(pvarPropVal) = client_.getReceivedMessageList().getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 18 : // DBList
        V_BSTR(pvarPropVal) = client_.getDBList().getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 19 : // DBGroupList
        V_BSTR(pvarPropVal) = client_.getDBGroupList().getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 20 : // UserList
        V_BSTR(pvarPropVal) = client_.getUserList().getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 21 : // SendingMessagesList
        V_BSTR(pvarPropVal) = client_.getSendingMessageList().getOLEString();
        V_VT(pvarPropVal) = VT_BSTR;
        break;
      case 22 : // MK1100Port
        if( V_VT(pvarPropVal) != VT_I4 ) hr = VariantChangeTypeEx(pvarPropVal,pvarPropVal,0,0,VT_I4);
        if( SUCCEEDED(hr) ) V_I4(pvarPropVal) = client_.mk1100Port_;
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
        if( SUCCEEDED(hr) ) name_ = V_BSTR(varPropVal);
        break;
      case 1 :
        if( V_VT(varPropVal) != VT_I4 ) hr = VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_I4);
        if( SUCCEEDED(hr) ){
          if( V_I4(varPropVal) != 0 && pAsyncEvent_ == NULL ){
            hr = pBackConnection_->QueryInterface(IID_IAsyncEvent,(void **) &pAsyncEvent_);
            if( SUCCEEDED(hr) ) hr = pAsyncEvent_->SetEventBufferDepth(1000000);
          }
          if( SUCCEEDED(hr) ){
            if( V_I4(varPropVal) != 0 ){
              client_.close();
              active_ = false;
              client_.pAsyncEvent_ = pAsyncEvent_;
              client_.name_ = name_;
              client_.user_ = user_;
              client_.key_ = key_;
              client_.groups_ = groups_;
              client_.mailServer_ = mailServer_;
              client_.configFile_ = configFile_;
              client_.logFile_ = logFile_;
              stdErr.fileName(logFile_);
              client_.config_->parse();
              stdErr.rotationThreshold(client_.config_->value("debug_file_rotate_threshold",1024 * 1024));
              stdErr.rotatedFileCount(client_.config_->value("debug_file_rotate_count",10));
              stdErr.setDebugLevels(client_.config_->value("debug_levels","+0,+1,+2,+3"));
              stdErr.fileName(
                client_.config_->value("log_file",stdErr.fileName())
              );
              stdErr.setRedirect(
                client_.config_->value("log_redirect",utf8::String())
              );
              if( !isRet() ) client_.open();
              active_ = true;
            }
            else {
              client_.close();
              active_ = false;
            }
            if( V_I4(varPropVal) == 0 && pAsyncEvent_ != NULL ){
              pAsyncEvent_->Release();
              pAsyncEvent_ = NULL;
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
        if( SUCCEEDED(hr) ) user_ = V_BSTR(varPropVal);
        break;
      case 5 : // Key
        if( V_VT(varPropVal) != VT_BSTR ) hr = VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_BSTR);
        if( SUCCEEDED(hr) ) key_ = V_BSTR(varPropVal);
        break;
      case 6 : // Groups
        if( V_VT(varPropVal) != VT_BSTR ) hr = VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_BSTR);
        if( SUCCEEDED(hr) ) groups_ = V_BSTR(varPropVal);
        break;
      case 7 : // MailServer
        if( V_VT(varPropVal) != VT_BSTR ) hr = VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_BSTR);
        if( SUCCEEDED(hr) ) mailServer_ = V_BSTR(varPropVal);
        break;
      case 8 : // ConfigFile
        if( V_VT(varPropVal) != VT_BSTR ) hr = VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_BSTR);
        if( SUCCEEDED(hr) ) configFile_ = V_BSTR(varPropVal);
        break;
      case 9 : // LogFile
        if( V_VT(varPropVal) != VT_BSTR ) hr = VariantChangeTypeEx(varPropVal,varPropVal,0,0,VT_BSTR);
        if( SUCCEEDED(hr) ) logFile_ = V_BSTR(varPropVal);
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
        hr = E_NOTIMPL;
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
    default : return E_NOTIMPL;
  }
  return S_OK;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::GetNMethods(long * plMethods)
{
  *plMethods = 22;
  return S_OK;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::FindMethod(BSTR bstrMethodName,long * plMethodNum)
{
  *plMethodNum = -1;
  if( _wcsicoll(bstrMethodName,L"LockFile") == 0 ) *plMethodNum = 0;
  else
  if( _wcsicoll(bstrMethodName,L"Ѕлокировать‘айл") == 0 ) *plMethodNum = 0;
  else
  if( _wcsicoll(bstrMethodName,L"UnlockFile") == 0 ) *plMethodNum = 1;
  else
  if( _wcsicoll(bstrMethodName,L"–азблокировать‘айл") == 0 ) *plMethodNum = 1;
  else
  if( _wcsicoll(bstrMethodName,L"GetLastError") == 0 ) *plMethodNum = 2;
  else
  if( _wcsicoll(bstrMethodName,L"ѕолучить одќшибки") == 0 ) *plMethodNum = 2;
  else
  if( _wcsicoll(bstrMethodName,L"Sleep") == 0 ) *plMethodNum = 3;
  else
  if( _wcsicoll(bstrMethodName,L"—пать") == 0 ) *plMethodNum = 3;
  else
  if( _wcsicoll(bstrMethodName,L"SleepIn") == 0 ) *plMethodNum = 4;
  else
  if( _wcsicoll(bstrMethodName,L"—пать¬") == 0 ) *plMethodNum = 4;
  else
  if( _wcsicoll(bstrMethodName,L"TextToFile") == 0 ) *plMethodNum = 5;
  else
  if( _wcsicoll(bstrMethodName,L"“екст¬‘айл") == 0 ) *plMethodNum = 5;
  else
  if( _wcsicoll(bstrMethodName,L"NewMessage") == 0 ) *plMethodNum = 6;
  else
  if( _wcsicoll(bstrMethodName,L"Ќовое—ообщение") == 0 ) *plMethodNum = 6;
  else
  if( _wcsicoll(bstrMethodName,L"SetMessageAttribute") == 0 ) *plMethodNum = 7;
  else
  if( _wcsicoll(bstrMethodName,L"”становитьјтрибут—ообщени€") == 0 ) *plMethodNum = 7;
  else
  if( _wcsicoll(bstrMethodName,L"GetMessageAttribute") == 0 ) *plMethodNum = 8;
  else
  if( _wcsicoll(bstrMethodName,L"ѕолучитьјтрибут—ообщени€") == 0 ) *plMethodNum = 8;
  else
  if( _wcsicoll(bstrMethodName,L"SendMessage") == 0 ) *plMethodNum = 9;
  else
  if( _wcsicoll(bstrMethodName,L"ѕослать—ообщение") == 0 ) *plMethodNum = 9;
  else
  if( _wcsicoll(bstrMethodName,L"RemoveMessage") == 0 ) *plMethodNum = 10;
  else
  if( _wcsicoll(bstrMethodName,L"”далить—ообщение") == 0 ) *plMethodNum = 10;
  else
  if( _wcsicoll(bstrMethodName,L"GetDB") == 0 ) *plMethodNum = 11;
  else
  if( _wcsicoll(bstrMethodName,L"ѕолучить»Ѕ") == 0 ) *plMethodNum = 11;
  else
  if( _wcsicoll(bstrMethodName,L"CopyMessage") == 0 ) *plMethodNum = 12;
  else
  if( _wcsicoll(bstrMethodName,L"—копировать—ообщение") == 0 ) *plMethodNum = 12;
  else
  if( _wcsicoll(bstrMethodName,L"RemoveMessageAttribute") == 0 ) *plMethodNum = 13;
  else
  if( _wcsicoll(bstrMethodName,L"”далитьјтрибут—ообщени€") == 0 ) *plMethodNum = 13;
  else
  if( _wcsicoll(bstrMethodName,L"DebugMessage") == 0 ) *plMethodNum = 14;
  else
  if( _wcsicoll(bstrMethodName,L"ќтладочное—ообщение") == 0 ) *plMethodNum = 14;
  else
  if( _wcsicoll(bstrMethodName,L"IsDirectory") == 0 ) *plMethodNum = 15;
  else
  if( _wcsicoll(bstrMethodName,L"Ёто аталог") == 0 ) *plMethodNum = 15;
  else
  if( _wcsicoll(bstrMethodName,L"RemoveDirectory") == 0 ) *plMethodNum = 16;
  else
  if( _wcsicoll(bstrMethodName,L"”далить аталог") == 0 ) *plMethodNum = 16;
  else
  if( _wcsicoll(bstrMethodName,L"MK1100SendBarCodeInfo") == 0 ) *plMethodNum = 17;
  else
  if( _wcsicoll(bstrMethodName,L"MK1100ѕослать»нформациюЎтрихкода") == 0 ) *plMethodNum = 17;
  else
  if( _wcsicoll(bstrMethodName,L"GetDBInGroupList") == 0 ) *plMethodNum = 18;
  else
  if( _wcsicoll(bstrMethodName,L"—писок»Ѕ¬√руппе") == 0 ) *plMethodNum = 18;
  else
  if( _wcsicoll(bstrMethodName,L"File2String") == 0 ) *plMethodNum = 19;
  else
  if( _wcsicoll(bstrMethodName,L"‘айл¬—троку") == 0 ) *plMethodNum = 19;
  else
  if( _wcsicoll(bstrMethodName,L"String2File") == 0 ) *plMethodNum = 20;
  else
  if( _wcsicoll(bstrMethodName,L"—троку¬‘айл") == 0 ) *plMethodNum = 20;
  else
  if( _wcsicoll(bstrMethodName,L"TimeFromTimeString") == 0 ) *plMethodNum = 21;
  else
  if( _wcsicoll(bstrMethodName,L"¬рем€»з¬ремени—трокой") == 0 ) *plMethodNum = 21;
  else
    return DISP_E_MEMBERNOTFOUND;
  return S_OK;
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
    case 9 : *plParams = 1; break;
    case 10 : *plParams = 1; break;
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
    default : return E_NOTIMPL;
  }
  return S_OK;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::GetParamDefValue(long lMethodNum,long lParamNum,VARIANT * pvarParamDefValue)
{
  return S_OK;
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
          if( !active_ ) Exception::throwSP(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__);
          V_BSTR(pvarRetValue) = client_.newMessage().getOLEString();
          V_VT(pvarRetValue) = VT_BSTR;
          break;
        case 7 : // SetMessageAttribute
          if( !active_ ) Exception::throwSP(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__);
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
                        V_BSTR(pvarRetValue) = client_.value(V_BSTR(pv0),V_BSTR(pv1),V_BSTR(pv2)).getOLEString();
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
          if( !active_ ) Exception::throwSP(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__);
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
                    hr = client_.value(V_BSTR(pv0),V_BSTR(pv1),pvarRetValue);
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 9 : // SendMessage
          if( !active_ ) Exception::throwSP(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__);
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
              if( SUCCEEDED(hr) ){
                if( isRet() ){
                  V_I4(pvarRetValue) = 1;
                  lastError_ = 0;
                }
                else {
                  V_I4(pvarRetValue) = client_.sendMessage(V_BSTR(pv0)) ? 1 : 0;
                  lastError_ = client_.workFiberLastError_ - (client_.workFiberLastError_ >= errorOffset ? errorOffset : 0);
                }
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 10 : // RemoveMessage
          if( !active_ ) Exception::throwSP(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__);
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
              if( SUCCEEDED(hr) ){
                V_I4(pvarRetValue) = client_.removeMessage(V_BSTR(pv0));
                lastError_ = client_.workFiberLastError_ - (client_.workFiberLastError_ >= errorOffset ? errorOffset : 0);
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 11 : // GetDB
          if( !active_ ) Exception::throwSP(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__);
          client_.getDB();
          V_I4(pvarRetValue) = 1;
          break;
        case 12 : // CopyMessage
          if( !active_ ) Exception::throwSP(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__);
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
              if( SUCCEEDED(hr) ){
                V_BSTR(pvarRetValue) = client_.copyMessage(V_BSTR(pv0)).getOLEString();
                V_VT(pvarRetValue) = VT_BSTR;
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 13 : // RemoveMessageAttribute
          if( !active_ ) Exception::throwSP(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__);
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
                    V_BSTR(pvarRetValue) = client_.removeValue(V_BSTR(pv0),V_BSTR(pv1)).getOLEString();
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
                  lastError_ = GetLastError();
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
          if( !active_ ) Exception::throwSP(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__);
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
//                  if( SUCCEEDED(hr) )
//                    hr = client_.value(V_BSTR(pv0),V_BSTR(pv1),pvarRetValue);
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
                V_BSTR(pvarRetValue) = client_.getDBInGroupList(V_BSTR(pv0)).getOLEString();
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
          if( !active_ ) Exception::throwSP(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__);
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
          
        default :
          hr = E_NOTIMPL;
      }
    }
    catch( ExceptionSP & e ){
      lastError_ = e->code() - (e->code() >= errorOffset ? errorOffset : 0);
      hr = HRESULT_FROM_WIN32(lastError_);
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
Cmsmail1c::LockedFile::~LockedFile()
{
  CloseHandle(handle_);
  CloseHandle(hEvent_);
}
//---------------------------------------------------------------------------
Cmsmail1c::LockedFile::LockedFile() :
  handle_(INVALID_HANDLE_VALUE), hEvent_(NULL), lastError_(0), locked_(false)
{
}
//---------------------------------------------------------------------------
Cmsmail1c::LockedFile * Cmsmail1c::findFileByName(const utf8::String & name)
{
  for( intptr_t i = files_.count() - 1; i >= 0; i-- )
    if( files_[i].name_.strcasecmp(name) == 0 ) return &files_[i];
  return NULL;
}
//---------------------------------------------------------------------------
Cmsmail1c::LockedFile * Cmsmail1c::addFile(const utf8::String & name)
{
  LockedFile & p = files_.safeAdd(newObject<LockedFile>());
  p.name_ = name;
  return &p;
}
//---------------------------------------------------------------------------
STDMETHODIMP Cmsmail1c::lockFile(IN BSTR name,IN ULONG minSleepTime,IN ULONG maxSleepTime,OUT LONG * pLastError)
{
  if( isRet() ){
    *pLastError = 0;
    return S_OK;
  }
//---------------------------------------------------------------------
  ULONG tm;
  if( maxSleepTime < minSleepTime ){
    tm = maxSleepTime;
    maxSleepTime = minSleepTime;
    minSleepTime = tm;
  }
  OVERLAPPED Overlapped;
  int32_t err;
  LockedFile * file = NULL;
  *pLastError = 0;
  try {
    file = findFileByName(name);
    if( file == NULL ) file = addFile(name);
    file->lastError_ = 0;
    if( file->handle_ == INVALID_HANDLE_VALUE ){
      SetLastError(0);
      file->handle_ = CreateFileW(
          file->name_.getUNICODEString(),
          GENERIC_READ | GENERIC_WRITE,
          FILE_SHARE_READ | FILE_SHARE_WRITE,
          NULL,
          CREATE_ALWAYS,
          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
          NULL
      );
      if( file->handle_ == INVALID_HANDLE_VALUE ){
        err = GetLastError() + errorOffset;
        Exception::throwSP(err,__PRETTY_FUNCTION__);
      }
    }
    SetLastError(0);
    if( file->hEvent_ == NULL ){
      if( (file->hEvent_ = CreateEvent(NULL,TRUE,FALSE,NULL)) == NULL ){
          err = GetLastError() + errorOffset;
          Exception::throwSP(err,__PRETTY_FUNCTION__);
      }
    }
    if( !file->locked_ ){
      memset(&Overlapped,0,sizeof(Overlapped));
      Overlapped.hEvent = file->hEvent_;
      Overlapped.Offset = 0;//1024u * 1024u;
      SetLastError(0);
      DWORD flags = LOCKFILE_EXCLUSIVE_LOCK;
      if( maxSleepTime == 0 && minSleepTime == 0 ) flags |= LOCKFILE_FAIL_IMMEDIATELY;
      BOOL lk = LockFileEx(file->handle_,flags,0,~DWORD(0),~DWORD(0),&Overlapped);
      if( lk == 0 && GetLastError() != ERROR_IO_PENDING && GetLastError() != ERROR_LOCK_VIOLATION ){
          err = GetLastError() + errorOffset;
          Exception::throwSP(err,__PRETTY_FUNCTION__);
      }
      if( GetLastError() == ERROR_IO_PENDING ){
        DWORD st = (DWORD) rnd_.random(maxSleepTime - minSleepTime) + minSleepTime;
        st = maxSleepTime == 0 && minSleepTime == 0 ? 0 : st;
        DWORD state = WaitForSingleObject(file->hEvent_,st);
        if( state == WAIT_TIMEOUT ){
          CancelIo(file->handle_);
          SetLastError(WAIT_TIMEOUT);
          err = GetLastError() + errorOffset;
          Exception::throwSP(err,__PRETTY_FUNCTION__);
        }
        else if( state == WAIT_ABANDONED ){
          SetLastError(WAIT_TIMEOUT);
          err = GetLastError() + errorOffset;
          Exception::throwSP(err,__PRETTY_FUNCTION__);
        }
        DWORD NumberOfBytesTransferred;
        if( GetOverlappedResult(file->handle_,&Overlapped,&NumberOfBytesTransferred,FALSE) == 0 ){
          err = GetLastError() + errorOffset;
          Exception::throwSP(err,__PRETTY_FUNCTION__);
        }
      }
      else if( GetLastError() == ERROR_LOCK_VIOLATION ){
          SetLastError(WAIT_TIMEOUT);
          err = GetLastError() + errorOffset;
          Exception::throwSP(err,__PRETTY_FUNCTION__);
      }
      /*char pid[10 + 1];
      memset(pid,'\0',sizeof(pid));
      _snprintf(pid,sizeof(pid) / sizeof(pid[0]),"%d",ksys::getpid());
      DWORD NumberOfBytesWritten = 0;
      memset(&Overlapped,0,sizeof(Overlapped));
      Overlapped.hEvent = file->hEvent_;
      SetLastError(0);
      lk = WriteFile(file->handle_,pid,DWORD(::strlen(pid) * sizeof(pid[0])),&NumberOfBytesWritten,&Overlapped);
      if( lk == 0 && GetLastError() == ERROR_IO_PENDING ){
        DWORD state = WaitForSingleObject(file->hEvent_,INFINITE);
        if( state == WAIT_TIMEOUT ){
          CancelIo(file->handle_);
        }
        else if( state == WAIT_ABANDONED ){
        }
        if( GetOverlappedResult(file->handle_,&Overlapped,&NumberOfBytesWritten,FALSE) == 0 ){
        }
      }
      SetFilePointer(file->handle_,NumberOfBytesWritten,NULL,FILE_BEGIN);
      SetEndOfFile(file->handle_);*/
      file->locked_ = true;
    }
  }
  catch( ExceptionSP & e ){
    *pLastError = e->code() - errorOffset;
    if( file != NULL ) file->lastError_ = *pLastError;
    e->writeStdError();
  }
  return S_OK;
}
//---------------------------------------------------------------------------
STDMETHODIMP Cmsmail1c::unlockFile(IN BSTR name,OUT LONG * pLastError)
{
  LockedFile * file = NULL;
  *pLastError = 0;
  try {
    file = findFileByName(name);
    if( file != NULL && file->locked_ ){
      OVERLAPPED Overlapped;
      memset(&Overlapped,0,sizeof(Overlapped));
      if( UnlockFileEx(file->handle_,0,~DWORD(0),~DWORD(0),&Overlapped) == 0 ){
        int32_t err = GetLastError() + errorOffset;
        Exception::throwSP(err,__PRETTY_FUNCTION__);
      }
//      CloseHandle(file->handle_);
//      file->handle_ = INVALID_HANDLE_VALUE;
      file->locked_ = false;
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
  LockedFile * file = NULL;
  try {
    file = findFileByName(name);
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
  Sleep((DWORD) rnd_.random(maxSleepTime - minSleepTime) + minSleepTime);
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
