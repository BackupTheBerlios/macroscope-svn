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
void Logger::main()
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

  verbose_ = config_->section("macroscope").value("verbose",false);

// print query form if is CGI and no CGI parameters
  cgi_.initialize();
//  if( cgi_.isCGI() ){
//    if( cgi_.paramCount() == 0 ){
      cgi_ <<
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
        "<HTML>\n"
        "<HEAD>\n"
        "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf8\" />\n"
        "<meta http-equiv=\"Content-Language\" content=\"en\" />\n"
        "<TITLE>Statistics query form</TITLE>\n"
        "</HEAD>\n"
	"<BODY LANG=EN BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\">\n"
        "<FORM ACTION=\"/cgi-bin/" + getNameFromPathName(getExecutableName()) + "\"" + " METHOD=\"POST\" accept-charset=\"utf8\">\n"
        "  <label for=\"if\">Interface</label>\n"
        "  <select name=\"if\" id=\"if\">\n"
      ;
      for( uintptr_t i = 0; i < config_->sectionByPath("macroscope.bpft").sectionCount(); i++ ){
        utf8::String sectionName(config_->sectionByPath("macroscope.bpft").section(i).name());
        if( sectionName.strcasecmp("decoration") == 0 ) continue;
        cgi_ << "    <option value=\"" + sectionName + "\"";
        if( i == 0 ) cgi_ << " selected=\"selected\"";
        cgi_ << ">" + sectionName + "</option>\n";
      }
      cgi_ <<
        "  </select>\n"
	"  <BR>\n"
        "  <label for=\"byear\">Start time</label>\n"
	"  <select name=\"byear\" id=\"byear\">\n"
      ;
      struct tm curTime = time2tm(gettimeofday());
      for( intptr_t i = curTime.tm_year + 1900 - 25; i <= curTime.tm_year + 1900 + 25; i++ ){
        cgi_ << "    <option value=\"" + utf8::int2Str(i) + "\"";
        if( i == curTime.tm_year + 1900 ) cgi_ << " selected=\"selected\"";
        cgi_ << ">" + utf8::int2Str(i) + "</option>\n";
      }
      cgi_ <<
        "  </select>\n"
      ;
      cgi_ <<
        "  <BR>\n"
        "  <INPUT TYPE=\"SUBMIT\" VALUE=\"Start\">\n"
        "</FORM>\n"
	"</BODY>\n"
	"</HTML>\n"
      ;
      return;
//    }
    verbose_ = false;
//  }

  ConfigSection dbParamsSection;
  dbParamsSection.addSection(config_->sectionByPath("libadicpp.default_connection"));

  database_ = Database::newDatabase(&dbParamsSection);
  statement_ = database_->newAttachedStatement();

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
//    "DROP TABLE INET_BPFT_STAT" <<
//    "DROP TABLE INET_BPFT_STAT_IP" <<
//    "CREATE TABLE INET_BPFT_STAT_IP ("
//    " st_ip        CHAR(8) CHARACTER SET ascii NOT NULL UNIQUE PRIMARY KEY"
//    ")" <<
    "CREATE TABLE INET_BPFT_STAT ("
    " st_if            CHAR(8) CHARACTER SET ascii NOT NULL,"
    " st_start         DATETIME NOT NULL,"
//    " st_stop          DATETIME NOT NULL,"
    " st_src_ip        CHAR(8) CHARACTER SET ascii NOT NULL,"
    " st_dst_ip        CHAR(8) CHARACTER SET ascii NOT NULL,"
    " st_ip_proto      SMALLINT NOT NULL,"
    " st_src_port      INTEGER NOT NULL,"
    " st_dst_port      INTEGER NOT NULL,"
    " st_dgram_bytes   INTEGER NOT NULL,"
    " st_data_bytes    INTEGER NOT NULL"
//    " st_src_name      VARCHAR(" + utf8::int2Str(NI_MAXHOST + NI_MAXSERV + 1) + ") NOT NULL,"
//    " st_dst_name      VARCHAR(" + utf8::int2Str(NI_MAXHOST + NI_MAXSERV + 1) + ") NOT NULL"
//    " FOREIGN KEY (st_src_ip) REFERENCES INET_BPFT_STAT_IP(st_ip) ON DELETE CASCADE,"
//    " FOREIGN KEY (st_dst_ip) REFERENCES INET_BPFT_STAT_IP(st_ip) ON DELETE CASCADE"
    ")" <<
    "CREATE INDEX INET_BPFT_STAT_IDX1 ON INET_BPFT_STAT (st_if,st_src_ip)" <<
    "CREATE INDEX INET_BPFT_STAT_IDX2 ON INET_BPFT_STAT (st_if,st_dst_ip)" <<
    "CREATE INDEX INET_BPFT_STAT_IDX3 ON INET_BPFT_STAT (st_if,st_start,st_src_ip)" <<
    "CREATE INDEX INET_BPFT_STAT_IDX4 ON INET_BPFT_STAT (st_if,st_start,st_dst_ip)" <<
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
    "CREATE INDEX INET_BPFT_STAT_CACHE_IDX1 ON INET_BPFT_STAT_CACHE (st_if,st_bt,st_et,st_filter_hash,st_threshold,st_src_ip)" <<
    "CREATE INDEX INET_BPFT_STAT_CACHE_IDX2 ON INET_BPFT_STAT_CACHE (st_if,st_bt,st_et,st_filter_hash,st_threshold,st_dst_ip)" <<
    "CREATE TABLE INET_DNS_CACHE ("
    " st_ip            CHAR(8) CHARACTER SET ascii NOT NULL PRIMARY KEY,"
    " st_name          VARCHAR(" + utf8::int2Str(NI_MAXHOST + NI_MAXSERV + 1) + ") CHARACTER SET ascii NOT NULL"
    ")" <<
    "CREATE UNIQUE INDEX INET_USERS_TRAF_IDX1 ON INET_USERS_TRAF (ST_USER,ST_TIMESTAMP)" <<
    "CREATE INDEX INET_USERS_TRAF_IDX4 ON INET_USERS_TRAF (ST_TIMESTAMP)" <<
    "CREATE INDEX INET_USERS_TRAF_IDX3 ON INET_USERS_TRAF (ST_TRAF_SMTP,ST_TIMESTAMP)"
  ;
  if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL ){
    metadata << "CREATE DESC INDEX INET_USERS_TRAF_IDX2 ON INET_USERS_TRAF (ST_TIMESTAMP)";
    metadata << "CREATE DESC INDEX INET_USERS_MONTHLY_TOP_URL_IDX1 ON INET_USERS_MONTHLY_TOP_URL (ST_USER,ST_TIMESTAMP,ST_URL_HASH)";
  }
  else if( dynamic_cast<MYSQLDatabase *>(statement_->database()) != NULL ){
    metadata << "CREATE INDEX INET_USERS_TRAF_IDX2 ON INET_USERS_TRAF (ST_TIMESTAMP)";
    metadata << "CREATE INDEX INET_USERS_MONTHLY_TOP_URL_IDX1 ON INET_USERS_MONTHLY_TOP_URL (ST_USER,ST_TIMESTAMP,ST_URL_HASH)";
  }
  if( (bool) config_->section("macroscope").value("DROP_DATABASE",false) ){
    database_->attach();
    database_->drop();
    database_->create();
  }

//#if !__FreeBSD__
  if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL ){
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
      if( !e->searchCode(isc_no_meta_update,isc_random,ER_TABLE_EXISTS_ERROR,ER_DUP_KEYNAME,ER_BAD_TABLE_ERROR) ) throw;
    }
  }
  database_->detach();

  trafCacheSize_ = config_->valueByPath("macroscope.html_report.traffic_cache_size",0);
  threads_.safeAdd(newObjectR1C2C3<SquidSendmailThread>(*this,"macroscope","macroscope")).resume();

  dnsCacheHitCount_ = 0;
  dnsCacheMissCount_ = 0;
  dnsCacheSize_ = config_->valueByPath("macroscope.bpft.dns_cache_size",0);
  for( uintptr_t i = 0; i < config_->sectionByPath("macroscope.bpft").sectionCount(); i++ ){
    utf8::String sectionName(config_->sectionByPath("macroscope.bpft").section(i).name());
    if( sectionName.strcasecmp("decoration") == 0 ) continue;
    threads_.safeAdd(newObjectR1C2C3<BPFTThread>(*this,"macroscope.bpft." + sectionName,sectionName)).resume();
  }
  threads_.clear();
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
  int errcode = -1;
  adicpp::AutoInitializer autoInitializer(_argc,_argv);
  autoInitializer = autoInitializer;
//  fprintf(stderr,"%s\n",(const char *) utf8::time2Str(getlocaltimeofday()).getOEMString());
//  fprintf(stderr,"%s\n",(const char *) utf8::time2Str(gettimeofday()).getOEMString());
//  fprintf(stderr,"%s\n",(const char *) utf8::int2Str(getgmtoffset()).getOEMString());
  utf8::String::Stream stream;
  try {
    uintptr_t i;
    stdErr.fileName(SYSLOG_DIR("macroscope/") + "macroscope.log");
    Config::defaultFileName(SYSCONF_DIR("") + "macroscope.conf");
    bool dispatch = true;
    for( i = 1; i < argv().count(); i++ ){
      if( argv()[i].strcmp("--version") == 0 ){
        stdErr.debug(9,utf8::String::Stream() << macroscope_version.tex_ << "\n");
        fprintf(stdout,"%s\n",macroscope_version.tex_);
        dispatch = false;
        continue;
      }
      if( argv()[i].strcmp("-c") == 0 && i + 1 < argv().count() ){
        Config::defaultFileName(argv()[i + 1]);
      }
      else if( argv()[i].strcmp("--log") == 0 && i + 1 < argv().count() ){
        stdErr.fileName(argv()[i + 1]);
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
    if( dispatch ){
      macroscope::Logger logger;
      stdErr.debug(0,utf8::String::Stream() << macroscope_version.gnu_ << " started\n");
      logger.main();
      stdErr.debug(0,utf8::String::Stream() << macroscope_version.gnu_ << " stoped\n");
    }
    errcode = 0;
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
