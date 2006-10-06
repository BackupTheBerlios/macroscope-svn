/*-
 * Copyright 2006 Guram Dukashvili
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
#if defined(__WIN32__) || defined(__WIN64__)
#include <Winternl.h>
#endif
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DirectoryChangeNotification::~DirectoryChangeNotification()
{
  stop();
}
//---------------------------------------------------------------------------
DirectoryChangeNotification::DirectoryChangeNotification() :
#if defined(__WIN32__) || defined(__WIN64__)
  hFFCNotification_(INVALID_HANDLE_VALUE),
  hDirectory_(INVALID_HANDLE_VALUE),
  bufferSize_(0)
#endif
{
}
//---------------------------------------------------------------------------
void DirectoryChangeNotification::monitor(const utf8::String & pathName,uint64_t timeout)
{
#if defined(__WIN32__) || defined(__WIN64__)
  assert( 
    (hFFCNotification_ == INVALID_HANDLE_VALUE && hDirectory_ == INVALID_HANDLE_VALUE) || 
    ((hFFCNotification_ != INVALID_HANDLE_VALUE) ^ (hDirectory_ != INVALID_HANDLE_VALUE)) 
  );
  if( isWin9x() ){
    if( hFFCNotification_ == INVALID_HANDLE_VALUE ){
      hFFCNotification_ = FindFirstChangeNotificationA(
        pathName.getOEMString(),
        FALSE,
        FILE_NOTIFY_CHANGE_FILE_NAME
      );
      if( hFFCNotification_ == INVALID_HANDLE_VALUE ){
        int32_t err = GetLastError() + errorOffset;
        Exception::throwSP(err,__PRETTY_FUNCTION__);
      }
    }
    else if( FindNextChangeNotification(hFFCNotification_) == 0 ){
      int32_t err = GetLastError() + errorOffset;
      stop();
      Exception::throwSP(err,__PRETTY_FUNCTION__);
    }
  }
  else {
    if( buffer_ == NULL ){
      bufferSize_ = getpagesize() / sizeof(FILE_NOTIFY_INFORMATION);
      bufferSize_ *= sizeof(FILE_NOTIFY_INFORMATION);
      buffer_.alloc(bufferSize_);
    }
    if( hDirectory_ == INVALID_HANDLE_VALUE ){
      hDirectory_ = CreateFileW(
          pathName.getUNICODEString(),
          GENERIC_READ,
          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
          NULL,
          OPEN_EXISTING,
          FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
          NULL
      );
      if( hDirectory_ == INVALID_HANDLE_VALUE ){
        int32_t err = GetLastError() + errorOffset;
        Exception::throwSP(err,__PRETTY_FUNCTION__);
      }
    }
  }
  assert( currentFiber() != NULL );
  currentFiber()->event_.abort_ = false;
  currentFiber()->event_.position_ = 0;
  currentFiber()->event_.directoryChangeNotification_ = this;
  currentFiber()->event_.timeout_ = timeout;
  currentFiber()->event_.type_ = etDirectoryChangeNotification;
  currentFiber()->thread()->postRequest();
  currentFiber()->switchFiber(currentFiber()->mainFiber());
  assert( currentFiber()->event_.type_ == etDirectoryChangeNotification );
  if( currentFiber()->event_.errno_ == ERROR_REQUEST_ABORTED ) stop();
  if( currentFiber()->event_.errno_ != 0 && currentFiber()->event_.errno_ != ERROR_NOTIFY_ENUM_DIR ){
    throw ExceptionSP(
      newObject<EFileError>(currentFiber()->event_.errno_ + errorOffset,__PRETTY_FUNCTION__)
    );
  }
#endif
}
//---------------------------------------------------------------------------
void DirectoryChangeNotification::stop()
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( hFFCNotification_ != INVALID_HANDLE_VALUE ){
    FindCloseChangeNotification(hFFCNotification_);
    hFFCNotification_ = INVALID_HANDLE_VALUE;
  }
  if( hDirectory_ != INVALID_HANDLE_VALUE ){
    CloseHandle(hDirectory_);
    hDirectory_ = INVALID_HANDLE_VALUE;
  }
  buffer_.free();
  bufferSize_ = 0;
#endif
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
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
uintmax_t fibonacci(uintmax_t n)
{
  if( n == 0 ) return 0;
  uintmax_t prev = 0, curr = 1, next;
  while( --n > 0 ){
    next = curr + prev;
    prev = curr;
    curr = next;
  }
  return curr;
}
//---------------------------------------------------------------------------
void createUUID(UUID & uuid)
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( FAILED(CoCreateGuid(&uuid)) ){
    int32_t err = GetLastError() + errorOffset;
#elif HAVE_UUIDGEN
  if( uuidgen(&uuid,1) != 0 ){
    int32_t err = errno;
#endif
    Exception::throwSP(err,__PRETTY_FUNCTION__);
  }
}
//---------------------------------------------------------------------------
utf8::String screenChar(const utf8::String::Iterator & ii)
{
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
        return "\\0x" +  utf8::int2HexStr(ii.getChar(),SIZEOF_WCHAR_T * 2);
      memset(b,0,sizeof(b));
      memcpy(b,ii.c_str(),ii.seqLen());
      p = b;
  }
  return utf8::plane(p);
}
//---------------------------------------------------------------------------
/*utf8::String screenString(const utf8::String & s)
{
  utf8::String a;
  utf8::String::Iterator i(s);
  while( !i.eof() ){
    a += screenChar(i);
    i.next();
  }
  return a;
}*/
//---------------------------------------------------------------------------
static inline void screenChar(const char * seq,uintptr_t & seql,char * b,uintptr_t & l)
{
  uintptr_t c;
  switch( c = utf8::utf82ucs(seq,seql) ){
    case '\a' :
      if( b != NULL ) memcpy(b,"\\a",2);
      l = 2;
      break;
    case '\b' :
      if( b != NULL ) memcpy(b,"\\b",2);
      l = 2;
      break;
    case '\t' :
      if( b != NULL ) memcpy(b,"\\t",2);
      l = 2;
      break;
    case '\v' :
      if( b != NULL ) memcpy(b,"\\v",2);
      l = 2;
      break;
    case '\f' :
      if( b != NULL ) memcpy(b,"\\f",2);
      l = 2;
      break;
    case '\r' :
      if( b != NULL ) memcpy(b,"\\r",2);
      l = 2;
      break;
    case '\n' :
      if( b != NULL ) memcpy(b,"\\n",2);
      l = 2;
      break;
    case '\\' :
      if( b != NULL ) memcpy(b,"\\\\",2);
      l = 2;
      break;
    case ',' :
      if( b != NULL ) memcpy(b,"\\,",2);
      l = 2;
      break;
    case ';' :
      if( b != NULL ) memcpy(b,"\\;",2);
      l = 2;
      break;
    case '\"' :
      if( b != NULL ) memcpy(b,"\\\"",2);
      l = 2;
      break;
    case '=' :
      if( b != NULL ) memcpy(b,"\\=",2);
      l = 2;
      break;
    default  :
      if( (utf8::getC1Type(c) & (C1_CNTRL | C1_SPACE)) != 0 ){
        if( b != NULL ) memcpy(b,"\\0x",3);
        utf8::String s(utf8::int2HexStr(c,SIZEOF_WCHAR_T * 2));
        l = s.size();
        if( b != NULL ) memcpy(b + 3,s.c_str(),l);
        l += 3;
      }
      else {
        l = seql;
        if( b != NULL ) memcpy(b,seq,l);
      }
  }
}
//---------------------------------------------------------------------------
utf8::String screenString(const utf8::String & s)
{
  const char * p;
  uintptr_t seql, l, len = 0;
  AutoPtr<char> a;
  for( p = s.c_str(); *p != '\0'; p += seql, len += l ) screenChar(p,seql,NULL,l);
  a.alloc(len + 1);
  char * q = a;
  for( p = s.c_str(); *p != '\0'; p += seql, q += l ) screenChar(p,seql,q,l);
  a[len] = '\0';
  return newObject<utf8::String::Container>(1,a.ptr(NULL));
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
          if( utf8::tryStr2Int(utf8::String(ii + 2,ii + SIZEOF_WCHAR_T * 2 + 2),a,16) ){
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
          if( utf8::tryStr2Int(utf8::String(ii + 1,ii + SIZEOF_WCHAR_T * 2 + 3),a,10) ){
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
l1:   s = utf8::String(ii,ii + 1);
  }
  return s;
}
//---------------------------------------------------------------------------
/*utf8::String unScreenString(const utf8::String & s)
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
      a += utf8::String(i,i + 1);
      i.next();
    }
  }
  return a;
}*/
//---------------------------------------------------------------------------
static inline void unScreenChar(uintptr_t c,const char * seq,uintptr_t & seql,char * b,uintptr_t & l)
{
  uintptr_t i;
  switch( c ){
    case '0' :
      if( utf8::utf8c2UpperUCS(seq + seql) == 'X' ){
        for( i = 0; i < SIZEOF_WCHAR_T * 2; i++ ){
          if( (utf8::getC1Type(utf8::utf82ucs(seq + seql + i + 1)) & (C1_XDIGIT | C1_DIGIT)) == 0 ) break;
        }
        if( i == SIZEOF_WCHAR_T * 2 ){
          intmax_t a;
          if( utf8::tryStr2Int(utf8::String(seq + seql + 1,SIZEOF_WCHAR_T * 2),a,16) ){
            char buf[6];
            buf[l = utf8::ucs2utf8seq(buf,(uintptr_t) a)] = '\0';
            if( b != NULL ) memcpy(b,buf,l);
            seql = SIZEOF_WCHAR_T * 2 + 2;
            break;
          }
        }
      }
      goto l1;
    case 'a' :
      if( b != NULL ) *b = '\a';
      l = 1;
      break;
    case 'b' :
      if( b != NULL ) *b = '\b';
      l = 1;
      break;
    case 't' :
      if( b != NULL ) *b = '\t';
      l = 1;
      break;
    case 'v' :
      if( b != NULL ) *b = '\v';
      l = 1;
      break;
    case 'f' :
      if( b != NULL ) *b = '\f';
      l = 1;
      break;
    case 'r' :
      if( b != NULL ) *b = '\r';
      l = 1;
      break;
    case 'n' :
      if( b != NULL ) *b = '\n';
      l = 1;
      break;
    case '\\' :
      if( b != NULL ) *b = '\\';
      l = 1;
      break;
    default :
l1:   if( b != NULL ) memcpy(b,seq,seql);
      l = seql;
  }
}
//---------------------------------------------------------------------------
utf8::String unScreenString(const utf8::String & s)
{
  const char * p;
  uintptr_t seql, l, len = 0, c;
  AutoPtr<char> a;
  for( p = s.c_str(); *p != '\0'; p += seql, len += l ){
    c = utf8::utf82ucs(p,seql);
    l = seql;
    if( c == '\\' && p[1] != '\0' ){
      p += seql;
      c = utf8::utf82ucs(p,seql);
      unScreenChar(c,p,seql,NULL,l);
    }
  }
  a.alloc(len + 1);
  char * q = a;
  for( p = s.c_str(); *p != '\0'; p += seql, q += l ){
    c = utf8::utf82ucs(p,seql);
    l = seql;
    if( c == '\\' && p[1] != '\0' ){
      p += seql;
      c = utf8::utf82ucs(p,seql);
      unScreenChar(c,p,seql,q,l);
    }
    else {
      memcpy(q,p,seql);
    }
  }
  a[len] = '\0';
  return newObject<utf8::String::Container>(1,a.ptr(NULL));
}
//---------------------------------------------------------------------------
uintptr_t enumStringParts(const utf8::String & s,const char * delim)
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
            if( k == n ) return utf8::String(q,i);
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
  return i.bof() || n > 0 ? utf8::String() : s;
}
//---------------------------------------------------------------------------
utf8::String splitString(const utf8::String & s,utf8::String & s0,utf8::String & s1,const utf8::String & separator)
{
  utf8::String::Iterator i(s.strcasestr(separator));
  if( i.eof() ) s0 = s; else s0 = utf8::String(s,i);
  s1 = i + separator.strlen();
  return s;
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
      if( (uint8_t *) me.modBaseAddr                  <= (uint8_t *) addr &&
          (uint8_t *) me.modBaseAddr + me.modBaseSize >  (uint8_t *) addr ){
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
    return moduleFileName.ptr();
  }
  AutoPtr<wchar_t> moduleFileName;
  for(;;){
    moduleFileName.realloc(a * sizeof(wchar_t));
    if( (b = GetModuleFileNameW(h,moduleFileName,a)) < a ) break;
    a <<= 1;
  }
  moduleFileName.realloc((b + 1) * sizeof(wchar_t));
  return moduleFileName.ptr();
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
    return dirName.ptr();
  }
  AutoPtr<wchar_t> dirName;
  dirName.realloc((a = GetCurrentDirectoryW(0,NULL)) * sizeof(wchar_t));
  GetCurrentDirectoryW(a,dirName);
  return dirName.ptr();
#else
  AutoPtr<char> dirName((char *) kmalloc(MAXPATHLEN + 1 + 1));
  if( getcwd(dirName,MAXPATHLEN + 1) == NULL ) strcpy(dirName,".");
  strcat(dirName,pathDelimiterStr);
  return dirName.ptr();
#endif
}
//---------------------------------------------------------------------------
void changeCurrentDir(const utf8::String & name)
{
  int32_t err;
#if defined(__WIN32__) || defined(__WIN64__)
  SetLastError(0);
  if( isWin9x() ){
    SetCurrentDirectoryA((const char *) name.getANSIString());
  }
  else {
    SetCurrentDirectoryW((const wchar_t *) name.getUNICODEString());
  }
  err = GetLastError();
#else
  errno = 0;
  chdir((const char *) dirName.getANSIString());
  err = errno;
#endif
  if( err != 0 )
    Exception::throwSP(err + errorOffset,__PRETTY_FUNCTION__);
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
    return dirName.ptr();
  }
  AutoPtr<wchar_t> dirName;
  dirName.realloc((a = GetTempPathW(0,NULL) + 2) * sizeof(wchar_t));
  GetTempPathW(a,dirName);
  if( dirName[lstrlenW(dirName) - 1] != '\\' ) lstrcatW(dirName,L"\\");
  return dirName.ptr();
#elif HAVE_PATHS_H
  return _PATH_TMP;
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
        return utf8::String(utf8::String::Iterator(fileName),i) + extension;
      default   : i.prev();
    }
  }
  return fileName + extension;
}
//---------------------------------------------------------------------------
utf8::String getFileExt(const utf8::String & fileName)
{
  utf8::String::Iterator i(fileName);
  i.last();
  while( !i.bof() ){
    switch( i.getChar() ){
      case '\\' : case ':' : break;
      case '.'  : return utf8::String(i);
      default   : i.prev();
    }
  }
  return utf8::String();
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
  return utf8::String(utf8::String::Iterator(name),i + 1);
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
  if( currentFiber() != NULL ){
    currentFiber()->event_.string0_ = name;
    currentFiber()->event_.type_ = etCreateDir;
    currentFiber()->thread()->postRequest();
    currentFiber()->switchFiber(currentFiber()->mainFiber());
    assert( currentFiber()->event_.type_ == etCreateDir );
#if defined(__WIN32__) || defined(__WIN64__)
    if( currentFiber()->event_.errno_ - errorOffset == ERROR_ALREADY_EXISTS ) return false;
#else
    if( currentFiber()->event_.errno_ == EEXIST ) return false;
#endif
    if( currentFiber()->event_.errno_ != 0 )
      throw ExceptionSP(
        newObject<EFileError>(currentFiber()->event_.errno_,__PRETTY_FUNCTION__)
      );
    return currentFiber()->event_.rval_;
  }
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
    if( createDirectory(getPathFromPathName(name)) ) return createDirectory(name);
    err = oserror();
  }
#if defined(__WIN32__) || defined(__WIN64__)
  if( err != 0 && err != ERROR_ALREADY_EXISTS )
#else
  if( err != 0 && err != EEXIST )
#endif
    Exception::throwSP(err + errorOffset,__PRETTY_FUNCTION__);
  oserror(err);
  return err == 0;
}
//---------------------------------------------------------------------------
static int32_t removeDirectoryHelper(const utf8::String & name)
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
#else
  if( rmdir(anyPathName2HostPathName(name).getANSIString()) != 0 ) err = errno;
#endif
  oserror(err);
  return err;
}
//---------------------------------------------------------------------------
bool removeDirectory(const utf8::String & name,bool recursive)
{
  if( currentFiber() != NULL ){
    currentFiber()->event_.string0_ = name;
    currentFiber()->event_.recursive_ = recursive;
    currentFiber()->event_.type_ = etRemoveDir;
    currentFiber()->thread()->postRequest();
    currentFiber()->switchFiber(currentFiber()->mainFiber());
    assert( currentFiber()->event_.type_ == etRemoveDir );
#if defined(__WIN32__) || defined(__WIN64__)
    if( currentFiber()->event_.errno_ - errorOffset == ERROR_PATH_NOT_FOUND ||
        currentFiber()->event_.errno_ - errorOffset == ERROR_FILE_NOT_FOUND ) return false;
#else
    if( currentFiber()->event_.errno_ == ENOENT ) return false;
#endif
    if( currentFiber()->event_.errno_ != 0 )
      throw ExceptionSP(
        newObject<EFileError>(currentFiber()->event_.errno_,__PRETTY_FUNCTION__)
      );
    return currentFiber()->event_.rval_;
  }
  int32_t err = removeDirectoryHelper(name);
#if defined(__WIN32__) || defined(__WIN64__)
  if( err == ERROR_DIR_NOT_EMPTY ){
#else
  if( err == ENOTEMPTY ){
#endif
    Vector<utf8::String> list;
    getDirList(list,includeTrailingPathDelimiter(name) + "*",utf8::String(),recursive);
    err = 0;
    bool rval = true;
    for( intptr_t i = list.count() - 1; i >= 0; i-- ){
      struct Stat st;
      stat(list[i],st);
      if( (st.st_mode & S_IFDIR) != 0 && recursive ){
        if( !removeDirectory(list[i],true) ){
          if( rval ){
            err = oserror();
            rval = false;
          }
        }
      }
      else {
        if( !remove(list[i]) ){
          if( rval ){
            err = oserror();
            rval = false;
          }
        }
      }
    }
    if( rval ){
      err = removeDirectoryHelper(name);
    }
    else {
      oserror(err);
      return rval;
    }
  }
#if defined(__WIN32__) || defined(__WIN64__)
  if( err == ERROR_PATH_NOT_FOUND || err == ERROR_FILE_NOT_FOUND ) return false;
#else
  if( err == ENOENT ) return false;
#endif
  oserror(err);
  if( err != 0 )
    Exception::throwSP(err + errorOffset,__PRETTY_FUNCTION__);
  return true;
}
//---------------------------------------------------------------------------
bool remove(const utf8::String & name)
{
  if( currentFiber() != NULL ){
    currentFiber()->event_.string0_ = name;
    currentFiber()->event_.type_ = etRemoveFile;
    currentFiber()->thread()->postRequest();
    currentFiber()->switchFiber(currentFiber()->mainFiber());
    assert( currentFiber()->event_.type_ == etRemoveFile );
#if defined(__WIN32__) || defined(__WIN64__)
    if( currentFiber()->event_.errno_ - errorOffset == ERROR_PATH_NOT_FOUND ||
        currentFiber()->event_.errno_ - errorOffset == ERROR_FILE_NOT_FOUND ) return false;
#else
    if( currentFiber()->event_.errno_ == ENOENT ) return false;
#endif
    if( currentFiber()->event_.errno_ != 0 )
      throw ExceptionSP(
        newObject<EFileError>(currentFiber()->event_.errno_,__PRETTY_FUNCTION__)
      );
    return currentFiber()->event_.rval_;
  }
  int32_t err = 0;
  oserror(0);
#if defined(__WIN32__) || defined(__WIN64__)
  if( isWin9x() ){
    if( DeleteFileA(anyPathName2HostPathName(name).getANSIString()) == 0 )
      err = GetLastError();
  }
  else {
    if( DeleteFileW(anyPathName2HostPathName(name).getUNICODEString()) == 0 )
      err = GetLastError();
  }
  if( err == ERROR_PATH_NOT_FOUND || err == ERROR_FILE_NOT_FOUND ) return false;
#else
  if( unlink(anyPathName2HostPathName(name).getANSIString()) != 0 ) err = errno;
  if( err == ENOENT ) return false;
#endif
  if( err != 0 ){
    struct Stat st;
    stat(name,st);
    if( (st.st_mode & S_IFDIR) != 0 ){
      if( removeDirectory(name,true) ) return remove(name);
      err = oserror();
    }
    Exception::throwSP(err + errorOffset,__PRETTY_FUNCTION__);
  }
  return true;
}
//---------------------------------------------------------------------------
#if !defined(__WIN32__) && !defined(__WIN64__)
void chModOwn(
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
    Exception::throwSP(err,__PRETTY_FUNCTION__);
  }
  const struct passwd * u = getpwnam(utf8::String(user).getANSIString());
  uid_t userID(u != NULL ? u->pw_uid : (uid_t) user);
  const struct group * g = getgrnam(utf8::String(group).getANSIString());
  gid_t groupID(g != NULL ? g->gr_gid : (gid_t) group);
  if( chown(anyPathName2HostPathName(pathName).getANSIString(),userID,groupID) != 0 ){
    err = errno;
    Exception::throwSP(err,__PRETTY_FUNCTION__);
  }
}
#else
void chModOwn(const utf8::String &,const Mutant &,const Mutant &,const Mutant &)
{
}
#endif
//---------------------------------------------------------------------------
utf8::String getPathFromPathName(const utf8::String & pathName)
{
  utf8::String::Iterator i(pathName);
  i.last();
  while( !i.bof() ){
    if( i.getChar() == (uintptr_t) pathDelimiter || i.getChar() == ':' )
      return utf8::String(utf8::String::Iterator(pathName),i);
    i.prev();
  }
  return ".";
}
//---------------------------------------------------------------------------
utf8::String getNameFromPathName(const utf8::String & pathName)
{
  utf8::String::Iterator i(pathName);
  i.last();
  while( !i.bof() ){
    if( i.getChar() == (uintptr_t) pathDelimiter )
      return utf8::String(i + 1,utf8::String::Iterator(pathName).last());
    i.prev();
  }
  return i;
}
//---------------------------------------------------------------------------
utf8::String anyPathName2HostPathName(const utf8::String & pathName)
{
  if( pathDelimiter == '\\' ) return pathName.replaceAll("/",pathDelimiterStr);
  return pathName.replaceAll("\\",pathDelimiterStr);
}
//---------------------------------------------------------------------------
bool nameFitMask(const utf8::String & name,const utf8::String & mask)
{
  utf8::String::Iterator ni(name), mi(mask);
  while( !ni.eof() && !mi.eof() ){
    if( mi.getChar() == '?' ){
      Exception::throwSP(ENOSYS,utf8::String(__PRETTY_FUNCTION__) + " " + utf8::int2Str(__LINE__) + " FIXME");
    }
    else if( mi.getChar() == '*' ){
      mi.next();
#if defined(__WIN32__) || defined(__WIN64__)
      while( ni.getUpperChar() != mi.getUpperChar() ){
        ni.next();
        if( ni.eof() ) break;
      }
      if( utf8::String(ni).strcasecmp(mi) != 0 ) mi.prev();
#else
      while( !mi.eof() && !ni.eof() && ni.getChar() != mi.getChar() ){
        ni.next();
        if( ni.eof() ) break;
      }
      if( utf8::String(ni).strcmp(mi) != 0 ) mi.prev();
#endif
    }
    else if( mi.getChar() == '[' ){
//      mi.next();
      Exception::throwSP(ENOSYS,utf8::String(__PRETTY_FUNCTION__) + " " + utf8::int2Str(__LINE__) + " FIXME");
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
void rename(const utf8::String & oldPathName,const utf8::String & newPathName)
{
  if( currentFiber() != NULL ){
    currentFiber()->event_.string0_ = oldPathName;
    currentFiber()->event_.string1_ = newPathName;
    currentFiber()->event_.type_ = etRename;
    currentFiber()->thread()->postRequest();
    currentFiber()->switchFiber(currentFiber()->mainFiber());
    assert( currentFiber()->event_.type_ == etRename );
    if( currentFiber()->event_.errno_ != 0 )
      throw ExceptionSP(
        newObject<EFileError>(currentFiber()->event_.errno_,__PRETTY_FUNCTION__)
      );
  }
  else {
#if defined(__WIN32__) || defined(__WIN64__)
    BOOL r;
    if( isWin9x() ){
      r = MoveFileA(oldPathName.getANSIString(),newPathName.getANSIString());
    }
    else {
      r = MoveFileExW(oldPathName.getUNICODEString(),newPathName.getUNICODEString(),MOVEFILE_COPY_ALLOWED);
    }
    if( r == 0 )
#else
    if( rename(oldPathName.getANSIString(),newPathName.getANSIString()) != 0 )
#endif
      Exception::throwSP(oserror() + errorOffset,utf8::String(__PRETTY_FUNCTION__));
  }
}
//---------------------------------------------------------------------------
void sleep(uint64_t timeout)
{
  if( currentFiber() != NULL ){
    currentFiber()->event_.abort_ = false;
    currentFiber()->event_.timerStartTime_ = gettimeofday();
    currentFiber()->event_.timeout_ = timeout;
    currentFiber()->event_.type_ = etTimer;
    currentFiber()->thread()->postRequest();
    currentFiber()->switchFiber(currentFiber()->mainFiber());
    assert( currentFiber()->event_.type_ == etTimer );
    if( currentFiber()->event_.errno_ != 0 )
      throw ExceptionSP(
        newObject<EFileError>(currentFiber()->event_.errno_ + errorOffset,__PRETTY_FUNCTION__)
      );
  }
  else {
#if HAVE_NANOSLEEP
    struct timespec rqtp;
    rqtp.tv_sec = timeout / 1000000u;
    rqtp.tv_nsec = timeout % 1000000u;
    nanosleep(&rqtp, NULL);
#elif defined(__WIN32__) || defined(__WIN64__)
    Sleep((DWORD) (timeout / 1000u + (timeout < 1000000u)));
#elif HAVE_USLEEP
    usleep(timeout / 1000u);
#elif HAVE_SLEEP
    sleep(timeout / 1000000u);
#endif
  }
}
//---------------------------------------------------------------------------
static bool getDirListHelper(
  const utf8::String & name,
  const utf8::String & mask,
  const utf8::String & exMask,
  bool exMaskAsList)
{
  bool add = false;
  if( nameFitMask(name,mask) ){
    if( exMaskAsList ){
      add = true;
      for( intptr_t i = enumStringParts(exMask) - 1; i >= 0; i-- )
        if( nameFitMask(name,stringPartByNo(exMask,i)) ){
          add = false;
          break;
        }
    }
    else {
      add = !nameFitMask(name,exMask);
    }
  }
  return add;
}
//---------------------------------------------------------------------------
void getDirList(
  Vector<utf8::String> & list,
  const utf8::String & dirAndMask,
  const utf8::String & exMask,
  bool recursive,
  bool includeDirs,
  bool exMaskAsList)
{
  if( currentFiber() != NULL ){
    currentFiber()->event_.dirList_ = &list;
    currentFiber()->event_.string0_ = dirAndMask;
    currentFiber()->event_.string1_ = exMask;
    currentFiber()->event_.recursive_ = recursive;
    currentFiber()->event_.includeDirs_ = includeDirs;
    currentFiber()->event_.exMaskAsList_ = exMaskAsList;
    currentFiber()->event_.type_ = etDirList;
    currentFiber()->thread()->postRequest();
    currentFiber()->switchFiber(currentFiber()->mainFiber());
    assert( currentFiber()->event_.type_ == etDirList );
    if( currentFiber()->event_.errno_ != 0 )
      throw ExceptionSP(
        newObject<EFileError>(currentFiber()->event_.errno_,__PRETTY_FUNCTION__)
      );
    return;
  }
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
      Exception::throwSP(err,__PRETTY_FUNCTION__);
    }
    try {
      do {
        if( strcmp(fda.cFileName,".") == 0 || strcmp(fda.cFileName,"..") == 0 ) continue;
        const char * d_name = fda.cFileName;
        if( (fda.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ){
          bool fit = getDirListHelper(d_name,mask,exMask,exMaskAsList);
          if( includeDirs && fit )
            list.add(path + pathDelimiterStr + d_name);
          if( recursive && fit )
#else
    DIR * dir = opendir(anyPathName2HostPathName(path).getANSIString());
    if( dir == NULL ){
      err = errno;
      Exception::throwSP(err,__PRETTY_FUNCTION__);
    }
    try {
      struct dirent * ent;
#if HAVE_READDIR_R
      struct dirent * result, res;
      ent = &res;
      for(;;){
        if( readdir_r(dir,ent,&result) != 0 ){
          err = errno;
          Exception::throwSP(err,__PRETTY_FUNCTION__);
        }
        if( result == NULL ) break;
#else
        while( (ent = readdir(dir)) != NULL ){
#endif
        if( strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0 ) continue;
        const char * d_name = ent->d_name;
#ifdef DT_DIR
        if( ent->d_type == DT_DIR ){
          bool fit = getDirListHelper(d_name,maskmexMask,exMaskAsList);
          if( includeDirs && fit )
            list.add(path + pathDelimiterStr + d_name);
          if( recursive && fit )
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
              recursive,
              includeDirs,
              exMaskAsList
            );
        }
        else if( getDirListHelper(d_name,mask,exMask,exMaskAsList) ){
          list.add(path + pathDelimiterStr + d_name);
        }
#if defined(__WIN32__) || defined(__WIN64__)
      } while( FindNextFileA(handle,&fda) != 0 );
      if( GetLastError() != ERROR_NO_MORE_FILES ){
        err = GetLastError() + errorOffset;
        Exception::throwSP(err,__PRETTY_FUNCTION__);
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
      Exception::throwSP(err,__PRETTY_FUNCTION__);
    }
    try {
      do {
        if( lstrcmpW(fdw.cFileName,L".") == 0 || lstrcmpW(fdw.cFileName,L"..") == 0 ) continue;
        if( (fdw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ){
          bool fit = getDirListHelper(fdw.cFileName,mask,exMask,exMaskAsList);
          if( includeDirs && fit )
            list.add(path + pathDelimiterStr + fdw.cFileName);
          if( recursive && fit )
            getDirList(
              list,
              path + pathDelimiterStr + fdw.cFileName + pathDelimiterStr + mask,
              exMask,
              recursive,
              includeDirs,
              exMaskAsList
            );
        }
        else if( getDirListHelper(fdw.cFileName,mask,exMask,exMaskAsList) ){
          list.add(path + pathDelimiterStr + fdw.cFileName);
        }
      } while( FindNextFileW(handle,&fdw) != 0 );
      if( GetLastError() != ERROR_NO_MORE_FILES ){
        err = GetLastError() + errorOffset;
        Exception::throwSP(err,__PRETTY_FUNCTION__);
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
    Exception::throwSP(err,__PRETTY_FUNCTION__);
  }
  if( EmptyClipboard() == 0 ){
    err = GetLastError() + errorOffset;
    Exception::throwSP(err,__PRETTY_FUNCTION__);
  }
  if( isWin9x() ){
    utf8::AnsiString pass(s.getANSIString());
    HGLOBAL w = GlobalAlloc(
      GMEM_MOVEABLE | GMEM_DDESHARE,
      (strlen(pass) + 1) * sizeof(char)
    );
    if( w == NULL ){
      err = GetLastError() + errorOffset;
      CloseClipboard();
      Exception::throwSP(err,__PRETTY_FUNCTION__);
    }
    LPVOID ww = GlobalLock(w);
    if( ww == NULL ){
      err = GetLastError() + errorOffset;
      CloseClipboard();
      Exception::throwSP(err,__PRETTY_FUNCTION__);
    }
    memcpy(ww,(const char *) pass,(strlen(pass) + 1) * sizeof(char));
    GlobalUnlock(ww);
    if( SetClipboardData(CF_TEXT,w) == NULL ){
      err = GetLastError() + errorOffset;
      GlobalFree(w);
      CloseClipboard();
      Exception::throwSP(err,__PRETTY_FUNCTION__);
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
      Exception::throwSP(err,__PRETTY_FUNCTION__);
    }
    LPVOID ww = GlobalLock(w);
    if( ww == NULL ){
      err = GetLastError() + errorOffset;
      CloseClipboard();
      Exception::throwSP(err,__PRETTY_FUNCTION__);
    }
    memcpy(ww,(const wchar_t *) pass,(lstrlenW(pass) + 1) * sizeof(wchar_t));
    GlobalUnlock(ww);
    if( SetClipboardData(CF_UNICODETEXT,w) == NULL ){
      err = GetLastError() + errorOffset;
      GlobalFree(w);
      CloseClipboard();
      Exception::throwSP(err,__PRETTY_FUNCTION__);
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
int64_t getProcessStartTime(bool toLocalTime)
{
#if defined(__WIN32__) || defined(__WIN64__)
  int32_t err = 0;
  SetLastError(ERROR_SUCCESS);
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,GetCurrentProcessId());
  union {
    FILETIME ft;
    ULARGE_INTEGER sti;
  } creationTime, exitTime, kernelTime, userTime;
  creationTime.sti.QuadPart = 0;
  if( hProcess != NULL )
    if( GetProcessTimes(hProcess,&creationTime.ft,&exitTime.ft,&kernelTime.ft,&userTime.ft) != 0 )
      if( toLocalTime ) FileTimeToLocalFileTime(&creationTime.ft,&creationTime.ft);
  err = GetLastError();
  if( hProcess != NULL ) CloseHandle(hProcess);
  if( err != ERROR_SUCCESS ) Exception::throwSP(err + errorOffset,__PRETTY_FUNCTION__);
  return (creationTime.sti.QuadPart - UINT64_C(11644473600) * 10000000u) / 10u;
#else
#error Not implemented
  Exception::throwSP(ENOSYS,__PRETTY_FUNCTION__);
#endif
}
//---------------------------------------------------------------------------
intptr_t strToMonth(const utf8::String & month)
{
  if( month.strcasecmp("Jan") == 0 ) return 0;
  if( month.strcasecmp("Feb") == 0 ) return 1;
  if( month.strcasecmp("Mar") == 0 ) return 2;
  if( month.strcasecmp("Apr") == 0 ) return 3;
  if( month.strcasecmp("Mai") == 0 ) return 4;
  if( month.strcasecmp("May") == 0 ) return 4;
  if( month.strcasecmp("Jun") == 0 ) return 5;
  if( month.strcasecmp("Jul") == 0 ) return 6;
  if( month.strcasecmp("Aug") == 0 ) return 7;
  if( month.strcasecmp("Sep") == 0 ) return 8;
  if( month.strcasecmp("Okt") == 0 ) return 9;
  if( month.strcasecmp("Oct") == 0 ) return 9;
  if( month.strcasecmp("Nov") == 0 ) return 10;
  if( month.strcasecmp("Dez") == 0 ) return 11;
  if( month.strcasecmp("Dec") == 0 ) return 11;
  return -1;
}
//------------------------------------------------------------------------------
utf8::String getTimestamp(const utf8::String & date,const utf8::String & time)
{
  utf8::String::Iterator i(date);
  return 
    utf8::String(i + 4,i + 4 + 2).replaceAll(" ","0") + "." +
    utf8::int2Str0(strToMonth(utf8::String(i,i + 3)),2) + "." +
    utf8::String(i + 7,i + 7 + 4) + " " +
    time/*.replaceAll(":","")*/
  ;
}
//---------------------------------------------------------------------------
void reverseByteArray(void * array,uintptr_t size)
{
  for( uintptr_t i = size; i > size / 2; i-- )
    xchg(((uint8_t *) array) [i - 1],((uint8_t *) array) [size - i]);
}
//---------------------------------------------------------------------------
void reverseByteArray(void * dst,const void * src,uintptr_t size)
{
  if( dst == src ){
    for( uintptr_t i = size; i > size / 2; i-- )
  	  xchg(((uint8_t *) dst) [i - 1],((uint8_t *) dst) [size - i]);
  }
  else {
    for( intptr_t i = size - 1; i >= 0; i-- )
      ((uint8_t *) dst) [size - i - 1] = ((const uint8_t *) src) [i];
  }
}
//---------------------------------------------------------------------------
// base64 routines
//---------------------------------------------------------------------------
static const char base64Table[64] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '+', '/'
};
//---------------------------------------------------------------------------
uintptr_t rfcBase64Encode(
  const uint8_t * inStr,
  size_t inLen, 
  uint8_t * outStr,
  size_t outLen)
{
  size_t currOutLen = 0, i = 0;
  while( i < inLen ){
    uint8_t a = inStr[i];
    uint8_t b = (i + 1 >= inLen) ? 0 : inStr[i + 1];
    uint8_t c = (i + 2 >= inLen) ? 0 : inStr[i + 2];
    if( i + 2 < inLen ){
      if( currOutLen + 4 <= outLen ){
        outStr[currOutLen++] = base64Table[(a >> 2) & 0x3F];
        outStr[currOutLen++] = base64Table[((a << 4) & 0x30) + ((b >> 4) & 0xf)];
        outStr[currOutLen++] = base64Table[((b << 2) & 0x3c) + ((c >> 6) & 0x3)];
        outStr[currOutLen++] = base64Table[c & 0x3F];
      }
      else {
        currOutLen += 4;
      }
    }
    else if( i + 1 < inLen ){
      if( currOutLen + 4 <= outLen ){
        outStr[currOutLen++] = base64Table[(a >> 2) & 0x3F];
        outStr[currOutLen++] = base64Table[((a << 4) & 0x30) + ((b >> 4) & 0xf)];
        outStr[currOutLen++] = base64Table[((b << 2) & 0x3c) + ((c >> 6) & 0x3)];
        outStr[currOutLen++] = '=';
      }
      else {
        currOutLen += 4;
      }
    }
    else {
      if( currOutLen + 4 <= outLen ){
        outStr[currOutLen++] = base64Table[(a >> 2) & 0x3F];
        outStr[currOutLen++] = base64Table[((a << 4) & 0x30) + ((b >> 4) & 0xf)];
        outStr[currOutLen++] = '=';
        outStr[currOutLen++] = '=';
      }
      else {
        currOutLen += 4;
      }
    }
    i += 3;
  }
  if( currOutLen < outLen ) outStr[currOutLen] = '\0';
  return currOutLen;
}
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
    if( c < 1 || c >= 256 ) Exception::throwSP(EINVAL,__PRETTY_FUNCTION__);
    c = base64DecodeTable[c];
    if( c >= 64 ) Exception::throwSP(EINVAL,__PRETTY_FUNCTION__);
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
  return i >> 3;
}
//---------------------------------------------------------------------------
// base32 routines
//---------------------------------------------------------------------------
static const char base32Table[32] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
  'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z',
  '0', '1', '2', '3', '4', '5'
};
//---------------------------------------------------------------------------
static uint8_t base32DecodeTable[256];
//---------------------------------------------------------------------------
static inline uint8_t base32Encode5(const uint8_t * p,uintptr_t i)
{
  return (uint8_t) (((*(uint16_t *) (p + (i >> 3))) >> (i & 7)) & 31u);
}
//---------------------------------------------------------------------------
utf8::String base32Encode(const void * p,uintptr_t l)
{
  utf8::String s;
  if( l > 0 ){
    uintptr_t i, k;
    s.resize(((l << 3) / 5) + ((l << 3) % 5 != 0));
    char * sp = s.c_str();
    k = (l << 3) - 8;
    for( i = 0; i < k; i += 5 ){
      *sp++ = base32Table[base32Encode5((uint8_t *) p,i)];
    }
    union {
      uint8_t c[2];
      uint32_t c32;
    };
    c32 = *((uint8_t *) p + (i >> 3));
    while( i < k + 8 ){
      *sp++ = base32Table[base32Encode5(c,i & 7)];
      i += 5;
    }
  }
  return s;
}
//---------------------------------------------------------------------------
uintptr_t base32Decode(const utf8::String & s,void * p,uintptr_t size)
{
  uintptr_t i = 0;
  utf8::String::Iterator sp(s);
  size <<= 3;
  while( !sp.eof() ){
    uintptr_t c = sp.getChar();
    if( c < 1 || c >= 256 ) Exception::throwSP(EINVAL,__PRETTY_FUNCTION__);
    c = base32DecodeTable[c];
    if( c >= 32 ) Exception::throwSP(EINVAL,__PRETTY_FUNCTION__);
    if( i < size ){
      if( size - i <= 8 ){
        *(uint8_t *) ((uint8_t *) p + (i >> 3)) &= (uint8_t) (~(31u << (i & 7)));
        *(uint8_t *) ((uint8_t *) p + (i >> 3)) |= (uint8_t) (c << (i & 7));
      }
      else {
        *(uint16_t *) ((uint8_t *) p + (i >> 3)) &= (uint16_t) (~(31u << (i & 7)));
        *(uint16_t *) ((uint8_t *) p + (i >> 3)) |= (uint16_t) (c << (i & 7));
      }
    }
    i += 5;
    sp++;
  }
  return i >> 3;
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
pid_t getpid()
{
  return GetCurrentProcessId();
}
//---------------------------------------------------------------------------
uid_t getuid()
{
  // TODO: rewrite for windows
  return 0;
}
//---------------------------------------------------------------------------
gid_t getgid()
{
// TODO: rewrite for windows
  return 0;
}
//---------------------------------------------------------------------------
#else
//---------------------------------------------------------------------------
pid_t getpid()
{
  return ::getpid();
}
//---------------------------------------------------------------------------
uid_t getuid()
{
  return ::getuid();
}
//---------------------------------------------------------------------------
gid_t getgid()
{
  return ::getgid();
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
bool isWow64()
{
  typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
  LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
    GetModuleHandle("kernel32"),
    "IsWow64Process"
  );
  BOOL bIsWow64 = FALSE;
  if( fnIsWow64Process != NULL && fnIsWow64Process(GetCurrentProcess(),&bIsWow64) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    Exception::throwSP(err,__PRETTY_FUNCTION__);
  }
  return bIsWow64 == FALSE ? false : true;
}
#endif
//---------------------------------------------------------------------------
utf8::String getMachineUniqueKey()
{
#if PRIVATE_RELEASE
#pragma comment(lib,"wbemuuid.lib")
  utf8::String s;

  VARIANT vtName;
  VariantInit(&vtName);
  VARIANT vtDeviceId;
  VariantInit(&vtDeviceId);
  VARIANT vtDescription;
  VariantInit(&vtDescription);
  VARIANT vtProcessorId;
  VariantInit(&vtProcessorId);
  VARIANT vtMACAddress;
  VariantInit(&vtMACAddress);
  VARIANT vtAdapterTypeId;
  VariantInit(&vtAdapterTypeId);
  HRESULT CoInit_hres;
  try {
    HRESULT hres;
    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------
    CoInit_hres = CoInitializeEx(0,COINIT_MULTITHREADED);
    if( FAILED(CoInit_hres) )
      Exception::throwSP(HRESULT_CODE(CoInit_hres) + errorOffset,__PRETTY_FUNCTION__);
    try {
      // Step 2: --------------------------------------------------
      // Set general COM security levels --------------------------
      // Note: If you are using Windows 2000, you need to specify -
      // the default authentication credentials for a user by using
      // a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
      // parameter of CoInitializeSecurity ------------------------
      if( CoInit_hres == S_OK ){
        hres = CoInitializeSecurity(
          NULL, 
          -1,                          // COM authentication
          NULL,                        // Authentication services
          NULL,                        // Reserved
          RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
          RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
          NULL,                        // Authentication info
          EOAC_NONE,                   // Additional capabilities 
          NULL                         // Reserved
        );
        if( FAILED(hres) )
          Exception::throwSP(HRESULT_CODE(hres) + errorOffset,__PRETTY_FUNCTION__);
      }
      // Step 3: ---------------------------------------------------
      // Obtain the initial locator to WMI -------------------------
      IWbemLocator *pLoc = NULL;
      hres = CoCreateInstance(
        CLSID_WbemLocator,             
        0, 
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator, (LPVOID *) &pLoc
      );
      if( FAILED(hres) )
        Exception::throwSP(HRESULT_CODE(hres) + errorOffset,__PRETTY_FUNCTION__);
      try {
        // Step 4: -----------------------------------------------------
        // Connect to WMI through the IWbemLocator::ConnectServer method
        IWbemServices *pSvc = NULL;
        // Connect to the root\cimv2 namespace with
        // the current user and obtain pointer pSvc
        // to make IWbemServices calls.
        hres = pLoc->ConnectServer(
          L"ROOT\\CIMV2", // Object path of WMI namespace
          NULL,                    // User name. NULL = current user
          NULL,                    // User password. NULL = current
          0,                       // Locale. NULL indicates current
          NULL,                    // Security flags.
          0,                       // Authority (e.g. Kerberos)
          0,                       // Context object 
          &pSvc                    // pointer to IWbemServices proxy
        );
        if( FAILED(hres) )
          Exception::throwSP(HRESULT_CODE(hres) + errorOffset,__PRETTY_FUNCTION__);
        try {
          // Step 5: --------------------------------------------------
          // Set security levels on the proxy -------------------------
          hres = CoSetProxyBlanket(
             pSvc,                        // Indicates the proxy to set
             RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
             RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
             NULL,                        // Server principal name 
             RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
             RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
             NULL,                        // client identity
             EOAC_NONE                    // proxy capabilities 
          );
          if( FAILED(hres) )
            Exception::throwSP(HRESULT_CODE(hres) + errorOffset,__PRETTY_FUNCTION__);
          // Step 6: --------------------------------------------------
          // Use the IWbemServices pointer to make requests of WMI ----
          IEnumWbemClassObject * pEnumerator = NULL;
          hres = pSvc->ExecQuery(
            SysAllocString(L"WQL"),
            SysAllocString(L"SELECT * FROM Win32_Processor"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
            NULL,
            &pEnumerator
          );
          if( FAILED(hres) )
            Exception::throwSP(HRESULT_CODE(hres) + errorOffset,__PRETTY_FUNCTION__);
          try {
            // Step 7: -------------------------------------------------
            // Get the data from the query in step 6 -------------------
            IWbemClassObject * pclsObj;
            ULONG uReturn = 0;
            while( pEnumerator != NULL ){
              HRESULT hr = pEnumerator->Next(WBEM_INFINITE,1,&pclsObj,&uReturn);
              if( uReturn == 0 ) break;
              try {
                hr = pclsObj->Get(L"Name", 0, &vtName, 0, 0);
                if( FAILED(hr) )
                  Exception::throwSP(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__);
                if( V_VT(&vtName) != VT_BSTR ){
                  hr = VariantChangeTypeEx(&vtName,&vtName,0,0,VT_BSTR);
                  Exception::throwSP(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__);
                }
                hr = pclsObj->Get(L"DeviceId", 0, &vtDeviceId, 0, 0);
                if( FAILED(hr) )
                  Exception::throwSP(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__);
                if( V_VT(&vtDeviceId) != VT_BSTR ){
                  hr = VariantChangeTypeEx(&vtDeviceId,&vtDeviceId,0,0,VT_BSTR);
                  Exception::throwSP(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__);
                }
                hr = pclsObj->Get(L"Description", 0, &vtDescription, 0, 0);
                if( FAILED(hr) )
                  Exception::throwSP(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__);
                if( V_VT(&vtDescription) != VT_BSTR ){
                  hr = VariantChangeTypeEx(&vtDescription,&vtDescription,0,0,VT_BSTR);
                  Exception::throwSP(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__);
                }
                hr = pclsObj->Get(L"ProcessorId", 0, &vtProcessorId, 0, 0);
                if( FAILED(hr) )
                  Exception::throwSP(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__);
                if( V_VT(&vtProcessorId) != VT_BSTR ){
                  hr = VariantChangeTypeEx(&vtProcessorId,&vtProcessorId,0,0,VT_BSTR);
                  Exception::throwSP(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__);
                }
                if( s.strlen() > 0 ) s += "\n";
                s += utf8::String(V_BSTR(&vtDeviceId)) + " " +
                  V_BSTR(&vtName) + " " +
                  V_BSTR(&vtDescription) + " " +
                  V_BSTR(&vtProcessorId)
                ;
              }
              catch( ... ){
                pclsObj->Release();
                throw;
              }
              pclsObj->Release();
            }
          }
          catch( ... ){
            pEnumerator->Release();
            throw;
          }
          pEnumerator->Release();

          hres = pSvc->ExecQuery(
            SysAllocString(L"WQL"),
            SysAllocString(L"SELECT * FROM Win32_NetworkAdapter"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
            NULL,
            &pEnumerator
          );
          if( FAILED(hres) )
            Exception::throwSP(HRESULT_CODE(hres) + errorOffset,__PRETTY_FUNCTION__);
          try {
            // Step 7: -------------------------------------------------
            // Get the data from the query in step 6 -------------------
            IWbemClassObject * pclsObj;
            ULONG uReturn = 0;
            while( pEnumerator != NULL ){
              HRESULT hr = pEnumerator->Next(WBEM_INFINITE,1,&pclsObj,&uReturn);
              if( uReturn == 0 ) break;
              try {
                hr = pclsObj->Get(L"Name", 0, &vtName, 0, 0);
                if( FAILED(hr) )
                  Exception::throwSP(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__);
                if( V_VT(&vtName) != VT_BSTR ){
                  hr = VariantChangeTypeEx(&vtName,&vtName,0,0,VT_BSTR);
                  Exception::throwSP(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__);
                }
                hr = pclsObj->Get(L"AdapterTypeId", 0, &vtAdapterTypeId, 0, 0);
                if( FAILED(hr) ){
                  HRESULT hr2 = pclsObj->Get(L"AdapterType", 0, &vtAdapterTypeId, 0, 0);
                  if( FAILED(hr2) ){
                    hr = hr2;
                    Exception::throwSP(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__);
                  }
                  if( V_VT(&vtAdapterTypeId) != VT_NULL ){
                    if( V_VT(&vtAdapterTypeId) != VT_BSTR ){
                      hr = VariantChangeTypeEx(&vtAdapterTypeId,&vtAdapterTypeId,0,0,VT_BSTR);
                      Exception::throwSP(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__);
                    }
                  }
                }
                if( V_VT(&vtAdapterTypeId) != VT_NULL ){
                  if( V_VT(&vtAdapterTypeId) == VT_BSTR ){
                    if( utf8::String("Ethernet").strncasecmp(V_BSTR(&vtAdapterTypeId),8) == 0 ){
                      VariantClear(&vtAdapterTypeId);
                      V_VT(&vtAdapterTypeId) = VT_I4;
                      V_I4(&vtAdapterTypeId) = 0;
                    }
                    else {
                      VariantClear(&vtAdapterTypeId);
                      V_VT(&vtAdapterTypeId) = VT_I4;
                      V_I4(&vtAdapterTypeId) = 3;
                    }
                  }
                  if( V_VT(&vtAdapterTypeId) != VT_I4 ){
                    hr = VariantChangeTypeEx(&vtAdapterTypeId,&vtAdapterTypeId,0,0,VT_I4);
                    Exception::throwSP(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__);
                  }
                  if( V_I4(&vtAdapterTypeId) >= 0 && V_I4(&vtAdapterTypeId) != 3 ){
                    hr = pclsObj->Get(L"MACAddress", 0, &vtMACAddress, 0, 0);
                    if( FAILED(hr) )
                      Exception::throwSP(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__);
                    if( V_VT(&vtMACAddress) != VT_NULL ){
                      if( V_VT(&vtMACAddress) != VT_BSTR ){
                        hr = VariantChangeTypeEx(&vtMACAddress,&vtMACAddress,0,0,VT_BSTR);
                        Exception::throwSP(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__);
                      }
                      if( s.strlen() > 0 ) s += "\n";
                      s += utf8::String(V_BSTR(&vtName)) + " " + V_BSTR(&vtMACAddress);
                    }
                  }
                }
              }
              catch( ... ){
                pclsObj->Release();
                throw;
              }
              pclsObj->Release();
            }
          }
          catch( ... ){
            pEnumerator->Release();
            throw;
          }
          pEnumerator->Release();
        }
        catch( ... ){
          pSvc->Release();
          throw;
        }
        pSvc->Release();
      }
      catch( ... ){
        pLoc->Release();
        throw;
      }
      pLoc->Release();
    }
    catch( ... ){
      if( CoInit_hres == S_OK ) CoUninitialize();
      throw;
    }
    if( CoInit_hres == S_OK ) CoUninitialize();
  }
  catch( ... ){
    VariantClear(&vtProcessorId);
    VariantClear(&vtDescription);
    VariantClear(&vtDeviceId);
    VariantClear(&vtName);
    VariantClear(&vtMACAddress);
    throw;
  }
  VariantClear(&vtProcessorId);
  VariantClear(&vtDescription);
  VariantClear(&vtDeviceId);
  VariantClear(&vtName);
  VariantClear(&vtMACAddress);
  return s;
#else
  return utf8::String();
#endif
}
//---------------------------------------------------------------------------
utf8::String getMachineCleanUniqueKey()
{
  utf8::String key(getMachineUniqueKey());
  return base64Encode(key.c_str(),key.size());
}
//---------------------------------------------------------------------------
utf8::String getMachineCryptedUniqueKey(const utf8::String & text)
{
  SHA256 sha;
  sha.make(text.c_str(),text.size());
  return base64Encode(sha.sha256(),sha.size());
}
//---------------------------------------------------------------------------
#if PRIVATE_RELEASE
void checkMachineBinding(const utf8::String & key)
{
  AutoLock<InterlockedMutex> lock(giant());
  if( machineUniqueCryptedKey().strlen() == 0 )
    machineUniqueCryptedKey() = getMachineCryptedUniqueKey(getMachineCleanUniqueKey());
  if( machineUniqueCryptedKey().strcmp(key) != 0 )
    Exception::throwSP(EINVAL,"Pirate copy detected");
}
#endif
//---------------------------------------------------------------------------
uint8_t machineUniqueCryptedKeyHolder[sizeof(utf8::String)];
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// System initialization and finalization ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
void initializeArguments(int argc,char ** argv)
{
  ksys::argv().clear();
  for( intptr_t i = 0; i < argc; i++ ) ksys::argv().add(argv[i]);
}
//---------------------------------------------------------------------------
void initialize()
{
// runtime checks for system or compiler incompatibilities
  assert( sizeof(char) == 1 );
  if( sizeof(char) != 1 ){
    fprintf(stderr,"sizeof(char) != 1\n");
    abort();
  }
  assert( (unsigned int) false == 0 );
  if( (unsigned int) false != 0 ){
    fprintf(stderr,"(unsigned int) false != 0\n");
    abort();
  }
  assert( (unsigned int) !1 == 0 );
  if( (unsigned int) !1 != 0 ){
    fprintf(stderr,"(unsigned int) !1 != 0\n");
    abort();
  }
  assert( (unsigned int) !0 == 1 );
  if( (unsigned int) !0 != 1 ){
    fprintf(stderr,"(unsigned int) !0 != 1\n");
    abort();
  }
#if HAVE__TZSET
  _tzset();
#elif HAVE_TZSET
  tzset();
#endif
#if defined(__WIN32__) || defined(__WIN64__)
// force the system to create the message queue for main thread (if not already created)
  MSG msg;
  SetLastError(0);
  PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
  if( GetLastError() != 0 ){
    perror(NULL);
    abort();
  }
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
  {
    intptr_t i;
    memset(base64DecodeTable,~0u,sizeof(base64DecodeTable));
    for( i = sizeof(base64Table) / sizeof(base64Table[0]) - 1; i >= 0; i-- )
      base64DecodeTable[uintptr_t(base64Table[i])] = (uint8_t) i;
    memset(base32DecodeTable,~0u,sizeof(base32DecodeTable));
    for( i = sizeof(base32Table) / sizeof(base32Table[0]) - 1; i >= 0; i-- )
      base32DecodeTable[uintptr_t(base32Table[i])] = (uint8_t) i;
  }
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
  BaseThread::initialize();
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
  new (machineUniqueCryptedKeyHolder) utf8::String();
}
//---------------------------------------------------------------------------
void cleanup()
{
  machineUniqueCryptedKey().~String();
  ksock::api.cleanup();
  BaseThread::cleanup();
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
