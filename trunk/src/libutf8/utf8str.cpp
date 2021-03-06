/*-
 * Copyright 2005-2008 Guram Dukashvili
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
namespace utf8 {
//---------------------------------------------------------------------------
extern const uint8_t utf8SeqLens[64] = {
//  000000  000001  000010, 000011, 000100, 000101, 000110, 000111
         1,      1,      1,      1,      1,      1,      1,      1,
//  001000  001001  001011, 001011, 001101, 001101, 001111, 001111
         1,      1,      1,      1,      1,      1,      1,      1,
//  010000  010001  010011, 010011, 010101, 010101, 010111, 010111
         1,      1,      1,      1,      1,      1,      1,      1,
//  011000  011001  011011, 011011, 011101, 011101, 011111, 011111
         1,      1,      1,      1,      1,      1,      1,      1,
//  100000  100001  100010, 100011, 100100, 100101, 100110, 100111
         0,      0,      0,      0,      0,      0,      0,      0,
//  101000  101001  101010, 101011, 101100, 101101, 101110, 101111
         0,      0,      0,      0,      0,      0,      0,      0,
//  110000  110001  110010, 110011, 110100, 110101, 110110, 110111
         2,      2,      2,      2,      2,      2,      2,      2,
//  111000  111001  111010, 111011, 111100, 111101, 111110, 111111
         3,      3,      3,      3,      4,      4,      5,      6
};
//---------------------------------------------------------------------------
intptr_t (utf8::String::* const strCmpFuncs[2])(const utf8::String &) const = {
    &utf8::String::casecompare, &utf8::String::compare
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
String::Container::Container() : string_(NULL), refCount_(0)/*, mutex_(0)*/
{
}
//---------------------------------------------------------------------------
String::Container * String::Container::container(uintptr_t l)
{
  ksys::AutoPtr<char,AutoPtrMemoryDestructor> string;
  string.alloc(l + 1);
  String::Container * cp = newObjectV1V2<Container,int,char *,AutoPtrNonVirtualClassDestructor>(0,string.ptr());
  string.ptr(NULL);
  return cp;
}
//---------------------------------------------------------------------------
/*void String::Container::acquire()
{
  for(;;){
    if( ksys::interlockedCompareExchange(mutex_,-1,0) == 0 ) break;
    ksys::ksleep1();
  }
}*/
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
char          String::nullString_[1]  = "";
unsigned char String::nullContainer_[sizeof(String::Container)];
//---------------------------------------------------------------------------
void String::initialize()
{
  new (&nullContainer_) Container(1,nullString_);
  detectCodePages();
}
//---------------------------------------------------------------------------
void String::cleanup()
{
  //  nullContainer().~Container();
}
//---------------------------------------------------------------------------
String plane(const char * s,uintptr_t size)
{
  const char * a = s;
  while( size > 0 && *s != '\0' ){
    uintptr_t l = utf8seqlen(s);
    if( l > size || l == 0 )
      newObjectV1C2<ksys::Exception>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
    s += l;
    size -= l;
  }
  if( s - a == 0 ) return String();
  String::Container * container = String::Container::container(uintptr_t(s - a));
  memcpy(container->string_,a,s - a);
  container->string_[s - a] = '\0';
  return container;
}
//---------------------------------------------------------------------------
String plane0(ksys::AutoPtr<char,AutoPtrMemoryDestructor> & s)
{
  String::Container * container = newObjectV1V2<String::Container,int,char *,AutoPtrNonVirtualClassDestructor>(0,s.ptr());
  s.ptr(NULL);
  return container;
}
//---------------------------------------------------------------------------
String plane(ksys::AutoPtr<char,AutoPtrMemoryDestructor> & s,uintptr_t size)
{
  String str(plane(s.ptr(),size));
  s.ptr(NULL);
  return str;
}
//---------------------------------------------------------------------------
String::String(const String::Iterator & i) : container_(&nullContainer())
{
  String::Container * container;
  if( i.container_.ptr() != &String::nullContainer() ){
    uintptr_t l = strlen(i.container_->string_ + i.cursor_);
    container = Container::container(l);
    memcpy(container->string_, i.container_->string_ + i.cursor_, l + 1);
  }
  else{
    container = &String::nullContainer();
  }
  container_ = container;
}
//---------------------------------------------------------------------------
String::String(const String::Iterator & i1, const String::Iterator & i2) : container_(&nullContainer())
{
  String::Container * container;
  assert(i1.container_ == i2.container_);
  if( i1.cursor_ > i2.cursor_ ){
    *this = String(i2, i1);
  }
  else {
    if( i1.container_.ptr() != &String::nullContainer() ){
      container = Container::container(i2.cursor_ - i1.cursor_);
      memcpy(container->string_, i1.container_->string_ + i1.cursor_, i2.cursor_ - i1.cursor_);
      container->string_[i2.cursor_ - i1.cursor_] = '\0';
    }
    else{
      container = &String::nullContainer();
    }
    container_ = container;
  }
}
//---------------------------------------------------------------------------
String::String(const char * s,uintptr_t l) : container_(&nullContainer())
{
  String::Container * container;
  uintptr_t len = mbcs2utf8s(CP_ACP,NULL,0,s,l);
  if( len > 0 ){
    container = Container::container(len);
    mbcs2utf8s(CP_ACP,container->string_,len + 1,s,l);
  }
  else {
    container = &String::nullContainer();
  }
  container_ = container;
}
//---------------------------------------------------------------------------
String::String(const unsigned char * s,uintptr_t l) : container_(&nullContainer())
{
  String::Container * container;
  uintptr_t len = mbcs2utf8s(CP_ACP,NULL,0,(const char *) s,l);
  if( len > 0 ){
    container = Container::container(len);
    mbcs2utf8s(CP_ACP,container->string_,len + 1,(const char *) s,l);
  }
  else {
    container = &String::nullContainer();
  }
  container_ = container;
}
//---------------------------------------------------------------------------
String::String(const wchar_t * s, uintptr_t l) : container_(&nullContainer())
{
  uintptr_t       ul, ql, ll;
  const wchar_t * a;
  for( ul = 0, ll = l, a = s; ll > 0 && *a != L'\0'; a++, ll-- ){
    ql = ucs2utf8seq((char *) NULL, 0, *a);
    if( ql == 0 )
      newObjectV1C2<ksys::Exception>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
    ul += ql;
  }
  String::Container * container;
  if( ul > 0 ){
    container = Container::container(ul);
    for( ul = 0, ll = l, a = s; ll > 0 && *a != L'\0'; a++, ll-- ){
      ql = ucs2utf8seq(container->string_ + ul, ~(uintptr_t) 0 >> 1, *a);
      ul += ql;
    }
    container->string_[ul] = '\0';
  }
  else{
    container = &String::nullContainer();
  }
  container_ = container;
}
//---------------------------------------------------------------------------
String & String::operator =(const char * str)
{
  uintptr_t len = mbcs2utf8s(CP_ACP, NULL, 0, str, ~uintptr_t(0) >> 1);
  if( len > 0 ){
    Container * container = Container::container(len);
    mbcs2utf8s(CP_ACP, container->string_, len + 1, str, ~uintptr_t(0) >> 1);
    container_ = container;
  }
  else {
    container_ = &nullContainer();
  }
  return *this;
}
//---------------------------------------------------------------------------
String & String::operator =(const wchar_t * str)
{
  return operator = (String(str));
}
//---------------------------------------------------------------------------
String & String::operator +=(const char * str)
{
  if( *str != '\0' ){
    uintptr_t l1, l2  = mbcs2utf8s(CP_ACP, NULL, 0, str, ~uintptr_t(0) >> 1);
    if( container_.ptr() != &nullContainer() ){
      container_->string_ = (char *) ksys::krealloc(container_->string_, (l1 = strlen(container_->string_)) + l2 + 1);
    }
    else{
      container_ = Container::container(l2);
      l1 = 0;
    }
    mbcs2utf8s(CP_ACP, container_->string_ + l1, l2 + 1, str, ~uintptr_t(0) >> 1);
  }
  return *this;
}
//---------------------------------------------------------------------------
String & String::operator +=(const wchar_t * str)
{
  return operator += (String(str));
}
//---------------------------------------------------------------------------
String String::operator +(const char * str) const
{
  return operator +(String(str));
}
//---------------------------------------------------------------------------
String String::operator +(const wchar_t * str) const
{
  return operator +(String(str));
}
//---------------------------------------------------------------------------
String String::operator +(const String & str) const
{
  Container * container;
  if( container_.ptr() != &nullContainer() || str.container_.ptr() != &nullContainer() ){
    uintptr_t l1  = strlen(container_->string_);
    uintptr_t l2  = strlen(str.container_->string_);
    container = Container::container(l1 + l2);
    memcpy(container->string_, container_->string_, l1);
    memcpy(container->string_ + l1, str.container_->string_, l2 + 1);
  }
  else{
    container = &nullContainer();
  }
  return container;
}
//---------------------------------------------------------------------------
String & String::operator +=(const String & str)
{
  if( str.container_.ptr() != &nullContainer() ){
    uintptr_t l1, l2  = strlen(str.container_->string_);
    if( container_.ptr() != &nullContainer() ){
      if( container_.ptr() != str.container_.ptr() )
        l1 = strlen(container_->string_);
      else
        l1 = l2;
      container_->string_ = (char *) ksys::krealloc(container_->string_, l1 + l2 + 1);
    }
    else{
      container_ = Container::container(l2);
      l1 = 0;
    }
    memcpy(container_->string_ + l1, str.container_->string_, l2 + 1);
  }
  return *this;
}
//---------------------------------------------------------------------------
String String::unique() const
{
  ksys::SPRC<Container> rc(container_);
  uintptr_t l = strlen(container_->string_);
  Container * container = Container::container(l);
  memcpy(container->string_,container_->string_,l + 1);
  return container;
}
//---------------------------------------------------------------------------
String & String::resize(uintptr_t bCount)
{
  if( bCount > 0 ){
    uintptr_t l = strlen(container_->string_);
    if( bCount != l ){
      if( container_.ptr() != &nullContainer() ){
        container_->string_ = (char *) ksys::krealloc(container_->string_, bCount + 1);
      }
      else{
        container_ = Container::container(bCount);
      }
      if( bCount > l ) memset(container_->string_ + l, ' ', bCount - l);
      container_->string_[bCount] = '\0';
    }
  }
  else if( container_.ptr() != &nullContainer() ){
    container_ = &nullContainer();
  }
  return *this;
}
//---------------------------------------------------------------------------
String String::lower() const
{
  uintptr_t l = strlen(container_->string_);
  Container * container;
  if( container_.ptr() != &nullContainer() ){
    container = Container::container(l);
    utf8s2Lower(container->string_, l + 1, container_->string_, l + 1);
    if( memcmp(container->string_, container_->string_, l) == 0 ){
      delete container;
      container = container_.ptr();
    }
  }
  else{
    container = &nullContainer();
  }
  return container;
}
//---------------------------------------------------------------------------
String String::upper() const
{
  uintptr_t l = strlen(container_->string_);
  Container * container;
  if( container_.ptr() != &nullContainer() ){
    container = Container::container(l);
    utf8s2Upper(container->string_, l + 1, container_->string_, l + 1);
    if( memcmp(container->string_, container_->string_, l) == 0 ){
      delete container;
      container = container_.ptr();
    }
  }
  else{
    container = &nullContainer();
  }
  return container;
}
//---------------------------------------------------------------------------
intptr_t String::compare(const String & s) const
{
  intptr_t              c;
  uintptr_t             l1, l2;
  const unsigned char * s1  = container_->ustring_, * s2 = s.container_->ustring_;
  for( ; ; ){
    c = utf82ucs(s1, l1) - utf82ucs(s2, l2);
    if( c != 0 || *s1 == 0 || *s2 == 0 ) break;
    s1 += l1;
    s2 += l2;
  }
  return c;
}
//---------------------------------------------------------------------------
intptr_t String::ncompare(const String & s, uintptr_t n) const
{
  intptr_t              c   = 0;
  uintptr_t             l1, l2;
  const unsigned char * s1  = container_->ustring_, * s2 = s.container_->ustring_;
  while( n-- > 0 ){
    c = utf82ucs(s1, l1) - utf82ucs(s2, l2);
    if( c != 0 || *s1 == 0 || *s2 == 0 ) break;
    s1 += l1;
    s2 += l2;
  }
  return c;
}
//---------------------------------------------------------------------------
intptr_t String::casecompare(const String & s) const
{
  intptr_t c;
  uintptr_t l1, l2;
  const char *  s1  = container_->string_, * s2 = s.container_->string_;
  for(;;){
    c = utf8c2UpperUCS(s1, l1) - utf8c2UpperUCS(s2, l2);
    if( c != 0 || *s1 == 0 || *s2 == 0 ) break;
    s1 += l1;
    s2 += l2;
  }
  return c;
}
//---------------------------------------------------------------------------
intptr_t String::ncasecompare(const String & s, uintptr_t n) const
{
  intptr_t      c   = 0;
  uintptr_t     l1, l2;
  const char *  s1  = container_->string_, * s2 = s.container_->string_;
  while( n-- > 0 ){
    c = utf8c2UpperUCS(s1,l1) - utf8c2UpperUCS(s2,l2);
    if( l1 == 0 || l2 == 0 )
      newObjectV1C2<ksys::Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    if( c != 0 || *s1 == 0 || *s2 == 0 ) break;
    s1 += l1;
    s2 += l2;
  }
  return c;
}
//---------------------------------------------------------------------------
String::Iterator String::strstr(const String & str) const
{
  intptr_t c;
  uintptr_t l0, l1, l2;
  Iterator i(*this), i2(i);
  i2.cursor_ = i2.position_ = -1;
  if( str.container_->string_[0] != '\0' ){
    while( !i.eos() ){
      const char * s1 = container_->string_ + i.cursor_, * s2 = str.container_->string_;
      c = utf82ucs(s1,l0) - utf82ucs(s2,l2);
      l1 = l0;
      for(;;){
        if( *s2 == '\0' ) return i;
        if( *s1 == '\0' || c != 0 ) break;
        s1 += l1;
        s2 += l2;
        c = utf82ucs(s1,l1) - utf82ucs(s2,l2);
      }
      i.cursor_ += l0;
      i.position_++;
    }
  }
  return i2;
}
//---------------------------------------------------------------------------
String::Iterator String::strcasestr(const String & str) const
{
  intptr_t c;
  uintptr_t l0, l1, l2;
  Iterator i(*this), i2(i);
  i2.cursor_ = i2.position_ = -1;
  if( str.container_->string_[0] != '\0' ){
    while( !i.eos() ){
      const char * s1 = container_->string_ + i.cursor_, * s2 = str.container_->string_;
      c = utf8c2UpperUCS(s1,l0) - utf8c2UpperUCS(s2,l2);
      l1 = l0;
      for(;;){
        if( *s2 == '\0' ) return i;
        if( *s1 == '\0' || c != 0 ) break;
        s1 += l1;
        s2 += l2;
        c = utf8c2UpperUCS(s1,l1) - utf8c2UpperUCS(s2,l2);
      }
      i.cursor_ += l0;
      i.position_++;
    }
  }
  return i2;
}
//---------------------------------------------------------------------------
String::Iterator String::strrstr(const String & str) const
{
  intptr_t  c = 0;
  uintptr_t l0, l1, l2;
  Iterator i(*this), i2(i);
  i2.cursor_ = i2.position_ = -1;
  if( str.container_->string_[0] != '\0' ){
    while( !i.eos() ){
      const char * s1 = container_->string_ + i.cursor_, * s2 = str.container_->string_;
      c = utf82ucs(s1,l0) - utf82ucs(s2,l2);
      l1 = l0;
      for(;;){
        if( *s2 == '\0' ){ i2 = i; break; }
        if( *s1 == '\0' || c != 0 ) break;
        s1 += l1;
        s2 += l2;
        c = utf82ucs(s1,l1) - utf82ucs(s2,l2);
      }
      i.cursor_ += l0;
      i.position_++;
    }
  }
  return i2;
}
//---------------------------------------------------------------------------
String::Iterator String::strrcasestr(const String & str) const
{
  intptr_t  c = 0;
  uintptr_t l0, l1, l2;
  Iterator i(*this), i2(i);
  i2.cursor_ = i2.position_ = -1;
  if( str.container_->string_[0] != '\0' ){
    while( !i.eos() ){
      const char * s1 = container_->string_ + i.cursor_, * s2 = str.container_->string_;
      c = utf8c2UpperUCS(s1,l0) - utf8c2UpperUCS(s2,l2);
      l1 = l0;
      for(;;){
        if( *s2 == '\0' ){ i2 = i; break; }
        if( *s1 == '\0' || c != 0 ) break;
        s1 += l1;
        s2 += l2;
        c = utf8c2UpperUCS(s1,l1) - utf8c2UpperUCS(s2,l2);
      }
      i.cursor_ += l0;
      i.position_++;
    }
  }
  return i2;
}
//---------------------------------------------------------------------------
AnsiString String::getANSIString() const
{
  AnsiString  p ((char *) ksys::kmalloc(utf8s2mbcs(CP_ACP, NULL, 0, container_->string_, ~uintptr_t(0) >> 1) + 1));
  utf8s2mbcs(CP_ACP, p, ~(uintptr_t) 0 >> 1, container_->string_, ~uintptr_t(0) >> 1);
  return p;
}
//---------------------------------------------------------------------------
OemString String::getOEMString() const
{
  OemString p ((char *) ksys::kmalloc(utf8s2mbcs(CP_OEMCP, NULL, 0, container_->string_, ~uintptr_t(0) >> 1) + 1));
  utf8s2mbcs(CP_OEMCP, p, ~(uintptr_t) 0 >> 1, container_->string_, ~uintptr_t(0) >> 1);
  return p;
}
//---------------------------------------------------------------------------
WideString String::getUNICODEString() const
{
  WideString  p ((wchar_t *) ksys::kmalloc((length() + 1) * sizeof(wchar_t)));
  Iterator i(*this);
  for(;;){
    p[i.position()] = (wchar_t) i.getChar();
    if( i.eos() ) break;
    i.next();
  }
  return p;
}
//---------------------------------------------------------------------------
uintptr_t String::getMBCSString(const char * string,uintptr_t codePage,ksys::AutoPtrBuffer & s,bool eos)
{
  intptr_t sl;
  if( codePage != CP_UTF8 ){
    sl = utf8s2mbcs(codePage,NULL,0,string,~uintptr_t(0) >> 1);
    if( sl < 0 ){
      int32_t err = errno;
      newObjectV1C2<ksys::Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    if( eos ) sl += codePage == CP_UNICODE ? sizeof(wchar_t) : sizeof(char);
    s.realloc(sl);
    mbcs2utf8s(codePage,(char *) s.ptr(),sl,string,~uintptr_t(0) >> 1);
  }
  else {
    sl = ::strlen(string);
    if( eos ) sl += 1;
    s.realloc(sl);
    memcpy(s,string,sl);
  }
  return sl;
}
//---------------------------------------------------------------------------
uintptr_t String::getMBCSString(uintptr_t codePage,ksys::AutoPtrBuffer & s,bool eos) const
{
  return getMBCSString(container_->string_,codePage,s,eos);
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
BSTR String::getOLEString() const
{
  uintptr_t l = length();
  UINT t = 0;
  t = ~t;
  if( l > t ) newObjectV1C2<ksys::Exception>(ERROR_INVALID_DATA,__PRETTY_FUNCTION__)->throwSP();
  BSTR p = SysAllocStringLen(NULL,UINT(l));
  if( p == NULL ) newObjectV1C2<ksys::EOutOfMemory>(
    ERROR_NOT_ENOUGH_MEMORY + ksys::errorOffset,__PRETTY_FUNCTION__)->throwSP();
  Iterator i(*this);
  while( !i.eos() ){
    p[i.position()] = (wchar_t) i.getChar();
    i.next();
  }
  return p;
}
#endif
//---------------------------------------------------------------------------
String String::trimLeft() const
{
  Iterator sl(*this);
  while( !sl.eos() && sl.isSpace() ) sl.next();
  Container * container;
  if( sl.eos() ){
    container = &nullContainer();
  }
  else if( sl.position() > 0 ){
    container = Container::container(strlen(container_->string_) - sl.cursor());
    strcpy(container->string_,sl.c_str());
  }
  else{
    container = container_.ptr();
  }
  return container;
}
//---------------------------------------------------------------------------
String String::trimRight() const
{
  Iterator sr(*this);
  sr.last();
  while( sr.prev() && sr.isSpace() );
  Container * container;
  if( sr.bos() ){
    container = &nullContainer();
  }
  else if( sr.position() > 0 ){
    sr++;
    container = Container::container(sr.cursor());
    memcpy(container->string_,container_->string_,sr.cursor());
    container->string_[sr.cursor()] = '\0';
  }
  else {
    container = container_.ptr();
  }
  return container;
}
//---------------------------------------------------------------------------
String String::trim() const
{
  Iterator sl(*this);
  while( !sl.eos() && sl.isSpace() ) sl.next();
  Iterator sr(sl);
  sr.last();
  while( !sr.bos() && (sr.isSpace() || sr.eos()) ) sr.prev();
  Container * container;
  if( sl.eos() && sr.bos() ){
    container = &nullContainer();
  }
  else if( sl.isFirst() && sr.isLast() ){
    container = container_.ptr();
  }
  else{
    sr.next();
    container = Container::container(sr.cursor() - sl.cursor());
    memcpy(container->string_, container_->string_ + sl.cursor_, sr.cursor() - sl.cursor());
    container->string_[sr.cursor() - sl.cursor()] = '\0';
  }
  return container;
}
//---------------------------------------------------------------------------
#ifdef __BCPLUSPLUS__
String String::cut(const Iterator & i) const
{
  return cut(i, i + 1);
}
#endif
//---------------------------------------------------------------------------
String String::cut(const Iterator & i1, const Iterator & i2) const
{
  assert(container_.ptr() == i1.container_.ptr() && i1.container_.ptr() == i2.container_.ptr() && i1.cursor_ <= i2.cursor_);
  Container * container;
  if( i1.cursor_ < i2.cursor_ ){
    container = Container::container(i2.cursor_ - i1.cursor_);
    memcpy(container->string_, i1.c_str(), i2.cursor_ - i1.cursor_);
    container->string_[i2.cursor_ - i1.cursor_] = '\0';
    uintptr_t l = strlen(i2.c_str());
    if( l == 0 ){
      container_ = &nullContainer();
      i1.container_ = i2.container_ = container_;
    }
    else{
      memcpy(container->string_, i2.c_str(), l + 1);
      container_->string_ = (char *) ksys::krealloc(container_->string_, i1.cursor_ + l + 1);
    }
  }
  else{
    container = &String::nullContainer();
  }
  return container;
}
//---------------------------------------------------------------------------
String & String::replace(const Iterator & d1,const Iterator & d2, const Iterator & s1, const Iterator & s2)
{
  assert( container_ == d1.container_ );
  assert( d1.container_ == d2.container_ );
  assert( s1.container_ == s2.container_ );
  if( container_.ptr() == &nullContainer() ) return *this = String(s1, s2);
  if( s1.container_ == d1.container_ ){
    String a(String(s1,s2));
    return replace(d1,d2,String::Iterator(a),String::Iterator(a).last());
  }
  if( d1.cursor_ > d2.cursor_ ) return replace(d2, d1, s1, s2);
  if( s1.cursor_ > s2.cursor_ ) return replace(d1, d1, s2, s1);
  uintptr_t l = strlen(container_->string_);
  intptr_t  k = (s2.cursor_ - s1.cursor_) - (d2.cursor_ - d1.cursor_);
  if( k > 0 ) container_->string_ = (char *) ksys::krealloc(container_->string_, l + k + 1);
  memmove((char *) d2.c_str() + k, d2.c_str(), l - d2.cursor_ + 1);
  memcpy((char *) d1.c_str(), s1.c_str(), s2.cursor_ - s1.cursor_);
  if( k < 0 ) container_->string_ = (char *) ksys::krealloc(container_->string_, l + k + 1);
  return *this;
}
//---------------------------------------------------------------------------
String String::replaceAll(const String & what,const String & onWhat) const
{
  String s(unique());
  Iterator i(s.strstr(what));
  while( !i.eos() ){
    s = s.replace(i,i + what.length(),onWhat);
    i = s.strstr(what);
  }
  return s;
}
//---------------------------------------------------------------------------
String String::replaceCaseAll(const String & what,const String & onWhat) const
{
  String s(unique());
  Iterator i(s.strcasestr(what));
  while( !i.eos() ){
    s = s.replace(i,i + what.length(),onWhat);
    i = s.strstr(what);
  }
  return s;
}
//---------------------------------------------------------------------------
String String::left(uintptr_t symbols) const
{
  return String(Iterator(*this), Iterator(*this) + symbols);
}
//---------------------------------------------------------------------------
String String::right(uintptr_t symbols) const
{
  Iterator i(*this);
  i.last();
  return String(i - symbols, i);
}
//---------------------------------------------------------------------------
String String::middle(uintptr_t pos, uintptr_t symbols) const
{
  Iterator i(*this);
  i += pos;
  return String(i, i + symbols);
}
//---------------------------------------------------------------------------
bool String::isValid() const
{
  const unsigned char * p = container_->ustring_;
  while( *p != '\0' ){
    uintptr_t l, c = utf82ucs(p,l);
    if( c > 0x7FFFFFFF ) return false;
    switch( l ){
      case 6 :
        if( *p++ == '\0' ) return false;
      case 5 :
        if( *p++ == '\0' ) return false;
      case 4 :
        if( *p++ == '\0' ) return false;
      case 3 :
        if( *p++ == '\0' ) return false;
      case 2 :
        if( *p++ == '\0' ) return false;
      case 1 :
        if( *p++ == '\0' ) return false;
        break;
      case 0 :
        return false;
    }
  }
  return true;
}
//---------------------------------------------------------------------------
String String::catPrint(const char * fmt, ...)
{
  int a = 31;
  va_list arglist;
  String s;
  
  s.resize(a);
  va_start(arglist,fmt);
#if HAVE_VSNPRINTF
  a = vsnprintf(s.c_str(),a + 1,fmt,arglist);
#elif HAVE__VSNPRINTF
  a = _vsnprintf(s.c_str(),a + 1,fmt,arglist);
#endif
  va_end(arglist);
  if( a < 0 ){ // hack for ugly msvcrt.dll versions
    s.resize(a = 65535);
    va_start(arglist,fmt);
    a =
#if __BCPLUSPLUS__
      std::
#endif
#if HAVE_VSNPRINTF
      vsnprintf(s.c_str(),a + 1,fmt,arglist);
#elif HAVE__VSNPRINTF
      _vsnprintf(s.c_str(),a + 1,fmt,arglist);
#endif
    va_end(arglist);
    s.resize(a);
  }
  else {
    s.resize(a);
    va_start(arglist,fmt);
#if HAVE_VSNPRINTF
    vsnprintf(s.c_str(),a + 1,fmt,arglist);
#elif HAVE__VSNPRINTF
    _vsnprintf(s.c_str(),a + 1,fmt,arglist);
#endif
    va_end(arglist);
  }
  return *this + s;
}
//---------------------------------------------------------------------------
String String::print(const char * fmt, ...)
{
  int a = 31;
  va_list arglist;
  String s;
  
  s.resize(a);
  va_start(arglist,fmt);
#if HAVE_VSNPRINTF
  a = vsnprintf(s.c_str(),a + 1,fmt,arglist);
#elif HAVE__VSNPRINTF
  a = _vsnprintf(s.c_str(),a + 1,fmt,arglist);
#endif
  va_end(arglist);
  if( a < 0 ){ // hack for ugly msvcrt.dll versions
    s.resize(a = 65535);
    va_start(arglist,fmt);
    a =
#if __BCPLUSPLUS__
      std::
#endif
#if HAVE_VSNPRINTF
      vsnprintf(s.c_str(),a + 1,fmt,arglist);
#elif HAVE__VSNPRINTF
      _vsnprintf(s.c_str(),a + 1,fmt,arglist);
#endif
    va_end(arglist);
    s.resize(a);
  }
  else {
    s.resize(a);
    va_start(arglist,fmt);
#if HAVE_VSNPRINTF
    vsnprintf(s.c_str(),a + 1,fmt,arglist);
#elif HAVE__VSNPRINTF
    _vsnprintf(s.c_str(),a + 1,fmt,arglist);
#endif
    va_end(arglist);
  }
  return s;
}
//---------------------------------------------------------------------------
uintptr_t String::hash(bool caseSensitive,uintptr_t h) const
{
  if( caseSensitive ){
    h = ksys::HF::hash(container_->string_,h);
  }
  else {
    uintptr_t l;
    const char * s = container_->string_;
    for(;;){
      if( *s == '\0' ) break;
      h = ksys::HF::hash(uint32_t(utf8c2UpperUCS(s,l)),h);
      s += l;
      if( *s == '\0' ) break;
      h = ksys::HF::hash(uint32_t(utf8c2UpperUCS(s,l)),h);
      s += l;
      if( *s == '\0' ) break;
      h = ksys::HF::hash(uint32_t(utf8c2UpperUCS(s,l)),h);
      s += l;
      if( *s == '\0' ) break;
      h = ksys::HF::hash(uint32_t(utf8c2UpperUCS(s,l)),h);
      s += l;
    }
  }
  return h;
}
//---------------------------------------------------------------------------
uint64_t String::hash_ll(bool caseSensitive,uint64_t h) const
{
  if( caseSensitive ){
    h = ksys::HF::hash_ll(container_->string_,h);
  }
  else {
    uintptr_t l;
    const char * s = container_->string_;
    for(;;){
      if( *s == '\0' ) break;
      h = ksys::HF::hash_ll(uint32_t(utf8c2UpperUCS(s,l)),h);
      s += l;
      if( *s == '\0' ) break;
      h = ksys::HF::hash_ll(uint32_t(utf8c2UpperUCS(s,l)),h);
      s += l;
      if( *s == '\0' ) break;
      h = ksys::HF::hash_ll(uint32_t(utf8c2UpperUCS(s,l)),h);
      s += l;
      if( *s == '\0' ) break;
      h = ksys::HF::hash_ll(uint32_t(utf8c2UpperUCS(s,l)),h);
      s += l;
    }
  }
  return h;
}
//---------------------------------------------------------------------------
bool String::isDigit() const
{
  Iterator i(*this);
  while( !i.eos() && !i.isDigit() ) i.next();
  return !i.eos();
}
//---------------------------------------------------------------------------
bool String::isAlpha() const
{
  Iterator i(*this);
  while( !i.eos() && !i.isAlpha() ) i.next();
  return !i.eos();
}
//---------------------------------------------------------------------------
bool String::isAlnum() const
{
  Iterator i(*this);
  while( !i.eos() && !i.isAlnum() ) i.next();
  return !i.eos();
}
//---------------------------------------------------------------------------
bool String::isPrint() const
{
  Iterator i(*this);
  while( !i.eos() && !i.isPrint() ) i.next();
  return !i.eos();
}
//---------------------------------------------------------------------------
bool String::isSpace() const
{
  Iterator i(*this);
  while( !i.eos() && !i.isSpace() ) i.next();
  return !i.eos();
}
//---------------------------------------------------------------------------
bool String::isPunct() const
{
  Iterator i(*this);
  while( !i.eos() && !i.isPunct() ) i.next();
  return !i.eos();
}
//---------------------------------------------------------------------------
bool String::isCntrl() const
{
  Iterator i(*this);
  while( !i.eos() && !i.isCntrl() ) i.next();
  return !i.eos();
}
//---------------------------------------------------------------------------
bool String::isBlank() const
{
  Iterator i(*this);
  while( !i.eos() && !i.isBlank() ) i.next();
  return !i.eos();
}
//---------------------------------------------------------------------------
bool String::isXdigit() const
{
  Iterator i(*this);
  while( !i.eos() && !i.isXdigit() ) i.next();
  return !i.eos();
}
//---------------------------------------------------------------------------
bool String::isUpper() const
{
  Iterator i(*this);
  while( !i.eos() && !i.isUpper() ) i.next();
  return !i.eos();
}
//---------------------------------------------------------------------------
bool String::isLower() const
{
  Iterator i(*this);
  while( !i.eos() && !i.isLower() ) i.next();
  return !i.eos();
}
//---------------------------------------------------------------------------
String operator + (const char * s1,const String & s2)
{
  return String(s1) + s2;
}
//---------------------------------------------------------------------------
String operator + (const wchar_t * s1,const String & s2)
{
  return String(s1) + s2;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
String::Iterator & String::Iterator::operator += (intptr_t relPos)
{
  while( relPos > 0 ) if( next() ) relPos--; else break;
  while( relPos < 0 ) if( prev() ) relPos++; else break;
  return *this;
}
//---------------------------------------------------------------------------
String::Iterator & String::Iterator::operator -=(intptr_t relPos)
{
  while( relPos > 0 ) if( prev() ) relPos--; else break;
  while( relPos < 0 ) if( next() ) relPos++; else break;
  return *this;
}
//---------------------------------------------------------------------------
#ifdef __BCPLUSPLUS__
String::Iterator String::Iterator::operator +(intptr_t relPos) const
{
  return Iterator(*this) += relPos;
}
//---------------------------------------------------------------------------
String::Iterator String::Iterator::operator -(intptr_t relPos) const
{
  return Iterator(*this) -= relPos;
}
#endif
//---------------------------------------------------------------------------
String::Iterator & String::Iterator::reset()
{
  intptr_t position = position_;
  cursor_ = 0;
  position_ = 0;
  while( position_ < position && container_->ustring_[cursor_] != 0 ){
    uintptr_t sl = utf8seqlen(container_->ustring_ + cursor_);
    if( sl == 0 )
      newObjectV1C2<ksys::Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    cursor_ += sl;
    position_++;
  }
  return *this;
}
//---------------------------------------------------------------------------
String::Iterator & String::Iterator::last()
{
  if( position_ < 0 || cursor_ < 0 ) position_ = cursor_ = 0;
  uintptr_t size;
  position_ += utf8strlen(container_->string_ + cursor_,size);
  cursor_ += size;
  return *this;
}
//---------------------------------------------------------------------------
bool String::Iterator::prev()
{
  if( cursor_ == 0 ) return false;
  for(;;){
    uintptr_t c = container_->ustring_[--cursor_];
    if( c < 0x80 || (c & 0xC0) == 0xC0 ) break;
  }
  position_--;
  return true;
}
//---------------------------------------------------------------------------
bool String::Iterator::next()
{
  if( container_->ustring_[cursor_] == 0 ) return false;
  for( ; ; ){
    uintptr_t c = container_->ustring_[++cursor_];
    if( c < 0x80 || (c & 0xC0) == 0xC0 ) break;
  }
  position_++;
  return true;
}
//---------------------------------------------------------------------------
#ifdef __BCPLUSPLUS__
String::Iterator String::Iterator::operator[](intptr_t relPos) const
{
  return Iterator(*this) += relPos;
}
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// misc string manipulators /////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT
//---------------------------------------------------------------------------
String int2Str(int a)
{
  int x;
  uintptr_t l = int2StrLen((intmax_t) a);
  if( a < 0 ) x = -a; else x = a;
  String::Container * container = String::Container::container(l);
  char * p;
  *(p = container->string_ + l) = '\0';
  do{
    *--p = (char) (x % 10u + '0');
    x /= 10u;
  } while( x != 0 );
  if( a < 0 ) *--p = '-';
  return container;
}
//---------------------------------------------------------------------------
String int2Str(unsigned int a)
{
  uintptr_t           l         = int2StrLen((uintmax_t) a);
  String::Container * container = String::Container::container(l);
  char *              p;
  *(p = container->string_ + l) = '\0';
  do{
    *--p = (char) (a % 10u + '0');
    a /= 10u;
  } while( a != 0 );
  return container;
}
//---------------------------------------------------------------------------
String int2Str0(int a,uintptr_t padding)
{
  int x;
  uintptr_t l = int2StrLen((intmax_t) a);
  if( padding > l ) l = padding;
  if( a < 0 ) x = -a; else x = a;
  String::Container * container = String::Container::container(l);
  char * p;
  *(p = container->string_ + l) = '\0';
  do {
    *--p = (char) (x % 10u + '0');
    x /= 10u;
    --l;
  } while( x != 0 );
  if( a < 0 ){
    while( l-- > 1 ) *--p = '0';
    *--p = '-';
  }
  else {
    while( l-- > 0 ) *--p = '0';
  }
  return container;
}
//---------------------------------------------------------------------------
String int2Str0(unsigned int a,uintptr_t padding)
{
  uintptr_t l = int2StrLen((uintmax_t) a);
  if( padding > l ) l = padding;
  String::Container * container = String::Container::container(l);
  char * p;
  *(p = container->string_ + l) = '\0';
  do {
    *--p = (char) (a % 10u + '0');
    a /= 10u;
  } while( a != 0 || --l > 0 );
  return container;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
String int2Str(intptr_t a)
{
  intptr_t  x;
  uintptr_t l = int2StrLen((intmax_t) a);
  if( a < 0 ) x = -a; else x = a;
  String::Container * container = String::Container::container(l);
  char *              p;
  *(p = container->string_ + l) = '\0';
  do {
    *--p = (char) (x % 10u + '0');
    x /= 10u;
  } while( x != 0 );
  if( a < 0 ) *--p = '-';
  return container;
}
//---------------------------------------------------------------------------
String int2Str(uintptr_t a)
{
  uintptr_t           l         = int2StrLen((uintmax_t) a);
  String::Container * container = String::Container::container(l);
  char *              p;
  *(p = container->string_ + l) = '\0';
  do{
    *--p = (char) (a % 10u + '0');
    a /= 10u;
  }
  while( a != 0 );
  return container;
}
//---------------------------------------------------------------------------
String int2Str0(intptr_t a,uintptr_t padding)
{
  intptr_t  x;
  uintptr_t l = int2StrLen((intmax_t) a);
  if( padding > l ) l = padding;
  if( a < 0 ) x = -a; else x = a;
  String::Container * container = String::Container::container(l);
  char * p;
  *(p = container->string_ + l) = '\0';
  do {
    *--p = (char) (x % 10u + '0');
    x /= 10u;
    --l;
  } while( x != 0 );
  if( a < 0 ){
    while( l-- > 1 ) *--p = '0';
    *--p = '-';
  }
  else {
    while( l-- > 0 ) *--p = '0';
  }
  return container;
}
//---------------------------------------------------------------------------
String int2Str0(uintptr_t a,uintptr_t padding)
{
  uintptr_t l = int2StrLen((uintmax_t) a);
  if( padding > l ) l = padding;
  String::Container * container = String::Container::container(l);
  char * p;
  *(p = container->string_ + l) = '\0';
  do {
    *--p = (char) (a % 10u + '0');
    a /= 10u;
  } while( a != 0 || --l > 0 );
  return container;
}
//---------------------------------------------------------------------------
static inline uintmax_t sfSHL(uintmax_t c)
{
  uintmax_t a = (c << 3) + (c << 1);
  return a > c ? a : ~uintmax_t(0);
}
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INTMAX_T
String int2Str(intmax_t a)
{
  intmax_t  x;
  uintptr_t l = int2StrLen(a);
  if( a < 0 ) x = -a; else x = a;
  String::Container * container = String::Container::container(l);
  char * p;
  *(p = container->string_ + l) = '\0';
  do {
    *--p = (char) (x % 10u + '0');
    x /= 10u;
  } while( x != 0 );
  if( a < 0 ) *--p = '-';
  return container;
}
//---------------------------------------------------------------------------
String int2Str(uintmax_t a)
{
  uintptr_t l = int2StrLen(a);
  String::Container * container = String::Container::container(l);
  char * p;
  *(p = container->string_ + l) = '\0';
  do {
    *--p = (char) (a % 10u + '0');
    a /= 10u;
  } while( a != 0 );
  return container;
}
//---------------------------------------------------------------------------
uintptr_t int2StrLen(intmax_t a)
{
  uintptr_t l;
  uintmax_t x, c;

  if( a < 0 ) x = -a; else x = a;
  for( l = 1, c = 10; c < x; c = sfSHL(c), l++ );
  return l + (a < 0) + (c == x);
}
//---------------------------------------------------------------------------
uintptr_t int2StrLen(uintmax_t a)
{
  uintptr_t l;
  uintmax_t c;

  for( l = 1, c = 10; c < a; c = sfSHL(c), l++ );
  return l + (c == a);
}
//---------------------------------------------------------------------------
String int2Str0(intmax_t a,uintptr_t padding)
{
  intmax_t x;
  uintptr_t l = int2StrLen(a);
  if( padding > l ) l = padding;
  if( a < 0 ) x = -a; else x = a;
  String::Container * container = String::Container::container(l);
  char * p;
  *(p = container->string_ + l) = '\0';
  do {
    *--p = (char) (x % 10u + '0');
    x /= 10u;
    --l;
  } while( x != 0 );
  if( a < 0 ){
    while( l-- > 1 ) *--p = '0';
    *--p = '-';
  }
  else {
    while( l-- > 0 ) *--p = '0';
  }
  return container;
}
//---------------------------------------------------------------------------
String int2Str0(uintmax_t a,uintptr_t padding)
{
  uintptr_t l = int2StrLen(a);
  if( padding > l ) l = padding;
  String::Container * container = String::Container::container(l);
  char * p;
  *(p = container->string_ + l) = '\0';
  do {
    *--p = (char) (a % 10u + '0');
    a /= 10u;
    l--;
  } while( a != 0 );
  while( l-- > 0 ) *--p = '0';
  return container;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
uintptr_t int2StrLen(intptr_t a)
{
  uintptr_t l;
  uintmax_t x, c;

  if( a < 0 ) x = -a; else x = a;
  for( l = 1, c = 10; c < x; c = sfSHL(c), l++ );
  return l + (a < 0) + (c == x);
}
//---------------------------------------------------------------------------
uintptr_t int2StrLen(uintptr_t a)
{
  uintptr_t l;
  uintmax_t c;

  for( l = 1, c = 10; c < a; c = sfSHL(c), l++ );
  return l + (c == a);
}
//---------------------------------------------------------------------------
String int2HexStr(uintmax_t a,uintptr_t padding)
{
  if( padding > 16 ) padding = 16;
  char b[17], * p;
  *(p = b + sizeof(b) - 1) = '\0';
  do {
    uintptr_t q = uintptr_t(a % 16u);
    if( q > 9 )
      *--p = (char) (q + 'A' - 10);
    else
      *--p = (char) (q + '0');
    a /= 16u;
    if( padding > 0 ) padding--;
  } while( a != 0 );
  while( padding > 0 ){
    *--p = '0';
    padding--;
  }
  return plane(p);
}
//---------------------------------------------------------------------------
bool tryStr2Int(const String & str, intmax_t & a, uintptr_t pow)
{
  union {
    struct {
      uint8_t plus              : 1;
      uint8_t minus             : 1;
      uint8_t digit             : 1;
      uint8_t spaceAfterDigit   : 1;
      uint8_t spaceBeforeDigit  : 1;
    };
    unsigned char m;
  } m;
  m.m = 0;
  uintmax_t x = 0;
  String::Iterator i(str);
  while( !i.eos() ){
    if( i.isSpace() ){
      if( !m.digit )
        m.spaceBeforeDigit = true;
      else
        m.spaceAfterDigit = true;
    }
    else if( (i.isDigit() || (i.isXdigit() && pow > 10)) && !m.spaceAfterDigit ){
      switch( pow ){
        case 2 :
          x = x * 2u + i.getChar() - '0';
          break;
        case 8 :
          x = x * 8u + i.getChar() - '0';
          break;
        default :
        case 10 :
          x = x * 10u + i.getChar() - '0';
          break;
        case 16 :
          if( i.isDigit() )
            x = x * 16u + i.getChar() - '0';
          else
            x = x * 16u + i.getUpperChar() - 'A' + 10;
          break;
      }
      m.digit = true;
    }
    else if( i.getChar() == '-' && !m.plus && !m.minus && !m.digit && !m.spaceAfterDigit ){
      m.minus = true;
    }
    else if( i.getChar() == '+' && !m.plus && !m.minus && !m.digit && !m.spaceAfterDigit ){
      m.plus = true;
    }
    else
      break;
    i.next();
  }
  if( i.eos() ){
    if( m.minus ) x = -intmax_t(x);
    a = x;
  }
  return i.eos();
}
//---------------------------------------------------------------------------
intmax_t str2Int(const String & str,uintptr_t pow)
{
  intmax_t a = 0;
  if( !tryStr2Int(str,a,pow) )
    newObjectV1C2<EStr2Scalar>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
  return a;
}
//---------------------------------------------------------------------------
String ptr2Str(const void * p)
{
  union {
    uintmax_t     m;
    const void *  a;
  };
  m = 0;
  a = p;
  return int2Str(m);
}
//---------------------------------------------------------------------------
bool tryStr2tm(const String & str, struct tm & tv)
{
  int64_t us  = 0;
  memset(&tv, 0, sizeof(tv));
  int k = sscanf(str.c_str(), "%02u.%02u.%04u %02u:%02u:%02u.%06" PRId64, &tv.tm_mday, &tv.tm_mon, &tv.tm_year, &tv.tm_hour, &tv.tm_min, &tv.tm_sec, &us);
  tv.tm_year -= 1900;
  tv.tm_mon--;
  return k >= 3 && mktime(&tv) >= 0 && (uint64_t) us < 1000000;
}
//---------------------------------------------------------------------------
bool tryStr2Time(const String & str, int64_t & tv)
{
  int64_t   tvs, us = 0;
  struct tm t;
  memset(&t, 0, sizeof(t));
  int k = sscanf(str.c_str(), "%02u.%02u.%04u %02u:%02u:%02u.%06" PRId64, &t.tm_mday, &t.tm_mon, &t.tm_year, &t.tm_hour, &t.tm_min, &t.tm_sec, &us);
  t.tm_year -= 1900;
  t.tm_mon--;
  tv = (tvs = mktime(&t)) * UINT64_C(1000000) + us;
  return k >= 3 && tvs >= 0 && (uint64_t) us < 1000000;
}
//---------------------------------------------------------------------------
bool tryStr2Timeval(const String & str,struct timeval & tv)
{
  struct tm t;
  memset(&t,0,sizeof(t));
  tv.tv_usec = 0;
  int k = sscanf(str.c_str(), "%02u.%02u.%04u %02u:%02u:%02u.%06lu", &t.tm_mday, &t.tm_mon, &t.tm_year, &t.tm_hour, &t.tm_min, &t.tm_sec, &tv.tv_usec);
  t.tm_year -= 1900;
  t.tm_mon--;
#ifdef _MSC_VER
#pragma warning(disable:4244)
#endif
  tv.tv_sec = mktime(&t);
#ifdef _MSC_VER
#pragma warning(default:4244)
#endif
  return k >= 3 && tv.tv_sec >= 0 && tv.tv_usec < 1000000;
}
//---------------------------------------------------------------------------
struct tm str2tm(const String & str)
{
  struct tm tv;
  if( !tryStr2tm(str, tv) )
    newObjectV1C2<EStr2Scalar>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  return tv;
}
//---------------------------------------------------------------------------
int64_t str2Time(const String & str)
{
  int64_t tv;
  if( !tryStr2Time(str, tv) )
    newObjectV1C2<EStr2Scalar>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  return tv;
}
//---------------------------------------------------------------------------
struct timeval str2Timeval(const String & str)
{
  struct timeval  tv;
  if( !tryStr2Timeval(str, tv) )
    newObjectV1C2<EStr2Scalar>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  return tv;
}
//---------------------------------------------------------------------------
String tm2Str(struct tm tv)
{
  uintptr_t l = 10;
  if( tv.tm_hour != 0 || tv.tm_min != 0 || tv.tm_sec != 0 ) l += 9;
  String::Container * container = String::Container::container(l);
  sprintf(container->string_, "%02u.%02u.%04u", tv.tm_mday, tv.tm_mon + 1, tv.tm_year + 1900);
  if( tv.tm_hour != 0 || tv.tm_min != 0 || tv.tm_sec != 0 ){
    sprintf(container->string_ + 10, " %02u:%02u:%02u", tv.tm_hour, tv.tm_min, tv.tm_sec);
  }
  return container;
}
//---------------------------------------------------------------------------
String time2Str(int64_t tv)
{
  uint64_t us = uint64_t(tv) % 1000000u;
  time_t tt = (time_t) (uint64_t(tv) / 1000000u);
  struct tm t = *gmtime(&tt);
  uintptr_t l = 10;
  if( t.tm_hour != 0 || t.tm_min != 0 || t.tm_sec != 0 || us != 0 ) l += 9;
  if( us != 0 ) l += 7;
  String::Container * container = String::Container::container(l);
  sprintf(container->string_, "%02u.%02u.%04u", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);
  if( t.tm_hour != 0 || t.tm_min != 0 || t.tm_sec != 0 || us != 0 ){
    sprintf(container->string_ + 10, " %02u:%02u:%02u", t.tm_hour, t.tm_min, t.tm_sec);
    if( us != 0 ) sprintf(container->string_ + 10 + 9, ".%06" PRIu64, us);
  }
  return container;
}
//---------------------------------------------------------------------------
String timeval2Str(const struct timeval & tv)
{
  time_t tt = tv.tv_sec / 1000000u;
  struct tm t = *gmtime(&tt);
  uintptr_t l = 10;
  if( t.tm_hour != 0 || t.tm_min != 0 || t.tm_sec != 0 || tv.tv_usec != 0 ) l += 9;
  if( tv.tv_usec != 0 ) l += 7;
  String::Container * container = String::Container::container(l);
  sprintf(container->string_, "%02u.%02u.%04u", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);
  if( t.tm_hour != 0 || t.tm_min != 0 || t.tm_sec != 0 || tv.tv_usec != 0 ){
    sprintf(container->string_ + 10, " %02u:%02u:%02u", t.tm_hour, t.tm_min, t.tm_sec);
    if( tv.tv_usec != 0 )
      sprintf(container->string_ + 10 + 9, ".%06ld", tv.tv_usec);
  }
  return container;
}
//---------------------------------------------------------------------------
String elapsedTime2Str(int64_t t)
{
  uint64_t
    a = uint64_t(t) / 1000000u,
    days = a / 60 / 60 / 24,
    hours = a / 60 / 60 - days * 24,
    mins = a / 60 - days * 24 * 60 - hours * 60,
    secs = a - days * 24 * 60 * 60 - hours * 60 * 60 - mins * 60,
    msecs = uint64_t(t) % 1000000u;
  String s;
  s.resize(3 + 3 + 3 + 6);
  sprintf(s.c_str(), "%02d:%02d:%02d.%06d", int(hours), int(mins), int(secs), int(msecs));
  String rs(int2Str(days));
  rs += ":";
  rs += s;
  return rs;
}
//---------------------------------------------------------------------------
String float2Str(ldouble a)
{
#if HAVE_SNPRINTF
  int l = snprintf(NULL, 0, "%"PRF_LDBL"g", a);
#elif HAVE__SNPRINTF
  int l = _snprintf(NULL, 0, "%"PRF_LDBL"g", a);
#else
#error Not implemented
#endif
  if( l <= 0 ) l = 24;
  String::Container * container = String::Container::container(l);
  sprintf(container->string_, "%"PRF_LDBL"g", a);
  return container;
}
//---------------------------------------------------------------------------
bool tryStr2Float(const String & str, ldouble & a)
{
  ldouble x;
  bool r = sscanf(str.c_str(),"%"PRF_LDBL"g",&x) == 1;
  if( r ) a = x;
  return r;
}
//---------------------------------------------------------------------------
ldouble str2Float(const String & str)
{
  ldouble a = 0;
  if( !tryStr2Float(str, a) )
    newObjectV1C2<EStr2Scalar>(EINVAL, str + " " + __PRETTY_FUNCTION__)->throwSP();
  return a;
}
//---------------------------------------------------------------------------
char * strstr(const char * string,const char * str)
{
  intptr_t c;
  uintptr_t l0, l1, l2;
  while( *string != '\0' ){
    const char * s1 = string, * s2 = str;
    c = utf82ucs(s1,l0) - utf82ucs(s2,l2);
    l1 = l0;
    for(;;){
      if( *s2 == '\0' ) return (char *) string;
      if( *s1 == '\0' || c != 0 ) break;
      s1 += l1;
      s2 += l2;
      c = utf82ucs(s1,l1) - utf82ucs(s2,l2);
    }
    string += l0;
  }
  return NULL;
}
//---------------------------------------------------------------------------
char * strcasestr(const char * string,const char * str)
{
  intptr_t c;
  uintptr_t l0, l1, l2;
  while( *string != '\0' ){
    const char * s1 = string, * s2 = str;
    c = utf8c2UpperUCS(s1,l0) - utf8c2UpperUCS(s2,l2);
    l1 = l0;
    for(;;){
      if( *s2 == '\0' ) return (char *) string;
      if( *s1 == '\0' || c != 0 ) break;
      s1 += l1;
      s2 += l2;
      c = utf8c2UpperUCS(s1,l1) - utf8c2UpperUCS(s2,l2);
    }
    string += l0;
  }
  return NULL;
}
//---------------------------------------------------------------------------
char * strrstr(const char * string,const char * str)
{
  intptr_t c;
  const char * i = NULL;
  uintptr_t l0, l1, l2;
  while( *string != '\0' ){
    const char * s1 = string, * s2 = str;
    c = utf82ucs(s1,l0) - utf82ucs(s2,l2);
    l1 = l0;
    for(;;){
      if( *s2 == '\0' ){ i = string; break; }
      if( *s1 == '\0' || c != 0 ) break;
      s1 += l1;
      s2 += l2;
      c = utf82ucs(s1,l1) - utf82ucs(s2,l2);
    }
    string += l0;
  }
  return (char *) i;
}
//---------------------------------------------------------------------------
char * strrcasestr(const char * string,const char * str)
{
  intptr_t c;
  const char * i = NULL;
  uintptr_t l0, l1, l2;
  while( *string != '\0' ){
    const char * s1 = string, * s2 = str;
    c = utf8c2UpperUCS(s1,l0) - utf8c2UpperUCS(s2,l2);
    l1 = l0;
    for(;;){
      if( *s2 == '\0' ){ i = string; break; }
      if( *s1 == '\0' || c != 0 ) break;
      s1 += l1;
      s2 += l2;
      c = utf8c2UpperUCS(s1,l1) - utf8c2UpperUCS(s2,l2);
    }
    string += l0;
  }
  return (char *) i;
}
//---------------------------------------------------------------------------
void strncpy0(char * dst,size_t size,const char * src,size_t count)
{
  char * p = dst + size - 1;
  while( size > 0 && count > 0 && (*dst++ = *src++) != '\0' ){
    size--;
    count--;
  }
  while( size > 0 ){
    *dst++ = '\0';
    size--;
  }
  *p = '\0';
}
//---------------------------------------------------------------------------
void strncat0(char * dst,size_t size,const char * src,size_t count)
{
  size_t l = strlen(dst);
  if( l > size ) l = size;
  strncpy0(dst + l,size - l,src,count);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
String::Stream::Stream(const utf8::String & s) : count_(0)
{
  operator << (s);
}
//---------------------------------------------------------------------------
void String::Stream::Format::checkFormat2(const char * v0,const char * v1) const
{
  const char * a = utf8::strrstr(fmt_,v0);
  if( a != NULL && a[::strlen(v0)] == '\0' ) return;
  a = utf8::strrstr(fmt_,v1);
  if( a != NULL && a[::strlen(v1)] == '\0' ) return;
  strncat0(fmt_,sizeof(fmt_),v0);
}
//---------------------------------------------------------------------------
void String::Stream::Format::checkFormat3(const char * v0,const char * v1,const char * v2) const
{
  const char * a = utf8::strrstr(fmt_,v0);
  if( a != NULL && a[::strlen(v0)] == '\0' ) return;
  a = utf8::strrcasestr(fmt_,v1);
  if( a != NULL && a[::strlen(v1)] == '\0' ) return;
  a = utf8::strrcasestr(fmt_,v2);
  if( a != NULL && a[::strlen(v2)] == '\0' ) return;
  strncat0(fmt_,sizeof(fmt_),v0);
}
//---------------------------------------------------------------------------
int String::Stream::Format::print(char * buffer,size_t count) const
{
#if HAVE_SNPRINTF
#define SNPRINTF snprintf
#elif HAVE__SNPRINTF
#define SNPRINTF _snprintf
#else
#error You system not implement snprintf function
#endif
  switch( type_ ){
    case CI :
      checkFormat2("d","x");
      return SNPRINTF(buffer,count,fmt_,ci_);
    case CU :
      checkFormat2("u","x");
      return SNPRINTF(buffer,count,fmt_,cu_);
    case SI :
      checkFormat2("d","x");
      return SNPRINTF(buffer,count,fmt_,si_);
    case SU :
      checkFormat2("u","x");
      return SNPRINTF(buffer,count,fmt_,su_);
    case LI :
      checkFormat2("ld","lx");
      return SNPRINTF(buffer,count,fmt_,li_);
    case LU :
      checkFormat2("lu","lx");
      return SNPRINTF(buffer,count,fmt_,lu_);
    case II :
      checkFormat2("d","x");
      return SNPRINTF(buffer,count,fmt_,ii_);
    case IU :
      checkFormat2("u","x");
      return SNPRINTF(buffer,count,fmt_,iu_);
    case MI :
      checkFormat2(PRIdMAX,PRIxMAX);
      return SNPRINTF(buffer,count,fmt_,mi_);
    case MU :
      checkFormat2(PRIuMAX,PRIxMAX);
      return SNPRINTF(buffer,count,fmt_,mu_);
    case PI :
      checkFormat2(PRIdPTR,PRIxPTR);
      return SNPRINTF(buffer,count,fmt_,pi_);
    case PU :
      checkFormat2(PRIuPTR,PRIxPTR);
      return SNPRINTF(buffer,count,fmt_,pu_);
    case F  :
      checkFormat3("f","e","g");
      return SNPRINTF(buffer,count,fmt_,f_);
    case D  :
      checkFormat3("f","e","g");
      return SNPRINTF(buffer,count,fmt_,d_);
    case LD :
      checkFormat3("lf","le","lg");
      return SNPRINTF(buffer,count,fmt_,ld_);
    case S :
      checkFormat2("s","s");
      return SNPRINTF(buffer,count,fmt_,s_);
    default:
      errno = EINVAL;
  }
#undef SNPRINTF
  return -1;
}
//---------------------------------------------------------------------------
intptr_t String::Stream::Format::format(char * buffer) const
{
  char buf[32], * p = buf;
  int size = -1, size2 = sizeof(buf);
  if( buffer != NULL ){
    p = buffer;
    size2 = INT_MAX;
  }
  ksys::AutoPtr<char,AutoPtrMemoryDestructor> b;
  if( fmt_[0] != '%' ){ errno = EINVAL; goto l1; }
  for(;;){
    errno = 0;
    size = print(p,size2);
    /*if( size > 0 ){
      for( int i = 0; i < size; i++ ) fprintf(stderr,"%c",p[i]);
      fprintf(stderr,"\\n\n");
    }*/
    if( (size == -1 && errno != ERANGE) || (size >= 0 && size + 1 < size2) || buffer != NULL ) break;
    p = b.realloc(size2 <<= 1);
  }
  if( size == -1 ){
l1: int32_t err = errno;
    newObjectV1C2<ksys::Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
//  if( buffer != NULL ) memcpy(buffer,p,size);
  return size;
}
//---------------------------------------------------------------------------
String::Stream & String::Stream::operator << (const char * s)
{
  return *this << String(s);
}
//---------------------------------------------------------------------------
String::Stream & String::Stream::operator << (const wchar_t * s)
{
  return *this << String(s);
}
//---------------------------------------------------------------------------
String::Stream & String::Stream::operator << (const utf8::String & s)
{
  uintptr_t size = s.size();
  stream_.realloc(count_ + size + 1);
  memcpy(stream_.ptr() + count_,s.c_str(),size + 1);
  count_ += size;
  return *this;
}
//---------------------------------------------------------------------------
String::Stream & String::Stream::operator << (const Format & a)
{
  intptr_t size = a.format(NULL);
  stream_.realloc(count_ + size + 1);
  a.format(stream_.ptr() + count_);
  count_ += size;
  stream_[count_] = '\0';
  return *this;
}
//---------------------------------------------------------------------------
utf8::String String::Stream::string()
{
  if( stream_.ptr() == NULL ) return String();
  String::Container * container = newObjectV1V2<String::Container,int,char *,AutoPtrNonVirtualClassDestructor>(0,stream_.ptr());
  stream_.ptr(NULL);
  count_ = 0;
  return container;
}
//---------------------------------------------------------------------------
} // namespace utf8
//---------------------------------------------------------------------------
