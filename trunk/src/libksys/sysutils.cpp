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
uint8_t argvPlaceHolder[sizeof(Array<utf8::String>)];
#if defined(__WIN32__) || defined(__WIN64__)
char pathDelimiter = '\\';
char pathDelimiterStr[2] = "\\";
#else
char pathDelimiter = '/';
char pathDelimiterStr[2] = "/";
#endif
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
DWORD mainThreadId;
UINT threadFinishMessage;
UINT fiberFinishMessage;
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
UUID createGuid()
{
#if defined(__WIN32__) || defined(__WIN64__)
  union {
    GUID guid;
    UUID u;
  };
  if( FAILED(CoCreateGuid(&guid)) ){
    int32_t err = GetLastError() + errorOffset;
#elif HAVE_UUIDGEN
  UUID u;
  if( uuidgen(&u,1) != 0 ){
    int32_t err = errno;
#endif
    throw ExceptionSP(new Exception(err,utf8::string(__PRETTY_FUNCTION__)));
  }
  return u;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
utf8::String screenChar(const utf8::String::Iterator & ii)
{
  utf8::String a;
  char * p, b[12];
  switch( ii.getChar() ){
    case '\a' :
      p = "\\a";
      break;
    case '\b' :
      p = "\\b";
      break;
    case '\t' :
      p = "\\t";
      break;
    case '\v' :
      p = "\\v";
      break;
    case '\f' :
      p = "\\f";
      break;
    case '\r' :
      p = "\\r";
      break;
    case '\n' :
      p = "\\n";
      break;
    case '\\' :
      p = "\\\\";
      break;
    case ',' :
      p = "\\,";
      break;
    case ';' :
      p = "\\;";
      break;
    case '\"' :
      p = "\\\"";
      break;
    case '=' :
      p = "\\=";
      break;
    default  :
      if( ii.isCntrl() || ii.isSpace() )
        return utf8::string("\\0x") +  utf8::int2HexStr(ii.getChar(),SIZEOF_WCHAR_T * 2);
      memset(b,0,sizeof(b));
      memcpy(b,ii.c_str(),ii.seqLen());
      p = b;
  }
  return utf8::plane(p);
}
//---------------------------------------------------------------------------
utf8::String unScreenChar(const utf8::String::Iterator & ii,uintptr_t & screenLen)
{
  utf8::String s;
  screenLen = 1;
  switch( ii.getChar() ){
    case '0' :
      if( ii[screenLen].getUpperChar() == 'X' ){
        screenLen++;
        while( screenLen < SIZEOF_WCHAR_T * 2 + 2 && ii[screenLen].isXdigit() ) screenLen++;
        if( screenLen == SIZEOF_WCHAR_T * 2 + 2 ){
          intmax_t a;
          if( utf8::tryStr2Int(utf8::string(ii + 2,ii + SIZEOF_WCHAR_T * 2 + 2),a,16) ){
            char b[5];
            b[utf8::ucs2utf8seq(b,(uintptr_t) a)] = '\0';
            s = utf8::plane(b);
            break;
          }
        }
      }
      else {
        while( screenLen < SIZEOF_WCHAR_T * 2 + 3 && ii[screenLen].isDigit() ) screenLen++;
        if( screenLen == SIZEOF_WCHAR_T * 2 + 3 ){
          intmax_t a;
          if( utf8::tryStr2Int(utf8::string(ii + 1,ii + SIZEOF_WCHAR_T * 2 + 3),a,10) ){
            char b[5];
            b[utf8::ucs2utf8seq(b,(uintptr_t) a)] = '\0';
            s = utf8::plane(b);
            break;
          }
        }
      }
      screenLen = 1;
      goto l1;
    case 'a' :
      s = "\a";
      break;
    case 'b' :
      s = "\b";
      break;
    case 't' :
      s = "\t";
      break;
    case 'v' :
      s = "\v";
      break;
    case 'f' :
      s = "\f";
      break;
    case 'r' :
      s = "\r";
      break;
    case 'n' :
      s = "\n";
      break;
    case '\\' :
      s = "\\";
      break;
    default :
l1:   s = utf8::string(ii,ii + 1);
  }
  return s;
}
//---------------------------------------------------------------------------
utf8::String unScreenString(const utf8::String & s)
{
  utf8::String a;
  utf8::String::Iterator i(s);
  while( !i.eof() ){
    if( i.getChar() == '\\' && i.next() ){
      uintptr_t screenLen;
      a += unScreenChar(i,screenLen);
      i += screenLen;
    }
    else {
      a += utf8::string(i,i + 1);
      i.next();
    }
  }
  return a;
}
//---------------------------------------------------------------------------
utf8::String screenString(const utf8::String & s)
{
  utf8::String a;
  utf8::String::Iterator i(s);
  while( !i.eof() ){
    a += screenChar(i);
    i.next();
  }
  return a;
}
//---------------------------------------------------------------------------
uintptr_t enumStringParts(const utf8::String & s,const char * const delim)
{
  bool inQuotationMarks = false;
  uintptr_t l, k = 0;
  utf8::String::Iterator i(s);
  if( !i.eof() ) for(;;){
    uintptr_t c = i.getChar();
    if( c == '\\' ){
      if( i.next() ){
        uintptr_t screenLen;
        unScreenChar(i,screenLen);
        i += screenLen;
      }
    }
    else {
      if( c == '\"' ){
        inQuotationMarks = !inQuotationMarks;
      }
      else if( !inQuotationMarks ){
        const char * d = delim;
        for(;;){
          if( utf8::utf82ucs(d,l) == c ){
            k++;
            break;
          }
          if( *d == '\0' ) break;
          d += l;
        }
      }
      i.next();
    }
    if( c == 0 ) break;
  }
  return k;
}
//---------------------------------------------------------------------------
utf8::String stringPartByNo(const utf8::String & s,uintptr_t n,const char * delim)
{
  bool inQuotationMarks = false;
  uintptr_t l, k = 0;
  utf8::String::Iterator i(s), q(s);
  for(;;){
    uintptr_t c = i.getChar();
    if( c == '\\' ){
      if( i.next() ){
        uintptr_t screenLen;
        unScreenChar(i,screenLen);
        i += screenLen;
      }
    }
    else {
      if( c == '\"' ){
        inQuotationMarks = !inQuotationMarks;
      }
      else if( !inQuotationMarks ){
        const char * d = delim;
        for(;;){
          if( utf8::utf82ucs(d,l) == c ){
            if( k == n ) return utf8::string(q,i);
            q = i + 1;
            k++;
            break;
          }
          if( *d == '\0' ) break;
          d += l;
        }
      }
      i.next();
    }
    if( c == 0 ) break;
  }
  return utf8::String();
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
HMODULE getModuleHandleByAddr(void * addr)
{
  MODULEENTRY32 me;
  HANDLE hSnapshot;
  HMODULE h = NULL;
  if( addr == NULL ) addr = (void *) getModuleHandleByAddr;
  if( (hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,0)) != INVALID_HANDLE_VALUE ){
    me.dwSize = sizeof(me);
    if( Module32First(hSnapshot,&me) == TRUE ) do {
      if( (uintptr_t) me.modBaseAddr                  <= (uintptr_t) addr &&
          (uintptr_t) me.modBaseAddr + me.modBaseSize >  (uintptr_t) addr ){
        h = me.hModule;
        break;
      }
    } while( Module32Next(hSnapshot,&me) == TRUE );
    CloseHandle(hSnapshot);
  }
  return h;
}
#endif
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
utf8::String getModuleFileNameByHandle(HMODULE h)
{
  DWORD a = 1, b;
  if( isWin9x() ){
    AutoPtr<char> moduleFileName;
    for(;;){
      moduleFileName.realloc(a * sizeof(char));
      if( (b = GetModuleFileNameA(h,moduleFileName,a)) < a ) break;
      a <<= 1;
    }
    moduleFileName.realloc((b + 1) * sizeof(char));
    return utf8::string(moduleFileName);
  }
  AutoPtr<wchar_t> moduleFileName;
  for(;;){
    moduleFileName.realloc(a * sizeof(wchar_t));
    if( (b = GetModuleFileNameW(h,moduleFileName,a)) < a ) break;
    a <<= 1;
  }
  moduleFileName.realloc((b + 1) * sizeof(wchar_t));
  return utf8::string(moduleFileName);
}
#endif
//---------------------------------------------------------------------------
utf8::String getCurrentDir()
{
#if defined(__WIN32__) || defined(__WIN64__)
  DWORD a;
  if( isWin9x() ){
    AutoPtr<char> dirName;
    dirName.realloc((a = GetCurrentDirectoryA(0,NULL)) * sizeof(char));
    GetCurrentDirectoryA(a,dirName);
    return utf8::string(dirName);
  }
  AutoPtr<wchar_t> dirName;
  dirName.realloc((a = GetCurrentDirectoryW(0,NULL)) * sizeof(wchar_t));
  GetCurrentDirectoryW(a,dirName);
  return utf8::string(dirName);
#else
  AutoPtr<char> dirName((char *) kmalloc(MAXPATHLEN + 1 + 1));
  if( getcwd(dirName,MAXPATHLEN + 1) == NULL ) strcpy(dirName,".");
  strcat(dirName,pathDelimiterStr);
  return utf8::string(dirName);
#endif
}
//---------------------------------------------------------------------------
utf8::String getTempPath()
{
#if defined(__WIN32__) || defined(__WIN64__)
  DWORD a;
  if( isWin9x() ){
    AutoPtr<char> dirName;
    dirName.realloc((a = GetTempPathA(0,NULL) + 2) * sizeof(char));
    GetTempPathA(a,dirName);
    if( dirName[strlen(dirName) - 1] != '\\' ) strcat(dirName,"\\");
    return utf8::string(dirName);
  }
  AutoPtr<wchar_t> dirName;
  dirName.realloc((a = GetTempPathW(0,NULL) + 2) * sizeof(wchar_t));
  GetTempPathW(a,dirName);
  if( dirName[lstrlenW(dirName) - 1] != '\\' ) lstrcatW(dirName,L"\\");
  return utf8::string(dirName);
#elif HAVE_PATHS_H
  return utf8::string(_PATH_TMP);
#else
#error you system not have paths.h
#endif
}
//---------------------------------------------------------------------------
utf8::String changeFileExt(const utf8::String & fileName,const utf8::String & extension)
{
  utf8::String::Iterator i(fileName);
  i.last();
  while( !i.bof() ){
    switch( i.getChar() ){
      case '\\' : case ':' : break;
      case '.'  :
        return utf8::string(utf8::String::Iterator(fileName),i) + "." + extension;
      default   : i.prev();
    }
  }
  return fileName + "." + extension;
}
//---------------------------------------------------------------------------
utf8::String getExecutableName()
{
#if defined(__WIN32__) || defined(__WIN64__)
  HMODULE h = getModuleHandleByAddr();
  if( h == NULL ) h = GetModuleHandle(NULL);
  return getModuleFileNameByHandle(h);
#else
  assert( argv().count() > 0 );
  return argv()[0];
#endif
}
//---------------------------------------------------------------------------
utf8::String getExecutablePath()
{
  utf8::String name(getExecutableName());
  utf8::String::Iterator i(name);
  i.last();
  while( !i.bof() ){
    if( i.getChar() == (uintptr_t) pathDelimiter ) break;
    i.prev();
  }
  return utf8::string(utf8::String::Iterator(name),i + 1);
}
//---------------------------------------------------------------------------
utf8::String excludeTrailingPathDelimiter(const utf8::String & path)
{
  utf8::String s(path);
  utf8::String::Iterator i(s);
  i.last();
  if( i.getChar() == (uintptr_t) pathDelimiter )
    s = s.unique().resize(s.size() - i.seqLen());
  return s;
}
//---------------------------------------------------------------------------
utf8::String includeTrailingPathDelimiter(const utf8::String & path)
{
  utf8::String s(path);
  utf8::String::Iterator i(s);
  i.last();
  if( i.getChar() != (uintptr_t) pathDelimiter ){
    char b[2];
    b[0] = pathDelimiter;
    b[1] = '\0';
    s = s.unique() + b;
  }
  return s;
}
//---------------------------------------------------------------------------
bool isPathNameRelative(const utf8::String & pathName)
{
  utf8::String::Iterator i(pathName);
#if defined(__WIN32__) || defined(__WIN64__)
  uintptr_t c = 0;
#endif
  while( !i.eof() ){
    if( !i.isSpace() ){
      if( i.getChar() == (uintptr_t) pathDelimiter ) return false;
#if defined(__WIN32__) || defined(__WIN64__)
      if( c != 0 && i.getChar() == ':' ) return false;
      if( c == 0 && (i.getUpperChar() < 'A' || i.getUpperChar() > 'Z') ) break;
      c = i.getChar();
#endif
    }
    i.next();
  }
  return true;
}
//---------------------------------------------------------------------------
utf8::String absolutePathNameFromWorkDir(
  const utf8::String & workDir,
  const utf8::String & pathName)
{
  if( isPathNameRelative(pathName) ) return workDir + pathName;
  return pathName;
}
//---------------------------------------------------------------------------
bool createDirectory(const utf8::String & name)
{
  int32_t err = 0;
#if defined(__WIN32__) || defined(__WIN64__)
  if( isWin9x() ){
    if( CreateDirectoryA(anyPathName2HostPathName(name).getANSIString(),NULL) == 0 )
      err = GetLastError();
  }
  else {
    if( CreateDirectoryW(anyPathName2HostPathName(name).getUNICODEString(),NULL) == 0 )
      err = GetLastError();
  }
  if( err == ERROR_PATH_NOT_FOUND ){
#else
  mode_t um = umask(0);
  umask(um);
  if( mkdir(anyPathName2HostPathName(name).getANSIString(),um | S_IRUSR | S_IWUSR | S_IXUSR) != 0 )
    err = errno;
  if( err == ENOENT ){
#endif
    utf8::String::Iterator i(anyPathName2HostPathName(name));
    i.last();
    while( !i.bof() ){
      if( i.getChar() == (uintptr_t) pathDelimiter ) break;
      i.prev();
    }
    if( i.bof() || (i - 1).getChar() == ':' )
      throw ExceptionSP(new Exception(-1,utf8::string(__PRETTY_FUNCTION__)));
    createDirectory(utf8::string(utf8::String::Iterator(name),i));
    return createDirectory(name);
  }
#if defined(__WIN32__) || defined(__WIN64__)
  if( err != 0 && err != ERROR_ALREADY_EXISTS )
    throw ExceptionSP(new Exception(err + errorOffset,utf8::string(__PRETTY_FUNCTION__)));
#else
  if( err != 0 && err != EEXIST )
    throw ExceptionSP(new Exception(err,utf8::string(__PRETTY_FUNCTION__)));
#endif
  return err == 0;
}
//---------------------------------------------------------------------------
bool removeDirectory(const utf8::String & name,bool recursive)
{
  int32_t err = 0;
#if defined(__WIN32__) || defined(__WIN64__)
  if( isWin9x() ){
    if( RemoveDirectoryA(anyPathName2HostPathName(name).getANSIString()) == 0 )
      err = GetLastError();
  }
  else {
    if( RemoveDirectoryW(anyPathName2HostPathName(name).getUNICODEString()) == 0 )
      err = GetLastError();
  }
  if( err == ERROR_PATH_NOT_FOUND || err == ERROR_FILE_NOT_FOUND ) return false;
  if( err == ERROR_DIR_NOT_EMPTY ){
#else
  if( rmdir(anyPathName2HostPathName(name).getANSIString()) != 0 )
    err = errno;
  if( err == ENOENT ) return false;
  if( err == ENOTEMPTY ){
#endif
    Vector<utf8::String> list;
    getDirList(list,name + pathDelimiterStr + "*",utf8::String(),recursive);
    bool haveDir = false;
    for( intptr_t i = list.count() - 1; i >= 0; i-- ){
      struct Stat st;
      stat(list[i],st);
      haveDir = (st.st_mode & S_IFDIR) != 0 || haveDir;
      if( haveDir && recursive ){
        removeDirectory(list[i],true);
      }
      else {
        remove(list[i]);
      }
    }
    return !(haveDir && !recursive);
  }
  if( err != 0 )
#if defined(__WIN32__) || defined(__WIN64__)
    throw ExceptionSP(new Exception(err + errorOffset,utf8::string(__PRETTY_FUNCTION__)));
#else
    throw ExceptionSP(new Exception(err,utf8::string(__PRETTY_FUNCTION__)));
#endif
  return true;
}
//---------------------------------------------------------------------------
bool remove(const utf8::String & name)
{
  int32_t err = 0;
#if defined(__WIN32__) || defined(__WIN64__)
  if( isWin9x() ){
    if( DeleteFileA(anyPathName2HostPathName(name).getANSIString()) == 0 )
      err = GetLastError();
  }
  else {
    if( DeleteFileW(anyPathName2HostPathName(name).getUNICODEString()) == 0 )
      err = GetLastError();
  }
  if( err == ERROR_PATH_NOT_FOUND || err == ERROR_FILE_NOT_FOUND )  return false;
#else
  if( unlink(anyPathName2HostPathName(name).getANSIString()) != 0 )
    err = errno;
  if( err == ENOENT ) return false;
#endif
  if( err != 0 ){
    struct Stat st;
    stat(name,st);
    if( (st.st_mode & S_IFDIR) != 0 ){
      removeDirectory(name,true);
      return remove(name);
    }
#if defined(__WIN32__) || defined(__WIN64__)
    throw ExceptionSP(new Exception(err + errorOffset,utf8::string(__PRETTY_FUNCTION__)));
#else
    throw ExceptionSP(new Exception(err,utf8::string(__PRETTY_FUNCTION__)));
#endif
  }
  return true;
}
//---------------------------------------------------------------------------
void chModOwn(
#if !defined(__WIN32__) && !defined(__WIN64__)
  const utf8::String & pathName,
  const Mutant & mode,const Mutant & user,const Mutant & group)
{
  int32_t err;
  intmax_t m = umask(0);
  umask((mode_t) m);
  if( mode.type() == mtInt ){
    m = mode;
  }
  else {
    utf8::tryStr2Int(mode,m,8);
  }
  if( chmod(anyPathName2HostPathName(pathName).getANSIString(),(mode_t) m) != 0 ){
    err = errno;
    throw ExceptionSP(new Exception(err,utf8::string(__PRETTY_FUNCTION__)));
  }
  const struct passwd * u = getpwnam(utf8::String(user).getANSIString());
  uid_t userID(u != NULL ? u->pw_uid : (uid_t) user);
  const struct group * g = getgrnam(utf8::String(group).getANSIString());
  gid_t groupID(g != NULL ? g->gr_gid : (gid_t) group);
  if( chown(anyPathName2HostPathName(pathName).getANSIString(),userID,groupID) != 0 ){
    err = errno;
    throw ExceptionSP(new Exception(err,utf8::string(__PRETTY_FUNCTION__)));
  }
#else
  const utf8::String &,const Mutant &,const Mutant &,const Mutant &)
{
#endif
}
//---------------------------------------------------------------------------
utf8::String getPathFromPathName(const utf8::String & pathName)
{
  utf8::String::Iterator i(pathName);
  i.last();
  while( !i.bof() ){
    if( i.getChar() == (uintptr_t) pathDelimiter )
      return utf8::string(utf8::String::Iterator(pathName),i);
    i.prev();
  }
  return utf8::string(".");
}
//---------------------------------------------------------------------------
utf8::String getNameFromPathName(const utf8::String & pathName)
{
  utf8::String::Iterator i(pathName);
  i.last();
  while( !i.bof() ){
    if( i.getChar() == (uintptr_t) pathDelimiter )
      return utf8::string(i + 1,utf8::String::Iterator(pathName).last());
    i.prev();
  }
  return utf8::string(i);
}
//---------------------------------------------------------------------------
utf8::String anyPathName2HostPathName(const utf8::String & pathName)
{
  utf8::String::Iterator i(
    pathName.strstr(utf8::string(pathDelimiter == '\\' ? "/" : "\\"))
  );
  utf8::String s(pathName);
  if( i.position() >= 0 ){
    s = s.unique();
    i = s;
    uintptr_t c = pathDelimiter == '\\' ? '/' : '\\';
    while( !i.eof() ){
      if( i.getChar() == c )
        s.replace(i,i + 1,utf8::string(pathDelimiterStr));
      i.prev(); 
    }
  }
  return s;
}
//---------------------------------------------------------------------------
bool nameFitMask(const utf8::String & name,const utf8::String & mask)
{
  utf8::String::Iterator ni(name), mi(mask);
  while( !ni.eof() && !mi.eof() ){
    if( mi.getChar() == '?' ){
    }
    else if( mi.getChar() == '*' ){
      mi.next();
#if defined(__WIN32__) || defined(__WIN64__)
      while( ni.next() && ni.getUpperChar() != mi.getUpperChar() );
#else
      while( ni.next() && ni.getChar() != mi.getChar() );
#endif
      mi.next();
      continue;
    }
    else if( mi.getChar() == '[' ){
//      mi.next();
      throw ExceptionSP(
        new Exception(-1,utf8::string(__PRETTY_FUNCTION__) + " FIXME")
      );
    }
    else
#if defined(__WIN32__) || defined(__WIN64__)
      if( ni.getUpperChar() != mi.getUpperChar() ) break;
#else
      if( ni.getChar() != mi.getChar() ) break;
#endif
    ni.next();
    mi.next();
  }
  return ni.eof() && mi.eof();
}
//---------------------------------------------------------------------------
void getDirList(
  Vector<utf8::String> & list,
  const utf8::String & dirAndMask,
  const utf8::String & exMask,
  bool recursive)
{
  int32_t err;
  utf8::String path(getPathFromPathName(dirAndMask));
  utf8::String mask(getNameFromPathName(dirAndMask));

#if defined(__WIN32__) || defined(__WIN64__)
  union {
    WIN32_FIND_DATAA fda;
    WIN32_FIND_DATAW fdw;
  };
  HANDLE handle;
  if( isWin9x() ){
    handle = FindFirstFileA(anyPathName2HostPathName(path + pathDelimiterStr + "*").getANSIString(),&fda);
    if( handle == INVALID_HANDLE_VALUE ){
      err = GetLastError() + errorOffset;
      throw ExceptionSP(new Exception(err,utf8::string(__PRETTY_FUNCTION__)));
    }
    try {
      do {
        if( strcmp(fda.cFileName,".") == 0 || strcmp(fda.cFileName,"..") == 0 ) continue;
        const char * d_name = fda.cFileName;
        if( (fda.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ){
          if( recursive )
#else
    DIR * dir = opendir(anyPathName2HostPathName(path).getANSIString());
    if( dir == NULL ){
      err = errno;
      throw ExceptionSP(new Exception(err,utf8::string(__PRETTY_FUNCTION__)));
    }
    try {
      struct dirent * ent;
#if HAVE_READDIR_R
      struct dirent * result, res;
      ent = &res;
      for(;;){
        if( readdir_r(dir,ent,&result) != 0 ){
          err = errno;
          throw ExceptionSP(new Exception(err,utf8::string(__PRETTY_FUNCTION__)));
        }
        if( result == NULL ) break;
#else
        while( (ent = readdir(dir)) != NULL ){
#endif
        if( strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0 ) continue;
        const char * d_name = ent->d_name;
#ifdef DT_DIR
        if( ent->d_type == DT_DIR ){
          if( recursive )
#else
        struct Stat st;
        stat(path + pathDelimiterStr + ent->d_name,st);
        if( (st.st_mode & S_IFDIR) != 0 ){
          if( recursive )
#endif
#endif
            getDirList(
              list,
              path + pathDelimiterStr + d_name + pathDelimiterStr + mask,
              exMask,
              recursive
            );
        }
        else if( nameFitMask(utf8::string(d_name),mask) && !nameFitMask(utf8::string(d_name),exMask) ){
          list.add(path + pathDelimiterStr + d_name);
        }
#if defined(__WIN32__) || defined(__WIN64__)
      } while( FindNextFileA(handle,&fda) != 0 );
      if( GetLastError() != ERROR_NO_MORE_FILES ){
        err = GetLastError() + errorOffset;
        throw ExceptionSP(new Exception(err,utf8::string(__PRETTY_FUNCTION__)));
      }
    }
    catch( ... ){
      FindClose(handle);
      throw;
    }
    FindClose(handle);
  }
  else {
    handle = FindFirstFileW(anyPathName2HostPathName(path + pathDelimiterStr + "*").getUNICODEString(),&fdw);
    if( handle == INVALID_HANDLE_VALUE ){
      err = GetLastError() + errorOffset;
      throw ExceptionSP(new Exception(err,utf8::string(__PRETTY_FUNCTION__)));
    }
    try {
      do {
        if( lstrcmpW(fdw.cFileName,L".") == 0 || lstrcmpW(fdw.cFileName,L"..") == 0 ) continue;
        if( (fdw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ){
          if( recursive )
            getDirList(
              list,
              path + pathDelimiterStr + fdw.cFileName + pathDelimiterStr + mask,
              exMask,
              recursive
            );
        }
        else if( nameFitMask(utf8::string(fdw.cFileName),mask) && !nameFitMask(utf8::string(fdw.cFileName),exMask) ){
          list.add(path + pathDelimiterStr + fdw.cFileName);
        }
      } while( FindNextFileW(handle,&fdw) != 0 );
      if( GetLastError() != ERROR_NO_MORE_FILES ){
        err = GetLastError() + errorOffset;
        throw ExceptionSP(new Exception(err,utf8::string(__PRETTY_FUNCTION__)));
      }
    }
    catch( ... ){
      FindClose(handle);
      throw;
    }
    FindClose(handle);
  }
#else
      }
    }
    catch( ... ){
      closedir(dir);
      throw;
    }
    closedir(dir);
#endif
}
//---------------------------------------------------------------------------
// переключение раскладки клавиатуры
// LoadKeyboardLayout("00000419", KLF_ACTIVATE); //русский
//---------------------------------------------------------------------------
void copyStrToClipboard(const utf8::String & s)
{
#if defined(__WIN32__) || defined(__WIN64__)
  int32_t err;
  if( OpenClipboard(NULL) == 0 ){
    err = GetLastError() + errorOffset;
    throw ksys::ExceptionSP(new ksys::Exception(err,utf8::string(__PRETTY_FUNCTION__)));
  }
  if( EmptyClipboard() == 0 ){
    err = GetLastError() + errorOffset;
    throw ksys::ExceptionSP(new ksys::Exception(err,utf8::string(__PRETTY_FUNCTION__)));
  }
  if( ksys::isWin9x() ){
    utf8::AnsiString pass(s.getANSIString());
    HGLOBAL w = GlobalAlloc(
      GMEM_MOVEABLE | GMEM_DDESHARE,
      (strlen(pass) + 1) * sizeof(char)
    );
    if( w == NULL ){
      err = GetLastError() + errorOffset;
      CloseClipboard();
      throw ksys::ExceptionSP(new ksys::Exception(err,utf8::string(__PRETTY_FUNCTION__)));
    }
    LPVOID ww = GlobalLock(w);
    if( ww == NULL ){
      err = GetLastError() + errorOffset;
      CloseClipboard();
      throw ksys::ExceptionSP(new ksys::Exception(err,utf8::string(__PRETTY_FUNCTION__)));
    }
    memcpy(ww,(const char *) pass,(strlen(pass) + 1) * sizeof(char));
    GlobalUnlock(ww);
    if( SetClipboardData(CF_TEXT,w) == NULL ){
      err = GetLastError() + errorOffset;
      GlobalFree(w);
      CloseClipboard();
      throw ksys::ExceptionSP(new ksys::Exception(err,utf8::string(__PRETTY_FUNCTION__)));
    }
  }
  else {
    utf8::WideString pass(s.getUNICODEString());
    HGLOBAL w = GlobalAlloc(
      GMEM_MOVEABLE | GMEM_DDESHARE,
      (lstrlenW(pass) + 1) * sizeof(wchar_t)
    );
    if( w == NULL ){
      err = GetLastError() + errorOffset;
      CloseClipboard();
      throw ksys::ExceptionSP(new ksys::Exception(err,utf8::string(__PRETTY_FUNCTION__)));
    }
    LPVOID ww = GlobalLock(w);
    if( ww == NULL ){
      err = GetLastError() + errorOffset;
      CloseClipboard();
      throw ksys::ExceptionSP(new ksys::Exception(err,utf8::string(__PRETTY_FUNCTION__)));
    }
    memcpy(ww,(const wchar_t *) pass,(lstrlenW(pass) + 1) * sizeof(wchar_t));
    GlobalUnlock(ww);
    if( SetClipboardData(CF_UNICODETEXT,w) == NULL ){
      err = GetLastError() + errorOffset;
      GlobalFree(w);
      CloseClipboard();
      throw ksys::ExceptionSP(new ksys::Exception(err,utf8::string(__PRETTY_FUNCTION__)));
    }
  }
  CloseClipboard();
#endif
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
static BOOL WINAPI consoleCtrlHandler(DWORD dwCtrlType)
{
  switch( dwCtrlType ){
    default                  :
    case CTRL_C_EVENT        :
    case CTRL_BREAK_EVENT    :
    case CTRL_CLOSE_EVENT    :
    case CTRL_LOGOFF_EVENT   :
    case CTRL_SHUTDOWN_EVENT :
      while( PostThreadMessage(mainThreadId,WM_QUIT,0,0) == 0 ) Sleep(1);
  }
  return TRUE;
}
#endif
//---------------------------------------------------------------------------
// base64 routines
//---------------------------------------------------------------------------
static const char base64Table[64] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q',
  'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
  'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
  'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '-'
};
//---------------------------------------------------------------------------
static uint8_t base64DecodeTable[256];
//---------------------------------------------------------------------------
static inline uint8_t base64Encode6(const uint8_t * p,uintptr_t i)
{
  return (uint8_t) (((*(uint16_t *) (p + (i >> 3))) >> (i & 7)) & 63u);
}
//---------------------------------------------------------------------------
utf8::String base64Encode(const void * p,uintptr_t l)
{
  utf8::String s;
  if( l > 0 ){
    uintptr_t i, k;
    s.resize(((l << 3) / 6) + ((l << 3) % 6 != 0));
    char * sp = s.c_str();
    k = (l << 3) - 8;
    for( i = 0; i < k; i += 6 ){
      *sp++ = base64Table[base64Encode6((uint8_t *) p,i)];
    }
    union {
      uint8_t c[2];
      uint32_t c32;
    };
    c32 = *((uint8_t *) p + (i >> 3));
    while( i < k + 8 ){
      *sp++ = base64Table[base64Encode6(c,i & 7)];
      i += 6;
    }
  }
  return s;
}
//---------------------------------------------------------------------------
uintptr_t base64Decode(const utf8::String & s,void * p,uintptr_t size)
{
  uintptr_t i = 0;
  utf8::String::Iterator sp(s);
  size <<= 3;
  while( !sp.eof() ){
    uintptr_t c = sp.getChar();
    if( c >= 256 ) throw ExceptionSP(new Exception(-1,utf8::string(__PRETTY_FUNCTION__)));
    c = base64DecodeTable[c];
    if( c >= 64 ) throw ExceptionSP(new Exception(-1,utf8::string(__PRETTY_FUNCTION__)));
    if( i < size ){
      if( size - i <= 8 ){
        *(uint8_t *) ((uint8_t *) p + (i >> 3)) &= (uint8_t) (~(63u << (i & 7)));
        *(uint8_t *) ((uint8_t *) p + (i >> 3)) |= (uint8_t) (c << (i & 7));
      }
      else {
        *(uint16_t *) ((uint8_t *) p + (i >> 3)) &= (uint16_t) (~(63u << (i & 7)));
        *(uint16_t *) ((uint8_t *) p + (i >> 3)) |= (uint16_t) (c << (i & 7));
      }
    }
    i += 6;
    sp++;
  }
  return (i >> 3) + ((i & 7) != 0);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// System initialization and finalization ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
void initializeArguments(int argc,char ** argv)
{
  ksys::argv().clear();
  for( int i = 0; i < argc; i++ ) ksys::argv().add(utf8::string(argv[i]));
}
//---------------------------------------------------------------------------
void initialize()
{
#if defined(__WIN32__) || defined(__WIN64__)
  mainThreadId = GetCurrentThreadId();
  threadFinishMessage = RegisterWindowMessageA("F07E92E6-81C9-441D-98D6-4D9802D81956");
  fiberFinishMessage = RegisterWindowMessageA("A08C6FDA-1417-43C5-A17D-D146500B5886");
  SetProcessShutdownParameters(0x400,SHUTDOWN_NORETRY);
  SetConsoleCtrlHandler(consoleCtrlHandler,TRUE);
#elif HAVE_SIGNAL_H
  sigset_t ss;
  if( sigemptyset(&ss) != 0 ||
      sigaddset(&ss,SIGSYS) != 0 ||
      sigprocmask(SIG_BLOCK,&ss,NULL) != 0 ){
    perror(NULL);
    abort();
  }
#endif
  LZO1X::initialize();
//---------------------------------------------------------------------------
  memset(base64DecodeTable,~0u,sizeof(base64DecodeTable));
  for( intptr_t i = sizeof(base64Table) / sizeof(base64Table[0]) - 1; i >= 0; i-- )
    base64DecodeTable[uintptr_t(base64Table[i])] = (uint8_t) i;
//---------------------------------------------------------------------------
  InterlockedMutex::initialize();
  Thread::initialize();
  MemoryStream::initialize();
  Mutant::initialize();
  utf8::String::initialize();
  new (argvPlaceHolder) Array<utf8::String>;
  strErrorInitialize();
  LogFile::initialize();
  Config::initialize();
#ifdef NETMAIL_ENABLE_PROFILER
  TProfiler::initialize();
#endif
  Fiber::initialize();
  AsyncDescriptorsCluster::initialize();
  ksock::api.initialize();
  try {
    utf8::String cwd(getCurrentDir());
    utf8::String::Iterator i(cwd);
    while( !i.eof() ){
      uintptr_t c = i.getChar();
      if( c == '\\' || c == '/' ){
        pathDelimiter = char(c);
        pathDelimiterStr[0] = char(c);
        pathDelimiterStr[1] = '\0';
        break;
      }
      i.next();
    }
  }
  catch( ... ){
  }
}
//---------------------------------------------------------------------------
void cleanup()
{
  ksock::api.cleanup();
  AsyncDescriptorsCluster::cleanup();
  Fiber::cleanup();
#ifdef NETMAIL_ENABLE_PROFILER
  TProfiler::cleanup();
#endif
  Config::cleanup();
  LogFile::cleanup();
  strErrorCleanup();
  argv().~Array<utf8::String>();
  utf8::String::cleanup();
  Mutant::cleanup();
  MemoryStream::cleanup();
  Thread::cleanup();
  InterlockedMutex::cleanup();
#if defined(__WIN32__) || defined(__WIN64__)
  SetConsoleCtrlHandler(consoleCtrlHandler,FALSE);
#endif
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------

