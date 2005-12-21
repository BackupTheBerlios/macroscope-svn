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
#include <adicpp/ksys.h>
//------------------------------------------------------------------------------
namespace ksock {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
/*#elif HAVE_SIGNAL_H
  int32_t err;

  if( fcntl(descriptor_,F_SETOWN,getpid()) < 0 ) goto l1;
#ifdef F_SETSIG
  if( fcntl(descriptor_,F_SETSIG,sig_) < 0 ) goto l1;
#endif
  if( fcntl(descriptor_,F_SETFL,fcntl(descriptor_,F_GETFL) | O_NONBLOCK | O_ASYNC) < 0 ) goto l1;
#ifdef O_ONESIGFD
  if( fcntl(descriptor_,F_SETAUXFL,O_ONESIGFD) < 0 ) goto l1;
#endif
l2:;
#endif*/
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::open(int domain, int type, int protocol)
{
  int32_t err;
  if( socket_ == INVALID_SOCKET ){
    clearCompressionStatistic();
    api.open();
#if defined(__WIN32__) || defined(__WIN64__)
//  WSAPROTOCOL_INFO protocolInfo;
//  memset(&protocolInfo,0,sizeof(protocolInfo));
//  protocolInfo.dwMessageSize = 1;
    if( ksys::isWin9x() )
      socket_ = api.WSASocketA(domain,type,protocol,NULL,0,0);
    else
      socket_ = api.WSASocketW(domain,type,protocol,NULL,0,WSA_FLAG_OVERLAPPED);
    if( socket_ == INVALID_SOCKET ){
#else
    if( (socket_ = api.socket(domain,type,protocol)) == INVALID_SOCKET ){
#endif
      err = errNo();
      api.close();
      throw ksys::ExceptionSP(new EAsyncSocket(err,utf8::string(__PRETTY_FUNCTION__)));
    }
#if defined(__WIN32__) || defined(__WIN64__)
    int bufLen = 0;
    setsockopt(SOL_SOCKET,SO_RCVBUF,&bufLen,sizeof(bufLen));
    setsockopt(SOL_SOCKET,SO_SNDBUF,&bufLen,sizeof(bufLen));
#else
    if( fcntl(socket_,F_SETFL,fcntl(socket_,F_GETFL,0) | O_NONBLOCK | O_DIRECT) != 0 ){
      err = errno;
      throw ksys::ExceptionSP(new EAsyncSocket(err,utf8::string(__PRETTY_FUNCTION__)));
    }
    int ka = true;
    setsockopt(SOL_SOCKET,SO_KEEPALIVE,&ka,sizeof(ka));
#endif
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::close()
{
  if( socket_ != INVALID_SOCKET ){
    if( api.closesocket(socket_) != 0 ){
      int32_t err = errNo();
      throw ksys::ExceptionSP(new EAsyncSocket(err,utf8::string(__PRETTY_FUNCTION__)));
    }
    api.close();
    socket_ = INVALID_SOCKET;
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::shutdown(int how)
{
  if( socket_ != INVALID_SOCKET ){
    if( api.shutdown(socket_,how) != 0 ){
      int32_t err = errNo();
      if( err != ENOTCONN && err != ENOTSOCK )
        throw ksys::ExceptionSP(new EAsyncSocket(err,utf8::string(__PRETTY_FUNCTION__)));
    }
    ksys::AsyncIoSlave * ioThread =
#ifndef NDEBUG
      dynamic_cast<ksys::AsyncIoSlave *>
#else
      static_cast<ksys::AsyncIoSlave *>
#endif
    (fiber()->io().ioThread_);
    if( ioThread != NULL ) ioThread->cancelEvent(fiber()->io());
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::getsockopt(int level,int optname,void * optval,socklen_t & optlen)
{
  if( api.getsockopt(socket_,level,optname,(char *) optval,&optlen) != 0 ){
    int32_t err = errNo();
    throw ksys::ExceptionSP(new EAsyncSocket(err,utf8::string(__PRETTY_FUNCTION__)));
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::setsockopt(int level,int optname,const void * optval,socklen_t optlen)
{
  if( api.setsockopt(socket_,level,optname,(const char *) optval,optlen) != 0 ){
    int32_t err = errNo();
    throw ksys::ExceptionSP(new EAsyncSocket(err,utf8::string(__PRETTY_FUNCTION__)));
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::bind(const SockAddr & sockAddr)
{
  if( api.bind(socket_,(struct sockaddr *) &sockAddr.addr4_,(socklen_t) sockAddr.length()) != 0 ){
    int32_t err = errNo();
    throw ksys::ExceptionSP(new EAsyncSocket(err,utf8::string(__PRETTY_FUNCTION__)));
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::listen()
{
#if defined(__WIN32__) || defined(__WIN64__)
  pAcceptExBuffer_.realloc(sizeof(AcceptExBuffer));
#endif
  if( api.listen(socket_,SOMAXCONN) != 0 ){
    int32_t err = errNo();
    throw ksys::ExceptionSP(new EAsyncSocket(err,utf8::string(__PRETTY_FUNCTION__)));
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::accept(AsyncSocket & socket)
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( socket.socket_ == INVALID_SOCKET ){
    socket.open();
    cluster()->postIoRequest(this,0,(void *) socket.socket_,0,ksys::etAccept);
  }
  fiber()->switchFiber(fiber()->mainFiber());
  assert( fiber()->event().event_ == ksys::etAccept );
  if( fiber()->event().errno_ != 0 ){
    socket.close();
    throw ksys::ExceptionSP(
      new EAsyncSocket(
        fiber()->event().errno_ + ksys::errorOffset,utf8::string(__PRETTY_FUNCTION__)
      )
    );
  }
///      LPSOCKADDR plsa, prsa;
//      INT lsaLen, rsaLen;
//      apiEx.GetAcceptExSockaddrs(
//        pAcceptExBuffer_,
//        0,
//        sizeof(pAcceptExBuffer_->pLocalAddr4_),
//        sizeof(pAcceptExBuffer_->pRemoteAddr4_),
//        &plsa,
//        &lsaLen,
//        &prsa,
//        &rsaLen
//      );
//      assert(
//        lsaLen == sizeof(pAcceptExBuffer_->localAddress_.addr4_) && 
//        rsaLen == sizeof(pAcceptExBuffer_->remoteAddress_.addr4_)
//      );
//      memmove(&socket.localAddress_.addr4_,plsa,sizeof(socket.localAddress_.addr4_));
//      memmove(&socket.remoteAddress_.addr4_,prsa,sizeof(socket.remoteAddress_.addr4_));
  }
#elif HAVE_KQUEUE
  if( socket.socket_ == INVALID_SOCKET )
    cluster()->postIoRequest(this,0,0,0,ksys::etAccept);
  fiber()->switchFiber(fiber()->mainFiber());
  assert( fiber()->event().event_ == ksys::etAccept );
  if( fiber()->event().errno_ != 0 )
    throw ksys::ExceptionSP(
      new EAsyncSocket(
        fiber()->event().errno_ + ksys::errorOffset,utf8::string(__PRETTY_FUNCTION__)
      )
    );
  socket.socket_ = (int) fiber()->event().data_;
#endif
  return *this;
}
//------------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option push -w-8004
#endif
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::connect(const SockAddr & addr)
{
  cluster()->postIoRequest(this,addr);
  fiber()->switchFiber(fiber()->mainFiber());
  assert( fiber()->event().event_ == ksys::etConnect );
  if( fiber()->event().errno_ != 0 )
    throw ksys::ExceptionSP(
      new EAsyncSocket(
        fiber()->event().errno_ + ksys::errorOffset,utf8::string(__PRETTY_FUNCTION__)
      )
    );
  return *this;
}
//------------------------------------------------------------------------------
uint64_t AsyncSocket::sysRecv(void * buf,uint64_t len)
{
  uint64_t r = 0;
#if HAVE_KQUEUE
l1:
#endif
  cluster()->postIoRequest(this,0,buf,len,ksys::etRead);
  fiber()->switchFiber(fiber()->mainFiber());
  assert( fiber()->event().event_ == ksys::etRead );
#if defined(__WIN32__) || defined(__WIN64__)
  if( fiber()->event().errno_ != 0 || fiber()->event().count_ == 0 ){
#elif HAVE_KQUEUE
  switch( fiber()->event().errno_ ){
    case 0        :
      if( fiber()->event().count_ > 0 ) break; else goto l2;
    case EMSGSIZE :
      if( len > 0 ){
        len >>= 1;
        goto l1;
      }
    default       :
l2:
#endif
     throw ksys::ExceptionSP(new EAsyncSocket(
        fiber()->event().count_ > 0 ? fiber()->event().errno_ + ksys::errorOffset : 0,
        utf8::string(__PRETTY_FUNCTION__))
      );
  }
  r = fiber()->event().count_;
  nrb_ += r;
  return r;
}
//------------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option pop
#endif
//------------------------------------------------------------------------------
uint64_t AsyncSocket::recv(void * buf,uint64_t len)
{
  flush();
  uint64_t r = 0;
  if( ksys::LZO1X::active() ){
    while( (r = ksys::LZO1X::read(buf,len)) == 0 ){ // read next compressed packet
      union {
        void * vbuf;
        uint8_t * buf;
      };
      int32_t l, ll;
      int32_t cps; // compressed packet size
      for( vbuf = &cps, l = 0; l < (int32_t) sizeof(cps); l += (int32_t) sysRecv(buf + l,sizeof(cps) - l) );
      if( ksys::SHA256Filter::active() ) decrypt(&cps,sizeof(cps));
      int32_t ups; // uncompressed packet size
      if( cps >= 0 ){
        for( vbuf = &ups, l = 0; l < (int32_t) sizeof(ups); l += (int32_t) sysRecv(buf + l,sizeof(ups) - l) );
        if( ksys::SHA256Filter::active() ) decrypt(&ups,sizeof(ups));
      }
      else {
        cps = -cps;
        ups = cps;
      }
      rBufSize(ups);
      ksys::AutoPtr<uint8_t> p;
      if( cps >= ups ){
        ll = ups;
        vbuf = rBuf();
      }
      else {
        p.alloc(ll = cps);
        vbuf = p.ptr();
      }
      for( l = 0; l < ll; l += (int32_t) sysRecv(buf + l,ll - l) );
      if( ksys::SHA256Filter::active() ) decrypt(buf,ll);
      if( cps < ups ) decompress(p,cps);
      rBufPos(0);
    }
  }
  else if( ksys::SHA256Filter::active() ){
    r = sysRecv(buf,len);
    decrypt(buf,(uintptr_t) r);
  }
  else {
    r = sysRecv(buf,len);
  }
  return r;
}
//------------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option push -w-8004
#endif
//------------------------------------------------------------------------------
uint64_t AsyncSocket::sysSend(const void * buf,uint64_t len)
{
  uint64_t w = 0;
  if( len > maxSendSize_ ) len = maxSendSize_;
#if HAVE_KQUEUE
l1:
#endif
  cluster()->postIoRequest(this,0,buf,len,ksys::etWrite);
  fiber()->switchFiber(fiber()->mainFiber());
  assert( fiber()->event().event_ == ksys::etWrite );
#if defined(__WIN32__) || defined(__WIN64__)
  if( fiber()->event().errno_ != 0 || fiber()->event().count_ == 0 ){
#elif HAVE_KQUEUE
  switch( fiber()->event().errno_ ){
    case 0        :
      if( fiber()->event().count_ > 0 ) break; else goto l2;
    case EMSGSIZE :
      if( len > 0 ){
        len >>= 1;
        goto l1;
      }
    default       :
l2:
#endif  
     throw ksys::ExceptionSP(new EAsyncSocket(
        fiber()->event().count_ > 0 ? fiber()->event().errno_ + ksys::errorOffset : 0,
        utf8::string(__PRETTY_FUNCTION__))
      );
  }
  w = fiber()->event().count_;
  nsb_ += w;
  return w;
}
//------------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option pop
#endif
//------------------------------------------------------------------------------
uint64_t AsyncSocket::send(const void * buf,uint64_t len)
{
  uint64_t w = 0;
  ksys::AutoPtr<char> p;
  if( ksys::LZO1X::active() ){
    while( (w = ksys::LZO1X::write(buf,len)) == 0 ) flush();
  }
  else if( ksys::SHA256Filter::active() ){
    len = len > 1024 * 1024 * 1024 ? 1024 * 1024 * 1024 : len;
    p.alloc((size_t) len);
    encrypt(p,buf,(uintptr_t) len);
    buf = p.ptr();
    w = len;
    while( len > 0 ){
      uint64_t l = sysSend(buf,len);
      buf = (uint8_t *) buf + (size_t) l;
      len -= l;
    }
  }
  else {
    w = sysSend(buf,len);
  }
  return w;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::flush()
{
  if( ksys::LZO1X::active() && wBufPos() > 0 ){
    ksys::AutoPtr<uint8_t> p;
    compress(p);
    union {
      void * vbuf;
      uint8_t * buf;
    };
    int32_t l, ll;
    if( ((int32_t *) p.ptr())[0] >= ((int32_t *) p.ptr())[1] ){
      buf = wBuf() + sizeof(int32_t);
      ll = *(int32_t *) buf + sizeof(int32_t);
      *(int32_t *) buf = -*(int32_t *) buf;
      p.free();
    }
    else {
      buf = p.ptr();
      ll = ((int32_t *) p.ptr())[0] + sizeof(int32_t) * 2;
    }
    if( ksys::SHA256Filter::active() ) encrypt(buf,ll);
    for( l = 0; l < ll; l += (uint32_t) sysSend(buf + l,ll - l) );
    wBufPos(0);
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::read(void * buf,uint64_t len)
{
  for(;;){
    if( fiber()->terminated() )
      throw ksys::ExceptionSP(
        new EAsyncSocket(0,utf8::string(__PRETTY_FUNCTION__))
      );
    if( len <= 0 ) break;
    uint64_t l = recv(buf,len);
    buf = (uint8_t *) buf + (size_t) l;
    len -= l;
    srb_ += l;
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::write(const void * buf,uint64_t len)
{
  for(;;){
    if( fiber()->terminated() )
      throw ksys::ExceptionSP(
        new EAsyncSocket(0,utf8::string(__PRETTY_FUNCTION__))
      );
    if( len <= 0 ) break;
    uint64_t l = send(buf,len);
    buf = (uint8_t *) buf + (size_t) l;
    len -= l;
    ssb_ += l;
  }
  return *this;
}
//------------------------------------------------------------------------------
utf8::String AsyncSocket::readString()
{
  utf8::String s;
  *this >> s;
  return s;
}
//------------------------------------------------------------------------------
const uint8_t AsyncSocket::authMagic_[16] = {
  0x39, 0x0A, 0x34, 0x9F, 0xED, 0xB9, 0x4F, 0x86,
  0xB0, 0xBC, 0x04, 0xC7, 0x19, 0xFC, 0x4A, 0x7E
};
//------------------------------------------------------------------------------
enum rad { radRequired, radAllow, radDisabled };
//------------------------------------------------------------------------------
static uint8_t authChannelHelper(const utf8::String & s)
{
  if( s.strcasecmp(utf8::string("required")) == 0 ) return 0;
  if( s.strcasecmp(utf8::string("allow")) == 0 ) return 1;
  if( s.strcasecmp(utf8::string("disabled")) == 0 ) return 2;
  return 0;
}
//------------------------------------------------------------------------------
AsyncSocket::AuthErrorType AsyncSocket::serverAuth(
  const ksys::HashedObjectList<utf8::String,utf8::String> & usersDatabase,
  const utf8::String & encryption,
  uintptr_t threshold,
  const utf8::String & compression,
  uintptr_t level,
  uintptr_t bufferSize,
  utf8::String & user)
{
  uint8_t authMagic[sizeof(authMagic_)];
  read(authMagic,sizeof(authMagic));
  if( memcmp(authMagic,authMagic_,sizeof(authMagic_)) != 0 ){
    *this << int32_t(aeMagic);
    return aeMagic;
  }
  *this << int32_t(aeOK);
  utf8::String * password = usersDatabase.objectOfKey(user = readString());
  if( password == NULL ){
    *this << int32_t(aeUser);
    return aeUser;
  }
  *this << int32_t(aeOK);
  uint8_t passwordSHA256[32];
  memset(passwordSHA256,0,sizeof(passwordSHA256));
  ksys::SHA256 SHA256;
  if( password->strncasecmp(utf8::string("sha256:"),7) == 0 ){
    ksys::base64Decode(
      utf8::string(utf8::String::Iterator(*password) + 7),
      passwordSHA256,
      sizeof(passwordSHA256)
    );
  }
  else {
    SHA256.make(password->c_str(),password->size());
    memcpy(passwordSHA256,SHA256.sha256(),sizeof(passwordSHA256));
  }
  SHA256.make(passwordSHA256,sizeof(passwordSHA256));
  uint8_t rpassword2SHA256[32];
  read(rpassword2SHA256,sizeof(rpassword2SHA256));
  if( memcmp(rpassword2SHA256,SHA256.sha256(),sizeof(rpassword2SHA256)) != 0 ){
    *this << int32_t(aePassword);
    return aePassword;
  }
  *this << int32_t(aeOK);
  uint8_t se, ce;
  se = authChannelHelper(encryption);
  *this >> ce;
  *this << se;
  if( se == radRequired && ce == radDisabled ){
    *this << int32_t(aeEncryptionServerRequiredButClientDisabled);
    return aeEncryptionServerRequiredButClientDisabled;
  }
  if( se == radDisabled && ce == radRequired ){
    *this << int32_t(aeEncryptionServerDisabledButClientRequired);
    return aeEncryptionServerRequiredButClientDisabled;
  }
  *this << int32_t(aeOK);
  uint8_t sc, cc, clevel;
  sc = authChannelHelper(compression);
  uint32_t cbs, cthreshold;
  *this >> cc >> clevel >> cbs >> cthreshold;
  if( (uintptr_t) clevel < level ) level = clevel;
  if( cbs > 0 && cbs < (uint32_t) bufferSize ) bufferSize = cbs;
  if( cthreshold < (uint32_t) threshold ) threshold = cthreshold; 
  *this << sc << uint8_t(level) << uint32_t(bufferSize) << uint32_t(threshold);
  if( sc == radRequired && cc == radDisabled ){
    *this << int32_t(aeCompressionServerRequiredButClientDisabled);
    return aeCompressionServerRequiredButClientDisabled;
  }
  if( sc == radDisabled && cc == radRequired ){
    *this << int32_t(aeCompressionServerDisabledButClientRequired);
    return aeCompressionServerDisabledButClientRequired;
  }
  *this << int32_t(aeOK);

  if( (se == radRequired || se == radAllow) && (ce == radRequired || ce == radAllow) ){
    this->threshold(threshold);
    activateEncryption(passwordSHA256);
  }
  if( (sc == radRequired || sc == radAllow) && (cc == radRequired || cc == radAllow) )
    activateCompression(level,bufferSize);
  return aeOK;
}
//------------------------------------------------------------------------------
AsyncSocket::AuthErrorType AsyncSocket::clientAuth(
  const utf8::String & user,
  const utf8::String & password,
  const utf8::String & encryption,
  uintptr_t threshold,
  const utf8::String & compression,
  uintptr_t level,
  uintptr_t bufferSize)
{
  union {
    int32_t e;
    AuthErrorType ae;
  };
  e = ae = aeOK;
  
  write(authMagic_,sizeof(authMagic_));
  *this >> e;
  if( e != aeOK ) return ae;
  *this << user;
  *this >> e;
  if( e != aeOK ) return ae;

  uint8_t passwordSHA256[32];
  memset(passwordSHA256,0,sizeof(passwordSHA256));
  ksys::SHA256 SHA256;
  if( password.strncasecmp(utf8::string("sha256:"),7) == 0 ){
    ksys::base64Decode(
      utf8::string(utf8::String::Iterator(password) + 7),
      passwordSHA256,
      sizeof(passwordSHA256)
    );
  }
  else {
    SHA256.make(password.c_str(),password.size());
    memcpy(passwordSHA256,SHA256.sha256(),sizeof(passwordSHA256));
  }
  SHA256.make(passwordSHA256,sizeof(passwordSHA256));
  write(SHA256.sha256(),sizeof(passwordSHA256));
  *this >> e;
  if( e != aeOK ) return ae;

  uint8_t se, ce;
  ce = authChannelHelper(encryption);
  *this << ce;
  *this >> se >> e;
  if( e != aeOK ) return ae;

  uint8_t sc, cc, slevel;
  cc = authChannelHelper(compression);
  *this << cc << uint8_t(level) << uint32_t(bufferSize) << uint32_t(threshold);
  uint32_t sbs, sthreshold;
  *this >> sc >> slevel >> sbs >> sthreshold;
  *this >> e;
  if( e != aeOK ) return ae;

  if( (se == radRequired || se == radAllow) && (ce == radRequired || ce == radAllow) ){
    this->threshold(sthreshold);
    activateEncryption(passwordSHA256);
  }
  if( (sc == radRequired || sc == radAllow) && (cc == radRequired || cc == radAllow) )
    activateCompression(level,bufferSize);
  return aeOK;
}
//------------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
int AsyncSocket::WSAEnumNetworkEvents(WSAEVENT hEventObject,DWORD event)
{
  WSANETWORKEVENTS networkEvents;
  memset(&networkEvents,0,sizeof(networkEvents));
  if( api.WSAEnumNetworkEvents(socket_,hEventObject,&networkEvents) == 0 ){
    SetLastError(networkEvents.iErrorCode[event]);
    return 0;
  }
  return SOCKET_ERROR;
}
//---------------------------------------------------------------------------
BOOL AsyncSocket::AcceptEx(
  SOCKET sAcceptSocket,
  PVOID lpOutputBuffer,
  DWORD dwReceiveDataLength,
  DWORD dwLocalAddressLength,
  DWORD dwRemoteAddressLength,
  LPDWORD lpdwBytesReceived,
  LPOVERLAPPED lpOverlapped)
{
  if( lpOutputBuffer == NULL ){
    lpOutputBuffer = pAcceptExBuffer_;
    dwReceiveDataLength = 0;
    dwLocalAddressLength = sizeof(pAcceptExBuffer_->pLocalAddr4_);
    dwRemoteAddressLength = sizeof(pAcceptExBuffer_->pRemoteAddr4_);
  }
  return apiEx.AcceptEx(socket_,sAcceptSocket,lpOutputBuffer,dwReceiveDataLength,dwLocalAddressLength,dwRemoteAddressLength,lpdwBytesReceived,lpOverlapped);
}
//---------------------------------------------------------------------------
BOOL AsyncSocket::Connect(HANDLE event,ksys::IoRequest * request)
{
  if( api.WSAEventSelect(socket_,event,FD_CONNECT) != 0 ) return FALSE;
  IoRequestConnect * req = dynamic_cast<IoRequestConnect *>(request);
  assert( req != NULL );
  SetLastError(ERROR_SUCCESS);
  return api.connect(socket_,(struct sockaddr *) &req->addr4_,(socklen_t) req->length()) != SOCKET_ERROR ? TRUE : FALSE;
}
//---------------------------------------------------------------------------
BOOL AsyncSocket::Read(
  LPVOID lpBuffer,
  DWORD nNumberOfBytesToRead,
  LPDWORD lpNumberOfBytesRead,
  LPOVERLAPPED lpOverlapped)
{
  WSABUF buf;
  buf.buf = (char *) lpBuffer;
  buf.len = nNumberOfBytesToRead;
  DWORD Flags = 0;
  return api.WSARecv(socket_,&buf,1,lpNumberOfBytesRead,&Flags,lpOverlapped,NULL) != SOCKET_ERROR ? TRUE : FALSE;
}
//---------------------------------------------------------------------------
BOOL AsyncSocket::Write(
  LPCVOID lpBuffer,
  DWORD nNumberOfBytesToWrite,
  LPDWORD lpNumberOfBytesWritten,
  LPOVERLAPPED lpOverlapped)
{
  WSABUF buf;
  buf.buf = (char *) lpBuffer;
  buf.len = nNumberOfBytesToWrite;
  return api.WSASend(socket_,&buf,1,lpNumberOfBytesWritten,0,lpOverlapped,NULL) != SOCKET_ERROR ? TRUE : FALSE;
}
//---------------------------------------------------------------------------
BOOL AsyncSocket::GetOverlappedResult(
  LPOVERLAPPED lpOverlapped,
  LPDWORD lpNumberOfBytesTransferred,
  BOOL bWait,
  LPDWORD lpdwFlags)
{
  DWORD dwFlags = 0;
  if( lpdwFlags == NULL ) lpdwFlags = &dwFlags;
  return api.WSAGetOverlappedResult(socket_,lpOverlapped,lpNumberOfBytesTransferred,bWait,lpdwFlags);
}
//---------------------------------------------------------------------------
#elif HAVE_KQUEUE
//---------------------------------------------------------------------------
int AsyncSocket::accept()
{
  errno = 0;
  return api.accept(socket_,NULL,0);
}
//---------------------------------------------------------------------------
void AsyncSocket::connect(ksys::IoRequest * request)
{
  errno = 0;
  api.connect(
    socket_,
    (const struct sockaddr *) &request->address_,
    (socklen_t) request->address_.length()
  );
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
void AsyncSocket::shutdown2()
{
  shutdown();
}
//------------------------------------------------------------------------------
void AsyncSocket::flush2()
{
  flush();
}
//------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
EAsyncSocket::EAsyncSocket(int32_t code,const utf8::String & what) : ksys::Exception(code,what)
{
}
//---------------------------------------------------------------------------
} // namespace ksock
//------------------------------------------------------------------------------
