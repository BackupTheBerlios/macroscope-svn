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
//------------------------------------------------------------------------------
namespace macroscope {
//------------------------------------------------------------------------------
static const char * const trafTypeColumnName[]  = {
  "ST_TRAF_SMTP",
  "ST_TRAF_WWW",
  "ST_TRAF"
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Logger::SquidSendmailThread::~SquidSendmailThread()
{
}
//------------------------------------------------------------------------------
Logger::SquidSendmailThread::SquidSendmailThread(Logger & logger,const utf8::String & section,const utf8::String & sectionName) :
  logger_(&logger), section_(section), sectionName_(sectionName)
{
  shortUrl_ = "://";
  
  ConfigSection dbParamsSection;
  dbParamsSection.addSection(logger_->config_->sectionByPath("libadicpp.default_connection"));
      
  database_ = Database::newDatabase(&dbParamsSection);
	
  statement_ = database_->newAttachedStatement();
  stTrafIns_ = database_->newAttachedStatement();
  stTrafUpd_ = database_->newAttachedStatement();
  stMonUrlSel_ = database_->newAttachedStatement();
  stMonUrlIns_ = database_->newAttachedStatement();
  stMonUrlUpd_ = database_->newAttachedStatement();
  stFileStat_[stSel] = database_->newAttachedStatement();
  stFileStat_[stIns] = database_->newAttachedStatement();
  stFileStat_[stUpd] = database_->newAttachedStatement();
  stMsgsIns_ = database_->newAttachedStatement();
  stMsgsSel_ = database_->newAttachedStatement();
  stMsgsDel_ = database_->newAttachedStatement();
  stMsgsDel2_ = database_->newAttachedStatement();
  stMsgsSelCount_ = database_->newAttachedStatement();
			     
  stFileStat_[stSel] = database_->newAttachedStatement();
  stFileStat_[stIns] = database_->newAttachedStatement();
  stFileStat_[stUpd] = database_->newAttachedStatement();
}
//------------------------------------------------------------------------------
void Logger::SquidSendmailThread::threadExecute()
{
  AutoDatabaseDetach autoDatabaseDetach(database_);
  section_ = "macroscope";
  if( (bool) logger_->config_->valueByPath(section_ + ".process_squid_log",true) ){
    Mutant m0(logger_->config_->valueByPath(section_ + ".squid.log_file_name"));
    Mutant m1(logger_->config_->valueByPath(section_ + ".squid.top10_url",true));
    Mutant m2(logger_->config_->textByPath(section_ + ".squid.skip_url").lower());
    parseSquidLogFile(m0,m1,m2);
  }
  if( (bool) logger_->config_->valueByPath(section_ + ".process_sendmail_log",true) ){
    Mutant m0(logger_->config_->valueByPath(section_ + ".sendmail.log_file_name"));
    Mutant m1(utf8::String("@") + logger_->config_->valueByPath(section_ + ".sendmail.main_domain"));
    Mutant m2(logger_->config_->valueByPath(section_ + ".sendmail.start_year"));
    parseSendmailLogFile(m0,m1,m2);
  }
  groups_ = perGroupReport_ = false;
  ellapsed_ = getlocaltimeofday();
  writeHtmlYearOutput();
  perGroupReport_ = true;
  for( uintptr_t i = 0; i < logger_->config_->sectionByPath(section_ + ".html_report.groups_report_directories").valueCount(); i++ ){
    perGroupReportDir_ = logger_->config_->sectionByPath(section_ + ".html_report.groups_report_directories").text(i,&perGroupReportName_);
    ellapsed_ = getlocaltimeofday();
    writeHtmlYearOutput();
  }
}
//------------------------------------------------------------------------------
void Logger::SquidSendmailThread::decoration()
{
  static const char * const nicks[] = { ".smtp", ".www", ".all" };
  struct deco {
    const char * path;
    utf8::String * colors;
  } decos[] = {
    { "heads", trafTypeNick_ },
    { "colors.head", trafTypeHeadColor_ },
    { "colors.body",  trafTypeBodyColor_ },
    { "colors.tail",  trafTypeTailColor_ },
    { "colors.details.head", trafTypeHeadDataColor_ },
    { "colors.details.body", trafTypeBodyDataColor_ },
    { "colors.details.tail", trafTypeTailDataColor_ }
  };
  utf8::String basePath(section_ + ".html_report.decoration.");
  for( intptr_t i = sizeof(decos) / sizeof(decos[0]) - 1; i >= 0; i-- )
    for( intptr_t j = ttAll; j >= 0; j-- ){
      decos[i].colors[j] = logger_->config_->valueByPath(basePath + decos[i].path + nicks[j]);
    }
}
//------------------------------------------------------------------------------
void Logger::SquidSendmailThread::writeUserTop(
  const utf8::String & file,
  const utf8::String & user,
  uintptr_t isGroup,
  const struct tm & beginTime,
  const struct tm & endTime)
{
  if( !isGroup && !(bool) logger_->config_->valueByPath(section_ + ".html_report.top10_url",true) ) return;
  if( isGroup && !(bool) logger_->config_->valueByPath(section_ + ".html_report.group_top10_url",false) ) return;
  utf8::String users(genUserFilter(user,isGroup));
  statement_->text(
    "SELECT"
    "    A.*"
    "FROM"
    "    ("
    "        SELECT"
    "          ST_URL, SUM(ST_URL_TRAF) AS SUM1, SUM(ST_URL_COUNT) AS SUM2"
    "        FROM"
    "            INET_USERS_MONTHLY_TOP_URL"
    "        WHERE " + users +
    "                ST_TIMESTAMP >= :BT AND"
    "                ST_TIMESTAMP <= :ET"
    "        GROUP BY ST_URL"
    "    ) AS A "
    "WHERE"
    "  A.SUM1 >= :threshold "
    "ORDER BY A.SUM1"
  );
  intptr_t i;
  statement_->prepare();
  for( i = enumStringParts(user) - 1; i >= 0; i-- )
    statement_->paramAsString("U" + utf8::int2Str(i),stringPartByNo(user,i));
  uintmax_t threshold = logger_->config_->valueByPath(section_ + ".html_report.top10_min_significant_threshold",0);
  statement_->
    paramAsMutant("BT",time2tm(tm2Time(beginTime) - getgmtoffset()))->
    paramAsMutant("ET",time2tm(tm2Time(endTime) - getgmtoffset()))->
    paramAsMutant("threshold",threshold)->
    execute()->fetchAll();
  if( statement_->rowCount() > 0 ){
    AsyncFile f(file);
    f.createIfNotExist(true).open().resize(0);
    Mutant m0(logger_->config_->valueByPath(section_ + ".html_report.file_mode",0644));
    Mutant m1(logger_->config_->valueByPath(section_ + ".html_report.file_user",ksys::getuid()));
    Mutant m2(logger_->config_->valueByPath(section_ + ".html_report.file_group",ksys::getgid()));
    chModOwn(f.fileName(),m0,m1,m2);
    writeHtmlHead(f);
    f <<
      "<TABLE WIDTH=100 BORDER=1 CELLSPACING=0 CELLPADDING=2>\n"
      "<TR>\n"
      "  <TH ALIGN=center BGCOLOR=\"" + utf8::String(trafTypeHeadDataColor_[ttAll]) + "\" wrap>\n"
      "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
      "URL\n"
      "    </FONT>\n"
      "  </TH>\n"
      "  <TH ALIGN=center BGCOLOR=\"" + trafTypeHeadDataColor_[ttSMTP] + "\" nowrap>\n"
      "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
      "HIT\n"
      "    </FONT>\n"
      "  </TH>\n"
      "  <TH ALIGN=center BGCOLOR=\"" + trafTypeHeadDataColor_[ttSMTP] + "\" nowrap>\n"
      "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
      "KB/HIT\n"
      "    </FONT>\n"
      "  </TH>\n"
      "  <TH ALIGN=center BGCOLOR=\"" + trafTypeHeadDataColor_[ttWWW] + "\" nowrap>\n"
      "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
      "KB\n"
      "    </FONT>\n"
      "  </TH>\n"
      "</TR>\n"
    ;
    uint64_t at = 0;
    for( i = statement_->rowCount() - 1; i >= 0; i-- ){
      statement_->selectRow(i);
      at += (uint64_t) statement_->valueAsMutant(1);
    }
    for( i = statement_->rowCount() - 1; i >= 0; i-- ){
      statement_->selectRow(i);
      f <<
        "<TR>\n"
        "  <TH WITH=10 ALIGN=left BGCOLOR=\"" + utf8::String(trafTypeBodyDataColor_[ttAll]) + "\" wrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\" wrap>\n" +
        "      <A HREF=\"" + statement_->valueAsMutant(0) + "\" wrap>\n" +
        statement_->valueAsMutant(0) + "\n"
        "      </A>\n"
        "    </FONT>\n"
        "  </TH>\n"
        "  <TH ALIGN=right BGCOLOR=\"" + trafTypeBodyDataColor_[ttSMTP] + "\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" +
        statement_->valueAsMutant(2) + "\n"
        "    </FONT>\n"
        "  </TH>\n"
        "  <TH ALIGN=right BGCOLOR=\"" + trafTypeBodyDataColor_[ttSMTP] + "\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" +
        utf8::int2Str(
          (
           (uint64_t) statement_->valueAsMutant(1) / 
            (
              (uint64_t) statement_->valueAsMutant(2) > 0 ? 
              (uint64_t) statement_->valueAsMutant(2) : 1u
            )
          ) / 1024u
        ) + "\n"
        "    </FONT>\n"
        "  </TH>\n"
        "  <TH ALIGN=right BGCOLOR=\"" + utf8::String(trafTypeBodyDataColor_[ttWWW]) + "\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" +
        formatTraf(statement_->valueAsMutant(1),at) +
        "\n"
        "    </FONT>\n"
        "  </TH>\n"
        "</TR>\n"
      ;
    }
    f << "</TABLE>\n<BR>\n<BR>\n";
    writeHtmlTail(f,ellapsed_);
    f.resize(f.tell());
    if( logger_->verbose_ ) fprintf(stderr,"%s\n",(const char *) getNameFromPathName(f.fileName()).getOEMString());
  }
}
//------------------------------------------------------------------------------
void Logger::SquidSendmailThread::writeMonthHtmlOutput(const utf8::String & file,const struct tm & year,bool threaded)
{
  AsyncFile f(file);
  f.createIfNotExist(true).open().resize(0);
  Mutant m0(logger_->config_->valueByPath(section_ + ".html_report.file_mode",0644));
  Mutant m1(logger_->config_->valueByPath(section_ + ".html_report.file_user",ksys::getuid()));
  Mutant m2(logger_->config_->valueByPath(section_ + ".html_report.file_group",ksys::getgid()));
  chModOwn(file,m0,m1,m2);
  writeHtmlHead(f);
  struct tm beginTime = year, endTime = year;
  beginTime.tm_mon = 0;
  beginTime.tm_mday = 1;
  beginTime.tm_hour = 0;
  beginTime.tm_min = 0;
  beginTime.tm_sec = 0;
  endTime.tm_mon = 11;
  endTime.tm_mday = 31;
  endTime.tm_hour = 23;
  endTime.tm_min = 59;
  endTime.tm_sec = 59;
  struct tm beginTime2, bt, et;
  while( endTime.tm_mon >= 0 ){
    endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900, endTime.tm_mon);
    beginTime2 = beginTime;
    beginTime.tm_mon = endTime.tm_mon;
    if( getTraf(ttAll,beginTime,endTime) > 0 ){
      if( logger_->verbose_ ) fprintf(stderr,"%s %s\n",
        (const char *) utf8::tm2Str(beginTime).getOEMString(),
        (const char *) utf8::tm2Str(endTime).getOEMString()
      );
      utf8::String trafByMonthFile(utf8::String::print("users-traf-by-%04d%02d.html",endTime.tm_year + 1900,endTime.tm_mon + 1));
      f <<
        "<TABLE WIDTH=400 BORDER=1 CELLSPACING=0 CELLPADDING=2>\n"
        "<TR>\n"
        "  <TH BGCOLOR=\"" +
        logger_->getDecor("table_head",section_) +
        "\" COLSPAN=" +
        utf8::int2Str(uintmax_t(nonZeroMonthDaysColumns(endTime) + ttAll + 2)) +
        " ALIGN=left nowrap>\n" +
        "<A HREF=\"" + trafByMonthFile + "\">" +
        utf8::int2Str(uintmax_t(endTime.tm_mon + 1)) + "\n"
        "</A>\n"
        "  </TH>\n"
        "</TR>\n"
        "<TR>\n"
        "  <TH HEIGHT=4></TH>\n"
        "</TR>\n"
        "<TR>\n"
        "  <TH ROWSPAN=2 ALIGN=center BGCOLOR=\"" + logger_->getDecor("head.user",section_) + "\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
        "User\n"
        "    </FONT>\n"
        "  </TH>\n"
      ;
      intptr_t i, j, k;
      // ѕечатаем заголовки суммарных трафиков пользовател€ за мес€ц
      for( i = ttAll; i >= 0; i-- ){
        f <<
	  "  <TH ROWSPAN=2 ALIGN=center BGCOLOR=\"" +
          utf8::String(trafTypeHeadColor_[i]) + "\" wrap>\n" +
          "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" + trafTypeNick_[i] + "\n"
          "    </FONT>\n"
	  "  </TH>\n"
        ;
      }
      // ѕечатаем заголовки только тех дней мес€ца в которых был не нулевой трафик
      while( endTime.tm_mday > 0 ){
        bt = endTime;
        bt.tm_hour = 0;
        bt.tm_min = 0;
        bt.tm_sec = 0;
        if( getTraf(ttAll, bt, endTime) > 0 ){
          f <<
	    "  <TH ALIGN=center COLSPAN=" +
            utf8::int2Str((getTraf(ttAll, bt, endTime) > 0) +
            (getTraf(ttWWW, bt, endTime) > 0) +
            (getTraf(ttSMTP, bt, endTime) > 0)) + " BGCOLOR=\"" +
            logger_->getDecor("detail_head",section_) +
            "\" nowrap>\n"
	    "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" +
            utf8::String::print("%02d", endTime.tm_mday) + "\n"
            "    </FONT>\n"
	    "  </TH>\n"
          ;
        }
        endTime.tm_mday--;
      }
      f << "</TR>\n<TR>\n";
      endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900, endTime.tm_mon);
      // ѕечатаем заголовки трафиков пользовател€ за мес€цы
      while( endTime.tm_mday > 0 ){
        bt = endTime;
        bt.tm_hour = 0;
        bt.tm_min = 0;
        bt.tm_sec = 0;
        if( getTraf(ttAll, bt, endTime) > 0 ){
          for( i = ttAll; i >= 0; i-- ){
            if( getTraf(TrafType(i), bt, endTime) == 0 ) continue;
            f <<
	      "  <TH ALIGN=center BGCOLOR=\"" +
	      utf8::String(trafTypeHeadDataColor_[i]) + "\" wrap>\n" +
              "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" + trafTypeNick_[i] + "\n"
              "    </FONT>\n"
	      "  </TH>\n"
            ;
          }
        }
        endTime.tm_mday--;
      }
      f << "</TR>\n";
      endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900, endTime.tm_mon);
      // ѕечатаем трафик пользователей
      Vector<Table<Mutant> > usersTrafTables;
      genUsersTable(usersTrafTables,beginTime,endTime);
      for( k = usersTrafTables.count() - 1; k >= 0; k-- ){
        Table<Mutant> & usersTrafTable = usersTrafTables[k];
        for( i = usersTrafTable.rowCount() - 1; i >= 0; i-- ){
	  bool isGroup = usersTrafTable(i,"ST_IS_GROUP");
          if( getTraf(ttAll,beginTime,endTime,usersTrafTable(i,"ST_USER"),usersTrafTable(i,"ST_IS_GROUP")) == 0 ) continue;
          utf8::String user(usersTrafTable(i,isGroup ? "ST_GROUP" : "ST_USER"));
	  utf8::String alias(logger_->config_->textByPath(section_ + ".aliases." + user,user));
          utf8::String topByUserFile(
            utf8::String::print(
              "top-%04d%02d-",
              endTime.tm_year + 1900,
              endTime.tm_mon + 1
            ) + user.replaceAll(":","-").replaceAll(" ","") + ".html"
          );
          if( !(bool) logger_->config_->valueByPath(section_ + ".html_report.refresh_only_current",true) || (curTime_.tm_year == endTime.tm_year &&    	curTime_.tm_mon == endTime.tm_mon) ){
            writeUserTop(
              includeTrailingPathDelimiter(htmlDir_) + topByUserFile,
              user,
  	      usersTrafTable(i,"ST_IS_GROUP"),
              beginTime,
              endTime
            );
	  }
          f <<
            "<TR>\n"
            "  <TH ALIGN=left BGCOLOR=\"" + logger_->getDecor("body.user",section_) + "\" nowrap>\n"
            "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
            "<A HREF=\"" + topByUserFile + "\">" +
            alias +
	    (alias.strcasecmp(user) == 0 ? utf8::String() : " (" + user + ")") + "\n"
            "</A>\n"
            "    </FONT>\n"
            "  </TH>\n"
          ;
          for( j = ttAll; j >= 0; j-- ){
            f <<
	      "  <TH ALIGN=right BGCOLOR=\"" + utf8::String(trafTypeBodyColor_[j]) + "\" nowrap>\n"
	      "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" +
              formatTraf(usersTrafTable(i,trafTypeColumnName[j]),getTraf(ttAll,beginTime,endTime)) +
	      "    </FONT>\n"
	      "  </TH>\n"
            ;
          }
        // ѕечатаем трафик пользователей по дн€м
          while( endTime.tm_mday > 0 ){
            bt = endTime;
            bt.tm_hour = 0;
            bt.tm_min = 0;
            bt.tm_sec = 0;
            if( getTraf(ttAll, bt, endTime) > 0 ){
              for( j = ttAll; j >= 0; j-- ){
                if( getTraf(TrafType(j), bt, endTime) == 0 ) continue;
                f <<
                  "  <TH ALIGN=right BGCOLOR=\"" + utf8::String(trafTypeBodyDataColor_[j]) + "\" nowrap>\n"
                  "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" +
                  formatTraf(
                    getTraf(
		      TrafType(j),
                      bt,
                      endTime,
                      usersTrafTable(i,"ST_USER"),
		      usersTrafTable(i,"ST_IS_GROUP")
                    ),
                    getTraf(ttAll,bt,endTime)
                  ) +
		  "    </FONT>\n"
		  "  </TH>\n"
                ;
              }
            }
            endTime.tm_mday--;
          }
          f << "</TR>\n";
          endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900, endTime.tm_mon);
        }
      }
      // ѕечатаем итоговый трафик пользователей за мес€ц
      f <<
        "<TR>\n"
        "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("tail.user",section_) + "\" wrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
        "Summary traffic of all users:"
      ;
      f <<
        "\n"
        "    </FONT>\n"
        "  </TH>\n"
      ;
      for( j = ttAll; j >= 0; j-- ){
        f <<
          "  <TH ALIGN=right BGCOLOR=\"" + utf8::String(trafTypeTailColor_[j]) + "\" nowrap>\n"
          "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" +
	  formatTraf(getTraf(TrafType(j),beginTime,endTime),getTraf(ttAll,beginTime,endTime)) +
	  "    </FONT>\n"
	  "  </TH>\n"
        ;
      }
      // ѕечатаем итоговый трафик пользователей за дни
      et = endTime;
      while( et.tm_mday > 0 ){
        bt = et;
        bt.tm_hour = 0;
        bt.tm_min = 0;
        bt.tm_sec = 0;
        if( getTraf(ttAll,bt,et) > 0 ){
          for( j = ttAll; j >= 0; j-- ){
            if( getTraf(TrafType(j), bt, et) == 0 ) continue;
            f << 
              "  <TH ALIGN=right BGCOLOR=\"" + utf8::String(trafTypeTailDataColor_[j]) + "\" nowrap>\n" 
              "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" +
              (j == ttAll ? formatTraf(getTraf(TrafType(j),bt,et),getTraf(ttAll,beginTime,endTime)) :
                            formatTraf(getTraf(TrafType(j),bt,et),getTraf(ttAll,bt,et))
	      ) +
	      "    </FONT>\n"
	      "  </TH>\n"
            ;
          }
        }
        et.tm_mday--;
      }
      f << "</TR>\n</TABLE>\n<BR>\n<BR>\n";
    }
    endTime.tm_mon--;
    beginTime = beginTime2;
  }
  writeHtmlTail(f,ellapsed_);
  f.resize(f.tell());
  if( logger_->verbose_ ) fprintf(stderr,"%s\n",(const char *) getNameFromPathName(f.fileName()).getOEMString());
}
//------------------------------------------------------------------------------
intptr_t Logger::SquidSendmailThread::sortUsersTrafTable(uintptr_t row1,uintptr_t row2,const Table<Mutant> & table)
{
  intptr_t g1 = table(row1,"ST_IS_GROUP"), g2 = table(row2,"ST_IS_GROUP");
  intptr_t c = g1 > g2 ? 1 : g1 < g2 ? -1 : 0;
  if( c == 0 ){
    int64_t t1 = table(row1,"ST_TRAF"), t2 = table(row2,"ST_TRAF");
    c = t1 > t2 ? 1 : t1 < t2 ? -1 : 0;
    if( c == 0 )
      c = utf8::String(table(row1,"ST_USER")).strcasecmp(table(row2,"ST_USER"));
  }
  return c;
}
//------------------------------------------------------------------------------
intptr_t Logger::SquidSendmailThread::sortUsersTrafTables(Table<Mutant> * & p1,Table<Mutant> * & p2)
{
  intmax_t s1(p1->sum("ST_TRAF")), s2(p2->sum("ST_TRAF"));
  return s1 > s2 ? 1 : s1 < s2 ? -1 : 0;
}
//------------------------------------------------------------------------------
void Logger::SquidSendmailThread::genUsersTable(Vector<Table<Mutant> > & usersTrafTables,const struct tm & beginTime,const struct tm & endTime)
{
  intptr_t i, k, u;
  if( groups_ ){
    utf8::String groupedUsers;
    usersTrafTables.resize(gCount_);
    for( k = usersTrafTables.count() - 1; k >= 0; k-- ){
      utf8::String key, value(logger_->config_->sectionByPath(section_ + ".groups").value(k,&key));
      if( perGroupReport_ )
        value = logger_->config_->sectionByPath(section_ + ".groups").value(key = perGroupReportName_);
      statement_->text(
        "SELECT DISTINCT ST_USER FROM INET_USERS_TRAF WHERE " +
	genUserFilter(value,1) +
        " ST_TIMESTAMP >= :BT AND ST_TIMESTAMP <= :ET"
      );
      statement_->prepare();
      for( u = enumStringParts(value) - 1; u >= 0; u-- )
        statement_->paramAsString(u,stringPartByNo(value,u));
      statement_->
        paramAsMutant("BT",time2tm(tm2Time(beginTime) - getgmtoffset()))->
        paramAsMutant("ET",time2tm(tm2Time(endTime) - getgmtoffset()))->
        execute()->fetchAll()->
        unload(usersTrafTables[k]);
      usersTrafTables[k].
        addColumn("ST_TRAF").
        addColumn("ST_TRAF_WWW").
        addColumn("ST_TRAF_SMTP").
        addColumn("ST_GROUP").
        addColumn("ST_IS_GROUP");
      uintptr_t j = usersTrafTables[k].rowCount();
      if( j == 0 ){
        usersTrafTables.remove(k);
        continue;
      }
      if( !perGroupReport_ ){
        usersTrafTables[k].addRow()(j,"ST_IS_GROUP") = 1;
        usersTrafTables[k](j,"ST_USER") = value;
        usersTrafTables[k](j,"ST_TRAF") = 0;
        usersTrafTables[k](j,"ST_TRAF_WWW") = 0;
        usersTrafTables[k](j,"ST_GROUP") = "group: " + key;
        usersTrafTables[k](j,"ST_TRAF_SMTP") = 0;
      }
      if( groupedUsers.strlen() > 0 ) groupedUsers += ",";
      groupedUsers += value;
    }
    if( !perGroupReport_ ){
      statement_->text(
        "SELECT DISTINCT ST_USER FROM INET_USERS_TRAF WHERE " +
        genUserFilter(groupedUsers,2) +
        " ST_TIMESTAMP >= :BT AND ST_TIMESTAMP <= :ET"
      );
      statement_->prepare();
      for( u = enumStringParts(groupedUsers) - 1; u >= 0; u-- )
        statement_->paramAsString("U" + utf8::int2Str(u),stringPartByNo(groupedUsers,u));
      usersTrafTables.resize(usersTrafTables.count() + 1);
      k = usersTrafTables.count() - 1;
      statement_->
        paramAsMutant("BT",time2tm(tm2Time(beginTime) - getgmtoffset()))->
        paramAsMutant("ET",time2tm(tm2Time(endTime) - getgmtoffset()))->
        execute()->fetchAll()->
        unload(usersTrafTables[k]);
      usersTrafTables[k].
        addColumn("ST_TRAF").
        addColumn("ST_TRAF_WWW").
        addColumn("ST_TRAF_SMTP").
        addColumn("ST_GROUP").
        addColumn("ST_IS_GROUP");
      uintptr_t j = usersTrafTables[k].rowCount();
      if( j == 0 ){
        usersTrafTables.remove(k);
      }
      else {
        usersTrafTables[k].addRow()(j,"ST_IS_GROUP") = 2;
        usersTrafTables[k](j,"ST_USER") = groupedUsers;
        usersTrafTables[k](j,"ST_TRAF") = 0;
        usersTrafTables[k](j,"ST_TRAF_WWW") = 0;
        usersTrafTables[k](j,"ST_GROUP") = "group: ungrouped";
        usersTrafTables[k](j,"ST_TRAF_SMTP") = 0;
      }
    }
  }
  else {
    usersTrafTables.resize(1);
    Table<Mutant> & usersTrafTable = usersTrafTables[0];
    statement_->text(
      "SELECT DISTINCT ST_USER FROM INET_USERS_TRAF WHERE "
      "ST_TIMESTAMP >= :BT AND ST_TIMESTAMP <= :ET"
    );
    statement_->prepare()->
      paramAsMutant("BT",time2tm(tm2Time(beginTime) - getgmtoffset()))->
      paramAsMutant("ET",time2tm(tm2Time(endTime) - getgmtoffset()))->
      execute()->fetchAll()->unload(usersTrafTable);
    usersTrafTable.
      addColumn("ST_TRAF").
      addColumn("ST_TRAF_WWW").
      addColumn("ST_TRAF_SMTP").
      addColumn("ST_GROUP").
      addColumn("ST_IS_GROUP");
  }
  for( k = usersTrafTables.count() - 1; k >= 0; k-- ){
    Table<Mutant> & usersTrafTable = usersTrafTables[k];
    i = usersTrafTable.rowCount();
    if( i == 0 ) continue;
    i--;
    if( (bool) usersTrafTable(i,"ST_IS_GROUP") ){
      utf8::String users(usersTrafTable(i,"ST_USER"));
      usersTrafTable(i,"ST_TRAF") = int64_t(usersTrafTable(i,"ST_TRAF_WWW") = 
        getTraf(ttWWW,beginTime,endTime,users,usersTrafTable(i,"ST_IS_GROUP"))) + 
        int64_t(usersTrafTable(i,"ST_TRAF_SMTP") =
          getTraf(ttSMTP,beginTime,endTime,users,usersTrafTable(i,"ST_IS_GROUP")));
    }
  }
  qSort(usersTrafTables.ptr(),0,usersTrafTables.count() - 1,sortUsersTrafTables);
  for( k = usersTrafTables.count() - 1; k >= 0; k-- ){
    Table<Mutant> & usersTrafTable = usersTrafTables[k];
    for( i = usersTrafTable.rowCount() - 1; i >= 0; i-- ){
      if( (bool) usersTrafTable(i,"ST_IS_GROUP") ) continue;
      utf8::String user(usersTrafTable(i,"ST_USER"));
        usersTrafTable(i,"ST_TRAF") = int64_t(usersTrafTable(i,"ST_TRAF_WWW") = 
          getTraf(ttWWW,beginTime,endTime,user)) + 
          int64_t(usersTrafTable(i,"ST_TRAF_SMTP") = 
          getTraf(ttSMTP,beginTime,endTime,user));
    }
    usersTrafTable.sort(sortUsersTrafTable,usersTrafTable);
  }
}
//------------------------------------------------------------------------------
void Logger::SquidSendmailThread::writeHtmlYearOutput()
{
  if( !(bool) logger_->config_->valueByPath(section_ + ".html_report.enabled",true) ) return;
  if( logger_->verbose_ ) fprintf(stderr,"\n");
  decoration();
  struct tm beginTime, endTime;
  curTime_ = time2tm(getlocaltimeofday());
  database_->start();
  statement_->text("SELECT ");
  if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL )
    statement_->text(statement_->text() + "FIRST 1 ");
  statement_->text(statement_->text() + "ST_TIMESTAMP FROM INET_USERS_TRAF ORDER BY ST_TIMESTAMP");
  if( dynamic_cast<MYSQLDatabase *>(statement_->database()) != NULL )
    statement_->text(statement_->text() + " LIMIT 0,1");
  statement_->execute()->fetchAll();
  if( statement_->rowCount() > 0 ){
    beginTime = time2tm((uint64_t) statement_->valueAsMutant("ST_TIMESTAMP") + getgmtoffset());
    statement_->text("SELECT ");
    if( dynamic_cast<FirebirdDatabase *>(statement_->database()) != NULL )
      statement_->text(statement_->text() + "FIRST 1 ");
    statement_->text(statement_->text() + "ST_TIMESTAMP FROM INET_USERS_TRAF ORDER BY ST_TIMESTAMP DESC");
    if( dynamic_cast<MYSQLDatabase *>(statement_->database()) != NULL )
      statement_->text(statement_->text() + " LIMIT 0,1");
    statement_->execute()->fetchAll();
    endTime = time2tm((uint64_t) statement_->valueAsMutant("ST_TIMESTAMP") + getgmtoffset());
    htmlDir_ = excludeTrailingPathDelimiter(logger_->config_->valueByPath(section_ + ".html_report.directory"));
    if( perGroupReport_ ) htmlDir_ = excludeTrailingPathDelimiter(perGroupReportDir_);
    AsyncFile f(
      includeTrailingPathDelimiter(htmlDir_) + logger_->config_->valueByPath(section_ + ".html_report.index_file_name","index.html")
    );
    f.createIfNotExist(true).open().resize(0);
    Mutant m0(logger_->config_->valueByPath(section_ + ".html_report.directory_mode",0755));
    Mutant m1(logger_->config_->valueByPath(section_ + ".html_report.directory_user",ksys::getuid()));
    Mutant m2(logger_->config_->valueByPath(section_ + ".html_report.directory_group",ksys::getgid()));
    chModOwn(htmlDir_,m0,m1,m2);
    m0 = logger_->config_->valueByPath(section_ + ".html_report.file_mode",0644);
    m1 = logger_->config_->valueByPath(section_ + ".html_report.file_user",ksys::getuid());
    m2 = logger_->config_->valueByPath(section_ + ".html_report.file_group",ksys::getgid());
    chModOwn(f.fileName(),m0,m1,m2);
    writeHtmlHead(f);
    beginTime.tm_mon = 0;
    beginTime.tm_mday = 1;
    beginTime.tm_hour = 0;
    beginTime.tm_min = 0;
    beginTime.tm_sec = 0;
    endTime.tm_mon = 11;
    endTime.tm_mday = 31;
    endTime.tm_hour = 23;
    endTime.tm_min = 59;
    endTime.tm_sec = 59;
    struct tm beginTime2, bt, et;
    gCount_ = logger_->config_->sectionByPath(section_ + ".groups").valueCount();
    groups_ = (bool) logger_->config_->valueByPath(section_ + ".html_report.groups",false) && gCount_ > 0;
    if( perGroupReport_ ){
      gCount_ = 1;
      groups_ = true;
    }
    while( tm2Time(endTime) >= tm2Time(beginTime) ){
      beginTime2 = beginTime;
      beginTime.tm_year = endTime.tm_year;
      if( getTraf(ttAll,beginTime,endTime) > 0 ){
        if( logger_->verbose_ ) fprintf(stderr,"%s %s\n",
          (const char *) utf8::tm2Str(beginTime).getOEMString(),
          (const char *) utf8::tm2Str(endTime).getOEMString()
        );
        utf8::String trafByYearFile(utf8::String::print("users-traf-by-%04d.html",endTime.tm_year + 1900));
        f <<
          "<TABLE WIDTH=400 BORDER=1 CELLSPACING=0 CELLPADDING=2>\n"
          "<TR>\n"
          "  <TH BGCOLOR=\"" +
          logger_->getDecor("table_head",section_) +
          "\" COLSPAN=" +
          utf8::int2Str(uintmax_t(nonZeroYearMonthsColumns(endTime) + ttAll + 2)) +
          " ALIGN=left nowrap>\n" +
          "   <A HREF=\"" + trafByYearFile + "\">\n" +
          utf8::int2Str(endTime.tm_year + 1900) + "\n"
          "   </A>\n"
          "  </TH>\n"
          "</TR>\n"
          "<TR>\n"
          "  <TH HEIGHT=4></TH>\n"
          "</TR>\n"
          "<TR>\n"
          "  <TH ROWSPAN=2 ALIGN=center BGCOLOR=\"" + logger_->getDecor("head.user",section_) + "\" nowrap>\n"
          "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
          "User\n"
          "    </FONT>\n" "  </TH>\n"
        ;
        intptr_t i, j, k;
        // ѕечатаем заголовки суммарных трафиков пользовател€ за год
        for( i = ttAll; i >= 0; i-- ){
          f <<
	    "  <TH ROWSPAN=2 ALIGN=center BGCOLOR=\"" + utf8::String(trafTypeHeadColor_[i]) + "\" wrap>\n" +
	    "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" + trafTypeNick_[i] +
	    "\n"
	    "    </FONT>\n"
	    "  </TH>\n"
          ;
        }
        // ѕечатаем заголовки только тех мес€цев в которых был не нулевой трафик
        while( endTime.tm_mon >= 0 ){
          bt = endTime;
          bt.tm_mday = 1;
          bt.tm_hour = 0;
          bt.tm_min = 0;
          bt.tm_sec = 0;
          endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900, endTime.tm_mon);
          if( getTraf(ttAll, bt, endTime) > 0 ){
            f <<
	      "  <TH ALIGN=center COLSPAN=" +
              utf8::int2Str((getTraf(ttAll, bt, endTime) > 0) +
              (getTraf(ttWWW, bt, endTime) > 0) +
              (getTraf(ttSMTP, bt, endTime) > 0)) +
              " BGCOLOR=\"" << logger_->getDecor("detail_head",section_) + "\" nowrap>\n"
	      "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" + utf8::String::print("%02d",endTime.tm_mon + 1) + "\n"
	      "    </FONT>\n"
	      "  </TH>\n"
            ;
          }
          endTime.tm_mon--;
        }
        f << "</TR>\n<TR>\n";
        endTime.tm_mon = 11;
        endTime.tm_mday = 31;
        // ѕечатаем заголовки трафиков пользовател€ за мес€цы
        while( endTime.tm_mon >= 0 ){
          bt = endTime;
          bt.tm_mday = 1;
          bt.tm_hour = 0;
          bt.tm_min = 0;
          bt.tm_sec = 0;
          endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900, endTime.tm_mon);
          if( getTraf(ttAll, bt, endTime) > 0 ){
            for( i = ttAll; i >= 0; i-- ){
              if( getTraf(TrafType(i), bt, endTime) == 0 ) continue;
              f <<
	        "  <TH ALIGN=center BGCOLOR=\"" + utf8::String(trafTypeHeadDataColor_[i]) + "\" wrap>\n" +
	        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" + trafTypeNick_[i] + "\n"
	        "    </FONT>\n"
	        "  </TH>\n"
              ;
            }
          }
          endTime.tm_mon--;
        }
        f << "</TR>\n";
        endTime.tm_mon = 11;
        endTime.tm_mday = 31;
        // ѕечатаем трафик пользователей
        Vector<Table<Mutant> > usersTrafTables;
        genUsersTable(usersTrafTables,beginTime,endTime);
        for( k = usersTrafTables.count() - 1; k >= 0; k-- ){
          Table<Mutant> & usersTrafTable = usersTrafTables[k];
          for( i = usersTrafTable.rowCount() - 1; i >= 0; i-- ){
	    bool isGroup = usersTrafTable(i,"ST_IS_GROUP");
            if( getTraf(ttAll,beginTime,endTime,usersTrafTable(i,"ST_USER"),usersTrafTable(i,"ST_IS_GROUP")) == 0 ) continue;
	    utf8::String user(usersTrafTable(i,isGroup ? "ST_GROUP" : "ST_USER"));
            utf8::String alias(logger_->config_->textByPath(section_ + ".aliases." + user,user));
            f <<
  	      "<TR>\n"
	      "  <TH ALIGN=left BGCOLOR=\"" + logger_->getDecor("body.user",section_) + "\" nowrap>\n"
              "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" +
              alias +
	      (alias.strcasecmp(user) == 0 ? utf8::String() : " (" + user + ")") + "\n"
	      "    </FONT>\n"
	      "  </TH>\n"
            ;
            for( j = ttAll; j >= 0; j-- ){
              f <<
	        "  <TH ALIGN=right BGCOLOR=\"" + utf8::String(trafTypeBodyColor_[j]) + "\" nowrap>\n"
  	        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" +
	        formatTraf(usersTrafTable(i,trafTypeColumnName[j]),getTraf(ttAll,beginTime,endTime)) +
	        "    </FONT>\n"
	        "  </TH>\n"
	      ;
            }
          // ѕечатаем трафик пользователей помес€чно
            while( endTime.tm_mon >= 0 ){
              bt = endTime;
              bt.tm_mday = 1;
              bt.tm_hour = 0;
              bt.tm_min = 0;
              bt.tm_sec = 0;
              endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900, endTime.tm_mon);
              if( getTraf(ttAll,bt,endTime) > 0 ){
                for( j = ttAll; j >= 0; j-- ){
                  if( getTraf(TrafType(j),bt,endTime) == 0 ) continue;
                  f <<
		    "  <TH ALIGN=right BGCOLOR=\"" + utf8::String(trafTypeBodyDataColor_[j]) + "\" nowrap>\n"
		    "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" +
		    formatTraf(
  		      getTraf(
		        TrafType(j),
		        bt,
		        endTime,
		        usersTrafTable(i,"ST_USER"),
		        usersTrafTable(i,"ST_IS_GROUP")
		      ),
		      getTraf(ttAll,bt,endTime)
		    ) +
		    "    </FONT>\n"
		    "  </TH>\n"
                  ;
                }
              }
              endTime.tm_mon--;
            }
            f << "</TR>\n";
            endTime.tm_mon = 11;
            endTime.tm_mday = 31;
          }
        }
        // ѕечатаем итоговый трафик пользователей за год
        f <<
          "<TR>\n"
	  "  <TH ALIGN=right BGCOLOR=\"" + logger_->getDecor("tail.user",section_) + "\" wrap>\n"
	  "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
	  "Summary traffic of all users: "
        ;
        f <<
          "    </FONT>\n"
          "  </TH>\n"
        ;
        for( j = ttAll; j >= 0; j-- ){
          f <<
	    "  <TH ALIGN=right BGCOLOR=\"" + utf8::String(trafTypeTailColor_[j]) + "\" nowrap>\n"
	    "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" +
	    formatTraf(getTraf(TrafType(j),beginTime,endTime),getTraf(ttAll,beginTime,endTime)) +
	    "    </FONT>\n"
	    "  </TH>\n"
          ;
        }
        // ѕечатаем итоговый трафик пользователей за мес€цы
        et = endTime;
        while( et.tm_mon >= 0 ){
          bt = et;
          bt.tm_mday = 1;
          bt.tm_hour = 0;
          bt.tm_min = 0;
          bt.tm_sec = 0;
          et.tm_mday = (int) monthDays(et.tm_year + 1900, et.tm_mon);
          if( getTraf(ttAll, bt, et) > 0 ){
            for( j = ttAll; j >= 0; j-- ){
              if( getTraf(TrafType(j), bt, et) == 0 ) continue;
              f <<
                "  <TH ALIGN=right BGCOLOR=\"" + utf8::String(trafTypeTailDataColor_[j]) + "\" nowrap>\n"
                "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" +
                (j == ttAll ? formatTraf(getTraf(TrafType(j), bt, et),getTraf(ttAll,beginTime,endTime)) :
                              formatTraf(getTraf(TrafType(j), bt, et),getTraf(ttAll,bt,et))
	        ) +
                "    </FONT>\n"
                "  </TH>\n"
              ;
            }
          }
          et.tm_mon--;
        }
        f << "</TR>\n</TABLE>\n<BR>\n<BR>\n";
        if( !(bool) logger_->config_->valueByPath(section_ + ".html_report.refresh_only_current",true) || curTime_.tm_year == endTime.tm_year ){
          utf8::String fileName(
            includeTrailingPathDelimiter(htmlDir_) + trafByYearFile
          );
          writeMonthHtmlOutput(fileName,endTime);
        }
      }
      endTime.tm_year--;
      beginTime = beginTime2;
    }
    database_->commit();
    {
      AutoLock<InterlockedMutex> lock(logger_->trafCacheMutex_);
      f << "Cache size: " + utf8::int2Str((uintmax_t) logger_->trafCache_.count()) + "<BR>\n";
    }
    writeHtmlTail(f,ellapsed_);
    f.resize(f.tell());
    if( logger_->verbose_ ) fprintf(stderr,"%s\n",(const char *) getNameFromPathName(f.fileName()).getOEMString());
  }
}
//------------------------------------------------------------------------------
uintptr_t Logger::SquidSendmailThread::nonZeroYearMonthsColumns(struct tm byear)
{
  byear.tm_mday = 1;
  byear.tm_hour = byear.tm_min = byear.tm_sec = 0;
  uintptr_t a = 0;
  while( byear.tm_mon >= 0 ){
    struct tm eyear = byear;
    eyear.tm_mday = (int) monthDays(byear.tm_year + 1900, byear.tm_mon);
    eyear.tm_hour = 23;
    eyear.tm_min = 59;
    eyear.tm_sec = 59;
    a += getTraf(ttAll, byear, eyear) > 0;
    a += getTraf(ttWWW, byear, eyear) > 0;
    a += getTraf(ttSMTP, byear, eyear) > 0;
    byear.tm_mon--;
  }
  return a;
}
//------------------------------------------------------------------------------
uintptr_t Logger::SquidSendmailThread::nonZeroMonthDaysColumns(struct tm bmon)
{
  bmon.tm_mday = (int) monthDays(bmon.tm_year + 1900, bmon.tm_mon);
  bmon.tm_hour = bmon.tm_min = bmon.tm_sec = 0;
  uintptr_t a = 0;
  while( bmon.tm_mday > 0 ){
    struct tm emon  = bmon;
    emon.tm_hour = 23;
    emon.tm_min = 59;
    emon.tm_sec = 59;
    a += getTraf(ttAll,bmon,emon) > 0;
    a += getTraf(ttWWW,bmon,emon) > 0;
    a += getTraf(ttSMTP,bmon,emon) > 0;
    bmon.tm_mday--;
  }
  return a;
}
//------------------------------------------------------------------------------
utf8::String Logger::SquidSendmailThread::genUserFilter(const utf8::String & user,uintptr_t isGroup)
{
  utf8::String users;
  intptr_t i, j;
  for( i = enumStringParts(user) - 1, j = i; i >= 0; i-- ){
    if( i == j ) users += isGroup > 1 ? " " : " (";
    users += (isGroup > 1 ? "ST_USER <> :U" : "ST_USER = :U") + utf8::int2Str(i);
    if( i > 0 ) users += isGroup > 1 ? " AND " : " OR ";
    else
    if( i >= 0 ) users += isGroup > 1 ? " AND" : ") AND";
  }
  return users;
}
//------------------------------------------------------------------------------
int64_t Logger::SquidSendmailThread::getTrafNL(TrafType tt,const struct tm & bt,const struct tm & et,const utf8::String & user,uintptr_t isGroup)
{
  AutoPtr<TrafCacheEntry> tce(newObjectC1C2C3V4<TrafCacheEntry>(user,bt,et,tt));
  tce->bt_.tm_wday = 0;
  tce->bt_.tm_yday = 0;
  tce->et_.tm_wday = 0;
  tce->et_.tm_yday = 0;
  TrafCacheEntry * pEntry = logger_->trafCache_.find(tce);
  logger_->trafCache_.insert(tce,false,false,&pEntry);
  if( pEntry == tce ){
    tce.ptr(NULL);
    utf8::String users;
    switch( tt ){
      case ttSMTP :
      case ttWWW  :
        users = genUserFilter(user,isGroup);
        statement_->text(utf8::String("SELECT ") +
          (tt == ttWWW ? "SUM(ST_TRAF_WWW)" : "") +
          (tt == ttSMTP ? "SUM(ST_TRAF_SMTP) " : " ") +
          "FROM INET_USERS_TRAF WHERE" +
          (user.strlen() > 0 ? users : utf8::String()) +
          " ST_TIMESTAMP >= :BT AND ST_TIMESTAMP <= :ET"
        );
        statement_->prepare();
        for( intptr_t i = enumStringParts(user) - 1; i >= 0; i-- )
          statement_->paramAsString("U" + utf8::int2Str(i),stringPartByNo(user,i));
        statement_->
          paramAsMutant("BT",time2tm(tm2Time(bt) - getgmtoffset()))->
	  paramAsMutant("ET",time2tm(tm2Time(et) - getgmtoffset()))->
          execute()->fetchAll();
        pEntry->traf_ = statement_->valueAsMutant("SUM");
        break;
      case ttAll  :
        pEntry->traf_ = getTrafNL(ttWWW,bt,et,user,isGroup) + getTrafNL(ttSMTP,bt,et,user,isGroup);
        break;
      default     :
        break;
    }
    if( logger_->trafCacheSize_ > 0 && logger_->trafCache_.count() >= logger_->trafCacheSize_ )
      logger_->trafCache_.drop(logger_->trafCacheLRU_.remove(*logger_->trafCacheLRU_.last()));
  }
  else {
    logger_->trafCacheLRU_.remove(*pEntry);
  }
  logger_->trafCacheLRU_.insToHead(*pEntry);
  return pEntry->traf_;
}
//------------------------------------------------------------------------------
int64_t Logger::SquidSendmailThread::getTraf(TrafType tt,const struct tm & bt,const struct tm & et,const utf8::String & user,uintptr_t isGroup)
{
  AutoLock<InterlockedMutex> lock(logger_->trafCacheMutex_);
  return getTrafNL(tt,bt,et,user,isGroup);
}
//------------------------------------------------------------------------------
void Logger::SquidSendmailThread::parseSquidLogLine(char * p,uintptr_t size,Array<const char *> & slcp)
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
utf8::String Logger::SquidSendmailThread::shortUrl(const utf8::String & url)
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
void Logger::SquidSendmailThread::parseSquidLogFile(const utf8::String & logFileName, bool top10, const utf8::String & skipUrl)
{
  stMonUrlSel_->text(
    "SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(stMonUrlSel_->database()) != NULL ? " SQL_NO_CACHE" : "") +
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
  AsyncFile flog(logFileName);
  flog.readOnly(true).open();
/*
  statement_->text("DELETE FROM INET_USERS_TRAF")->execute();
  updateLogFileLastOffset(logFileName,0);
 */
  database_->start();
  if( (bool) logger_->config_->valueByPath(section_ + ".squid.reset_log_file_position",false) )
    updateLogFileLastOffset(stFileStat_,logFileName,0);
  uint64_t offset = fetchLogFileLastOffset(stFileStat_,logFileName);
  if( offset > flog.size() ) updateLogFileLastOffset(stFileStat_,logFileName,offset = 0);
  if( flog.seekable() ) flog.seek(offset);
  fallBackToNewLine(flog);
  int64_t lineNo = 1, tma = 0;
  uint64_t startTime = timeFromTimeString(logger_->config_->valueByPath(section_ + ".squid.start_time","01.01.1980"));
  uintptr_t size;
  Array<const char *> slcp;
  int64_t cl = getlocaltimeofday();
  AsyncFile::LineGetBuffer lgb(flog);
  lgb.codePage_ = logger_->config_->valueByPath(section_ + ".squid.log_file_codepage",CP_ACP);
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
      if( validLine && logger_->verbose_ && !startTimeLinePrinted ){
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
      Mutant timeStamp(Logger::timeStampRoundToMin(timeStamp1));
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
      if( flog.seekable() ) updateLogFileLastOffset(stFileStat_,logFileName,lgb.tell() - (validLine ? 0 : size));
      database_->commit();
      database_->start();
    }
    logger_->printStat(lineNo,offset,lgb.tell(),flog.size(),cl,&tma);
    lineNo++;
  }
  if( validLine && flog.seekable() ) updateLogFileLastOffset(stFileStat_,logFileName,lgb.tell() - (validLine ? 0 : size));
  database_->commit();
  logger_->printStat(lineNo,offset,lgb.tell(),flog.size(),cl);
}
//------------------------------------------------------------------------------
void Logger::SquidSendmailThread::parseSendmailLogFile(const utf8::String & logFileName, const utf8::String & domain,uintptr_t startYear)
{
  stMsgsIns_->text(
    "INSERT INTO INET_SENDMAIL_MESSAGES (ST_FROM,ST_MSGID,ST_MSGSIZE) " 
    "VALUES (:ST_FROM,:ST_MSGID,:ST_MSGSIZE);"
  );
  stMsgsSel_->text(
    "SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(stMsgsSel_->database()) != NULL ? " SQL_NO_CACHE" : "") +
    " * FROM INET_SENDMAIL_MESSAGES WHERE ST_MSGID = :ST_MSGID"
  );
  stMsgsDel_->text("DELETE FROM INET_SENDMAIL_MESSAGES");
  stMsgsDel2_->text("DELETE FROM INET_SENDMAIL_MESSAGES WHERE ST_MSGID = :ST_MSGID");
  stMsgsSelCount_->text(
    "SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(stMsgsSelCount_->database()) != NULL ? " SQL_NO_CACHE" : "") +
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
    "SELECT" + utf8::String(dynamic_cast<MYSQLDatabase *>(statement_->database()) != NULL ? " SQL_NO_CACHE" : "") +
    " MAX(ST_TIMESTAMP) AS ST_TIMESTAMP "
    "FROM INET_USERS_TRAF WHERE ST_TRAF_SMTP > 0"
  );
  AsyncFile flog(logFileName);
  flog.readOnly(true).open();
  database_->start();
  if( (bool) logger_->config_->valueByPath(section_ + ".sendmail.reset_log_file_position",false) )
    updateLogFileLastOffset(stFileStat_,logFileName,0);
  statement_->execute()->fetchAll();
  memset(&lt,0,sizeof(lt));
  if( statement_->rowCount() > 0 && statement_->fieldIndex("ST_TIMESTAMP") >= 0 && !statement_->valueIsNull("ST_TIMESTAMP") ){
    lt = statement_->valueAsMutant("ST_TIMESTAMP");
    startYear = lt.tm_year + 1900;
  }
  uint64_t offset = fetchLogFileLastOffset(stFileStat_,logFileName);
  if( offset > flog.size() ) updateLogFileLastOffset(stFileStat_,logFileName,offset = 0);
  if( flog.seekable() ) flog.seek(offset);
  fallBackToNewLine(flog);
  int64_t   lineNo  = 1, tma = 0;
  uintptr_t size;
  intptr_t  mon     = 0;
  int64_t   cl      = getlocaltimeofday();
  AsyncFile::LineGetBuffer lgb(flog);
  lgb.codePage_ = logger_->config_->valueByPath(section_ + ".sendmail.log_file_codepage",CP_ACP);
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
      if( flog.seekable() ) updateLogFileLastOffset(stFileStat_,logFileName,lgb.tell());
      database_->commit();
      database_->start();
    }
    logger_->printStat(lineNo,offset,lgb.tell(),flog.size(),cl,&tma);
    lineNo++;
  }
  if( flog.seekable() ) updateLogFileLastOffset(stFileStat_,logFileName,lgb.tell());
  stMsgsDel_->execute();
  stMsgsSelCount_->execute()->fetchAll();
  database_->commit();
  logger_->printStat(lineNo,offset,lgb.tell(),flog.size(),cl);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
utf8::String Logger::TrafCacheEntry::id() const
{
  return
    user_ + utf8::tm2Str(bt_) + utf8::tm2Str(et_) + trafTypeColumnName[trafType_]
  ;
}
//------------------------------------------------------------------------------
void Logger::writeHtmlHead(AsyncFile & f)
{
  f <<
    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
    "<HTML>\n"
    "<HEAD>\n"
    "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf8\">\n"
    "<meta http-equiv=\"Content-Language\" content=\"en\">\n"
  //    "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"600\">\n"
  //    "<META HTTP-EQUIV=\"Pragma\" CONTENT=\"no-cache\">\n"
  //    "<META HTTP-EQUIV=\"Cache-Control\" content=\"no-cache\">\n"
  "<TITLE>Statistics</TITLE>\n"
  "</HEAD>\n"
  "<BODY LANG=EN BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\">\n"
  "<FONT FACE=\"Arial\" SIZE=\"2\">\n"
  ;
}
//------------------------------------------------------------------------------
void Logger::writeHtmlTail(AsyncFile & f,int64_t ellapsed)
{
#if HAVE_UNAME
  struct utsname un;
  uname(&un);
#else
  struct {
    utf8::String nodename;
  } un;
  ksock::APIAutoInitializer ksockAPIAutoInitializer;
  un.nodename = ksock::SockAddr::gethostname();
#endif
  f <<
    "Start time: " + utf8::time2Str(ellapsed) +
    "<BR>\n" +
    "Finish time: " + utf8::time2Str(getlocaltimeofday()) +
    "<BR>\n" +
    "Ellapsed time: " + utf8::elapsedTime2Str(uintmax_t(getlocaltimeofday() - ellapsed)) + "\n<BR>\n" +
    "Generated on " + un.nodename + ", by " + macroscope_version.gnu_ + "\n<BR>\n"
#ifndef PRIVATE_RELEASE
    "<A HREF=\"http://developer.berlios.de/projects/macroscope/\">\n"
    "  http://developer.berlios.de/projects/macroscope/\n"
    "</A>\n"
#endif
    "</FONT>\n"
    "</BODY>\n"
    "</HTML>\n"
  ;
}
//------------------------------------------------------------------------------
} // namespace macrosocope
//------------------------------------------------------------------------------
