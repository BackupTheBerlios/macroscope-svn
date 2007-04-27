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
#include "sniffer.h"
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
Sniffer::~Sniffer()
{
}
//------------------------------------------------------------------------------
Sniffer::Sniffer(Database * database) : database_(database)
{
}
//------------------------------------------------------------------------------
void Sniffer::insertPacketsInDatabase(uint64_t bt,uint64_t et,const HashedPacket * packets,uintptr_t count)
{
  if( !database_->attached() ) database_->attach();
  if( statement_ == NULL ) statement_ = database_->newAttachedStatement();
  if( !statement_->prepared() )
    statement_->text(
      "INSERT INTO INET_BPFT_STAT ("
      "  st_if,st_start,st_src_ip,st_dst_ip,st_ip_proto,st_src_port,st_dst_port,st_dgram_bytes,st_data_bytes"
      ") VALUES ("
      "  :st_if,:st_start,:st_src_ip,:st_dst_ip,:st_ip_proto,:st_src_port,:st_dst_port,:st_dgram_bytes,:st_data_bytes"
      ")"
    )->prepare()->paramAsString(0/*"st_if"*/,/*iface()*/ "test");
  database_->start();
  while( count-- > 0 ){
    statement_->
      paramAsMutant(1/*"st_src_ip"*/,     ksock::SockAddr::addr2Index(packets[count].srcAddr_))->
      paramAsMutant(2/*"st_dst_ip"*/,     ksock::SockAddr::addr2Index(packets[count].dstAddr_))->
      paramAsMutant(3/*"st_ip_proto"*/,   packets[count].proto_)->
      paramAsMutant(4/*"st_src_port"*/,   packets[count].srcPort_)->
      paramAsMutant(5/*"st_dst_port"*/,   packets[count].dstPort_)->
      paramAsMutant(6/*"st_dgram_bytes"*/,packets[count].pktSize_)->
      paramAsMutant(7/*"st_data_bytes"*/, packets[count].dataSize_)->
      execute();
  }
  database_->commit();
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------