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
Logger & Logger::writeBPFTHtmlReport(AsyncFile & f)
{
/*!
 * \TODO По ип сортировать по отношению к адресу нашего шлюза
 * \TODO далее детализация по портам, протоколу
 */
  return *this;
}
//------------------------------------------------------------------------------
void Logger::parseBPFTLogFile(const ConfigSection & section)
{
  AsyncFile flog(section.text("log_file_name"));
  flog.readOnly(true).open();
  database_->start();
  int64_t offset = fetchLogFileLastOffset(flog.fileName());
  flog.seek(offset);
  int64_t lineNo = 0;
  int64_t cl = getlocaltimeofday();
  AutoPtr<Statement> statement(database_->newAttachedStatement());
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
      AutoPtr<DNSCacheEntry> srcAddr(newObject<DNSCacheEntry>());
      AutoPtr<DNSCacheEntry> dstAddr(newObject<DNSCacheEntry>());
      if( log32bitOsCompatible ){
        statement_->paramAsMutant("st_src_ip",entries32[i].srcIp_.s_addr);
        statement_->paramAsMutant("st_dst_ip",entries32[i].dstIp_.s_addr);
        statement_->paramAsMutant("st_ip_proto",entries32[i].ipProtocol_);
        statement_->paramAsMutant("st_src_port",entries32[i].srcPort_);
        statement_->paramAsMutant("st_dst_port",entries32[i].dstPort_);
        statement_->paramAsMutant("st_dgram_bytes",entries32[i].dgramSize_);
        statement_->paramAsMutant("st_data_bytes",entries32[i].dataSize_);
        srcAddr->addr4_.sin_addr = entries32[i].srcIp_;
        srcAddr->addr4_.sin_port = 0;//entries32[i].srcPort_;
        dstAddr->addr4_.sin_addr = entries32[i].dstIp_;
        dstAddr->addr4_.sin_port = 0;//entries32[i].dstPort_;
      }
      else {
        statement_->paramAsMutant("st_src_ip",entries[i].srcIp_.s_addr);
        statement_->paramAsMutant("st_dst_ip",entries[i].dstIp_.s_addr);
        statement_->paramAsMutant("st_ip_proto",entries[i].ipProtocol_);
        statement_->paramAsMutant("st_src_port",entries[i].srcPort_);
        statement_->paramAsMutant("st_dst_port",entries[i].dstPort_);
        statement_->paramAsMutant("st_dgram_bytes",entries[i].dgramSize_);
        statement_->paramAsMutant("st_data_bytes",entries[i].dataSize_);
        srcAddr->addr4_.sin_addr = entries[i].srcIp_;
        srcAddr->addr4_.sin_port = 0;//entries[i].srcPort_;
        dstAddr->addr4_.sin_addr = entries[i].dstIp_;
        dstAddr->addr4_.sin_port = 0;//entries[i].dstPort_;
      }
      DNSCacheEntry * pAddr;
      srcAddr->addr4_.sin_family = PF_INET;
      dnsCache_.insert(srcAddr,false,false,&pAddr);
      if( pAddr == srcAddr ){
        srcAddr.ptr(NULL)->name_ = srcAddr->resolveAddr();
//	if( verbose_ ) fprintf(stderr,"%s\n",(const char *) pAddr->name_.getOEMString());
      }
      else {
        dnsCacheLRU_.remove(*pAddr);
      }
      dnsCacheLRU_.insToHead(*pAddr);
      statement_->paramAsMutant("st_src_name",pAddr->name_);
      dstAddr->addr4_.sin_family = PF_INET;
      dnsCache_.insert(dstAddr,false,false,&pAddr);
      if( pAddr == dstAddr ){
        dstAddr.ptr(NULL)->name_ = dstAddr->resolveAddr();
//	if( verbose_ ) fprintf(stderr,"%s\n",(const char *) pAddr->name_.getOEMString());
      }
      else {
        dnsCacheLRU_.remove(*pAddr);
      }
      dnsCacheLRU_.insToHead(*pAddr);
      statement_->paramAsMutant("st_dst_name",pAddr->name_);
      statement_->execute();
    }
    updateLogFileLastOffset(flog.fileName(),flog.tell());
    database_->commit();
    database_->start();
    printStat(lineNo,offset,flog.tell(),flog.size(),cl);
    lineNo += entriesCount;
  }
  printStat(lineNo,offset,flog.tell(),flog.size(),cl);
  updateLogFileLastOffset(flog.fileName(),flog.tell());
  database_->commit();
}
//------------------------------------------------------------------------------
} // namespace macrosocope
//------------------------------------------------------------------------------
