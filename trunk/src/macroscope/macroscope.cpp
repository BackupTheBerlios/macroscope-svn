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
#define ENABLE_GD_INTERFACE 1
#define ENABLE_PCAP_INTERFACE 1
#define ENABLE_ODBC_INTERFACE 1
#define ENABLE_MYSQL_INTERFACE 1
#define ENABLE_FIREBIRD_INTERFACE 1
#include <adicpp/adicpp.h>
//------------------------------------------------------------------------------
#include "sniffer.h"
#include "macroscope.h"
//------------------------------------------------------------------------------
namespace macroscope {
//------------------------------------------------------------------------------
Logger::~Logger()
{
}
//------------------------------------------------------------------------------
Logger::Logger(bool sniffer,bool daemon) :
  config_(newObject<InterlockedConfig<InterlockedMutex> >()),
  sniffer_(sniffer),
  daemon_(daemon),
  configReaded_(false),
  trafCacheAutoDrop_(trafCache_),
  trafCacheSize_(0),
  dnsCacheAutoDrop_(dnsCache_),
  dnsCacheSize_(0)
{
  cgi_.contentType("text/html").initialize();
}
//------------------------------------------------------------------------------
Mutant Logger::timeStampRoundToMin(uint64_t ts)
{
  struct tm t = time2tm(ts);
  t.tm_sec = 0;
  return t;
}
//------------------------------------------------------------------------------
void Logger::printStat(int64_t lineNo,int64_t spos,int64_t pos,int64_t size,int64_t cl,int64_t * tma)
{
  if( verbose_ && (tma == NULL || getlocaltimeofday() - *tma >= 1000000) ){
    if( tma != NULL ) *tma = getlocaltimeofday();
    int64_t ct  = getlocaltimeofday() - cl;
    int64_t q   = (ct * (size - pos)) / (pos - spos <= 0 ? 1 : pos - spos);
    size -= spos;
    if( size <= 0 ) size = 1;
    pos -= spos;
    if( pos <= 0 ) pos = 1;
    if( ct <= 0 ) ct = 1;
    int64_t a = pos * 100 / size, b = lineNo * 1000000 / ct;
#ifdef HAVE__ISATTY
#ifdef HAVE__FILENO
    if( _isatty(_fileno(stderr)) ) fprintf(stderr, "\r");
#else
    if( _isatty(fileno(stderr)) )  fprintf(stderr, "\r");
#endif
#elif HAVE_ISATTY
#ifdef HAVE__FILENO
    if( isatty(_fileno(stderr)) ) fprintf(stderr, "\r");
#else
    if( isatty(fileno(stderr)) ) fprintf(stderr, "\r");
#endif
#endif
    fprintf(stderr,
      "%3"PRId64".%04"PRId64"%%, %7"PRId64".%04"PRId64" lps, ",
      a,
      pos * 1000000 / size - a * 10000,
      b,
      lineNo * INT64_C(10000000000) / ct - b * 10000
    );
    fprintf(stderr,
      "%s, elapsed: %-20s",
      (const char *) utf8::elapsedTime2Str(q).getOEMString(),
      (const char *) utf8::elapsedTime2Str(ct).getOEMString()
    );
#ifdef HAVE__ISATTY
#ifdef HAVE__FILENO
    if( !_isatty(_fileno(stderr)) )
#else
    if( !_isatty(fileno(stderr)) )
#endif
#elif HAVE_ISATTY
#ifdef HAVE__FILENO
    if( !isatty(_fileno(stderr)) )
#else
    if( !isatty(fileno(stderr)) )
#endif
#endif
    fprintf(stderr,"\n");
  //    printf("%-20s\n",utf8::elapsedTime2Str(size * ct / pos - ct).c_str());
  }
}
//------------------------------------------------------------------------------
int64_t Logger::fetchLogFileLastOffset(AutoPtr<Statement> st[3],const utf8::String & logFileName)
{
  if( !st[stSel]->prepared() ){
    st[stSel]->text(
      "SELECT " + utf8::String(dynamic_cast<MYSQLDatabase *>(st[stSel]->database()) != NULL ? " SQL_NO_CACHE" : "") +
      " ST_LAST_OFFSET FROM INET_LOG_FILE_STAT " 
      "WHERE ST_LOG_FILE_NAME = :ST_LOG_FILE_NAME"
    )->prepare();
  }
  st[stSel]->paramAsString("ST_LOG_FILE_NAME",logFileName)->execute();
  int64_t offset = 0;
  if( st[stSel]->fetch() )
    offset = st[stSel]->fetchAll()->valueAsMutant("ST_LAST_OFFSET");
  return offset;
}
//------------------------------------------------------------------------------
void Logger::updateLogFileLastOffset(AutoPtr<Statement> st[3],const utf8::String & logFileName,int64_t offset)
{
  if( !st[stSel]->prepared() ){
    st[stSel]->text(
      "SELECT " + utf8::String(dynamic_cast<MYSQLDatabase *>(st[stSel]->database()) != NULL ? " SQL_NO_CACHE" : "") +
      " ST_LAST_OFFSET FROM INET_LOG_FILE_STAT " 
      "WHERE ST_LOG_FILE_NAME = :ST_LOG_FILE_NAME"
    )->prepare();
  }
  st[stSel]->paramAsString("ST_LOG_FILE_NAME",logFileName)->execute();
  if( st[stSel]->fetch() ){
    st[stSel]->fetchAll();
    if( !st[stUpd]->prepared() ){
      st[stUpd]->text(
        "UPDATE INET_LOG_FILE_STAT SET ST_LAST_OFFSET = :ST_LAST_OFFSET " 
        "WHERE ST_LOG_FILE_NAME = :ST_LOG_FILE_NAME"
      )->prepare();
    }
    st[stUpd]->paramAsString("ST_LOG_FILE_NAME",logFileName)->
      paramAsMutant("ST_LAST_OFFSET",offset)->execute();
  }
  else {
    if( !st[stIns]->prepared() ){
      st[stIns]->text(
        "INSERT INTO INET_LOG_FILE_STAT" 
        "(ST_LOG_FILE_NAME, ST_LAST_OFFSET)" 
        "VALUES" 
        "(:ST_LOG_FILE_NAME,:ST_LAST_OFFSET)"
      )->prepare();
    }
    st[stIns]->paramAsString("ST_LOG_FILE_NAME",logFileName)->
      paramAsMutant("ST_LAST_OFFSET",offset)->execute();
  }
}
//------------------------------------------------------------------------------
void Logger::fallBackToNewLine(AsyncFile & f)
{
  if( f.seekable() && f.size() > 0 && f.tell() > 0 ){
    char c = '\0';
    if( f.size() <= f.tell() ) f.seek(f.size() - 1);
    for(;;){
      f.readBuffer(&c,1);
      if( c == '\n' || f.tell() < 2 ) break;
      f.seek(f.tell() - 2);
    }
  }
}
//------------------------------------------------------------------------------
void Logger::readConfig()
{
  if( configReaded_ ) return;
  config_->parse().override();
  stdErr.bufferDataTTA(
    (uint64_t) config_->value("debug_file_max_collection_time",60) * 1000000u
  );
  stdErr.rotationThreshold(
    config_->value("debug_file_rotate_threshold",1024 * 1024)
  );
  stdErr.rotatedFileCount(
    config_->value("debug_file_rotate_count",10)
  );
  stdErr.setDebugLevels(
    config_->value("debug_levels","+0,+1,+2,+3")
  );
  stdErr.fileName(
    config_->value("log_file",stdErr.fileName())
  );
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  checkMachineBinding(config_->value("machine_key"),true);
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

  verbose_ = config_->valueByPath("macroscope.verbose",false);

  if( cgi_.isCGI() )
    setProcessPriority(config_->valueByPath("macroscope.cgi_process_priority","IDLE_PRIORITY_CLASS"),true);
  else
    setProcessPriority(config_->valueByPath("macroscope.process_priority",getProcessPriority()),true);

  connection_ = config_->textByPath("macroscope.connection","default_connection");
  if( database_ == NULL ){
    ConfigSection dbParamsSection;
    dbParamsSection.addSection(config_->sectionByPath(connection_));

    database_ = Database::newDatabase(&dbParamsSection);
    statement_ = database_->newAttachedStatement();
    statement2_ = database_->newAttachedStatement();
  }
  configReaded_ = true;
}
//------------------------------------------------------------------------------
Logger & Logger::createDatabase()
{
  try {
    if( (bool) config_->section("macroscope").value("DROP_DATABASE",false) ){
      database_->attach();
      database_->drop();
      database_->create();
    }
    bool createDatabaseStructure = config_->valueByPath("macroscope.create_database_structure",true);
    if( createDatabaseStructure ){
      Vector<utf8::String> metadata;
      //if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL )
      //  metadata << "CREATE DOMAIN DATETIME AS TIMESTAMP";
      metadata <<
        "CREATE TABLE INET_USERS_TRAF ("
        " ST_USER               VARCHAR(80) CHARACTER SET ascii NOT NULL,"
        " ST_TIMESTAMP          DATETIME NOT NULL,"
        " ST_TRAF_WWW           BIGINT NOT NULL,"
        " ST_TRAF_SMTP          BIGINT NOT NULL"
        ")" <<
        "CREATE TABLE INET_USERS_MONTHLY_TOP_URL ("
        " URL_ID                CHAR(26) CHARACTER SET ascii NOT NULL,"
        " ST_USER               VARCHAR(80) CHARACTER SET ascii NOT NULL,"
        " ST_TIMESTAMP          DATETIME NOT NULL,"
        " ST_URL_TRAF           BIGINT NOT NULL,"
        " ST_URL_COUNT          BIGINT NOT NULL"
        ")" << 
        "CREATE TABLE INET_UMTU_INDEX ("
        " URL_ID                CHAR(26) CHARACTER SET ascii NOT NULL PRIMARY KEY,"
        " URL_HASH              BIGINT NOT NULL,"
        " URL                   VARCHAR(4096) NOT NULL"
        ")" <<
        "CREATE TABLE INET_SENDMAIL_MESSAGES ("
        " ST_USER               VARCHAR(80) CHARACTER SET ascii NOT NULL,"
        " ST_FROM               VARCHAR(240) CHARACTER SET ascii NOT NULL,"
        " ST_MSGID              VARCHAR(14) CHARACTER SET ascii NOT NULL PRIMARY KEY,"
        " ST_MSGSIZE            BIGINT NOT NULL,"
        " ST_NRCPTS             BIGINT NOT NULL"
        ")" <<
        "CREATE TABLE INET_USERS_TOP_MAIL ("
        " ST_USER               VARCHAR(80) CHARACTER SET ascii NOT NULL,"
        " ST_TIMESTAMP          DATETIME NOT NULL,"
        " ST_FROM               VARCHAR(240) CHARACTER SET ascii NOT NULL,"
        " ST_TO                 VARCHAR(240) CHARACTER SET ascii NOT NULL,"
        " ST_MAIL_TRAF          BIGINT NOT NULL,"
        " ST_MAIL_COUNT         BIGINT NOT NULL"
        ")" << 
        "CREATE TABLE INET_LOG_FILE_STAT ("
        " ST_LOG_FILE_NAME      VARCHAR(4096) NOT NULL,"
        " ST_LAST_OFFSET        BIGINT NOT NULL"
        ")" <<
        //"CREATE TABLE INET_BPFT_STAT_TOTALS ("
        //" st_if            CHAR(16) CHARACTER SET ascii NOT NULL,"
        //" st_start         DATETIME NOT NULL,"
        //" st_src_ip        CHAR(8) CHARACTER SET ascii NOT NULL,"
        //" st_dst_ip        CHAR(8) CHARACTER SET ascii NOT NULL,"
        //" st_ip_proto      SMALLINT NOT NULL,"
        //" st_src_port      INTEGER NOT NULL,"
        //" st_dst_port      INTEGER NOT NULL,"
        //" st_dgram_bytes   BIGINT NOT NULL,"
        //" st_data_bytes    BIGINT NOT NULL"
        //")" <<
        "CREATE TABLE INET_BPFT_STAT ("
        " st_if            CHAR(16) CHARACTER SET ascii NOT NULL,"
        " st_start         DATETIME NOT NULL,"
        " st_src_ip        CHAR(8) CHARACTER SET ascii NOT NULL,"
        " st_dst_ip        CHAR(8) CHARACTER SET ascii NOT NULL,"
        " st_ip_proto      SMALLINT NOT NULL,"
        " st_src_port      INTEGER NOT NULL,"
        " st_dst_port      INTEGER NOT NULL,"
        " st_dgram_bytes   BIGINT NOT NULL,"
        " st_data_bytes    BIGINT NOT NULL"
        ")" <<
        "CREATE TABLE INET_IFACES ("
        " iface            CHAR(16) CHARACTER SET ascii NOT NULL PRIMARY KEY"
        ")"
      ;
      utf8::String templ(
        "CREATE TABLE INET_SNIFFER_STAT_@0001@ ("
        " iface         CHAR(16) CHARACTER SET ascii NOT NULL,"
        " ts            DATETIME NOT NULL,"
        " src_ip        CHAR(8) CHARACTER SET ascii NOT NULL,"
        " src_port      INTEGER NOT NULL,"
        " dst_ip        CHAR(8) CHARACTER SET ascii NOT NULL,"
        " dst_port      INTEGER NOT NULL,"
        " ip_proto      SMALLINT NOT NULL,"
        " dgram         BIGINT NOT NULL,"
        " data          BIGINT NOT NULL"
        ")"
      );
      for( uintptr_t i = 0; i < PCAP::pgpCount; i++ ){
        metadata << templ.replaceAll("@0001@",Sniffer::pgpNames[i]);
      }
      for( uintptr_t i = 0; i < PCAP::pgpCount; i++ ){
        metadata <<
          //utf8::String("CREATE INDEX ISS_@0001@_01 ON INET_SNIFFER_STAT_@0001@ (iface)").replaceAll("@0001@",Sniffer::pgpNames[i]) <<
          //utf8::String("CREATE INDEX ISS_@0001@_02 ON INET_SNIFFER_STAT_@0001@ (ts)").replaceAll("@0001@",Sniffer::pgpNames[i]) <<
          //utf8::String("CREATE INDEX ISS_@0001@_03 ON INET_SNIFFER_STAT_@0001@ (src_ip)").replaceAll("@0001@",Sniffer::pgpNames[i]) <<
          //utf8::String("CREATE INDEX ISS_@0001@_04 ON INET_SNIFFER_STAT_@0001@ (src_port)").replaceAll("@0001@",Sniffer::pgpNames[i]) <<
          //utf8::String("CREATE INDEX ISS_@0001@_05 ON INET_SNIFFER_STAT_@0001@ (dst_ip)").replaceAll("@0001@",Sniffer::pgpNames[i]) <<
          //utf8::String("CREATE INDEX ISS_@0001@_06 ON INET_SNIFFER_STAT_@0001@ (dst_port)").replaceAll("@0001@",Sniffer::pgpNames[i]) <<
          //utf8::String("CREATE INDEX ISS_@0001@_07 ON INET_SNIFFER_STAT_@0001@ (ip_proto)").replaceAll("@0001@",Sniffer::pgpNames[i]) <<
          utf8::String("CREATE INDEX ISS_@0001@_08 ON INET_SNIFFER_STAT_@0001@ (iface,ts,src_ip,src_port,dst_ip,dst_port,ip_proto)").replaceAll("@0001@",Sniffer::pgpNames[i]) <<
          utf8::String("CREATE INDEX ISS_@0001@_09 ON INET_SNIFFER_STAT_@0001@ (iface,ts,src_ip,dst_ip)").replaceAll("@0001@",Sniffer::pgpNames[i])
        ;
        //if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL ){
        //  metadata <<
        //    utf8::String("CREATE DESC INDEX ISS_@0001@_08 ON INET_SNIFFER_STAT_@0001@ (iface,ts)").replaceAll("@0001@",Sniffer::pgpNames[i])
        //  ;
      }
      metadata <<
        /*"CREATE TABLE INET_BPFT_STAT_CACHE ("
        " st_if            CHAR(8) CHARACTER SET ascii NOT NULL,"
        " st_bt            DATETIME NOT NULL,"
        " st_et            DATETIME NOT NULL,"
        " st_src_ip        CHAR(8) CHARACTER SET ascii NOT NULL,"
        " st_dst_ip        CHAR(8) CHARACTER SET ascii NOT NULL,"
        " st_filter_hash   CHAR(43) CHARACTER SET ascii NOT NULL,"
        " st_threshold     BIGINT NOT NULL,"
        " st_dgram_bytes   BIGINT NOT NULL,"
        " st_data_bytes    BIGINT NOT NULL"
        ")" <<*/
        "CREATE TABLE INET_DNS_CACHE ("
        " st_ip            CHAR(8) CHARACTER SET ascii NOT NULL PRIMARY KEY,"
        " st_name          VARCHAR(" + utf8::int2Str(NI_MAXHOST + NI_MAXSERV + 1) + ") CHARACTER SET ascii NOT NULL"
        ")" <<

        "CREATE UNIQUE INDEX IUT_IDX1 ON INET_USERS_TRAF (ST_USER,ST_TIMESTAMP)" <<
        "CREATE INDEX IUT_IDX4 ON INET_USERS_TRAF (ST_TIMESTAMP)" <<
        "CREATE INDEX IUT_IDX3 ON INET_USERS_TRAF (ST_TRAF_SMTP,ST_TIMESTAMP)" <<
        "CREATE INDEX IUMTU_IDX1 ON INET_USERS_MONTHLY_TOP_URL (ST_USER,ST_TIMESTAMP,URL_ID)" <<
        "CREATE INDEX IUMTUI_IDX1 ON INET_UMTU_INDEX (URL_HASH)" <<
        "CREATE INDEX IUTM_IDX1 ON INET_USERS_TOP_MAIL (ST_USER,ST_TIMESTAMP)"
        //"CREATE INDEX IUTM_IDX1 ON INET_USERS_TOP_MAIL (ST_USER,ST_TIMESTAMP,ST_FROM,ST_TO)"
      ;
      utf8::String templ2;
      if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL ){
        metadata << "CREATE DESC INDEX IUT_IDX2 ON INET_USERS_TRAF (ST_TIMESTAMP)";
        templ =
          "CREATE PROCEDURE INET_UPDATE_SNIFFER_STAT_@0001@ (\n"
          "  ifaceP     CHAR(16) CHARACTER SET ascii,\n"
          "  ts0P       DATETIME,\n"
          "  ts1P       DATETIME,\n"
          "  ts2P       DATETIME,\n"
          "  ts3P       DATETIME,\n"
          "  ts4P       DATETIME,\n"
          "  ts5P       DATETIME,\n"
          "  ts6P       DATETIME,\n"
          "  src_ipP    CHAR(16) CHARACTER SET ascii,\n"
          "  src_portP  INTEGER,\n"
          "  dst_ipP    CHAR(16) CHARACTER SET ascii,\n"
          "  dst_portP  INTEGER,\n"
          "  protoP     SMALLINT,\n"
          "  dgramP     BIGINT,\n"
          "  dataP      BIGINT,\n"
          "  portsP     INTEGER,\n"
          "  protocolsP INTEGER,\n"
          "  mtP        INTEGER\n"
          ")\n"
          "AS\n"
          "  DECLARE dgram0    BIGINT;\n"
          "  DECLARE data0     BIGINT;\n"
          "  DECLARE cur@0001@ CURSOR FOR (\n"
          "    SELECT\n"
          "      dgram,data\n"
          "    FROM INET_SNIFFER_STAT_@0001@\n"
          "    WHERE\n"
          "      iface = :ifaceP AND ts = :ts@0003@P AND\n"
          "      src_ip = :src_ipP AND src_port = :src_portP AND\n"
          "      dst_ip = :dst_ipP AND dst_port = :dst_portP AND\n"
          "      ip_proto = :protoP\n"
          "    FOR UPDATE\n"
          "  );\n"
          "BEGIN\n"
          "  OPEN cur@0001@;\n"
          "  FETCH cur@0001@ INTO :dgram0,:data0;\n"
          "  IF (ROW_COUNT <> 0) THEN\n"
          "    UPDATE INET_SNIFFER_STAT_@0001@ SET\n"
          "      dgram = dgram + :dgramP, data = data + :dataP\n"
          "    WHERE\n"
          "      iface = :ifaceP AND ts = :ts@0003@P AND\n"
          "      src_ip = :src_ipP AND src_port = :src_portP AND\n"
          "      dst_ip = :dst_ipP AND dst_port = :dst_portP AND\n"
          "      ip_proto = :protoP;\n"
          "  ELSE\n"
          "    INSERT INTO INET_SNIFFER_STAT_@0001@\n"
          "      (iface,ts,src_ip,src_port,dst_ip,dst_port,ip_proto,dgram,data) VALUES\n"
          "      (:ifaceP,:ts@0003@P,:src_ipP,:src_portP,:dst_ipP,:dst_portP,:protoP,:dgramP,:dataP);\n"
          "  CLOSE cur@0001@;\n"
          "  IF (portsP <> 0 AND protocolsP <> 0) THEN\n"
          "  BEGIN\n"
          "    EXECUTE PROCEDURE INET_UPDATE_SNIFFER_STAT_@0001@(ifaceP,ts0P,ts1P,ts2P,ts3P,ts4P,ts5P,ts6P,src_ipP,0,dst_ipP,0,protoP,dgramP,dataP,0,0,6);\n"
          "    EXECUTE PROCEDURE INET_UPDATE_SNIFFER_STAT_@0001@(ifaceP,ts0P,ts1P,ts2P,ts3P,ts4P,ts5P,ts6P,src_ipP,src_portP,dst_ipP,dst_portP,-1,dgramP,dataP,0,0,6);\n"
          "  END\n"
          "  IF (portsP <> 0 OR protocolsP <> 0) THEN\n"
          "    EXECUTE PROCEDURE INET_UPDATE_SNIFFER_STAT_@0001@(ifaceP,ts0P,ts1P,ts2P,ts3P,ts4P,ts5P,ts6P,src_ipP,0,dst_ipP,0,-1,dgramP,dataP,0,0,6);\n"
          "@0002@"
          "END;\n"
        ;
        for( intptr_t i = Sniffer::pgpCount - 2; i >= 0; i-- ){
          templ2 += utf8::String(
            "  IF (mtP <= " + utf8::int2Str(i) + ") THEN\n"
            "    EXECUTE PROCEDURE INET_UPDATE_SNIFFER_STAT_@0001@(ifaceP,ts0P,ts1P,ts2P,ts3P,ts4P,ts5P,ts6P,src_ipP,src_portP,dst_ipP,dst_portP,protoP,dgramP,dataP,portsP,protocolsP,6);\n"
          ).replaceAll("@0001@",Sniffer::pgpNames[i]);
        }
      }
      else if( dynamic_cast<MYSQLDatabase *>(statement_->database()) != NULL ){
        metadata << "CREATE INDEX IUT_IDX2 ON INET_USERS_TRAF (ST_TIMESTAMP)";
        // partitioning
        metadata <<
          "alter table INET_SNIFFER_STAT_NSEC partition by hash(microsecond(ts)) partitions " <<
          "alter table INET_SNIFFER_STAT_SEC partition by hash(second(ts)) partitions " <<
          "alter table INET_SNIFFER_STAT_MIN partition by hash(minute(ts)) partitions " <<
          "alter table INET_SNIFFER_STAT_HOUR partition by hash(hour(ts)) partitions " <<
          "alter table INET_SNIFFER_STAT_DAY partition by hash(day(ts)) partitions " <<
          "alter table INET_SNIFFER_STAT_MON partition by hash(month(ts)) partitions " <<
          "alter table INET_SNIFFER_STAT_YEAR partition by hash(year(ts)) partitions " <<
          "alter table INET_USERS_TRAF partition by hash(month(ST_TIMESTAMP)) partitions " <<
          "alter table INET_USERS_MONTHLY_TOP_URL partition by hash(month(ST_TIMESTAMP)) partitions " <<
          "alter table INET_USERS_TOP_MAIL partition by hash(month(ST_TIMESTAMP)) partitions "
        ;
        templ =
          "CREATE PROCEDURE INET_UPDATE_SNIFFER_STAT_@0001@ (\n"
          "  IN ifaceP     CHAR(16) CHARACTER SET ascii,\n"
          "  IN ts0P       DATETIME,\n"
          "  IN ts1P       DATETIME,\n"
          "  IN ts2P       DATETIME,\n"
          "  IN ts3P       DATETIME,\n"
          "  IN ts4P       DATETIME,\n"
          "  IN ts5P       DATETIME,\n"
          "  IN ts6P       DATETIME,\n"
          "  IN src_ipP    CHAR(16) CHARACTER SET ascii,\n"
          "  IN src_portP  INTEGER,\n"
          "  IN dst_ipP    CHAR(16) CHARACTER SET ascii,\n"
          "  IN dst_portP  INTEGER,\n"
          "  IN protoP     SMALLINT,\n"
          "  IN dgramP     BIGINT,\n"
          "  IN dataP      BIGINT,\n"
          "  IN portsP     INTEGER,\n"
          "  IN protocolsP INTEGER,\n"
          "  IN mtP        INTEGER\n"
          ")\n"
          "BEGIN\n"
          "  DECLARE fetched   INTEGER DEFAULT 1;\n"
          "  DECLARE dgram0    BIGINT;\n"
          "  DECLARE data0     BIGINT;\n"
          "  DECLARE cur@0001@ CURSOR FOR\n"
          "    SELECT\n"
          "      dgram,data\n"
          "    FROM INET_SNIFFER_STAT_@0001@\n"
          "    WHERE\n"
          "      iface = ifaceP AND ts = ts@0003@P AND\n"
          "      src_ip = src_ipP AND src_port = src_portP AND\n"
          "      dst_ip = dst_ipP AND dst_port = dst_portP AND\n"
          "      ip_proto = protoP\n"
          "    FOR UPDATE;\n"
          "  DECLARE CONTINUE HANDLER FOR NOT FOUND SET fetched = 0;\n"
          "\n"
          "  OPEN cur@0001@;\n"
          "  FETCH cur@0001@ INTO dgram0,data0;\n"
          "  IF fetched THEN\n"
          "    UPDATE INET_SNIFFER_STAT_@0001@ SET\n"
          "      dgram = dgram + dgramP, data = data + dataP\n"
          "    WHERE\n"
          "      iface = ifaceP AND ts = ts@0003@P AND\n"
          "      src_ip = src_ipP AND src_port = src_portP AND\n"
          "      dst_ip = dst_ipP AND dst_port = dst_portP AND\n"
          "      ip_proto = protoP;\n"
          "  ELSE\n"
          "    INSERT INTO INET_SNIFFER_STAT_@0001@\n"
          "    (iface,ts,src_ip,src_port,dst_ip,dst_port,ip_proto,dgram,data) VALUES\n"
          "    (ifaceP,ts@0003@P,src_ipP,src_portP,dst_ipP,dst_portP,protoP,dgramP,dataP);\n"
          "  END IF;\n"
          "  CLOSE cur@0001@;\n"
          "  IF portsP AND protocolsP THEN\n"
          "    CALL INET_UPDATE_SNIFFER_STAT_@0001@(ifaceP,ts0P,ts1P,ts2P,ts3P,ts4P,ts5P,ts6P,src_ipP,0,dst_ipP,0,protoP,dgramP,dataP,0,0,6);\n"
          "    CALL INET_UPDATE_SNIFFER_STAT_@0001@(ifaceP,ts0P,ts1P,ts2P,ts3P,ts4P,ts5P,ts6P,src_ipP,src_portP,dst_ipP,dst_portP,-1,dgramP,dataP,0,0,6);\n"
          "  END IF;\n"
          "  IF portsP OR protocolsP THEN\n"
          "    CALL INET_UPDATE_SNIFFER_STAT_@0001@(ifaceP,ts0P,ts1P,ts2P,ts3P,ts4P,ts5P,ts6P,src_ipP,0,dst_ipP,0,-1,dgramP,dataP,0,0,6);\n"
          "  END IF;\n"
          "@0002@"
          "END\n"
        ;
        for( intptr_t i = Sniffer::pgpCount - 2; i >= 0; i-- ){
          templ2 += utf8::String(
            "  IF mtP <= " + utf8::int2Str(i) + " THEN\n"
            "    CALL INET_UPDATE_SNIFFER_STAT_@0001@(ifaceP,ts0P,ts1P,ts2P,ts3P,ts4P,ts5P,ts6P,src_ipP,src_portP,dst_ipP,dst_portP,protoP,dgramP,dataP,portsP,protocolsP,6);\n"
            "  END IF;\n"
          ).replaceAll("@0001@",Sniffer::pgpNames[i]);
        }
      }
      uintptr_t pos = metadata.count();
      for( intptr_t i = Sniffer::pgpCount - 1; i >= 0; i-- ){
        metadata.insert(pos,templ.replaceAll("@0001@",Sniffer::pgpNames[i]).replaceAll("@0002@",templ2).replaceAll("@0003@",utf8::int2Str(i)));
        templ2.resize(0);
      }
      database_->create();
      database_->attach();
      for( uintptr_t i = 0; i < metadata.count(); i++ ){
        if( dynamic_cast<MYSQLDatabase *>(statement_->database()) != NULL ){
          if( metadata[i].strncasecmp("CREATE TABLE",12) == 0 ){
            metadata[i] += " ENGINE = " + config_->textByPath("macroscope.mysql_table_type","INNODB");
          }
          else if( metadata[i].strncasecmp("ALTER TABLE",11) == 0 && !metadata[i].strcasestr(" partitions ").eos() ){
            Mutant m(config_->valueByPath("macroscope.mysql_table_partitions",8));
            if( (uintmax_t) m == 0 ) continue;
            metadata[i] += (utf8::String) m;
          }
        }
        else if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL ){
          metadata[i] = metadata[i].replaceAll("DATETIME","TIMESTAMP");
        }
        try {
          statement_->execute(metadata[i]);
        }
        catch( ExceptionSP & e ){
          //if( e->searchCode(isc_keytoobig) ) throw;
          if( e->searchCode(isc_dsql_error) ) throw;
          if( !e->searchCode(isc_no_meta_update,isc_random,ER_TABLE_EXISTS_ERROR,
                ER_DUP_KEYNAME,ER_BAD_TABLE_ERROR,ER_DUP_ENTRY_WITH_KEY_NAME) &&
              e->what().strcasestr("already exists").eos() ) throw;
        }
      }
      database_->detach();
    }

//#if !__FreeBSD__
    if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL &&
        (bool) config_->valueByPath(connection_ + ".firebird.set_properties",false) ){
      utf8::String hostName, dbName;
      uintptr_t port;
      database_->separateDBName(database_->name(),hostName,dbName,port);
      //  if( hostName.trim().strlen() > 0 ){
      utf8::String serviceName(hostName + (hostName.trim().strlen() > 0 ? ":" : "") + "service_mgr");
      fbcpp::Service service;
      service.params().
        add("user_name",config_->valueByPath(connection_ + ".firebird.user"));
      service.params().
        add("password",config_->valueByPath(connection_ + ".firebird.password"));
      try {
        service.attach(serviceName);

        service.request().clear().
          add("action_svc_properties").add("dbname",dbName).
          add("prp_set_sql_dialect",config_->valueByPath(connection_ + ".firebird.dialect",3));
        service.invoke();
        service.request().clear().
          add("action_svc_properties").add("dbname",dbName).
          add("prp_reserve_space",
            (bool) config_->valueByPath(connection_ + ".firebird.reserve_space",false) ?
              isc_spb_prp_res : isc_spb_prp_res_use_full
          );
        service.invoke();
        service.request().clear().add("action_svc_properties").add("dbname",dbName).
          add("prp_page_buffers",
            config_->valueByPath(connection_ + ".firebird.page_buffers",2048u)
          );
        service.invoke();
        service.request().clear().
          add("action_svc_properties").add("dbname", dbName).
          add("prp_write_mode",
            (bool) config_->valueByPath(connection_ + ".firebird.async_write",0) ?
              isc_spb_prp_wm_async : isc_spb_prp_wm_sync
          );
        service.invoke();
        service.attach(serviceName);
        service.request().clear().
          add("action_svc_properties").add("dbname", dbName).
          add("prp_sweep_interval",
            config_->valueByPath(connection_ + ".firebird.sweep_interval",10000u)
          );
        service.invoke();
      }
      catch( ExceptionSP & e ){
        if( !e->searchCode(isc_network_error) ) throw;
#ifndef NDEBUG
        e->writeStdError();
#endif
      }
    }
//#endif
  }
  catch( ExceptionSP & e ){
    if( sniffer_ ) e->writeStdError(); else throw;
  }
  return *this;
}
//------------------------------------------------------------------------------
void Logger::enumInterfaces(Statement * statement,Array<utf8::String> & ifaces)
{
  statement->text("SELECT iface FROM INET_IFACES ORDER BY iface")->execute();
  while( statement->fetch() ) ifaces.add(statement->valueAsString(0).trimRight());
}
//------------------------------------------------------------------------------
Logger & Logger::writeCGIInterfaceAndTimeSelect(bool addUnionIf)
{
  cgi_ <<
    "  <label for=\"if\">Interface</label>\n"
    "  <select name=\"if\" id=\"if\">\n"
  ;
  database_->attach()->start();
  Array<utf8::String> ifaces;
  enumInterfaces(statement_,ifaces);
  for( uintptr_t i = 0; i < ifaces.count(); i++ ){
    cgi_ << "    <option value=\"" + ifaces[i] + "\"";
    if( i == 0 && !addUnionIf ) cgi_ << " selected=\"selected\"";
    cgi_ << ">" + ifaces[i] + "</option>\n";
  }
  if( addUnionIf )
    cgi_ <<
      "    <option value=\"All\" selected=\"selected\">All</option>\n"
    ;
  database_->commit()->detach();
  cgi_ <<
    "  </select>\n"
    "  <BR>\n"
    "  <label for=\"bhour\">Begin time</label>\n"
  ;
  struct tm curTime = time2tm(getlocaltimeofday());
  cgi_ <<
    "  <select name=\"bhour\" id=\"bhour\">\n"
  ;
  for( intptr_t i = 0; i <= 23; i++ ){
    cgi_ << "    <option value=\"" + utf8::int2Str(i) + "\"";
    if( i == curTime.tm_hour ) cgi_ << " selected=\"selected\"";
    cgi_ << ">" + utf8::int2Str0(i,2) + "</option>\n";
  }
  cgi_ <<
    "  </select>\n"
    "  <select name=\"bmin\" id=\"bmin\">\n"
  ;
  for( intptr_t i = 0; i <= 59; i++ ){
    cgi_ << "    <option value=\"" + utf8::int2Str(i) + "\"";
    if( i == curTime.tm_min ) cgi_ << " selected=\"selected\"";
    cgi_ << ">" + utf8::int2Str0(i,2) + "</option>\n";
  }
  cgi_ <<
    "  </select>\n"
    "  <label for=\"bday\">date</label>\n"
    "  <select name=\"bday\" id=\"bday\">\n"
  ;
  for( intptr_t i = 1; i <= 31; i++ ){
    cgi_ << "    <option value=\"" + utf8::int2Str(i) + "\"";
    if( i == curTime.tm_mday ) cgi_ << " selected=\"selected\"";
    cgi_ << ">" + utf8::int2Str0(i,2) + "</option>\n";
  }
  cgi_ <<
    "  </select>\n"
    "  <select name=\"bmon\" id=\"bmon\">\n"
  ;
  for( intptr_t i = 1; i <= 12; i++ ){
    cgi_ << "    <option value=\"" + utf8::int2Str(i) + "\"";
    if( i == curTime.tm_mon + 1 ) cgi_ << " selected=\"selected\"";
    cgi_ << ">" + utf8::int2Str0(i,2) + "</option>\n";
  }
  cgi_ <<
    "  </select>\n"
    "  <select name=\"byear\" id=\"byear\">\n"
  ;
  for( intptr_t i = curTime.tm_year + 1900 - 25; i <= curTime.tm_year + 1900 + 25; i++ ){
    cgi_ << "    <option value=\"" + utf8::int2Str(i) + "\"";
    if( i == curTime.tm_year + 1900 ) cgi_ << " selected=\"selected\"";
    cgi_ << ">" + utf8::int2Str(i) + "</option>\n";
  }
  cgi_ <<
    "  </select>\n"
    "  <BR>\n"
  ;

  cgi_ <<
    "  <label for=\"ehour\">End time</label>\n"
    "  <select name=\"ehour\" id=\"ehour\">\n"
  ;
  for( intptr_t i = 0; i <= 23; i++ ){
    cgi_ << "    <option value=\"" + utf8::int2Str(i) + "\"";
    if( i == curTime.tm_hour ) cgi_ << " selected=\"selected\"";
    cgi_ << ">" + utf8::int2Str0(i,2) + "</option>\n";
  }
  cgi_ <<
    "  </select>\n"
    "  <select name=\"emin\" id=\"emin\">\n"
  ;
  for( intptr_t i = 0; i <= 59; i++ ){
    cgi_ << "    <option value=\"" + utf8::int2Str(i) + "\"";
    if( i == curTime.tm_min ) cgi_ << " selected=\"selected\"";
    cgi_ << ">" + utf8::int2Str0(i,2) + "</option>\n";
  }
  cgi_ <<
    "  </select>\n"
    "  <label for=\"eday\">date</label>\n"
    "  <select name=\"eday\" id=\"eday\">\n"
  ;
  for( intptr_t i = 1; i <= 31 /*(int) monthDays(curTime.tm_year + 1900,curTime.tm_mon)*/; i++ ){
    cgi_ << "    <option value=\"" + utf8::int2Str(i) + "\"";
    if( i == curTime.tm_mday ) cgi_ << " selected=\"selected\"";
    cgi_ << ">" + utf8::int2Str0(i,2) + "</option>\n";
  }
  cgi_ <<
    "  </select>\n"
    "  <select name=\"emon\" id=\"emon\">\n"
  ;
  for( intptr_t i = 1; i <= 12; i++ ){
    cgi_ << "    <option value=\"" + utf8::int2Str(i) + "\"";
    if( i == curTime.tm_mon + 1 ) cgi_ << " selected=\"selected\"";
    cgi_ << ">" + utf8::int2Str0(i,2) + "</option>\n";
  }
  cgi_ <<
    "  </select>\n"
    "  <select name=\"eyear\" id=\"eyear\">\n"
  ;
  for( intptr_t i = curTime.tm_year + 1900 - 25; i <= curTime.tm_year + 1900 + 25; i++ ){
    cgi_ << "    <option value=\"" + utf8::int2Str(i) + "\"";
    if( i == curTime.tm_year + 1900 ) cgi_ << " selected=\"selected\"";
    cgi_ << ">" + utf8::int2Str(i) + "</option>\n";
  }
  cgi_ <<
    "  </select>\n"
  ;
  return *this;
}
//------------------------------------------------------------------------------
void Logger::reactivateIndices(bool reactivate,bool setStat)
{
  database_->attach();
  if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL ){
    statement_->text(
      "SELECT"
      "  * "
      "FROM"
      "  RDB$INDICES "
      "WHERE"
      "  NOT RDB$RELATION_NAME LIKE 'RDB$%' "
      "ORDER BY RDB$RELATION_NAME DESC, RDB$INDEX_NAME DESC")->execute()->fetchAll();
    for( intptr_t i = statement_->rowCount() - 1; i >= 0; i-- ){
      statement_->selectRow(i);
      if( statement_->valueAsString("RDB$RELATION_NAME").strncasecmp("RDB$",4) == 0 ) continue;
      utf8::String tableName(statement_->valueAsString("RDB$RELATION_NAME").trimRight());
      utf8::String indexName(statement_->valueAsString("RDB$INDEX_NAME").trimRight());
      int64_t ellapsed;
      try {
        if( reactivate ){
          if( verbose_ ) fprintf(stderr,"Deactivate index %s",
            (const char *) indexName.getOEMString()
          );
          if( cgi_.isCGI() ) cgi_ << "Deactivate index " << indexName;
          ellapsed = gettimeofday();
          statement2_->text("ALTER INDEX " + indexName + " INACTIVE")->execute();
          if( verbose_ ) fprintf(stderr," done, ellapsed time: %s\n",
            (const char *) utf8::elapsedTime2Str(gettimeofday() - ellapsed).getOEMString()
          );
          if( cgi_.isCGI() )
            cgi_ << " done, ellapsed time: " << utf8::elapsedTime2Str(gettimeofday() - ellapsed) << "<BR>\n";
          if( verbose_ ) fprintf(stderr,"Activate index %s",
            (const char *) indexName.getOEMString()
          );
          if( cgi_.isCGI() ) cgi_ << "Activate index " << indexName;
          ellapsed = gettimeofday();
          statement2_->text("ALTER INDEX " + indexName + " ACTIVE")->execute();
          if( verbose_ ) fprintf(stderr," done, ellapsed time: %s\n",
            (const char *) utf8::elapsedTime2Str(gettimeofday() - ellapsed).getOEMString()
          );
          if( cgi_.isCGI() )
            cgi_ << " done, ellapsed time: " << utf8::elapsedTime2Str(gettimeofday() - ellapsed) << "<BR>\n";
        }
        if( setStat ){
          if( verbose_ ) fprintf(stderr,"Set statistics on index %s",
            (const char *) indexName.getOEMString()
          );
          ldouble statOld = (ldouble) statement_->valueAsMutant("rdb$statistics");
          if( cgi_.isCGI() )
            cgi_ << "Set statistics on table " << tableName << " index " << indexName <<
            " S(" << utf8::String::print("%-.*"PRF_LDBL"f",19,statOld) << ")";
          ellapsed = gettimeofday();
          statement2_->text("SET STATISTICS INDEX " + indexName)->execute();
          statement2_->text("SELECT rdb$statistics FROM RDB$INDICES WHERE RDB$INDEX_NAME = :index")->prepare()->
            paramAsString("index",indexName)->execute()->fetchAll();

          if( verbose_ ) fprintf(stderr," done, ellapsed time: %s\n",
            (const char *) utf8::elapsedTime2Str(gettimeofday() - ellapsed).getOEMString()
          );
          ldouble statNew = statement2_->valueAsMutant(0);
          if( cgi_.isCGI() )
            cgi_ << " done S(" <<
            (statOld != statNew ? "<FONT COLOR=\"#FF0000\">" : "") <<
            utf8::String::print("%-.*"PRF_LDBL"f",19,statNew) <<
            (statOld != statNew ? "</FONT>" : "") <<
            "), ellapsed time: " << utf8::elapsedTime2Str(gettimeofday() - ellapsed) << "<BR>\n";
        }
      }
      catch( ExceptionSP & e ){
        if( !e->searchCode(isc_integ_fail,isc_integ_deactivate_primary,isc_lock_conflict,isc_update_conflict) ) throw;
        if( verbose_ ){
          if( e->searchCode(isc_integ_deactivate_primary) ){
            fprintf(stderr," failed. Cannot deactivate index used by a PRIMARY/UNIQUE constraint.\n");
            if( cgi_.isCGI() )
              cgi_ << " failed. Cannot deactivate index used by a PRIMARY/UNIQUE constraint." << "<BR>\n";
          }
          else if( e->searchCode(isc_lock_conflict) ){
            fprintf(stderr," failed. Lock conflict on no wait transaction.\n");
            if( cgi_.isCGI() )
              cgi_ << " failed. Lock conflict on no wait transaction." << "<BR>\n";
          }
          else if( e->searchCode(isc_update_conflict) ){
            fprintf(stderr," failed. Update conflicts with concurrent update.\n");
            if( cgi_.isCGI() )
              cgi_ << " failed. Update conflicts with concurrent update." << "<BR>\n";
          }
        }
      }
    }
  }
  else if( dynamic_cast<MYSQLDatabase *>(statement_->database()) != NULL ){
    if( reactivate ){
      Table<utf8::String> tables;
      utf8::String hostName, dbName;
      uintptr_t port;
      database_->separateDBName(database_->name(),hostName,dbName,port);
      statement_->text(
        "SELECT table_name,table_schema "
        "FROM INFORMATION_SCHEMA.TABLES "
        "WHERE upper(table_schema) = :schema"
      )->prepare()->
        paramAsString("schema",dbName.upper())->
        execute()->fetchAll()->unloadByIndex(tables);
      //Table<Mutant> indices;
      utf8::String engine(config_->textByPath("macroscope.mysql_table_type","INNODB"));
      uint64_t ellapsed;
      for( intptr_t i = tables.rowCount() - 1; i >= 0; i-- ){
        /*statement_->text("SHOW INDEX FROM " + tables(i,0))->execute();
        while( statement_->fetch() )
    if( (intmax_t) statement_->valueAsMutant("Seq_in_index") == 1 )
      statement_->unloadRowByIndex(indices);*/
        if( verbose_ ) fprintf(stderr,"Alter table %s",(const char *) tables(i,0).getOEMString());
        if( cgi_.isCGI() ) cgi_ << "Alter table " << tables(i,0);
        ellapsed = gettimeofday();
        statement_->text("ALTER TABLE " + tables(i,0) + " ENGINE=" + engine)->execute();
        if( verbose_ ) fprintf(stderr," done, ellapsed time: %s\n",
          (const char *) utf8::elapsedTime2Str(gettimeofday() - ellapsed).getOEMString()
        );
        if( cgi_.isCGI() ) cgi_ << " done, ellapsed time: " << utf8::elapsedTime2Str(gettimeofday() - ellapsed) << "<BR>\n";
      }
    }
    /*for( intptr_t i = indices.rowCount() - 1; i >= 0; i-- ){
      utf8::String name(indices(i,"Key_name"));
      if( verbose_ ) fprintf(stderr,"Drop index %s",(const char *) name.getOEMString());
      int64_t ellapsed = gettimeofday();
      statement_->text("DROP INDEX " + name + " ON " + indices(i,"Table"))->execute();
      if( verbose_ ) fprintf(stderr," done, ellapsed time: %s\n",
        (const char *) utf8::elapsedTime2Str(gettimeofday() - ellapsed).getOEMString()
      );
    }*/
  }
  database_->detach();
}
//------------------------------------------------------------------------------
int32_t Logger::main()
{
  readConfig();
// print query form if is CGI and no CGI parameters
  if( cgi_.isCGI() ){
    if( cgi_.paramCount() == 0 || cgi_.paramIndex("admin") >= 0 ){
      cgi_ <<
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
        "<HTML>\n"
        "<HEAD>\n"
        "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
        "<meta http-equiv=\"Content-Language\" content=\"en\">\n"
        "<TITLE>Macroscope webinterface</TITLE>\n"
        "</HEAD>\n"
	      "<BODY LANG=EN BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\">\n"
      ;
      if( cgi_.paramIndex("admin") < 0 ){
        cgi_ <<
          "<FORM name=\"report_form\" ACTION=\"" + getEnv("SCRIPT_NAME") + "\"" + " METHOD=\"" + getEnv("USE_REQUEST_METHOD","GET") + "\" accept-charset=\"utf8\">\n"
          "  <B>Traffic report generation form</B>\n"
          "  <BR>\n"
        ;
        writeCGIInterfaceAndTimeSelect(false);
        cgi_ <<
          "  <BR>\n"
          "  <label for=\"max_totals\">Maximum totals</label>\n"
          "  <select name=\"max_totals\" id=\"max_totals\">\n"
          "    <option value=\"Min\">\n"
	        "      Min\n"
	        "    </option>\n"
          "    <option value=\"Hour\">\n"
	        "      Hour\n"
	        "    </option>\n"
          "    <option value=\"Day\" selected=\"selected\">\n"
	        "      Day\n"
	        "    </option>\n"
          "    <option value=\"Mon\">\n"
	        "      Mon\n"
	        "    </option>\n"
          "    <option value=\"Year\">\n"
	        "      Year\n"
	        "    </option>\n"
          "  </select>\n"
          "  <label for=\"min_totals\">Minimum totals</label>\n"
          "  <select name=\"min_totals\" id=\"min_totals\">\n"
          "    <option value=\"Min\">\n"
	        "      Min\n"
	        "    </option>\n"
          "    <option value=\"Hour\">\n"
	        "      Hour\n"
	        "    </option>\n"
          "    <option value=\"Day\" selected=\"selected\">\n"
	        "      Day\n"
	        "    </option>\n"
          "    <option value=\"Mon\">\n"
	        "      Mon\n"
	        "    </option>\n"
          "    <option value=\"Year\">\n"
	        "      Year\n"
	        "    </option>\n"
          "  </select>\n"
          "  <BR>\n"
          "  <input type=\"checkbox\" name=\"bidirectional\" id=\"bidirectional\" checked=\"checked\">\n"
          "  <label for=\"bidirectional\">Bidirectional</label>\n"
          "  <BR>\n"
          "  <input type=\"checkbox\" name=\"protocols\" id=\"protocols\" checked=\"checked\">\n"
          "  <label for=\"protocols\">Protocols</label>\n"
          "  <BR>\n"
          "  <input type=\"checkbox\" name=\"ports\" id=\"ports\" checked=\"checked\">\n"
          "  <label for=\"ports\">Ports</label>\n"
          "  <BR>\n"
          "  <input type=\"checkbox\" name=\"resolve\" id=\"resolve\" checked=\"checked\">\n"
          "  <label for=\"resolve\">Resolve addresses by DNS</label>\n"
          "  <BR>\n"
          "  <label for=\"threshold\">Minimal significant threshold</label>\n"
          "  <select name=\"threshold\" id=\"threshold\">\n"
          "    <option value=\"64K\">\n"
	        "      64K\n"
	        "    </option>\n"
          "    <option value=\"128K\">\n"
	        "      128K\n"
	        "    </option>\n"
          "    <option value=\"256K\">\n"
	        "      256K\n"
	        "    </option>\n"
          "    <option value=\"512K\">\n"
	        "      512K\n"
	        "    </option>\n"
          "    <option value=\"1M\">\n"
	        "      1M\n"
	        "    </option>\n"
          "    <option value=\"2M\">\n"
	        "      2M\n"
	        "    </option>\n"
          "    <option value=\"4M\" selected=\"selected\">\n"
	        "      4M\n"
	        "    </option>\n"
          "    <option value=\"8M\">\n"
	        "      8M\n"
	        "    </option>\n"
          "    <option value=\"16M\">\n"
	        "      16M\n"
	        "    </option>\n"
          "    <option value=\"32M\">\n"
	        "      32M\n"
	        "    </option>\n"
          "    <option value=\"64M\">\n"
	        "      64M\n"
	        "    </option>\n"
          "    <option value=\"128M\">\n"
	        "      128M\n"
	        "    </option>\n"
          "    <option value=\"256M\">\n"
	        "      256M\n"
	        "    </option>\n"
          "  </select>\n"
          "  <label for=\"threshold2\">or type you custom value in bytes</label>\n"
          "  <input type=\"text\" name=\"threshold2\" id=\"threshold2\">\n"
          "  <BR>\n"
	        "  <P>Please type address filter or leave empty</P>\n"
          "  <P>Example: src (amber or dst amber) and (src_port 80 or dst_port www) and proto tcp</P>\n"
	        "  <textarea name=\"filter\" rows=\"4\" cols=\"80\"></textarea>\n"
          "  <BR>\n"
          "  <BR>\n"
          "  <input name=\"report\" type=\"SUBMIT\" value=\"Start\">\n"
          "  <input name=\"admin\" type=\"SUBMIT\" value=\"Admin\">\n"
          "</FORM>\n"
        ;
      }
      else {
        cgi_ <<
          "<B>Administrative functions</B>\n"
          "<FORM name=\"admin_form\" ACTION=\"" + getEnv("SCRIPT_NAME") + "\"" + " METHOD=\"" + getEnv("USE_REQUEST_METHOD","GET") + "\" accept-charset=\"utf8\">\n"
        ;
        writeCGIInterfaceAndTimeSelect(true);
        cgi_ <<
          "  <BR>\n"
          "  <label for=\"newIfName\">New interface name</label>\n"
          "  <input type=\"text\" name=\"newIfName\" id=\"newIfName\">\n"
          "  <BR>\n"
          "  <label for=\"filter\">Filter</label>\n"
          "  <input type=\"text\" name=\"filter\" id=\"filter\">\n"
          "  <BR>\n"
          "  <label for=\"filter\">Connection</label>\n"
          "  <input type=\"text\" name=\"connection\" id=\"connection\">\n"
          "  <BR>\n"
          "  <input name=\"rolloutif\" type=\"SUBMIT\" value=\"Rollout\">\n"
          "  <input name=\"renameif\" type=\"SUBMIT\" value=\"Rename\">\n"
          "  <input name=\"copyif\" type=\"SUBMIT\" value=\"Copy\">\n"
          "  <input name=\"dropif\" type=\"SUBMIT\" value=\"Drop\">\n"
          "  <input name=\"dropdns\" type=\"SUBMIT\" value=\"Drop DNS cache\">\n"
          //"  <input name=\"recalc_totals\" type=\"SUBMIT\" value=\"Recalculate totals\">\n"
          "  <input name=\"reactivate_indices\" type=\"SUBMIT\" value=\"Reactivate indices\">\n"
          "  <input name=\"set_indices_statistics\" type=\"SUBMIT\" value=\"Set indices statistics\">\n"
          "</FORM>\n"
        ;
      }
      cgi_ <<
        "<HR>\n"
        "GMT: " + utf8::time2Str(gettimeofday()) + "\n<BR>\n"
        "Local time: " + utf8::time2Str(getlocaltimeofday()) + "\n<BR>\n"
        "Generated on " + getHostName(true,"Unknown") + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
#ifndef PRIVATE_RELEASE
        "<A HREF=\"http://developer.berlios.de/projects/macroscope/\">\n"
        "  http://developer.berlios.de/projects/macroscope/\n"
        "</A>\n"
#endif
	      "</BODY>\n"
	      "</HTML>\n"
      ;
      return 0;
    }
    verbose_ = false;
  }
  else {
    createDatabase();
  }
// parse log files
  int32_t err0 = doWork(0);
  int32_t err1 = waitThreads();
// optimize database (optional)
  if( !sniffer_ && !cgi_.isCGI() ){
    bool v0 = config_->section("macroscope").value("reactivate_indices",true);
    bool v1 = config_->section("macroscope").value("set_indices_statistics",true);
    reactivateIndices(v0,v1);
  }
// generate reports
  int32_t err2 = doWork(1);
  if( sniffer_ ) Thread::waitForSignal();
  int32_t err3 = waitThreads();
  return err0 != 0 ? err0 : err1 != 0 ? err1 : err2 != 0 ? err2 : err3;
}
//------------------------------------------------------------------------------
Sniffer * Logger::getSnifferBySection(const utf8::String & sectionName)
{
  utf8::String name("macroscope.bpft." + sectionName + ".sniffer.");
  ConfigSection dbParamsSection;
  utf8::String connection(config_->valueByPath(name + "connection",connection_));
  dbParamsSection.addSection(config_->sectionByPath(connection));
  AutoPtr<Database> database(Database::newDatabase(&dbParamsSection));
  AutoPtr<Database> database2(Database::newDatabase(&dbParamsSection));
  AutoPtr<Sniffer> sniffer(newObjectV1V2<Sniffer>(database.ptr(),database2.ptr()));
  database.ptr(NULL);
  database2.ptr(NULL);
  sniffer->ifName(sectionName);
  sniffer->iface(config_->textByPath(name + "interface"));
  sniffer->tempFile(config_->textByPath(name + "temp_file",sniffer->tempFile()));
  sniffer->filter(config_->textByPath(name + "filter"));
  sniffer->pcapReadTimeout(config_->valueByPath(name + "pcap_read_timeout",sniffer->pcapReadTimeout()));
  sniffer->swapThreshold(config_->valueByPath(name + "swap_threshold",sniffer->swapThreshold()));
  sniffer->pregroupingBufferSize(config_->valueByPath(name + "pregrouping_buffer_size",sniffer->pregroupingBufferSize()));
  sniffer->pregroupingWindowSize(config_->valueByPath(name + "pregrouping_window_size",sniffer->pregroupingWindowSize()));
  sniffer->promisc(config_->valueByPath(name + "promiscuous",sniffer->promisc()));
  sniffer->ports(config_->valueByPath(name + "ports",sniffer->ports()));
  sniffer->protocols(config_->valueByPath(name + "protocols",sniffer->protocols()));
  sniffer->gmtInLog(config_->valueByPath(name + "greenwich_mean_time_in_log",sniffer->gmtInLog()));
  sniffer->swapLowWatermark(config_->valueByPath(name + "swap_low_watermark",sniffer->swapLowWatermark()));
  sniffer->swapHighWatermark(config_->valueByPath(name + "swap_high_watermark",sniffer->swapHighWatermark()));
  sniffer->swapWatchTime((uint64_t) config_->valueByPath(name + "swap_watch_time",sniffer->swapWatchTime()) * 1000000u);
  sniffer->groupingPeriod(PCAP::stringToGroupingPeriod(config_->textByPath(name + "grouping_period","none")));
  sniffer->totalsPeriod(PCAP::stringToGroupingPeriod(config_->textByPath(name + "totals_period","day")));
  sniffer->packetsInTransaction(config_->valueByPath(name + "packets_in_transaction",0));
  sniffer->maintenance((uint64_t) config_->valueByPath(name + "maintenance",86400) * 1000000u);
  sniffer->maintenanceThreshold(config_->valueByPath(name + "maintenance_threshold",0.1));
  sniffer->user(dbParamsSection.textByPath(dbParamsSection.text("server_type","MYSQL").lower() + ".user"));
  sniffer->password(dbParamsSection.textByPath(dbParamsSection.text("server_type","MYSQL").lower() + ".password"));

  utf8::String joined(config_->textByPath(name + "join"));
  for( intptr_t i = 0, j = enumStringParts(joined); i < j; i++ ){
    if( !(bool) config_->valueByPath(name + "enabled",false) ) continue;
    sniffer->join(getSnifferBySection(stringPartByNo(joined,i)));
  }
  return sniffer.ptr(NULL);
}
//------------------------------------------------------------------------------
int32_t Logger::doWork(uintptr_t stage)
{
  int32_t exitCode = 0;
  if( sniffer_ && !cgi_.isCGI() ){
    if( stage == 1 ){
      bool ml;
#if HAVE_MLOCKALL
      ml = config_->valueByPath("macroscope.bpft.mlockall",false);
      if( ml && mlockall(MCL_FUTURE) != 0 ){
        int32_t err = errno;
        Exception e(err,"mlockall failed.");
	      e.writeStdError();
      }
#endif
      ml = config_->valueByPath("macroscope.bpft.mlockall",true);
      if( ml ) MemoryManager::globalMemoryManager().lock(true);
      utf8::String joined;
      for( uintptr_t i = 0; i < config_->sectionByPath("macroscope.bpft").sectionCount(); i++ ){
        utf8::String sectionName(config_->sectionByPath("macroscope.bpft").section(i).name());
        if( sectionName.strcasecmp("decoration") == 0 ) continue;
        if( !(bool) config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.enabled",false) ) continue;
        utf8::String join(config_->textByPath("macroscope.bpft." + sectionName + ".sniffer.join"));
        if( !join.isNull() ){
          if( !joined.isNull() ) joined += ",";
          joined += join;
          threads_.safeAdd(getSnifferBySection(sectionName));
          threads_[threads_.count() - 1].resume();
          //if( threads_.count() > 0 ) ksys::ksleep(2000000);
        }
      }
      for( uintptr_t i = 0; i < config_->sectionByPath("macroscope.bpft").sectionCount(); i++ ){
        utf8::String sectionName(config_->sectionByPath("macroscope.bpft").section(i).name());
        if( sectionName.strcasecmp("decoration") == 0 ) continue;
        if( !(bool) config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.enabled",false) ) continue;
        utf8::String join(config_->textByPath("macroscope.bpft." + sectionName + ".sniffer.join"));
        if( join.isNull() && findStringPart(joined,sectionName,false) < 0 ){
          threads_.safeAdd(getSnifferBySection(sectionName));
          threads_[threads_.count() - 1].resume();
          //if( threads_.count() > 0 ) ksys::ksleep(2000000);
        }
      }
    }
  }
  else if( stage == 1 && cgi_.isCGI() && cgi_.paramIndex("rolloutif") >= 0 ){
    uint64_t ellapsed = gettimeofday();
    struct tm cgiBT, cgiET;
    memset(&cgiBT,0,sizeof(cgiBT));
    cgiBT.tm_year = (int) cgi_.paramAsMutant("byear") - 1900;
    cgiBT.tm_mon = (int) cgi_.paramAsMutant("bmon") - 1;
    cgiBT.tm_mday = (int) cgi_.paramAsMutant("bday");
    memset(&cgiET,0,sizeof(cgiET));
    cgiET.tm_year = (int) cgi_.paramAsMutant("eyear") - 1900;
    cgiET.tm_mon = (int) cgi_.paramAsMutant("emon") - 1;
    cgiET.tm_mday = (int) cgi_.paramAsMutant("eday");
    cgiET.tm_hour = 23;
    cgiET.tm_min = 59;
    cgiET.tm_sec = 59;
    if( tm2Time(cgiBT) > tm2Time(cgiET) ) ksys::xchg(cgiBT,cgiET);
    rolloutBPFTByIPs(
      utf8::time2Str(tm2Time(cgiBT)),
      utf8::time2Str(tm2Time(cgiET) + 999999),
      cgi_.paramAsString("if").strcasecmp("all") == 0 ? utf8::String() : cgi_.paramAsString("if"),
      cgi_.paramAsString("filter")
    );
    cgi_ <<
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
      "<HTML>\n"
      "<HEAD>\n"
      "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
      "<meta http-equiv=\"Content-Language\" content=\"en\">\n"
      "<TITLE>Macroscope webinterface administrative function status report</TITLE>\n"
      "</HEAD>\n"
      "<BODY LANG=EN BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\">\n"
      "<B>Rollout operation completed successfuly.</B>\n"
      "<HR>\n"
      "GMT: " + utf8::time2Str(gettimeofday()) + "\n<BR>\n"
      "Local time: " + utf8::time2Str(getlocaltimeofday()) + "\n<BR>\n" +
      "Ellapsed time: " + utf8::elapsedTime2Str(uintmax_t(getlocaltimeofday() - ellapsed)) + "\n<BR>\n" +
      "Generated on " + getHostName(true,"Unknown") + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
#ifndef PRIVATE_RELEASE
      "<A HREF=\"http://developer.berlios.de/projects/macroscope/\">\n"
      "  http://developer.berlios.de/projects/macroscope/\n"
      "</A>\n"
#endif
      "</BODY>\n"
      "</HTML>\n"
    ;
  }
  else if( stage == 1 && cgi_.isCGI() && cgi_.paramIndex("renameif") >= 0 ){
    uint64_t ellapsed = gettimeofday();
    AutoDatabaseDetach autoDatabaseDetach(database_);
    bool all = cgi_.paramAsString("if").strcasecmp("all") == 0;
    database_->start();
    statement_->text("DELETE FROM INET_IFACES" + utf8::String(all ? "" : " WHERE iface = :if"));
    if( !all ) statement_->prepare()->paramAsString("if",cgi_.paramAsString("if"));
    statement_->execute();
    statement_->text("INSERT INTO INET_IFACES (iface) VALUES (:if)")->
      prepare()->paramAsString("if",cgi_.paramAsString("newIfName"))->execute();
    for( intptr_t i = PCAP::pgpCount - 1; i >= 0; i-- ){
      statement_->text("UPDATE INET_SNIFFER_STAT_" + utf8::String(Sniffer::pgpNames[i]) +
        " SET iface = :newif" + utf8::String(all ? "" : " WHERE iface = :if"))->
        prepare()->paramAsString("newif",cgi_.paramAsString("newIfName"));
      if( !all ) statement_->paramAsString("if",cgi_.paramAsString("if"));
      statement_->execute();
    }
    database_->commit();
    cgi_ <<
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
      "<HTML>\n"
      "<HEAD>\n"
      "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
      "<meta http-equiv=\"Content-Language\" content=\"en\">\n"
      "<TITLE>Macroscope webinterface administrative function status report</TITLE>\n"
      "</HEAD>\n"
      "<BODY LANG=EN BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\">\n"
      "<B>Rename operation completed successfuly.</B>\n"
      "<HR>\n"
      "GMT: " + utf8::time2Str(gettimeofday()) + "\n<BR>\n"
      "Local time: " + utf8::time2Str(getlocaltimeofday()) + "\n<BR>\n" +
      "Ellapsed time: " + utf8::elapsedTime2Str(uintmax_t(gettimeofday() - ellapsed)) + "\n<BR>\n" +
      "Generated on " + getHostName(true,"Unknown") + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
#ifndef PRIVATE_RELEASE
      "<A HREF=\"http://developer.berlios.de/projects/macroscope/\">\n"
      "  http://developer.berlios.de/projects/macroscope/\n"
      "</A>\n"
#endif
      "</BODY>\n"
      "</HTML>\n"
    ;
  }
  else if( stage == 1 && cgi_.isCGI() && cgi_.paramIndex("copydb") >= 0 ){
    uint64_t ellapsed = gettimeofday();
    AutoDatabaseDetach autoDatabaseDetach(database_);

    ConfigSection dbParamsSection;
    utf8::String connection(cgi_.paramAsString("connection"));
    dbParamsSection.addSection(config_->sectionByPath(connection));
    AutoPtr<Database> database2(Database::newDatabase(&dbParamsSection));
    
    database2->attach();

    database_->start();
    database2->start();

    database2->commit();
    database_->commit();
    cgi_ <<
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
      "<HTML>\n"
      "<HEAD>\n"
      "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
      "<meta http-equiv=\"Content-Language\" content=\"en\">\n"
      "<TITLE>Macroscope webinterface administrative function status report</TITLE>\n"
      "</HEAD>\n"
      "<BODY LANG=EN BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\">\n"
      "<B>Copy database operation completed successfuly.</B>\n"
      "<HR>\n"
      "GMT: " + utf8::time2Str(gettimeofday()) + "\n<BR>\n"
      "Local time: " + utf8::time2Str(getlocaltimeofday()) + "\n<BR>\n" +
      "Ellapsed time: " + utf8::elapsedTime2Str(uintmax_t(gettimeofday() - ellapsed)) + "\n<BR>\n" +
      "Generated on " + getHostName(true,"Unknown") + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
#ifndef PRIVATE_RELEASE
      "<A HREF=\"http://developer.berlios.de/projects/macroscope/\">\n"
      "  http://developer.berlios.de/projects/macroscope/\n"
      "</A>\n"
#endif
      "</BODY>\n"
      "</HTML>\n"
    ;
  }
  else if( stage == 1 && cgi_.isCGI() && cgi_.paramIndex("copyif") >= 0 ){
    uint64_t ellapsed = gettimeofday();
    AutoDatabaseDetach autoDatabaseDetach(database_);

    ConfigSection dbParamsSection;
    utf8::String connection(cgi_.paramAsString("connection"));
    dbParamsSection.addSection(config_->sectionByPath(connection));
    AutoPtr<Database> database2(Database::newDatabase(&dbParamsSection));
    
    if( !connection.isNull() ) database2->attach();

    bool all = cgi_.paramAsString("if").strcasecmp("all") == 0;
    database_->start();
    if( connection.isNull() ){
      statement_->text("INSERT INTO INET_IFACES (iface) VALUES (:if)")->
        prepare()->paramAsString("if",cgi_.paramAsString("newIfName"))->execute();
      for( intptr_t i = PCAP::pgpCount - 1; i >= 0; i-- ){
        statement_->text(
          "INSERT INTO INET_SNIFFER_STAT_" + utf8::String(Sniffer::pgpNames[i]) +
          "  SELECT '" + cgi_.paramAsString("newIfName") + "' as iface, ts,"
          "    src_ip,dst_ip,ip_proto,"
          "    src_port,dst_port,"
          "    dgram,data"
          "  FROM INET_SNIFFER_STAT_" + utf8::String(Sniffer::pgpNames[i]) +
          utf8::String(all ? "" : " WHERE iface = :if"))->prepare();
        if( !all ) statement_->paramAsString("if",cgi_.paramAsString("if"));
        try {
          statement_->execute();
        }
        catch( ksys::ExceptionSP & e ){
          if( !e->searchCode(isc_convert_error) ) throw;
          statement_->text(
            "  SELECT ts,"
            "    src_ip,dst_ip,ip_proto,"
            "    src_port,dst_port,"
            "    dgram,data"
            "  FROM INET_SNIFFER_STAT_" + utf8::String(Sniffer::pgpNames[i]) +
            utf8::String(all ? "" : " WHERE iface = :if"))->prepare();
          if( !all ) statement_->paramAsString("if",cgi_.paramAsString("if"));
          statement_->execute()->fetchAll();
          statement2_->text(
            "INSERT INTO INET_SNIFFER_STAT_" + utf8::String(Sniffer::pgpNames[i]) +
            "(iface,ts,src_ip,dst_ip,ip_proto,src_port,dst_port,dgram,data) VALUES "
            "(:if,:ts,:src_ip,:dst_ip,:ip_proto,:src_port,:dst_port,:dgram,:data)"
          )->prepare()->paramAsString("if",cgi_.paramAsString("newIfName"));
          for( intptr_t j = statement_->rowCount() - 1; j >= 0; j-- ){
            statement_->selectRow(j);
            statement2_->
              paramAsMutant("ts",statement_->valueAsMutant("ts"))->
              paramAsMutant("src_ip",statement_->valueAsMutant("src_ip"))->
              paramAsMutant("src_port",statement_->valueAsMutant("src_port"))->
              paramAsMutant("dst_ip",statement_->valueAsMutant("dst_ip"))->
              paramAsMutant("dst_port",statement_->valueAsMutant("dst_port"))->
              paramAsMutant("ip_proto",statement_->valueAsMutant("ip_proto"))->
              paramAsMutant("dgram",statement_->valueAsMutant("dgram"))->
              paramAsMutant("data",statement_->valueAsMutant("data"))->
              execute();
          }
        }
      }
    }
    else {
      AutoPtr<Statement> stdb2_(database2->newAttachedStatement());
      database2->start();
      stdb2_->text("INSERT INTO INET_IFACES (iface) VALUES (:if)")->prepare();
      if( all ){
        statement_->execute("SELECT iface FROM INET_IFACES")->fetchAll();
        for( intptr_t j = statement_->rowCount() - 1; j >= 0; j-- ){
          statement_->selectRow(j);
          stdb2_->paramAsString("if",statement_->valueAsString(0))->execute();
        }
      }
      else {
        stdb2_->paramAsString("if",cgi_.paramAsString("newIfName"))->execute();
      }
      for( intptr_t i = PCAP::pgpCount - 1; i >= 0; i-- ){
        statement_->text(
          "  SELECT iface, ts,"
          "    src_ip,dst_ip,ip_proto,"
          "    src_port,dst_port,"
          "    dgram,data"
          "  FROM INET_SNIFFER_STAT_" + utf8::String(Sniffer::pgpNames[i]) +
          utf8::String(all ? "" : " WHERE iface = :if"))->prepare();
        if( !all ) statement_->paramAsString("if",cgi_.paramAsString("if"));
        statement_->execute()->fetchAll();
        stdb2_->text(
          "INSERT INTO INET_SNIFFER_STAT_" + utf8::String(Sniffer::pgpNames[i]) +
          "(iface,ts,src_ip,dst_ip,ip_proto,src_port,dst_port,dgram,data) VALUES "
          "(:if,:ts,:src_ip,:dst_ip,:ip_proto,:src_port,:dst_port,:dgram,:data)"
        )->prepare();
        for( intptr_t j = statement_->rowCount() - 1; j >= 0; j-- ){
          statement_->selectRow(j);
          stdb2_->paramAsString("if",all ? statement_->valueAsString("iface") : cgi_.paramAsString("newIfName"));
          stdb2_->
            paramAsMutant("ts",statement_->valueAsMutant("ts"))->
            paramAsMutant("src_ip",statement_->valueAsString("src_ip"))->
            paramAsMutant("src_port",statement_->valueAsMutant("src_port"))->
            paramAsMutant("dst_ip",statement_->valueAsString("dst_ip"))->
            paramAsMutant("dst_port",statement_->valueAsMutant("dst_port"))->
            paramAsMutant("ip_proto",statement_->valueAsMutant("ip_proto"))->
            paramAsMutant("dgram",statement_->valueAsMutant("dgram"))->
            paramAsMutant("data",statement_->valueAsMutant("data"))->
            execute();
        }
      }
      database2->commit();
    }
    database_->commit();
    cgi_ <<
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
      "<HTML>\n"
      "<HEAD>\n"
      "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
      "<meta http-equiv=\"Content-Language\" content=\"en\">\n"
      "<TITLE>Macroscope webinterface administrative function status report</TITLE>\n"
      "</HEAD>\n"
      "<BODY LANG=EN BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\">\n"
      "<B>Copy operation completed successfuly.</B>\n"
      "<HR>\n"
      "GMT: " + utf8::time2Str(gettimeofday()) + "\n<BR>\n"
      "Local time: " + utf8::time2Str(getlocaltimeofday()) + "\n<BR>\n" +
      "Ellapsed time: " + utf8::elapsedTime2Str(uintmax_t(gettimeofday() - ellapsed)) + "\n<BR>\n" +
      "Generated on " + getHostName(true,"Unknown") + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
#ifndef PRIVATE_RELEASE
      "<A HREF=\"http://developer.berlios.de/projects/macroscope/\">\n"
      "  http://developer.berlios.de/projects/macroscope/\n"
      "</A>\n"
#endif
      "</BODY>\n"
      "</HTML>\n"
    ;
  }
  else if( stage == 1 && cgi_.isCGI() && cgi_.paramIndex("dropif") >= 0 ){
    uint64_t ellapsed = gettimeofday();
    AutoDatabaseDetach autoDatabaseDetach(database_);
    utf8::String filter(cgi_.paramAsString("filter"));
    bool all = cgi_.paramAsString("if").strcasecmp("all") == 0;
    database_->start();
    for( intptr_t i = PCAP::pgpCount - 1; i >= 0; i-- ){
      statement_->text("DELETE FROM INET_SNIFFER_STAT_" + utf8::String(Sniffer::pgpNames[i]) +
        utf8::String(all ? filter.isNull() ? utf8::String() : " WHERE " + getIPFilter(filter) : filter.isNull() ? " WHERE iface = :if" : " WHERE iface = :if AND " + getIPFilter(filter)))->prepare();
      if( !all ) statement_->paramAsString("if",cgi_.paramAsString("if"));
      statement_->execute();
    }
    if( filter.isNull() ){
      statement_->text("DELETE FROM INET_IFACES" + utf8::String(all ? "" : " WHERE iface = :if"));
      if( !all ) statement_->prepare()->paramAsString("if",cgi_.paramAsString("if"));
      statement_->execute();
    }
    database_->commit();
    cgi_ <<
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
      "<HTML>\n"
      "<HEAD>\n"
      "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
      "<meta http-equiv=\"Content-Language\" content=\"en\">\n"
      "<TITLE>Macroscope webinterface administrative function status report</TITLE>\n"
      "</HEAD>\n"
      "<BODY LANG=EN BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\">\n"
      "<B>Drop operation completed successfuly.</B>\n"
      "<HR>\n"
      "GMT: " + utf8::time2Str(gettimeofday()) + "\n<BR>\n"
      "Local time: " + utf8::time2Str(getlocaltimeofday()) + "\n<BR>\n" +
      "Ellapsed time: " + utf8::elapsedTime2Str(uintmax_t(gettimeofday() - ellapsed)) + "\n<BR>\n" +
      "Generated on " + getHostName(true,"Unknown") + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
#ifndef PRIVATE_RELEASE
      "<A HREF=\"http://developer.berlios.de/projects/macroscope/\">\n"
      "  http://developer.berlios.de/projects/macroscope/\n"
      "</A>\n"
#endif
      "</BODY>\n"
      "</HTML>\n"
    ;
  }
  else if( stage == 1 && cgi_.isCGI() && cgi_.paramIndex("dropdns") >= 0 ){
    uint64_t ellapsed = gettimeofday();
    AutoDatabaseDetach autoDatabaseDetach(database_);
    database_->start();
    statement_->text("delete from INET_DNS_CACHE")->execute();
    database_->commit();
    cgi_ <<
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
      "<HTML>\n"
      "<HEAD>\n"
      "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
      "<meta http-equiv=\"Content-Language\" content=\"en\">\n"
      "<TITLE>Macroscope webinterface administrative function status report</TITLE>\n"
      "</HEAD>\n"
      "<BODY LANG=EN BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\">\n"
      "<B>Drop DNS cache operation completed successfuly.</B>\n"
      "<HR>\n"
      "GMT: " + utf8::time2Str(gettimeofday()) + "\n<BR>\n"
      "Local time: " + utf8::time2Str(getlocaltimeofday()) + "\n<BR>\n" +
      "Ellapsed time: " + utf8::elapsedTime2Str(uintmax_t(gettimeofday() - ellapsed)) + "\n<BR>\n" +
      "Generated on " + getHostName(true,"Unknown") + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
#ifndef PRIVATE_RELEASE
      "<A HREF=\"http://developer.berlios.de/projects/macroscope/\">\n"
      "  http://developer.berlios.de/projects/macroscope/\n"
      "</A>\n"
#endif
      "</BODY>\n"
      "</HTML>\n"
    ;
  }
  else if( stage == 1 && cgi_.isCGI() && cgi_.paramIndex("recalc_totals") >= 0 ){
    uint64_t ellapsed = gettimeofday();
    bool all = cgi_.paramAsString("if").strcasecmp("all") == 0;
    Array<utf8::String> ifaces;
    if( all ){
      database_->attach()->start();
      enumInterfaces(statement_,ifaces);
      database_->commit()->detach();
    }
    else {
      ifaces.add(cgi_.paramAsString("if"));
    }
    for( uintptr_t i = 0; i < ifaces.count(); i++ ){
      AutoPtr<Sniffer> sniffer(getSnifferBySection(ifaces[i]));
      sniffer->recalcTotals();
    }
    cgi_ <<
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
      "<HTML>\n"
      "<HEAD>\n"
      "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
      "<meta http-equiv=\"Content-Language\" content=\"en\">\n"
      "<TITLE>Macroscope webinterface administrative function status report</TITLE>\n"
      "</HEAD>\n"
      "<BODY LANG=EN BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\">\n"
      "<B>Recalculation totals operation completed successfuly.</B>\n"
      "<HR>\n"
      "GMT: " + utf8::time2Str(gettimeofday()) + "\n<BR>\n"
      "Local time: " + utf8::time2Str(getlocaltimeofday()) + "\n<BR>\n" +
      "Ellapsed time: " + utf8::elapsedTime2Str(uintmax_t(gettimeofday() - ellapsed)) + "\n<BR>\n" +
      "Generated on " + getHostName(true,"Unknown") + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
#ifndef PRIVATE_RELEASE
      "<A HREF=\"http://developer.berlios.de/projects/macroscope/\">\n"
      "  http://developer.berlios.de/projects/macroscope/\n"
      "</A>\n"
#endif
      "</BODY>\n"
      "</HTML>\n"
    ;
  }
  else if( stage == 1 && cgi_.isCGI() && cgi_.paramIndex("reactivate_indices") >= 0 ){
    uint64_t ellapsed = gettimeofday();
    cgi_ <<
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
      "<HTML>\n"
      "<HEAD>\n"
      "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
      "<meta http-equiv=\"Content-Language\" content=\"en\">\n"
      "<TITLE>Macroscope webinterface administrative function status report</TITLE>\n"
      "</HEAD>\n"
      "<BODY LANG=EN BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\">\n"
    ;
    reactivateIndices(true,false);
    cgi_ <<
      "<BR>\n"
      "<B>Reactivate indices operation completed successfuly.</B>\n"
      "<HR>\n"
      "GMT: " + utf8::time2Str(gettimeofday()) + "\n<BR>\n"
      "Local time: " + utf8::time2Str(getlocaltimeofday()) + "\n<BR>\n" +
      "Ellapsed time: " + utf8::elapsedTime2Str(uintmax_t(gettimeofday() - ellapsed)) + "\n<BR>\n" +
      "Generated on " + getHostName(true,"Unknown") + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
#ifndef PRIVATE_RELEASE
      "<A HREF=\"http://developer.berlios.de/projects/macroscope/\">\n"
      "  http://developer.berlios.de/projects/macroscope/\n"
      "</A>\n"
#endif
      "</BODY>\n"
      "</HTML>\n"
    ;
  }
  else if( stage == 1 && cgi_.isCGI() && cgi_.paramIndex("set_indices_statistics") >= 0 ){
    uint64_t ellapsed = gettimeofday();
    cgi_ <<
      "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
      "<HTML>\n"
      "<HEAD>\n"
      "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
      "<meta http-equiv=\"Content-Language\" content=\"en\">\n"
      "<TITLE>Macroscope webinterface administrative function status report</TITLE>\n"
      "</HEAD>\n"
      "<BODY LANG=EN BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\">\n"
    ;
    reactivateIndices(false,true);
    cgi_ <<
      "<BR>\n"
      "<B>Set indices statistics operation completed successfuly.</B>\n"
      "<HR>\n"
      "GMT: " + utf8::time2Str(gettimeofday()) + "\n<BR>\n"
      "Local time: " + utf8::time2Str(getlocaltimeofday()) + "\n<BR>\n" +
      "Ellapsed time: " + utf8::elapsedTime2Str(uintmax_t(gettimeofday() - ellapsed)) + "\n<BR>\n" +
      "Generated on " + getHostName(true,"Unknown") + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
#ifndef PRIVATE_RELEASE
      "<A HREF=\"http://developer.berlios.de/projects/macroscope/\">\n"
      "  http://developer.berlios.de/projects/macroscope/\n"
      "</A>\n"
#endif
      "</BODY>\n"
      "</HTML>\n"
    ;
  }
  else if( stage == 1 && cgi_.isCGI() && cgi_.paramIndex("chart") >= 0 ){
    cgi_.contentType("image/png");
    GDChart chart;
    intptr_t row = cgi_.paramAsMutant("rc",0);
    for( --row; row >= 0; row-- ){
      intptr_t col = cgi_.paramAsMutant("r" + utf8::int2Str(row) + "cc",0);
      if( chart.data().count() <= (uintptr_t) row ) chart.data().resize(row + 1);
      for( --col; col >= 0; col-- ){
        while( chart.data()[row].count() <= (uintptr_t) col ) chart.data()[row].add(0);
        chart.data()[row][col] = cgi_.paramAsMutant("r" + utf8::int2Str(row) + "c" + utf8::int2Str(col),0);
      }
    }
    chart.xlvs(cgi_.paramAsMutant("xlvs",0));
    chart.width(cgi_.paramAsMutant("width",640));
    chart.height(cgi_.paramAsMutant("height",480));
    chart.createChart();
    cgi_.writeBuffer(chart.png(),chart.pngSize());
  }
  else {
    utf8::String mtMode(config_->textByPath("macroscope.multithreaded_mode","BOTH"));
    if( !cgi_.isCGI() ){
      trafCacheSize_ = config_->valueByPath("macroscope.html_report.traffic_cache_size",0);
      threads_.safeAdd(newObjectR1C2C3C4<SquidSendmailThread>(*this,"macroscope","macroscope",stage));
      if( mtMode.strcasecmp("BOTH") == 0 ||
          (stage == 0 && mtMode.strcasecmp("FILL") == 0) ||
          (stage == 1 && mtMode.strcasecmp("REPORT") == 0) ){
        threads_[threads_.count() - 1].resume();
      }
      else {
        threads_[threads_.count() - 1].threadExecute();
      }
    }
    dnsCacheHitCount_ = 0;
    dnsCacheMissCount_ = 0;
    dnsCacheSize_ = config_->valueByPath("macroscope.bpft.dns_cache_size",0);
    for( uintptr_t i = 0; i < config_->sectionByPath("macroscope.bpft").sectionCount() || cgi_.isCGI(); i++ ){
      utf8::String sectionName(cgi_.isCGI() ? cgi_.paramAsString("if") : config_->sectionByPath("macroscope.bpft").section(i).name());
      if( sectionName.strcasecmp("decoration") == 0 ) continue;
      if( cgi_.isCGI() && (stage < 1 || cgi_.paramIndex("report") < 0) ) break;
      threads_.safeAdd(
        newObjectR1C2C3C4<BPFTThread>(
          *this,"macroscope.bpft." + sectionName,sectionName,stage)
        );
      if( !cgi_.isCGI() && 
          (mtMode.strcasecmp("BOTH") == 0 ||
            (stage == 0 && mtMode.strcasecmp("FILL") == 0) ||
            (stage == 1 && mtMode.strcasecmp("REPORT") == 0)
          )
      ){
        threads_[threads_.count() - 1].resume();
      }
      else {
        threads_[threads_.count() - 1].threadExecute();
      }
      if( cgi_.isCGI() ) break;
    }
  }
  return exitCode;
}
//------------------------------------------------------------------------------
int32_t Logger::waitThreads()
{
  int32_t exitCode = 0;
  for( intptr_t i = threads_.count() - 1; i >= 0; i-- ){
    threads_[i].wait();
    if( exitCode == 0 && threads_[i].exitCode() != 0 ){
      exitCode = (int32_t) threads_[i].exitCode();
      if( exitCode >= errorOffset ) exitCode -= exitCode;
    }
  }
  threads_.clear();
  return exitCode;
}
//------------------------------------------------------------------------------
Logger & Logger::rolloutBPFTByIPs(const utf8::String & bt,const utf8::String & et,const utf8::String & ifName,const utf8::String & filter)
{
  Mutant btt(0), ett(0);
  if( !bt.isNull() ) btt = utf8::str2Time(bt) - getgmtoffset();
  if( !et.isNull() ) ett = utf8::str2Time(et) - getgmtoffset();
  btt.changeType(mtTime);
  ett.changeType(mtTime);
  readConfig();
  AutoDatabaseDetach autoDatabaseDetach(database_);
  database_->start();
  utf8::String range;
  if( !ifName.isNull() ) range += " iface = :if";
  if( (uint64_t) btt != 0 ){
    if( !ifName.isNull() ) range += " AND";
    range += " ts >= :bt";
  }
  if( (uint64_t) ett != 0 ){
    if( (uint64_t) btt != 0 ) range += " AND";
    range += " ts <= :et";
  }
  if( !range.isNull() ) range += " AND";
  range = " WHERE" + range + " (src_port <> 0 OR dst_port <> 0 OR ip_proto <> -1)";
  range += filter.isNull() ? utf8::String() : " AND " + getIPFilter(filter);
  for( intptr_t i = PCAP::pgpCount - 1; i >= 0; i-- ){
    statement_->text("DELETE FROM INET_SNIFFER_STAT_" + utf8::String(Sniffer::pgpNames[i]) + range)->prepare();
    if( !ifName.isNull() ) statement_->paramAsString("if",ifName);
    if( (uint64_t) btt != 0 ) statement_->paramAsMutant("bt",btt);
    if( (uint64_t) ett != 0 ) statement_->paramAsMutant("et",ett);
    statement_->execute();
  }
  database_->commit();
  return *this;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
SnifferService::SnifferService() : logger_(NULL)
{
#if defined(__WIN32__) || defined(__WIN64__)
  serviceType_ = SERVICE_WIN32_OWN_PROCESS;
  startType_ = SERVICE_AUTO_START;
  errorControl_ = SERVICE_ERROR_IGNORE;
  binaryPathName_ = getExecutableName();
  serviceStatus_.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
  serviceStatus_.dwWaitHint = 60000; // give me 600 seconds for start or stop 
#endif
}
//------------------------------------------------------------------------------
void SnifferService::install()
{
  logger_->readConfig();
  serviceName_ = logger_->config_->textByPath("macroscope.service_name","macroscope");
  displayName_ = logger_->config_->textByPath("macroscope.service_display_name","Macroscope Packet Collection Service");
#if defined(__WIN32__) || defined(__WIN64__)
  utf8::String startType(logger_->config_->textByPath("macroscope.service_start_type","auto"));
  if( startType.strcasecmp("auto") == 0 ){
    startType_ = SERVICE_AUTO_START;
  }
  else if( startType.strcasecmp("manual") == 0 ){
    startType_ = SERVICE_DEMAND_START;
  }
#endif
}
//------------------------------------------------------------------------------
void SnifferService::uninstall()
{
  install();
}
//------------------------------------------------------------------------------
void SnifferService::start()
{
  install();
  logger_->createDatabase();
  logger_->doWork(1);
  stdErr.debug(0,utf8::String::Stream() << macroscope_version.gnu_ << " started (" << serviceName_ << ")\n");
}
//------------------------------------------------------------------------------
void SnifferService::stop()
{
  logger_->waitThreads();
  stdErr.debug(0,utf8::String::Stream() << macroscope_version.gnu_ << " stopped (" << serviceName_ << ")\n");
}
//------------------------------------------------------------------------------
bool SnifferService::active()
{
  return logger_ != NULL && logger_->threads_.count() > 0;
}
//------------------------------------------------------------------------------
} // namespace macroscope
//------------------------------------------------------------------------------
//#if HAVE__MALLOC_OPTIONS
//const char * _malloc_options = "HR";
//#endif
//------------------------------------------------------------------------------
int main(int _argc,char * _argv[])
{
  //Sleep(15000);

  int errcode = EINVAL;
  adicpp::AutoInitializer autoInitializer(_argc,_argv);
  autoInitializer = autoInitializer;

  bool isDaemon = isDaemonCommandLineOption(), isCGI = false;
  if( isDaemon ) daemonize();
  utf8::String::Stream stream;
  try {
    uintptr_t i;
    stdErr.fileName(SYSLOG_DIR("macroscope/") + "macroscope.log");
    Config::defaultFileName(SYSCONF_DIR("") + "macroscope.conf");
    Config::defaultFileName(getEnv("MACROSCOPE_CONFIG").isNull() ? Config::defaultFileName() : getEnv("MACROSCOPE_CONFIG"));
    Services services(macroscope_version.gnu_);
    AutoPtr<macroscope::SnifferService> serviceAP(newObject<macroscope::SnifferService>());
    services.add(serviceAP);
    macroscope::SnifferService * service = serviceAP.ptr(NULL);
    bool dispatch = true, sniffer = false, svc = false, rollout = false, install = false, uninstall = false;
    utf8::String pidFileName, rolloutParams;
    for( i = 1; i < argv().count(); i++ ){
      if( argv()[i].strcmp("--chdir") == 0 && i + 1 < argv().count() ){
        changeCurrentDir(argv()[i + 1]);
      }
      else if( argv()[i].strcmp("-c") == 0 && i + 1 < argv().count() ){
        Config::defaultFileName(argv()[i + 1]);
      }
      else if( argv()[i].strcmp("--log") == 0 && i + 1 < argv().count() ){
        stdErr.fileName(argv()[i + 1]);
      }
      else if( argv()[i].strcmp("--pid") == 0 && i + 1 < argv().count() ){
        pidFileName = argv()[i + 1];
      }
      else if( argv()[i].strcmp("--query") == 0 && i + 1 < argv().count() ){
        setEnv("GATEWAY_INTERFACE","CGI/1.1");
        setEnv("REQUEST_METHOD","GET");
        setEnv("QUERY_STRING",argv()[i + 1]);
        setEnv("CONTENT_TYPE","");
      }
    }
    for( i = 1; i < argv().count(); i++ ){
      if( argv()[i].strcmp("--version") == 0 ){
        stdErr.debug(9,utf8::String::Stream() << macroscope_version.tex_ << "\n");
        fprintf(stdout,"%s\n",macroscope_version.tex_);
        dispatch = false;
        continue;
      }
      if( argv()[i].strcmp("--sniffer") == 0 ){
        sniffer = true;        
      }
      else if( argv()[i].strcmp("--iflist") == 0 ){
        PCAP::printAllDevices();
        dispatch = false;
      }
      else if( argv()[i].strncmp("--rollout-sniffer-db=",21) == 0 ){
        rolloutParams = utf8::String(utf8::String::Iterator(argv()[i]) + 21);
        rollout = true;
        dispatch = false;
      }
      else if( argv()[i].strcmp("--benchmark") == 0 ){
        heapBenchmark();
//        RBTreeBenchmarkTree tree;
//        tree.benchmark(10000000,3);
        dispatch = false;
      }
      else if( argv()[i].strcmp("--service") == 0 ){
        svc = true;
      }
      else if( argv()[i].strcmp("--install") == 0 ){
        for( uintptr_t j = i + 1; j < argv().count(); j++ )
          if( argv()[j].isSpace() )
            service->args(service->args() + " \"" + argv()[j] + "\"");
          else
            service->args(service->args() + " " + argv()[j]);
        install = true;
        dispatch = false;
      }
      else if( argv()[i].strcmp("--uninstall") == 0 ){
        uninstall = true;
        dispatch = false;
      }
#if PRIVATE_RELEASE
      else if( argv()[i].strcmp("--machine-key") == 0 ){
        utf8::String key(getMachineCleanUniqueKey());
        fprintf(stdout,"%s\n",(const char *) key.getOEMString());
        copyStrToClipboard(key);
        dispatch = false;
      }
#endif
    }
#ifndef NDEBUG
    //static const uint8_t text[] = 
    //  "0123456789ABCDEF"
    //  "0123456789ABCDEF"
    //  "FEDCBA9876543210"
    //  "FEDCBA9876543210"
    //;
    //uint8_t buf[1024];
    //uint8_t dText[sizeof(text)];
//    AutoPtr<uint8_t> cText;
//    LZWFilter lzw;
    
    //Vector<utf8::String> fileNamesVector;
    //getDirList(fileNamesVector,"c:/windows/system32/*.dll");
    //getDirList(fileNamesVector,"c:/windows/system32/*.exe");
    //getDirList(fileNamesVector,getExecutableName());
    //getDirList(fileNamesVector,"G:/Library/Коллекция/*.txt");
    //Array<utf8::String> fileNames(fileNamesVector);
    //lzw.genStatisticTable(fileNames);
    
//    lzw.initialize();
    //lzw.compress(text,sizeof(text),true);
    //cText.xchg(lzw.out());
    //uintptr_t outSize = lzw.outSize();
    //lzw.initialize();
    //lzw.decompress(cText,outSize,true);
    //dText[0] = dText[0];
    //HuffmanFilter filter;
    //filter.compressFile("G:/Library/Коллекция/V_ANECD/V_ANECD.TXT",getExecutableName() + ".compressed");
    //LZMAFilter filter;
    //filter.initializeCompression();
    //filter.compress(text,sizeof(text));
    //filter.finishCompression();
    //LZMAFileFilter fileFilter;
    //for( uintptr_t ii = 0; ii < 1000; ii++ ){
    //  fileFilter.compressFile("G:/Library/Коллекция/V_ANECD/V_ANECD.TXT",getExecutablePath() + "V_ANECD.TXT.compressed");
    //  fileFilter.decompressFile(getExecutablePath() + "V_ANECD.TXT.compressed",getExecutablePath() + "V_ANECD.TXT.decompressed");
    //}
#endif
    errcode = 0;
    if( dispatch || sniffer || rollout || install || uninstall ){
#ifndef NDEBUG
      /*setEnv("GATEWAY_INTERFACE","CGI/1.1");
      setEnv("REQUEST_METHOD","GET");
      setEnv("QUERY_STRING",""
        //"if=pleh&bday=7&bmon=12&byear=2007&eday=7&emon=12&eyear=2007&totals=Day&bidirectional=on&protocols=on&ports=on&threshold=1M&threshold2=&filter=src+87.242.73.67+or+dst+87.242.73.67%0D%0A&report=Start"*/
        /*"if=win_test&"
        "bday=01&bmon=12&byear=2007&"
        "eday=31&emon=12&eyear=2007&"
        "resolve=off&"
        "bidirectional=on&"
        "protocols=off&"
        "ports=off&"
        "threshold=4M&"
        "threshold2=&"
        "totals=Day&"
        "filter=&"
        //"filter=(src+amber+or+dst+amber)+and+(src_port+8010+or+dst_port+8010)+and+proto+tcp&"
        "report=Start"
      );*/
#endif
      macroscope::Logger logger(sniffer,isDaemon);
      isCGI = logger.cgi().isCGI();
      if( dispatch && svc && sniffer ){
        service->logger_ = &logger;
        services.startServiceCtrlDispatcher();
      }
      else if( dispatch ){
        if( !pidFileName.isNull() ){
          AsyncFile pidFile(pidFileName);
  	      pidFile.createIfNotExist(true).open() << utf8::int2Str(ksys::getpid());
	        pidFile.resize(pidFile.size());
        }
        stdErr.debug(0,utf8::String::Stream() << macroscope_version.gnu_ << " started" << (isCGI ? " (CGI)" : "") << "\n");
        errcode = logger.main();
        stdErr.debug(0,utf8::String::Stream() << macroscope_version.gnu_ << " stopped" << (isCGI ? " (CGI)" : "") << "\n");
      }
      else if( rollout ){
        logger.rolloutBPFTByIPs(
          stringPartByNo(rolloutParams,0).trim(),
          stringPartByNo(rolloutParams,1).trim(),
          stringPartByNo(rolloutParams,2),
          utf8::String()
        );
      }
      else if( install ){
        service->logger_ = &logger;
        services.install();
      }
      else if( uninstall ){
        service->logger_ = &logger;
        services.uninstall();
      }
    }
  }
  catch( ExceptionSP & e ){
    if( isCGI ){
      fprintf(stdout,"%s%s\n%s%s\n%s",
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
        "<HTML>\n"
        "<HEAD>\n"
        "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
        "<meta http-equiv=\"Content-Language\" content=\"en\">\n"
        "<TITLE>Macroscope webinterface error report</TITLE>\n"
        "</HEAD>\n"
        "<BODY LANG=EN BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\">\n",
        (const char *) e->stdError().getANSIString(),
        macroscope_version.gnu_ ,
        " terminated with error(s), see above.\n",
        (const char *) utf8::String("<HR>\n"
        "GMT: " + utf8::time2Str(gettimeofday()) + "\n<BR>\n"
        "Local time: " + utf8::time2Str(getlocaltimeofday()) + "\n<BR>\n"
        "Generated on " + getHostName(true,"Unknown") + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
#ifndef PRIVATE_RELEASE
        "<A HREF=\"http://developer.berlios.de/projects/macroscope/\">\n"
        "  http://developer.berlios.de/projects/macroscope/\n"
        "</A>\n"
#endif
        "</BODY>\n"
        "</HTML>\n").getANSIString()
      );
    }
    e->writeStdError();
    stdErr.debug(0,stream << macroscope_version.gnu_ << " terminated with error(s), see above.\n");
    errcode = e->code() >= errorOffset ? e->code() - errorOffset : e->code();
  }
  catch( ... ){
  }
  return errcode;
}
//------------------------------------------------------------------------------
