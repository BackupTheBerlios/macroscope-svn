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
}
//------------------------------------------------------------------------------
Message::Message() : attributesAutoDrop_(attributes_)
{
  ksys::UUID uuid;
  createUUID(uuid);
  utf8::String suuid(base32Encode(&uuid,sizeof(uuid)));
  attributes_.insert(*newObject<Attribute>(messageIdKey,suuid));
}
//------------------------------------------------------------------------------
Message::Message(const utf8::String & sid) : attributesAutoDrop_(attributes_)
{
  value(messageIdKey,sid);
}
//------------------------------------------------------------------------------
Message::Message(const Message & a) : attributes_(a.attributes_), attributesAutoDrop_(attributes_)
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
    Exception::throwSP(
#if defined(__WIN32__) || defined(__WIN64__)
      ERROR_NOT_FOUND + errorOffset
#else
      ENOENT
#endif
      ,__PRETTY_FUNCTION__
    );
  return p->value_;
}
//------------------------------------------------------------------------------
Message & Message::value(const utf8::String & key,const utf8::String & value)
{
  Attribute * p = attributes_.find(key);
  if( p == NULL ){
    attributes_.insert(*newObject<Attribute>(key,value));
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
    Exception::throwSP(
#if defined(__WIN32__) || defined(__WIN64__)
      ERROR_NOT_FOUND + errorOffset
#else
      ENOENT
#endif
      ,__PRETTY_FUNCTION__
    );
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
    if( utf8::String(list[i]->key_).strncmp(key,l) == 0 ) attributes_.drop(*list[i]);
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
          Exception::throwSP(ERROR_INVALID_DATA + errorOffset,__PRETTY_FUNCTION__);
#else
          Exception::throwSP(EINVAL,__PRETTY_FUNCTION__);
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
    s << utf8::String(list[i]->key_) << list[i]->value_;
  }
  return s;
}
//------------------------------------------------------------------------------
AsyncFile & operator >> (AsyncFile & s,Message & a)
{
  AsyncFile::LineGetBuffer buffer;
  buffer.removeNewLine_ = true;
  uint64_t lastHeaderPos;

  utf8::String str, key, value;
  bool eof, header = false;

  for(;;){
    lastHeaderPos = s.tell();
    eof = s.gets(str,&buffer);
    if( eof ) break;
    utf8::String::Iterator i(str), ia(i);
    ia.last();
    if( (ia - 1).getChar() == '\n' ) ia.prev();
    i = str.strstr(": ");
    if( !i.eof() ){
      if( str.strncmp("#",1) == 0 ){
        header = true;
        key = utf8::String(str,i);
        value = unScreenString(utf8::String(i + 2,ia));
      }
      else {
        if( header ){
          s.seek(lastHeaderPos);
          break;
        }
        key = unScreenString(utf8::String(str,i));
        value = unScreenString(utf8::String(i + 2,ia));
      }
      a.value(key,value);
    }
    else {
#if defined(__WIN32__) || defined(__WIN64__)
      Exception::throwSP(ERROR_INVALID_DATA + errorOffset,__PRETTY_FUNCTION__);
#else
      Exception::throwSP(EINVAL,__PRETTY_FUNCTION__);
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
    if( utf8::String(list[i]->key_).strncmp("#",1) != 0 ) continue;
    v = utf8::String(list[i]->key_) + ": " + unScreenString(list[i]->value_) + "\n";
    s.writeBuffer(v.c_str(),v.size());
  }
  for( i = 0; i < list.count(); i++ ){
    if( utf8::String(list[i]->key_).strncmp("#",1) == 0 ) continue;
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
UserInfo::UserInfo() :
  atime_(gettimeofday()), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
UserInfo::UserInfo(const utf8::String & name) :
  atime_(gettimeofday()), name_(name), sendedToAutoDrop_(sendedTo_)
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
  name_ = a.name_;
  sendedTo_ = a.sendedTo_;
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
  return s << a.name_ << ", atime: " << getTimeString(a.atime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
KeyInfo::~KeyInfo()
{
}
//------------------------------------------------------------------------------
KeyInfo::KeyInfo() :
  atime_(gettimeofday()), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
KeyInfo::KeyInfo(const utf8::String & name) :
  atime_(gettimeofday()), name_(name), sendedToAutoDrop_(sendedTo_)
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
  name_ = a.name_;
  sendedTo_ = a.sendedTo_;
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
  return s << a.name_ << ", atime: " << getTimeString(a.atime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
GroupInfo::~GroupInfo()
{
}
//------------------------------------------------------------------------------
GroupInfo::GroupInfo() :
  atime_(gettimeofday()), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
GroupInfo::GroupInfo(const utf8::String & name) :
  atime_(gettimeofday()), name_(name), sendedToAutoDrop_(sendedTo_)
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
  name_ = a.name_;
  sendedTo_ = a.sendedTo_;
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
  atime_(gettimeofday()), stime_(0), type_(stStandalone), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
ServerInfo::ServerInfo(const utf8::String & name,ServerType type) :
  atime_(gettimeofday()), stime_(0), name_(name), type_(type), sendedToAutoDrop_(sendedTo_)
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
  return s << a.name_ << " " << serverTypeName_[a.type_] << ", atime: " << getTimeString(a.atime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
User2KeyLink::~User2KeyLink()
{
}
//------------------------------------------------------------------------------
User2KeyLink::User2KeyLink() :
  atime_(gettimeofday()), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
User2KeyLink::User2KeyLink(const utf8::String & userName,const utf8::String & keyName) :
  atime_(gettimeofday()), user_(userName), key_(keyName), sendedToAutoDrop_(sendedTo_)
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
  user_ = a.user_;
  key_ = a.key_;
  sendedTo_ = a.sendedTo_;
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
  return s << a.user_ << " " << a.key_ << ", atime: " << getTimeString(a.atime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Key2GroupLink::~Key2GroupLink()
{
}
//------------------------------------------------------------------------------
Key2GroupLink::Key2GroupLink() :
  atime_(gettimeofday()), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
Key2GroupLink::Key2GroupLink(const utf8::String & keyName,const utf8::String & groupName) :
  atime_(gettimeofday()), key_(keyName), group_(groupName), sendedToAutoDrop_(sendedTo_)
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
  key_ = a.key_;
  group_ = a.group_;
  sendedTo_ = a.sendedTo_;
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
  return s << a.key_ << " " << a.group_ << ", atime: " << getTimeString(a.atime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Key2ServerLink::~Key2ServerLink()
{
}
//------------------------------------------------------------------------------
Key2ServerLink::Key2ServerLink() :
  atime_(gettimeofday()), sendedToAutoDrop_(sendedTo_)
{
}
//------------------------------------------------------------------------------
Key2ServerLink::Key2ServerLink(const utf8::String & keyName,const utf8::String & serverName) :
  atime_(gettimeofday()), key_(keyName), server_(serverName), sendedToAutoDrop_(sendedTo_)
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
  key_ = a.key_;
  server_ = a.server_;
  sendedTo_ = a.sendedTo_;
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
  return s << a.key_ << " " << a.server_ << ", atime: " << getTimeString(a.atime_);
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
  UserInfo * p = users_.find(info);
  if( p == NULL ){
    if( info.sendedTo_.find(sendingTo) == NULL ){
      users_.insert(*newObject<UserInfo>(info));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    Array<InfoLinkKey *> list;
    info.sendedTo_.list(list);
    for( intptr_t i = list.count() - 1; i >= 0; i-- )
      p->sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
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
    if( info.sendedTo_.find(sendingTo) == NULL ){
      keys_.insert(*newObject<KeyInfo>(info));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    Array<InfoLinkKey *> list;
    info.sendedTo_.list(list);
    for( intptr_t i = list.count() - 1; i >= 0; i-- )
      p->sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
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
    if( info.sendedTo_.find(sendingTo) == NULL ){
      groups_.insert(*newObject<GroupInfo>(info));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    Array<InfoLinkKey *> list;
    info.sendedTo_.list(list);
    for( intptr_t i = list.count() - 1; i >= 0; i-- )
      p->sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
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
    if( info.sendedTo_.find(sendingTo) == NULL ){
      servers_.insert(*newObject<ServerInfo>(info));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    Array<InfoLinkKey *> list;
    info.sendedTo_.list(list);
    for( intptr_t i = list.count() - 1; i >= 0; i-- )
      p->sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
    if( info.type_ == stNode && info.type_ != p->type_ ){
      p->type_ = info.type_;
      p->sendedTo_.drop();
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
    if( link.sendedTo_.find(sendingTo) == NULL ){
      user2KeyLinks_.insert(*newObject<User2KeyLink>(link));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    Array<InfoLinkKey *> list;
    link.sendedTo_.list(list);
    for( intptr_t i = list.count() - 1; i >= 0; i-- )
      p->sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
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
    if( link.sendedTo_.find(sendingTo) == NULL ){
      key2GroupLinks_.insert(*newObject<Key2GroupLink>(link));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    Array<InfoLinkKey *> list;
    link.sendedTo_.list(list);
    for( intptr_t i = list.count() - 1; i >= 0; i-- )
      p->sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
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
    if( link.sendedTo_.find(sendingTo) == NULL ){
      key2ServerLinks_.insert(*newObject<Key2ServerLink>(link));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    Array<InfoLinkKey *> list;
    link.sendedTo_.list(list);
    for( intptr_t i = list.count() - 1; i >= 0; i-- )
      p->sendedTo_.insert(*newObject<InfoLinkKey>(*list[i]),false);
    if( p->server_.strcasecmp(link.server_) != 0 ){
      p->server_ = link.server_;
      p->sendedTo_.drop();
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
  Array<UserInfo *> userList;
  users_.list(userList);
  for( i = userList.count() - 1; i >= 0; i-- )
    if( userList[i]->sendedTo_.find(sendingTo) == NULL ) u++;
  socket << u;
  for( i = userList.count() - 1; i >= 0; i-- )
    if( userList[i]->sendedTo_.find(sendingTo) == NULL ) socket << *userList[i];
  u = 0;
  Array<KeyInfo *> keyList;
  keys_.list(keyList);
  for( i = keyList.count() - 1; i >= 0; i-- )
    if( keyList[i]->sendedTo_.find(sendingTo) == NULL ) u++;
  socket << u;
  for( i = keyList.count() - 1; i >= 0; i-- )
    if( keyList[i]->sendedTo_.find(sendingTo) == NULL ) socket << *keyList[i];
  u = 0;
  Array<GroupInfo *> groupList;
  groups_.list(groupList);
  for( i = groupList.count() - 1; i >= 0; i-- )
    if( groupList[i]->sendedTo_.find(sendingTo) == NULL ) u++;
  socket << u;
  for( i = groupList.count() - 1; i >= 0; i-- )
    if( groupList[i]->sendedTo_.find(sendingTo) == NULL ) socket << *groupList[i];
  u = 0;
  Array<ServerInfo *> serverList;
  servers_.list(serverList);
  for( i = serverList.count() - 1; i >= 0; i-- )
    if( serverList[i]->sendedTo_.find(sendingTo) == NULL ) u++;
  socket << u;
  for( i = serverList.count() - 1; i >= 0; i-- )
    if( serverList[i]->sendedTo_.find(sendingTo) == NULL ) socket << *serverList[i];
  u = 0;
  Array<User2KeyLink *> user2KeyLinkList;
  user2KeyLinks_.list(user2KeyLinkList);
  for( i = user2KeyLinkList.count() - 1; i >= 0; i-- )
    if( user2KeyLinkList[i]->sendedTo_.find(sendingTo) == NULL ) u++;
  socket << u;
  for( i = user2KeyLinkList.count() - 1; i >= 0; i-- )
    if( user2KeyLinkList[i]->sendedTo_.find(sendingTo) == NULL ) socket << *user2KeyLinkList[i];
  u = 0;
  Array<Key2GroupLink *> key2GroupLinkList;
  key2GroupLinks_.list(key2GroupLinkList);
  for( i = key2GroupLinkList.count() - 1; i >= 0; i-- )
    if( key2GroupLinkList[i]->sendedTo_.find(sendingTo) == NULL ) u++;
  socket << u;
  for( i = key2GroupLinkList.count() - 1; i >= 0; i-- )
    if( key2GroupLinkList[i]->sendedTo_.find(sendingTo) == NULL ) socket << *key2GroupLinkList[i];
  u = 0;
  Array<Key2ServerLink *> key2ServerLinkList;
  key2ServerLinks_.list(key2ServerLinkList);
  for( i = key2ServerLinkList.count() - 1; i >= 0; i-- )
    if( key2ServerLinkList[i]->sendedTo_.find(sendingTo) == NULL ) u++;
  socket << u;
  for( i = key2ServerLinkList.count() - 1; i >= 0; i-- )
    if( key2ServerLinkList[i]->sendedTo_.find(sendingTo) == NULL ) socket << *key2ServerLinkList[i];
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
Server::Data & Server::Data::clear()
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
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
bool Server::Data::sweepNL(uint64_t stime,utf8::String::Stream * log)
{
  if( stime_ >= stime ) return false;
  intptr_t i;
  bool r = false;
  Array<UserInfo *> userList;
  users_.list(userList);
  for( i = userList.count() - 1; i >= 0; i-- ){
    if( userList[i]->atime_ < stime ){
      if( log != NULL ) *log << *userList[i] << "\n";
      users_.drop(*userList[i]);
      r = true;
    }
  }
  Array<KeyInfo *> keyList;
  keys_.list(keyList);
  for( i = keyList.count() - 1; i >= 0; i-- ){
    if( keyList[i]->atime_ < stime ){
      if( log != NULL ) *log << *keyList[i] << "\n";
      keys_.drop(*keyList[i]);
      r = true;
    }
  }
  Array<GroupInfo *> groupList;
  groups_.list(groupList);
  for( i = groupList.count() - 1; i >= 0; i-- ){
    if( groupList[i]->atime_ < stime ){
      if( log != NULL ) *log << *groupList[i] << "\n";
      groups_.drop(*groupList[i]);
      r = true;
    }
  }
  Array<ServerInfo *> serverList;
  servers_.list(serverList);
  for( i = serverList.count() - 1; i >= 0; i-- ){
    if( serverList[i]->atime_ < stime ){
      if( log != NULL ) *log << *serverList[i] << "\n";
      servers_.drop(*serverList[i]);
      r = true;
    }
  }
  Array<User2KeyLink *> user2KeyLinkList;
  user2KeyLinks_.list(user2KeyLinkList);
  for( i = user2KeyLinkList.count() - 1; i >= 0; i-- ){
    if( user2KeyLinkList[i]->atime_ < stime ){
      if( log != NULL ) *log << *user2KeyLinkList[i] << "\n";
      user2KeyLinks_.drop(*user2KeyLinkList[i]);
      r = true;
    }
  }
  Array<Key2GroupLink *> key2GroupLinkList;
  key2GroupLinks_.list(key2GroupLinkList);
  for( i = key2GroupLinkList.count() - 1; i >= 0; i-- ){
    if( key2GroupLinkList[i]->atime_ < stime ){
      if( log != NULL ) *log << *key2GroupLinkList[i] << "\n";
      key2GroupLinks_.drop(*key2GroupLinkList[i]);
      r = true;
    }
  }
  Array<Key2ServerLink *> key2ServerLinkList;
  key2ServerLinks_.list(key2ServerLinkList);
  for( i = key2ServerLinkList.count() - 1; i >= 0; i-- ){
    if( key2ServerLinkList[i]->atime_ < stime ){
      if( log != NULL ) *log << *key2ServerLinkList[i] << "\n";
      key2ServerLinks_.drop(*key2ServerLinkList[i]);
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
