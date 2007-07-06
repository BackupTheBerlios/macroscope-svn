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
#define ENABLE_PCAP_INTERFACE 1
#define ENABLE_ODBC_INTERFACE 1
#define ENABLE_MYSQL_INTERFACE 1
#define ENABLE_FIREBIRD_INTERFACE 1
#include <adicpp/adicpp.h>
//------------------------------------------------------------------------------
#include "sniffer.h"
#include "macroscope.h"
#ifndef NDEBUG
#include <adicpp/lzw.h>
#include <adicpp/lzssk.h>
#include <adicpp/stmflt.h>
#endif
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
  cgi_.initialize();
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
      if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL )
        metadata << "CREATE DOMAIN DATETIME AS TIMESTAMP";
      metadata <<
        "CREATE TABLE INET_USERS_TRAF ("
        " ST_USER               VARCHAR(80) CHARACTER SET ascii NOT NULL,"
        " ST_TIMESTAMP          DATETIME NOT NULL,"
        " ST_TRAF_WWW           BIGINT NOT NULL,"
        " ST_TRAF_SMTP          BIGINT NOT NULL"
        ")" <<
        "CREATE TABLE INET_USERS_MONTHLY_TOP_URL ("
        " ST_USER               VARCHAR(80) CHARACTER SET ascii NOT NULL,"
        " ST_TIMESTAMP          DATETIME NOT NULL,"
        " ST_URL                VARCHAR(4096) NOT NULL,"
        " ST_URL_HASH           BIGINT NOT NULL,"
        " ST_URL_TRAF           BIGINT NOT NULL,"
        " ST_URL_COUNT          BIGINT NOT NULL"
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
        "CREATE INDEX IBS_IDX3 ON INET_BPFT_STAT (st_if,st_start,st_src_ip,st_dst_ip,st_src_port,st_dst_port,st_ip_proto)" <<
        "CREATE INDEX IBS_IDX4 ON INET_BPFT_STAT (st_if,st_start,st_src_ip,st_src_port,st_dst_ip,st_dst_port,st_ip_proto)" <<
//        "CREATE INDEX IBSC_IDX1 ON INET_BPFT_STAT_CACHE (st_if,st_bt,st_et,st_filter_hash,st_threshold,st_src_ip,st_dst_ip)" <<
        "CREATE UNIQUE INDEX IUT_IDX1 ON INET_USERS_TRAF (ST_USER,ST_TIMESTAMP)" <<
        "CREATE INDEX IUT_IDX4 ON INET_USERS_TRAF (ST_TIMESTAMP)" <<
        "CREATE INDEX IUT_IDX3 ON INET_USERS_TRAF (ST_TRAF_SMTP,ST_TIMESTAMP)" <<
        "CREATE INDEX IUMTU_IDX1 ON INET_USERS_MONTHLY_TOP_URL (ST_USER,ST_TIMESTAMP,ST_URL_HASH)" <<
        "CREATE INDEX IUTM_IDX1 ON INET_USERS_TOP_MAIL (ST_USER,ST_TIMESTAMP,ST_FROM,ST_TO)"
      ;
      if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL ){
        metadata << "CREATE DESC INDEX IBS_IDX5 ON INET_BPFT_STAT (st_if,st_start)";
        metadata << "CREATE DESC INDEX IUT_IDX2 ON INET_USERS_TRAF (ST_TIMESTAMP)";
      }
      else if( dynamic_cast<MYSQLDatabase *>(statement_->database()) != NULL ){
        metadata << "CREATE INDEX IUT_IDX2 ON INET_USERS_TRAF (ST_TIMESTAMP)";
      }
      database_->create();
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
        service.request().
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
Logger & Logger::writeCGIInterfaceAndTimeSelect(bool addUnionIf)
{
  cgi_ <<
    "  <label for=\"if\">Interface</label>\n"
    "  <select name=\"if\" id=\"if\">\n"
  ;
  database_->attach()->start();
  statement_->text("select distinct st_if from INET_BPFT_STAT")->execute();
  while( statement_->fetch() ){
    utf8::String sectionName(statement_->valueAsString("st_if"));
    cgi_ << "    <option value=\"" + sectionName + "\"";
    if( statement_->rowIndex() == 0 && !addUnionIf ) cgi_ << " selected=\"selected\"";
    cgi_ << ">" + sectionName + "</option>\n";
  }
  if( addUnionIf )
    cgi_ <<
      "    <option value=\"All\" selected=\"selected\">All</option>\n"
    ;
  database_->commit()->detach();
  cgi_ <<
    "  </select>\n"
    "  <BR>\n"
    "  <label for=\"bday\">Begin time</label>\n"
    "  <select name=\"bday\" id=\"bday\">\n"
  ;
  struct tm curTime = time2tm(getlocaltimeofday());
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
  ;
  return *this;
}
//------------------------------------------------------------------------------
int32_t Logger::main()
{
  readConfig();
// print query form if is CGI and no CGI parameters
  /*setEnv("GATEWAY_INTERFACE","CGI/1.1");
  setEnv("REQUEST_METHOD","GET");
  setEnv("QUERY_STRING",
    "if=win_test&"
    "bday=01&bmon=5&byear=2007&"
    "eday=07&emon=5&eyear=2007&"
    "resolve=on&"
    "bidirectional=on&"
    "protocols=on&"
    "ports=on&"
    "threshold=4M&"
    "threshold2=&"
    "totals=Day&"
    "filter="
//    "filter=(src+amber+or+dst+amber)+and+(src_port+8010+or+dst_port+8010)+and+proto+tcp"
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
        "<FORM name=\"report_form\" ACTION=\"" + getEnv("SCRIPT_NAME") + "\"" + " METHOD=\"" + getEnv("USE_REQUEST_METHOD","GET") + "\" accept-charset=\"utf8\">\n"
        "  <B>Traffic report generation form</B>\n"
        "  <BR>\n"
      ;
      writeCGIInterfaceAndTimeSelect(false);
      cgi_ <<
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
        "  <P>Example: src (amber or dst amber) and (src_port 80 or dst_port www) and proto tcp</P>\n"
	      "  <textarea name=\"filter\" rows=\"4\" cols=\"80\"></textarea>\n"
      ;
      cgi_ <<
        "  <BR>\n"
        "  <BR>\n"
        "  <input name=\"report\" type=\"SUBMIT\" value=\"Start\">\n"
      ;
      if( cgi_.paramIndex("admin") < 0 ){
        cgi_ <<
//          "</FORM>\n"
//          "<FORM name=\"report_form\" ACTION=\"" + getEnv("SCRIPT_NAME") + "\"" + " METHOD=\"" + getEnv("USE_REQUEST_METHOD","GET") + "\" accept-charset=\"utf8\">\n"
          "  <input name=\"admin\" type=\"SUBMIT\" value=\"Admin\">\n"
          "</FORM>\n"
        ;
      }
      else {
        cgi_ <<
          "</FORM>\n"
          "<HR>\n"
          "<B>Administrative functions</B>\n"
          "<FORM name=\"admin_form\" ACTION=\"" + getEnv("SCRIPT_NAME") + "\"" + " METHOD=\"" + getEnv("USE_REQUEST_METHOD","GET") + "\" accept-charset=\"utf8\">\n"
        ;
        writeCGIInterfaceAndTimeSelect(true);
        cgi_ <<
          "  <BR>\n"
          "  <label for=\"newIfName\">New interface name</label>\n"
          "  <input type=\"text\" name=\"newIfName\" id=\"newIfName\">\n"
          "  <BR>\n"
          "  <input name=\"rolloutif\" type=\"SUBMIT\" value=\"Rollout\">\n"
          "  <input name=\"renameif\" type=\"SUBMIT\" value=\"Rename\">\n"
          "  <input name=\"copyif\" type=\"SUBMIT\" value=\"Copy\">\n"
          "  <input name=\"dropif\" type=\"SUBMIT\" value=\"Drop\">\n"
          "  <input name=\"dropdns\" type=\"SUBMIT\" value=\"Drop DNS cache\">\n"
          "</FORM>\n"
        ;
      }
      cgi_ <<
        "<HR>\n"
        "GMT: " + utf8::time2Str(gettimeofday()) + "\n<BR>\n"
        "Local time: " + utf8::time2Str(getlocaltimeofday()) + "\n<BR>\n"
        "Generated on " + getHostName() + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
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
  if( sniffer_ ) Thread::waitForSignal();
  int32_t err3 = waitThreads();
  return err0 != 0 ? err0 : err1 != 0 ? err1 : err2 != 0 ? err2 : err3;
}
//------------------------------------------------------------------------------
Sniffer * Logger::getSnifferBySection(const utf8::String & sectionName)
{
  ConfigSection dbParamsSection;
  utf8::String connection(config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.connection",connection_));
  dbParamsSection.addSection(config_->sectionByPath(connection));
  AutoPtr<Database> database(Database::newDatabase(&dbParamsSection));
  AutoPtr<Sniffer> sniffer(newObjectV1<Sniffer>(database.ptr()));
  database.ptr(NULL);
  sniffer->ifName(sectionName);
  sniffer->iface(config_->textByPath("macroscope.bpft." + sectionName + ".sniffer.interface"));
  sniffer->tempFile(config_->textByPath("macroscope.bpft." + sectionName + ".sniffer.temp_file",sniffer->tempFile()));
  sniffer->filter(config_->textByPath("macroscope.bpft." + sectionName + ".sniffer.filter"));
  sniffer->pcapReadTimeout(config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.pcap_read_timeout",sniffer->pcapReadTimeout()));
  sniffer->swapThreshold(config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.swap_threshold",sniffer->swapThreshold()));
  sniffer->pregroupingBufferSize(config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.pregrouping_buffer_size",sniffer->pregroupingBufferSize()));
  sniffer->pregroupingWindowSize(config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.pregrouping_window_size",sniffer->pregroupingWindowSize()));
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
  utf8::String joined(config_->textByPath("macroscope.bpft." + sectionName + ".sniffer.join"));
  for( intptr_t i = 0, j = enumStringParts(joined); i < j; i++ ){
    if( !(bool) config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.enabled",false) ) continue;
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
        if( !(bool) config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.enabled",false) ) continue;
        utf8::String join(config_->textByPath("macroscope.bpft." + sectionName + ".sniffer.join"));
        if( !join.isNull() ){
          if( !joined.isNull() ) joined += ",";
          joined += join;
          threads_.safeAdd(getSnifferBySection(sectionName));
          threads_[threads_.count() - 1].resume();
        }
      }
      for( uintptr_t i = 0; i < config_->sectionByPath("macroscope.bpft").sectionCount(); i++ ){
        utf8::String sectionName(config_->sectionByPath("macroscope.bpft").section(i).name());
        if( !(bool) config_->valueByPath("macroscope.bpft." + sectionName + ".sniffer.enabled",false) ) continue;
        utf8::String join(config_->textByPath("macroscope.bpft." + sectionName + ".sniffer.join"));
        if( join.isNull() && findStringPart(joined,sectionName,false) < 0 ){
          threads_.safeAdd(getSnifferBySection(sectionName));
          threads_[threads_.count() - 1].resume();
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
      cgi_.paramAsString("if").strcasecmp("all") == 0 ? utf8::String() : cgi_.paramAsString("if")
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
      "Generated on " + getHostName() + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
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
    statement_->
      text("update INET_BPFT_STAT set st_if = :newif" + utf8::String(all ? "" : " where st_if = :if"))->
      prepare()->
      paramAsString("newif",cgi_.paramAsString("newIfName"));
    if( !all ) statement_->paramAsString("if",cgi_.paramAsString("if"));
    statement_->execute();
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
      "Generated on " + getHostName() + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
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
    bool all = cgi_.paramAsString("if").strcasecmp("all") == 0;
    statement_->
      text("insert into INET_BPFT_STAT"
           "  select '" + cgi_.paramAsString("newIfName") + "' as st_if, st_start,"
           "    st_src_ip,st_dst_ip,st_ip_proto,"
           "    st_src_port,st_dst_port,"
           "    st_dgram_bytes,st_data_bytes"
           "  from INET_BPFT_STAT" + utf8::String(all ? "" : " where st_if = :if"))->
      prepare();
    if( !all ) statement_->paramAsString("if",cgi_.paramAsString("if"));
    statement_->execute();
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
      "Generated on " + getHostName() + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
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
    bool all = cgi_.paramAsString("if").strcasecmp("all") == 0;
    statement_->
      text("delete from INET_BPFT_STAT" + utf8::String(all ? "" : " where st_if = :if"))->
      prepare();
    if( !all ) statement_->paramAsString("if",cgi_.paramAsString("if"));
    statement_->execute();
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
      "Generated on " + getHostName() + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
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
    statement_->text("delete from INET_DNS_CACHE")->execute();
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
      "Generated on " + getHostName() + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
#ifndef PRIVATE_RELEASE
      "<A HREF=\"http://developer.berlios.de/projects/macroscope/\">\n"
      "  http://developer.berlios.de/projects/macroscope/\n"
      "</A>\n"
#endif
      "</BODY>\n"
      "</HTML>\n"
    ;
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
Logger & Logger::rolloutBPFTByIPs(const utf8::String & bt,const utf8::String & et,const utf8::String & ifName)
{
  Mutant btt(0), ett(0);
  if( !bt.isNull() ) btt = utf8::str2Time(bt) - getgmtoffset();
  if( !et.isNull() ) ett = utf8::str2Time(et) - getgmtoffset();
  btt.changeType(mtTime);
  ett.changeType(mtTime);
  readConfig();
  AutoDatabaseDetach autoDatabaseDetach(database_);
  database_->start();
  utf8::String range(" WHERE");
  if( !ifName.isNull() ) range += " st_if = :if";
  if( (uint64_t) btt != 0 ){
    if( !range.isNull() ) range += " AND";
    range += " st_start >= :bt";
  }
  if( (uint64_t) ett != 0 ){
    if( !range.isNull() ) range += " AND";
    range += " st_start <= :et";
  }
  if( !range.isNull() ) range += " AND";
  range += " (st_src_port != 0 OR st_dst_port != 0 OR st_ip_proto != -1)";
  statement_->text(
    "INSERT INTO INET_BPFT_STAT "
    "SELECT st_if, st_start, st_src_ip, st_dst_ip,"
    " -1 as st_ip_proto, 0 as st_src_port, 0 as st_dst_port,"
    " sum(st_dgram_bytes) as st_dgram_bytes,"
    " sum(st_data_bytes) as st_data_bytes"
    " FROM INET_BPFT_STAT" +
    range +
    " GROUP by st_if, st_start, st_src_ip, st_dst_ip"
  )->prepare();
  if( !ifName.isNull() ) statement_->paramAsString("if",ifName);
  if( (uint64_t) btt != 0 ) statement_->paramAsMutant("bt",btt);
  if( (uint64_t) ett != 0 ) statement_->paramAsMutant("et",ett);
  statement_->execute();
/*
    statement_->fetchAll();
    statement2_->text(
      "INSERT INTO INET_BPFT_STAT ("
      "  st_if,st_start,st_src_ip,st_dst_ip,st_ip_proto,st_src_port,st_dst_port,st_dgram_bytes,st_data_bytes"
      ") VALUES ("
      "  :st_if,:st_start,:st_src_ip,:st_dst_ip,:st_ip_proto,:st_src_port,:st_dst_port,:st_dgram_bytes,:st_data_bytes"
      ")"
    )->prepare();
    for( intptr_t row = statement_->rowCount() - 1; row >= 0; row-- ){
      statement_->selectRow(row);
      for( intptr_t field = statement_->fieldCount() - 1; field >= 0; field-- )
          statement2_->paramAsMutant(statement_->fieldName(field),statement_->valueAsMutant(field));
      statement2_->execute();
    }
*/
  statement_->text("DELETE FROM INET_BPFT_STAT" + range)->prepare();
  if( !ifName.isNull() ) statement_->paramAsString("if",ifName);
  if( (uint64_t) btt != 0 ) statement_->paramAsMutant("bt",btt);
  if( (uint64_t) ett != 0 ) statement_->paramAsMutant("et",ett);
  statement_->execute();
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
  serviceStatus_.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  serviceStatus_.dwWaitHint = 600000; // give me 600 seconds for start or stop 
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
#if HAVE__MALLOC_OPTIONS
const char * _malloc_options = "HR";
#endif
//------------------------------------------------------------------------------
int main(int _argc,char * _argv[])
{
//  Sleep(15000);
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
    bool dispatch = true, sniffer = false, svc = false, rollout = false;
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
    static const uint8_t text[] = 
      "0123456789ABCDEF"
      "0123456789ABCDEF"
      "FEDCBA9876543210"
      "FEDCBA9876543210"
    ;
    uint8_t buf[1024];
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
    LZMAFileFilter fileFilter;
    for( uintptr_t ii = 0; i < 1000; i++ ){
      fileFilter.compressFile("G:/Library/Коллекция/V_ANECD/V_ANECD.TXT",getExecutablePath() + "V_ANECD.TXT.compressed");
      fileFilter.decompressFile(getExecutablePath() + "V_ANECD.TXT.compressed",getExecutablePath() + "V_ANECD.TXT.decompressed");
    }
#endif
    errcode = 0;
    if( dispatch || sniffer || rollout ){
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
        stdErr.debug(0,utf8::String::Stream() << macroscope_version.gnu_ << " started\n");
        errcode = logger.main();
        stdErr.debug(0,utf8::String::Stream() << macroscope_version.gnu_ << " stoped\n");
      }
      else if( rollout ){
        logger.rolloutBPFTByIPs(
          stringPartByNo(rolloutParams,0).trim(),
          stringPartByNo(rolloutParams,1).trim(),
          stringPartByNo(rolloutParams,2)
        );
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
        "Generated on " + getHostName() + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
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
