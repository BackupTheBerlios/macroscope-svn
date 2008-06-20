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
//---------------------------------------------------------------------------
#include <adicpp/ksys.h>
#include <adicpp/lzssk.h>
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
HuffmanFilter::~HuffmanFilter()
{
}
//------------------------------------------------------------------------------
HuffmanFilter::HuffmanFilter()
{
}
//------------------------------------------------------------------------------
HuffmanFilter & HuffmanFilter::initializeCompression()
{
  for( uintptr_t i = 0; i < 256; i++ ){
    abcFreq_[i] = 0;
    abc2Idx_[i] = uint8_t(i);
    idx2abc_[i] = uint8_t(i);
  }
  outputBitPos_ = 0;
  outputSize_ = 0;
  outputMaxSize_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
HuffmanFilter & HuffmanFilter::updateABC(uintptr_t c)
{
  uintptr_t a;
  abcFreq_[c]++;
  if( abc2Idx_[c] > 0 && abcFreq_[c] > abcFreq_[a = idx2abc_[abc2Idx_[c] - 1]] ){
    xchg(abc2Idx_[c],abc2Idx_[a]);
    xchg(idx2abc_[abc2Idx_[c]],idx2abc_[abc2Idx_[a]]);
  }
  return *this;
}
//---------------------------------------------------------------------------
HuffmanFilter & HuffmanFilter::compress(const void * buf,uintptr_t count)
{
  while( count > 0 ){
    uintptr_t c = *(uint8_t *) buf;
    uintptr_t idx = abc2Idx_[c];
    updateABC(c);
    if( outputSize_ + 2 >= outputMaxSize_ ){
      c = (outputMaxSize_ << 1) + ((outputMaxSize_ == 0) << 4);
      output_.realloc(c);
      memset(output_.ptr() + outputMaxSize_,0,c - outputMaxSize_);
      outputMaxSize_ = c;
    }
    uint16_t * out = (uint16_t *) (output_.ptr() + (outputBitPos_ >> 3));
    if( idx <= 1 ){
      *out |= uint16_t(0u | (idx << 3)) << (outputBitPos_ & 7);
      outputBitPos_ += 4;
    }
    else if( idx <= 5 ){
      *out |= uint16_t(1u | (idx << 3)) << (outputBitPos_ & 7);
      outputBitPos_ += 5;
    }
    else if( idx <= 13 ){
      *out |= uint16_t(2u | (idx << 3)) << (outputBitPos_ & 7);
      outputBitPos_ += 6;
    }
    else if( idx <= 29 ){
      *out |= uint16_t(3u | (idx << 4)) << (outputBitPos_ & 7);
      outputBitPos_ += 7;
    }
    else if( idx <= 61 ){
      *out |= uint16_t(4u | (idx << 4)) << (outputBitPos_ & 7);
      outputBitPos_ += 8;
    }
    else if( idx <= 125 ){
      *out |= uint16_t(5u | (idx << 4)) << (outputBitPos_ & 7);
      outputBitPos_ += 9;
    }
    else if( idx <= 253 ){
      *out |= uint16_t(6u | (idx << 4)) << (outputBitPos_ & 7);
      outputBitPos_ += 10;
    }
    else {
      *out |= uint16_t(7u | (idx << 4)) << (outputBitPos_ & 7);
      outputBitPos_ += 11;
    }
    outputSize_ = (outputBitPos_ >> 3) + ((outputBitPos_ & 7) != 0);
    buf = (uint8_t *) buf + 1;
    count--;
  }
  return *this;
}
//---------------------------------------------------------------------------
HuffmanFilter & HuffmanFilter::finishCompression()
{
  if( outputSize_ + 2 >= outputMaxSize_ ){
    uintptr_t c = (outputMaxSize_ << 1) + ((outputMaxSize_ == 0) << 4);
    output_.realloc(c);
    memset(output_.ptr() + outputMaxSize_,0,c - outputMaxSize_);
    outputMaxSize_ = c;
  }
  uint16_t * out = (uint16_t *) (output_.ptr() + (outputBitPos_ >> 3));
  *out |= uint16_t(7u) << (outputBitPos_ & 7);
  outputBitPos_ += 11;
  outputSize_ = (outputBitPos_ >> 3) + ((outputBitPos_ & 7) != 0);
  return *this;
}
//---------------------------------------------------------------------------
HuffmanFilter & HuffmanFilter::initializeDecompression()
{
  outputSize_ = 0;
  for( uintptr_t i = 0; i < 256; i++ ){
    abcFreq_[i] = 0;
    abc2Idx_[i] = uint8_t(i);
    idx2abc_[i] = uint8_t(i);
  }
  return *this;
}
//---------------------------------------------------------------------------
HuffmanFilter & HuffmanFilter::decompress(const void * buf,uintptr_t count)
{
  return *this;
}
//---------------------------------------------------------------------------
HuffmanFilter & HuffmanFilter::compressFile(const utf8::String & src,const utf8::String & dst)
{
  AutoPtr<uint8_t> buf;
  buf.alloc(getpagesize() * 16);
  AsyncFile srcFile(src), dstFile(dst);
  srcFile.readOnly(true).open();
  dstFile.createIfNotExist(true).open().resize(0);
  initializeCompression();
  int64_t r;
  for(;;){
    r = srcFile.read(buf,getpagesize() * 16);
    if( r < 0 ) break;
    compress(buf,uintptr_t(r));
    if( r < getpagesize() * 16 ){
      finishCompression();
      break;
    }
  }
  dstFile.writeBuffer(output_,outputSize_);
  outputSize_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
HuffmanFilter & HuffmanFilter::decompressFile(const utf8::String & src,const utf8::String & dst)
{
  return *this;
}
//---------------------------------------------------------------------------
void HuffmanFilter::genStatisticTable(const Array<utf8::String> & fileNames)
{
  class TEntry {
    public:
      uintmax_t freq_;
      uint8_t i_;

      bool operator == (const TEntry & e) const { return freq_ == e.freq_; }
      bool operator < (const TEntry & e) const { return freq_ > e.freq_; }
      bool operator > (const TEntry & e) const { return freq_ < e.freq_; }
  };
  Array<TEntry> table;
  table.resize(256);
  uintptr_t i, j;
  for( i = 0; i < 256; i++ ){
    table[i].freq_ = 0;
    table[i].i_ = (uint8_t) i;
  }
  AutoPtr<uint8_t> b;
  b.alloc(getpagesize() * 16);
  AsyncFile f;
  for( i = 0; i < fileNames.count(); i++ ){
    f.fileName(fileNames[i]).readOnly(true).open();
    for(;;){
      int64_t r = f.read(b,getpagesize() * 16);
      if( r <= 0 ) break;
      while( --r >= 0 ) table[(uintptr_t) b[(uintptr_t) r]].freq_++;
    }
    f.close();
  }
  f.fileName("stdout").open();
  qSort(table.ptr(),0,table.count() - 1);
  f <<
    "const uint8_t table[256] = {\n"
  ;
  for( i = 0; i < 256; i += 8 ){
    for( j = 0; j < 8; j++ )
      f <<
        utf8::String(j == 0 ? "  " : "") +
        "0x" + utf8::int2HexStr(table[i + j].i_,2) +
        (i + j < 255 ? ", " : "") + (j < 7 ? "" : "\n");
  }
  f <<
    "};\n"
  ;
}
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
LZSSKFilter::~LZSSKFilter()
{
}
//------------------------------------------------------------------------------
LZSSKFilter::LZSSKFilter() : dict_(NULL), dictSize_(0)
{
}
//------------------------------------------------------------------------------
LZSSKFilter & LZSSKFilter::initializeCompression(uintptr_t dictSize)
{
  maxStrLen_ = 1u << 4;
  for( dictSize_ = maxStrLen_ << 1; dictSize_ < dictSize && dictSize_ < (1u << 18); dictSize_ <<= 1 );
  dictSizeMask_ = dictSize_ - 1;
  buffer_.realloc(sizeof(DictChar) * dictSize_);
  dict_ = (DictChar *) buffer_.ptr();
  for( uintptr_t i = 0; i < dictSize_; i++ )
    new (dict_ + i) DictChar;
  dictPos_ = 0;
  lastStrLen_ = 0;
  lastCodedLen_ = 0;
  aheadWindowSize_ = 0;
  dictTree_.clear().param(this);
  HuffmanFilter::initializeCompression();
  initialized_ = false;
  finalize_ = false;
  return *this;
}
//---------------------------------------------------------------------------
/*LZSSKFilter & LZSSKFilter::compress(const void * buf,uintptr_t count)
{
  if( !initialized_ ){
    while( count > 0 && aheadWindowSize_ < maxStrLen_ ){
      dict_[(dictPos_ + aheadWindowSize_) & dictSizeMask_].c_ = *(uint8_t *) buf;
      aheadWindowSize_++;
      buf = (uint8_t *) buf + 1;
      count--;
    }
    initialized_ = true;
  }
  if( lastCodedLen_ > 0 ) goto l1;
  while( count > 0 || (aheadWindowSize_ > 0 && finalize_) ){
    DictChar * pDictStr;
    lastStrLen_ = 1;
    dictTree_.insert(dict_[dictPos_],false,false,&pDictStr);
    if( dict_ + dictPos_ != pDictStr && lastStrLen_ >= 3 ){ // string match, code string
      if( dictPos_ < (1u << 10) ){
        output_.realloc(outputSize_ + 2);
        *(uint16_t *)(output_.ptr() + outputSize_) =
          uint16_t(ct2Seq | (lastStrIndex_ << 2) | ((lastStrLen_ - 3) << (10 + 2)));
        outputSize_ += 2;
      }
      else {
        output_.realloc(outputSize_ + 4);
        *(uint32_t *)(output_.ptr() + outputSize_) =
          uint32_t(ct3Seq | (lastStrIndex_ << 2) | ((lastStrLen_ - 3) << (18 + 2)));
        outputSize_ += 3;
      }
    }
    else { // not match, code single char
      if( abc2Idx_[dict_[dictPos_].c_] < (1u << 6) ){
        output_.realloc(outputSize_ + 1);
        output_[outputSize_++] = uint8_t(ct6Char | (abc2Idx_[dict_[dictPos_].c_] << 2));
      }
      else {
        output_.realloc(outputSize_ + 2);
        *(uint16_t *)(output_.ptr() + outputSize_) = uint16_t(ct8Char | (abc2Idx_[dict_[dictPos_].c_] << 2));
        outputSize_ += 2;
      }
      lastStrLen_ = 1;
    }
    lastCodedLen_ = lastStrLen_;
    for(;;){
l1:   updateABC(dict_[dictPos_].c_);
      dictPos_ = (dictPos_ + 1) & dictSizeMask_;
      aheadWindowSize_--;
      if( count > 0 && aheadWindowSize_ < maxStrLen_ ){
        uintptr_t pos = (dictPos_ + aheadWindowSize_) & dictSizeMask_;
        if( dict_[pos].treeNode_.parent_ != NULL ){
          dictTree_.remove(dict_[pos]);
          dict_[pos].treeNode_.parent_ = NULL;
        }
        dict_[pos].c_ = *(uint8_t *) buf;
        aheadWindowSize_++;
        buf = (uint8_t *) buf + 1;
        count--;
      }
      if( --lastCodedLen_ == 0 ) break;
      if( count == 0 && !finalize_ ) break;
      dictTree_.insert(dict_[dictPos_],false,false);
    }
  }
  return *this;
}*/
//---------------------------------------------------------------------------
LZSSKFilter & LZSSKFilter::compress(const void * buf,uintptr_t count)
{
  if( !initialized_ ){
    while( count > 0 && aheadWindowSize_ < maxStrLen_ ){
      dict_[(dictPos_ + aheadWindowSize_) & dictSizeMask_].c_ = *(uint8_t *) buf;
      aheadWindowSize_++;
      buf = (uint8_t *) buf + 1;
      count--;
    }
    initialized_ = true;
  }
  if( lastCodedLen_ > 0 ) goto l1;
  while( count > 0 || (aheadWindowSize_ > 0 && finalize_) ){
    DictChar * pDictStr;
    uintptr_t v;
    lastStrLen_ = 1;
    dictTree_.insert(dict_[dictPos_],false,false,&pDictStr);
    if( dict_ + dictPos_ != pDictStr && lastStrLen_ >= 3 ){ // string match, code string
      if( dictPos_ < (1u << 10) ){
        v = ct2Seq | (lastStrIndex_ << 2) | ((lastStrLen_ - 3) << (10 + 2));
        HuffmanFilter::compress(&v,2);
      }
      else {
        v = ct3Seq | (lastStrIndex_ << 2) | ((lastStrLen_ - 3) << (18 + 2));
        HuffmanFilter::compress(&v,3);
      }
    }
    else { // not match, code single char
      if( dict_[dictPos_].c_ < (1u << 6) ){
        v = ct6Char | (dict_[dictPos_].c_ << 2);
        HuffmanFilter::compress(&v,1);
      }
      else {
        v = ct8Char | (dict_[dictPos_].c_ << 2);
        HuffmanFilter::compress(&v,2);
      }
      lastStrLen_ = 1;
    }
    lastCodedLen_ = lastStrLen_;
    for(;;){
l1:
      dictPos_ = (dictPos_ + 1) & dictSizeMask_;
      aheadWindowSize_--;
      if( count > 0 && aheadWindowSize_ < maxStrLen_ ){
        uintptr_t pos = (dictPos_ + aheadWindowSize_) & dictSizeMask_;
        if( dict_[pos].treeNode_.parent_ != NULL ){
          dictTree_.remove(dict_[pos]);
          dict_[pos].treeNode_.parent_ = NULL;
        }
        dict_[pos].c_ = *(uint8_t *) buf;
        aheadWindowSize_++;
        buf = (uint8_t *) buf + 1;
        count--;
      }
      if( --lastCodedLen_ == 0 ) break;
      if( count == 0 && !finalize_ ) break;
      dictTree_.insert(dict_[dictPos_],false,false);
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
LZSSKFilter & LZSSKFilter::finishCompression()
{
  finalize_ = true;
  compress(NULL,0);
//  output_.realloc(outputSize_ + 2);
//  *(uint16_t *)(output_.ptr() + outputSize_) = 0xFFFF;
//  outputSize_ += 2;
  uintptr_t v = 0xFFFF;
  HuffmanFilter::compress(&v,2);
  return *this;
}
//---------------------------------------------------------------------------
LZSSKFilter & LZSSKFilter::initializeDecompression()
{
  dictPos_ = 0;
  outputSize_ = 0;
  for( uintptr_t i = 0; i < 256; i++ ){
    abcFreq_[i] = 0;
    abc2Idx_[i] = uint8_t(i);
    idx2abc_[i] = uint8_t(i);
  }
  return *this;
}
//---------------------------------------------------------------------------
LZSSKFilter & LZSSKFilter::decompress(const void * buf,uintptr_t count)
{
  while( count > 0 ){
    uintptr_t ct = *(uint8_t *) buf, index, len;
    switch( CodeType(ct & 3) ){
      case ct6Char :
        output_.realloc(outputSize_ + 1);
        updateABC(
          output_[outputSize_++] = dict_[dictPos_].c_ = idx2abc_[ct >> 2]
        );
        dictPos_ = (dictPos_ + 1) & dictSizeMask_;
        buf = (uint8_t *) buf + 1;
        count--;
        break;
      case ct2Seq  :
        index = *(uint16_t *) buf >> 2;
        len = (index >> 10) + 3;
        index &= (1u << 10) - 1;
        output_.realloc(outputSize_ + len);
        while( len > 0 ){
          updateABC(
            output_[outputSize_++] = dict_[dictPos_].c_ = dict_[index].c_
          );
          index = (index + 1) & dictSizeMask_;
          dictPos_ = (dictPos_ + 1) & dictSizeMask_;
          len--;
        }
        buf = (uint8_t *) buf + 2;
        count -= 2;
        break;
      case ct3Seq  :
        index = *(uint32_t *) buf >> 2;
        len = ((index >> 18) & ((1u << 4) - 1)) + 3;
        index &= (1u << 18) - 1;
        output_.realloc(outputSize_ + len);
        while( len > 0 ){
          updateABC(
            output_[outputSize_++] = dict_[dictPos_].c_ = dict_[index].c_
          );
          index = (index + 1) & dictSizeMask_;
          dictPos_ = (dictPos_ + 1) & dictSizeMask_;
          len--;
        }
        buf = (uint8_t *) buf + 3;
        count -= 2;
        break;
      case ct8Char :
        output_.realloc(outputSize_ + 1);
        ct = *(uint16_t *) buf;
        if( ct == 0xFFFF ){
          eos_ = true;
        }
        else {
          updateABC(
            output_[outputSize_++] = dict_[dictPos_].c_ = idx2abc_[ct >> 2]
          );
          dictPos_ = (dictPos_ + 1) & dictSizeMask_;
        }
        buf = (uint8_t *) buf + 2;
        count -= 2;
        break;
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
LZSSKFilter & LZSSKFilter::compressFile(const utf8::String & src,const utf8::String & dst)
{
  AutoPtr<uint8_t> buf;
  buf.alloc(getpagesize() * 16);
  AsyncFile srcFile(src), dstFile(dst);
  srcFile.readOnly(true).open();
  dstFile.createIfNotExist(true).open().resize(0);
  initializeCompression(256 * 1024);
  int64_t r;
  for(;;){
    r = srcFile.read(buf,getpagesize() * 16);
    if( r < 0 ) break;
    compress(buf,uintptr_t(r));
    if( r < getpagesize() * 16 ){
      finishCompression();
      break;
    }
  }
  dstFile.writeBuffer(output_,outputSize_);
  outputSize_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
LZSSKFilter & LZSSKFilter::decompressFile(const utf8::String & src,const utf8::String & dst)
{
  return *this;
}
//---------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
