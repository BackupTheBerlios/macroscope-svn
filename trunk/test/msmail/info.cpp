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
#include <adicpp/adicpp.h>
#include "msmail.h"
//------------------------------------------------------------------------------
namespace msmail {
//------------------------------------------------------------------------------
extern const char messageIdKey[] = "#MessageId";
extern const char * serverTypeName[] = { "NODE", "STANDALONE" };
extern const char * serverConfSectionName[] = { "node", "standalone" };
//------------------------------------------------------------------------------
Message::~Message()
{
}
//------------------------------------------------------------------------------
Message::Message() :
  attributesAutoDrop_(attributes_), residentSize_(0), codePage_(CP_UTF8)
{
  utf8::String mId(createGUIDAsBase32String());
  file_.fileName(getTempPath() + mId + ".msg");
  attributes_.insert(*newObject<Attribute>(messageIdKey,mId));
}
//------------------------------------------------------------------------------
Message::Message(const utf8::String & mId) :
  attributesAutoDrop_(attributes_), residentSize_(0), codePage_(CP_UTF8)
{
  file_.fileName(getTempPath() + mId + ".msg");
  attributes_.insert(*newObject<Attribute>(messageIdKey,mId));
}
//------------------------------------------------------------------------------
/*Message::Message(const Message & a) : attributesAutoDrop_(attributes_)
{
  operator = (a);
}*/
//------------------------------------------------------------------------------
/*Message & Message::operator = (const Message & a)
{
  attributes_ = a.attributes_;
  residentSize_ = a.residentSize_;
  codePage_ = a.codePage_;
  file_.fileName(a.file_.fileName());
  file_.createIfNotExist(!a.file_.isOpen());
  return *this;
}*/
//------------------------------------------------------------------------------
Message & Message::id(const utf8::String & id)
{
  value(messageIdKey,id);
  return *this;
}
//------------------------------------------------------------------------------
bool Message::isValue(const utf8::String & key) const
{
  return attributes_.find(key) != NULL;
}
//------------------------------------------------------------------------------
utf8::String Message::value(const utf8::String & key,Attribute ** pAttribute) const
{
  Attribute * p = attributes_.find(key);
  if( p == NULL )
    newObject<Exception>(
#if defined(__WIN32__) || defined(__WIN64__)
      ERROR_NOT_FOUND + errorOffset
#else
      ENOENT
#endif
      ,__PRETTY_FUNCTION__
    )->throwSP();
  if( pAttribute != NULL ) *pAttribute = p;
  if( p->index_ == 0 ) return p->value_;
  if( p->size_ == 0 ) return utf8::String();
  file_.open();
  AutoPtr<char> s;
  s.alloc((uintptr_t) p->size_ + 1);
  s[(uintptr_t) p->size_] = '\0';
  file_.readBuffer(p->index_,s,p->size_);
  utf8::String::Container * container = newObject<utf8::String::Container>(0,s.ptr());
  s.ptr(NULL);
  utf8::String ss(container);
  if( key.c_str()[0] != '#' ) ss = unScreenString(ss);
  return ss;
}
//------------------------------------------------------------------------------
Message & Message::value(const utf8::String & key,const utf8::String & value,Attribute ** pAttribute)
{
  uintptr_t keySize = key.size(), valueSize = value.size();
  Attribute * p = attributes_.find(key);
  if( p == NULL ){
    p = newObject<Attribute>(key);
    attributes_.insert(*p);
    residentSize_ += sizeof(Attribute) + keySize + 2;
  }
  if( pAttribute == NULL && residentSize_ - p->value_.size() + valueSize >= getpagesize() * 16u ){
    file_.open();
    file_.seek(file_.size());
    utf8::String v(key), v2(value);
    if( key.strncmp("#",1) != 0 ){
      v = screenString(key);
      v2 = screenString(value);
    }
    v = v + ": index " + utf8::int2Str(p->size_ = v2.size()) + "\n";
    file_.writeBuffer(v.c_str(),v.size());
    p->index_ = file_.tell();
    v2 += "\n";
    file_.writeBuffer(v2.c_str(),p->size_ + 1);
    p->value_ = utf8::String();
  }
  else {
    residentSize_ -= p->value_.size();
    p->value_ = value;
    residentSize_ += valueSize;
  }
  if( pAttribute != NULL ) *pAttribute = p;
  return *this;
}
//------------------------------------------------------------------------------
utf8::String Message::removeValue(const utf8::String & key)
{
  Attribute * p = attributes_.find(key);
  if( p == NULL )
    newObject<Exception>(
#if defined(__WIN32__) || defined(__WIN64__)
      ERROR_NOT_FOUND + errorOffset
#else
      ENOENT
#endif
      ,__PRETTY_FUNCTION__
    )->throwSP();
  utf8::String oldValue(p->value_);
  attributes_.drop(*p);
  residentSize_ -= sizeof(Attribute) + key.size() + oldValue.size() + 2;
  return oldValue;
}
//------------------------------------------------------------------------------
Message & Message::removeValueByLeft(const utf8::String & key)
{
  Array<Attribute *> list;
  attributes_.list(list);
  for( intptr_t l = key.strlen(), i = list.count() - 1; i >= 0; i-- ){
    utf8::String k(list[i]->key_);
    if( k.strncmp(key,l) == 0 ){
      residentSize_ -= sizeof(Attribute) + k.size() + list[i]->value_.size() + 2;
      attributes_.drop(*list[i]);
    }
  }
  return *this;
}
//------------------------------------------------------------------------------
void Message::validateKey(const utf8::String & key)
{
  bool invalid = false;
  if( !key.strstr(": ").eof() ) invalid = true;
  else
  if( !key.strstr(": index ").eof() ) invalid = true;
  else
  if( key.strncmp("#",1) == 0 ){
    utf8::String::Iterator i(key);
    while( i.eof() )
      if( i.isSpace() ){
        invalid = true;
        break;
      }
  }
  if( invalid )
#if defined(__WIN32__) || defined(__WIN64__)
    newObject<Exception>(ERROR_INVALID_DATA + errorOffset,__PRETTY_FUNCTION__)->throwSP();
#else
    newObject<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
#endif
}
//------------------------------------------------------------------------------
static inline intptr_t attributeCompare(const Message::Attribute * const & p1,const Message::Attribute * const & p2)
{
  uintptr_t c1 = utf8::String::Iterator(p1->key_).getChar(), c2 = utf8::String::Iterator(p2->key_).getChar();
  if( c1 == '#' ){
    if( c2 == '#' ) return utf8::String(p1->key_).strcmp(p2->key_);
    return -1;
  }
  if( c2 == '#' ) return 1;
  int64_t a = p1->value_.size() + p1->size_ - p2->value_.size() - p2->size_;
  return a > 0 ? 1 : a < 0 ? -1 : 0;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,Message & a)
{
  uint64_t i;
  s >> i;
  while( i > 0 ){
    i--;
    utf8::String key, value;
    s >> key >> value;
    a.value(key,value);
  }
  return s;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const Message & a)
{
  Array<Message::Attribute *> list;
  a.attributes_.list(list);
  qSort(list.ptr(),0,list.count() - 1,attributeCompare);
  uintptr_t i;
  s << uint64_t(i = list.count());
  for( i = 0; i < list.count(); i++ )
    s << list[i]->key_ << a.value(list[i]->key_);
  return s;
}
//------------------------------------------------------------------------------
AsyncFile & operator >> (AsyncFile & s,Message & a)
{
  AsyncFile::LineGetBuffer buffer(s);
  buffer.removeNewLine_ = true;
  buffer.codePage_ = a.codePage_;

  Message::Attribute * pAttribute;
  utf8::String str, key, value, numberSign("#"), index(": index "), colon(": "), codepage("#codepage");
  bool eof;

  for(;;){
    uint64_t pos = s.tell();
    eof = s.gets(str,&buffer);
    if( eof ) break;
    bool isNumberSign = str.strncmp(numberSign,1) == 0;
    utf8::String::Iterator i(str), ia(i);
    ia.last();
    if( !(i = str.strstr(index)).eof() ){
      if( isNumberSign ){
        key = utf8::String(str,i);
      }
      else {
        key = unScreenString(utf8::String(str,i));
      }
      uint64_t q = utf8::str2Int(utf8::String(i + 8,ia));
      a.value(key,utf8::String(),&pAttribute);
      pAttribute->index_ = s.tell();
      pAttribute->size_ = uintptr_t(q);
      q = s.tell() + pAttribute->size_ + 1;
      buffer.seek(q);
    }
    else if( !(i = str.strstr(colon)).eof() ){
      if( isNumberSign ){
        key = utf8::String(str,i);
        value = utf8::String(i + 2,ia);
        if( str.strcmp(codepage) == 0 ){
          buffer.codePage_ = (uintptr_t) utf8::str2Int(value);
        }
      }
      else {
        key = unScreenString(utf8::String(str,i));
        value = unScreenString(utf8::String(i + 2,ia));
      }
      if( a.residentSize_ + ia.cursor() - i.cursor() - 2 + 1 >= getpagesize() * 16u ){
        a.value(key,utf8::String(),&pAttribute);
        pAttribute->index_ = pos + i.cursor() + 2;
        pAttribute->size_ = ia.cursor() - i.cursor() - 2;
      }
      else {
        a.value(key,value,&pAttribute);
      }
    }
    else {
#if defined(__WIN32__) || defined(__WIN64__)
      newObject<Exception>(ERROR_INVALID_DATA + errorOffset,__PRETTY_FUNCTION__)->throwSP();
#else
      newObject<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
#endif
    }
  }
  a.file_.fileName(s.fileName());
  return s;
}
//------------------------------------------------------------------------------
AsyncFile & operator << (AsyncFile & s,const Message & a)
{
  Array<Message::Attribute *> list;
  a.attributes_.list(list);
  qSort(list.ptr(),0,list.count() - 1,attributeCompare);
  utf8::String numberSign("#");
  size_t bl = getpagesize() * 16;
  AutoPtr<uint8_t> b;
  uintptr_t i, j, k, sz;
  for( k = i = 0; i < list.count(); i++ ){
    bool isNumberSign = utf8::String(list[i]->key_).strncmp(numberSign,1) == 0;
    j = uintptr_t(list[i]->value_.size() + list[i]->size_ + 2 + sizeof(Message::Attribute));
    utf8::String v;
    if( k + j < bl ){
      if( isNumberSign ){
        v = utf8::String(list[i]->key_) + ": " + a.value(list[i]->key_) + "\n";
      }
      else {
        v = screenString(list[i]->key_) + ": " + screenString(a.value(list[i]->key_)) + "\n";
      }
      s.writeBuffer(v.c_str(),v.size());
    }
    else {
      utf8::String v2;
      if( list[i]->index_ == 0 ){
        if( isNumberSign ){
          v2 = list[i]->value_ + "\n";
        }
        else {
          v2 = screenString(list[i]->value_) + "\n";
        }
        sz = v2.size() - 1;
      }
      else {
        sz = list[i]->size_;
      }
      if( isNumberSign ){
        v = utf8::String(list[i]->key_) + ": index " + utf8::int2Str(sz) + "\n";
      }
      else {
        v = screenString(list[i]->key_) + ": index " + utf8::int2Str(sz) + "\n";
      }
      s.writeBuffer(v.c_str(),v.size());
      if( list[i]->index_ == 0 ){
        s.writeBuffer(v2.c_str(),sz + 1);
      }
      else {
        if( b.ptr() == NULL ) b.alloc(bl);
        a.file().open();
        for( uint64_t ll, lp = 0, l = sz + 1; l > 0; l -= ll, lp += ll ){
          ll = l > bl ? bl : l;
          a.file().readBuffer(list[i]->index_ + lp,b,ll);
          s.writeBuffer(b,ll);
        }
      }
    }
    k += j;
  }
  return s;
}
//------------------------------------------------------------------------------
Message & Message::copyUserAttributes(const Message & msg)
{
  utf8::String numberSign("#");
  Array<Attribute *> list;
  msg.attributes_.list(list);
  for( intptr_t i = list.count() - 1; i >= 0; i-- )
    if( utf8::String(list[i]->key_).strncmp(numberSign,1) != 0 )
      value(msg.value(list[i]->key_));
  return *this;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
UserInfo::~UserInfo()
{
}
//------------------------------------------------------------------------------
UserInfo::UserInfo() :
  atime_(gettimeofday()), rtime_(0), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
UserInfo::UserInfo(const utf8::String & name) :
  atime_(gettimeofday()), rtime_(0), name_(name), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
UserInfo::UserInfo(const UserInfo & a) : sendedToAutoDrop_(sendedTo_)
{
  operator = (a);
}
//------------------------------------------------------------------------------
UserInfo & UserInfo::operator = (const UserInfo & a)
{
  atime_ = a.atime_;
  rtime_ = a.rtime_;
  name_ = a.name_;
  Array<InfoLinkKey *> list;
  sendedTo_.drop();
  a.sendedTo_.list(list);
  for( intptr_t i = list.count() - 1; i >= 0; i-- )
    sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,UserInfo & a)
{
  s >> a.name_;
  a.sendedTo_.get(s);
  return s;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const UserInfo & a)
{
  s << a.name_;
  return a.sendedTo_.put(s);
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const UserInfo & a)
{
  return s << a.name_ <<
    ", atime: " << getTimeString(a.atime_) <<
    (a.rtime_ != 0 ? ", rtime: " + getTimeString(a.rtime_) : utf8::String());
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
KeyInfo::~KeyInfo()
{
}
//------------------------------------------------------------------------------
KeyInfo::KeyInfo() :
  atime_(gettimeofday()), rtime_(0), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
KeyInfo::KeyInfo(const utf8::String & name) :
  atime_(gettimeofday()), rtime_(0), name_(name), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
KeyInfo::KeyInfo(const KeyInfo & a) : sendedToAutoDrop_(sendedTo_)
{
  operator = (a);
}
//------------------------------------------------------------------------------
KeyInfo & KeyInfo::operator = (const KeyInfo & a)
{
  atime_ = a.atime_;
  rtime_ = a.rtime_;
  name_ = a.name_;
  Array<InfoLinkKey *> list;
  sendedTo_.drop();
  a.sendedTo_.list(list);
  for( intptr_t i = list.count() - 1; i >= 0; i-- )
    sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,KeyInfo & a)
{
  s >> a.name_ >> a.atime_;
  return a.sendedTo_.get(s);
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const KeyInfo & a)
{
  s << a.name_ << a.atime_;
  return a.sendedTo_.put(s);
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const KeyInfo & a)
{
  return s << a.name_ <<
    ", atime: " << getTimeString(a.atime_) <<
    (a.rtime_ != 0 ? ", rtime: " + getTimeString(a.rtime_) : utf8::String());
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
GroupInfo::~GroupInfo()
{
}
//------------------------------------------------------------------------------
GroupInfo::GroupInfo() :
  atime_(gettimeofday()), rtime_(0), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
GroupInfo::GroupInfo(const utf8::String & name) :
  atime_(gettimeofday()), rtime_(0), name_(name), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
GroupInfo::GroupInfo(const GroupInfo & a) : sendedToAutoDrop_(sendedTo_)
{
  operator = (a);
}
//------------------------------------------------------------------------------
GroupInfo & GroupInfo::operator = (const GroupInfo & a)
{
  atime_ = a.atime_;
  rtime_ = a.rtime_;
  name_ = a.name_;
  Array<InfoLinkKey *> list;
  sendedTo_.drop();
  a.sendedTo_.list(list);
  for( intptr_t i = list.count() - 1; i >= 0; i-- )
    sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,GroupInfo & a)
{
  s >> a.name_ >> a.atime_;
  return a.sendedTo_.get(s);
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const GroupInfo & a)
{
  s << a.name_ << a.atime_;
  return a.sendedTo_.put(s);
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const GroupInfo & a)
{
  return s << a.name_ <<
    ", atime: " << getTimeString(a.atime_) <<
    (a.rtime_ != 0 ? ", rtime: " + getTimeString(a.rtime_) : utf8::String());
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
ServerInfo::~ServerInfo()
{
}
//------------------------------------------------------------------------------
ServerInfo::ServerInfo() :
  atime_(gettimeofday()), rtime_(0), stime_(0),
  type_(stStandalone), sendedToAutoDrop_(sendedTo_),
  connectErrorCount_(0),
  lastFailedConnectTime_(0)
{
}
//------------------------------------------------------------------------------
ServerInfo::ServerInfo(const utf8::String & name,ServerType type) :
  atime_(gettimeofday()), rtime_(0), stime_(0), name_(name),
  type_(type), sendedToAutoDrop_(sendedTo_),
  connectErrorCount_(0),
  lastFailedConnectTime_(0)
{
}
//------------------------------------------------------------------------------
ServerInfo::ServerInfo(const ServerInfo & a) : sendedToAutoDrop_(sendedTo_)
{
  operator = (a);
}
//------------------------------------------------------------------------------
ServerInfo & ServerInfo::operator = (const ServerInfo & a)
{
  atime_ = a.atime_;
  rtime_ = a.rtime_;
  stime_ = a.stime_;
  name_ = a.name_;
  type_ = a.type_;
  Array<InfoLinkKey *> list;
  sendedTo_.drop();
  a.sendedTo_.list(list);
  for( intptr_t i = list.count() - 1; i >= 0; i-- )
    sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
  connectErrorCount_ = a.connectErrorCount_;
  lastFailedConnectTime_ = a.lastFailedConnectTime_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,ServerInfo & a)
{
  uint8_t v;
  s >> a.name_ >> a.atime_ >> v;
  a.sendedTo_.get(s);
  a.type_ = ServerType(v);
  return s;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const ServerInfo & a)
{
  s << a.name_ << a.atime_ << (uint8_t) a.type_;
  return a.sendedTo_.put(s);
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const ServerInfo & a)
{
  return s << a.name_ << " " << serverTypeName[a.type_] <<
    ", atime: " << getTimeString(a.atime_) <<
    (a.rtime_ != 0 ? ", rtime: " + getTimeString(a.rtime_) : utf8::String());
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
User2KeyLink::~User2KeyLink()
{
}
//------------------------------------------------------------------------------
User2KeyLink::User2KeyLink() :
  atime_(gettimeofday()), rtime_(0), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
User2KeyLink::User2KeyLink(const utf8::String & userName,const utf8::String & keyName) :
  atime_(gettimeofday()), rtime_(0), user_(userName), key_(keyName), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
User2KeyLink::User2KeyLink(const User2KeyLink & a) : sendedToAutoDrop_(sendedTo_)
{
  operator = (a);
}
//------------------------------------------------------------------------------
User2KeyLink & User2KeyLink::operator = (const User2KeyLink & a)
{
  atime_ = a.atime_;
  rtime_ = a.rtime_;
  user_ = a.user_;
  key_ = a.key_;
  Array<InfoLinkKey *> list;
  sendedTo_.drop();
  a.sendedTo_.list(list);
  for( intptr_t i = list.count() - 1; i >= 0; i-- )
    sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,User2KeyLink & a)
{
  s >> a.user_ >> a.key_ >> a.atime_;
  return a.sendedTo_.get(s);
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const User2KeyLink & a)
{
  s << a.user_ << a.key_ << a.atime_;
  return a.sendedTo_.put(s);
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const User2KeyLink & a)
{
  return s << a.user_ << " " << a.key_ <<
    ", atime: " << getTimeString(a.atime_) <<
    (a.rtime_ != 0 ? ", rtime: " + getTimeString(a.rtime_) : utf8::String());
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Key2GroupLink::~Key2GroupLink()
{
}
//------------------------------------------------------------------------------
Key2GroupLink::Key2GroupLink() :
  atime_(gettimeofday()), rtime_(0), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
Key2GroupLink::Key2GroupLink(const utf8::String & keyName,const utf8::String & groupName) :
  atime_(gettimeofday()), rtime_(0), key_(keyName), group_(groupName), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
Key2GroupLink::Key2GroupLink(const Key2GroupLink & a) : sendedToAutoDrop_(sendedTo_)
{
  operator = (a);
}
//------------------------------------------------------------------------------
Key2GroupLink & Key2GroupLink::operator = (const Key2GroupLink & a)
{
  atime_ = a.atime_;
  rtime_ = a.rtime_;
  key_ = a.key_;
  group_ = a.group_;
  Array<InfoLinkKey *> list;
  sendedTo_.drop();
  a.sendedTo_.list(list);
  for( intptr_t i = list.count() - 1; i >= 0; i-- )
    sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,Key2GroupLink & a)
{
  s >> a.key_ >> a.group_ >> a.atime_;
  return a.sendedTo_.get(s);
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const Key2GroupLink & a)
{
  s << a.key_ << a.group_ << a.atime_;
  return a.sendedTo_.put(s);
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const Key2GroupLink & a)
{
  return s << a.key_ << " " << a.group_ <<
    ", atime: " << getTimeString(a.atime_) <<
    (a.rtime_ != 0 ? ", rtime: " + getTimeString(a.rtime_) : utf8::String());
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Key2ServerLink::~Key2ServerLink()
{
}
//------------------------------------------------------------------------------
Key2ServerLink::Key2ServerLink() :
  atime_(gettimeofday()), rtime_(0), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
Key2ServerLink::Key2ServerLink(const utf8::String & keyName,const utf8::String & serverName) :
  atime_(gettimeofday()), rtime_(0), key_(keyName), server_(serverName), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
Key2ServerLink::Key2ServerLink(const Key2ServerLink & a) : sendedToAutoDrop_(sendedTo_)
{
  operator = (a);
}
//------------------------------------------------------------------------------
Key2ServerLink & Key2ServerLink::operator = (const Key2ServerLink & a)
{
  atime_ = a.atime_;
  rtime_ = a.rtime_;
  key_ = a.key_;
  server_ = a.server_;
  Array<InfoLinkKey *> list;
  sendedTo_.drop();
  a.sendedTo_.list(list);
  for( intptr_t i = list.count() - 1; i >= 0; i-- )
    sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,Key2ServerLink & a)
{
  s >> a.key_ >> a.server_ >> a.atime_;
  return a.sendedTo_.get(s);
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const Key2ServerLink & a)
{
  s << a.key_ << a.server_ << a.atime_;
  return a.sendedTo_.put(s);
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const Key2ServerLink & a)
{
  return s << a.key_ << " " << a.server_ <<
    ", atime: " << getTimeString(a.atime_) <<
    (a.rtime_ != 0 ? ", rtime: " + getTimeString(a.rtime_) : utf8::String());
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Server::Data::~Data()
{
}
//------------------------------------------------------------------------------
Server::Data::Data() :
  stime_(gettimeofday()),
  usersAutoDrop_(users_),
  keysAutoDrop_(keys_),
  groupsAutoDrop_(groups_),
  serversAutoDrop_(servers_),
  user2KeyLinksAutoDrop_(user2KeyLinks_),
  key2GroupLinksAutoDrop_(key2GroupLinks_),
  key2ServerLinksAutoDrop_(key2ServerLinks_)
{
}
//------------------------------------------------------------------------------
bool Server::Data::registerUserNL(const UserInfo & info,const utf8::String & sendingTo)
{
  bool r = false;
  UserInfo * p = users_.find(info);
  if( p == NULL ){
    if( info.sendedTo_.find(sendingTo) == NULL ){
      users_.insert(*newObject<UserInfo>(info));
      r = true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    if( p->rtime_ != 0 ){
      p->sendedTo_.drop();
      p->rtime_ = 0;
      r = true;
    }
    Array<InfoLinkKey *> list;
    info.sendedTo_.list(list);
    for( intptr_t i = list.count() - 1; i >= 0; i-- )
      p->sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
  }
  return r;
}
//------------------------------------------------------------------------------
bool Server::Data::registerUser(const UserInfo & info,const utf8::String & sendingTo)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerUserNL(info,sendingTo);
}
//------------------------------------------------------------------------------
bool Server::Data::registerKeyNL(const KeyInfo & info,const utf8::String & sendingTo)
{
  bool r = false;
  KeyInfo * p = keys_.find(info);
  if( p == NULL ){
    if( info.sendedTo_.find(sendingTo) == NULL ){
      keys_.insert(*newObject<KeyInfo>(info));
      r = true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    if( p->rtime_ != 0 ){
      p->sendedTo_.drop();
      p->rtime_ = 0;
      r = true;
    }
    Array<InfoLinkKey *> list;
    info.sendedTo_.list(list);
    for( intptr_t i = list.count() - 1; i >= 0; i-- )
      p->sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
  }
  return r;
}
//------------------------------------------------------------------------------
bool Server::Data::registerKey(const KeyInfo & info,const utf8::String & sendingTo)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerKeyNL(info,sendingTo);
}
//------------------------------------------------------------------------------
bool Server::Data::registerGroupNL(const GroupInfo & info,const utf8::String & sendingTo)
{
  bool r = false;
  GroupInfo * p = groups_.find(info);
  if( p == NULL ){
    if( info.sendedTo_.find(sendingTo) == NULL ){
      groups_.insert(*newObject<GroupInfo>(info));
      r = true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    if( p->rtime_ != 0 ){
      p->sendedTo_.drop();
      p->rtime_ = 0;
      r = true;
    }
    Array<InfoLinkKey *> list;
    info.sendedTo_.list(list);
    for( intptr_t i = list.count() - 1; i >= 0; i-- )
      p->sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
  }
  return r;
}
//------------------------------------------------------------------------------
bool Server::Data::registerGroup(const GroupInfo & info,const utf8::String & sendingTo)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerGroupNL(info,sendingTo);
}
//------------------------------------------------------------------------------
bool Server::Data::registerServerNL(const ServerInfo & info,const utf8::String & sendingTo)
{
  bool r = false;
  ServerInfo * p = servers_.find(info);
  if( p == NULL ){
    if( info.sendedTo_.find(sendingTo) == NULL ){
      servers_.insert(*newObject<ServerInfo>(info));
      r = true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    if( p->rtime_ != 0 ){
      p->sendedTo_.drop();
      p->rtime_ = 0;
      r = true;
    }
    Array<InfoLinkKey *> list;
    info.sendedTo_.list(list);
    for( intptr_t i = list.count() - 1; i >= 0; i-- )
      p->sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
    if( info.type_ == stNode && info.type_ != p->type_ ){
      p->type_ = info.type_;
      p->sendedTo_.drop();
      r = true;
    }
  }
  return r;
}
//------------------------------------------------------------------------------
bool Server::Data::registerServer(const ServerInfo & info,const utf8::String & sendingTo)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerServerNL(info,sendingTo);
}
//------------------------------------------------------------------------------
bool Server::Data::registerUser2KeyLinkNL(const User2KeyLink & link,const utf8::String & sendingTo)
{
  bool r = false;
  User2KeyLink * p = user2KeyLinks_.find(link);
  if( p == NULL ){
    if( link.sendedTo_.find(sendingTo) == NULL ){
      user2KeyLinks_.insert(*newObject<User2KeyLink>(link));
      r = true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    if( p->rtime_ != 0 ){
      p->sendedTo_.drop();
      p->rtime_ = 0;
      r = true;
    }
    Array<InfoLinkKey *> list;
    link.sendedTo_.list(list);
    for( intptr_t i = list.count() - 1; i >= 0; i-- )
      p->sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
  }
  return r;
}
//------------------------------------------------------------------------------
bool Server::Data::registerUser2KeyLink(const User2KeyLink & link,const utf8::String & sendingTo)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerUser2KeyLinkNL(link,sendingTo);
}
//------------------------------------------------------------------------------
bool Server::Data::registerKey2GroupLinkNL(const Key2GroupLink & link,const utf8::String & sendingTo)
{
  bool r = false;
  Key2GroupLink * p = key2GroupLinks_.find(link);
  if( p == NULL ){
    if( link.sendedTo_.find(sendingTo) == NULL ){
      key2GroupLinks_.insert(*newObject<Key2GroupLink>(link));
      r = true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    if( p->rtime_ != 0 ){
      p->sendedTo_.drop();
      p->rtime_ = 0;
      r = true;
    }
    Array<InfoLinkKey *> list;
    link.sendedTo_.list(list);
    for( intptr_t i = list.count() - 1; i >= 0; i-- )
      p->sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
  }
  return r;
}
//------------------------------------------------------------------------------
bool Server::Data::registerKey2GroupLink(const Key2GroupLink & link,const utf8::String & sendingTo)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerKey2GroupLinkNL(link,sendingTo);
}
//------------------------------------------------------------------------------
bool Server::Data::registerKey2ServerLinkNL(const Key2ServerLink & link,const utf8::String & sendingTo)
{
  bool r = false;
  Key2ServerLink * p = key2ServerLinks_.find(link);
  if( p == NULL ){
    if( link.sendedTo_.find(sendingTo) == NULL ){
      key2ServerLinks_.insert(*newObject<Key2ServerLink>(link));
      r = true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    if( p->rtime_ != 0 ){
      p->sendedTo_.drop();
      p->rtime_ = 0;
      r = true;
    }
    if( p->server_.strcasecmp(link.server_) != 0 ){
      p->server_ = link.server_;
      p->sendedTo_.drop();
      r = true;
    }
    Array<InfoLinkKey *> list;
    link.sendedTo_.list(list);
    for( intptr_t i = list.count() - 1; i >= 0; i-- )
      p->sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
  }
  return r;
}
//------------------------------------------------------------------------------
bool Server::Data::registerKey2ServerLink(const Key2ServerLink & link,const utf8::String & sendingTo)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerKey2ServerLinkNL(link,sendingTo);
}
//------------------------------------------------------------------------------
void Server::Data::sendDatabaseNL(ksock::AsyncSocket & socket,const utf8::String & sendingTo)
{
  intptr_t i;
  uint64_t u;

  u = 0;
  Array<UserInfo *> userList;
  users_.list(userList);
  for( i = userList.count() - 1; i >= 0; i-- )
    if( userList[i]->rtime_ == 0 && userList[i]->sendedTo_.find(sendingTo) == NULL ) u++;
  socket << u;
  for( i = userList.count() - 1; i >= 0; i-- )
    if( userList[i]->rtime_ == 0 && userList[i]->sendedTo_.find(sendingTo) == NULL ) socket << *userList[i];
  u = 0;
  Array<KeyInfo *> keyList;
  keys_.list(keyList);
  for( i = keyList.count() - 1; i >= 0; i-- )
    if( keyList[i]->rtime_ == 0 && keyList[i]->sendedTo_.find(sendingTo) == NULL ) u++;
  socket << u;
  for( i = keyList.count() - 1; i >= 0; i-- )
    if( keyList[i]->rtime_ == 0 && keyList[i]->sendedTo_.find(sendingTo) == NULL ) socket << *keyList[i];
  u = 0;
  Array<GroupInfo *> groupList;
  groups_.list(groupList);
  for( i = groupList.count() - 1; i >= 0; i-- )
    if( groupList[i]->rtime_ == 0 && groupList[i]->sendedTo_.find(sendingTo) == NULL ) u++;
  socket << u;
  for( i = groupList.count() - 1; i >= 0; i-- )
    if( groupList[i]->rtime_ == 0 && groupList[i]->sendedTo_.find(sendingTo) == NULL ) socket << *groupList[i];
  u = 0;
  Array<ServerInfo *> serverList;
  servers_.list(serverList);
  for( i = serverList.count() - 1; i >= 0; i-- )
    if( serverList[i]->rtime_ == 0 && serverList[i]->sendedTo_.find(sendingTo) == NULL ) u++;
  socket << u;
  for( i = serverList.count() - 1; i >= 0; i-- )
    if( serverList[i]->rtime_ == 0 && serverList[i]->sendedTo_.find(sendingTo) == NULL ) socket << *serverList[i];
  u = 0;
  Array<User2KeyLink *> user2KeyLinkList;
  user2KeyLinks_.list(user2KeyLinkList);
  for( i = user2KeyLinkList.count() - 1; i >= 0; i-- )
    if( user2KeyLinkList[i]->rtime_ == 0 && user2KeyLinkList[i]->sendedTo_.find(sendingTo) == NULL ) u++;
  socket << u;
  for( i = user2KeyLinkList.count() - 1; i >= 0; i-- )
    if( user2KeyLinkList[i]->rtime_ == 0 && user2KeyLinkList[i]->sendedTo_.find(sendingTo) == NULL ) socket << *user2KeyLinkList[i];
  u = 0;
  Array<Key2GroupLink *> key2GroupLinkList;
  key2GroupLinks_.list(key2GroupLinkList);
  for( i = key2GroupLinkList.count() - 1; i >= 0; i-- )
    if( key2GroupLinkList[i]->rtime_ == 0 && key2GroupLinkList[i]->sendedTo_.find(sendingTo) == NULL ) u++;
  socket << u;
  for( i = key2GroupLinkList.count() - 1; i >= 0; i-- )
    if( key2GroupLinkList[i]->rtime_ == 0 && key2GroupLinkList[i]->sendedTo_.find(sendingTo) == NULL ) socket << *key2GroupLinkList[i];
  u = 0;
  Array<Key2ServerLink *> key2ServerLinkList;
  key2ServerLinks_.list(key2ServerLinkList);
  for( i = key2ServerLinkList.count() - 1; i >= 0; i-- )
    if( key2ServerLinkList[i]->rtime_ == 0 && key2ServerLinkList[i]->sendedTo_.find(sendingTo) == NULL ) u++;
  socket << u;
  for( i = key2ServerLinkList.count() - 1; i >= 0; i-- )
    if( key2ServerLinkList[i]->rtime_ == 0 && key2ServerLinkList[i]->sendedTo_.find(sendingTo) == NULL ) socket << *key2ServerLinkList[i];
}
//------------------------------------------------------------------------------
void Server::Data::sendDatabase(ksock::AsyncSocket & socket,const utf8::String & sendingTo)
{
  AutoMutexRDLock<FiberMutex> lock(mutex_);
  sendDatabaseNL(socket,sendingTo);
}
//------------------------------------------------------------------------------
void Server::Data::recvDatabaseNL(ksock::AsyncSocket & socket,const utf8::String & sendingTo)
{
  union {
    intptr_t i;
    uint64_t u;
  };
  socket >> u;
  while( u-- > 0 ){
    UserInfo info;
    socket >> info;
    registerUserNL(info,sendingTo);
  }
  socket >> u;
  while( u-- > 0 ){
    KeyInfo info;
    socket >> info;
    registerKeyNL(info,sendingTo);
  }
  socket >> u;
  while( u-- > 0 ){
    GroupInfo info;
    socket >> info;
    registerGroupNL(info,sendingTo);
  }
  socket >> u;
  while( u-- > 0 ){
    ServerInfo info;
    socket >> info;
    registerServerNL(info,sendingTo);
  }
  socket >> u;
  while( u-- > 0 ){
    User2KeyLink link;
    socket >> link;
    registerUser2KeyLinkNL(link,sendingTo);
  }
  socket >> u;
  while( u-- > 0 ){
    Key2GroupLink link;
    socket >> link;
    registerKey2GroupLinkNL(link,sendingTo);
  }
  socket >> u;
  while( u-- > 0 ){
    Key2ServerLink link;
    socket >> link;
    registerKey2ServerLinkNL(link,sendingTo);
  }
}
//------------------------------------------------------------------------------
void Server::Data::recvDatabase(ksock::AsyncSocket & socket,const utf8::String & sendingTo)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  recvDatabaseNL(socket,sendingTo);
}
//------------------------------------------------------------------------------
utf8::String Server::Data::getNodeListNL() const
{
  utf8::String list;
  Array<ServerInfo *> serverList;
  servers_.list(serverList);
  for( intptr_t i = serverList.count() - 1; i >= 0; i-- ){
    if( !serverList[i]->type_ == stNode ) continue;
    if( list.strlen() > 0 ) list += ", ";
    list += serverList[i]->name_;
  }
  return list;
}
//------------------------------------------------------------------------------
utf8::String Server::Data::getNodeList() const
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return getNodeListNL();
}
//------------------------------------------------------------------------------
void Server::Data::dumpNL(utf8::String::Stream & stream) const
{
  intptr_t i;
  if( users_.count() > 0 ){
    stream << "users:\n";
    Array<UserInfo *> list;
    users_.list(list);
    for( i = list.count() - 1; i >= 0; i-- )
      stream << "  " << *list[i] << "\n";
  }
  if( keys_.count() > 0 ){
    stream << "keys:\n";
    Array<KeyInfo *> list;
    keys_.list(list);
    for( i = list.count() - 1; i >= 0; i-- )
      stream << "  " << *list[i] << "\n";
  }
  if( groups_.count() > 0 ){
    stream << "groups:\n";
    Array<GroupInfo *> list;
    groups_.list(list);
    for( i = list.count() - 1; i >= 0; i-- )
      stream << "  " << *list[i] << "\n";
  }
  if( servers_.count() > 0 ){
    stream << "servers:\n";
    Array<ServerInfo *> list;
    servers_.list(list);
    for( i = list.count() - 1; i >= 0; i-- )
      stream << "  " << *list[i] << "\n";
  }
  if( user2KeyLinks_.count() > 0 ){
    stream << "user2key links:\n";
    Array<User2KeyLink *> list;
    user2KeyLinks_.list(list);
    for( i = list.count() - 1; i >= 0; i-- )
      stream << "  " << *list[i] << "\n";
  }
  if( key2GroupLinks_.count() > 0 ){
    stream << "key2group links:\n";
    Array<Key2GroupLink *> list;
    key2GroupLinks_.list(list);
    for( i = list.count() - 1; i >= 0; i-- )
      stream << "  " << *list[i] << "\n";
  }
  if( key2ServerLinks_.count() > 0 ){
    stream << "key2server links:\n";
    Array<Key2ServerLink *> list;
    key2ServerLinks_.list(list);
    for( i = list.count() - 1; i >= 0; i-- )
      stream << "  " << *list[i] << "\n";
  }
}
//------------------------------------------------------------------------------
void Server::Data::dump(utf8::String::Stream & stream) const
{
  AutoMutexRDLock<FiberMutex> lock(mutex_);
  dumpNL(stream);
}
//------------------------------------------------------------------------------
bool Server::Data::orNL(const Data & a,const utf8::String & sendingTo)
{
  intptr_t i;
  bool r = false;
  Array<UserInfo *> userList;
  a.users_.list(userList);
  for( i = userList.count() - 1; i >= 0; i-- )
    r = registerUserNL(*userList[i],sendingTo) || r;
  Array<KeyInfo *> keyList;
  a.keys_.list(keyList);
  for( i = keyList.count() - 1; i >= 0; i-- )
    r = registerKeyNL(*keyList[i],sendingTo) || r;
  Array<GroupInfo *> groupList;
  a.groups_.list(groupList);
  for( i = groupList.count() - 1; i >= 0; i-- )
    r = registerGroupNL(*groupList[i],sendingTo) || r;
  Array<ServerInfo *> serverList;
  a.servers_.list(serverList);
  for( i = serverList.count() - 1; i >= 0; i-- )
    r = registerServerNL(*serverList[i],sendingTo) || r;
  Array<User2KeyLink *> user2KeyLinkList;
  a.user2KeyLinks_.list(user2KeyLinkList);
  for( i = user2KeyLinkList.count() - 1; i >= 0; i-- )
    r = registerUser2KeyLinkNL(*user2KeyLinkList[i],sendingTo) || r;
  Array<Key2GroupLink *> key2GroupLinkList;
  a.key2GroupLinks_.list(key2GroupLinkList);
  for( i = key2GroupLinkList.count() - 1; i >= 0; i-- )
    r = registerKey2GroupLinkNL(*key2GroupLinkList[i],sendingTo) || r;
  Array<Key2ServerLink *> key2ServerLinkList;
  a.key2ServerLinks_.list(key2ServerLinkList);
  for( i = key2ServerLinkList.count() - 1; i >= 0; i-- )
    r = registerKey2ServerLinkNL(*key2ServerLinkList[i],sendingTo) || r;
  return r;
}
//------------------------------------------------------------------------------
bool Server::Data::ore(const Data & a,const utf8::String & sendingTo)
{
  AutoMutexWRLock<FiberMutex> lock0(mutex_);
  AutoMutexRDLock<FiberMutex> lock1(a.mutex_);
  return orNL(a,sendingTo);
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::xorNL(const Data & data1,const Data & data2,const utf8::String & sendingTo)
{
  intptr_t i;
  Array<UserInfo *> userList;
  data2.users_.list(userList);
  for( i = userList.count() - 1; i >= 0; i-- )
    if( data1.users_.find(*userList[i]) == NULL )
      registerUserNL(*userList[i],sendingTo);
  Array<KeyInfo *> keyList;
  data2.keys_.list(keyList);
  for( i = keyList.count() - 1; i >= 0; i-- )
    if( data1.keys_.find(*keyList[i]) == NULL )
      registerKeyNL(*keyList[i],sendingTo);
  Array<GroupInfo *> groupList;
  data2.groups_.list(groupList);
  for( i = groupList.count() - 1; i >= 0; i-- )
    if( data1.groups_.find(*groupList[i]) == NULL )
      registerGroupNL(*groupList[i],sendingTo);
  Array<ServerInfo *> serverList;
  data2.servers_.list(serverList);
  for( i = serverList.count() - 1; i >= 0; i-- )
    if( data1.servers_.find(*serverList[i]) == NULL )
      registerServerNL(*serverList[i],sendingTo);
  Array<User2KeyLink *> user2KeyLinkList;
  data2.user2KeyLinks_.list(user2KeyLinkList);
  for( i = user2KeyLinkList.count() - 1; i >= 0; i-- )
    if( data1.user2KeyLinks_.find(*user2KeyLinkList[i]) == NULL )
      registerUser2KeyLinkNL(*user2KeyLinkList[i],sendingTo);
  Array<Key2GroupLink *> key2GroupLinkList;
  data2.key2GroupLinks_.list(key2GroupLinkList);
  for( i = key2GroupLinkList.count() - 1; i >= 0; i-- )
    if( data1.key2GroupLinks_.find(*key2GroupLinkList[i]) == NULL )
      registerKey2GroupLinkNL(*key2GroupLinkList[i],sendingTo);
  Array<Key2ServerLink *> key2ServerLinkList;
  data2.key2ServerLinks_.list(key2ServerLinkList);
  for( i = key2ServerLinkList.count() - 1; i >= 0; i-- )
    if( data1.key2ServerLinks_.find(*key2ServerLinkList[i]) == NULL )
      registerKey2ServerLinkNL(*key2ServerLinkList[i],sendingTo);
  return *this;
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::xore(const Data & data1,const Data & data2,const utf8::String & sendingTo)
{
  AutoMutexWRLock<FiberMutex> lock0(mutex_);
  AutoMutexRDLock<FiberMutex> lock1(data1.mutex_);
  AutoMutexRDLock<FiberMutex> lock2(data2.mutex_);
  return xorNL(data1,data2,sendingTo);
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::setSendedToNL(const utf8::String & sendingTo)
{
  intptr_t i;
  Array<UserInfo *> userList;
  users_.list(userList);
  for( i = userList.count() - 1; i >= 0; i-- )
    userList[i]->sendedTo_.insert(*newObject<InfoLinkKey>(sendingTo),false);
  Array<KeyInfo *> keyList;
  keys_.list(keyList);
  for( i = keyList.count() - 1; i >= 0; i-- )
    keyList[i]->sendedTo_.insert(*newObject<InfoLinkKey>(sendingTo),false);
  Array<GroupInfo *> groupList;
  groups_.list(groupList);
  for( i = groupList.count() - 1; i >= 0; i-- )
    groupList[i]->sendedTo_.insert(*newObject<InfoLinkKey>(sendingTo),false);
  Array<ServerInfo *> serverList;
  servers_.list(serverList);
  for( i = serverList.count() - 1; i >= 0; i-- )
    serverList[i]->sendedTo_.insert(*newObject<InfoLinkKey>(sendingTo),false);
  Array<User2KeyLink *> user2KeyLinkList;
  user2KeyLinks_.list(user2KeyLinkList);
  for( i = user2KeyLinkList.count() - 1; i >= 0; i-- )
    user2KeyLinkList[i]->sendedTo_.insert(*newObject<InfoLinkKey>(sendingTo),false);
  Array<Key2GroupLink *> key2GroupLinkList;
  key2GroupLinks_.list(key2GroupLinkList);
  for( i = key2GroupLinkList.count() - 1; i >= 0; i-- )
    key2GroupLinkList[i]->sendedTo_.insert(*newObject<InfoLinkKey>(sendingTo),false);
  Array<Key2ServerLink *> key2ServerLinkList;
  key2ServerLinks_.list(key2ServerLinkList);
  for( i = key2ServerLinkList.count() - 1; i >= 0; i-- )
    key2ServerLinkList[i]->sendedTo_.insert(*newObject<InfoLinkKey>(sendingTo),false);
  return *this;
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::setSendedTo(const utf8::String & sendingTo)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return setSendedToNL(sendingTo);
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::clearSendedToNL()
{
  intptr_t i;
  Array<UserInfo *> userList;
  users_.list(userList);
  for( i = userList.count() - 1; i >= 0; i-- ) userList[i]->sendedTo_.drop();
  Array<KeyInfo *> keyList;
  keys_.list(keyList);
  for( i = keyList.count() - 1; i >= 0; i-- ) keyList[i]->sendedTo_.drop();
  Array<GroupInfo *> groupList;
  groups_.list(groupList);
  for( i = groupList.count() - 1; i >= 0; i-- ) groupList[i]->sendedTo_.drop();
  Array<ServerInfo *> serverList;
  servers_.list(serverList);
  for( i = serverList.count() - 1; i >= 0; i-- ) serverList[i]->sendedTo_.drop();
  Array<User2KeyLink *> user2KeyLinkList;
  user2KeyLinks_.list(user2KeyLinkList);
  for( i = user2KeyLinkList.count() - 1; i >= 0; i-- ) user2KeyLinkList[i]->sendedTo_.drop();
  Array<Key2GroupLink *> key2GroupLinkList;
  key2GroupLinks_.list(key2GroupLinkList);
  for( i = key2GroupLinkList.count() - 1; i >= 0; i-- ) key2GroupLinkList[i]->sendedTo_.drop();
  Array<Key2ServerLink *> key2ServerLinkList;
  key2ServerLinks_.list(key2ServerLinkList);
  for( i = key2ServerLinkList.count() - 1; i >= 0; i-- ) key2ServerLinkList[i]->sendedTo_.drop();
  return *this;
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::clearSendedTo()
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return clearSendedToNL();
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::clearNL()
{
  users_.drop();
  keys_.drop();
  groups_.drop();
  servers_.drop();
  user2KeyLinks_.drop();
  key2GroupLinks_.drop();
  key2ServerLinks_.drop();
  return *this;
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::clear()
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return clearNL();
}
//------------------------------------------------------------------------------
template <typename T,typename LT> static inline
bool sweepT(LT & list,uint64_t stime,uint64_t rtime,utf8::String::Stream * log)
{
  bool r = false;
  Array<T *> ist;
  list.list(ist);
  for( intptr_t i = ist.count() - 1; i >= 0; i-- ){
// проверить если помечен на удаление но произошло обновление после пометки на удаление
// то удалить историю пересылки что бы обновить на узлах для того что бы узлы не удалили
    if( ist[i]->rtime_ != 0 && ist[i]->atime_ >= ist[i]->rtime_ ){
      ist[i]->sendedTo_.drop();
      ist[i]->rtime_ = 0;
    }
// иначе если ещё не помечен на удаление то помечаем
    else if( ist[i]->rtime_ == 0 && ist[i]->atime_ < stime ){
      ist[i]->rtime_ = gettimeofday();
      if( log != NULL ) *log << *ist[i] << ", rtime: " + getTimeString(ist[i]->rtime_) << "\n";
      r = true;
    }
// иначе если уже помечен на удаление то удаляем
    else if( ist[i]->rtime_ != 0 && ist[i]->rtime_ < rtime ){
      if( log != NULL ) *log << *ist[i] << "removed\n";
      list.drop(*ist[i]);
      r = true;
    }
  }
  return r;
}
//------------------------------------------------------------------------------
bool Server::Data::sweepNL(uint64_t stime,uint64_t rtime,utf8::String::Stream * log)
{
  stime = gettimeofday() - stime;
  rtime = gettimeofday() - rtime;
  if( stime_ >= stime ) return false;
  bool r = sweepT<UserInfo,Users>(users_,stime,rtime,log);
  r = sweepT<KeyInfo,Keys>(keys_,stime,rtime,log) || r;
  r = sweepT<GroupInfo,Groups>(groups_,stime,rtime,log) || r;
  r = sweepT<ServerInfo,Servers>(servers_,stime,rtime,log) || r;
  r = sweepT<User2KeyLink,User2KeyLinks>(user2KeyLinks_,stime,rtime,log) || r;
  r = sweepT<Key2GroupLink,Key2GroupLinks>(key2GroupLinks_,stime,rtime,log) || r;
  r = sweepT<Key2ServerLink,Key2ServerLinks>(key2ServerLinks_,stime,rtime,log) || r;
  stime_ = stime;
  return r;
}
//------------------------------------------------------------------------------
bool Server::Data::sweep(uint64_t stime,uint64_t rtime,utf8::String::Stream * log)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return sweepNL(stime,rtime,log);
}
//------------------------------------------------------------------------------
utf8::String Server::Data::getUserListNL(bool quoted) const
{
  utf8::String list;
  Array<UserInfo *> userList;
  users_.list(userList);
  for( intptr_t i = userList.count() - 1; i >= 0; i-- ){
    if( quoted ) list += "\"";
    list += userList[i]->name_;
    if( quoted ) list += "\"";
    if( i > 0 ) list += ",";
  }
  return list;
}
//------------------------------------------------------------------------------
utf8::String Server::Data::getUserList(bool quoted) const
{
  AutoMutexRDLock<FiberMutex> lock(mutex_);
  return getUserListNL(quoted);
}
//------------------------------------------------------------------------------
utf8::String Server::Data::getKeyListNL(bool quoted) const
{
  utf8::String list;
  Array<KeyInfo *> keyList;
  keys_.list(keyList);
  for( intptr_t i = keyList.count() - 1; i >= 0; i-- ){
    if( quoted ) list += "\"";
    list += keyList[i]->name_;
    if( quoted ) list += "\"";
    if( i > 0 ) list += ",";
  }
  return list;
}
//------------------------------------------------------------------------------
utf8::String Server::Data::getKeyList(bool quoted) const
{
  AutoMutexRDLock<FiberMutex> lock(mutex_);
  return getKeyListNL(quoted);
}
//------------------------------------------------------------------------------
utf8::String Server::Data::getKeyGroupListNL(const utf8::String & groups,bool quoted) const
{
  utf8::String list;
  Array<KeyInfo *> keyList;
  keys_.list(keyList);
  for( intptr_t j, i = keyList.count() - 1; i >= 0; i-- ){
    for( j = enumStringParts(groups) - 1; j >= 0; j-- )
      if( key2GroupLinks_.find(Key2GroupLink(keyList[i]->name_,stringPartByNo(groups,j))) != NULL ) break;
    if( j >= 0 ){
      if( list.strlen() > 0 ) list += ",";
      if( quoted ) list += "\"";
      list += keyList[i]->name_;
      if( quoted ) list += "\"";
    }
  }
  return list;
}
//------------------------------------------------------------------------------
utf8::String Server::Data::getKeyGroupList(const utf8::String & groups,bool quoted) const
{
  AutoMutexRDLock<FiberMutex> lock(mutex_);
  return getKeyGroupListNL(groups,quoted);
}
//------------------------------------------------------------------------------
utf8::String Server::Data::getKeyInGroupListNL(const utf8::String & group,bool quoted) const
{
  utf8::String list;
  Array<KeyInfo *> keyList;
  keys_.list(keyList);
  for( intptr_t i = keyList.count() - 1; i >= 0; i-- ){
    if( key2GroupLinks_.find(Key2GroupLink(keyList[i]->name_,group)) == NULL ) continue;
    if( list.strlen() > 0 ) list += ",";
    if( quoted ) list += "\"";
    list += keyList[i]->name_;
    if( quoted ) list += "\"";
  }
  return list;
}
//------------------------------------------------------------------------------
utf8::String Server::Data::getKeyInGroupList(const utf8::String & group,bool quoted) const
{
  AutoMutexRDLock<FiberMutex> lock(mutex_);
  return getKeyInGroupListNL(group,quoted);
}
//------------------------------------------------------------------------------
bool Server::Data::isEmptyNL() const
{
  return
    users_.count() == 0 &&
    keys_.count() == 0 &&
    groups_.count() == 0 &&
    servers_.count() == 0 &&
    user2KeyLinks_.count() == 0 &&
    key2GroupLinks_.count() == 0 &&
    key2ServerLinks_.count() == 0
  ;
}
//------------------------------------------------------------------------------
bool Server::Data::isEmpty() const
{
  AutoMutexRDLock<FiberMutex> lock(mutex_);
  return isEmptyNL();
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
