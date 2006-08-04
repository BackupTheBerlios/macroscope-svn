/*-
 * Copyright 2006 Guram Dukashvili
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
//------------------------------------------------------------------------------
namespace macroscope {
//------------------------------------------------------------------------------
Logger::~Logger()
{
}
//------------------------------------------------------------------------------
Logger::Logger() : shortUrl_("://"), ellapsed_(getlocaltimeofday())
{
  trafCache_.caseSensitive(false);
}
//------------------------------------------------------------------------------
ksys::Mutant Logger::timeStampRoundToMin(const ksys::Mutant & timeStamp)
{
  struct tm t = timeStamp;
  t.tm_sec = 0;
  return t;
}
//------------------------------------------------------------------------------
void Logger::printStat(int64_t lineNo, int64_t spos, int64_t pos, int64_t size, int64_t cl)
{
  if( verbose_ ){
    int64_t ct  = getlocaltimeofday() - cl;
    int64_t q   = (ct * (size - pos)) / (pos - spos <= 0 ? 1 : pos - spos);
    size -= spos;
    if( size <= 0 )
      size = 1;
    pos -= spos;
    if( pos <= 0 )
      pos = 1;
    if( ct <= 0 )
      ct = 1;
    int64_t a = pos * 100 / size, b = lineNo * 1000000 / ct;
#ifdef HAVE__ISATTY
#ifdef HAVE__FILENO
    if( _isatty(_fileno(stderr)) )
      fprintf(stderr, "\r");
#else
    if( _isatty(fileno(stderr)) )
      fprintf(stderr, "\r");
#endif
#elif HAVE_ISATTY
#ifdef HAVE__FILENO
    if( isatty(_fileno(stderr)) )
      fprintf(stderr, "\r");
#else
    if( isatty(fileno(stderr)) )
      fprintf(stderr, "\r");
#endif
#endif
    fprintf(stderr, "%3"PRId64".%04"PRId64"%%, %7"PRId64".%04"PRId64" lps, ", a, pos * 1000000 / size - a * 10000, b, lineNo * INT64_C(10000000000) / ct - b * 10000);
    fprintf(stderr, "%s, elapsed: %-20s", (const char *) utf8::elapsedTime2Str(q).getANSIString(), (const char *) utf8::elapsedTime2Str(ct).getANSIString());
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
            fprintf(stderr, "\n");
    //    printf("%-20s\n",utf8::elapsedTime2Str(size * ct / pos - ct).c_str());
  }
}
//------------------------------------------------------------------------------
void Logger::parseSquidLogLine(char * p, uintptr_t size,ksys::Array<const char *> & slcp)
{
  char *  rb  = p + size, * a, * s;
  for( a = p; *a != '\r' && *a != '\n' && a < rb; a++ );
  slcp = NULL;
  for( uintptr_t i = 0; p < a; i++ ){
    while( p < a && *p == ' ' ) p++;
    s = p;
    while( s < a ){
      if( *s == '[' ){
        while( s < a && *s != ']' ) s++;
      }
      else if( *s == '(' ){
        while( s < a && *s != ')' ){
          if( *s == '\'' ){
            for( ++s; s < a && *s != '\''; s++ );
            if( *s == '\'' ) s++;
          }
          else {
            s++;
          }
        }
      }
      if( s < a ){
        if( *s == ' ' ) break;
        s++;
      }
    }
    if( i >= slcp.count() ) slcp.resize(i + 1);
    slcp[i] = p;
    *s++ = '\0';
    if( i > 0 ) if( strcmp(slcp[i - 1], slcp[i]) == 0 ) i--;
    p = s;
  }
}
//------------------------------------------------------------------------------
utf8::String Logger::shortUrl(const utf8::String & url)
{
  utf8::String s(url);
  utf8::String::Iterator j(url);
  j.last().prev();
  if( j.getChar() == '?' ){
    utf8::String::Iterator i(url.strstr(shortUrl_));
    if( i.position() >= 0 ){
      i += 3;
      while( i.next() && i.getChar() != '/' );
      s = utf8::String(utf8::String::Iterator(s),i);
    }
  }
  return s;
}
//------------------------------------------------------------------------------
int64_t Logger::fetchLogFileLastOffset(const utf8::String & logFileName)
{
  statement_->text("SELECT ST_LAST_OFFSET FROM INET_LOG_FILE_STAT WHERE ST_LOG_FILE_NAME = :ST_LOG_FILE_NAME");
  statement_->prepare();
  statement_->paramAsString("ST_LOG_FILE_NAME", logFileName);
  statement_->execute();
  int64_t offset  = 0;
  if( statement_->fetch() ){
    statement_->fetchAll();
    offset = statement_->valueAsMutant("ST_LAST_OFFSET");
  }
  return offset;
}
//------------------------------------------------------------------------------
Logger & Logger::updateLogFileLastOffset(const utf8::String & logFileName, int64_t offset)
{
  stFileStatSel_->prepare();
  stFileStatSel_->paramAsString("ST_LOG_FILE_NAME", logFileName);
  stFileStatSel_->execute();
  if( stFileStatSel_->fetch() ){
    stFileStatSel_->fetchAll();
    stFileStatUpd_->prepare();
    stFileStatUpd_->paramAsString("ST_LOG_FILE_NAME", logFileName);
    stFileStatUpd_->paramAsMutant("ST_LAST_OFFSET", offset);
    stFileStatUpd_->execute();
  }
  else{
    stFileStatIns_->prepare();
    stFileStatIns_->paramAsString("ST_LOG_FILE_NAME", logFileName);
    stFileStatIns_->paramAsMutant("ST_LAST_OFFSET", offset);
    stFileStatIns_->execute();
  }
  return *this;
}
//------------------------------------------------------------------------------
void Logger::parseSquidLogFile(const utf8::String & logFileName, bool top10, const utf8::String & skipUrl)
{
  ksys::AsyncFile flog(logFileName);
  flog.open();
  stTrafIns_->text(
    "INSERT INTO INET_USERS_TRAF"
    "(ST_USER, ST_TIMESTAMP, ST_TRAF_WWW, ST_TRAF_SMTP)"
    "VALUES (:ST_USER, :ST_TIMESTAMP, :ST_TRAF_WWW, 0)"
  );
  stTrafUpd_->text(
    "UPDATE INET_USERS_TRAF SET ST_TRAF_WWW = ST_TRAF_WWW + :ST_TRAF_WWW "
    "WHERE ST_USER = :ST_USER AND ST_TIMESTAMP = :ST_TIMESTAMP"
  );

  database_->start();
  int64_t offset = fetchLogFileLastOffset(logFileName);
  flog.seek(offset);

//  ksys::AutoPtr<char> b;
  int64_t lineNo = 1;
  uintptr_t size;
  ksys::Array<const char *> slcp;
  int64_t cl = getlocaltimeofday();
  ksys::AsyncFile::LineGetBuffer lgb;
  for(;;){
//    if( (size = flog.gets(b)) <= 0 ) break;
//    if( b[size - 1] == '\n' ){
    utf8::String sb;
    if( flog.gets(sb,&lgb) ) break;
    size = sb.size();
    if( size > 0 && sb.c_str()[size - 1] == '\n' ){
      parseSquidLogLine(sb.c_str(),size,slcp);
      intmax_t  traf  (utf8::str2Int(slcp[4]));
      if( traf > 0 && slcp[3] != NULL && slcp[7] != NULL && strchr(slcp[7], '%') == NULL && strcmp(slcp[7], "-") != 0 && strncmp(slcp[3], "NONE", 4) != 0 && strncmp(slcp[3], "TCP_DENIED", 10) != 0 && strncmp(slcp[3], "UDP_DENIED", 10) != 0 ){
        double timeStamp1;
        sscanf(slcp[0],"%lf",&timeStamp1);
        utf8::String  st_user (slcp[7]), st_url (slcp[6]);
        st_user = st_user.lower();
        if( st_url.strcasestr(skipUrl).position() < 0 ){
          st_url = shortUrl(st_url).lower();
          ksys::Mutant timeStamp(timeStampRoundToMin(timeStamp1 * 1000000));
          try{
            stTrafIns_->prepare();
            stTrafIns_->paramAsString("ST_USER",st_user);
            stTrafIns_->paramAsMutant("ST_TIMESTAMP",timeStamp);
            stTrafIns_->paramAsMutant("ST_TRAF_WWW",traf);
            stTrafIns_->execute();
          }
          catch( ksys::ExceptionSP & e ){
            if( !e->searchCode(isc_no_dup, ER_DUP_ENTRY) ) throw;
            stTrafUpd_->prepare();
            stTrafUpd_->paramAsString("ST_USER", st_user);
            stTrafUpd_->paramAsMutant("ST_TIMESTAMP", timeStamp);
            stTrafUpd_->paramAsMutant("ST_TRAF_WWW", traf);
            stTrafUpd_->execute();
          }
          if( top10 ){
            //int64_t urlHash = st_url.hash_ll(true);

            stMonUrlSel_->prepare()->
              paramAsString("ST_USER", st_user)->
              paramAsMutant("ST_TIMESTAMP", timeStamp)->
              paramAsMutant("ST_URL", st_url)->
            //paramAsMutant(utf8::string("ST_URL_HASH"),urlHash)->
              execute()->fetchAll();
            if( stMonUrlSel_->rowCount() > 0 ){
              stMonUrlUpd_->prepare()->
                paramAsString("ST_USER", st_user)->
                paramAsMutant("ST_TIMESTAMP", timeStamp)->
                paramAsMutant("ST_URL", st_url)->
                //paramAsMutant(utf8::string("ST_URL_HASH"),urlHash)->
                paramAsMutant("ST_URL_TRAF", traf)->execute();
            }
            else {
              stMonUrlIns_->prepare()->
                paramAsString("ST_USER", st_user)->
                paramAsMutant("ST_TIMESTAMP", timeStamp)->
                paramAsMutant("ST_URL", st_url)->
                //paramAsMutant(utf8::string("ST_URL_HASH"),urlHash)->
                paramAsMutant("ST_URL_TRAF", traf)->execute();
            }
          }
        }
      }
    }
    if( lineNo % 8192 == 0 ){
      updateLogFileLastOffset(logFileName, flog.tell());
      database_->commit();
      database_->start();
    }
    if( lineNo % 1024 == 0 ){
      printStat(lineNo, offset, flog.tell(), flog.size(), cl);
    }
    lineNo++;
  }
  printStat(lineNo, offset, flog.tell(), flog.size(), cl);
  updateLogFileLastOffset(logFileName, flog.tell());
  database_->commit();
}
//------------------------------------------------------------------------------
void Logger::parseSendmailLogFile(const utf8::String & logFileName, const utf8::String & domain, uintptr_t startYear)
{
  ksys::AsyncFile flog(logFileName);
  flog.open();
  stTrafIns_->text("INSERT INTO INET_USERS_TRAF" "(ST_USER, ST_TIMESTAMP, ST_TRAF_WWW, ST_TRAF_SMTP)" "VALUES (:ST_USER, :ST_TIMESTAMP, 0, :ST_TRAF_SMTP)");
  stTrafUpd_->text("UPDATE INET_USERS_TRAF SET ST_TRAF_SMTP = ST_TRAF_SMTP + :ST_TRAF_SMTP " "WHERE ST_USER = :ST_USER AND ST_TIMESTAMP = :ST_TIMESTAMP");

//  ksys::AutoPtr< char>  b;
  database_->start();
  int64_t offset  = fetchLogFileLastOffset(logFileName);
  flog.seek(offset);
  int64_t   lineNo  = 1;
  uintptr_t size;
  intptr_t  mon     = 0;
  int64_t   cl      = getlocaltimeofday();
  ksys::AsyncFile::LineGetBuffer lgb;
  for( ; ; ){
//    if( (size = flog.gets(b)) <= 0 ) break;
//    if( b[size - 1] == '\n' ){
    utf8::String sb;
    if( flog.gets(sb,&lgb) ) break;
    size = sb.size();
    if( size > 0 && sb.c_str()[size - 1] == '\n' ){
      char * a, * id, * idl, * prefix, * prefixl, * from, * to, * stat; //* relay;
      from = strstr(sb.c_str(), "from=");
      to = strstr(sb.c_str(), " to=");
      if( ((from != NULL || to != NULL)) && (id = strstr(sb.c_str(), "sm-mta[")) != NULL ){
        // get time
        tm  lt;
        memset(&lt, 0, sizeof(lt));
        lt.tm_mon = (int) ksys::str2Month(sb.c_str());
        if( lt.tm_mon < mon ) startYear++;
        mon = lt.tm_mon;
        lt.tm_year = int(startYear - 1900);
        sscanf(sb.c_str() + 4, "%u %u:%u:%u", &lt.tm_mday, &lt.tm_hour, &lt.tm_min, &lt.tm_sec);
        // get msgid
        prefix = id;
        for( prefixl = prefix; *prefixl != '['; prefixl++ )
          ;
        while( *id != ']' )
          id++;
        while( *++id == ':' );
        while( *++id == ' ' );
        for( idl = id; *idl != ':'; idl++ );
        utf8::String  st_user;
        // get from
        if( from != NULL ){
          if( from[5] == '<' ){
            from += 6;
            a = from;
            while( *a != '@' && *a != '>' && *a != ',' ){
              if( *a == ':' ) from = a + 1;
              a++;
            }
            if( *a == '@' && utf8::String(a,domain.strlen()).strcasecmp(domain) == 0 ){
              st_user = utf8::String(from, a - from).lower();
            }
          }
        }
        // get to
        if( to != NULL ){
          if( to[4] == '<' ){
            to += 5;
            a = to;
            while( *a != '@' && *a != '>' && *a != ',' ){
              if( *a == ':' ) to = a + 1;
              a++;
            }
            if( *a == '@' && utf8::String(a, domain.strlen()).strcasecmp(domain) == 0 ){
              st_user = utf8::String(to, a - to).lower();
            }
          }
        }
        // get size
        uintptr_t msgSize = 0;
        if( (a = strstr(sb.c_str(), "size=")) != NULL ){
          sscanf(a + 5, "%"PRIuPTR, &msgSize);
        }
        // get nrcpts
        uintptr_t nrcpts  = 0;
        if( (a = strstr(sb.c_str(), "nrcpts=")) != NULL ){
          sscanf(a + 7, "%"PRIuPTR, &nrcpts);
        }
        // get stat
        if( (stat = strstr(sb.c_str(), "stat=")) != NULL ){
          stat += 5;
        }
        if( from != NULL && msgSize > 0 ){
          try{
            stMsgsIns_->prepare()->
            paramAsString("ST_FROM", st_user)->
            paramAsString("ST_MSGID", utf8::String(id, idl - id))->
            paramAsMutant("ST_MSGSIZE", msgSize)->execute();
          }
          catch( ksys::ExceptionSP & e ){
            if( !e->searchCode(isc_no_dup, ER_DUP_ENTRY) )
              throw;
          }
        }
        else if( to != NULL && stat != NULL && strncmp(stat, "Sent", 4) == 0 ){
          stMsgsSel_->prepare()->
          paramAsString("ST_MSGID", utf8::String(id, idl - id))->
          execute()->fetchAll();
          if( stMsgsSel_->rowCount() > 0 ){
            if( st_user.strlen() == 0 )
              st_user = stMsgsSel_->valueAsString("ST_FROM");
            if( st_user.strlen() > 0 ){
              msgSize = stMsgsSel_->valueAsMutant("ST_MSGSIZE");
              try{
                stTrafIns_->prepare()->
                paramAsString("ST_USER", st_user)->
                paramAsMutant("ST_TIMESTAMP", timeStampRoundToMin(lt))->
                paramAsMutant("ST_TRAF_SMTP", msgSize)->execute();
              }
              catch( ksys::ExceptionSP & e ){
                if( !e->searchCode(isc_no_dup, ER_DUP_ENTRY) )
                  throw;
                stTrafUpd_->prepare()->
                paramAsString("ST_USER", st_user)->
                paramAsMutant("ST_TIMESTAMP", timeStampRoundToMin(lt))->
                paramAsMutant("ST_TRAF_SMTP", msgSize)->execute();
              }
              stMsgsDel2_->prepare()->
              paramAsString("ST_MSGID", stMsgsSel_->paramAsString("ST_MSGID"))->execute();
            }
          }
        }
      }
    }
    if( lineNo % 65536 == 0 ){
      updateLogFileLastOffset(logFileName, flog.tell());
      database_->commit();
      database_->start();
    }
    if( lineNo % 4096 == 0 ){
      printStat(lineNo, offset, flog.tell(), flog.size(), cl);
    }
    lineNo++;
  }
  printStat(lineNo, offset, flog.tell(), flog.size(), cl);
  updateLogFileLastOffset(logFileName, flog.tell());
  stMsgsDel_->execute();
  stMsgsSelCount_->execute()->fetchAll();
  database_->commit();
}
//------------------------------------------------------------------------------
/*
struct t_entry {
  struct in_addr in_ip, out_ip;  // src ip addr and dst ip addr
  u_char  ip_protocol;    // which protocol been used (/etc/protocols)
  u_short o_port;         // source port
  u_short p_port;         // destination port
  u_long  n_psize;        // how many bytes in ip datagrams passed
  u_long  n_bytes;        // how many data bytes passed
};

struct t_header {
  int    t_size;
  struct timeval start;
  struct timeval stop;
};

void Logger::parseBPFTLogFile(const utf8::String & logFileName)
{
  ksys::FileHandleContainer flog(logFileName);
  flog.open();
}
*/
//------------------------------------------------------------------------------
void Logger::main()
{
  config_.parse().override();
  ksys::stdErr.setDebugLevels(config_.value("debug_levels","+0,+1,+2,+3"));

/*  utf8::String lockFileName(ksys::getTempPath() +
    ksys::unScreenString(
      config_.section("macroscope").text(
        "lock_file_name", "F2BC263F-C902-4398-AF11-5173F2B6B4F4")
      )
  );

  ksys::FileHandleContainer lf(lockFileName);
  lf.exclusive(true);
  lf.removeAfterClose(true);
  if( !lf.tryOpen(true) ){
    int32_t err = errno;
    fprintf(stderr, "lower frequency of program starting needed\n");
    ksys::Exception::throwSP(err,"lower frequency of program starting needed");
  }
  lf << utf8::int2Str(ksys::getpid());*/

  verbose_ = config_.section("macroscope").value("verbose", false);

  database_.ptr(Database::newDatabase(&config_));
  statement_.ptr(database_->newAttachedStatement());
  stTrafIns_.ptr(database_->newAttachedStatement());
  stTrafUpd_.ptr(database_->newAttachedStatement());
  stMonUrlSel_.ptr(database_->newAttachedStatement());
  stMonUrlIns_.ptr(database_->newAttachedStatement());
  stMonUrlUpd_.ptr(database_->newAttachedStatement());
  stFileStatSel_.ptr(database_->newAttachedStatement());
  stFileStatIns_.ptr(database_->newAttachedStatement());
  stFileStatUpd_.ptr(database_->newAttachedStatement());
  stMsgsIns_.ptr(database_->newAttachedStatement());
  stMsgsSel_.ptr(database_->newAttachedStatement());
  stMsgsDel_.ptr(database_->newAttachedStatement());
  stMsgsDel2_.ptr(database_->newAttachedStatement());
  stMsgsSelCount_.ptr(database_->newAttachedStatement());

  stMonUrlSel_->text(
    "SELECT ST_URL FROM INET_USERS_MONTHLY_TOP_URL " 
    "WHERE ST_USER = :ST_USER AND ST_TIMESTAMP = :ST_TIMESTAMP AND " 
    "ST_URL = :ST_URL"
  );
  stMonUrlIns_->text(
    "INSERT INTO INET_USERS_MONTHLY_TOP_URL " 
    "(ST_USER, ST_TIMESTAMP, ST_URL, ST_URL_TRAF, ST_URL_COUNT) VALUES" 
    "(:ST_USER, :ST_TIMESTAMP, :ST_URL, :ST_URL_TRAF, 1)"
  );
  stMonUrlUpd_->text(
    "UPDATE INET_USERS_MONTHLY_TOP_URL "
    "SET "
    "ST_URL_TRAF = :ST_URL_TRAF + :ST_URL_TRAF," 
    "ST_URL_COUNT = ST_URL_COUNT + 1 "
    "WHERE ST_USER = :ST_USER AND ST_TIMESTAMP = :ST_TIMESTAMP AND " 
    "ST_URL = :ST_URL"
  );
  stFileStatSel_->text(
    "SELECT ST_LAST_OFFSET FROM INET_LOG_FILE_STAT " 
    "WHERE ST_LOG_FILE_NAME = :ST_LOG_FILE_NAME"
  );
  stFileStatIns_->text(
    "INSERT INTO INET_LOG_FILE_STAT" 
    "(ST_LOG_FILE_NAME, ST_LAST_OFFSET)" 
    "VALUES" 
    "(:ST_LOG_FILE_NAME,:ST_LAST_OFFSET)"
  );
  stFileStatUpd_->text(
    "UPDATE INET_LOG_FILE_STAT SET ST_LAST_OFFSET = :ST_LAST_OFFSET " 
    "WHERE ST_LOG_FILE_NAME = :ST_LOG_FILE_NAME"
  );
  stMsgsIns_->text(
    "INSERT INTO INET_SENDMAIL_MESSAGES (ST_FROM,ST_MSGID,ST_MSGSIZE) " 
    "VALUES (:ST_FROM,:ST_MSGID,:ST_MSGSIZE);"
  );
  stMsgsSel_->text(
    "SELECT * FROM INET_SENDMAIL_MESSAGES WHERE ST_MSGID = :ST_MSGID"
  );
  stMsgsDel_->text("DELETE FROM INET_SENDMAIL_MESSAGES");
  stMsgsDel2_->text("DELETE FROM INET_SENDMAIL_MESSAGES WHERE ST_MSGID = :ST_MSGID");
  stMsgsSelCount_->text("SELECT COUNT(*) FROM INET_SENDMAIL_MESSAGES");

  database_->create();

//TODO: must be rewriten, not worked
  /*if( dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL ){
    const ksys::ConfigSection & section = config_.section("libadicpp").
      section("default_connection").section("firebird");
    utf8::String hostName, dbName;
    uintptr_t port;
    database_->separateDBName(database_->name(),hostName,dbName,port);
    //  if( hostName.trim().strlen() > 0 ){
    utf8::String serviceName(hostName + (hostName.trim().strlen() > 0 ? ":" : "") + "service_mgr");
    fbcpp::Service service;
    service.params().
      add("user_name", section.value("user")).
      add("password", section.value("password"));
    service.attach(serviceName);
    service.request().clear().
      add("action_svc_properties").add("dbname",dbName).
      add("prp_set_sql_dialect",3);
    service.invoke().detach();
    service.attach(serviceName);
    service.request().clear().
      add("action_svc_properties").add("dbname",dbName).
      add("prp_reserve_space",isc_spb_prp_res_use_full);
    service.invoke().detach();
//    service.request().clear().add("action_svc_properties").add("dbname","macroscope").
//      add("prp_page_buffers",8 * 1024 * 1024 / database.params().pageSize());
//    service.invoke();
    service.attach(serviceName);
    service.request().clear().
      add("action_svc_properties").add("dbname", dbName).
      add("prp_write_mode", isc_spb_prp_wm_async);
    service.invoke().detach();
    service.attach(serviceName);
    service.request().clear().
      add("action_svc_properties").add("dbname", dbName).
      add("prp_sweep_interval", 10000);
    service.invoke().detach();
    //  }
  }*/
  ksys::Vector< utf8::String> metadata;
  if( dynamic_cast< FirebirdDatabase *>(database_.ptr()) != NULL )
    metadata << "CREATE DOMAIN DATETIME AS TIMESTAMP";
    metadata <<
      "CREATE TABLE INET_USERS_TRAF ("
      " ST_USER               VARCHAR(80) NOT NULL,"
      " ST_TIMESTAMP          DATETIME NOT NULL,"
      " ST_TRAF_WWW           INTEGER NOT NULL,"
      " ST_TRAF_SMTP          INTEGER NOT NULL" ")" <<
      "CREATE TABLE INET_USERS_MONTHLY_TOP_URL ("
      " ST_USER               VARCHAR(80) NOT NULL,"
      " ST_TIMESTAMP          DATETIME NOT NULL,"
      " ST_URL                VARCHAR(1000) NOT NULL,"
//      " ST_URL_HASH           BIGINT NOT NULL,"
      " ST_URL_TRAF           INTEGER NOT NULL,"
      " ST_URL_COUNT          INTEGER NOT NULL"
      ")" << 
      "CREATE TABLE INET_SENDMAIL_MESSAGES ("
      " ST_FROM               VARCHAR(240) NOT NULL,"
      " ST_MSGID              VARCHAR(14) NOT NULL PRIMARY KEY,"
      " ST_MSGSIZE            INTEGER NOT NULL" ")" <<
      "CREATE TABLE INET_LOG_FILE_STAT ("
      " ST_LOG_FILE_NAME      VARCHAR(4096) NOT NULL,"
      " ST_LAST_OFFSET        BIGINT NOT NULL"
      ")" <<
      "CREATE TABLE INET_STAT ("
      " st_bpft_start         DATETIME NOT NULL,"
      " st_bpft_stop          DATETIME NOT NULL,"
      " st_bpft_in_ip         VARCHAR(26) NOT NULL,"
      " st_bpft_out_ip        VARCHAR(26) NOT NULL,"
      " st_bpft_ip_proto      SMALLINT NOT NULL,"
      " st_bpft_s_port        SMALLINT NOT NULL,"
      " st_bpft_d_port        SMALLINT NOT NULL,"
      " st_bpft_bytes         SMALLINT NOT NULL,"
      " st_bpft_data_bytes    SMALLINT NOT NULL" ")" <<
      "CREATE UNIQUE INDEX INET_USERS_TRAF_IDX1 ON INET_USERS_TRAF (ST_USER,ST_TIMESTAMP)" <<
      "CREATE DESC INDEX INET_USERS_MONTHLY_TOP_URL_IDX1 ON INET_USERS_MONTHLY_TOP_URL (ST_USER,ST_TIMESTAMP,ST_URL)"
  ;
  if( dynamic_cast< FirebirdDatabase *>(database_.ptr()) != NULL ){
    metadata << "CREATE DESC INDEX INET_USERS_TRAF_IDX2 ON INET_USERS_TRAF (ST_TIMESTAMP)";
  }
  else if( dynamic_cast< MYSQLDatabase *>(database_.ptr()) != NULL ){
    metadata << "CREATE INDEX INET_USERS_TRAF_IDX2 ON INET_USERS_TRAF (ST_TIMESTAMP)";
  }

  if( (bool) config_.section("macroscope").value("DROP_DATABASE", false) ){
    database_->attach();
    database_->drop();
    database_->create();
  }
  database_->attach();

  for( uintptr_t i = 0; i < metadata.count(); i++ ){
    if( dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL )
      if( metadata[i].strncasecmp("CREATE TABLE",12) == 0 )
        metadata[i] += "TYPE = " + config_.section("macroscope").text("mysql_table_type","INNODB");
    try {
      statement_->execute(metadata[i]);
    }
    catch( ksys::ExceptionSP & e ){
      if( e->searchCode(isc_keytoobig) ) throw;
      if( !e->searchCode(isc_no_meta_update,isc_random,ER_TABLE_EXISTS_ERROR,ER_DUP_KEYNAME) ) throw;
    }
  }
#if defined(__WIN32__) || defined(__WIN64__)
  utf8::String  prefix  ("macroscope.windows.");
#else
  utf8::String  prefix  ("macroscope.unix.");
#endif

  if( (bool) config_.valueByPath("macroscope.process_squid_log", true) ){
    parseSquidLogFile(ksys::unScreenString(
      config_.valueByPath(prefix + "squid.log_file_name")),
      config_.valueByPath("macroscope.top10_url", true), 
      config_.valueByPath("macroscope.skip_url"));
  }
  if( (bool) config_.valueByPath("macroscope.process_sendmail_log", true) ){
    parseSendmailLogFile(ksys::unScreenString(
      config_.valueByPath(prefix + "sendmail.log_file_name")),
      utf8::String("@") + config_.valueByPath("macroscope.sendmail.main_domain"),
      config_.valueByPath("macroscope.sendmail.start_year"));
  }
  writeHtmlYearOutput();
}
//------------------------------------------------------------------------------
} // namespace macroscope
//------------------------------------------------------------------------------
int main(int argc, char * argv[])
{
  int errcode = -1;
  adicpp::AutoInitializer autoInitializer;
  autoInitializer = autoInitializer;
  try{
    uintptr_t i;
    ksys::initializeArguments(argc, argv);
    ksys::Config::defaultFileName(SYSCONF_DIR + "macroscope.conf");
    bool dispatch = true;
    for( i = 1; i < ksys::argv().count(); i++ ){
      if( ksys::argv()[i].strcmp("--version") == 0 ){
        ksys::stdErr.debug(9,utf8::String::Stream() << macroscope_version.tex_ << "\n");
        fprintf(stdout,"%s\n",macroscope_version.tex_);
        dispatch = false;
        continue;
      }
      if( ksys::argv()[i].strcmp("-c") == 0 && i + 1 < ksys::argv().count() ){
        ksys::Config::defaultFileName(ksys::argv()[i + 1]);
      }
    }
    if( dispatch ){
      macroscope::Logger logger;
      ksys::stdErr.debug(0,utf8::String::Stream() << macroscope_version.gnu_ << " started\n");
      logger.main();
      ksys::stdErr.debug(0,utf8::String::Stream() << macroscope_version.gnu_ << " stoped\n");
    }
  }
  catch( ksys::ExceptionSP & e ){
    e->writeStdError();
    errcode = e->code();
  }
  catch( ... ){
  }
  return errcode;
}
//------------------------------------------------------------------------------
