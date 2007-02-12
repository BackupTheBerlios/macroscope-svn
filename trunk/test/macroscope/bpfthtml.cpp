/*-
 * Copyright 2007 Guram Dukashvili
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
#include "bpft.h"
//------------------------------------------------------------------------------
namespace macroscope {
//------------------------------------------------------------------------------
static inline bool isAddressMask(uint32_t ip)
{
  return (ip & 0x80000000) != 0;
}
//------------------------------------------------------------------------------
static inline bool isMulticastAddress(uint32_t ip)
{
  return ((ip & 0xf0000000) == 0xe0000000);
}
//------------------------------------------------------------------------------
static inline bool isSiteLocalAddress(uint32_t ip)
{
// refer to RFC 1918
// 10/8 prefix
// 172.16/12 prefix
// 192.168/16 prefix
  return (((ip >> 24) & 0xFF) == 10)
      || ((((ip >> 24) & 0xFF) == 172)
      && (((ip >> 16) & 0xF0) == 16))
      || ((((ip >> 24) & 0xFF) == 192)
      && (((ip >> 16) & 0xFF) == 168));
}
//------------------------------------------------------------------------------
static inline bool isMCGlobal(uint32_t ip)
{
  union {
    uint8_t byteAddr[4];
    uint32_t a;
  };
  a = ip;
// 224.0.1.0 to 238.255.255.255
  return ((byteAddr[0] & 0xff) >= 224 && (byteAddr[0] & 0xff) <= 238 ) &&
        !((byteAddr[0] & 0xff) == 224 && byteAddr[1] == 0 && byteAddr[2] == 0);
}
//------------------------------------------------------------------------------
static inline bool isMCLinkLocal(uint32_t ip)
{
// 224.0.0/24 prefix and ttl == 1
  return (((ip >> 24) & 0xFF) == 224)
      && (((ip >> 16) & 0xFF) == 0)
      && (((ip >> 8) & 0xFF) == 0);
}
//------------------------------------------------------------------------------
utf8::String Logger::ip4AddrToIndex(uint32_t ip4)
{
  AutoPtr<char> b;
  b.alloc(sizeof(ip4) * 2 + 1);
  uintptr_t shift = sizeof(ip4) * 8 - 4, i;
  for( i = 0; i < sizeof(ip4) * 2; i++, shift -= 4 ) b[i] = "0123456789ABCDEF"[(ip4 >> shift) & 0xF];
  b[i] = '\0';
  return utf8::plane0(b);
}
//------------------------------------------------------------------------------
uint32_t Logger::indexToIp4Addr(const utf8::String & index)
{
  static const uint8_t m[2] = { '0', 'A' };
  uint32_t ip4 = 0, a;
  uintptr_t shift = sizeof(ip4) * 8 - 4, i;
  for( i = 0; i < sizeof(ip4) * 2; i++, shift -= 4 ){
    a = index.c_str()[i];
    a -= m[a > '9'];
    ip4 |= a << shift;
  }
  return ip4;
}
//------------------------------------------------------------------------------
utf8::String Logger::formatTraf(uintmax_t traf,uintmax_t allTraf)
{
  uintmax_t q, b, c, t1, t2, t3;
  char * postfix;

  q = traf * 10000u / allTraf;
  b = q / 100u;
  c = q % 100u;
  if( traf >= 1024u * 1024u * 1024u ){
    t2 = 1024u * 1024u * 1024u;
    postfix = "G";
  }
  else if( traf >= uintmax_t(1024u) * 1024u * 1024u * 1024u ){
    t2 = uintmax_t(1024u) * 1024u * 1024u * 1024u;
    postfix = "T";
  }
  else if( traf >= 1024u * 1024u ){
    t2 = 1024u * 1024u;
    postfix = "M";
  }
  else if( traf >= 1024u ){
    t2 = 1024u;
    postfix = "K";
  }
  else {
    return utf8::String::print(
      traf > 0 ? "%"PRIuMAX"<FONT FACE=\"Times New Roman\" SIZE=\"0\"> (%"PRIuMAX".%02"PRIuMAX"%%)</FONT>" :  "-",
      traf,
      b,
      c
    );
  }
  t1 = traf / t2;
  t3 = traf % t2;
  ldouble mantissa = t3 / (t2 / 10000.);
  return utf8::String::print(
    traf > 0 ? "%"PRIuMAX".%04"PRIuMAX"%s<FONT FACE=\"Times New Roman\" SIZE=\"0\"> (%"PRIuMAX".%02"PRIuMAX"%%)</FONT>" :  "-",
    t1,
    uintmax_t(mantissa),
    postfix,
    b,
    c
  );
}
//------------------------------------------------------------------------------
utf8::String Logger::resolveAddr(uint32_t ip4,bool numeric)
{
  AutoPtr<DNSCacheEntry> addr(newObject<DNSCacheEntry>());
  addr->addr4_.sin_addr.s_addr = ip4;
  addr->addr4_.sin_port = 0;
  addr->addr4_.sin_family = PF_INET;
  if( numeric ) return addr->resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV);
  DNSCacheEntry * pAddr = addr;
  dnsCache_.insert(addr,false,false,&pAddr);
  if( pAddr == addr ){
    if( resolveDNSNames_ ){
      /*if( statement2_->paramAsMutant("ip",ip4AddrToIndex(ip4))->execute()->fetch() ){
        addr.ptr(NULL)->name_ = statement2_->valueAsString(
          (uint32_t) statement2_->valueAsMutant("st_src_ip") == ip4 ? "st_src_name" : "st_dst_name"
        );
      }
      else {*/
        pAddr->name_ = pAddr->resolveAddr();
	addr.ptr(NULL);
	/*statement3_->paramAsMutant("name",pAddr->name_)->paramAsMutant("ip",ip4AddrToIndex(ip4))->execute();
	statement4_->paramAsMutant("name",pAddr->name_)->paramAsMutant("ip",ip4AddrToIndex(ip4))->execute();
      }*/
    }
    else {
      pAddr->name_ = pAddr->resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV);
      addr.ptr(NULL);
    }
    if( dnsCacheSize_ > 0 && dnsCache_.count() >= dnsCacheSize_ )
      dnsCache_.drop(dnsCacheLRU_.remove(*dnsCacheLRU_.last()));
  }
  else {
    dnsCacheLRU_.remove(*pAddr);
  }
  dnsCacheLRU_.insToHead(*pAddr);
  return pAddr->name_;
}
//------------------------------------------------------------------------------
void Logger::writeBPFTMonthHtmlReport(const struct tm & year)
{
  struct tm beginTime, beginTime2, endTime;
  beginTime = endTime = year;
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
  AsyncFile f(
    includeTrailingPathDelimiter(htmlDir_) + utf8::String::print("bpft-traf-by-%04d.html",year.tm_year + 1900)
  );
  f.createIfNotExist(true).open();
#ifndef NDEBUG
  f.resize(0);
#endif
  Mutant m0(config_->valueByPath(section_ + "html_report.file_mode",755));
  Mutant m1(config_->valueByPath(section_ + "html_report.file_user",ksys::getuid()));
  Mutant m2(config_->valueByPath(section_ + "html_report.file_group",ksys::getgid()));
  chModOwn(f.fileName(),m0,m1,m2);
  writeHtmlHead(f);
  while( tm2Time(endTime) >= tm2Time(beginTime) ){
    beginTime2 = beginTime;
    beginTime.tm_mon = endTime.tm_mon;
    statement_->paramAsMutant("BT",time2tm(tm2Time(beginTime) + getgmtoffset()));
    statement_->paramAsMutant("ET",time2tm(tm2Time(endTime) + getgmtoffset()));
    statement_->paramAsMutant("gateway",ip4AddrToIndex(gateway_.addr4_.sin_addr.s_addr));
    statement_->execute()->fetchAll();
    if( (uintmax_t) statement_->sum("SUM1") > 0 ){
      if( verbose_ ) fprintf(stderr,"%s %s\n",
        (const char *) utf8::tm2Str(beginTime).getOEMString(),
        (const char *) utf8::tm2Str(endTime).getOEMString()
      );
      Vector<Table<Mutant> > table;
      table.resize(32);
      statement_->unload(table[0]);
      uintptr_t dayCount = 0;
      while( endTime.tm_mday > 0 ){
        beginTime.tm_mday = endTime.tm_mday;
        statement_->paramAsMutant("BT",time2tm(tm2Time(beginTime) + getgmtoffset()));
        statement_->paramAsMutant("ET",time2tm(tm2Time(endTime) + getgmtoffset()));
        statement_->paramAsMutant("gateway",ip4AddrToIndex(gateway_.addr4_.sin_addr.s_addr));
        statement_->execute()->fetchAll();
        statement_->unload(table[beginTime.tm_mday]);
        dayCount += (uintmax_t) table[beginTime.tm_mday].sum("SUM1") > 0;
        endTime.tm_mday--;
      }
      endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
      f <<
        "<TABLE WIDTH=400 BORDER=1 CELLSPACING=0 CELLPADDING=2>\n"
        "<TR>\n"
        "  <TH BGCOLOR=\"#00A0FF\" COLSPAN=\"" + utf8::int2Str(dayCount * 2 + 3) + "\" ALIGN=left nowrap>\n" <<
        "    <A HREF=\"" << utf8::String::print("bpft-traf-by-%04d%02d.html",endTime.tm_year + 1900,endTime.tm_mon + 1) << "\">" <<
             utf8::String::print("%02d.%04d",endTime.tm_mon + 1,endTime.tm_year + 1900) << "\n"
        "    </A>\n"
        "  </TH>\n"
        "</TR>\n"
        "<TR>\n"
        "  <TH HEIGHT=4>"
        "  </TH>\n"
        "</TR>\n"
        "<TR>\n"
        "  <TH ALIGN=center BGCOLOR=\"#00A0FF\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
        "      Host\n"
        "    </FONT>\n"
        "  </TH>\n"
        "  <TH ALIGN=center BGCOLOR=\"#00A0FF\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
        "      Data bytes\n"
        "    </FONT>\n"
        "  </TH>\n"
        "  <TH ALIGN=center BGCOLOR=\"#00A0FF\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
        "      Datagram bytes\n"
        "    </FONT>\n"
        "  </TH>\n"
      ;
      while( endTime.tm_mday > 0 ){
        if( (uintmax_t) table[endTime.tm_mday].sum("SUM1") > 0 )
	  f <<
            "  <TH COLSPAN=2 ALIGN=center BGCOLOR=\"#00A0FF\" nowrap>\n"
            "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
            "      " + utf8::int2Str(endTime.tm_mday) + "\n"
            "    </FONT>\n"
            "  </TH>\n"
	  ;
        endTime.tm_mday--;
      }
      f <<
        "<TR>\n"
      ;
      endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
      for( intptr_t i = table[0].rowCount() - 1; i >= 0; i-- ){
        if( (uintmax_t) table[0](i,"SUM1") < minSignificantThreshold_ ) continue;
        uint32_t ip4 = indexToIp4Addr(table[0](i,"st_ip"));
        f <<
          "<TR>\n"
  	  "  <TH ALIGN=left BGCOLOR=\"#00E0FF\" nowrap>\n"
	  "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
          "      <A HREF=\"http://" << resolveAddr(ip4) << "\">\n" <<
          resolveAddr(ip4) << "\n"
          "      </A>\n"
          "    </FONT>\n"
          "  </TH>\n"
	  "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
	  "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
          formatTraf(table[0](i,"SUM2"),table[0].sum("SUM1")) << "\n"
          "    </FONT>\n"
          "  </TH>\n"
	  "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
	  "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
          formatTraf(table[0](i,"SUM1"),table[0].sum("SUM1")) << "\n"
          "    </FONT>\n"
          "  </TH>\n"
        ;
        while( endTime.tm_mday > 0 ){
          beginTime.tm_mday = endTime.tm_mday;
          if( (uintmax_t) table[endTime.tm_mday].sum("SUM1") > 0 ){
            statement6_->paramAsMutant("BT",time2tm(tm2Time(beginTime) + getgmtoffset()));
            statement6_->paramAsMutant("ET",time2tm(tm2Time(endTime) + getgmtoffset()));
            statement6_->paramAsMutant("gateway",ip4AddrToIndex(gateway_.addr4_.sin_addr.s_addr));
            statement6_->paramAsMutant("host",table[0](i,"st_ip"));
	    uintmax_t sum1 = 0, sum2 = 0;
            if( statement6_->execute()->fetch() ){
              statement6_->fetchAll();
	      sum1 = statement6_->valueAsMutant("SUM1");
	      sum2 = statement6_->valueAsMutant("SUM2");
	    }
            f <<
  	      "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
	      "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
              formatTraf(sum2,table[endTime.tm_mday].sum("SUM1")) << "\n"
              "    </FONT>\n"
              "  </TH>\n"
	      "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
	      "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
              formatTraf(sum1,table[endTime.tm_mday].sum("SUM1")) << "\n"
              "    </FONT>\n"
              "  </TH>\n"
            ;
	  }
          endTime.tm_mday--;
        }
        f <<
          "</TR>\n"
        ;
        endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
      }
      f <<
        "<TR>\n"
        "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
        "      Summary:\n"
        "    </FONT>\n"
        "  </TH>\n"
        "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
        formatTraf(table[0].sum("SUM2"),table[0].sum("SUM1")) << "\n"
        "    </FONT>\n"
        "  </TH>\n"
        "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
        formatTraf(table[0].sum("SUM1"),table[0].sum("SUM1")) << "\n"
        "    </FONT>\n"
        "  </TH>\n"
      ;
      while( endTime.tm_mday > 0 ){
        if( (uintmax_t) table[endTime.tm_mday].sum("SUM1") > 0 )
          f <<
            "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
            "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
            formatTraf(table[endTime.tm_mday].sum("SUM2"),table[0].sum("SUM1")) << "\n"
            "    </FONT>\n"
            "  </TH>\n"
            "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
            "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
            formatTraf(table[endTime.tm_mday].sum("SUM1"),table[0].sum("SUM1")) << "\n"
            "    </FONT>\n"
            "  </TH>\n"
          ;
        endTime.tm_mday--;
      }
      f <<
        "</TR>\n"
        "</TABLE>\n<BR>\n<BR>\n"
      ;
      endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
    }
    if( endTime.tm_mon == 0 ){
      endTime.tm_mon = 11;
      endTime.tm_year--;
    }
    else {
      endTime.tm_mon--;
    }
    endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
    beginTime = beginTime2;
  }
  writeHtmlTail(f);
  f.resize(f.tell());
}
//------------------------------------------------------------------------------
void Logger::writeBPFTHtmlReport()
{
/*!
 * \TODO По ип сортировать по отношению к адресу нашего шлюза
 * \TODO далее детализация по портам, протоколу
 */
  if( !(bool) config_->valueByPath(section_ + "html_report.enabled",true) ) return;
  minSignificantThreshold_ = config_->valueByPath(section_ + "html_report.min_significant_threshold",0);
  gateway_.resolveName(config_->valueByPath(section_ + "gateway"));
  if( verbose_ ) fprintf(stderr,"\ngateway resolved as %s, in db %"PRIuMAX"\n",
    (const char *) gateway_.resolveAddr(0,NI_NUMERICHOST | NI_NUMERICSERV).getOEMString(),
    uintmax_t(gateway_.addr4_.sin_addr.s_addr)
  );
  struct tm beginTime, beginTime2, endTime;
  statement_->text(
    utf8::String("select ") +
    (dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL ? "FIRST 1 " : "") +
    "st_start FROM INET_BPFT_STAT" +
    (dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL ? " LIMIT 0,1" : "")
  )->execute()->fetchAll();
  beginTime = time2tm((uint64_t) statement_->valueAsMutant("st_start") - getgmtoffset());
  statement_->text(
    utf8::String("select ") +
    (dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL ? "FIRST 1 " : "") +
    "st_start FROM INET_BPFT_STAT ORDER BY st_start DESC" +
    (dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL ? " LIMIT 0,1" : "")
  )->execute()->fetchAll();
  endTime = time2tm((uint64_t) statement_->valueAsMutant("st_start") - getgmtoffset());
  statement_->text(
    "SELECT"
    "  A.*"
    "FROM ("
    "  SELECT"
    "    B.st_ip AS st_ip, SUM(B.SUM1) AS SUM1, SUM(B.SUM2) AS SUM2"
    "  FROM ("
    "      SELECT"
    "        st_dst_ip AS st_ip, SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2"
    "      FROM"
    "        INET_BPFT_STAT"
    "      WHERE "
    "        st_start >= :BT AND st_start <= :ET AND"
    "        st_src_ip = :gateway"
    "      GROUP BY st_dst_ip"
    "    UNION ALL"
    "      SELECT"
    "        st_src_ip AS st_ip, SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2"
    "      FROM"
    "        INET_BPFT_STAT"
    "      WHERE "
    "        st_start >= :BT AND st_start <= :ET AND"
    "        st_dst_ip = :gateway"
    "      GROUP BY st_src_ip"
    "  ) AS B "
    "  GROUP BY B.st_ip"
    ") AS A "
    "ORDER BY A.SUM1"
  );
  statement_->prepare();
  statement5_->text(statement_->text());
  statement5_->prepare();
  statement6_->text(
    "  SELECT"
    "    B.st_ip AS st_ip, SUM(B.SUM1) AS SUM1, SUM(B.SUM2) AS SUM2"
    "  FROM ("
    "      SELECT"
    "        st_dst_ip AS st_ip, SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2"
    "      FROM"
    "        INET_BPFT_STAT"
    "      WHERE "
    "        st_start >= :BT AND st_start <= :ET AND"
    "        st_src_ip = :gateway AND"
    "        st_dst_ip = :host"
    "      GROUP BY st_dst_ip"
    "    UNION ALL"
    "      SELECT"
    "        st_src_ip AS st_ip, SUM(st_dgram_bytes) AS SUM1, SUM(st_data_bytes) AS SUM2"
    "      FROM"
    "        INET_BPFT_STAT"
    "      WHERE "
    "        st_start >= :BT AND st_start <= :ET AND"
    "        st_dst_ip = :gateway AND"
    "        st_src_ip = :host"
    "      GROUP BY st_src_ip"
    "  ) AS B "
    "  GROUP BY B.st_ip"
  );
  statement6_->prepare();
  database_->start();
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
  htmlDir_ = excludeTrailingPathDelimiter(config_->valueByPath(section_ + "html_report.directory"));
  Mutant m0(config_->valueByPath(section_ + "html_report.directory_mode",755));
  Mutant m1(config_->valueByPath(section_ + "html_report.directory_user",ksys::getuid()));
  Mutant m2(config_->valueByPath(section_ + "html_report.directory_group",ksys::getgid()));
  AsyncFile f(
    includeTrailingPathDelimiter(htmlDir_) + config_->valueByPath(section_ + "html_report.index_file_name","index.html")
  );
  f.createIfNotExist(true).open();
#ifndef NDEBUG
  f.resize(0);
#endif
  chModOwn(htmlDir_,m0,m1,m2);
  m0 = config_->valueByPath(section_ + "html_report.file_mode",755);
  m1 = config_->valueByPath(section_ + "html_report.file_user",ksys::getuid());
  m2 = config_->valueByPath(section_ + "html_report.file_group",ksys::getgid());
  chModOwn(f.fileName(),m0,m1,m2);
  writeHtmlHead(f);
  while( tm2Time(endTime) >= tm2Time(beginTime) ){
    beginTime2 = beginTime;
    beginTime.tm_year = endTime.tm_year;
    statement_->paramAsMutant("BT",time2tm(tm2Time(beginTime) + getgmtoffset()));
    statement_->paramAsMutant("ET",time2tm(tm2Time(endTime) + getgmtoffset()));
    statement_->paramAsMutant("gateway",ip4AddrToIndex(gateway_.addr4_.sin_addr.s_addr));
    statement_->execute()->fetchAll();
    if( (uintmax_t) statement_->sum("SUM1") > 0 ){
      if( verbose_ ) fprintf(stderr,"%s %s\n",
        (const char *) utf8::tm2Str(beginTime).getOEMString(),
        (const char *) utf8::tm2Str(endTime).getOEMString()
      );
      Vector<Table<Mutant> > table;
      table.resize(13);
      statement_->unload(table[0]);
      uintptr_t monCount = 0;
      while( endTime.tm_mon >= 0 ){
        endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
        beginTime.tm_mon = endTime.tm_mon;
        statement_->paramAsMutant("BT",time2tm(tm2Time(beginTime) + getgmtoffset()));
        statement_->paramAsMutant("ET",time2tm(tm2Time(endTime) + getgmtoffset()));
        statement_->paramAsMutant("gateway",ip4AddrToIndex(gateway_.addr4_.sin_addr.s_addr));
        statement_->execute()->fetchAll();
        statement_->unload(table[beginTime.tm_mon + 1]);
        monCount += (uintmax_t) table[beginTime.tm_mon + 1].sum("SUM1") > 0;
        endTime.tm_mon--;
      }
      endTime.tm_mon = 11;
      endTime.tm_mday = 31;
      f <<
/*      <SCRIPT LANGUAGE="JavaScript">
<!--
function DNSQuery(name)
{
  this.name = name;
    this.resolveAddr = resolveAddr;
    }
    function resolveAddr(obj)
    {
      obj.value = 'yandex.ru';
        //document.write("<B>This is written from JavaScript</B>")
	}
	-->
	</SCRIPT>
	<FORM>
	<INPUT TYPE=\"text\" name="rn0" onMouseOver="resolveAddr(this.form.rn0)" value="87.250.251.11">
	</FORM>*/
        "<TABLE WIDTH=400 BORDER=1 CELLSPACING=0 CELLPADDING=2>\n"
        "<TR>\n"
        "  <TH BGCOLOR=\"#00A0FF\" COLSPAN=\"" + utf8::int2Str(monCount * 2 + 3) + "\" ALIGN=left nowrap>\n" <<
        "    <A HREF=\"" << utf8::String::print("bpft-traf-by-%04d.html",endTime.tm_year + 1900) << "\">" <<
             utf8::int2Str(endTime.tm_year + 1900) << "\n"
        "    </A>\n"
        "  </TH>\n"
        "</TR>\n"
        "<TR>\n"
        "  <TH HEIGHT=4>"
        "  </TH>\n"
        "</TR>\n"
        "<TR>\n"
        "  <TH ALIGN=center BGCOLOR=\"#00A0FF\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
        "      Host\n"
        "    </FONT>\n"
        "  </TH>\n"
        "  <TH ALIGN=center BGCOLOR=\"#00A0FF\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
        "      Data bytes\n"
        "    </FONT>\n"
        "  </TH>\n"
        "  <TH ALIGN=center BGCOLOR=\"#00A0FF\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
        "      Datagram bytes\n"
        "    </FONT>\n"
        "  </TH>\n"
      ;
      while( endTime.tm_mon >= 0 ){
        if( (uintmax_t) table[endTime.tm_mon + 1].sum("SUM1") > 0 )
	  f <<
            "  <TH COLSPAN=2 ALIGN=center BGCOLOR=\"#00A0FF\" nowrap>\n"
            "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
            "      " + utf8::int2Str(endTime.tm_mon + 1) + "\n"
            "    </FONT>\n"
            "  </TH>\n"
	  ;
        endTime.tm_mon--;
      }
      endTime.tm_mon = 11;
      for( intptr_t i = table[0].rowCount() - 1; i >= 0; i-- ){
        if( (uintmax_t) table[0](i,"SUM1") < minSignificantThreshold_ ) continue;
        uint32_t ip4 = indexToIp4Addr(table[0](i,"st_ip"));
        f <<
          "<TR>\n"
  	  "  <TH ALIGN=left BGCOLOR=\"#00E0FF\" nowrap>\n"
	  "    <FONT FACE=\"Arial\" SIZE=\"2\">\n"
          "      <A HREF=\"http://" << resolveAddr(ip4) << "\">\n" <<
          resolveAddr(ip4) << "\n"
          "      </A>\n"
          "    </FONT>\n"
//        "    <OBJECT classid=\"java:DNSQuery.class\" type=\"application/x-java-applet\" alt=\"" <<
//	resolveAddr(table(i,"st_ip"),true) <<
//	"\" height=\"20\" width=\"150\">\n"
//        "      <PARAM name=\"name\" value=\"" << resolveAddr(table(i,"st_ip"),true) << "\">\n"
//        "    </OBJECT>\n"
          "  </TH>\n"
	  "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
	  "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
          formatTraf(table[0](i,"SUM2"),table[0].sum("SUM1")) << "\n"
          "    </FONT>\n"
          "  </TH>\n"
	  "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
	  "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
          formatTraf(table[0](i,"SUM1"),table[0].sum("SUM1")) << "\n"
          "    </FONT>\n"
          "  </TH>\n"
        ;
        while( endTime.tm_mon >= 0 ){
          endTime.tm_mday = (int) monthDays(endTime.tm_year + 1900,endTime.tm_mon);
          beginTime.tm_mon = endTime.tm_mon;
          if( (uintmax_t) table[endTime.tm_mon + 1].sum("SUM1") > 0 ){
            statement6_->paramAsMutant("BT",time2tm(tm2Time(beginTime) + getgmtoffset()));
            statement6_->paramAsMutant("ET",time2tm(tm2Time(endTime) + getgmtoffset()));
            statement6_->paramAsMutant("gateway",ip4AddrToIndex(gateway_.addr4_.sin_addr.s_addr));
            statement6_->paramAsMutant("host",table[0](i,"st_ip"));
	    uintmax_t sum1 = 0, sum2 = 0;
            if( statement6_->execute()->fetch() ){
              statement6_->fetchAll();
	      sum1 = statement6_->valueAsMutant("SUM1");
	      sum2 = statement6_->valueAsMutant("SUM2");
	    }
            f <<
  	      "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
	      "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
              formatTraf(sum2,table[endTime.tm_mon + 1].sum("SUM1")) << "\n"
              "    </FONT>\n"
              "  </TH>\n"
	      "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
	      "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
              formatTraf(sum1,table[endTime.tm_mon + 1].sum("SUM1")) << "\n"
              "    </FONT>\n"
              "  </TH>\n"
            ;
	  }
          endTime.tm_mon--;
        }
        f <<
          "</TR>\n"
        ;
        endTime.tm_mon = 11;
        endTime.tm_mday = 31;
      }
      f <<
        "<TR>\n"
        "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
        "      Summary:\n"
        "    </FONT>\n"
        "  </TH>\n"
        "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
        formatTraf(table[0].sum("SUM2"),table[0].sum("SUM1")) << "\n"
        "    </FONT>\n"
        "  </TH>\n"
        "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
        "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
        formatTraf(table[0].sum("SUM1"),table[0].sum("SUM1")) << "\n"
        "    </FONT>\n"
        "  </TH>\n"
      ;
      while( endTime.tm_mon >= 0 ){
        if( (uintmax_t) table[endTime.tm_mon + 1].sum("SUM1") > 0 )
          f <<
            "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
            "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
            formatTraf(table[endTime.tm_mon + 1].sum("SUM2"),table[0].sum("SUM1")) << "\n"
            "    </FONT>\n"
            "  </TH>\n"
            "  <TH ALIGN=right BGCOLOR=\"#00E0FF\" nowrap>\n"
            "    <FONT FACE=\"Arial\" SIZE=\"2\">\n" <<
            formatTraf(table[endTime.tm_mon + 1].sum("SUM1"),table[0].sum("SUM1")) << "\n"
            "    </FONT>\n"
            "  </TH>\n"
          ;
        endTime.tm_mon--;
      }
      f <<
        "</TR>\n"
        "</TABLE>\n<BR>\n<BR>\n"
      ;
      endTime.tm_mon = 11;
      endTime.tm_mday = 31;
      writeBPFTMonthHtmlReport(endTime);
    }
    endTime.tm_year--;
    beginTime = beginTime2;
  }	  
  database_->commit();
  f <<
    "Ellapsed time: " + utf8::elapsedTime2Str(uintmax_t(getlocaltimeofday() - ellapsed_)) + "\n<BR>\n"
  ;
  writeHtmlTail(f);
  f.resize(f.tell());
  dnsCache_.drop();
}
//------------------------------------------------------------------------------
void Logger::parseBPFTLogFile()
{
  resolveDNSNames_ = config_->valueByPath(section_ + "resolve_dns_names",false);
  bpftOnlyCurrentYear_ = config_->valueByPath(section_ + "only_current_year",false);
/*  statement2_->text(
    utf8::String("select ") +
    (dynamic_cast<FirebirdDatabase *>(database_.ptr()) != NULL ? "FIRST 1 " : "") +
    "st_src_name, st_dst_name FROM INET_BPFT_STAT where st_src_ip = :ip OR st_dst_ip = :ip" +
    (dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL ? " LIMIT 0,1" : "")
  )->prepare();
  statement3_->text(
    "update INET_BPFT_STAT set st_src_name = :name where st_src_ip = :ip"
  )->prepare();
  statement4_->text(
    "update INET_BPFT_STAT set st_dst_name = :name where st_dst_ip = :ip"
  )->prepare();*/
  dnsCacheSize_ = config_->valueByPath(section_ + "dns_cache_size",0);
  AsyncFile flog(config_->valueByPath(section_ + "log_file_name"));
  /*statement_->text("DELETE FROM INET_BPFT_STAT")->execute();
  stFileStatUpd_->prepare()->
    paramAsString("ST_LOG_FILE_NAME",flog.fileName())->
    paramAsMutant("ST_LAST_OFFSET",0)->execute();*/
  flog.readOnly(true).open();
  database_->start();
  int64_t offset = fetchLogFileLastOffset(flog.fileName());
  flog.seek(offset);
  int64_t lineNo = 0, tma = 0;
  int64_t cl = getlocaltimeofday();
  union {
    BPFTHeader header;
    BPFTHeader32 header32;
  };
  statement_->text(
    "INSERT INTO INET_BPFT_STAT ("
    "  st_start,st_src_ip,st_dst_ip,st_ip_proto,st_src_port,st_dst_port,st_dgram_bytes,st_data_bytes"
    ") VALUES ("
    "  :st_start,:st_src_ip,:st_dst_ip,:st_ip_proto,:st_src_port,:st_dst_port,:st_dgram_bytes,:st_data_bytes"
    ")"
  );
  statement_->prepare();
  bool log32bitOsCompatible = config_->valueByPath(section_ + "log_32bit_os_compatible",SIZEOF_VOID_P < 8);
  struct tm start, stop, curTime = time2tm(gettimeofday());
  ksock::APIAutoInitializer ksockAPIAutoInitializer;
  for(;;){
    uintptr_t entriesCount;
    uint64_t safePos = flog.tell();
    if( log32bitOsCompatible ){
      if( flog.read(&header32,sizeof(header32)) != sizeof(header32) ){
        flog.seek(safePos);
        break;
      }
      start = timeval2tm(header32.start_);
      stop = timeval2tm(header32.stop_);
      entriesCount = header32.eCount_;
    }
    else {
      if( flog.read(&header,sizeof(header)) != sizeof(header) ){
        flog.seek(safePos);
        break;
      }
      start = timeval2tm(header.start_);
      stop = timeval2tm(header.stop_);
      entriesCount = header.eCount_;
    }
    Array<BPFTEntry> entries;
    Array<BPFTEntry32> entries32;
    if( log32bitOsCompatible ){
      if( bpftOnlyCurrentYear_ && start.tm_year != curTime.tm_year ){
        flog.seek(flog.tell() + sizeof(BPFTEntry32) * entriesCount);
        lineNo += entriesCount;
        entriesCount = 0;
      }
      else {
        entries32.resize(entriesCount);
        if( flog.read(entries32,sizeof(BPFTEntry32) * entriesCount) != int64_t(sizeof(BPFTEntry32) * entriesCount) ){
          flog.seek(safePos);
          break;
        }
      }
    }
    else {
      if( bpftOnlyCurrentYear_ && start.tm_year != curTime.tm_year ){
        flog.seek(flog.tell() + sizeof(BPFTEntry) * entriesCount);
        lineNo += entriesCount;
        entriesCount = 0;
      }
      else {
        entries.resize(entriesCount);
        if( flog.read(entries,sizeof(BPFTEntry) * entriesCount) != int64_t(sizeof(BPFTEntry) * entriesCount) ){
          flog.seek(safePos);
          break;
        }
      }
    }
    if( dynamic_cast<MYSQLDatabase *>(database_.ptr()) != NULL && entriesCount > 0 ){
      bool executed = true;
      for( intptr_t i = entriesCount - 1; i >= 0; i-- ){
        if( executed ){
	   statement2_->text(
            "INSERT INTO INET_BPFT_STAT ("
            "  st_start,st_src_ip,st_dst_ip,st_ip_proto,st_src_port,st_dst_port,st_dgram_bytes,st_data_bytes"
            ") VALUES "
          );
	   executed = false;
	}
        statement2_->text(statement2_->text() + "(");
        if( log32bitOsCompatible ){
	  statement2_->text(statement2_->text() +
            "'" + mycpp::tm2Str(start) + "'," +
	    "'" + ip4AddrToIndex(entries32[i].srcIp_.s_addr) + "'," +
            "'" + ip4AddrToIndex(entries32[i].dstIp_.s_addr) +"'," +
            utf8::int2Str(entries32[i].ipProtocol_) + "," +
            utf8::int2Str(entries32[i].srcPort_) + "," +
            utf8::int2Str(entries32[i].dstPort_) + "," +
            utf8::int2Str(entries32[i].dgramSize_) + "," +
            utf8::int2Str(entries32[i].dataSize_)
	  );
        }
        else {
	  statement2_->text(statement2_->text() +
            "'" + mycpp::tm2Str(start) + "'," +
	    "'" + ip4AddrToIndex(entries[i].srcIp_.s_addr) + "'," +
            "'" + ip4AddrToIndex(entries[i].dstIp_.s_addr) +"'," +
            utf8::int2Str(entries[i].ipProtocol_) + "," +
            utf8::int2Str(entries[i].srcPort_) + "," +
            utf8::int2Str(entries[i].dstPort_) + "," +
            utf8::int2Str(entries[i].dgramSize_) + "," +
            utf8::int2Str(entries[i].dataSize_)
	  );
	}
        statement2_->text(statement2_->text() + (i % 128 != 0 ? ")," : ")"));
	if( i % 128 == 0 ){
          statement2_->execute();
	  executed = true;
	}
      }
      assert( executed );
    }
    else for( intptr_t i = entriesCount - 1; i >= 0; i-- ){
      if( log32bitOsCompatible ){
//        if( isMulticastAddress(ksock::api.ntohl(entries32[i].srcIp_.s_addr)) || isMulticastAddress(ksock::api.ntohl(entries32[i].dstIp_.s_addr)) ) continue;
//        if( isMCGlobal(ksock::api.ntohl(entries32[i].srcIp_.s_addr)) || isMCGlobal(ksock::api.ntohl(entries32[i].dstIp_.s_addr)) ) continue;
        statement_->paramAsMutant("st_src_ip",ip4AddrToIndex(entries32[i].srcIp_.s_addr));
        statement_->paramAsMutant("st_dst_ip",ip4AddrToIndex(entries32[i].dstIp_.s_addr));
        statement_->paramAsMutant("st_ip_proto",entries32[i].ipProtocol_);
        statement_->paramAsMutant("st_src_port",entries32[i].srcPort_);
        statement_->paramAsMutant("st_dst_port",entries32[i].dstPort_);
        statement_->paramAsMutant("st_dgram_bytes",entries32[i].dgramSize_);
        statement_->paramAsMutant("st_data_bytes",entries32[i].dataSize_);
	/*if( resolveDNSNames_ ){
          statement_->paramAsMutant("st_src_name",resolveAddr(entries32[i].srcIp_.s_addr));
          statement_->paramAsMutant("st_dst_name",resolveAddr(entries32[i].dstIp_.s_addr));
	}*/
      }
      else {
//        if( isMulticastAddress(ksock::api.ntohl(entries[i].srcIp_.s_addr)) || isMulticastAddress(ksock::api.ntohl(entries[i].dstIp_.s_addr)) ) continue;
//        if( isMCGlobal(ksock::api.ntohl(entries[i].srcIp_.s_addr)) || isMCGlobal(ksock::api.ntohl(entries[i].dstIp_.s_addr)) ) continue;
        statement_->paramAsMutant("st_src_ip",ip4AddrToIndex(entries[i].srcIp_.s_addr));
        statement_->paramAsMutant("st_dst_ip",ip4AddrToIndex(entries[i].dstIp_.s_addr));
        statement_->paramAsMutant("st_ip_proto",entries[i].ipProtocol_);
        statement_->paramAsMutant("st_src_port",entries[i].srcPort_);
        statement_->paramAsMutant("st_dst_port",entries[i].dstPort_);
        statement_->paramAsMutant("st_dgram_bytes",entries[i].dgramSize_);
        statement_->paramAsMutant("st_data_bytes",entries[i].dataSize_);
	/*if( resolveDNSNames_ ){
          statement_->paramAsMutant("st_src_name",resolveAddr(entries[i].srcIp_.s_addr));
          statement_->paramAsMutant("st_dst_name",resolveAddr(entries[i].dstIp_.s_addr));
	}*/
      }
      statement_->paramAsMutant("st_start",start);
      statement_->execute();
      printStat(lineNo,offset,flog.tell(),flog.size(),cl,&tma);
      lineNo++;
      entriesCount = 0;
    }
    printStat(lineNo,offset,flog.tell(),flog.size(),cl,&tma);
    updateLogFileLastOffset(flog.fileName(),flog.tell());
    database_->commit();
    database_->start();
    lineNo += entriesCount;
  }
  printStat(lineNo,offset,flog.tell(),flog.size(),cl);
  updateLogFileLastOffset(flog.fileName(),flog.tell());
  database_->commit();
}
//------------------------------------------------------------------------------
} // namespace macrosocope
//------------------------------------------------------------------------------
