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
Logger::Logger() :
  shortUrl_(utf8::string("://")), ellapsed_(gettimeofday())
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
intptr_t Logger::strToMonth(const char * month)
{
  if( strncmp(month,"Jan",3) == 0 ) return 0;
  if( strncmp(month,"Feb",3) == 0 ) return 1;
  if( strncmp(month,"Mar",3) == 0 ) return 2;
  if( strncmp(month,"Apr",3) == 0 ) return 3;
  if( strncmp(month,"Mai",3) == 0 ) return 4;
  if( strncmp(month,"May",3) == 0 ) return 4;
  if( strncmp(month,"Jun",3) == 0 ) return 5;
  if( strncmp(month,"Jul",3) == 0 ) return 6;
  if( strncmp(month,"Aug",3) == 0 ) return 7;
  if( strncmp(month,"Sep",3) == 0 ) return 8;
  if( strncmp(month,"Okt",3) == 0 ) return 9;
  if( strncmp(month,"Oct",3) == 0 ) return 9;
  if( strncmp(month,"Nov",3) == 0 ) return 10;
  if( strncmp(month,"Dez",3) == 0 ) return 11;
  if( strncmp(month,"Dec",3) == 0 ) return 11;
  return -1;
}
//------------------------------------------------------------------------------
void Logger::printStat(
  int64_t lineNo,int64_t spos,int64_t pos,int64_t size,int64_t cl)
{
  if( verbose_ ){
    int64_t ct = gettimeofday() - cl;
    int64_t q = (ct * (size - pos)) / (pos - spos <= 0 ? 1 : pos - spos);
    size -= spos;
    if( size <= 0 ) size = 1;
    pos -= spos;
    if( pos <= 0 ) pos = 1;
    if( ct <= 0 ) ct = 1;
    int64_t a = pos * 100 / size, b = lineNo * 1000000 / ct;
#ifdef HAVE_ISATTY
    if( isatty(fileno(stderr)) ) fprintf(stderr,"\r");
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
      (const char *) utf8::elapsedTime2Str(q).getANSIString(),
      (const char *) utf8::elapsedTime2Str(ct).getANSIString()
    );
#ifdef HAVE_ISATTY
    if( !isatty(fileno(stderr)) )
#endif
      fprintf(stderr,"\n");
//    printf("%-20s\n",utf8::elapsedTime2Str(size * ct / pos - ct).c_str());
  }
}
//------------------------------------------------------------------------------
void Logger::parseSquidLogLine(char * p,uintptr_t size,ksys::Array<const char *> & slcp)
{
  char * rb = p + size, * a, * s;
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
    if( i > 0 ) if( strcmp(slcp[i - 1],slcp[i]) == 0 ) i--;
    p = s;
  }
}
//------------------------------------------------------------------------------
utf8::String Logger::shortUrl(const utf8::String & url)
{
  utf8::String s(url);
  utf8::String::Iterator i(url.strstr(shortUrl_));
  if( i.position() >= 0 ){
    i += 3;
    while( i.next() && i.getChar() != '/' );
    s = utf8::string(utf8::String::Iterator(s),i);
  }
  return s;
}
//------------------------------------------------------------------------------
int64_t Logger::fetchLogFileLastOffset(const utf8::String & logFileName)
{
  statement_->text(utf8::string(
    "SELECT ST_LAST_OFFSET FROM INET_LOG_FILE_STAT WHERE ST_LOG_FILE_NAME = :ST_LOG_FILE_NAME"
  ));
  statement_->prepare();
  statement_->paramAsString(utf8::string("ST_LOG_FILE_NAME"),logFileName);
  statement_->execute();
  int64_t offset = 0;
  if( statement_->fetch() ){
    statement_->fetchAll();
    offset = statement_->valueAsMutant(utf8::string("ST_LAST_OFFSET"));
  }
  return offset;
}
//------------------------------------------------------------------------------
Logger & Logger::updateLogFileLastOffset(const utf8::String & logFileName,int64_t offset)
{
  stFileStatSel_->prepare();
  stFileStatSel_->paramAsString(utf8::string("ST_LOG_FILE_NAME"),logFileName);
  stFileStatSel_->execute();
  if( stFileStatSel_->fetch() ){
    stFileStatSel_->fetchAll();
    stFileStatUpd_->prepare();
    stFileStatUpd_->paramAsString(utf8::string("ST_LOG_FILE_NAME"),logFileName);
    stFileStatUpd_->paramAsMutant(utf8::string("ST_LAST_OFFSET"),offset);
    stFileStatUpd_->execute();
  }
  else {
    stFileStatIns_->prepare();
    stFileStatIns_->paramAsString(utf8::string("ST_LOG_FILE_NAME"),logFileName);
    stFileStatIns_->paramAsMutant(utf8::string("ST_LAST_OFFSET"),offset);
    stFileStatIns_->execute();
  }
  return *this;
}
//------------------------------------------------------------------------------
void Logger::parseSquidLogFile(
  const utf8::String & logFileName,
  bool top10,
  const utf8::String & skipUrl)
{
  ksys::FileHandleContainer flog(logFileName);
  if( flog.tryOpen() ){
    stTrafIns_->text(utf8::string(
      "INSERT INTO INET_USERS_TRAF"
      "(ST_USER, ST_TIMESTAMP, ST_TRAF_WWW, ST_TRAF_SMTP)"
      "VALUES (:ST_USER, :ST_TIMESTAMP, :ST_TRAF_WWW, 0)"
    ));
    stTrafUpd_->text(utf8::string(
      "UPDATE INET_USERS_TRAF SET ST_TRAF_WWW = ST_TRAF_WWW + :ST_TRAF_WWW "
      "WHERE ST_USER = :ST_USER AND ST_TIMESTAMP = :ST_TIMESTAMP"
    ));

    database_->start();
    int64_t offset = fetchLogFileLastOffset(logFileName);
    flog.seek(offset);

    ksys::AutoPtr<char> b;
    int64_t lineNo = 1;
    uintptr_t size;
    ksys::Array<const char *> slcp;
    int64_t cl = gettimeofday();
    for(;;){
//   	  printf("%"PRId64"\n",lineNo);
      if( (size = flog.gets(b)) <= 0 ) break;
      if( b[size - 1] == '\n' ){
        parseSquidLogLine(b,size,slcp);
        intmax_t traf(utf8::str2Int(utf8::string(slcp[4])));
        if( traf > 0 &&
              slcp[3] != NULL && slcp[7] != NULL &&
              strchr(slcp[7],'%') == NULL && strcmp(slcp[7],"-") != 0 &&
              strncmp(slcp[3],"NONE",4) != 0 &&
              strncmp(slcp[3],"TCP_DENIED",10) != 0 &&
              strncmp(slcp[3],"UDP_DENIED",10) != 0 ){
          double timeStamp1;
          sscanf(slcp[0],"%lf",&timeStamp1);
          utf8::String st_user(utf8::string(slcp[7])), st_url(utf8::string(slcp[6]));
          st_user = st_user.lower();
          if( st_url.strcasestr(skipUrl).position() < 0 ){
            st_url = shortUrl(st_url).lower();
            ksys::Mutant timeStamp(timeStampRoundToMin(timeStamp1 * 1000000));
            try {
              stTrafIns_->prepare();
              stTrafIns_->paramAsString(utf8::string("ST_USER"),st_user);
              stTrafIns_->paramAsMutant(utf8::string("ST_TIMESTAMP"),timeStamp);
              stTrafIns_->paramAsMutant(utf8::string("ST_TRAF_WWW"),traf);
              stTrafIns_->execute();
      	    }
            catch( ksys::ExceptionSP & e ){
              if( !e->searchCode(isc_no_dup,ER_DUP_ENTRY) ) throw;
              stTrafUpd_->prepare();
              stTrafUpd_->paramAsString(utf8::string("ST_USER"),st_user);
              stTrafUpd_->paramAsMutant(utf8::string("ST_TIMESTAMP"),timeStamp);
              stTrafUpd_->paramAsMutant(utf8::string("ST_TRAF_WWW"),traf);
              stTrafUpd_->execute();
            }

            int64_t urlHash = st_url.hash_ll(true);

            stMonUrlSel_->prepare()->
              paramAsString(utf8::string("ST_USER"),st_user)->
              paramAsMutant(utf8::string("ST_TIMESTAMP"),timeStamp)->
              paramAsMutant(utf8::string("ST_URL"),st_url)->
              paramAsMutant(utf8::string("ST_URL_HASH"),urlHash)->execute()->fetchAll();
            if( stMonUrlSel_->rowCount() > 0 ){
              stMonUrlUpd_->prepare()->
                paramAsString(utf8::string("ST_USER"),st_user)->
                paramAsMutant(utf8::string("ST_TIMESTAMP"),timeStamp)->
                paramAsMutant(utf8::string("ST_URL"),st_url)->
                paramAsMutant(utf8::string("ST_URL_HASH"),urlHash)->
                paramAsMutant(utf8::string("ST_URL_TRAF"),traf)->execute();
       	    }
            else {
              stMonUrlIns_->prepare()->
                paramAsString(utf8::string("ST_USER"),st_user)->
                paramAsMutant(utf8::string("ST_TIMESTAMP"),timeStamp)->
                paramAsMutant(utf8::string("ST_URL"),st_url)->
                paramAsMutant(utf8::string("ST_URL_HASH"),urlHash)->
                paramAsMutant(utf8::string("ST_URL_TRAF"),traf)->execute();
      	    }
          }
        }
      }
      if( lineNo % 8192 == 0 ){
        updateLogFileLastOffset(logFileName,flog.tell());
        database_->commit();
        database_->start();
      }
      if( lineNo % 1024 == 0 ){
        printStat(lineNo,offset,flog.tell(),flog.size(),cl);
      }
      lineNo++;
    }
    printStat(lineNo,offset,flog.tell(),flog.size(),cl);
    updateLogFileLastOffset(logFileName,flog.tell());
    database_->commit();
  }
}
//------------------------------------------------------------------------------
void Logger::parseSendmailLogFile(const utf8::String & logFileName,const utf8::String & domain,uintptr_t startYear)
{
  ksys::FileHandleContainer flog(logFileName);
  flog.open();
  stTrafIns_->text(utf8::string(
    "INSERT INTO INET_USERS_TRAF"
    "(ST_USER, ST_TIMESTAMP, ST_TRAF_WWW, ST_TRAF_SMTP)"
    "VALUES (:ST_USER, :ST_TIMESTAMP, 0, :ST_TRAF_SMTP)"
  ));
  stTrafUpd_->text(utf8::string(
    "UPDATE INET_USERS_TRAF SET ST_TRAF_SMTP = ST_TRAF_SMTP + :ST_TRAF_SMTP "
    "WHERE ST_USER = :ST_USER AND ST_TIMESTAMP = :ST_TIMESTAMP"
  ));

  ksys::AutoPtr<char> b;
  database_->start();
  int64_t offset = fetchLogFileLastOffset(logFileName);
  flog.seek(offset);
  int64_t lineNo = 1;
  uintptr_t size;
  intptr_t mon = 0;
  int64_t cl = gettimeofday();
  for(;;){
    if( (size = flog.gets(b)) <= 0 ) break;
    if( b[size - 1] == '\n' ){
      char * a, * id, * idl, * prefix, * prefixl, * from, * to, * stat; //* relay;
      from = strstr(b.ptr(),"from=");
      to = strstr(b.ptr()," to=");
      if( ((from != NULL || to != NULL)) && (id = strstr(b.ptr(),"sm-mta[")) != NULL ){
// get time
        tm lt;
        memset(&lt,0,sizeof(lt));
        lt.tm_mon = strToMonth(b);
        if( lt.tm_mon < mon ) startYear++;
        mon = lt.tm_mon;
        lt.tm_year = startYear - 1900;
        sscanf(b.ptr() + 4,"%u %u:%u:%u",&lt.tm_mday,&lt.tm_hour,&lt.tm_min,&lt.tm_sec);
// get msgid
        prefix = id;
        for( prefixl = prefix; *prefixl != '['; prefixl++ );
        while( *id != ']' ) id++;
        while( *++id == ':' );
        while( *++id == ' ' );
        for( idl = id; *idl != ':'; idl++ );
        utf8::String st_user;
// get from
        if( from != NULL ){
          if( from[5] == '<' ){
            from += 6;
            a = from;
            while( *a != '@' && *a != '>' && *a != ',' ){
              if( *a == ':' ) from = a + 1;
              a++;
            }
            if( *a == '@' && utf8::string(a,domain.strlen()).strcasecmp(domain) == 0 ){
              st_user = utf8::string(from,a - from).lower();
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
            if( *a == '@' && utf8::string(a,domain.strlen()).strcasecmp(domain) == 0 ){
              st_user = utf8::string(to,a - to).lower();
            }
          }
        }
// get size
        uintptr_t msgSize = 0;
        if( (a = strstr(b.ptr(),"size=")) != NULL ){
          sscanf(a + 5,"%"PRIuPTR,&msgSize);
        }
// get nrcpts
        uintptr_t nrcpts = 0;
        if( (a = strstr(b.ptr(),"nrcpts=")) != NULL ){
          sscanf(a + 7,"%"PRIuPTR,&nrcpts);
        }
// get stat
        if( (stat = strstr(b.ptr(),"stat=")) != NULL ){
          stat += 5;
        }
        if( from != NULL && msgSize > 0 ){
          try {
            stMsgsIns_->prepare()->
              paramAsString(utf8::string("ST_FROM"),st_user)->
              paramAsString(utf8::string("ST_MSGID"),utf8::string(id,idl - id))->
              paramAsMutant(utf8::string("ST_MSGSIZE"),msgSize)->execute();
          }
          catch( ksys::ExceptionSP & e ){
            if( !e->searchCode(isc_no_dup,ER_DUP_ENTRY) ) throw;
	        }
        }
        else if( to != NULL && stat != NULL && strncmp(stat,"Sent",4) == 0 ){
          stMsgsSel_->prepare()->
            paramAsString(utf8::string("ST_MSGID"),utf8::string(id,idl - id))->
            execute()->fetchAll();
          if( stMsgsSel_->rowCount() > 0 ){
            if( st_user.strlen() == 0 ) st_user = stMsgsSel_->valueAsString(utf8::string("ST_FROM"));
            if( st_user.strlen() > 0 ){
              msgSize = stMsgsSel_->valueAsMutant(utf8::string("ST_MSGSIZE"));
              try {
                stTrafIns_->prepare()->
                  paramAsString(utf8::string("ST_USER"),st_user)->
                  paramAsMutant(utf8::string("ST_TIMESTAMP"),timeStampRoundToMin(lt))->
                  paramAsMutant(utf8::string("ST_TRAF_SMTP"),msgSize)->execute();
              }
              catch( ksys::ExceptionSP & e ){
                if( !e->searchCode(isc_no_dup,ER_DUP_ENTRY) ) throw;
                stTrafUpd_->prepare()->
                  paramAsString(utf8::string("ST_USER"),st_user)->
                  paramAsMutant(utf8::string("ST_TIMESTAMP"),timeStampRoundToMin(lt))->
                  paramAsMutant(utf8::string("ST_TRAF_SMTP"),msgSize)->execute();
              }
              stMsgsDel2_->prepare()->
                paramAsString(utf8::string("ST_MSGID"),
                  stMsgsSel_->paramAsString(utf8::string("ST_MSGID"))
                )->execute();
            }
          }
        }
      }
    }
    if( lineNo % 65536 == 0 ){
      updateLogFileLastOffset(logFileName,flog.tell());
      database_->commit();
      database_->start();
    }
    if( lineNo % 4096 == 0 ){
      printStat(lineNo,offset,flog.tell(),flog.size(),cl);
    }
    lineNo++;
  }
  printStat(lineNo,offset,flog.tell(),flog.size(),cl);
  updateLogFileLastOffset(logFileName,flog.tell());
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
  
  utf8::String lockFileName(
    ksys::getTempPath() +
    ksys::unScreenString(
      config_.section(utf8::string("macroscope")).text(
        utf8::string("lock_file_name"),
        utf8::string("F2BC263F-C902-4398-AF11-5173F2B6B4F4")
      )
    )
  );

  ksys::FileHandleContainer lf(lockFileName,true);
  lf.removeAfterClose(true);
  if( !lf.tryOpen(true) ){
    int32_t err = errno;
    fprintf(stderr,"lower frequency of program starting needed\n");
    throw ksys::ExceptionSP(
      new ksys::Exception(err,utf8::string("lower frequency of program starting needed"))
    );
  }
  lf << utf8::int2Str(getpid());
  
  verbose_ = config_.section(utf8::string("macroscope")).value(utf8::string("verbose"),false);

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

  stMonUrlSel_->text(utf8::string(
    "SELECT ST_URL_HASH FROM INET_USERS_MONTHLY_TOP_URL "
	  "WHERE ST_USER = :ST_USER AND ST_TIMESTAMP = :ST_TIMESTAMP AND "
	  "ST_URL_HASH = :ST_URL_HASH AND ST_URL = :ST_URL"
  ));

  stMonUrlIns_->text(utf8::string(
    "INSERT INTO INET_USERS_MONTHLY_TOP_URL "
    "(ST_USER, ST_TIMESTAMP, ST_URL, ST_URL_HASH, ST_URL_TRAF) VALUES"
	  "(:ST_USER, :ST_TIMESTAMP, :ST_URL, :ST_URL_HASH, :ST_URL_TRAF)"
  ));

  stMonUrlUpd_->text(utf8::string(
    "UPDATE INET_USERS_MONTHLY_TOP_URL SET ST_URL_TRAF = :ST_URL_TRAF + :ST_URL_TRAF "
	  "WHERE ST_USER = :ST_USER AND ST_TIMESTAMP = :ST_TIMESTAMP AND "
	  "ST_URL_HASH = :ST_URL_HASH AND ST_URL = :ST_URL"
  ));

  stFileStatSel_->text(utf8::string(
   "SELECT ST_LAST_OFFSET FROM INET_LOG_FILE_STAT "
   "WHERE ST_LOG_FILE_NAME = :ST_LOG_FILE_NAME"
  ));

  stFileStatIns_->text(utf8::string(
    "INSERT INTO INET_LOG_FILE_STAT"
	  "(ST_LOG_FILE_NAME, ST_LAST_OFFSET)"
    "VALUES"
	  "(:ST_LOG_FILE_NAME,:ST_LAST_OFFSET)"
  ));

  stFileStatUpd_->text(utf8::string(
   "UPDATE INET_LOG_FILE_STAT SET ST_LAST_OFFSET = :ST_LAST_OFFSET "
   "WHERE ST_LOG_FILE_NAME = :ST_LOG_FILE_NAME"
  ));

  stMsgsIns_->text(utf8::string(
    "INSERT INTO INET_SENDMAIL_MESSAGES (ST_FROM,ST_MSGID,ST_MSGSIZE) "
    "VALUES (:ST_FROM,:ST_MSGID,:ST_MSGSIZE);"
  ));
  stMsgsSel_->text(utf8::string(
    "SELECT * FROM INET_SENDMAIL_MESSAGES WHERE ST_MSGID = :ST_MSGID"
  ));
  stMsgsDel_->text(utf8::string(
    "DELETE FROM INET_SENDMAIL_MESSAGES"
  ));
  stMsgsDel2_->text(utf8::string(
    "DELETE FROM INET_SENDMAIL_MESSAGES WHERE ST_MSGID = :ST_MSGID"
  ));
  stMsgsSelCount_->text(utf8::string(
    "SELECT COUNT(*) FROM INET_SENDMAIL_MESSAGES"
  ));

  database_->create();

  if( dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL ){
    const ksys::ConfigSection & section =
      config_.section(utf8::string("libadicpp")).
        section(utf8::string("default_connection")).
	        section(utf8::string("firebird"));
    utf8::String hostName, dbName;
    uintptr_t port;
    database_->separateDBName(database_->name(),hostName,dbName,port);
    fbcpp::Service service;
    service.params().
      add(utf8::string("user_name"),section.value(utf8::string("user"))).
      add(utf8::string("password"),section.value(utf8::string("password")));
    service.attach(hostName + ":service_mgr");
    service.request().clear().
      add(utf8::string("action_svc_properties")).add(utf8::string("dbname"),dbName).
      add(utf8::string("prp_reserve_space"),isc_spb_prp_res_use_full);
    service.invoke();
    service.request().clear().
      add(utf8::string("action_svc_properties")).add(utf8::string("dbname"),dbName).
      add(utf8::string("prp_set_sql_dialect"),3);
    service.invoke();
//      service.request().clear().add("action_svc_properties").add("dbname","macroscope").
//        add("prp_page_buffers",8 * 1024 * 1024 / database.params().pageSize());
    service.invoke();
    service.request().clear().
      add(utf8::string("action_svc_properties")).add(utf8::string("dbname"),dbName).
      add(utf8::string("prp_write_mode"),isc_spb_prp_wm_async);
    service.invoke();
    service.request().clear().
      add(utf8::string("action_svc_properties")).add(utf8::string("dbname"),dbName).
      add(utf8::string("prp_sweep_interval"),10000);
    service.invoke();
  }
  ksys::Vector<utf8::String> metadata;
  if( dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL )
    metadata << utf8::string("CREATE DOMAIN DATETIME AS TIMESTAMP");
  metadata <<
    utf8::string(
      "CREATE TABLE INET_USERS_TRAF ("
      " ST_USER               VARCHAR(80) NOT NULL,"
      " ST_TIMESTAMP          DATETIME NOT NULL,"
      " ST_TRAF_WWW           INTEGER NOT NULL,"
      " ST_TRAF_SMTP          INTEGER NOT NULL"
      ")"
    ) <<
    utf8::string(
      "CREATE TABLE INET_USERS_MONTHLY_TOP_URL ("
      " ST_USER               VARCHAR(80) NOT NULL,"
      " ST_TIMESTAMP          DATETIME NOT NULL,"
      " ST_URL                VARCHAR(4096) NOT NULL,"
      " ST_URL_HASH           BIGINT NOT NULL,"
      " ST_URL_TRAF           INTEGER NOT NULL"
      ")"
    ) <<
    utf8::string(
      "CREATE TABLE INET_SENDMAIL_MESSAGES ("
      " ST_FROM               VARCHAR(240) NOT NULL,"
      " ST_MSGID              VARCHAR(14) NOT NULL PRIMARY KEY,"
      " ST_MSGSIZE            INTEGER NOT NULL"
      ")"
    ) <<
    utf8::string(
      "CREATE TABLE INET_LOG_FILE_STAT ("
      " ST_LOG_FILE_NAME      VARCHAR(4096) NOT NULL,"
      " ST_LAST_OFFSET        BIGINT NOT NULL"
      ")"
    ) <<
    utf8::string(
      "CREATE TABLE INET_STAT ("
      " st_bpft_start         DATETIME NOT NULL,"
      " st_bpft_stop          DATETIME NOT NULL,"
      " st_bpft_in_ip         VARCHAR(26) NOT NULL,"
      " st_bpft_out_ip        VARCHAR(26) NOT NULL,"
      " st_bpft_ip_proto      SMALLINT NOT NULL,"
      " st_bpft_s_port        SMALLINT NOT NULL,"
      " st_bpft_d_port        SMALLINT NOT NULL,"
      " st_bpft_bytes         SMALLINT NOT NULL,"
      " st_bpft_data_bytes    SMALLINT NOT NULL"
      ")"
    ) <<
    utf8::string(
      "CREATE UNIQUE INDEX INET_USERS_TRAF_IDX1 ON INET_USERS_TRAF (ST_USER,ST_TIMESTAMP)"
    ) <<
    utf8::string(
      "CREATE INDEX INET_USERS_MONTHLY_TOP_URL_IDX1 ON INET_USERS_MONTHLY_TOP_URL (ST_USER,ST_TIMESTAMP,ST_URL_HASH)"
    )
  ;
  if( dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL ){
    metadata << utf8::string(
      "CREATE DESC INDEX INET_USERS_TRAF_IDX2 ON INET_USERS_TRAF (ST_TIMESTAMP)"
    );
  }
  else if( dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL ){
    metadata << utf8::string(
      "CREATE INDEX INET_USERS_TRAF_IDX2 ON INET_USERS_TRAF (ST_TIMESTAMP)"
    );
  }

  if( (bool) config_.section(utf8::string("macroscope")).value(utf8::string("DROP_DATABASE"),false) ){
    database_->attach();
    database_->drop();
    database_->create();
  }
  database_->attach();

  for( uintptr_t i = 0; i < metadata.count(); i++ ){
    if( dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL )
      if( metadata[i].strncasecmp(utf8::string("CREATE TABLE"),12) == 0 )
        metadata[i] += utf8::string("TYPE = ") +
          config_.section(utf8::string("macroscope")).
	    text(utf8::string("mysql_table_type"),utf8::string("INNODB"));
    try {
      statement_->execute(metadata[i]);
    }
    catch( ksys::ExceptionSP & e ){
      if( !e->searchCode(
            isc_no_meta_update,
            isc_random,
            ER_TABLE_EXISTS_ERROR,
            ER_DUP_KEYNAME) ) throw;
    }
  }

  utf8::String prefix(utf8::string("macroscope."
#if defined(__WIN32__) || defined(__WIN64__)
    "windows."
#else
    "unix."
#endif
  ));

  if( (bool) config_.valueByPath(utf8::string("macroscope.process_squid_log"),true) ){
    parseSquidLogFile(
      ksys::unScreenString(config_.valueByPath(prefix + "squid.log_file_name")),
      config_.valueByPath(utf8::string("macroscope.top10_url"),true),
      config_.valueByPath(utf8::string("macroscope.skip_url"))
    );
  }
  if( (bool) config_.valueByPath(utf8::string("macroscope.process_sendmail_log"),true) ){
    parseSendmailLogFile(
      ksys::unScreenString(config_.valueByPath(prefix + "sendmail.log_file_name")),
      utf8::string("@") + config_.valueByPath(utf8::string("macroscope.sendmail.main_domain")),
      config_.valueByPath(utf8::string("macroscope.sendmail.start_year"))
    );
  }
  writeHtmlYearOutput();
}
//------------------------------------------------------------------------------
} // namespace macroscope
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  int errcode = -1;
  adicpp::initialize();
  try {
    uintptr_t i;
    ksys::initializeArguments(argc,argv);
    ksys::Config::defaultFileName(ksys::getExecutablePath() +
#if !defined(__WIN32__) && !defined(__WIN64__) && !defined(_DEBUG) && !defined(DEBUG) && defined(NDEBUG)
      "../etc/"
#endif
      "macroscope.conf"
    );
    for( i = 1; i < ksys::argv().count(); i++ ){
      if( ksys::argv()[i].strcmp(utf8::string("-c")) == 0 && i + 1 < ksys::argv().count() ){
         ksys::Config::defaultFileName(ksys::argv()[i + 1]);
      }
    }
    macroscope::Logger logger;
    logger.main();
  }
  catch( ksys::ExceptionSP & e ){
    e->writeStdError();
    errcode = e->code();
  }
  catch( ... ){
  }
  adicpp::cleanup();
  return errcode;
}
//------------------------------------------------------------------------------
