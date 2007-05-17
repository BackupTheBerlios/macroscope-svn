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
//------------------------------------------------------------------------------
#include <adicpp/adicpp.h>
//------------------------------------------------------------------------------
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
utf8::String Logger::getIPFilter(const utf8::String & text)
{
  utf8::String filter;
  utf8::String::Iterator i(text);
  uintptr_t c[6], prev = '\0';
  for(;;){
    if( i.eof() ) break;
    c[0] = i.getLowerChar();
    c[1] = (i + 1).getLowerChar();
    c[2] = (i + 2).getLowerChar();
    c[3] = (i + 3).getLowerChar();
    c[4] = (i + 4).getLowerChar();
    c[5] = (i + 5).getLowerChar();
    if( (prev == '\0' || prev == '(' || prev == ')' || (utf8::getC1Type(prev) & C1_SPACE) != 0) &&
        (c[0] == 's' && c[1] == 'r' && c[2] == 'c' && (c[3] == '_' || utf8::getC1Type(c[3]) & C1_SPACE) != 0) ||
        (c[0] == 'd' && c[1] == 's' && c[2] == 't' && (c[3] == '_' || utf8::getC1Type(c[3]) & C1_SPACE) != 0) ||
        (c[0] == 'p' && c[1] == 'r' && c[2] == 'o' && c[3] == 't' && c[4] == 'o' && (utf8::getC1Type(c[5]) & C1_SPACE) != 0) ){
      utf8::String::Iterator shift(i);
      while( shift.next() && !shift.isSpace() );
      utf8::String sd(utf8::String(i,shift));
      i = shift;
      while( i.next() && i.isSpace() );
      if( (i - 1).isSpace() ){
        ksock::SockAddr addr;
        if( sd.strcasecmp("src") == 0 || sd.strcasecmp("dst") == 0 ){
          sd = "st_" + sd.lower() + "_ip";
          utf8::String::Iterator j(i), net(j);
          bool isNetwork = false;
          while( !j.eof() && !j.isSpace() && j.getChar() != ')' ){
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
        else if( sd.strcasecmp("src_port") == 0 || sd.strcasecmp("dst_port") == 0 ){
          sd = "st_" + sd.lower();
          utf8::String::Iterator j(i);
          while( j.next() && !j.isSpace() && j.getChar() != ')' );
          filter += sd + " = " +
	          utf8::int2Str(addr.resolveName(":" + utf8::String(i,j)).addr4_.sin_port)
	        ;
          i = j;
	      }
        else if( sd.strcasecmp("proto") == 0 ){
          sd = "st_ip_proto";
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
        if( !e->searchCode(isc_no_dup,ER_DUP_ENTRY) ) throw;
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
  uintmax_t q, b, c, t1, t2, t3;
  char * postfix;

  q = traf * 10000u / allTraf;
  b = q / 100u;
  c = q % 100u;
  if( traf >= uintmax_t(1024u) * 1024u * 1024u * 1024u ){
    t2 = uintmax_t(1024u) * 1024u * 1024u * 1024u;
    postfix = "T";
  }
  else if( traf >= 1024u * 1024u * 1024u ){
    t2 = 1024u * 1024u * 1024u;
    postfix = "G";
  }
  else if( traf >= 1024u * 1024u ){
    t2 = 1024u * 1024u;
    postfix = "M";
  }
  else if( traf >= 1024u ){
    t2 = 1024u;
    postfix = "K";
  }
  else {
    return utf8::String::print(
      traf > 0 ? "%"PRIuMAX"<FONT SIZE=0>(%"PRIuMAX".%02"PRIuMAX"%%)</FONT>" :  "-",
      traf,
      b,
      c
    );
  }
  t1 = traf / t2;
  t3 = traf % t2;
  return utf8::String::print(
    traf > 0 ? "%"PRIuMAX".%04"PRIuMAX"%s<FONT SIZE=0>(%"PRIuMAX".%02"PRIuMAX"%%)</FONT>" :  "-",
    t1,
    uintmax_t(t3 / (t2 / 1024u)),
    postfix,
    b,
    c
  );
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
  stBPFTSel_ = database_->newAttachedStatement();
  stBPFTHostSel_ = database_->newAttachedStatement();
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
void Logger::BPFTThread::clearBPFTCache()
{
  /*struct tm curTimeBTHour = curTime_;
  curTimeBTHour.tm_min = 0;
  curTimeBTHour.tm_sec = 0;
  struct tm curTimeETHour = curTime_;
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
  curTimeETYear.tm_mon = 11;
  if( !stBPFTCacheDel_->prepared() ){
    stBPFTCacheDel_->text(
      "DELETE FROM INET_BPFT_STAT_CACHE WHERE "
      "st_if = :if AND ("
      "  (st_bt = :BTYear AND st_et = :ETYear) OR"
      "  (st_bt = :BTMon  AND st_et = :ETMon) OR"
      "  (st_bt = :BTDay AND st_et = :ETDay) OR"
      "  (st_bt = :BTHour AND st_et = :ETHour)"
      ")"
      " AND st_filter_hash = :hash AND st_threshold = :threshold"
    )->prepare();
  }
  stBPFTCacheDel_->
    paramAsString("if",sectionName_)->
    paramAsMutant("BTYear",time2tm(tm2Time(cu3rTimeBTYear) - getgmtoffset()))->
    paramAsMutant("ETYear",time2tm(tm2Time(curTimeETYear) - getgmtoffset()))->
    paramAsMutant("BTMon",time2tm(tm2Time(curTimeBTMon) - getgmtoffset()))->
    paramAsMutant("ETMon",time2tm(tm2Time(curTimeETMon) - getgmtoffset()))->
    paramAsMutant("BTMon",time2tm(tm2Time(curTimeBTDay) - getgmtoffset()))->
    paramAsMutant("ETMon",time2tm(tm2Time(curTimeETDay) - getgmtoffset()))->
    paramAsMutant("BTHour",time2tm(tm2Time(curTimeBTHour) - getgmtoffset()))->
    paramAsMutant("ETHour",time2tm(tm2Time(curTimeETHour) - getgmtoffset()))->
    paramAsMutant("hash",filterHash_)->
    paramAsMutant("threshold",minSignificantThreshold_)->execute();
  if( logger_->verbose_ ) fprintf(stderr,
    "bpft cache cleared for:\n  %s %s\n  %s %s\n  %s %s\n  %s %s\n  %"PRId64"\n  %"PRId64"\n",
    (const char *) utf8::tm2Str(curTimeBTYear).getOEMString(),
    (const char *) utf8::tm2Str(curTimeETYear).getOEMString(),
    (const char *) utf8::tm2Str(curTimeBTMon).getOEMString(),
    (const char *) utf8::tm2Str(curTimeETMon).getOEMString(),
    (const char *) utf8::tm2Str(curTimeBTDay).getOEMString(),
    (const char *) utf8::tm2Str(curTimeETDay).getOEMString(),
    (const char *) utf8::tm2Str(curTimeBTHour).getOEMString(),
    (const char *) utf8::tm2Str(curTimeETHour).getOEMString(),
    filterHash_,
    minSignificantThreshold_
  );*/
}
//------------------------------------------------------------------------------
bool Logger::BPFTThread::getBPFTCachedHelper(Statement * & pStatement)
{
  bool updateCache = true;
  if( pStatement == stBPFTSel_ ){
    if( !stBPFTCacheSel_->prepared() ){
      struct in_addr baddr;
      baddr.s_addr = INADDR_BROADCAST;
      stBPFTCacheSel_->text(
        "SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(stBPFTCacheSel_->database()) != NULL ? " SQL_NO_CACHE" : "") +
        " * FROM ("
        "  SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(stBPFTCacheSel_->database()) != NULL ? " SQL_NO_CACHE" : "") +
        "    st_src_ip, st_dst_ip, st_dgram_bytes as SUM1, st_data_bytes as SUM2 "
        "  FROM INET_BPFT_STAT_CACHE "
        "  WHERE"
        "    st_if = :if AND st_bt = :BT AND st_et = :ET AND st_filter_hash = :hash AND st_threshold = :threshold AND"
        "    st_src_ip <> '@' AND st_dst_ip <> '@'" +
        "  ORDER BY SUM1, st_src_ip, st_dst_ip "
        ") AS A "
        "WHERE (A.st_src_ip = '" + ksock::SockAddr::addr2Index(baddr) +
        "' AND A.st_dst_ip = '" + ksock::SockAddr::addr2Index(baddr) + "') OR A.SUM1 >= :threshold"
      )->prepare();
      stBPFTCacheSel_->paramAsString("if",sectionName_)->
        paramAsString("hash",filterHash_)->
        paramAsMutant("threshold",minSignificantThreshold_);
    }
    stBPFTCacheSel_->paramAsMutant("BT",pStatement->paramAsMutant("BT"))->
      paramAsMutant("ET",pStatement->paramAsMutant("ET"));
    if( stBPFTCacheSel_->execute()->fetch() ){
      stBPFTCacheSel_->fetchAll();
      pStatement = stBPFTCacheSel_;
      updateCache = false;
    }
  }
  else if( pStatement == stBPFTHostSel_ ){
    if( !stBPFTCacheHostSel_->prepared() ){
      stBPFTCacheHostSel_->text(
        "SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(stBPFTCacheHostSel_->database()) != NULL ? " SQL_NO_CACHE" : "") +
        "  st_src_ip, st_dst_ip, st_dgram_bytes as SUM1, st_data_bytes as SUM2 "
        "FROM INET_BPFT_STAT_CACHE "
        "WHERE"
        "  st_if = :if AND st_bt = :BT AND st_et = :ET AND"
        "  st_filter_hash = :hash AND st_threshold = :threshold AND " +
        "  st_src_ip = :src AND st_dst_ip = :dst"
      )->prepare();
      stBPFTCacheHostSel_->paramAsString("if",sectionName_)->
        paramAsString("hash",filterHash_)->
        paramAsMutant("threshold",minSignificantThreshold_);
    }
    stBPFTCacheHostSel_->paramAsMutant("BT",pStatement->paramAsMutant("BT"))->
      paramAsMutant("ET",pStatement->paramAsMutant("ET"))->
      paramAsString("src",pStatement->paramAsString("src"))->
      paramAsString("dst",pStatement->paramAsString("dst"));
    if( stBPFTCacheHostSel_->execute()->fetch() ){
      stBPFTCacheHostSel_->fetchAll();
      pStatement = stBPFTCacheHostSel_;
      updateCache = false;
    }
  }
  else {
    assert( 0 );
  }
  return updateCache;
}
//------------------------------------------------------------------------------
void Logger::BPFTThread::getBPFTCached(Statement * pStatement,Table<Mutant> * pResult,uintmax_t * pDgramBytes,uintmax_t * pDataBytes)
{
  /*bool curIntr = Logger::isCurrentTimeInterval(
    curTime_,
    time2tm((uint64_t) pStatement->paramAsMutant("BT") + getgmtoffset()),
    time2tm((uint64_t) pStatement->paramAsMutant("ET") + getgmtoffset())
  );
  bool updateCache = getBPFTCachedHelper(pStatement);
  if( logger_->verbose_ && (pStatement == stBPFTSel_ || pStatement == stBPFTCacheSel_) ){
    fprintf(stderr,
      "bpft cache %s%s: %s %s\n",
      (curIntr ? "current time " : ""),
      (updateCache ? "miss" : "hit"),
      (const char *) utf8::time2Str((uint64_t) pStatement->paramAsMutant("BT") + getgmtoffset()).getOEMString(),
      (const char *) utf8::time2Str((uint64_t) pStatement->paramAsMutant("ET") + getgmtoffset()).getOEMString()
    );
  }
  if( updateCache ){
    dbtrUpdate_->start();
    if( !stBPFTCacheIns_->prepared() ){
      stBPFTCacheIns_->text(
        "INSERT INTO INET_BPFT_STAT_CACHE ("
        "  st_if, st_bt, st_et, st_src_ip, st_dst_ip, st_filter_hash, st_threshold, st_dgram_bytes, st_data_bytes"
        ") VALUES ("
        "  :st_if, :st_bt, :st_et, :st_src_ip, :st_dst_ip, :st_filter_hash, :st_threshold, :st_dgram_bytes, :st_data_bytes"
        ")"
      )->prepare()->
        paramAsString("st_if",sectionName_)->
        paramAsString("st_filter_hash",filterHash_)->
        paramAsMutant("st_threshold",minSignificantThreshold_);
    }
    stBPFTCacheIns_->
      paramAsMutant("st_bt",pStatement->paramAsMutant("BT"))->
      paramAsMutant("st_et",pStatement->paramAsMutant("ET"));
    if( !stBPFTCacheIns2_->prepared() )
      stBPFTCacheIns2_->text(stBPFTCacheIns_->text())->prepare();
// lock for update cache record
    if( !stBPFTCacheSelForUpdate_->prepared() ){
      stBPFTCacheSelForUpdate_->text(
        "SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(stBPFTCacheHostSel_->database()) != NULL ? " SQL_NO_CACHE" : "") +
        "  st_src_ip, st_dst_ip "
        "FROM INET_BPFT_STAT_CACHE "
        "WHERE"
        "  st_if = :if AND st_bt = :BT AND st_et = :ET AND"
        "  st_filter_hash = :hash AND st_threshold = :threshold AND " +
        "  st_src_ip = :src AND st_dst_ip = :dst "
	      "FOR UPDATE"
      )->prepare()->
        paramAsString("if",sectionName_)->
        paramAsString("hash",filterHash_)->
        paramAsMutant("threshold",minSignificantThreshold_)->
        paramAsString("src","@")->
        paramAsString("dst","@");
    }
    stBPFTCacheSelForUpdate_->
      paramAsMutant("BT",pStatement->paramAsMutant("BT"))->
      paramAsMutant("ET",pStatement->paramAsMutant("ET"));
    if( !stBPFTCacheUpd_->prepared() ){
      stBPFTCacheUpd_->text(
        "UPDATE INET_BPFT_STAT_CACHE SET"
        "  st_dgram_bytes = st_dgram_bytes + 1 "
        "WHERE"
        "  st_if = :if AND st_bt = :BT AND st_et = :ET AND"
        "  st_filter_hash = :hash AND st_threshold = :threshold AND "
        "  st_src_ip = :src AND st_dst_ip = :dst "
      )->prepare()->
        paramAsString("if",sectionName_)->
        paramAsString("hash",filterHash_)->
        paramAsMutant("threshold",minSignificantThreshold_)->
        paramAsString("src","@")->
        paramAsString("dst","@");
    }
    if( !curIntr ){
      stBPFTCacheSelForUpdate_->execute();
      if( !stBPFTCacheSelForUpdate_->fetch() ){
        stBPFTCacheIns_->
          paramAsString("st_src_ip","@")->
          paramAsString("st_dst_ip","@")->
          paramAsMutant("st_dgram_bytes",0)->
          paramAsMutant("st_data_bytes",0)->execute();
        dbtrUpdate_->commit()->start();
        stBPFTCacheSelForUpdate_->execute();
      }
      stBPFTCacheSelForUpdate_->fetchAll();
      assert( stBPFTCacheSelForUpdate_->rowCount() > 0 );
// use fake update for locking records
      stBPFTCacheUpd_->
        paramAsMutant("BT",pStatement->paramAsMutant("BT"))->
        paramAsMutant("ET",pStatement->paramAsMutant("ET"))->
        execute();      
// try to fetch cache filled from concurrent transaction
      updateCache = getBPFTCachedHelper(pStatement);
    }
    if( updateCache ){  
      pStatement->execute()->fetchAll();
      if( pResult != NULL ) pStatement->unloadColumns(*pResult);
      for( intptr_t i = pStatement->rowCount() - 1; i >= 0; i-- ){
        pStatement->selectRow(i);
        stBPFTCacheIns_->
          paramAsString("st_src_ip",pStatement->valueAsString("st_src_ip"))->
          paramAsString("st_dst_ip",pStatement->valueAsString("st_dst_ip"));
        uintmax_t sum1 = pStatement->valueAsMutant("SUM1"), sum2 = pStatement->valueAsMutant("SUM2");
        if( pStatement == stBPFTSel_ && sum1 < minSignificantThreshold_ ){
          stBPFTCacheIns_->
            paramAsString("st_src_ip",ip4AddrToIndex(0xFFFFFFFF))->
            paramAsString("st_dst_ip",ip4AddrToIndex(0xFFFFFFFF))->
            paramAsMutant("st_dgram_bytes",pStatement->sum("SUM1",0,i))->
            paramAsMutant("st_data_bytes",pStatement->sum("SUM2",0,i));
          if( curIntr ) stBPFTCacheIns2_->copyParams(stBPFTCacheIns_)->execute(); else stBPFTCacheIns_->execute();
          if( pResult != NULL ){
  	        uintptr_t row = pResult->rowCount();
            pResult->addRow();
            pResult->cell(row,"st_src_ip") = stBPFTCacheIns_->paramAsString("st_src_ip");
            pResult->cell(row,"st_dst_ip") = stBPFTCacheIns_->paramAsString("st_dst_ip");
            pResult->cell(row,"SUM1") = stBPFTCacheIns_->paramAsMutant("st_dgram_bytes");
            pResult->cell(row,"SUM2") = stBPFTCacheIns_->paramAsMutant("st_data_bytes");
          }
          break;
        }
        stBPFTCacheIns_->
          paramAsMutant("st_dgram_bytes",sum1)->
          paramAsMutant("st_data_bytes",sum2);
        if( curIntr ) stBPFTCacheIns2_->copyParams(stBPFTCacheIns_)->execute(); else stBPFTCacheIns_->execute();
        if( pResult != NULL ) pStatement->unloadRowByIndex(pResult->addRow());
      }
    }
    dbtrUpdate_->commit();
    if( pResult != NULL ){
      pResult->sort("SUM1,st_src_ip,st_dst_ip");
      return;
    }
  }*/
  CachedPacketSum * p;
  AutoPtr<CachedPacketSum> pktSum;
  if( pResult == NULL ){
    pktSum = p = newObject<CachedPacketSum>();
    p->bt_ = pStatement->paramAsMutant("BT");
    p->et_ = pStatement->paramAsMutant("ET");
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
    bool threshold = false;
    pStatement->execute()->fetchAll()->unloadColumns(*pResult);
    for( intptr_t i = pStatement->rowCount() - 1; i >= 0; i-- ){
      pStatement->selectRow(i);
      uintptr_t row = pResult->rowCount();
      uintmax_t sum1 = pStatement->valueAsMutant("SUM1");
      pktSum = p = newObject<CachedPacketSum>();
      p->bt_ = pStatement->paramAsMutant("BT");
      p->et_ = pStatement->paramAsMutant("ET");
      if( threshold ){
        p->srcAddr_ = ksock::SockAddr::indexToAddr4(pStatement->valueAsMutant("st_src_ip"));
        p->dstAddr_ = ksock::SockAddr::indexToAddr4(pStatement->valueAsMutant("st_dst_ip"));
        p->srcPort_ = ports_ ? (uint16_t) pStatement->valueAsMutant("st_src_port") : 0;
        p->dstPort_ = ports_ ? (uint16_t) pStatement->valueAsMutant("st_dst_port") : 0;
        p->proto_ = protocols_ ? (int16_t) pStatement->valueAsMutant("st_ip_proto") : -1;
        p->pktSize_ = pStatement->valueAsMutant("SUM1");
        p->dataSize_ = pStatement->valueAsMutant("SUM2");
      }
      else {
        if( sum1 < minSignificantThreshold_ ){
          struct in_addr baddr;
          baddr.s_addr = INADDR_BROADCAST;
          pResult->addRow();
          pResult->cell(row,"st_src_ip") = ksock::SockAddr::addr2Index(baddr);
          pResult->cell(row,"st_dst_ip") = ksock::SockAddr::addr2Index(baddr);
          if( ports_ ){
            pResult->cell(row,"st_src_port") = 0;
            pResult->cell(row,"st_src_port") = 0;
          }
          if( protocols_ )
            pResult->cell(row,"st_ip_proto") = -1;
          pResult->cell(row,"SUM1") = pStatement->sum("SUM1",0,i);
          pResult->cell(row,"SUM2") = pStatement->sum("SUM2",0,i);
          threshold = true;
        }
        else {
          pStatement->unloadRowByIndex(pResult->addRow());
        }
        p->srcAddr_ = ksock::SockAddr::indexToAddr4(pResult->cell(row,"st_src_ip"));
        p->dstAddr_ = ksock::SockAddr::indexToAddr4(pResult->cell(row,"st_dst_ip"));
        p->srcPort_ = ports_ ? (uint16_t) pResult->cell(row,"st_src_port") : 0;
        p->dstPort_ = ports_ ? (uint16_t) pResult->cell(row,"st_dst_port") : 0;
        p->proto_ = protocols_ ? (int16_t) pResult->cell(row,"st_ip_proto") : -1;
        p->pktSize_ = pResult->cell(row,"SUM1");
        p->dataSize_ = pResult->cell(row,"SUM2");
      }
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
//    pStatement->unloadByIndex(*pResult);
    pResult->sort("SUM1");
  }
}
//------------------------------------------------------------------------------
utf8::String Logger::BPFTThread::genHRef(const in_addr & ip,uintptr_t port)
{
  utf8::String name(logger_->resolveAddr(stDNSCache_,resolveDNSNames_,ip));
  utf8::String addr(logger_->resolveAddr(stDNSCache_,resolveDNSNames_,ip,true));
  return
    "    <A HREF=\"http://" + name + "\">\n" + name +
    (name.strcmp(addr) == 0 ?utf8::String() : " (" + addr + ")") +
    (port > 0 ? ":" + utf8::int2Str(port) : utf8::String()) +
    "\n    </A>\n"
  ;
}
//------------------------------------------------------------------------------
void Logger::BPFTThread::writeBPFTHtmlReport(intptr_t level,const struct tm * rt)
{
  struct tm beginTime, beginTime2, endTime, endTime2;
  Mutant m0, m1, m2;
  AsyncFile f;
  intptr_t totalsLevel = 0;

  if( level == rlYear ){
    if( !logger_->cgi_.isCGI() && !(bool) logger_->config_->valueByPath(section_ + ".html_report.enabled",true) ) return;
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
    filter_ = getIPFilter(filter_.trim());
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
    if( !bidirectional_ ) protocols_ = ports_ = false;
    database_->start();
//    clearBPFTCache();
    if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL ){
      statement_->text(
        "SELECT FIRST 1 st_start AS BT FROM INET_BPFT_STAT "
        "WHERE st_if = :st_if ORDER BY st_if, st_start"
      )->prepare()->paramAsString("st_if",sectionName_)->execute()->fetchAll();
      if( statement_->fieldIndex("BT") < 0 || statement_->rowCount() == 0 ) goto l1;
      beginTime = time2tm((uint64_t) statement_->valueAsMutant("BT") + getgmtoffset());
      statement_->text(
        "SELECT FIRST 1 st_start AS ET FROM INET_BPFT_STAT "
        "WHERE st_if = :st_if ORDER BY st_if desc, st_start desc"
      )->prepare()->paramAsString("st_if",sectionName_)->execute()->fetchAll();
      if( statement_->fieldIndex("ET") < 0 || statement_->rowCount() == 0 ) goto l1;
      endTime = time2tm((uint64_t) statement_->valueAsMutant("ET") + getgmtoffset());
    }
    else if( dynamic_cast<MYSQLDatabase *>(statement_->database()) != NULL ){
      statement_->text(
        "SELECT MIN(st_start) AS BT, MAX(st_start) AS ET FROM INET_BPFT_STAT WHERE st_if = :st_if"
      )->prepare()->paramAsString("st_if",sectionName_)->execute()->fetchAll();
      if( statement_->fieldIndex("BT") < 0 || statement_->fieldIndex("ET") < 0 ||
          statement_->rowCount() == 0 ||
          (statement_->valueAsMutant("BT") == Mutant(0) &&
           statement_->valueAsMutant("ET") == Mutant(0)) ) goto l1;
      beginTime = time2tm((uint64_t) statement_->valueAsMutant("BT") + getgmtoffset());
      endTime = time2tm((uint64_t) statement_->valueAsMutant("ET") + getgmtoffset());
    }
    else {
      assert( 0 );
    }
//    stdErr.debug(9,utf8::String::Stream() << __FILE__ << ", " << __LINE__ << "\n").flush();
    if( bidirectional_ ){
      stBPFTSel_->text(
        "  SELECT " +
        utf8::String(ports_ ? "st_src_ip, st_src_port, st_dst_ip, st_dst_port," : "st_src_ip, st_dst_ip,") +
        utf8::String(protocols_ ? " st_ip_proto," : "") +
        "    SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2"
        "  FROM"
        "    INET_BPFT_STAT"
        "  WHERE "
        "    st_if = :st_if AND"
        "    st_start >= :BT AND st_start <= :ET " +
        (filter_.isNull() ? utf8::String() : " AND " + filter_) +
        "  GROUP BY " +
        (ports_ ? "st_src_ip, st_src_port, st_dst_ip, st_dst_port " : "st_src_ip, st_dst_ip ") +
        (protocols_ ? ", st_ip_proto " : "") +
        "  ORDER BY SUM1"
        /* +
        utf8::String(dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL ?
          "  PLAN (INET_BPFT_STAT INDEX (IBS_IDX4))" : "") +*/
      );
      stBPFTHostSel_->text(
        "SELECT"
        "  SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2 "
        "FROM"
        "  INET_BPFT_STAT "
        "WHERE"
        "  st_if = :st_if AND"
        "  st_start >= :BT AND st_start <= :ET AND"
        "  st_src_ip = :src AND st_dst_ip = :dst" +
        utf8::String(ports_ ? 
          " AND st_src_port = :src_port AND st_dst_port = :dst_port" : ""
        ) +
        utf8::String(protocols_ ? 
          " AND st_ip_proto = :proto " : ""
        )
        /* +
        utf8::String(dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL ?
          " PLAN (INET_BPFT_STAT INDEX (IBS_IDX4))" : "")*/
      );
    }
    else {
      stBPFTSel_->text(
        "SELECT"
        "  *"
        "FROM ("
        "  SELECT"
        "    B.st_ip AS st_src_ip, B.st_ip AS st_dst_ip, SUM(B.SUM1) AS SUM1, SUM(B.SUM2) AS SUM2"
        "  FROM ("
        "      SELECT"
        "        st_dst_ip AS st_ip, SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2"
        "      FROM"
        "        INET_BPFT_STAT"
        "      WHERE "
        "        st_if = :st_if AND"
        "        st_start >= :BT AND st_start <= :ET "
        "      GROUP BY st_dst_ip"
        "    UNION ALL"
        "      SELECT"
        "        st_src_ip AS st_ip, SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2"
        "      FROM"
        "        INET_BPFT_STAT"
        "      WHERE "
        "        st_if = :st_if AND"
        "        st_start >= :BT AND st_start <= :ET "
        "      GROUP BY st_src_ip"
        "  ) AS B "
        "  GROUP BY B.st_ip"
        ") AS A " +
        (filter_.isNull() ? utf8::String() : "WHERE "  + filter_) +
        "ORDER BY SUM1"
      );
      stBPFTHostSel_->text(// переписать через вложеный 
        "SELECT"
        "  SUM(A.SUM1) AS SUM1, SUM(A.SUM2) AS SUM2 "
        "FROM ("
        "      SELECT"
        "        SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2, 0 AS COLIDX"
        "      FROM"
        "        INET_BPFT_STAT"
        "      WHERE "
        "        st_if = :st_if AND"
        "        st_start >= :BT AND st_start <= :ET AND"
        "        st_dst_ip = :dst"
        "      GROUP BY st_dst_ip"
        "    UNION ALL"
        "      SELECT"
        "        SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2, 0 AS COLIDX"
        "      FROM"
        "        INET_BPFT_STAT"
        "      WHERE "
        "        st_if = :st_if AND"
        "        st_start >= :BT AND st_start <= :ET AND"
        "        st_src_ip = :src"
        "      GROUP BY st_src_ip"
        "  ) AS A "
        "GROUP BY A.COLIDX"
      );
    }
    stBPFTSel_->prepare()->paramAsString("st_if",sectionName_);
    stBPFTHostSel_->prepare()->paramAsString("st_if",sectionName_);
    htmlDir_ = excludeTrailingPathDelimiter(logger_->config_->valueByPath(section_ + ".html_report.directory"));
    if( logger_->cgi_.isCGI() ){
      memset(&cgiBT_,0,sizeof(cgiBT_));
      cgiBT_.tm_year = (int) logger_->cgi_.paramAsMutant("byear") - 1900;
      cgiBT_.tm_mon = (int) logger_->cgi_.paramAsMutant("bmon") - 1;
      cgiBT_.tm_mday = (int) logger_->cgi_.paramAsMutant("bday");
      memset(&cgiET_,0,sizeof(cgiET_));
      cgiET_.tm_year = (int) logger_->cgi_.paramAsMutant("eyear") - 1900;
      cgiET_.tm_mon = (int) logger_->cgi_.paramAsMutant("emon") - 1;
      cgiET_.tm_mday = (int) logger_->cgi_.paramAsMutant("eday");
      cgiET_.tm_hour = 23;
      cgiET_.tm_min = 59;
      cgiET_.tm_sec = 59;
      if( tm2Time(cgiBT_) > tm2Time(cgiET_) ) ksys::xchg(cgiBT_,cgiET_);
      if( tm2Time(cgiBT_) < tm2Time(beginTime) ) cgiBT_ = beginTime;
      if( tm2Time(cgiET_) > tm2Time(endTime) ) cgiET_ = endTime;
      cgiBT_.tm_hour = 0;
      cgiBT_.tm_min = 0;
      cgiBT_.tm_sec = 0;
      cgiET_.tm_hour = 23;
      cgiET_.tm_min = 59;
      cgiET_.tm_sec = 59;
    }
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
      f.fileName(
        includeTrailingPathDelimiter(htmlDir_) + 
        "bpft-" + sectionName_ +
        utf8::String::print("-traf-by-%04d%02d.html",endTime.tm_year + 1900,endTime.tm_mon + 1)
      );
      break;
    case rlCount :
      return;
    default :
      pi = NULL;
      sv = av = fv = 0;
  }
  if( logger_->cgi_.isCGI() ){
    f.fileName("stdout").open();
    if( level == rlYear ) logger_->writeHtmlHead(f);
    utf8::String totals(logger_->cgi_.paramAsString("totals"));
    if( totals.strcasecmp("Day") == 0 ) totalsLevel = rlDay;
    else
    if( totals.strcasecmp("Mon") == 0 ) totalsLevel = rlMon;
    else
    if( totals.strcasecmp("Year") == 0 ) totalsLevel = rlYear;
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
  while( tm2Time(endTime) >= tm2Time(beginTime) ){
    beginTime2 = beginTime;
    endTime2 = endTime;
    switch( level ){
      case rlYear :
        beginTime.tm_year = endTime.tm_year;
        break;
      case rlMon :
        beginTime.tm_mon = endTime.tm_mon;
        break;
      case rlDay :
        beginTime.tm_mday = endTime.tm_mday;
        break;
      default    :
        assert( 0 );
    }
    Vector<Table<Mutant> > table;
    if( logger_->cgi_.isCGI() &&
        (tm2Time(cgiET_) < tm2Time(beginTime) || tm2Time(cgiBT_) > tm2Time(endTime) ||
         level < totalsLevel) ){
    }
    else {
      stBPFTSel_->
        paramAsMutant("BT",time2tm(tm2Time(beginTime) - getgmtoffset()))->
        paramAsMutant("ET",time2tm(tm2Time(endTime) - getgmtoffset()));
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
        default    :
          assert( 0 );
      }
      getBPFTCached(stBPFTSel_,&table[0]);
      if( (uintmax_t) table[0].sum("SUM1") > 0 ){
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
            default    :
              assert( 0 );
          }
          stBPFTSel_->
            paramAsMutant("BT",time2tm(tm2Time(beginTime) - getgmtoffset()))->
            paramAsMutant("ET",time2tm(tm2Time(endTime) - getgmtoffset()));
          getBPFTCached(stBPFTSel_,&table[*pi + av]);
          colCount += (uintmax_t) table[*pi + av].sum("SUM1") > 0;
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
          default    :
            assert( 0 );
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
          if( (uintmax_t) table[*pi + av].sum("SUM1") > 0 )
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
          default    :
            assert( 0 );
        }
        for( intptr_t i = table[0].rowCount() - 1; i >= 0; i-- ){
          struct in_addr ip41 = ksock::SockAddr::indexToAddr4(table[0](i,"st_src_ip"));
          intptr_t ip41Port = ports_ ? ksock::api.ntohs(table[0](i,"st_src_port")) : 0;
          struct in_addr ip42 = ksock::SockAddr::indexToAddr4(table[0](i,"st_dst_ip"));
          intptr_t ip42Port = ports_ ? ksock::api.ntohs(table[0](i,"st_dst_port")) : 0;
          intptr_t ipProto = protocols_ ? (int16_t) table[0](i,"st_ip_proto") : -1;
	        utf8::String row(
            "<TR>\n"
            "  <TH ALIGN=left BGCOLOR=\"" + logger_->getDecor("body.host",section_) + "\" nowrap>\n" +
	          genHRef(ip41,ip41Port) +
            (bidirectional_ ?
              " <B>--></B> " + genHRef(ip42,ip42Port) :
              utf8::String()
            ) +
            (ipProto >= 0 ? " " + ksock::SockAddr::protoAsString(ipProto) : "") +
            "  </TH>\n"
            "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("body.data",section_) + "\" nowrap>\n" +
            formatTraf(table[0](i,"SUM2"),table[0].sum("SUM1")) + "\n"
            "  </TH>\n"
            "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("body.dgram",section_) + "\" nowrap>\n" +
            formatTraf(table[0](i,"SUM1"),table[0].sum("SUM1")) + "\n"
            "  </TH>\n"
          );
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
              default    :
                assert( 0 );
            }
            if( (uintmax_t) table[*pi + av].sum("SUM1") > 0 ){
              stBPFTHostSel_->
	              paramAsMutant("BT",time2tm(tm2Time(beginTime) - getgmtoffset()))->
                paramAsMutant("ET",time2tm(tm2Time(endTime) - getgmtoffset()))->
                paramAsMutant("src",table[0](i,"st_src_ip"))->
                paramAsMutant("dst",table[0](i,"st_dst_ip"));
              if( ports_ )
                stBPFTHostSel_->
                  paramAsMutant("src_port",table[0](i,"st_src_port"))->
                  paramAsMutant("dst_port",table[0](i,"st_dst_port"));
              if( protocols_ )
                stBPFTHostSel_->paramAsMutant("proto",ipProto);
              uintmax_t sum1, sum2;
              getBPFTCached(stBPFTHostSel_,NULL,&sum1,&sum2);
              row +=
                "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("details.body.data",section_) + "\" nowrap>\n" +
                formatTraf(sum2,table[*pi + av].sum("SUM1")) + "\n"
                "  </TH>\n"
                "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("details.body.dgram",section_) + "\" nowrap>\n" +
                formatTraf(sum1,table[*pi + av].sum("SUM1")) + "\n"
                "  </TH>\n"
              ;
	          }
	          (*pi)--;
          }
	        f << row + "</TR>\n";
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
          formatTraf(table[0].sum("SUM2"),table[0].sum("SUM1")) + "\n"
          "  </TH>\n"
          "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("tail.dgram",section_) + "\" nowrap>\n" +
          formatTraf(table[0].sum("SUM1"),table[0].sum("SUM1")) + "\n"
          "  </TH>\n"
        ;
        while( *pi >= sv ){
          if( (uintmax_t) table[*pi + av].sum("SUM1") > 0 )
            f <<
              "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("details.tail.data",section_) + "\" nowrap>\n" +
              formatTraf(table[*pi + av].sum("SUM2"),table[0].sum("SUM1")) + "\n"
              "  </TH>\n"
              "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("details.tail.dgram",section_) + "\" nowrap>\n" +
              formatTraf(table[*pi + av].sum("SUM1"),table[0].sum("SUM1")) + "\n"
              "  </TH>\n"
            ;
          (*pi)--;
        }
        f <<
          "</TR>\n"
          "</TABLE>\n<BR>\n<BR>\n"
        ;
      }
    }
    bool nextLevel = !(bool) logger_->config_->valueByPath(section_ + ".html_report.refresh_only_current",false) || logger_->cgi_.isCGI();
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
      default    :
        assert( 0 );
    }
    if( nextLevel ){
      table.clear();
      writeBPFTHtmlReport(level + 1,&endTime);
    }
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
      default    :
        assert( 0 );
    }
    beginTime = beginTime2;
  }
  if( !logger_->cgi_.isCGI() || level == rlYear ){
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
  if( level == rlYear ){
l1: database_->rollback();
  }
}
//------------------------------------------------------------------------------
void Logger::BPFTThread::parseBPFTLogFile()
{
  if( !(bool) logger_->config_->valueByPath(section_ + ".enabled",true) ) return;
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
