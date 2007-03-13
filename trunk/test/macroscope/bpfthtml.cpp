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
  uintptr_t shift = sizeof(ip4) * 8 - 4, i;
  for( i = 0; i < sizeof(ip4) * 2; i++, shift -= 4 ) b[i] = "0123456789ABCDEF"[(ip4 >> shift) & 0xF];
  b[i] = '\0';
  return utf8::plane0(b);
}
//------------------------------------------------------------------------------
uint32_t Logger::indexToIp4Addr(const utf8::String & index)
{
  static const uint8_t m[2] = { '0', 'A' - 10 };
  uint32_t ip4 = 0, a;
  uintptr_t shift = sizeof(ip4) * 8 - 4, i;
  for( i = 0; i < sizeof(ip4) * 2; i++, shift -= 4 ){
    a = index.c_str()[i];
    a -= m[a > '9'];
    ip4 |= a << shift;
  }
  return ip4;
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
utf8::String Logger::resolveAddr(uint32_t ip4,bool numeric)
{
  AutoPtr<DNSCacheEntry> addr(newObject<DNSCacheEntry>());
  addr->addr4_.sin_addr.s_addr = ip4;
  addr->addr4_.sin_port = 0;
  addr->addr4_.sin_family = PF_INET;
  if( numeric ) return addr->resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV);
  DNSCacheEntry * pAddr = addr;
  dnsCache_.insert(addr,false,false,&pAddr);
  if( pAddr == addr ){
    utf8::String name;
    if( !stDNSCacheSel_->prepared() )
      stDNSCacheSel_->text("SELECT st_name from INET_DNS_CACHE WHERE st_ip = :ip")->prepare();
    if( stDNSCacheSel_->paramAsString("ip",ip4AddrToIndex(ip4))->execute()->fetch() ){
      stDNSCacheSel_->fetchAll();
      name = stDNSCacheSel_->valueAsString("st_name");
      dnsCacheHitCount_++;
    }
    else {
      if( resolveDNSNames_ ){
        name = pAddr->resolveAddr();
        if( !stDNSCacheIns_->prepared() )
          stDNSCacheIns_->text("INSERT INTO INET_DNS_CACHE (st_ip,st_name) VALUES (:ip,:name)")->prepare();
        stDNSCacheIns_->
          paramAsString("ip",ip4AddrToIndex(ip4))->
          paramAsString("name",name)->
          execute();
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
void Logger::getBPFTCached(Statement * pStatement,Table<Mutant> * pResult,uintmax_t * pDgramBytes,uintmax_t * pDataBytes)
{
  intptr_t i, cycle = 0;
  bool updateCache = true;
  for(;;){
    if( pStatement == statement_ ){
      if( !statement2_->prepared() ){
        statement2_->text(
          "SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL ? " SQL_NO_CACHE" : "") +
	  " * FROM ("
          "  SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL ? " SQL_NO_CACHE" : "") +
          "    st_ip, st_dgram_bytes as SUM1, st_data_bytes as SUM2 "
	  "  FROM INET_BPFT_STAT_CACHE "
          "  WHERE"
          "    st_if = :if AND st_bt = :BT AND st_et = :ET "
          "  ORDER BY SUM1, st_ip "
	  ") AS A "
	  "WHERE A.st_ip = '" + ip4AddrToIndex(0xFFFFFFFF) + "' OR A.SUM1 >= :threshold"
        )->prepare();
        statement2_->paramAsString("if",sectionName_);
      }
      statement2_->paramAsMutant("BT",pStatement->paramAsMutant("BT"));
      statement2_->paramAsMutant("ET",pStatement->paramAsMutant("ET"));
      statement2_->paramAsMutant("threshold",minSignificantThreshold_);
      if( statement2_->execute()->fetch() ){
        statement2_->fetchAll();
        pStatement = statement2_;
        updateCache = false;
#ifndef NDEBUG
        if( verbose_ ) fprintf(stderr,
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
          "SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL ? " SQL_NO_CACHE" : "") +
	        "  st_ip, st_dgram_bytes as SUM1, st_data_bytes as SUM2 "
	        "FROM INET_BPFT_STAT_CACHE "
	        "WHERE"
	        "  st_if = :if AND st_bt = :BT AND st_et = :ET AND st_ip = :host"
        )->prepare();
        statement3_->paramAsString("if",sectionName_);
      }
      statement3_->paramAsMutant("BT",pStatement->paramAsMutant("BT"));
      statement3_->paramAsMutant("ET",pStatement->paramAsMutant("ET"));
      statement3_->paramAsString("host",pStatement->paramAsString("host"));
      if( statement3_->execute()->fetch() ){
        statement3_->fetchAll();
        pStatement = statement3_;
        updateCache = false;
      }
    }
    else {
      assert( 0 );
    }
    if( !updateCache ) break;
    if( !pStatement->execute()->fetch() ) break;
    pStatement->fetchAll();
    if( !statement4_->prepared() ){
      statement4_->text(
        "INSERT INTO INET_BPFT_STAT_CACHE ("
        "  st_if, st_bt, st_et, st_ip, st_dgram_bytes, st_data_bytes"
        ") VALUES ("
        "  :st_if, :st_bt, :st_et, :st_ip, :st_dgram_bytes, :st_data_bytes"
        ")"
      )->prepare();
      statement4_->paramAsString("st_if",sectionName_);
    }
    statement4_->paramAsMutant("st_bt",pStatement->paramAsMutant("BT"));
    statement4_->paramAsMutant("st_et",pStatement->paramAsMutant("ET"));
    bool sign = true;
    for( i = pStatement->rowCount() - 1; i >= 0; i-- ){
      pStatement->selectRow(i);
      statement4_->paramAsString("st_ip",pStatement->valueAsString("st_ip"));
      uintmax_t sum1 = pStatement->valueAsMutant("SUM1"), sum2 = pStatement->valueAsMutant("SUM2");
      if( pStatement == statement_ && sum1 < minSignificantThreshold_ && sign ){
        statement4_->paramAsString("st_ip",ip4AddrToIndex(0xFFFFFFFF));
        statement4_->paramAsMutant("st_dgram_bytes",pStatement->sum("SUM1",0,i));
        statement4_->paramAsMutant("st_data_bytes",pStatement->sum("SUM2",0,i));
        statement4_->execute();
        sign = false;
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
void Logger::clearBPFTCache()
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
  bool clearCache = config_->valueByPath(section_ + ".html_report.clear_cache",false);
  statement_->text(
    utf8::String("DELETE FROM INET_BPFT_STAT_CACHE") +
    (clearCache ? "" :
      " WHERE"
      " st_if = :if AND ("
      "  (st_bt = :BTYear AND st_et = :ETYear) OR"
      "  (st_bt = :BTMon  AND st_et = :ETMon) OR"
      "  (st_bt = :BTDay AND st_et = :ETDay) OR"
      "  (st_bt = :BTHour AND st_et = :ETHour)"
      ")"
    )
  )->prepare();
  if( !clearCache ){
    statement_->
      paramAsString("if",sectionName_)->
      paramAsMutant("BTYear",time2tm(tm2Time(curTimeBTYear) - getgmtoffset()))->
      paramAsMutant("ETYear",time2tm(tm2Time(curTimeETYear) - getgmtoffset()))->
      paramAsMutant("BTMon",time2tm(tm2Time(curTimeBTMon) - getgmtoffset()))->
      paramAsMutant("ETMon",time2tm(tm2Time(curTimeETMon) - getgmtoffset()))->
      paramAsMutant("BTHour",time2tm(tm2Time(curTimeBTHour) - getgmtoffset()))->
      paramAsMutant("ETHour",time2tm(tm2Time(curTimeETHour) - getgmtoffset()));
  }
  statement_->execute();
  if( verbose_ ) fprintf(stderr,
    "bpft cache cleared for:\n  %s %s\n  %s %s\n  %s %s\n",
    (const char *) utf8::tm2Str(curTimeBTYear).getOEMString(),
    (const char *) utf8::tm2Str(curTimeETYear).getOEMString(),
    (const char *) utf8::tm2Str(curTimeBTMon).getOEMString(),
    (const char *) utf8::tm2Str(curTimeETMon).getOEMString(),
    (const char *) utf8::tm2Str(curTimeBTHour).getOEMString(),
    (const char *) utf8::tm2Str(curTimeETHour).getOEMString()
  );
}
//------------------------------------------------------------------------------
utf8::String Logger::getDecor(const utf8::String & dname)
{
  utf8::String defDecor(config_->textByPath(section_ + "..decoration.colors." + dname));
  return config_->textByPath( section_ + ".html_report.decoration.colors." + dname,defDecor);
}
//------------------------------------------------------------------------------
void Logger::writeBPFTHtmlReport(intptr_t level,const struct tm * rt)
{
  struct tm beginTime, beginTime2, endTime;
  Mutant m0, m1, m2;
  AsyncFile f;
  if( level == rlYear ){
    dnsCacheHitCount_ = 0;
    dnsCacheMissCount_ = 0;
    if( !(bool) config_->valueByPath(section_ + ".html_report.enabled",true) ) return;
    if( verbose_ ) fprintf(stderr,"\n");
    resolveDNSNames_ = config_->valueByPath(section_ + ".html_report.resolve_dns_names",false);
    dnsCacheSize_ = config_->valueByPath(section_ + ".html_report.dns_cache_size",0);
    minSignificantThreshold_ = config_->valueByPath(section_ + ".html_report.min_significant_threshold",0);
    useGateway_ = config_->isValueByPath(section_ + ".html_report.gateway");
    if( useGateway_ ){
      gateway_.resolveName(config_->valueByPath(section_ + ".html_report.gateway"));
      if( verbose_ ) fprintf(stderr,"\ngateway resolved as %s, in db %s\n",
        (const char *) gateway_.resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV).getOEMString(),
        (const char *) ip4AddrToIndex(gateway_.addr4_.sin_addr.s_addr).getOEMString()
      );
    }
    utf8::String excludeSrcIp, excludeDstIp;
    if( config_->isValueByPath(section_ + ".html_report.exclude") ){
      utf8::String ex(config_->valueByPath(section_ + ".html_report.exclude"));
      for( intptr_t j = enumStringParts(ex) - 1, i = j; i >= 0; i-- ){
        if( j == i ){
          excludeSrcIp = " AND (";
          excludeDstIp = " AND (";
        }
        utf8::String index(ip4AddrToIndex(
          ksock::SockAddr().resolveName(stringPartByNo(ex,i)).addr4_.sin_addr.s_addr
        ));
        excludeSrcIp += "st_src_ip <> '" + index + "'";
        excludeDstIp += "st_dst_ip <> '" + index + "'";
        if( i > 0 ){
          excludeSrcIp += " AND ";
          excludeDstIp += " AND ";
        }
        else {
          excludeSrcIp += ") ";
          excludeDstIp += ") ";
        }
      }
    }
    curTime_ = time2tm(getlocaltimeofday());
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
      utf8::String(useGateway_ ? " AND st_src_ip = :gateway" : "") +
      excludeDstIp +
      "      GROUP BY st_dst_ip"
      "    UNION ALL"
      "      SELECT"
      "        st_src_ip AS st_ip, SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2"
      "      FROM"
      "        INET_BPFT_STAT"
      "      WHERE "
      "        st_if = :st_if AND"
      "        st_start >= :BT AND st_start <= :ET" +
      utf8::String(useGateway_ ? " AND st_dst_ip = :gateway" : "") +
      excludeSrcIp +
      "      GROUP BY st_src_ip"
      "  ) AS B "
      "  GROUP BY B.st_ip"
      ") AS A "
      "ORDER BY A.SUM1"
    );
    statement_->prepare()->paramAsString("st_if",sectionName_);
    if( useGateway_ )
      statement_->paramAsMutant("gateway",ip4AddrToIndex(gateway_.addr4_.sin_addr.s_addr));
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
      "        st_start >= :BT AND st_start <= :ET AND" +
      utf8::String(useGateway_ ? " st_src_ip = :gateway AND" : "") +
      "        st_dst_ip = :host"
      "      GROUP BY st_dst_ip"
      "    UNION ALL"
      "      SELECT"
      "        st_src_ip AS st_ip, SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2"
      "      FROM"
      "        INET_BPFT_STAT"
      "      WHERE "
      "        st_if = :st_if AND"
      "        st_start >= :BT AND st_start <= :ET AND" +
      utf8::String(useGateway_ ? " st_dst_ip = :gateway AND" : "") +
      "        st_src_ip = :host"
      "      GROUP BY st_src_ip"
      "  ) AS B "
      "  GROUP BY B.st_ip"
    );
    statement6_->prepare();
    statement6_->paramAsString("st_if",sectionName_);
    if( useGateway_ )
      statement6_->paramAsMutant("gateway",ip4AddrToIndex(gateway_.addr4_.sin_addr.s_addr));
    htmlDir_ = excludeTrailingPathDelimiter(config_->valueByPath(section_ + ".html_report.directory"));
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
        config_->valueByPath(section_ + ".html_report.index_file_name","index.html")
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
  m0 = config_->valueByPath(section_ + ".html_report.directory_mode",0755);
  m1 = config_->valueByPath(section_ + ".html_report.directory_user",ksys::getuid());
  m2 = config_->valueByPath(section_ + ".html_report.directory_group",ksys::getgid());
  chModOwn(htmlDir_,m0,m1,m2);
  m0 = config_->valueByPath(section_ + ".html_report.file_mode",0644);
  m1 = config_->valueByPath(section_ + ".html_report.file_user",ksys::getuid());
  m2 = config_->valueByPath(section_ + ".html_report.file_group",ksys::getgid());
  chModOwn(f.fileName(),m0,m1,m2);
  writeHtmlHead(f);
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
      if( verbose_ ) fprintf(stderr,"%s %s\n",
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
        "  <TH BGCOLOR=\"" + getDecor("table_head") + "\" COLSPAN=\"" +
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
        "  <TH ALIGN=center BGCOLOR=\"" + getDecor("head.host") + "\" nowrap>\n"
        "    Host\n"
        "  </TH>\n"
        "  <TH ALIGN=center BGCOLOR=\"" + getDecor("head.data") + "\" nowrap>\n"
        "    Data bytes\n"
        "  </TH>\n"
        "  <TH ALIGN=center BGCOLOR=\"" + getDecor("head.dgram") + "\" nowrap>\n"
        "    Datagram bytes\n"
        "  </TH>\n"
      ;
      while( *pi >= sv ){
        if( (uintmax_t) table[*pi + av].sum("SUM1") > 0 )
          f <<
            "  <TH COLSPAN=2 ALIGN=center BGCOLOR=\"" + getDecor("detail_head") + "\" nowrap>\n"
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
        uint32_t ip4 = indexToIp4Addr(table[0](i,"st_ip"));
        f <<
          "<TR>\n"
          "  <TH ALIGN=left BGCOLOR=\"" + getDecor("body.host") + "\" nowrap>\n"
          "    <A HREF=\"http://" + resolveAddr(ip4) + "\">\n" +
          resolveAddr(ip4) + (resolveAddr(ip4).strcmp(resolveAddr(ip4,true)) == 0 ? utf8::String() : " (" + resolveAddr(ip4,true) + ")") + "\n" +
          "    </A>\n"
          "  </TH>\n"
          "  <TH ALIGN=right BGCOLOR=\"" + getDecor("body.data") + "\" nowrap>\n" +
          formatTraf(table[0](i,"SUM2"),table[0].sum("SUM1")) + "\n"
          "  </TH>\n"
          "  <TH ALIGN=right BGCOLOR=\"" + getDecor("body.dgram") + "\" nowrap>\n" +
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
            statement6_->paramAsMutant("host",table[0](i,"st_ip"));
            uintmax_t sum1, sum2;
            getBPFTCached(statement6_,NULL,&sum1,&sum2);
            f <<
              "  <TH ALIGN=right BGCOLOR=\"" + getDecor("details.body.data") + "\" nowrap>\n" +
              formatTraf(sum2,table[*pi + av].sum("SUM1")) + "\n"
              "  </TH>\n"
              "  <TH ALIGN=right BGCOLOR=\"" + getDecor("details.body.dgram") + "\" nowrap>\n" +
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
        "  <TH ALIGN=right BGCOLOR=\"" + getDecor("tail.host") + "\" nowrap>\n"
        "    Summary:\n"
        "  </TH>\n"
        "  <TH ALIGN=right BGCOLOR=\"" + getDecor("tail.data") + "\" nowrap>\n" +
        formatTraf(table[0].sum("SUM2"),table[0].sum("SUM1")) + "\n"
        "  </TH>\n"
        "  <TH ALIGN=right BGCOLOR=\"" + getDecor("tail.dgram") + "\" nowrap>\n" +
        formatTraf(table[0].sum("SUM1"),table[0].sum("SUM1")) + "\n"
        "  </TH>\n"
      ;
      while( *pi >= sv ){
        if( (uintmax_t) table[*pi + av].sum("SUM1") > 0 )
          f <<
            "  <TH ALIGN=right BGCOLOR=\"" + getDecor("details.tail.data") + "\" nowrap>\n" +
            formatTraf(table[*pi + av].sum("SUM2"),table[0].sum("SUM1")) + "\n"
            "  </TH>\n"
            "  <TH ALIGN=right BGCOLOR=\"" + getDecor("details.tail.dgram") + "\" nowrap>\n" +
            formatTraf(table[*pi + av].sum("SUM1"),table[0].sum("SUM1")) + "\n"
            "  </TH>\n"
          ;
        (*pi)--;
      }
      f <<
        "</TR>\n"
        "</TABLE>\n<BR>\n<BR>\n"
      ;
      bool nextLevel = !(bool) config_->valueByPath(section_ + ".html_report.refresh_only_current",false);
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
  f <<
    "DNS cache size: " + utf8::int2Str((uintmax_t) dnsCache_.count()) + ", "
    "hit: " + utf8::int2Str(dnsCacheHitCount_) + ", " +
    "miss: " + utf8::int2Str(dnsCacheMissCount_) + ", " +
    "hit ratio: " + (
      dnsCacheHitCount_ + dnsCacheHitCount_ > 0 && dnsCacheHitCount_ > 0 ?
        utf8::int2Str(dnsCacheHitCount_ * 100u / (dnsCacheHitCount_ + dnsCacheHitCount_)) + "." +
        utf8::int2Str(
          dnsCacheHitCount_ * 10000u / (dnsCacheHitCount_ + dnsCacheHitCount_) - 
          dnsCacheHitCount_ * 100u / (dnsCacheHitCount_ + dnsCacheHitCount_) * 100u
        )
        :
        utf8::String("-")
    ) + ", " +
    "miss ratio: " + (
      dnsCacheHitCount_ + dnsCacheHitCount_ > 0 && dnsCacheMissCount_ > 0 ?
        utf8::int2Str(dnsCacheMissCount_ * 100u / (dnsCacheHitCount_ + dnsCacheHitCount_)) + "." +
        utf8::int2Str(
          dnsCacheMissCount_ * 10000u / (dnsCacheHitCount_ + dnsCacheHitCount_) - 
          dnsCacheMissCount_ * 100u / (dnsCacheHitCount_ + dnsCacheHitCount_) * 100u
        )
        :
        utf8::String("-")
    ) +
    "<BR>\n"
  ;
  writeHtmlTail(f);
  f.resize(f.tell());
  if( level == rlYear ){
l1: if( (bool) config_->valueByPath(section_ + ".html_report.update_cache",true) )
      database_->commit();
    else
      database_->rollback();
    dnsCache_.drop();
  }
}
//------------------------------------------------------------------------------
void Logger::parseBPFTLogFile()
{
  AsyncFile flog(config_->valueByPath(section_ + ".log_file_name"));
/*
  statement_->text("DELETE FROM INET_BPFT_STAT")->execute();
  stFileStatUpd_->prepare()->
    paramAsString("ST_LOG_FILE_NAME",flog.fileName())->
    paramAsMutant("ST_LAST_OFFSET",0)->execute();
 */
  flog.readOnly(true).open();
  database_->start();
  if( (bool) config_->valueByPath(section_ + ".reset_log_file_position",false) )
    updateLogFileLastOffset(flog.fileName(),0);
  int64_t offset = fetchLogFileLastOffset(flog.fileName());
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
  bool log32bitOsCompatible = config_->valueByPath(section_ + ".log_32bit_os_compatible",SIZEOF_VOID_P < 8);
  struct tm start, stop;
  ksock::APIAutoInitializer ksockAPIAutoInitializer;
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
    if( dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL && entriesCount > 0 ){
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
      printStat(lineNo,offset,flog.tell(),flog.size(),cl,&tma);
      lineNo++;
      entriesCount = 0;
    }
    printStat(lineNo,offset,flog.tell(),flog.size(),cl,&tma);
    if( flog.seekable() ) updateLogFileLastOffset(flog.fileName(),flog.tell());
    database_->commit();
    database_->start();
    lineNo += entriesCount;
  }
  printStat(lineNo,offset,flog.tell(),flog.size(),cl);
  database_->commit();
}
//------------------------------------------------------------------------------
} // namespace macrosocope
//------------------------------------------------------------------------------
