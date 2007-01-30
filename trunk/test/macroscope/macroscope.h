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
#define _VERSION_C_AS_HEADER_
#include "version.c"
#undef _VERSION_C_AS_HEADER_
//------------------------------------------------------------------------------
using namespace ksys;
using namespace adicpp;
//------------------------------------------------------------------------------
namespace macroscope {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Logger {
  public:
    ~Logger();
    Logger();

    void main();
  protected:
    class MTLogParser : public Thread {
      public:
        ~MTLogParser();
        MTLogParser() {}
        MTLogParser(Logger & logger);

        void threadExecute();
      protected:
      private:
        Logger * logger_;
    };
    friend class MTLogParser;
    class MTWriter : public Thread {
      public:
        ~MTWriter();
        MTWriter() {}
        MTWriter(Logger & logger,const utf8::String & file,const struct tm & year);

        void threadExecute();
      protected:
        Logger * logger_;
        utf8::String file_;
        struct tm year_;
      private:
    };
    friend class MTWriter;
    
    Vector<Thread> threads_;
    
    void parseSquidLogFile(const utf8::String & logFileName,bool top10,const utf8::String & skipUrl);
    void parseSendmailLogFile(const utf8::String & logFileName,const utf8::String & domain,uintptr_t startYear);
    void parseBPFTLogFile(const ConfigSection & section);
    void writeHtmlYearOutput();
  private:
    utf8::String shortUrl_;
    utf8::String prefix_;
    utf8::String section_;
    ConfigSPi config_;
    AutoPtr<Database> database_;
    AutoPtr<Statement> statement_;
    AutoPtr<Statement> stTrafIns_;
    AutoPtr<Statement> stTrafUpd_;
    AutoPtr<Statement> stMonUrlSel_;
    AutoPtr<Statement> stMonUrlIns_;
    AutoPtr<Statement> stMonUrlUpd_;
    AutoPtr<Statement> stFileStatSel_;
    AutoPtr<Statement> stFileStatIns_;
    AutoPtr<Statement> stFileStatUpd_;
    AutoPtr<Statement> stMsgsIns_;
    AutoPtr<Statement> stMsgsSel_;
    AutoPtr<Statement> stMsgsDel_;
    AutoPtr<Statement> stMsgsDel2_;
    AutoPtr<Statement> stMsgsSelCount_;
    int64_t ellapsed_;
    bool verbose_;

    void printStat(int64_t lineNo, int64_t spos, int64_t pos, int64_t size, int64_t cl);
    void parseSquidLogLine(char * p, uintptr_t size, Array< const char *> & slcp);
    utf8::String  squidStrToWideString(const char * str);
    Mutant  timeStampRoundToMin(const Mutant & timeStamp);
    utf8::String  shortUrl(const utf8::String & url);
    int64_t       fetchLogFileLastOffset(const utf8::String & logFileName);
    Logger &      updateLogFileLastOffset(const utf8::String & logFileName, int64_t offset);
    // html reporter
    enum TrafType { ttSMTP, ttWWW, ttAll, ttCount };
    class TrafCacheEntry {
      public:
        int64_t       traf_;
        utf8::String  user_;
        struct tm     bt_;
        struct tm     et_;
        TrafType      trafType_;

        TrafCacheEntry(){}
        TrafCacheEntry(const utf8::String & user,const struct tm & bt,const struct tm & et,TrafType trafType)
          : user_(user), bt_(bt), et_(et), trafType_(trafType) {}

        utf8::String id() const;

        static EmbeddedHashNode<TrafCacheEntry> & keyNode(const TrafCacheEntry & object){
          return object.keyNode_;
        }
        static TrafCacheEntry & keyNodeObject(const EmbeddedHashNode<TrafCacheEntry> & node,TrafCacheEntry * p){
          return node.object(p->keyNode_);
        }
        static uintptr_t keyNodeHash(const TrafCacheEntry & object){
          return object.id().hash(false);
        }
        static bool keyHashNodeEqu(const TrafCacheEntry & object1,const TrafCacheEntry & object2){
          return object1.id().strcasecmp(object2.id()) == 0;
        }
        
        mutable EmbeddedHashNode<TrafCacheEntry> keyNode_;

        static EmbeddedListNode<TrafCacheEntry> & listNode(const TrafCacheEntry & object){
          return object.listNode_;
        }
        static TrafCacheEntry & listNodeObject(const EmbeddedListNode<TrafCacheEntry> & node,TrafCacheEntry * p = NULL){
          return node.object(p->listNode_);
        }

        mutable EmbeddedListNode<TrafCacheEntry> listNode_;
    };
    typedef EmbeddedHash<
      TrafCacheEntry,
      TrafCacheEntry::keyNode,
      TrafCacheEntry::keyNodeObject,
      TrafCacheEntry::keyNodeHash,
      TrafCacheEntry::keyHashNodeEqu
    > TrafCache;
    TrafCache trafCache_;
    AutoHashDrop<TrafCache> trafCacheAutoDrop_;
    typedef EmbeddedList<
      TrafCacheEntry,
      TrafCacheEntry::listNode,
      TrafCacheEntry::listNodeObject
    > TrafCacheLRU;
    TrafCacheLRU trafCacheLRU_;
    uintptr_t cacheSize_;

    utf8::String htmlDir_;

    int64_t getTraf(TrafType tt,const struct tm & bt,const struct tm & et,const utf8::String & user = utf8::String(),uintptr_t isGroup = 0);
    void writeHtmlHead(AsyncFile & f);
    void writeHtmlTail(AsyncFile & f);
    void writeUserTop(const utf8::String & file,const utf8::String & user,const struct tm & beginTime,const struct tm & endTime);
    void writeMonthHtmlOutput(const utf8::String & file,const struct tm & year,bool threaded = false);
    Logger & writeBPFTHtmlReport(AsyncFile & f);
    uintptr_t nonZeroYearMonthsColumns(struct tm byear);
    uintptr_t nonZeroMonthDaysColumns(struct tm bmon);
    static intptr_t sortUsersTrafTable(uintptr_t row1,uintptr_t row2,const Table<Mutant> & table);
    static intptr_t sortUsersTrafTables(Table<Mutant> * & p1,Table<Mutant> * & p2);
    static void writeTraf(AsyncFile & f,uint64_t qi,uint64_t qj);

    utf8::String trafTypeNick_[ttCount];
    utf8::String trafTypeHeadColor_[ttCount];
    utf8::String trafTypeBodyColor_[ttCount];
    utf8::String trafTypeTailColor_[ttCount];
    utf8::String trafTypeHeadDataColor_[ttCount];
    utf8::String trafTypeBodyDataColor_[ttCount];
    utf8::String trafTypeTailDataColor_[ttCount];

    void decoration();
};
//------------------------------------------------------------------------------
} // namespace macroscope
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
