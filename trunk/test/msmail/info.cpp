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
//------------------------------------------------------------------------------
#include <adicpp/adicpp.h>
#include "msmail.h"
//------------------------------------------------------------------------------
namespace msmail {
//------------------------------------------------------------------------------
extern const char messageIdKey[] = "#MessageId";
//------------------------------------------------------------------------------
Message::~Message()
{
}
//------------------------------------------------------------------------------
Message::Message()
{
}
//------------------------------------------------------------------------------
Message::Message(const utf8::String & sid)
{
  value(messageIdKey,sid);
}
//------------------------------------------------------------------------------
Message::Message(const Message & a) : attributes_(a.attributes_)
{
}
//------------------------------------------------------------------------------
Message & Message::operator = (const Message & a)
{
  attributes_ = a.attributes_;
  return *this;
}
//------------------------------------------------------------------------------
bool Message::isValue(const utf8::String & key) const
{
  return attributes_.bSearch(Attribute(key,utf8::String())) >= 0;
}
//------------------------------------------------------------------------------
const utf8::String & Message::value(const utf8::String & key) const
{
  if( attributes_.count() == 0 ){
    UUID uuid;
    createUUID(uuid);
    utf8::String suuid(base32Encode(&uuid,sizeof(uuid)));
    attributes_.add(Attribute(messageIdKey,suuid));
  }
  intptr_t i = attributes_.bSearch(Attribute(key,utf8::String()));
  if( i < 0 )
    throw ExceptionSP(new Exception(
#if defined(__WIN32__) || defined(__WIN64__)
      ERROR_NOT_FOUND + errorOffset
#else
      ENOENT
#endif
      ,__PRETTY_FUNCTION__
    ));
  return attributes_[i].value_;
}
//------------------------------------------------------------------------------
Message & Message::value(const utf8::String & key,const utf8::String & value)
{
  intptr_t i, c;
  if( attributes_.count() == 0 && key.strcmp(messageIdKey) == 0 && value.strlen() == 0 ){
    UUID uuid;
    createUUID(uuid);
    utf8::String suuid(base32Encode(&uuid,sizeof(uuid)));
    i = attributes_.bSearch(Attribute(messageIdKey,utf8::String()),c);
    attributes_.add(Attribute(messageIdKey,suuid));
  }
  i = attributes_.bSearch(Attribute(key,utf8::String()),c);
  if( c != 0 ){
    attributes_.insert(i + (c > 0),Attribute(key,value));
  }
  else {
    attributes_[i].value_ = value;
  }
  return *this;
}
//------------------------------------------------------------------------------
utf8::String Message::separateValue(const utf8::String & key,utf8::String & s0,utf8::String & s1,const utf8::String & separator) const
{
  utf8::String value(value(key));
  utf8::String::Iterator i(value.strcasestr(separator));
  s0 = utf8::String(value,i);
  s1 = i + 1;
  return value;
}
//---------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,Message & a)
{
  uint64_t i;
  utf8::String key, value;
  s >> i;
  while( i > 0 ){
    i--;
    s >> key >> value;
    a.value(key,value);
  }
  return s;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const Message & a)
{
  uintptr_t i;
  s << uint64_t(i = a.attributes().count());
  while( i > 0 ){
    i--;
    s << a.attributes()[i].key_ << a.attributes()[i].value_;
  }
  return s;
}
//------------------------------------------------------------------------------
AsyncFile & operator >> (AsyncFile & s,Message & a)
{
  utf8::String key, value;
  bool eof;

  for(;;){
    key = s.gets(&eof);
    if( eof ) break;
    utf8::String::Iterator i(key);
    i.last().prev();
    if( i.getChar() == '\n' ) key.remove(i);
    i = key;
    i.last().prev();
    if( i.getChar() == '\r' ) key.remove(i);
    i = key.strstr(": ");
    if( !i.eof() ){
      a.value(
        /*unScreenString(*/utf8::String(key,i)/*)*/,
        /*unScreenString(*/i + 2/*)*/
      );
    }
    else if( !(i = key.strstr(" -#=#- ")).eof() ){
      a.value(
        unScreenString(utf8::String(key,i)),
        unScreenString(i + 7)
      );
    }
    else {
#if defined(__WIN32__) || defined(__WIN64__)
      throw ExceptionSP(new Exception(ERROR_INVALID_DATA + errorOffset,__PRETTY_FUNCTION__));
#else
      throw ExceptionSP(new Exception(EINVAL,__PRETTY_FUNCTION__));
#endif
    }
  }
  return s;
}
//------------------------------------------------------------------------------
AsyncFile & operator << (AsyncFile & s,const Message & a)
{
  uintptr_t i;
  utf8::String v = a.id();
  for( i = 0; i < a.attributes().count(); i++ ){
    if( a.attributes()[i].key_.strncmp("#",1) != 0 ) continue;
    v =
      /*screenString(*/a.attributes()[i].key_/*)*/ + ": " +
      /*screenString(*/a.attributes()[i].value_/*)*/ + "\n"
    ;
    s.writeBuffer(v.c_str(),v.size());
  }
  for( i = 0; i < a.attributes().count(); i++ ){
    if( a.attributes()[i].key_.strncmp("#",1) == 0 ) continue;
    v =
      screenString(a.attributes()[i].key_) + " -#=#- " +
      screenString(a.attributes()[i].value_) + "\n"
    ;
    s.writeBuffer(v.c_str(),v.size());
  }
  return s;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
UserInfo::~UserInfo()
{
  assert( refCount_ == 0 );
}
//------------------------------------------------------------------------------
UserInfo::UserInfo() : refCount_(0), atime_(0), mtime_(0)
{
}
//------------------------------------------------------------------------------
UserInfo::UserInfo(const utf8::String & name) : refCount_(0), atime_(0), mtime_(0), name_(name)
{
}
//------------------------------------------------------------------------------
UserInfo::UserInfo(const UserInfo & a) : atime_(a.atime_), mtime_(a.mtime_), name_(a.name_)
{
}
//------------------------------------------------------------------------------
UserInfo & UserInfo::operator = (const UserInfo & a)
{
  atime_ = a.atime_;
  mtime_ = a.mtime_;
  name_ = a.name_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,UserInfo & a)
{
  return s >> a.atime_ >> a.mtime_ >> a.name_;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const UserInfo & a)
{
  return s << a.atime_ << a.mtime_ << a.name_;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
KeyInfo::~KeyInfo()
{
  assert( refCount_ == 0 );
}
//------------------------------------------------------------------------------
KeyInfo::KeyInfo() : refCount_(0), atime_(0), mtime_(0)
{
}
//------------------------------------------------------------------------------
KeyInfo::KeyInfo(const KeyInfo & a) : atime_(a.atime_), mtime_(a.mtime_), key_(a.key_), users_(a.users_)
{
}
//------------------------------------------------------------------------------
KeyInfo::KeyInfo(const utf8::String & key) : refCount_(0), atime_(0), mtime_(0), key_(key)
{
}
//------------------------------------------------------------------------------
KeyInfo & KeyInfo::operator = (const KeyInfo & a)
{
  atime_ = a.atime_;
  mtime_ = a.mtime_;
  key_ = a.key_;
  users_ = a.users_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,KeyInfo & a)
{
  return s >> a.atime_ >> a.mtime_ >> a.key_;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const KeyInfo & a)
{
  return s << a.atime_ << a.mtime_ << a.key_;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
GroupInfo::~GroupInfo()
{
  assert( refCount_ == 0 );
}
//------------------------------------------------------------------------------
GroupInfo::GroupInfo() : refCount_(0), atime_(0), mtime_(0)
{
}
//------------------------------------------------------------------------------
GroupInfo::GroupInfo(const utf8::String & name) : refCount_(0), atime_(0), mtime_(0), name_(name)
{
}
//------------------------------------------------------------------------------
GroupInfo::GroupInfo(const GroupInfo & a) : atime_(a.atime_), mtime_(a.mtime_), name_(a.name_), keys_(keys_)
{
}
//------------------------------------------------------------------------------
GroupInfo & GroupInfo::operator = (const GroupInfo & a)
{
  atime_ = a.atime_;
  mtime_ = a.mtime_;
  name_ = a.name_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,GroupInfo & a)
{
  return s >> a.atime_ >> a.mtime_ >> a.name_;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const GroupInfo & a)
{
  return s << a.atime_ << a.mtime_ << a.name_;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
ServerInfo::~ServerInfo()
{
  assert( refCount_ == 0 );
}
//------------------------------------------------------------------------------
ServerInfo::ServerInfo() : refCount_(0), atime_(0), mtime_(0), node_(false)
{
}
//------------------------------------------------------------------------------
ServerInfo::ServerInfo(const ServerInfo & a) : atime_(a.atime_), mtime_(a.mtime_), name_(a.name_), node_(a.node_)
{
}
//------------------------------------------------------------------------------
ServerInfo & ServerInfo::operator = (const ServerInfo & a)
{
  atime_ = a.atime_;
  mtime_ = a.mtime_;
  name_ = a.name_;
  node_ = a.node_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,ServerInfo & a)
{
  return s >> a.atime_ >> a.mtime_ >> a.name_;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const ServerInfo & a)
{
  return s << a.atime_ << a.mtime_ << a.name_;
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
