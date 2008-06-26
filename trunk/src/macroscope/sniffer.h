/*-
 * Copyright 2007-2008 Guram Dukashvili
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
#ifndef _snifferH_
#define _snifferH_
//------------------------------------------------------------------------------
using namespace ksys;
using namespace adicpp;
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Sniffer : public PCAP {
  public:
    virtual ~Sniffer();
    Sniffer(Database * database = NULL);

    PacketGroupingPeriod totalsPeriod() const { return totalsPeriod_; }
    Sniffer & totalsPeriod(PacketGroupingPeriod period);

    void recalcTotals();
    static void getTrafficPeriod(Statement * statement,const utf8::String & sectionName,struct tm & beginTime,struct tm & endTime,bool gmt = false);

    static void setTotalsBounds(PacketGroupingPeriod tp,struct tm & bt,struct tm & et,struct tm & btt,struct tm & ett);
    static void shiftPeriod(PacketGroupingPeriod tp,struct tm & t,intptr_t v);

    static const char * const pgpNames[pgpCount];
  protected:
    AutoPtr<Database> database_;
    AutoPtr<Statement> statement_;
    AutoPtr<Statement> statement2_;
    AutoPtr<Statement> stTotals_[3];
    AutoPtr<Statement> totals_[3][pgpCount];
    AutoPtr<Statement> ifaces_;

    PacketGroupingPeriod totalsPeriod_;

    enum { stSel, stIns, stUpd };

    void updateTotals(const Mutant & m,const in_addr & srcAddr,uintptr_t srcPort,const in_addr & dstAddr,uintptr_t dstPort,intptr_t proto,uintmax_t dgram,uintmax_t data);    
    bool insertPacketsInDatabase(uint64_t bt,uint64_t et,const HashedPacket * packets,uintptr_t count,Thread * caller) throw();
  private:
    void threadExecute();

    Sniffer(const Sniffer &);
    void operator = (const Sniffer &);
};
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
