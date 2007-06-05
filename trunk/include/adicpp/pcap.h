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
#ifndef pcapH
#define pcapH
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#elif defined(__BCPLUSPLUS__)
#pragma option push -a1
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
/* Ethernet header */
struct PACKED EthernetPacketHeader {
  uint8_t dstAddr_[6]; /* Destination host address */
  uint8_t srcAddr_[6]; /* Source host address */
  uint16_t type_; /* IP? ARP? RARP? etc */
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
/* IP header */
struct PACKED IPPacketHeader {
  uint8_t vhl_;          /* version << 4 | header length >> 2 */
  uint8_t tos_;          /* type of service */
  uint16_t len_;         /* total length */
  uint16_t id_;          /* identification */
  uint16_t off_;         /* fragment offset field */
  uint8_t ttl_;          /* time to live */
  uint8_t proto_;        /* protocol */
  uint16_t sum_;         /* checksum */
  struct in_addr src_, dst_; /* source and dest address */
  
  static const uint16_t RF; /* reserved fragment flag */
  static const uint16_t DF; /* dont fragment flag */
  static const uint16_t MF; /* more fragments flag */
  static const uint16_t OFFMASK; /* mask for fragmenting bits */
  
  uint8_t hl() const { return vhl_ & 0x0f; }
  uint8_t v() const { return vhl_ >> 4; }
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
/* TCP header */
struct PACKED TCPPacketHeader {
  uint16_t srcPort_;       /* source port */
  uint16_t dstPort_;       /* destination port */
  uint32_t seq_;         /* sequence number */
  uint32_t ack_;         /* acknowledgement number */
  uint8_t offx2_;        /* data offset, rsvd */
  uint8_t flags_;
  uint16_t win_;         /* window */
  uint16_t sum_;         /* checksum */
  uint16_t urp_;         /* urgent pointer */
  
  static const uint8_t FIN;
  static const uint8_t SYN;
  static const uint8_t RST;
  static const uint8_t PUSH;
  static const uint8_t ACK;
  static const uint8_t URG;
  static const uint8_t ECE;
  static const uint8_t CWR;
  static const uint8_t FLAGS;
  
  uint8_t off() const { return (offx2_ & 0xf0) >> 4; }
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
/* UDP header */
struct PACKED UDPPacketHeader {
  uint16_t srcPort_;               /* source port */
  uint16_t dstPort_;               /* destination port */
  uint16_t ulen_;                /* udp length */
  uint16_t sum_;                 /* udp checksum */
};				
//---------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma pack(pop)
#elif defined(__BCPLUSPLUS__)
#pragma option pop
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class PCAP : public Thread {
 public:
    virtual ~PCAP();
    PCAP();

    enum PacketGroupingPeriod { pgpNone, pgpSec, pgpMin, pgpHour, pgpDay, pgpMon, pgpYear };

    static void initialize();
    static void cleanup();

    const utf8::String & ifName() const;
    PCAP & ifName(const utf8::String & ifName);
    const utf8::String & iface() const;
    PCAP & iface(const utf8::String & iface);
    const utf8::String & filter() const;
    PCAP & filter(const utf8::String & filter);
    const utf8::String & tempFile() const;
    PCAP & tempFile(const utf8::String & tempFile);
    const uintptr_t & pcapReadTimeout() const;
    PCAP & pcapReadTimeout(uintptr_t a);
    const uintptr_t & swapThreshold() const;
    PCAP & swapThreshold(uintptr_t a);
    const uintptr_t & pregroupingBufferSize() const;
    PCAP & pregroupingBufferSize(uintptr_t a);
    const uintptr_t & pregroupingWindowSize() const;
    PCAP & pregroupingWindowSize(uintptr_t a);
    const ldouble & swapLowWatermark() const;
    PCAP & swapLowWatermark(ldouble a);
    const ldouble & swapHighWatermark() const;
    PCAP & swapHighWatermark(ldouble a);
    const uint64_t & swapWatchTime() const;
    PCAP & swapWatchTime(uint64_t a);
    const bool & promisc() const;
    PCAP & promisc(bool a);
    const bool & ports() const;
    PCAP & ports(bool a);
    const bool & protocols() const;
    PCAP & protocols(bool a);

    const PacketGroupingPeriod & groupingPeriod() const;
    PCAP & groupingPeriod(PacketGroupingPeriod groupingPeriod);

    static void printAllDevices();

    void setBounds(uint64_t timestamp,uint64_t & bt,uint64_t & et) const;

    PCAP & join(PCAP * pcap);
  protected:
    class Packet {
      public:
        uint64_t pktSize_;
        uint64_t dataSize_;
        uint64_t timestamp_;
        struct in_addr srcAddr_;
        struct in_addr dstAddr_;
        uint16_t srcPort_;
	      uint16_t dstPort_;
        int16_t proto_;

        bool operator == (const Packet & object) const {
          bool r = timestamp_ == object.timestamp_;
          if( r ){
            r = memcmp(&srcAddr_,&object.srcAddr_,sizeof(srcAddr_)) == 0;
	          if( r ){
	            r = memcmp(&dstAddr_,&object.dstAddr_,sizeof(dstAddr_)) == 0;
	            if( r ){
                r = srcPort_ == object.srcPort_;
                if( r ){
                  r = dstPort_ == object.dstPort_;
                  if( r ) r = proto_ == object.proto_;
                }
	            }
	          }
          }
          return r;
        }
    };
    class HashedPacket {
      public:
        static EmbeddedHashNode<HashedPacket,uintptr_t> & ehNLT(const uintptr_t & link,const AutoPtr<HashedPacket> * & param){
	        return keyNode((*param)[link - 1]);
	      }
	      static uintptr_t ehLTN(const EmbeddedHashNode<HashedPacket,uintptr_t> & node,const AutoPtr<HashedPacket> * & param){
	        return &keyNodeObject(node,NULL) - param->ptr() + 1;
	      }
	      static EmbeddedHashNode<HashedPacket,uintptr_t> & keyNode(const HashedPacket & object){
	        return object.keyNode_;
	      }
	      static HashedPacket & keyNodeObject(const EmbeddedHashNode<HashedPacket,uintptr_t> & node,HashedPacket * p){
	        return node.object(p->keyNode_);
	      }
	      static uintptr_t keyNodeHash(const HashedPacket & object){
	        uintptr_t h = HF::hash(&object.srcAddr_,sizeof(object.srcAddr_));
	        h = HF::hash(&object.dstAddr_,sizeof(object.dstAddr_),h);
	        h = HF::hash(&object.srcPort_,sizeof(object.srcPort_),h);
	        h = HF::hash(&object.dstPort_,sizeof(object.dstPort_),h);
	        return HF::hash(&object.proto_,sizeof(object.proto_),h);
        }
        static bool keyHashNodeEqu(const HashedPacket & object1,const HashedPacket & object2){
          intptr_t c = memcmp(&object1.srcAddr_,&object2.srcAddr_,sizeof(&object1.srcAddr_));
	        if( c == 0 ){
	          c = memcmp(&object1.dstAddr_,&object2.dstAddr_,sizeof(&object1.dstAddr_));
	          if( c == 0 ){
              c = intptr_t(object1.srcPort_) - intptr_t(object2.srcPort_);
              if( c == 0 ){
                c = intptr_t(object1.dstPort_) - intptr_t(object2.dstPort_);
                if( c == 0 ) c = intptr_t(object1.proto_) - intptr_t(object2.proto_);
              }
	          }
	        }
          return c == 0;
        }
        mutable EmbeddedHashNode<HashedPacket,uintptr_t> keyNode_;
	
        uintmax_t pktSize_;
        uintmax_t dataSize_;
        struct in_addr srcAddr_;
        struct in_addr dstAddr_;
        uint16_t srcPort_;
        uint16_t dstPort_;
        int16_t proto_;
    };
    virtual bool insertPacketsInDatabase(uint64_t bt,uint64_t et,const HashedPacket * pkts,uintptr_t count,Thread * caller) throw();
    void threadExecute();
  private:
    class Packets : public Array<Packet> {
      public:
	      Packets() {}
	      Packets(uintptr_t packets) : packets_(0) { resize(packets); }

        uintptr_t packets_;
	
        static EmbeddedListNode<Packets> & listNode(const Packets & object){
          return object.listNode_;
        }
        static Packets & listObject(const EmbeddedListNode<Packets> & node,Packets * p){
          return node.object(p->listNode_);
  	    }
      	mutable EmbeddedListNode<Packets> listNode_;
    };
    typedef EmbeddedList<Packets,Packets::listNode,Packets::listObject> PacketsList;

    typedef EmbeddedHash<
      HashedPacket,
      uintptr_t,
      const AutoPtr<HashedPacket> *,
      HashedPacket::ehNLT,
      HashedPacket::ehLTN,
      HashedPacket::keyNode,
      HashedPacket::keyNodeObject,
      HashedPacket::keyNodeHash,
      HashedPacket::keyHashNodeEqu
    > PacketsHash;

    class PacketGroup {
      public:
        PacketGroup() : maxCount_(0) { packetsHash_.param() = &packets_; }
      
        class SwapFileHeader {
          public:
            SwapFileHeader() : count_(0) {}

            uint64_t bt_;
            uint64_t et_;
            uintptr_t count_;
        };
        SwapFileHeader header_;
      
        AutoPtr<HashedPacket> packets_;
        uintptr_t maxCount_;
        PacketsHash packetsHash_;
	
        PacketGroup & joinGroup(const PacketGroup & group,volatile uilock_t & memoryUsage);

	      bool isInBounds(uint64_t timestamp) const { return timestamp >= header_.bt_ && timestamp <= header_.et_; }
      
        static RBTreeNode & treeO2N(const PacketGroup & object){
          return object.treeNode_;
	      }
	      static PacketGroup & treeN2O(const RBTreeNode & node){
	        PacketGroup * p = NULL;
	        return node.object<PacketGroup>(p->treeNode_);
	      }
	      static intptr_t treeCO(const PacketGroup & a0,const PacketGroup & a1){
	        return a0.header_.bt_ > a1.header_.et_ ? 1 : a0.header_.et_ < a1.header_.bt_ ? -1 : 0;
	      }
	      mutable RBTreeNode treeNode_;
    };
    typedef
      RBTree<
        PacketGroup,
        PacketGroup::treeO2N,
        PacketGroup::treeN2O,
        PacketGroup::treeCO
    > PacketGroupTree;
    
    class Grouper : public Thread {
      public:
        Grouper(PCAP * pcap = NULL) : pcap_(pcap) {}
      private:
        PCAP * pcap_;
	
	      Packets * get();
        void threadBeforeWait();
        void threadExecute();
    };
    friend class Grouper;

    class DatabaseInserter : public Thread {
      public:
        DatabaseInserter(PCAP * pcap = NULL) : pcap_(pcap) {}
      private:
        PCAP * pcap_;
	
        void threadBeforeWait();
        void threadExecute();
    };
    friend class DatabaseInserter;

    class LazyWriter : public Thread {
      public:
        LazyWriter(PCAP * pcap = NULL) : pcap_(pcap), lastSwapOut_(0), lastSwapIn_(0) {}
      private:
        PCAP * pcap_;
        uint64_t lastSwapOut_;
        uint64_t lastSwapIn_;
	
        void threadBeforeWait();
        void threadExecute();
        void swapOut(AsyncFile & tempFile,AutoPtr<PacketGroup> & group);
        void swapIn(AsyncFile & tempFile);
    };
    friend class LazyWriter;

    static EmbeddedListNode<PCAP> & listNode(const PCAP & object){
      return object.listNode_;
    }
    static PCAP & listObject(const EmbeddedListNode<PCAP> & node,PCAP * p = NULL){
      return node.object(p->listNode_);
    }
  	mutable EmbeddedListNode<PCAP> listNode_;
    EmbeddedList<PCAP,listNode,listObject> joined_;
    PCAP * joinMaster_;

    void * handle_;
    void * fp_;
    PacketGroupingPeriod groupingPeriod_;
    PacketsList packetsList_;
    InterlockedMutex packetsListMutex_;
    AutoDrop<PacketsList> packetsAutoDrop_;
    Semaphore grouperSem_;
    AutoPtr<Grouper> grouper_;
    Semaphore databaseInserterSem_;
    AutoPtr<DatabaseInserter> databaseInserter_;
    Semaphore lazyWriterSem_;
    AutoPtr<LazyWriter> lazyWriter_;
    AutoPtr<Packets> packets_;
    InterlockedMutex groupTreeMutex_;
    PacketGroupTree groupTree_;
    AutoDrop<PacketGroupTree> groupTreeAutoDrop_;
    utf8::String ifName_;
    utf8::String iface_;
    utf8::String filter_;
    utf8::String tempFile_;
    uintptr_t pcapReadTimeout_;
    uintptr_t swapThreshold_;
    uintptr_t pregroupingBufferSize_;
    uintptr_t pregroupingWindowSize_;
    volatile uilock_t memoryUsage_;
    ldouble swapLowWatermark_;
    ldouble swapHighWatermark_;
    uint64_t swapWatchTime_;
    uint64_t curPeriod_;
    bool promisc_;
    bool ports_;
    bool protocols_;
    bool fc_;
    
    void shutdown();
    void threadBeforeWait();
    void threadAfterWait();
    void groupPackets();
    static void pcapCallback(void *,const void *,const void *);
    void capture(uint64_t timestamp,uintptr_t capLen,uintptr_t len,const uint8_t * packet);

    PCAP(const PCAP &);
    void operator = (const PCAP &);
};
//---------------------------------------------------------------------------
inline const utf8::String & PCAP::ifName() const
{
  return ifName_;
}
//---------------------------------------------------------------------------
inline PCAP & PCAP::ifName(const utf8::String & ifName)
{
  ifName_ = ifName;
  tempFile_ = getTempPath() + base32Encode(ifName_.c_str(),ifName_.size()) + ".tmp";
  return *this;
}
//---------------------------------------------------------------------------
inline const utf8::String & PCAP::iface() const
{
  return iface_;
}
//---------------------------------------------------------------------------
inline PCAP & PCAP::iface(const utf8::String & iface)
{
  iface_ = iface;
  return *this;
}
//---------------------------------------------------------------------------
inline const utf8::String & PCAP::filter() const
{
  return filter_;
}
//---------------------------------------------------------------------------
inline PCAP & PCAP::filter(const utf8::String & filter)
{
  filter_ = filter;
  return *this;
}
//---------------------------------------------------------------------------
inline const utf8::String & PCAP::tempFile() const
{
  return tempFile_;
}
//---------------------------------------------------------------------------
inline PCAP & PCAP::tempFile(const utf8::String & tempFile)
{
  tempFile_ = tempFile;
  return *this;
}
//---------------------------------------------------------------------------
inline const bool & PCAP::promisc() const
{
  return promisc_;
}
//---------------------------------------------------------------------------
inline PCAP & PCAP::promisc(bool a)
{
  promisc_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const bool & PCAP::ports() const
{
  return ports_;
}
//---------------------------------------------------------------------------
inline PCAP & PCAP::ports(bool a)
{
  ports_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const bool & PCAP::protocols() const
{
  return protocols_;
}
//---------------------------------------------------------------------------
inline PCAP & PCAP::protocols(bool a)
{
  protocols_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const uintptr_t & PCAP::pcapReadTimeout() const
{
  return pcapReadTimeout_;
}
//---------------------------------------------------------------------------
inline PCAP & PCAP::pcapReadTimeout(uintptr_t a)
{
  pcapReadTimeout_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const uintptr_t & PCAP::swapThreshold() const
{
  return swapThreshold_;
}
//---------------------------------------------------------------------------
inline PCAP & PCAP::swapThreshold(uintptr_t a)
{
  if( a < (uintptr_t) getpagesize() ) a = getpagesize();
  swapThreshold_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const uintptr_t & PCAP::pregroupingBufferSize() const
{
  return pregroupingBufferSize_;
}
//---------------------------------------------------------------------------
inline PCAP & PCAP::pregroupingBufferSize(uintptr_t a)
{
  if( a < (uintptr_t) getpagesize() ) a = getpagesize();
  pregroupingBufferSize_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const uintptr_t & PCAP::pregroupingWindowSize() const
{
  return pregroupingWindowSize_;
}
//---------------------------------------------------------------------------
inline PCAP & PCAP::pregroupingWindowSize(uintptr_t a)
{
  pregroupingWindowSize_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const PCAP::PacketGroupingPeriod & PCAP::groupingPeriod() const
{
  return groupingPeriod_;
}
//---------------------------------------------------------------------------
inline PCAP & PCAP::groupingPeriod(PacketGroupingPeriod groupingPeriod)
{
  groupingPeriod_ = groupingPeriod;
  return *this;
}
//---------------------------------------------------------------------------
inline const ldouble & PCAP::swapLowWatermark() const
{
  return swapLowWatermark_;
}
//---------------------------------------------------------------------------
inline PCAP & PCAP::swapLowWatermark(ldouble a)
{
  if( a > 100 || a <= 0 ) a = 50;
  swapLowWatermark_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const ldouble & PCAP::swapHighWatermark() const
{
  return swapHighWatermark_;
}
//---------------------------------------------------------------------------
inline PCAP & PCAP::swapHighWatermark(ldouble a)
{
  if( a > 100 || a <= 0 ) a = 90;
  swapHighWatermark_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const uint64_t & PCAP::swapWatchTime() const
{
  return swapWatchTime_;
}
//---------------------------------------------------------------------------
inline PCAP & PCAP::swapWatchTime(uint64_t a)
{
  swapWatchTime_ = a;
  return *this;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
