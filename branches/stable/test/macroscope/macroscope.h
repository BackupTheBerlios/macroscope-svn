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
#ifndef _macroscopeH_
#define _macroscopeH_
//------------------------------------------------------------------------------
using namespace adicpp;
//------------------------------------------------------------------------------
namespace macroscope {

//------------------------------------------------------------------------------
class Logger {
  public:
                  ~Logger();
                  Logger();

    void          main();
  protected:
    void          parseSquidLogFile(const utf8::String & logFileName, bool top10, const utf8::String & skipUrl);
    void          parseSendmailLogFile(const utf8::String & logFileName, const utf8::String & domain, uintptr_t startYear);
    void          parseBPFTLogFile(const utf8::String & logFileName);
    void          writeHtmlYearOutput();
  private:
    utf8::String              shortUrl_;
    ksys::Config              config_;
    ksys::AutoPtr< Database>  database_;
    ksys::AutoPtr< Statement> statement_;
    ksys::AutoPtr< Statement> stTrafIns_;
    ksys::AutoPtr< Statement> stTrafUpd_;
    ksys::AutoPtr< Statement> stMonUrlSel_;
    ksys::AutoPtr< Statement> stMonUrlIns_;
    ksys::AutoPtr< Statement> stMonUrlUpd_;
    ksys::AutoPtr< Statement> stFileStatSel_;
    ksys::AutoPtr< Statement> stFileStatIns_;
    ksys::AutoPtr< Statement> stFileStatUpd_;
    ksys::AutoPtr< Statement> stMsgsIns_;
    ksys::AutoPtr< Statement> stMsgsSel_;
    ksys::AutoPtr< Statement> stMsgsDel_;
    ksys::AutoPtr< Statement> stMsgsDel2_;
    ksys::AutoPtr< Statement> stMsgsSelCount_;
    int64_t                   ellapsed_;
    bool                      verbose_;

    void          printStat(int64_t lineNo, int64_t spos, int64_t pos, int64_t size, int64_t cl);
    void          parseSquidLogLine(char * p, uintptr_t size, ksys::Array< const char *> & slcp);
    utf8::String  squidStrToWideString(const char * str);
    ksys::Mutant  timeStampRoundToMin(const ksys::Mutant & timeStamp);
    utf8::String  shortUrl(const utf8::String & url);
    int64_t       fetchLogFileLastOffset(const utf8::String & logFileName);
    Logger &      updateLogFileLastOffset(const utf8::String & logFileName, int64_t offset);
    intptr_t      strToMonth(const char * month);
    // html reporter
    enum TrafType { ttSMTP, ttWWW, ttAll, ttCount };
    class TrafCacheEntry {
      public:
        int64_t       traf_;
        utf8::String  user_;
        struct tm     bt_;
        struct tm     et_;
        TrafType      trafType_;

        TrafCacheEntry()
        {
        }
        TrafCacheEntry(const utf8::String & user, const struct tm & bt, const struct tm & et, TrafType trafType)
          : user_(user),
            bt_(bt),
            et_(et),
            trafType_(trafType)
        {
        }

        utf8::String  id() const;

        bool hashKeyEqu(const TrafCacheEntry & key, bool caseSensitive) const
        {
          return id().hashKeyEqu(key.id(), caseSensitive);
        }
        uintptr_t hash(bool caseSensitive) const
        {
          return id().hash(caseSensitive);
        }
    };
    ksys::HashedObjectList< utf8::String,TrafCacheEntry>  trafCache_;
    uintptr_t                                             cacheSize_;

    int64_t         getTraf(TrafType tt, const struct tm & bt, const struct tm & et, const utf8::String & user = utf8::String());
    void            writeHtmlHead(ksys::FileHandleContainer & f);
    void            writeHtmlTail(ksys::FileHandleContainer & f);
    void            writeMonthHtmlOutput(const utf8::String & file, const struct tm & year);
    uintptr_t       nonZeroYearMonthsColumns(struct tm byear);
    uintptr_t       nonZeroMonthDaysColumns(struct tm bmon);
    static intptr_t sortUsersTrafTable(uintptr_t row1, uintptr_t row2, const ksys::Table< ksys::Mutant> & table);
    static void     writeTraf(ksys::FileHandleContainer & f, uint64_t qi, uint64_t qj);

    utf8::String                                          trafTypeNick_[ttCount];
    utf8::String                                          trafTypeHeadColor_[ttCount];
    utf8::String                                          trafTypeBodyColor_[ttCount];
    utf8::String                                          trafTypeTailColor_[ttCount];
    utf8::String                                          trafTypeHeadDataColor_[ttCount];
    utf8::String                                          trafTypeBodyDataColor_[ttCount];
    utf8::String                                          trafTypeTailDataColor_[ttCount];

    void            decoration();
};
//------------------------------------------------------------------------------
} // namespace macroscope
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
