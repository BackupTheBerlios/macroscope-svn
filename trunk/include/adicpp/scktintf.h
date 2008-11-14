/*-
 * Copyright 2005-2007 Guram Dukashvili
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
  friend void ksys::initialize(int,char **);
  friend void ksys::cleanup();
  public:
#if defined(__WIN32__) || defined(__WIN64__)
#ifdef USE_STATIC_SOCKET_LIBRARY
    int WSAGetLastError(void){
      return ::WSAGetLastError();
    }
    void WSASetLastError(int iError){
      ::WSASetLastError(iError);
    }
    int WSAStartup(WORD wVersionRequested,LPWSADATA lpWSAData){
      return ::WSAStartup(wVersionRequested,lpWSAData);
    }
    int WSACleanup(void){
      return ::WSACleanup();
    }
    int WSAAsyncSelect(SOCKET s, HWND hWnd, unsigned int wMsg, long lEvent){
      return ::WSAAsyncSelect(s,hWnd,wMsg,lEvent);
    }
    SOCKET socket(int domain, int type, int protocol){
      return ::socket(domain,type,protocol);
    }
    int closesocket(SOCKET s){
      return ::closesocket(s);
    }
    int shutdown(SOCKET s, int how){
      return ::shutdown(s,how);
    }
    int bind(SOCKET s, const struct sockaddr FAR * name, socklen_t namelen){
      return ::bind(s,name,namelen);
    }
    int listen(SOCKET s, int backlog){
      return ::listen(s,backlog);
    }
    SOCKET accept(SOCKET s, struct sockaddr FAR * addr, socklen_t FAR * addrlen){
      return ::accept(s,addr,addrlen);
    }
    int recv(SOCKET s, char FAR * buf, int len, int flags){
      return ::recv(s,buf,len,flags);
    }
    int recvfrom(SOCKET s, char FAR * buf, int len, int flags, struct sockaddr FAR * from, socklen_t FAR * fromlen){
      return ::recvfrom(s,buf,len,flags,from,fromlen);
    }
    int send(SOCKET s, const char FAR * buf, int len, int flags){
      return ::send(s,buf,len,flags);
    }
    int sendto(SOCKET s, const char FAR * buf, int len, int flags, const struct sockaddr FAR * to, socklen_t tolen){
      return ::sendto(s,buf,len,flags,to,tolen);
    }
    int getsockopt(SOCKET s, int level, int optname, char FAR * optval, socklen_t FAR * optlen){
      return ::getsockopt(s,level,optname,optval,optlen);
    }
    int setsockopt(SOCKET s, int level, int optname, const char FAR * optval, socklen_t optlen){
      return ::setsockopt(s,level,optname,optval,optlen);
    }
    int connect(SOCKET s,const struct sockaddr FAR * name,socklen_t namelen){
      return ::connect(s,name,namelen);
    }
    u_long htonl(u_long hostlong){
      return ::htonl(hostlong);
    }
    u_long ntohl(u_long netlong){
      return ::ntohl(netlong);
    }
    u_short htons(u_short hostshort){
      return ::htons(hostshort);
    }
    u_short ntohs(u_short netshort){
      return ::ntohs(netshort);
    }
    char FAR * inet_ntoa(struct in_addr in){
      return ::inet_ntoa(in);
    }
    unsigned long inet_addr(const char FAR * cp){
      return ::inet_addr(cp);
    }
    int WSARecv(
      SOCKET s,
      LPWSABUF lpBuffers,
      DWORD dwBufferCount,
      LPDWORD lpNumberOfBytesRecvd,
      LPDWORD lpFlags,
      LPWSAOVERLAPPED lpOverlapped,
      LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionROUTINE)
    {
      return ::WSARecv(
        s,lpBuffers,dwBufferCount,lpNumberOfBytesRecvd,lpFlags,lpOverlapped,lpCompletionROUTINE
      );
    }
    int WSASend(
      SOCKET s,
      LPWSABUF lpBuffers,
      DWORD dwBufferCount,
      LPDWORD lpNumberOfBytesSent,
      DWORD dwFlags,
      LPWSAOVERLAPPED lpOverlapped,
      LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionROUTINE)
    {
      return ::WSASend(
        s,lpBuffers,dwBufferCount,lpNumberOfBytesSent,dwFlags,lpOverlapped,lpCompletionROUTINE
      );
    }
    SOCKET WSASocketA(int af, int type, int protocol, LPWSAPROTOCOL_INFOA lpProtocolInfo, GROUP g, DWORD dwFlags){
      return ::WSASocketA(af,type,protocol,lpProtocolInfo,g,dwFlags);
    }
    SOCKET WSASocketW(int af, int type, int protocol,LPWSAPROTOCOL_INFOW lpProtocolInfo, GROUP g, DWORD dwFlags){
      return ::WSASocketW(af,type,protocol,lpProtocolInfo,g,dwFlags);
    }
    BOOL WSAGetOverlappedResult(SOCKET s,LPWSAOVERLAPPED lpOverlapped, LPDWORD lpcbTransfer,BOOL fWait,LPDWORD lpdwFlags){
      return ::WSAGetOverlappedResult(s,lpOverlapped,lpcbTransfer,fWait,lpdwFlags);
    }
    struct hostent * gethostbyname(const char * name){
      return ::gethostbyname(name);
    }
    struct hostent * gethostbyaddr(const char FAR * addr,int len,int type)
    {
      return ::gethostbyaddr(addr,len,type);
    }
    int gethostname(char * name,int namelen)
    {
      return ::gethostname(name,namelen);
    }
    int WSAEventSelect(SOCKET s, WSAEVENT hEventObject,long lNetworkEvents){
      return ::WSAEventSelect(s,hEventObject,lNetworkEvents);
    }
    int WSAEnumNetworkEvents(SOCKET s, WSAEVENT hEventObject,LPWSANETWORKEVENTS lpNetworkEvents){
      return ::WSAEnumNetworkEvents(s,hEventObject,lpNetworkEvents);
    }
    HANDLE WSAAsyncGetHostByName(HWND hWnd,unsigned int wMsg,const char * name,char * buf,int buflen){
      return WSAAsyncGetHostByName(hWnd,wMsg,name,buf,buflen);
    }
    int getaddrinfo(const char FAR * nodename,const char FAR * servname,const struct addrinfo FAR * hints,struct addrinfo FAR * FAR * res)
    {
      return ::getaddrinfo(nodename,servname,hints,res);
    }
    int GetAddrInfoA(const char FAR * nodename,const char FAR * servname,const struct addrinfo FAR * hints,struct addrinfo FAR * FAR * res)
    {
      return ::getaddrinfo(nodename,servname,hints,res);
    }
    int GetAddrInfoW(PCWSTR pNodeName,PCWSTR pServiceName,const ADDRINFOW * pHints,PADDRINFOW * ppResult)
    {
      return ::GetAddrInfoW(pNodeName,pServiceName,pHints,ppResult);
    }
    void freeaddrinfo(LPADDRINFO pAddrInfo){
      ::freeaddrinfo(pAddrInfo);
    }
    void FreeAddrInfoA(LPADDRINFO pAddrInfo){
      ::freeaddrinfo(pAddrInfo);
    }
    void FreeAddrInfoW(PADDRINFOW pAddrInfo)
    {
      return ::FreeAddrInfoW(pAddrInfo);
    }
    int getnameinfo(const struct sockaddr FAR * sa,socklen_t salen,char FAR * host,DWORD hostlen,char FAR * serv,DWORD servlen,int flags)
    {
      return ::getnameinfo(sa,salen,host,hostlen,serv,servlen,flags);
    }
    int GetNameInfoA(const struct sockaddr FAR * sa,socklen_t salen,char FAR * host,DWORD hostlen,char FAR * serv,DWORD servlen,int flags)
    {
      return ::getnameinfo(sa,salen,host,hostlen,serv,servlen,flags);
    }
    INT GetNameInfoW(const SOCKADDR * pSockaddr,socklen_t SockaddrLength,PWCHAR pNodeBuffer,DWORD NodeBufferSize,PWCHAR pServiceBuffer,DWORD ServiceBufferSize,INT Flags)
    {
      return ::GetNameInfoW(pSockaddr,SockaddrLength,pNodeBuffer,NodeBufferSize,pServiceBuffer,ServiceBufferSize,Flags);
    }
    int getsockname(SOCKET s,struct sockaddr * name,int * namelen)
    {
      return ::getsockname(SOCKET s,name,namelen);
    }
    int getpeername(SOCKET s,struct sockaddr* name,int* namelen)
    {
      return ::getpeername(s,name,namelen);
    }
    int WSAIoctl(
      SOCKET s,
      DWORD dwIoControlCode,
      LPVOID lpvInBuffer,
      DWORD cbInBuffer,
      LPVOID lpvOutBuffer,
      DWORD cbOutBuffer,
      LPDWORD lpcbBytesReturned,
      LPWSAOVERLAPPED lpOverlapped,
      LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine){
        return ::WSAIoctl(s,dwIoControlCode,lpvInBuffer,cbInBuffer,lpvOutBuffer,lpcbBytesReturned,lpOverlapped,lpCompletionRoutine);
    }
  };
#else
#if _MSC_VER
#pragma warning(push,3)
#endif
    struct {
      union {
        int (WSAAPI * WSAGetLastError)(void); 
        void *  p_WSAGetLastError;
      };
      union {
        void (WSAAPI * WSASetLastError)(int iError);
        void *  p_WSASetLastError;
      };
      union {
        int (WSAAPI * WSAStartup)(WORD wVersionRequested,
                                  LPWSADATA lpWSAData);
        void *  p_WSAStartup;
      };
      union {
        int (WSAAPI * WSACleanup)(void);
        void *  p_WSACleanup;
      };
      union {
        int (WSAAPI * WSAAsyncSelect)(SOCKET s, HWND hWnd,
                                      unsigned int wMsg, long lEvent);
        void *  p_WSAAsyncSelect;
      };
      union {
        SOCKET (WSAAPI * socket)(int domain, int type, int protocol);
        void *  p_socket;
      };
      union {
        int (WSAAPI * closesocket)(SOCKET s);
        void *  p_closesocket;
      };
      union {
        int (WSAAPI * shutdown)(SOCKET s, int how);
        void *  p_shutdown;
      };
      union {
        int (WSAAPI * bind)(SOCKET s,
                            const struct sockaddr FAR * name,
                            socklen_t namelen);
        void *  p_bind;
      };
      union {
        int (WSAAPI * listen)(SOCKET s, int backlog);
        void *  p_listen;
      };
      union {
        SOCKET (WSAAPI * accept)(SOCKET s, struct sockaddr FAR * addr,
                                 socklen_t FAR * addrlen);
        void *  p_accept;
      };
      union {
        int (WSAAPI * recv)(SOCKET s, char FAR * buf, int len,
                            int flags);
        void *  p_recv;
      };
      union {
        int (WSAAPI * recvfrom)(SOCKET s, char FAR * buf, int len,
                                int flags, struct sockaddr FAR * from,
                                socklen_t FAR * fromlen);
        void *  p_recvfrom;
      };
      union {
        int (WSAAPI * send)(SOCKET s, const char FAR * buf, int len,int flags);
        void *  p_send;
      };
      union {
        int (WSAAPI * sendto)(SOCKET s, const char FAR * buf, int len,
                              int flags,
                              const struct sockaddr FAR * to,
                              socklen_t tolen);
        void *  p_sendto;
      };
      union {
        int (WSAAPI * getsockopt)(SOCKET s, int level, int optname,
                                  char FAR * optval,
                                  socklen_t FAR * optlen);
        void *  p_getsockopt;
      };
      union {
        int (WSAAPI * setsockopt)(SOCKET s, int level, int optname,
                                  const char FAR * optval,
                                  socklen_t optlen);
        void *  p_setsockopt;
      };
      union {
        int (WSAAPI * connect)(SOCKET s,
                               const struct sockaddr FAR * name,
                               socklen_t namelen);
        void *  p_connect;
      };
      union {
        u_long (WSAAPI * htonl)(u_long hostlong);
        void *  p_htonl;
      };
      union {
        u_long (WSAAPI * ntohl)(u_long netlong);
        void *  p_ntohl;
      };
      union {
        u_short (WSAAPI * htons)(u_short hostshort);
        void *  p_htons;
      };
      union {
        u_short (WSAAPI * ntohs)(u_short netshort);
        void *  p_ntohs;
      };
      union {
        char FAR * (WSAAPI * inet_ntoa)(struct in_addr in);
        void *  p_inet_ntoa;
      };
      union {
        unsigned long (WSAAPI * inet_addr)(const char FAR * cp);
        void *  p_inet_addr;
      };
      union {
        int (WSAAPI * WSARecv)(
          SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount,
          LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags,
          LPWSAOVERLAPPED lpOverlapped,
          LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionROUTINE);
        void *  p_WSARecv;
      };
      union {
        int (WSAAPI * WSASend)(
          SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount,
          LPDWORD lpNumberOfBytesSent, DWORD dwFlags,
          LPWSAOVERLAPPED lpOverlapped,
          LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionROUTINE);
        void *  p_WSASend;
      };
      union {
        SOCKET (WSAAPI * WSASocketA)(
          int af, int type, int protocol,
          LPWSAPROTOCOL_INFOA lpProtocolInfo, GROUP g, DWORD dwFlags);
        void *  p_WSASocketA;
      };
      union {
        SOCKET (WSAAPI * WSASocketW)(
          int af, int type, int protocol,
          LPWSAPROTOCOL_INFOW lpProtocolInfo, GROUP g, DWORD dwFlags);
        void *  p_WSASocketW;
      };
      union {
        BOOL (WSAAPI * WSAGetOverlappedResult)(
          SOCKET s,
          LPWSAOVERLAPPED lpOverlapped, LPDWORD lpcbTransfer,
          BOOL fWait,
          LPDWORD lpdwFlags);
        void *  p_WSAGetOverlappedResult;
      };
      union {
        struct hostent * (WSAAPI * gethostbyname)(const char * name);
        void *  p_gethostbyname;
      };
      union {
        struct hostent * (WSAAPI * gethostbyaddr)(const char FAR * addr,int len,int type);
        void * p_gethostbyaddr;
      };
      union {
        int (WSAAPI * gethostname)(char * name,int namelen);
        void * p_gethostname;
      };
      union {
        int (WSAAPI * WSAEventSelect)(SOCKET s, WSAEVENT hEventObject,long lNetworkEvents);
        void *  p_WSAEventSelect;
      };
      union {
        int (WSAAPI * WSAEnumNetworkEvents)(SOCKET s,WSAEVENT hEventObject,LPWSANETWORKEVENTS lpNetworkEvents);
      };
      union {
        HANDLE (WSAAPI * WSAAsyncGetHostByName)(
          HWND hWnd,
          unsigned int wMsg,
          const char * name,
          char * buf,
          int buflen
        );
        void * p_WSAAsyncGetHostByName;
      };
      union {
        int (WSAAPI * getsockname)(SOCKET s,struct sockaddr * name,int * namelen);
        void * p_getsockname;
      };
      union {
        int (WSAAPI * getpeername)(SOCKET s,struct sockaddr* name,int* namelen);
        void * p_getpeername;
      };
      union {
        int (WSAAPI * getaddrinfo)(
            const char FAR * nodename,
            const char FAR * servname,
            const struct addrinfo FAR * hints,
            struct addrinfo FAR * FAR * res
        );
        void * p_getaddrinfo;
      };
      union {
        void (WSAAPI * freeaddrinfo)(LPADDRINFO pAddrInfo);
        void * p_freeaddrinfo;
      };
      union {
        int (WSAAPI * getnameinfo)(
            const struct sockaddr FAR * sa,
            socklen_t       salen,
            char FAR *      host,
            DWORD           hostlen,
            char FAR *      serv,
            DWORD           servlen,
            int             flags
        );
        void * p_getnameinfo;
      };
      union {
        int (WSAAPI * GetAddrInfoW)(
          PCWSTR pNodeName,
          PCWSTR pServiceName,
          const ADDRINFOW * pHints,
          PADDRINFOW * ppResult
        );
        void * p_GetAddrInfoW;
      };
      union {
        void (WSAAPI * FreeAddrInfoW)(PADDRINFOW pAddrInfo);
        void * p_FreeAddrInfoW;
      };
      union {
        INT (WSAAPI * GetNameInfoW)(
          const SOCKADDR *    pSockaddr,
          socklen_t           SockaddrLength,
          PWCHAR              pNodeBuffer,
          DWORD               NodeBufferSize,
          PWCHAR              pServiceBuffer,
          DWORD               ServiceBufferSize,
          INT                 Flags
        );
        void * p_GetNameInfoW;
      };
      union {
        int (WSAAPI * WSAIoctl)(
          SOCKET s,
          DWORD dwIoControlCode,
          LPVOID lpvInBuffer,
          DWORD cbInBuffer,
          LPVOID lpvOutBuffer,
          DWORD cbOutBuffer,
          LPDWORD lpcbBytesReturned,
          LPWSAOVERLAPPED lpOverlapped,
          LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
        void *  p_WSAIoctl;
      };
    };
#if _MSC_VER
#pragma warning(pop)
#endif
#endif
    void  open();
    void  close();
#else
    int socket(int domain, int type, int protocol)
    {
      return ::socket(domain, type, protocol);
    }
    int closesocket(int s)
    {
      return ::close(s);
    }
    int shutdown(int s, int how)
    {
      return ::shutdown(s, how);
    }
    int bind(int s, const struct sockaddr * name, int namelen)
    {
      return ::bind(s, name, namelen);
    }
    int listen(int s, int backlog)
    {
      return ::listen(s, backlog);
    }
    int accept(int s, struct sockaddr * addr, socklen_t * addrlen)
    {
      return ::accept(s, addr, addrlen);
    }
    int recv(int s, char * buf, int len, int flags)
    {
      return ::recv(s, buf, len, flags);
    }
    int recvfrom(int s, char * buf, int len, int flags, struct sockaddr * from, socklen_t * fromlen)
    {
      return ::recvfrom(s, buf, len, flags, from, fromlen);
    }
    int send(int s, const char * buf, int len, int flags)
    {
      return ::send(s, buf, len, flags);
    }
    int sendto(int s, const char * buf, int len, int flags, const struct sockaddr * to, socklen_t tolen)
    {
      return ::sendto(s, buf, len, flags, to, tolen);
    }
    int getsockopt(int s, int level, int optname, char * optval, socklen_t * optlen)
    {
      return ::getsockopt(s, level, optname, optval, optlen);
    }
    int setsockopt(int s, int level, int optname, const char * optval, socklen_t optlen)
    {
      return ::setsockopt(s, level, optname, optval, optlen);
    }
    int connect(int s, const struct sockaddr * name, int namelen)
    {
      return ::connect(s, name, namelen);
    }
#undef htonl
    uint32_t htonl(uint32_t hostlong)
    {
      return ::htonl(hostlong);
    }
#undef ntohl
    uint32_t ntohl(uint32_t netlong)
    {
      return ::ntohl(netlong);
    }
#undef htons
    uint16_t htons(uint16_t hostshort)
    {
      return ::htons(hostshort);
    }
#undef ntohs
    uint16_t ntohs(uint16_t netshort)
    {
      return ::ntohs(netshort);
    }
    char * inet_ntoa(struct in_addr in)
    {
      return ::inet_ntoa(in);
    }
    in_addr_t inet_addr(const char * cp)
    {
      return ::inet_addr(cp);
    }
    struct hostent * gethostbyname(const char * name)
    {
      return ::gethostbyname(name);
    }
    int getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, size_t hostlen, char *serv, size_t servlen, int flags)
    {
      return ::getnameinfo(sa,salen,host,hostlen,serv,servlen,flags);
    }
    int getaddrinfo(const char *hostname, const char *servname,const struct addrinfo *hints, struct addrinfo **res)
    {
      return ::getaddrinfo(hostname,servname,hints,res);
    }
    void freeaddrinfo(struct addrinfo *ai)
    {
      ::freeaddrinfo(ai);
    }
    int getsockname(int s,struct sockaddr * name,socklen_t * namelen)
    {
      return ::getsockname(s,name,namelen);
    }
    int getpeername(int s, struct sockaddr * name,socklen_t * namelen)
    {
      return ::getpeername(s,name,namelen);
    }
    int gethostname(char *name, size_t namelen)
    {
      return ::gethostname(name,namelen);
    }
    void open()
    {
    }
    void close()
    {
    }
#endif
    static ksys::WriteLock & mutex();
  protected:
#if defined(__WIN32__) || defined(__WIN64__)
    static WSADATA            wsaData_;
    static HINSTANCE          handle_;

    static uintptr_t          count_;

    static const char * const symbols_[];
#endif
    static uint8_t            mutex_[];
  private:
    void                            initialize();
    void                            cleanup();
};
//---------------------------------------------------------------------------
inline ksys::WriteLock & API::mutex()
{
  return *reinterpret_cast< ksys::WriteLock *>(mutex_);
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class APIEx {
  friend class API;
  public:
#ifdef USE_STATIC_SOCKET_LIBRARY
    BOOL AcceptEx(
      SOCKET sListenSocket,
      SOCKET sAcceptSocket,
      PVOID lpOutputBuffer,
      DWORD dwReceiveDataLength,
      DWORD dwLocalAddressLength,
      DWORD dwRemoteAddressLength,
      LPDWORD lpdwBytesReceived,
      LPOVERLAPPED lpOverlapped)
    {
      return ::AcceptEx(
        sListenSocket,sAcceptSocket,lpOutputBuffer,dwReceiveDataLength,dwLocalAddressLength,
        dwRemoteAddressLength,lpdwBytesReceived,lpOverlapped
      );
    }
    VOID GetAcceptExSockaddrs(
      PVOID lpOutputBuffer,
      DWORD dwReceiveDataLength,
      DWORD dwLocalAddressLength,
      DWORD dwRemoteAddressLength,
      LPSOCKADDR * LocalSockaddr,
      LPINT LocalSockaddrLength,
      LPSOCKADDR * RemoteSockaddr,
      LPINT RemoteSockaddrLength)
    {
      return ::GetAcceptExSockaddrs(
        lpOutputBuffer,dwReceiveDataLength,dwLocalAddressLength,dwRemoteAddressLength,
        LocalSockaddr,LocalSockaddrLength,RemoteSockaddr,RemoteSockaddrLength
        );
    }
#else
#if _MSC_VER
#pragma warning(push,3)
#endif
    typedef BOOL (WSAAPI * AcceptExPtr)(
      SOCKET sListenSocket,
      SOCKET sAcceptSocket,
      PVOID lpOutputBuffer,
      DWORD dwReceiveDataLength,
      DWORD dwLocalAddressLength,
      DWORD dwRemoteAddressLength,
      LPDWORD lpdwBytesReceived,
      LPOVERLAPPED lpOverlapped
    );
    union {
      struct {
        union {
          AcceptExPtr AcceptEx;
          void * p_AcceptEx;
        };
        union {
          VOID (WSAAPI * GetAcceptExSockaddrs)(
            PVOID lpOutputBuffer,
            DWORD dwReceiveDataLength,
            DWORD dwLocalAddressLength,
            DWORD dwRemoteAddressLength,
            LPSOCKADDR *
            LocalSockaddr,
            LPINT LocalSockaddrLength,
            LPSOCKADDR * RemoteSockaddr, LPINT RemoteSockaddrLength);
          void *  p_GetAcceptExSockaddrs;
        };
      };
    };
#if _MSC_VER
#pragma warning(pop)
#endif
  protected:
    static const char * const symbols_[];
    static HINSTANCE          handle_;
#endif
  private:
};
//---------------------------------------------------------------------------
extern APIEx apiEx;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class IPHLPAPI {
  friend class API;
  public:
#if _MSC_VER
#pragma warning(push,3)
#endif
    struct {
      union {
        DWORD (WINAPI * GetAdaptersAddresses)(
          ULONG Family,
          DWORD Flags,
          PVOID Reserved,
          PIP_ADAPTER_ADDRESSES pAdapterAddresses,
          PULONG pOutBufLen
        );
        void * p_GetAdaptersAddresses;
      };
      union {
        DWORD (WINAPI * GetAdaptersInfo)(PIP_ADAPTER_INFO pAdapterInfo,PULONG pOutBufLen);
        void * p_GetAdaptersInfo;
      };
    };
#if _MSC_VER
#pragma warning(pop)
#endif
  protected:
    static const char * const symbols_[];
    static HINSTANCE handle_;
  private:
};
//---------------------------------------------------------------------------
extern IPHLPAPI iphlpapi;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class WSHIP6API {
  friend class API;
  public:
#if _MSC_VER
#pragma warning(push,3)
#endif
    struct {
      union {
        int (WSAAPI * GetAddrInfoA)(
            const char FAR * nodename,
            const char FAR * servname,
            const struct addrinfo FAR * hints,
            struct addrinfo FAR * FAR * res
        );
        void * p_getaddrinfo;
      };
      union {
        void (WSAAPI * FreeAddrInfoA)(LPADDRINFO pAddrInfo);
        void * p_freeaddrinfo;
      };
      union {
        int (WSAAPI * GetNameInfoA)(
            const struct sockaddr FAR * sa,
            socklen_t       salen,
            char FAR *      host,
            DWORD           hostlen,
            char FAR *      serv,
            DWORD           servlen,
            int             flags
        );
        void * p_getnameinfo;
      };
    };
#if _MSC_VER
#pragma warning(pop)
#endif
  protected:
    static const char * const symbols_[];
    static HINSTANCE handle_;
  private:
};
//---------------------------------------------------------------------------
extern WSHIP6API wship6api;
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
extern API api;
//---------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class APIAutoInitializer {
  public:
    ~APIAutoInitializer() { api.close(); }
    APIAutoInitializer() { api.open(); }
};
//---------------------------------------------------------------------------
} // namespace ksock
//---------------------------------------------------------------------------
#endif /* _scktintf_H_ */
//---------------------------------------------------------------------------
