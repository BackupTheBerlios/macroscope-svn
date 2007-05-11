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
#define PROTOF(x) (* x)
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
    int PROTOF(pcap_stats)(pcap_t *,struct pcap_stat *);
    struct pcap_stat * PROTOF(pcap_stats_ex)(pcap_t *,int *);
    char * PROTOF(pcap_geterr)(pcap_t *);
    int PROTOF(pcap_setmode)(pcap_t *,int mode);

    PCAP_API & open();
    PCAP_API & close();
  protected:
#if defined(__WIN32__) || defined(__WIN64__)
    HINSTANCE handle_;
#else
    void * handle_;
#endif
    intptr_t count_;
    uint8_t mutex_[sizeof(InterlockedMutex)];
    InterlockedMutex & mutex(){ return *reinterpret_cast<InterlockedMutex *>(mutex_); }
  private:
    PCAP_API(const PCAP_API &);
    void operator = (const PCAP_API &);
};
//------------------------------------------------------------------------------
static PCAP_API api;
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
    "pcap_stats",
    "pcap_stats_ex",
    "pcap_geterr",
    "pcap_setmode"
  };
  AutoLock<InterlockedMutex> lock(api.mutex());
#if defined(__WIN32__) || defined(__WIN64__)
#define LIB_NAME "wpcap.dll"
//  if( isWin9x() ){
    api.handle_ = LoadLibraryExA(LIB_NAME,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
//  }
//  else{
//    api.handle_ = LoadLibraryExW(L"wpcap.dll",NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
//  }
#elif HAVE_DLFCN_H
#define LIB_NAME "libpcap.so"
    api.handle_ = dlopen(LIB_NAME,
#ifdef __linux__
      RTLD_GLOBAL | RTLD_NOW
#else
      0
#endif
    );
#endif
  if( handle_ == NULL ){
    int32_t err = oserror() + errorOffset;
    stdErr.debug(
      9,
      utf8::String::Stream() << "Load " LIB_NAME " failed\n"
    );
    newObjectV1C2<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
#undef LIB_NAME
  for( uintptr_t i = 0; i < sizeof(symbols) / sizeof(symbols[0]); i++ ){
#if defined(__WIN32__) || defined(__WIN64__)
#define FUNC_NAME "GetProcAddress"
    ((void **) &api.pcap_breakloop)[i] = GetProcAddress(handle_,symbols[i]);
#elif HAVE_DLFCN_H
#define FUNC_NAME "dlsym"
    ((void **) &api.pcap_breakloop)[i] = dlsym(handle_,symbols[i]);
#endif
    if( ((void **) &api.pcap_breakloop)[i] == NULL &&
        (void **) &api.pcap_breakloop + i != (void **) &api.pcap_stats_ex &&
	(void **) &api.pcap_breakloop + i != (void **) &api.pcap_setmode ){
      int32_t err = oserror() + errorOffset;
#if defined(__WIN32__) || defined(__WIN64__)
      FreeLibrary(handle_);
#elif HAVE_DLFCN_H
      dlclose(handle_);
#endif
      handle_ = NULL;
      stdErr.debug(
        9,
        utf8::String::Stream() << FUNC_NAME "(\"" << symbols[i] << "\") falied\n"
      );
      newObjectV1C2<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
    }
#undef FUNC_NAME
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
#if defined(__WIN32__) || defined(__WIN64__)
    FreeLibrary(handle_);
#elif HAVE_DLFCN_H
    dlclose(handle_);
#endif
    handle_ = NULL;
  }
  count_--;
  return *this;
}
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
PCAP::~PCAP()
{
}
//------------------------------------------------------------------------------
PCAP::PCAP() : handle_(NULL), fp_(NULL), groupingPeriod_(pgpNone),
  packetsAutoDrop_(packetsList_), groupTreeAutoDrop_(groupTree_),
  swapThreshold_(16u * 1024u * 1024u),
  pregroupingBufferSize_(
#ifndef NDEBUG
    getpagesize() * 1u / sizeof(Packet)
#else
    getpagesize() * 16u / sizeof(Packet)
#endif
  ),
  pregroupingWindowSize_(5),
  swapLowWatermark_(50),
  swapHighWatermark_(90),
  swapWatchTime_(5000000),
  promisc_(false),
  ports_(true),
  protocols_(true),
  fc_(false)
{
//  tempFile_ = getTempPath() + createGUIDAsBase32String() + ".tmp";
}
//------------------------------------------------------------------------------
void PCAP::printAllDevices()
{
#if HAVE_PCAP_H
  pcap_if_t * alldevs;
  pcap_if_t * d;
  char errbuf[PCAP_ERRBUF_SIZE + 1];

/* Retrieve the device list */
  api.open();
  memset(errbuf,0,sizeof(errbuf));
  oserror(0);
  if( api.pcap_findalldevs(&alldevs,errbuf) != 0 || errbuf[0] != '\0' ){
    int32_t err = oserror() + errorOffset;
    api.close();
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + errbuf)->throwSP();
  }
/* Scan the list printing every entry */
  for( d = alldevs; d != NULL; d = d->next ){
    pcap_addr_t * a;
    /* Name */
    fprintf(stdout,"%s\n",d->name);
    /* Description */
    if( d->description != NULL )
      fprintf(stdout,"\tDescription: %s\n",d->description);
    /* Loopback Address*/
    fprintf(stdout,"\tLoopback: %s\n",(d->flags & PCAP_IF_LOOPBACK) ? "yes" : "no");
    /* IP addresses */
    for( a = d->addresses; a != NULL; a = a->next ){
//      fprintf(stdout,"\tAddress Family: #%d\n",a->addr->sa_family);
      fprintf(stdout,"\t%s",(const char *) ksock::SockAddr::addressFamilyAsString(a->addr->sa_family).lower().getOEMString());
      ksock::SockAddr * addr;
      switch( a->addr->sa_family ){
        /*case AF_LINK  :
          fprintf(stdout,"\tAddress: %s\n",(const char *) (
            utf8::int2HexStr(((uint8_t *) a->addr)[0],2) + ":" +
            utf8::int2HexStr(((uint8_t *) a->addr)[1],2) + ":" +
            utf8::int2HexStr(((uint8_t *) a->addr)[2],2) + ":" +
            utf8::int2HexStr(((uint8_t *) a->addr)[3],2) + ":" +
            utf8::int2HexStr(((uint8_t *) a->addr)[4],2) + ":").getOEMString()
	  );
	  break;*/
        case AF_INET  :
#if SIZEOF_SOCKADDR_IN6
        case AF_INET6 :
#endif
#if SIZEOF_SOCKADDR_DL
        case AF_LINK :
#endif
	  addr = (ksock::SockAddr *) a->addr;
          fprintf(stdout," %s",(const char *) addr->resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV).getOEMString());
          if( a->netmask != NULL && a->addr->sa_family != AF_INET6 ){
	    addr = (ksock::SockAddr *) a->netmask;
            fprintf(stdout," netmask %s",(const char *) addr->resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV).getOEMString());
	  }
          if( a->broadaddr != NULL ){
	    addr = (ksock::SockAddr *) a->broadaddr;
            fprintf(stdout," broadcast %s",(const char *) addr->resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV).getOEMString());
	  }
          if( a->dstaddr != NULL ){
	    addr = (ksock::SockAddr *) a->dstaddr;
            fprintf(stdout," destination address %s",(const char *) addr->resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV).getOEMString());
	  }
          break;
      }
      fprintf(stdout,"\n");
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
#if !defined(__WIN32__) && !defined(__WIN64__) && HAVE_PTHREAD_H
    if( (errno = pthread_cancel(Thread::handle_)) != 0 ){
      perror(NULL);
      abort();
    }
#endif
  }
#endif
}
//------------------------------------------------------------------------------
void PCAP::threadAfterWait()
{
  shutdown();
}
//------------------------------------------------------------------------------
void PCAP::shutdown()
{
  if( packets_ != NULL ){
    AutoLock<InterlockedMutex> lock(packetsListMutex_);
    packetsList_.insToTail(*packets_.ptr(NULL));
    grouperSem_.post();
  }
  if( databaseInserter_ != NULL ) databaseInserter_->wait();
  if( grouper_ != NULL ) grouper_->wait();
  if( lazyWriter_ != NULL ) lazyWriter_->wait();
  grouper_ = NULL;
  databaseInserter_ = NULL;
  lazyWriter_ = NULL;
  if( handle_ != NULL ){
    struct bpf_program * fp = (struct bpf_program *) fp_;
    if( fp != NULL ){
      if( fc_ ){
        api.pcap_freecode(fp);
	fc_ = false;
      }
      kfree(fp);
      fp_ = NULL;
    }
    api.pcap_close((pcap_t *) handle_);
    handle_ = NULL;
  }
  stdErr.debug(1,utf8::String::Stream() << "Device: " << iface_ << ", capture stopped.\n");
}
//------------------------------------------------------------------------------
void PCAP::threadExecute()
{
//  for( uintptr_t len = 65536; len <= swapThreshold_; len += 65536 )
//    fprintf(stderr,"%s\n",formatByteLength(len,swapThreshold_).c_str());
#if HAVE_PCAP_H
  char errbuf[PCAP_ERRBUF_SIZE + 1]; // Error string
  struct bpf_program * fp;         // The compiled filter expression
  bpf_u_int32 mask;              // The netmask of our sniffing device
  bpf_u_int32 net;               // The IP of our sniffing device
//  stdErr.bufferDataTTA(0);
  utf8::String::Stream stream;
  api.open();
  try {
    fp_ = kmalloc(sizeof(struct bpf_program));
    fp = (struct bpf_program *) fp_;
    if( api.pcap_lookupnet(iface_.getANSIString(),&net,&mask,errbuf) != 0 ) goto errexit;
    handle_ = api.pcap_open_live(iface_.getANSIString(),INT_MAX,promisc_,0,errbuf);
    if( handle_ == NULL ) goto errexit;
    if( !filter_.isNull() ){
      if( api.pcap_compile((pcap_t *) handle_,fp,filter_.getANSIString(),0,net) != 0 ) goto errexit;
      fc_ = true;
      if( api.pcap_setfilter((pcap_t *) handle_,fp) != 0 ) goto errexit;
    }
//    fprintf(stderr,"%s %d\n",__FILE__,__LINE__); fflush(stderr);
    //if( api.pcap_setmode((pcap_t *) handle_,MODE_MON) != 0 ) goto errexit;
    grouper_ = newObjectV1<Grouper>(this);
    databaseInserter_ = newObjectV1<DatabaseInserter>(this);
    lazyWriter_ = newObjectV1<LazyWriter>(this);
    memoryUsage_ = 0;
    curPeriod_ = 0;
    grouper_->resume();
    databaseInserter_->resume();
    lazyWriter_->resume();
    stream << "Device: " << iface_ << ", capture started.\n";
    stdErr.debug(1,stream);
    api.pcap_loop((pcap_t *) handle_,-1,(pcap_handler) pcapCallback,(u_char *) this);
    oserror(0);
  }
  catch( ExceptionSP & ){
    shutdown();
    api.close();
    throw;
  }
errexit:
  int32_t err = oserror();
  shutdown();
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
#ifndef ETHERTYPE_IP
#define ETHERTYPE_IP 0x0800
#endif
  if( capLen < SIZE_ETHERNET ){
    /*if( stdErr.debugLevel(80) )
      stdErr.debug(80,utf8::String::Stream() <<
        "Device: " << iface_ << ", captured length less then ethernet frame header.\n"
      );*/
    return;
  }
  const EthernetPacketHeader * ethernet = (const EthernetPacketHeader *)(packet);
  if( be16toh(ethernet->type_) != ETHERTYPE_IP ){
    if( stdErr.debugLevel(80) )
      stdErr.debug(80,utf8::String::Stream() <<
        "Device: " << iface_ <<
        ", unsupported ethernet frame type: 0x" <<
        utf8::int2HexStr(ethernet->type_,4) <<
        ", from MAC: " <<
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
        utf8::int2HexStr(ethernet->dstAddr_[5],2) << "\n"
      );
    return;
  }
  if( capLen < SIZE_ETHERNET + 20 ){
    if( stdErr.debugLevel(80) )
      stdErr.debug(80,utf8::String::Stream() <<
        "Device: " << iface_ << ", captured length less then IP packet header, from MAC: " <<
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
        utf8::int2HexStr(ethernet->dstAddr_[5],2) << "\n"
      );
    return;
  }
  const IPPacketHeader * ip = (const IPPacketHeader *)(packet + SIZE_ETHERNET);
  uintptr_t sizeIp = 0;
  sizeIp = ip->hl() * 4;
  if( sizeIp < 20 ){
    if( stdErr.debugLevel(80) )
      stdErr.debug(80,utf8::String::Stream() <<
        "Device: " << iface_ <<
        ", invalid IP header length: " <<
	      sizeIp << " bytes, from MAC: " <<
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
        utf8::int2HexStr(ethernet->dstAddr_[5],2) << "\n"
      );
    return;
  }
  const TCPPacketHeader * tcp = (const TCPPacketHeader *)(packet + SIZE_ETHERNET + sizeIp);
  uintptr_t sizeTcp = 0;
  const UDPPacketHeader * udp = (const UDPPacketHeader *)(packet + SIZE_ETHERNET + sizeIp);
  uintptr_t sizeUdp = 0;
  if( sizeIp >= 20 ){
    ksock::SockAddr src, dst;
    src.addr4_.sin_family = AF_INET;
    src.addr4_.sin_addr = ip->src_;
    src.addr4_.sin_port = 0;
    dst.addr4_.sin_family = AF_INET;
    dst.addr4_.sin_addr = ip->dst_;
    dst.addr4_.sin_port = 0;
    uintptr_t size = 0;
    if( ip->proto_ == IPPROTO_TCP ) size = 20;
    else if( ip->proto_ == IPPROTO_UDP ) size = sizeof(UDPPacketHeader);
    if( capLen < SIZE_ETHERNET + sizeIp + size ){
      if( stdErr.debugLevel(80) )
        stdErr.debug(80,utf8::String::Stream() <<
          "Device: " << iface_ << ", captured length less then " <<
	  ksock::SockAddr::protoAsString(ip->proto_) <<
	  " packet header, from: " <<
          src.resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV) << " to: " <<
          dst.resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV) << "\n"
        );
      return;
    }
    if( ip->proto_ == IPPROTO_TCP && (sizeTcp = tcp->off() * 4) < 20 ){
      if( stdErr.debugLevel(80) ){
        src.addr4_.sin_port = tcp->srcPort_;
        dst.addr4_.sin_port = tcp->dstPort_;
        stdErr.debug(80,utf8::String::Stream() <<
          "Device: " << iface_ << ", invalid TCP header length: " << sizeTcp << " bytes, from: " <<
          src.resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV) << " to: " <<
          dst.resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV) << "\n"
        );
      }
      return;
    }
    else if( ip->proto_ == IPPROTO_UDP ){
      sizeUdp = sizeof(UDPPacketHeader);
    }
  }
  uint64_t bt, et;
  setBounds(timestamp,bt,et);
  if( packets_ == NULL ||
      (packets_->packets_ == packets_->count() && packets_->packets_ > 0) ||
      (groupingPeriod_ > pgpNone && bt != curPeriod_) ){
    if( packets_ != NULL ){
      AutoLock<InterlockedMutex> lock(packetsListMutex_);
      packetsList_.insToTail(*packets_.ptr(NULL));
      grouperSem_.post();
    }
    curPeriod_ = bt;
    packets_.ptr(newObjectV1<Packets>(pregroupingBufferSize_ / sizeof(Packet)));
    interlockedIncrement(
      memoryUsage_,
      uilock_t(packets_->count() * sizeof(Packet) + sizeof(Packets))
    );
    lazyWriterSem_.post();
  }
  const uint8_t * payload = packet + SIZE_ETHERNET + sizeIp + sizeTcp + sizeUdp;
  Packet & pkt = (*packets_.ptr())[packets_->packets_];
  pkt.timestamp_ = bt;
  pkt.pktSize_ = len;
  pkt.dataSize_ = len - (payload - packet);
#ifndef NDEBUG
//  static const uint8_t bc[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
//  if( memcmp(ethernet->dstAddr_,bc,6) == 0 || memcmp(ethernet->srcAddr_,bc,6) == 0 ){
//    len = len;
//  }
//  if( pkt.pktSize_ < pkt.dataSize_ ){
//    len = len;
//  }
#endif
  memcpy(&pkt.srcAddr_,&ip->src_,sizeof(pkt.srcAddr_));
  memcpy(&pkt.dstAddr_,&ip->dst_,sizeof(pkt.dstAddr_));
  pkt.srcPort_ = 0;
  pkt.dstPort_ = 0;
  if( ports_ ){
    if( ip->proto_ == IPPROTO_TCP ){
      pkt.srcPort_ = tcp->srcPort_;
      pkt.dstPort_ = tcp->dstPort_;
    }
    else if( ip->proto_ == IPPROTO_UDP ){
      pkt.srcPort_ = udp->srcPort_;
      pkt.dstPort_ = udp->dstPort_;
    }
  }
  pkt.proto_ = protocols_ ? ip->proto_ : -1;
  for( intptr_t i = packets_->packets_ - 1, j = i - pregroupingWindowSize_; i >= 0 && i >= j; i-- )
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
bool PCAP::insertPacketsInDatabase(uint64_t,uint64_t,const HashedPacket *,uintptr_t,Thread *) throw()
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
#ifndef __FreeBSD__
  priority(THREAD_PRIORITY_ABOVE_NORMAL);
#endif
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
      pcap_->databaseInserterSem_.post();
      pcap_->lazyWriterSem_.post();
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
      struct pcap_stat * ps, stat;
      if( api.pcap_stats_ex == NULL ){
        if( api.pcap_stats((pcap_t *) pcap_->handle_,&stat) != 0 ) ps = NULL; else ps = &stat;
      }
      else {
        ps = api.pcap_stats_ex((pcap_t *) pcap_->handle_,&pcapStatSize);
      }
      if( ps != NULL ){
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
#ifndef __FreeBSD__
  priority(THREAD_PRIORITY_HIGHEST);
#endif
  bool success = true;
  while( !terminated_ ){
    PacketGroup * pGroup;
    AutoPtr<PacketGroup> group;
    {
      AutoLock<InterlockedMutex> lock(pcap_->groupTreeMutex_);
      PacketGroupTree::Walker walker(pcap_->groupTree_);
      uint64_t ct = gettimeofday();
      while( walker.next() ){
        pGroup = &walker.object();
        if( pGroup->header_.bt_ > ct || pGroup->header_.et_ < ct ){
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
    if( !success ) pcap_->databaseInserterSem_.timedWait(1000000);
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

    success = pcap_->insertPacketsInDatabase(
      group->header_.bt_,
      group->header_.et_,
      group->packets_,
      group->header_.count_,
      this
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
  pcap_->insertPacketsInDatabase(0,0,NULL,0,this);
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
#ifndef __FreeBSD__
  priority(THREAD_PRIORITY_HIGHEST);
#endif
  AsyncFile tempFile(pcap_->tempFile_);
  tempFile.createIfNotExist(false).tryOpen();
  lastSwapIn_ = 0;
  lastSwapOut_ = gettimeofday() - pcap_->swapWatchTime_;
  for(;;){
    uilock_t memoryUsage = interlockedIncrement(pcap_->memoryUsage_,0);
    bool swapin = memoryUsage < pcap_->swapThreshold() * pcap_->swapLowWatermark_ / 100;
    bool swapout = memoryUsage >= pcap_->swapThreshold() * pcap_->swapHighWatermark_ / 100;
    while( (swapout && gettimeofday() - lastSwapIn_ >= pcap_->swapWatchTime_) || terminated_ ){
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
    while( !terminated_ && swapin && gettimeofday() - lastSwapOut_ >= pcap_->swapWatchTime_ && tempFile.isOpen() ){
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
