/*-
 * Copyright 2006-2008 Guram Dukashvili
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
#include <adicpp/stmflt.h>
//------------------------------------------------------------------------------
#if __MINGW32__
  typedef VOID WINAPI (* LPFN_GETACCEPTEXSOCKADDRS)(
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT struct sockaddr **LocalSockaddr,
    OUT LPINT LocalSockaddrLength,
    OUT struct sockaddr **RemoteSockaddr,
    OUT LPINT RemoteSockaddrLength
  );
#define WSAID_GETACCEPTEXSOCKADDRS { 0xB5367DF2, 0xCBAC, 0x11CF, { 0x95, 0xCA, 0x00, 0x80, 0x5F, 0x48, 0xA1, 0x92 } }
  typedef BOOL WINAPI (* LPFN_ACCEPTEX)(
    IN SOCKET sListenSocket,
    IN SOCKET sAcceptSocket,
    IN PVOID lpOutputBuffer,
    IN DWORD dwReceiveDataLength,
    IN DWORD dwLocalAddressLength,
    IN DWORD dwRemoteAddressLength,
    OUT LPDWORD lpdwBytesReceived,
    IN LPOVERLAPPED lpOverlapped
  );
#define WSAID_ACCEPTEX { 0xB5367DF1, 0xCBAC, 0x11CF, { 0x95, 0xCA, 0x00, 0x80, 0x5F, 0x48, 0xA1, 0x92 } }
#endif
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
AsyncSocket::~AsyncSocket()
{
  close();
  ksys::Requester::requester().detachDescriptor(*this);
}
//------------------------------------------------------------------------------
AsyncSocket::AsyncSocket() :
  maxRecvSize_(~uintptr_t(0)), maxSendSize_(~uintptr_t(0)),
  recvTimeout_(~uint64_t(0)), sendTimeout_(~uint64_t(0))
{
#if defined(__WIN32__) || defined(__WIN64__)
  socket_ = INVALID_SOCKET;
#endif
  ksys::Requester::requester().attachDescriptor(*this);
}
//------------------------------------------------------------------------------
bool AsyncSocket::isSocket() const
{
  return true;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::open(int domain, int type, int protocol)
{
  int32_t err;
  if( socket_ == INVALID_SOCKET ){
    api.open();
#if defined(__WIN32__) || defined(__WIN64__)
//  WSAPROTOCOL_INFO protocolInfo;
//  memset(&protocolInfo,0,sizeof(protocolInfo));
//  protocolInfo.dwMessageSize = 1;
    if( ksys::isWin9x() )
      socket_ = api.WSASocketA(domain,type,protocol,NULL,0,WSA_FLAG_OVERLAPPED);
    else
      socket_ = api.WSASocketW(domain,type,protocol,NULL,0,WSA_FLAG_OVERLAPPED);
#else
    socket_ = api.socket(domain,type,protocol);
#endif
    if( socket_ == INVALID_SOCKET ){
      err = errNo();
      api.close();
      newObjectV1C2<EAsyncSocket>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    try {
#if !defined(__WIN32__) && !defined(__WIN64__)
      int flags = fcntl(socket_,F_GETFL,0);
      if( flags == -1 || fcntl(socket_,F_SETFL,flags | O_NONBLOCK) == -1 ){
        err = errno;
        newObjectV1C2<EAsyncSocket>(err,__PRETTY_FUNCTION__)->throwSP();
      }
#endif
#if defined(IP_TOS) && defined(IPTOS_LOWDELAY)
      int tos = IPTOS_LOWDELAY;
      setsockopt(IPPROTO_IP,IP_TOS,&tos,sizeof(tos));
#endif
#ifdef TCP_NODELAY
      int on = 1;
      setsockopt(IPPROTO_TCP,TCP_NODELAY,&on,sizeof(on));
#endif
      if( type == SOCK_STREAM ){
#if defined(__WIN32__) || defined(__WIN64__)
        int bufLen = 0;
        setsockopt(SOL_SOCKET,SO_RCVBUF,&bufLen,sizeof(bufLen));
        setsockopt(SOL_SOCKET,SO_SNDBUF,&bufLen,sizeof(bufLen));
#else
/*      socklen_t optlen;
        int lowait;
        optlen = sizeof(lowait);
        getsockopt(SOL_SOCKET,SO_SNDLOWAT,&lowait,optlen);
        if( lowait < 1024 ){
          lowait = 2048;
          setsockopt(SOL_SOCKET,SO_SNDLOWAT,&lowait,sizeof(lowait));
        }
        getsockopt(SOL_SOCKET,SO_RCVLOWAT,&lowait,optlen);
        lowait = 1;
        setsockopt(SOL_SOCKET,SO_RCVLOWAT,&lowait,sizeof(lowait));
        struct timeval timeo;
        optlen = sizeof(timeo);
        getsockopt(SOL_SOCKET,SO_SNDTIMEO,&timeo,optlen);
        getsockopt(SOL_SOCKET,SO_RCVTIMEO,&timeo,optlen);
        timeo.tv_sec = LONG_MAX >> 10;
        timeo.tv_usec = 0;
        setsockopt(SOL_SOCKET,SO_RCVTIMEO,&timeo,sizeof(timeo));
        setsockopt(SOL_SOCKET,SO_SNDTIMEO,&timeo,sizeof(timeo));*/
#endif
        int ka = 1;
        setsockopt(SOL_SOCKET,SO_KEEPALIVE,&ka,sizeof(ka));
        struct linger lg;
        lg.l_onoff = 0;
        lg.l_linger = 0;
        setsockopt(SOL_SOCKET,SO_LINGER,&lg,sizeof(lg));
//#ifndef __linux__
        int reuse = 1;
//        socklen_t rlen = sizeof(reuse);
//        getsockopt(SOL_SOCKET,SO_REUSEADDR,&reuse,rlen);
        setsockopt(SOL_SOCKET,SO_REUSEADDR,&reuse,(socklen_t) sizeof(reuse));
//#endif
      }
    }
    catch( ... ){
      close();
      throw;
    }
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::close()
{
  bool closed = false;
  {
    ksys::AutoLock<ksys::LiteWriteLock> lock(api.mutex());
    if( socket_ != INVALID_SOCKET ){
      if( api.closesocket(socket_) != 0 ){
        int32_t err = errNo();
        newObjectV1C2<EAsyncSocket>(err,__PRETTY_FUNCTION__)->throwSP();
      }
      socket_ = INVALID_SOCKET;
      closed = true;
    }
  }
  if( closed ){
    api.close();
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::shutdown(int how)
{
  ksys::AutoLock<ksys::LiteWriteLock> lock(api.mutex());
  if( socket_ != INVALID_SOCKET ){
    if( api.shutdown(socket_,how) != 0 ){
      int32_t err = errNo();
      if( err != ENOTCONN && err != ENOTSOCK && err != EWSANOTINITIALISED )
        newObjectV1C2<EAsyncSocket>(err,__PRETTY_FUNCTION__)->throwSP();
    }
/*#if HAVE_KQUEUE
    ksys::Fiber * fiber = ksys::currentFiber();
    if( fiber->event_.type_ == ksys::etAccept ){
      ksys::AsyncIoSlave * slave = dynamic_cast<ksys::AsyncIoSlave *>(fiber->event_.ioSlave_);
      if( slave != NULL ) slave->cancelEvent(fiber->event_);
    }
#endif*/
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::getsockopt(int level,int optname,void * optval,socklen_t & optlen)
{
  if( api.getsockopt(socket_,level,optname,(char *) optval,&optlen) != 0 ){
    int32_t err = errNo();
    newObjectV1C2<EAsyncSocket>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::setsockopt(int level,int optname,const void * optval,socklen_t optlen)
{
  if( api.setsockopt(socket_,level,optname,(const char *) optval,optlen) != 0 ){
    int32_t err = errNo();
    newObjectV1C2<EAsyncSocket>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::bind(const SockAddr & sockAddr)
{
  if( api.bind(socket_,(struct sockaddr *) &sockAddr.addr4_,sockAddr.sockAddrSize()) != 0 ){
    int32_t err = errNo();
    newObjectV1C2<EAsyncSocket>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::listen()
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( pAcceptExBuffer_ == NULL )
    pAcceptExBuffer_.realloc(sizeof(AcceptExBuffer));
#endif
  if( api.listen(socket_,SOMAXCONN) != 0 ){
    int32_t err = errNo();
    newObjectV1C2<EAsyncSocket>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return *this;
}
//------------------------------------------------------------------------------
const SockAddr & AsyncSocket::remoteAddress() const
{
  if( remoteAddress_ == NULL )
#if defined(__WIN32__) || defined(__WIN64__)
    newObjectV1C2<EAsyncSocket>(ERROR_INVALID_DATA + ksys::errorOffset,__PRETTY_FUNCTION__)->throwSP();
#else
    newObjectV1C2<EAsyncSocket>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
#endif
  return remoteAddress_;
}
//---------------------------------------------------------------------------
AsyncSocket & AsyncSocket::accept(AsyncSocket & socket)
{
  if( socket.remoteAddress_ == NULL ) socket.remoteAddress_.alloc(sizeof(SockAddr));
#if defined(__WIN32__) || defined(__WIN64__)
  if( pAcceptExBuffer_ == NULL ) pAcceptExBuffer_.alloc(sizeof(AcceptExBuffer));
  assert( socket.socket_ == INVALID_SOCKET );
  socket.open();
  ksys::Fiber * fiber = ksys::currentFiber();
  assert( fiber != NULL );
  fiber->event_.timeout_ = ~uint64_t(0);
  fiber->event_.socket_ = socket.socket_;
  fiber->event_.type_ = ksys::etAccept;
  fiber->thread()->postRequest(this);
  fiber->switchFiber(fiber->mainFiber());
  assert( fiber->event_.type_ == ksys::etAccept );
  if( fiber->event_.errno_ != 0 ){
    socket.close();
    newObjectV1C2<EAsyncSocket>(fiber->event_.errno_ + ksys::errorOffset,__PRETTY_FUNCTION__)->throwSP();
  }
  LPSOCKADDR plsa, prsa;
  INT lsaLen, rsaLen;
//----------------------------------------
  // Load the GetAcceptExSockaddrs function into memory using WSAIoctl.
  LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs = NULL;
  GUID GuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
  DWORD dwBytes;
  api.WSAIoctl(socket_,
    SIO_GET_EXTENSION_FUNCTION_POINTER, 
    &GuidGetAcceptExSockaddrs,
    sizeof(GuidGetAcceptExSockaddrs),
    &lpfnGetAcceptExSockaddrs,
    sizeof(lpfnGetAcceptExSockaddrs),
    &dwBytes,
    NULL,
    NULL
  );
  lpfnGetAcceptExSockaddrs(
    pAcceptExBuffer_,
    0,
    sizeof(pAcceptExBuffer_->pLocalAddr4_),
    sizeof(pAcceptExBuffer_->pRemoteAddr4_),
    &plsa,
    &lsaLen,
    &prsa,
    &rsaLen
  );
  //memmove(&socket.localAddress_.addr4_,plsa,sizeof(socket.localAddress_.addr4_));
  memmove(&socket.remoteAddress_->addr4_,prsa,sizeof(socket.remoteAddress_->addr4_));
#else
  ksys::Fiber * fiber = ksys::currentFiber();
  assert( fiber != NULL );
  assert( socket.socket_ == INVALID_SOCKET );
  fiber->event_.timeout_ = ~uint64_t(0);
  fiber->event_.type_ = ksys::etAccept;
  fiber->thread()->postRequest(this);
  fiber->switchFiber(fiber->mainFiber());
  assert( fiber->event_.type_ == ksys::etAccept );
  if( fiber->event_.errno_ != 0 )
    newObjectV1C2<EAsyncSocket>(fiber->event_.errno_ + ksys::errorOffset,__PRETTY_FUNCTION__)->throwSP();
  socket.socket_ = (int) fiber->event_.data_;
  if( fcntl(socket.socket_,F_SETFL,fcntl(socket.socket_,F_GETFL,0) | O_NONBLOCK) != 0 ){
    int32_t err = errno;
    newObjectV1C2<EAsyncSocket>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  int ka = true;
  socket.setsockopt(SOL_SOCKET,SO_KEEPALIVE,&ka,sizeof(ka));
  struct linger lg;
  lg.l_onoff = 0;
  lg.l_linger = 0;
  setsockopt(SOL_SOCKET,SO_LINGER,&lg,sizeof(lg));
  int reuse = true;
//  socklen_t rlen = sizeof(reuse);
//  getsockopt(SOL_SOCKET,SO_REUSEADDR,&reuse,rlen);
  setsockopt(SOL_SOCKET,SO_REUSEADDR,&reuse,(socklen_t) sizeof(reuse));
#endif
  socket.deActivateCompression();
  socket.deActivateEncryption();
  socket.clearStatistic();
  return *this;
}
//------------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option push -w-8004
#endif
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::connect(const SockAddr & addr)
{
  open();
  ksys::Fiber * fiber = ksys::currentFiber();
  assert( fiber != NULL );
  fiber->event_.timeout_ = ~uint64_t(0);
  fiber->event_.position_ = 0;
  memcpy(fiber->event_.address_,&addr,sizeof(addr));
  fiber->event_.type_ = ksys::etConnect;
  fiber->thread()->postRequest(this);
  fiber->switchFiber(fiber->mainFiber());
  assert( ksys::currentFiber()->event_.type_ == ksys::etConnect );
  if( fiber->event_.errno_ != 0 )
    newObjectV1C2<EAsyncSocket>(fiber->event_.errno_ + ksys::errorOffset,__PRETTY_FUNCTION__)->throwSP();
  deActivateCompression();
  deActivateEncryption();
  clearStatistic();
  return *this;
}
//------------------------------------------------------------------------------
uint64_t AsyncSocket::sysRecv(void * buf,uint64_t len)
{
  ksys::currentFiber()->checkFiberStackOverflow();
  uint64_t r = 0;
  if( len > maxRecvSize_ ) len = maxRecvSize_;
#if HAVE_KQUEUE || HAVE_AIO_SUSPEND || HAVE_AIO_WAITCOMPLETE
l1:
#endif
  ksys::Fiber * fiber = ksys::currentFiber();
  fiber->event_.timeout_ = recvTimeout_;
  fiber->event_.buffer_ = buf;
  fiber->event_.length_ = len;
  fiber->event_.type_ = ksys::etRead;
  fiber->thread()->postRequest(this);
  fiber->switchFiber(fiber->mainFiber());
  assert( fiber->event_.type_ == ksys::etRead );
#if defined(__WIN32__) || defined(__WIN64__)
  if( fiber->event_.errno_ != 0 || fiber->event_.count_ == 0 ){
#elif HAVE_KQUEUE || HAVE_AIO_SUSPEND || HAVE_AIO_WAITCOMPLETE
  switch( fiber->event_.errno_ ){
    case 0           :
      if( fiber->event_.count_ == 0 ) goto l2;
      break;
    case EMSGSIZE    :
      if( len > 0 ){
        len >>= 1;
        goto l1;
      }
    default          :
l2:
#endif
     newObjectV1C2<EAsyncSocket>(fiber->event_.errno_ + ksys::errorOffset,__PRETTY_FUNCTION__)->throwSP();
  }
  r = fiber->event_.count_;
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
  len = len > 1024 * 1024 * 1024 ? 1024 * 1024 * 1024 : len;
  uint64_t r = 0;
  if( compressor_ != NULL ){
    flush();
    uintptr_t rb, wb;//, crcSize = 0;
      //if( crc_ != NULL ){
      //  crc_->make(
      //    compressor_->encoderOutputBuffer(),
      //    compressor_->encoderOutputBufferSize(),
      //    compressor_->encoderOutputBuffer() + compressor_->encoderOutputBufferSize(),
      //    &crcSize
      //  );
      //}
    for(;;){
      rb = wb = 0;
      compressor_->decodeBuffer(
        compressor_->decoderInputBuffer() + compressor_->decoderReadBytes2(),
        compressor_->decoderReadBytes() - compressor_->decoderReadBytes2(),
        buf,
        uintptr_t(len),
        &rb,
        &wb
      );
      compressor_->decoderReadBytes2(compressor_->decoderReadBytes2() + rb);
      if( wb > 0 ) break;
      if( compressor_->decoderReadBytes2() == compressor_->decoderReadBytes2() ){
        if( compressor_->decoderReadBytes() == compressor_->decoderInputBufferSize() ){
          compressor_->decoderReadBytes(0);
          compressor_->decoderReadBytes2(0);
        }
        r = sysRecv(
          compressor_->decoderInputBuffer() + compressor_->decoderReadBytes(),
          compressor_->decoderInputBufferSize() - compressor_->decoderReadBytes()
        );
        if( cryptor_ != NULL ){
          cryptor_->decrypt(
            compressor_->decoderInputBuffer() + compressor_->decoderReadBytes(),
            compressor_->decoderInputBuffer() + compressor_->decoderReadBytes(),
            uintptr_t(r)
          );
        }
        compressor_->decoderReadBytes(compressor_->decoderReadBytes() + uintptr_t(r));
      }
    }
    r = wb;
  }
  else if( cryptor_ != NULL ){
    r = sysRecv(buf,len);
    cryptor_->decrypt(buf,buf,uintptr_t(r));
  }
  else if( ksys::LZO1X::active() ){
    flush();
    while( (r = ksys::LZO1X::read(buf,len)) == 0 ){ // read next compressed packet
      union {
        void * vp;
        uint8_t * ui8p;
      };
      int32_t l, cps; // compressed packet size
      for( vp = &cps, l = 0; l < (int32_t) sizeof(cps); l += (int32_t) sysRecv(ui8p + l,sizeof(cps) - l) );
      if( ksys::SHA256Filter::active() ) decrypt(&cps,sizeof(cps));
      if( cps < 0 ){
        rBufSize(-cps);
        for( l = 0; l < -cps; l += (int32_t) sysRecv(rBuf() + l,-cps - l) );
        if( ksys::SHA256Filter::active() ) decrypt(rBuf(),-cps);
      }
      else if( cps >= 0 ){
        ksys::AutoPtrBuffer b;
        b.alloc(cps);
        for( l = sizeof(int32_t); l < cps; l += (int32_t) sysRecv(b.ptr() + l,cps - l) );
        *(int32_t *) b.ptr() = cps;
        if( ksys::SHA256Filter::active() ) decrypt(b.ptr() + sizeof(cps),cps - sizeof(cps));
        decompress(b);
      }
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
  srb_ += r;
  return r;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::read(void * buf,uint64_t len)
{
  for(;;){
    if( ksys::currentFiber()->terminated() )
      newObjectV1C2<EAsyncSocket>(ECONNABORTED,__PRETTY_FUNCTION__)->throwSP();
    if( len <= 0 ) break;
    uint64_t l = recv(buf,len);
    buf = (uint8_t *) buf + (size_t) l;
    len -= l;
  }
  return *this;
}
//------------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option push -w-8004
#endif
//------------------------------------------------------------------------------
uint64_t AsyncSocket::sysSend(const void * buf,uint64_t len)
{
  ksys::currentFiber()->checkFiberStackOverflow();
  uint64_t w = 0;
  if( len > maxSendSize_ ) len = maxSendSize_;
#if HAVE_KQUEUE || HAVE_AIO_SUSPEND || HAVE_AIO_WAITCOMPLETE
l1:
#endif
  ksys::Fiber * fiber = ksys::currentFiber();
  fiber->event_.timeout_ = sendTimeout_;
  fiber->event_.cbuffer_ = buf;
  fiber->event_.length_ = len;
  fiber->event_.type_ = ksys::etWrite;
  fiber->thread()->postRequest(this);
  fiber->switchFiber(fiber->mainFiber());
  assert( fiber->event_.type_ == ksys::etWrite );
#if defined(__WIN32__) || defined(__WIN64__)
  if( fiber->event_.errno_ != 0 || fiber->event_.count_ == 0 ){
#elif HAVE_KQUEUE || HAVE_AIO_SUSPEND || HAVE_AIO_WAITCOMPLETE
  switch( fiber->event_.errno_ ){
    case 0           :
      if( fiber->event_.count_ == 0 ) goto l2;
      break;
    case EMSGSIZE    :
      if( len > 0 ){
        len >>= 1;
        goto l1;
      }
    default          :
l2:
#endif  
     newObjectV1C2<EAsyncSocket>(fiber->event_.errno_ + ksys::errorOffset,__PRETTY_FUNCTION__)->throwSP();
  }
  w = fiber->event_.count_;
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
  len = len > 1024 * 1024 * 1024 ? 1024 * 1024 * 1024 : len;
  uint64_t w = 0;
  if( compressor_ != NULL ){
    uintptr_t rb = 0, wb = 0, crcSize = 0;
    compressor_->encodeBuffer(
      buf,
      uintptr_t(len),
      compressor_->encoderOutputBuffer() + compressor_->encoderWriteBytes(),
      compressor_->encoderOutputBufferSize() - compressor_->encoderWriteBytes(),
      &rb,
      &wb
    );
    compressor_->encoderWriteBytes(compressor_->encoderWriteBytes() + wb);
    if( compressor_->encoderWriteBytes() == compressor_->encoderOutputBufferSize() ){
      if( cryptor_ != NULL ){
        cryptor_->encrypt(
          compressor_->encoderOutputBuffer(),
          compressor_->encoderOutputBuffer(),
          compressor_->encoderOutputBufferSize()
        );
      }
      //if( crc_ != NULL && crc_->active() ){
      //  crc_->make(
      //    compressor_->encoderOutputBuffer(),
      //    compressor_->encoderOutputBufferSize(),
      //    compressor_->encoderOutputBuffer() + compressor_->encoderOutputBufferSize(),
      //    &crcSize
      //  );
      //}
      buf = compressor_->encoderOutputBuffer();
      len = compressor_->encoderOutputBufferSize() + crcSize;
      while( len > 0 ){
        uint64_t l = sysSend(buf,len);
        buf = (uint8_t *) buf + (size_t) l;
        len -= l;
      }
      compressor_->encoderWriteBytes(0);
    }
    w = rb;
  }
  else if( cryptor_ != NULL ){
    ksys::AutoPtrBuffer p;
    p.alloc((size_t) len);
    buf = p.ptr();
    cryptor_->encrypt(p,p,uintptr_t(len));
    w = len;
    while( len > 0 ){
      uint64_t l = sysSend(buf,len);
      buf = (uint8_t *) buf + (size_t) l;
      len -= l;
    }
  }
  else if( ksys::LZO1X::active() ){
    while( (w = ksys::LZO1X::write(buf,len)) == 0 ) flush();
  }
  else if( ksys::SHA256Filter::active() ){
    ksys::AutoPtrBuffer p;
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
  ssb_ += w;
  return w;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::write(const void * buf,uint64_t len)
{
  for(;;){
    if( ksys::currentFiber()->terminated() )
      newObjectV1C2<EAsyncSocket>(ECONNABORTED,__PRETTY_FUNCTION__)->throwSP();
    if( len <= 0 ) break;
    uint64_t l = send(buf,len);
    buf = (uint8_t *) buf + (size_t) l;
    len -= l;
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::flush()
{
  if( compressor_ != NULL && compressor_->encoderWriteBytes() > 0 ){
    uintptr_t wb = 0, crcSize = 0;
    if( cryptor_ != NULL ){
      cryptor_->encrypt(
        compressor_->encoderOutputBuffer(),
        compressor_->encoderOutputBuffer(),
        compressor_->encoderWriteBytes()
      );
    }
    void * buf = compressor_->encoderOutputBuffer();
    uintptr_t len = compressor_->encoderWriteBytes() + crcSize;
    while( len > 0 ){
      uint64_t l = sysSend(buf,len);
      buf = (uint8_t *) buf + (size_t) l;
      len -= (size_t) l;
    }
    compressor_->encoderWriteBytes(0);
    compressor_->flush(
      compressor_->encoderOutputBuffer() + compressor_->encoderWriteBytes(),
      &wb
    );
    compressor_->encoderWriteBytes(compressor_->encoderWriteBytes() + wb);
    if( cryptor_ != NULL ){
      cryptor_->encrypt(
        compressor_->encoderOutputBuffer(),
        compressor_->encoderOutputBuffer(),
        compressor_->encoderWriteBytes()
      );
    }
    buf = compressor_->encoderOutputBuffer();
    len = compressor_->encoderWriteBytes() + crcSize;
    while( len > 0 ){
      uint64_t l = sysSend(buf,len);
      buf = (uint8_t *) buf + (size_t) l;
      len -= (size_t) l;
    }
    compressor_->encoderWriteBytes(0);
  }
  else if( ksys::LZO1X::active() && wBufPos() > 0 ){
    ksys::AutoPtrBuffer buf;
    uint8_t * p;
    int32_t l, ll;
    compress(buf,p,ll);
    if( ksys::SHA256Filter::active() ) encrypt(p,ll);
    for( l = 0; l < ll; l += (uint32_t) sysSend(p + l,ll - l) );
    wBufPos(0);
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
AsyncSocket & AsyncSocket::operator << (const utf8::String & s)
{
  union {
    uint64_t l;
    uint8_t b[sizeof(uint64_t)];
  };
  uint64_t ll;
  ll = l = s.size();
  if( l > (~uint64_t(0) >> 1) )
    newObjectV1C2<ksys::Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
  if( l > 32767 ){
    for( uintptr_t i = sizeof(b); i > sizeof(b) / 2; i-- )
      ksys::xchg(b[sizeof(b) - i],((uint8_t *) &l)[i - 1]);
    b[0] |= uint8_t(0x80);
    return write(b,sizeof(uint64_t)).write(s.c_str(),ll);
  }
  ksys::xchg(b[0],b[1]);
  return write(&l,sizeof(int16_t)).write(s.c_str(),ll);
}
//---------------------------------------------------------------------------
AsyncSocket & AsyncSocket::operator >> (utf8::String & s)
{
  union {
    uint64_t l;
    uint8_t b[sizeof(uint64_t)];
  };
  l = 0;
  read(b,sizeof(int16_t));
  if( b[0] & 0x80 ){
    b[0] &= ~0x80;
    read(b + sizeof(int16_t),sizeof(uint64_t) - sizeof(int16_t));
    for( uintptr_t i = sizeof(b); i > sizeof(b) / 2; i-- )
      ksys::xchg(b[sizeof(b) - i],((uint8_t *) &l)[i - 1]);
  }
  else {
    ksys::xchg(b[0],b[1]);
  }
  uintptr_t q = 0;
  q = ~q;
  if( l > q ) newObjectV1C2<ksys::Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
  ksys::AutoPtr<utf8::String::Container,AutoPtrNonVirtualClassDestructor> t(utf8::String::Container::container(uintptr_t(l)));
  read(t->string_,l);
  t->string_[uintptr_t(l)] = '\0';
  utf8::String e(t.ptr(NULL));
  if( !e.isValid() )
    newObjectV1C2<ksys::Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
  s = e;
  return *this;
}
//---------------------------------------------------------------------------
const uint8_t AsyncSocket::authMagic_[16] = {
  0x39, 0x0A, 0x34, 0x9F, 0xED, 0xB9, 0x4F, 0x86,
  0xB0, 0xBC, 0x04, 0xC7, 0x19, 0xFC, 0x4A, 0x7E
};
const uint8_t AsyncSocket::authMagic2_[16] = {
  0xB8, 0xDA, 0x85, 0xEF, 0xA8, 0x0D, 0x4A, 0xBE,
  0x91, 0x06, 0xDD, 0x12, 0xAF, 0x76, 0xF2, 0xAC
};
//------------------------------------------------------------------------------
enum rad { radRequired, radAllow, radDisabled };
//------------------------------------------------------------------------------
static uint8_t authChannelHelper(const utf8::String & s)
{
  if( s.casecompare("required") == 0 ) return radRequired;
  if( s.casecompare("allow") == 0 ) return radAllow;
  if( s.casecompare("disabled") == 0) return radDisabled;
  if( s.casecompare("default") == 0 || s.isNull() ) return radAllow;
  newObjectV1C2<ksys::Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
  throw 0;
}
//------------------------------------------------------------------------------
static uint8_t authChannelHelper2(const utf8::String & s)
{
  if( s.casecompare("LZO1X_1") == 0 ) return ksys::LZO1X::LZO1X_1;
  if( s.casecompare("LZO1X_1_11") == 0 ) return ksys::LZO1X::LZO1X_1_11;
  if( s.casecompare("LZO1X_1_12") == 0 ) return ksys::LZO1X::LZO1X_1_12;
  if( s.casecompare("LZO1X_1_15") == 0 ) return ksys::LZO1X::LZO1X_1_15; // fastest
  if( s.casecompare("LZO1X_999") == 0 ) return ksys::LZO1X::LZO1X_999; // slowest, best compression
  if( s.casecompare("LZMA") == 0 ) return ksys::LZO1X::LZO1X_999 + 1;
  if( s.casecompare("RANGE32") == 0 ) return ksys::LZO1X::LZO1X_999 + 2;
  if( s.casecompare("LZSSRBT") == 0 ) return ksys::LZO1X::LZO1X_999 + 3;
  if( s.casecompare("default") == 0 || s.isNull() ) return ksys::LZO1X::LZO1X_999 + 1;
  newObjectV1C2<ksys::Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
  throw 0;
}
//------------------------------------------------------------------------------
static uint8_t authChannelHelper3(const utf8::String & s)
{
  if( s.casecompare("disabled") == 0 ) return ksys::LZO1X::CRCNone;
  if( s.casecompare("CRC32") == 0 ) return ksys::LZO1X::CRC32;
  if( s.casecompare("ADLER32") == 0 ) return ksys::LZO1X::ADLER32;
  if( s.casecompare("SHA256") == 0 ) return ksys::LZO1X::ADLER32 + 1;
  if( s.casecompare("default") == 0 || s.isNull() ) return ksys::LZO1X::ADLER32;
  newObjectV1C2<ksys::Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
  throw 0;
}
//------------------------------------------------------------------------------
AsyncSocket::AuthParams::~AuthParams()
{
}
//------------------------------------------------------------------------------
AsyncSocket::AuthParams::AuthParams() :
  maxRecvSize_(~uintptr_t(0)), maxSendSize_(~uintptr_t(0)),
  recvTimeout_(~uint64_t(0)), sendTimeout_(~uint64_t(0)),
  threshold_(1024u * 1024u), level_(9), optimize_(true), noAuth_(false)
{
}
//------------------------------------------------------------------------------
/*AsyncSocket::AuthErrorType AsyncSocket::serverAuth2(const AuthParams & sp)
{
  AuthParams cp;
  uf8::String name, value;
  for(;;){
    name = readString();
    if( name.isNull() ) break;
    value = readString();
    cp.param(name,value);
  }
}*/
//------------------------------------------------------------------------------
AsyncSocket::AuthErrorType AsyncSocket::serverAuth(const AuthParams & ap)
{
  ksys::currentFiber()->checkFiberStackOverflow();
  maxRecvSize_ = ap.maxRecvSize_;
  maxSendSize_ = ap.maxSendSize_;
  recvTimeout_ = ap.recvTimeout_;
  sendTimeout_ = ap.sendTimeout_;
  uint8_t passwordSHA256[32];
  uint8_t se = radDisabled, ce = radDisabled;
  if( !ap.noAuth_ ){
    uint8_t authMagic[sizeof(authMagic_)];
    read(authMagic,sizeof(authMagic));
   // if( memcmp(authMagic,authMagic2_,sizeof(authMagic2_)) == 0 ) return serverAuth2(ap);
    if( memcmp(authMagic,authMagic_,sizeof(authMagic_)) != 0 ){
      *this << int32_t(aeMagic);
      return aeMagic;
    }
    *this << int32_t(aeOK);
    utf8::String user(readString());
    if( !isValidUser(user) ){
      utf8::String::Stream stream;
      stream << "Invalid user: " << user << ", " << __PRETTY_FUNCTION__ << "\n";
      ksys::stdErr.debug(99,stream);
      *this << int32_t(aeUser);
      return aeUser;
    }
    *this << int32_t(aeOK);
    utf8::String password(getUserPassword(user,&ap));
    memset(passwordSHA256,0,sizeof(passwordSHA256));
    ksys::SHA256 SHA256;
    if( password.ncasecompare("sha256:",7) == 0 ){
      ksys::base64Decode(
        utf8::String::Iterator(password) + 7,
        passwordSHA256,
        sizeof(passwordSHA256)
      );
    }
    else {
      SHA256.make(password.c_str(),password.size());
      memcpy(passwordSHA256,SHA256.sha256(),sizeof(passwordSHA256));
    }
    SHA256.make(passwordSHA256,sizeof(passwordSHA256));
    uint8_t rpassword2SHA256[32];
    read(rpassword2SHA256,sizeof(rpassword2SHA256));
    if( memcmp(rpassword2SHA256,SHA256.sha256(),sizeof(rpassword2SHA256)) != 0 ){
      utf8::String::Stream stream;
      stream << "Invalid password: " << password << " for user: " << user << ", " << __PRETTY_FUNCTION__ << "\n";
      ksys::stdErr.debug(99,stream);
      *this << int32_t(aePassword);
      return aePassword;
    }
    *this << int32_t(aeOK);
    se = authChannelHelper(ap.encryption_);
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
  }
  uint8_t sc, cc, smethod, cmethod, scrc, ccrc, clevel;
  sc = authChannelHelper(ap.compression_);
  smethod = authChannelHelper2(ap.compressionType_);
  scrc = authChannelHelper3(ap.crc_);
  uint32_t cbs, cthreshold;
  *this >> cc >> cmethod >> ccrc >> clevel >> cbs >> cthreshold;
  if( (uintptr_t) clevel < ap.level_ ) ap.level_ = clevel;
  if( cbs > 0 && cbs < (uint32_t) ap.bufferSize_ ) ap.bufferSize_ = cbs;
  if( cthreshold < (uint32_t) ap.threshold_ ) ap.threshold_ = cthreshold; 
  *this << sc << smethod << scrc << uint8_t(ap.level_) << uint32_t(ap.bufferSize_) << uint32_t(ap.threshold_);
  if( sc == radRequired && cc == radDisabled ){
    *this << int32_t(aeCompressionServerRequiredButClientDisabled);
    return aeCompressionServerRequiredButClientDisabled;
  }
  if( sc == radDisabled && cc == radRequired ){
    *this << int32_t(aeCompressionServerDisabledButClientRequired);
    return aeCompressionServerDisabledButClientRequired;
  }
  *this << int32_t(aeOK);

  if( (sc == radRequired || sc == radAllow) && (cc == radRequired || cc == radAllow) )
    activateCompression(smethod,scrc,ap.level_,ap.optimize_,ap.bufferSize_);
  if( (se == radRequired || se == radAllow) && (ce == radRequired || ce == radAllow) ){
    this->threshold(ap.threshold_);
    activateEncryption(passwordSHA256,sizeof(passwordSHA256));
  }
  return aeOK;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::activateCompression(uintptr_t method,uintptr_t crc,uintptr_t level,bool optimize,uintptr_t wBufSize)
{
  if( method <= ksys::LZO1X::LZO1X_999 ){
    ksys::LZO1X::wBufSize(wBufSize > 1024 * 1024 * 1024 ? ksys::LZO1X::wBufSize() : (uint32_t) wBufSize);
    ksys::LZO1X::method(method);
    ksys::LZO1X::crc(crc);
    ksys::LZO1X::level(level);
    ksys::LZO1X::optimize(optimize);
    ksys::LZO1X::active(true);
  }
  else if( method == ksys::LZO1X::LZO1X_999 + 2 ){ // range32
    compressor_ = newObject<ksys::Range32CoderFilter>();
  }
  else if( method == ksys::LZO1X::LZO1X_999 + 3 ){ // lzssrbt
    compressor_ = newObject<ksys::LZSSRBTFilter>();
  }
  if( compressor_ != NULL ){
    wBufSize = wBufSize > 1024 * 1024 * 1024 ? 1024 * 1024 * 1024 : wBufSize > 0 ? wBufSize : getpagesize() * 16u;
    compressor_->encoderOutputBufferSize(wBufSize);
    compressor_->decoderInputBufferSize(wBufSize);
    compressor_->initializeEncoder();
    compressor_->initializeDecoder();
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::deActivateCompression()
{
  compressor_ = NULL;
  ksys::LZO1X::active(false);
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::activateEncryption(const void * key, uintptr_t keyLen)
{
  if( compressor_ != NULL ){
    cryptor_ = newObject<ksys::SHA256CryptFilter>();
    cryptor_->initializeCrypting(key,keyLen);
    cryptor_->initializeDecrypting(key,keyLen);
  }
  else {
    init(key,keyLen);
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::deActivateEncryption()
{
  cryptor_ = NULL;
  ksys::SHA256Filter::active(false);
  return *this;
}
//------------------------------------------------------------------------------
AsyncSocket::AuthErrorType AsyncSocket::clientAuth(const AuthParams & ap)
{
  ksys::currentFiber()->checkFiberStackOverflow();
  maxRecvSize_ = ap.maxRecvSize_;
  maxSendSize_ = ap.maxSendSize_;
  recvTimeout_ = ap.recvTimeout_;
  sendTimeout_ = ap.sendTimeout_;
  union {
    int32_t e;
    AuthErrorType ae;
  };
  e = ae = aeOK;
  uint8_t passwordSHA256[32];
  uint8_t se = radDisabled, ce = radDisabled;
  if( !ap.noAuth_ ){
    write(authMagic_,sizeof(authMagic_));
    *this >> e;
    if( e != aeOK ) return ae;
    *this << ap.user_;
    *this >> e;
    if( e != aeOK ) return ae;

    memset(passwordSHA256,0,sizeof(passwordSHA256));
    ksys::SHA256 SHA256;
    if( ap.password_.ncasecompare("sha256:",7) == 0 ){
      ksys::base64Decode(
        utf8::String::Iterator(ap.password_) + 7,
        passwordSHA256,
        sizeof(passwordSHA256)
      );
    }
    else {
      SHA256.make(ap.password_.c_str(),ap.password_.size());
      memcpy(passwordSHA256,SHA256.sha256(),sizeof(passwordSHA256));
    }
    SHA256.make(passwordSHA256,sizeof(passwordSHA256));
    write(SHA256.sha256(),sizeof(passwordSHA256));
    *this >> e;
    if( e != aeOK ) return ae;

    ce = authChannelHelper(ap.encryption_);
    *this << ce;
    *this >> se >> e;
    if( e != aeOK ) return ae;
  }
  uint8_t sc, cc, smethod, cmethod, scrc, ccrc, slevel;
  cc = authChannelHelper(ap.compression_);
  cmethod = authChannelHelper2(ap.compressionType_);
  ccrc = authChannelHelper3(ap.crc_);
  *this << cc << cmethod << ccrc << uint8_t(ap.level_) << uint32_t(ap.bufferSize_) << uint32_t(ap.threshold_);
  uint32_t sbs, sthreshold;
  *this >> sc >> smethod >> scrc >> slevel >> sbs >> sthreshold;
  *this >> e;
  if( e != aeOK ) return ae;

  if( (sc == radRequired || sc == radAllow) && (cc == radRequired || cc == radAllow) )
    activateCompression(cmethod,scrc,ap.level_,ap.optimize_,ap.bufferSize_);
  if( (se == radRequired || se == radAllow) && (ce == radRequired || ce == radAllow) ){
    this->threshold(sthreshold);
    activateEncryption(passwordSHA256,sizeof(passwordSHA256));
  }
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
//----------------------------------------
  // Load the AcceptEx function into memory using WSAIoctl.
  // The WSAIoctl function is an extension of the ioctlsocket()
  // function that can use overlapped I/O. The function's 3rd
  // through 6th parameters are input and output buffers where
  // we pass the pointer to our AcceptEx function. This is used
  // so that we can call the AcceptEx function directly, rather
  // than refer to the Mswsock.lib library.
  LPFN_ACCEPTEX lpfnAcceptEx = NULL;
  GUID GuidAcceptEx = WSAID_ACCEPTEX;
  DWORD dwBytes;
  api.WSAIoctl(socket_,
    SIO_GET_EXTENSION_FUNCTION_POINTER, 
    &GuidAcceptEx,
    sizeof(GuidAcceptEx),
    &lpfnAcceptEx,
    sizeof(lpfnAcceptEx),
    &dwBytes,
    NULL,
    NULL
  );
  return lpfnAcceptEx(socket_,sAcceptSocket,lpOutputBuffer,dwReceiveDataLength,dwLocalAddressLength,dwRemoteAddressLength,lpdwBytesReceived,lpOverlapped);
}
//---------------------------------------------------------------------------
BOOL AsyncSocket::Connect(HANDLE event,ksys::AsyncEvent * request)
{
  if( api.WSAEventSelect(socket_,event,FD_CONNECT) != 0 ) return FALSE;
  SetLastError(ERROR_SUCCESS);
  return
    api.connect(
      socket_,
      (struct sockaddr *) &((SockAddr *) request->address_)->addr4_,
      ((SockAddr *) request->address_)->sockAddrSize()
    ) != SOCKET_ERROR ? TRUE : FALSE;
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
#else
//---------------------------------------------------------------------------
int AsyncSocket::listen(int)
{
  errno = 0;
  return api.listen(socket_,SOMAXCONN);
}
//---------------------------------------------------------------------------
int AsyncSocket::accept()
{
  errno = 0;
  return api.accept(socket_,NULL,NULL);
}
//---------------------------------------------------------------------------
void AsyncSocket::connect(ksys::AsyncEvent * request)
{
  errno = 0;
  api.connect(
    socket_,
    (const struct sockaddr *) &((SockAddr *) request->address_)->addr4_,
    (socklen_t) ((SockAddr *) request->address_)->sockAddrSize()
  );
}
//---------------------------------------------------------------------------
int64_t AsyncSocket::read2(void * buf,uint64_t len)
{
  return api.recv(socket_,(char *) buf,(size_t) len,0);
}
//---------------------------------------------------------------------------
int64_t AsyncSocket::write2(const void * buf,uint64_t len)
{
  return api.send(socket_,(const char *) buf,(size_t) len,0);
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
void AsyncSocket::close2()
{
  close();
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::getSockAddr(SockAddr & addr) const
{
  int32_t err = -1;
  socklen_t len;
#if HAVE_STRUCT_SOCKADDR_IN6
  len = sizeof(addr.addr6_);
  err = api.getsockname(socket_,(sockaddr *) &addr.addr6_,&len);
#endif
  if( err != 0 ){
    len = sizeof(addr.addr4_);
    err = api.getsockname(socket_,(sockaddr *) &addr.addr4_,&len);
  }
  if( err != 0 ) err = errNo();
  if( err != 0 )
    newObjectV1C2<EAsyncSocket>(err,__PRETTY_FUNCTION__)->throwSP();
  return *const_cast<AsyncSocket *>(this);
}
//------------------------------------------------------------------------------
AsyncSocket & AsyncSocket::getPeerAddr(SockAddr & addr) const
{
  int32_t err = -1;
  socklen_t len;
#if HAVE_STRUCT_SOCKADDR_IN6
  len = sizeof(addr.addr6_);
  err = api.getpeername(socket_,(sockaddr *) &addr.addr6_,&len);
#endif
  if( err != 0 ){
    len = sizeof(addr.addr4_);
    err = api.getpeername(socket_,(sockaddr *) &addr.addr4_,&len);
  }
  if( err != 0 ) err = errNo();
  if( err != 0 )
    newObjectV1C2<EAsyncSocket>(err,__PRETTY_FUNCTION__)->throwSP();
  return *const_cast<AsyncSocket *>(this);
}
//------------------------------------------------------------------------------
} // namespace ksock
//------------------------------------------------------------------------------
