/*-
 * Copyright 2007 Guram Dukashvili
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
#if HAVE_PCAP_H
#include <pcap.h>
#elif defined(__WIN32__) || defined(__WIN64__)
#define HAVE_PCAP_H 1
#include <adicpp/pcap/pcap.h>
#endif
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
// Module static functions
//------------------------------------------------------------------------------
static utf8::String formatByteLength(uintmax_t len,uintmax_t all)
{
  uintmax_t q, b, c, t1, t2, t3;
  char * postfix;

  q = len * 10000u / all;
  b = q / 100u;
  c = q % 100u;
  if( len >= uintmax_t(1024u) * 1024u * 1024u * 1024u ){
    t2 = 1024u;
    t2 *= 1024u * 1024u * 1024u;
    postfix = "T";
  }
  if( len >= 1024u * 1024u * 1024u ){
    t2 = 1024u * 1024u * 1024u;
    postfix = "G";
  }
  else if( len >= 1024u * 1024u ){
    t2 = 1024u * 1024u;
    postfix = "M";
  }
  else if( len >= 1024u ){
    t2 = 1024u;
    postfix = "K";
  }
  else {
    return utf8::String::print(
      len > 0 ? "%"PRIuMAX"(%"PRIuMAX".%02"PRIuMAX"%%)" :  "-",
      len,
      b,
      c
    );
  }
  t1 = len / t2;
  t3 = len % t2;
  return utf8::String::print(
    len > 0 ? "%"PRIuMAX".%04"PRIuMAX"%s(%"PRIuMAX".%02"PRIuMAX"%%)" :  "-",
    t1,
    uintmax_t(t3 / (t2 / 1024u)),
    postfix,
    b,
    c
  );
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
const uint16_t IPPacketHeader::RF = 0x8000;
const uint16_t IPPacketHeader::DF = 0x4000;
const uint16_t IPPacketHeader::MF = 0x2000;
const uint16_t IPPacketHeader::OFFMASK = 0x1fff;
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
const uint8_t TCPPacketHeader::FIN = 0x01;
const uint8_t TCPPacketHeader::SYN = 0x02;
const uint8_t TCPPacketHeader::RST = 0x04;
const uint8_t TCPPacketHeader::PUSH = 0x08;
const uint8_t TCPPacketHeader::ACK = 0x10;
const uint8_t TCPPacketHeader::URG = 0x20;
const uint8_t TCPPacketHeader::ECE = 0x40;
const uint8_t TCPPacketHeader::CWR = 0x80;
const uint8_t TCPPacketHeader::FLAGS = FIN | SYN | RST | ACK | URG | ECE | CWR;
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class PCAP_API {
  friend class PCAP;
  public:
    ~PCAP_API() {}
    PCAP_API() {}
#if defined(__WIN32__) || defined(__WIN64__)
#define PROTOF(x) (__cdecl * x)
#else
#define PROTOF(x) x
#endif
    void PROTOF(pcap_breakloop)(pcap_t *);
    int	PROTOF(pcap_lookupnet)(const char *,bpf_u_int32 *,bpf_u_int32 *,char *);
    pcap_t * PROTOF(pcap_open_live)(const char *,int,int,int,char *);
    int	PROTOF(pcap_compile)(pcap_t *,struct bpf_program *,char *,int,bpf_u_int32);
    int	PROTOF(pcap_setfilter)(pcap_t *,struct bpf_program *);
    int	PROTOF(pcap_loop)(pcap_t *,int,pcap_handler,u_char *);
    void PROTOF(pcap_freecode)(struct bpf_program *);
    void PROTOF(pcap_close)(pcap_t *);
    int	PROTOF(pcap_findalldevs)(pcap_if_t **,char *);
    void PROTOF(pcap_freealldevs)(pcap_if_t *);
    struct pcap_stat * PROTOF(pcap_stats_ex)(pcap_t *,int *);
    char * PROTOF(pcap_geterr)(pcap_t *);
    int PROTOF(pcap_setmode)(pcap_t *,int mode);

#if defined(__WIN32__) || defined(__WIN64__)
    PCAP_API & open();
    PCAP_API & close();
#else
    PCAP_API & open() { return *this; }
    PCAP_API & close() { return *this; }
#endif
  protected:
#if defined(__WIN32__) || defined(__WIN64__)
    HINSTANCE handle_;
    intptr_t count_;
    uint8_t mutex_[sizeof(InterlockedMutex)];
    InterlockedMutex & mutex(){ return *reinterpret_cast<InterlockedMutex *>(mutex_); }
#endif
  private:
    PCAP_API(const PCAP_API &);
    void operator = (const PCAP_API &);
};
//------------------------------------------------------------------------------
static PCAP_API api;
//------------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//------------------------------------------------------------------------------
PCAP_API & PCAP_API::open()
{
  static const char * const symbols[] = {
    "pcap_breakloop",
    "pcap_lookupnet",
    "pcap_open_live",
    "pcap_compile",
    "pcap_setfilter",
    "pcap_loop",
    "pcap_freecode",
    "pcap_close",
    "pcap_findalldevs",
    "pcap_freealldevs",
    "pcap_stats_ex",
    "pcap_geterr",
    "pcap_setmode"
  };
  AutoLock<InterlockedMutex> lock(api.mutex());
  if( isWin9x() ){
    api.handle_ = LoadLibraryExA("wpcap.dll",NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
  }
  else{
    api.handle_ = LoadLibraryExW(L"wpcap.dll",NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
  }
  if( handle_ == NULL ){
    int32_t err = GetLastError() + errorOffset;
    stdErr.debug(
      9,
      utf8::String::Stream() << "Load wpcap.dll failed\n"
    );
    newObjectV1C2<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
  for( uintptr_t i = 0; i < sizeof(symbols) / sizeof(symbols[0]); i++ ){
    ((void **) &api.pcap_breakloop)[i] = GetProcAddress(handle_,symbols[i]);
    if( ((void **) &api.pcap_breakloop)[i] == NULL ){
      int32_t err = GetLastError() + errorOffset;
      FreeLibrary(handle_);
      handle_ = NULL;
      stdErr.debug(
        9,
        utf8::String::Stream() << "GetProcAddress(\"" << symbols[i] << "\")\n"
      );
      newObjectV1C2<Exception>(err + errorOffset, __PRETTY_FUNCTION__)->throwSP();
    }
  }
  api.count_++;
  return *this;
}
//------------------------------------------------------------------------------
PCAP_API & PCAP_API::close()
{
  AutoLock<InterlockedMutex> lock(mutex());
  assert( count_ > 0 );
  if( count_ == 1 ){
    FreeLibrary(handle_);
    handle_ = NULL;
  }
  count_--;
  return *this;
}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void PCAP::initialize()
{
#if defined(__WIN32__) || defined(__WIN64__)
  new (api.mutex_) InterlockedMutex;
  api.count_ = 0;
#endif
}
//------------------------------------------------------------------------------
void PCAP::cleanup()
{
#if defined(__WIN32__) || defined(__WIN64__)
  api.mutex().~InterlockedMutex();
#endif
}
//------------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
#define IPPROTO_ST              7               /* Stream protocol II */
#define IPPROTO_EGP             8               /* exterior gateway protocol */
#define IPPROTO_PIGP            9               /* private interior gateway */
#define IPPROTO_RCCMON          10              /* BBN RCC Monitoring */
#define IPPROTO_NVPII           11              /* network voice protocol*/
#define IPPROTO_PUP             12              /* pup */
#define IPPROTO_ARGUS           13              /* Argus */
#define IPPROTO_EMCON           14              /* EMCON */
#define IPPROTO_XNET            15              /* Cross Net Debugger */
#define IPPROTO_CHAOS           16              /* Chaos*/
#define IPPROTO_MUX             18              /* Multiplexing */
#define IPPROTO_MEAS            19              /* DCN Measurement Subsystems */
#define IPPROTO_HMP             20              /* Host Monitoring */
#define IPPROTO_PRM             21              /* Packet Radio Measurement */
#define IPPROTO_IDP             22              /* xns idp */
#define IPPROTO_TRUNK1          23              /* Trunk-1 */
#define IPPROTO_TRUNK2          24              /* Trunk-2 */
#define IPPROTO_LEAF1           25              /* Leaf-1 */
#define IPPROTO_LEAF2           26              /* Leaf-2 */
#define IPPROTO_RDP             27              /* Reliable Data */
#define IPPROTO_IRTP            28              /* Reliable Transaction */
#define IPPROTO_TP              29              /* tp-4 w/ class negotiation */
#define IPPROTO_BLT             30              /* Bulk Data Transfer */
#define IPPROTO_NSP             31              /* Network Services */
#define IPPROTO_INP             32              /* Merit Internodal */
#define IPPROTO_SEP             33              /* Sequential Exchange */
#define IPPROTO_3PC             34              /* Third Party Connect */
#define IPPROTO_IDPR            35              /* InterDomain Policy Routing */
#define IPPROTO_XTP             36              /* XTP */
#define IPPROTO_DDP             37              /* Datagram Delivery */
#define IPPROTO_CMTP            38              /* Control Message Transport */
#define IPPROTO_TPXX            39              /* TP++ Transport */
#define IPPROTO_IL              40              /* IL transport protocol */
#define IPPROTO_IPV6            41              /* IP6 header */
#define IPPROTO_SDRP            42              /* Source Demand Routing */
#define IPPROTO_ROUTING         43              /* IP6 routing header */
#define IPPROTO_FRAGMENT        44              /* IP6 fragmentation header */
#define IPPROTO_IDRP            45              /* InterDomain Routing*/
#define IPPROTO_RSVP            46              /* resource reservation */
#define IPPROTO_GRE             47              /* General Routing Encap. */
#define IPPROTO_MHRP            48              /* Mobile Host Routing */
#define IPPROTO_BHA             49              /* BHA */
#define IPPROTO_ESP             50              /* IP6 Encap Sec. Payload */
#define IPPROTO_INLSP           52              /* Integ. Net Layer Security */
#define IPPROTO_SWIPE           53              /* IP with encryption */
#define IPPROTO_NHRP            54              /* Next Hop Resolution */
#define IPPROTO_MOBILE          55              /* IP Mobility */
#define IPPROTO_TLSP            56              /* Transport Layer Security */
#define IPPROTO_SKIP            57              /* SKIP */
#define IPPROTO_ICMPV6          58              /* ICMP6 */
#define IPPROTO_NONE            59              /* IP6 no next header */
#define IPPROTO_DSTOPTS         60              /* IP6 destination option */
#define IPPROTO_AHIP            61              /* any host internal protocol */
#define IPPROTO_CFTP            62              /* CFTP */
#define IPPROTO_HELLO           63              /* "hello" routing protocol */
#define IPPROTO_SATEXPAK        64              /* SATNET/Backroom EXPAK */
#define IPPROTO_KRYPTOLAN       65              /* Kryptolan */
#define IPPROTO_RVD             66              /* Remote Virtual Disk */
#define IPPROTO_IPPC            67              /* Pluribus Packet Core */
#define IPPROTO_ADFS            68              /* Any distributed FS */
#define IPPROTO_SATMON          69              /* Satnet Monitoring */
#define IPPROTO_VISA            70              /* VISA Protocol */
#define IPPROTO_IPCV            71              /* Packet Core Utility */
#define IPPROTO_CPNX            72              /* Comp. Prot. Net. Executive */
#define IPPROTO_CPHB            73              /* Comp. Prot. HeartBeat */
#define IPPROTO_WSN             74              /* Wang Span Network */
#define IPPROTO_PVP             75              /* Packet Video Protocol */
#define IPPROTO_BRSATMON        76              /* BackRoom SATNET Monitoring */
#define IPPROTO_ND              77              /* Sun net disk proto (temp.) */
#define IPPROTO_WBMON           78              /* WIDEBAND Monitoring */
#define IPPROTO_WBEXPAK         79              /* WIDEBAND EXPAK */
#define IPPROTO_EON             80              /* ISO cnlp */
#define IPPROTO_VMTP            81              /* VMTP */
#define IPPROTO_SVMTP           82              /* Secure VMTP */
#define IPPROTO_VINES           83              /* Banyon VINES */
#define IPPROTO_TTP             84              /* TTP */
#define IPPROTO_IGP             85              /* NSFNET-IGP */
#define IPPROTO_DGP             86              /* dissimilar gateway prot. */
#define IPPROTO_TCF             87              /* TCF */
#define IPPROTO_IGRP            88              /* Cisco/GXS IGRP */
#define IPPROTO_OSPFIGP         89              /* OSPFIGP */
#define IPPROTO_SRPC            90              /* Strite RPC protocol */
#define IPPROTO_LARP            91              /* Locus Address Resoloution */
#define IPPROTO_MTP             92              /* Multicast Transport */
#define IPPROTO_AX25            93              /* AX.25 Frames */
#define IPPROTO_IPEIP           94              /* IP encapsulated in IP */
#define IPPROTO_MICP            95              /* Mobile Int.ing control */
#define IPPROTO_SCCSP           96              /* Semaphore Comm. security */
#define IPPROTO_ETHERIP         97              /* Ethernet IP encapsulation */
#define IPPROTO_ENCAP           98              /* encapsulation header */
#define IPPROTO_APES            99              /* any private encr. scheme */
#define IPPROTO_GMTP            100             /* GMTP*/
#define IPPROTO_IPCOMP          108             /* payload compression (IPComp) */
#define IPPROTO_SCTP            132             /* SCTP */
#define IPPROTO_PIM             103             /* Protocol Independent Mcast */
#define IPPROTO_CARP            112             /* CARP */
#define IPPROTO_PGM             113             /* PGM */
#define IPPROTO_PFSYNC          240             /* PFSYNC */
#endif
utf8::String PCAP::protoAsString(uintptr_t proto)
{
  const char * s;
  switch( proto ){
    case IPPROTO_IP              : s = ""; break;
    case IPPROTO_ICMP            : s = "ICMP"; break;
    case IPPROTO_IGMP            : s = "IGMP"; break;
    case IPPROTO_GGP             : s = "GGP"; break;
    case IPPROTO_IPV4            : s = "IPV4";
    case IPPROTO_TCP             : s = "TCP"; break;
    case IPPROTO_ST              : s = "ST"; break;
    case IPPROTO_EGP             : s = "EGP"; break;
    case IPPROTO_PIGP            : s = "PIGP"; break;
    case IPPROTO_RCCMON          : s = "RCCMON"; break;
    case IPPROTO_NVPII           : s = "NVPII"; break;
    case IPPROTO_PUP             : s = "PUP"; break;
    case IPPROTO_ARGUS           : s = "ARGUS"; break;
    case IPPROTO_EMCON           : s = "EMCON"; break;
    case IPPROTO_XNET            : s = "XNET"; break;
    case IPPROTO_CHAOS           : s = "CHAOS"; break;
    case IPPROTO_UDP             : s = "UDP"; break;
    case IPPROTO_MUX             : s = "MUX"; break;
    case IPPROTO_MEAS            : s = "MEAS"; break;
    case IPPROTO_HMP             : s = "HMP"; break;
    case IPPROTO_PRM             : s = "PRM"; break;
    case IPPROTO_IDP             : s = "IDP"; break;
    case IPPROTO_TRUNK1          : s = "TRUNK1"; break;
    case IPPROTO_TRUNK2          : s = "TRUNK2"; break;
    case IPPROTO_LEAF1           : s = "LEAF1"; break;
    case IPPROTO_LEAF2           : s = "LEAF2"; break;
    case IPPROTO_RDP             : s = "RDP"; break;
    case IPPROTO_IRTP            : s = "IRTP"; break;
    case IPPROTO_TP              : s = "TP"; break;
    case IPPROTO_BLT             : s = "BLT"; break;
    case IPPROTO_NSP             : s = "NSP"; break;
    case IPPROTO_INP             : s = "INP"; break;
    case IPPROTO_SEP             : s = "SEP"; break;
    case IPPROTO_3PC             : s = "3PC"; break;
    case IPPROTO_IDPR            : s = "IDPR"; break;
    case IPPROTO_XTP             : s = "XTP"; break;
    case IPPROTO_DDP             : s = "DDP"; break;
    case IPPROTO_CMTP            : s = "CMTP"; break;
    case IPPROTO_TPXX            : s = "TPXX"; break;
    case IPPROTO_IL              : s = "IL"; break;
    case IPPROTO_IPV6            : s = "IPV6"; break;
    case IPPROTO_SDRP            : s = "SDRP"; break;
    case IPPROTO_ROUTING         : s = "ROUTING"; break;
    case IPPROTO_FRAGMENT        : s = "FRAGMENT"; break;
    case IPPROTO_IDRP            : s = "IDRP"; break;
    case IPPROTO_RSVP            : s = "RSVP"; break;
    case IPPROTO_GRE             : s = "GRE"; break;
    case IPPROTO_MHRP            : s = "MHRP"; break;
    case IPPROTO_BHA             : s = "BHA"; break;
    case IPPROTO_ESP             : s = "ESP"; break;
    case IPPROTO_AH              : s = "AH"; break;
    case IPPROTO_INLSP           : s = "INLSP"; break;
    case IPPROTO_SWIPE           : s = "SWIPE"; break;
    case IPPROTO_NHRP            : s = "NHRP"; break;
    case IPPROTO_MOBILE          : s = "MOBILE"; break;
    case IPPROTO_TLSP            : s = "TLSP"; break;
    case IPPROTO_SKIP            : s = "SKIP"; break;
    case IPPROTO_ICMPV6          : s = "ICMPV6"; break;
    case IPPROTO_NONE            : s = "NONE"; break;
    case IPPROTO_DSTOPTS         : s = "DSTOPTS"; break;
    case IPPROTO_AHIP            : s = "AHIP"; break;
    case IPPROTO_CFTP            : s = "CFTP"; break;
    case IPPROTO_HELLO           : s = "HELLO"; break;
    case IPPROTO_SATEXPAK        : s = "SATEXPAK"; break;
    case IPPROTO_KRYPTOLAN       : s = "KRYPTOLAN"; break;
    case IPPROTO_RVD             : s = "RVD"; break;
    case IPPROTO_IPPC            : s = "IPPC"; break;
    case IPPROTO_ADFS            : s = "ADFS"; break;
    case IPPROTO_SATMON          : s = "SATMON"; break;
    case IPPROTO_VISA            : s = "VISA"; break;
    case IPPROTO_IPCV            : s = "IPCV"; break;
    case IPPROTO_CPNX            : s = "CPNX"; break;
    case IPPROTO_CPHB            : s = "CPHB"; break;
    case IPPROTO_WSN             : s = "WSN"; break;
    case IPPROTO_PVP             : s = "PVP"; break;
    case IPPROTO_BRSATMON        : s = "BRSATMON"; break;
    case IPPROTO_ND              : s = "ND"; break;
    case IPPROTO_WBMON           : s = "WBMON"; break;
    case IPPROTO_WBEXPAK         : s = "WBEXPAK"; break;
    case IPPROTO_EON             : s = "EON"; break;
    case IPPROTO_VMTP            : s = "VMTP"; break;
    case IPPROTO_SVMTP           : s = "SVMTP"; break;
    case IPPROTO_VINES           : s = "VINES"; break;
    case IPPROTO_TTP             : s = "TTP"; break;
    case IPPROTO_IGP             : s = "IGP"; break;
    case IPPROTO_DGP             : s = "DGP"; break;
    case IPPROTO_TCF             : s = "TCF"; break;
    case IPPROTO_IGRP            : s = "IGRP"; break;
    case IPPROTO_OSPFIGP         : s = "OSPFIGP"; break;
    case IPPROTO_SRPC            : s = "SRPC"; break;
    case IPPROTO_LARP            : s = "LARP"; break;
    case IPPROTO_MTP             : s = "MTP"; break;
    case IPPROTO_AX25            : s = "AX25"; break;
    case IPPROTO_IPEIP           : s = "IPEIP"; break;
    case IPPROTO_MICP            : s = "MICP"; break;
    case IPPROTO_SCCSP           : s = "SCCSP"; break;
    case IPPROTO_ETHERIP         : s = "ETHERIP"; break;
    case IPPROTO_ENCAP           : s = "ENCAP"; break;
    case IPPROTO_APES            : s = "APES"; break;
    case IPPROTO_GMTP            : s = "GMTP"; break;
    case IPPROTO_IPCOMP          : s = "IPCOMP"; break;
    case IPPROTO_SCTP            : s = "SCTP"; break;
    case IPPROTO_PIM             : s = "PIM"; break;
    case IPPROTO_CARP            : s = "CARP"; break;
    case IPPROTO_PGM             : s = "PGM"; break;
    case IPPROTO_PFSYNC          : s = "PFSYNC"; break;
    default                      :
      return utf8::int2Str(proto);
  }
  return s;
}
//------------------------------------------------------------------------------
PCAP::~PCAP()
{
}
//------------------------------------------------------------------------------
PCAP::PCAP() : handle_(NULL), groupingPeriod_(pgpNone),
  packetsAutoDrop_(packetsList_), groupTreeAutoDrop_(groupTree_),
  swapThreshold_(16u * 1024u * 1024u),
  swapLowWatermark_(50),
  swapHighWatermark_(90),
  swapWatchTime_(5000000),
  promisc_(false),
  ports_(true),
  protocols_(true)
{
//  tempFile_ = getTempPath() + createGUIDAsBase32String() + ".tmp";
}
//------------------------------------------------------------------------------
/* From tcptraceroute, convert a numeric IP address to a string */
static inline utf8::String iptos(u_long in)
{
  char output[3 * 4 + 3 + 1];
  u_char * p;
  
  p = (u_char *)&in;
  sprintf(output,"%d.%d.%d.%d",p[0],p[1],p[2],p[3]);
  return output;
}
//------------------------------------------------------------------------------
/*#ifndef __MINGW32__ // Cygnus doesn't have IPv6
static inline char * ip6tos(struct sockaddr * sockaddr,char * address,int addrlen)
{
	socklen_t sockaddrlen;
#if defined(__WIN32__) || defined(__WIN64__)
	sockaddrlen = sizeof(struct sockaddr_in6);
#else
	sockaddrlen = sizeof(struct sockaddr_storage);
#endif
	if( getnameinfo(sockaddr, 
		    sockaddrlen, 
		    address, 
		    addrlen, 
		    NULL, 
		    0, 
		    NI_NUMERICHOST) != 0 ) address = NULL;
	return address;
}
#endif // __MINGW32__*/
//------------------------------------------------------------------------------
void PCAP::printAllDevices()
{
#if HAVE_PCAP_H
	pcap_if_t * alldevs;
	pcap_if_t * d;
	char errbuf[PCAP_ERRBUF_SIZE + 1];
	
	/* Retrieve the device list */
  api.open();
  if( api.pcap_findalldevs(&alldevs,errbuf) != 0 ){
    int32_t err = oserror() + errorOffset;
    api.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + errbuf)->throwSP();
  }
	/* Scan the list printing every entry */
  for( d = alldevs; d != NULL; d = d->next ){
    pcap_addr_t * a;
    //char ip6str[128];
    /* Name */
    fprintf(stdout,"%s\n",d->name);
    /* Description */
    if( d->description )
      fprintf(stdout,"\tDescription: %s\n",d->description);
    /* Loopback Address*/
    fprintf(stdout,"\tLoopback: %s\n",(d->flags & PCAP_IF_LOOPBACK)?"yes":"no");
    /* IP addresses */
    for( a = d->addresses; a != NULL; a = a->next ){
      fprintf(stdout,"\tAddress Family: #%d\n",a->addr->sa_family);
      switch( a->addr->sa_family ){
        case AF_INET:
          fprintf(stdout,"\tAddress Family Name: AF_INET\n");
          if( a->addr )
            fprintf(stdout,"\tAddress: %s\n",(const char *) iptos(((struct sockaddr_in *)a->addr)->sin_addr.s_addr).getOEMString());
          if( a->netmask )
            fprintf(stdout,"\tNetmask: %s\n",(const char *) iptos(((struct sockaddr_in *)a->netmask)->sin_addr.s_addr).getOEMString());
          if( a->broadaddr )
            fprintf(stdout,"\tBroadcast Address: %s\n",(const char *) iptos(((struct sockaddr_in *)a->broadaddr)->sin_addr.s_addr).getOEMString());
          if( a->dstaddr )
            fprintf(stdout,"\tDestination Address: %s\n",(const char *) iptos(((struct sockaddr_in *)a->dstaddr)->sin_addr.s_addr).getOEMString());
          break;
  	    /*case AF_INET6:
          fprintf(stdout,"\tAddress Family Name: AF_INET6\n");
  #ifndef __MINGW32__ // Cygnus doesn't have IPv6
          if( a->addr )
            fprintf(stdout,"\tAddress: %s\n", ip6tos(a->addr, ip6str, sizeof(ip6str)));
  #endif
		      break;*/
        default:
          fprintf(stdout,"\tAddress Family Name: Unknown\n");
          break;
      }
    }
    fprintf(stdout,"\n");
  }
/* Free the device list */
  api.pcap_freealldevs(alldevs);
  api.close();
#else
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
#endif
}
//------------------------------------------------------------------------------
void PCAP::threadBeforeWait()
{
#if HAVE_PCAP_H
  if( handle_ != NULL ){
    terminate();
    api.pcap_breakloop((pcap_t *) handle_);
  }
#endif
}
//------------------------------------------------------------------------------
void PCAP::shutdown(void * fp)
{
  if( grouper_ != NULL ) grouper_->wait();
  if( databaseInserter_ != NULL ) databaseInserter_->wait();
  if( lazyWriter_ != NULL ) lazyWriter_->wait();
  grouper_ = NULL;
  databaseInserter_ = NULL;
  lazyWriter_ = NULL;
  if( handle_ != NULL ){
    if( fp != NULL ) api.pcap_freecode((struct bpf_program *) fp);
    api.pcap_close((pcap_t *) handle_);
    handle_ = NULL;
  }
}
//------------------------------------------------------------------------------
void PCAP::threadExecute()
{
//  for( uintptr_t len = 65536; len <= swapThreshold_; len += 65536 )
//    fprintf(stderr,"%s\n",formatByteLength(len,swapThreshold_).c_str());
#if HAVE_PCAP_H
  char errbuf[PCAP_ERRBUF_SIZE + 1]; // Error string
  struct bpf_program fp;         // The compiled filter expression
  bpf_u_int32 mask;              // The netmask of our sniffing device
  bpf_u_int32 net;               // The IP of our sniffing device
  bool freeCode = false;
//  stdErr.bufferDataTTA(0);
  try {
    api.open();
    if( api.pcap_lookupnet(iface_.getANSIString(),&net,&mask,errbuf) != 0 ) goto errexit;
    handle_ = api.pcap_open_live(iface_.getANSIString(),0,promisc_,0,errbuf);
    if( handle_ == NULL ) goto errexit;
    if( !filter_.isNull() ){
      if( api.pcap_compile((pcap_t *) handle_,&fp,filter_.getANSIString(),0,net) != 0 ) goto errexit;
      freeCode = true;
      if( api.pcap_setfilter((pcap_t *) handle_,&fp) != 0 ) goto errexit;
    }
    //if( api.pcap_setmode((pcap_t *) handle_,MODE_MON) != 0 ) goto errexit;
    grouper_ = newObjectV1<Grouper>(this);
    databaseInserter_ = newObjectV1<DatabaseInserter>(this);
    lazyWriter_ = newObjectV1<LazyWriter>(this);
    memoryUsage_ = 0;
    grouper_->resume();
    databaseInserter_->resume();
    lazyWriter_->resume();
    api.pcap_loop((pcap_t *) handle_,-1,(pcap_handler) pcapCallback,(u_char *) this);
    if( packets_ != NULL ){
      AutoLock<InterlockedMutex> lock(packetsListMutex_);
      packetsList_.insToTail(*packets_.ptr(NULL));
      grouperSem_.post();
    }
    oserror(0);
  }
  catch( ExceptionSP & ){
    shutdown(freeCode ? &fp : NULL);
    api.close();
    throw;
  }
errexit:
  int32_t err = oserror();
  shutdown(freeCode ? &fp : NULL);
  api.close();
  if( err != 0 )
    newObjectV1C2<Exception>(err + errorOffset,__PRETTY_FUNCTION__ + utf8::String(" ") + errbuf + ", device: " + iface_)->throwSP();
#else
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
#endif
}
//------------------------------------------------------------------------------
void PCAP::pcapCallback(void * args,const void * header,const void * packet)
{
#if HAVE_PCAP_H
  reinterpret_cast<PCAP *>(args)->capture(
    timeval2Time(reinterpret_cast<const struct pcap_pkthdr *>(header)->ts),
    reinterpret_cast<const struct pcap_pkthdr *>(header)->caplen,
    reinterpret_cast<const struct pcap_pkthdr *>(header)->len,
    (const uint8_t *) packet
  );
#endif
}
//------------------------------------------------------------------------------
void PCAP::setBounds(uint64_t timestamp,uint64_t & bt,uint64_t & et) const
{
  struct tm t = time2tm(timestamp);
  switch( groupingPeriod_ ){
    case pgpNone :
      bt = et = timestamp;
      break;
    case pgpSec  :
      bt = tm2Time(t);
      et = bt + 999999;
      break;
    case pgpMin  :
      t.tm_sec = 0;
      bt = tm2Time(t);
      t.tm_sec = 59;
      et = tm2Time(t) + 999999;
      break;
    case pgpHour :
      t.tm_sec = 0;
      t.tm_min = 0;
      bt = tm2Time(t);
      t.tm_sec = 59;
      t.tm_min = 59;
      et = tm2Time(t) + 999999;
      break;
    case pgpDay  :
      t.tm_sec = 0;
      t.tm_min = 0;
      t.tm_hour = 0;
      bt = tm2Time(t);
      t.tm_sec = 59;
      t.tm_min = 59;
      t.tm_hour = 23;
      et = tm2Time(t) + 999999;
      break;
    case pgpMon  :
      t.tm_sec = 0;
      t.tm_min = 0;
      t.tm_hour = 0;
      t.tm_mday = 1;
      bt = tm2Time(t);
      t.tm_sec = 59;
      t.tm_min = 59;
      t.tm_hour = 23;
      t.tm_mday = (int) monthDays(t.tm_year + 1900,t.tm_mon);
      et = tm2Time(t) + 999999;
      break;
    case pgpYear :
      t.tm_sec = 0;
      t.tm_min = 0;
      t.tm_hour = 0;
      t.tm_mday = 1;
      t.tm_mon = 0;
      bt = tm2Time(t);
      t.tm_sec = 59;
      t.tm_min = 59;
      t.tm_hour = 23;
      t.tm_mday = 31;
      t.tm_mon = 11;
      et = tm2Time(t) + 999999;
      break;
  }
}
//------------------------------------------------------------------------------
void PCAP::capture(uint64_t timestamp,uintptr_t capLen,uintptr_t len,const uint8_t * packet)
{
#if HAVE_PCAP_H
#define SIZE_ETHERNET 14
  const EthernetPacketHeader * ethernet = (const EthernetPacketHeader *)(packet);
  const IPPacketHeader * ip = (const IPPacketHeader *)(packet + SIZE_ETHERNET);
  uintptr_t sizeIp = ip->hl() * 4;
  if( sizeIp < 20 ){
    if( stdErr.debugLevel(80) )
      stdErr.debug(80,utf8::String::Stream() <<
        "Device: " << iface_ << ", invalid IP header length: " << sizeIp << " bytes, from MAC: " <<
        utf8::int2HexStr(ethernet->srcAddr_[0],2) << ":" <<
        utf8::int2HexStr(ethernet->srcAddr_[1],2) << ":" <<
        utf8::int2HexStr(ethernet->srcAddr_[2],2) << ":" <<
        utf8::int2HexStr(ethernet->srcAddr_[3],2) << ":" <<
        utf8::int2HexStr(ethernet->srcAddr_[4],2) << ":" <<
        utf8::int2HexStr(ethernet->srcAddr_[5],2) << " to MAC: " <<
        utf8::int2HexStr(ethernet->dstAddr_[0],2) << ":" <<
        utf8::int2HexStr(ethernet->dstAddr_[1],2) << ":" <<
        utf8::int2HexStr(ethernet->dstAddr_[2],2) << ":" <<
        utf8::int2HexStr(ethernet->dstAddr_[3],2) << ":" <<
        utf8::int2HexStr(ethernet->dstAddr_[4],2) << ":" <<
        utf8::int2HexStr(ethernet->dstAddr_[5],2) << ", " <<
        __PRETTY_FUNCTION__ << "\n"
      );
    return;
  }
  const TCPPacketHeader * tcp = (const TCPPacketHeader *)(packet + SIZE_ETHERNET + sizeIp);
  uintptr_t sizeTcp = 0;
  if( ip->proto_ == IPPROTO_TCP ){
    sizeTcp = tcp->off() * 4;
    if( sizeTcp < 20 ){
      if( stdErr.debugLevel(80) ){
        ksock::SockAddr src, dst;
        src.addr4_.sin_addr = ip->src_;
        src.addr4_.sin_port = tcp->srcPort_;
        dst.addr4_.sin_addr = ip->dst_;
        dst.addr4_.sin_port = tcp->dstPort_;
        stdErr.debug(80,utf8::String::Stream() <<
          "Device: " << iface_ << ", invalid TCP header length: " << sizeTcp << " bytes, from: " <<
          src.resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV) << " to: " <<
          dst.resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV) <<
        __PRETTY_FUNCTION__ << "\n"
        );
      }
      return;
    }
  }
  const uint8_t * payload = packet + SIZE_ETHERNET + sizeIp + sizeTcp;
  if( packets_ == NULL || packets_->packets_ == packets_->count() ){
    if( packets_ != NULL ){
      AutoLock<InterlockedMutex> lock(packetsListMutex_);
      packetsList_.insToTail(*packets_.ptr(NULL));
      grouperSem_.post();
    }
    packets_ = newObjectV1<Packets>(
#ifndef NDEBUG
      getpagesize() * 1u / sizeof(Packet)
#else
      getpagesize() * 16 / sizeof(Packet)
#endif
    );
    interlockedIncrement(
      memoryUsage_,
      uilock_t(packets_->count() * sizeof(Packet) + sizeof(Packets))
    );
    lazyWriterSem_.post();
  }
  uint64_t bt, et;
  setBounds(timestamp,bt,et);
  Packet & pkt = (*packets_.ptr())[packets_->packets_];
  pkt.timestamp_ = bt;
  pkt.pktSize_ = len;
  pkt.dataSize_ = len - (payload - packet);
  memcpy(&pkt.srcAddr_,&ip->src_,sizeof(pkt.srcAddr_));
  memcpy(&pkt.dstAddr_,&ip->dst_,sizeof(pkt.dstAddr_));
  pkt.srcPort_ = ip->proto_ == IPPROTO_TCP && ports_ ? tcp->srcPort_ : 0;
  pkt.dstPort_ = ip->proto_ == IPPROTO_TCP && ports_ ? tcp->dstPort_ : 0;
  pkt.proto_ = protocols_ ? ip->proto_ : -1;
  for( intptr_t i = packets_->packets_ - 1, j = i - 5; i >= 0 && i >= j; i-- )
    if( pkt == (*packets_.ptr())[i] ){
      Packet & pkt2 = (*packets_.ptr())[i];
      pkt2.pktSize_ += pkt.pktSize_;
      pkt2.dataSize_ += pkt.dataSize_;
      return;
    }
  packets_->packets_++;
#endif
}
//------------------------------------------------------------------------------
bool PCAP::insertPacketsInDatabase(uint64_t,uint64_t,const HashedPacket *,uintptr_t) throw()
{
  return true;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void PCAP::Grouper::threadBeforeWait()
{
  if( pcap_ != NULL ){
    terminate();
    pcap_->grouperSem_.post().post();
  }
}
//------------------------------------------------------------------------------
void PCAP::Grouper::threadExecute()
{
  priority(THREAD_PRIORITY_ABOVE_NORMAL);
  uintptr_t count = 0;
  for(;;){
    bool term;
    AutoPtr<Packets> packets;
    {
      AutoLock<InterlockedMutex> lock(pcap_->packetsListMutex_);
      if( pcap_->packetsList_.count() > 0 )
        packets.ptr(&pcap_->packetsList_.remove(*pcap_->packetsList_.first()));
      term = terminated_;
    }
    if( packets == NULL || packets->packets_ == 0 ){
      if( count > 0 ){
        pcap_->databaseInserterSem_.post();
        pcap_->lazyWriterSem_.post();
        count = 0;
      }
      if( term ) break;
      pcap_->grouperSem_.wait();
      continue;
    }
    PacketGroup * pGroup;
    PacketGroup::SwapFileHeader header;
    AutoPtr<PacketGroup> group;
    try {
      for( intptr_t i = packets->packets_ - 1; i >= 0; i-- ){
        const Packet & pkt = (*packets.ptr())[i];
        if( group == NULL || pkt.timestamp_ < header.bt_ || pkt.timestamp_ > header.et_ ){
          if( group != NULL ){
            AutoLock<InterlockedMutex> lock(pcap_->groupTreeMutex_);
            pcap_->groupTree_.insert(group,false,false,&pGroup);
          }
          group.ptr(newObject<PacketGroup>());
          pcap_->setBounds(pkt.timestamp_,group->header_.bt_,group->header_.et_);
          {
            AutoLock<InterlockedMutex> lock(pcap_->groupTreeMutex_);
            pcap_->groupTree_.insert(group,false,false,&pGroup);
            pcap_->groupTree_.remove(*pGroup);
            if( pGroup != group ){
              group = pGroup;
            }
            else {
              interlockedIncrement(pcap_->memoryUsage_,sizeof(PacketGroup));
            }
            header = pGroup->header_;
          }
        }
        if( pGroup->header_.count_ == pGroup->maxCount_ ){
          uintptr_t count = (pGroup->maxCount_ << 1) + ((pGroup->maxCount_ == 0) << 4);
          pGroup->packets_.realloc(count * sizeof(HashedPacket));
          interlockedIncrement(
            pcap_->memoryUsage_,
            uilock_t((count - pGroup->maxCount_) * sizeof(HashedPacket))
          );
          pGroup->maxCount_ = count;
        }
        HashedPacket & hpkt = pGroup->packets_[pGroup->header_.count_], * p;
        hpkt.pktSize_ = pkt.pktSize_;
        hpkt.dataSize_ = pkt.dataSize_;
        memcpy(&hpkt.srcAddr_,&pkt.srcAddr_,sizeof(pkt.srcAddr_));
        memcpy(&hpkt.dstAddr_,&pkt.dstAddr_,sizeof(pkt.dstAddr_));
        hpkt.srcPort_ = pkt.srcPort_;
        hpkt.dstPort_ = pkt.dstPort_;
        hpkt.proto_ = pkt.proto_;
        pGroup->packetsHash_.insert(hpkt,false,false,&p);
        if( p != &hpkt ){
          p->pktSize_ += pkt.pktSize_;
          p->dataSize_ += pkt.dataSize_;
        }
        else {
          pGroup->header_.count_++;
        }
      }
      AutoLock<InterlockedMutex> lock(pcap_->groupTreeMutex_);
      pcap_->groupTree_.insert(group,false,false,&pGroup);
      if( group.ptr() != pGroup ){
        pGroup->joinGroup(group,pcap_->memoryUsage_);
        interlockedIncrement(
          pcap_->memoryUsage_,
          -ilock_t(group->maxCount_ * sizeof(HashedPacket) + sizeof(PacketGroup))
        );
      }
      else {
        group.ptr(NULL);
      }
      count++;
    }
    catch( ExceptionSP & ){
      AutoLock<InterlockedMutex> lock(pcap_->groupTreeMutex_);
      pcap_->groupTree_.insert(group,false,false,&pGroup);
      if( group.ptr() != pGroup ){
        pGroup->joinGroup(group,pcap_->memoryUsage_);
        interlockedIncrement(
          pcap_->memoryUsage_,
          -ilock_t(group->maxCount_ * sizeof(HashedPacket) + sizeof(PacketGroup))
        );
      }
      else {
        group.ptr(NULL);
      }
      throw;
    }
    interlockedIncrement(
      pcap_->memoryUsage_,
      -ilock_t(packets->count() * sizeof(Packet) + sizeof(Packets))
    );
    if( stdErr.debugLevel(4) ){
      int pcapStatSize;
      struct pcap_stat * ps;
      if( (ps = api.pcap_stats_ex((pcap_t *) pcap_->handle_,&pcapStatSize)) != NULL ){
        stdErr.debug(4,utf8::String::Stream() <<
          "Device: " << pcap_->iface_ <<
          ", recv: " << ps->ps_recv <<
          ", drop: " << ps->ps_drop <<
          ", ifdrop: " << ps->ps_ifdrop <<
          "\n"
        );
        if( stdErr.debugLevel(6) )
          stdErr.debug(6,utf8::String::Stream() <<
            "Memory usage: " <<
            formatByteLength(
              interlockedIncrement(pcap_->memoryUsage_,0),
              pcap_->swapThreshold()
            ) << "\n"
          );
      }
    }
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void PCAP::DatabaseInserter::threadBeforeWait()
{
  if( pcap_ != NULL ){
    terminate();
    pcap_->databaseInserterSem_.post().post();
  }
}
//------------------------------------------------------------------------------
void PCAP::DatabaseInserter::threadExecute()
{
  priority(THREAD_PRIORITY_HIGHEST);
  while( !terminated_ ){
    PacketGroup * pGroup;
    AutoPtr<PacketGroup> group;
    {
      AutoLock<InterlockedMutex> lock(pcap_->groupTreeMutex_);
      PacketGroupTree::Walker walker(pcap_->groupTree_);
      uint64_t ct = gettimeofday();
      while( walker.next() ){
        pGroup = &walker.object();
        if( pGroup->header_.et_ < ct ){
          group.ptr(pGroup);
          pcap_->groupTree_.remove(*pGroup);
          break;
        }
      }
    }
    if( group == NULL ){
      pcap_->databaseInserterSem_.wait();
      continue;
    }
    if( stdErr.debugLevel(7) )
      stdErr.debug(7,utf8::String::Stream() <<
        "Device: " << pcap_->iface_ << ", start processing packets group: " <<
        utf8::time2Str(group->header_.bt_) << " - " <<
        utf8::time2Str(group->header_.et_) << ", count: " <<
        group->header_.count_ << "\n"
      );
    if( stdErr.debugLevel(6) )
      stdErr.debug(6,utf8::String::Stream() <<
        "Memory usage: " <<
        formatByteLength(
          interlockedIncrement(pcap_->memoryUsage_,0),
          pcap_->swapThreshold()
        ) << "\n"
      );

    bool success = pcap_->insertPacketsInDatabase(
      group->header_.bt_,
      group->header_.et_,
      group->packets_,
      group->header_.count_
    );
    PacketGroup::SwapFileHeader header = group->header_;
    if( success ){
      interlockedIncrement(
        pcap_->memoryUsage_,
        -ilock_t(group->maxCount_ * sizeof(HashedPacket) + sizeof(PacketGroup))
      );
      pcap_->lazyWriterSem_.post();
    }
    else {
      AutoLock<InterlockedMutex> lock(pcap_->groupTreeMutex_);
      pcap_->groupTree_.insert(group,false,false,&pGroup);
      if( pGroup != group ){
        pGroup->joinGroup(group,pcap_->memoryUsage_);
        interlockedIncrement(
          pcap_->memoryUsage_,
          -ilock_t(group->maxCount_ * sizeof(HashedPacket) + sizeof(PacketGroup))
        );
      }
      else {
        group.ptr(NULL);
      }
    }
    if( stdErr.debugLevel(7) )
      stdErr.debug(7,utf8::String::Stream() <<
        "Device: " << pcap_->iface_ << ", stop processing packets group: " <<
        utf8::time2Str(header.bt_) << " - " <<
        utf8::time2Str(header.et_) << ", processed: " <<
        (success ? header.count_ : 0) << "\n"
      );
    if( stdErr.debugLevel(6) )
      stdErr.debug(6,utf8::String::Stream() <<
        "Memory usage: " <<
        formatByteLength(
          interlockedIncrement(pcap_->memoryUsage_,0),
          pcap_->swapThreshold()
        ) << "\n"
      );
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void PCAP::LazyWriter::threadBeforeWait()
{
  if( pcap_ != NULL ){
    terminate();
    pcap_->lazyWriterSem_.post().post();
  }
}
//------------------------------------------------------------------------------
void PCAP::LazyWriter::swapOut(AsyncFile & tempFile,AutoPtr<PacketGroup> & group)
{
  PacketGroup::SwapFileHeader header;
  bool swapped = false;
  try {
    tempFile.createIfNotExist(true).open();
    uint64_t size = tempFile.size();
    try {
      header = group->header_;
      tempFile.seek(size).
        writeBuffer(group->packets_,header.count_ * sizeof(HashedPacket)).
        writeBuffer(&header,sizeof(header))
      ;
      size += header.count_ * sizeof(HashedPacket) + sizeof(header);
      swapped = true;
      interlockedIncrement(
        pcap_->memoryUsage_,
        -ilock_t(group->maxCount_ * sizeof(HashedPacket) + sizeof(PacketGroup))
      );
      group = NULL;
    }
    catch( ExceptionSP & e ){
      e->writeStdError();
    }
    tempFile.resize(size);
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
  }
  if( swapped && stdErr.debugLevel(5) )
    stdErr.debug(5,utf8::String::Stream() <<
      "Device: " << pcap_->iface_ << ", swapout packets group: " <<
      utf8::time2Str(header.bt_) << " - " <<
      utf8::time2Str(header.et_) << ", count: " <<
      header.count_ << ", to: " <<
      tempFile.fileName() << "\n"
    );
  if( swapped && stdErr.debugLevel(6) )
    stdErr.debug(6,utf8::String::Stream() <<
      "Memory usage: " <<
      formatByteLength(
        interlockedIncrement(pcap_->memoryUsage_,0),
        pcap_->swapThreshold()
      ) << "\n"
    );
}
//------------------------------------------------------------------------------
void PCAP::LazyWriter::swapIn(AsyncFile & tempFile)
{
  if( tempFile.size() > 0 ){
    uint64_t pos = tempFile.size() - sizeof(PacketGroup::SwapFileHeader);
    PacketGroup::SwapFileHeader header;
    tempFile.readBuffer(pos,&header,sizeof(header));
    AutoPtr<PacketGroup> group(newObject<PacketGroup>());
    group->header_ = header;
    group->maxCount_ = header.count_;
    uintptr_t groupSize = header.count_ * sizeof(HashedPacket);
    group->packets_.alloc(groupSize);
    tempFile.readBuffer(pos - groupSize,group->packets_,groupSize);
    tempFile.resize(pos - groupSize);
    PacketGroup * pGroup;
    {
      AutoLock<InterlockedMutex> lock(pcap_->groupTreeMutex_);
      pcap_->groupTree_.insert(group,false,false,&pGroup);
      if( pGroup == group ){
        group.ptr(NULL);
        interlockedIncrement(
          pcap_->memoryUsage_,
          uilock_t(sizeof(PacketGroup) + groupSize)
        );
      }
      else {
        pGroup->joinGroup(group,pcap_->memoryUsage_);
      }
    }
    if( stdErr.debugLevel(5) )
      stdErr.debug(5,utf8::String::Stream() <<
        "Device: " << pcap_->iface_ << ", swapin packets group: " <<
        utf8::time2Str(header.bt_) << " - " <<
        utf8::time2Str(header.et_) << ", count: " <<
        header.count_ << ", from: " <<
        tempFile.fileName() << "\n"
      );
    if( stdErr.debugLevel(6) )
      stdErr.debug(6,utf8::String::Stream() <<
        "Memory usage: " <<
        formatByteLength(
          interlockedIncrement(pcap_->memoryUsage_,0),
          pcap_->swapThreshold()
        ) << "\n"
      );
    pcap_->databaseInserterSem_.post();
  }
  if( tempFile.size() == 0 ){
    tempFile.close();
    remove(tempFile.fileName(),true);
  }
}
//------------------------------------------------------------------------------
void PCAP::LazyWriter::threadExecute()
{
  priority(THREAD_PRIORITY_HIGHEST);
  AsyncFile tempFile(pcap_->tempFile_);
  tempFile.createIfNotExist(false).tryOpen();
  for(;;){
    uilock_t memoryUsage = interlockedIncrement(pcap_->memoryUsage_,0);
    bool swapin = memoryUsage < pcap_->swapThreshold() * pcap_->swapLowWatermark_ / 100;
    bool swapout = memoryUsage >= pcap_->swapThreshold() * pcap_->swapHighWatermark_ / 100;
    while( swapout || terminated_ ){
      AutoPtr<PacketGroup> group;
      bool term;
      {
        AutoLock<InterlockedMutex> lock(pcap_->groupTreeMutex_);
        PacketGroupTree::Walker walker(pcap_->groupTree_);
        if( walker.next() ){
          group.ptr(&walker.object());
          pcap_->groupTree_.remove(walker.object());
        }
        term = terminated_;
      }
      if( group == NULL ) if( term ) return; else break;
      swapOut(tempFile,group);
      if( group != NULL ){
        AutoLock<InterlockedMutex> lock(pcap_->groupTreeMutex_);
        PacketGroup * pGroup;
        pcap_->groupTree_.insert(group,false,false,&pGroup);
        if( pGroup == group ){
          group.ptr(NULL);
        }
        else {
          pGroup->joinGroup(group,pcap_->memoryUsage_);
          interlockedIncrement(
            pcap_->memoryUsage_,
            -ilock_t(group->maxCount_ * sizeof(HashedPacket) + sizeof(PacketGroup))
          );
        }
      }
      else {
        lastSwapOut_ = gettimeofday();
      }
      memoryUsage = interlockedIncrement(pcap_->memoryUsage_,0);
      swapout = memoryUsage > pcap_->swapThreshold() * pcap_->swapLowWatermark_ / 100;
    }
    while( swapin && gettimeofday() - lastSwapOut_ >= pcap_->swapWatchTime_ && tempFile.isOpen() ){
      swapIn(tempFile);
      lastSwapIn_ = gettimeofday();
      memoryUsage = interlockedIncrement(pcap_->memoryUsage_,0);
      swapin = memoryUsage < pcap_->swapThreshold() * pcap_->swapLowWatermark_ / 100;
    }
    pcap_->lazyWriterSem_.wait();
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
PCAP::PacketGroup & PCAP::PacketGroup::joinGroup(const PacketGroup & group,uilock_t & memoryUsage)
{
  for( intptr_t i = group.header_.count_ - 1; i >= 0; i-- ){
    if( header_.count_ == maxCount_ ){
      uintptr_t count = (maxCount_ << 1) + ((maxCount_ == 0) << 4);
      packets_.realloc(count * sizeof(HashedPacket));
      interlockedIncrement(
        memoryUsage,
        uilock_t((count - maxCount_) * sizeof(HashedPacket))
      );
      maxCount_ = count;
    }
    HashedPacket & pkt = packets_[header_.count_], * p;
    pkt = group.packets_[i];
    packetsHash_.insert(pkt,false,false,&p);
    if( p != &pkt ){
      p->pktSize_ += pkt.pktSize_;
      p->dataSize_ += pkt.dataSize_;
    }
    else {
      header_.count_++;
    }
  }
  return *this;
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
