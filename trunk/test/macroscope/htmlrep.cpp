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
static const char * const trafTypeColumnName[]  = {
  "ST_TRAF_SMTP", "ST_TRAF_WWW", "ST_TRAF"
};
//------------------------------------------------------------------------------
void Logger::decoration()
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
  utf8::String  basePath("macroscope.decoration.");
  for( intptr_t i = sizeof(decos) / sizeof(decos[0]) - 1; i >= 0; i-- )
    for( intptr_t j = ttAll; j >= 0; j-- ){
      decos[i].colors[j] = config_->valueByPath(basePath + decos[i].path + nicks[j]);
    }
}
//------------------------------------------------------------------------------
void Logger::writeUserTop(
  const utf8::String & file,
  const utf8::String & user,
  const struct tm & beginTime,
  const struct tm & endTime)
{
  if( !(bool) config_->valueByPath(prefix_ + "squid.top10_url",true) ) return;
  statement_->text(
    "SELECT"
    "    A.*"
    "FROM"
    "    ("
    "        SELECT"
    "          ST_URL, SUM(ST_URL_TRAF) AS SUM1, SUM(ST_URL_COUNT) AS SUM2"
    "        FROM"
    "            INET_USERS_MONTHLY_TOP_URL"
    "        WHERE   ST_USER = :USER AND"
    "                ST_TIMESTAMP >= :BT AND"
    "                ST_TIMESTAMP <= :ET"
    "        GROUP BY ST_URL"
    "    ) AS A "
    "ORDER BY A.SUM1"
  );
  statement_->prepare()->
    paramAsMutant("USER",user)->
    paramAsMutant("BT",beginTime)->
    paramAsMutant("ET",endTime)->execute()->fetchAll();
  if( statement_->rowCount() > 0 ){
    AsyncFile f(file);
    f.createIfNotExist(true).open();
    writeHtmlHead(f);
    f <<
      "<TABLE WIDTH=100 BORDER=1 CELLSPACING=0 CELLPADDING=2>\n"
      "<TR>\n"
      "  <TH ALIGN=center BGCOLOR=\"" << trafTypeHeadDataColor_[ttAll] << "\" wrap>\n"
      "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
      "URL\n"
      "    </FONT>\n"
      "  </TH>\n"
      "  <TH ALIGN=center BGCOLOR=\"" << trafTypeHeadDataColor_[ttSMTP] << "\" nowrap>\n"
      "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
      "HIT\n"
      "    </FONT>\n"
      "  </TH>\n"
      "  <TH ALIGN=center BGCOLOR=\"" << trafTypeHeadDataColor_[ttSMTP] << "\" nowrap>\n"
      "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
      "KB/HIT\n"
      "    </FONT>\n"
      "  </TH>\n"
      "  <TH ALIGN=center BGCOLOR=\"" << trafTypeHeadDataColor_[ttWWW] << "\" nowrap>\n"
      "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
      "KB\n"
      "    </FONT>\n"
      "  </TH>\n"
      "</TR>\n"
    ;
    uint64_t at = 0;
    for( intptr_t i = statement_->rowCount() - 1; i >= 0; i-- ){
      statement_->selectRow(i);
      at += (uint64_t) statement_->valueAsMutant(1);
    }
    for( intptr_t i = statement_->rowCount() - 1; i >= 0; i-- ){
      statement_->selectRow(i);
      f <<
        "<TR>\n"
        "  <TH WITH=10 ALIGN=left BGCOLOR=\"" << trafTypeBodyDataColor_[ttAll] << "\" wrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\" wrap>\n" <<
        "      <A HREF=\"" << statement_->valueAsMutant(0) << "\" wrap>\n" <<
        statement_->valueAsMutant(0) << "\n"
        "      </A>\n"
        "    </FONT>\n"
        "  </TH>\n"
        "  <TH ALIGN=right BGCOLOR=\"" << trafTypeBodyDataColor_[ttSMTP] << "\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
        statement_->valueAsMutant(2) << "\n"
        "    </FONT>\n"
        "  </TH>\n"
        "  <TH ALIGN=right BGCOLOR=\"" << trafTypeBodyDataColor_[ttSMTP] << "\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
        utf8::int2Str(
          (
           (uint64_t) statement_->valueAsMutant(1) / 
            (
              (uint64_t) statement_->valueAsMutant(2) > 0 ? 
              (uint64_t) statement_->valueAsMutant(2) : 1u
            )
          ) / 1024u
        ) << "\n"
        "    </FONT>\n"
        "  </TH>\n"
        "  <TH ALIGN=right BGCOLOR=\"" << trafTypeBodyDataColor_[ttWWW] << "\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
      ;
      writeTraf(f,statement_->valueAsMutant(1),at);
      f << "\n"
        "    </FONT>\n"
        "  </TH>\n"
        "</TR>\n"
      ;
    }
    writeHtmlTail(f);
    f.resize(f.tell());
    if( verbose_ ) fprintf(stderr,"%s\n",(const char *) getNameFromPathName(f.fileName()).getOEMString());
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Logger::MTWriter::~MTWriter()
{
}
//------------------------------------------------------------------------------
Logger::MTWriter::MTWriter(Logger & logger,const utf8::String & file,const struct tm & year) :
  logger_(logger), file_(file), year_(year)
{
}
//------------------------------------------------------------------------------
void Logger::MTWriter::threadExecute()
{
  logger_.writeMonthHtmlOutput(file_,year_,true);
}
//------------------------------------------------------------------------------
void Logger::writeMonthHtmlOutput(const utf8::String & file,const struct tm & year,bool threaded)
{
  if( !threaded && (bool) config_->valueByPath("macroscope.multithreaded_html_writer",false) )
    threads_.add(newObjectR1C2C3<MTWriter>(*this,file,year)).resume();
  AsyncFile f(file);
  f.createIfNotExist(true).open();
  Mutant m0(config_->valueByPath(section_ + "file_mode",755));
  Mutant m1(config_->valueByPath(section_ + "file_user",ksys::getuid()));
  Mutant m2(config_->valueByPath(section_ + "file_group",ksys::getgid()));
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
      utf8::String trafByMonthFile(utf8::String::print("users-traf-by-%04d%02d.html", endTime.tm_year + 1900, endTime.tm_mon + 1));
      f <<
        "<TABLE WIDTH=400 BORDER=1 CELLSPACING=0 CELLPADDING=2>\n"
        "<TR>\n"
        "  <TH BGCOLOR=\"" <<
        config_->valueByPath("macroscope.decoration.colors.table_head") <<
        "\" COLSPAN=" <<
        utf8::int2Str(uintmax_t(nonZeroMonthDaysColumns(endTime) + ttAll + 2)) <<
        " ALIGN=left nowrap>\n" <<
        "<A HREF=\"" << trafByMonthFile << "\">" <<
        utf8::int2Str(uintmax_t(endTime.tm_mon + 1)) << "\n"
        "</A>\n"
        "  </TH>\n"
        "</TR>\n"
        "<TR>\n"
        "  <TH HEIGHT=4></TH>\n"
        "</TR>\n"
        "<TR>\n"
        "  <TH ROWSPAN=2 ALIGN=center BGCOLOR=\"#00A0FF\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
        "User\n"
        "    </FONT>\n"
        "  </TH>\n"
      ;
      intptr_t  i, j;
      // ѕечатаем заголовки суммарных трафиков пользовател€ за мес€ц
      for( i = ttAll; i >= 0; i-- ){
        f << "  <TH ROWSPAN=2 ALIGN=center BGCOLOR=\"" <<
          trafTypeHeadColor_[i] << "\" wrap>\n" <<
          "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" << trafTypeNick_[i] << "\n"
          "    </FONT>\n" "  </TH>\n"
        ;
      }
      // ѕечатаем заголовки только тех дней мес€ца в которых был не нулевой трафик
      while( endTime.tm_mday > 0 ){
        bt = endTime;
        bt.tm_hour = 0;
        bt.tm_min = 0;
        bt.tm_sec = 0;
        if( getTraf(ttAll, bt, endTime) > 0 ){
          f << "  <TH ALIGN=center COLSPAN=" <<
            utf8::int2Str((getTraf(ttAll, bt, endTime) > 0) +
            (getTraf(ttWWW, bt, endTime) > 0) +
            (getTraf(ttSMTP, bt, endTime) > 0)) << " BGCOLOR=\"" <<
            config_->valueByPath("macroscope.decoration.colors.detail_head") <<
            "\" nowrap>\n" "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
            utf8::String::print("%02d", endTime.tm_mday) << "\n"
            "    </FONT>\n" "  </TH>\n"
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
            f << "  <TH ALIGN=center BGCOLOR=\"" << trafTypeHeadDataColor_[i] << "\" wrap>\n" <<
              "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" << trafTypeNick_[i] << "\n"
              "    </FONT>\n" "  </TH>\n"
            ;
          }
        }
        endTime.tm_mday--;
      }
      f << "</TR>\n";
      endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900, endTime.tm_mon);
      // ѕечатаем трафик пользователей
      statement_->text(
        "SELECT DISTINCT ST_USER FROM INET_USERS_TRAF " 
        "WHERE ST_TIMESTAMP >= :BT AND ST_TIMESTAMP <= :ET"
      );
      Table<Mutant> usersTrafTable;
      statement_->prepare()->
        paramAsMutant("BT",beginTime)->paramAsMutant("ET",endTime)->
          execute()->unload(usersTrafTable);
      usersTrafTable.
        addColumn("ST_TRAF").addColumn("ST_TRAF_WWW").addColumn("ST_TRAF_SMTP");
      for( i = usersTrafTable.rowCount() - 1; i >= 0; i-- ){
        utf8::String  user(usersTrafTable(i, "ST_USER"));
        usersTrafTable(i, "ST_TRAF") = int64_t(usersTrafTable(i, "ST_TRAF_WWW") =
          getTraf(ttWWW, beginTime, endTime, user)) +
          int64_t(usersTrafTable(i, "ST_TRAF_SMTP") = getTraf(ttSMTP, beginTime, endTime, user));
      }
      usersTrafTable.sort(sortUsersTrafTable,usersTrafTable);
      for( i = usersTrafTable.rowCount() - 1; i >= 0; i-- ){
        if( getTraf(ttAll,beginTime,endTime,usersTrafTable(i,"ST_USER")) == 0 ) continue;
        utf8::String user(usersTrafTable(i,"ST_USER"));
        utf8::String topByUserFile(
          utf8::String::print(
            "top-%04d%02d-",
            endTime.tm_year + 1900,
            endTime.tm_mon + 1
          ) + user + ".html"
        );
        writeUserTop(
          includeTrailingPathDelimiter(htmlDir_) + topByUserFile,
          user,
          beginTime,
          endTime
        );
        f <<
          "<TR>\n"
          "  <TH ALIGN=left BGCOLOR=\"#00E0FF\" nowrap>\n"
          "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
          "<A HREF=\"" << topByUserFile << "\">" <<
            user << "\n"
          "</A>\n"
//          utf8::String(usersTrafTable(i,"ST_USER")) << "\n"
          "    </FONT>\n"
          "  </TH>\n"
        ;

        for( j = ttAll; j >= 0; j-- ){
          f <<
	    "  <TH ALIGN=right BGCOLOR=\"" << trafTypeBodyColor_[j] << "\" nowrap>\n"
	    "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
          ;
          writeTraf(f, usersTrafTable(i,trafTypeColumnName[j]),getTraf(ttAll, beginTime, endTime));
          f <<
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
                "  <TH ALIGN=right BGCOLOR=\"" << trafTypeBodyDataColor_[j] << "\" nowrap>\n"
                "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
              ;
              writeTraf(
                f,
                getTraf(TrafType(j),
                  bt,
                  endTime,
                  usersTrafTable(i,"ST_USER")
                ),
                getTraf(ttAll, bt, endTime)
              );
              f << "    </FONT>\n" "  </TH>\n"
              ;
            }
          }
          endTime.tm_mday--;
        }
        f << "</TR>\n";
        endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900, endTime.tm_mon);
      }
      // ѕечатаем итоговый трафик пользователей за мес€ц
      f <<
        "<TR>\n"
        "  <TH ALIGN=right BGCOLOR=\"#00A0FF\" wrap>\n"
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
          "  <TH ALIGN=right BGCOLOR=\"" << trafTypeTailColor_[j] << "\" nowrap>\n"
          "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
        ;
        writeTraf(f, getTraf(TrafType(j), beginTime, endTime), getTraf(ttAll, beginTime, endTime));
        f << "    </FONT>\n" "  </TH>\n"
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
              "  <TH ALIGN=right BGCOLOR=\"" << trafTypeTailDataColor_[j] << "\" nowrap>\n" 
              "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
            ;
            if( j == ttAll ){
              writeTraf(f, getTraf(TrafType(j), bt, et), getTraf(ttAll, beginTime, endTime));
            }
            else {
              writeTraf(f, getTraf(TrafType(j), bt, et), getTraf(ttAll, bt, et));
            }
            f <<
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
  writeHtmlTail(f);
  f.resize(f.tell());
  if( verbose_ ) fprintf(stderr,"%s\n",(const char *) getNameFromPathName(f.fileName()).getOEMString());
}
//------------------------------------------------------------------------------
void Logger::writeHtmlYearOutput()
{
  if( verbose_ ) fprintf(stderr,"\n");
  cacheSize_ = config_->section("macroscope").value("traffic_cache_size",0);
  decoration();
  struct tm beginTime, endTime, curTime;
  statement_->text("SELECT ");
  if( dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL )
    statement_->text(statement_->text() + "FIRST 1 ");
  statement_->text(statement_->text() + "ST_TIMESTAMP FROM INET_USERS_TRAF ORDER BY ST_TIMESTAMP");
  if( dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL )
    statement_->text(statement_->text() + " LIMIT 0,1");
  statement_->execute()->fetchAll();
  if( statement_->rowCount() > 0 ){
    beginTime = statement_->valueAsMutant("ST_TIMESTAMP");
    statement_->text("SELECT ");
    if( dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL )
      statement_->text(statement_->text() + "FIRST 1 ");
    statement_->text(statement_->text() + "ST_TIMESTAMP FROM INET_USERS_TRAF ORDER BY ST_TIMESTAMP DESC");
    if( dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL )
      statement_->text(statement_->text() + " LIMIT 0,1");
    statement_->execute()->fetchAll();
    endTime = statement_->valueAsMutant("ST_TIMESTAMP");
  }
  else {
    beginTime = endTime = time2tm(getlocaltimeofday());
  }
  curTime = time2tm(getlocaltimeofday());
  htmlDir_ = excludeTrailingPathDelimiter(config_->valueByPath(section_ + "directory"));
  AsyncFile f(
    includeTrailingPathDelimiter(htmlDir_) + config_->valueByPath(section_ + "index_file_name","index.html")
  );
  f.createIfNotExist(true).open();
  Mutant m0(config_->valueByPath(section_ + "directory_mode",755));
  Mutant m1(config_->valueByPath(section_ + "directory_user",ksys::getuid()));
  Mutant m2(config_->valueByPath(section_ + "directory_group",ksys::getgid()));
  chModOwn(htmlDir_,m0,m1,m2);
  m0 = config_->valueByPath(section_ + "file_mode",755);
  m1 = config_->valueByPath(section_ + "file_user",ksys::getuid());
  m2 = config_->valueByPath(section_ + "file_group",ksys::getgid());
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
  while( tm2Time(endTime) >= tm2Time(beginTime) ){
    beginTime2 = beginTime;
    beginTime.tm_year = endTime.tm_year;
    bool process = !(bool) config_->valueByPath(section_ + "refresh_only_current_year",true) || curTime.tm_year == beginTime.tm_year;
    if( getTraf(ttAll,beginTime,endTime) > 0 ){
      utf8::String trafByYearFile(utf8::String::print("users-traf-by-%04d.html",endTime.tm_year + 1900));
      f <<
        "<TABLE WIDTH=400 BORDER=1 CELLSPACING=0 CELLPADDING=2>\n"
        "<TR>\n"
        "  <TH BGCOLOR=\"" <<
        config_->valueByPath("macroscope.decoration.colors.table_head") <<
        "\" COLSPAN=" <<
        utf8::int2Str(uintmax_t(nonZeroYearMonthsColumns(endTime) + ttAll + 2)) <<
        " ALIGN=left nowrap>\n" <<
        "<A HREF=\"" << trafByYearFile << "\">" <<
        utf8::int2Str(endTime.tm_year + 1900) << "\n"
        "</A>\n"
        "  </TH>\n"
        "</TR>\n"
        "<TR>\n"
        "  <TH HEIGHT=4></TH>\n"
        "</TR>\n"
        "<TR>\n"
        "  <TH ROWSPAN=2 ALIGN=center BGCOLOR=\"#00A0FF\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
        "User\n"
        "    </FONT>\n" "  </TH>\n"
      ;
      intptr_t  i, j;
      // ѕечатаем заголовки суммарных трафиков пользовател€ за год
      for( i = ttAll; i >= 0; i-- ){
        f <<
	  "  <TH ROWSPAN=2 ALIGN=center BGCOLOR=\"" << trafTypeHeadColor_[i] << "\" wrap>\n" <<
	  "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" << trafTypeNick_[i] <<
	  "\n" "    </FONT>\n" "  </TH>\n"
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
	    "  <TH ALIGN=center COLSPAN=" <<
            utf8::int2Str((getTraf(ttAll, bt, endTime) > 0) +
            (getTraf(ttWWW, bt, endTime) > 0) +
            (getTraf(ttSMTP, bt, endTime) > 0)) <<
            " BGCOLOR=\"" << config_->valueByPath("macroscope.decoration.colors.detail_head") << "\" nowrap>\n"
	    "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" << utf8::String::print("%02d", endTime.tm_mon + 1) << "\n"
	    "    </FONT>\n" "  </TH>\n"
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
	      "  <TH ALIGN=center BGCOLOR=\"" << trafTypeHeadDataColor_[i] << "\" wrap>\n" <<
	      "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" << trafTypeNick_[i] << "\n"
	      "    </FONT>\n" "  </TH>\n"
            ;
          }
        }
        endTime.tm_mon--;
      }
      f << "</TR>\n";
      endTime.tm_mon = 11;
      endTime.tm_mday = 31;
      // ѕечатаем трафик пользователей
      statement_->text(
        "SELECT DISTINCT ST_USER FROM INET_USERS_TRAF "
        "WHERE ST_TIMESTAMP >= :BT AND ST_TIMESTAMP <= :ET"
      );
      Table<Mutant> usersTrafTable;
      statement_->prepare()->
        paramAsMutant("BT", beginTime)->paramAsMutant("ET", endTime)->
        execute()->unload(usersTrafTable);
      usersTrafTable.
        addColumn("ST_TRAF").
        addColumn("ST_TRAF_WWW").
        addColumn("ST_TRAF_SMTP");
      for( i = usersTrafTable.rowCount() - 1; i >= 0; i-- ){
        utf8::String user(usersTrafTable(i, "ST_USER"));
        usersTrafTable(i, "ST_TRAF") = int64_t(usersTrafTable(i, "ST_TRAF_WWW") = 
          getTraf(ttWWW, beginTime, endTime, user)) + 
          int64_t(usersTrafTable(i,"ST_TRAF_SMTP") = 
          getTraf(ttSMTP,beginTime,endTime,user));
      }
      usersTrafTable.sort(sortUsersTrafTable, usersTrafTable);
      for( i = usersTrafTable.rowCount() - 1; i >= 0; i-- ){
        if( getTraf(ttAll, beginTime, endTime, usersTrafTable(i, "ST_USER")) == 0 ) continue;
        f <<
	  "<TR>\n" "  <TH ALIGN=left BGCOLOR=\"#00E0FF\" nowrap>\n"
          "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
          utf8::String(usersTrafTable(i, "ST_USER")) << "\n" "    </FONT>\n"
	  "  </TH>\n"
        ;
        for( j = ttAll; j >= 0; j-- ){
          f <<
	    "  <TH ALIGN=right BGCOLOR=\"" << trafTypeBodyColor_[j] << "\" nowrap>\n"
	    "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
          ;
          writeTraf(f, usersTrafTable(i, trafTypeColumnName[j]), getTraf(ttAll, beginTime, endTime));
          f <<
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
          if( getTraf(ttAll, bt, endTime) > 0 ){
            for( j = ttAll; j >= 0; j-- ){
              if( getTraf(TrafType(j), bt, endTime) == 0 )
                continue;
              f << "  <TH ALIGN=right BGCOLOR=\"" << trafTypeBodyDataColor_[j] << "\" nowrap>\n" "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
              ;
              writeTraf(f, getTraf(TrafType(j), bt, endTime, usersTrafTable(i, "ST_USER")), getTraf(ttAll, bt, endTime));
              f << "    </FONT>\n" "  </TH>\n"
              ;
            }
          }
          endTime.tm_mon--;
        }
        f << "</TR>\n";
        endTime.tm_mon = 11;
        endTime.tm_mday = 31;
      }
      // ѕечатаем итоговый трафик пользователей за год
      f <<
        "<TR>\n"
	"  <TH ALIGN=right BGCOLOR=\"#00A0FF\" wrap>\n"
	"    <FONT FACE=\"Arial\" SIZE=\"2\">\n" "Summary traffic of all users: "
      ;
      f <<
        "    </FONT>\n"
        "  </TH>\n"
      ;
      for( j = ttAll; j >= 0; j-- ){
        f <<
	  "  <TH ALIGN=right BGCOLOR=\"" << trafTypeTailColor_[j] << "\" nowrap>\n"
	  "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
        ;
        writeTraf(f, getTraf(TrafType(j), beginTime, endTime), getTraf(ttAll, beginTime, endTime));
        f <<
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
              "  <TH ALIGN=right BGCOLOR=\"" << trafTypeTailDataColor_[j] << "\" nowrap>\n"
              "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
            ;
            if( j == ttAll ){
              writeTraf(f, getTraf(TrafType(j), bt, et), getTraf(ttAll, beginTime, endTime));
            }
            else{
              writeTraf(f, getTraf(TrafType(j), bt, et), getTraf(ttAll, bt, et));
            }
            f <<
              "    </FONT>\n"
              "  </TH>\n"
            ;
          }
        }
        et.tm_mon--;
      }
      f << "</TR>\n</TABLE>\n<BR>\n<BR>\n";
      if( process ){
        utf8::String fileName(
          includeTrailingPathDelimiter(htmlDir_) + trafByYearFile
        );
        writeMonthHtmlOutput(fileName, endTime);
      }
    }
    endTime.tm_year--;
    beginTime = beginTime2;
  }
  writeBPFTHtmlReport(f);
  f << "Ellapsed time: " <<
    utf8::elapsedTime2Str(uintmax_t(getlocaltimeofday() - ellapsed_)) << "\n<BR>\n" <<
    utf8::int2Str((uintmax_t) trafCache_.count()) << "<BR>\n";
  writeHtmlTail(f);
  f.resize(f.tell());
  if( verbose_ ) fprintf(stderr,"%s\n",(const char *) getNameFromPathName(f.fileName()).getOEMString());
}
//------------------------------------------------------------------------------
uintptr_t Logger::nonZeroYearMonthsColumns(struct tm byear)
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
uintptr_t Logger::nonZeroMonthDaysColumns(struct tm bmon)
{
  bmon.tm_mday = (int) monthDays(bmon.tm_year + 1900, bmon.tm_mon);
  bmon.tm_hour = bmon.tm_min = bmon.tm_sec = 0;
  uintptr_t a = 0;
  while( bmon.tm_mday > 0 ){
    struct tm emon  = bmon;
    emon.tm_hour = 23;
    emon.tm_min = 59;
    emon.tm_sec = 59;
    a += getTraf(ttAll, bmon, emon) > 0;
    a += getTraf(ttWWW, bmon, emon) > 0;
    a += getTraf(ttSMTP, bmon, emon) > 0;
    bmon.tm_mday--;
  }
  return a;
}
//------------------------------------------------------------------------------
intptr_t Logger::sortUsersTrafTable(uintptr_t row1, uintptr_t row2, const Table< Mutant> & table)
{
  int64_t   t1  = table(row1, "ST_TRAF"), t2 = table(row2, "ST_TRAF");
  intptr_t  c   = t1 > t2 ? 1 : t1 < t2 ? -1 : 0;
  if( c == 0 )
    c = utf8::String(table(row1, "ST_USER")).strcasecmp(table(row2, "ST_USER"));
  return c;
}
//------------------------------------------------------------------------------
void Logger::writeTraf(AsyncFile & f, uint64_t qi, uint64_t qj)
{
  uint64_t  q, a  = qi % 1024u != 0, b , c ;

  qj += qj == 0;
  q = qi * 10000u / qj;
  b = q / 100u;
  c = q % 100u;
  c += b == 0 && c == 0;
  f << utf8::String::print(
    qi > 0 ? "%"PRIu64"<FONT FACE=\"Times New Roman\" SIZE=\"0\"> (%"PRIu64".%02"PRIu64"%%)</FONT>\n" :
    "-", (qi / 1024u) + a, b, c
  );
}
//------------------------------------------------------------------------------
utf8::String Logger::TrafCacheEntry::id() const
{
  return
    user_ + utf8::time2Str(tm2Time(bt_)) + utf8::time2Str(tm2Time(et_)) + trafTypeColumnName[trafType_]
  ;
}
//------------------------------------------------------------------------------
void Logger::writeHtmlHead(AsyncFile & f)
{
  f << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
    "<HTML>\n" "<HEAD>\n"
  //    "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"600\">\n"
  //    "<META HTTP-EQUIV=\"Pragma\" CONTENT=\"no-cache\">\n"
  //    "<META HTTP-EQUIV=\"Cache-Control\" content=\"no-cache\">\n"
  "<TITLE>Statistics by user</TITLE>\n"
  "</HEAD>\n"
  "<BODY lang=EN BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\">\n"
  ;
}
//------------------------------------------------------------------------------
void Logger::writeHtmlTail(AsyncFile & f)
{
#if HAVE_UNAME
  struct utsname  un;
  uname(&un);
  f << un.nodename << "\n<BR>\n";
#endif
  f << "</BODY>\n"
    "</HTML>\n";
}
//------------------------------------------------------------------------------
int64_t Logger::getTraf(TrafType tt,const struct tm & bt,const struct tm & et,const utf8::String & user)
{
  AutoPtr<TrafCacheEntry> tce(newObjectV1C2C3C4<TrafCacheEntry>(user,bt,et,tt));
  tce->bt_.tm_wday = 0;
  tce->bt_.tm_yday = 0;
  tce->et_.tm_wday = 0;
  tce->et_.tm_yday = 0;
  TrafCacheEntry * pEntry = trafCache_.find(tce);
  trafCache_.insert(tce,false,false,&pEntry);
  if( pEntry == tce ){
    tce.ptr(NULL);
    statement_->text(utf8::String("SELECT ") +
      (tt == ttAll || tt == ttWWW ? "SUM(ST_TRAF_WWW)" : "") +
      (tt == ttAll ? "," : "") +
      (tt == ttAll || tt == ttSMTP ? "SUM(ST_TRAF_SMTP)" : "") +
      "FROM INET_USERS_TRAF WHERE " +
      (user.strlen() > 0 ? "ST_USER = :ST_USER AND " : "") +
      "ST_TIMESTAMP >= :BT AND ST_TIMESTAMP <= :ET");
    statement_->prepare();
    if( user.strlen() > 0 )
      statement_->paramAsString("ST_USER", user);
    statement_->paramAsMutant("BT", bt)->paramAsMutant("ET", et);
    statement_->execute()->fetchAll();
#ifndef NDEBUG
//    for( intptr_t i = statement_->fieldCount() - 1; i >= 0; i-- )
//      fprintf(stderr,"%s %d\n",(const char *) statement_->fieldName(i).getANSIString(),(int) i);
#endif
    switch( tt ){
      case ttSMTP :
      case ttWWW  :
        pEntry->traf_ = statement_->valueAsMutant("SUM");
        break;
      case ttAll  :
        pEntry->traf_ =
          (int64_t) statement_->valueAsMutant("SUM") +
          (int64_t) statement_->valueAsMutant("SUM_1");
        break;
      default     :
        break;
    }
    if( cacheSize_ > 0 && trafCache_.count() >= cacheSize_ )
      trafCache_.drop(trafCacheLRU_.remove(*trafCacheLRU_.last()));
  }
  else {
    trafCacheLRU_.remove(*pEntry);
  }
  trafCacheLRU_.insToHead(*pEntry);
  return pEntry->traf_;
}
//------------------------------------------------------------------------------
/*  fprintf(fyear,
    "<FONT FACE=\"Arial\">\n"
    "  <A HREF=\"bpft-traf.html\">—татистика пакетного фильтра bpft</A><BR><BR>\n"
    "</FONT>\n"
  );
  uint64_t k = AllTraf();
  fprintf(fyear,
    "<FONT FACE=\"Arial\">\n"
    "—уммарный трафик: %qu,%04qu ( б) %qu,%04qu (ћб) %qu,%04qu (√б)\n"
    "</FONT>\n<BR>\n<BR>\n",
    k / 1024ULL,                     k % 1024ULL,
    k / 1024ULL / 1024ULL,           (k / 1024ULL) % 1024ULL,
    k / 1024ULL / 1024ULL / 1024ULL, (k / 1024ULL / 1024ULL) % 1024ULL
  );*/
//------------------------------------------------------------------------------
} // namespace macrosocope
//------------------------------------------------------------------------------
