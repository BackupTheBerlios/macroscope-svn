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
//------------------------------------------------------------------------------
#ifndef _macroscopeH_
#define _macroscopeH_
//------------------------------------------------------------------------------
#define _VERSION_C_AS_HEADER_
#include "version.c"
#undef _VERSION_C_AS_HEADER_
//------------------------------------------------------------------------------
using namespace ksys;
using namespace adicpp;
//------------------------------------------------------------------------------
namespace macroscope {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class SnifferService;
//------------------------------------------------------------------------------
class Logger {
  friend class SnifferService;
  public:
    virtual ~Logger();
    Logger(bool sniffer,bool daemon);

    void readConfig();
    int32_t main();
    Logger & rolloutBPFTByIPs(const utf8::String & bt,const utf8::String & et,const utf8::String & ifName,const utf8::String & filter);
    static utf8::String formatTraf(uintmax_t traf,uintmax_t allTraf);
    const CGI & cgi() const { return cgi_; }
  protected:
    enum { stSel, stIns, stUpd };

    CGI cgi_;
    ConfigSPi config_;
    utf8::String connection_;
    AutoPtr<Database> database_;
    AutoPtr<Statement> statement_;
    AutoPtr<Statement> statement2_;
    bool verbose_;
    bool sniffer_;
    bool daemon_;
    bool configReaded_;

    // html reporter
    enum TrafType { ttSMTP, ttWWW, ttAll, ttCount };
    class TrafCacheEntry {
      public:
        int64_t       traf_;
        utf8::String  user_;
        struct tm     bt_;
        struct tm     et_;
        TrafType      trafType_;

        virtual ~TrafCacheEntry() {}
        TrafCacheEntry(){}
        TrafCacheEntry(const utf8::String & user,const struct tm & bt,const struct tm & et,TrafType trafType)
          : user_(user), bt_(bt), et_(et), trafType_(trafType) {}

        utf8::String id() const;

        static EmbeddedHashNode<TrafCacheEntry,uintptr_t> & ehNLT(const uintptr_t & link,uintptr_t * &){
          return *reinterpret_cast<EmbeddedHashNode<TrafCacheEntry,uintptr_t> *>(link);
        }
        static uintptr_t ehLTN(const EmbeddedHashNode<TrafCacheEntry,uintptr_t> & node,uintptr_t * &){
          return reinterpret_cast<uintptr_t>(&node);
        }			    
        static EmbeddedHashNode<TrafCacheEntry,uintptr_t> & keyNode(const TrafCacheEntry & object){
          return object.keyNode_;
        }
        static TrafCacheEntry & keyNodeObject(const EmbeddedHashNode<TrafCacheEntry,uintptr_t> & node,TrafCacheEntry * p){
          return node.object(p->keyNode_);
        }
        static uintptr_t keyNodeHash(const TrafCacheEntry & object){
          return object.id().hash(false);
        }
        static bool keyHashNodeEqu(const TrafCacheEntry & object1,const TrafCacheEntry & object2){
          return object1.id().casecompare(object2.id()) == 0;
        }
        
        mutable EmbeddedHashNode<TrafCacheEntry,uintptr_t> keyNode_;

        static EmbeddedListNode<TrafCacheEntry> & listNode(const TrafCacheEntry & object){
          return object.listNode_;
        }
        static TrafCacheEntry & listNodeObject(const EmbeddedListNode<TrafCacheEntry> & node,TrafCacheEntry * p = NULL){
          return node.object(p->listNode_);
        }

        mutable EmbeddedListNode<TrafCacheEntry> listNode_;
    };
    typedef EmbeddedHash<
      TrafCacheEntry,
      uintptr_t,
      uintptr_t *,
      TrafCacheEntry::ehNLT,
      TrafCacheEntry::ehLTN,
      TrafCacheEntry::keyNode,
      TrafCacheEntry::keyNodeObject,
      TrafCacheEntry::keyNodeHash,
      TrafCacheEntry::keyHashNodeEqu
    > TrafCache;
    TrafCache trafCache_;
    AutoDrop<TrafCache> trafCacheAutoDrop_;
    typedef EmbeddedList<
      TrafCacheEntry,
      TrafCacheEntry::listNode,
      TrafCacheEntry::listNodeObject
    > TrafCacheLRU;
    TrafCacheLRU trafCacheLRU_;
    uintptr_t trafCacheSize_;
    WriteLock trafCacheReadWriteLock_;

    class SquidSendmailThread : public Thread {
      public:
        virtual ~SquidSendmailThread();
        SquidSendmailThread() {}
        SquidSendmailThread(Logger & logger,const utf8::String & section,const utf8::String & sectionName,uintptr_t stage);

        void threadExecute();
      protected:
        Logger * logger_;
        utf8::String section_;
        utf8::String sectionName_;
        utf8::String shortUrl_;
        utf8::String htmlDir_;
        utf8::String perGroupReportName_;
        utf8::String perGroupReportDir_;
        int64_t ellapsed_;
        struct tm curTime_;
        uintptr_t gCount_;
        bool groups_;
        bool perGroupReport_;
        uintptr_t stage_;

        AutoPtr<Database> database_;
	
        AutoPtr<Statement> statement_;
        AutoPtr<Statement> stTrafIns_;
        AutoPtr<Statement> stTrafUpd_;
        AutoPtr<Statement> stMonUrlSel_;
        AutoPtr<Statement> stMonUrlIns_;
        AutoPtr<Statement> stMonUrlUpd_;
    
        AutoPtr<Statement> stFileStat_[3];
    
        AutoPtr<Statement> stMsgsIns_;
        AutoPtr<Statement> stMsgsUpd_;
        AutoPtr<Statement> stMsgsSel_;
        AutoPtr<Statement> stMsgsDel_;
        AutoPtr<Statement> stMsgsDel2_;
        AutoPtr<Statement> stMsgsSelCount_;

        utf8::String trafTypeNick_[ttCount];
        utf8::String trafTypeHeadColor_[ttCount];
        utf8::String trafTypeBodyColor_[ttCount];
        utf8::String trafTypeTailColor_[ttCount];
        utf8::String trafTypeHeadDataColor_[ttCount];
        utf8::String trafTypeBodyDataColor_[ttCount];
        utf8::String trafTypeTailDataColor_[ttCount];

        void decoration();

        void parseSquidLogFile(const utf8::String & logFileName,bool top10,const utf8::String & skipUrl);
        void parseSendmailLogFile(const utf8::String & logFileName,const utf8::String & domain,uintptr_t startYear);
        void writeHtmlYearOutput();

        void parseSquidLogLine(char * p, uintptr_t size,Array<const char *> & slcp);
        utf8::String squidStrToWideString(const char * str);
        utf8::String shortUrl(const utf8::String & url);

        int64_t getTrafNL(TrafType tt,const struct tm & bt,const struct tm & et,const utf8::String & user = utf8::String(),uintptr_t isGroup = 0);
        int64_t getTraf(TrafType tt,const struct tm & bt,const struct tm & et,const utf8::String & user = utf8::String(),uintptr_t isGroup = 0);
        void writeUserTop(const utf8::String & file,const utf8::String & user,uintptr_t isGroup,const struct tm & beginTime,const struct tm & endTime,TrafType tt);
        void writeMonthHtmlOutput(const utf8::String & file,const struct tm & year,bool threaded = false);
        uintptr_t nonZeroYearMonthsColumns(struct tm byear,const utf8::String & user,uintptr_t isGroup);
        uintptr_t nonZeroMonthDaysColumns(struct tm bmon,const utf8::String & user,uintptr_t isGroup);
        static intptr_t sortUsersTrafTable(uintptr_t row1,uintptr_t row2,const Table<Mutant> & table);
        static intptr_t sortUsersTrafTables(Table<Mutant> * & p1,Table<Mutant> * & p2);
        void genUsersTable(Vector<Table<Mutant> > & usersTrafTables,const struct tm & beginTime,const struct tm & endTime);
        utf8::String genUserFilter(const utf8::String & user,uintptr_t isGroup);
      private:
    };
    friend class SquidSendmailThread;
  
    class DNSCacheEntry : public ksock::SockAddr {
      public:
        utf8::String name_;

        virtual ~DNSCacheEntry() {}
        DNSCacheEntry() {}

        static EmbeddedHashNode<DNSCacheEntry,uintptr_t> & ehNLT(const uintptr_t & link,uintptr_t * &){
          return *reinterpret_cast<EmbeddedHashNode<DNSCacheEntry,uintptr_t> *>(link);
        }
        static uintptr_t ehLTN(const EmbeddedHashNode<DNSCacheEntry,uintptr_t> & node,uintptr_t * &){
          return reinterpret_cast<uintptr_t>(&node);
        }			    
        static EmbeddedHashNode<DNSCacheEntry,uintptr_t> & keyNode(const DNSCacheEntry & object){
          return object.keyNode_;
        }
        static DNSCacheEntry & keyNodeObject(const EmbeddedHashNode<DNSCacheEntry,uintptr_t> & node,DNSCacheEntry * p){
          return node.object(p->keyNode_);
        }
        static uintptr_t keyNodeHash(const DNSCacheEntry & object){
#if SIZEOF_SOCKADDR_IN6
	        if( object.addr4_.sin_family == PF_INET6 )
            return HF::hash(&object.addr6_.sin6_addr,sizeof(object.addr6_.sin6_addr));
#endif
          return HF::hash(&object.addr4_.sin_addr,sizeof(object.addr4_.sin_addr));
	      }
        static bool keyHashNodeEqu(const DNSCacheEntry & object1,const DNSCacheEntry & object2){
	        assert( object1.addr4_.sin_family == object2.addr4_.sin_family );
#if SIZEOF_SOCKADDR_IN6
	        if( object1.addr4_.sin_family == PF_INET6 )
            return memcmp(&object1.addr6_.sin6_addr,&object2.addr6_.sin6_addr,sizeof(object2.addr6_.sin6_addr)) == 0;
#endif
          return memcmp(&object1.addr4_.sin_addr,&object2.addr4_.sin_addr,sizeof(object2.addr4_.sin_addr)) == 0;
        }
        
        mutable EmbeddedHashNode<DNSCacheEntry,uintptr_t> keyNode_;

        static EmbeddedListNode<DNSCacheEntry> & listNode(const DNSCacheEntry & object){
          return object.listNode_;
        }
        static DNSCacheEntry & listNodeObject(const EmbeddedListNode<DNSCacheEntry> & node,DNSCacheEntry * p = NULL){
          return node.object(p->listNode_);
        }

        mutable EmbeddedListNode<DNSCacheEntry> listNode_;
    };
    typedef EmbeddedHash<
      DNSCacheEntry,
      uintptr_t,
      uintptr_t *,
      DNSCacheEntry::ehNLT,
      DNSCacheEntry::ehLTN,
      DNSCacheEntry::keyNode,
      DNSCacheEntry::keyNodeObject,
      DNSCacheEntry::keyNodeHash,
      DNSCacheEntry::keyHashNodeEqu
    > DNSCache;
    DNSCache dnsCache_;
    AutoDrop<DNSCache> dnsCacheAutoDrop_;
    typedef EmbeddedList<
      DNSCacheEntry,
      DNSCacheEntry::listNode,
      DNSCacheEntry::listNodeObject
    > DNSCacheLRU;
    DNSCacheLRU dnsCacheLRU_;
    uintptr_t dnsCacheSize_;
    uintmax_t dnsCacheHitCount_;
    uintmax_t dnsCacheMissCount_;
    WriteLock dnsReadWriteLock_;

    class BPFTThread : public Thread {
      public:
        virtual ~BPFTThread();
        BPFTThread();
        BPFTThread(Logger & logger,const utf8::String & section,const utf8::String & sectionName,uintptr_t stage);

        void threadExecute();
      protected:
        class CachedPacketSum {
          public:
            virtual ~CachedPacketSum() {}
            CachedPacketSum() {}
            static EmbeddedHashNode<CachedPacketSum,uintptr_t> & ehNLT(const uintptr_t & link,uintptr_t &){
	            return *reinterpret_cast<EmbeddedHashNode<CachedPacketSum,uintptr_t> *>(link);
	          }
	          static uintptr_t ehLTN(const EmbeddedHashNode<CachedPacketSum,uintptr_t> & node,uintptr_t &){
	            return reinterpret_cast<uintptr_t>(&node);
	          }
	          static EmbeddedHashNode<CachedPacketSum,uintptr_t> & keyNode(const CachedPacketSum & object){
	            return object.keyNode_;
	          }
	          static CachedPacketSum & keyNodeObject(const EmbeddedHashNode<CachedPacketSum,uintptr_t> & node,CachedPacketSum * p){
	            return node.object(p->keyNode_);
	          }
	          static uintptr_t keyNodeHash(const CachedPacketSum & object){
	            uintptr_t h = HF::hash(&object.bt_,sizeof(object.bt_));
              h = HF::hash(&object.et_,sizeof(object.et_),h);
	            h = HF::hash(&object.srcAddr_,sizeof(object.srcAddr_),h);
	            h = HF::hash(&object.dstAddr_,sizeof(object.dstAddr_),h);
	            h = HF::hash(&object.srcPort_,sizeof(object.srcPort_),h);
	            h = HF::hash(&object.dstPort_,sizeof(object.dstPort_),h);
	            return HF::hash(&object.proto_,sizeof(object.proto_),h);
            }
            static bool keyHashNodeEqu(const CachedPacketSum & object1,const CachedPacketSum & object2){
              bool c = object1.bt_ == object2.bt_;
              if( c ){
                c = object1.et_ == object2.et_;
                if( c ){
                  c = memcmp(&object1.srcAddr_,&object2.srcAddr_,sizeof(&object1.srcAddr_)) == 0;
                  if( c ){
                    c = memcmp(&object1.dstAddr_,&object2.dstAddr_,sizeof(&object1.dstAddr_)) == 0;
                    if( c ){
                      c = object1.srcPort_ == object2.srcPort_;
                      if( c ){
                        c = object1.dstPort_ == object2.dstPort_;
                        if( c ){
                          c = object1.proto_ == object2.proto_;
                        }
                      }
                    }
                  }
                }
	            }
              return c;
            }
            mutable EmbeddedHashNode<CachedPacketSum,uintptr_t> keyNode_;

            static EmbeddedListNode<CachedPacketSum> & listNode(const CachedPacketSum & object){
              return object.listNode_;
            }
            static CachedPacketSum & listNodeObject(const EmbeddedListNode<CachedPacketSum> & node,CachedPacketSum * p = NULL){
              return node.object(p->listNode_);
            }
            mutable EmbeddedListNode<CachedPacketSum> listNode_;

            AutoPtr<Table<Mutant> > table_;
    	      uint64_t bt_;
    	      uint64_t et_;
            uintmax_t pktSize_;
            uintmax_t dataSize_;
            struct in_addr srcAddr_;
            struct in_addr dstAddr_;
            uint16_t srcPort_;
            uint16_t dstPort_;
            int16_t proto_;
        };
        typedef EmbeddedHash<
          CachedPacketSum,
          uintptr_t,
          uintptr_t,
          CachedPacketSum::ehNLT,
          CachedPacketSum::ehLTN,
          CachedPacketSum::keyNode,
          CachedPacketSum::keyNodeObject,
          CachedPacketSum::keyNodeHash,
          CachedPacketSum::keyHashNodeEqu
        > CachedPacketSumHash;
        CachedPacketSumHash cachedPacketSumHash_;
        typedef EmbeddedList<
          CachedPacketSum,
          CachedPacketSum::listNode,
          CachedPacketSum::listNodeObject
        > CachedPacketSumLRU;
        CachedPacketSumLRU cachedPacketSumLRU_;
        AutoDrop<CachedPacketSumLRU> cachedPacketSumLRUAutoDrop_;
        uintptr_t cachedPacketSumSize_;
        uintmax_t cachedPacketSumHitCount_;
        uintmax_t cachedPacketSumMissCount_;

        Logger * logger_;
        utf8::String section_;
        utf8::String sectionName_;
        utf8::String htmlDir_;
        int64_t ellapsed_;
        uintmax_t minSignificantThreshold_;
        struct tm curTime_;
      	struct tm cgiBT_;
	      struct tm cgiET_;
        utf8::String filter_;
        utf8::String filterHash_;
        uintptr_t stage_;
        bool resolveDNSNames_;
        bool bidirectional_;
        bool protocols_;
        bool ports_;
        bool refreshOnlyCurrent_;

        AutoPtr<Database> database_;
        AutoPtr<Statement> statement_;
        AutoPtr<Statement> stFileStat_[3];
        AutoPtr<Statement> stBPFTSel_[PCAP::pgpCount];
        AutoPtr<Statement> stBPFTHostSel_[PCAP::pgpCount];
        AutoPtr<Statement> stBPFTIns_;
        AutoPtr<Statement> stBPFTIns2_;
        AutoPtr<Statement> stBPFTCacheSel_;
        AutoPtr<Statement> stBPFTCacheIns2_;
        AutoPtr<Statement> stBPFTCacheDel_;
        AutoPtr<Statement> stBPFTCacheHostSel_;

        AutoPtr<Database> dbtrUpdate_;
	      AutoPtr<Statement> stBPFTCacheSelForUpdate_;
        AutoPtr<Statement> stBPFTCacheIns_;
	      AutoPtr<Statement> stBPFTCacheUpd_;
        AutoPtr<Statement> stDNSCache_[3];

        PCAP::PacketGroupingPeriod groupingPeriod_;
        PCAP::PacketGroupingPeriod totalsPeriod_;
        intptr_t maxTotalsLevel_, minTotalsLevel_;
        Vector<Table<Mutant> > table_;

        void parseBPFTLogFile();

        enum { rlYear, rlMon, rlDay, rlHour, rlMin, rlSec, rlNone, rlCount };

        static PCAP::PacketGroupingPeriod rl2pgp(intptr_t rl);

        void writeBPFTHtmlReportHelper1(
          intptr_t i,
          intptr_t level,
          struct tm & beginTime,
          struct tm & endTime,
          struct tm & btt,
          struct tm & ett,
          struct tm & bta,
          struct tm & eta,
          uintmax_t & sum1,
          uintmax_t & sum2
        );
        void writeBPFTHtmlReport(intptr_t level = rlYear,const struct tm * rt = NULL);
        void getBPFTCached(Statement * pStatement,Table<Mutant> * pResult,uintmax_t * pDgramBytes = NULL,uintmax_t * pDataBytes = NULL);
        utf8::String genHRef(const in_addr & ip,uintptr_t port);
      private:
    };
    friend class BPFTThread;
    
    Vector<Thread> threads_;
    
    static void fallBackToNewLine(AsyncFile & f);
    static Mutant timeStampRoundToMin(uint64_t ts);
    void printStat(int64_t lineNo,int64_t spos,int64_t pos,int64_t size,int64_t cl,int64_t * tma = NULL);
    static int64_t fetchLogFileLastOffset(AutoPtr<Statement> st[3],const utf8::String & logFileName);
    static void updateLogFileLastOffset(AutoPtr<Statement> st[3],const utf8::String & logFileName,int64_t offset);

    static void writeHtmlHead(AsyncFile & f);
    static void writeHtmlTail(AsyncFile & f,int64_t ellapsed);
    static void writeTraf(AsyncFile & f,uint64_t qi,uint64_t qj);
    utf8::String resolveAddr(AutoPtr<Statement> st[3],bool resolveDNSNames,const struct in_addr & ip4,bool numeric = false);
    utf8::String getDecor(const utf8::String & dname,const utf8::String & section);
    static utf8::String getIPFilter(const utf8::String & text,const utf8::String & tableName = utf8::String());
    static bool isCurrentTimeInterval(const struct tm & curTime,const struct tm bt,const struct tm et);
    int32_t doWork(uintptr_t stage);
    int32_t waitThreads();
    Logger & createDatabase();
    Sniffer * getSnifferBySection(const utf8::String & sectionName);
    static void enumInterfaces(Statement * statement,Array<utf8::String> & ifaces);
    Logger & writeCGIInterfaceAndTimeSelect(bool addUnionIf);
    void reactivateIndices(bool reactivate,bool setStat);
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class SnifferService : public Service {
  public:
    Logger * logger_;

    virtual ~SnifferService() {}
    SnifferService();
  protected:
  private:
    void install();
    void uninstall();
    void start();
    void stop();
    bool active();
};
//------------------------------------------------------------------------------
} // namespace macroscope
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
