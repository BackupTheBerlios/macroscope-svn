/**
  Utils to make debugging running application under Windows
  Copyright Roger Rene Kommer (kommer@artefaktur.com) 2003

  dbgutils The sources either can be in a shared library (.dll) or
  has to be linked directly to the executable. 
  Compiling dbgutils as static library (.lib) won't work!

  Requirements dbghelp.h .dll .lib from microsoft
*/
#ifndef DbgString_h
#define DbgString_h

//#define DBGUTILS_USE_STD_STRING
//#define DBGUTILS_USE_MFC_CSTRING



#if defined(DBGUTILS_USE_STD_STRING)

#include <string>
typedef std::string DbgString;
#define DBGSTRING2CHARPTR(str) (str).c_str()
#define DBGSTRING_EQUALS(str, text) ((str) == text)
#define DBGSTRING_STARTSWITH(str, text) ((str).find(text) == 0)
#define DBGSTRING_LASTINDEX(str, text) ((str).rfind(text))
#define DBGSTRING_SIZE(str) ((str).size())
#define DBGSTRING_SUBSTR1(str, start) ((str).substr(start))
#define DBGSTRING_SUBSTR2(str, start, end) ((str).substr(start, end))
#define DBGSTRING_CONSTSTR(str) (str)

/*
inline
DbgStream&
operator<<(DbgStream& out, const DbgString& str)
{
  out << DBGSTRING2CHARPTR(str);
  return out;
}
*/
#elif defined(DBGUTILS_USE_MFC_CSTRING)

#include <afx.h>
typedef CString DbgString;
#define DBGSTRING2CHARPTR(str) ((const char*)(str))
#define DBGSTRING_EQUALS(str, text) ((str) == text)
#define DBGSTRING_STARTSWITH(str, text) ((str).Find(text) == 0)
int DBGUTILS_EXPORT CString_ReverseFind(const CString& str, const char* text);
#define DBGSTRING_LASTINDEX(str, text) (CString_ReverseFind(str, text))
#define DBGSTRING_SIZE(str) ((str).GetLength())
#define DBGSTRING_SUBSTR1(str, start) ((str).Mid(start))
#define DBGSTRING_SUBSTR2(str, start, end) ((str).Mid(start, end))
#define DBGSTRING_CONSTSTR(str) (str)
#else // defined(DBGUTILS_USE_STD_STRING)

// use internal DbgString implementation
#include "DbgVector.h"
#include <stdio.h>
#define DBGSTRING2CHARPTR(str) (str).c_str()
#define DBGSTRING_EQUALS(str, text) ((str) == text)
#define DBGSTRING_STARTSWITH(str, text) ((str).startsWith(text))
#define DBGSTRING_LASTINDEX(str, text) ((str).lastIndexOf(text))
#define DBGSTRING_SIZE(str) ((str).size())
#define DBGSTRING_SUBSTR1(str, start) ((str).substr(start))
#define DBGSTRING_SUBSTR2(str, start, end) ((str).substr(start, end))
#define DBGSTRING_CONSTSTR(str) DbgString(false, str)

namespace pdbutils {

/**
 @defgroup helper_classes Helper Classes
           Helper classes string and vector.
*/
/**
  @addtogroup helper_classes
  @{
*/

/**
  a very thin C++ wrapper to char*
  If DBGUTILS_USE_STD_STRING is define instead of DbgString a std::string will be used
  If DBGUTILS_USE_MFC_CSTRING is defined instead of DbgString a CString from MFC will be used
  @author Roger Rene Kommer (kommer@artefaktur.com)
*/
class /*DBGUTILS_EXPORT*/ DbgString
{
  char* _ptr;
  /**
    if true the pointer is a const char
  */
  bool _owns;
public:
  /**
    Standard construktur. Empty unitialized String
  */
  DbgString() : _ptr(0), _owns(true) {}
  /**
    Initializes String with character pointer
    @param ptr null terminated string. Must not be 0
    @param length length of ptr string. If length == -1 (default) str will be 
            used until 0 termination
  */
  DbgString(const char* ptr, int length = -1)
  : _ptr(0)
  , _owns(true)
  {
    _assign(ptr, length);
  }
  /**
    Used to initialize string optional with a const char* pointer
    @param owns if owns == false string will not be copied.
           should only be used for literal string
    @param ptr 0 terminated string
  */
  DbgString(bool owns, const char* ptr)
  : _ptr(const_cast<char*>(ptr))
  , _owns(owns)
  {
    if (_owns == true)
      _assign(ptr);
  }
  DbgString(const DbgString& str)
  : _owns(str._owns)
  {
    if (_owns == true)
      _assign(str._ptr);
    else
      _ptr = str._ptr;
  }
  ~DbgString()
  {
    _release();
  }
  intptr_t size() const { return _ptr == 0 ? 0 : strlen(_ptr); }
  /**
    to enabme ostream<<() return type is not const char* (silly, isn't it)
  */
  const char* c_str() const { return _ptr == 0 ? (const char*)"" : _ptr; } 
  operator const char*() const { return _ptr == 0 ? (const char*)"" : _ptr; } 
  operator char*() { return _ptr == 0 ? (char*)"" : _ptr; }
  DbgString& operator=(const char* ptr)
  {
    if (ptr == _ptr)
      return *this;
    _release();
    _assign(ptr);
    return *this;
  }
  DbgString& operator=(const DbgString& str)
  {
    if (str._ptr == _ptr)
      return *this;
    _release();
    _owns = str._owns;
    if (str._owns == false)
    {
      _ptr = str._ptr;
      return *this;
    }
    _assign(str._ptr);
    return *this;
  }
  bool operator==(const char* text) const
  {
    if (_ptr == 0 && text == 0)
      return true;
    if (_ptr == 0 || text == 0)
      return false;
    return strcmp(_ptr, text) == 0;
  }
  bool operator==(const DbgString& other) const
  {
    return operator==(other._ptr);
  }
  intptr_t indexOf(const char* text) const
  {
    if (_ptr == 0)
      return -1;
    const char* ptr = strstr(_ptr, text);
    if (ptr == 0)
      return -1;
    return ptr - _ptr;
  }
  bool startsWith(const char* text) const
  {
    if (_ptr == 0)
      return false;
    const char* ptr = strstr(_ptr, text);
    return ptr == _ptr;
  }
  
  intptr_t lastIndexOf(const char* text) const;
  DbgString substr(intptr_t offset, intptr_t length = -1) const;
  DbgString operator+(const char* text) const;

  DbgString operator+(const DbgString& other) const
  {
    return operator+(other._ptr);
  }
protected:
  void _release();
  void _assign(const char* ptr, intptr_t length = -1);
};

/**
  A replacement for std::ostringstream used to render 
  DbgTypes into a string.
*/
class DbgStream
{
  DbgVector<char> _buffer;
  typedef DbgVector<char>::iterator iterator;
public:
  DbgStream(intptr_t buffSize = 256) 
    : _buffer(buffSize < 1 ? 2 : buffSize)
  {
    *_buffer.begin() = 0;
  }
  void append(const char* text, intptr_t textlen)
  {
    _buffer.ensureCapacity(_buffer.size() + textlen + 2);
    iterator it = _buffer.end();
    iterator newend = it + textlen;
    for (intptr_t i = 0; i < textlen; ++i, ++it)
      *it = text[i];
    *it = 0;
    _buffer.setEnd(newend);
  }
  void append(void* ptr)
  {
    char ibuff[64];
    sprintf(ibuff, "%08P", ptr);
    append(ibuff, strlen(ibuff));
  }
  void append(intptr_t i)
  {
    char ibuff[64];
    sprintf(ibuff, "%d", i);
    append(ibuff, strlen(ibuff));
  }
  inline
  DbgStream& 
  operator<<(const DbgString& text)
  {
    append(text.c_str(), strlen(text.c_str()));
    return *this;
  }
  inline
  DbgStream& 
  operator<<(const char* text)
  {
    append(text, strlen(text));
    return *this;
  }
  inline DbgStream& operator<<(void* ptr)
  {
    append(ptr);
    return *this;
  }
  inline DbgStream&  operator<<(int i)
  {
    append(i);
    return *this;
  }

  DbgString str() 
  { 
    return DbgString(_buffer.begin()); 
  }
};

/** @} */


} // 

#endif // defined(DBGUTILS_USE_STD_STRING)

#endif //DbgString_h