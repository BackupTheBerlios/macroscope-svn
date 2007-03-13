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
  shortUrl_("://"),
  config_(newObject<InterlockedConfig<InterlockedMutex> >()),
  trafCacheAutoDrop_(trafCache_),
  cacheSize_(0),
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
void Logger::parseSquidLogLine(char * p,uintptr_t size,Array<const char *> & slcp)
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
    // to ascii
    /*while( *p != '\0' ){
      uint8_t c = *p;
      if( c < ' ' || (c & 0x80) != 0 ){
        c &= 0x7F;
        *p = c >= ' ' ? c : '?';
      }
      p++;
    }*/
    if( i > 0 ) if( strcmp(slcp[i - 1],slcp[i]) == 0 ) i--;
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
  stFileStatSel_->prepare();
  stFileStatSel_->paramAsString("ST_LOG_FILE_NAME",logFileName);
  stFileStatSel_->execute();
  int64_t offset  = 0;
  if( stFileStatSel_->fetch() ){
    stFileStatSel_->fetchAll();
    offset = stFileStatSel_->valueAsMutant("ST_LAST_OFFSET");
  }
  return offset;
}
//------------------------------------------------------------------------------
Logger & Logger::updateLogFileLastOffset(const utf8::String & logFileName,int64_t offset)
{
  stFileStatSel_->prepare();
  stFileStatSel_->paramAsString("ST_LOG_FILE_NAME",logFileName);
  stFileStatSel_->execute();
  if( stFileStatSel_->fetch() ){
    stFileStatSel_->fetchAll();
    stFileStatUpd_->prepare();
    stFileStatUpd_->paramAsString("ST_LOG_FILE_NAME",logFileName);
    stFileStatUpd_->paramAsMutant("ST_LAST_OFFSET",offset);
    stFileStatUpd_->execute();
  }
  else{
    stFileStatIns_->prepare();
    stFileStatIns_->paramAsString("ST_LOG_FILE_NAME",logFileName);
    stFileStatIns_->paramAsMutant("ST_LAST_OFFSET",offset);
    stFileStatIns_->execute();
  }
  return *this;
}
//------------------------------------------------------------------------------
static void fallBackToNewLine(AsyncFile & f)
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
void Logger::parseSquidLogFile(const utf8::String & logFileName, bool top10, const utf8::String & skipUrl)
{
  AsyncFile flog(logFileName);
  flog.readOnly(true).open();
  stMonUrlSel_->text(
    "SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL ? " SQL_NO_CACHE" : "") +
    " ST_URL FROM INET_USERS_MONTHLY_TOP_URL " 
    "WHERE ST_USER = :ST_USER AND ST_TIMESTAMP = :ST_TIMESTAMP AND ST_URL_HASH = :ST_URL_HASH AND "
    "ST_URL = :ST_URL"
  );
  stMonUrlIns_->text(
    "INSERT INTO INET_USERS_MONTHLY_TOP_URL " 
    "(ST_USER, ST_TIMESTAMP, ST_URL, ST_URL_HASH, ST_URL_TRAF, ST_URL_COUNT) VALUES" 
    "(:ST_USER, :ST_TIMESTAMP, :ST_URL, :ST_URL_HASH, :ST_URL_TRAF, 1)"
  );
  stMonUrlUpd_->text(
    "UPDATE INET_USERS_MONTHLY_TOP_URL "
    "SET "
    "ST_URL_TRAF = ST_URL_TRAF + :ST_URL_TRAF,"
    "ST_URL_COUNT = ST_URL_COUNT + 1 "
    "WHERE ST_USER = :ST_USER AND ST_TIMESTAMP = :ST_TIMESTAMP AND ST_URL_HASH = :ST_URL_HASH AND "
    "ST_URL = :ST_URL"
  );
  stTrafIns_->text(
    "INSERT INTO INET_USERS_TRAF"
    "(ST_USER, ST_TIMESTAMP, ST_TRAF_WWW, ST_TRAF_SMTP)"
    "VALUES (:ST_USER, :ST_TIMESTAMP, :ST_TRAF_WWW, 0)"
  );
  stTrafUpd_->text(
    "UPDATE INET_USERS_TRAF SET ST_TRAF_WWW = ST_TRAF_WWW + :ST_TRAF_WWW "
    "WHERE ST_USER = :ST_USER AND ST_TIMESTAMP = :ST_TIMESTAMP"
  );
/*
  statement_->text("DELETE FROM INET_USERS_TRAF")->execute();
  updateLogFileLastOffset(logFileName,0);
 */
  database_->start();
  if( (bool) config_->valueByPath(section_ + ".squid.reset_log_file_position",false) )
    updateLogFileLastOffset(logFileName,0);
  int64_t offset = fetchLogFileLastOffset(logFileName);
  if( flog.seekable() ) flog.seek(offset);
  fallBackToNewLine(flog);
  int64_t lineNo = 1, tma = 0;
  uint64_t startTime = timeFromTimeString(config_->valueByPath(section_ + ".squid.start_time","01.01.1980"));
  uintptr_t size;
  Array<const char *> slcp;
  int64_t cl = getlocaltimeofday();
  AsyncFile::LineGetBuffer lgb(flog);
  lgb.codePage_ = config_->valueByPath(section_ + ".squid.log_file_codepage",CP_ACP);
  stTrafIns_->prepare();
  stTrafUpd_->prepare();
  stMonUrlSel_->prepare();
  stMonUrlUpd_->prepare();
  stMonUrlIns_->prepare();
  bool validLine = false, startTimeLinePrinted = false;
  for(;;){
    utf8::String sb;
    if( flog.gets(sb,&lgb) ) break;
    size = sb.size();
    validLine = size > 0 && sb.c_str()[size - 1] == '\n';
    parseSquidLogLine(sb.c_str(),size,slcp);
    validLine = validLine && slcp.count() >= 7 && slcp[7] != NULL;
    uint64_t timeStamp1;
    if( validLine ){
      uint64_t a;
      int r = sscanf(slcp[0],"%"PRIu64".%"PRIu64,&timeStamp1,&a);
      timeStamp1 *= 1000000u;
      timeStamp1 += a * 1000u;
      timeStamp1 -= getgmtoffset(); // in database must be in GMT
      validLine = validLine && r == 2 && timeStamp1 >= startTime;
      if( validLine && verbose_ && !startTimeLinePrinted ){
        fprintf(stderr,"\nstart time %s, line: %"PRId64", offset: %"PRIu64"\n",
	  (const char * ) utf8::time2Str(int64_t(timeStamp1)).getOEMString(),lineNo,lgb.tell() - size
	);
	startTimeLinePrinted = true;
      }
    }
    uintmax_t traf;
    if( validLine ){
      int r = sscanf(slcp[4],"%"PRIuMAX,&traf);
      validLine = validLine && r == 1 && traf > 0;
    }
    if( validLine ){
      validLine = validLine && strchr(slcp[7],'%') == NULL;
      validLine = validLine && strcmp(slcp[7],"-") != 0;
      validLine = validLine && strncmp(slcp[3],"NONE",4) != 0;
      validLine = validLine && strncmp(slcp[3],"TCP_DENIED",10) != 0;
      validLine = validLine && strncmp(slcp[3],"UDP_DENIED",10) != 0;
    }
    utf8::String st_user, st_url;
    if( validLine ){
      st_user = utf8::plane(slcp[7]).left(80).replaceAll("\"","").lower();
      st_url = utf8::plane(slcp[6]);
      validLine = validLine && st_url.strcasestr(skipUrl).position() < 0;
    }
    if( validLine ){
      st_url = shortUrl(st_url).left(4096).lower();
      Mutant timeStamp(timeStampRoundToMin(timeStamp1));
      try {
        stTrafIns_->paramAsString("ST_USER",st_user);
        stTrafIns_->paramAsMutant("ST_TIMESTAMP",timeStamp);
        stTrafIns_->paramAsMutant("ST_TRAF_WWW",traf);
        stTrafIns_->execute();
      }
      catch( ExceptionSP & e ){
        if( !e->searchCode(isc_no_dup,ER_DUP_ENTRY) ) throw;
        stTrafUpd_->paramAsString("ST_USER",st_user);
        stTrafUpd_->paramAsMutant("ST_TIMESTAMP",timeStamp);
        stTrafUpd_->paramAsMutant("ST_TRAF_WWW",traf);
        stTrafUpd_->execute();
      }
      if( top10 ){
        int64_t urlHash = st_url.hash_ll(true);
        stMonUrlSel_->
          paramAsString("ST_USER", st_user)->
          paramAsMutant("ST_TIMESTAMP", timeStamp)->
          paramAsMutant("ST_URL", st_url)->
          paramAsMutant("ST_URL_HASH",urlHash)->
          execute()->fetchAll();
        if( stMonUrlSel_->rowCount() > 0 ){
          stMonUrlUpd_->
            paramAsString("ST_USER", st_user)->
            paramAsMutant("ST_TIMESTAMP", timeStamp)->
            paramAsMutant("ST_URL", st_url)->
            paramAsMutant("ST_URL_HASH",urlHash)->
            paramAsMutant("ST_URL_TRAF", traf)->execute();
        }
        else {
          stMonUrlIns_->
            paramAsString("ST_USER", st_user)->
            paramAsMutant("ST_TIMESTAMP", timeStamp)->
            paramAsMutant("ST_URL", st_url)->
            paramAsMutant("ST_URL_HASH",urlHash)->
            paramAsMutant("ST_URL_TRAF", traf)->execute();
        }
      }
    }
    if( lineNo % 8192 == 0 ){
      if( flog.seekable() ) updateLogFileLastOffset(logFileName,lgb.tell() - (validLine ? 0 : size));
      database_->commit();
      database_->start();
    }
    printStat(lineNo,offset,lgb.tell(),flog.size(),cl,&tma);
    lineNo++;
  }
  if( validLine && flog.seekable() ) updateLogFileLastOffset(logFileName,lgb.tell() - (validLine ? 0 : size));
  database_->commit();
  printStat(lineNo,offset,lgb.tell(),flog.size(),cl);
}
//------------------------------------------------------------------------------
void Logger::parseSendmailLogFile(const utf8::String & logFileName, const utf8::String & domain,uintptr_t startYear)
{
  AsyncFile flog(logFileName);
  flog.readOnly(true).open();
  stMsgsIns_->text(
    "INSERT INTO INET_SENDMAIL_MESSAGES (ST_FROM,ST_MSGID,ST_MSGSIZE) " 
    "VALUES (:ST_FROM,:ST_MSGID,:ST_MSGSIZE);"
  );
  stMsgsSel_->text(
    "SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL ? " SQL_NO_CACHE" : "") +
    " * FROM INET_SENDMAIL_MESSAGES WHERE ST_MSGID = :ST_MSGID"
  );
  stMsgsDel_->text("DELETE FROM INET_SENDMAIL_MESSAGES");
  stMsgsDel2_->text("DELETE FROM INET_SENDMAIL_MESSAGES WHERE ST_MSGID = :ST_MSGID");
  stMsgsSelCount_->text(
    "SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL ? " SQL_NO_CACHE" : "") +
    " COUNT(*) FROM INET_SENDMAIL_MESSAGES"
  );
  stTrafIns_->text(
    "INSERT INTO INET_USERS_TRAF"
    "(ST_USER, ST_TIMESTAMP, ST_TRAF_WWW, ST_TRAF_SMTP)"
    "VALUES (:ST_USER, :ST_TIMESTAMP, 0, :ST_TRAF_SMTP)"
  );
  stTrafUpd_->text(
    "UPDATE INET_USERS_TRAF SET ST_TRAF_SMTP = ST_TRAF_SMTP + :ST_TRAF_SMTP "
    "WHERE ST_USER = :ST_USER AND ST_TIMESTAMP = :ST_TIMESTAMP"
  );
/*
  statement_->text("DELETE FROM INET_USERS_TRAF")->execute();
  updateLogFileLastOffset(flog.fileName(),0);
 */
  tm lt;
  statement_->text(
    "SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL ? " SQL_NO_CACHE" : "") +
    " MAX(ST_TIMESTAMP) AS ST_TIMESTAMP "
    "FROM INET_USERS_TRAF WHERE ST_TRAF_SMTP > 0"
  );
  database_->start();
  if( (bool) config_->valueByPath(section_ + ".sendmail.reset_log_file_position",false) )
    updateLogFileLastOffset(logFileName,0);
  statement_->execute()->fetchAll();
  memset(&lt,0,sizeof(lt));
  if( statement_->rowCount() > 0 && statement_->fieldIndex("ST_TIMESTAMP") >= 0 && !statement_->valueIsNull("ST_TIMESTAMP") ){
    lt = statement_->valueAsMutant("ST_TIMESTAMP");
    startYear = lt.tm_year + 1900;
  }
  int64_t offset = fetchLogFileLastOffset(logFileName);
  if( flog.seekable() ) flog.seek(offset);
  fallBackToNewLine(flog);
  int64_t   lineNo  = 1, tma = 0;
  uintptr_t size;
  intptr_t  mon     = 0;
  int64_t   cl      = getlocaltimeofday();
  AsyncFile::LineGetBuffer lgb(flog);
  lgb.codePage_ = config_->valueByPath(section_ + ".sendmail.log_file_codepage",CP_ACP);
  for(;;){
    utf8::String sb;
    if( flog.gets(sb,&lgb) ) break;
    size = sb.size();
    if( size > 0 && sb.c_str()[size - 1] == '\n' ){
      char * a, * id, * idl, * prefix, * prefixl, * from, * to, * stat; //* relay;
      from = strstr(sb.c_str(), "from=");
      to = strstr(sb.c_str(), " to=");
      if( ((from != NULL || to != NULL)) && (id = strstr(sb.c_str(), "sm-mta[")) != NULL ){
        // get time
        memset(&lt, 0, sizeof(lt));
        lt.tm_mon = (int) str2Month(sb.c_str());
        if( lt.tm_mon < mon ) startYear++;
        mon = lt.tm_mon;
        lt.tm_year = int(startYear - 1900);
        sscanf(sb.c_str() + 4, "%u %u:%u:%u", &lt.tm_mday, &lt.tm_hour, &lt.tm_min, &lt.tm_sec);
        // get msgid
        prefix = id;
        for( prefixl = prefix; *prefixl != '['; prefixl++ );
        while( *id != ']' ) id++;
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
            if( *a == '@' && utf8::plane(a,domain.strlen()).strcasecmp(domain) == 0 ){
              st_user = utf8::plane(from,a - from).lower();
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
            if( *a == '@' && utf8::plane(a,domain.strlen()).strcasecmp(domain) == 0 ){
              st_user = utf8::plane(to,a - to).lower();
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
	      st_user = st_user.lower().replaceAll("\"","");
        if( from != NULL && msgSize > 0 ){
          try {
            stMsgsIns_->prepare()->
              paramAsString("ST_FROM",st_user)->
              paramAsString("ST_MSGID",utf8::plane(id, idl - id))->
              paramAsMutant("ST_MSGSIZE",msgSize)->execute();
          }
          catch( ExceptionSP & e ){
            if( !e->searchCode(isc_no_dup, ER_DUP_ENTRY) ) throw;
          }
        }
        else if( to != NULL && stat != NULL && strncmp(stat, "Sent", 4) == 0 ){
          stMsgsSel_->prepare()->
            paramAsString("ST_MSGID",utf8::plane(id,idl - id))->
            execute()->fetchAll();
          if( stMsgsSel_->rowCount() > 0 ){
            if( st_user.strlen() == 0 ) st_user = stMsgsSel_->valueAsString("ST_FROM");
            if( st_user.strlen() > 0 ){
              msgSize = stMsgsSel_->valueAsMutant("ST_MSGSIZE");
	      // time in database must be in GMT
              try {
                stTrafIns_->prepare()->
                  paramAsString("ST_USER", st_user)->
                  paramAsMutant("ST_TIMESTAMP",timeStampRoundToMin(tm2Time(lt) - getgmtoffset()))->
                  paramAsMutant("ST_TRAF_SMTP",msgSize)->execute();
              }
              catch( ExceptionSP & e ){
                if( !e->searchCode(isc_no_dup, ER_DUP_ENTRY) ) throw;
                stTrafUpd_->prepare()->
                  paramAsString("ST_USER",st_user)->
                  paramAsMutant("ST_TIMESTAMP",timeStampRoundToMin(tm2Time(lt) - getgmtoffset()))->
                  paramAsMutant("ST_TRAF_SMTP",msgSize)->execute();
              }
              stMsgsDel2_->prepare()->
                paramAsString("ST_MSGID",stMsgsSel_->paramAsString("ST_MSGID"))->execute();
            }
          }
        }
      }
    }
    if( lineNo % 1024 == 0 ){
      if( flog.seekable() ) updateLogFileLastOffset(logFileName,lgb.tell());
      database_->commit();
      database_->start();
    }
    printStat(lineNo,offset,lgb.tell(),flog.size(),cl,&tma);
    lineNo++;
  }
  if( flog.seekable() ) updateLogFileLastOffset(logFileName,lgb.tell());
  stMsgsDel_->execute();
  stMsgsSelCount_->execute()->fetchAll();
  database_->commit();
  printStat(lineNo,offset,lgb.tell(),flog.size(),cl);
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

  verbose_ = config_->section("macroscope").value("verbose", false);

  database_.ptr(Database::newDatabase(config_.ptr()));
  statement_.ptr(database_->newAttachedStatement());
  statement2_.ptr(database_->newAttachedStatement());
  statement3_.ptr(database_->newAttachedStatement());
  statement4_.ptr(database_->newAttachedStatement());
  statement5_.ptr(database_->newAttachedStatement());
  statement6_.ptr(database_->newAttachedStatement());
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
  stDNSCacheSel_.ptr(database_->newAttachedStatement());
  stDNSCacheIns_.ptr(database_->newAttachedStatement());

#ifndef NDEBUG
  if( verbose_ && dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL ){
    fprintf(stderr,"%"PRId64"\n",gettimeofday() / 1000000);
    fbcpp::api.open();
    struct tm tm0 = time2tm(gettimeofday()), tm1 = time2tm(getlocaltimeofday());
    ISC_TIMESTAMP isct0 = fbcpp::tm2IscTimeStamp(tm0), isct1 = fbcpp::tm2IscTimeStamp(tm1);
    ISC_TIMESTAMP isctm0, isctm1;
    fbcpp::api.isc_encode_timestamp(&tm0,&isctm0);
    fbcpp::api.isc_encode_timestamp(&tm1,&isctm1);
    fprintf(stderr,"%"PRIdMAX", %"PRIdMAX"\n",
      intmax_t(intmax_t(isctm0.timestamp_time) - isct0.timestamp_time),
      intmax_t(intmax_t(isctm1.timestamp_time) - isct1.timestamp_time)
    );
    fbcpp::api.close();
  }
#endif

  stFileStatSel_->text(
    "SELECT " + utf8::String(dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL ? " SQL_NO_CACHE" : "") +
    " ST_LAST_OFFSET FROM INET_LOG_FILE_STAT " 
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

  database_->create();

  Vector<utf8::String> metadata;
  if( dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL )
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
    " st_ip            CHAR(8) CHARACTER SET ascii NOT NULL,"
    " st_dgram_bytes   BIGINT NOT NULL,"
    " st_data_bytes    BIGINT NOT NULL"
    ")" <<
    "CREATE INDEX INET_BPFT_STAT_CACHE_IDX1 ON INET_BPFT_STAT_CACHE (st_if,st_bt,st_et,st_ip)" <<
    "CREATE TABLE INET_DNS_CACHE ("
    " st_ip            CHAR(8) CHARACTER SET ascii NOT NULL PRIMARY KEY,"
    " st_name          VARCHAR(" + utf8::int2Str(NI_MAXHOST + NI_MAXSERV + 1) + ") CHARACTER SET ascii NOT NULL"
    ")" <<
    "CREATE UNIQUE INDEX INET_USERS_TRAF_IDX1 ON INET_USERS_TRAF (ST_USER,ST_TIMESTAMP)" <<
    "CREATE INDEX INET_USERS_TRAF_IDX4 ON INET_USERS_TRAF (ST_TIMESTAMP)" <<
    "CREATE INDEX INET_USERS_TRAF_IDX3 ON INET_USERS_TRAF (ST_TRAF_SMTP,ST_TIMESTAMP)"
  ;
  if( dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL ){
    metadata << "CREATE DESC INDEX INET_USERS_TRAF_IDX2 ON INET_USERS_TRAF (ST_TIMESTAMP)";
    metadata << "CREATE DESC INDEX INET_USERS_MONTHLY_TOP_URL_IDX1 ON INET_USERS_MONTHLY_TOP_URL (ST_USER,ST_TIMESTAMP,ST_URL_HASH)";
  }
  else if( dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL ){
    metadata << "CREATE INDEX INET_USERS_TRAF_IDX2 ON INET_USERS_TRAF (ST_TIMESTAMP)";
    metadata << "CREATE INDEX INET_USERS_MONTHLY_TOP_URL_IDX1 ON INET_USERS_MONTHLY_TOP_URL (ST_USER,ST_TIMESTAMP,ST_URL_HASH)";
  }
  if( (bool) config_->section("macroscope").value("DROP_DATABASE", false) ){
    database_->attach();
    database_->drop();
    database_->create();
  }

//#if !__FreeBSD__
  if( dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL ){
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
    if( dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL )
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
  section_ = "macroscope";
  if( (bool) config_->valueByPath("macroscope.process_squid_log",true) ){
    Mutant m0(config_->valueByPath("macroscope.squid.log_file_name"));
    Mutant m1(config_->valueByPath("macroscope.squid.top10_url",true));
    Mutant m2(config_->textByPath("macroscope.squid.skip_url").lower());
    parseSquidLogFile(m0,m1,m2);
  }
  if( (bool) config_->valueByPath("macroscope.process_sendmail_log",true) ){
    Mutant m0(config_->valueByPath("macroscope.sendmail.log_file_name"));
    Mutant m1(utf8::String("@") + config_->valueByPath("macroscope.sendmail.main_domain"));
    Mutant m2(config_->valueByPath("macroscope.sendmail.start_year"));
    parseSendmailLogFile(m0,m1,m2);
  }
  ellapsed_ = getlocaltimeofday();
  writeHtmlYearOutput();
  if( (bool) config_->valueByPath("macroscope.process_bpft_log",true) ){
    for( uintptr_t i = 0; i < config_->sectionByPath("macroscope.bpft").sectionCount(); i++ ){
      sectionName_ = config_->sectionByPath("macroscope.bpft").section(i).name();
      if( sectionName_.strcasecmp("decoration") == 0 ) continue;
      section_ = "macroscope.bpft." + sectionName_;
      parseBPFTLogFile();
    }
  }
  for( uintptr_t i = 0; i < config_->sectionByPath("macroscope.bpft").sectionCount(); i++ ){
    sectionName_ = config_->sectionByPath("macroscope.bpft").section(i).name();
    if( sectionName_.strcasecmp("decoration") == 0 ) continue;
    section_ = "macroscope.bpft." + sectionName_;
    ellapsed_ = getlocaltimeofday();
    writeBPFTHtmlReport();
  }
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
