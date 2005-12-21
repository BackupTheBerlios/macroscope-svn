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
#ifndef _scktintf_H_
#define _scktintf_H_
//---------------------------------------------------------------------------
namespace ksock {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class API {
  friend void ksys::initialize();
  friend void ksys::cleanup();
  public:
#if defined(__WIN32__) || defined(__WIN64__)
    union {
      struct {
        union {
          int (WSAAPI * WSAGetLastError)(void);	
          void * p_WSAGetLastError;
        };
        union {
          void (WSAAPI * WSASetLastError)(int iError);
          void * p_WSASetLastError;
        };
        union {
          int (WSAAPI * WSAStartup)(WORD wVersionRequested,LPWSADATA lpWSAData);
          void * p_WSAStartup;
        };
        union {
          int (WSAAPI * WSACleanup)(void);
          void * p_WSACleanup;
        };
        union {
          int (WSAAPI * WSAAsyncSelect)(SOCKET s,HWND hWnd,unsigned int wMsg,long lEvent);
          void * p_WSAAsyncSelect;
        };
        union {
          int (WSAAPI * socket)(int domain, int type, int protocol);
          void * p_socket;
        };
        union {
          int (WSAAPI * closesocket)(SOCKET s);
          void * p_closesocket;
        };
        union {
          int (WSAAPI * shutdown)(SOCKET s,int how);
          void * p_shutdown;
        };
        union {
          int (WSAAPI * bind)(SOCKET s,const struct sockaddr FAR * name,socklen_t namelen);
          void * p_bind;
        };
        union {
          int (WSAAPI * listen)(SOCKET s,int backlog);
          void * p_listen;
        };
        union {
          SOCKET (WSAAPI * accept)(SOCKET s,struct sockaddr FAR * addr,socklen_t FAR * addrlen);
          void * p_accept;
        };
        union {
          int (WSAAPI * recv)(SOCKET s,char FAR* buf,int len,int flags);
          void * p_recv;
        };
        union {
          int (WSAAPI * recvfrom)(SOCKET s,char FAR* buf,int len,int flags,struct sockaddr FAR* from,socklen_t FAR* fromlen);
          void * p_recvfrom;
        };
        union {
          int (WSAAPI * send)(SOCKET s,const char FAR * buf,int len,int flags);
          void * p_send;
        };
        union {
          int (WSAAPI * sendto)(SOCKET s,const char FAR * buf,int len,int flags,const struct sockaddr FAR * to,socklen_t tolen);
          void * p_sendto;
        };
        union {
          int (WSAAPI * getsockopt)(SOCKET s,int level,int optname,char FAR * optval,socklen_t FAR * optlen);
          void * p_getsockopt;
        };
        union {
          int (WSAAPI * setsockopt)(SOCKET s,int level,int optname,const char FAR * optval,socklen_t optlen);
          void * p_setsockopt;
        };
        union {
          int (WSAAPI * connect)(SOCKET s,const struct sockaddr FAR * name,socklen_t namelen);
          void * p_connect;
        };
        union {
          u_long (WSAAPI * htonl)(u_long hostlong);
          void * p_htonl;
        };
        union {
          u_long (WSAAPI * ntohl)(u_long netlong);
          void * p_ntohl;
        };
        union {
          u_short (WSAAPI * htons)(u_short hostshort);
          void * p_htons;
        };
        union {
          u_short (WSAAPI * ntohs)(u_short netshort);
          void * p_ntohs;
        };
        union {
          char FAR * (WSAAPI * inet_ntoa)(struct in_addr in);
          void * p_inet_ntoa;
        };
        union {
          unsigned long (WSAAPI * inet_addr)(const char FAR * cp);
          void * p_inet_addr;
        };
        union {
          int (WSAAPI * WSARecv)(
            SOCKET s,	
            LPWSABUF lpBuffers,	
            DWORD dwBufferCount,	
            LPDWORD lpNumberOfBytesRecvd,	
            LPDWORD lpFlags,	
            LPWSAOVERLAPPED lpOverlapped,	
            LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionROUTINE	
          );
          void * p_WSARecv;
        };
        union {
          int (WSAAPI * WSASend)(
            SOCKET s,	
            LPWSABUF lpBuffers,	
            DWORD dwBufferCount,	
            LPDWORD lpNumberOfBytesSent,	
            DWORD dwFlags,	
            LPWSAOVERLAPPED lpOverlapped,	
            LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionROUTINE	
          );
          void * p_WSASend;
        };
        union {
          SOCKET (WSAAPI * WSASocketA)(
            int af,
            int type,
            int protocol,
            LPWSAPROTOCOL_INFO lpProtocolInfo,
            GROUP g,
            DWORD dwFlags
          );
          void * p_WSASocketA;
        };
        union {
          SOCKET (WSAAPI * WSASocketW)(
            int af,
            int type,
            int protocol,
            LPWSAPROTOCOL_INFO lpProtocolInfo,
            GROUP g,
            DWORD dwFlags
          );
          void * p_WSASocketW;
        };
        union {
          BOOL (WSAAPI * WSAGetOverlappedResult)(
            SOCKET s,	
            LPWSAOVERLAPPED lpOverlapped,	
            LPDWORD lpcbTransfer,	
            BOOL fWait,	
            LPDWORD lpdwFlags	
          );
          void * p_WSAGetOverlappedResult;
        };
        union {
          struct hostent * (WSAAPI * gethostbyname)(const char * name);
          void * p_gethostbyname;
        };
        union {
          int (WSAAPI * WSAEventSelect)(SOCKET s,WSAEVENT hEventObject,long lNetworkEvents);
          void * p_WSAEventSelect;
        };
        union {
          int (WSAAPI * WSAEnumNetworkEvents)(
            SOCKET s,
            WSAEVENT hEventObject,
            LPWSANETWORKEVENTS lpNetworkEvents
          );
        };
      };
    };
    void open();
    void close();
#else
    int socket(int domain, int type, int protocol){
      return ::socket(domain,type,protocol);
    }
    int closesocket(int s){
      return ::close(s);
    }
    int shutdown(int s, int how){
      return ::shutdown(s,how);
    }
    int bind(int s,const struct sockaddr * name,int namelen){
      return ::bind(s,name,namelen);
    }
    int listen(int s,int backlog){
      return ::listen(s,backlog);
    }
    int accept(int s,struct sockaddr * addr,socklen_t * addrlen){
      return ::accept(s,addr,addrlen);
    }
    int recv(int s,char * buf,int len,int flags){
      return ::recv(s,buf,len,flags);
    }
    int recvfrom(int s,char * buf,int len,int flags,struct sockaddr * from,socklen_t * fromlen){
      return ::recvfrom(s,buf,len,flags,from,fromlen);
    }
    int send(int s,const char * buf,int len,int flags){
      return ::send(s,buf,len,flags);
    }
    int sendto(int s,const char * buf,int len,int flags,const struct sockaddr * to,socklen_t tolen){
      return ::sendto(s,buf,len,flags,to,tolen);
    }
    int getsockopt(int s,int level,int optname,char * optval,socklen_t * optlen){
      return ::getsockopt(s,level,optname,optval,optlen);
    }
    int setsockopt(int s,int level,int optname,const char * optval,socklen_t optlen){
      return ::setsockopt(s,level,optname,optval,optlen);
    }
    int connect(int s,const struct sockaddr * name,int namelen){
      return ::connect(s,name,namelen);
    }
    uint32_t htonl(uint32_t hostlong){
      return ::htonl(hostlong);
    }
#undef ntohl
    uint32_t ntohl(uint32_t netlong){
      return ::ntohl(netlong);
    }
    uint16_t htons(uint16_t hostshort){
      return ::htons(hostshort);
    }
#undef ntohs
    uint16_t ntohs(uint16_t netshort){
      return ::ntohs(netshort);
    }
    char * inet_ntoa(struct in_addr in){
      return ::inet_ntoa(in);
    }
    in_addr_t inet_addr(const char * cp){
      return ::inet_addr(cp);
    }
    struct hostent * gethostbyname(const char * name){
      return ::gethostbyname(name);
    }
    void open() {}
    void close() {}
#endif
  protected:
#if defined(__WIN32__) || defined(__WIN64__)
    static WSADATA wsaData_;
    static HINSTANCE handle_;

    static uint8_t mutex_[];
    static ksys::InterlockedMutex & mutex();
    static uintptr_t count_;

    static const char * const symbols_[];
#endif
  private:
    void initialize();
    void cleanup();
};
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
inline ksys::InterlockedMutex & API::mutex()
{
  return *reinterpret_cast<ksys::InterlockedMutex *>(mutex_);
}
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
class APIEx {
  friend class API;
  public:
    union {
      struct {
        union {
          BOOL (WSAAPI * AcceptEx)( 
            SOCKET sListenSocket,	
            SOCKET sAcceptSocket,	
            PVOID lpOutputBuffer,	
            DWORD dwReceiveDataLength,	
            DWORD dwLocalAddressLength,	
            DWORD dwRemoteAddressLength,	
            LPDWORD lpdwBytesReceived,	
            LPOVERLAPPED lpOverlapped	
          );
          void * p_AcceptEx;
        };
        union {
          VOID (WSAAPI * GetAcceptExSockaddrs)(
            PVOID lpOutputBuffer,
            DWORD dwReceiveDataLength,
            DWORD dwLocalAddressLength,
            DWORD dwRemoteAddressLength,
            LPSOCKADDR *LocalSockaddr,
            LPINT LocalSockaddrLength,
            LPSOCKADDR *RemoteSockaddr,
            LPINT RemoteSockaddrLength
          );
          void * p_GetAcceptExSockaddrs;
        };
      };
    };
  protected:
    static const char * const symbols_[];
    static HINSTANCE handle_;
  private:
};
//---------------------------------------------------------------------------
extern APIEx apiEx;
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
extern API api;
//---------------------------------------------------------------------------
} // namespace ksock
//---------------------------------------------------------------------------
#endif /* _scktintf_H_ */
//---------------------------------------------------------------------------
