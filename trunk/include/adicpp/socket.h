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
#ifndef _socket_H_
//---------------------------------------------------------------------------
#define _socket_H_
//---------------------------------------------------------------------------
namespace ksock {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
// You not need to use Finite State Machine (FSM)
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
typedef HWND sig_t;
typedef SOCKET sock_t;
inline int32_t errNo(){ return ksock::api.WSAGetLastError() + ksys::errorOffset; }
const EINPROGRESS = WSAEWOULDBLOCK + ksys::errorOffset;
//const EMSGSIZE = WSAEMSGSIZE + ksys::errorOffset;
const ENOTCONN = WSAENOTCONN + ksys::errorOffset;
const ENOTSOCK = WSAENOTSOCK + ksys::errorOffset;
const SHUT_RDWR = SD_BOTH;
#else
typedef int sig_t;
typedef int sock_t;
const int INVALID_SOCKET = -1;
inline int32_t errNo(){ return errno; }
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncSocket :
  public ksys::AsyncDescriptor,
  private ksys::LZO1X,
  private ksys::SHA256Filter
{
  friend class Server;
  public:
    virtual ~AsyncSocket();
    AsyncSocket();
// low level methods
    AsyncSocket & setlinger();

    AsyncSocket & open(int domain = PF_INET,int type = SOCK_STREAM,int protocol = IPPROTO_IP);
    AsyncSocket & close();
    AsyncSocket & shutdown(int how = SHUT_RDWR);

    AsyncSocket & getsockopt(int level,int optname,void * optval,socklen_t & optlen);
    AsyncSocket & setsockopt(int level,int optname,const void * optval,socklen_t optlen);

    AsyncSocket & connect(const SockAddr & addr);
    AsyncSocket & bind(const SockAddr & sockAddr);
    AsyncSocket & listen();
    AsyncSocket & accept(AsyncSocket & socket);

    uint64_t recv(void * buf,uint64_t len);
    uint64_t sysRecv(void * buf,uint64_t len);
    uint64_t send(const void * buf,uint64_t len);
    uint64_t sysSend(const void * buf,uint64_t len);

    AsyncSocket & flush();

// high level methods
    AsyncSocket & read(void * buf,uint64_t len);
    AsyncSocket & write(const void * buf,uint64_t len);

    utf8::String readString();

    AsyncSocket & operator << (int8_t a);
    AsyncSocket & operator << (uint8_t a);
    AsyncSocket & operator << (int16_t a);
    AsyncSocket & operator << (uint16_t a);
    AsyncSocket & operator << (int32_t a);
    AsyncSocket & operator << (uint32_t a);
    AsyncSocket & operator << (int64_t a);
    AsyncSocket & operator << (uint64_t a);
    AsyncSocket & operator << (const utf8::String & s);

    AsyncSocket & operator >> (int8_t & a);
    AsyncSocket & operator >> (uint8_t & a);
    AsyncSocket & operator >> (int16_t & a);
    AsyncSocket & operator >> (uint16_t & a);
    AsyncSocket & operator >> (int32_t & a);
    AsyncSocket & operator >> (uint32_t & a);
    AsyncSocket & operator >> (int64_t & a);
    AsyncSocket & operator >> (uint64_t & a);
    AsyncSocket & operator >> (utf8::String & s);

    AsyncSocket & maxSendSize(uintptr_t mss);
    const uintptr_t & maxSendSize() const;

    enum AuthErrorType {
      aeOK, aeMagic = 2000, aeUser, aePassword,
      aeEncryptionServerRequiredButClientDisabled,
      aeEncryptionServerDisabledButClientRequired,
      aeCompressionServerRequiredButClientDisabled,
      aeCompressionServerDisabledButClientRequired,
      aeCount
    };

    AuthErrorType serverAuth(
      const ksys::HashedObjectList<utf8::String,utf8::String> & usersDatabase,
      const utf8::String & encryption,
      uintptr_t threshold,
      const utf8::String & compression,
      uintptr_t level,
      uintptr_t bufferSize,
      utf8::String & user
    );
    AuthErrorType clientAuth(
      const utf8::String & user,
      const utf8::String & password,
      const utf8::String & encryption,
      uintptr_t threshold,
      const utf8::String & compression,
      uintptr_t levle,
      uintptr_t bufferSize
    );

    AsyncSocket & clearCompressionStatistic();
    AsyncSocket & activateEncryption(const uint8_t sha256[32]);
    AsyncSocket & activateEncryption(const void * key,uintptr_t keyLen);
    AsyncSocket & deActivateEncryption();
    bool encryptionActive() const;
    AsyncSocket & activateCompression(uintptr_t level,uintptr_t wBufSize);
    AsyncSocket & deActivateCompression();
    bool compressionActive() const;

    int64_t rcDifference() const;
    uint64_t rcRatio() const;
    int64_t scDifference() const;
    uint64_t scRatio() const;
    int64_t rscDifference() const;
    uint64_t rscRatio() const;
  protected:
  private:
    static const uint8_t authMagic_[16];
    uintptr_t maxSendSize_;
    uint64_t srb_, nrb_, ssb_, nsb_;

#if defined(__WIN32__) || defined(__WIN64__)
    int WSAEnumNetworkEvents(WSAEVENT hEventObject,DWORD event);

    struct AcceptExBuffer {
      union {
        SockAddr localAddress_;
        uint8_t pLocalAddr4_[sizeof(struct sockaddr_in) + 16];
      };
      union {
        SockAddr remoteAddress_;
        uint8_t pRemoteAddr4_[sizeof(struct sockaddr_in) + 16];
      };
    };
    ksys::AutoPtr<AcceptExBuffer> pAcceptExBuffer_;
    
    BOOL AcceptEx(
      SOCKET sAcceptSocket,
      PVOID lpOutputBuffer,
      DWORD dwReceiveDataLength,
      DWORD dwLocalAddressLength,
      DWORD dwRemoteAddressLength,
      LPDWORD lpdwBytesReceived,
      LPOVERLAPPED lpOverlapped
    );
    
    BOOL Connect(HANDLE event,ksys::IoRequest * request);
    BOOL Read(
      LPVOID lpBuffer,
      DWORD nNumberOfBytesToRead,
      LPDWORD lpNumberOfBytesRead,
      LPOVERLAPPED lpOverlapped
    );
    BOOL Write(
      LPCVOID lpBuffer,
      DWORD nNumberOfBytesToWrite,
      LPDWORD lpNumberOfBytesWritten,
      LPOVERLAPPED lpOverlapped
    );
    BOOL GetOverlappedResult(
      LPOVERLAPPED lpOverlapped,
      LPDWORD lpNumberOfBytesTransferred,
      BOOL bWait,
      LPDWORD lpdwFlags
    );
#elif HAVE_KQUEUE
    int accept();
    void connect(ksys::IoRequest * request);
#endif
    void shutdown2();
    void flush2();
};
//---------------------------------------------------------------------------
inline AsyncSocket::~AsyncSocket()
{
  detach();
  close();
}
//------------------------------------------------------------------------------
inline AsyncSocket::AsyncSocket() : maxSendSize_(~(uintptr_t) 0)
{
  socket_ = INVALID_SOCKET;
#if defined(__WIN32__) || defined(__WIN64__)
  specification_ = 0;
#endif
}
//------------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::setlinger()
{
#if defined(__WIN32__) || defined(__WIN64__)
  struct linger lg;
  lg.l_onoff = 0;
  lg.l_linger = 0;
  setsockopt(SOL_SOCKET,SO_LINGER,&lg,sizeof(lg));
#endif
  return *this;
}
//------------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::activateEncryption(const uint8_t sha256[32])
{
  init(sha256);
  return *this;
}
//------------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::activateEncryption(const void * key,uintptr_t keyLen)
{
  init(key,keyLen);
  return *this;
}
//------------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::deActivateEncryption()
{
  flush();
  ksys::SHA256Filter::active(false);
  return *this;
}
//------------------------------------------------------------------------------
inline bool AsyncSocket::encryptionActive() const
{
  return ksys::SHA256Filter::active();
}
//------------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::activateCompression(uintptr_t level,uintptr_t wBufSize)
{
  ksys::LZO1X::wBufSize(wBufSize > 1024 * 1024 * 1024 ? 1024 * 1024 * 1024 : (uint32_t) wBufSize);
  ksys::LZO1X::level(level);
  ksys::LZO1X::active(true);
  return *this;
}
//------------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::deActivateCompression()
{
  flush();
  ksys::LZO1X::active(false);
  return *this;
}
//------------------------------------------------------------------------------
inline bool AsyncSocket::compressionActive() const
{
  return ksys::LZO1X::active();
}
//------------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator << (int8_t a)
{
  return write(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator << (uint8_t a)
{
  return write(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator << (int16_t a)
{
  return write(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator << (uint16_t a)
{
  return write(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator << (int32_t a)
{
  return write(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator << (uint32_t a)
{
  return write(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator << (int64_t a)
{
  return write(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator << (uint64_t a)
{
  return write(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator << (const utf8::String & s)
{
  assert( s.size() < 2048u * 1024u * 1024u );
  uint32_t l = (uint32_t) s.size();
  return write(&l,sizeof(l)).write(s.c_str(),l);
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >> (int8_t & a)
{
  return read(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >> (uint8_t & a)
{
  return read(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >> (int16_t & a)
{
  return read(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >> (uint16_t & a)
{
  return read(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >> (int32_t & a)
{
  return read(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >> (uint32_t & a)
{
  return read(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >> (int64_t & a)
{
  return read(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >> (uint64_t & a)
{
  return read(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >> (utf8::String & s)
{
  uint32_t l;
  read(&l,sizeof(l));
  s.resize(l);
  return read(s.c_str(),l);
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::maxSendSize(uintptr_t mss)
{
  maxSendSize_ = mss;
  return *this;
}
//---------------------------------------------------------------------------
inline const uintptr_t & AsyncSocket::maxSendSize() const
{
  return maxSendSize_;
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::clearCompressionStatistic()
{
  srb_ = nrb_ = ssb_ = nsb_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline int64_t AsyncSocket::rcDifference() const
{
  return srb_ - nrb_;
}
//---------------------------------------------------------------------------
inline uint64_t AsyncSocket::rcRatio() const
{
  return nrb_ * 10000u / (srb_ + (srb_ == 0));
}
//---------------------------------------------------------------------------
inline int64_t AsyncSocket::scDifference() const
{
  return ssb_ - nsb_;
}
//---------------------------------------------------------------------------
inline uint64_t AsyncSocket::scRatio() const
{
  return nsb_ * 10000u / (ssb_ + (ssb_ == 0));
}
//---------------------------------------------------------------------------
inline int64_t AsyncSocket::rscDifference() const
{
  return (srb_ + ssb_) - (nrb_ + nsb_);
}
//---------------------------------------------------------------------------
inline uint64_t AsyncSocket::rscRatio() const
{
  return (nrb_ + nsb_) * 10000u / (srb_ + ssb_ + (srb_ == 0 && ssb_ == 0));
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ServerFiber : public ksys::BaseFiber, public AsyncSocket {
  friend class AcceptFiber;
  public:
    virtual ~ServerFiber();
    ServerFiber();
  protected:
    ServerFiber & attachDescriptor(ksys::AsyncDescriptor & descriptor);
    virtual void main() = 0;
  private:
    void execute();
};
//---------------------------------------------------------------------------
inline ServerFiber::~ServerFiber()
{
}
//---------------------------------------------------------------------------
inline ServerFiber::ServerFiber()
{
}
//---------------------------------------------------------------------------
inline ServerFiber & ServerFiber::attachDescriptor(ksys::AsyncDescriptor & descriptor)
{
  descriptor.attach(*this,*thread());
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AcceptFiber : public ksys::BaseFiber, public AsyncSocket {
  public:
    virtual ~AcceptFiber();
    AcceptFiber();
  protected:
  private:
    void execute();
};
//---------------------------------------------------------------------------
inline AcceptFiber::~AcceptFiber()
{
}
//---------------------------------------------------------------------------
inline AcceptFiber::AcceptFiber()
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Server : public ksys::BaseServer {
  friend class AcceptFiber;
  friend class ServerFiber;
  public:
    virtual ~Server();
    Server();

    Server & open();
    Server & close();

    Server & clearBind();
    Server & addBind(const SockAddr & addr);
  protected:
    void attachFiber(ksys::BaseFiber & fiber);
  private:
    ksys::Array<SockAddr> bindAddrs_;
    AcceptFiber * acceptFiber_;
    ksys::BaseFiber * afNewFiber();
    void sweepFibers();
};
//------------------------------------------------------------------------------
inline Server::~Server()
{
  close();
}
//------------------------------------------------------------------------------
inline Server::Server() : acceptFiber_(NULL)
{
}
//------------------------------------------------------------------------------
inline Server & Server::clearBind()
{
  bindAddrs_.clear();
  return *this;
}
//------------------------------------------------------------------------------
inline Server & Server::addBind(const SockAddr & addr)
{
  bindAddrs_.add(addr);
  return *this;
}
//------------------------------------------------------------------------------
inline ksys::BaseFiber * Server::afNewFiber()
{
  return newFiber();
}
//------------------------------------------------------------------------------
inline void Server::sweepFibers()
{
  BaseServer::sweepFibers();
}
//------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ClientFiber : public ksys::BaseFiber {
  public:
    virtual ~ClientFiber();
    ClientFiber();
  protected:
    virtual void main() = 0;
    ClientFiber & attachDescriptor(ksys::AsyncDescriptor & descriptor);
  private:
    void execute();
};
//---------------------------------------------------------------------------
inline ClientFiber::~ClientFiber()
{
}
//---------------------------------------------------------------------------
inline ClientFiber::ClientFiber()
{
}
//---------------------------------------------------------------------------
inline ClientFiber & ClientFiber::attachDescriptor(ksys::AsyncDescriptor & descriptor)
{
  descriptor.attach(*this,*thread());
  return *this;
}
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Client : public ksys::BaseServer {
  friend class ClientFiber;
  public:
    virtual ~Client();
    Client();

    Client & open();
    Client & close();
  protected:
    void attachFiber(ksys::BaseFiber & fiber);
  private:
    void execute();
    ksys::BaseFiber * newFiber(){ return NULL; }
};
//------------------------------------------------------------------------------
inline Client::~Client()
{
  close();
}
//------------------------------------------------------------------------------
inline Client::Client()
{
}
//------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EAsyncSocket : public ksys::Exception {
  public:
    EAsyncSocket(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
void initialize();
void cleanup();
//---------------------------------------------------------------------------
} // namespace ksock
//---------------------------------------------------------------------------
#endif /* _socket_H_ */
//---------------------------------------------------------------------------
