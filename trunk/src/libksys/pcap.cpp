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

    PCAP_API & open();
    PCAP_API & close();
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
    "pcap_freealldevs"
  };
  AutoLock<InterlockedMutex> lock(api.mutex());
#if defined(__WIN32__) || defined(__WIN64__)
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
#endif
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
    handle_ = NULL;
#endif
  }
  count_--;
  return *this;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void PCAP::initialize()
{
  new (api.mutex_) InterlockedMutex;
  api.count_ = 0;
}
//------------------------------------------------------------------------------
void PCAP::cleanup()
{
  api.mutex().~InterlockedMutex();
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
  swapHighWatermark_(98),
  swapInWatchTime_(1000000),
  promisc_(false),
  ports_(true),
  protocols_(true)
{
//  tempFile_ = getTempPath() + createGUIDAsBase32String() + ".tmp";
}
//------------------------------------------------------------------------------
/* From tcptraceroute, convert a numeric IP address to a string */
#define IPTOSBUFFERS	12
static inline char * iptos(u_long in)
{
	static char output[IPTOSBUFFERS][3*4+3+1];
	static short which;
	u_char *p;

	p = (u_char *)&in;
	which = (which + 1 == IPTOSBUFFERS ? 0 : which + 1);
	sprintf(output[which], "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
	return output[which];
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
            fprintf(stdout,"\tAddress: %s\n",iptos(((struct sockaddr_in *)a->addr)->sin_addr.s_addr));
          if( a->netmask )
            fprintf(stdout,"\tNetmask: %s\n",iptos(((struct sockaddr_in *)a->netmask)->sin_addr.s_addr));
          if( a->broadaddr )
            fprintf(stdout,"\tBroadcast Address: %s\n",iptos(((struct sockaddr_in *)a->broadaddr)->sin_addr.s_addr));
          if( a->dstaddr )
            fprintf(stdout,"\tDestination Address: %s\n",iptos(((struct sockaddr_in *)a->dstaddr)->sin_addr.s_addr));
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
#if HAVE_PCAP_H
  char errbuf[PCAP_ERRBUF_SIZE + 1]; // Error string
  struct bpf_program fp;         // The compiled filter expression
  bpf_u_int32 mask;              // The netmask of our sniffing device
  bpf_u_int32 net;               // The IP of our sniffing device
  bool freeCode = false;
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
    grouper_ = newObjectV1<Grouper>(this);
    databaseInserter_ = newObjectV1<DatabaseInserter>(this);
    lazyWriter_ = newObjectV1<LazyWriter>(this);
    memoryUsage_ = 0;
    grouper_->resume();
    databaseInserter_->resume();
    lazyWriter_->resume();
    api.pcap_loop((pcap_t *) handle_,-1,(pcap_handler) pcapCallback,(u_char *) this);
    if( freeCode ) api.pcap_freecode(&fp);
    api.pcap_close((pcap_t *) handle_);
    handle_ = NULL;
    if( packets_ != NULL ){
      AutoLock<InterlockedMutex> lock(packetsListMutex_);
      packetsList_.insToTail(*packets_.ptr(NULL));
      grouperSem_.post();
    }
  }
  catch( ExceptionSP & ){
    shutdown(freeCode ? &fp : NULL);
    api.close();
    throw;
  }
errexit:
  int32_t err = oserror() + errorOffset;
  shutdown(freeCode ? &fp : NULL);
  api.close();
  if( err != 0 )
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + errbuf + ", device: " + iface_)->throwSP();
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
void PCAP::capture(uint64_t timestamp,uintptr_t capLen,uintptr_t len,const uint8_t * packet)
{
#if HAVE_PCAP_H
#define SIZE_ETHERNET 14
  const EthernetPacketHeader * ethernet = (const EthernetPacketHeader *)(packet);
  const IPPacketHeader * ip = (const IPPacketHeader *)(packet + SIZE_ETHERNET);
  uintptr_t sizeIp = ip->hl() * 4;
  if( sizeIp < 20 ){
    if( stdErr.debugLevel(8) )
      stdErr.debug(8,utf8::String::Stream() <<
        "Invalid IP header length: " << sizeIp << " bytes, from MAC: " <<
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
      if( stdErr.debugLevel(8) ){
        ksock::SockAddr src, dst;
        src.addr4_.sin_addr = ip->src_;
        src.addr4_.sin_port = tcp->srcPort_;
        dst.addr4_.sin_addr = ip->dst_;
        dst.addr4_.sin_port = tcp->dstPort_;
        stdErr.debug(8,utf8::String::Stream() <<
          "Invalid TCP header length: " << sizeTcp << " bytes, from: " <<
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
      getpagesize() * 16u / sizeof(Packet)
#else
      getpagesize() * 16u / sizeof(Packet)
#endif
    );
  }
  Packet & pkt = (*packets_.ptr())[packets_->packets_];
  pkt.timestamp_ = timestamp;
  pkt.pktSize_ = uint16_t(len);
  pkt.dataSize_ = uint16_t(len - (payload - packet));
  memcpy(&pkt.srcAddr_,&ip->src_,sizeof(pkt.srcAddr_));
  memcpy(&pkt.dstAddr_,&ip->dst_,sizeof(pkt.dstAddr_));
  pkt.srcPort_ = ip->proto_ == IPPROTO_TCP && ports_ ? tcp->srcPort_ : 0;
  pkt.dstPort_ = ip->proto_ == IPPROTO_TCP && ports_ ? tcp->dstPort_ : 0;
  pkt.proto_ = protocols_ ? ip->proto_ : -1;
  packets_->packets_++;
#endif
}
//------------------------------------------------------------------------------
void PCAP::insertPacketsInDatabase(uint64_t,uint64_t,const HashedPacket *,uintptr_t)
{
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
PCAP::PacketGroup & PCAP::PacketGroup::setBounds(uint64_t timestamp,PacketGroupingPeriod groupingPeriod)
{
  struct tm t = time2tm(timestamp);
  switch( groupingPeriod ){
    case pgpNone :
      header_.bt_ = header_.et_ = timestamp;
      break;
    case pgpSec  :
      header_.bt_ = header_.et_ = tm2Time(t);
      break;
    case pgpMin  :
      t.tm_sec = 0;
      header_.bt_ = tm2Time(t);
      t.tm_sec = 59;
      header_.et_ = tm2Time(t);
      break;
    case pgpHour :
      t.tm_sec = 0;
      t.tm_min = 0;
      header_.bt_ = tm2Time(t);
      t.tm_sec = 59;
      t.tm_min = 59;
      header_.et_ = tm2Time(t);
      break;
    case pgpDay  :
      t.tm_sec = 0;
      t.tm_min = 0;
      t.tm_hour = 0;
      header_.bt_ = tm2Time(t);
      t.tm_sec = 59;
      t.tm_min = 59;
      t.tm_hour = 23;
      header_.et_ = tm2Time(t);
      break;
    case pgpMon  :
      t.tm_sec = 0;
      t.tm_min = 0;
      t.tm_hour = 0;
      t.tm_mday = 1;
      header_.bt_ = tm2Time(t);
      t.tm_sec = 59;
      t.tm_min = 59;
      t.tm_hour = 23;
      t.tm_mday = (int) monthDays(t.tm_year + 1900,t.tm_mon);
      header_.et_ = tm2Time(t);
      break;
    case pgpYear :
      t.tm_sec = 0;
      t.tm_min = 0;
      t.tm_hour = 0;
      t.tm_mday = 1;
      t.tm_mon = 0;
      header_.bt_ = tm2Time(t);
      t.tm_sec = 59;
      t.tm_min = 59;
      t.tm_hour = 23;
      t.tm_mday = 31;
      t.tm_mon = 11;
      header_.et_ = tm2Time(t);
      break;
  }
  return *this;
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
  while( !terminated_ ){
    pcap_->grouperSem_.wait();
    AutoPtr<Packets> packets;
    {
      AutoLock<InterlockedMutex> lock(pcap_->packetsListMutex_);
      if( pcap_->packetsList_.count() > 0 )
        packets.ptr(&pcap_->packetsList_.remove(*pcap_->packetsList_.first()));
    }
    if( packets == NULL || packets->packets_ == 0 ) continue;
    AutoPtr<PacketGroup> group;
    for( intptr_t i = packets->packets_ - 1; i >= 0; i-- ){
      const Packet & pkt = (*packets.ptr())[i];
      if( group == NULL ) group.ptr(newObject<PacketGroup>());
      group->setBounds(pkt.timestamp_,pcap_->groupingPeriod_);
      PacketGroup * pGroup;
      {
        AutoLock<InterlockedMutex> lock(pcap_->groupTreeMutex_);
        pcap_->groupTree_.insert(group,false,false,&pGroup);
      }
      if( pGroup == group ){
        group.ptr(NULL);
        interlockedIncrement(pcap_->memoryUsage_,sizeof(PacketGroup));
      }
      if( pGroup->header_.count_ == pGroup->maxCount_ ){
        uintptr_t count = (pGroup->header_.count_ << 1) + ((pGroup->header_.count_ == 0) << 4);
        pGroup->packets_.realloc(count * sizeof(HashedPacket));
        interlockedIncrement(pcap_->memoryUsage_,uilock_t((count - pGroup->maxCount_) * sizeof(HashedPacket)));
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
    pcap_->databaseInserterSem_.post();
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
static utf8::String formatByteLength(uintptr_t len,uintptr_t all)
{
  uintptr_t q, b, c, t1, t2, t3;
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
      len > 0 ? "%"PRIuPTR"(%"PRIuPTR".%02"PRIuPTR"%%)" :  "-",
      len,
      b,
      c
    );
  }
  t1 = len / t2;
  t3 = len % t2;
  return utf8::String::print(
    len > 0 ? "%"PRIuPTR".%04"PRIuPTR"%s(%"PRIuPTR".%02"PRIuPTR"%%)" :  "-",
    t1,
    uintptr_t(t3 / (t2 / 1024u)),
    postfix,
    b,
    c
  );
}
//------------------------------------------------------------------------------
void PCAP::DatabaseInserter::threadExecute()
{
  while( !terminated_ ){
    pcap_->databaseInserterSem_.wait();
    for(;;){
      bool swapOut = interlockedIncrement(pcap_->memoryUsage_,0) >= pcap_->swapThreshold() * pcap_->swapHighWatermark_ / 100 || terminated_;
      PacketGroup * group = NULL;
      {
        AutoLock<InterlockedMutex> lock(pcap_->groupTreeMutex_);
        PacketGroupTree::Walker walker(pcap_->groupTree_);
        if( walker.next() ) group = &walker.object();
      }
      uint64_t ct = gettimeofday();
      if( swapOut ){
        pcap_->lazyWriterSem_.post();
      }
      else {
        if( group == NULL || group->header_.et_ >= ct ) break;
        if( stdErr.debugLevel(7) )
          stdErr.debug(7,utf8::String::Stream() <<
            "Start processing packets group in: " <<
            utf8::time2Str(group->header_.bt_) << " - " <<
            utf8::time2Str(group->header_.et_) << ", count: " <<
            group->header_.count_ << ", memory usage: " <<
            formatByteLength(interlockedIncrement(pcap_->memoryUsage_,0),pcap_->swapThreshold()) << ", " <<
            __PRETTY_FUNCTION__ << "\n"
          );

        pcap_->insertPacketsInDatabase(group->header_.bt_,group->header_.et_,group->packets_,group->header_.count_);
        AutoLock<InterlockedMutex> lock(pcap_->groupTreeMutex_);
        interlockedIncrement(pcap_->memoryUsage_,-ilock_t(group->header_.count_ * sizeof(HashedPacket) + sizeof(PacketGroup)));
        pcap_->groupTree_.drop(*group);
      }
    }    
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
void PCAP::LazyWriter::threadExecute()
{
  AsyncFile tempFile(pcap_->tempFile_);
  tempFile.createIfNotExist(true).random(false);
  while( !terminated_ ){
    if( !tempFile.isOpen() || tempFile.size() == 0 ){
      pcap_->lazyWriterSem_.wait();
    }
    else {
      pcap_->lazyWriterSem_.timedWait(pcap_->swapInWatchTime_);
    }
    for(;;){
      bool swapOut = interlockedIncrement(pcap_->memoryUsage_,0) >= pcap_->swapThreshold() * pcap_->swapLowWatermark_ / 100 || terminated_;
      if( swapOut ){
        AutoPtr<PacketGroup> group;
        {
          AutoLock<InterlockedMutex> lock(pcap_->groupTreeMutex_);
          PacketGroupTree::Walker walker(pcap_->groupTree_);
          if( walker.next() ){
            group.ptr(&walker.object());
            pcap_->groupTree_.remove(walker.object());
          }
        }
        if( group == NULL ) break;
        tempFile.open().seek(tempFile.size()).
          writeBuffer(group->packets_,group->header_.count_ * sizeof(HashedPacket)).
          writeBuffer(&group->header_,sizeof(group->header_))
        ;
      }
      else if( tempFile.isOpen() && tempFile.size() > 0 ){
        uint64_t pos = tempFile.size() - sizeof(PacketGroup::SwapFileHeader);
        AutoPtr<PacketGroup> group(newObject<PacketGroup>());
        tempFile.readBuffer(pos,&group->header_,sizeof(group->header_));
        group->maxCount_ = group->header_.count_;
        uintptr_t groupSize = group->header_.count_ * sizeof(HashedPacket);
        group->packets_.alloc(groupSize);
        tempFile.readBuffer(pos - groupSize,group->packets_,groupSize);
      }
    }
  }
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------