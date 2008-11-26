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
//---------------------------------------------------------------------------
#ifndef stmfltH
#define stmfltH
//---------------------------------------------------------------------------
#ifndef STREAM_FILTER_ABSTRATION
#define _NO_EXCEPTIONS 1
//#include <adicpp/lzma/7zip/Compress/LZMA/LZMAEncoder.h>
//#include <adicpp/lzma/7zip/Compress/LZMA/LZMADecoder.h>
#undef _NO_EXCEPTIONS
#endif
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class StreamCRCFilter {
  public:
    virtual ~StreamCRCFilter();
    StreamCRCFilter();

    virtual StreamCRCFilter & make(const void * inp,uintptr_t inpSize,void * crc,uintptr_t * crcSize = NULL) = 0;

  protected:
  private:
    StreamCRCFilter(const StreamCRCFilter &){}
    void operator = (const StreamCRCFilter &){}
};
//---------------------------------------------------------------------------
inline StreamCRCFilter::~StreamCRCFilter()
{
}
//---------------------------------------------------------------------------
inline StreamCRCFilter::StreamCRCFilter()
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class StreamCryptFilter {
  public:
    virtual ~StreamCryptFilter();
    StreamCryptFilter();

    virtual StreamCryptFilter * initializeCrypting(const void * key,uintptr_t keyLen) = 0;
    virtual StreamCryptFilter * encrypt(void * dst,const void * src,uintptr_t size) = 0;
    virtual StreamCryptFilter * initializeDecrypting(const void * key,uintptr_t keyLen) = 0;
    virtual StreamCryptFilter * decrypt(void * dst,const void * src,uintptr_t size) = 0;
  protected:
  private:
    StreamCryptFilter(const StreamCryptFilter &){}
    void operator = (const StreamCryptFilter &){}
};
//---------------------------------------------------------------------------
inline StreamCryptFilter::~StreamCryptFilter()
{
}
//---------------------------------------------------------------------------
inline StreamCryptFilter::StreamCryptFilter()
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class SHA256CryptFilter : public StreamCryptFilter {
  public:
    virtual ~SHA256CryptFilter() {}
    SHA256CryptFilter() {}

    SHA256CryptFilter * initializeCrypting(const void * key,uintptr_t keyLen) { encryptor_.init(key,keyLen); return this; }
    SHA256CryptFilter * encrypt(void * dst,const void * src,uintptr_t size) { encryptor_.crypt(dst,src,size); return this; }
    SHA256CryptFilter * initializeDecrypting(const void * key,uintptr_t keyLen) { decryptor_.init(key,keyLen); return this; }
    SHA256CryptFilter * decrypt(void * dst,const void * src,uintptr_t size) { decryptor_.crypt(dst,src,size); return this; }
  protected:
    SHA256Cryptor encryptor_;
    SHA256Cryptor decryptor_;
  private:
    SHA256CryptFilter(const SHA256CryptFilter &) {}
    void operator = (const SHA256CryptFilter &) {}
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class StreamCompressionFilter {
  public:
    virtual ~StreamCompressionFilter();
    StreamCompressionFilter();

    virtual StreamCompressionFilter * initializeEncoder() = 0;
    virtual StreamCompressionFilter * encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL) = 0;
    virtual StreamCompressionFilter * flush(void * out,uintptr_t * wb) = 0;

    virtual StreamCompressionFilter * initializeDecoder() = 0;
    virtual StreamCompressionFilter * decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL) = 0;

    uint8_t * encoderOutputBuffer() const { return encoderOutputBuffer_; }
    StreamCompressionFilter & encoderOutputBufferSize(uintptr_t v) { encoderOutputBuffer_.reallocT(v); encoderOutputBufferSize_ = v; return *this; }
    const uintptr_t & encoderOutputBufferSize() const { return encoderOutputBufferSize_; }
    StreamCompressionFilter & encoderWriteBytes(uintptr_t v) { encoderWriteBytes_ = v; return *this; }
    const uintptr_t & encoderWriteBytes() const { return encoderWriteBytes_; }

    uint8_t * decoderInputBuffer() const { return decoderInputBuffer_; }
    StreamCompressionFilter & decoderInputBufferSize(uintptr_t v) { decoderInputBuffer_.reallocT(v); decoderInputBufferSize_ = v; return *this; }
    const uintptr_t & decoderInputBufferSize() const { return decoderInputBufferSize_; }
    StreamCompressionFilter & decoderReadBytes(uintptr_t v) { decoderReadBytes_ = v; return *this; }
    const uintptr_t & decoderReadBytes() const { return decoderReadBytes_; }
    StreamCompressionFilter & decoderReadBytes2(uintptr_t v) { decoderReadBytes2_ = v; return *this; }
    const uintptr_t & decoderReadBytes2() const { return decoderReadBytes2_; }
  protected:
    mutable AutoPtrBuffer encoderOutputBuffer_;
    uintptr_t encoderOutputBufferSize_;
    uintptr_t encoderWriteBytes_;

    mutable AutoPtrBuffer decoderInputBuffer_;
    uintptr_t decoderInputBufferSize_;
    uintptr_t decoderReadBytes_;
    uintptr_t decoderReadBytes2_;
  private:
    StreamCompressionFilter(const StreamCompressionFilter &);
    void operator = (const StreamCompressionFilter &);
};
//---------------------------------------------------------------------------
inline StreamCompressionFilter::~StreamCompressionFilter()
{
}
//---------------------------------------------------------------------------
inline StreamCompressionFilter::StreamCompressionFilter() :
  encoderOutputBufferSize_(0),
  encoderWriteBytes_(0),
  decoderInputBufferSize_(0),
  decoderReadBytes_(0),
  decoderReadBytes2_(0)
{
}
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Range32CoderFilter : public StreamCompressionFilter {
  public:
    virtual ~Range32CoderFilter() {}
    Range32CoderFilter() {}

    Range32CoderFilter * initializeEncoder();
    Range32CoderFilter * encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL);
    Range32CoderFilter * flush(void * out,uintptr_t * wb);
    //Range32CoderFilter * encode(AsyncFile & inp,AsyncFile & out);

    Range32CoderFilter * initializeDecoder();
    Range32CoderFilter * decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL);
    //Range32CoderFilter * decode(AsyncFile & inp,AsyncFile & out);
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
inline Range32CoderFilter * Range32CoderFilter::initializeEncoder()
{
  eLow_ = 0;
  eRange_ = int32_t(-1);
	for( uintptr_t i = 0; i < sizeof(eFreq_) / sizeof(eFreq_[0]); i++ ) eFreq_[i] = uint32_t(i);
  eState_ = stInit;
  return this;
}
//------------------------------------------------------------------------------
inline Range32CoderFilter * Range32CoderFilter::encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb,uintptr_t * wb)
{
  uint32_t symbolLow, symbolHigh, totalRange;
  if( eState_ == stOut ) goto out;
  for(;;){
    if( inpSize == 0 ){
      eState_ = stInp;
      return this;
    }
    ec_ = *(const uint8_t *) inp;
    inp = (const uint8_t *) inp + sizeof(uint8_t);
    inpSize -= sizeof(uint8_t);
    if( rb != NULL ) *rb += sizeof(uint8_t);

    symbolLow = eFreq_[ec_];
    symbolHigh = eFreq_[ec_ + 1];
    totalRange = eFreq_[256];
    eLow_ += symbolLow * (eRange_ /= totalRange);
    eRange_ *= symbolHigh - symbolLow;

    while( (eLow_ ^ (eLow_ + eRange_)) < top_ || eRange_ < bottom_ && ((eRange_ = -int32_t(eLow_) & (bottom_ - 1)),1) ){
out:  if( outSize == 0 ){
        eState_ = stOut;
        return this;
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
  return this;
}
//------------------------------------------------------------------------------
inline Range32CoderFilter * Range32CoderFilter::flush(void * out,uintptr_t * wb)
{
	for( uintptr_t i = 0; i < 3; i++ ){
    *(uint8_t *) out = uint8_t(eLow_ >> 24);
    out = (uint8_t *) out + sizeof(uint8_t);
    if( wb != NULL ) *wb += sizeof(uint8_t);
    eLow_ <<= 8;
	}
  return this;
}
//------------------------------------------------------------------------------
//Range32CoderFilter * Range32CoderFilter::encode(AsyncFile & inp,AsyncFile & out)
//{
//  uintptr_t rbs = getpagesize() * 16;
//  uintptr_t wbs = getpagesize() * 16;
//  AutoPtrBuffer inpBuffer((uint8_t *) kmalloc(rbs));
//  AutoPtrBuffer outBuffer((uint8_t *) kmalloc(wbs));
//  uintptr_t rb = 0, wb = 0;
//  int64_t r;
//  while( (r = inp.read(inpBuffer,rbs)) > 0 ){
//    while( rb < uintptr_t(r) ){
//      encodeBuffer(&inpBuffer[rb],uintptr_t(r) - rb,&outBuffer[wb],wbs - wb,&rb,&wb);
//      if( wb == wbs ){
//        out.writeBuffer(outBuffer,wbs);
//        wb = 0;
//      }
//    }
//    rb = 0;
//  }
//  out.writeBuffer(outBuffer,wb);
//  wb = 0;
//  flush(outBuffer,&wb);
//  out.writeBuffer(outBuffer,wb);
//  return *this;
//}
//------------------------------------------------------------------------------
inline Range32CoderFilter * Range32CoderFilter::initializeDecoder()
{
  dLow_ = 0;
  dRange_ = int32_t(-1);
	for( uintptr_t i = 0; i < sizeof(dFreq_) / sizeof(dFreq_[0]); i++ ) dFreq_[i] = uint32_t(i);
	code_ = 0;
  di_ = 0;
  dState_ = stInit;
  return this;
}
//------------------------------------------------------------------------------
inline Range32CoderFilter * Range32CoderFilter::decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb,uintptr_t * wb)
{
  uint32_t symbolLow, symbolHigh, totalRange, count, dc, * freq = dFreq_;
  if( dState_ == stInp ) goto inp;
  if( dState_ == stOut ) goto out;
  while( di_ < 4 ){
    if( inpSize == 0 ) return this;
    code_ = (code_ << 8) | *(const uint8_t *) inp;
    inp = (const uint8_t *) inp + sizeof(uint8_t);
    inpSize -= sizeof(uint8_t);
    if( rb != NULL ) *rb += sizeof(uint8_t);
    di_++;
  }
  for(;;){
    totalRange = dFreq_[256];
    count = (code_ - dLow_) / (dRange_ /= totalRange);
    dc = 255;
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
      return this;
    }	     
    *(uint8_t *) out = uint8_t(dc_);
    out = (uint8_t *) out + sizeof(uint8_t);
    outSize -= sizeof(uint8_t);
    if( wb != NULL ) *wb += sizeof(uint8_t);

    symbolLow = freq[dc_];
    symbolHigh = freq[dc_ + 1];
    dLow_ += symbolLow * dRange_;
    dRange_ *= symbolHigh - symbolLow;

    while( (dLow_ ^ (dLow_ + dRange_)) < top_ || dRange_ < bottom_ && ((dRange_ = -int32_t(dLow_) & bottom_ - 1),1) ){
inp:  if( inpSize == 0 ){
        dState_ = stInp;
        return this;
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
  return this;
}
//------------------------------------------------------------------------------
//Range32CoderFilter * Range32CoderFilter::decode(AsyncFile & inp,AsyncFile & out)
//{
//  uintptr_t rbs = getpagesize() * 16;
//  uintptr_t wbs = getpagesize() * 16;
//  AutoPtrBuffer inpBuffer((uint8_t *) kmalloc(rbs));
//  AutoPtrBuffer outBuffer((uint8_t *) kmalloc(wbs));
//  uintptr_t rb = 0, wb = 0;
//  int64_t r;
//  while( (r = inp.read(inpBuffer,rbs)) > 0 ){
//    while( rb < uintptr_t(r) ){
//      decodeBuffer(&inpBuffer[rb],uintptr_t(r) - rb,&outBuffer[wb],wbs - wb,&rb,&wb);
//      if( wb == wbs ){
//        out.writeBuffer(outBuffer,wbs);
//        wb = 0;
//      }
//    }
//    rb = 0;
//  }
//  out.writeBuffer(outBuffer,wb);
//  return *this;
//}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class LZSSRBTFilter : public StreamCompressionFilter {
  public:
    virtual ~LZSSRBTFilter();
    LZSSRBTFilter();

    LZSSRBTFilter * initializeEncoder();
    LZSSRBTFilter * encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL);
    LZSSRBTFilter * flush(void * out,uintptr_t * wb = NULL);
    //LZSSRBTFilter * encode(AsyncFile & inp,AsyncFile & out);
    LZSSRBTFilter * initializeDecoder();
    LZSSRBTFilter * decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL);
    //LZSSRBTFilter * decode(AsyncFile & inp,AsyncFile & out);
  protected:
#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#elif defined(__BCPLUSPLUS__)
#pragma option push -a1
#endif
    struct PACKED RBTreeNode {
      RBTreeNode * left_;         /* left child */
      RBTreeNode * right_;        /* right child */
      RBTreeNode * parent_;       /* parent */
      uint8_t color_;             /* node color (BLACK, RED) */
      //mutable uint32_t idx_;
      //mutable uint8_t c_;
      uint8_t alignment_[(sizeof(RBTreeNode *) * 3 + 1 < 16 ? 16 : 32) - sizeof(RBTreeNode *) * 3 - 1];
    };
#ifdef _MSC_VER
#pragma pack(pop)
#elif defined(__BCPLUSPLUS__)
#pragma option pop
#endif
    typedef enum { BLACK, RED } RBTreeNodeColor;

    RBTreeNode sentinel_;
    RBTreeNode rootNode_;
    RBTreeNode * root_;

    intptr_t compare(RBTreeNode * a0,RBTreeNode * a1){
      intptr_t c, s1 = a0 - nodes_, s2 = a1 - nodes_;
//      uintptr_t l = mlen_ + 2;
      uintptr_t l = alen_;
      do {
        c = intptr_t(dict_[s1]) - dict_[s2];
        if( c != 0 ) break;
        s1 = (s1 + 1) & dmsk_;
        s2 = (s2 + 1) & dmsk_;
      } while( --l > 0 );
      l = s2 - l;
      if( l > bestMatchLen_ ){
        bestMatchLen_ = l;
        bestMatchNode_ = a1;
      }
      return c;
    }

    RBTreeNode * find(RBTreeNode * node)
    {
      RBTreeNode * current = root_->left_;
      for(;;){
        if( current == &sentinel_ ){ current = NULL; break; }
        intptr_t c = compare(node,current);
        if( c == 0 ) break;
        current = c < 0 ? current->left_ : current->right_;
      }
      return current;
    }

    void rotateLeft(RBTreeNode * x)
    {
      RBTreeNode * y = x->right_;
      x->right_ = y->left_;
      if( y->left_ != &sentinel_ ) y->left_->parent_ = x;
      y->parent_ = x->parent_;
      if( x == x->parent_->left_ ){
        x->parent_->left_ = y;
      }
      else {
        x->parent_->right_ = y;
      }
      y->left_ = x;
      x->parent_ = y;
    }

    void rotateRight(RBTreeNode * y)
    {
      RBTreeNode * x = y->left_;
      y->left_ = x->right_;
      if( x->right_ != &sentinel_ ) x->right_->parent_ = y;
      x->parent_ = y->parent_;
      if( y == y->parent_->left_ ){
        y->parent_->left_ = x;
      }
      else {
        y->parent_->right_ = x;
      }
      x->right_ = y;
      y->parent_ = x;
    }

    RBTreeNode * treeInsertHelp(RBTreeNode * z)
    {
      RBTreeNode * y = root_, * x = root_->left_;
      z->left_ = z->right_ = &sentinel_;
      intptr_t c = 0;
      while( x != &sentinel_ ){
        y = x;
        c = compare(z,x);
        if( c < 0 ){
          x = x->left_;
        }
        else if( c > 0 ){
          x = x->right_;
        }
        else
          return x;
      }
      z->parent_ = y;
      if( y == root_ || c < 0 ){ 
        y->left_ = z;
      }
      else {
        y->right_ = z;
      }
      return NULL;
    }

    void insert(RBTreeNode * x)
    {
      RBTreeNode * y;
      if( (y = treeInsertHelp(x)) != NULL ) return;
      x->color_ = RED;
      while( x->parent_->color_ == RED ){
        if( x->parent_ == x->parent_->parent_->left_ ){
          y = x->parent_->parent_->right_;
          if( y->color_ == RED ){
    	      x->parent_->color_ = BLACK;
	          y->color_ = BLACK;
	          x->parent_->parent_->color_ = RED;
	          x = x->parent_->parent_;
          }
          else {
	          if( x == x->parent_->right_){
	            x = x->parent_;
	            rotateLeft(x);
	          }
	          x->parent_->color_ = BLACK;
	          x->parent_->parent_->color_ = RED;
	          rotateRight(x->parent_->parent_);
          } 
        }
        else {
          y = x->parent_->parent_->left_;
          if( y->color_ == RED ){
	          x->parent_->color_ = BLACK;
	          y->color_ = BLACK;
	          x->parent_->parent_->color_ = RED;
	          x = x->parent_->parent_;
          }
          else {
	          if( x == x->parent_->left_ ){
	            x = x->parent_;
	            rotateRight(x);
	          }
	          x->parent_->color_ = BLACK;
	          x->parent_->parent_->color_ = RED;
	          rotateLeft(x->parent_->parent_);
          } 
        }
      }
      root_->left_->color_ = BLACK;
    }

    RBTreeNode * getSuccessorOf(RBTreeNode * x)
    {
      RBTreeNode * y;
/* assignment to y is intentional */  
      if( (y = x->right_) != &sentinel_ ){
/* returns the minium of the right subtree of x */  
        while( y->left_ != &sentinel_ ) y = y->left_;
        return y;
      }
      y = x->parent_;
/* sentinel used instead of checking for nil */    
      while( x == y->right_ ){
        x = y;
        y = y->parent_;
      }
      if( y == root_ ) return &sentinel_;
      return y;
    }

    void removeFixup(RBTreeNode * x)
    {
      RBTreeNode * w, * rootLeft = root_->left_;

      while( x->color_ != RED && rootLeft != x ){
        if( x == x->parent_->left_ ){
          w = x->parent_->right_;
          if( w->color_ == RED ){
	          w->color_ = BLACK;
	          x->parent_->color_ = RED;
	          rotateLeft(x->parent_);
	          w = x->parent_->right_;
          }
          if( w->right_->color_ != RED && w->left_->color_ != RED ){ 
	          w->color_ = RED;
	          x = x->parent_;
          }
          else {
	          if( w->right_->color_ != RED ){
	            w->left_->color_ = BLACK;
	            w->color_ = RED;
	            rotateRight(w);
	            w = x->parent_->right_;
	          }
	          w->color_ = x->parent_->color_;
	          x->parent_->color_ = BLACK;
	          w->right_->color_ = BLACK;
	          rotateLeft(x->parent_);
	          x = rootLeft; /* this is to exit while loop */
          }
        }
        else { /* the code below is has left and right switched from above */
          w = x->parent_->left_;
          if( w->color_ == RED ){
	          w->color_ = BLACK;
	          x->parent_->color_ = RED;
	          rotateRight(x->parent_);
	          w = x->parent_->left_;
          }
          if( w->right_->color_ != RED && w->left_->color_ != RED ){ 
	          w->color_ = RED;
	          x = x->parent_;
          }
          else {
	          if( w->left_->color_ != RED ){
	            w->right_->color_ = BLACK;
	            w->color_ = RED;
	            rotateLeft(w);
	            w = x->parent_->left_;
	          }
	          w->color_ = x->parent_->color_;
	          x->parent_->color_ = BLACK;
	          w->left_->color_ = BLACK;
	          rotateRight(x->parent_);
	          x = rootLeft; /* this is to exit while loop */
          }
        }
      }
      x->color_ = BLACK;
    }

    void remove(RBTreeNode * z)
    {
      RBTreeNode * x, * y;
      y = ((z->left_ == &sentinel_) || (z->right_ == &sentinel_)) ? z : getSuccessorOf(z);
      x = (y->left_ == &sentinel_) ? y->right_ : y->left_;
/* x is y's only child */
      if( y->left_ != &sentinel_ ) x = y->left_; else x = y->right_;
/* assignment of y->p to x->p is intentional */  
      if( root_ == (x->parent_ = y->parent_) ){
        root_->left_ = x;
      }
      else {
        if( y == y->parent_->left_ ){
          y->parent_->left_ = x;
        }
        else {
          y->parent_->right_ = x;
        }
      }
      if( y != z ){
        y->left_ = z->left_;
        y->right_ = z->right_;
        y->parent_ = z->parent_;
        z->left_->parent_ = z->right_->parent_ = y;
        if( z == z->parent_->left_ ){
          z->parent_->left_ = y;
        }
        else {
          z->parent_->right_ = y;
        }
        if( y->color_ != RED ){
          y->color_ = z->color_;
          removeFixup(x);
        }
        else {
          y->color_ = z->color_;
        }
      }
      else if( y->color_ != RED ){
        removeFixup(x);
      }
      z->parent_ = z->right_ = z->left_ = NULL;
      return;
    }


    // encoder stateful variables
    RBTreeNode * nodes_;
    uint8_t * dict_;
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
    RBTreeNode * bestMatchNode_;
    uintptr_t bestMatchLen_;

    // decoder stateful variables
    uint8_t * ddict_;
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
inline LZSSRBTFilter::~LZSSRBTFilter()
{
  kfree(ddict_);
  kfree(nodes_);
}
//------------------------------------------------------------------------------
inline LZSSRBTFilter::LZSSRBTFilter() : nodes_(NULL), dict_(NULL), ddict_(NULL)
{
}
//------------------------------------------------------------------------------
inline LZSSRBTFilter * LZSSRBTFilter::initializeEncoder()
{
  dpos_ = 0;
  alen_ = 0;
  mlen_ = 16;
  dlen_ = 0;
  eState_ = stInit;
  dcnt_ = 65536;
  dmsk_ = dcnt_ - 1;
  dcsz_ = getFirstBitIndex(dcnt_);
  dlsz_ = getFirstBitIndex(mlen_);
  dbits_ = 0;
  dbiti_ = 0;
  nodes_ = (RBTreeNode *) krealloc(nodes_,dcnt_ * sizeof(nodes_[0]) + dcnt_);
  dict_ = (uint8_t *) (nodes_ + dcnt_);
  for( intptr_t i = dmsk_; i >= 0; i-- ){
    nodes_[i].parent_ = NULL;
    //dict_[i].idx_ = uint32_t(i);
    dict_[i] = 0;
  }

  sentinel_.left_ = &sentinel_;
  sentinel_.right_ = &sentinel_;
  sentinel_.parent_ = NULL;
  sentinel_.color_ = BLACK;

  root_ = &rootNode_;
  rootNode_.left_ = &sentinel_;
  rootNode_.right_ = &sentinel_;
  rootNode_.parent_ = NULL;
  rootNode_.color_ = BLACK;

  return this;
}
//------------------------------------------------------------------------------
inline LZSSRBTFilter * LZSSRBTFilter::encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb,uintptr_t * wb)
{
  RBTreeNode * p;
  if( eState_ == stInit ){
    while( alen_ < mlen_ + 2 ){
      if( inpSize == 0 ) return this;
      dict_[(dpos_ + alen_) & dmsk_] = *(const uint8_t *) inp;
      alen_ += 1;
      inp = (const uint8_t *) inp + 1;
      inpSize -= 1;
      if( rb != NULL ) *rb += 1;
    }
    goto init;
  }
  if( eState_ == stInp ) goto inp;
  if( eState_ == stOut ) goto out;
  for(;;){
    do {
inp:  p = nodes_ + ((dpos_ + mlen_ + 2) & dmsk_);
      if( p->parent_ != NULL ) remove(p);
      if( inpSize == 0 ){
        eState_ = stInp;
        return this;
      }
      dict_[(dpos_ + mlen_ + 2) & dmsk_] = *(const uint8_t *) inp;
      inp = (const uint8_t *) inp + 1;
      inpSize -= 1;
      if( rb != NULL ) *rb += 1;
      dpos_ = (dpos_ + 1) & dmsk_;
init: bestMatchNode_ = NULL;
      bestMatchLen_ = 1;
      insert(nodes_ + dpos_);
    } while( --dlen_ > 0 );
    dlen_ = bestMatchLen_;
    if( dlen_ <= 2 ){
      dlen_ = 1;
      dcode_ = (uintptr_t(dict_[dpos_]) << 1) | 1u;
      dcount_ = 8 + 1;
    }
    else {
      dcode_ = (((bestMatchNode_ - nodes_) << dlsz_) | (dlen_ - 3)) << 1;
      dcount_ = dcsz_ + dlsz_ + 1;
    }
out:
    if( writeBits(dcode_,dcount_,out,outSize,wb) ){
      eState_ = stOut;
      return this;
    }
  }
  return this;
}
//------------------------------------------------------------------------------
inline LZSSRBTFilter * LZSSRBTFilter::flush(void * out,uintptr_t * wb)
{
  if( eState_ == stInit ) return this;
  uintptr_t outSize = ~uintptr_t(0);
  //struct {
  //  uintptr_t dpos_;
  //  uintptr_t alen_;
  //  uintptr_t dlen_;
  //} safe;
  for(;;){
    dpos_ = (dpos_ + dlen_) & dmsk_;
    if( (alen_ -= dlen_) == 0 ) goto exit;
    bestMatchNode_ = NULL;
    bestMatchLen_ = 1;
    find(nodes_ + dpos_);
    dlen_ = bestMatchLen_;
    if( dlen_ <= 2 ){
      dlen_ = 1;
      dcode_ = (uintptr_t(dict_[dpos_]) << 1) | 1u;
      dcount_ = 8 + 1;
    }
    else {
      dcode_ = (((bestMatchNode_ - nodes_) << dlsz_) | (dlen_ - 3)) << 1;
      dcount_ = dcsz_ + dlsz_ + 1;
    }
    writeBits(dcode_,dcount_,out,outSize,wb);
  }
exit:
  uintptr_t sz = (dbiti_ >> 3) + ((dbiti_ & 7) != 0);
  memcpy(out,&dbits_,sz);
  out = (uint8_t *) out + sz;
  if( wb != NULL ) *wb += sz;
  return this;
}
//------------------------------------------------------------------------------
//inline LZSSRBTFilter * LZSSRBTFilter::encode(AsyncFile & inp,AsyncFile & out)
//{
//  uintptr_t rbs = getpagesize() * 16;
//  uintptr_t wbs = getpagesize() * 16;
//  AutoPtrBuffer inpBuffer((uint8_t *) kmalloc(rbs));
//  AutoPtrBuffer outBuffer((uint8_t *) kmalloc(wbs));
//  uintptr_t rb = 0, wb = 0;
//  int64_t r;
//  while( (r = inp.read(inpBuffer,rbs)) > 0 ){
//    while( rb < uintptr_t(r) ){
//      encodeBuffer(&inpBuffer[rb],uintptr_t(r) - rb,&outBuffer[wb],wbs - wb,&rb,&wb);
//      if( wb == wbs ){
//        out.writeBuffer(outBuffer,wbs);
//        wb = 0;
//      }
//    }
//    rb = 0;
//  }
//  out.writeBuffer(outBuffer,wb);
//  wb = 0;
//  flush(outBuffer,&wb);
//  out.writeBuffer(outBuffer,wb);
//  return this;
//}
//------------------------------------------------------------------------------
inline LZSSRBTFilter * LZSSRBTFilter::initializeDecoder()
{
  ddpos_ = 0;
  ddcnt_ = 65536;
  ddmsk_ = dcnt_ - 1;
  dmlen_ = 16;
  ddcsz_ = getFirstBitIndex(ddcnt_);
  ddlsz_ = getFirstBitIndex(dmlen_);
  ddbits_ = 0;
  ddbiti_ = 0;
  ddbitc_ = 0;
  dState_ = stInit;
  ddict_ = (uint8_t *) krealloc(ddict_,dcnt_);
  memset(ddict_,0,ddcnt_);
  ddcode_ = 0;
  ddcount_ = 1;
  ddci_ = 0;
  return this;
}
//------------------------------------------------------------------------------
inline LZSSRBTFilter * LZSSRBTFilter::decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb,uintptr_t * wb)
{
  uint8_t * dict = ddict_;
  if( dState_ == stOut ) goto out;
  if( dState_ == stInp2 ) goto inp2;
  if( dState_ == stInp3 ) goto inp3;
  for(;;){
    if( readBits(ddcode_,ddci_,ddcount_,inp,inpSize,rb) ){
      dState_ = stInp;
      return this;
    }
    ddci_ = 0;
    if( ddcode_ == 0 ){
      ddcount_ = ddcsz_ + ddlsz_;
inp2:
      if( readBits(ddcode_,ddci_,ddcount_,inp,inpSize,rb) ){
        dState_ = stInp2;
        return this;
      }
      ddlen_ = (ddcode_ & ~(~uintptr_t(0) << ddlsz_)) + 3;
      dcpos_ = ddcode_ >> ddlsz_;
    }
    else {
      ddcode_ = 0;
      ddcount_ = 8;
inp3:
      if( readBits(ddcode_,ddci_,ddcount_,inp,inpSize,rb) ){
        dState_ = stInp3;
        return this;
      }
      dict[dcpos_ = ddpos_] = uint8_t(ddcode_);
      ddlen_ = 1;
    }
    do {
out:  if( outSize == 0 ){
        dState_ = stOut;
        return this;
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
  return this;
}
//------------------------------------------------------------------------------
//inline LZSSRBTFilter * LZSSRBTFilter::decode(AsyncFile & inp,AsyncFile & out)
//{
//  uintptr_t rbs = getpagesize() * 16;
//  uintptr_t wbs = getpagesize() * 16;
//  AutoPtrBuffer inpBuffer((uint8_t *) kmalloc(rbs));
//  AutoPtrBuffer outBuffer((uint8_t *) kmalloc(wbs));
//  uintptr_t rb = 0, wb = 0;
//  int64_t r;
//  while( (r = inp.read(inpBuffer,rbs)) > 0 ){
//    while( rb < uintptr_t(r) ){
//      decodeBuffer(&inpBuffer[rb],uintptr_t(r) - rb,&outBuffer[wb],wbs - wb,&rb,&wb);
//      if( wb == wbs ){
//        out.writeBuffer(outBuffer,wbs);
//        wb = 0;
//      }
//    }
//    rb = 0;
//  }
//  out.writeBuffer(outBuffer,wb);
//  return this;
//}
//------------------------------------------------------------------------------
#ifndef STREAM_FILTER_ABSTRATION
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class LZMAStreamFilter : public StreamCompressionFilter, private BaseServer
{
  public:
    virtual ~LZMAStreamFilter();
    LZMAStreamFilter();

    StreamCompressionFilter & initializeCompression();
    StreamCompressionFilter & compress(const void * buf = NULL,uintptr_t count = 0);
    StreamCompressionFilter & finishCompression();
    StreamCompressionFilter & initializeDecompression();
    StreamCompressionFilter & decompress(void * buf = NULL,uintptr_t count = 0);
    StreamCompressionFilter & finishDecompression();
  protected:
    virtual intptr_t encoderRead(void * buf,uintptr_t size) = 0;
    virtual void afterEncoderRead(void * buf,uintptr_t size) {}
    virtual intptr_t encoderWrite(const void * buf,uintptr_t size) = 0;
    virtual void beforeEncoderWrite(void * buf,uintptr_t size) {}
    class Encoder :
      public Fiber,
      public NCompress::NLZMA::CEncoder,
      public ISequentialInStream,
      public ISequentialOutStream
    {
      public:
        Encoder() { destroy_ = false; }

        STDMETHOD(QueryInterface)(REFIID,void **) { return E_FAIL; }
        ULONG STDMETHODCALLTYPE AddRef(void) { return 0; }
        ULONG STDMETHODCALLTYPE Release(void) { return 0; }

        STDMETHOD(Read)(void * data,UInt32 size,UInt32 * processedSize);
        STDMETHOD(Write)(const void *data,UInt32 size,UInt32 * processedSize);

        Semaphore sem_;
        LZMAStreamFilter * filter_;
        Fiber * guest_;
        int32_t err_;
        bool flush_;

        void fiberExecute();
    };
    friend class Encoder;
    AutoPtr<Encoder> encoder_;

    virtual intptr_t decoderRead(void * buf,uintptr_t size) = 0;
    virtual void afterDecoderRead(void * buf,uintptr_t size) {}
    virtual intptr_t decoderWrite(const void * buf,uintptr_t size) = 0;
    virtual void beforeDecoderWrite(void * buf,uintptr_t size) {}
    class Decoder :
      public Fiber,
      public NCompress::NLZMA::CDecoder,
      public ISequentialInStream,
      public ISequentialOutStream
    {
      public:
        Decoder() { destroy_ = false; }

        STDMETHOD(QueryInterface)(REFIID,void **) { return E_FAIL; }
        ULONG STDMETHODCALLTYPE AddRef(void) { return 0; }
        ULONG STDMETHODCALLTYPE Release(void) { return 0; }

        STDMETHOD(Read)(void * data,UInt32 size,UInt32 * processedSize);
        STDMETHOD(Write)(const void *data,UInt32 size,UInt32 * processedSize);

        Semaphore sem_;
        LZMAStreamFilter * filter_;
        Fiber * guest_;
        uint8_t coderProperties_[5];
        int32_t err_;
        bool flush_;

        void fiberExecute();
    };
    friend class Decoder;
    AutoPtr<Decoder> decoder_;

    LZMAStreamFilter & dropEncoder();
    LZMAStreamFilter & dropDecoder();
  private:
    LZMAStreamFilter(const LZMAStreamFilter &);
    void operator = (const LZMAStreamFilter &);

    Fiber * newFiber() { return NULL; }
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class LZMAFileFilter : public LZMAStreamFilter {
  public:
    StreamCompressionFilter & compressFile(const utf8::String & srcFileName,const utf8::String & dstFileName);
    StreamCompressionFilter & decompressFile(const utf8::String & srcFileName,const utf8::String & dstFileName);
  protected:
    AsyncFile srcFile_;
    AsyncFile dstFile_;

    intptr_t encoderRead(void * buf,uintptr_t size);
    intptr_t encoderWrite(const void * buf,uintptr_t size);
    intptr_t decoderRead(void * buf,uintptr_t size);
    intptr_t decoderWrite(const void * buf,uintptr_t size);
  private:
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class LZMADescriptorFilter : public LZMAStreamFilter {
  public:
    StreamCompressionFilter & compress(const void * buf,uintptr_t count);
    StreamCompressionFilter & decompress(void * buf,uintptr_t count);
  protected:
    AsyncDescriptor * descriptor_;
    const void * encodeBuffer_;
    uintptr_t encodeBytes_;
    void * decodeBuffer_;
    uintptr_t decodeBytes_;

    intptr_t encoderRead(void * buf,uintptr_t size);
    intptr_t encoderWrite(const void * buf,uintptr_t size);
    intptr_t decoderRead(void * buf,uintptr_t size);
    intptr_t decoderWrite(const void * buf,uintptr_t size);

    StreamCryptFilter * encoderCryptor_;
    StreamCryptFilter * decoderCryptor_;

    void afterEncoderRead(void * buf,uintptr_t size);
    void beforeEncoderWrite(void * buf,uintptr_t size);
    void afterDecoderRead(void * buf,uintptr_t size);
    void beforeDecoderWrite(void * buf,uintptr_t size);
  private:
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif /* stmfltH */
//---------------------------------------------------------------------------
