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
    if( i.getChar() == '\n' ){
      key.remove(i);
      i.prev();
    }
    if( i.getChar() == '\r' ) key.remove(i);
    i = key.strstr(": ");
    if( !i.eof() ){
      a.value(
        /*unScreenString(*/utf8::String(key,i)/*)*/,
        /*unScreenString(*/i + 2/*)*/
      );
    }
    else if( !(i = key.strstr(": ")).eof() ){
      a.value(
        unScreenString(utf8::String(key,i)),
        unScreenString(i + 2)
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
      screenString(a.attributes()[i].key_) + ": " +
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
}
//------------------------------------------------------------------------------
UserInfo::UserInfo() : atime_(gettimeofday()), mtime_(atime_)
{
}
//------------------------------------------------------------------------------
UserInfo::UserInfo(const utf8::String & name) : atime_(gettimeofday()), mtime_(atime_), name_(name)
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
  mtime_ = a.mtime_;
  name_ = a.name_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,UserInfo & a)
{
  return s >> a.name_ >> a.atime_ >> a.mtime_;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const UserInfo & a)
{
  return s << a.name_ << a.atime_ << a.mtime_;
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const UserInfo & a)
{
  return s << a.name_ << ", atime: " <<
    getTimeString(a.atime_) << ", mtime: " << getTimeString(a.mtime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
KeyInfo::~KeyInfo()
{
}
//------------------------------------------------------------------------------
KeyInfo::KeyInfo() : atime_(gettimeofday()), mtime_(atime_)
{
}
//------------------------------------------------------------------------------
KeyInfo::KeyInfo(const utf8::String & name) : atime_(gettimeofday()), mtime_(atime_), name_(name)
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
  mtime_ = a.mtime_;
  name_ = a.name_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,KeyInfo & a)
{
  return s >> a.name_ >> a.atime_ >> a.mtime_;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const KeyInfo & a)
{
  return s << a.name_ << a.atime_ << a.mtime_;
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const KeyInfo & a)
{
  return s << a.name_ <<
    ", atime: " << getTimeString(a.atime_) << ", mtime: " << getTimeString(a.mtime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
GroupInfo::~GroupInfo()
{
}
//------------------------------------------------------------------------------
GroupInfo::GroupInfo() : atime_(gettimeofday()), mtime_(atime_)
{
}
//------------------------------------------------------------------------------
GroupInfo::GroupInfo(const utf8::String & name) : atime_(gettimeofday()), mtime_(atime_), name_(name)
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
  mtime_ = a.mtime_;
  name_ = a.name_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,GroupInfo & a)
{
  return s >> a.name_ >> a.atime_ >> a.mtime_;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const GroupInfo & a)
{
  return s << a.name_ << a.atime_ << a.mtime_;
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const GroupInfo & a)
{
  return s << a.name_ <<
    ", atime: " << getTimeString(a.atime_) << ", mtime: " << getTimeString(a.mtime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
ServerInfo::~ServerInfo()
{
}
//------------------------------------------------------------------------------
ServerInfo::ServerInfo() :
  atime_(gettimeofday()), mtime_(atime_), stime_(0), type_(stStandalone)
{
}
//------------------------------------------------------------------------------
ServerInfo::ServerInfo(const utf8::String & name,ServerType type) :
  atime_(gettimeofday()), mtime_(atime_), name_(name), type_(type)
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
  mtime_ = a.mtime_;
  name_ = a.name_;
  type_ = a.type_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,ServerInfo & a)
{
  uint8_t v;
  s >> a.name_ >> a.atime_ >> a.mtime_ >> v;
  a.type_ = ServerType(v);
  return s;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const ServerInfo & a)
{
  return s << a.name_ << a.atime_ << a.mtime_ << (uint8_t) a.type_;
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const ServerInfo & a)
{
  return s << a.name_ << " " << serverTypeName_[a.type_] <<
    ", atime: " << getTimeString(a.atime_) << ", mtime: " << getTimeString(a.mtime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
User2KeyLink::~User2KeyLink()
{
}
//------------------------------------------------------------------------------
User2KeyLink::User2KeyLink() : atime_(gettimeofday()), mtime_(atime_)
{
}
//------------------------------------------------------------------------------
User2KeyLink::User2KeyLink(const utf8::String & userName,const utf8::String & keyName) :
  atime_(gettimeofday()), mtime_(atime_), user_(userName), key_(keyName)
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
  mtime_ = a.mtime_;
  user_ = a.user_;
  key_ = a.key_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,User2KeyLink & a)
{
  return s >> a.user_ >> a.key_ >> a.atime_ >> a.mtime_;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const User2KeyLink & a)
{
  return s << a.user_ << a.key_ << a.atime_ << a.mtime_;
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const User2KeyLink & a)
{
  return s << a.user_ << " " << a.key_ <<
    ", atime: " << getTimeString(a.atime_) << ", mtime: " << getTimeString(a.mtime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Key2GroupLink::~Key2GroupLink()
{
}
//------------------------------------------------------------------------------
Key2GroupLink::Key2GroupLink() : atime_(gettimeofday()), mtime_(atime_)
{
}
//------------------------------------------------------------------------------
Key2GroupLink::Key2GroupLink(const utf8::String & keyName,const utf8::String & groupName) :
  atime_(gettimeofday()), mtime_(atime_), key_(keyName), group_(groupName)
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
  mtime_ = a.mtime_;
  key_ = a.key_;
  group_ = a.group_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,Key2GroupLink & a)
{
  return s >> a.key_ >> a.group_ >> a.atime_ >> a.mtime_;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const Key2GroupLink & a)
{
  return s << a.key_ << a.group_ << a.atime_ << a.mtime_;
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const Key2GroupLink & a)
{
  return s << a.key_ << " " << a.group_ <<
    ", atime: " << getTimeString(a.atime_) << ", mtime: " << getTimeString(a.mtime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Key2ServerLink::~Key2ServerLink()
{
}
//------------------------------------------------------------------------------
Key2ServerLink::Key2ServerLink() : atime_(gettimeofday()), mtime_(atime_)
{
}
//------------------------------------------------------------------------------
Key2ServerLink::Key2ServerLink(const utf8::String & keyName,const utf8::String & serverName) :
  atime_(gettimeofday()), mtime_(atime_), key_(keyName), server_(serverName)
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
  mtime_ = a.mtime_;
  key_ = a.key_;
  server_ = a.server_;
  return *this;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,Key2ServerLink & a)
{
  return s >> a.key_ >> a.server_ >> a.atime_ >> a.mtime_;
}
//------------------------------------------------------------------------------
ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const Key2ServerLink & a)
{
  return s << a.key_ << a.server_ << a.atime_ << a.mtime_;
}
//------------------------------------------------------------------------------
utf8::String::Stream & operator << (utf8::String::Stream & s,const Key2ServerLink & a)
{
  return s << a.key_ << " " << a.server_ <<
    ", atime: " << getTimeString(a.atime_) << ", mtime: " << getTimeString(a.mtime_);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Server::Data::~Data()
{
}
//------------------------------------------------------------------------------
Server::Data::Data() : ftime_(0), stime_(gettimeofday())
{
}
//------------------------------------------------------------------------------
bool Server::Data::registerUserNL(const UserInfo & info,uint64_t ftime)
{
  UserInfo * p = users_.find(info);
  if( p == NULL ){
    if( info.mtime_ > ftime ){
      users_.insert(userList_.safeAdd(p = new UserInfo(info)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
  }
  return false;
}
//------------------------------------------------------------------------------
bool Server::Data::registerUser(const UserInfo & info,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerUserNL(info,ftime);
}
//------------------------------------------------------------------------------
bool Server::Data::registerKeyNL(const KeyInfo & info,uint64_t ftime)
{
  KeyInfo * p = keys_.find(info);
  if( p == NULL ){
    if( info.mtime_ > ftime ){
      keys_.insert(keyList_.safeAdd(p = new KeyInfo(info)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
  }
  return false;
}
//------------------------------------------------------------------------------
bool Server::Data::registerKey(const KeyInfo & info,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerKeyNL(info,ftime);
}
//------------------------------------------------------------------------------
bool Server::Data::registerGroupNL(const GroupInfo & info,uint64_t ftime)
{
  GroupInfo * p = groups_.find(info);
  if( p == NULL ){
    if( info.mtime_ > ftime ){
      groups_.insert(groupList_.safeAdd(p = new GroupInfo(info)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
  }
  return false;
}
//------------------------------------------------------------------------------
bool Server::Data::registerGroup(const GroupInfo & info,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerGroupNL(info,ftime);
}
//------------------------------------------------------------------------------
bool Server::Data::registerServerNL(const ServerInfo & info,uint64_t ftime)
{
  ServerInfo * p = servers_.find(info);
  if( p == NULL ){
    if( info.mtime_ > ftime ){
      servers_.insert(serverList_.safeAdd(p = new ServerInfo(info)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    if( info.mtime_ > ftime && info.type_ == stNode && info.type_ != p->type_ ){
      p->mtime_ = p->atime_;
      p->type_ = info.type_;
      return true;
    }
  }
  return false;
}
//------------------------------------------------------------------------------
bool Server::Data::registerServer(const ServerInfo & info,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerServerNL(info,ftime);
}
//------------------------------------------------------------------------------
bool Server::Data::registerUser2KeyLinkNL(const User2KeyLink & link,uint64_t ftime)
{
  User2KeyLink * p = user2KeyLinks_.find(link);
  if( p == NULL ){
    if( link.mtime_ > ftime ){
      user2KeyLinks_.insert(user2KeyLinkList_.safeAdd(p = new User2KeyLink(link)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
  }
  return false;
}
//------------------------------------------------------------------------------
bool Server::Data::registerUser2KeyLink(const User2KeyLink & link,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerUser2KeyLinkNL(link,ftime);
}
//------------------------------------------------------------------------------
bool Server::Data::registerKey2GroupLinkNL(const Key2GroupLink & link,uint64_t ftime)
{
  Key2GroupLink * p = key2GroupLinks_.find(link);
  if( p == NULL ){
    if( link.mtime_ > ftime ){
      key2GroupLinks_.insert(key2GroupLinkList_.safeAdd(p = new Key2GroupLink(link)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
  }
  return false;
}
//------------------------------------------------------------------------------
bool Server::Data::registerKey2GroupLink(const Key2GroupLink & link,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerKey2GroupLinkNL(link,ftime);
}
//------------------------------------------------------------------------------
bool Server::Data::registerKey2ServerLinkNL(const Key2ServerLink & link,uint64_t ftime)
{
  Key2ServerLink * p = key2ServerLinks_.find(link);
  if( p == NULL ){
    if( link.mtime_ > ftime ){
      key2ServerLinks_.insert(key2ServerLinkList_.safeAdd(p = new Key2ServerLink(link)));
      return true;
    }
  }
  else {
    p->atime_ = gettimeofday();
    if( link.mtime_ > ftime && p->server_.strcasecmp(link.server_) != 0 ){
      p->mtime_ = p->atime_;
      p->server_ = link.server_;
      return true;
    }
  }
  return false;
}
//------------------------------------------------------------------------------
bool Server::Data::registerKey2ServerLink(const Key2ServerLink & link,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  return registerKey2ServerLinkNL(link,ftime);
}
//------------------------------------------------------------------------------
void Server::Data::sendDatabaseNL(ksock::AsyncSocket & socket,uint64_t ftime)
{
  intptr_t i;
  uint64_t u;

  u = 0;
  for( i = userList_.count() - 1; i >= 0; i-- )
    if( userList_[i].mtime_ > ftime ) u++;
  socket << u;
  for( i = userList_.count() - 1; i >= 0; i-- )
    if( userList_[i].mtime_ > ftime ) socket << userList_[i];
  u = 0;
  for( i = keyList_.count() - 1; i >= 0; i-- )
    if( keyList_[i].mtime_ > ftime ) u++;
  socket << u;
  for( i = keyList_.count() - 1; i >= 0; i-- )
    if( keyList_[i].mtime_ > ftime ) socket << keyList_[i];
  u = 0;
  for( i = groupList_.count() - 1; i >= 0; i-- )
    if( groupList_[i].mtime_ > ftime ) u++;
  socket << u;
  for( i = groupList_.count() - 1; i >= 0; i-- )
    if( groupList_[i].mtime_ > ftime ) socket << groupList_[i];
  u = 0;
  for( i = serverList_.count() - 1; i >= 0; i-- )
    if( serverList_[i].mtime_ > ftime ) u++;
  socket << u;
  for( i = serverList_.count() - 1; i >= 0; i-- )
    if( serverList_[i].mtime_ > ftime ) socket << serverList_[i];
  u = 0;
  for( i = user2KeyLinkList_.count() - 1; i >= 0; i-- )
    if( user2KeyLinkList_[i].mtime_ > ftime ) u++;
  socket << u;
  for( i = user2KeyLinkList_.count() - 1; i >= 0; i-- )
    if( user2KeyLinkList_[i].mtime_ > ftime ) socket << user2KeyLinkList_[i];
  u = 0;
  for( i = key2GroupLinkList_.count() - 1; i >= 0; i-- )
    if( key2GroupLinkList_[i].mtime_ > ftime ) u++;
  socket << u;
  for( i = key2GroupLinkList_.count() - 1; i >= 0; i-- )
    if( key2GroupLinkList_[i].mtime_ > ftime ) socket << key2GroupLinkList_[i];
  u = 0;
  for( i = key2ServerLinkList_.count() - 1; i >= 0; i-- )
    if( key2ServerLinkList_[i].mtime_ > ftime ) u++;
  socket << u;
  for( i = key2ServerLinkList_.count() - 1; i >= 0; i-- )
    if( key2ServerLinkList_[i].mtime_ > ftime ) socket << key2ServerLinkList_[i];
}
//------------------------------------------------------------------------------
void Server::Data::sendDatabase(ksock::AsyncSocket & socket,uint64_t ftime)
{
  AutoMutexRDLock<FiberMutex> lock(mutex_);
  sendDatabaseNL(socket,ftime);
}
//------------------------------------------------------------------------------
void Server::Data::recvDatabaseNL(ksock::AsyncSocket & socket,uint64_t ftime)
{
  union {
    intptr_t i;
    uint64_t u;
  };
  socket >> u;
  while( u-- > 0 ){
    UserInfo info;
    socket >> info;
    registerUserNL(info,ftime);
  }
  socket >> u;
  while( u-- > 0 ){
    KeyInfo info;
    socket >> info;
    registerKeyNL(info,ftime);
  }
  socket >> u;
  while( u-- > 0 ){
    GroupInfo info;
    socket >> info;
    registerGroupNL(info,ftime);
  }
  socket >> u;
  while( u-- > 0 ){
    ServerInfo info;
    socket >> info;
    registerServerNL(info,ftime);
  }
  socket >> u;
  while( u-- > 0 ){
    User2KeyLink link;
    socket >> link;
    registerUser2KeyLinkNL(link,ftime);
  }
  socket >> u;
  while( u-- > 0 ){
    Key2GroupLink link;
    socket >> link;
    registerKey2GroupLinkNL(link,ftime);
  }
  socket >> u;
  while( u-- > 0 ){
    Key2ServerLink link;
    socket >> link;
    registerKey2ServerLinkNL(link,ftime);
  }
}
//------------------------------------------------------------------------------
void Server::Data::recvDatabase(ksock::AsyncSocket & socket,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock(mutex_);
  recvDatabaseNL(socket,ftime);
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
  ftime_ = 0;
  return *this;
}
//------------------------------------------------------------------------------
bool Server::Data::orNL(const Data & a,uint64_t ftime)
{
  intptr_t i;
  bool r = false;
  for( i = a.userList_.count() - 1; i >= 0; i-- )
    r = registerUserNL(a.userList_[i],ftime) || r;
  for( i = a.keyList_.count() - 1; i >= 0; i-- )
    r = registerKeyNL(a.keyList_[i],ftime) || r;
  for( i = a.groupList_.count() - 1; i >= 0; i-- )
    r = registerGroupNL(a.groupList_[i],ftime) || r;
  for( i = a.serverList_.count() - 1; i >= 0; i-- )
    r = registerServerNL(a.serverList_[i],ftime) || r;
  for( i = a.user2KeyLinkList_.count() - 1; i >= 0; i-- )
    r = registerUser2KeyLinkNL(a.user2KeyLinkList_[i],ftime) || r;
  for( i = a.key2GroupLinkList_.count() - 1; i >= 0; i-- )
    r = registerKey2GroupLinkNL(a.key2GroupLinkList_[i],ftime) || r;
  for( i = a.key2ServerLinkList_.count() - 1; i >= 0; i-- )
    r = registerKey2ServerLinkNL(a.key2ServerLinkList_[i],ftime) || r;
  return r;
}
//------------------------------------------------------------------------------
bool Server::Data::or(const Data & a,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock0(mutex_);
  AutoMutexRDLock<FiberMutex> lock1(a.mutex_);
  return orNL(a,ftime);
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::xorNL(const Data & data1,const Data & data2,uint64_t ftime)
{
  intptr_t i;
  for( i = data2.userList_.count() - 1; i >= 0; i-- )
    if( data1.users_.find(data2.userList_[i]) == NULL )
      registerUserNL(data2.userList_[i],ftime);
  for( i = data2.keyList_.count() - 1; i >= 0; i-- )
    if( data1.keys_.find(data2.keyList_[i]) == NULL )
      registerKeyNL(data2.keyList_[i],ftime);
  for( i = data2.groupList_.count() - 1; i >= 0; i-- )
    if( data1.groups_.find(data2.groupList_[i]) == NULL )
      registerGroupNL(data2.groupList_[i],ftime);
  for( i = data2.serverList_.count() - 1; i >= 0; i-- )
    if( data1.servers_.find(data2.serverList_[i]) == NULL )
      registerServerNL(data2.serverList_[i],ftime);
  for( i = data2.user2KeyLinkList_.count() - 1; i >= 0; i-- )
    if( data1.user2KeyLinks_.find(data2.user2KeyLinkList_[i]) == NULL )
      registerUser2KeyLinkNL(data2.user2KeyLinkList_[i],ftime);
  for( i = data2.key2GroupLinkList_.count() - 1; i >= 0; i-- )
    if( data1.key2GroupLinks_.find(data2.key2GroupLinkList_[i]) == NULL )
      registerKey2GroupLinkNL(data2.key2GroupLinkList_[i],ftime);
  for( i = data2.key2ServerLinkList_.count() - 1; i >= 0; i-- )
    if( data1.key2ServerLinks_.find(data2.key2ServerLinkList_[i]) == NULL )
      registerKey2ServerLinkNL(data2.key2ServerLinkList_[i],ftime);
  return *this;
}
//------------------------------------------------------------------------------
Server::Data & Server::Data::xor(const Data & data1,const Data & data2,uint64_t ftime)
{
  AutoMutexWRLock<FiberMutex> lock0(mutex_);
  AutoMutexRDLock<FiberMutex> lock1(data1.mutex_);
  AutoMutexRDLock<FiberMutex> lock2(data2.mutex_);
  return xorNL(data1,data2,ftime);
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
      if( quoted ) list += "\"";
      list += keyList_[i].name_;
      if( quoted ) list += "\"";
      if( i > 0 ) list += ",";
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
