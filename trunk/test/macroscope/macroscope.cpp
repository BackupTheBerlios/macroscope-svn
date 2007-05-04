/*-
 * Copyright 2006-2007 Guram Dukashvili
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
#include "sniffer.h"
#ifndef NDEBUG
#include <adicpp/lzw.h>
#endif
//------------------------------------------------------------------------------
namespace macroscope {
//------------------------------------------------------------------------------
Logger::~Logger()
{
}
//------------------------------------------------------------------------------
Logger::Logger() :
  config_(newObject<InterlockedConfig<InterlockedMutex> >()),
  trafCacheAutoDrop_(trafCache_),
  trafCacheSize_(0),
  dnsCacheAutoDrop_(dnsCache_),
  dnsCacheSize_(0)
{
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
  else{
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
  config_->parse().override();
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
  setProcessPriority(config_->valueByPath("macroscope.process_priority",getProcessPriority()));
}
//------------------------------------------------------------------------------
int32_t Logger::main(bool sniffer,bool daemon)
{
  sniffer_ = sniffer;
  daemon_ = daemon;

  readConfig();

  ConfigSection dbParamsSection;
  dbParamsSection.addSection(config_->sectionByPath("libadicpp.default_connection"));

  database_ = Database::newDatabase(&dbParamsSection);
  statement_ = database_->newAttachedStatement();

// print query form if is CGI and no CGI parameters
  /*setEnv("GATEWAY_INTERFACE","CGI/1.1");
  setEnv("REQUEST_METHOD","GET");
  setEnv("QUERY_STRING",
    "if=test&"
    "bday=04&bmon=5&byear=2007&"
    "eday=04&emon=5&eyear=2007&"
    "resolve=on&"
    "bidirectional=on&"
    "protocols=on&"
    "ports=on&"
    "threshold=64K&"
    "threshold2=&"
    "totals=Day&"
    "filter=(src+amber+or+dst+amber)+and+(src_port+8010+or+dst_port+8010)+and+proto+tcp"
  );*/
/*#if !defined(NDEBUG) && (defined(__WIN32__) || defined(__WIN64__))
  LPWSTR pEnv = (LPWSTR) GetEnvironmentStringsW();
  while( wcslen(pEnv) > 0 ){
    stdErr.debug(9,utf8::String::Stream() << utf8::String(pEnv) << "\n");
	  pEnv += wcslen(pEnv) + 1;
  }
  FreeEnvironmentStrings(pEnv);
  stdErr.flush(true);
#endif*/
  cgi_.initialize();
  if( cgi_.isCGI() ){
    if( cgi_.paramCount() == 0 ){
      cgi_ <<
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
        "<HTML>\n"
        "<HEAD>\n"
        "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
        "<meta http-equiv=\"Content-Language\" content=\"en\">\n"
        "<TITLE>Statistics query form</TITLE>\n"
        "</HEAD>\n"
	      "<BODY LANG=EN BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\">\n"
        "<FORM ACTION=\"" + getEnv("SCRIPT_NAME") + "\"" + " METHOD=\"POST\" accept-charset=\"utf8\">\n"
        "  <label for=\"if\">Interface</label>\n"
        "  <select name=\"if\" id=\"if\">\n"
      ;
      database_->attach()->start();
      statement_->text("select distinct st_if from INET_BPFT_STAT")->execute();
      while( statement_->fetch() ){
//      for( uintptr_t i = 0; i < config_->sectionByPath("macroscope.bpft").sectionCount(); i++ ){
//        utf8::String sectionName(config_->sectionByPath("macroscope.bpft").section(i).name());
        utf8::String sectionName(statement_->valueAsString("st_if"));
//        if( sectionName.strcasecmp("decoration") == 0 ) continue;
        cgi_ << "    <option value=\"" + sectionName + "\"";
        if( statement_->rowIndex() == 0 ) cgi_ << " selected=\"selected\"";
        cgi_ << ">" + sectionName + "</option>\n";
      }
      database_->commit()->detach();
      cgi_ <<
        "  </select>\n"
	      "  <BR>\n"
        "  <label for=\"bday\">Begin time</label>\n"
	      "  <select name=\"bday\" id=\"bday\">\n"
      ;
      struct tm curTime = time2tm(gettimeofday());
      for( intptr_t i = 1; i <= 31 /*(int) monthDays(curTime.tm_year + 1900,curTime.tm_mon)*/; i++ ){
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
        "  <label for=\"eday\">End time</label>\n"
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
        "  <BR>\n"
        "  <label for=\"totals\">Maximum totals</label>\n"
        "  <select name=\"totals\" id=\"totals\">\n"
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
        "  </select>\n"
        "  <label for=\"threshold2\">or type you custom value in bytes</label>\n"
        "  <input type=\"text\" name=\"threshold2\" id=\"threshold2\">\n"
        "  <BR>\n"
	      "  <P>Please type address filter or leave empty</P>\n"
        "  <BR>\n"
        "  <P>Example: src amber or dst amber) and (src_port 80 or dst_port www) and proto tcp</P>\n"
	      "  <textarea name=\"filter\" rows=\"4\" cols=\"80\"></textarea>\n"
      ;
      cgi_ <<
        "  <BR>\n"
        "  <BR>\n"
        "  <INPUT TYPE=\"SUBMIT\" VALUE=\"Start\">\n"
        "</FORM>\n"
	      "</BODY>\n"
	      "</HTML>\n"
      ;
      return 0;
    }
    verbose_ = false;
  }
  else {
    statement2_ = database_->newAttachedStatement();

    database_->create();

    Vector<utf8::String> metadata;
    if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL )
      metadata << "CREATE DOMAIN DATETIME AS TIMESTAMP";
    metadata <<
  //    "DROP TABLE INET_USERS_TRAF" <<
      "CREATE TABLE INET_USERS_TRAF ("
      " ST_USER               VARCHAR(80) CHARACTER SET ascii NOT NULL,"
      " ST_TIMESTAMP          DATETIME NOT NULL,"
      " ST_TRAF_WWW           INTEGER NOT NULL,"
      " ST_TRAF_SMTP          INTEGER NOT NULL"
      ")" <<
  //    "DROP TABLE INET_USERS_MONTHLY_TOP_URL" <<
      "CREATE TABLE INET_USERS_MONTHLY_TOP_URL ("
      " ST_USER               VARCHAR(80) CHARACTER SET ascii NOT NULL,"
      " ST_TIMESTAMP          DATETIME NOT NULL,"
      " ST_URL                VARCHAR(4096) NOT NULL,"
      " ST_URL_HASH           BIGINT NOT NULL,"
      " ST_URL_TRAF           INTEGER NOT NULL,"
      " ST_URL_COUNT          INTEGER NOT NULL"
      ")" << 
  //    "DROP TABLE INET_SENDMAIL_MESSAGES" <<
      "CREATE TABLE INET_SENDMAIL_MESSAGES ("
      " ST_FROM               VARCHAR(240) CHARACTER SET ascii NOT NULL,"
      " ST_MSGID              VARCHAR(14) CHARACTER SET ascii NOT NULL PRIMARY KEY,"
      " ST_MSGSIZE            INTEGER NOT NULL"
      ")" <<
      "CREATE TABLE INET_LOG_FILE_STAT ("
      " ST_LOG_FILE_NAME      VARCHAR(4096) NOT NULL,"
      " ST_LAST_OFFSET        BIGINT NOT NULL"
      ")" <<
      "CREATE TABLE INET_BPFT_STAT ("
      " st_if            CHAR(8) CHARACTER SET ascii NOT NULL,"
      " st_start         DATETIME NOT NULL,"
      " st_src_ip        CHAR(8) CHARACTER SET ascii NOT NULL,"
      " st_dst_ip        CHAR(8) CHARACTER SET ascii NOT NULL,"
      " st_ip_proto      SMALLINT NOT NULL,"
      " st_src_port      INTEGER NOT NULL,"
      " st_dst_port      INTEGER NOT NULL,"
      " st_dgram_bytes   BIGINT NOT NULL,"
      " st_data_bytes    BIGINT NOT NULL"
      ")" <<
      "CREATE TABLE INET_BPFT_STAT_CACHE ("
      " st_if            CHAR(8) CHARACTER SET ascii NOT NULL,"
      " st_bt            DATETIME NOT NULL,"
      " st_et            DATETIME NOT NULL,"
      " st_src_ip        CHAR(8) CHARACTER SET ascii NOT NULL,"
      " st_dst_ip        CHAR(8) CHARACTER SET ascii NOT NULL,"
      " st_filter_hash   CHAR(43) CHARACTER SET ascii NOT NULL,"
      " st_threshold     BIGINT NOT NULL,"
      " st_dgram_bytes   BIGINT NOT NULL,"
      " st_data_bytes    BIGINT NOT NULL"
      ")" <<
      "CREATE TABLE INET_DNS_CACHE ("
      " st_ip            CHAR(8) CHARACTER SET ascii NOT NULL PRIMARY KEY,"
      " st_name          VARCHAR(" + utf8::int2Str(NI_MAXHOST + NI_MAXSERV + 1) + ") CHARACTER SET ascii NOT NULL"
      ")" <<
//      "CREATE INDEX IBS_IDX1 ON INET_BPFT_STAT (st_if,st_src_ip)" <<
//      "CREATE INDEX IBS_IDX2 ON INET_BPFT_STAT (st_if,st_dst_ip)" <<
      "CREATE INDEX IBS_IDX3 ON INET_BPFT_STAT (st_if,st_start,st_src_ip,st_dst_ip)" <<
      "CREATE INDEX IBS_IDX4 ON INET_BPFT_STAT (st_if,st_start,st_dst_ip,st_src_ip)" <<
      "CREATE INDEX IBSC_IDX1 ON INET_BPFT_STAT_CACHE (st_if,st_bt,st_et,st_filter_hash,st_threshold,st_src_ip,st_dst_ip)" <<
//      "CREATE INDEX IBSC_IDX2 ON INET_BPFT_STAT_CACHE (st_if,st_bt,st_et,st_filter_hash,st_threshold,st_dst_ip)" <<
      "CREATE UNIQUE INDEX IUT_IDX1 ON INET_USERS_TRAF (ST_USER,ST_TIMESTAMP)" <<
      "CREATE INDEX IUT_IDX4 ON INET_USERS_TRAF (ST_TIMESTAMP)" <<
      "CREATE INDEX IUT_IDX3 ON INET_USERS_TRAF (ST_TRAF_SMTP,ST_TIMESTAMP)"
    ;
    if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL ){
      metadata << "CREATE DESC INDEX IBS_IDX5 ON INET_BPFT_STAT (st_if,st_start)";
      metadata << "CREATE DESC INDEX IUT_IDX2 ON INET_USERS_TRAF (ST_TIMESTAMP)";
      metadata << "CREATE DESC INDEX IUMTU_IDX1 ON INET_USERS_MONTHLY_TOP_URL (ST_USER,ST_TIMESTAMP,ST_URL_HASH)";
    }
    else if( dynamic_cast<MYSQLDatabase *>(statement_->database()) != NULL ){
      metadata << "CREATE INDEX IUT_IDX2 ON INET_USERS_TRAF (ST_TIMESTAMP)";
      metadata << "CREATE INDEX IUMTU_IDX1 ON INET_USERS_MONTHLY_TOP_URL (ST_USER,ST_TIMESTAMP,ST_URL_HASH)";
    }
    if( (bool) config_->section("macroscope").value("DROP_DATABASE",false) ){
      database_->attach();
      database_->drop();
      database_->create();
    }

//#if !__FreeBSD__
    if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL &&
        (bool) config_->valueByPath("libadicpp.default_connection.firebird.set_properties",false) ){
      utf8::String hostName, dbName;
      uintptr_t port;
      database_->separateDBName(database_->name(),hostName,dbName,port);
      //  if( hostName.trim().strlen() > 0 ){
      utf8::String serviceName(hostName + (hostName.trim().strlen() > 0 ? ":" : "") + "service_mgr");
      fbcpp::Service service;
      service.params().
        add("user_name",config_->valueByPath("libadicpp.default_connection.firebird.user"));
      service.params().
        add("password",config_->valueByPath("libadicpp.default_connection.firebird.password"));
      try {
        service.attach(serviceName);
        service.request().
          add("action_svc_properties").add("dbname",dbName).
          add("prp_set_sql_dialect",config_->valueByPath("libadicpp.default_connection.firebird.dialect",3));
        service.invoke();
        service.request().clear().
          add("action_svc_properties").add("dbname",dbName).
          add("prp_reserve_space",
            (bool) config_->valueByPath("libadicpp.default_connection.firebird.reserve_space",false) ?
              isc_spb_prp_res : isc_spb_prp_res_use_full
          );
        service.invoke();
        service.request().clear().add("action_svc_properties").add("dbname",dbName).
          add("prp_page_buffers",
            config_->valueByPath("libadicpp.default_connection.firebird.page_buffers",2048u)
          );
        service.invoke();
        service.request().clear().
          add("action_svc_properties").add("dbname", dbName).
          add("prp_write_mode",
            (bool) config_->valueByPath("libadicpp.default_connection.firebird.async_write",0) ?
              isc_spb_prp_wm_async : isc_spb_prp_wm_sync
          );
        service.invoke();
        service.attach(serviceName);
        service.request().clear().
          add("action_svc_properties").add("dbname", dbName).
          add("prp_sweep_interval",
            config_->valueByPath("libadicpp.default_connection.firebird.sweep_interval",10000u)
          );
        service.invoke();
      }
      catch( ExceptionSP & e ){
        if( !e->searchCode(isc_network_error) ) throw;
      }
    }
//#endif
    database_->attach();
    for( uintptr_t i = 0; i < metadata.count(); i++ ){
      if( dynamic_cast<MYSQLDatabase *>(statement_->database()) != NULL )
        if( metadata[i].strncasecmp("CREATE TABLE",12) == 0 )
          metadata[i] += " TYPE = " + config_->textByPath("macroscope.mysql_table_type","INNODB");
      try {
        statement_->execute(metadata[i]);
      }
      catch( ExceptionSP & e ){
        //if( e->searchCode(isc_keytoobig) ) throw;
        if( !e->searchCode(isc_no_meta_update,isc_random,ER_TABLE_EXISTS_ERROR,
            ER_DUP_KEYNAME,ER_BAD_TABLE_ERROR,ER_DUP_ENTRY_WITH_KEY_NAME) ) throw;
      }
    }
    database_->detach();
  }
// parse log files
  int32_t err0 = doWork(0);
  int32_t err1 = waitThreads();
// optimize database (optional)
  if( !sniffer && !cgi_.isCGI() ){
    database_->attach();
    if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL ){
      statement_->text("SELECT * FROM RDB$INDICES")->execute()->fetchAll();
      for( intptr_t i = statement_->rowCount() - 1; i >= 0; i-- ){
        statement_->selectRow(i);
        if( statement_->valueAsString("RDB$RELATION_NAME").strncasecmp("RDB$",4) == 0 ) continue;
        utf8::String indexName(statement_->valueAsString("RDB$INDEX_NAME").trimRight());
        int64_t ellapsed;
        try {
          if( (bool) config_->section("macroscope").value("reactivate_indices",true) ){
            if( verbose_ ) fprintf(stderr,"Deactivate index %s",
              (const char *) indexName.getOEMString()
            );
            ellapsed = gettimeofday();
            statement2_->text("ALTER INDEX " + indexName + " INACTIVE")->execute();
            if( verbose_ ) fprintf(stderr," done, ellapsed time: %s\n",
              (const char *) utf8::elapsedTime2Str(gettimeofday() - ellapsed).getOEMString()
            );
            if( verbose_ ) fprintf(stderr,"Activate index %s",
              (const char *) indexName.getOEMString()
            );
            ellapsed = gettimeofday();
            statement2_->text("ALTER INDEX " + indexName + " ACTIVE")->execute();
            if( verbose_ ) fprintf(stderr," done, ellapsed time: %s\n",
              (const char *) utf8::elapsedTime2Str(gettimeofday() - ellapsed).getOEMString()
            );
          }
          if( (bool) config_->section("macroscope").value("set_indices_statistics",true) ){
            if( verbose_ ) fprintf(stderr,"Set statistics on index %s",
              (const char *) indexName.getOEMString()
            );
            ellapsed = gettimeofday();
            statement2_->text("SET STATISTICS INDEX " + indexName)->execute();
            if( verbose_ ) fprintf(stderr," done, ellapsed time: %s\n",
              (const char *) utf8::elapsedTime2Str(gettimeofday() - ellapsed).getOEMString()
            );
          }
        }
        catch( ExceptionSP & e ){
          if( !e->searchCode(isc_integ_fail,isc_integ_deactivate_primary,isc_lock_conflict,isc_update_conflict) ) throw;
          if( verbose_ ){
            if( e->searchCode(isc_integ_deactivate_primary) )
              fprintf(stderr," failed. Cannot deactivate index used by a PRIMARY/UNIQUE constraint.\n");
            else if( e->searchCode(isc_lock_conflict) )
              fprintf(stderr," failed. Lock conflict on no wait transaction.\n");
            else if( e->searchCode(isc_update_conflict) )
              fprintf(stderr," failed. Update conflicts with concurrent update.\n");
          }
        }
      }
    }
    else if( dynamic_cast<MYSQLDatabase *>(statement_->database()) != NULL ){
      if( (bool) config_->section("macroscope").value("reactivate_indices",true) ){
        Table<utf8::String> tables;
        utf8::String hostName, dbName;
        uintptr_t port;
        database_->separateDBName(database_->name(),hostName,dbName,port);
        statement_->text(
          "SELECT table_name,table_schema "
          "FROM INFORMATION_SCHEMA.TABLES "
          "WHERE upper(table_schema) = :schema"
          )->
          prepare()->
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
          ellapsed = gettimeofday();
	        statement_->text("ALTER TABLE " + tables(i,0) + " ENGINE=" + engine)->execute();
          if( verbose_ ) fprintf(stderr," done, ellapsed time: %s\n",
            (const char *) utf8::elapsedTime2Str(gettimeofday() - ellapsed).getOEMString()
          );
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
// generate reports
  int32_t err2 = doWork(1);
  if( sniffer && daemon ) Thread::waitForSignal();
  int32_t err3 = waitThreads();
  return err0 != 0 ? err0 : err1 != 0 ? err1 : err2 != 0 ? err2 : err3;
}
//------------------------------------------------------------------------------
int32_t Logger::doWork(uintptr_t stage)
{
  int32_t exitCode = 0;
  if( sniffer_ && !cgi_.isCGI() ){
    if( stage == 1 ){
      readConfig();
      ConfigSection dbParamsSection;
      dbParamsSection.addSection(config_->sectionByPath("libadicpp.default_connection"));
      for( uintptr_t i = 0; i < config_->sectionByPath("macroscope.bpft").sectionCount(); i++ ){
        utf8::String sectionName(config_->sectionByPath("macroscope.bpft").section(i).name());
        if( !(bool) config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.enabled",false) ) continue;
        AutoPtr<Database> database(Database::newDatabase(&dbParamsSection));
        AutoPtr<Sniffer> sniffer(newObjectV1<Sniffer>(database.ptr()));
        database.ptr(NULL);
        sniffer->ifName(sectionName);
        sniffer->iface(config_->textByPath("macroscope.bpft." + sectionName + ".sniffer.interface"));
        sniffer->tempFile(config_->textByPath("macroscope.bpft." + sectionName + ".sniffer.temp_file",sniffer->tempFile()));
        sniffer->filter(config_->textByPath("macroscope.bpft." + sectionName + ".sniffer.filter"));
        sniffer->swapThreshold(config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.swap_threshold",sniffer->swapThreshold()));
        sniffer->promisc(config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.promiscuous",sniffer->promisc()));
        sniffer->ports(config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.ports",sniffer->ports()));
        sniffer->protocols(config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.protocols",sniffer->protocols()));
        sniffer->swapLowWatermark(config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.swap_low_watermark",sniffer->swapLowWatermark()));
        sniffer->swapHighWatermark(config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.swap_high_watermark",sniffer->swapHighWatermark()));
        sniffer->swapWatchTime(config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.swap_watch_time",sniffer->swapWatchTime()));
        utf8::String groupingPeriod(config_->textByPath("macroscope.bpft." + sectionName + ".sniffer.grouping_period","none"));
        if( groupingPeriod.strcasecmp("None") == 0 ) sniffer->groupingPeriod(PCAP::pgpNone);
        else
        if( groupingPeriod.strcasecmp("Sec") == 0 ) sniffer->groupingPeriod(PCAP::pgpSec);
        else
        if( groupingPeriod.strcasecmp("Min") == 0 ) sniffer->groupingPeriod(PCAP::pgpMin);
        else
        if( groupingPeriod.strcasecmp("Hour") == 0 ) sniffer->groupingPeriod(PCAP::pgpHour);
        else
        if( groupingPeriod.strcasecmp("Day") == 0 ) sniffer->groupingPeriod(PCAP::pgpDay);
        else
        if( groupingPeriod.strcasecmp("Mon") == 0 ) sniffer->groupingPeriod(PCAP::pgpMon);
        else
        if( groupingPeriod.strcasecmp("Year") == 0 ) sniffer->groupingPeriod(PCAP::pgpYear);
        else
          sniffer->groupingPeriod(PCAP::pgpNone);
        threads_.safeAdd(sniffer.ptr(NULL));
        threads_[threads_.count() - 1].resume();
      }
    }
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
      if( cgi_.isCGI() && stage < 1 ) break;
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
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
SnifferService::SnifferService() : logger_(NULL)
{
#if defined(__WIN32__) || defined(__WIN64__)
  serviceType_ = SERVICE_WIN32_OWN_PROCESS;
  startType_ = SERVICE_AUTO_START;
  errorControl_ = SERVICE_ERROR_IGNORE;
  binaryPathName_ = getExecutableName();
  serviceStatus_.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  serviceStatus_.dwWaitHint = 600000; // give me 600 seconds for start or stop 
#endif
}
//------------------------------------------------------------------------------
void SnifferService::install()
{
  Config config;
  config.parse().override();
  serviceName_ = config.textByPath("macroscope.service_name","macroscope");
  displayName_ = config.textByPath("macroscope.service_display_name","Macroscope Packet Collection Service");
#if defined(__WIN32__) || defined(__WIN64__)
  utf8::String startType(config.textByPath("macroscope.service_start_type","auto"));
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
#if HAVE__MALLOC_OPTIONS
const char * _malloc_options = "HR";
#endif
//------------------------------------------------------------------------------
int main(int _argc,char * _argv[])
{
  int errcode = EINVAL;
  adicpp::AutoInitializer autoInitializer(_argc,_argv);
  autoInitializer = autoInitializer;
  utf8::String::Stream stream;
  try {
    uintptr_t i;
    stdErr.fileName(SYSLOG_DIR("macroscope/") + "macroscope.log");
    Config::defaultFileName(SYSCONF_DIR("") + "macroscope.conf");
    Services services(macroscope_version.gnu_);
    AutoPtr<macroscope::SnifferService> serviceAP(newObject<macroscope::SnifferService>());
    services.add(serviceAP);
    macroscope::SnifferService * service = serviceAP.ptr(NULL);
    bool dispatch = true, sniffer = false, daemonize = false, svc = false;
    for( i = 1; i < argv().count(); i++ ){
      if( argv()[i].strcmp("-c") == 0 && i + 1 < argv().count() ){
        Config::defaultFileName(argv()[i + 1]);
      }
      else if( argv()[i].strcmp("--log") == 0 && i + 1 < argv().count() ){
        stdErr.fileName(argv()[i + 1]);
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
      else if( argv()[i].strcmp("--service") == 0 ){
        svc = true;
      }
      else if( argv()[i].strcmp("--daemon") == 0 ){
        daemonize = true;
      }
      else if( argv()[i].strcmp("--install") == 0 ){
        for( uintptr_t j = i + 1; j < argv().count(); j++ )
          if( argv()[j].isSpace() )
            service->args(service->args() + " \"" + argv()[j] + "\"");
          else
            service->args(service->args() + " " + argv()[j]);
        services.install();
        dispatch = false;
      }
      else if( argv()[i].strcmp("--uninstall") == 0 ){
        services.uninstall();
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
//    static const uint8_t text[] = 
//      "0123456789ABCDEF"
//      "0123456789ABCDEF"
//    ;
//    uint8_t dText[sizeof(text)];
//    AutoPtr<uint8_t> cText;
//    LZWFilter lzw;
    
    //Vector<utf8::String> fileNamesVector;
    //getDirList(fileNamesVector,"c:/windows/system32/*.dll");
    //getDirList(fileNamesVector,"c:/windows/system32/*.exe");
    //getDirList(fileNamesVector,"G:/Library/Коллекция/*.txt");
    //Array<utf8::String> fileNames(fileNamesVector);
    //lzw.genStatisticTable(fileNames << getExecutableName());
    
//    lzw.initialize();
    //lzw.compress(text,sizeof(text),true);
    //cText.xchg(lzw.out());
    //uintptr_t outSize = lzw.outSize();
    //lzw.initialize();
    //lzw.decompress(cText,outSize,true);
    //dText[0] = dText[0];
#endif
    errcode = 0;
    if( dispatch || sniffer ){
      macroscope::Logger logger;
      if( dispatch && svc && sniffer ){
        service->logger_ = &logger;
        services.startServiceCtrlDispatcher();
      }
      else if( dispatch ){
#if HAVE_DAEMON
        if( daemonize && daemon(1,1) != 0 ){
          int32_t err = errno;
          newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__);
        }
#endif
        stdErr.debug(0,utf8::String::Stream() << macroscope_version.gnu_ << " started\n");
        errcode = logger.main(sniffer,daemonize);
        stdErr.debug(0,utf8::String::Stream() << macroscope_version.gnu_ << " stoped\n");
      }
    }
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    stdErr.debug(0,stream << macroscope_version.gnu_ << " terminated with error(s), see above.\n");
    errcode = e->code() >= errorOffset ? e->code() - errorOffset : e->code();
  }
  catch( ... ){
  }
  return errcode;
}
//------------------------------------------------------------------------------
