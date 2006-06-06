#ifndef _Registry_H_
#define _Registry_H_
//---------------------------------------------------------------------------
bool RegistryKeyValueExists(HKEY RootKey,const wchar_t * KeyName,const wchar_t * Name);
HRESULT GetRegistryKeyValue(
  HKEY RootKey,
  const wchar_t * KeyName,
  const wchar_t * Name,
  VARIANT * Result);
HRESULT SetRegistryKeyValue(HKEY RootKey,const wchar_t * KeyName,const wchar_t * Name,const wchar_t * Val);
HRESULT DeleteRegistryKeyValue(HKEY RootKey,const wchar_t * KeyName,const wchar_t * Name);
HRESULT DeleteRegistryKey(HKEY RootKey,const wchar_t * KeyName);
//---------------------------------------------------------------------------
wchar_t * wset(const wchar_t * str);
wchar_t * wset(wchar_t * dst,const wchar_t * str);
wchar_t * wcat(wchar_t * dst,const wchar_t * str);
//---------------------------------------------------------------------------
#endif /* _Registry_H_ */
//---------------------------------------------------------------------------
