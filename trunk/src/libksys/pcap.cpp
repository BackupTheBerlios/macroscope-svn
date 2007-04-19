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
PCAP::~PCAP()
{
}
//------------------------------------------------------------------------------
PCAP::PCAP() : handle_(NULL), groupingPeriod_(pgpNone),
  packetsAutoDrop_(packetsList_), groupTreeAutoDrop_(groupTree_)
{
}
//------------------------------------------------------------------------------
void PCAP::threadBeforeWait()
{
#if HAVE_PCAP_H
  if( handle_ != NULL ){
    pcap_breakloop((pcap_t *) handle_);
  }
#endif
}
//------------------------------------------------------------------------------
void PCAP::threadExecute()
{
#if HAVE_PCAP_H
  char errbuf[PCAP_ERRBUF_SIZE]; // Error string
  struct bpf_program fp;         // The compiled filter expression
  bpf_u_int32 mask;              // The netmask of our sniffing device
  bpf_u_int32 net;               // The IP of our sniffing device
  bool freeCode = false;
  
  try {
    if( pcap_lookupnet(device_.getANSIString(),&net,&mask,errbuf) != 0 ) goto errexit;
    handle_ = pcap_open_live(device_.getANSIString(),0,promisc_,0,errbuf);
    if( handle_ == NULL ) goto errexit;
    if( pcap_compile((pcap_t *) handle_,&fp,filter_.getANSIString(),0,net) != 0 ) goto errexit;
    freeCode = true;
    if( pcap_setfilter((pcap_t *) handle_,&fp) != 0 ) goto errexit;
    pcap_loop((pcap_t *) handle_,-1,(pcap_handler) pcapCallback,(u_char *) this);
    pcap_freecode(&fp);
    pcap_close((pcap_t *) handle_);
    handle_ = NULL;
    if( packets_ != NULL ){
      AutoLock<InterlockedMutex> lock(packetsListMutex_);
      packetsList_.insToTail(*packets_.ptr(NULL));
      grouperSem_.post();
    }
    grouper_->terminate().wait();
    databaseInserter_->terminate().wait();
    lazyWriter_->terminate().wait();
  }
  catch( ExceptionSP & e ){
    if( handle_ != NULL ){
      if( freeCode ) pcap_freecode(&fp);
      pcap_close((pcap_t *) handle_);
      handle_ = NULL;
    }
    throw;
  }
errexit:  
  int32_t err = errno;
  if( handle_ != NULL ){
    if( freeCode ) pcap_freecode(&fp);
    pcap_close((pcap_t *) handle_);
    handle_ = NULL;
  }
  newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__ + utf8::String(" ") + errbuf)->throwSP();
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
//  const EthernetPacketHeader * ethernet = (const EthernetPacketHeader *)(packet);
  const IPPacketHeader * ip = (const IPPacketHeader *)(packet + SIZE_ETHERNET);
  uintptr_t sizeIp = ip->hl() * 4;
  if( sizeIp < 20 ){
    stdErr.debug(8,utf8::String::Stream() << __PRETTY_FUNCTION__ << ", Invalid IP header length: " << sizeIp << " bytes\n");
    return;
  }
  const TCPPacketHeader * tcp = (const TCPPacketHeader *)(packet + SIZE_ETHERNET + sizeIp);
  uintptr_t sizeTcp = 0;
  if( ip->proto_ == IPPROTO_TCP ){
    sizeTcp = tcp->off() * 4;
    if( sizeTcp < 20 ){
      stdErr.debug(8,utf8::String::Stream() << __PRETTY_FUNCTION__ << ", Invalid TCP header length: " << sizeTcp << " bytes\n");
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
    packets_ = newObjectV1<Packets>(getpagesize() * 16u / sizeof(Packet));
  }
  Packet & pkt = (*packets_.ptr())[packets_->packets_];
  pkt.timestamp_ = timestamp;
  pkt.pktSize_ = len;
  pkt.dataSize_ = len - (payload - packet);
  memcpy(&pkt.srcAddr_,&ip->src_,sizeof(pkt.srcAddr_));
  memcpy(&pkt.dstAddr_,&ip->dst_,sizeof(pkt.dstAddr_));
  pkt.srcPort_ = ip->proto_ == IPPROTO_TCP ? tcp->srcPort_ : 0;
  pkt.dstPort_ = ip->proto_ == IPPROTO_TCP ? tcp->dstPort_ : 0;
  pkt.proto_ = ip->proto_;							  
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
      bt_ = et_ = timestamp;
      break;
    case pgpSec  :
      bt_ = et_ = tm2Time(t);
      break;
    case pgpMin  :
      t.tm_sec = 0;
      bt_ = tm2Time(t);
      t.tm_sec = 59;
      et_ = tm2Time(t);
      break;
    case pgpHour :
      t.tm_sec = 0;
      t.tm_min = 0;
      bt_ = tm2Time(t);
      t.tm_sec = 59;
      t.tm_min = 59;
      et_ = tm2Time(t);
      break;
    case pgpDay  :
      t.tm_sec = 0;
      t.tm_min = 0;
      t.tm_hour = 0;
      bt_ = tm2Time(t);
      t.tm_sec = 59;
      t.tm_min = 59;
      t.tm_hour = 23;
      et_ = tm2Time(t);
      break;
    case pgpMon  :
      t.tm_sec = 0;
      t.tm_min = 0;
      t.tm_hour = 0;
      t.tm_mday = 1;
      bt_ = tm2Time(t);
      t.tm_sec = 59;
      t.tm_min = 59;
      t.tm_hour = 23;
      t.tm_mday = (int) monthDays(t.tm_year + 1900,t.tm_mon);
      et_ = tm2Time(t);
      break;
    case pgpYear :
      t.tm_sec = 0;
      t.tm_min = 0;
      t.tm_hour = 0;
      t.tm_mday = 1;
      t.tm_mon = 0;
      bt_ = tm2Time(t);
      t.tm_sec = 59;
      t.tm_min = 59;
      t.tm_hour = 23;
      t.tm_mday = 31;
      t.tm_mon = 11;
      et_ = tm2Time(t);
      break;
  }
  return *this;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void PCAP::Grouper::threadBeforeWait()
{
  if( pcap_ != NULL ) pcap_->grouperSem_.post();
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
      if( pGroup == group ) group.ptr(NULL);
      if( pGroup->count_ == pGroup->maxCount_ ){
        pGroup->packets_.realloc(((pGroup->count_ << 1) + ((pGroup->count_ == 0) << 4)) * sizeof(HashedPacket));
        pGroup->maxCount_ = (pGroup->count_ << 1) + ((pGroup->count_ == 0) << 4);
      }
      HashedPacket & hpkt = pGroup->packets_[pGroup->count_], * p;
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
        pGroup->count_++;
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
  if( pcap_ != NULL ) pcap_->databaseInserterSem_.post();
}
//------------------------------------------------------------------------------
void PCAP::DatabaseInserter::threadExecute()
{
  while( !terminated_ ){
    pcap_->databaseInserterSem_.wait();
    for(;;){
      PacketGroup * group = NULL;
      {
        AutoLock<InterlockedMutex> lock(pcap_->groupTreeMutex_);
        PacketGroupTree::Walker walker(pcap_->groupTree_);
	if( walker.next() ) group = &walker.object();
      }
      if( group == NULL ) break;
      pcap_->insertPacketsInDatabase(group->bt_,group->et_,group->packets_,group->count_);
      AutoLock<InterlockedMutex> lock(pcap_->groupTreeMutex_);
      pcap_->groupTree_.drop(*group);
    }    
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void PCAP::LazyWriter::threadBeforeWait()
{
  if( pcap_ != NULL ) pcap_->lazyWriterSem_.post();
}
//------------------------------------------------------------------------------
void PCAP::LazyWriter::threadExecute()
{
  while( !terminated_ ){
    pcap_->lazyWriterSem_.wait();
  }
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
