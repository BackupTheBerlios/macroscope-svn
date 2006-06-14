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
#ifndef _msmail_H_
#define _msmail_H_
//------------------------------------------------------------------------------
#define _VERSION_H_AS_HEADER_
extern "C" {
#if __BCPLUSPLUS__
typedef struct {
  int   v_hex;
  const char *v_short;
  const char *v_long;
  const char *v_tex;
  const char *v_gnu;
  const char *v_web;
  const char *v_sccs;
  const char *v_rcs;
} msmail_version_t;
extern msmail_version_t msmail_version;
#define _VERSION_H_
#endif
#if _MSC_VER
#pragma warning(push,3)
#endif
#include "version.h"
#if _MSC_VER
#pragma warning(pop)
#endif
};
#undef _VERSION_H_AS_HEADER_
//------------------------------------------------------------------------------
using namespace ksys;
//------------------------------------------------------------------------------
namespace msmail {
//------------------------------------------------------------------------------
const int defaultPort = 2525;
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
  cmRemoveMail
};
//------------------------------------------------------------------------------
enum ServerType {
  stNone,
  stNode,
  stStandalone,
  stCount
};
//------------------------------------------------------------------------------
enum Error {
  eOK,
  eFail = ksock::AsyncSocket::aeCount, // 2007
  eInvalidCommand,
  eInvalidServerType,
  eInvalidMessage,
  eLastMessage,
  eInvalidMessageId,
  eCount
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
extern const char messageIdKey[];
class Message {
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,Message & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const Message & a);
  public:
    ~Message();
    Message();
    Message(const utf8::String & sid);
    Message(const Message & a);
    Message & operator = (const Message & a);

    class Attribute {
      public:
        ~Attribute(){}
        Attribute(){}
        Attribute(const Attribute & a) : key_(a.key_), value_(a.value_) {}
        Attribute(const utf8::String & key,const utf8::String & value) : key_(key), value_(value) {}
        Attribute & operator = (const Attribute & a){
          key_ = a.key_;
          value_ = a.value_;
          return *this;
        }
        bool operator == (const Attribute & a) const { return key_.strcmp(a.key_) == 0; }
        bool operator >= (const Attribute & a) const { return key_.strcmp(a.key_) >= 0; }
        bool operator >  (const Attribute & a) const { return key_.strcmp(a.key_) >  0; }
        bool operator <= (const Attribute & a) const { return key_.strcmp(a.key_) <= 0; }
        bool operator <  (const Attribute & a) const { return key_.strcmp(a.key_) <  0; }

        utf8::String key_;
        utf8::String value_;
      protected:
      private:
    };

    bool operator == (const Message & a) const { return id().strcmp(a.id()) == 0; }
    bool operator >= (const Message & a) const { return id().strcmp(a.id()) >= 0; }
    bool operator >  (const Message & a) const { return id().strcmp(a.id()) >  0; }
    bool operator <= (const Message & a) const { return id().strcmp(a.id()) <= 0; }
    bool operator <  (const Message & a) const { return id().strcmp(a.id()) <  0; }

    const utf8::String & id() const;
    const utf8::String & value(const utf8::String & key) const;
    bool isValue(const utf8::String & key) const;
    Message & value(const utf8::String & key,const utf8::String & value);
    const Array<Attribute> & attributes() const;
    utf8::String separateValue(const utf8::String & key,utf8::String & s0,utf8::String & s1,const utf8::String & separator = "@") const;

/*    static EmbeddedHashNode<Message> & idNode(const Message & object){
      return object.idNode_;
    }
    static Message & idNodeObject(const EmbeddedHashNode<Message> & node,Message * p){
      return node.object(p->idNode_);
    }
    static uintptr_t idNodeHash(const Message & object){
      return object.id().hash(false);
    }
    static bool hashNodeEqu(const Message & object1,const Message & object2){
      return object1.id().strcasecmp(object2.id()) == 0;
    }
    mutable EmbeddedHashNode<Message> idNode_;*/
  protected:
  private:
    mutable Array<Attribute> attributes_;
};
//------------------------------------------------------------------------------
inline const utf8::String & Message::id() const
{
  return value(messageIdKey);
}
//------------------------------------------------------------------------------
inline const Array<Message::Attribute> & Message::attributes() const
{
  return attributes_;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class UserInfo {
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,UserInfo & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const UserInfo & a);
  public:
    ~UserInfo();
    UserInfo();
    UserInfo(const utf8::String & name);
    UserInfo(const UserInfo &);
    UserInfo & operator = (const UserInfo &);

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

    int64_t atime_; // время последнего обращения (для удаления устаревших)
    int64_t mtime_; // время последней регистрации (для передачи обновлений)
    mutable EmbeddedHashNode<UserInfo> hashNode_;
    utf8::String name_;
  protected:
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class KeyInfo {
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,KeyInfo & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const KeyInfo & a);
  public:
    ~KeyInfo();
    KeyInfo();
    KeyInfo(const utf8::String & name);
    KeyInfo(const KeyInfo &);
    KeyInfo & operator = (const KeyInfo &);

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

    int64_t atime_; // время последнего обращения (для удаления устаревших)
    int64_t mtime_; // время последней регистрации (для передачи обновлений)
    mutable EmbeddedHashNode<KeyInfo> hashNode_;
    utf8::String name_;
  protected:
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class GroupInfo {
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,GroupInfo & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const GroupInfo & a);
  friend AsyncFile & operator >> (AsyncFile & s,Message & a);
  friend AsyncFile & operator << (AsyncFile & s,const Message & a);
  public:
    ~GroupInfo();
    GroupInfo();
    GroupInfo(const utf8::String & name);
    GroupInfo(const GroupInfo &);
    GroupInfo & operator = (const GroupInfo &);

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

    int64_t atime_; // время последнего обращения (для удаления устаревших)
    int64_t mtime_; // время последней регистрации (для передачи обновлений)
    mutable EmbeddedHashNode<GroupInfo> hashNode_;
    utf8::String name_;
  protected:
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class ServerInfo {
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,ServerInfo & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const ServerInfo & a);
  public:
    ~ServerInfo();
    ServerInfo();
    ServerInfo(const utf8::String & name);
    ServerInfo(const ServerInfo &);
    ServerInfo & operator = (const ServerInfo &);

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

    int64_t atime_; // время последнего обращения (для удаления устаревших)
    int64_t mtime_; // время последней регистрации (для передачи обновлений)
    mutable EmbeddedHashNode<ServerInfo> hashNode_;
    utf8::String name_;
    bool node_; // признак того что сервер является узловым
  protected:
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class User2KeyLink {
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,User2KeyLink & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const User2KeyLink & a);
  public:
    ~User2KeyLink();
    User2KeyLink();
    User2KeyLink(const utf8::String & userName,const utf8::String & keyName);
    User2KeyLink(const User2KeyLink &);
    User2KeyLink & operator = (const User2KeyLink &);

    static EmbeddedHashNode<User2KeyLink> & hashNode(const User2KeyLink & object){
      return object.hashNode_;
    }
    static User2KeyLink & hashNodeObject(const EmbeddedHashNode<User2KeyLink> & node,User2KeyLink * p){
      return node.object(p->hashNode_);
    }
    static uintptr_t hashNodeHash(const User2KeyLink & object){
      uintptr_t h[2];
      h[0] = object.user_.hash(false);
      h[1] = object.key_.hash(false);
      return HF::hash(h,sizeof(h));
    }
    static bool hashNodeEqu(const User2KeyLink & object1,const User2KeyLink & object2){
      return object1.user_.strcasecmp(object2.user_) == 0 && object1.key_.strcasecmp(object2.key_) == 0;
    }

    int64_t atime_; // время последнего обращения (для удаления устаревших)
    int64_t mtime_; // время последней регистрации (для передачи обновлений)
    mutable EmbeddedHashNode<User2KeyLink> hashNode_;
    utf8::String user_;
    utf8::String key_;
  protected:
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Key2GroupLink {
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,Key2GroupLink & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const Key2GroupLink & a);
  public:
    ~Key2GroupLink();
    Key2GroupLink();
    Key2GroupLink(const utf8::String & keyName,const utf8::String & groupName);
    Key2GroupLink(const Key2GroupLink &);
    Key2GroupLink & operator = (const Key2GroupLink &);

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

    int64_t atime_; // время последнего обращения (для удаления устаревших)
    int64_t mtime_; // время последней регистрации (для передачи обновлений)
    mutable EmbeddedHashNode<Key2GroupLink> hashNode_;
    utf8::String key_;
    utf8::String group_;
  protected:
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Key2ServerLink {
  friend ksock::AsyncSocket & operator >> (ksock::AsyncSocket & s,Key2ServerLink & a);
  friend ksock::AsyncSocket & operator << (ksock::AsyncSocket & s,const Key2ServerLink & a);
  public:
    ~Key2ServerLink();
    Key2ServerLink();
    Key2ServerLink(const utf8::String & keyName,const utf8::String & serverName = utf8::String());
    Key2ServerLink(const Key2ServerLink &);
    Key2ServerLink & operator = (const Key2ServerLink &);

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

    int64_t atime_; // время последнего обращения (для удаления устаревших)
    int64_t mtime_; // время последней регистрации (для передачи обновлений)
    mutable EmbeddedHashNode<Key2ServerLink> hashNode_;
    utf8::String key_;
    utf8::String server_;
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
    ServerFiber(Server & server);
  protected:
    bool isValidUser(const utf8::String & user);
    utf8::String getUserPassword(const utf8::String & user);
    void main();
  private:
    Server & server_;
    utf8::String user_;
    ServerType serverType_;
    DirectoryChangeNotification dcn_;

    void putCode(int32_t code);
    void checkCode(int32_t code,int32_t noThrowCode = eOK);
    void getCode(int32_t noThrowCode = eOK);
    void auth();    
    void registerClient();
    void registerDB();
    void getDB();
    void sendMail();
    intptr_t processMailbox(
      const utf8::String & userMailBox,
      const utf8::String & mailForUser,
      const utf8::String & mailForKey,
      Array<utf8::String> & mids,
      uint8_t onlyNewMail
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
    intptr_t processQueue();
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
    intptr_t processQueue();
    void main();
  private:
    Server & server_;
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class NodeClient : public ksock::ClientFiber {
  public:
    virtual ~NodeClient();
    NodeClient(Server & server);
  protected:
    void checkCode(int32_t code,int32_t noThrowCode = eOK);
    void getCode(int32_t noThrowCode = eOK);
    void auth();
    void clearNodeClient();
    void main();
  private:
    Server & server_;
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

        void registerUserNL(const UserInfo & info);
        void registerUser(const UserInfo & info);
        void registerKeyNL(const KeyInfo & info);
        void registerKey(const KeyInfo & info);
        void registerGroupNL(const GroupInfo & info);
        void registerGroup(const GroupInfo & info);
        void registerServerNL(const ServerInfo & info);
        void registerServer(const ServerInfo & info);
        void registerUser2KeyLinkNL(const User2KeyLink & link);
        void registerUser2KeyLink(const User2KeyLink & link);
        void registerKey2GroupLinkNL(const Key2GroupLink & link);
        void registerKey2GroupLink(const Key2GroupLink & link);
        void registerKey2ServerLinkNL(const Key2ServerLink & link);
        void registerKey2ServerLink(const Key2ServerLink & link);
        void sendDatabaseNL(ksock::AsyncSocket & socket);
        void sendDatabase(ksock::AsyncSocket & socket);
        void recvDatabaseNL(ksock::AsyncSocket & socket);
        void recvDatabase(ksock::AsyncSocket & socket);
        void intersectionNL(const Data & a);
        void intersection(const Data & a);
      protected:
      private:
        int64_t ftime_; // last time when database flushed to node
        mutable FiberMutex mutex_;
        EmbeddedHash<
          UserInfo,
          UserInfo::hashNode,
          UserInfo::hashNodeObject,
          UserInfo::hashNodeHash,
          UserInfo::hashNodeEqu
        > users_;
        Vector<UserInfo> userList_;
        EmbeddedHash<
          KeyInfo,
          KeyInfo::hashNode,
          KeyInfo::hashNodeObject,
          KeyInfo::hashNodeHash,
          KeyInfo::hashNodeEqu
        > keys_;
        Vector<KeyInfo> keyList_;
        EmbeddedHash<
          GroupInfo,
          GroupInfo::hashNode,
          GroupInfo::hashNodeObject,
          GroupInfo::hashNodeHash,
          GroupInfo::hashNodeEqu
        > groups_;
        Vector<GroupInfo> groupList_;
        EmbeddedHash<
          ServerInfo,
          ServerInfo::hashNode,
          ServerInfo::hashNodeObject,
          ServerInfo::hashNodeHash,
          ServerInfo::hashNodeEqu
        > servers_;
        Vector<ServerInfo> serverList_;
        EmbeddedHash<
          User2KeyLink,
          User2KeyLink::hashNode,
          User2KeyLink::hashNodeObject,
          User2KeyLink::hashNodeHash,
          User2KeyLink::hashNodeEqu
        > user2KeyLinks_;
        Vector<User2KeyLink> user2KeyLinkList_;
        EmbeddedHash<
          Key2GroupLink,
          Key2GroupLink::hashNode,
          Key2GroupLink::hashNodeObject,
          Key2GroupLink::hashNodeHash,
          Key2GroupLink::hashNodeEqu
        > key2GroupLinks_;
        Vector<Key2GroupLink> key2GroupLinkList_;
        EmbeddedHash<
          Key2ServerLink,
          Key2ServerLink::hashNode,
          Key2ServerLink::hashNodeObject,
          Key2ServerLink::hashNodeHash,
          Key2ServerLink::hashNodeEqu
        > key2ServerLinks_;
        Vector<Key2ServerLink> key2ServerLinkList_;
    };
  protected:
    ConfigSP config_;
    Fiber * newFiber();
    void maintainFiber(Fiber * fiber);
    utf8::String spoolDir() const;
    utf8::String mailDir() const;
    utf8::String mqueueDir() const;
    utf8::String lckDir() const;
    void startNodeClient();
  private:
// списки даных узлового сервера
    Data nodeData_;
// списки даных рядового сервера
    Data standaloneData_;
    Data & data(ServerType type);
    FiberInterlockedMutex rndMutex_;
    SPEIA<Randomizer,FiberInterlockedMutex> rnd_;
    FiberInterlockedMutex nodeClientMutex_;
    NodeClient * nodeClient_;
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Service : public ksys::Service {
  public:
    Service();
  protected:
  private:
    ConfigSP msmailConfig_;
    Server msmail_;

    void start();
    void stop();
    bool active();
};
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
