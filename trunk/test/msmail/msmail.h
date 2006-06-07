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
  cmRegisterUser,
  cmRegisterKey,
  cmRegisterGroup,
  cmRegisterServer,
  cmRegisterUser2KeyLink,
  cmRegisterKey2GroupLink,
  cmGetUserList,
  cmGetKeyList,
  cmGetGroupList,
  cmGetServerList,
  cmSendMail,
  cmRecvMail
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

    void addRef(){ interlockedIncrement(refCount_,1); }
    UserInfo * remRef(){ return interlockedIncrement(refCount_,-1) == 1 ? this : NULL; }

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

    int32_t refCount_;
    int64_t atime_; // время последнего обращения (для удаления устаревших)
    int64_t mtime_; // время последней регистрации (для передачи обновлений)
    mutable EmbeddedHashNode<UserInfo> hashNode_;
    utf8::String name_;
  protected:
  private:
    UserInfo(const UserInfo &);
    UserInfo & operator = (const UserInfo &);
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
    KeyInfo(const utf8::String & key);

    void addRef(){ interlockedIncrement(refCount_,1); }
    KeyInfo * remRef(){ return interlockedIncrement(refCount_,-1) == 1 ? this : NULL; }

    static EmbeddedHashNode<KeyInfo> & hashNode(const KeyInfo & object){
      return object.hashNode_;
    }
    static KeyInfo & hashNodeObject(const EmbeddedHashNode<KeyInfo> & node,KeyInfo * p){
      return node.object(p->hashNode_);
    }
    static uintptr_t hashNodeHash(const KeyInfo & object){
      return object.key_.hash(true);
    }
    static bool hashNodeEqu(const KeyInfo & object1,const KeyInfo & object2){
      return object1.key_.strcasecmp(object2.key_) == 0;
    }

    int32_t refCount_;
    int64_t atime_; // время последнего обращения (для удаления устаревших)
    int64_t mtime_; // время последней регистрации (для передачи обновлений)
    mutable EmbeddedHashNode<KeyInfo> hashNode_;
    utf8::String key_;
    utf8::String server_;
    Array<utf8::String> users_;
  protected:
  private:
    KeyInfo(const KeyInfo &);
    KeyInfo & operator = (const KeyInfo &);
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

    void addRef(){ interlockedIncrement(refCount_,1); }
    GroupInfo * remRef(){ return interlockedIncrement(refCount_,-1) == 1 ? this : NULL; }

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

    int32_t refCount_;
    int64_t atime_; // время последнего обращения (для удаления устаревших)
    int64_t mtime_; // время последней регистрации (для передачи обновлений)
    mutable EmbeddedHashNode<GroupInfo> hashNode_;
    utf8::String name_;
    Array<utf8::String> keys_;
  protected:
  private:
    GroupInfo(const GroupInfo &);
    GroupInfo & operator = (const GroupInfo &);
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

    void addRef(){ interlockedIncrement(refCount_,1); }
    ServerInfo * remRef(){ return interlockedIncrement(refCount_,-1) == 1 ? this : NULL; }

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

    int32_t refCount_;
    int64_t atime_; // время последнего обращения (для удаления устаревших)
    int64_t mtime_; // время последней регистрации (для передачи обновлений)
    mutable EmbeddedHashNode<ServerInfo> hashNode_;
    utf8::String name_;
    bool node_; // признак того что сервер является узловым
    Array<utf8::String> keys_;
  protected:
  private:
    ServerInfo(const ServerInfo &);
    ServerInfo & operator = (const ServerInfo &);
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

    void putCode(int32_t code);
    void checkCode(int32_t code,int32_t noThrowCode = eOK);
    void getCode(int32_t noThrowCode = eOK);
    void auth();    
    void registerUser();
    void registerKey();
    void registerGroup();
    void registerServer();
    void registerUser2KeyLink();
    void registerKey2GroupLink();
    void getUserList();
    void getKeyList();
    void getGroupList();
    void getServerList();
    void sendMail();
    void recvMail();
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class SpoolWalker : public Fiber {
  public:
    virtual ~SpoolWalker();
    SpoolWalker(Server & server);
  protected:
    void fiberExecute();
  private:
    Server & server_;
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Server : public ksock::Server {
  friend class ServerFiber;
  friend class SpoolWalker;
  public:
    virtual ~Server();
    Server(const ConfigSP config);

    void open();
    void close();

    class Data {
      friend class Server;
      friend class ServerFiber;
      friend class SpoolWalker;
      public:
        ~Data();
        Data();

        template <typename InfoListT>
        void addRef(InfoListT & list)
        {
          AutoLock<FiberInterlockedMutex> lock(mutex_);
          for( intptr_t i = list.count() - 1; i >= 0; i-- ) list[i].addRef();
        }
        template <typename InfoListT,typename InfoHashT>
        void remRef(InfoListT & list,InfoHashT & hash)
        {
          AutoLock<FiberInterlockedMutex> lock(mutex_);
          for( intptr_t i = list.count() - 1; i >= 0; i-- ){
            if( list[i].remRef() != NULL ){
              hash.remove(list[i]);
              list.remove(i);
            }
          }
        }
      protected:
      private:
        FiberInterlockedMutex mutex_;
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
          ServerInfo,
          ServerInfo::hashNode,
          ServerInfo::hashNodeObject,
          ServerInfo::hashNodeHash,
          ServerInfo::hashNodeEqu
        > servers_;
        Vector<ServerInfo> serverList_;
        EmbeddedHash<
          GroupInfo,
          GroupInfo::hashNode,
          GroupInfo::hashNodeObject,
          GroupInfo::hashNodeHash,
          GroupInfo::hashNodeEqu
        > groups_;
        Vector<GroupInfo> groupList_;
    };
  protected:
    ConfigSP config_;
    Fiber * newFiber();
    utf8::String spoolDir() const;
    utf8::String mailDir() const;
    utf8::String mqueueDir() const;
  private:
// списки даных узлового сервера
    Data nodeData_;
// списки даных рядового сервера
    Data standaloneData_;
    Data & data(ServerType type);
    FiberInterlockedMutex rndMutex_;
    SPEIA<Randomizer,FiberInterlockedMutex> rnd_;
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
