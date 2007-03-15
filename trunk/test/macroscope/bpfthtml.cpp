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
utf8::String Logger::ip4AddrToIndex(uint32_t ip4)
{
  AutoPtr<char> b;
  b.alloc(sizeof(ip4) * 2 + 1);
  uintptr_t i;
  for( i = 0; i < sizeof(ip4); i++ )
    for( intptr_t j = 1; j >= 0; j-- ) b[i * 2 + 1 - j] = "0123456789ABCDEF"[(ip4 >> (i * 8 + j * 4)) & 0xF];
  b[i * 2] = '\0';
  return utf8::plane0(b);
}
//------------------------------------------------------------------------------
uint32_t Logger::indexToIp4Addr(const utf8::String & index)
{
//#ifndef NDEBUG
//  fprintf(stderr,"index: %s, %X\n",(const char *) index.getOEMString(),(uint32_t) utf8::str2Int(index,16));
//#endif
  return be32toh((uint32_t) utf8::str2Int(index,16));
}
//------------------------------------------------------------------------------
utf8::String Logger::getIPFilter(const utf8::String & text)
{
  utf8::String filter;
  utf8::String::Iterator i(text);
  uintptr_t c[3], prev = '\0';
  for(;;){
    if( i.eof() ) break;
    c[0] = i.getLowerChar();
    c[1] = (i + 1).getLowerChar();
    c[2] = (i + 2).getLowerChar();
    if( (prev == '\0' || prev == '(' || prev == ')' || (utf8::getC1Type(prev) & C1_SPACE) != 0) &&
        (c[0] == 's' && c[1] == 'r' && c[2] == 'c') ||
        (c[0] == 'd' && c[1] == 's' && c[2] == 't') ){
      utf8::String sd("st_" + utf8::String(i,i + 3).lower() + "_ip");
      i += 3;
      while( !i.eof() && i.isSpace() ) i.next();
      if( (i - 1).isSpace() ){
        utf8::String::Iterator j(i), net(j);
        bool isNetwork = false;
        while( !j.eof() && !j.isSpace() && j.getChar() != ')' ){
          if( j.getChar() == '/' ){
  	    isNetwork = true;
	    net = j;
	  }
	  j.next();
        }
        ksock::SockAddr addr;
        if( isNetwork ){
          uint32_t inet = addr.resolveName(utf8::String(i,net)).addr4_.sin_addr.s_addr;
	  uint32_t mask = ~(~uint32_t(0) << utf8::str2Int(utf8::String(net + 1,j)));
          filter += "(" + sd + " >= '" +
  	    ip4AddrToIndex(inet & mask) + "' AND " + sd + " <= '" +
	    ip4AddrToIndex(inet | ~mask) + "')"
	  ;
        }
        else {
          filter += sd + " = '" +
	    ip4AddrToIndex(addr.resolveName(utf8::String(i,j)).addr4_.sin_addr.s_addr) +
	    "'"
	  ;
        }
        i = j;
      }
      prev = (i - 1).getChar();
    }
    else {
      filter += utf8::String(i,i + 1);
      prev = i.getChar();
      i++;
    }
  }
  if( filter.strlen() > 0 ) filter = " AND (" + filter + ") ";
#ifndef NDEBUG
  fprintf(stderr,"filter:%s\n",(const char *) filter.getOEMString());
#endif
  return filter;
}
//------------------------------------------------------------------------------
utf8::String Logger::resolveAddr(AutoPtr<Statement> st[3],bool resolveDNSNames,uint32_t ip4,bool numeric)
{
  AutoPtr<DNSCacheEntry> addr(newObject<DNSCacheEntry>());
  addr->addr4_.sin_addr.s_addr = ip4;
  addr->addr4_.sin_port = 0;
  addr->addr4_.sin_family = PF_INET;
  if( numeric ) return addr->resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV);
  AutoLock<InterlockedMutex> lock(dnsMutex_);
  DNSCacheEntry * pAddr = addr;
  dnsCache_.insert(addr,false,false,&pAddr);
  if( pAddr == addr ){
    utf8::String name;
l1: if( !st[stSel]->prepared() )
      st[stSel]->text("SELECT st_name from INET_DNS_CACHE WHERE st_ip = :ip")->prepare();
    if( st[stSel]->paramAsString("ip",ip4AddrToIndex(ip4))->execute()->fetch() ){
      st[stSel]->fetchAll();
      name = st[stSel]->valueAsString("st_name");
      dnsCacheHitCount_++;
    }
    else {
      if( resolveDNSNames ){
        name = pAddr->resolveAddr();
        if( !st[stIns]->prepared() )
          st[stIns]->text("INSERT INTO INET_DNS_CACHE (st_ip,st_name) VALUES (:ip,:name)")->prepare();
        st[stIns]->
          paramAsString("ip",ip4AddrToIndex(ip4))->
          paramAsString("name",name);
	bool dup = false;
	try {
  	  st[stIns]->execute();
	}
        catch( ExceptionSP & e ){
          if( !e->searchCode(isc_no_dup,ER_DUP_ENTRY) ) throw;
	  dup = true;
	}
	if( dup ) goto l1;
      }
      else {
        name = pAddr->resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV);
      }
      dnsCacheMissCount_++;
    }
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
  if( traf >= 1024u * 1024u * 1024u ){
    t2 = 1024u * 1024u * 1024u;
    postfix = "G";
  }
  else if( traf >= uintmax_t(1024u) * 1024u * 1024u * 1024u ){
    t2 = uintmax_t(1024u) * 1024u * 1024u * 1024u;
    postfix = "T";
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
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Logger::BPFTThread::~BPFTThread()
{
}
//------------------------------------------------------------------------------
Logger::BPFTThread::BPFTThread(Logger & logger,const utf8::String & section,const utf8::String & sectionName) :
  logger_(&logger), section_(section), sectionName_(sectionName),
  database_(Database::newDatabase(logger_->config_.ptr())),
  statement_(database_->newAttachedStatement()),
  statement2_(database_->newAttachedStatement()),
  statement3_(database_->newAttachedStatement()),
  statement4_(database_->newAttachedStatement()),
  statement5_(database_->newAttachedStatement()),
  statement6_(database_->newAttachedStatement())
{
  stFileStat_[stSel].ptr(database_->newAttachedStatement());
  stFileStat_[stIns].ptr(database_->newAttachedStatement());
  stFileStat_[stUpd].ptr(database_->newAttachedStatement());
  stDNSCache_[stSel].ptr(database_->newAttachedStatement());
  stDNSCache_[stIns].ptr(database_->newAttachedStatement());
  stDNSCache_[stUpd].ptr(database_->newAttachedStatement());
}
//------------------------------------------------------------------------------
void Logger::BPFTThread::threadExecute()
{
  ksock::APIAutoInitializer ksockAPIAutoInitializer;
  database_->attach();
  parseBPFTLogFile();
  writeBPFTHtmlReport();
  database_->detach();
}
//------------------------------------------------------------------------------
void Logger::BPFTThread::clearBPFTCache()
{
  struct tm curTimeBTHour = curTime_;
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
  bool clearCache = logger_->config_->valueByPath(section_ + ".html_report.clear_cache",false);
  statement_->text(
    utf8::String("DELETE FROM INET_BPFT_STAT_CACHE WHERE ") +
    (clearCache ? "" :
      "st_if = :if AND ("
      "  (st_bt = :BTYear AND st_et = :ETYear) OR"
      "  (st_bt = :BTMon  AND st_et = :ETMon) OR"
      "  (st_bt = :BTDay AND st_et = :ETDay) OR"
      "  (st_bt = :BTHour AND st_et = :ETHour)"
      ")"
    ) +
    " AND st_filter_hash = :hash AND st_threshold = :threshold"
  )->prepare();
  if( !clearCache ){
    statement_->
      paramAsString("if",sectionName_)->
      paramAsMutant("BTYear",time2tm(tm2Time(curTimeBTYear) - getgmtoffset()))->
      paramAsMutant("ETYear",time2tm(tm2Time(curTimeETYear) - getgmtoffset()))->
      paramAsMutant("BTMon",time2tm(tm2Time(curTimeBTMon) - getgmtoffset()))->
      paramAsMutant("ETMon",time2tm(tm2Time(curTimeETMon) - getgmtoffset()))->
      paramAsMutant("BTMon",time2tm(tm2Time(curTimeBTDay) - getgmtoffset()))->
      paramAsMutant("ETMon",time2tm(tm2Time(curTimeETDay) - getgmtoffset()))->
      paramAsMutant("BTHour",time2tm(tm2Time(curTimeBTHour) - getgmtoffset()))->
      paramAsMutant("ETHour",time2tm(tm2Time(curTimeETHour) - getgmtoffset()));
  }
  statement_->paramAsMutant("hash",filterHash_)->
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
  );
}
//------------------------------------------------------------------------------
void Logger::BPFTThread::getBPFTCached(Statement * pStatement,Table<Mutant> * pResult,uintmax_t * pDgramBytes,uintmax_t * pDataBytes)
{
  intptr_t i, cycle = 0;
  bool useCache = (bool) logger_->config_->valueByPath(section_ + ".html_report.use_cache",true);
  bool updateCache = true;
  for(;;){
    if( pStatement == statement_ ){
      if( !statement2_->prepared() ){
        statement2_->text(
          "SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(statement2_->database()) != NULL ? " SQL_NO_CACHE" : "") +
	  " * FROM ("
          "  SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(statement2_->database()) != NULL ? " SQL_NO_CACHE" : "") +
          "    st_src_ip, st_dgram_bytes as SUM1, st_data_bytes as SUM2 "
	  "  FROM INET_BPFT_STAT_CACHE "
          "  WHERE"
          "    st_if = :if AND st_bt = :BT AND st_et = :ET AND st_filter_hash = :hash AND st_threshold = :threshold" +
          "  ORDER BY SUM1, st_src_ip "
	  ") AS A "
	  "WHERE A.st_src_ip = '" + ip4AddrToIndex(0xFFFFFFFF) + "' OR A.SUM1 >= :threshold"
        )->prepare();
        statement2_->paramAsString("if",sectionName_)->
          paramAsMutant("hash",filterHash_)->
          paramAsMutant("threshold",minSignificantThreshold_);
      }
      statement2_->paramAsMutant("BT",pStatement->paramAsMutant("BT"));
      statement2_->paramAsMutant("ET",pStatement->paramAsMutant("ET"));
      if( useCache && statement2_->execute()->fetch() ){
        statement2_->fetchAll();
        pStatement = statement2_;
        updateCache = false;
#ifndef NDEBUG
        if( logger_->verbose_ ) fprintf(stderr,
          "bpft cache %s: %s %s\n",
	  cycle == 0 ? "hit" : "miss",
          (const char *) utf8::time2Str((uint64_t) pStatement->paramAsMutant("BT") + getgmtoffset()).getOEMString(),
          (const char *) utf8::time2Str((uint64_t) pStatement->paramAsMutant("ET") + getgmtoffset()).getOEMString()
        );
#endif
      }
    }
    else if( pStatement == statement6_ ){
      if( !statement3_->prepared() ){
        statement3_->text(
          "SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(statement3_->database()) != NULL ? " SQL_NO_CACHE" : "") +
          "  st_src_ip, st_dgram_bytes as SUM1, st_data_bytes as SUM2 "
          "FROM INET_BPFT_STAT_CACHE "
          "WHERE"
          "  st_if = :if AND st_bt = :BT AND st_et = :ET AND"
	  "  st_filter_hash = :hash AND st_threshold = :threshold AND st_src_ip = :host"
        )->prepare();
        statement3_->paramAsString("if",sectionName_)->
          paramAsMutant("hash",filterHash_)->
          paramAsMutant("threshold",minSignificantThreshold_);
      }
      statement3_->paramAsMutant("BT",pStatement->paramAsMutant("BT"));
      statement3_->paramAsMutant("ET",pStatement->paramAsMutant("ET"));
      statement3_->paramAsString("host",pStatement->paramAsString("host"));
      if( useCache && statement3_->execute()->fetch() ){
        statement3_->fetchAll();
        pStatement = statement3_;
        updateCache = false;
      }
    }
    else {
      assert( 0 );
    }
    if( !updateCache || cycle > 0 ) break;
    if( !pStatement->execute()->fetch() ) break;
    pStatement->fetchAll();
    if( !statement4_->prepared() ){
      statement4_->text(
        "INSERT INTO INET_BPFT_STAT_CACHE ("
        "  st_if, st_bt, st_et, st_src_ip, st_dst_ip, st_filter_hash, st_threshold, st_dgram_bytes, st_data_bytes"
        ") VALUES ("
        "  :st_if, :st_bt, :st_et, :st_src_ip, :st_dst_ip, :st_filter_hash, :st_threshold, :st_dgram_bytes, :st_data_bytes"
        ")"
      )->prepare();
      statement4_->paramAsString("st_if",sectionName_)->
        paramAsMutant("st_filter_hash",filterHash_)->
        paramAsMutant("st_threshold",minSignificantThreshold_);
    }
    statement4_->paramAsMutant("st_bt",pStatement->paramAsMutant("BT"));
    statement4_->paramAsMutant("st_et",pStatement->paramAsMutant("ET"));
//    bool sign = true;
    for( i = pStatement->rowCount() - 1; i >= 0; i-- ){
      pStatement->selectRow(i);
      statement4_->paramAsString("st_src_ip",pStatement->valueAsString("st_ip"));
      statement4_->paramAsString("st_dst_ip",pStatement->valueAsString("st_ip"));
      uintmax_t sum1 = pStatement->valueAsMutant("SUM1"), sum2 = pStatement->valueAsMutant("SUM2");
      if( pStatement == statement_ && sum1 < minSignificantThreshold_ /*&& sign*/ ){
        statement4_->paramAsString("st_src_ip",ip4AddrToIndex(0xFFFFFFFF));
        statement4_->paramAsString("st_dst_ip",ip4AddrToIndex(0xFFFFFFFF));
        statement4_->paramAsMutant("st_dgram_bytes",pStatement->sum("SUM1",0,i));
        statement4_->paramAsMutant("st_data_bytes",pStatement->sum("SUM2",0,i));
        statement4_->execute();
//        sign = false;
	break;
      }
      statement4_->paramAsMutant("st_dgram_bytes",sum1);
      statement4_->paramAsMutant("st_data_bytes",sum2);
      statement4_->execute();
    }
    cycle++;
  }
  if( pResult == NULL ){
    *pDgramBytes = pStatement->sum("SUM1");
    *pDataBytes = pStatement->sum("SUM2");
  }
  else {
//    k = j = -1;
//    for( i = pStatement->rowCount() - 1; i >= 0; i-- ){
//      pStatement->selectRow(i);
//      if( (uintmax_t) pStatement->valueAsMutant("SUM1") < minSignificantThreshold_ ) j = i;
//      if( utf8::String(pStatement->valueAsMutant("st_ip")).strcmp(ip4AddrToIndex(0xFFFFFFFF)) == 0 ) k = i;
//      if( i >= 0 && k >= 0 ) break;
//    }
//    pStatement->unload(*pResult,j = j >= 0 ? j + 1 : 0);
//    if( k >= 0 && k <= j ){
//      pResult->addRow();
//      pStatement->unloadRow(*pResult,k);
//    }
    pStatement->unload(*pResult);
  }
}
//------------------------------------------------------------------------------
void Logger::BPFTThread::writeBPFTHtmlReport(intptr_t level,const struct tm * rt)
{
  utf8::String filter;
  struct tm beginTime, beginTime2, endTime;
  Mutant m0, m1, m2;
  AsyncFile f;
  if( level == rlYear ){
    if( !(bool) logger_->config_->valueByPath(section_ + ".html_report.enabled",true) ) return;
    if( logger_->verbose_ ) fprintf(stderr,"\n");
    ellapsed_ = getlocaltimeofday();
    minSignificantThreshold_ = logger_->config_->valueByPath(section_ + ".html_report.min_significant_threshold",0);
    filter_ = getIPFilter(logger_->config_->textByPath(section_ + ".html_report.filter"));
    filterHash_ = filter_.hash_ll(false);
    curTime_ = time2tm(ellapsed_);
    resolveDNSNames_ = logger_->config_->valueByPath("macroscope.bpft.resolve_dns_names",true);
    database_->start();
    clearBPFTCache();
    statement_->text(
      "SELECT MIN(st_start) AS BT, MAX(st_start) AS ET "
      "FROM INET_BPFT_STAT WHERE st_if = :st_if"
    )->prepare()->paramAsString("st_if",sectionName_)->execute()->fetchAll();
    if( statement_->fieldIndex("BT") < 0 || statement_->fieldIndex("ET") < 0 || statement_->rowCount() == 0 )
      goto l1;
    beginTime = time2tm((uint64_t) statement_->valueAsMutant("BT") + getgmtoffset());
    endTime = time2tm((uint64_t) statement_->valueAsMutant("ET") + getgmtoffset());
    statement_->text(
      "SELECT"
      "  A.*"
      "FROM ("
      "  SELECT"
      "    B.st_ip AS st_ip, SUM(B.SUM1) AS SUM1, SUM(B.SUM2) AS SUM2"
      "  FROM ("
      "      SELECT"
      "        st_dst_ip AS st_ip, SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2"
      "      FROM"
      "        INET_BPFT_STAT"
      "      WHERE "
      "        st_if = :st_if AND"
      "        st_start >= :BT AND st_start <= :ET" +
      filter_ +
      "      GROUP BY st_dst_ip"
      "    UNION ALL"
      "      SELECT"
      "        st_src_ip AS st_ip, SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2"
      "      FROM"
      "        INET_BPFT_STAT"
      "      WHERE "
      "        st_if = :st_if AND"
      "        st_start >= :BT AND st_start <= :ET" +
      filter_ +
      "      GROUP BY st_src_ip"
      "  ) AS B "
      "  GROUP BY B.st_ip"
      ") AS A "
      "ORDER BY A.SUM1"
    );
    statement_->prepare()->paramAsString("st_if",sectionName_);
    statement6_->text(
      "  SELECT"
      "    B.st_ip AS st_ip, SUM(B.SUM1) AS SUM1, SUM(B.SUM2) AS SUM2"
      "  FROM ("
      "      SELECT"
      "        st_dst_ip AS st_ip, SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2"
      "      FROM"
      "        INET_BPFT_STAT"
      "      WHERE "
      "        st_if = :st_if AND"
      "        st_start >= :BT AND st_start <= :ET AND"
      "        st_dst_ip = :host" +
      filter_ +
      "      GROUP BY st_dst_ip"
      "    UNION ALL"
      "      SELECT"
      "        st_src_ip AS st_ip, SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2"
      "      FROM"
      "        INET_BPFT_STAT"
      "      WHERE "
      "        st_if = :st_if AND"
      "        st_start >= :BT AND st_start <= :ET AND"
      "        st_src_ip = :host" +
      filter_ +
      "      GROUP BY st_src_ip"
      "  ) AS B "
      "  GROUP BY B.st_ip"
    );
    statement6_->prepare();
    statement6_->paramAsString("st_if",sectionName_);
    htmlDir_ = excludeTrailingPathDelimiter(logger_->config_->valueByPath(section_ + ".html_report.directory"));
  }
  else {
    beginTime = endTime = *rt;
  }
  int * pi, sv, av;
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
  }
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
  while( tm2Time(endTime) >= tm2Time(beginTime) ){
    beginTime2 = beginTime;
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
    statement_->paramAsMutant("BT",time2tm(tm2Time(beginTime) - getgmtoffset()));
    statement_->paramAsMutant("ET",time2tm(tm2Time(endTime) - getgmtoffset()));
    Vector<Table<Mutant> > table;
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
    getBPFTCached(statement_,&table[0]);
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
        statement_->paramAsMutant("BT",time2tm(tm2Time(beginTime) - getgmtoffset()));
        statement_->paramAsMutant("ET",time2tm(tm2Time(endTime) - getgmtoffset()));
        getBPFTCached(statement_,&table[*pi + av]);
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
        utf8::int2Str(colCount * 2 + 3) + "\" ALIGN=left nowrap>\n" +
        "    <A HREF=\""
      ;
      switch( level ){
        case rlYear :
          f <<
            "bpft-" + sectionName_ +
            utf8::String::print("-traf-by-%04d.html",endTime.tm_year + 1900) + "\">" +
            utf8::int2Str(endTime.tm_year + 1900)
          ;
          break;
        case rlMon :
	  f <<
            "bpft-" + sectionName_ +
  	    utf8::String::print("-traf-by-%04d%02d.html",endTime.tm_year + 1900,endTime.tm_mon + 1) + "\">" +
	    utf8::String::print("%02d.%04d",endTime.tm_mon + 1,endTime.tm_year + 1900)
	  ;
          break;
        case rlDay :
	  f <<
            "bpft-" + sectionName_ +
  	    utf8::String::print("-traf-by-%04d%02d%02d.html",endTime.tm_year + 1900,endTime.tm_mon + 1,endTime.tm_mday) + "\">" +
	    utf8::String::print("%02d.%02d.%04d",endTime.tm_mday,endTime.tm_mon + 1,endTime.tm_year + 1900)
	  ;
          break;
        default    :
          assert( 0 );
      }
      f <<
        "\n"
        "    </A>\n"
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
        uint32_t ip4 = logger_->indexToIp4Addr(table[0](i,"st_src_ip"));
        f <<
          "<TR>\n"
          "  <TH ALIGN=left BGCOLOR=\"" + logger_->getDecor("body.host",section_) + "\" nowrap>\n"
          "    <A HREF=\"http://" + logger_->resolveAddr(stDNSCache_,resolveDNSNames_,ip4) + "\">\n" +
          logger_->resolveAddr(stDNSCache_,resolveDNSNames_,ip4) + (
	    logger_->resolveAddr(stDNSCache_,resolveDNSNames_,ip4).strcmp(logger_->resolveAddr(stDNSCache_,resolveDNSNames_,ip4,true)) == 0 ?
	      utf8::String() :
	      " (" + logger_->resolveAddr(stDNSCache_,resolveDNSNames_,ip4,true) + ")"
	  ) + "\n" +
          "    </A>\n"
          "  </TH>\n"
          "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("body.data",section_) + "\" nowrap>\n" +
          formatTraf(table[0](i,"SUM2"),table[0].sum("SUM1")) + "\n"
          "  </TH>\n"
          "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("body.dgram",section_) + "\" nowrap>\n" +
          formatTraf(table[0](i,"SUM1"),table[0].sum("SUM1")) + "\n"
          "  </TH>\n"
        ;
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
            statement6_->paramAsMutant("BT",time2tm(tm2Time(beginTime) - getgmtoffset()));
            statement6_->paramAsMutant("ET",time2tm(tm2Time(endTime) - getgmtoffset()));
            statement6_->paramAsMutant("host",table[0](i,"st_src_ip"));
            uintmax_t sum1, sum2;
            getBPFTCached(statement6_,NULL,&sum1,&sum2);
            f <<
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
        f << "</TR>\n";
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
      bool nextLevel = !(bool) logger_->config_->valueByPath(section_ + ".html_report.refresh_only_current",false);
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
    }
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
  filter = logger_->config_->textByPath(section_ + ".html_report.filter");
  {
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
    f <<
      "Interface: " + sectionName_ + "<BR>\n" +
      "Filter: <B>" + filter + "</B>, hash: " + utf8::int2Str(filterHash_) + "<BR>\n" +
      "FilterSQL: <B>" + filter_ + "</B><BR>\n" +
      "DNS cache size: " + utf8::int2Str((uintmax_t) logger_->dnsCache_.count()) + ", "
      "hit: " + utf8::int2Str(logger_->dnsCacheHitCount_) + ", " +
      "hit ratio: " + hitRatio + ", " +
      "miss: " + utf8::int2Str(logger_->dnsCacheMissCount_) + ", " +
      "miss ratio: " + missRatio +
      "<BR>\n"
    ;
  }
  logger_->writeHtmlTail(f,ellapsed_);
  f.resize(f.tell());
  if( level == rlYear ){
l1: if( (bool) logger_->config_->valueByPath(section_ + ".html_report.update_cache",true) )
      database_->commit();
    else
      database_->rollback();
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
  int64_t offset = Logger::fetchLogFileLastOffset(stFileStat_,flog.fileName());
  if( flog.seekable() ) flog.seek(offset);
  int64_t lineNo = 0, tma = 0;
  int64_t cl = getlocaltimeofday();
  union {
    BPFTHeader header;
    BPFTHeader32 header32;
  };
  statement_->text(
    "INSERT INTO INET_BPFT_STAT ("
    "  st_if,st_start,st_src_ip,st_dst_ip,st_ip_proto,st_src_port,st_dst_port,st_dgram_bytes,st_data_bytes"
    ") VALUES ("
    "  :st_if,:st_start,:st_src_ip,:st_dst_ip,:st_ip_proto,:st_src_port,:st_dst_port,:st_dgram_bytes,:st_data_bytes"
    ")"
  );
  statement_->prepare();
  statement_->paramAsString("st_if",sectionName_);
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
            "'" + ip4AddrToIndex(entries32[i].srcIp_.s_addr) + "'," +
            "'" + ip4AddrToIndex(entries32[i].dstIp_.s_addr) +"'," +
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
            "'" + ip4AddrToIndex(entries[i].srcIp_.s_addr) + "'," +
            "'" + ip4AddrToIndex(entries[i].dstIp_.s_addr) +"'," +
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
          statement5_->text(text)->execute();
          executed = true;
        }
      }
      assert( executed );
    }
    else for( intptr_t i = entriesCount - 1; i >= 0; i-- ){
      if( log32bitOsCompatible ){
        statement_->paramAsMutant("st_src_ip",ip4AddrToIndex(entries32[i].srcIp_.s_addr));
        statement_->paramAsMutant("st_dst_ip",ip4AddrToIndex(entries32[i].dstIp_.s_addr));
        statement_->paramAsMutant("st_ip_proto",entries32[i].ipProtocol_);
        statement_->paramAsMutant("st_src_port",entries32[i].srcPort_);
        statement_->paramAsMutant("st_dst_port",entries32[i].dstPort_);
        statement_->paramAsMutant("st_dgram_bytes",entries32[i].dgramSize_);
        statement_->paramAsMutant("st_data_bytes",entries32[i].dataSize_);
      }
      else {
        statement_->paramAsMutant("st_src_ip",ip4AddrToIndex(entries[i].srcIp_.s_addr));
        statement_->paramAsMutant("st_dst_ip",ip4AddrToIndex(entries[i].dstIp_.s_addr));
        statement_->paramAsMutant("st_ip_proto",entries[i].ipProtocol_);
        statement_->paramAsMutant("st_src_port",entries[i].srcPort_);
        statement_->paramAsMutant("st_dst_port",entries[i].dstPort_);
        statement_->paramAsMutant("st_dgram_bytes",entries[i].dgramSize_);
        statement_->paramAsMutant("st_data_bytes",entries[i].dataSize_);
      }
      statement_->paramAsMutant("st_start",start);
      statement_->execute();
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
