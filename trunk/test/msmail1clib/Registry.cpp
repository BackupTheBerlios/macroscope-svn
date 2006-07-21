#include "stdafx.h"
#include <windows.h>
#include "Registry.h"
//---------------------------------------------------------------------------
bool RegistryKeyValueExists(HKEY RootKey,const wchar_t * KeyName,const wchar_t * Name)
{
  bool r = false;
  HKEY hkey;
  if( RegOpenKeyExW(RootKey,KeyName,0,KEY_READ,&hkey) == ERROR_SUCCESS ){
    DWORD ValueType, ValueSize;
    r = RegQueryValueExW(hkey,Name,NULL,&ValueType,NULL,&ValueSize) == ERROR_SUCCESS;
  }
  return r;
}
//---------------------------------------------------------------------------
HRESULT GetRegistryKeyValue(
  HKEY RootKey,
  const wchar_t * KeyName,
  const wchar_t * Name,
  VARIANT * Result)
{
  HRESULT hr = E_FAIL;
  HKEY hkey;
  DWORD err;
   
  err = RegOpenKeyExW(RootKey,KeyName,0,KEY_READ,&hkey);
  if( err == ERROR_SUCCESS ){
    DWORD ValueType, ValueSize, sLen;
    err = RegQueryValueExW(hkey,Name,NULL,&ValueType,NULL,&ValueSize);
    if( err == ERROR_SUCCESS ){
      PBYTE pValueData = (PBYTE) malloc(ValueSize);
      if( pValueData == NULL ){
        err = ERROR_NOT_ENOUGH_MEMORY;
      }
      else {
        err = RegQueryValueExW(hkey,Name,NULL,&ValueType,pValueData,&ValueSize);
        if( err == ERROR_SUCCESS ){
          BSTR sValue = NULL;
          switch( ValueType ){
            case REG_BINARY :
              break;
            case REG_DWORD :
              if( SUCCEEDED(VariantChangeTypeEx(Result,Result,0,0,VT_I4)) ){
                V_I4(Result) = *(DWORD *) pValueData;
                hr = S_OK;
              }
              break;
            case REG_EXPAND_SZ :
              sLen = ExpandEnvironmentStringsW((LPCWSTR) pValueData,sValue,0);
              if( sLen == 0 ){
                err = GetLastError();
              }
              else {
                sValue = SysAllocStringLen(NULL,sLen - 1);
                if( sValue == NULL ){
                  err = ERROR_NOT_ENOUGH_MEMORY;
                }
                else {
                  ExpandEnvironmentStringsW((LPCWSTR) pValueData,sValue,sLen);
                  if( SUCCEEDED(VariantChangeTypeEx(Result,Result,0,0,VT_EMPTY)) ){
                    V_BSTR(Result) = sValue;
                    V_VT(Result) = VT_BSTR;
                    hr = S_OK;
                    sValue = NULL;
                  }
                  SysFreeString(sValue);
                }
              }
              /*Val.resize(ExpandEnvironmentStringsW((LPCWSTR) pValueData,Val.c_str(),0) + 1);
              ExpandEnvironmentStringsW((LPCWSTR) pValueData,Val.c_str(),Val.strlen());
              Val.resize(wcslen(Val.c_str()));*/
              break;
            case REG_LINK :
            case REG_MULTI_SZ :
            case REG_NONE :
            case REG_RESOURCE_LIST :
              hr = E_FAIL;
              break;
            case REG_SZ :
              sLen = (DWORD) wcslen((LPCWSTR) pValueData);
              sValue = SysAllocStringLen(NULL,sLen);
              if( sValue == NULL ){
                err = ERROR_NOT_ENOUGH_MEMORY;
              }
              else {
                if( SUCCEEDED(VariantChangeTypeEx(Result,Result,0,0,VT_EMPTY)) ){
                  V_BSTR(Result) = sValue;
                  V_VT(Result) = VT_BSTR;
                  hr = S_OK;
                  sValue = NULL;
                }
                SysFreeString(sValue);
              }
              /*Val = (wchar_t *) pValueData;*/
              break;
          }
        }
        free(pValueData);
      }
    }
    RegCloseKey(hkey);
  }
  SetLastError(err);
  return hr;
}
//---------------------------------------------------------------------------
HRESULT SetRegistryKeyValue(HKEY RootKey,const wchar_t * KeyName,const wchar_t * Name,const wchar_t * Val)
{
  HRESULT hr;
  HKEY hkey;
  DWORD ckey = REG_CREATED_NEW_KEY;
  if( (hr = RegCreateKeyExW(RootKey,KeyName,0,L"",REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hkey,&ckey)) == ERROR_SUCCESS ){
    hr = RegSetValueExW(hkey,Name,0,REG_SZ,(const BYTE *) Val,DWORD((wcslen(Val) + 1) * sizeof(wchar_t)));
    RegCloseKey(hkey);
  }
  return hr;
}
//---------------------------------------------------------------------------
HRESULT DeleteRegistryKeyValue(HKEY RootKey,const wchar_t * KeyName,const wchar_t * Name)
{
  HRESULT hr;
  HKEY hkey;
  if( (hr = RegOpenKeyExW(RootKey,KeyName,0,KEY_ALL_ACCESS,&hkey)) == ERROR_SUCCESS ){
    hr = RegDeleteValueW(hkey,Name);
    RegCloseKey(hkey);
  }
  return hr;
}
//---------------------------------------------------------------------------
HRESULT DeleteRegistryKey(HKEY RootKey,const wchar_t * KeyName)
{
  HKEY hkey;
  HRESULT hr;
  if( (hr = RegOpenKeyExW(RootKey,KeyName,0,KEY_ENUMERATE_SUB_KEYS | KEY_READ | KEY_WRITE,&hkey)) == ERROR_SUCCESS ){
    DWORD SubKeyNameLen, SubKeyNameLen2;
    wchar_t * SubKeyName, * SubKeyName2;
    for(;;){
      LONG a;
      SubKeyName = (wchar_t *) malloc(sizeof(wchar_t));
      SubKeyName[0] = L'\0';
      SubKeyNameLen = 1;
      for(;;){
        SubKeyNameLen2 = SubKeyNameLen;
        a = RegEnumKeyExW(hkey,0,SubKeyName,&SubKeyNameLen2,NULL,NULL,NULL,NULL);
        if( a != ERROR_MORE_DATA ) break;
        SubKeyName2 = (wchar_t *) realloc(SubKeyName,sizeof(wchar_t) * (SubKeyNameLen << 1));
        if( SubKeyName2 == NULL ) break;
        SubKeyName = SubKeyName2;
        SubKeyNameLen <<= 1;
      }  
      if( a != ERROR_SUCCESS || a == ERROR_NO_MORE_ITEMS ){
        free(SubKeyName);
        break;
      }
      hr = RegDeleteKeyW(hkey,SubKeyName);
      if( FAILED(hr) ){
        SubKeyName2 = (wchar_t *) malloc((wcslen(KeyName) + 1 + wcslen(SubKeyName) + 1) * sizeof(wchar_t));
        if( SubKeyName2 != NULL ){
          wcscpy_s(SubKeyName2,~rsize_t(0),KeyName);
          wcscat_s(SubKeyName2,~rsize_t(0),L"\\");
          wcscat_s(SubKeyName2,~rsize_t(0),SubKeyName);
          hr = DeleteRegistryKey(RootKey,SubKeyName2);
          free(SubKeyName2);
        }
      }
      free(SubKeyName);
      if( FAILED(hr) ) break;
    }
    RegCloseKey(hkey);
    if( SUCCEEDED(hr) ) hr = RegDeleteKeyW(RootKey,KeyName);
  }
  return hr;
}
//---------------------------------------------------------------------------
wchar_t * wset(const wchar_t * str)
{
  wchar_t * a = (wchar_t *) malloc(sizeof(wchar_t) * ((str != NULL ? wcslen(str) : 0) + 1));
  if( str != NULL ) wcscpy_s(a,~rsize_t(0),str); else a[0] = L'\0';
  return a;
}
//---------------------------------------------------------------------------
wchar_t * wset(wchar_t * dst,const wchar_t * str)
{
  wchar_t * a = (wchar_t *) realloc(dst,sizeof(wchar_t) * ((str != NULL ? wcslen(str) : 0) + 1));
  if( a != NULL ){
    if( str != NULL ) wcscpy_s(a,~rsize_t(0),str);
  }
  else {
    a = dst;
  }
  return a;
}
//---------------------------------------------------------------------------
wchar_t * wcat(wchar_t * dst,const wchar_t * str)
{
  wchar_t * a = (wchar_t *) realloc(dst,sizeof(wchar_t) * ((dst != NULL ? wcslen(dst) : 0) + (str != NULL ? wcslen(str) : 0) + 1));
  if( a != NULL ){
    if( dst != NULL ) wcscpy_s(a,~rsize_t(0),dst); else a[0] = L'\0';
    if( str != NULL ) wcscat_s(a,~rsize_t(0),str);
  }
  else {
    a = dst;
  }
  return a;
}
//---------------------------------------------------------------------------
