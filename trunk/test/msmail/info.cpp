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
extern const char * serverTypeName_[] = { "NODE", "STANDALONE" };
extern const char * serverConfSectionName_[] = { "node", "standalone" };
//------------------------------------------------------------------------------
Message::~Message()
{
  attributes_.drop();
}
//------------------------------------------------------------------------------
Message::Message()
{
  UUID uuid;
  createUUID(uuid);
  utf8::String suuid(base32Encode(&uuid,sizeof(uuid)));
  attributes_.insert(*new Attribute(messageIdKey,suuid));
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
const utf8::String & Message::value(const utf8::String & key) const
{
  Attribute * p = attributes_.find(key);
  if( p == NULL )
    throw ExceptionSP(new Exception(
#if defined(__WIN32__) || defined(__WIN64__)
      ERROR_NOT_FOUND + errorOffset
#else
      ENOENT
#endif
      ,__PRETTY_FUNCTION__
    ));
  return p->value_;
}
//------------------------------------------------------------------------------
Message & Message::value(const utf8::String & key,const utf8::String & value)
{
  Attribute * p = attributes_.find(key);
  if( p == NULL ){
    attributes_.insert(*new Attribute(key,value));
  }
  else {
    p->value_ = value;
  }
  return *this;
}
//------------------------------------------------------------------------------
utf8::String Message::removeValue(const utf8::String & key)
{
  Attribute * p = attributes_.find(key);
  if( p == NULL )
    throw ExceptionSP(new Exception(
#if defined(__WIN32__) || defined(__WIN64__)
      ERROR_NOT_FOUND + errorOffset
#else
      ENOENT
#endif
      ,__PRETTY_FUNCTION__
    ));
  utf8::String oldValue(p->value_);
  attributes_.drop(*p);
  return oldValue;
}
//------------------------------------------------------------------------------
Message & Message::removeValueByLeft(const utf8::String & key)
{
  Array<Attribute *> list;
  attributes_.list(list);
  for( intptr_t l = key.strlen(), i = list.count() - 1; i >= 0; i-- )
    if( list[i]->key_.strncmp(key,l) == 0 ) attributes_.drop(*list[i]);
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,Message & a)
{
  uint64_t i;
  utf8::String key, value;
  s >> i;
  while( i > 0 ){
    i--;
    s >> key >> value;
    if( key.strncmp("#",1) == 0 ){
      utf8::String::Iterator i(key);
      while( i.eof() )
        if( i.isSpace() )
#if defined(__WIN32__) || defined(__WIN64__)
          throw ExceptionSP(new Exception(ERROR_INVALID_DATA + errorOffset,__PRETTY_FUNCTION__));
#else
          throw ExceptionSP(new Exception(EINVAL,__PRETTY_FUNCTION__));
#endif
    }
    a.value(key,value);
  }
  return s;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const Message & a)
{
  Array<Message::Attribute *> list;
  a.attributes_.list(list);
  uintptr_t i;
  s << uint64_t(i = list.count());
  while( i > 0 ){
    i--;
    s << list[i]->key_ << list[i]->value_;
  }
  return s;
}
//------------------------------------------------------------------------------
AsyncFile & operator >> (AsyncFile & s,Message & a)
{
  utf8::String str, key, value;
  bool eof;

  for(;;){
    str = s.gets(&eof);
    if( eof ) break;
    utf8::String::Iterator i(str), ia(i);
    ia.last();
    if( (ia - 1).getChar() == '\n' ) ia.prev();
    i = str.strstr(": ");
    if( !i.eof() ){
      if( str.strncmp("#",1) == 0 ){
        key = utf8::String(str,i);
        value = unScreenString(utf8::String(i + 2,ia));
      }
      else {
        key = unScreenString(utf8::String(str,i));
        value = unScreenString(utf8::String(i + 2,ia));
      }
      a.value(key,value);
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
  Array<Message::Attribute *> list;
  a.attributes_.list(list);
  uintptr_t i;
  utf8::String v = a.id();
  for( i = 0; i < list.count(); i++ ){
    if( list[i]->key_.strncmp("#",1) != 0 ) continue;
    v = list[i]->key_ + ": " + unScreenString(list[i]->value_) + "\n";
    s.writeBuffer(v.c_str(),v.size());
  }
  for( i = 0; i < list.count(); i++ ){
    if( list[i]->key_.strncmp("#",1) == 0 ) continue;
    v =
      screenString(list[i]->key_) + ": " +
      screenString(list[i]->value_) + "\n"
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
}
//------------------------------------------------------------------------------
UserInfo::UserInfo() : atime_(gettimeofday())
{
}
//------------------------------------------------------------------------------
UserInfo::UserInfo(const utf8::String & name) : atime_(gettimeofday()), name_(name)
{
}
//------------------------------------------------------------------------------
UserInfo::UserInfo(const UserInfo & a)
{
  operator = (a);
}
//------------------------------------------------------------------------------
UserInfo & UserInfo::operator = (const UserInfo & a)
{
  atime_ = a.atime_;
  name_ = a.name_;
  sendedTo_ = a.sendedTo_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,UserInfo & a)
{
  return s >> a.name_ >> a.sendedTo_;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const UserInfo & a)
{
  return s << a.name_ << a.sendedTo_;
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const UserInfo & a)
{
  return s << a.name_ << ", atime: " << getTimeString(a.atime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
KeyInfo::~KeyInfo()
{
}
//------------------------------------------------------------------------------
KeyInfo::KeyInfo() : atime_(gettimeofday())
{
}
//------------------------------------------------------------------------------
KeyInfo::KeyInfo(const utf8::String & name) : atime_(gettimeofday()), name_(name)
{
}
//------------------------------------------------------------------------------
KeyInfo::KeyInfo(const KeyInfo & a)
{
  operator = (a);
}
//------------------------------------------------------------------------------
KeyInfo & KeyInfo::operator = (const KeyInfo & a)
{
  atime_ = a.atime_;
  name_ = a.name_;
  sendedTo_ = a.sendedTo_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,KeyInfo & a)
{
  return s >> a.name_ >> a.atime_ >> a.sendedTo_;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const KeyInfo & a)
{
  return s << a.name_ << a.atime_ << a.sendedTo_;
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const KeyInfo & a)
{
  return s << a.name_ << ", atime: " << getTimeString(a.atime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
GroupInfo::~GroupInfo()
{
}
//------------------------------------------------------------------------------
GroupInfo::GroupInfo() : atime_(gettimeofday())
{
}
//------------------------------------------------------------------------------
GroupInfo::GroupInfo(const utf8::String & name) : atime_(gettimeofday()), name_(name)
{
}
//------------------------------------------------------------------------------
GroupInfo::GroupInfo(const GroupInfo & a)
{
  operator = (a);
}
//------------------------------------------------------------------------------
GroupInfo & GroupInfo::operator = (const GroupInfo & a)
{
  atime_ = a.atime_;
  name_ = a.name_;
  sendedTo_ = a.sendedTo_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,GroupInfo & a)
{
  return s >> a.name_ >> a.atime_ >> a.sendedTo_;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const GroupInfo & a)
{
  return s << a.name_ << a.atime_ << a.sendedTo_;
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const GroupInfo & a)
{
  return s << a.name_ << ", atime: " << getTimeString(a.atime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
ServerInfo::~ServerInfo()
{
}
//------------------------------------------------------------------------------
ServerInfo::ServerInfo() :
  atime_(gettimeofday()), stime_(0), type_(stStandalone)
{
}
//------------------------------------------------------------------------------
ServerInfo::ServerInfo(const utf8::String & name,ServerType type) :
  atime_(gettimeofday()), stime_(0), name_(name), type_(type)
{
}
//------------------------------------------------------------------------------
ServerInfo::ServerInfo(const ServerInfo & a)
{
  operator = (a);
}
//------------------------------------------------------------------------------
ServerInfo & ServerInfo::operator = (const ServerInfo & a)
{
  atime_ = a.atime_;
  stime_ = a.stime_;
  name_ = a.name_;
  type_ = a.type_;
  sendedTo_ = a.sendedTo_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,ServerInfo & a)
{
  uint8_t v;
  s >> a.name_ >> a.atime_ >> v >> a.sendedTo_;
  a.type_ = ServerType(v);
  return s;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const ServerInfo & a)
{
  return s << a.name_ << a.atime_ << (uint8_t) a.type_ << a.sendedTo_;
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const ServerInfo & a)
{
  return s << a.name_ << " " << serverTypeName_[a.type_] << ", atime: " << getTimeString(a.atime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
User2KeyLink::~User2KeyLink()
{
}
//------------------------------------------------------------------------------
User2KeyLink::User2KeyLink() : atime_(gettimeofday())
{
}
//------------------------------------------------------------------------------
User2KeyLink::User2KeyLink(const utf8::String & userName,const utf8::String & keyName) :
  atime_(gettimeofday()), user_(userName), key_(keyName)
{
}
//------------------------------------------------------------------------------
User2KeyLink::User2KeyLink(const User2KeyLink & a)
{
  operator = (a);
}
//------------------------------------------------------------------------------
User2KeyLink & User2KeyLink::operator = (const User2KeyLink & a)
{
  atime_ = a.atime_;
  user_ = a.user_;
  key_ = a.key_;
  sendedTo_ = a.sendedTo_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,User2KeyLink & a)
{
  return s >> a.user_ >> a.key_ >> a.atime_ >> a.sendedTo_;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const User2KeyLink & a)
{
  return s << a.user_ << a.key_ << a.atime_ << a.sendedTo_;
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const User2KeyLink & a)
{
  return s << a.user_ << " " << a.key_ << ", atime: " << getTimeString(a.atime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Key2GroupLink::~Key2GroupLink()
{
}
//------------------------------------------------------------------------------
Key2GroupLink::Key2GroupLink() : atime_(gettimeofday())
{
}
//------------------------------------------------------------------------------
Key2GroupLink::Key2GroupLink(const utf8::String & keyName,const utf8::String & groupName) :
  atime_(gettimeofday()), key_(keyName), group_(groupName)
{
}
//------------------------------------------------------------------------------
Key2GroupLink::Key2GroupLink(const Key2GroupLink & a)
{
  operator = (a);
}
//------------------------------------------------------------------------------
Key2GroupLink & Key2GroupLink::operator = (const Key2GroupLink & a)
{
  atime_ = a.atime_;
  key_ = a.key_;
  group_ = a.group_;
  sendedTo_ = a.sendedTo_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,Key2GroupLink & a)
{
  return s >> a.key_ >> a.group_ >> a.atime_ >> a.sendedTo_;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const Key2GroupLink & a)
{
  return s << a.key_ << a.group_ << a.atime_ << a.sendedTo_;
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const Key2GroupLink & a)
{
  return s << a.key_ << " " << a.group_ << ", atime: " << getTimeString(a.atime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Key2ServerLink::~Key2ServerLink()
{
}
//------------------------------------------------------------------------------
Key2ServerLink::Key2ServerLink() : atime_(gettimeofday())
{
}
//------------------------------------------------------------------------------
Key2ServerLink::Key2ServerLink(const utf8::String & keyName,const utf8::String & serverName) :
  atime_(gettimeofday()), key_(keyName), server_(serverName)
{
}
//------------------------------------------------------------------------------
Key2ServerLink::Key2ServerLink(const Key2ServerLink & a)
{
  operator = (a);
}
//------------------------------------------------------------------------------
Key2ServerLink & Key2ServerLink::operator = (const Key2ServerLink & a)
{
  atime_ = a.atime_;
  key_ = a.key_;
  server_ = a.server_;
  sendedTo_ = a.sendedTo_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,Key2ServerLink & a)
{
  return s >> a.key_ >> a.server_ >> a.atime_ >> a.sendedTo_;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const Key2ServerLink & a)
{
  return s << a.key_ << a.server_ << a.atime_ << a.sendedTo_;
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const Key2ServerLink & a)
{
  return s << a.key_ << " " << a.server_ << ", atime: " << getTimeString(a.atime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Server::Data::~Data()
{
}
//------------------------------------------------------------------------------
Server::Data::Data() : stime_(gettimeofday())
{
}
//------------------------------------------------------------------------------
bool Server::Data::registerUserNL(const UserInfo & info,const utf8::String & sendingTo)
{
  UserInfo * p = users_.find(info);
  if( p == NULL ){
    if( info.sendedTo_.bSearchCase(sendingTo) < 0 ){
      users_.insert(userList_.safeAdd(p = new UserInfo(info)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    for( intptr_t c, j, i = info.sendedTo_.count() - 1; i >= 0; i-- ){
      j = p->sendedTo_.bSearchCase(info.sendedTo_[i],c);
      if( c != 0 ) p->sendedTo_.insert(j + (c > 0),info.sendedTo_[i]);
    }
  }
  return false;
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
  KeyInfo * p = keys_.find(info);
  if( p == NULL ){
    if( info.sendedTo_.bSearchCase(sendingTo) < 0 ){
      keys_.insert(keyList_.safeAdd(p = new KeyInfo(info)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    for( intptr_t c, j, i = info.sendedTo_.count() - 1; i >= 0; i-- ){
      j = p->sendedTo_.bSearchCase(info.sendedTo_[i],c);
      if( c != 0 ) p->sendedTo_.insert(j + (c > 0),info.sendedTo_[i]);
    }
  }
  return false;
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
  GroupInfo * p = groups_.find(info);
  if( p == NULL ){
    if( info.sendedTo_.bSearchCase(sendingTo) < 0 ){
      groups_.insert(groupList_.safeAdd(p = new GroupInfo(info)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    for( intptr_t c, j, i = info.sendedTo_.count() - 1; i >= 0; i-- ){
      j = p->sendedTo_.bSearchCase(info.sendedTo_[i],c);
      if( c != 0 ) p->sendedTo_.insert(j + (c > 0),info.sendedTo_[i]);
    }
  }
  return false;
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
  ServerInfo * p = servers_.find(info);
  if( p == NULL ){
    if( info.sendedTo_.bSearchCase(sendingTo) < 0 ){
      servers_.insert(serverList_.safeAdd(p = new ServerInfo(info)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    for( intptr_t c, j, i = info.sendedTo_.count() - 1; i >= 0; i-- ){
      j = p->sendedTo_.bSearchCase(info.sendedTo_[i],c);
      if( c != 0 ) p->sendedTo_.insert(j + (c > 0),info.sendedTo_[i]);
    }
    if( info.type_ == stNode && info.type_ != p->type_ ){
      p->type_ = info.type_;
      p->sendedTo_.clear();
      return true;
    }
  }
  return false;
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
  User2KeyLink * p = user2KeyLinks_.find(link);
  if( p == NULL ){
    if( link.sendedTo_.bSearchCase(sendingTo) < 0 ){
      user2KeyLinks_.insert(user2KeyLinkList_.safeAdd(p = new User2KeyLink(link)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    for( intptr_t c, j, i = link.sendedTo_.count() - 1; i >= 0; i-- ){
      j = p->sendedTo_.bSearchCase(link.sendedTo_[i],c);
      if( c != 0 ) p->sendedTo_.insert(j + (c > 0),link.sendedTo_[i]);
    }
  }
  return false;
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
  Key2GroupLink * p = key2GroupLinks_.find(link);
  if( p == NULL ){
    if( link.sendedTo_.bSearchCase(sendingTo) < 0 ){
      key2GroupLinks_.insert(key2GroupLinkList_.safeAdd(p = new Key2GroupLink(link)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    for( intptr_t c, j, i = link.sendedTo_.count() - 1; i >= 0; i-- ){
      j = p->sendedTo_.bSearchCase(link.sendedTo_[i],c);
      if( c != 0 ) p->sendedTo_.insert(j + (c > 0),link.sendedTo_[i]);
    }
  }
  return false;
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
  Key2ServerLink * p = key2ServerLinks_.find(link);
  if( p == NULL ){
    if( link.sendedTo_.bSearchCase(sendingTo) < 0 ){
      key2ServerLinks_.insert(key2ServerLinkList_.safeAdd(p = new Key2ServerLink(link)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    for( intptr_t c, j, i = link.sendedTo_.count() - 1; i >= 0; i-- ){
      j = p->sendedTo_.bSearchCase(link.sendedTo_[i],c);
      if( c != 0 ) p->sendedTo_.insert(j + (c > 0),link.sendedTo_[i]);
    }
    if( p->server_.strcasecmp(link.server_) != 0 ){
      p->server_ = link.server_;
      p->sendedTo_.clear();
      return true;
    }
  }
  return false;
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
  for( i = userList_.count() - 1; i >= 0; i-- )
    if( userList_[i].sendedTo_.bSearchCase(sendingTo) < 0 ) u++;
  socket << u;
  for( i = userList_.count() - 1; i >= 0; i-- )
    if( userList_[i].sendedTo_.bSearchCase(sendingTo) < 0 ) socket << userList_[i];
  u = 0;
  for( i = keyList_.count() - 1; i >= 0; i-- )
    if( keyList_[i].sendedTo_.bSearchCase(sendingTo) < 0 ) u++;
  socket << u;
  for( i = keyList_.count() - 1; i >= 0; i-- )
    if( keyList_[i].sendedTo_.bSearchCase(sendingTo) < 0 ) socket << keyList_[i];
  u = 0;
  for( i = groupList_.count() - 1; i >= 0; i-- )
    if( groupList_[i].sendedTo_.bSearchCase(sendingTo) < 0 ) u++;
  socket << u;
  for( i = groupList_.count() - 1; i >= 0; i-- )
    if( groupList_[i].sendedTo_.bSearchCase(sendingTo) < 0 ) socket << groupList_[i];
  u = 0;
  for( i = serverList_.count() - 1; i >= 0; i-- )
    if( serverList_[i].sendedTo_.bSearchCase(sendingTo) < 0 ) u++;
  socket << u;
  for( i = serverList_.count() - 1; i >= 0; i-- )
    if( serverList_[i].sendedTo_.bSearchCase(sendingTo) < 0 ) socket << serverList_[i];
  u = 0;
  for( i = user2KeyLinkList_.count() - 1; i >= 0; i-- )
    if( user2KeyLinkList_[i].sendedTo_.bSearchCase(sendingTo) < 0 ) u++;
  socket << u;
  for( i = user2KeyLinkList_.count() - 1; i >= 0; i-- )
    if( user2KeyLinkList_[i].sendedTo_.bSearchCase(sendingTo) < 0 ) socket << user2KeyLinkList_[i];
  u = 0;
  for( i = key2GroupLinkList_.count() - 1; i >= 0; i-- )
    if( key2GroupLinkList_[i].sendedTo_.bSearchCase(sendingTo) < 0 ) u++;
  socket << u;
  for( i = key2GroupLinkList_.count() - 1; i >= 0; i-- )
    if( key2GroupLinkList_[i].sendedTo_.bSearchCase(sendingTo) < 0 ) socket << key2GroupLinkList_[i];
  u = 0;
  for( i = key2ServerLinkList_.count() - 1; i >= 0; i-- )
    if( key2ServerLinkList_[i].sendedTo_.bSearchCase(sendingTo) < 0 ) u++;
  socket << u;
  for( i = key2ServerLinkList_.count() - 1; i >= 0; i-- )
    if( key2ServerLinkList_[i].sendedTo_.bSearchCase(sendingTo) < 0 ) socket << key2ServerLinkList_[i];
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
  for( intptr_t i = serverList_.count() - 1; i >= 0; i-- ){
    if( !serverList_[i].type_ == stNode ) continue;
    if( list.strlen() > 0 ) list += ", ";
    list += serverList_[i].name_;
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
  if( userList_.count() > 0 ){
    stream << "users:\n";
    for( i = userList_.count() - 1; i >= 0; i-- )
      stream << "  " << userList_[i] << "\n";
  }
  if( keyList_.count() > 0 ){
    stream << "keys:\n";
    for( i = keyList_.count() - 1; i >= 0; i-- )
      stream << "  " << keyList_[i] << "\n";
  }
  if( groupList_.count() > 0 ){
    stream << "groups:\n";
    for( i = groupList_.count() - 1; i >= 0; i-- )
      stream << "  " << groupList_[i] << "\n";
  }
  if( serverList_.count() > 0 ){
    stream << "servers:\n";
    for( i = serverList_.count() - 1; i >= 0; i-- )
      stream << "  " << serverList_[i] << "\n";
  }
  if( user2KeyLinkList_.count() > 0 ){
    stream << "user2key links:\n";
    for( i = user2KeyLinkList_.count() - 1; i >= 0; i-- )
      stream << "  " << user2KeyLinkList_[i] << "\n";
  }
  if( key2GroupLinkList_.count() > 0 ){
    stream << "key2group links:\n";
    for( i = key2GroupLinkList_.count() - 1; i >= 0; i-- )
      stream << "  " << key2GroupLinkList_[i] << "\n";
  }
  if( key2ServerLinkList_.count() > 0 ){
    stream << "key2server links:\n";
    for( i = key2ServerLinkList_.count() - 1; i >= 0; i-- )
      stream << "  " << key2ServerLinkList_[i] << "\n";
  }
}
//------------------------------------------------------------------------------
void Server::Data::dump(utf8::String::Stream & stream) const
{
  AutoMutexRDLock<FiberMutex> lock(mutex_);
  dumpNL(stream);
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::clear()
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  users_.clear();
  userList_.clear();
  keys_.clear();
  keyList_.clear();
  groups_.clear();
  groupList_.clear();
  servers_.clear();
  serverList_.clear();
  user2KeyLinks_.clear();
  user2KeyLinkList_.clear();
  key2GroupLinks_.clear();
  key2GroupLinkList_.clear();
  key2ServerLinks_.clear();
  key2ServerLinkList_.clear();
  return *this;
}
//------------------------------------------------------------------------------
bool Server::Data::orNL(const Data & a,const utf8::String & sendingTo)
{
  intptr_t i;
  bool r = false;
  for( i = a.userList_.count() - 1; i >= 0; i-- )
    r = registerUserNL(a.userList_[i],sendingTo) || r;
  for( i = a.keyList_.count() - 1; i >= 0; i-- )
    r = registerKeyNL(a.keyList_[i],sendingTo) || r;
  for( i = a.groupList_.count() - 1; i >= 0; i-- )
    r = registerGroupNL(a.groupList_[i],sendingTo) || r;
  for( i = a.serverList_.count() - 1; i >= 0; i-- )
    r = registerServerNL(a.serverList_[i],sendingTo) || r;
  for( i = a.user2KeyLinkList_.count() - 1; i >= 0; i-- )
    r = registerUser2KeyLinkNL(a.user2KeyLinkList_[i],sendingTo) || r;
  for( i = a.key2GroupLinkList_.count() - 1; i >= 0; i-- )
    r = registerKey2GroupLinkNL(a.key2GroupLinkList_[i],sendingTo) || r;
  for( i = a.key2ServerLinkList_.count() - 1; i >= 0; i-- )
    r = registerKey2ServerLinkNL(a.key2ServerLinkList_[i],sendingTo) || r;
  return r;
}
//------------------------------------------------------------------------------
bool Server::Data::or(const Data & a,const utf8::String & sendingTo)
{
  AutoMutexWRLock<FiberMutex> lock0(mutex_);
  AutoMutexRDLock<FiberMutex> lock1(a.mutex_);
  return orNL(a,sendingTo);
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::xorNL(const Data & data1,const Data & data2,const utf8::String & sendingTo)
{
  intptr_t i;
  for( i = data2.userList_.count() - 1; i >= 0; i-- )
    if( data1.users_.find(data2.userList_[i]) == NULL )
      registerUserNL(data2.userList_[i],sendingTo);
  for( i = data2.keyList_.count() - 1; i >= 0; i-- )
    if( data1.keys_.find(data2.keyList_[i]) == NULL )
      registerKeyNL(data2.keyList_[i],sendingTo);
  for( i = data2.groupList_.count() - 1; i >= 0; i-- )
    if( data1.groups_.find(data2.groupList_[i]) == NULL )
      registerGroupNL(data2.groupList_[i],sendingTo);
  for( i = data2.serverList_.count() - 1; i >= 0; i-- )
    if( data1.servers_.find(data2.serverList_[i]) == NULL )
      registerServerNL(data2.serverList_[i],sendingTo);
  for( i = data2.user2KeyLinkList_.count() - 1; i >= 0; i-- )
    if( data1.user2KeyLinks_.find(data2.user2KeyLinkList_[i]) == NULL )
      registerUser2KeyLinkNL(data2.user2KeyLinkList_[i],sendingTo);
  for( i = data2.key2GroupLinkList_.count() - 1; i >= 0; i-- )
    if( data1.key2GroupLinks_.find(data2.key2GroupLinkList_[i]) == NULL )
      registerKey2GroupLinkNL(data2.key2GroupLinkList_[i],sendingTo);
  for( i = data2.key2ServerLinkList_.count() - 1; i >= 0; i-- )
    if( data1.key2ServerLinks_.find(data2.key2ServerLinkList_[i]) == NULL )
      registerKey2ServerLinkNL(data2.key2ServerLinkList_[i],sendingTo);
  return *this;
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::xor(const Data & data1,const Data & data2,const utf8::String & sendingTo)
{
  AutoMutexWRLock<FiberMutex> lock0(mutex_);
  AutoMutexRDLock<FiberMutex> lock1(data1.mutex_);
  AutoMutexRDLock<FiberMutex> lock2(data2.mutex_);
  return xorNL(data1,data2,sendingTo);
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::setSendedToNL(const utf8::String & sendingTo)
{
  intptr_t i, j, c;
  for( i = userList_.count() - 1; i >= 0; i-- ){
    j = userList_[i].sendedTo_.bSearchCase(sendingTo,c);
    if( c != 0 ) userList_[i].sendedTo_.insert(j + (c > 0),sendingTo);
  }
  for( i = keyList_.count() - 1; i >= 0; i-- ){
    j = keyList_[i].sendedTo_.bSearchCase(sendingTo,c);
    if( c != 0 ) keyList_[i].sendedTo_.insert(j + (c > 0),sendingTo);
  }
  for( i = groupList_.count() - 1; i >= 0; i-- ){
    j = groupList_[i].sendedTo_.bSearchCase(sendingTo,c);
    if( c != 0 ) groupList_[i].sendedTo_.insert(j + (c > 0),sendingTo);
  }
  for( i = serverList_.count() - 1; i >= 0; i-- ){
    j = serverList_[i].sendedTo_.bSearchCase(sendingTo,c);
    if( c != 0 ) serverList_[i].sendedTo_.insert(j + (c > 0),sendingTo);
  }
  for( i = user2KeyLinkList_.count() - 1; i >= 0; i-- ){
    j = user2KeyLinkList_[i].sendedTo_.bSearchCase(sendingTo,c);
    if( c != 0 ) user2KeyLinkList_[i].sendedTo_.insert(j + (c > 0),sendingTo);
  }
  for( i = key2GroupLinkList_.count() - 1; i >= 0; i-- ){
    j = key2GroupLinkList_[i].sendedTo_.bSearchCase(sendingTo,c);
    if( c != 0 ) key2GroupLinkList_[i].sendedTo_.insert(j + (c > 0),sendingTo);
  }
  for( i = key2ServerLinkList_.count() - 1; i >= 0; i-- ){
    j = key2ServerLinkList_[i].sendedTo_.bSearchCase(sendingTo,c);
    if( c != 0 ) key2ServerLinkList_[i].sendedTo_.insert(j + (c > 0),sendingTo);
  }
  return *this;
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::setSendedTo(const utf8::String & sendingTo)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return setSendedToNL(sendingTo);
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::setSendedToNL(const Data & data,const utf8::String & sendingTo)
{
  intptr_t i, j, c;
  for( i = data.userList_.count() - 1; i >= 0; i-- )
    if( users_.find(data.userList_[i]) != NULL ){
      j = userList_[i].sendedTo_.bSearchCase(sendingTo,c);
      if( c != 0 ) userList_[i].sendedTo_.insert(j + (c > 0),sendingTo);
    }
  for( i = data.keyList_.count() - 1; i >= 0; i-- )
    if( keys_.find(data.keyList_[i]) != NULL ){
      j = keyList_[i].sendedTo_.bSearchCase(sendingTo,c);
      if( c != 0 ) keyList_[i].sendedTo_.insert(j + (c > 0),sendingTo);
    }
  for( i = data.groupList_.count() - 1; i >= 0; i-- )
    if( groups_.find(data.groupList_[i]) != NULL ){
      j = groupList_[i].sendedTo_.bSearchCase(sendingTo,c);
      if( c != 0 ) groupList_[i].sendedTo_.insert(j + (c > 0),sendingTo);
    }
  for( i = data.serverList_.count() - 1; i >= 0; i-- )
    if( servers_.find(data.serverList_[i]) != NULL ){
      j = serverList_[i].sendedTo_.bSearchCase(sendingTo,c);
      if( c != 0 ) serverList_[i].sendedTo_.insert(j + (c > 0),sendingTo);
    }
  for( i = data.user2KeyLinkList_.count() - 1; i >= 0; i-- )
    if( user2KeyLinks_.find(data.user2KeyLinkList_[i]) != NULL ){
      j = user2KeyLinkList_[i].sendedTo_.bSearchCase(sendingTo,c);
      if( c != 0 ) user2KeyLinkList_[i].sendedTo_.insert(j + (c > 0),sendingTo);
    }
  for( i = data.key2GroupLinkList_.count() - 1; i >= 0; i-- )
    if( key2GroupLinks_.find(data.key2GroupLinkList_[i]) != NULL ){
      j = key2GroupLinkList_[i].sendedTo_.bSearchCase(sendingTo,c);
      if( c != 0 ) key2GroupLinkList_[i].sendedTo_.insert(j + (c > 0),sendingTo);
    }
  for( i = data.key2ServerLinkList_.count() - 1; i >= 0; i-- )
    if( key2ServerLinks_.find(data.key2ServerLinkList_[i]) != NULL ){
      j = key2ServerLinkList_[i].sendedTo_.bSearchCase(sendingTo,c);
      if( c != 0 ) key2ServerLinkList_[i].sendedTo_.insert(j + (c > 0),sendingTo);
    }
  return *this;
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::setSendedTo(const Data & data,const utf8::String & sendingTo)
{
  AutoMutexWRLock<FiberMutex> lock0(mutex_);
  AutoMutexRDLock<FiberMutex> lock1(data.mutex_);
  return setSendedToNL(data,sendingTo);
}
//------------------------------------------------------------------------------
bool Server::Data::sweepNL(uint64_t stime,utf8::String::Stream * log)
{
  if( stime_ >= stime ) return false;
  intptr_t i;
  bool r = false;
  for( i = userList_.count() - 1; i >= 0; i-- ){
    if( userList_[i].atime_ < stime ){
      if( log != NULL ) *log << userList_[i] << "\n";
      users_.remove(userList_[i]);
      userList_.remove(i);
      r = true;
    }
  }
  for( i = keyList_.count() - 1; i >= 0; i-- ){
    if( keyList_[i].atime_ < stime ){
      if( log != NULL ) *log << keyList_[i] << "\n";
      keys_.remove(keyList_[i]);
      keyList_.remove(i);
      r = true;
    }
  }
  for( i = groupList_.count() - 1; i >= 0; i-- ){
    if( groupList_[i].atime_ < stime ){
      if( log != NULL ) *log << groupList_[i] << "\n";
      groups_.remove(groupList_[i]);
      groupList_.remove(i);
      r = true;
    }
  }
  for( i = serverList_.count() - 1; i >= 0; i-- ){
    if( serverList_[i].atime_ < stime ){
      if( log != NULL ) *log << serverList_[i] << "\n";
      servers_.remove(serverList_[i]);
      serverList_.remove(i);
      r = true;
    }
  }
  for( i = user2KeyLinkList_.count() - 1; i >= 0; i-- ){
    if( user2KeyLinkList_[i].atime_ < stime ){
      if( log != NULL ) *log << user2KeyLinkList_[i] << "\n";
      user2KeyLinks_.remove(user2KeyLinkList_[i]);
      user2KeyLinkList_.remove(i);
      r = true;
    }
  }
  for( i = key2GroupLinkList_.count() - 1; i >= 0; i-- ){
    if( key2GroupLinkList_[i].atime_ < stime ){
      if( log != NULL ) *log << key2GroupLinkList_[i] << "\n";
      key2GroupLinks_.remove(key2GroupLinkList_[i]);
      key2GroupLinkList_.remove(i);
      r = true;
    }
  }
  for( i = key2ServerLinkList_.count() - 1; i >= 0; i-- ){
    if( key2ServerLinkList_[i].atime_ < stime ){
      if( log != NULL ) *log << key2ServerLinkList_[i] << "\n";
      key2ServerLinks_.remove(key2ServerLinkList_[i]);
      key2ServerLinkList_.remove(i);
      r = true;
    }
  }
  stime_ = stime;
  return r;
}
//------------------------------------------------------------------------------
bool Server::Data::sweep(uint64_t stime,utf8::String::Stream * log)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return sweepNL(stime,log);
}
//------------------------------------------------------------------------------
utf8::String Server::Data::getUserListNL(bool quoted) const
{
  utf8::String list;
  for( intptr_t i = userList_.count() - 1; i >= 0; i-- ){
    if( quoted ) list += "\"";
    list += userList_[i].name_;
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
  for( intptr_t i = keyList_.count() - 1; i >= 0; i-- ){
    if( quoted ) list += "\"";
    list += keyList_[i].name_;
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
  for( intptr_t j, i = keyList_.count() - 1; i >= 0; i-- ){
    for( j = enumStringParts(groups) - 1; j >= 0; j-- )
      if( key2GroupLinks_.find(Key2GroupLink(keyList_[i].name_,stringPartByNo(groups,j))) != NULL ) break;
    if( j >= 0 ){
      if( list.strlen() > 0 ) list += ",";
      if( quoted ) list += "\"";
      list += keyList_[i].name_;
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
bool Server::Data::isEmptyNL() const
{
  return
    userList_.count() == 0 &&
    keyList_.count() == 0 &&
    groupList_.count() == 0 &&
    serverList_.count() == 0 &&
    user2KeyLinkList_.count() == 0 &&
    key2GroupLinkList_.count() == 0 &&
    key2ServerLinkList_.count() == 0
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
