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
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
const char * const Sniffer::pgpNames[pgpCount] = { "NSEC", "SEC", "MIN", "HOUR", "DAY", "MON", "YEAR"  };
//------------------------------------------------------------------------------
Sniffer::~Sniffer()
{
}
//------------------------------------------------------------------------------
Sniffer::Sniffer(Database * database,Database * database2) :
  database_(database),
  database2_(database2),
  totalsPeriod_(pgpDay),
  packetsInTransaction_(0),
  updates_(0),
  updatesTime_(0),
  lastSweep_(gettimeofday()),
  maintenance_(86400),
  storagePeriodOfStatistics_(0),
  maintenanceThreshold_(0.1)
{
}
//------------------------------------------------------------------------------
void Sniffer::detach()
{
  database_->detach();
}
//------------------------------------------------------------------------------
Sniffer & Sniffer::totalsPeriod(PacketGroupingPeriod period)
{
  totalsPeriod_ = period;
  //totalsPeriod_ = period < pgpMin ? pgpMin : period;
  //totalsPeriod_ = (groupingPeriod() >= totalsPeriod_ ? PacketGroupingPeriod(groupingPeriod() + 1) : totalsPeriod_);
  //if( totalsPeriod_ > pgpYear )
  //  newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//------------------------------------------------------------------------------
//PacketGroupingPeriod Sniffer::detectPeriod(Statement * statement)
//{
//  if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL ){
//    statement_->text(
//      "SELECT FIRST 1 st_start AS BT FROM INET_BPFT_STAT_TOTALS ORDER BY st_start"
//    )->execute()->fetchAll();
//  }
//  else if( dynamic_cast<MYSQLDatabase *>(statement_->database()) != NULL ){
//    statement_->text(
//      "SELECT MIN(st_start) FROM INET_BPFT_STAT_TOTALS"
//    )->execute()->fetchAll();
//  }
//  if( statement_->rowCount() == 0 ) return pgpDay;
//  struct tm t = statement->valueAsMutant(0);
//  if( 
//}
//------------------------------------------------------------------------------
void Sniffer::shiftPeriod(PacketGroupingPeriod tp,struct tm & t,intptr_t v)
{
  while( v < -1 ){ shiftPeriod(tp,t,-1); v++; }
  while( v > +1 ){ shiftPeriod(tp,t,+1); v--; }
  switch( tp ){
    case pgpNone :
      assert( 0 );
      break;
    case pgpSec  :
      if( (t.tm_sec += int(v)) < 0 ){ t.tm_sec = 59; goto min; } else if( t.tm_sec > 59 ){ t.tm_sec = 0; goto min; }
      break;
    case pgpMin  :
min:  if( (t.tm_min += int(v)) < 0 ){ t.tm_min = 59; goto hour; } else if( t.tm_min > 59 ){ t.tm_min = 0; goto hour; }
      break;
    case pgpHour :
hour: if( (t.tm_hour += int(v) ) < 0 ){ t.tm_hour = 23; goto day; } else if( t.tm_hour > 23 ){ t.tm_hour = 0; goto day; }
      break;
    case pgpDay  :
day:  if( (t.tm_mday += int(v)) < 1 ){ goto mon; } else if( t.tm_mday > (int) monthDays(t.tm_year + 1900,t.tm_mon) ){ t.tm_mday = 1; goto mon; }
      break;
    case pgpMon  :
mon:  if( (t.tm_mon += int(v)) < 0 || t.tm_mon > 11 ){ t.tm_mon = 0; goto year; }
      break;
    case pgpYear :
year: t.tm_year += int(v);
      break;
    default : 
      assert( 0 );
  }
  if( t.tm_mday < 1 ) t.tm_mday = (int) monthDays(t.tm_year + 1900,t.tm_mon);
}
//------------------------------------------------------------------------------
void Sniffer::setTotalsBounds(PacketGroupingPeriod tp,struct tm & bt,struct tm & et,struct tm & btt,struct tm & ett)
{
  btt = bt;
  ett = et;
  switch( tp ){
    case pgpNone :
    case pgpSec  :
      break;
    case pgpMin  :
      btt.tm_sec = ett.tm_sec = 0;
      break;
    case pgpHour :
      btt.tm_sec = btt.tm_min = ett.tm_sec = ett.tm_min = 0;
      break;
    case pgpDay  :
      btt.tm_sec = btt.tm_min = btt.tm_hour = ett.tm_sec = ett.tm_min = ett.tm_hour = 0;
      break;
    case pgpMon  :
      btt.tm_sec = btt.tm_min = btt.tm_hour = ett.tm_sec = ett.tm_min = ett.tm_hour = 0;
      btt.tm_mday = ett.tm_mday = 1;
      break;
    case pgpYear :
      btt.tm_sec = btt.tm_min = btt.tm_hour = btt.tm_mon = ett.tm_sec = ett.tm_min = ett.tm_hour = ett.tm_mon = 0;
      btt.tm_mday = ett.tm_mday = 1;
      break;
    default : 
      assert( 0 );
  }
  shiftPeriod(tp,ett,+1);
  //uint64_t bta = tm2Time(bt), eta = tm2Time(et);
  //assert( bta <= eta );
  //uint64_t btta = tm2Time(btt), etta = tm2Time(ett);
  //assert( btta <= etta );
  //while( btta <= bta ){ shiftPeriod(tp,btt,+1); btta = tm2Time(btt); }
  //while( etta >= eta ){ shiftPeriod(tp,ett,-1); etta = tm2Time(ett); }
}
//------------------------------------------------------------------------------
void Sniffer::updateTotals(
  uintptr_t i,
  const Mutant & m,
  const in_addr & srcAddr,
  uintptr_t srcPort,
  const in_addr & dstAddr,
  uintptr_t dstPort,
  intptr_t proto,
  uintmax_t dgram,
  uintmax_t data)
{
  Statement * st = totals_[stSel][i];
  if( st == NULL ) totals_[stSel][i] = st = database_->newAttachedStatement();
  if( !st->prepared() )
    st->text(utf8::String(
      "SELECT iface,ts,src_ip,src_port,dst_port,dst_ip,ip_proto,dgram,data "
      "FROM INET_SNIFFER_STAT_@0001@ WHERE"
      " iface = :if AND ts = :ts AND src_ip = :src_ip AND src_port = :src_port AND dst_ip = :dst_ip AND dst_port = :dst_port AND ip_proto = :proto "
      "FOR UPDATE"
      ).replaceAll("@0001@",pgpNames[i])
    )->prepare()->paramAsString("if",ifName());
  st->
    paramAsMutant("ts",       Mutant(m).changeType(mtTime))->
    paramAsString("src_ip",   ksock::SockAddr::addr2Index(srcAddr))->
    paramAsString("dst_ip",   ksock::SockAddr::addr2Index(dstAddr))->
    paramAsMutant("src_port", srcPort)->
    paramAsMutant("dst_port", dstPort)->
    paramAsMutant("proto",    proto)->
    execute()->fetchAll();
  intptr_t op = st->rowCount() == 0 ? stIns : stUpd;
  st = totals_[op][i];
  if( st == NULL ) totals_[op][i] = st = database_->newAttachedStatement();
  if( !st->prepared() )
    st->text(utf8::String(
      op == stIns ?
        "INSERT INTO INET_SNIFFER_STAT_@0001@ ("
        "  iface,ts,src_ip,src_port,dst_ip,dst_port,ip_proto,dgram,data"
        ") VALUES ("
        "  :if,:ts,:src_ip,:src_port,:dst_ip,:dst_port,:proto,:dgram,:data"
        ")"
      :
        "UPDATE INET_SNIFFER_STAT_@0001@ SET "
        "  dgram = dgram + :dgram, data = data + :data "
        "WHERE"
        "  iface = :if AND ts = :ts AND src_ip = :src_ip AND src_port = :src_port AND dst_ip = :dst_ip AND dst_port = :dst_port AND ip_proto = :proto"
      ).replaceAll("@0001@",pgpNames[i]
    ))->prepare()->paramAsString("if",ifName());
  st->
    paramAsMutant("ts",       Mutant(m).changeType(mtTime))->
    paramAsString("src_ip",   ksock::SockAddr::addr2Index(srcAddr))->
    paramAsString("dst_ip",   ksock::SockAddr::addr2Index(dstAddr))->
    paramAsMutant("dgram",    dgram)->
    paramAsMutant("data",     data)->
    paramAsMutant("src_port", srcPort)->
    paramAsMutant("dst_port", dstPort)->
    paramAsMutant("proto",    proto)->
    execute();
}
//------------------------------------------------------------------------------
void Sniffer::getTrafficPeriod(Statement * statement,const utf8::String & sectionName,struct tm & beginTime,struct tm & endTime,bool gmt)
{
  memset(&beginTime,0,sizeof(beginTime));
  endTime = beginTime;
  if( dynamic_cast<FirebirdDatabase *>(statement->database()) != NULL ){
    statement->text(
      "SELECT MIN(BT) AS BT FROM ("
      "SELECT * FROM (SELECT FIRST 1 ts AS BT FROM INET_SNIFFER_STAT_YEAR WHERE iface = :if ORDER BY ts) "
      "UNION ALL "
      "SELECT * FROM (SELECT FIRST 1 ts AS BT FROM INET_SNIFFER_STAT_MON WHERE iface = :if ORDER BY ts) "
      "UNION ALL "
      "SELECT * FROM (SELECT FIRST 1 ts AS BT FROM INET_SNIFFER_STAT_DAY WHERE iface = :if ORDER BY ts) "
      "UNION ALL "
      "SELECT * FROM (SELECT FIRST 1 ts AS BT FROM INET_SNIFFER_STAT_HOUR WHERE iface = :if ORDER BY ts) "
      "UNION ALL "
      "SELECT * FROM (SELECT FIRST 1 ts AS BT FROM INET_SNIFFER_STAT_MIN WHERE iface = :if ORDER BY ts) "
      "UNION ALL "
      "SELECT * FROM (SELECT FIRST 1 ts AS BT FROM INET_SNIFFER_STAT_SEC WHERE iface = :if ORDER BY ts) "
      "UNION ALL "
      "SELECT * FROM (SELECT FIRST 1 ts AS BT FROM INET_SNIFFER_STAT_NSEC WHERE iface = :if ORDER BY ts)"
      ")"
    )->prepare()->paramAsString("if",sectionName)->execute()->fetchAll();
    if( statement->fieldIndex("BT") < 0 || statement->rowCount() == 0 ) return;
    beginTime = time2tm((uint64_t) statement->valueAsMutant("BT") + (gmt ? 0 : getgmtoffset()));
    statement->text(
      "SELECT MAX(ET) AS ET FROM ("
      "SELECT * FROM (SELECT FIRST 1 ts AS ET FROM INET_SNIFFER_STAT_YEAR WHERE iface = :if ORDER BY ts DESC) "
      "UNION ALL "
      "SELECT * FROM (SELECT FIRST 1 ts AS ET FROM INET_SNIFFER_STAT_MON WHERE iface = :if ORDER BY ts DESC) "
      "UNION ALL "
      "SELECT * FROM (SELECT FIRST 1 ts AS ET FROM INET_SNIFFER_STAT_DAY WHERE iface = :if ORDER BY ts DESC) "
      "UNION ALL "
      "SELECT * FROM (SELECT FIRST 1 ts AS ET FROM INET_SNIFFER_STAT_HOUR WHERE iface = :if ORDER BY ts DESC) "
      "UNION ALL "
      "SELECT * FROM (SELECT FIRST 1 ts AS ET FROM INET_SNIFFER_STAT_MIN WHERE iface = :if ORDER BY ts DESC) "
      "UNION ALL "
      "SELECT * FROM (SELECT FIRST 1 ts AS ET FROM INET_SNIFFER_STAT_SEC WHERE iface = :if ORDER BY ts DESC) "
      "UNION ALL "
      "SELECT * FROM (SELECT FIRST 1 ts AS ET FROM INET_SNIFFER_STAT_NSEC WHERE iface = :if ORDER BY ts DESC)"
      ")"
    )->prepare()->paramAsString("if",sectionName)->execute()->fetchAll();
    if( statement->fieldIndex("ET") < 0 || statement->rowCount() == 0 ) return;
    endTime = time2tm((uint64_t) statement->valueAsMutant("ET") + (gmt ? 0 : getgmtoffset()) + 1000000u);
  }
  else if( dynamic_cast<MYSQLDatabase *>(statement->database()) != NULL ){
    statement->text(
      "SELECT MIN(a.BT) AS BT, MAX(a.ET) AS ET FROM ("
      "SELECT MIN(ts) AS BT, MAX(ts) AS ET FROM INET_SNIFFER_STAT_YEAR WHERE iface = :if "
      "UNION ALL "
      "SELECT MIN(ts) AS BT, MAX(ts) AS ET FROM INET_SNIFFER_STAT_MON WHERE iface = :if  "
      "UNION ALL "
      "SELECT MIN(ts) AS BT, MAX(ts) AS ET FROM INET_SNIFFER_STAT_DAY WHERE iface = :if  "
      "UNION ALL "
      "SELECT MIN(ts) AS BT, MAX(ts) AS ET FROM INET_SNIFFER_STAT_HOUR WHERE iface = :if  "
      "UNION ALL "
      "SELECT MIN(ts) AS BT, MAX(ts) AS ET FROM INET_SNIFFER_STAT_MIN WHERE iface = :if  "
      "UNION ALL "
      "SELECT MIN(ts) AS BT, MAX(ts) AS ET FROM INET_SNIFFER_STAT_SEC WHERE iface = :if  "
      "UNION ALL "
      "SELECT MIN(ts) AS BT, MAX(ts) AS ET FROM INET_SNIFFER_STAT_NSEC WHERE iface = :if "
      ") AS a"
    )->prepare()->paramAsString("if",sectionName)->execute()->fetchAll();
    if( statement->fieldIndex("BT") < 0 || statement->fieldIndex("ET") < 0 ||
        statement->rowCount() == 0 ||
        (statement->valueAsMutant("BT") == Mutant(0) &&
         statement->valueAsMutant("ET") == Mutant(0)) ) return;
    beginTime = time2tm((uint64_t) statement->valueAsMutant("BT") + (gmt ? 0 : getgmtoffset()));
    endTime = time2tm((uint64_t) statement->valueAsMutant("ET") + (gmt ? 0 : getgmtoffset()) + 1000000u);
  }
  else {
    assert( 0 );
  }
}
//------------------------------------------------------------------------------
void Sniffer::recalcTotals()
{
  database_->attach()->start();
  if( statement_ == NULL ) statement_ = database_->newAttachedStatement();
  struct tm bt, et, et2;
  getTrafficPeriod(statement_,ifName(),bt,et,true);
  setTotalsBounds(totalsPeriod_,bt,et,bt,et);
  if( statement2_ == NULL ) statement2_ = database_->newAttachedStatement();
  statement2_->text("DELETE FROM INET_BPFT_STAT_TOTALS a WHERE a.st_if = :if")->
    prepare()->paramAsString("if",ifName())->execute();
  statement_->text(
    "SELECT"
    "  st_src_ip,st_dst_ip,st_ip_proto,st_src_port,st_dst_port,"
    "  SUM(st_dgram_bytes) AS st_dgram_bytes,SUM(st_data_bytes) AS st_data_bytes "
    "FROM"
    "  INET_BPFT_STAT "
    "WHERE"
    "  st_if = :if AND st_start >= :BT AND st_start < :ET "
    "GROUP BY st_src_ip,st_src_port,st_dst_ip,st_dst_port,st_ip_proto"
  )->prepare()->paramAsString("if",ifName());
  while( tm2Time(bt) < tm2Time(et) ){
    shiftPeriod(totalsPeriod_,et2 = bt,+1);
    statement_->paramAsMutant("BT",bt)->paramAsMutant("ET",et2)->execute();
    statement_->fetchAll();
    for( intptr_t i = statement_->rowCount() - 1; i >= 0; i-- ){
      statement_->selectRow(i);
      //updateTotals(
      //  bt,
      //  ksock::SockAddr::indexToAddr4(statement_->valueAsString("st_src_ip")),
      //  statement_->valueAsMutant("st_src_port"),
      //  ksock::SockAddr::indexToAddr4(statement_->valueString("st_dst_ip")),
      //  statement_->valueAsMutant("st_dst_port"),
      //  statement_->valueAsMutant("st_ip_proto"),
      //  statement_->valueAsMutant("st_dgram_bytes"),
      //  statement_->valueAsMutant("st_data_bytes")
      //);
    }
    shiftPeriod(totalsPeriod_,bt,+1);
  }
  database_->commit()->detach();
}
//------------------------------------------------------------------------------
bool Sniffer::insertPacketsInDatabase(uint64_t bt,uint64_t et,const HashedPacket * packets,uintptr_t & pCount,Thread * caller) throw()
{
  bool r = false;
  uintptr_t count = pCount;
  try {
    if( !database_->attached() ){
      database_->attach()->isolation("SERIALIZABLE");
      if( dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL ){
        if( statement_ == NULL ) statement_ = database_->newAttachedStatement();
        statement_->execute("set max_sp_recursion_depth = 3");
        statement_ = NULL;
      }
      updates_ = 0;
      updatesTime_ = 0;
    }
    uint64_t ellapsed = gettimeofday();
    database_->start();
    if( ifaces_ == NULL ) ifaces_ = database_->newAttachedStatement();
    if( !ifaces_->prepared() )
      ifaces_->text("INSERT INTO INET_IFACES (iface) VALUES (:if)")->prepare()->paramAsString("if",ifName());
    try {
      ifaces_->execute();
    }
    catch( ExceptionSP & e ){
      if( !e->searchCode(isc_no_dup,isc_unique_key_violation,isc_primary_key_exists,ER_DUP_ENTRY) ) throw;
    }
    while( !caller->terminated() && !terminated_ && count > 0 && (packetsInTransaction_ == 0 || pCount - count < packetsInTransaction_) ){
      const HashedPacket & packet = packets[count - 1];
      uint64_t mbt, met;
      if( statement_ == NULL ) statement_ = database_->newAttachedStatement();
      if( !statement_->prepared() ){
        utf8::String exec;
        if( dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL ) exec = "CALL";
        else
        if( dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL ) exec = "EXECUTE PROCEDURE";
        statement_->text(
          exec + " INET_UPDATE_SNIFFER_STAT_YEAR(:iface,:ts0,:ts1,:ts2,:ts3,:ts4,:ts5,:ts6,:src_ip,:src_port,:dst_ip,:dst_port,:proto,:dgram,:data,:ports,:protocols,:mt)"
        )->prepare()->
          paramAsMutant("iface",ifName())->
          paramAsMutant("ports",ports())->
          paramAsMutant("protocols",protocols())->
          paramAsMutant("mt",totalsPeriod_);
      }
      for( intptr_t i = pgpCount - 1; i >= totalsPeriod_; i-- ){
        setBounds(PacketGroupingPeriod(i),bt,mbt,met);
        statement_->paramAsMutant("ts" + utf8::int2Str(i),Mutant(mbt).changeType(mtTime));
      }
      statement_->
        paramAsString("src_ip",ksock::SockAddr::addr2Index(packet.srcAddr_))->
        paramAsMutant("src_port",packet.srcPort_)->
        paramAsString("dst_ip",ksock::SockAddr::addr2Index(packet.dstAddr_))->
        paramAsMutant("dst_port",packet.dstPort_)->
        paramAsMutant("proto",packet.proto_)->
        paramAsMutant("dgram",packet.pktSize_)->
        paramAsMutant("data",packet.dataSize_)->
        execute();
      count--;
    }
    if( caller->terminated() || terminated_ ){
      database_->rollback();
      r = false;
      count = pCount;
    }
    else {
      database_->commit();
      r = true;
      ellapsed = gettimeofday() - ellapsed;
      updatesTime_ += ellapsed;
      updates_ += pCount - count;
      maintenanceInternal();
    }
  }
  catch( ExceptionSP & e ){
    database_->rollback(true);
    e->writeStdError();
  }
  pCount = count;
  return r;
}
//------------------------------------------------------------------------------
void Sniffer::maintenanceInternal()
{
  uintmax_t a = updates_ > 0 ? updatesTime_ / updates_ : 0;
  if( stdErr.debugLevel(77) )
    stdErr.debug(77,utf8::String::Stream() <<
      "Interface: " << ifName() << ", average database update time per record " << utf8::elapsedTime2Str(a) << "\n"
    );
  //if( ((a >= 1000000 && gettimeofday() - lastSweep_ > uint64_t(3600) * 1000000) ||
  //     gettimeofday() - lastSweep_ >= uint64_t(86400) * 1000000) &&
  //    dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL ){
  if( maintenance_ > 0 && gettimeofday() - lastSweep_ > maintenance_ && dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL ){
    if( maintenanceThread_ == NULL || maintenanceThread_->finished() ){
      maintenanceThread_ = newObjectV1<MaintenanceThread>(this);
      maintenanceThread_->resume();
    }
  }
  if( storagePeriodOfStatistics_ > 0 ){
    if( storagePeriodOfStatisticsThread_ == NULL || storagePeriodOfStatisticsThread_->finished() ){
      storagePeriodOfStatisticsThread_ = newObjectV1<MaintenanceThread>(this);
      storagePeriodOfStatisticsThread_->resume();
    }
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void Sniffer::MaintenanceThread::threadExecute()
{
  uint64_t ellapsed = gettimeofday(), ellapsed2;
  if( this == sniffer_->storagePeriodOfStatisticsThread_ ){
    AutoDatabaseDetach autoDatabaseDetach(sniffer_->database2_);
    if( statement_ == NULL ) statement_ = sniffer_->database2_->newAttachedStatement();
    if( stdErr.debugLevel(7) )
      stdErr.debug(7,utf8::String::Stream() <<
        "Interface: " << sniffer_->ifName() <<
        ", deleting out-of-date statistics begin, checkpoint: " <<
        utf8::time2Str(ellapsed - sniffer_->storagePeriodOfStatistics_ - getgmtoffset()) << "\n"
      );
    statement_->database()->start();
    for( intptr_t i = PCAP::pgpCount - 1; i >= 0; i-- ){
      statement_->text(
        "DELETE FROM INET_SNIFFER_STAT_" + utf8::String(pgpNames[i]) + " WHERE iface = :if AND ts < :ts"
      )->
        prepare()->
        paramAsString("if",sniffer_->ifName())->
        paramAsMutant("ts",ellapsed - sniffer_->storagePeriodOfStatistics_)->
        execute();
    }
    statement_->database()->commit();
    if( stdErr.debugLevel(7) )
      stdErr.debug(7,utf8::String::Stream() <<
        "Interface: " << sniffer_->ifName() << ", deleting out-of-date statistics end, ellapsed: " <<
        utf8::elapsedTime2Str(gettimeofday() - ellapsed) << "\n"
      );
    return;
  }
  if( stdErr.debugLevel(7) )
    stdErr.debug(7,utf8::String::Stream() <<
      "Interface: " << sniffer_->ifName() << ", database sweep helper begin ...\n"
    );

  utf8::String hostName, dbName;
  uintptr_t port;
  sniffer_->database2_->separateDBName(sniffer_->database2_->name(),hostName,dbName,port);
  utf8::String serviceName(hostName + (hostName.trim().isNull() ? "" : ":") + "service_mgr");
  fbcpp::Service service;
  service.params().
    add("user_name",sniffer_->user_).
    add("password",sniffer_->password_);
  service.attach(serviceName).
    request().
      add("action_svc_db_stats").
      add("dbname",dbName).
      add("options","sts_record_versions");
  service.invoke().detach();

  Vector<utf8::String> tables;
  utf8::String relationName, tr("total records: "), tv("total versions: ");
  uintmax_t totalRelationRecords, totalRelationVersions, * vp;
  for( uintptr_t i = 0; i < service.response().count(); i++ ){
    utf8::String s(service.response()[i].trim().replaceAll("\r","").replaceAll("\n",""));
    utf8::String::Iterator it(s.strstr(" ("));
    vp = NULL;
    if( it.eos() ){
      it = s.strcasestr(tr);
      if( it.eos() ){
        it = s.strcasestr(tv);
        if( it.eos() ) continue;
        it += 16;
        vp = &totalRelationVersions;
      }
      else {
        it += 15;
        vp = &totalRelationRecords;
      }
      sscanf(it.c_str(),"%"PRIuMAX,vp);
    }
    else {
      relationName = utf8::String(s,it);
      totalRelationRecords = totalRelationVersions = 0;
      continue;
    }
    if( !relationName.isNull() && vp == &totalRelationVersions ){
      ldouble q = totalRelationRecords > 0 ? totalRelationVersions * 100. / totalRelationRecords : 0;
      if( stdErr.debugLevel(7) )
        stdErr.debug(7,utf8::String::Stream() <<
          "Relation " <<
          relationName <<
          " statistics: records " << totalRelationRecords << ", versions " << totalRelationVersions <<
          ", ratio " <<
          utf8::String::print("%.4"PRF_LDBL"f",q) <<
          "\n"
        );
      if( q >= sniffer_->maintenanceThreshold_ ) tables.add(relationName);
      relationName.resize(0);
    }
  }

  AutoDatabaseDetach autoDatabaseDetach(sniffer_->database2_);
  if( statement_ == NULL ) statement_ = sniffer_->database2_->newAttachedStatement();
  //statement2_->execute(
  //  "SELECT\n"
  //  " RDB$RELATION_NAME\n"
  //  "FROM\n"
  //  " RDB$RELATIONS\n"
  //  "WHERE\n"
  //  " NOT (RDB$RELATION_NAME LIKE 'RDB$%' OR RDB$RELATION_NAME LIKE 'MON$%')\n"
  //)->fetchAll();
  //for( intptr_t i = statement2_->rowCount() - 1; i >= 0; i-- ){
  //  statement2_->selectRow(i);
  //  tables.add(statement2_->valueAsString(0).trimRight());
  //}
  for( intptr_t i = tables.count() - 1; i >= 0; i-- ){
    ellapsed2 = gettimeofday();
    statement_->execute("SELECT COUNT(*) FROM " + tables[i]);
    if( stdErr.debugLevel(7) )
      stdErr.debug(7,utf8::String::Stream() <<
        "Interface: " << sniffer_->ifName() <<
        ", 'SELECT COUNT(*) FROM " << tables[i] <<
        "' done, ellapsed: " <<
        utf8::elapsedTime2Str(gettimeofday() - ellapsed2) << "\n"
      );
  }
  if( stdErr.debugLevel(7) )
    stdErr.debug(7,utf8::String::Stream() <<
      "Interface: " << sniffer_->ifName() << ", database sweep helper end, ellapsed: " << utf8::elapsedTime2Str(gettimeofday() - ellapsed) << "\n"
    );

  ellapsed = gettimeofday();
  if( stdErr.debugLevel(7) )
    stdErr.debug(7,utf8::String::Stream() <<
      "Interface: " << sniffer_->ifName() << ", set indices statistics begin ...\n"
    );
  statement_->text(
    "SELECT\n"
    "  RDB$INDEX_NAME\n"
    "FROM\n"
    "  RDB$INDICES\n"
    "WHERE\n"
    " NOT (RDB$RELATION_NAME LIKE 'RDB$%' OR RDB$RELATION_NAME LIKE 'MON$%')\n"
  )->execute()->fetchAll();
  Array<utf8::String> indices;
  for( intptr_t i = statement_->rowCount() - 1; i >= 0; i-- ){
    statement_->selectRow(i);
    indices.add(statement_->valueAsString(0).trimRight());
  }
  for( intptr_t i = indices.count() - 1; i >= 0; i-- ){
    ellapsed2 = gettimeofday();
    statement_->execute("SET STATISTICS INDEX " + indices[i]);
    if( stdErr.debugLevel(7) )
      stdErr.debug(7,utf8::String::Stream() <<
        "Interface: " << sniffer_->ifName() <<
        ", 'SET STATISTICS INDEX " << indices[i] <<
        "' done, ellapsed: " <<
        utf8::elapsedTime2Str(gettimeofday() - ellapsed2) << "\n"
      );
  }
  if( stdErr.debugLevel(7) )
    stdErr.debug(7,utf8::String::Stream() <<
      "Interface: " << sniffer_->ifName() << ", set indices statistics end, ellapsed: " << utf8::elapsedTime2Str(gettimeofday() - ellapsed) << "\n"
    );

  statement_ = NULL;
  sniffer_->lastSweep_ = gettimeofday();
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
