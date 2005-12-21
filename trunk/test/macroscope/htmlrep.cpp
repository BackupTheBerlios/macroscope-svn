//------------------------------------------------------------------------------
#include <adicpp/adicpp.h>
//------------------------------------------------------------------------------
#include "macroscope.h"
//------------------------------------------------------------------------------
namespace macroscope {
//------------------------------------------------------------------------------
static const char * const trafTypeColumnName[] = {
  "ST_TRAF_SMTP",
  "ST_TRAF_WWW",
  "ST_TRAF"
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
  utf8::String basePath(utf8::string("macroscope.decoration."));
  for( intptr_t i = sizeof(decos) / sizeof(decos[0]) - 1; i >= 0; i-- )
    for( intptr_t j = ttAll; j >= 0; j-- ){
      decos[i].colors[j] =
        ksys::unScreenString(config_.valueByPath(basePath + decos[i].path + nicks[j]));
    }
}
//------------------------------------------------------------------------------
void Logger::writeMonthHtmlOutput(const utf8::String & file,const struct tm & year)
{
  ksys::FileHandleContainer f(file);
  f.open().resize(0);
  utf8::String section(utf8::string(
    "macroscope."
#if defined(__WIN32__) || defined(__WIN64__)
    "windows."
#else
    "unix."
#endif
    "html_report."
  ));
  ksys::chModOwn(file,
    config_.valueByPath(section + "file_mode",755),
    config_.valueByPath(section + "file_user",
#if HAVE_GETUID
    getuid()
#else
    0
#endif
    ),
    config_.valueByPath(section + "file_group",
#if HAVE_GETGID
    getuid()
#else
    0
#endif
    )
  );
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
    endTime.tm_mday = (int) ksys::monthDays(endTime.tm_year + 1900,endTime.tm_mon);
    beginTime2 = beginTime;
    beginTime.tm_mon = endTime.tm_mon;
    if( getTraf(ttAll,beginTime,endTime) > 0 ){
      utf8::String trafByMonthFile(utf8::String::print(
        "users-traf-by-%04d%02d.html",
        endTime.tm_year + 1900,endTime.tm_mon + 1
      ));
      f << 
        "<TABLE WIDTH=400 BORDER=1 CELLSPACING=0 CELLPADDING=2>\n"
        "<TR>\n"
        "  <TH BGCOLOR=\"" <<
        config_.valueByPath(utf8::string("macroscope.decoration.colors.table_head")) <<
        "\" COLSPAN=" <<
        utf8::int2Str(nonZeroMonthDaysColumns(endTime) + ttAll + 2) <<
        " ALIGN=left nowrap>\n" <<
        "<A HREF=\"" << trafByMonthFile << "\">" <<
        utf8::int2Str(endTime.tm_mon + 1) << "\n"
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
      intptr_t i, j;
// ѕечатаем заголовки суммарных трафиков пользовател€ за мес€ц
      for( i = ttAll; i >= 0; i-- ){
        f <<
          "  <TH ROWSPAN=2 ALIGN=center BGCOLOR=\"" << trafTypeHeadColor_[i] << "\" wrap>\n" <<
          "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
          trafTypeNick_[i] << "\n"
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
        if( getTraf(ttAll,bt,endTime) > 0 ){
          f <<
            "  <TH ALIGN=center COLSPAN=" <<
            utf8::int2Str(
              (getTraf(ttAll,bt,endTime) > 0) +
              (getTraf(ttWWW,bt,endTime) > 0) +
              (getTraf(ttSMTP,bt,endTime) > 0)
            ) <<
            " BGCOLOR=\"" <<
            config_.valueByPath(utf8::string("macroscope.decoration.colors.detail_head")) <<
            "\" nowrap>\n"
            "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
            utf8::String::print("%02d",endTime.tm_mday) <<
            "\n"
            "    </FONT>\n"
            "  </TH>\n"
          ;
        }
        endTime.tm_mday--;
      }
      f << "</TR>\n<TR>\n";
      endTime.tm_mday = (int) ksys::monthDays(endTime.tm_year + 1900,endTime.tm_mon);
// ѕечатаем заголовки трафиков пользовател€ за мес€цы
      while( endTime.tm_mday > 0 ){
        bt = endTime;
        bt.tm_hour = 0;
        bt.tm_min = 0;
        bt.tm_sec = 0;
        if( getTraf(ttAll,bt,endTime) > 0 ){
          for( i = ttAll; i >= 0; i-- ){
            if( getTraf(TrafType(i),bt,endTime) == 0 ) continue;
            f <<
              "  <TH ALIGN=center BGCOLOR=\"" << trafTypeHeadDataColor_[i] << "\" wrap>\n" <<
              "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
              trafTypeNick_[i] << "\n"
              "    </FONT>\n"
              "  </TH>\n"
            ;
          }
        }
        endTime.tm_mday--;
      }
      f << "</TR>\n";
      endTime.tm_mday = (int) ksys::monthDays(endTime.tm_year + 1900,endTime.tm_mon);
// ѕечатаем трафик пользователей
      statement_->text(utf8::string(
        "SELECT DISTINCT ST_USER FROM INET_USERS_TRAF "
        "WHERE ST_TIMESTAMP >= :BT AND ST_TIMESTAMP <= :ET"
      ));
      ksys::Table<ksys::Mutant> usersTrafTable;
      statement_->prepare()->
        paramAsMutant(utf8::string("BT"),beginTime)->
	paramAsMutant(utf8::string("ET"),endTime)->
        execute()->unload(usersTrafTable);
      usersTrafTable.
        addColumn(utf8::string("ST_TRAF")).
        addColumn(utf8::string("ST_TRAF_WWW")).
        addColumn(utf8::string("ST_TRAF_SMTP"));
      for( i = usersTrafTable.rowCount() - 1; i >= 0; i-- ){
        utf8::String user(usersTrafTable(i,utf8::string("ST_USER")));
        usersTrafTable(i,utf8::string("ST_TRAF")) =
          int64_t(usersTrafTable(i,utf8::string("ST_TRAF_WWW")) = getTraf(ttWWW,beginTime,endTime,user)) +
          int64_t(usersTrafTable(i,utf8::string("ST_TRAF_SMTP")) = getTraf(ttSMTP,beginTime,endTime,user));
      }
      usersTrafTable.sort(sortUsersTrafTable,usersTrafTable);
      for( i = usersTrafTable.rowCount() - 1; i >= 0; i-- ){
        if( getTraf(ttAll,beginTime,endTime,usersTrafTable(i,utf8::string("ST_USER"))) == 0 ) continue;
        f <<
          "<TR>\n"
          "  <TH ALIGN=left BGCOLOR=\"#00E0FF\" nowrap>\n"
          "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
          utf8::String(usersTrafTable(i,utf8::string("ST_USER"))) << "\n"
          "    </FONT>\n"
          "  </TH>\n"
        ;
        for( j = ttAll; j >= 0; j-- ){
          f <<
            "  <TH ALIGN=right BGCOLOR=\"" << trafTypeBodyColor_[j] << "\" nowrap>\n"
            "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
          ;
          writeTraf(f,usersTrafTable(i,utf8::string(trafTypeColumnName[j])),
            getTraf(ttAll,beginTime,endTime));
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
          if( getTraf(ttAll,bt,endTime) > 0 ){
            for( j = ttAll; j >= 0; j-- ){
              if( getTraf(TrafType(j),bt,endTime) == 0 ) continue;
              f <<
                "  <TH ALIGN=right BGCOLOR=\"" << trafTypeBodyDataColor_[j] << "\" nowrap>\n"
                "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
              ;
              writeTraf(f,
                getTraf(TrafType(j),bt,endTime,usersTrafTable(i,utf8::string("ST_USER"))),
                getTraf(ttAll,bt,endTime));
              f <<
                "    </FONT>\n"
                "  </TH>\n"
              ;
            }
          }
          endTime.tm_mday--;
        }
        f << "</TR>\n";
        endTime.tm_mday = (int) ksys::monthDays(endTime.tm_year + 1900,endTime.tm_mon);
      }
// ѕечатаем итоговый трафик пользователей за мес€ц
      f <<
        "<TR>\n"
        "  <TH ALIGN=right BGCOLOR=\"#00A0FF\" wrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
        "Summary traffic of all users: "
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
        writeTraf(f,
          getTraf(TrafType(j),beginTime,endTime),
          getTraf(ttAll,beginTime,endTime)
        );
        f <<
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
            if( getTraf(TrafType(j),bt,et) == 0 ) continue;
            f <<
              "  <TH ALIGN=right BGCOLOR=\"" << trafTypeTailDataColor_[j] << "\" nowrap>\n"
              "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
            ;
            if( j == ttAll ){
              writeTraf(f,
                getTraf(TrafType(j),bt,et),
                getTraf(ttAll,beginTime,endTime)
              );
            }
            else {
              writeTraf(f,
                getTraf(TrafType(j),bt,et),
                getTraf(ttAll,bt,et)
              );
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
}
//------------------------------------------------------------------------------
void Logger::writeHtmlYearOutput()
{
  cacheSize_ = config_.section(utf8::string("macroscope")).value(
    utf8::string("traffic_cache_size"),8192
  );
  decoration();
  struct tm beginTime, endTime;
  statement_->text(utf8::string("SELECT "));
  if( dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL )
    statement_->text(statement_->text() + "FIRST 1 ");
  statement_->text(statement_->text() +
    "ST_TIMESTAMP FROM INET_USERS_TRAF ORDER BY ST_TIMESTAMP"
  );
  if( dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL )
    statement_->text(statement_->text() + " LIMIT 0,1");
  statement_->execute()->fetchAll();
  if( statement_->rowCount() > 0 ){
    beginTime = statement_->valueAsMutant(utf8::string("ST_TIMESTAMP"));
    statement_->text(utf8::string("SELECT "));
    if( dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL )
      statement_->text(statement_->text() + "FIRST 1 ");
    statement_->text(statement_->text() +
      "ST_TIMESTAMP FROM INET_USERS_TRAF ORDER BY ST_TIMESTAMP DESC"
    );
    if( dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL )
      statement_->text(statement_->text() + " LIMIT 0,1");
    statement_->execute()->fetchAll();
    endTime = statement_->valueAsMutant(utf8::string("ST_TIMESTAMP"));
  }
  else {
    beginTime = endTime = ksys::time2tm(gettimeofday());
  }
  utf8::String section(utf8::string(
    "macroscope."
#if defined(__WIN32__) || defined(__WIN64__)
    "windows."
#else
    "unix."
#endif
    "html_report."
  ));
  utf8::String dir(ksys::excludeTrailingPathDelimiter(
    ksys::unScreenString(config_.valueByPath(section + "directory"))
  ));
  ksys::createDirectory(dir);
  ksys::chModOwn(dir,
    config_.valueByPath(section + "directory_mode",755),
    config_.valueByPath(section + "directory_user",
#if HAVE_GETUID
    getuid()
#else
    0
#endif
    ),
    config_.valueByPath(section + "directory_group",
#if HAVE_GETGID
    getgid()
#else
    0
#endif
    )
  );
  ksys::FileHandleContainer f(
    ksys::includeTrailingPathDelimiter(dir) + "users-traf-by-year.html"
  );
  f.open().resize(0);
  ksys::chModOwn(f.fileName(),
    config_.valueByPath(section + "file_mode",755),
    config_.valueByPath(section + "file_user",
#if HAVE_GETUID
    getuid()
#else
    0
#endif
    ),
    config_.valueByPath(section + "file_group",
#if HAVE_GETGID
    getgid()
#else
    0
#endif
    )
  );
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
  while( ksys::tm2Time(endTime) >= ksys::tm2Time(beginTime) ){
    beginTime2 = beginTime;
    beginTime.tm_year = endTime.tm_year;
    if( getTraf(ttAll,beginTime,endTime) > 0 ){
      utf8::String trafByYearFile(
        utf8::String::print("users-traf-by-%04d.html",endTime.tm_year + 1900)
      );
      f <<
        "<TABLE WIDTH=400 BORDER=1 CELLSPACING=0 CELLPADDING=2>\n"
        "<TR>\n"
        "  <TH BGCOLOR=\"" <<
        config_.valueByPath(utf8::string("macroscope.decoration.colors.table_head")) <<
        "\" COLSPAN=" <<
        utf8::int2Str(nonZeroYearMonthsColumns(endTime) + ttAll + 2) <<
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
        "    </FONT>\n"
        "  </TH>\n"
      ;
      intptr_t i, j;
// ѕечатаем заголовки суммарных трафиков пользовател€ за год
      for( i = ttAll; i >= 0; i-- ){
        f <<
          "  <TH ROWSPAN=2 ALIGN=center BGCOLOR=\"" << trafTypeHeadColor_[i] << "\" wrap>\n" <<
          "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
          trafTypeNick_[i] << "\n"
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
        endTime.tm_mday = ksys::monthDays(endTime.tm_year + 1900,endTime.tm_mon);
        if( getTraf(ttAll,bt,endTime) > 0 ){
          f <<
            "  <TH ALIGN=center COLSPAN=" <<
            utf8::int2Str(
              (getTraf(ttAll,bt,endTime) > 0) +
              (getTraf(ttWWW,bt,endTime) > 0) +
              (getTraf(ttSMTP,bt,endTime) > 0)
            ) <<
            " BGCOLOR=\"" <<
            config_.valueByPath(utf8::string("macroscope.decoration.colors.detail_head")) <<
            "\" nowrap>\n"
            "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
            utf8::String::print("%02d",endTime.tm_mon + 1) <<
            "\n"
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
        endTime.tm_mday = ksys::monthDays(endTime.tm_year + 1900,endTime.tm_mon);
        if( getTraf(ttAll,bt,endTime) > 0 ){
          for( i = ttAll; i >= 0; i-- ){
            if( getTraf(TrafType(i),bt,endTime) == 0 ) continue;
            f <<
              "  <TH ALIGN=center BGCOLOR=\"" << trafTypeHeadDataColor_[i] << "\" wrap>\n" <<
              "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
              trafTypeNick_[i] << "\n"
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
      statement_->text(utf8::string(
        "SELECT DISTINCT ST_USER FROM INET_USERS_TRAF "
        "WHERE ST_TIMESTAMP >= :BT AND ST_TIMESTAMP <= :ET"
      ));
      ksys::Table<ksys::Mutant> usersTrafTable;
      statement_->prepare()->
        paramAsMutant(utf8::string("BT"),beginTime)->paramAsMutant(utf8::string("ET"),endTime)->
          execute()->unload(usersTrafTable);
      usersTrafTable.
        addColumn(utf8::string("ST_TRAF")).
        addColumn(utf8::string("ST_TRAF_WWW")).
        addColumn(utf8::string("ST_TRAF_SMTP"));
      for( i = usersTrafTable.rowCount() - 1; i >= 0; i-- ){
        utf8::String user(usersTrafTable(i,utf8::string("ST_USER")));
        usersTrafTable(i,utf8::string("ST_TRAF")) =
          int64_t(usersTrafTable(i,utf8::string("ST_TRAF_WWW")) = getTraf(ttWWW,beginTime,endTime,user)) +
          int64_t(usersTrafTable(i,utf8::string("ST_TRAF_SMTP")) = getTraf(ttSMTP,beginTime,endTime,user));
      }
      usersTrafTable.sort(sortUsersTrafTable,usersTrafTable);
      for( i = usersTrafTable.rowCount() - 1; i >= 0; i-- ){
        if( getTraf(ttAll,beginTime,endTime,usersTrafTable(i,utf8::string("ST_USER"))) == 0 ) continue;
        f <<
          "<TR>\n"
          "  <TH ALIGN=left BGCOLOR=\"#00E0FF\" nowrap>\n"
          "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
          utf8::String(usersTrafTable(i,utf8::string("ST_USER"))) << "\n"
          "    </FONT>\n"
          "  </TH>\n"
        ;
        for( j = ttAll; j >= 0; j-- ){
          f <<
            "  <TH ALIGN=right BGCOLOR=\"" << trafTypeBodyColor_[j] << "\" nowrap>\n"
            "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
          ;
          writeTraf(f,usersTrafTable(i,utf8::string(trafTypeColumnName[j])),
            getTraf(ttAll,beginTime,endTime));
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
          endTime.tm_mday = ksys::monthDays(endTime.tm_year + 1900,endTime.tm_mon);
          if( getTraf(ttAll,bt,endTime) > 0 ){
            for( j = ttAll; j >= 0; j-- ){
              if( getTraf(TrafType(j),bt,endTime) == 0 ) continue;
              f <<
                "  <TH ALIGN=right BGCOLOR=\"" << trafTypeBodyDataColor_[j] << "\" nowrap>\n"
                "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
              ;
              writeTraf(f,
                getTraf(TrafType(j),bt,endTime,usersTrafTable(i,utf8::string("ST_USER"))),
                getTraf(ttAll,bt,endTime));
              f <<
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
// ѕечатаем итоговый трафик пользователей за год
      f <<
        "<TR>\n"
        "  <TH ALIGN=right BGCOLOR=\"#00A0FF\" wrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
        "Summary traffic of all users: "
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
        writeTraf(f,
          getTraf(TrafType(j),beginTime,endTime),
          getTraf(ttAll,beginTime,endTime)
        );
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
        et.tm_mday = ksys::monthDays(et.tm_year + 1900,et.tm_mon);
        if( getTraf(ttAll,bt,et) > 0 ){
          for( j = ttAll; j >= 0; j-- ){
            if( getTraf(TrafType(j),bt,et) == 0 ) continue;
            f <<
              "  <TH ALIGN=right BGCOLOR=\"" <<
	      trafTypeTailDataColor_[j] << "\" nowrap>\n"
              "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
            ;
            if( j == ttAll ){
              writeTraf(f,
                getTraf(TrafType(j),bt,et),
                getTraf(ttAll,beginTime,endTime)
              );
            }
            else {
              writeTraf(f,
                getTraf(TrafType(j),bt,et),
                getTraf(ttAll,bt,et)
              );
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
      utf8::String fileName(ksys::includeTrailingPathDelimiter(dir) + trafByYearFile);
      writeMonthHtmlOutput(fileName,endTime);
    }
    endTime.tm_year--;
    beginTime = beginTime2;
  }
  f << "Ellapsed time: " <<
    utf8::elapsedTime2Str(gettimeofday() - ellapsed_) << "\n<BR>\n" <<
    utf8::int2Str(trafCache_.count()) << "<BR>\n";
  writeHtmlTail(f);
}
//------------------------------------------------------------------------------
uintptr_t Logger::nonZeroYearMonthsColumns(struct tm byear)
{
  byear.tm_mday = 1;
  byear.tm_hour = byear.tm_min = byear.tm_sec = 0;
  uintptr_t a = 0;
  while( byear.tm_mon >= 0 ){
    struct tm eyear = byear;
    eyear.tm_mday = ksys::monthDays(byear.tm_year + 1900,byear.tm_mon);
    eyear.tm_hour = 23;
    eyear.tm_min = 59;
    eyear.tm_sec = 59;
    a += getTraf(ttAll,byear,eyear) > 0;
    a += getTraf(ttWWW,byear,eyear) > 0;
    a += getTraf(ttSMTP,byear,eyear) > 0;
    byear.tm_mon--;
  }
  return a;
}
//------------------------------------------------------------------------------
uintptr_t Logger::nonZeroMonthDaysColumns(struct tm bmon)
{
  bmon.tm_mday = ksys::monthDays(bmon.tm_year + 1900,bmon.tm_mon);
  bmon.tm_hour = bmon.tm_min = bmon.tm_sec = 0;
  uintptr_t a = 0;
  while( bmon.tm_mday > 0 ){
    struct tm emon = bmon;
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
intptr_t Logger::sortUsersTrafTable(uintptr_t row1,uintptr_t row2,const ksys::Table<ksys::Mutant> & table)
{
  int64_t t1 = table(row1,utf8::string("ST_TRAF")), t2 = table(row2,utf8::string("ST_TRAF"));
  intptr_t c = t1 > t2 ? 1 : t1 < t2 ? -1 : 0;
  if( c == 0 )
    c = utf8::String(table(row1,utf8::string("ST_USER"))).strcasecmp(table(row2,utf8::string("ST_USER")));
  return c;
}
//------------------------------------------------------------------------------
void Logger::writeTraf(ksys::FileHandleContainer & f,uint64_t qi,uint64_t qj)
{
  uint64_t q, a = qi % 1024u != 0, b, c;

  qj += qj == 0;
  q = qi * 10000u / qj;
  b = q / 100u;
  c = q % 100u;
  c += b == 0 && c == 0;
  f << utf8::String::print(
    qi > 0 ? "%"PRIu64"<FONT FACE=\"Times New Roman\" SIZE=\"0\"> (%"PRIu64".%02"PRIu64"%%)</FONT>\n" : "-",
    (qi / 1024u) + a, b, c
  );
}
//------------------------------------------------------------------------------
utf8::String Logger::TrafCacheEntry::id() const
{
  return
    user_ +
    utf8::time2Str(ksys::tm2Time(bt_)) +
    utf8::time2Str(ksys::tm2Time(et_)) + trafTypeColumnName[trafType_]
  ;
}
//------------------------------------------------------------------------------
void Logger::writeHtmlHead(ksys::FileHandleContainer & f)
{
  f << 
    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
    "<HTML>\n"
    "<HEAD>\n"
//    "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"600\">\n"
//    "<META HTTP-EQUIV=\"Pragma\" CONTENT=\"no-cache\">\n"
//    "<META HTTP-EQUIV=\"Cache-Control\" content=\"no-cache\">\n"
    "<TITLE>Statistics by user</TITLE>\n"
    "</HEAD>\n"
    "<BODY lang=EN BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#0000FF\" VLINK=\"#FF0000\">\n"
  ;
}
//------------------------------------------------------------------------------
void Logger::writeHtmlTail(ksys::FileHandleContainer & f)
{
#if HAVE_UNAME
  struct utsname un;
  uname(&un);
  f << un.nodename << "\n<BR>\n";
#endif
  f << "</BODY>\n" "</HTML>\n";
}
//------------------------------------------------------------------------------
int64_t Logger::getTraf(
  TrafType tt,const struct tm & bt,const struct tm & et,const utf8::String & user)
{
  ksys::AutoPtr<TrafCacheEntry> tce(new TrafCacheEntry(user,bt,et,tt));
  tce->bt_.tm_wday = 0;
  tce->bt_.tm_yday = 0;
  tce->et_.tm_wday = 0;
  tce->et_.tm_yday = 0;
  ksys::HashedObjectListItem<utf8::String,TrafCacheEntry> * item;
  item = trafCache_.itemOfKey(tce->id());
  if( item == NULL ){
    statement_->text(
      utf8::string("SELECT ") +
      (tt == ttAll || tt == ttWWW ? "SUM(ST_TRAF_WWW)" : "") +
      (tt == ttAll ? "," : "") +
      (tt == ttAll || tt == ttSMTP ? "SUM(ST_TRAF_SMTP)" : "") +
      "FROM INET_USERS_TRAF WHERE " +
      (user.strlen() > 0 ? "ST_USER = :ST_USER AND " : "") +
      "ST_TIMESTAMP >= :BT AND ST_TIMESTAMP <= :ET"
    );
    statement_->prepare();
    if( user.strlen() > 0 )
      statement_->paramAsString(utf8::string("ST_USER"),user);
    statement_->paramAsMutant(utf8::string("BT"),bt)->paramAsMutant(utf8::string("ET"),et);
    statement_->execute()->fetchAll();
    switch( tt ){
      case ttSMTP :
      case ttWWW  :
        tce->traf_ = statement_->valueAsMutant(utf8::string("SUM"));
        break;
      case ttAll  :
        tce->traf_ = (int64_t) statement_->valueAsMutant(utf8::string("SUM")) +
                     (int64_t) statement_->valueAsMutant(utf8::string("SUM_1"));
        break;
      default     : break;
    }
//    if( trafCache_.count() >= cacheSize_ )
//      trafCache_.removeByIndex(trafCache_.count() - 1);
    trafCache_.add(tce.ptr(),tce->id(),&item);
    tce.ptr(NULL);
  }
  else {
//    trafCache_.changeIndex(item->index_,0);
  }
  return item->object_->traf_;
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
