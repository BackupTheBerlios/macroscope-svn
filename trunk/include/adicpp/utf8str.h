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
#ifndef _utf8strH_
#define _utf8strH_
//---------------------------------------------------------------------------
#include "utf8embd.h"
//---------------------------------------------------------------------------
namespace ksys { class Exception; }
//---------------------------------------------------------------------------
namespace utf8 {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T> class StringT {
  friend class String;
  public:
    ~StringT();
    StringT(const StringT<T> & string);

    StringT<T> & operator = (const StringT<T> & string);

    operator T * ();
    operator T * const () const;
    T & operator [] (intptr_t i);
    const T & operator [] (intptr_t i) const;
    T & operator [] (uintptr_t i);
    const T & operator [] (uintptr_t i) const;
  protected:
  private:
    StringT(T * string);
    class Container {
      public:
        ksys::AutoPtr<T> string_;
        int32_t refCount_;

        ~Container(){}

        Container(T * string) : string_(string), refCount_(0){}

        void addRef(){ ksys::interlockedIncrement(refCount_, 1); }
        void remRef(){ if( ksys::interlockedIncrement(refCount_, -1) == 1 ) delete this; }
      private:
        Container(const Container &) {}
        void operator = (const Container &) {}
    };
    ksys::SPRC<Container> container_;
};
//---------------------------------------------------------------------------
template <typename T> inline StringT<T>::~StringT()
{
}
//---------------------------------------------------------------------------
template <typename T> inline
StringT<T>::StringT(T * string) : container_(newObject<Container>(string))
{
}
//---------------------------------------------------------------------------
template <typename T> inline
StringT<T>::StringT(const StringT<T> & string) : container_(string.container_)
{
}
//---------------------------------------------------------------------------
template <typename T> inline
StringT<T> & StringT<T>::operator = (const StringT<T> & string)
{
  container_ = string.container_;
  return *this;
}
//---------------------------------------------------------------------------
template <typename T> StringT<T>::operator T *()
{
  return container_->string_;
}
//---------------------------------------------------------------------------
template <typename T> StringT<T>::operator T * const () const
{
  return container_->string_;
}
//---------------------------------------------------------------------------
template <typename T> T & StringT<T>::operator [] (intptr_t i)
{
  return container_->string_[i];
}
//---------------------------------------------------------------------------
template <typename T> const T & StringT<T>::operator [] (intptr_t i) const
{
  return container_->string_[i];
}
//---------------------------------------------------------------------------
template <typename T> T & StringT<T>::operator [] (uintptr_t i)
{
  return container_->string_[i];
}
//---------------------------------------------------------------------------
template <typename T> const T & StringT<T>::operator [] (uintptr_t i) const
{
  return container_->string_[i];
}
//---------------------------------------------------------------------------
typedef StringT<char>    AnsiString;
typedef StringT<char>    OemString;
typedef StringT<wchar_t> WideString;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class String {
  friend void ksys::initialize(int,char **);
  friend void ksys::cleanup();
  public:
    class Container {
      public:
        union {
            char *          string_;
            unsigned char * ustring_;
        };

        ~Container();
        Container();
        Container(int32_t refCount, char * string);

        static Container * container(uintptr_t l);

        void addRef();
        void remRef();
        void acquire();
        void release();
      private:
        int32_t refCount_;
//        int32_t mutex_;
    };
    class Iterator {
      friend class BaseString;
      friend class String;
      // typed constructors
      friend String plane(const char * s, uintptr_t size = ~uintptr_t(0) >> 1);
      friend String plane(ksys::AutoPtr<char> & s, uintptr_t size = ~uintptr_t(0) >> 1);
      public:
        ~Iterator();
        Iterator(const Iterator & i);
        Iterator(const String & str);

        Iterator &        operator =(const Iterator & i);
        Iterator &        operator =(const String & str);

        Iterator &        operator +=(intptr_t relPos);
        Iterator &        operator -=(intptr_t relPos);

        Iterator          operator +(intptr_t relPos) const;
        Iterator          operator -(intptr_t relPos) const;

        intptr_t          operator -(const Iterator & i) const;

        Iterator &        operator ++();
        Iterator &        operator --();
        Iterator &        operator ++(int);
        Iterator &        operator --(int);

        bool              operator ==(const Iterator & i) const;
        bool              operator !=(const Iterator & i) const;
        bool              operator >(const Iterator & i) const;
        bool              operator >=(const Iterator & i) const;
        bool              operator <(const Iterator & i) const;
        bool              operator <=(const Iterator & i) const;

        Iterator          operator[](intptr_t relPos) const;

        Iterator &        reset();

        const intptr_t &  cursor() const;
        const intptr_t &  position() const;

        Iterator &        first();
        Iterator &        last();
        bool              isFirst() const;
        bool              isLast() const;
        bool              prev();
        bool              next();
        bool              bof() const;
        bool              eof() const;

        bool              isDigit() const;
        bool              isAlpha() const;
        bool              isAlnum() const;
        bool              isPrint() const;
        bool              isSpace() const;
        bool              isPunct() const;
        bool              isCntrl() const;
        bool              isBlank() const;
        bool              isXdigit() const;
        bool              isUpper() const;
        bool              isLower() const;

        uintptr_t         seqLen() const;
        uintptr_t         getChar() const;
        uintptr_t         getUpperChar() const;
        uintptr_t         getLowerChar() const;
        char *            c_str();
        const char *      c_str() const;
      private:
        mutable ksys::SPRC< Container>  container_;
        mutable intptr_t                cursor_;
        mutable intptr_t                position_;

        Iterator(const ksys::SPRC< Container> & container, uintptr_t cursor, uintptr_t position);
    };
    friend class Iterator;
    class Stream {
      public:
        class Format {
          friend class Stream;
          public:
            ~Format();

            Format(const Format & a){ operator = (a); }
            Format & operator = (const Format & a){ memcpy(fmt_,a.fmt_,sizeof(fmt_)); return *this; }

            Format(char n,const char * fmt = "%c");
            Format(unsigned char n,const char * fmt = "%c");
            Format(short n,const char * fmt = "%hd");
            Format(unsigned short n,const char * fmt = "%hd");
#if !HAVE_INTPTR_T_AS_LONG
            Format(long n,const char * fmt = "%ld");
            Format(unsigned long n,const char * fmt = "%ld");
#endif
#if !HAVE_INTPTR_T_AS_INT
            Format(int n,const char * fmt = "%d");
            Format(unsigned int n,const char * fmt = "%d");
#endif
#if !HAVE_INTPTR_T_AS_INTMAX_T
            Format(intmax_t n,const char * fmt = "%" PRIdMAX);
            Format(uintmax_t n,const char * fmt = "%" PRIuMAX);
#endif
            Format(intptr_t n,const char * fmt = "%" PRIdPTR);
            Format(uintptr_t n,const char * fmt = "%" PRIuPTR);
            Format(const char * p,const char * fmt = "%s");
            Format(const void * const p,const char * fmt = "%" PRIxPTR);
            Format(float a,const char * fmt = "%f");
            Format(double a,const char * fmt = "%f");
#if HAVE_LONG_DOUBLE
            Format(long double a,const char * fmt = "%lf");
#endif
          protected:
            union {
              char ci_;
              unsigned char cu_;
              short si_;
              unsigned short su_;
              long li_;
              unsigned long lu_;
              int ii_;
              unsigned int iu_;
              intmax_t mi_;
              uintmax_t mu_;
              intptr_t pi_;
              uintptr_t pu_;
              float f_;
              double d_;
#if HAVE_LONG_DOUBLE
              long double ld_;
#endif
              const char * s_;
            };
            enum {
              CI, CU, SI, SU, LI, LU, II, IU, MI, MU, PI, PU, F, D, LD, S
            } type_;
            mutable char fmt_[16];

            intptr_t format(char * buffer) const;
          private:
            Format(){}
            void checkFormat2(const char * v0,const char * v1) const;
            void checkFormat3(const char * v0,const char * v1,const char * v2) const;
            int print(char * buffer,size_t count) const;
        };

        ~Stream();
        Stream();

        Stream & operator << (char n);
        Stream & operator << (unsigned char n);
        Stream & operator << (short n);
        Stream & operator << (unsigned short n);
#if !HAVE_INTPTR_T_AS_LONG
        Stream & operator << (long n);
        Stream & operator << (unsigned long n);
#endif
#if !HAVE_INTPTR_T_AS_INT    
        Stream & operator << (int n);
        Stream & operator << (unsigned int n);
#endif
#if !HAVE_INTPTR_T_AS_INTMAX_T
        Stream & operator << (intmax_t n);
        Stream & operator << (uintmax_t n);
#endif
        Stream & operator << (intptr_t n);
        Stream & operator << (uintptr_t n);
        Stream & operator << (const char * p);
        Stream & operator << (const void * const p);
        Stream & operator << (float n);
        Stream & operator << (double n);
#if HAVE_LONG_DOUBLE
        Stream & operator << (long double n);
#endif
        Stream & operator << (const utf8::String & s);
        Stream & operator << (const Format & a);

        const char * plane() const;
        const uintptr_t & count() const;
        utf8::String string();
        Stream & clear();
      protected:
      private:
        ksys::AutoPtr<char> stream_;
        uintptr_t count_;

        Stream(const Stream &){}
        void operator = (const Stream &){}
    };
    friend class Stream;
    // typed constructors
    friend String       plane(const char * s, uintptr_t size);
    friend String       plane(ksys::AutoPtr<char> & s, uintptr_t size);
    friend String       operator +(const char * s1, const String & s2);
    friend String       operator +(const wchar_t * s1, const String & s2);
  public:
    ~String();
    String();
    String(Container * container);
    String(const char * s, uintptr_t l = ~uintptr_t(0) >> 1);
    String(const wchar_t * s, uintptr_t l = ~uintptr_t(0) >> 1);
    String(const Iterator & i);
    String(const Iterator & i1, const Iterator & i2);
    // copy constructor
    String(const String & str);

    // copy operator
    String &            operator =(const char * string);
    String &            operator =(const wchar_t * string);
    String &            operator =(const String & str);
    // concatenation operator
    String              operator +(const char * str) const;
    String              operator +(const wchar_t * str) const;
    String              operator +(const String & str) const;
    String &            operator +=(const char * str);
    String &            operator +=(const wchar_t * str);
    String &            operator +=(const String & str);
    // manipulators
    String              unique() const;
    char *              c_str();
    const char *        c_str() const;
    String &            resize(uintptr_t bSize);
    String              lower() const;
    String              upper() const;
    String &            lowerInPlace();
    String &            upperInPlace();
    uintptr_t           strlen() const;
    uintptr_t           size() const;
#ifndef __BCPLUSPLUS__
#undef strcmp
#endif
    bool operator == (const utf8::String & s) const { return strcmp(s) == 0; }
    bool operator != (const utf8::String & s) const { return strcmp(s) != 0; }
    bool operator >= (const utf8::String & s) const { return strcmp(s) >= 0; }
    bool operator >  (const utf8::String & s) const { return strcmp(s) >  0; }
    bool operator <= (const utf8::String & s) const { return strcmp(s) <= 0; }
    bool operator <  (const utf8::String & s) const { return strcmp(s) <  0; }
    intptr_t            strcmp(const String & s) const;
    intptr_t            strncmp(const String & s, uintptr_t n) const;
    intptr_t            strcasecmp(const String & s) const;
    intptr_t            strncasecmp(const String & s, uintptr_t n) const;
    Iterator            strstr(const String & str) const;
    Iterator            strcasestr(const String & str) const;
    Iterator            strrstr(const String & str) const;
    Iterator            strrcasestr(const String & str) const;
    AnsiString          getANSIString() const;
    OemString           getOEMString() const;
    WideString          getUNICODEString() const;
    static uintptr_t getMBCSString(const char * string,uintptr_t codePage,ksys::AutoPtr<uint8_t> & s,bool eos = true);
    uintptr_t getMBCSString(uintptr_t codePage,ksys::AutoPtr<uint8_t> & s,bool eos = true) const;
#if defined(__WIN32__) || defined(__WIN64__)
    BSTR getOLEString() const;
#endif
    String              trimLeft() const;
    String              trimRight() const;
    String              trim() const;

    String & insert(const Iterator & i, const String & str);
    String & insert(const Iterator & i, const Iterator & i1, const Iterator & i2);
    String cut(const Iterator & i) const;
    String cut(const Iterator & i1, const Iterator & i2) const;
    String & remove(const Iterator & i);
    String & remove(const Iterator & i1, const Iterator & i2);
    String & replace(const Iterator & i, const String & str);
    String & replace(const Iterator & i1, const Iterator & i2, const String & str);
    String & replace(const Iterator & d1, const Iterator & d2, const Iterator & s1, const Iterator & s2);
    String replaceAll(const String & what,const String & onWhat) const;
    String left(uintptr_t symbols) const;
    String right(uintptr_t symbols) const;
    String middle(uintptr_t pos, uintptr_t symbols) const;

    static String       print(const char * fmt, ...);
    String              catPrint(const char * fmt, ...);

    bool                isEqu(const String & s) const;

    bool                hashKeyEqu(const utf8::String & key, bool caseSensitive) const;
    uintptr_t           hash(bool caseSensitive) const;
    uint64_t            hash_ll(bool caseSensitive) const;

    bool isDigit() const;
    bool isAlpha() const;
    bool isAlnum() const;
    bool isPrint() const;
    bool isSpace() const;
    bool isPunct() const;
    bool isCntrl() const;
    bool isBlank() const;
    bool isXdigit() const;
    bool isUpper() const;
    bool isLower() const;
  protected:
    mutable ksys::SPRC<Container> container_;
    static char nullString_[];
    static uint8_t nullContainer_[];

    static Container & nullContainer();
  private:
    static void         initialize();
    static void         cleanup();
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline String::Container & String::nullContainer()
{
  return *reinterpret_cast< Container *>(nullContainer_);
}
//---------------------------------------------------------------------------
inline String::Container::~Container()
{
  ksys::xfree(string_);
}
//---------------------------------------------------------------------------
inline String::Container::Container(int32_t refCount,char * str) :
  string_(str), refCount_(refCount)/*, mutex_(0)*/
{
}
//---------------------------------------------------------------------------
inline void String::Container::addRef()
{
  ksys::interlockedIncrement(refCount_,1);
}
//---------------------------------------------------------------------------
inline void String::Container::remRef()
{
  if( ksys::interlockedIncrement(refCount_,-1) == 1 ) delete this;
}
//---------------------------------------------------------------------------
inline void String::Container::release()
{
//  ksys::interlockedIncrement(mutex_,1);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline String::~String()
{
}
//---------------------------------------------------------------------------
inline String::String(Container * container) : container_(container)
{
}
//---------------------------------------------------------------------------
inline String::String() : container_(&nullContainer())
{
}
//---------------------------------------------------------------------------
inline String::String(const String & str) : container_(str.container_)
{
}
//---------------------------------------------------------------------------
inline String & String::operator =(const String & str)
{
  container_ = str.container_;
  return *this;
}
//---------------------------------------------------------------------------
inline char * String::c_str()
{
  return (char *) container_->string_;
}
//---------------------------------------------------------------------------
inline const char * String::c_str() const
{
  return (const char *) container_->string_;
}
//---------------------------------------------------------------------------
inline String & String::lowerInPlace()
{
  utf8s2Lower(container_->string_, ~uintptr_t(0) >> 1, container_->string_, ~uintptr_t(0) >> 1);
  return *this;
}
//---------------------------------------------------------------------------
inline String & String::upperInPlace()
{
  utf8s2Upper(container_->string_, ~uintptr_t(0) >> 1, container_->string_, ~uintptr_t(0) >> 1);
  return *this;
}
//---------------------------------------------------------------------------
inline uintptr_t String::strlen() const
{
  return utf8strlen(container_->string_);
}
//---------------------------------------------------------------------------
inline uintptr_t String::size() const
{
  return (uintptr_t) ::strlen((const char *) container_->string_);
}
//---------------------------------------------------------------------------
inline String & String::insert(const Iterator & i, const String & str)
{
  return insert(i, Iterator(str), Iterator(str).last());
}
//---------------------------------------------------------------------------
inline String & String::insert(const Iterator & i, const Iterator & i1, const Iterator & i2)
{
  return replace(i, i, i1, i2);
}
//---------------------------------------------------------------------------
#ifndef __BCPLUSPLUS__
inline String String::cut(const Iterator & i) const
{
  return cut(i, i + 1);
}
#endif
//---------------------------------------------------------------------------
inline String & String::remove(const Iterator & i)
{
  return remove(i, i + 1);
}
//---------------------------------------------------------------------------
inline String & String::remove(const Iterator & i1, const Iterator & i2)
{
  return replace(i1, i2, i1, i1);
}
//---------------------------------------------------------------------------
inline String & String::replace(const Iterator & i, const String & str)
{
  return replace(i, i + 1, Iterator(str), Iterator(str).last());
}
//---------------------------------------------------------------------------
inline String & String::replace(const Iterator & i1, const Iterator & i2, const String & str)
{
  return replace(i1, i2, Iterator(str), Iterator(str).last());
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline String::Iterator::~Iterator()
{
}
//---------------------------------------------------------------------------
inline String::Iterator::Iterator(const ksys::SPRC< Container> & container, uintptr_t cursor, uintptr_t position)
  : container_(container), cursor_(cursor), position_(position)
{
}
//---------------------------------------------------------------------------
inline String::Iterator::Iterator(const Iterator & i)
  : container_(i.container_), cursor_(i.cursor_), position_(i.position_)
{
}
//---------------------------------------------------------------------------
inline String::Iterator::Iterator(const String & str)
  : container_(str.container_), cursor_(0), position_(0)
{
}
//---------------------------------------------------------------------------
inline String::Iterator & String::Iterator::operator =(const Iterator & i)
{
  container_ = i.container_;
  cursor_ = i.cursor_;
  position_ = i.position_;
  return *this;
}
//---------------------------------------------------------------------------
inline String::Iterator & String::Iterator::operator =(const String & str)
{
  container_ = str.container_;
  cursor_ = 0;
  position_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
#ifndef __BCPLUSPLUS__
inline String::Iterator String::Iterator::operator +(intptr_t relPos) const
{
  return Iterator(*this) += relPos;
}
//---------------------------------------------------------------------------
inline String::Iterator String::Iterator::operator -(intptr_t relPos) const
{
  return Iterator(*this) -= relPos;
}
#endif
//---------------------------------------------------------------------------
inline intptr_t String::Iterator::operator -(const Iterator & i) const
{
  return position_ - i.position_;
}
//---------------------------------------------------------------------------
inline String::Iterator & String::Iterator::operator ++()
{
  return *this += 1;
}
//---------------------------------------------------------------------------
inline String::Iterator & String::Iterator::operator --()
{
  return *this -= 1;
}
//---------------------------------------------------------------------------
inline String::Iterator & String::Iterator::operator ++(int)
{
  return *this += 1;
}
//---------------------------------------------------------------------------
inline String::Iterator & String::Iterator::operator --(int)
{
  return *this -= 1;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::operator ==(const Iterator & i) const
{
  return position_ == i.position_;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::operator !=(const Iterator & i) const
{
  return position_ != i.position_;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::operator >(const Iterator & i) const
{
  return position_ > i.position_;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::operator >=(const Iterator & i) const
{
  return position_ >= i.position_;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::operator <(const Iterator & i) const
{
  return position_ < i.position_;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::operator <=(const Iterator & i) const
{
  return position_ <= i.position_;
}
//---------------------------------------------------------------------------
#ifndef __BCPLUSPLUS__
inline String::Iterator String::Iterator::operator[](intptr_t relPos) const
{
  return Iterator(*this) += relPos;
}
#endif
//---------------------------------------------------------------------------
inline String::Iterator & String::Iterator::first()
{
  cursor_ = position_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::isFirst() const
{
  return position_ == 0 && cursor_ == 0 && *container_->string_ != '\0';
}
//---------------------------------------------------------------------------
inline bool String::Iterator::isLast() const
{
  return position_ < 0 || container_->string_[cursor_] != '\0' && container_->string_[cursor_ + utf8seqlen(container_->string_ + cursor_)] == '\0';
}
//---------------------------------------------------------------------------
inline bool String::Iterator::bof() const
{
  return position_ <= 0;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::eof() const
{
  return position_ < 0 || container_->string_[cursor_] == 0;
}
//---------------------------------------------------------------------------
inline const intptr_t & String::Iterator::cursor() const
{
  return cursor_;
}
//---------------------------------------------------------------------------
inline const intptr_t & String::Iterator::position() const
{
  return position_;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::isDigit() const
{
  return (getC1Type(utf82ucs(container_->ustring_ + cursor_)) & C1_DIGIT) != 0;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::isAlpha() const
{
  return (getC1Type(utf82ucs(container_->ustring_ + cursor_)) & C1_ALPHA) != 0;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::isAlnum() const
{
  return (getC1Type(utf82ucs(container_->ustring_ + cursor_)) & (C1_ALPHA | C1_DIGIT)) != 0;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::isPrint() const
{
  return (getC1Type(utf82ucs(container_->ustring_ + cursor_)) & C1_CNTRL) == 0;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::isSpace() const
{
  return (getC1Type(utf82ucs(container_->ustring_ + cursor_)) & C1_SPACE) != 0;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::isPunct() const
{
  return (getC1Type(utf82ucs(container_->ustring_ + cursor_)) & C1_PUNCT) != 0;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::isCntrl() const
{
  return (getC1Type(utf82ucs(container_->ustring_ + cursor_)) & C1_CNTRL) != 0;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::isBlank() const
{
  return (getC1Type(utf82ucs(container_->ustring_ + cursor_)) & C1_BLANK) != 0;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::isXdigit() const
{
  return (getC1Type(utf82ucs(container_->ustring_ + cursor_)) & C1_XDIGIT) != 0;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::isUpper() const
{
  return (getC1Type(utf82ucs(container_->ustring_ + cursor_)) & C1_UPPER) != 0;
}
//---------------------------------------------------------------------------
inline bool String::Iterator::isLower() const
{
  return (getC1Type(utf82ucs(container_->ustring_ + cursor_)) & C1_LOWER) != 0;
}
//---------------------------------------------------------------------------
inline uintptr_t String::Iterator::seqLen() const
{
  return utf8seqlen(container_->string_ + cursor_);
}
//---------------------------------------------------------------------------
inline uintptr_t String::Iterator::getChar() const
{
  return utf82ucs(container_->ustring_ + cursor_);
}
//---------------------------------------------------------------------------
inline uintptr_t String::Iterator::getUpperChar() const
{
  return utf8c2UpperUCS(container_->string_ + cursor_);
}
//---------------------------------------------------------------------------
inline uintptr_t String::Iterator::getLowerChar() const
{
  return utf8c2LowerUCS(container_->string_ + cursor_);
}
//---------------------------------------------------------------------------
inline char * String::Iterator::c_str()
{
  return (char *) container_->string_ + cursor_;
}
//---------------------------------------------------------------------------
inline const char * String::Iterator::c_str() const
{
  return (const char *) container_->string_ + cursor_;
}
//---------------------------------------------------------------------------
inline bool String::isEqu(const String & s) const
{
  return container_.ptr() == s.container_.ptr();
}
//---------------------------------------------------------------------------
inline bool String::hashKeyEqu(const String & key, bool caseSensitive) const
{
  return (caseSensitive ? strcmp(key) : strcasecmp(key)) == 0;
}
//---------------------------------------------------------------------------
String          operator +(const char * s1, const String & s2);
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// misc string manipulators /////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INTMAX_T
uintptr_t       int2StrLen(intmax_t a);
uintptr_t       int2StrLen(uintmax_t a);
String          int2Str(intmax_t a);
String          int2Str(uintmax_t a);
String int2Str0(intmax_t a,uintptr_t padding = 0);
String int2Str0(uintmax_t a,uintptr_t padding = 0);
#endif
#if !HAVE_INTPTR_T_AS_INT
String          int2Str(int a);
String          int2Str(unsigned int a);
String int2Str0(int a,uintptr_t padding = 0);
String int2Str0(unsigned int a,uintptr_t padding = 0);
#endif
String          int2Str(intptr_t a);
String          int2Str(uintptr_t a);
String int2Str0(intptr_t a,uintptr_t padding = 0);
String int2Str0(uintptr_t a,uintptr_t padding = 0);
uintptr_t       int2StrLen(intptr_t a);
uintptr_t       int2StrLen(uintptr_t a);
String          int2HexStr(uintmax_t a, uintptr_t padding);
String          ptr2Str(const void * p);
//---------------------------------------------------------------------------
bool            tryStr2Int(const String & str, intmax_t & a, uintptr_t pow = 10);
//---------------------------------------------------------------------------
intmax_t        str2Int(const String & str);
//---------------------------------------------------------------------------
bool            tryStr2tm(const String & str, struct tm & tv);
bool            tryStr2Time(const String & str, int64_t & tv);
bool            tryStr2Timeval(const String & str, struct timeval & tv);
//---------------------------------------------------------------------------
struct tm       str2tm(const String & str);
int64_t         str2Time(const String & str);
struct timeval  str2Timeval(const String & str);
//---------------------------------------------------------------------------
String          tm2Str(struct tm tv);
String          time2Str(int64_t tv);
String          timeval2Str(const struct timeval & tv);
String          elapsedTime2Str(int64_t t);
//---------------------------------------------------------------------------
#if HAVE_LONG_DOUBLE
String          float2Str(long double a);
bool            tryStr2Float(const String & str, long double & a);
long double     str2Float(const String & str);
#else
String          float2Str(double a);
bool            tryStr2Float(const String & str, double & a);
double          str2Float(const String & str);
#endif
//---------------------------------------------------------------------------
char * strstr(const char * string,const char * str);
char * strrstr(const char * string,const char * str);
char * strcasestr(const char * string,const char * str);
char * strrcasestr(const char * string,const char * str);
void strncpy0(char * dst,size_t size,const char * src,size_t count = ~size_t(0));
void strncat0(char * dst,size_t size,const char * src,size_t count = ~size_t(0));
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// String Stream implementation /////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline String::Stream::~Stream()
{
}
//---------------------------------------------------------------------------
inline String::Stream::Stream() : count_(0)
{
}
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::operator << (char n)
{
  return *this << Format(n);
}
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::operator << (unsigned char n)
{
  return *this << Format(n);
}
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::operator << (short n)
{
  return *this << Format(n);
}
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::operator << (unsigned short n)
{
  return *this << Format(n);
}
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_LONG
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::operator << (long n)
{
  return *this << Format(n);
}
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::operator << (unsigned long n)
{
  return *this << Format(n);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT    
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::operator << (int n)
{
  return *this << Format(n);
}
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::operator << (unsigned int n)
{
  return *this << Format(n);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INTMAX_T
inline String::Stream & String::Stream::operator << (intmax_t n)
{
  return *this << Format(n);
}
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::operator << (uintmax_t n)
{
  return *this << Format(n);
}
#endif
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::operator << (intptr_t n)
{
  return *this << Format(n);
}
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::operator << (uintptr_t n)
{
  return *this << Format(n);
}
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::operator << (const char * p)
{
  return *this << Format(p);
}
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::operator << (const void * const p)
{
  return *this << Format(p);
}
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::operator << (float n)
{
  return *this << Format(n);
}
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::operator << (double n)
{
  return *this << Format(n);
}
//---------------------------------------------------------------------------
#if HAVE_LONG_DOUBLE
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::operator << (long double n)
{
  return *this << Format(n);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
/*inline String::Stream & String::Stream::operator << (const utf8::String & s)
{
  uintptr_t size = s.size();
  stream_.realloc(count_ + size);
  memcpy(stream_.ptr() + count_,s.c_str(),size);
  count_ += size;
  return *this;
}*/
//---------------------------------------------------------------------------
/*inline String::Stream & String::Stream::operator << (const Format & a)
{
  intptr_t size = a.format(NULL);
  stream_.realloc(count_ + size);
  memset(stream_.ptr() + count_,'A',size);
  //a.format(stream_.ptr() + count_);
  count_ += size;
  return *this;
}*/
//---------------------------------------------------------------------------
inline const char * String::Stream::plane() const
{
  return stream_;
}
//---------------------------------------------------------------------------
inline const uintptr_t & String::Stream::count() const
{
  return count_;
}
//---------------------------------------------------------------------------
/*inline utf8::String String::Stream::string()
{
  stream_.realloc(count_ + 1);
  stream_[count_] = '\0';
  String::Container * container = newObject<String::Container>(0,stream_.ptr());
  stream_.ptr(NULL);
  count_ = 0;
  return container;
}*/
//---------------------------------------------------------------------------
inline String::Stream & String::Stream::clear()
{
  stream_.free();
  count_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline String::Stream::Format::~Format()
{
}
//---------------------------------------------------------------------------
inline String::Stream::Format::Format(char n,const char * fmt) : ci_(n), type_(CI)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
//---------------------------------------------------------------------------
inline String::Stream::Format::Format(unsigned char n,const char * fmt) : cu_(n), type_(CU)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
//---------------------------------------------------------------------------
inline String::Stream::Format::Format(short n,const char * fmt) : si_(n), type_(SI)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
//---------------------------------------------------------------------------
inline String::Stream::Format::Format(unsigned short n,const char * fmt) : su_(n), type_(SU)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_LONG
//---------------------------------------------------------------------------
inline String::Stream::Format::Format(long n,const char * fmt) : li_(n), type_(LI)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
//---------------------------------------------------------------------------
inline String::Stream::Format::Format(unsigned long n,const char * fmt) : lu_(n), type_(LU)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT
//---------------------------------------------------------------------------
inline String::Stream::Format::Format(int n,const char * fmt) : ii_(n), type_(II)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
//---------------------------------------------------------------------------
inline String::Stream::Format::Format(unsigned int n,const char * fmt) : iu_(n), type_(IU)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INTMAX_T
inline String::Stream::Format::Format(intmax_t n,const char * fmt) : mi_(n), type_(MI)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
//---------------------------------------------------------------------------
inline String::Stream::Format::Format(uintmax_t n,const char * fmt) : mu_(n), type_(MU)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
#endif
//---------------------------------------------------------------------------
inline String::Stream::Format::Format(intptr_t n,const char * fmt) : pi_(n), type_(PI)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
//---------------------------------------------------------------------------
inline String::Stream::Format::Format(uintptr_t n,const char * fmt) : pu_(n), type_(PU)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
//---------------------------------------------------------------------------
inline String::Stream::Format::Format(const char * p,const char * fmt) : s_(p), type_(S)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
//---------------------------------------------------------------------------
inline String::Stream::Format::Format(const void * const p,const char * fmt) : pu_((uintptr_t) p), type_(PU)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
//---------------------------------------------------------------------------
inline String::Stream::Format::Format(float a,const char * fmt) : f_(a), type_(F)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
//---------------------------------------------------------------------------
inline String::Stream::Format::Format(double a,const char * fmt) : d_(a), type_(D)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
//---------------------------------------------------------------------------
#if HAVE_LONG_DOUBLE
//---------------------------------------------------------------------------
inline String::Stream::Format::Format(long double a,const char * fmt) : ld_(a), type_(LD)
{
  strncpy0(fmt_,sizeof(fmt_),fmt);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T> class EStr2ScalarT : public T {
  public:
    EStr2ScalarT(int32_t code, const utf8::String & what);
};
//---------------------------------------------------------------------------
template <typename T> inline
EStr2ScalarT<T>::EStr2ScalarT(int32_t code,const utf8::String & what) : T(code, what)
{
}
//---------------------------------------------------------------------------
typedef EStr2ScalarT<ksys::Exception> EStr2Scalar;
//---------------------------------------------------------------------------
} // namespace utf8
//---------------------------------------------------------------------------
inline uintptr_t hash(const utf8::String & s,bool caseSensitive = true)
{
  return s.hash(caseSensitive);
}
//---------------------------------------------------------------------------
inline uintptr_t compareObjects(const utf8::String & s1,const utf8::String & s2,bool caseSensitive = true)
{
  static intptr_t (utf8::String::* const cmp[2])(const utf8::String &) const = {
    &utf8::String::strcasecmp, &utf8::String::strcmp
  };
  return (s1.*cmp[caseSensitive])(s2);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
