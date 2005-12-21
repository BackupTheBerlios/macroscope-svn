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
#ifndef _Sysutils_H_
#define _Sysutils_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
extern uint8_t argvPlaceHolder[sizeof(Array<utf8::String>)];
inline Array<utf8::String> & argv(){
  return *reinterpret_cast<Array<utf8::String> *>(argvPlaceHolder);
}
extern char pathDelimiter;
extern char pathDelimiterStr[2];
//---------------------------------------------------------------------------
void initializeArguments(int argc,char ** argv);
void initialize();
void cleanup();
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
extern DWORD mainThreadId;
extern UINT threadFinishMessage;
extern UINT fiberFinishMessage;
inline bool isWin9x()
{
  DWORD dwVersion = GetVersion();
  return (dwVersion >> 31) == 1 && (dwVersion & 0xFF) == 4;
}
class UUID : public GUID {};
#else
inline bool isWin9x()
{
  return false;
}
#if HAVE_SYS_UUID_H
class UUID : public uuid {};
#else
#error you system not have sys/uuid.h
#endif
#endif
//---------------------------------------------------------------------------
UUID createUUID();
void copyStrToClipboard(const utf8::String & s);
//---------------------------------------------------------------------------
inline uintptr_t strlen(const char * s)
{
  return (uintptr_t) ::strlen(s);
}
//---------------------------------------------------------------------------
utf8::String screenChar(const utf8::String::Iterator & ii);
utf8::String screenString(const utf8::String & s);
utf8::String unScreenChar(const utf8::String::Iterator & ii,uintptr_t & screenLen);
utf8::String unScreenString(const utf8::String & s);
uintptr_t enumStringParts(const utf8::String & s,const char * const delim = ",;");
utf8::String stringPartByNo(const utf8::String & s,uintptr_t n,const char * delim = ",;");
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
HMODULE getModuleHandleByAddr(void * addr = NULL);
utf8::String getModuleFileNameByHandle(HMODULE h);
#endif
utf8::String getCurrentDir();
utf8::String getTempPath();
utf8::String changeFileExt(const utf8::String & fileName,const utf8::String & extension);
utf8::String getExecutableName();
utf8::String getExecutablePath();
utf8::String excludeTrailingPathDelimiter(const utf8::String & path);
utf8::String includeTrailingPathDelimiter(const utf8::String & path);
bool createDirectory(const utf8::String & name);
void chModOwn(
  const utf8::String & pathName,
  const Mutant & mode,
  const Mutant & user,
  const Mutant & group
);
bool removeDirectory(const utf8::String & name,bool recursive = true);
bool remove(const utf8::String & name);
utf8::String getPathFromPathName(const utf8::String & pathName);
utf8::String getNameFromPathName(const utf8::String & pathName);
utf8::String anyPathName2HostPathName(const utf8::String & pathName);
void getDirList(
  Vector<utf8::String> & list,
  const utf8::String & dirAndMask,
  const utf8::String & exMask = utf8::String(),
  bool recursive = true
);
bool isPathNameRelative(const utf8::String & pathName);
utf8::String absolutePathNameFromWorkDir(
  const utf8::String & workDir,
  const utf8::String & pathName);
bool utime(const utf8::String & pathName,const struct utimbuf & times);
//---------------------------------------------------------------------------
utf8::String base64Encode(const void * p,uintptr_t l);
uintptr_t base64Decode(const utf8::String & s,void * p,uintptr_t size);
//---------------------------------------------------------------------------
inline utf8::String uuid2base64(const UUID & u)
{
  return base64Encode(&u,sizeof(u));
}
//---------------------------------------------------------------------------
inline UUID base642uuid(const utf8::String & s)
{
  UUID uuid;
  base64Decode(s,&uuid,sizeof(uuid));
  return uuid;
}
//---------------------------------------------------------------------------
struct Stat : public
#if HAVE_STATI64
  stati64
#elif HAVE__STATI64
  _stati64
#else
  stat
#endif
{
};
bool stat(const utf8::String & pathName,struct Stat & st);
//---------------------------------------------------------------------------
typedef utf8::String (* StrErrorHandler)(int32_t);
typedef Array<StrErrorHandler> StrErrorHandlers;
extern uint8_t strErrorHandlersHolder[];
inline StrErrorHandlers & strErrorHandlers()
{
  return *reinterpret_cast<StrErrorHandlers *>(strErrorHandlersHolder);
}
void addStrErrorHandler(StrErrorHandler strErrorHandler);
utf8::String strError(int32_t err);
void strErrorInitialize();
void strErrorCleanup();
#if defined(__WIN32__) || defined(__WIN64__)
const int errorOffset = 5000;
#else
const int errorOffset = 0;
#endif
//---------------------------------------------------------------------------
/*struct PACKED MFA {
  void * func;
  void * obj;
  MFA();
  
  bool IsNull() const;
};

inline MFA::MFA() : func(NULL), obj(NULL)
{
}  

inline bool MFA::IsNull() const
{
  return func == NULL || obj == NULL;
}  

template <class T,class A>
struct PACKED MFA_ {
  union PACKED {
    T ptr;
    MFA a;
  };
  MFA_(const T & aptr,A * aobj);
};

template <class T,class A> inline
MFA_<T,A>::MFA_(const T & aptr,A * aobj)
{ 
  ptr = aptr;
  a.obj = aobj;
}

template <class T,class A> inline
MFA GetMFA(const T & ptr,A * obj)
{
  return MFA_<T,A>(ptr,obj).a;
} */
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#if !HAVE_GETPAGESIZE
#if defined(__WIN32__) || defined(__WIN64__)
inline uintptr_t getpagesize()
{
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return si.dwPageSize;
}
#elif HAVE_SYSCONF && defined(_SC_PAGESIZE)
inline uintptr_t getpagesize()
{
  return sysconf(_SC_PAGESIZE);
}
#elif HAVE_SYSCONF && defined(_SC_PAGE_SIZE)
inline uintptr_t getpagesize()
{
  return sysconf(_SC_PAGE_SIZE);
}
#elif __i386__
inline uintptr_t getpagesize()
{
  return 4096;
}
#else
inline uintptr_t getpagesize()
{
  return 8192;
}
#endif
#endif
//---------------------------------------------------------------------------
#if !HAVE_GETPID && (defined(__WIN32__) || defined(__WIN64__))
inline uintptr_t getpid()
{
  return GetCurrentProcessId();
}
#endif
//---------------------------------------------------------------------------
#endif /* _Sysutils_H_ */
