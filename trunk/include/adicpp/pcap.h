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
    ~PCAP();
    PCAP();

    enum PacketGroupingPeriod { pgpNone, pgpSec, pgpMin, pgpHour, pgpDay, pgpMon, pgpYear };

    const utf8::String & device() const;
    PCAP & device(const utf8::String & device);
    const utf8::String & filter() const;
    PCAP & filter(const utf8::String & filter);
    const bool & promisc() const;
    PCAP & promisc(bool a);

    const PacketGroupingPeriod & groupingPeriod() const;
    PCAP & groupingPeriod(PacketGroupingPeriod groupingPeriod);
  protected:
    class Packet {
      public:
        uint64_t timestamp_;
        struct in_addr srcAddr_;
        struct in_addr dstAddr_;
        uint16_t srcPort_;
	      uint16_t dstPort_;
        uint16_t pktSize_;
        uint16_t dataSize_;
        uint8_t proto_;
    };
    class HashedPacket {
      public:
        static EmbeddedHashNode<HashedPacket,uintptr_t> & ehNLT(const uintptr_t & link,const AutoPtr<HashedPacket> * & param){
	        return keyNode(param[link - 1]);
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
	
        uint64_t pktSize_;
        uint64_t dataSize_;
        struct in_addr srcAddr_;
        struct in_addr dstAddr_;
        uint16_t srcPort_;
	      uint16_t dstPort_;
        uint8_t proto_;
    };
    virtual void insertPacketsInDatabase(uint64_t bt,uint64_t et,const HashedPacket * pkts,uintptr_t count);
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
        PacketGroup() : count_(0), maxCount_(0) { packetsHash_.param() = &packets_; }
      
        uint64_t bt_;
        uint64_t et_;
      
        AutoPtr<HashedPacket> packets_;
        uintptr_t count_;
        uintptr_t maxCount_;
        PacketsHash packetsHash_;
	
        PacketGroup & setBounds(uint64_t timestamp,PacketGroupingPeriod groupingPeriod);
	bool isInBounds(uint64_t timestamp) const { return timestamp >= bt_ && timestamp <= et_; }
      
        static RBTreeNode & treeO2N(const PacketGroup & object){
          return object.treeNode_;
	}
	static PacketGroup & treeN2O(const RBTreeNode & node){
	  PacketGroup * p = NULL;
	  return node.object<PacketGroup>(p->treeNode_);
	}
	static intptr_t treeCO(const PacketGroup & a0,const PacketGroup & a1){
	  assert( (a0.bt_ > a1.et_ && a0.et_ > a1.et_) || (a0.et_ < a1.bt_ && a0.bt_ < a1.bt_));
	  return a0.bt_ > a1.bt_ ? 1 : a0.bt_ < a1.bt_ ? -1 : 0;
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
        LazyWriter(PCAP * pcap = NULL) : pcap_(pcap) {}
      private:
        PCAP * pcap_;
	
        void threadBeforeWait();
        void threadExecute();
    };
    friend class LazyWriter;
	        
    void * handle_;
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
    utf8::String device_;
    utf8::String filter_;
    bool promisc_;
        
    void threadBeforeWait();
    void threadExecute();
    static void pcapCallback(void *,const void *,const void *);
    void capture(uint64_t timestamp,uintptr_t capLen,uintptr_t len,const uint8_t * packet);
    PCAP(const PCAP &);
    void operator = (const PCAP &);
};
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
