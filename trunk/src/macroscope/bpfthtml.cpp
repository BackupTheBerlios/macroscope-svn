/*-
 * Copyright 2007-2008 Guram Dukashvili
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
#include <adicpp/adicpp.h>
//------------------------------------------------------------------------------
#include "sniffer.h"
#include "macroscope.h"
#include "bpft.h"
//------------------------------------------------------------------------------
namespace macroscope {
//------------------------------------------------------------------------------
//Data URL  
//The  data url format allows the value of a small object to be encoded inline as base64 content. This looks like this:
//<IMG SRC="data:image/gif;base64,[...]">
// where the [...] is replaced with the base64 encoded image data. Since the content of an html element is limited,
//  in worst case, to 1024 bytes, only fairly small images could be displayed this way. (I'm not sure where my memory of this limit comes from.)
//------------------------------------------------------------------------------
static inline bool isAddressMask(uint32_t ip)
{
  return (ip & 0x80000000) != 0;
}
//------------------------------------------------------------------------------
static inline bool isMulticastAddress(uint32_t ip)
{
  return ((ip & 0xf0000000) == 0xe0000000);
}
//------------------------------------------------------------------------------
static inline bool isSiteLocalAddress(uint32_t ip)
{
// refer to RFC 1918
// 10/8 prefix
// 172.16/12 prefix
// 192.168/16 prefix
  return (((ip >> 24) & 0xFF) == 10)
      || ((((ip >> 24) & 0xFF) == 172)
      && (((ip >> 16) & 0xF0) == 16))
      || ((((ip >> 24) & 0xFF) == 192)
      && (((ip >> 16) & 0xFF) == 168));
}
//------------------------------------------------------------------------------
static inline bool isMCGlobal(uint32_t ip)
{
  union {
    uint8_t byteAddr[4];
    uint32_t a;
  };
  a = ip;
// 224.0.1.0 to 238.255.255.255
  return ((byteAddr[0] & 0xff) >= 224 && (byteAddr[0] & 0xff) <= 238 ) &&
        !((byteAddr[0] & 0xff) == 224 && byteAddr[1] == 0 && byteAddr[2] == 0);
}
//------------------------------------------------------------------------------
static inline bool isMCLinkLocal(uint32_t ip)
{
// 224.0.0/24 prefix and ttl == 1
  return (((ip >> 24) & 0xFF) == 224)
      && (((ip >> 16) & 0xFF) == 0)
      && (((ip >> 8) & 0xFF) == 0);
}
//------------------------------------------------------------------------------
utf8::String Logger::getIPFilter(const utf8::String & text,const utf8::String & tableName)
{
  utf8::String filter;
  utf8::String::Iterator i(text);
  uintptr_t c[6], prev = '\0';
  for(;;){
    if( i.eos() ) break;
    c[0] = i.getLowerChar();
    c[1] = (i + 1).getLowerChar();
    c[2] = (i + 2).getLowerChar();
    c[3] = (i + 3).getLowerChar();
    c[4] = (i + 4).getLowerChar();
    c[5] = (i + 5).getLowerChar();
    if( (prev == '\0' || prev == '(' || prev == ')' || (utf8::getC1Type(prev) & C1_SPACE) != 0) && (
        (c[0] == 's' && c[1] == 'r' && c[2] == 'c' && (c[3] == '_' || utf8::getC1Type(c[3]) & C1_SPACE) != 0) ||
        (c[0] == 'd' && c[1] == 's' && c[2] == 't' && (c[3] == '_' || utf8::getC1Type(c[3]) & C1_SPACE) != 0) ||
        (c[0] == 'p' && c[1] == 'r' && c[2] == 'o' && c[3] == 't' && c[4] == 'o' && (utf8::getC1Type(c[5]) & C1_SPACE) != 0)) ){
      utf8::String::Iterator shift(i);
      while( shift.next() && !shift.isSpace() );
      utf8::String sd(utf8::String(i,shift));
      i = shift;
      while( i.next() && i.isSpace() );
      if( (i - 1).isSpace() ){
        ksock::SockAddr addr;
        if( sd.casecompare("src") == 0 || sd.casecompare("dst") == 0 ){
          sd = tableName + sd.lower() + "_ip";
          utf8::String::Iterator j(i), net(j);
          bool isNetwork = false;
          while( !j.eos() && !j.isSpace() && j.getChar() != ')' ){
            if( j.getChar() == '/' ){
              isNetwork = true;
  	          net = j;
	          }
	          j.next();
          }
          if( isNetwork ){
            union {
	            struct in_addr ineta;
	            uint32_t inet;
            };
	          ineta = addr.resolveName(utf8::String(i,net)).addr4_.sin_addr;
	          uint32_t mask = ~(~uint32_t(0) << utf8::str2Int(utf8::String(net + 1,j)));
	          inet &= mask;
            struct in_addr inet1 = ineta;
	          ineta = addr.addr4_.sin_addr;
	          inet |= ~mask;
            struct in_addr inet2 = ineta;
            filter += "(" + sd + " >= '" +
              ksock::SockAddr::addr2Index(inet1) + "' AND " + sd + " <= '" +
	            ksock::SockAddr::addr2Index(inet2) + "')"
	          ;
          }
          else {
            filter += sd + " = '" +
	            ksock::SockAddr::addr2Index(addr.resolveName(utf8::String(i,j)).addr4_.sin_addr) +
	             "'"
	          ;
          }
          i = j;
	      }
        else if( sd.casecompare("src_port") == 0 || sd.casecompare("dst_port") == 0 ){
          sd = tableName + sd.lower();
          utf8::String::Iterator j(i);
          while( j.next() && !j.isSpace() && j.getChar() != ')' );
          filter += sd + " = " +
	          utf8::int2Str(addr.resolveName(":" + utf8::String(i,j)).addr4_.sin_port)
	        ;
          i = j;
	      }
        else if( sd.casecompare("proto") == 0 ){
          sd = tableName + "ip_proto";
          utf8::String::Iterator j(i);
          while( j.next() && !j.isSpace() && j.getChar() != ')' );
          filter += sd + " = " + utf8::int2Str(ksock::SockAddr::stringAsProto(utf8::String(i,j)));
          i = j;
	      }
      }
      prev = (i - 1).getChar();
    }
    else {
      filter += utf8::String(i,i + 1);
      prev = i.getChar();
      i++;
    }
  }
  if( !filter.isNull() ) filter = " (" + filter + ") ";
  return filter;
}
//------------------------------------------------------------------------------
utf8::String Logger::resolveAddr(AutoPtr<Statement> st[3],bool resolveDNSNames,const struct in_addr & ip4,bool numeric)
{
  AutoPtr<DNSCacheEntry> addr(newObject<DNSCacheEntry>());
  addr->addr4_.sin_addr = ip4;
  addr->addr4_.sin_port = 0;
  addr->addr4_.sin_family = PF_INET;
  if( numeric || !resolveDNSNames ) return addr->resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV);
  AutoLock<InterlockedMutex> lock(dnsMutex_);
  DNSCacheEntry * pAddr = addr;
  dnsCache_.insert(addr,false,false,&pAddr);
  if( pAddr == addr ){
    utf8::String name;
    st[stSel]->database()->start();
    if( !st[stSel]->prepared() )
      st[stSel]->text("SELECT st_name from INET_DNS_CACHE WHERE st_ip = :ip")->prepare();
    if( st[stSel]->paramAsString("ip",ksock::SockAddr::addr2Index(ip4))->execute()->fetch() ){
      st[stSel]->fetchAll();
      name = st[stSel]->valueAsString("st_name");
      dnsCacheHitCount_++;
    }
    else {
      name = pAddr->resolveAddr();
      if( !st[stIns]->prepared() )
        st[stIns]->text("INSERT INTO INET_DNS_CACHE (st_ip,st_name) VALUES (:ip,:name)")->prepare();
      st[stIns]->
        paramAsString("ip",ksock::SockAddr::addr2Index(ip4))->
	      paramAsString("name",name);
      try {
        st[stIns]->execute();
      }
      catch( ExceptionSP & e ){
        if( !e->searchCode(isc_no_dup,ER_DUP_KEY,ER_DUP_ENTRY) ) throw;
      }
      dnsCacheMissCount_++;
    }
    st[stSel]->database()->commit();
    pAddr->name_ = name;
    addr.ptr(NULL);
    if( dnsCacheSize_ > 0 && dnsCache_.count() >= dnsCacheSize_ )
      dnsCache_.drop(dnsCacheLRU_.remove(*dnsCacheLRU_.last()));
  }
  else {
    dnsCacheLRU_.remove(*pAddr);
    dnsCacheHitCount_++;
  }
  dnsCacheLRU_.insToHead(*pAddr);
  return pAddr->name_;
}
//------------------------------------------------------------------------------
utf8::String Logger::formatTraf(uintmax_t traf,uintmax_t allTraf)
{
  return printTraffic(traf,allTraf);
}
//------------------------------------------------------------------------------
utf8::String Logger::getDecor(const utf8::String & dname,const utf8::String & section)
{
  utf8::String defDecor(config_->textByPath(section + "..decoration.colors." + dname));
  return config_->textByPath(section + ".html_report.decoration.colors." + dname,defDecor);
}
//------------------------------------------------------------------------------
bool Logger::isCurrentTimeInterval(const struct tm & curTime,const struct tm bt,const struct tm et)
{
  struct tm curTimeBTHour = curTime;
  curTimeBTHour.tm_min = 0;
  curTimeBTHour.tm_sec = 0;
  struct tm curTimeETHour = curTime;
  curTimeETHour.tm_min = 59;
  curTimeETHour.tm_sec = 59;
  struct tm curTimeBTDay = curTimeBTHour;
  curTimeBTDay.tm_hour = 0;
  struct tm curTimeETDay = curTimeETHour;
  curTimeETDay.tm_hour = 23;
  struct tm curTimeBTMon = curTimeBTDay;
  curTimeBTMon.tm_mday = 1;
  struct tm curTimeETMon = curTimeETDay;
  curTimeETMon.tm_mday = (int) monthDays(curTimeETMon.tm_year + 1900,curTimeETMon.tm_mon);
  struct tm curTimeBTYear = curTimeBTMon;
  curTimeBTYear.tm_mon = 0;
  struct tm curTimeETYear = curTimeETMon;
  curTimeETYear.tm_mday = 31;
  curTimeETYear.tm_mon = 11;
  int64_t bt0 = tm2Time(bt), et0 = tm2Time(et);
  return
    (
      bt.tm_year == et.tm_year && bt.tm_mon == et.tm_mon && bt.tm_mday == et.tm_mday && bt.tm_hour == et.tm_hour &&
      bt0 == tm2Time(curTimeBTHour) && et0 == tm2Time(curTimeETHour)
    ) ||
    (
      bt.tm_year == et.tm_year && bt.tm_mon == et.tm_mon && bt.tm_mday == et.tm_mday &&
      bt0 == tm2Time(curTimeBTDay) && et0 == tm2Time(curTimeETDay)
    ) ||
    (
      bt.tm_year == et.tm_year && bt.tm_mon == et.tm_mon &&
      bt0 == tm2Time(curTimeBTMon) && et0 == tm2Time(curTimeETMon)
    ) ||
    (
      bt.tm_year == et.tm_year &&
      bt0 == tm2Time(curTimeBTYear) && et0 == tm2Time(curTimeETYear)
    )
  ;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Logger::BPFTThread::~BPFTThread()
{
}
//------------------------------------------------------------------------------
Logger::BPFTThread::BPFTThread() : cachedPacketSumLRUAutoDrop_(cachedPacketSumLRU_)
{
}
//------------------------------------------------------------------------------
Logger::BPFTThread::BPFTThread(
  Logger & logger,const utf8::String & section,const utf8::String & sectionName,uintptr_t stage) :
  cachedPacketSumLRUAutoDrop_(cachedPacketSumLRU_),
  cachedPacketSumSize_(0),
  cachedPacketSumHitCount_(0),
  cachedPacketSumMissCount_(0),
  logger_(&logger),
  section_(section),
  sectionName_(sectionName),
  stage_(stage)
{
  ConfigSection dbParamsSection;
  dbParamsSection.addSection(logger_->config_->sectionByPath(logger_->connection_));
  
  database_ = Database::newDatabase(&dbParamsSection);

  statement_ = database_->newAttachedStatement();
  stBPFTIns_ = database_->newAttachedStatement();
  stBPFTIns2_ = database_->newAttachedStatement();
  stBPFTCacheSel_ = database_->newAttachedStatement();
  stBPFTCacheIns2_ = database_->newAttachedStatement();
  stBPFTCacheDel_ = database_->newAttachedStatement();
  stBPFTCacheHostSel_ = database_->newAttachedStatement();
						         
  stFileStat_[stSel] = database_->newAttachedStatement();
  stFileStat_[stIns] = database_->newAttachedStatement();
  stFileStat_[stUpd] = database_->newAttachedStatement();
  
  dbtrUpdate_ = Database::newDatabase(&dbParamsSection);
  
  stBPFTCacheSelForUpdate_ = dbtrUpdate_->newAttachedStatement();
  stBPFTCacheIns_ = dbtrUpdate_->newAttachedStatement();
  stBPFTCacheUpd_ = dbtrUpdate_->newAttachedStatement();
  
  stDNSCache_[stSel] = dbtrUpdate_->newAttachedStatement();
  stDNSCache_[stIns] = dbtrUpdate_->newAttachedStatement();
  stDNSCache_[stUpd] = dbtrUpdate_->newAttachedStatement();
}
//------------------------------------------------------------------------------
void Logger::BPFTThread::threadExecute()
{
  ksock::APIAutoInitializer ksockAPIAutoInitializer;
  AutoDatabaseDetach autoDatabaseDetach(database_);
  AutoDatabaseDetach autoDBTRUpdateDetach(dbtrUpdate_);
  switch( stage_ ){
    case 0 :
      if( !logger_->cgi_.isCGI() ) parseBPFTLogFile();
      break;
    case 1 :
      writeBPFTHtmlReport();
      break;
    default : assert( 0 );
  }
}
//------------------------------------------------------------------------------
void Logger::BPFTThread::getBPFTCached(Statement * pStatement,Table<Mutant> * pResult,uintmax_t * pDgramBytes,uintmax_t * pDataBytes)
{
  CachedPacketSum * p;
  AutoPtr<CachedPacketSum> pktSum;
  if( pResult == NULL ){
    pktSum = p = newObject<CachedPacketSum>();
    p->bt_ = pStatement->paramAsMutant("BTT");
    p->et_ = pStatement->paramAsMutant("ETT");
    p->srcAddr_ = ksock::SockAddr::indexToAddr4(pStatement->paramAsString("src"));
    p->dstAddr_ = ksock::SockAddr::indexToAddr4(pStatement->paramAsString("dst"));
    p->srcPort_ = ports_ ? (uint16_t) pStatement->paramAsMutant("src_port") : 0;
    p->dstPort_ = ports_ ? (uint16_t) pStatement->paramAsMutant("dst_port") : 0;
    p->proto_ = protocols_ ? (int16_t) pStatement->paramAsMutant("proto") : -1;
    cachedPacketSumHash_.insert(*p,false,false,&p);
    if( p == pktSum ){
      pStatement->execute()->fetchAll();
      p->pktSize_ = pStatement->sum("SUM1");
      p->dataSize_ = pStatement->sum("SUM2");
      pktSum.ptr(NULL);
      if( cachedPacketSumSize_ > 0 && cachedPacketSumLRU_.count() * sizeof(CachedPacketSum) > cachedPacketSumSize_ )
        cachedPacketSumLRU_.drop(*cachedPacketSumLRU_.last());
      cachedPacketSumMissCount_++;
    }
    else {
      cachedPacketSumLRU_.remove(*p);
      cachedPacketSumHitCount_++;
    }
    cachedPacketSumLRU_.insToHead(*p);
    *pDgramBytes = p->pktSize_;
    *pDataBytes = p->dataSize_;
  }
  else {
    intptr_t threshold = -1, row, i;
    pStatement->execute()->fetchAll()->unloadColumns(*pResult);
    for( i = pStatement->rowCount() - 1; i >= 0; i-- ){
      pStatement->selectRow(i);

      row = pResult->rowCount();
      pStatement->unloadRowByIndex(pResult->addRow());

hash:
      pktSum = p = newObject<CachedPacketSum>();
      p->bt_ = pStatement->paramAsMutant("BTT");
      p->et_ = pStatement->paramAsMutant("ETT");
      p->srcAddr_ = ksock::SockAddr::indexToAddr4(pResult->cell(row,"src_ip"));
      p->dstAddr_ = ksock::SockAddr::indexToAddr4(pResult->cell(row,"dst_ip"));
      p->srcPort_ = ports_ ? (uint16_t) pResult->cell(row,"src_port") : 0;
      p->dstPort_ = ports_ ? (uint16_t) pResult->cell(row,"dst_port") : 0;
      p->proto_ = protocols_ ? (int16_t) pResult->cell(row,"ip_proto") : -1;
      p->pktSize_ = pResult->cell(row,"SUM1");
      p->dataSize_ = pResult->cell(row,"SUM2");

      cachedPacketSumHash_.insert(*p,false,false,&p);
      if( p == pktSum ){
        pktSum.ptr(NULL);
        if( cachedPacketSumSize_ > 0 && cachedPacketSumLRU_.count() * sizeof(CachedPacketSum) > cachedPacketSumSize_ )
          cachedPacketSumLRU_.drop(*cachedPacketSumLRU_.last());
        cachedPacketSumMissCount_++;
      }
      else {
        cachedPacketSumLRU_.remove(*p);
        cachedPacketSumHitCount_++;
      }
      cachedPacketSumLRU_.insToHead(*p);
    }
    pResult->sort("SUM1");
    if( threshold < 0 ){
      for( intptr_t k = pResult->rowCount() - 1; k >= 0; k-- )
        if( (uintmax_t) pResult->cell(k,"SUM1") < minSignificantThreshold_ ){
          threshold = k;
          break;
        }
    }
    if( threshold >= 0 && i == -1 ){
      struct in_addr baddr;
      baddr.s_addr = INADDR_BROADCAST;
      pResult->insertRow(threshold + 1);
      pResult->cell(threshold + 1,"src_ip") = ksock::SockAddr::addr2Index(baddr);
      pResult->cell(threshold + 1,"dst_ip") = ksock::SockAddr::addr2Index(baddr);
      if( ports_ ){
        pResult->cell(threshold + 1,"src_port") = 0;
        pResult->cell(threshold + 1,"src_port") = 0;
      }
      if( protocols_ )
        pResult->cell(threshold + 1,"ip_proto") = -1;
      pResult->cell(threshold + 1,"SUM1") = pResult->sum("SUM1",0,threshold);
      pResult->cell(threshold + 1,"SUM2") = pResult->sum("SUM2",0,threshold);
      pResult->removeRows(0,threshold);
      //for( intptr_t k = pResult->rowCount() - 1; k >= 0; k-- ){
      //  const char * p = utf8::String(pResult->cell(k,"st_src_ip")).c_str();
      //  uintmax_t b = pResult->cell(k,"SUM1");
      //  p = p;
      //}
      row = 0;
      goto hash;
    }
  }
}
//------------------------------------------------------------------------------
utf8::String Logger::BPFTThread::genHRef(const in_addr & ip,uintptr_t port)
{
  utf8::String name(logger_->resolveAddr(stDNSCache_,resolveDNSNames_,ip));
  utf8::String addr(logger_->resolveAddr(stDNSCache_,resolveDNSNames_,ip,true));
  return
    "    <A HREF=\"http://" + name + "\">\n" + name +
    (name.compare(addr) == 0 ? utf8::String() : " (" + addr + ")") +
    (port > 0 ? ":" + utf8::int2Str(port) : utf8::String()) +
    "\n    </A>\n"
  ;
}
//------------------------------------------------------------------------------
PCAP::PacketGroupingPeriod Logger::BPFTThread::rl2pgp(intptr_t rl)
{ 
  switch( rl ){
    case rlYear : return PCAP::pgpYear;
    case rlMon  : return PCAP::pgpMon;
    case rlDay  : return PCAP::pgpDay;
    case rlHour : return PCAP::pgpHour;
    case rlMin  : return PCAP::pgpMin;
    case rlSec  : return PCAP::pgpSec;
    case rlNone : return PCAP::pgpNone;
  }
  return PCAP::pgpNone;
}
//------------------------------------------------------------------------------
void Logger::BPFTThread::writeBPFTHtmlReportHelper1(
  intptr_t i,
  intptr_t level,
  struct tm & beginTime,
  struct tm & endTime,
  struct tm & btt,
  struct tm & ett,
  struct tm & bta,
  struct tm & eta,
  uintmax_t & sum1,
  uintmax_t & sum2)
{
  Vector<Table<Mutant> > & table = table_;
  btt = time2tm(tm2Time(beginTime) - (level + 1 >= rlHour ? getgmtoffset() : 0));
  ett = time2tm(tm2Time(endTime) - (level + 1 >= rlHour ? getgmtoffset() : 0));
  Sniffer::setTotalsBounds(rl2pgp(level + 1),btt,ett,bta,eta);
  if( !stBPFTHostSel_[rl2pgp(level + 1)]->prepared() )
    stBPFTHostSel_[rl2pgp(level + 1)]->prepare()->paramAsString("if",sectionName_);
  stBPFTHostSel_[rl2pgp(level + 1)]->
    paramAsMutant("BTT",bta)->paramAsMutant("ETT",eta)->
    paramAsMutant("src",table[0](i,"src_ip"))->
    paramAsMutant("dst",table[0](i,"dst_ip"));
  if( ports_ ){
    stBPFTHostSel_[rl2pgp(level + 1)]->
      paramAsMutant("src_port",table[0](i,"src_port"))->
      paramAsMutant("dst_port",table[0](i,"dst_port"));
  }
  if( protocols_ ){
    stBPFTHostSel_[rl2pgp(level + 1)]->paramAsMutant("proto",table[0](i,"ip_proto"));
  }
  getBPFTCached(stBPFTHostSel_[rl2pgp(level + 1)],NULL,&sum1,&sum2);
}
//------------------------------------------------------------------------------
void Logger::BPFTThread::writeBPFTHtmlReport(intptr_t level,const struct tm * rt)
{
  struct tm beginTime, beginTime2, endTime, endTime2;
  memset(&beginTime,0,sizeof(beginTime));
  Mutant m0, m1, m2;
  AsyncFile f;

  if( level == rlYear ){
    if( !logger_->cgi_.isCGI() && !(bool) logger_->config_->valueByPath(section_ + ".html_report.enabled",false) ) return;
    if( logger_->verbose_ ) fprintf(stderr,"\n");
    ellapsed_ = getlocaltimeofday();
    minSignificantThreshold_ = logger_->config_->valueByPath(section_ + ".html_report.min_significant_threshold",0);
    if( logger_->cgi_.isCGI() )
      minSignificantThreshold_ = logger_->cgi_.paramAsMutant(
        logger_->cgi_.paramAsString("threshold2").isNull() ?
        "threshold" :
        "threshold2"
      );
    filter_ = logger_->config_->textByPath(section_ + ".html_report.filter");
    if( logger_->cgi_.isCGI() )
      filter_ = logger_->cgi_.paramAsString("filter");
    filter_ = getIPFilter(filter_.trim(),"a.");
    filterHash_ = SHA256().sha256AsBase64String(filter_,false);
    curTime_ = time2tm(ellapsed_);
    resolveDNSNames_ = logger_->config_->valueByPath(section_ + ".html_report.resolve_dns_names",true);
    if( logger_->cgi_.isCGI() )
      resolveDNSNames_ = logger_->cgi_.paramAsMutant("resolve");
    bidirectional_ = logger_->config_->valueByPath(section_ + ".html_report.bidirectional",false);
    if( logger_->cgi_.isCGI() )
      bidirectional_ = logger_->cgi_.paramAsMutant("bidirectional");
    protocols_ = logger_->config_->valueByPath(section_ + ".html_report.protocols",false);
    if( logger_->cgi_.isCGI() )
      protocols_ = logger_->cgi_.paramAsMutant("protocols");
    ports_ = logger_->config_->valueByPath(section_ + ".html_report.ports",false);
    if( logger_->cgi_.isCGI() )
      ports_ = logger_->cgi_.paramAsMutant("ports");
    //if( !bidirectional_ ) protocols_ = ports_ = false;
    groupingPeriod_ = PCAP::stringToGroupingPeriod(logger_->config_->valueByPath(section_ + ".sniffer.grouping_period","day"));
    totalsPeriod_ = PCAP::stringToGroupingPeriod(logger_->config_->valueByPath(section_ + ".sniffer.totals_period","day"));
    database_->isolation("SERIALIZABLE")->start();
    if( !logger_->cgi_.isCGI() ) Sniffer::getTrafficPeriod(statement_,sectionName_,beginTime,endTime);
    if( bidirectional_ ){
      utf8::String templ1 =
        "  SELECT\n"
        "    a.src_ip, a.dst_ip,\n" +
        utf8::String(ports_ ? "a.src_port, a.dst_port,\n" : "") +
        utf8::String(protocols_ ? " a.ip_proto,\n" : "") +
        "    a.dgram AS sum1,\n"
        "    a.data AS sum2\n"
        "  FROM\n"
        "      INET_SNIFFER_STAT_@@0002@@ a\n"
        "  WHERE\n"
        "      a.iface = :if\n"
        "      @@0001@@\n"
      ;
      utf8::String templ2(
        "  AND a.src_ip = :src\n" +
        utf8::String(ports_ ? " AND a.src_port = :src_port\n" : "") +
        "  AND a.dst_ip = :dst\n" +
        utf8::String(ports_ ? " AND a.dst_port = :dst_port\n" : "") +
        utf8::String(protocols_ ? " AND a.ip_proto = :proto \n" : "")
      );
      for( intptr_t i = PCAP::pgpCount - 1; i >= 0; i-- ){
        if( stBPFTSel_[i] == NULL ) stBPFTSel_[i] = database_->newAttachedStatement();
        stBPFTSel_[i]->text(templ1.replaceAll("@@0001@@"," AND a.ts >= :BTT AND a.ts < :ETT\n" +
          (filter_.isNull() ? utf8::String() : " AND " + filter_)).replaceAll("@@0002@@",Sniffer::pgpNames[i]) +
          utf8::String(ports_ ? " AND a.src_port <> 0\n" : " AND a.src_port = 0\n") +
          utf8::String(ports_ ? " AND a.dst_port <> 0\n" : " AND a.src_port = 0\n") +
          utf8::String(protocols_ ? " AND a.ip_proto >= 0\n" : " AND a.ip_proto < 0\n")
        );

        if( stBPFTHostSel_[i] == NULL ) stBPFTHostSel_[i] = database_->newAttachedStatement();
        stBPFTHostSel_[i]->text(templ1.replaceAll("@@0001@@",
          " AND a.ts >= :BTT AND a.ts < :ETT\n" + templ2).replaceAll("@@0002@@",Sniffer::pgpNames[i])
        );
      }
    }
    else {
      utf8::String templ1 =
        "SELECT\n"
        "  a.src_ip, a.dst_ip,\n" +
        utf8::String(ports_ ? "a.src_port, a.dst_port,\n" : "") +
        utf8::String(protocols_ ? " a.ip_proto,\n" : "") +
        "  sum(a.dgram) AS sum1,\n"
        "  sum(a.data) AS sum2\n"
        "FROM (\n"
        "  SELECT\n"
        "    a.src_ip AS src_ip, a.src_ip AS dst_ip,\n" +
        utf8::String(ports_ ? "a.src_port AS src_port, a.src_port AS dst_port,\n" : "") +
        utf8::String(protocols_ ? " a.ip_proto,\n" : "") +
        "    a.dgram,\n"
        "    a.data\n"
        "  FROM\n"
        "      INET_SNIFFER_STAT_@@0002@@ a\n"
        "  WHERE\n"
        "      a.iface = :if\n"
        "      @@0001@@\n"
        "  UNION ALL\n"
        "  SELECT\n"
        "    a.dst_ip AS src_ip, a.dst_ip AS dst_ip,\n" +
        utf8::String(ports_ ? "a.dst_port AS src_port, a.dst_port AS dst_port,\n" : "") +
        utf8::String(protocols_ ? " a.ip_proto,\n" : "") +
        "    a.dgram,\n"
        "    a.data\n"
        "  FROM\n"
        "      INET_SNIFFER_STAT_@@0002@@ a\n"
        "  WHERE\n"
        "      a.iface = :if\n"
        "      @@0001@@\n"
        ") a\n"
        "GROUP BY\n"
        "  a.src_ip, a.dst_ip\n" +
        utf8::String(ports_ ? ", a.src_port, a.dst_port\n" : "") +
        utf8::String(protocols_ ? ", a.ip_proto\n" : "")
      ;
      utf8::String templ2(
        "  AND a.src_ip = :src\n" +
        utf8::String(ports_ ? " AND a.src_port = :src_port\n" : "") +
        "  AND a.dst_ip = :dst\n" +
        utf8::String(ports_ ? " AND a.dst_port = :dst_port\n" : "") +
        utf8::String(protocols_ ? " AND a.ip_proto = :proto\n" : "")
      );
      for( intptr_t i = PCAP::pgpCount - 1; i >= 0; i-- ){
        if( stBPFTSel_[i] == NULL ) stBPFTSel_[i] = database_->newAttachedStatement();
        stBPFTSel_[i]->text(templ1.replaceAll("@@0001@@"," AND a.ts >= :BTT AND a.ts < :ETT\n" +
          (filter_.isNull() ? utf8::String() : " AND " + filter_) +
          utf8::String(ports_ ? " AND a.src_port <> 0\n" : " AND a.src_port = 0\n") +
          utf8::String(ports_ ? " AND a.dst_port <> 0\n" : " AND a.src_port = 0\n") +
          utf8::String(protocols_ ? " AND a.ip_proto >= 0\n" : " AND a.ip_proto < 0\n")
          ).replaceAll("@@0002@@",Sniffer::pgpNames[i])
        );

        if( stBPFTHostSel_[i] == NULL ) stBPFTHostSel_[i] = database_->newAttachedStatement();
        stBPFTHostSel_[i]->text(templ1.replaceAll("@@0001@@",
          " AND a.ts >= :BTT AND a.ts < :ETT\n" + templ2).replaceAll("@@0002@@",Sniffer::pgpNames[i])
        );
      }
    }
    htmlDir_ = excludeTrailingPathDelimiter(logger_->config_->valueByPath(section_ + ".html_report.directory"));
    if( logger_->cgi_.isCGI() ){
      memset(&cgiBT_,0,sizeof(cgiBT_));
      cgiBT_.tm_year = (int) logger_->cgi_.paramAsMutant("byear") - 1900;
      cgiBT_.tm_mon = (int) logger_->cgi_.paramAsMutant("bmon") - 1;
      cgiBT_.tm_mday = (int) logger_->cgi_.paramAsMutant("bday");
      if( cgiBT_.tm_mday < 1 || cgiBT_.tm_mday > (int) monthDays(cgiBT_.tm_year + 1900,cgiBT_.tm_mon) )
        cgiBT_.tm_mday = (int) monthDays(cgiET_.tm_year + 1900,cgiET_.tm_mon);
      cgiBT_.tm_hour = (int) logger_->cgi_.paramAsMutant("bhour");
      cgiBT_.tm_min = (int) logger_->cgi_.paramAsMutant("bmin");
      cgiBT_.tm_sec = 0;
      memset(&cgiET_,0,sizeof(cgiET_));
      cgiET_.tm_year = (int) logger_->cgi_.paramAsMutant("eyear") - 1900;
      cgiET_.tm_mon = (int) logger_->cgi_.paramAsMutant("emon") - 1;
      cgiET_.tm_mday = (int) logger_->cgi_.paramAsMutant("eday");
      if( cgiET_.tm_mday < 1 || cgiET_.tm_mday > (int) monthDays(cgiET_.tm_year + 1900,cgiET_.tm_mon) )
        cgiET_.tm_mday = (int) monthDays(cgiET_.tm_year + 1900,cgiET_.tm_mon);
      cgiET_.tm_hour = (int) logger_->cgi_.paramAsMutant("ehour");
      cgiET_.tm_min = (int) logger_->cgi_.paramAsMutant("emin");
      cgiET_.tm_sec = 59;
      if( tm2Time(cgiBT_) > tm2Time(cgiET_) ) ksys::xchg(cgiBT_,cgiET_);
      //if( tm2Time(cgiBT_) < tm2Time(beginTime) ) cgiBT_ = beginTime;
      //if( tm2Time(cgiET_) > tm2Time(endTime) ) cgiET_ = endTime;
      beginTime = cgiBT_;
      endTime = cgiET_;

      utf8::String maxTotals(logger_->cgi_.paramAsString("max_totals"));
      utf8::String minTotals(logger_->cgi_.paramAsString("min_totals"));

      if( maxTotals.casecompare("Min") == 0 ) maxTotalsLevel_ = rlMin;
      else
      if( maxTotals.casecompare("Hour") == 0 ) maxTotalsLevel_ = rlHour;
      else
      if( maxTotals.casecompare("Day") == 0 ) maxTotalsLevel_ = rlDay;
      else
      if( maxTotals.casecompare("Mon") == 0 ) maxTotalsLevel_ = rlMon;
      else
      if( maxTotals.casecompare("Year") == 0 ) maxTotalsLevel_ = rlYear;

      if( minTotals.casecompare("Min") == 0 ) minTotalsLevel_ = rlMin;
      else
      if( minTotals.casecompare("Hour") == 0 ) minTotalsLevel_ = rlHour;
      else
      if( minTotals.casecompare("Day") == 0 ) minTotalsLevel_ = rlDay;
      else
      if( minTotals.casecompare("Mon") == 0 ) minTotalsLevel_ = rlMon;
      else
      if( minTotals.casecompare("Year") == 0 ) minTotalsLevel_ = rlYear;

      if( minTotalsLevel_ < maxTotalsLevel_ ) ksys::xchg(minTotalsLevel_,maxTotalsLevel_);
      level = maxTotalsLevel_;
    }
    refreshOnlyCurrent_ = logger_->config_->valueByPath(section_ + ".html_report.refresh_only_current",false);
  }
  else {
    assert( rt != NULL );
    beginTime = endTime = *rt;
  }
  int * pi, sv, av, fv;
  switch( level ){
    case rlYear :
      beginTime.tm_mon = 0;
      beginTime.tm_mday = 1;
      beginTime.tm_hour = 0;
      beginTime.tm_min = 0;
      beginTime.tm_sec = 0;
      endTime.tm_mon = 11;
      endTime.tm_mday = 31;
      endTime.tm_hour = 23;
      endTime.tm_min = 59;
      endTime.tm_sec = 59;
      pi = &endTime.tm_mon;
      sv = 0;
      av = 1;
      fv = 11;
      if( !logger_->cgi_.isCGI() )
        f.fileName(
          includeTrailingPathDelimiter(htmlDir_) +
          logger_->config_->valueByPath(section_ + ".html_report.index_file_name","index.html")
        );
      break;
    case rlMon :
      beginTime.tm_mon = 0;
      beginTime.tm_mday = 1;
      beginTime.tm_hour = 0;
      beginTime.tm_min = 0;
      beginTime.tm_sec = 0;
      endTime.tm_mon = 11;
      endTime.tm_mday = 31;
      endTime.tm_hour = 23;
      endTime.tm_min = 59;
      endTime.tm_sec = 59;
      pi = &endTime.tm_mday;
      sv = 1;
      av = 0;
      if( !logger_->cgi_.isCGI() )
        f.fileName(
          includeTrailingPathDelimiter(htmlDir_) +
          "bpft-" + sectionName_ +
          utf8::String::print("-traf-by-%04d.html",endTime.tm_year + 1900)
        );
      break;
    case rlDay :
      beginTime.tm_mday = 1;
      beginTime.tm_hour = 0;
      beginTime.tm_min = 0;
      beginTime.tm_sec = 0;
      endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
      endTime.tm_hour = 23;
      endTime.tm_min = 59;
      endTime.tm_sec = 59;
      pi = &endTime.tm_hour;
      sv = 0;
      av = 1;
      if( !logger_->cgi_.isCGI() )
        f.fileName(
          includeTrailingPathDelimiter(htmlDir_) + 
          "bpft-" + sectionName_ +
          utf8::String::print("-traf-by-%04d%02d.html",endTime.tm_year + 1900,endTime.tm_mon + 1)
        );
      break;
    case rlHour :
      beginTime.tm_hour = 0;
      beginTime.tm_min = 0;
      beginTime.tm_sec = 0;
      endTime.tm_hour = 23;
      endTime.tm_min = 59;
      endTime.tm_sec = 59;
      pi = &endTime.tm_min;
      sv = 0;
      av = 1;
      if( !logger_->cgi_.isCGI() )
        f.fileName(
          includeTrailingPathDelimiter(htmlDir_) + 
          "bpft-" + sectionName_ +
          utf8::String::print("-traf-by-%04d%02d%02d.html",endTime.tm_year + 1900,endTime.tm_mon + 1,endTime.tm_mday,endTime.tm_hour)
        );
      break;
    case rlMin :
      beginTime.tm_min = 0;
      beginTime.tm_sec = 0;
      endTime.tm_min = 59;
      endTime.tm_sec = 59;
      pi = &endTime.tm_sec;
      sv = 0;
      av = 1;
      if( !logger_->cgi_.isCGI() )
        f.fileName(
          includeTrailingPathDelimiter(htmlDir_) + 
          "bpft-" + sectionName_ +
          utf8::String::print("-traf-by-%04d%02d%02d%02d.html",endTime.tm_year + 1900,endTime.tm_mon + 1,endTime.tm_mday,endTime.tm_hour,endTime.tm_min)
        );
      break;
    case rlSec :
    case rlNone :
    case rlCount :
      return;
    default :
      pi = NULL;
      sv = av = fv = 0;
  }
  if( logger_->cgi_.isCGI() && level == maxTotalsLevel_ ){
    logger_->cgi_ << utf8::String(); // write content type
    f.fileName("stdout").open();
    logger_->writeHtmlHead(f);
  }
  else {
    f.createIfNotExist(true).open().resize(0);
    m0 = logger_->config_->valueByPath(section_ + ".html_report.directory_mode",0755);
    m1 = logger_->config_->valueByPath(section_ + ".html_report.directory_user",ksys::getuid());
    m2 = logger_->config_->valueByPath(section_ + ".html_report.directory_group",ksys::getgid());
    chModOwn(htmlDir_,m0,m1,m2);
    m0 = logger_->config_->valueByPath(section_ + ".html_report.file_mode",0644);
    m1 = logger_->config_->valueByPath(section_ + ".html_report.file_user",ksys::getuid());
    m2 = logger_->config_->valueByPath(section_ + ".html_report.file_group",ksys::getgid());
    chModOwn(f.fileName(),m0,m1,m2);
    logger_->writeHtmlHead(f);
  }
  Vector<Table<Mutant> > & table = table_;
  while( tm2Time(endTime) >= tm2Time(beginTime) ){
    beginTime2 = beginTime;
    endTime2 = endTime;
    switch( level ){
      case rlSec :
        beginTime.tm_sec = endTime.tm_sec;
      case rlMin :
        beginTime.tm_min = endTime.tm_min;
      case rlHour :
        beginTime.tm_hour = endTime.tm_hour;
      case rlDay :
        beginTime.tm_mday = endTime.tm_mday;
      case rlMon :
        beginTime.tm_mon = endTime.tm_mon;
      case rlYear :
        beginTime.tm_year = endTime.tm_year;
        break;
      default    :
        assert( 0 );
    }
    if( beginTime.tm_mday < 1 || beginTime.tm_mday > (int) monthDays(beginTime.tm_year + 1900,beginTime.tm_mon) )
      beginTime.tm_mday = (int) monthDays(beginTime.tm_year + 1900,beginTime.tm_mon);
    if( logger_->cgi_.isCGI() &&
        (tm2Time(cgiET_) < tm2Time(beginTime) || tm2Time(cgiBT_) > tm2Time(endTime) ||
         level < maxTotalsLevel_ || level > minTotalsLevel_) ){
    }
    else {
      struct tm btt = beginTime, bta, ett = endTime, eta;
      btt = time2tm(tm2Time(btt) - (level >= rlHour ? getgmtoffset() : 0));
      ett = time2tm(tm2Time(ett) - (level >= rlHour ? getgmtoffset() : 0));
      Sniffer::setTotalsBounds(rl2pgp(level),btt,ett,bta,eta);
      if( !stBPFTSel_[rl2pgp(level)]->prepared() )
        stBPFTSel_[rl2pgp(level)]->prepare()->paramAsString("if",sectionName_);
      stBPFTSel_[rl2pgp(level)]->
        paramAsMutant("BTT",bta)->paramAsMutant("ETT",eta);
      switch( level ){
        case rlYear :
          table.resize(13);
          break;
        case rlMon :
          table.resize(32);
          break;
        case rlDay :
          table.resize(25);
          break;
        case rlHour :
          table.resize(61);
          break;
        case rlMin :
          table.resize(61);
          break;
        default    :
          assert( 0 );
      }
      Array<uintmax_t> sums;
      sums.resize(table.count());
      getBPFTCached(stBPFTSel_[rl2pgp(level)],&table[0]);
      if( (sums[0] = table[0].sum("SUM1")) > 0 ){
        if( logger_->verbose_ ) fprintf(stderr,"%s %s\n",
          (const char *) utf8::tm2Str(beginTime).getOEMString(),
          (const char *) utf8::tm2Str(endTime).getOEMString()
        );
        uintptr_t colCount = 0;
        while( *pi >= sv ){
          switch( level ){
            case rlYear :
              beginTime.tm_mon = endTime.tm_mon;
              endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
              break;
            case rlMon :
              beginTime.tm_mday = endTime.tm_mday;
              break;
            case rlDay :
              beginTime.tm_hour = endTime.tm_hour;
              break;
            case rlHour :
              beginTime.tm_min = endTime.tm_min;
              break;
            case rlMin :
              beginTime.tm_sec = endTime.tm_sec;
              break;
            default    :
              assert( 0 );
          }
          btt = time2tm(tm2Time(beginTime) - (level + 1 >= rlHour ? getgmtoffset() : 0));
          ett = time2tm(tm2Time(endTime) - (level + 1 >= rlHour ? getgmtoffset() : 0));
          Sniffer::setTotalsBounds(rl2pgp(level + 1),btt,ett,bta,eta);
          if( !stBPFTSel_[rl2pgp(level + 1)]->prepared() )
            stBPFTSel_[rl2pgp(level + 1)]->prepare()->paramAsString("if",sectionName_);
          stBPFTSel_[rl2pgp(level + 1)]->
            paramAsMutant("BTT",bta)->paramAsMutant("ETT",eta);
          getBPFTCached(stBPFTSel_[rl2pgp(level + 1)],&table[*pi + av]);
          if( (sums[*pi + av] = table[*pi + av].sum("SUM1")) > 0 ) colCount++;
          (*pi)--;
        }
        switch( level ){
          case rlYear :
            endTime.tm_mon = 11;
            endTime.tm_mday = 31;
            break;
          case rlMon :
            endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
            break;
          case rlDay :
            endTime.tm_hour = 23;
            break;
          case rlHour :
            endTime.tm_min = 59;
            break;
          case rlMin :
            endTime.tm_sec = 59;
            break;
          default    :
            assert( 0 );
        }
        if( !f.isOpen() ){
          if( logger_->cgi_.isCGI() ) f.fileName("stdout");
          f.open();
        }
        f <<
          "<TABLE WIDTH=400 BORDER=1 CELLSPACING=0 CELLPADDING=2>\n"
          "<TR>\n"
          "  <TH BGCOLOR=\"" + logger_->getDecor("table_head",section_) + "\" COLSPAN=\"" +
          utf8::int2Str(colCount * 2 + 3) + "\" ALIGN=left nowrap>\n"
        ;
        switch( level ){
          case rlYear :
            f <<
      	      (logger_->cgi_.isCGI() ? utf8::String() :
                "    <A HREF=\"bpft-" + sectionName_ +
                utf8::String::print("-traf-by-%04d.html",endTime.tm_year + 1900) + "\">"
              ) +
              utf8::int2Str(endTime.tm_year + 1900) + "\n" +
	            (logger_->cgi_.isCGI() ? "" : "    </A>\n")
            ;
            break;
          case rlMon :
            f <<
  	          (logger_->cgi_.isCGI() ? utf8::String() :
                "    <A HREF=\"bpft-" + sectionName_ +
                utf8::String::print("-traf-by-%04d%02d.html",endTime.tm_year + 1900,endTime.tm_mon + 1) + "\">"
	            ) +
              utf8::String::print("%02d.%04d",endTime.tm_mon + 1,endTime.tm_year + 1900) + "\n" +
	            (logger_->cgi_.isCGI() ? "" : "    </A>\n")
            ;
            break;
          case rlDay :
            f <<
	            (logger_->cgi_.isCGI() ? utf8::String() :
                "    <A HREF=\"bpft-" + sectionName_ +
                utf8::String::print("-traf-by-%04d%02d%02d.html",endTime.tm_year + 1900,endTime.tm_mon + 1,endTime.tm_mday) + "\">"
	            ) +
              utf8::String::print("%02d.%02d.%04d",endTime.tm_mday,endTime.tm_mon + 1,endTime.tm_year + 1900) + "\n" +
	            (logger_->cgi_.isCGI() ? "" : "    </A>\n")
            ;
            break;
          case rlHour :
            f <<
	            (logger_->cgi_.isCGI() ? utf8::String() :
                "    <A HREF=\"bpft-" + sectionName_ +
                utf8::String::print("-traf-by-%04d%02d%02d%02d.html",endTime.tm_year + 1900,endTime.tm_mon + 1,endTime.tm_mday,endTime.tm_hour) + "\">"
	            ) +
              utf8::String::print("%02d %02d.%02d.%04d",endTime.tm_hour,endTime.tm_mday,endTime.tm_mon + 1,endTime.tm_year + 1900) + "\n" +
	            (logger_->cgi_.isCGI() ? "" : "    </A>\n")
            ;
            break;
          case rlMin :
            f <<
	            (logger_->cgi_.isCGI() ? utf8::String() :
                "    <A HREF=\"bpft-" + sectionName_ +
                utf8::String::print("-traf-by-%04d%02d%02d%02d%02d.html",endTime.tm_year + 1900,endTime.tm_mon + 1,endTime.tm_mday,endTime.tm_hour,endTime.tm_min) + "\">"
	            ) +
              utf8::String::print("%02d:%02d %02d.%02d.%04d",endTime.tm_hour,endTime.tm_min,endTime.tm_mday,endTime.tm_mon + 1,endTime.tm_year + 1900) + "\n" +
	            (logger_->cgi_.isCGI() ? "" : "    </A>\n")
            ;
            break;
          default    :
            assert( 0 );
        }
        f <<
          "  </TH>\n"
          "</TR>\n"
          "<TR>\n"
          "  <TH HEIGHT=4>"
          "  </TH>\n"
          "</TR>\n"
          "<TR>\n"
          "  <TH ALIGN=center BGCOLOR=\"" + logger_->getDecor("head.host",section_) + "\" nowrap>\n"
          "    Host\n"
          "  </TH>\n"
          "  <TH ALIGN=center BGCOLOR=\"" + logger_->getDecor("head.data",section_) + "\" nowrap>\n"
          "    Data bytes\n"
          "  </TH>\n"
          "  <TH ALIGN=center BGCOLOR=\"" + logger_->getDecor("head.dgram",section_) + "\" nowrap>\n"
          "    Datagram bytes\n"
          "  </TH>\n"
        ;
        while( *pi >= sv ){
          if( sums[*pi + av] > 0 )
            f <<
              "  <TH COLSPAN=2 ALIGN=center BGCOLOR=\"" + logger_->getDecor("detail_head",section_) + "\" nowrap>\n"
              "    " + utf8::int2Str(*pi + (level == rlYear)) + "\n"
              "  </TH>\n"
	          ;
          (*pi)--;
        }
        switch( level ){
          case rlYear :
            endTime.tm_mon = 11;
            break;
          case rlMon :
            endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
            break;
          case rlDay :
            endTime.tm_hour = 23;
            break;
          case rlHour :
            endTime.tm_min = 59;
            break;
          case rlMin :
            endTime.tm_sec = 59;
            break;
          default    :
            assert( 0 );
        }
        utf8::String detailedChartData;
        for( intptr_t i = table[0].rowCount() - 1; i >= 0; i-- ){
          struct in_addr ip41 = ksock::SockAddr::indexToAddr4(table[0](i,"src_ip"));
          intptr_t ip41Port = ports_ ? ksock::api.ntohs(table[0](i,"src_port")) : 0;
          struct in_addr ip42 = ksock::SockAddr::indexToAddr4(table[0](i,"dst_ip"));
          intptr_t ip42Port = ports_ ? ksock::api.ntohs(table[0](i,"dst_port")) : 0;
          intptr_t ipProto = protocols_ ? (int16_t) table[0](i,"ip_proto") : -1;
          utf8::String row(
            "<TR>\n"
            "  <TH ALIGN=left BGCOLOR=\"" + logger_->getDecor("body.host",section_) + "\" nowrap>\n" +
	          genHRef(ip41,ip41Port) +
            (bidirectional_ ?
              " <B>--></B> " + genHRef(ip42,ip42Port) :
              utf8::String()
            ) +
            (ipProto >= 0 ? " " + ksock::SockAddr::protoAsString(ipProto) : "")
          );
          utf8::String row2(
            "  </TH>\n"
            "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("body.data",section_) + "\" nowrap>\n" +
            formatTraf(table[0](i,"SUM2"),sums[0]) + "\n"
            "  </TH>\n"
            "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("body.dgram",section_) + "\" nowrap>\n" +
            formatTraf(table[0](i,"SUM1"),sums[0]) + "\n"
            "  </TH>\n"
          );
          utf8::String lineChartData, lineChartData2;
          while( *pi >= sv ){
            switch( level ){
              case rlYear :
                beginTime.tm_mon = endTime.tm_mon;
                endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
                break;
              case rlMon :
                beginTime.tm_mday = endTime.tm_mday;
                break;
              case rlDay :
                beginTime.tm_hour = endTime.tm_hour;
                break;
              case rlHour :
                beginTime.tm_min = endTime.tm_min;
                break;
              case rlMin :
                beginTime.tm_sec = endTime.tm_sec;
                break;
              default    :
                assert( 0 );
            }
            if( sums[*pi + av] > 0 ){
              uintmax_t sum1, sum2;
              writeBPFTHtmlReportHelper1(i,level,beginTime,endTime,btt,ett,bta,eta,sum1,sum2);
              row2 +=
                "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("details.body.data",section_) + "\" nowrap>\n" +
                formatTraf(sum2,sums[*pi + av]) + "\n"
                "  </TH>\n"
                "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("details.body.dgram",section_) + "\" nowrap>\n" +
                formatTraf(sum1,sums[*pi + av]) + "\n"
                "  </TH>\n"
              ;
              if( logger_->cgi_.isCGI() ){
                if( sum1 > 0 )
                  lineChartData += "&r0c" + utf8::int2Str(*pi) + "=" + utf8::int2Str(sum1);
                if( sum2 > 0 )
                  lineChartData += "&r1c" + utf8::int2Str(*pi) + "=" + utf8::int2Str(sum2);
                if( sum1 > 0 )
                  detailedChartData += "&r" + utf8::int2Str(i) + "c" + utf8::int2Str(*pi) + "=" + utf8::int2Str(sum1);
              }
	          }
	          (*pi)--;
          }
          if( logger_->cgi_.isCGI() ){
            utf8::String src(/*"http://" + getEnv("HTTP_HOST") + */getEnv("SCRIPT_NAME") + "?chart=&width=1024&height=768"), v;
            v += "&rc=2";
            v += "&r0cc=" + utf8::int2Str(table.count() - 1);
            v += "&r1cc=" + utf8::int2Str(table.count() - 1);
            v += lineChartData;
            if( level < rlDay ) v += "&xlvs=1";
            row += "<A HREF=\"" + src + v + "\"><BIG><B> -C- </B></BIG></A>";
          }
          f << row + row2 + "</TR>\n";
          switch( level ){
            case rlYear :
              endTime.tm_mon = 11;
              endTime.tm_mday = 31;
              break;
            case rlMon :
              endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
              break;
            case rlDay :
              endTime.tm_hour = 23;
              break;
            case rlHour :
              endTime.tm_min = 59;
              break;
            case rlMin :
              endTime.tm_sec = 59;
              break;
            default    :
              assert( 0 );
          }
        }
        f <<
          "<TR>\n"
          "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("tail.host",section_) + "\" nowrap>\n"
          "    Summary:\n"
          "  </TH>\n"
          "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("tail.data",section_) + "\" nowrap>\n" +
          formatTraf(table[0].sum("SUM2"),sums[0]) + "\n"
          "  </TH>\n"
          "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("tail.dgram",section_) + "\" nowrap>\n" +
          formatTraf(sums[0],sums[0]) + "\n"
          "  </TH>\n"
        ;
        while( *pi >= sv ){
          if( sums[*pi + av] > 0 )
            f <<
              "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("details.tail.data",section_) + "\" nowrap>\n" +
              formatTraf(table[*pi + av].sum("SUM2"),sums[0]) + "\n"
              "  </TH>\n"
              "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("details.tail.dgram",section_) + "\" nowrap>\n" +
              formatTraf(sums[*pi + av],sums[0]) + "\n"
              "  </TH>\n"
            ;
          (*pi)--;
        }
        f <<
          "</TR>\n"
          "</TABLE>\n<BR>\n<BR>\n"
        ;
        if( logger_->cgi_.isCGI() ){
          utf8::String src(/*"http://" + getEnv("HTTP_HOST") + */getEnv("SCRIPT_NAME") + "?chart=&width=1024&height=768"), v;
          v = "&rc=2";
          v += "&r0cc=" + utf8::int2Str(table.count() - 1);
          v += "&r1cc=" + utf8::int2Str(table.count() - 1);
          for( uintptr_t i = 1; i < table.count(); i++ ){
            uintmax_t s1 = sums[i];
            if( s1 != 0 ){
              uintmax_t s2 = table[i].sum("SUM2");
              v += "&r0c" + utf8::int2Str(i - 1) + "=" + utf8::int2Str(s1);
              if( s2 > 0 )
                v += "&r1c" + utf8::int2Str(i - 1) + "=" + utf8::int2Str(s2);
            }
          }
          if( level < rlDay ) v += "&xlvs=1";
          f << "<A HREF=\"" + src + v + "\">Summary chart</A>\n<BR>\n<BR>\n";

          v = "&rc=" + utf8::int2Str(table[0].rowCount());
          for( uintptr_t j = 0; j < table[0].rowCount(); j++ ){
            v += "&r" + utf8::int2Str(j) + "cc=" + utf8::int2Str(table.count() - 1);
          }
          v += detailedChartData;
          if( level < rlDay ) v += "&xlvs=1";
          f << "<A HREF=\"" + src + v + "\">Detailed chart</A>\n<BR>\n<BR>\n";
        }
      }
    }
    bool nextLevel = refreshOnlyCurrent_ || logger_->cgi_.isCGI();
    switch( level ){
      case rlYear :
        endTime.tm_mon = 11;
        endTime.tm_mday = 31;
        nextLevel = nextLevel || endTime.tm_year == curTime_.tm_year;
        break;
      case rlMon :
        endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
        nextLevel = nextLevel || endTime.tm_mon == curTime_.tm_mon;
        break;
      case rlDay :
        endTime.tm_hour = 23;
        nextLevel = nextLevel || endTime.tm_mday == curTime_.tm_mday;
        break;
      case rlHour :
        endTime.tm_min = 59;
        nextLevel = nextLevel || endTime.tm_hour == curTime_.tm_hour;
        break;
      case rlMin :
        endTime.tm_sec = 59;
        nextLevel = nextLevel || endTime.tm_min == curTime_.tm_min;
        break;
      case rlSec :
        nextLevel = nextLevel || endTime.tm_sec == curTime_.tm_sec;
        break;
      case rlNone :
      default    :;
    }
    if( nextLevel && !logger_->cgi_.isCGI() || level < minTotalsLevel_ )
      writeBPFTHtmlReport(level + 1,&endTime);
    endTime = endTime2;
    switch( level ){
      case rlYear :
        endTime.tm_year--;
        break;
      case rlMon :
        if( endTime.tm_mon == 0 ){
          endTime.tm_mon = 11;
          endTime.tm_year--;
        }
        else {
          endTime.tm_mon--;
        }
        endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
        break;
      case rlDay :
        if( endTime.tm_mday == 1 ){
          if( endTime.tm_mon == 0 ){
            endTime.tm_mon = 11;
            endTime.tm_year--;
          }
          else {
            endTime.tm_mon--;
          }
          endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
        }
        else {
          endTime.tm_mday--;
        }
        break;
      case rlHour :
        if( endTime.tm_hour == 0 ){
          if( endTime.tm_mday == 1 ){
            if( endTime.tm_mon == 0 ){
              endTime.tm_mon = 11;
              endTime.tm_year--;
            }
            else {
              endTime.tm_mon--;
            }
            endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
          }
          else {
            endTime.tm_mday--;
          }
        }
        else {
          endTime.tm_hour--;
        }
        break;
      case rlMin :
        if( endTime.tm_min == 0 ){
          if( endTime.tm_hour == 0 ){
            if( endTime.tm_mday == 1 ){
              if( endTime.tm_mon == 0 ){
                endTime.tm_mon = 11;
                endTime.tm_year--;
              }
              else {
                endTime.tm_mon--;
              }
              endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
            }
            else {
              endTime.tm_mday--;
            }
          }
          else {
            endTime.tm_hour--;
          }
        }
        else {
          endTime.tm_min--;
        }
        break;
      case rlSec :
        if( endTime.tm_sec == 0 ){
          if( endTime.tm_min == 0 ){
            if( endTime.tm_hour == 0 ){
              if( endTime.tm_mday == 1 ){
                if( endTime.tm_mon == 0 ){
                  endTime.tm_mon = 11;
                  endTime.tm_year--;
                }
                else {
                  endTime.tm_mon--;
                }
                endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
              }
              else {
                endTime.tm_mday--;
              }
            }
            else {
              endTime.tm_hour--;
            }
          }
          else {
            endTime.tm_min--;
          }
        }
        else {
          endTime.tm_sec--;
        }
        break;
      default    :
        assert( 0 );
    }
    beginTime = beginTime2;
  }
  if( !logger_->cgi_.isCGI() || level == maxTotalsLevel_ ){
    utf8::String filter(logger_->config_->textByPath(section_ + ".html_report.filter"));
    if( logger_->cgi_.isCGI() )
      filter = logger_->cgi_.paramAsString("filter");
    AutoLock<InterlockedMutex> lock(logger_->dnsMutex_);
    uintmax_t m0 = logger_->dnsCacheHitCount_ + logger_->dnsCacheMissCount_;
    utf8::String hitRatio("-"), missRatio("-");
    if( m0 > 0 ){
      if( logger_->dnsCacheHitCount_ > 0 )
        hitRatio =
          utf8::int2Str(logger_->dnsCacheHitCount_ * 100u / m0) + "." +
          utf8::int2Str0(
            logger_->dnsCacheHitCount_ * 10000u / m0 - 
            logger_->dnsCacheHitCount_ * 100u / m0 * 100u,
	          2
          )
        ;
      if( logger_->dnsCacheMissCount_ > 0 )
        missRatio =
          utf8::int2Str(logger_->dnsCacheMissCount_ * 100u / m0) + "." +
          utf8::int2Str0(
            logger_->dnsCacheMissCount_ * 10000u / m0 - 
            logger_->dnsCacheMissCount_ * 100u / m0 * 100u,
            2
          )
        ;
    }
    m0 = cachedPacketSumHitCount_ + cachedPacketSumMissCount_;
    utf8::String hitRatio2("-"), missRatio2("-");
    if( m0 > 0 ){
      if( cachedPacketSumHitCount_ > 0 )
        hitRatio2 =
          utf8::int2Str(cachedPacketSumHitCount_ * 100u / m0) + "." +
          utf8::int2Str0(
            cachedPacketSumHitCount_ * 10000u / m0 - 
            cachedPacketSumHitCount_ * 100u / m0 * 100u,
	          2
          )
        ;
      if( cachedPacketSumMissCount_ > 0 )
        missRatio2 =
          utf8::int2Str(cachedPacketSumMissCount_ * 100u / m0) + "." +
          utf8::int2Str0(
            cachedPacketSumMissCount_ * 10000u / m0 - 
            cachedPacketSumMissCount_ * 100u / m0 * 100u,
            2
          )
        ;
    }
    bool showFilter = logger_->config_->valueByPath(section_ + ".html_report.show_filter",false);
    f <<
      "Interface: " + sectionName_ + "<BR>\n" +
      utf8::String(showFilter ?
        "Filter: <B>" + filter + "</B>, hash: " + filterHash_ + "<BR>\n" +
        "FilterSQL: <B>" + filter_ + "</B><BR>\n" :
	      utf8::String()
      ) +
      "DNS cache size: " + utf8::int2Str((uintmax_t) logger_->dnsCache_.count()) + ", "
      "hit: " + utf8::int2Str(logger_->dnsCacheHitCount_) + ", " +
      "hit ratio: " + hitRatio + ", " +
      "miss: " + utf8::int2Str(logger_->dnsCacheMissCount_) + ", " +
      "miss ratio: " + missRatio +
      "<BR>\n"
      "PKTSUM cache size: " + utf8::int2Str(cachedPacketSumLRU_.count() * sizeof(CachedPacketSum)) + ", "
      "hit: " + utf8::int2Str(cachedPacketSumHitCount_) + ", " +
      "hit ratio: " + hitRatio2 + ", " +
      "miss: " + utf8::int2Str(cachedPacketSumMissCount_) + ", " +
      "miss ratio: " + missRatio2 +
      "<BR>\n"
    ;
    logger_->writeHtmlTail(f,ellapsed_);
  }
  if( level == rlYear || (logger_->cgi_.isCGI() && level == maxTotalsLevel_) )
    database_->commit();
}
//------------------------------------------------------------------------------
void Logger::BPFTThread::parseBPFTLogFile()
{
  if( !(bool) logger_->config_->valueByPath(section_ + ".enabled",false) ) return;
  AsyncFile flog(logger_->config_->valueByPath(section_ + ".log_file_name"));
/*
  statement_->text("DELETE FROM INET_BPFT_STAT")->execute();
  stFileStatUpd_->prepare()->
    paramAsString("ST_LOG_FILE_NAME",flog.fileName())->
    paramAsMutant("ST_LAST_OFFSET",0)->execute();
 */
  flog.readOnly(true).open();
  database_->start();
  if( (bool) logger_->config_->valueByPath(section_ + ".reset_log_file_position",false) )
    Logger::updateLogFileLastOffset(stFileStat_,flog.fileName(),0);
  uint64_t offset = Logger::fetchLogFileLastOffset(stFileStat_,flog.fileName());
  if( offset > flog.size() ) updateLogFileLastOffset(stFileStat_,flog.fileName(),offset = 0);
  if( flog.seekable() ) flog.seek(offset);
  int64_t lineNo = 0, tma = 0;
  int64_t cl = getlocaltimeofday();
  union {
    BPFTHeader header;
    BPFTHeader32 header32;
  };
  stBPFTIns_->text(
    "INSERT INTO INET_BPFT_STAT ("
    "  st_if,st_start,st_src_ip,st_dst_ip,st_ip_proto,st_src_port,st_dst_port,st_dgram_bytes,st_data_bytes"
    ") VALUES ("
    "  :st_if,:st_start,:st_src_ip,:st_dst_ip,:st_ip_proto,:st_src_port,:st_dst_port,:st_dgram_bytes,:st_data_bytes"
    ")"
  )->prepare()->paramAsString("st_if",sectionName_);
  bool log32bitOsCompatible = logger_->config_->valueByPath(section_ + ".log_32bit_os_compatible",SIZEOF_VOID_P < 8);
  struct tm start, stop;
  for(;;){
    uintptr_t entriesCount;
    if( log32bitOsCompatible ){
      if( flog.read(&header32,sizeof(header32)) != sizeof(header32) ) break;
      start = timeval2tm(header32.start_);
      stop = timeval2tm(header32.stop_);
      entriesCount = header32.eCount_;
    }
    else {
      if( flog.read(&header,sizeof(header)) != sizeof(header) ) break;
      start = timeval2tm(header.start_);
      stop = timeval2tm(header.stop_);
      entriesCount = header.eCount_;
    }
    // TODO: clear cache on start and stop
    Array<BPFTEntry> entries;
    Array<BPFTEntry32> entries32;
    if( log32bitOsCompatible ){
      entries32.resize(entriesCount);
      if( flog.read(entries32,sizeof(BPFTEntry32) * entriesCount) != int64_t(sizeof(BPFTEntry32) * entriesCount) ) break;
    }
    else {
      entries.resize(entriesCount);
      if( flog.read(entries,sizeof(BPFTEntry) * entriesCount) != int64_t(sizeof(BPFTEntry) * entriesCount) ) break;
    }
    if( dynamic_cast<MYSQLDatabase *>(statement_->database()) != NULL && entriesCount > 0 ){
      bool executed = true;
      utf8::String text;
      for( intptr_t i = entriesCount - 1; i >= 0; i-- ){
        if( executed ){
          text =
            "INSERT INTO INET_BPFT_STAT ("
            "  st_if,st_start,st_src_ip,st_dst_ip,st_ip_proto,st_src_port,st_dst_port,st_dgram_bytes,st_data_bytes"
            ") VALUES "
          ;
          executed = false;
        }
        text += "(";
        if( log32bitOsCompatible ){
          text +=
            "'" + sectionName_ + "'," +
            "'" + mycpp::tm2Str(start) + "'," +
            "'" + ksock::SockAddr::addr2Index(entries32[i].srcIp_) + "'," +
            "'" + ksock::SockAddr::addr2Index(entries32[i].dstIp_) +"'," +
            utf8::int2Str(entries32[i].ipProtocol_) + "," +
            utf8::int2Str(entries32[i].srcPort_) + "," +
            utf8::int2Str(entries32[i].dstPort_) + "," +
            utf8::int2Str(entries32[i].dgramSize_) + "," +
            utf8::int2Str(entries32[i].dataSize_)
          ;
        }
        else {
          text +=
            "'" + sectionName_ + "'," +
            "'" + mycpp::tm2Str(start) + "'," +
            "'" + ksock::SockAddr::addr2Index(entries[i].srcIp_) + "'," +
            "'" + ksock::SockAddr::addr2Index(entries[i].dstIp_) +"'," +
            utf8::int2Str(entries[i].ipProtocol_) + "," +
            utf8::int2Str(entries[i].srcPort_) + "," +
            utf8::int2Str(entries[i].dstPort_) + "," +
            utf8::int2Str(entries[i].dgramSize_) + "," +
            utf8::int2Str(entries[i].dataSize_)
          ;
        }
#ifndef NDEBUG
        const uintptr_t period = 8;
#else
        const uintptr_t period = 256;
#endif
        text += i % period != 0 ? ")," : ")";
        if( i % period == 0 ){
          stBPFTIns2_->text(text)->execute();
          executed = true;
        }
      }
      assert( executed );
    }
    else for( intptr_t i = entriesCount - 1; i >= 0; i-- ){
      if( log32bitOsCompatible ){
        stBPFTIns_->
	        paramAsMutant("st_src_ip",ksock::SockAddr::addr2Index(entries32[i].srcIp_))->
          paramAsMutant("st_dst_ip",ksock::SockAddr::addr2Index(entries32[i].dstIp_))->
          paramAsMutant("st_ip_proto",entries32[i].ipProtocol_)->
          paramAsMutant("st_src_port",entries32[i].srcPort_)->
          paramAsMutant("st_dst_port",entries32[i].dstPort_)->
          paramAsMutant("st_dgram_bytes",entries32[i].dgramSize_)->
          paramAsMutant("st_data_bytes",entries32[i].dataSize_);
      }
      else {
        stBPFTIns_->
          paramAsMutant("st_src_ip",ksock::SockAddr::addr2Index(entries[i].srcIp_))->
          paramAsMutant("st_dst_ip",ksock::SockAddr::addr2Index(entries[i].dstIp_))->
          paramAsMutant("st_ip_proto",entries[i].ipProtocol_)->
          paramAsMutant("st_src_port",entries[i].srcPort_)->
          paramAsMutant("st_dst_port",entries[i].dstPort_)->
          paramAsMutant("st_dgram_bytes",entries[i].dgramSize_)->
          paramAsMutant("st_data_bytes",entries[i].dataSize_);
      }
      stBPFTIns_->paramAsMutant("st_start",start)->execute();
      logger_->printStat(lineNo,offset,flog.tell(),flog.size(),cl,&tma);
      lineNo++;
      entriesCount = 0;
    }
    logger_->printStat(lineNo,offset,flog.tell(),flog.size(),cl,&tma);
    if( flog.seekable() ) Logger::updateLogFileLastOffset(stFileStat_,flog.fileName(),flog.tell());
    database_->commit();
    database_->start();
    lineNo += entriesCount;
  }
  logger_->printStat(lineNo,offset,flog.tell(),flog.size(),cl);
  database_->commit();
}
//------------------------------------------------------------------------------
} // namespace macrosocope
//------------------------------------------------------------------------------
