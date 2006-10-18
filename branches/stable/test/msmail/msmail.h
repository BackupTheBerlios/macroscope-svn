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
#ifndef _msmail_H_
#define _msmail_H_
//------------------------------------------------------------------------------
using namespace ksys;
//------------------------------------------------------------------------------
namespace msmail {
//------------------------------------------------------------------------------
const unsigned defaultPort = 2525;
//------------------------------------------------------------------------------
enum CmdType {
  cmQuit,
  cmGetProcessStartTime,
  cmSelectServerType,
  cmRegisterClient,
  cmRegisterDB,
  cmGetDB,
  cmSendMail,
  cmRecvMail,
  cmRemoveMail,
  cmSelectProtocol
};
//------------------------------------------------------------------------------
enum ServerType {
  stNone = -1,
  stNode = 0,
  stStandalone = 1,
  stCount
};
//------------------------------------------------------------------------------
extern const char * serverTypeName_[];
extern const char * serverConfSectionName_[];
//------------------------------------------------------------------------------
enum Error {
  eOK,
  eFail = ksock::AsyncSocket::aeCount, // 2007
  eInvalidCommand,
  eInvalidServerType,
  eInvalidMessage,
  eLastMessage,
  eInvalidMessageId,
  eInvalidProtocol,
  eCount
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
extern const char messageIdKey[];
class Message {
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,Message & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const Message & a);
  friend AsyncFile & operator >> (AsyncFile & s,Message & a);
  friend AsyncFile & operator << (AsyncFile & s,const Message & a);
  public:
    ~Message();
    Message();
    Message(const utf8::String & sid);
    Message(const Message & a);
    Message & operator = (const Message & a);

    class Key {
      public:
        ~Key(){}
        Key(){}
        Key(const Key & a) : key_(a.key_) {}
        Key(const utf8::String & key) : key_(key) {}

        Key & operator = (const Key & a){
          key_ = a.key_;
          return *this;
        }

        Key & operator = (const utf8::String & key){
          key_ = key_;
          return *this;
        }

        bool operator == (const Key & a) const { return key_.strcmp(a.key_) == 0; }
        bool operator != (const Key & a) const { return key_.strcmp(a.key_) != 0; }
        bool operator >= (const Key & a) const { return key_.strcmp(a.key_) >= 0; }
        bool operator >  (const Key & a) const { return key_.strcmp(a.key_) >  0; }
        bool operator <= (const Key & a) const { return key_.strcmp(a.key_) <= 0; }
        bool operator <  (const Key & a) const { return key_.strcmp(a.key_) <  0; }

        operator const utf8::String & () const { return key_; }

        static EmbeddedHashNode<Key> & keyNode(const Key & object){
          return object.keyNode_;
        }
        static Key & keyNodeObject(const EmbeddedHashNode<Key> & node,Key * p){
          return node.object(p->keyNode_);
        }
        static uintptr_t keyNodeHash(const Key & object){
          return object.key_.hash(true);
        }
        static bool keyHashNodeEqu(const Key & object1,const Key & object2){
          return object1.key_.strcmp(object2.key_) == 0;
        }
      protected:
      private:
        mutable EmbeddedHashNode<Key> keyNode_;
        utf8::String key_;
    };
    typedef EmbeddedHash<
      Key,
      Key::keyNode,
      Key::keyNodeObject,
      Key::keyNodeHash,
      Key::keyHashNodeEqu
    > Keys;

    class Attribute {
      friend class Message;
      public:
        ~Attribute(){}
        Attribute(){}
        Attribute(const Attribute & a) : key_(a.key_), value_(a.value_) {}
        Attribute(const utf8::String & key,const utf8::String & value = utf8::String()) :
          key_(key), value_(value) {}
        Attribute(const Key & key,const utf8::String & value = utf8::String()) :
          key_(key), value_(value) {}
        Attribute & operator = (const Attribute & a){
          key_ = a.key_;
          value_ = a.value_;
          return *this;
        }
        bool operator == (const Attribute & a) const { return key_ == a.key_; }
        bool operator != (const Attribute & a) const { return key_ != a.key_; }
        bool operator >= (const Attribute & a) const { return key_ >= a.key_; }
        bool operator >  (const Attribute & a) const { return key_ >  a.key_; }
        bool operator <= (const Attribute & a) const { return key_ <= a.key_; }
        bool operator <  (const Attribute & a) const { return key_ <  a.key_; }

        Key key_;
        utf8::String value_;
      protected:
      private:
        static EmbeddedHashNode<Attribute> & keyNode(const Attribute & object){
          return object.keyNode_;
        }
        static Attribute & keyNodeObject(const EmbeddedHashNode<Attribute> & node,Attribute * p){
          return node.object(p->keyNode_);
        }
        static uintptr_t keyNodeHash(const Attribute & object){
          return utf8::String(object.key_).hash(true);
        }
        static bool keyHashNodeEqu(const Attribute & object1,const Attribute & object2){
          return object1.key_ == object2.key_;
        }
        mutable EmbeddedHashNode<Attribute> keyNode_;
    };

    bool operator == (const Message & a) const { return id().strcmp(a.id()) == 0; }
    bool operator >= (const Message & a) const { return id().strcmp(a.id()) >= 0; }
    bool operator >  (const Message & a) const { return id().strcmp(a.id()) >  0; }
    bool operator <= (const Message & a) const { return id().strcmp(a.id()) <= 0; }
    bool operator <  (const Message & a) const { return id().strcmp(a.id()) <  0; }

    const utf8::String & id() const;
    Message & id(const utf8::String & id);
    const utf8::String & value(const utf8::String & key) const;
    bool isValue(const utf8::String & key) const;
    Message & value(const utf8::String & key,const utf8::String & value);
    utf8::String removeValue(const utf8::String & key);
    Message & removeValueByLeft(const utf8::String & key);
    Message & copyUserAttributes(const Message & msg);

    static EmbeddedHashNode<Message> & idNode(const Message & object){
      return object.idNode_;
    }
    static Message & idNodeObject(const EmbeddedHashNode<Message> & node,Message * p){
      return node.object(p->idNode_);
    }
    static uintptr_t idNodeHash(const Message & object){
      return object.id().hash(true);
    }
    static bool idHashNodeEqu(const Message & object1,const Message & object2){
      return object1.id().strcmp(object2.id()) == 0;
    }
  protected:
  private:
    mutable EmbeddedHashNode<Message> idNode_;
    typedef EmbeddedHash<
      Attribute,
      Attribute::keyNode,
      Attribute::keyNodeObject,
      Attribute::keyNodeHash,
      Attribute::keyHashNodeEqu
    > Attributes;
    mutable Attributes attributes_;
    AutoHashDrop<Attributes> attributesAutoDrop_;
};
//------------------------------------------------------------------------------
inline const utf8::String & Message::id() const
{
  return value(messageIdKey);
}
//------------------------------------------------------------------------------
typedef EmbeddedHash<
  Message,
  Message::idNode,
  Message::idNodeObject,
  Message::idNodeHash,
  Message::idHashNodeEqu
> Messages;
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class InfoLinkKey {
  friend AsyncFile & operator >> (AsyncFile & s,InfoLinkKey & a);
  friend AsyncFile & operator << (AsyncFile & s,const InfoLinkKey & a);
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,InfoLinkKey & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const InfoLinkKey & a);
  public:
    ~InfoLinkKey(){}
    InfoLinkKey(){}
    InfoLinkKey(const InfoLinkKey & a) : key_(a.key_) {}
    InfoLinkKey(const utf8::String & key) : key_(key) {}

    InfoLinkKey & operator = (const InfoLinkKey & a){
      key_ = a.key_;
      return *this;
    }
    InfoLinkKey & operator = (const utf8::String & key){
      key_ = key_;
      return *this;
    }
    bool operator == (const InfoLinkKey & a) const { return key_.strcasecmp(a.key_) == 0; }
    bool operator != (const InfoLinkKey & a) const { return key_.strcasecmp(a.key_) != 0; }
    bool operator >= (const InfoLinkKey & a) const { return key_.strcasecmp(a.key_) >= 0; }
    bool operator >  (const InfoLinkKey & a) const { return key_.strcasecmp(a.key_) >  0; }
    bool operator <= (const InfoLinkKey & a) const { return key_.strcasecmp(a.key_) <= 0; }
    bool operator <  (const InfoLinkKey & a) const { return key_.strcasecmp(a.key_) <  0; }
    operator const utf8::String & () const { return key_; }

    static EmbeddedHashNode<InfoLinkKey> & keyNode(const InfoLinkKey & object){
      return object.keyNode_;
    }
    static InfoLinkKey & keyNodeObject(const EmbeddedHashNode<InfoLinkKey> & node,InfoLinkKey * p){
      return node.object(p->keyNode_);
    }
    static uintptr_t keyNodeHash(const InfoLinkKey & object){
      return object.key_.hash(false);
    }
    static bool keyHashNodeEqu(const InfoLinkKey & object1,const InfoLinkKey & object2){
      return object1.key_.strcasecmp(object2.key_) == 0;
    }

  protected:
  private:
    mutable EmbeddedHashNode<InfoLinkKey> keyNode_;
    utf8::String key_;
};
//------------------------------------------------------------------------------
/*inline AsyncFile & operator >> (AsyncFile & s,InfoLinkKey & a)
{
  return s >> a.key_;
}
//------------------------------------------------------------------------------
inline AsyncFile & operator << (AsyncFile & s,const InfoLinkKey & a)
{
  return s << a.key_;
}*/
//------------------------------------------------------------------------------
inline ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,InfoLinkKey & a)
{
  return s >> a.key_;
}
//------------------------------------------------------------------------------
inline ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const InfoLinkKey & a)
{
  return s << a.key_;
}
//------------------------------------------------------------------------------
typedef EmbeddedHash<
  InfoLinkKey,
  InfoLinkKey::keyNode,
  InfoLinkKey::keyNodeObject,
  InfoLinkKey::keyNodeHash,
  InfoLinkKey::keyHashNodeEqu
> InfoLinkKeys;
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class UserInfo {
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,UserInfo & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const UserInfo & a);
  friend utf8::String::Stream & operator << (utf8::String::Stream & s,const UserInfo & a);
  public:
    ~UserInfo();
    UserInfo();
    UserInfo(const utf8::String & name);
    UserInfo(const UserInfo &);
    UserInfo & operator = (const UserInfo &);

    /*bool operator == (const UserInfo & a) const { return name_.strcasecmp(a.name_) == 0; }
    bool operator != (const UserInfo & a) const { return name_.strcasecmp(a.name_) != 0; }
    bool operator >= (const UserInfo & a) const { return name_.strcasecmp(a.name_) >= 0; }
    bool operator >  (const UserInfo & a) const { return name_.strcasecmp(a.name_) >  0; }
    bool operator <= (const UserInfo & a) const { return name_.strcasecmp(a.name_) <= 0; }
    bool operator <  (const UserInfo & a) const { return name_.strcasecmp(a.name_) <  0; }*/

    static EmbeddedHashNode<UserInfo> & hashNode(const UserInfo & object){
      return object.hashNode_;
    }
    static UserInfo & hashNodeObject(const EmbeddedHashNode<UserInfo> & node,UserInfo * p){
      return node.object(p->hashNode_);
    }
    static uintptr_t hashNodeHash(const UserInfo & object){
      return object.name_.hash(false);
    }
    static bool hashNodeEqu(const UserInfo & object1,const UserInfo & object2){
      return object1.name_.strcasecmp(object2.name_) == 0;
    }

    uint64_t atime_; // время последнего обращения (для удаления устаревших)
    uint64_t rtime_; // время пометки на удаление
    mutable EmbeddedHashNode<UserInfo> hashNode_;
    utf8::String name_;
    InfoLinkKeys sendedTo_;
    AutoHashDrop<InfoLinkKeys> sendedToAutoDrop_;
  protected:
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class KeyInfo {
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,KeyInfo & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const KeyInfo & a);
  friend utf8::String::Stream & operator << (utf8::String::Stream & s,const KeyInfo & a);
  public:
    ~KeyInfo();
    KeyInfo();
    KeyInfo(const utf8::String & name);
    KeyInfo(const KeyInfo &);
    KeyInfo & operator = (const KeyInfo &);

    /*bool operator == (const KeyInfo & a) const { return name_.strcasecmp(a.name_) == 0; }
    bool operator != (const KeyInfo & a) const { return name_.strcasecmp(a.name_) != 0; }
    bool operator >= (const KeyInfo & a) const { return name_.strcasecmp(a.name_) >= 0; }
    bool operator >  (const KeyInfo & a) const { return name_.strcasecmp(a.name_) >  0; }
    bool operator <= (const KeyInfo & a) const { return name_.strcasecmp(a.name_) <= 0; }
    bool operator <  (const KeyInfo & a) const { return name_.strcasecmp(a.name_) <  0; }*/

    static EmbeddedHashNode<KeyInfo> & hashNode(const KeyInfo & object){
      return object.hashNode_;
    }
    static KeyInfo & hashNodeObject(const EmbeddedHashNode<KeyInfo> & node,KeyInfo * p){
      return node.object(p->hashNode_);
    }
    static uintptr_t hashNodeHash(const KeyInfo & object){
      return object.name_.hash(true);
    }
    static bool hashNodeEqu(const KeyInfo & object1,const KeyInfo & object2){
      return object1.name_.strcasecmp(object2.name_) == 0;
    }

    uint64_t atime_; // время последнего обращения (для удаления устаревших)
    uint64_t rtime_; // время пометки на удаление
    mutable EmbeddedHashNode<KeyInfo> hashNode_;
    utf8::String name_;
    InfoLinkKeys sendedTo_;
    AutoHashDrop<InfoLinkKeys> sendedToAutoDrop_;
  protected:
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class GroupInfo {
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,GroupInfo & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const GroupInfo & a);
  friend utf8::String::Stream & operator << (utf8::String::Stream & s,const GroupInfo & a);
  public:
    ~GroupInfo();
    GroupInfo();
    GroupInfo(const utf8::String & name);
    GroupInfo(const GroupInfo &);
    GroupInfo & operator = (const GroupInfo &);

    /*bool operator == (const GroupInfo & a) const { return name_.strcasecmp(a.name_) == 0; }
    bool operator != (const GroupInfo & a) const { return name_.strcasecmp(a.name_) != 0; }
    bool operator >= (const GroupInfo & a) const { return name_.strcasecmp(a.name_) >= 0; }
    bool operator >  (const GroupInfo & a) const { return name_.strcasecmp(a.name_) >  0; }
    bool operator <= (const GroupInfo & a) const { return name_.strcasecmp(a.name_) <= 0; }
    bool operator <  (const GroupInfo & a) const { return name_.strcasecmp(a.name_) <  0; }*/

    static EmbeddedHashNode<GroupInfo> & hashNode(const GroupInfo & object){
      return object.hashNode_;
    }
    static GroupInfo & hashNodeObject(const EmbeddedHashNode<GroupInfo> & node,GroupInfo * p){
      return node.object(p->hashNode_);
    }
    static uintptr_t hashNodeHash(const GroupInfo & object){
      return object.name_.hash(true);
    }
    static bool hashNodeEqu(const GroupInfo & object1,const GroupInfo & object2){
      return object1.name_.strcasecmp(object2.name_) == 0;
    }

    uint64_t atime_; // время последнего обращения (для удаления устаревших)
    uint64_t rtime_; // время пометки на удаление
    mutable EmbeddedHashNode<GroupInfo> hashNode_;
    utf8::String name_;
    InfoLinkKeys sendedTo_;
    AutoHashDrop<InfoLinkKeys> sendedToAutoDrop_;
  protected:
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class ServerInfo {
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,ServerInfo & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const ServerInfo & a);
  friend utf8::String::Stream & operator << (utf8::String::Stream & s,const ServerInfo & a);
  public:
    ~ServerInfo();
    ServerInfo();
    ServerInfo(const utf8::String & name,ServerType type = stNone);
    ServerInfo(const ServerInfo &);
    ServerInfo & operator = (const ServerInfo &);

    /*bool operator == (const ServerInfo & a) const { return name_.strcasecmp(a.name_) == 0; }
    bool operator != (const ServerInfo & a) const { return name_.strcasecmp(a.name_) != 0; }
    bool operator >= (const ServerInfo & a) const { return name_.strcasecmp(a.name_) >= 0; }
    bool operator >  (const ServerInfo & a) const { return name_.strcasecmp(a.name_) >  0; }
    bool operator <= (const ServerInfo & a) const { return name_.strcasecmp(a.name_) <= 0; }
    bool operator <  (const ServerInfo & a) const { return name_.strcasecmp(a.name_) <  0; }*/

    static EmbeddedHashNode<ServerInfo> & hashNode(const ServerInfo & object){
      return object.hashNode_;
    }
    static ServerInfo & hashNodeObject(const EmbeddedHashNode<ServerInfo> & node,ServerInfo * p){
      return node.object(p->hashNode_);
    }
    static uintptr_t hashNodeHash(const ServerInfo & object){
      return object.name_.hash(false);
    }
    static bool hashNodeEqu(const ServerInfo & object1,const ServerInfo & object2){
      return object1.name_.strcasecmp(object2.name_) == 0;
    }

    uint64_t atime_; // время последнего обращения (для удаления устаревших)
    uint64_t rtime_; // время пометки на удаление
    uint64_t stime_; // время старта процесса
    mutable EmbeddedHashNode<ServerInfo> hashNode_;
    utf8::String name_;
    ServerType type_;
    InfoLinkKeys sendedTo_;
    AutoHashDrop<InfoLinkKeys> sendedToAutoDrop_;
    uintptr_t connectErrorCount_;
  protected:
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class User2KeyLink {
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,User2KeyLink & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const User2KeyLink & a);
  friend utf8::String::Stream & operator << (utf8::String::Stream & s,const User2KeyLink & a);
  public:
    ~User2KeyLink();
    User2KeyLink();
    User2KeyLink(const utf8::String & userName,const utf8::String & keyName);
    User2KeyLink(const User2KeyLink &);
    User2KeyLink & operator = (const User2KeyLink &);

    /*bool operator == (const User2KeyLink & a) const { return user_.strcasecmp(a.user_) == 0 && key_.strcasecmp(a.key_) == 0; }
    bool operator != (const User2KeyLink & a) const { return user_.strcasecmp(a.user_) != 0 && key_.strcasecmp(a.key_) != 0; }
    bool operator >= (const User2KeyLink & a) const { return user_.strcasecmp(a.user_) >= 0 && key_.strcasecmp(a.key_) >= 0; }
    bool operator >  (const User2KeyLink & a) const { return user_.strcasecmp(a.user_) >= 0 && key_.strcasecmp(a.key_) >  0; }
    bool operator <= (const User2KeyLink & a) const { return user_.strcasecmp(a.user_) <= 0 && key_.strcasecmp(a.key_) <= 0; }
    bool operator <  (const User2KeyLink & a) const { return user_.strcasecmp(a.user_) <= 0 && key_.strcasecmp(a.key_) <  0; }*/

    static EmbeddedHashNode<User2KeyLink> & hashNode(const User2KeyLink & object){
      return object.hashNode_;
    }
    static User2KeyLink & hashNodeObject(const EmbeddedHashNode<User2KeyLink> & node,User2KeyLink * p){
      return node.object(p->hashNode_);
    }
    static uintptr_t hashNodeHash(const User2KeyLink & object){
      return (object.user_ + object.key_).hash(false);
    }
    static bool hashNodeEqu(const User2KeyLink & object1,const User2KeyLink & object2){
      return object1.user_.strcasecmp(object2.user_) == 0 && object1.key_.strcasecmp(object2.key_) == 0;
    }

    uint64_t atime_; // время последнего обращения (для удаления устаревших)
    uint64_t rtime_; // время пометки на удаление
    mutable EmbeddedHashNode<User2KeyLink> hashNode_;
    utf8::String user_;
    utf8::String key_;
    InfoLinkKeys sendedTo_;
    AutoHashDrop<InfoLinkKeys> sendedToAutoDrop_;
  protected:
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Key2GroupLink {
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,Key2GroupLink & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const Key2GroupLink & a);
  friend utf8::String::Stream & operator << (utf8::String::Stream & s,const Key2GroupLink & a);
  public:
    ~Key2GroupLink();
    Key2GroupLink();
    Key2GroupLink(const utf8::String & keyName,const utf8::String & groupName);
    Key2GroupLink(const Key2GroupLink &);
    Key2GroupLink & operator = (const Key2GroupLink &);

    /*bool operator == (const Key2GroupLink & a) const { return key_.strcasecmp(a.key_) == 0 && group_.strcasecmp(a.group_) == 0; }
    bool operator != (const Key2GroupLink & a) const { return key_.strcasecmp(a.key_) != 0 && group_.strcasecmp(a.group_) != 0; }
    bool operator >= (const Key2GroupLink & a) const { return key_.strcasecmp(a.key_) >= 0 && group_.strcasecmp(a.group_) >= 0; }
    bool operator >  (const Key2GroupLink & a) const { return key_.strcasecmp(a.key_) >= 0 && group_.strcasecmp(a.group_) >  0; }
    bool operator <= (const Key2GroupLink & a) const { return key_.strcasecmp(a.key_) <= 0 && group_.strcasecmp(a.group_) <= 0; }
    bool operator <  (const Key2GroupLink & a) const { return key_.strcasecmp(a.key_) <= 0 && group_.strcasecmp(a.group_) <  0; }*/

    static EmbeddedHashNode<Key2GroupLink> & hashNode(const Key2GroupLink & object){
      return object.hashNode_;
    }
    static Key2GroupLink & hashNodeObject(const EmbeddedHashNode<Key2GroupLink> & node,Key2GroupLink * p){
      return node.object(p->hashNode_);
    }
    static uintptr_t hashNodeHash(const Key2GroupLink & object){
      uintptr_t h[2];
      h[0] = object.key_.hash(false);
      h[1] = object.group_.hash(false);
      return HF::hash(h,sizeof(h));
    }
    static bool hashNodeEqu(const Key2GroupLink & object1,const Key2GroupLink & object2){
      return object1.key_.strcasecmp(object2.key_) == 0 && object1.group_.strcasecmp(object2.group_) == 0;
    }

    uint64_t atime_; // время последнего обращения (для удаления устаревших)
    uint64_t rtime_; // время пометки на удаление
    mutable EmbeddedHashNode<Key2GroupLink> hashNode_;
    utf8::String key_;
    utf8::String group_;
    InfoLinkKeys sendedTo_;
    AutoHashDrop<InfoLinkKeys> sendedToAutoDrop_;
  protected:
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Key2ServerLink {
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,Key2ServerLink & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const Key2ServerLink & a);
  friend utf8::String::Stream & operator << (utf8::String::Stream & s,const Key2ServerLink & a);
  public:
    ~Key2ServerLink();
    Key2ServerLink();
    Key2ServerLink(const utf8::String & keyName,const utf8::String & serverName = utf8::String());
    Key2ServerLink(const Key2ServerLink &);
    Key2ServerLink & operator = (const Key2ServerLink &);

    /*bool operator == (const Key2ServerLink & a) const { return key_.strcasecmp(a.key_) == 0; }
    bool operator != (const Key2ServerLink & a) const { return key_.strcasecmp(a.key_) != 0; }
    bool operator >= (const Key2ServerLink & a) const { return key_.strcasecmp(a.key_) >= 0; }
    bool operator >  (const Key2ServerLink & a) const { return key_.strcasecmp(a.key_) >= 0; }
    bool operator <= (const Key2ServerLink & a) const { return key_.strcasecmp(a.key_) <= 0; }
    bool operator <  (const Key2ServerLink & a) const { return key_.strcasecmp(a.key_) <= 0; }*/

    static EmbeddedHashNode<Key2ServerLink> & hashNode(const Key2ServerLink & object){
      return object.hashNode_;
    }
    static Key2ServerLink & hashNodeObject(const EmbeddedHashNode<Key2ServerLink> & node,Key2ServerLink * p){
      return node.object(p->hashNode_);
    }
    static uintptr_t hashNodeHash(const Key2ServerLink & object){
      return object.key_.hash(false);
    }
    static bool hashNodeEqu(const Key2ServerLink & object1,const Key2ServerLink & object2){
      return object1.key_.strcasecmp(object2.key_) == 0;
    }

    uint64_t atime_; // время последнего обращения (для удаления устаревших)
    uint64_t rtime_; // время пометки на удаление
    mutable EmbeddedHashNode<Key2ServerLink> hashNode_;
    utf8::String key_;
    utf8::String server_;
    InfoLinkKeys sendedTo_;
    AutoHashDrop<InfoLinkKeys> sendedToAutoDrop_;
  protected:
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class ServerFiber : public ksock::ServerFiber {
  friend class Server;
  public:
    virtual ~ServerFiber();
    ServerFiber(Server & server,utf8::String user = utf8::String(),utf8::String key = utf8::String());
  protected:
    bool isValidUser(const utf8::String & user);
    utf8::String getUserPassword(const utf8::String & user);
    void main();
  private:
    Server & server_;
    ServerType serverType_;
    uint8_t protocol_;

    ServerFiber(const ServerFiber &);
    void operator = (const ServerFiber &);
    
    static EmbeddedHashNode<ServerFiber> & hashNode(const ServerFiber & object){
      return object.hashNode_;
    }
    static ServerFiber & hashNodeObject(const EmbeddedHashNode<ServerFiber> & node,ServerFiber * p){
      return node.object(p->hashNode_);
    }
    static uintptr_t hashNodeHash(const ServerFiber & object){
      uintptr_t h[2];
      h[0] = object.user_.hash(false);
      h[1] = object.key_.hash(false);
      return HF::hash(h,sizeof(h));
    }
    static bool hashNodeEqu(const ServerFiber & object1,const ServerFiber & object2){
      return object1.user_.strcasecmp(object2.user_) == 0 && object1.key_.strcasecmp(object2.key_) == 0;
    }
    mutable EmbeddedHashNode<ServerFiber> hashNode_;
    utf8::String user_;
    utf8::String key_;
    DirectoryChangeNotification dcn_;

    void putCode(int32_t code);
    void checkCode(int32_t code,int32_t noThrowCode = eOK);
    void getCode(int32_t noThrowCode = eOK);
    void auth();    
    void registerClient();
    void registerDB();
    void getDB();
    void sendMail();
    void processMailbox(
      const utf8::String & userMailBox,
      Message::Keys & mids,
      bool onlyNewMail,
      bool & wait
    );
    void recvMail();
    void removeMail();
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class SpoolWalker : public Fiber {
  public:
    virtual ~SpoolWalker();
    SpoolWalker(Server & server);
  protected:
    DirectoryChangeNotification dcn_;
    void processQueue(bool & timeWait);
    void fiberExecute();
  private:
    Server & server_;
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class MailQueueWalker : public ksock::ClientFiber {
  public:
    virtual ~MailQueueWalker();
    MailQueueWalker(Server & server);
  protected:
    DirectoryChangeNotification dcn_;
    void checkCode(int32_t code,int32_t noThrowCode = eOK);
    void getCode(int32_t noThrowCode = eOK);
    void auth();
    void processQueue(bool & timeWait);
    void main();
  private:
    Server & server_;
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class NodeClient : public ksock::ClientFiber {
  friend class Server;
  public:
    virtual ~NodeClient();
    NodeClient(Server & server);
    static NodeClient * newClient(Server & server,ServerType dataType,const utf8::String & nodeHostName,bool periodicaly);
  protected:
    void checkCode(int32_t code,int32_t noThrowCode = eOK);
    void checkCode(int32_t code,int32_t noThrowCode1,int32_t noThrowCode2);
    int32_t getCode(int32_t noThrowCode = eOK);
    int32_t getCode(int32_t noThrowCode1,int32_t noThrowCode2);
    void auth();
    void sweepHelper(ServerType serverType);
    void main();
  private:
    Server & server_;
    ServerType dataType_;
    utf8::String nodeHostName_;
    bool periodicaly_;
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Server : public ksock::Server {
  friend class ServerFiber;
  friend class SpoolWalker;
  friend class MailQueueWalker;
  friend class NodeClient;
  public:
    virtual ~Server();
    Server(const ConfigSP config);

    void open();
    void close();

    class Data {
      friend class Server;
      friend class ServerFiber;
      friend class SpoolWalker;
      friend class MailQueueWalker;
      friend class NodeClient;
      public:
        ~Data();
        Data();

        FiberMutex & mutex() const;
        uint64_t & stime() const;

        bool registerUserNL(const UserInfo & info,const utf8::String & sendingTo = utf8::String());
        bool registerUser(const UserInfo & info,const utf8::String & sendingTo = utf8::String());
        bool registerKeyNL(const KeyInfo & info,const utf8::String & sendingTo = utf8::String());
        bool registerKey(const KeyInfo & info,const utf8::String & sendingTo = utf8::String());
        bool registerGroupNL(const GroupInfo & info,const utf8::String & sendingTo = utf8::String());
        bool registerGroup(const GroupInfo & info,const utf8::String & sendingTo = utf8::String());
        bool registerServerNL(const ServerInfo & info,const utf8::String & sendingTo = utf8::String());
        bool registerServer(const ServerInfo & info,const utf8::String & sendingTo = utf8::String());
        bool registerUser2KeyLinkNL(const User2KeyLink & link,const utf8::String & sendingTo = utf8::String());
        bool registerUser2KeyLink(const User2KeyLink & link,const utf8::String & sendingTo = utf8::String());
        bool registerKey2GroupLinkNL(const Key2GroupLink & link,const utf8::String & sendingTo = utf8::String());
        bool registerKey2GroupLink(const Key2GroupLink & link,const utf8::String & sendingTo = utf8::String());
        bool registerKey2ServerLinkNL(const Key2ServerLink & link,const utf8::String & sendingTo = utf8::String());
        bool registerKey2ServerLink(const Key2ServerLink & link,const utf8::String & sendingTo = utf8::String());
        void sendDatabaseNL(ksock::AsyncSocket & socket,const utf8::String & sendingTo = utf8::String());
        void sendDatabase(ksock::AsyncSocket & socket,const utf8::String & sendingTo = utf8::String());
        void recvDatabaseNL(ksock::AsyncSocket & socket,const utf8::String & sendingTo = utf8::String());
        void recvDatabase(ksock::AsyncSocket & socket,const utf8::String & sendingTo = utf8::String());
        utf8::String getNodeListNL() const;
        utf8::String getNodeList() const;
        void dumpNL(utf8::String::Stream & stream) const;
        void dump(utf8::String::Stream & stream) const;
        Data & clear();
        bool orNL(const Data & a,const utf8::String & sendingTo = utf8::String());
        bool ore(const Data & a,const utf8::String & sendingTo = utf8::String());
        Data & xorNL(const Data & data1,const Data & data2,const utf8::String & sendingTo = utf8::String());
        Data & xore(const Data & data1,const Data & data2,const utf8::String & sendingTo = utf8::String());
        Data & setSendedToNL(const utf8::String & sendingTo);
        Data & setSendedTo(const utf8::String & sendingTo);
        Data & clearSendedToNL();
        Data & clearSendedTo();
        bool sweepNL(uint64_t stime,uint64_t rtime,utf8::String::Stream * log = NULL);
        bool sweep(uint64_t stime,uint64_t rtime,utf8::String::Stream * log = NULL);
        utf8::String getUserListNL(bool quoted = false) const;
        utf8::String getUserList(bool quoted = false) const;
        utf8::String getKeyListNL(bool quoted = false) const;
        utf8::String getKeyList(bool quoted = false) const;
        utf8::String getKeyGroupListNL(const utf8::String & groups,bool quoted) const;
        utf8::String getKeyGroupList(const utf8::String & groups,bool quoted) const;
        utf8::String getKeyInGroupListNL(const utf8::String & group,bool quoted) const;
        utf8::String getKeyInGroupList(const utf8::String & group,bool quoted) const;
        bool isEmptyNL() const;
        bool isEmpty() const;
      protected:
      private:
        Data(const Data &);
        void operator = (const Data &){}

// last time when database sweep
        mutable uint64_t stime_;
        mutable FiberMutex mutex_;
        typedef EmbeddedHash<
          UserInfo,
          UserInfo::hashNode,
          UserInfo::hashNodeObject,
          UserInfo::hashNodeHash,
          UserInfo::hashNodeEqu
        > Users;
        Users users_;
        AutoHashDrop<Users> usersAutoDrop_;
        typedef EmbeddedHash<
          KeyInfo,
          KeyInfo::hashNode,
          KeyInfo::hashNodeObject,
          KeyInfo::hashNodeHash,
          KeyInfo::hashNodeEqu
        > Keys;
        Keys keys_;
        AutoHashDrop<Keys> keysAutoDrop_;
        typedef EmbeddedHash<
          GroupInfo,
          GroupInfo::hashNode,
          GroupInfo::hashNodeObject,
          GroupInfo::hashNodeHash,
          GroupInfo::hashNodeEqu
        > Groups;
        Groups groups_;
        AutoHashDrop<Groups> groupsAutoDrop_;
        typedef EmbeddedHash<
          ServerInfo,
          ServerInfo::hashNode,
          ServerInfo::hashNodeObject,
          ServerInfo::hashNodeHash,
          ServerInfo::hashNodeEqu
        > Servers;
        Servers servers_;
        AutoHashDrop<Servers> serversAutoDrop_;
        typedef EmbeddedHash<
          User2KeyLink,
          User2KeyLink::hashNode,
          User2KeyLink::hashNodeObject,
          User2KeyLink::hashNodeHash,
          User2KeyLink::hashNodeEqu
        > User2KeyLinks;
        User2KeyLinks user2KeyLinks_;
        AutoHashDrop<User2KeyLinks> user2KeyLinksAutoDrop_;
        typedef EmbeddedHash<
          Key2GroupLink,
          Key2GroupLink::hashNode,
          Key2GroupLink::hashNodeObject,
          Key2GroupLink::hashNodeHash,
          Key2GroupLink::hashNodeEqu
        > Key2GroupLinks;
        Key2GroupLinks key2GroupLinks_;
        AutoHashDrop<Key2GroupLinks> key2GroupLinksAutoDrop_;
        typedef EmbeddedHash<
          Key2ServerLink,
          Key2ServerLink::hashNode,
          Key2ServerLink::hashNodeObject,
          Key2ServerLink::hashNodeHash,
          Key2ServerLink::hashNodeEqu
        > Key2ServerLinks;
        Key2ServerLinks key2ServerLinks_;
        AutoHashDrop<Key2ServerLinks> key2ServerLinksAutoDrop_;
    };
  protected:
    Fiber * newFiber();
    utf8::String spoolDir() const;
    utf8::String mailDir() const;
    utf8::String mqueueDir() const;
    utf8::String lckDir() const;
    utf8::String incompleteDir() const;
    void clearNodeClient(NodeClient * client);
    void startNodeClientNL(ServerType dataType,const utf8::String & nodeHostName = utf8::String());
    void startNodeClient(ServerType dataType,const utf8::String & nodeHostName = utf8::String());
    void startNodesExchangeNL();
    void startNodesExchange();
  private:
    ConfigSP config_;
// база
    Data data_[2];
// misc
    Data & data(ServerType type);
    FiberInterlockedMutex rndMutex_;
    SPEIA<Randomizer,FiberInterlockedMutex> rnd_;
    FiberInterlockedMutex nodeClientMutex_;
    NodeClient * nodeClient_;
    int32_t skippedNodeClientStarts_;
    Array<NodeClient *> nodeExchangeClients_;
    int32_t skippedNodeExchangeStarts_;
    FiberInterlockedMutex recvMailFibersMutex_;
    EmbeddedHash<
      ServerFiber,
      ServerFiber::hashNode,
      ServerFiber::hashNodeObject,
      ServerFiber::hashNodeHash,
      ServerFiber::hashNodeEqu
    > recvMailFibers_;
    void addRecvMailFiber(ServerFiber & fiber);
    bool remRecvMailFiber(ServerFiber & fiber);
    ServerFiber * findRecvMailFiberNL(const ServerFiber & fiber);
    ServerFiber * findRecvMailFiber(const ServerFiber & fiber);
    void sendRobotMessage(
      const utf8::String & recepient,
      const utf8::String & sender,
      const utf8::String & sended,
      const utf8::String & key,
      const utf8::String & value,
      Message * msg = NULL);
    void sendUserWatchdog(const utf8::String & user);
};
//------------------------------------------------------------------------------
inline Server::Data & Server::data(ServerType type)
{
  assert( type == stNode || type == stStandalone );
//  Exception::throwSP(EINVAL,__PRETTY_FUNCTION__);
  return data_[type];
}
//------------------------------------------------------------------------------
inline FiberMutex & Server::Data::mutex() const
{
  return mutex_;
}
//------------------------------------------------------------------------------
inline uint64_t & Server::Data::stime() const
{
  return stime_;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Service : public ksys::Service {
  public:
    Service();

    const ConfigSP & msmailConfig() const;
  protected:
  private:
    ConfigSP msmailConfig_;
    Server msmail_;

    void install();
    void uninstall();
    void start();
    void stop();
    bool active();
};
//------------------------------------------------------------------------------
inline const ConfigSP & Service::msmailConfig() const
{
  return msmailConfig_;
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
