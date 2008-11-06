/*-
 * Copyright 2008 Guram Dukashvili
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
#define _VERSION_C_AS_HEADER_
#include "version.c"
#undef _VERSION_C_AS_HEADER_
#define ENABLE_GD_INTERFACE 1
//#define ENABLE_PCAP_INTERFACE 1
//#define ENABLE_ODBC_INTERFACE 1
#define ENABLE_MYSQL_INTERFACE 1
#define ENABLE_FIREBIRD_INTERFACE 1
#include <adicpp/adicpp.h>
#include "Parser.h"
#include "Scanner.h"
#include "CodeGenerator.h"
#include "SymbolTable.h"
#include "Compiler.h"
#include "varnum.h"
//------------------------------------------------------------------------------
using namespace ksys;
using namespace ksys::kvm;
using namespace adicpp;
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class RangeCoderFilter32 {
  public:
    RangeCoderFilter32 & initializeEncoder();
    RangeCoderFilter32 & encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL);
    RangeCoderFilter32 & flush(void * out,uintptr_t * wb);
    RangeCoderFilter32 & encode(AsyncFile & inp,AsyncFile & out);

    RangeCoderFilter32 & initializeDecoder();
    RangeCoderFilter32 & decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL);
    RangeCoderFilter32 & decode(AsyncFile & inp,AsyncFile & out);
  private:
	  static const uint32_t top_ = uint32_t(1) << 24;
    static const uint32_t bottom_ = uint32_t(1) << 16;
	  static const uint32_t maxRange_ = bottom_;

    // encoder stateful members
    uint32_t eFreq_[257];
    uint32_t eLow_;
    uint32_t eRange_;
    uint32_t ec_;

    // decoder stateful members
    uint32_t dFreq_[257];
    uint32_t code_;
    uint32_t dLow_;
    uint32_t dRange_;
    uint32_t dc_;
    uint32_t di_;

    enum { stInit, stFlush, stInp, stOut } eState_, dState_;

    void rescale(uint32_t * freq)
    {
      register uint32_t * q = freq + 1;
      freq = q + 256;
	    while( q < freq ){
		    q[0] /= 2;
		    if( q[0] <= q[-1] ) q[0] = q[-1] + 1;
		    q[1] /= 2;
		    if( q[1] <= q[-1 + 1] ) q[1] = q[-1 + 1] + 1;
		    q[2] /= 2;
		    if( q[1] <= q[-1 + 2] ) q[2] = q[-1 + 2] + 1;
		    q[3] /= 2;
		    if( q[3] <= q[-1 + 3] ) q[3] = q[-1 + 3] + 1;
		    q[4] /= 2;
		    if( q[4] <= q[-1 + 4] ) q[4] = q[-1 + 4] + 1;
		    q[5] /= 2;
		    if( q[5] <= q[-1 + 5] ) q[5] = q[-1 + 5] + 1;
		    q[6] /= 2;
		    if( q[6] <= q[-1 + 6] ) q[6] = q[-1 + 6] + 1;
		    q[7] /= 2;
		    if( q[7] <= q[-1 + 7] ) q[7] = q[-1 + 7] + 1;
        q += 8;
	    }
    }

    void update(uint32_t c,uint32_t * freq)
    {
      //for( uintptr_t j = c + 1; j < 257; j++ ) freq[j]++;
      switch( c + 1 ){
        case   1 : freq[1]++;
        case   2 : freq[2]++;
        case   3 : freq[3]++;
        case   4 : freq[4]++;
        case   5 : freq[5]++;
        case   6 : freq[6]++;
        case   7 : freq[7]++;
        case   8 : freq[8]++;
        case   9 : freq[9]++;
        case  10 : freq[10]++;
        case  11 : freq[11]++;
        case  12 : freq[12]++;
        case  13 : freq[13]++;
        case  14 : freq[14]++;
        case  15 : freq[15]++;
        case  16 : freq[16]++;
        case  17 : freq[17]++;
        case  18 : freq[18]++;
        case  19 : freq[19]++;
        case  20 : freq[20]++;
        case  21 : freq[21]++;
        case  22 : freq[22]++;
        case  23 : freq[23]++;
        case  24 : freq[24]++;
        case  25 : freq[25]++;
        case  26 : freq[26]++;
        case  27 : freq[27]++;
        case  28 : freq[28]++;
        case  29 : freq[29]++;
        case  30 : freq[30]++;
        case  31 : freq[31]++;
        case  32 : freq[32]++;
        case  33 : freq[33]++;
        case  34 : freq[34]++;
        case  35 : freq[35]++;
        case  36 : freq[36]++;
        case  37 : freq[37]++;
        case  38 : freq[38]++;
        case  39 : freq[39]++;
        case  40 : freq[40]++;
        case  41 : freq[41]++;
        case  42 : freq[42]++;
        case  43 : freq[43]++;
        case  44 : freq[44]++;
        case  45 : freq[45]++;
        case  46 : freq[46]++;
        case  47 : freq[47]++;
        case  48 : freq[48]++;
        case  49 : freq[49]++;
        case  50 : freq[50]++;
        case  51 : freq[51]++;
        case  52 : freq[52]++;
        case  53 : freq[53]++;
        case  54 : freq[54]++;
        case  55 : freq[55]++;
        case  56 : freq[56]++;
        case  57 : freq[57]++;
        case  58 : freq[58]++;
        case  59 : freq[59]++;
        case  60 : freq[60]++;
        case  61 : freq[61]++;
        case  62 : freq[62]++;
        case  63 : freq[63]++;
        case  64 : freq[64]++;
        case  65 : freq[65]++;
        case  66 : freq[66]++;
        case  67 : freq[67]++;
        case  68 : freq[68]++;
        case  69 : freq[69]++;
        case  70 : freq[70]++;
        case  71 : freq[71]++;
        case  72 : freq[72]++;
        case  73 : freq[73]++;
        case  74 : freq[74]++;
        case  75 : freq[75]++;
        case  76 : freq[76]++;
        case  77 : freq[77]++;
        case  78 : freq[78]++;
        case  79 : freq[79]++;
        case  80 : freq[80]++;
        case  81 : freq[81]++;
        case  82 : freq[82]++;
        case  83 : freq[83]++;
        case  84 : freq[84]++;
        case  85 : freq[85]++;
        case  86 : freq[86]++;
        case  87 : freq[87]++;
        case  88 : freq[88]++;
        case  89 : freq[89]++;
        case  90 : freq[90]++;
        case  91 : freq[91]++;
        case  92 : freq[92]++;
        case  93 : freq[93]++;
        case  94 : freq[94]++;
        case  95 : freq[95]++;
        case  96 : freq[96]++;
        case  97 : freq[97]++;
        case  98 : freq[98]++;
        case  99 : freq[99]++;

        case 100 : freq[100]++;
        case 101 : freq[101]++;
        case 102 : freq[102]++;
        case 103 : freq[103]++;
        case 104 : freq[104]++;
        case 105 : freq[105]++;
        case 106 : freq[106]++;
        case 107 : freq[107]++;
        case 108 : freq[108]++;
        case 109 : freq[109]++;
        case 110 : freq[110]++;
        case 111 : freq[111]++;
        case 112 : freq[112]++;
        case 113 : freq[113]++;
        case 114 : freq[114]++;
        case 115 : freq[115]++;
        case 116 : freq[116]++;
        case 117 : freq[117]++;
        case 118 : freq[118]++;
        case 119 : freq[119]++;
        case 120 : freq[120]++;
        case 121 : freq[121]++;
        case 122 : freq[122]++;
        case 123 : freq[123]++;
        case 124 : freq[124]++;
        case 125 : freq[125]++;
        case 126 : freq[126]++;
        case 127 : freq[127]++;
        case 128 : freq[128]++;
        case 129 : freq[129]++;
        case 130 : freq[130]++;
        case 131 : freq[131]++;
        case 132 : freq[132]++;
        case 133 : freq[133]++;
        case 134 : freq[134]++;
        case 135 : freq[135]++;
        case 136 : freq[136]++;
        case 137 : freq[137]++;
        case 138 : freq[138]++;
        case 139 : freq[139]++;
        case 140 : freq[140]++;
        case 141 : freq[141]++;
        case 142 : freq[142]++;
        case 143 : freq[143]++;
        case 144 : freq[144]++;
        case 145 : freq[145]++;
        case 146 : freq[146]++;
        case 147 : freq[147]++;
        case 148 : freq[148]++;
        case 149 : freq[149]++;
        case 150 : freq[150]++;
        case 151 : freq[151]++;
        case 152 : freq[152]++;
        case 153 : freq[153]++;
        case 154 : freq[154]++;
        case 155 : freq[155]++;
        case 156 : freq[156]++;
        case 157 : freq[157]++;
        case 158 : freq[158]++;
        case 159 : freq[159]++;
        case 160 : freq[160]++;
        case 161 : freq[161]++;
        case 162 : freq[162]++;
        case 163 : freq[163]++;
        case 164 : freq[164]++;
        case 165 : freq[165]++;
        case 166 : freq[166]++;
        case 167 : freq[167]++;
        case 168 : freq[168]++;
        case 169 : freq[169]++;
        case 170 : freq[170]++;
        case 171 : freq[171]++;
        case 172 : freq[172]++;
        case 173 : freq[173]++;
        case 174 : freq[174]++;
        case 175 : freq[175]++;
        case 176 : freq[176]++;
        case 177 : freq[177]++;
        case 178 : freq[178]++;
        case 179 : freq[179]++;
        case 180 : freq[180]++;
        case 181 : freq[181]++;
        case 182 : freq[182]++;
        case 183 : freq[183]++;
        case 184 : freq[184]++;
        case 185 : freq[185]++;
        case 186 : freq[186]++;
        case 187 : freq[187]++;
        case 188 : freq[188]++;
        case 189 : freq[189]++;
        case 190 : freq[190]++;
        case 191 : freq[191]++;
        case 192 : freq[192]++;
        case 193 : freq[193]++;
        case 194 : freq[194]++;
        case 195 : freq[195]++;
        case 196 : freq[196]++;
        case 197 : freq[197]++;
        case 198 : freq[198]++;
        case 199 : freq[199]++;

        case 200 : freq[200]++;
        case 201 : freq[201]++;
        case 202 : freq[202]++;
        case 203 : freq[203]++;
        case 204 : freq[204]++;
        case 205 : freq[205]++;
        case 206 : freq[206]++;
        case 207 : freq[207]++;
        case 208 : freq[208]++;
        case 209 : freq[209]++;
        case 210 : freq[210]++;
        case 211 : freq[211]++;
        case 212 : freq[212]++;
        case 213 : freq[213]++;
        case 214 : freq[214]++;
        case 215 : freq[215]++;
        case 216 : freq[216]++;
        case 217 : freq[217]++;
        case 218 : freq[218]++;
        case 219 : freq[219]++;
        case 220 : freq[220]++;
        case 221 : freq[221]++;
        case 222 : freq[222]++;
        case 223 : freq[223]++;
        case 224 : freq[224]++;
        case 225 : freq[225]++;
        case 226 : freq[226]++;
        case 227 : freq[227]++;
        case 228 : freq[228]++;
        case 229 : freq[229]++;
        case 230 : freq[230]++;
        case 231 : freq[231]++;
        case 232 : freq[232]++;
        case 233 : freq[233]++;
        case 234 : freq[234]++;
        case 235 : freq[235]++;
        case 236 : freq[236]++;
        case 237 : freq[237]++;
        case 238 : freq[238]++;
        case 239 : freq[239]++;
        case 240 : freq[240]++;
        case 241 : freq[241]++;
        case 242 : freq[242]++;
        case 243 : freq[243]++;
        case 244 : freq[244]++;
        case 245 : freq[245]++;
        case 246 : freq[246]++;
        case 247 : freq[247]++;
        case 248 : freq[248]++;
        case 249 : freq[249]++;
        case 250 : freq[250]++;
        case 251 : freq[251]++;
        case 252 : freq[252]++;
        case 253 : freq[253]++;
        case 254 : freq[254]++;
        case 255 : freq[255]++;
        case 256 : freq[256]++;
 
      }
		  if( freq[256] >= maxRange_ ) rescale(freq);
    }
};
//------------------------------------------------------------------------------
RangeCoderFilter32 & RangeCoderFilter32::initializeEncoder()
{
  eLow_ = 0;
  eRange_ = int32_t(-1);
	for( uintptr_t i = 0; i < sizeof(eFreq_) / sizeof(eFreq_[0]); i++ ) eFreq_[i] = uint32_t(i);
  eState_ = stInit;
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter32 & RangeCoderFilter32::encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb,uintptr_t * wb)
{
  if( eState_ == stOut ) goto out;
  for(;;){
    if( inpSize == 0 ){
      eState_ = stInp;
      return *this;
    }
    ec_ = *(const uint8_t *) inp;
    inp = (const uint8_t *) inp + sizeof(uint8_t);
    inpSize -= sizeof(uint8_t);
    if( rb != NULL ) *rb += sizeof(uint8_t);

    uint32_t symbolLow = eFreq_[ec_], symbolHigh = eFreq_[ec_ + 1], totalRange = eFreq_[256];
    eLow_ += symbolLow * (eRange_ /= totalRange);
    eRange_ *= symbolHigh - symbolLow;

    while( (eLow_ ^ (eLow_ + eRange_)) < top_ || eRange_ < bottom_ && ((eRange_ = -int32_t(eLow_) & (bottom_ - 1)),1) ){
out:  if( outSize == 0 ){
        eState_ = stOut;
        return *this;
      }	     
      *(uint8_t *) out = uint8_t(eLow_ >> 24);
      out = (uint8_t *) out + sizeof(uint8_t);
      outSize -= sizeof(uint8_t);
      if( wb != NULL ) *wb += sizeof(uint8_t);

      eRange_ <<= 8;
      eLow_ <<= 8;
    }
    update(ec_,eFreq_);
	}
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter32 & RangeCoderFilter32::flush(void * out,uintptr_t * wb)
{
	for( uintptr_t i = 0; i < 3; i++ ){
    *(uint8_t *) out = uint8_t(eLow_ >> 24);
    out = (uint8_t *) out + sizeof(uint8_t);
    if( wb != NULL ) *wb += sizeof(uint8_t);
    eLow_ <<= 8;
	}
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter32 & RangeCoderFilter32::encode(AsyncFile & inp,AsyncFile & out)
{
  uintptr_t rbs = getpagesize() * 16;
  uintptr_t wbs = getpagesize() * 16;
  AutoPtr<uint8_t> inpBuffer((uint8_t *) kmalloc(rbs));
  AutoPtr<uint8_t> outBuffer((uint8_t *) kmalloc(wbs));
  uintptr_t rb = 0, wb = 0;
  int64_t r;
  while( (r = inp.read(inpBuffer,rbs)) > 0 ){
    while( rb < uintptr_t(r) ){
      encodeBuffer(&inpBuffer[rb],uintptr_t(r) - rb,&outBuffer[wb],wbs - wb,&rb,&wb);
      if( wb == wbs ){
        out.writeBuffer(outBuffer,wbs);
        wb = 0;
      }
    }
    rb = 0;
  }
  out.writeBuffer(outBuffer,wb);
  wb = 0;
  flush(outBuffer,&wb);
  out.writeBuffer(outBuffer,wb);
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter32 & RangeCoderFilter32::initializeDecoder()
{
  dLow_ = 0;
  dRange_ = int32_t(-1);
	for( uintptr_t i = 0; i < sizeof(dFreq_) / sizeof(dFreq_[0]); i++ ) dFreq_[i] = uint32_t(i);
	code_ = 0;
  di_ = 0;
  dState_ = stInit;
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter32 & RangeCoderFilter32::decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb,uintptr_t * wb)
{
  if( dState_ == stInp ) goto inp;
  if( dState_ == stOut ) goto out;
  while( di_ < 4 ){
    if( inpSize == 0 ) return *this;
	  code_ = (code_ << 8) | *(const uint8_t *) inp;
    inp = (const uint8_t *) inp + sizeof(uint8_t);
    inpSize -= sizeof(uint8_t);
    if( rb != NULL ) *rb += sizeof(uint8_t);
    di_++;
  }
  for(;;){
    uint32_t totalRange = dFreq_[256];
    register uint32_t count = (code_ - dLow_) / (dRange_ /= totalRange);
    register uintptr_t dc = 255;
    register uint32_t * freq = dFreq_;
    for(;;){
      if( freq[dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
    }
    dc_ = uint32_t(dc);
out:
    if( outSize == 0 ){
      dState_ = stOut;
      return *this;
    }	     
    *(uint8_t *) out = uint8_t(dc_);
    out = (uint8_t *) out + sizeof(uint8_t);
    outSize -= sizeof(uint8_t);
    if( wb != NULL ) *wb += sizeof(uint8_t);

    uint32_t symbolLow = dFreq_[dc_], symbolHigh = dFreq_[dc_ + 1];
    dLow_ += symbolLow * dRange_;
    dRange_ *= symbolHigh - symbolLow;

    while( (dLow_ ^ (dLow_ + dRange_)) < top_ || dRange_ < bottom_ && ((dRange_ = -int32_t(dLow_) & bottom_ - 1),1) ){
inp:  if( inpSize == 0 ){
        dState_ = stInp;
        return *this;
      }
      code_ = code_ << 8 | *(const uint8_t *) inp;
      inp = (const uint8_t *) inp + sizeof(uint8_t);
      inpSize -= sizeof(uint8_t);
      if( rb != NULL ) *rb += sizeof(uint8_t);

      dRange_ <<= 8;
      dLow_ <<= 8;
    }

    update(dc_,dFreq_);
  }
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter32 & RangeCoderFilter32::decode(AsyncFile & inp,AsyncFile & out)
{
  uintptr_t rbs = getpagesize() * 16;
  uintptr_t wbs = getpagesize() * 16;
  AutoPtr<uint8_t> inpBuffer((uint8_t *) kmalloc(rbs));
  AutoPtr<uint8_t> outBuffer((uint8_t *) kmalloc(wbs));
  uintptr_t rb = 0, wb = 0;
  int64_t r;
  while( (r = inp.read(inpBuffer,rbs)) > 0 ){
    while( rb < uintptr_t(r) ){
      decodeBuffer(&inpBuffer[rb],uintptr_t(r) - rb,&outBuffer[wb],wbs - wb,&rb,&wb);
      if( wb == wbs ){
        out.writeBuffer(outBuffer,wbs);
        wb = 0;
      }
    }
    rb = 0;
  }
  out.writeBuffer(outBuffer,wb);
  return *this;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class RangeCoderFilter64 {
  public:
    RangeCoderFilter64 & initializeEncoder();
    RangeCoderFilter64 & encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL);
    RangeCoderFilter64 & flush(void * out,uintptr_t * wb);
    RangeCoderFilter64 & encode(AsyncFile & inp,AsyncFile & out);

    RangeCoderFilter64 & initializeDecoder();
    RangeCoderFilter64 & decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL);
    RangeCoderFilter64 & decode(AsyncFile & inp,AsyncFile & out);
  private:
	  static const uint64_t top_ = uint64_t(1) << 56;
    static const uint64_t bottom_ = uint64_t(1) << 48;
	  static const uint64_t maxRange_ = bottom_;

    // encoder stateful members
    uint32_t eFreq_[257];
    uint64_t eLow_;
    uint64_t eRange_;
    uint32_t ec_;

    // decoder stateful members
    uint32_t dFreq_[257];
    uint64_t code_;
    uint64_t dLow_;
    uint64_t dRange_;
    uint32_t dc_;
    uint32_t di_;

    enum { stInit, stFlush, stInp, stOut } eState_, dState_;

    void rescale(uint32_t * freq)
    {
      register uint32_t * q = freq + 1;
      freq = q + 256;
	    while( q < freq ){
		    q[0] /= 2;
		    if( q[0] <= q[-1] ) q[0] = q[-1] + 1;
		    q[1] /= 2;
		    if( q[1] <= q[-1 + 1] ) q[1] = q[-1 + 1] + 1;
		    q[2] /= 2;
		    if( q[1] <= q[-1 + 2] ) q[2] = q[-1 + 2] + 1;
		    q[3] /= 2;
		    if( q[3] <= q[-1 + 3] ) q[3] = q[-1 + 3] + 1;
		    q[4] /= 2;
		    if( q[4] <= q[-1 + 4] ) q[4] = q[-1 + 4] + 1;
		    q[5] /= 2;
		    if( q[5] <= q[-1 + 5] ) q[5] = q[-1 + 5] + 1;
		    q[6] /= 2;
		    if( q[6] <= q[-1 + 6] ) q[6] = q[-1 + 6] + 1;
		    q[7] /= 2;
		    if( q[7] <= q[-1 + 7] ) q[7] = q[-1 + 7] + 1;
        q += 8;
	    }
    }

    void update(uint32_t c,uint32_t * freq)
    {
      //for( uintptr_t j = c + 1; j < 257; j++ ) freq[j]++;
      switch( c + 1 ){
        case   1 : freq[1]++;
        case   2 : freq[2]++;
        case   3 : freq[3]++;
        case   4 : freq[4]++;
        case   5 : freq[5]++;
        case   6 : freq[6]++;
        case   7 : freq[7]++;
        case   8 : freq[8]++;
        case   9 : freq[9]++;
        case  10 : freq[10]++;
        case  11 : freq[11]++;
        case  12 : freq[12]++;
        case  13 : freq[13]++;
        case  14 : freq[14]++;
        case  15 : freq[15]++;
        case  16 : freq[16]++;
        case  17 : freq[17]++;
        case  18 : freq[18]++;
        case  19 : freq[19]++;
        case  20 : freq[20]++;
        case  21 : freq[21]++;
        case  22 : freq[22]++;
        case  23 : freq[23]++;
        case  24 : freq[24]++;
        case  25 : freq[25]++;
        case  26 : freq[26]++;
        case  27 : freq[27]++;
        case  28 : freq[28]++;
        case  29 : freq[29]++;
        case  30 : freq[30]++;
        case  31 : freq[31]++;
        case  32 : freq[32]++;
        case  33 : freq[33]++;
        case  34 : freq[34]++;
        case  35 : freq[35]++;
        case  36 : freq[36]++;
        case  37 : freq[37]++;
        case  38 : freq[38]++;
        case  39 : freq[39]++;
        case  40 : freq[40]++;
        case  41 : freq[41]++;
        case  42 : freq[42]++;
        case  43 : freq[43]++;
        case  44 : freq[44]++;
        case  45 : freq[45]++;
        case  46 : freq[46]++;
        case  47 : freq[47]++;
        case  48 : freq[48]++;
        case  49 : freq[49]++;
        case  50 : freq[50]++;
        case  51 : freq[51]++;
        case  52 : freq[52]++;
        case  53 : freq[53]++;
        case  54 : freq[54]++;
        case  55 : freq[55]++;
        case  56 : freq[56]++;
        case  57 : freq[57]++;
        case  58 : freq[58]++;
        case  59 : freq[59]++;
        case  60 : freq[60]++;
        case  61 : freq[61]++;
        case  62 : freq[62]++;
        case  63 : freq[63]++;
        case  64 : freq[64]++;
        case  65 : freq[65]++;
        case  66 : freq[66]++;
        case  67 : freq[67]++;
        case  68 : freq[68]++;
        case  69 : freq[69]++;
        case  70 : freq[70]++;
        case  71 : freq[71]++;
        case  72 : freq[72]++;
        case  73 : freq[73]++;
        case  74 : freq[74]++;
        case  75 : freq[75]++;
        case  76 : freq[76]++;
        case  77 : freq[77]++;
        case  78 : freq[78]++;
        case  79 : freq[79]++;
        case  80 : freq[80]++;
        case  81 : freq[81]++;
        case  82 : freq[82]++;
        case  83 : freq[83]++;
        case  84 : freq[84]++;
        case  85 : freq[85]++;
        case  86 : freq[86]++;
        case  87 : freq[87]++;
        case  88 : freq[88]++;
        case  89 : freq[89]++;
        case  90 : freq[90]++;
        case  91 : freq[91]++;
        case  92 : freq[92]++;
        case  93 : freq[93]++;
        case  94 : freq[94]++;
        case  95 : freq[95]++;
        case  96 : freq[96]++;
        case  97 : freq[97]++;
        case  98 : freq[98]++;
        case  99 : freq[99]++;

        case 100 : freq[100]++;
        case 101 : freq[101]++;
        case 102 : freq[102]++;
        case 103 : freq[103]++;
        case 104 : freq[104]++;
        case 105 : freq[105]++;
        case 106 : freq[106]++;
        case 107 : freq[107]++;
        case 108 : freq[108]++;
        case 109 : freq[109]++;
        case 110 : freq[110]++;
        case 111 : freq[111]++;
        case 112 : freq[112]++;
        case 113 : freq[113]++;
        case 114 : freq[114]++;
        case 115 : freq[115]++;
        case 116 : freq[116]++;
        case 117 : freq[117]++;
        case 118 : freq[118]++;
        case 119 : freq[119]++;
        case 120 : freq[120]++;
        case 121 : freq[121]++;
        case 122 : freq[122]++;
        case 123 : freq[123]++;
        case 124 : freq[124]++;
        case 125 : freq[125]++;
        case 126 : freq[126]++;
        case 127 : freq[127]++;
        case 128 : freq[128]++;
        case 129 : freq[129]++;
        case 130 : freq[130]++;
        case 131 : freq[131]++;
        case 132 : freq[132]++;
        case 133 : freq[133]++;
        case 134 : freq[134]++;
        case 135 : freq[135]++;
        case 136 : freq[136]++;
        case 137 : freq[137]++;
        case 138 : freq[138]++;
        case 139 : freq[139]++;
        case 140 : freq[140]++;
        case 141 : freq[141]++;
        case 142 : freq[142]++;
        case 143 : freq[143]++;
        case 144 : freq[144]++;
        case 145 : freq[145]++;
        case 146 : freq[146]++;
        case 147 : freq[147]++;
        case 148 : freq[148]++;
        case 149 : freq[149]++;
        case 150 : freq[150]++;
        case 151 : freq[151]++;
        case 152 : freq[152]++;
        case 153 : freq[153]++;
        case 154 : freq[154]++;
        case 155 : freq[155]++;
        case 156 : freq[156]++;
        case 157 : freq[157]++;
        case 158 : freq[158]++;
        case 159 : freq[159]++;
        case 160 : freq[160]++;
        case 161 : freq[161]++;
        case 162 : freq[162]++;
        case 163 : freq[163]++;
        case 164 : freq[164]++;
        case 165 : freq[165]++;
        case 166 : freq[166]++;
        case 167 : freq[167]++;
        case 168 : freq[168]++;
        case 169 : freq[169]++;
        case 170 : freq[170]++;
        case 171 : freq[171]++;
        case 172 : freq[172]++;
        case 173 : freq[173]++;
        case 174 : freq[174]++;
        case 175 : freq[175]++;
        case 176 : freq[176]++;
        case 177 : freq[177]++;
        case 178 : freq[178]++;
        case 179 : freq[179]++;
        case 180 : freq[180]++;
        case 181 : freq[181]++;
        case 182 : freq[182]++;
        case 183 : freq[183]++;
        case 184 : freq[184]++;
        case 185 : freq[185]++;
        case 186 : freq[186]++;
        case 187 : freq[187]++;
        case 188 : freq[188]++;
        case 189 : freq[189]++;
        case 190 : freq[190]++;
        case 191 : freq[191]++;
        case 192 : freq[192]++;
        case 193 : freq[193]++;
        case 194 : freq[194]++;
        case 195 : freq[195]++;
        case 196 : freq[196]++;
        case 197 : freq[197]++;
        case 198 : freq[198]++;
        case 199 : freq[199]++;

        case 200 : freq[200]++;
        case 201 : freq[201]++;
        case 202 : freq[202]++;
        case 203 : freq[203]++;
        case 204 : freq[204]++;
        case 205 : freq[205]++;
        case 206 : freq[206]++;
        case 207 : freq[207]++;
        case 208 : freq[208]++;
        case 209 : freq[209]++;
        case 210 : freq[210]++;
        case 211 : freq[211]++;
        case 212 : freq[212]++;
        case 213 : freq[213]++;
        case 214 : freq[214]++;
        case 215 : freq[215]++;
        case 216 : freq[216]++;
        case 217 : freq[217]++;
        case 218 : freq[218]++;
        case 219 : freq[219]++;
        case 220 : freq[220]++;
        case 221 : freq[221]++;
        case 222 : freq[222]++;
        case 223 : freq[223]++;
        case 224 : freq[224]++;
        case 225 : freq[225]++;
        case 226 : freq[226]++;
        case 227 : freq[227]++;
        case 228 : freq[228]++;
        case 229 : freq[229]++;
        case 230 : freq[230]++;
        case 231 : freq[231]++;
        case 232 : freq[232]++;
        case 233 : freq[233]++;
        case 234 : freq[234]++;
        case 235 : freq[235]++;
        case 236 : freq[236]++;
        case 237 : freq[237]++;
        case 238 : freq[238]++;
        case 239 : freq[239]++;
        case 240 : freq[240]++;
        case 241 : freq[241]++;
        case 242 : freq[242]++;
        case 243 : freq[243]++;
        case 244 : freq[244]++;
        case 245 : freq[245]++;
        case 246 : freq[246]++;
        case 247 : freq[247]++;
        case 248 : freq[248]++;
        case 249 : freq[249]++;
        case 250 : freq[250]++;
        case 251 : freq[251]++;
        case 252 : freq[252]++;
        case 253 : freq[253]++;
        case 254 : freq[254]++;
        case 255 : freq[255]++;
        case 256 : freq[256]++;
 
      }
		  if( freq[256] >= maxRange_ ) rescale(freq);
    }
};
//------------------------------------------------------------------------------
RangeCoderFilter64 & RangeCoderFilter64::initializeEncoder()
{
  eLow_ = 0;
  eRange_ = int64_t(-1);
	for( uintptr_t i = 0; i < sizeof(eFreq_) / sizeof(eFreq_[0]); i++ ) eFreq_[i] = uint32_t(i);
  eState_ = stInit;
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter64 & RangeCoderFilter64::encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb,uintptr_t * wb)
{
  if( eState_ == stOut ) goto out;
  for(;;){
    if( inpSize == 0 ){
      eState_ = stInp;
      return *this;
    }
    ec_ = *(const uint8_t *) inp;
    inp = (const uint8_t *) inp + sizeof(uint8_t);
    inpSize -= sizeof(uint8_t);
    if( rb != NULL ) *rb += sizeof(uint8_t);

    uint32_t symbolLow = eFreq_[ec_], symbolHigh = eFreq_[ec_ + 1], totalRange = eFreq_[256];
    eLow_ += symbolLow * (eRange_ /= totalRange);
    eRange_ *= symbolHigh - symbolLow;

    while( (eLow_ ^ (eLow_ + eRange_)) < top_ || eRange_ < bottom_ && ((eRange_ = -int64_t(eLow_) & (bottom_ - 1)),1) ){
out:  if( outSize == 0 ){
        eState_ = stOut;
        return *this;
      }	     
      *(uint8_t *) out = uint8_t(eLow_ >> 56);
      out = (uint8_t *) out + sizeof(uint8_t);
      outSize -= sizeof(uint8_t);
      if( wb != NULL ) *wb += sizeof(uint8_t);

      eRange_ <<= 8;
      eLow_ <<= 8;
    }
    update(ec_,eFreq_);
	}
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter64 & RangeCoderFilter64::flush(void * out,uintptr_t * wb)
{
	for( uintptr_t i = 0; i < 7; i++ ){
    *(uint8_t *) out = uint8_t(eLow_ >> 56);
    out = (uint8_t *) out + sizeof(uint8_t);
    if( wb != NULL ) *wb += sizeof(uint8_t);
    eLow_ <<= 8;
	}
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter64 & RangeCoderFilter64::encode(AsyncFile & inp,AsyncFile & out)
{
  uintptr_t rbs = getpagesize() * 16;
  uintptr_t wbs = getpagesize() * 16;
  AutoPtr<uint8_t> inpBuffer((uint8_t *) kmalloc(rbs));
  AutoPtr<uint8_t> outBuffer((uint8_t *) kmalloc(wbs));
  uintptr_t rb = 0, wb = 0;
  int64_t r;
  while( (r = inp.read(inpBuffer,rbs)) > 0 ){
    while( rb < uintptr_t(r) ){
      encodeBuffer(&inpBuffer[rb],uintptr_t(r) - rb,&outBuffer[wb],wbs - wb,&rb,&wb);
      if( wb == wbs ){
        out.writeBuffer(outBuffer,wbs);
        wb = 0;
      }
    }
    rb = 0;
  }
  out.writeBuffer(outBuffer,wb);
  wb = 0;
  flush(outBuffer,&wb);
  out.writeBuffer(outBuffer,wb);
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter64 & RangeCoderFilter64::initializeDecoder()
{
  dLow_ = 0;
  dRange_ = int64_t(-1);
	for( uintptr_t i = 0; i < sizeof(dFreq_) / sizeof(dFreq_[0]); i++ ) dFreq_[i] = uint32_t(i);
	code_ = 0;
  di_ = 0;
  dState_ = stInit;
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter64 & RangeCoderFilter64::decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb,uintptr_t * wb)
{
  if( dState_ == stInp ) goto inp;
  if( dState_ == stOut ) goto out;
  while( di_ < 8 ){
    if( inpSize == 0 ) return *this;
	  code_ = (code_ << 8) | *(const uint8_t *) inp;
    inp = (const uint8_t *) inp + sizeof(uint8_t);
    inpSize -= sizeof(uint8_t);
    if( rb != NULL ) *rb += sizeof(uint8_t);
    di_++;
  }
  for(;;){
    uint32_t totalRange = dFreq_[256];
    register uint32_t count = uint32_t((code_ - dLow_) / (dRange_ /= totalRange));
    register uint32_t dc = 255;
    register uint32_t * freq = dFreq_;
    for(;;){
      if( freq[dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
      if( freq[--dc] <= count ) break;
    }
    dc_ = dc;
out:
    if( outSize == 0 ){
      dState_ = stOut;
      return *this;
    }	     
    *(uint8_t *) out = uint8_t(dc_);
    out = (uint8_t *) out + sizeof(uint8_t);
    outSize -= sizeof(uint8_t);
    if( wb != NULL ) *wb += sizeof(uint8_t);

    uint32_t symbolLow = dFreq_[dc_], symbolHigh = dFreq_[dc_ + 1];
    dLow_ += symbolLow * dRange_;
    dRange_ *= symbolHigh - symbolLow;

    while( (dLow_ ^ (dLow_ + dRange_)) < top_ || dRange_ < bottom_ && ((dRange_ = -int64_t(dLow_) & bottom_ - 1),1) ){
inp:  if( inpSize == 0 ){
        dState_ = stInp;
        return *this;
      }
      code_ = code_ << 8 | *(const uint8_t *) inp;
      inp = (const uint8_t *) inp + sizeof(uint8_t);
      inpSize -= sizeof(uint8_t);
      if( rb != NULL ) *rb += sizeof(uint8_t);

      dRange_ <<= 8;
      dLow_ <<= 8;
    }

    update(dc_,dFreq_);
  }
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter64 & RangeCoderFilter64::decode(AsyncFile & inp,AsyncFile & out)
{
  uintptr_t rbs = getpagesize() * 16;
  uintptr_t wbs = getpagesize() * 16;
  AutoPtr<uint8_t> inpBuffer((uint8_t *) kmalloc(rbs));
  AutoPtr<uint8_t> outBuffer((uint8_t *) kmalloc(wbs));
  uintptr_t rb = 0, wb = 0;
  int64_t r;
  while( (r = inp.read(inpBuffer,rbs)) > 0 ){
    while( rb < uintptr_t(r) ){
      decodeBuffer(&inpBuffer[rb],uintptr_t(r) - rb,&outBuffer[wb],wbs - wb,&rb,&wb);
      if( wb == wbs ){
        out.writeBuffer(outBuffer,wbs);
        wb = 0;
      }
    }
    rb = 0;
  }
  out.writeBuffer(outBuffer,wb);
  return *this;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class ArithmeticCoderFilter32 {
  public:
    ArithmeticCoderFilter32 & initializeEncoder();
    ArithmeticCoderFilter32 & encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL);
    ArithmeticCoderFilter32 & flush(void * out,uintptr_t * wb);
    ArithmeticCoderFilter32 & encode(AsyncFile & inp,AsyncFile & out);

    ArithmeticCoderFilter32 & initializeDecoder();
    ArithmeticCoderFilter32 & decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL);
    ArithmeticCoderFilter32 & decode(AsyncFile & inp,AsyncFile & out);
  private:
	  static const uint32_t maxRange_ = 0x3fff;

    // encoder stateful members
    uint32_t eFreq_[257];
    uint32_t eLow_;
    uint32_t eHigh_;
    uint32_t underflowCount_;
    uint32_t ec_;
    uint8_t eb_;
    uint8_t ei_;

    // decoder stateful members
    uint32_t dFreq_[257];
    uint32_t code_;
    uint32_t dLow_;
    uint32_t dHigh_;
    uint32_t dc_;
    uint32_t dii_;
    uint8_t db_;
    uint8_t di_;

    enum { stInit, stFlush, stInp, stOut, stOut2 } eState_, dState_;

    void rescale(uint32_t * freq)
    {
	    for( uint32_t i = 1; i < 257; i++ ){
		    freq[i] /= 2;
		    if( freq[i] <= freq[i - 1] ) freq[i] = freq[i - 1] + 1;
	    }
    }

    void update(uint32_t c,uint32_t * freq)
    {
      for( uint32_t j = c + 1; j < 257; j++ ) freq[j]++;
		  if( freq[256] >= maxRange_ ) rescale(freq);
    }

    bool writeBit(uint32_t bit,void * & out,uintptr_t & outSize,uintptr_t * wb)
    {
      if( ei_ < 8 ){
        eb_ |= uint8_t(bit << ei_);
        ei_++;
        return false;
      }
      else if( outSize > 0 ){
        *(uint8_t *) out = eb_;
        eb_ = uint8_t(bit);
        ei_ = 1;
        out = (uint8_t *) out + sizeof(uint8_t);
        outSize -= sizeof(uint8_t);
        if( wb != NULL ) *wb += sizeof(uint8_t);
        return false;
      }
      return true;
    }

    bool readBit(uint8_t & bit,const void * & inp,uintptr_t & inpSize,uintptr_t * rb)
    {
      if( di_ < 8 ){
        bit = (db_ >> di_) & 1;
        di_++;
        return false;
      }
      else if( inpSize > 0 ){
        bit = (db_ = *(const uint8_t *) inp) & 1;
        di_ = 1;
        inp = (const uint8_t *) inp + sizeof(uint8_t);
        inpSize -= sizeof(uint8_t);
        if( rb != NULL ) *rb += sizeof(uint8_t);
        return false;
      }
      return true;
    }
};
//------------------------------------------------------------------------------
ArithmeticCoderFilter32 & ArithmeticCoderFilter32::initializeEncoder()
{
  eLow_ = 0;
  eHigh_ = 0xffff;
  underflowCount_ = 0;
  eb_ = 0;
  ei_ = 0;
	for( uint32_t i = 0; i < sizeof(eFreq_) / sizeof(eFreq_[0]); i++ ) eFreq_[i] = uint32_t(i);
  eState_ = stInit;
  return *this;
}
//------------------------------------------------------------------------------
ArithmeticCoderFilter32 & ArithmeticCoderFilter32::encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb,uintptr_t * wb)
{
  if( eState_ == stOut ) goto out;
  if( eState_ == stOut2 ) goto out2;
  for(;;){
    if( inpSize == 0 ){
      eState_ = stInp;
      return *this;
    }
    ec_ = *(const uint8_t *) inp;
    inp = (const uint8_t *) inp + sizeof(uint8_t);
    inpSize -= sizeof(uint8_t);
    if( rb != NULL ) *rb += sizeof(uint8_t);

    uint32_t symbolLow = eFreq_[ec_], symbolHigh = eFreq_[ec_ + 1], totalRange = eFreq_[256];
	  uint32_t tempRange = (eHigh_ - eLow_) + 1;
	  eHigh_ = eLow_ + ((tempRange * symbolHigh) / totalRange) - 1;
	  eLow_ = eLow_ + ((tempRange * symbolLow) / totalRange);

	  for(;;){
		  if( (eHigh_ & 0x8000) == (eLow_ & 0x8000) ){
out:    if( writeBit(eHigh_ >> 15,out,outSize,wb) ){
          eState_ = stOut;
          return *this;
        }
			  while( underflowCount_ > 0 ){
out2:     if( writeBit((eHigh_ >> 15) ^ 1,out,outSize,wb) ){
            eState_ = stOut2;
            return *this;
          }
				  underflowCount_--;
			  }
		  }
		  else {
			  if( (eLow_	& 0x4000) && !(eHigh_ & 0x4000) ){
				  underflowCount_++;
				  eLow_ &= 0x3FFF;
				  eHigh_ |= 0x4000;
			  }
			  else
				  break;
		  }
		  eLow_ = (eLow_ << 1) & 0xFFFF;
		  eHigh_ = ((eHigh_ << 1) | 1) & 0xFFFF;
	  }

    update(ec_,eFreq_);
	}
  return *this;
}
//------------------------------------------------------------------------------
ArithmeticCoderFilter32 & ArithmeticCoderFilter32::flush(void * out,uintptr_t * wb)
{
  void * out2 = out;
  uintptr_t outSize = ~uintptr_t(0);
  writeBit((eLow_ >> 14) & 1,out2,outSize,wb);
  underflowCount_++;
  while( underflowCount_ > 0 ){
	  writeBit(((eLow_ >> 14) ^ 1) & 1,out2,outSize,wb);
		underflowCount_--;
  }
  for( uintptr_t i = ei_; i <= 8; i++ ) writeBit(0,out2,outSize,wb);
  return *this;
}
//------------------------------------------------------------------------------
ArithmeticCoderFilter32 & ArithmeticCoderFilter32::encode(AsyncFile & inp,AsyncFile & out)
{
  uintptr_t rbs = getpagesize() * 16;
  uintptr_t wbs = getpagesize() * 16;
  AutoPtr<uint8_t> inpBuffer((uint8_t *) kmalloc(rbs));
  AutoPtr<uint8_t> outBuffer((uint8_t *) kmalloc(wbs));
  uintptr_t rb = 0, wb = 0;
  int64_t r;
  while( (r = inp.read(inpBuffer,rbs)) > 0 ){
    while( rb < uintptr_t(r) ){
      encodeBuffer(&inpBuffer[rb],uintptr_t(r) - rb,&outBuffer[wb],wbs - wb,&rb,&wb);
      if( wb == wbs ){
        out.writeBuffer(outBuffer,wbs);
        wb = 0;
      }
    }
    rb = 0;
  }
  out.writeBuffer(outBuffer,wb);
  wb = 0;
  flush(outBuffer,&wb);
  out.writeBuffer(outBuffer,wb);
  return *this;
}
//------------------------------------------------------------------------------
ArithmeticCoderFilter32 & ArithmeticCoderFilter32::initializeDecoder()
{
  dLow_ = 0;
  dHigh_ = 0xffff;
	for( uint32_t i = 0; i < sizeof(dFreq_) / sizeof(dFreq_[0]); i++ ) dFreq_[i] = uint32_t(i);
	code_ = 0;
  db_ = 0;
  di_ = 8;
  dii_ = 0;
  dState_ = stInit;
  return *this;
}
//------------------------------------------------------------------------------
ArithmeticCoderFilter32 & ArithmeticCoderFilter32::decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb,uintptr_t * wb)
{
  if( dState_ == stInp ) goto inp;
  if( dState_ == stOut ) goto out;
  uint8_t bit;
  while( dii_ < 16 ){
    if( readBit(bit,inp,inpSize,rb) ) return *this;
	  code_ = (code_ << 1) | bit;
    dii_++;
  }
  for(;;){
    uint32_t totalRange = dFreq_[256], tempRange = (dHigh_ - dLow_) + 1;
    uint32_t count = (uint32_t)(((((code_ - dLow_) + 1) * (uint64_t) totalRange) - 1) / tempRange);

    for( dc_ = 255; dFreq_[dc_] > count; dc_-- );
out:
    if( outSize == 0 ){
      dState_ = stOut;
      return *this;
    }	     
    *(uint8_t *) out = uint8_t(dc_);
    out = (uint8_t *) out + sizeof(uint8_t);
    outSize -= sizeof(uint8_t);
    if( wb != NULL ) *wb += sizeof(uint8_t);

    uint32_t symbolLow = dFreq_[dc_], symbolHigh = dFreq_[dc_ + 1];
	  dHigh_ = dLow_ + ((tempRange * symbolHigh) / totalRange) - 1;
	  dLow_ = dLow_ + ((tempRange * symbolLow) / totalRange);

	  for(;;){
		  if( (dHigh_ & 0x8000) == (dLow_ & 0x8000) ){
		  }
		  else {
			  if( (dLow_ & 0x4000) && !(dHigh_ & 0x4000) ){
				  code_ ^= 0x4000;
				  dLow_ &= 0x3FFF;
				  dHigh_ |= 0x4000;
			  }
			  else
				  break;
		  }
		  dLow_ = (dLow_ << 1) & 0xFFFF;
		  dHigh_ = ((dHigh_ << 1) | 1) & 0xFFFF;
inp:
      if( readBit(bit,inp,inpSize,rb) ){
        dState_ = stInp;
        return *this;
      }
	    code_ = ((code_ << 1) | bit) & 0xFFFF;
	  }

    update(dc_,dFreq_);
  }
  return *this;
}
//------------------------------------------------------------------------------
ArithmeticCoderFilter32 & ArithmeticCoderFilter32::decode(AsyncFile & inp,AsyncFile & out)
{
  uintptr_t rbs = getpagesize() * 16;
  uintptr_t wbs = getpagesize() * 16;
  AutoPtr<uint8_t> inpBuffer((uint8_t *) kmalloc(rbs));
  AutoPtr<uint8_t> outBuffer((uint8_t *) kmalloc(wbs));
  uintptr_t rb = 0, wb = 0;
  int64_t r;
  while( (r = inp.read(inpBuffer,rbs)) > 0 ){
    while( rb < uintptr_t(r) ){
      decodeBuffer(&inpBuffer[rb],uintptr_t(r) - rb,&outBuffer[wb],wbs - wb,&rb,&wb);
      if( wb == wbs ){
        out.writeBuffer(outBuffer,wbs);
        wb = 0;
      }
    }
    rb = 0;
  }
  out.writeBuffer(outBuffer,wb);
  return *this;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class LZKFilter {
  public:
    LZKFilter & initializeEncoder();
    LZKFilter & encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL);
    LZKFilter & flush(void * out,uintptr_t * wb = NULL);
    LZKFilter & encode(AsyncFile & inp,AsyncFile & out);
    LZKFilter & initializeDecoder();
    LZKFilter & decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL);
    LZKFilter & decode(AsyncFile & inp,AsyncFile & out);
  protected:
    struct Node;
    struct TreeParams {
      LZKFilter * filter_;
      Node * dict_;
      uintptr_t dmsk_;
      uintptr_t mlen_;
      Node * bestMatchNode_;
      uintptr_t bestMatchLen_;
    };
    struct PACKED Node {
      mutable RBTreeNode treeNode_;
      mutable uint32_t idx_;
      mutable uint8_t c_;
      mutable uint8_t alignment_[(sizeof(RBTreeNode) + 2 + 1 < 16 ? 16 : 32) - sizeof(RBTreeNode) - 2 - 1];

      Node & operator = (const Node & )
      {
        assert(0);
        return *this;
      }

      static RBTreeNode & treeO2N(const Node & object,TreeParams *){
        return object.treeNode_;
      }

      static Node & treeN2O(const RBTreeNode & node,TreeParams *){
        Node * p = NULL;
        return node.object<Node>(p->treeNode_);
      }

      static intptr_t treeCO(const Node & a0,const Node & a1,TreeParams * params){
        intptr_t c, s1 = &a0 - params->dict_, s2 = &a1 - params->dict_;
        uintptr_t l = params->mlen_;
        do {
          c = intptr_t(params->dict_[s1].c_) - params->dict_[s2].c_;
          if( c != 0 ) break;
          s1 = (s1 + 1) & params->dmsk_;
          s2 = (s2 + 1) & params->dmsk_;
        } while( --l > 0 );
        l = params->mlen_ - l;
        if( l > params->bestMatchLen_ ){
          params->bestMatchNode_ = params->dict_ + s2 - l;
          params->bestMatchLen_ = l;
        }
        return c;
      }
    };

    typedef
      RBTree<
        Node,
        TreeParams,
        Node::treeO2N,
        Node::treeN2O,
        Node::treeCO
    > NodeTree;
    NodeTree tree_;
    TreeParams params_;

    // encoder stateful variables
    AutoPtr<Node> nodes_;
    Node * dict_;
    uintptr_t dpos_;
    uintptr_t dcnt_; // dict size
    uintptr_t dmsk_; // dict mask
    uintptr_t dcsz_;
    uintptr_t dlsz_;
    uintptr_t alen_; // look ahead string length
    uintptr_t mlen_; // maximum look ahead string length
    intptr_t dlen_;
    uintmax_t dbits_;
    uintptr_t dbiti_;
    uintptr_t dcode_;
    uintptr_t dcount_;

    // decoder stateful variables
    AutoPtr<uint8_t> ddict_;
    uintptr_t ddpos_;
    uintptr_t dcpos_;
    uintptr_t ddcnt_; // decode dict size
    uintptr_t ddmsk_; // decode dict mask
    uintptr_t dmlen_;
    uintptr_t ddcsz_;
    uintptr_t ddlsz_;
    uintptr_t ddlen_;
    uintmax_t ddbits_;
    uintptr_t ddbiti_;
    uintptr_t ddbitc_;
    uintptr_t ddcode_;
    uintptr_t ddcount_;
    uintptr_t ddci_;

    enum { stInit, stInp, stInp2, stInp3, stOut } eState_, dState_;

    bool writeBits(uintptr_t & bits,uintptr_t & count,void * & out,uintptr_t & outSize,uintptr_t * wb)
    {
      while( count > 0 ){
        if( dbiti_ >= sizeof(dbits_) * 8u ){
          if( outSize == 0 ) return true;
          uintptr_t sz = tmin(dbiti_ >> 3,outSize);
          memcpy(out,&dbits_,sz);
          out = (uint8_t *) out + sz;
          outSize -= sz;
          if( wb != NULL ) *wb += sz;
          sz <<= 3;
          if( sz == sizeof(dbits_) * 8u ) dbits_ = 0; else dbits_ >>= sz;
          dbiti_ -= sz;
        }
        uintptr_t bc = tmin(count,sizeof(dbits_) * 8u - dbiti_);
        dbits_ |= uintmax_t(bits & ~(~uintptr_t(0) << bc)) << dbiti_;
        count -= bc;
        dbiti_ += bc;
        bits >>= bc;
      }
      return false;
    }

    bool readBits(uintptr_t & bits,uintptr_t & pos,uintptr_t & count,const void * & inp,uintptr_t & inpSize,uintptr_t * rb)
    {
      while( count > 0 ){
        if( ddbiti_ >= ddbitc_ ){
          if( inpSize == 0 ) return true;
          uintptr_t sz = tmin(sizeof(ddbits_),inpSize);
          memcpy(&ddbits_,inp,sz);
          inp = (const uint8_t *) inp + sz;
          inpSize -= sz;
          if( rb != NULL ) *rb += sz;
          ddbiti_ = 0;
          ddbitc_ = sz << 3;
        }
        uintptr_t bc = tmin(count,ddbitc_ - ddbiti_);
        bits |= (uintptr_t(ddbits_ >> ddbiti_) & ~(~uintptr_t(0) << bc)) << pos;
        count -= bc;
        ddbiti_ += bc;
        pos += bc;
      }
      return false;
    }

    intptr_t getFirstBitIndex(uintptr_t bits)
    {
      intptr_t i;
      for( i = sizeof(bits) * 8u - 1; i >= 0 && (bits & (uintptr_t(1) << i)) == 0; i-- );
      return i;
    }
  private:
};
//------------------------------------------------------------------------------
LZKFilter & LZKFilter::initializeEncoder()
{
  dpos_ = 0;
  alen_ = 0;
  mlen_ = 8;
  dlen_ = 0;
  eState_ = stInit;
  dcnt_ = 65536;
  dmsk_ = dcnt_ - 1;
  dcsz_ = getFirstBitIndex(dcnt_);
  dlsz_ = getFirstBitIndex(mlen_);
  dbits_ = 0;
  dbiti_ = 0;
  nodes_.reallocT(dcnt_);
  dict_ = nodes_;
  for( intptr_t i = dmsk_; i >= 0; i-- ){
    dict_[i].treeNode_.parent_ = NULL;
    dict_[i].idx_ = uint32_t(i);
    dict_[i].c_ = 0;
  }
  params_.filter_ = this;
  params_.dict_ = dict_;
  params_.dmsk_ = dmsk_;
  params_.mlen_ = mlen_;
  params_.bestMatchNode_ = NULL;
  params_.bestMatchLen_ = 0;
  tree_.param(&params_);
  return *this;
}
//------------------------------------------------------------------------------
LZKFilter & LZKFilter::encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb,uintptr_t * wb)
{
  if( eState_ == stInit ){
    while( alen_ < mlen_ ){
      if( inpSize == 0 ){
        eState_ = stInp;
        return *this;
      }
      dict_[(dpos_ + alen_) & dmsk_].c_ = *(const uint8_t *) inp;
      alen_ += sizeof(uint8_t);
      inp = (const uint8_t *) inp + sizeof(uint8_t);
      inpSize -= sizeof(uint8_t);
      if( rb != NULL ) *rb += sizeof(uint8_t);
    }
    goto init;
  }
  if( eState_ == stInp ) goto inp;
  if( eState_ == stOut ) goto out;
  for(;;){
    do {
inp:  Node * p = dict_ + ((dpos_ + mlen_) & dmsk_);
      if( p->treeNode_.parent_ != NULL ) tree_.remove(*p);
      if( inpSize == 0 ){
        eState_ = stInp;
        return *this;
      }
      p->c_ = *(const uint8_t *) inp;
      inp = (const uint8_t *) inp + sizeof(uint8_t);
      inpSize -= sizeof(uint8_t);
      if( rb != NULL ) *rb += sizeof(uint8_t);
      dpos_ = (dpos_ + sizeof(uint8_t)) & dmsk_;
init: params_.bestMatchNode_ = dict_ + dict_[dpos_].c_;
      params_.bestMatchLen_ = 1;
      tree_.insert(dict_[dpos_],false,false);
    } while( --dlen_ >= 0 );
    dlen_ = params_.bestMatchLen_ - 1;
    if( dlen_ == 0 ){
      dcode_ = ((params_.bestMatchNode_ - dict_) << 1) | 1u;
      dcount_ = 8 + 1;
    }
    else {
      dcode_ = (((params_.bestMatchNode_ - dict_) << dlsz_) | dlen_) << 1;
      dcount_ = dcsz_ + dlsz_ + 1;
    }
out:
    if( writeBits(dcode_,dcount_,out,outSize,wb) ){
      eState_ = stOut;
      return *this;
    }
  }
  return *this;
}
//------------------------------------------------------------------------------
LZKFilter & LZKFilter::flush(void * out,uintptr_t * wb)
{
  if( eState_ == stInit ) return *this;
  uintptr_t outSize = ~uintptr_t(0);
  for(;;){
    do {
      dict_[(dpos_ + mlen_) & dmsk_].c_ = 0;
      dpos_ = (dpos_ + 1) & dmsk_;
      if( --alen_ == 0 ){
        eState_ = stInit;
        goto exit;
      }
    } while( --dlen_ >= 0 );
    params_.bestMatchNode_ = dict_ + dict_[dpos_].c_;
    params_.bestMatchLen_ = 1;
    tree_.find(dict_[dpos_]);
    dlen_ = tmin(alen_,params_.bestMatchLen_) - 1;
    if( dlen_ == 0 ){
      dcode_ = ((params_.bestMatchNode_ - dict_) << 1) | 1u;
      dcount_ = 8 + 1;
    }
    else {
      dcode_ = (((params_.bestMatchNode_ - dict_) << dlsz_) | dlen_) << 1;
      dcount_ = dcsz_ + dlsz_ + 1;
    }
    writeBits(dcode_,dcount_,out,outSize,wb);
  }
exit:
  uintptr_t sz = (dbiti_ >> 3) + ((dbiti_ & 7) != 0);
  memcpy(out,&dbits_,sz);
  out = (uint8_t *) out + sz;
  if( wb != NULL ) *wb += sz;
  return *this;
}
//------------------------------------------------------------------------------
LZKFilter & LZKFilter::encode(AsyncFile & inp,AsyncFile & out)
{
  uintptr_t rbs = getpagesize() * 16;
  uintptr_t wbs = getpagesize() * 16;
  AutoPtr<uint8_t> inpBuffer((uint8_t *) kmalloc(rbs));
  AutoPtr<uint8_t> outBuffer((uint8_t *) kmalloc(wbs));
  uintptr_t rb = 0, wb = 0;
  int64_t r;
  while( (r = inp.read(inpBuffer,rbs)) > 0 ){
    while( rb < uintptr_t(r) ){
      encodeBuffer(&inpBuffer[rb],uintptr_t(r) - rb,&outBuffer[wb],wbs - wb,&rb,&wb);
      if( wb == wbs ){
        out.writeBuffer(outBuffer,wbs);
        wb = 0;
      }
    }
    rb = 0;
  }
  out.writeBuffer(outBuffer,wb);
  wb = 0;
  flush(outBuffer,&wb);
  out.writeBuffer(outBuffer,wb);
  return *this;
}
//------------------------------------------------------------------------------
LZKFilter & LZKFilter::initializeDecoder()
{
  ddpos_ = 0;
  ddcnt_ = 65536;
  ddmsk_ = dcnt_ - 1;
  dmlen_ = 8;
  ddcsz_ = getFirstBitIndex(ddcnt_);
  ddlsz_ = getFirstBitIndex(dmlen_);
  ddbits_ = 0;
  ddbiti_ = 0;
  ddbitc_ = 0;
  dState_ = stInit;
  ddict_.reallocT(ddcnt_);
  memset(ddict_,0,ddcnt_);
  ddcode_ = 0;
  ddcount_ = 1;
  ddci_ = 0;
  return *this;
}
//------------------------------------------------------------------------------
LZKFilter & LZKFilter::decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb,uintptr_t * wb)
{
  uint8_t * dict = ddict_;
  if( dState_ == stOut ) goto out;
  if( dState_ == stInp2 ) goto inp2;
  if( dState_ == stInp3 ) goto inp3;
  for(;;){
    if( readBits(ddcode_,ddci_,ddcount_,inp,inpSize,rb) ){
      dState_ = stInp;
      return *this;
    }
    ddci_ = 0;
    if( ddcode_ == 0 ){
      ddcount_ = ddcsz_ + ddlsz_;
inp2:
      if( readBits(ddcode_,ddci_,ddcount_,inp,inpSize,rb) ){
        dState_ = stInp2;
        return *this;
      }
    }
    else {
      ddcode_ = 0;
      ddcount_ = 8;
inp3:
      if( readBits(ddcode_,ddci_,ddcount_,inp,inpSize,rb) ){
        dState_ = stInp3;
        return *this;
      }
      ddcode_ <<= ddlsz_;
    }
    ddlen_ = (ddcode_ & ~(~uintptr_t(0) << ddlsz_)) + 1;
    if( ddlen_ == 1 ){
      dict[dcpos_ = ddpos_] = uint8_t(ddcode_ >> ddlsz_);
      goto out;
    }
    dcpos_ = ddcode_ >> ddlsz_;
    do {
out:  if( outSize == 0 ){
        dState_ = stOut;
        return *this;
      }
      *(uint8_t *) out = dict[ddpos_] = dict[dcpos_];
      out = (uint8_t *) out + 1;
      outSize -= 1;
      if( wb != NULL ) *wb += 1;
      ddpos_ = (ddpos_ + 1) & ddmsk_;
      dcpos_ = (dcpos_ + 1) & ddmsk_;
    } while( --ddlen_ > 0 );
    ddcode_ = 0;
    ddcount_ = 1;
    ddci_ = 0;
  }
  return *this;
}
//------------------------------------------------------------------------------
LZKFilter & LZKFilter::decode(AsyncFile & inp,AsyncFile & out)
{
  uintptr_t rbs = getpagesize() * 16;
  uintptr_t wbs = getpagesize() * 16;
  AutoPtr<uint8_t> inpBuffer((uint8_t *) kmalloc(rbs));
  AutoPtr<uint8_t> outBuffer((uint8_t *) kmalloc(wbs));
  uintptr_t rb = 0, wb = 0;
  int64_t r;
  while( (r = inp.read(inpBuffer,rbs)) > 0 ){
    while( rb < uintptr_t(r) ){
      decodeBuffer(&inpBuffer[rb],uintptr_t(r) - rb,&outBuffer[wb],wbs - wb,&rb,&wb);
      if( wb == wbs ){
        out.writeBuffer(outBuffer,wbs);
        wb = 0;
      }
    }
    rb = 0;
  }
  out.writeBuffer(outBuffer,wb);
  return *this;
}
//------------------------------------------------------------------------------
int main(int _argc,char * _argv[])
{
  //{
    //intmax_t aa = 0x10000000i64, bb = 0x10000001i64, cc;
    //cc = aa * bb;
  //  intmax_t aa = 0, bb = -1, cc;
  //  cc = aa - bb;

  //  VarInteger a, b, c;

  //  for( uintptr_t i = 33; i < 300; i++ ){
  //    a = 0xFFFFFFFFFFFFFF00ui64;
  //    a <<= i;
  //  }

  //  a = 6ui64;
  //  b = 10;
  //  c = a - b;


  //  a = 0xFFFFFFFFFFFFFFFFi64;
  //  b = 0xFFFFFFFFFFFFFFFFi64;
  //  c = a + b;
  //  c = c + -2;
  //  b = -b;
  //  a = 3;
  //  b = 5;
  //  c = a * b;
  //  AutoPtr<char> s((char *) kmalloc(c.print() + 1));
  //  c.print(s);
  //  a = 0x100000000ui64;
  //  b = 3;
  //  c = a * b;
  //  AutoPtr<char> s2((char *) kmalloc(c.print(NULL,16) + 1));
  //  c.print(s2,16);
  //  c = c;
  //  ldouble r = sqrt(ldouble(2));
  //  VarNumber sqrt(2);
  //  sqrt = sqrt.sqrt();
  //  AutoPtr<char> s3((char *) kmalloc(sqrt.print() + 1));
  //  sqrt.print(s3);
  //  sqrt = sqrt;
  //}

  ////Sleep(15000);
  int errcode = EINVAL;
  adicpp::AutoInitializer autoInitializer(_argc,_argv);
  autoInitializer = autoInitializer;

  try {
// tests
    ksys::setProcessPriority("REALTIME_PRIORITY_CLASS");

    AsyncFile file("C:/Korvin/trunk/test/kvm/test.txt");
    AsyncFile::LineGetBuffer lgb(file);
    file.open();

    AsyncFile encFile("C:/Korvin/trunk/test/kvm/test.enc");
    encFile.createIfNotExist(true).open().resize(0);

    AsyncFile decFile("C:/Korvin/trunk/test/kvm/test.dec");
    decFile.createIfNotExist(true).open().resize(0);

    uint64_t ellapsed;

    //RangeCoderFilter32 filter2;
    //filter2.initializeEncoder();
    //ellapsed = gettimeofday();
    //filter2.encode(file,encFile);
    //ellapsed = gettimeofday() - ellapsed;
    //fprintf(stderr,"encode: ellapsed %s, %lf kbps, ratio %lf\n",
    //  (const char *) utf8::elapsedTime2Str(ellapsed).getOEMString(),
    //  (file.size() * 1000000. / ellapsed) / 1024,
    //  100 - encFile.size() * 100. / file.size()
    //);
    //filter2.initializeDecoder();
    //ellapsed = gettimeofday();
    //filter2.decode(encFile.seek(0),decFile.seek(0));
    //ellapsed = gettimeofday() - ellapsed;
    //fprintf(stderr,"decode: ellapsed %s, %lf kbps\n",
    //  (const char *) utf8::elapsedTime2Str(ellapsed).getOEMString(),
    //  (decFile.size() * 1000000. / (ellapsed + (ellapsed == 0))) / 1024
    //);

    //RangeCoderFilter64 filter3;
    //filter3.initializeEncoder();
    //ellapsed = gettimeofday();
    //filter3.encode(file.seek(0),encFile.seek(0).resize(0));
    //ellapsed = gettimeofday() - ellapsed;
    //fprintf(stderr,"encode: ellapsed %s, %lf kbps, ratio %lf\n",
    //  (const char *) utf8::elapsedTime2Str(ellapsed).getOEMString(),
    //  (file.size() * 1000000. / ellapsed) / 1024,
    //  100 - encFile.size() * 100. / file.size()
    //);
    //filter3.initializeDecoder();
    //ellapsed = gettimeofday();
    //filter3.decode(encFile.seek(0),decFile.seek(0).resize(0));
    //ellapsed = gettimeofday() - ellapsed;
    //fprintf(stderr,"decode: ellapsed %s, %lf kbps\n",
    //  (const char *) utf8::elapsedTime2Str(ellapsed).getOEMString(),
    //  (decFile.size() * 1000000. / (ellapsed + (ellapsed == 0))) / 1024
    //);

    //return 0;

    //ArithmeticCoderFilter32 filter4;
    //filter4.initializeEncoder();
    //ellapsed = gettimeofday();
    //filter4.encode(file.seek(0),encFile.seek(0).resize(0));
    //ellapsed = gettimeofday() - ellapsed;
    //fprintf(stderr,"encode: ellapsed %s, %lf kbps, ratio %lf\n",
    //  (const char *) utf8::elapsedTime2Str(ellapsed).getOEMString(),
    //  (file.size() * 1000000. / ellapsed) / 1024,
    //  100 - encFile.size() * 100. / file.size()
    //);
    //filter4.initializeDecoder();
    //ellapsed = gettimeofday();
    //filter4.decode(encFile.seek(0),decFile.seek(0).resize(0));
    //ellapsed = gettimeofday() - ellapsed;
    //fprintf(stderr,"decode: ellapsed %s, %lf kbps\n",
    //  (const char *) utf8::elapsedTime2Str(ellapsed).getOEMString(),
    //  (decFile.size() * 1000000. / (ellapsed + (ellapsed == 0))) / 1024
    //);

    //return 0;

    LZKFilter filter;
    filter.initializeEncoder();
    ellapsed = gettimeofday();
    filter.encode(file.seek(0),encFile.seek(0).resize(0));
    ellapsed = gettimeofday() - ellapsed;
    fprintf(stderr,"encode: ellapsed %s, %lf kbps, ratio %lf\n",
      (const char *) utf8::elapsedTime2Str(ellapsed).getOEMString(),
      (file.size() * 1000000. / ellapsed) / 1024,
      100 - encFile.size() * 100. / file.size()
    );
    filter.initializeDecoder();
    ellapsed = gettimeofday();
    filter.decode(encFile.seek(0),decFile.seek(0).resize(0));
    ellapsed = gettimeofday() - ellapsed;
    fprintf(stderr,"decode: ellapsed %s, %lf kbps\n",
      (const char *) utf8::elapsedTime2Str(ellapsed).getOEMString(),
      (decFile.size() * 1000000. / (ellapsed + (ellapsed == 0))) / 1024
    );
    return 0;
//

    stdErr.fileName(includeTrailingPathDelimiter(SYSLOG_DIR(kvm_version.tag_)) + kvm_version.tag_ + ".log");
    Config::defaultFileName(SYSCONF_DIR("") + kvm_version.tag_ + ".conf");
    ConfigSP config(newObject<InterlockedConfig<FiberInterlockedMutex> >());
    Array<utf8::String> sources;
    for( uintptr_t i = 1; i < argv().count(); i++ ){
      if( argv()[i].strcmp("--version") == 0 ){
        stdErr.debug(9,utf8::String::Stream() << kvm_version.tex_ << "\n");
        fprintf(stdout,"%s\n",kvm_version.tex_);
        break;
      }
      else if( argv()[i].strcmp("--chdir") == 0 && i + 1 < argv().count() ){
        changeCurrentDir(argv()[++i]);
      }
      else if( argv()[i].strcmp("--log") == 0 && i + 1 < argv().count() ){
        stdErr.fileName(argv()[++i]);
      }
      else if( argv()[i].strcmp("-c") == 0 && i + 1 < argv().count() ){
        Config::defaultFileName(argv()[i + 1]);
        config->fileName(argv()[++i]);
      }
      else {
        sources.add(argv()[i]);
      }
    }
    config->silent(true).parse().override();
    stdErr.rotationThreshold(
      config->value("debug_file_rotate_threshold",1024 * 1024)
    );
    stdErr.rotatedFileCount(
      config->value("debug_file_rotate_count",10)
    );
    stdErr.setDebugLevels(
      config->value("debug_levels","+0,+1,+2,+3")
    );
    stdErr.fileName(
      config->value("log_file",stdErr.fileName())
    );
    stdErr.debug(0,
      utf8::String::Stream() << kvm_version.gnu_ << " started\n"
    );
    config->silent(false);
    utf8::String defaultConfigSectionName(config->text("default_config",kvm_version.gnu_));
    utf8::String defaultConnectionSectionName(config->textByPath(defaultConfigSectionName + ".connection","default_connection"));
    AutoPtr<Database> database(Database::newDatabase(&config->section(defaultConnectionSectionName)));
    
    for( uintptr_t i = 0; i < sources.count(); i++ ){
      AutoPtr<wchar_t> fileName(coco_string_create(sources[i].getUNICODEString()));
      AutoPtr<Scanner> scanner(newObjectV1<Scanner>(fileName.ptr()));
      AutoPtr<Parser> parser(newObjectV1<Parser>(scanner.ptr()));
      parser->gen = newObject<CodeGenerator>();
      parser->tab = newObjectV1<SymbolTable>(parser->gen.ptr());
	    parser->Parse();
	    if( parser->errors->count > 0 ){
        exit(EINVAL);
	    }
      Compiler compiler;
      compiler.detect(config);
      compiler.test(includeTrailingPathDelimiter(getPathFromPathName(sources[i])) + "config.h");
      parser->gen->generate(compiler,changeFileExt(sources[i],".cxx"));
    }
    stdErr.debug(0,
      utf8::String::Stream() << kvm_version.gnu_ << " stopped\n"
    );
    errcode = 0;
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    errcode = e->code() >= errorOffset ? e->code() - errorOffset : e->code();
  }
  catch( ... ){
  }
  return errcode;
}
//------------------------------------------------------------------------------
