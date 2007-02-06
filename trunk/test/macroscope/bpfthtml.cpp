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
utf8::String Logger::resolveAddr(uint32_t ip4)
{
  AutoPtr<DNSCacheEntry> addr(newObject<DNSCacheEntry>());
  addr->addr4_.sin_addr.s_addr = ip4;
  addr->addr4_.sin_port = 0;
  addr->addr4_.sin_family = PF_INET;
  DNSCacheEntry * pAddr = addr;
  dnsCache_.insert(addr,false,false,&pAddr);
  if( pAddr == addr ){
    if( resolveDNSNames_ ){
      if( statement2_->paramAsMutant("ip",ip4)->execute()->fetch() ){
        addr.ptr(NULL)->name_ = statement2_->valueAsString(
          (uint32_t) statement2_->valueAsMutant("st_src_ip") == ip4 ? "st_src_name" : "st_dst_name"
        );
      }
      else {
        addr.ptr(NULL)->name_ = addr->resolveAddr();
	statement3_->paramAsMutant("name",pAddr->name_)->paramAsMutant("ip",ip4)->execute();
	statement4_->paramAsMutant("name",pAddr->name_)->paramAsMutant("ip",ip4)->execute();
      }
    }
    else {
      addr.ptr(NULL)->name_ = addr->resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV);
    }
    if( dnsCacheSize_ > 0 && dnsCache_.count() >= dnsCacheSize_ )
      dnsCache_.drop(dnsCacheLRU_.remove(*dnsCacheLRU_.last()));
  }
  else {
    dnsCacheLRU_.remove(*pAddr);
  }
  dnsCacheLRU_.insToHead(*pAddr);
  return pAddr->name_;
}
//------------------------------------------------------------------------------
Logger & Logger::writeBPFTHtmlReport(const ConfigSection & section)
{
/*!
 * \TODO По ип сортировать по отношению к адресу нашего шлюза
 * \TODO далее детализация по портам, протоколу
 */
  ksock::SockAddr gateway;
  gateway.resolveName(section.text("gateway"));
  database_->start();
  struct tm beginTime, beginTime2, endTime;
  beginTime = endTime = time2tm(getlocaltimeofday());
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
  while( tm2Time(endTime) >= tm2Time(beginTime) ){
    if( verbose_ ) fprintf(stderr,"%s\n",(const char *) utf8::tm2Str(endTime).getOEMString());
    beginTime2 = beginTime;
    beginTime.tm_year = endTime.tm_year;
    statement_->text(
      "SELECT"
      "  A.*"
      "FROM ("
      "  SELECT"
      "    B.st_ip AS st_ip, SUM(B.SUM1) AS SUM1, SUM(B.SUM2) AS SUM2"
      "  FROM ("
      "      SELECT"
      "        st_src_ip AS st_ip, SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2"
      "      FROM"
      "        INET_BPFT_STAT"
      "      WHERE "
      "        st_start >= :BT AND st_stop <= :ET AND"
      "        st_ip = :gateway"
      "      GROUP BY st_ip"
      "    UNION"
      "      SELECT"
      "        st_dst_ip AS st_ip, SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2"
      "      FROM"
      "        INET_BPFT_STAT"
      "      WHERE "
      "        st_start >= :BT AND st_stop <= :ET AND"
      "        st_ip = :gateway"
      "      GROUP BY st_ip"
      "  ) AS B "
      "  GROUP BY B.st_ip"
      ") AS A "
      "ORDER BY A.SUM1 DESC"
    );
    statement_->prepare();
    statement_->paramAsMutant("BT",beginTime);
    statement_->paramAsMutant("ET",endTime);
    statement_->paramAsMutant("gateway",int(gateway.addr4_.sin_addr.s_addr));
    statement_->execute();
    while( statement_->fetch() ){
      if( verbose_ ){
        for( uintptr_t i = 0; i < statement_->fieldCount(); i++ ){
          fprintf(stderr,"%s = %s%s",
	    (const char *) statement_->fieldName(i).getOEMString(),
	    (const char *) (statement_->fieldName(i).strcasecmp("st_ip") == 0 ?
 	      statement_->valueAsString(i) : resolveAddr(statement_->valueAsMutant(i))
	    ).getOEMString(),
	    i < statement_->fieldCount() - 1 ? ", " : "\n"
          );
	}
      }        
    }
    endTime.tm_year--;
    beginTime = beginTime2;
  }	  
  database_->commit();
  return *this;
}
//------------------------------------------------------------------------------
void Logger::parseBPFTLogFile(const ConfigSection & section)
{
  resolveDNSNames_ = section.value("resolve_dns_names",false);
  statement2_->text(
    utf8::String("select ") +
    (dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL ? "FIRST 1 " : "") +
    "st_src_name, st_dst_name FROM INET_BPFT_STAT where st_src_ip = :ip OR st_dst_ip = :ip" +
    (dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL ? " LIMIT 0,1" : "")
  )->prepare();
  statement3_->text(
    "update INET_BPFT_STAT set st_src_name = :name where st_src_ip = :ip"
  )->prepare();
  statement4_->text(
    "update INET_BPFT_STAT set st_dst_name = :name where st_dst_ip = :ip"
  )->prepare();
  dnsCacheSize_ = section.value("dns_cache_size",0);
  AsyncFile flog(section.text("log_file_name"));
  flog.readOnly(true).open();
  database_->start();
  int64_t offset = fetchLogFileLastOffset(flog.fileName());
  flog.seek(offset);
  int64_t lineNo = 0, tma = getlocaltimeofday();
  int64_t cl = getlocaltimeofday();
  union {
    BPFTHeader header;
    BPFTHeader32 header32;
  };
//  statement_->text("DELETE FROM INET_BPFT_STAT")->execute();
  statement_->text(
    "INSERT INTO INET_BPFT_STAT ("
    "  st_start,st_stop,st_src_ip,st_dst_ip,st_ip_proto,st_src_port,st_dst_port,st_dgram_bytes,st_data_bytes,st_src_name,st_dst_name"
    ") VALUES ("
    "  :st_start,:st_stop,:st_src_ip,:st_dst_ip,:st_ip_proto,:st_src_port,:st_dst_port,:st_dgram_bytes,:st_data_bytes,:st_src_name,:st_dst_name"
    ")"
  );
  statement_->prepare();
  bool log32bitOsCompatible = section.value("log_32bit_os_compatible",SIZEOF_VOID_P < 8);
  for(;;){
    struct tm start, stop;
    uintptr_t entriesCount;
    uint64_t safePos = flog.tell();
    if( log32bitOsCompatible ){
      if( flog.read(&header32,sizeof(header32)) != sizeof(header32) ){
        flog.seek(safePos);
        break;
      }
      start = timeval2tm(header32.start_);
      stop = timeval2tm(header32.stop_);
      entriesCount = header32.eCount_;
    }
    else {
      if( flog.read(&header,sizeof(header)) != sizeof(header) ){
        flog.seek(safePos);
        break;
      }
      start = timeval2tm(header.start_);
      stop = timeval2tm(header.stop_);
      entriesCount = header.eCount_;
    }
    Array<BPFTEntry> entries;
    Array<BPFTEntry32> entries32;
    if( log32bitOsCompatible ){
      entries32.resize(entriesCount);
      if( flog.read(entries32,sizeof(BPFTEntry32) * entriesCount) != int64_t(sizeof(BPFTEntry32) * entriesCount) ){
        flog.seek(safePos);
        break;
      }
    }
    else {
      entries.resize(entriesCount);
      if( flog.read(entries,sizeof(BPFTEntry) * entriesCount) != int64_t(sizeof(BPFTEntry) * entriesCount) ){
        flog.seek(safePos);
        break;
      }
    }
    for( intptr_t i = entriesCount - 1; i >= 0; i-- ){
      statement_->paramAsMutant("st_start",start);
      statement_->paramAsMutant("st_stop",stop);
      if( log32bitOsCompatible ){
        statement_->paramAsMutant("st_src_ip",entries32[i].srcIp_.s_addr);
        statement_->paramAsMutant("st_dst_ip",entries32[i].dstIp_.s_addr);
        statement_->paramAsMutant("st_ip_proto",entries32[i].ipProtocol_);
        statement_->paramAsMutant("st_src_port",entries32[i].srcPort_);
        statement_->paramAsMutant("st_dst_port",entries32[i].dstPort_);
        statement_->paramAsMutant("st_dgram_bytes",entries32[i].dgramSize_);
        statement_->paramAsMutant("st_data_bytes",entries32[i].dataSize_);
	if( resolveDNSNames_ ){
          statement_->paramAsMutant("st_src_name",resolveAddr(entries32[i].srcIp_.s_addr));
          statement_->paramAsMutant("st_dst_name",resolveAddr(entries32[i].dstIp_.s_addr));
	}
      }
      else {
        statement_->paramAsMutant("st_src_ip",entries[i].srcIp_.s_addr);
        statement_->paramAsMutant("st_dst_ip",entries[i].dstIp_.s_addr);
        statement_->paramAsMutant("st_ip_proto",entries[i].ipProtocol_);
        statement_->paramAsMutant("st_src_port",entries[i].srcPort_);
        statement_->paramAsMutant("st_dst_port",entries[i].dstPort_);
        statement_->paramAsMutant("st_dgram_bytes",entries[i].dgramSize_);
        statement_->paramAsMutant("st_data_bytes",entries[i].dataSize_);
	if( resolveDNSNames_ ){
          statement_->paramAsMutant("st_src_name",resolveAddr(entries[i].srcIp_.s_addr));
          statement_->paramAsMutant("st_dst_name",resolveAddr(entries[i].dstIp_.s_addr));
	}
      }
      statement_->execute();
      if( gettimeofday() - tma >= 1000000 ){
	tma = getlocaltimeofday();
        printStat(lineNo,offset,flog.tell(),flog.size(),cl);
      }
      lineNo++;
    }
    database_->commit();
    database_->start();
    lineNo += entriesCount;
  }
  printStat(lineNo,offset,flog.tell(),flog.size(),cl);
  updateLogFileLastOffset(flog.fileName(),flog.tell());
  database_->commit();
}
//------------------------------------------------------------------------------
} // namespace macrosocope
//------------------------------------------------------------------------------
