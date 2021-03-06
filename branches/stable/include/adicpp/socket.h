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
#ifndef _socket_H_
//---------------------------------------------------------------------------
#define _socket_H_
//---------------------------------------------------------------------------
#define STREAM_FILTER_ABSTRATION
#include <adicpp/stmflt.h>
#undef STREAM_FILTER_ABSTRATION
//---------------------------------------------------------------------------
namespace ksock {
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
typedef HWND sig_t;
inline int32_t errNo()
{
  return ksock::api.WSAGetLastError() + ksys::errorOffset;
}
const int EINPROGRESS = WSAEWOULDBLOCK + ksys::errorOffset;
//const EMSGSIZE = WSAEMSGSIZE + ksys::errorOffset;
const int ENOTCONN = WSAENOTCONN + ksys::errorOffset;
const int ENOTSOCK = WSAENOTSOCK + ksys::errorOffset;
const int EWSANOTINITIALISED = WSANOTINITIALISED + ksys::errorOffset;
const int SHUT_RDWR = SD_BOTH;
const int ECONNABORTED = WSAECONNABORTED + ksys::errorOffset;
#else
typedef int sig_t;
const int EWSANOTINITIALISED = ENOTSOCK;
inline int32_t errNo()
{
  return errno;
}
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncSocket : public ksys::AsyncDescriptor, private ksys::LZO1X, private ksys::SHA256Filter {
  friend class SockAddr;
  friend class Server;
  public:
    virtual ~AsyncSocket();
    AsyncSocket();

    bool isSocket() const;
    // low level methods
    AsyncSocket &     open(int domain = AF_INET,int type = SOCK_STREAM,int protocol = IPPROTO_IP);
    AsyncSocket &     close();
    AsyncSocket &     shutdown(int how = SHUT_RDWR);

    AsyncSocket &     getsockopt(int level, int optname, void * optval, socklen_t & optlen);
    AsyncSocket &     setsockopt(int level, int optname, const void * optval, socklen_t optlen);

    AsyncSocket &     connect(const SockAddr & addr);
    AsyncSocket &     bind(const SockAddr & sockAddr);
    AsyncSocket &     listen();
    AsyncSocket &     accept(AsyncSocket & socket);

    uint64_t          recv(void * buf, uint64_t len);
    uint64_t          sysRecv(void * buf, uint64_t len);
    uint64_t          send(const void * buf, uint64_t len);
    uint64_t          sysSend(const void * buf, uint64_t len);

    AsyncSocket &     flush();

    AsyncSocket & getSockAddr(SockAddr & addr) const;
    AsyncSocket & getPeerAddr(SockAddr & addr) const;

    // high level methods
    int64_t readV(void * buf,uint64_t size){ return sysRecv(buf,size); }
    int64_t writeV(const void * buf,uint64_t size){ return sysSend(buf,size); }

    AsyncSocket &     read(void * buf, uint64_t len);
    AsyncSocket &     readBuffer(void * buf, uint64_t len);
    AsyncSocket &     write(const void * buf, uint64_t len);
    AsyncSocket &     writeBuffer(const void * buf, uint64_t len);

    utf8::String      readString();

    AsyncSocket &     operator <<(bool a);
    AsyncSocket &     operator <<(int8_t a);
    AsyncSocket &     operator <<(uint8_t a);
    AsyncSocket &     operator <<(int16_t a);
    AsyncSocket &     operator <<(uint16_t a);
    AsyncSocket &     operator <<(int32_t a);
    AsyncSocket &     operator <<(uint32_t a);
    AsyncSocket &     operator <<(int64_t a);
    AsyncSocket &     operator <<(uint64_t a);
    AsyncSocket &     operator <<(const utf8::String & s);

    AsyncSocket &     operator >>(bool & a);
    AsyncSocket &     operator >>(int8_t & a);
    AsyncSocket &     operator >>(uint8_t & a);
    AsyncSocket &     operator >>(int16_t & a);
    AsyncSocket &     operator >>(uint16_t & a);
    AsyncSocket &     operator >>(int32_t & a);
    AsyncSocket &     operator >>(uint32_t & a);
    AsyncSocket &     operator >>(int64_t & a);
    AsyncSocket &     operator >>(uint64_t & a);
    AsyncSocket &     operator >>(utf8::String & s);

    AsyncSocket & maxRecvSize(uintptr_t mrs);
    const uintptr_t & maxRecvSize() const;
    AsyncSocket & maxSendSize(uintptr_t mss);
    const uintptr_t & maxSendSize() const;

    AsyncSocket & recvTimeout(uint64_t a);
    const uint64_t & recvTimeout() const;
    AsyncSocket & sendTimeout(uint64_t a);
    const uint64_t & sendTimeout() const;

    enum AuthErrorType { 
      aeOK,
      aeMagic = 2000,
      aeUser,
      aePassword,
      aeEncryptionServerRequiredButClientDisabled,
      aeEncryptionServerDisabledButClientRequired,
      aeCompressionServerRequiredButClientDisabled,
      aeCompressionServerDisabledButClientRequired,
      aeCount
    };

    class AuthParams {
      public:
        virtual ~AuthParams();
        AuthParams();

        AuthParams & param(const utf8::String & name,const ksys::Mutant & value);
        ksys::Mutant & param(const utf8::String & name) const;

        mutable utf8::String user_;
        mutable utf8::String password_;
        mutable utf8::String encryption_;
        mutable utf8::String compression_;
        mutable utf8::String compressionType_;
        mutable utf8::String crc_;
        mutable uintptr_t maxRecvSize_;
        mutable uintptr_t maxSendSize_;
        mutable uint64_t recvTimeout_;
        mutable uint64_t sendTimeout_;
        mutable uintptr_t threshold_;
        mutable uintptr_t level_;
        mutable uintptr_t bufferSize_;
        mutable bool optimize_;
        mutable bool noAuth_;
    };

    AuthErrorType serverAuth(const AuthParams & ap);
    AuthErrorType clientAuth(const AuthParams & ap);

    AsyncSocket & clearStatistic();
    AsyncSocket & activateEncryption(const void * key,uintptr_t keyLen);
    AsyncSocket & deActivateEncryption();
    bool encryptionActive() const;
    AsyncSocket & activateCompression(uintptr_t method,uintptr_t crc,uintptr_t level,bool optimize,uintptr_t wBufSize);
    AsyncSocket & deActivateCompression();
    bool compressionActive() const;

    uint64_t recvBytes() const;
    uint64_t sendBytes() const;
    uint64_t allBytes() const;
    int64_t rcDifference() const;
    uint64_t rcRatio() const;
    int64_t scDifference() const;
    uint64_t scRatio() const;
    int64_t rscDifference() const;
    uint64_t rscRatio() const;

    const SockAddr & remoteAddress() const;
  protected:
    virtual bool isValidUser(const utf8::String & /*user*/){ return false; }
    virtual utf8::String getUserPassword(const utf8::String & /*user*/,const AuthParams * /*ap*/){ return utf8::String(); }
    ksys::AutoPtr<SockAddr,AutoPtrNonVirtualClassDestructor> remoteAddress_; // client address which accept returns
  private:
    static const uint8_t authMagic_[16];
    static const uint8_t authMagic2_[16];
    uintptr_t maxRecvSize_;
    uintptr_t maxSendSize_;
    uint64_t recvTimeout_;
    uint64_t sendTimeout_;
    uint64_t srb_, nrb_, ssb_, nsb_;

    ksys::AutoPtr<ksys::StreamCRCFilter> crc_;
    ksys::AutoPtr<ksys::StreamCryptFilter> cryptor_;
    ksys::AutoPtr<ksys::StreamCompressionFilter> compressor_;

#if defined(__WIN32__) || defined(__WIN64__)
    int WSAEnumNetworkEvents(WSAEVENT hEventObject,DWORD event);

    class AcceptExBuffer {
      public:
        uint8_t pLocalAddr4_[sizeof(struct sockaddr_in) + 16];
        uint8_t pRemoteAddr4_[sizeof(struct sockaddr_in) + 16];
    };
    ksys::AutoPtr<AcceptExBuffer,AutoPtrNonVirtualClassDestructor> pAcceptExBuffer_;

    BOOL    AcceptEx(SOCKET sAcceptSocket, PVOID lpOutputBuffer, DWORD dwReceiveDataLength, DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, LPDWORD lpdwBytesReceived, LPOVERLAPPED lpOverlapped);

    BOOL    Connect(HANDLE event, ksys::AsyncEvent * request);
    BOOL    Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
    BOOL    Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
    BOOL    GetOverlappedResult(LPOVERLAPPED lpOverlapped, LPDWORD lpNumberOfBytesTransferred, BOOL bWait, LPDWORD lpdwFlags);
#else
    int     listen(int);
    int     accept();
    void    connect(ksys::AsyncEvent * request);
    int64_t read2(void * buf, uint64_t len);
    int64_t write2(const void * buf, uint64_t len);
#endif
    void shutdown2();
    void flush2();
    void close2();
};
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::readBuffer(void * buf,uint64_t len)
{
  return read(buf,len);
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::writeBuffer(const void * buf,uint64_t len)
{
  return write(buf,len);
}
//---------------------------------------------------------------------------
inline bool AsyncSocket::encryptionActive() const
{
  return ksys::SHA256Filter::active();
}
//------------------------------------------------------------------------------
inline bool AsyncSocket::compressionActive() const
{
  return ksys::LZO1X::active();
}
//------------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator <<(bool a)
{ // portable accross network
  uint8_t v = uint8_t(a ? 1 : 0);
  return write(&v,sizeof(v));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator <<(int8_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator <<(uint8_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator <<(int16_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator <<(uint16_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator <<(int32_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator <<(uint32_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator <<(int64_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator <<(uint64_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >>(bool & a)
{
  uint8_t v;
  read(&v,sizeof(v));
  a = v != 0 ? true : false;
  return *this;
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >>(int8_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >>(uint8_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >>(int16_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >>(uint16_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >>(int32_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >>(uint32_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >>(int64_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::operator >>(uint64_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::recvTimeout(uint64_t a)
{
  recvTimeout_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const uint64_t & AsyncSocket::recvTimeout() const
{
  return recvTimeout_;
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::sendTimeout(uint64_t a)
{
  sendTimeout_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const uint64_t & AsyncSocket::sendTimeout() const
{
  return sendTimeout_;
}
//---------------------------------------------------------------------------
inline AsyncSocket & AsyncSocket::maxRecvSize(uintptr_t mrs)
{
  maxRecvSize_ = mrs;
  return *this;
}
//---------------------------------------------------------------------------
inline const uintptr_t & AsyncSocket::maxRecvSize() const
{
  return maxRecvSize_;
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
inline AsyncSocket & AsyncSocket::clearStatistic()
{
  srb_ = nrb_ = ssb_ = nsb_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline uint64_t AsyncSocket::recvBytes() const
{
  return nrb_;
}
//---------------------------------------------------------------------------
inline uint64_t AsyncSocket::sendBytes() const
{
  return nsb_;
}
//---------------------------------------------------------------------------
inline uint64_t AsyncSocket::allBytes() const
{
  return nrb_ + nsb_;
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
class ServerFiber : virtual public ksys::Fiber, virtual public AsyncSocket {
  friend class AcceptFiber;
  public:
    virtual ~ServerFiber();
    ServerFiber();
  protected:
    virtual void main() = 0;
  private:
    void fiberExecute();
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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AcceptFiber : virtual public ksys::Fiber, virtual public AsyncSocket {
  friend class Server;
  public:
    virtual ~AcceptFiber();
    AcceptFiber();
  protected:
  private:
    void fiberExecute();
    void fiberBreakExecution();
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Server : virtual public ksys::BaseServer {
  friend class AcceptFiber;
  friend class ServerFiber;
  public:
    virtual ~Server();
    Server();

    void open();
    void close();

    Server & clearBind();
    Server & addBind(const SockAddr & addr);
    const ksys::Array<SockAddr> & bindAddrs() const;
    const AcceptFiber * acceptFiber() const;
  protected:
  private:
    ksys::Array<SockAddr> bindAddrs_;
    AcceptFiber * acceptFiber_;
};
//------------------------------------------------------------------------------
inline const ksys::Array<SockAddr> & Server::bindAddrs() const
{
  return bindAddrs_;
}
//------------------------------------------------------------------------------
inline const AcceptFiber * Server::acceptFiber() const
{
  return acceptFiber_;
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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ClientFiber : virtual public ksys::Fiber, virtual public AsyncSocket {
  public:
    virtual ~ClientFiber();
    ClientFiber();
  protected:
    virtual void main() = 0;
  private:
    void fiberExecute();
};
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Client : virtual public ksys::BaseServer {
  friend class ClientFiber;
  public:
    virtual ~Client();
    Client();

    void open();
    void close();
  protected:
  private:
    ksys::Fiber * newFiber(){ return NULL; }
};
//------------------------------------------------------------------------------
inline Client::~Client()
{
}
//------------------------------------------------------------------------------
inline Client::Client()
{
  howCloseServer(csWait | csDWM);
}
//------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EAsyncSocket : public ksys::Exception {
  public:
    EAsyncSocket() {}
    EAsyncSocket(int32_t code,const char * what);
    EAsyncSocket(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EAsyncSocket::EAsyncSocket(int32_t code,const char * what) : ksys::Exception(code,what)
{
}
//---------------------------------------------------------------------------
inline EAsyncSocket::EAsyncSocket(int32_t code,const utf8::String & what) : ksys::Exception(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
void initialize();
void cleanup();
//---------------------------------------------------------------------------
} // namespace ksock
//---------------------------------------------------------------------------
#endif /* _socket_H_ */
//---------------------------------------------------------------------------
