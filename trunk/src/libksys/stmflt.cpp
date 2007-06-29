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
#include <adicpp/stmflt.h>
#define _NO_EXCEPTIONS 1
#include <adicpp/lzma/Common/Alloc.cpp>
#include <adicpp/lzma/Common/CRC.cpp>
#include <adicpp/lzma/7zip/Common/InBuffer.cpp>
#include <adicpp/lzma/7zip/Common/OutBuffer.cpp>
#include <adicpp/lzma/7zip/Common/StreamUtils.cpp>
#include <adicpp/lzma/7zip/Compress/RangeCoder/RangeCoderBit.cpp>
#include <adicpp/lzma/7zip/Compress/LZ/LZInWindow.cpp>
#include <adicpp/lzma/7zip/Compress/LZ/LZOutWindow.cpp>
#include <adicpp/lzma/7zip/Compress/LZMA/LZMAEncoder.cpp>
#include <adicpp/lzma/7zip/Compress/LZMA/LZMADecoder.cpp>
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
extern "C" const char IID_ICompressWriteCoderProperties[] = "";
extern "C" const char IID_ICompressSetCoderProperties[] = "";
extern "C" const char IID_ICompressSetOutStream[] = "";
extern "C" const char IID_ICompressGetInStreamProcessedSize[] = "";
extern "C" const char IID_ICompressSetDecoderProperties2[] = "";
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void LZMAFilter::Encoder::fiberExecute()
{
  while( !terminated_ ){
    HRESULT hr = WriteCoderProperties(this);
    if( SUCCEEDED(hr) ){
      hr = Code(this,this,NULL,NULL,NULL);
      assert( flush_ );
    }
    err_ = HRESULT_CODE(hr);
    if( guest_ == NULL ){
      sem_.post();
    }
    else {
      guest_->event_.type_ = etDispatch;
      guest_->thread()->postEvent(&guest_->event_);
    }
    switchFiber(mainFiber());
  }
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE LZMAFilter::Encoder::Read(void * data,UInt32 size,UInt32 * processedSize)
{
  if( processedSize != NULL ) *processedSize = 0;
  for(;;){
    UInt32 r = size > count_ ? UInt32(count_) : size;
    if( r > 0 ){
      memcpy(data,data_,r);
      data = (uint8_t *) data + r;
      data_ = (const uint8_t *) data_ + r;
      count_ -= r;
      size -= r;
      if( processedSize != NULL ) *processedSize += r;
    }
    if( size == 0 || flush_ ) break;
    if( count_ == 0 ){
      if( guest_ == NULL ){
        sem_.post();
      }
      else {
        guest_->event_.type_ = etDispatch;
        guest_->thread()->postEvent(&guest_->event_);
      }
      switchFiber(mainFiber());
    }
  }
  return S_OK;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE LZMAFilter::Encoder::Write(const void *data,UInt32 size,UInt32 * processedSize)
{
  HRESULT hr = S_OK;
  try {
    filter_->writeCompressedBuffer(data,size);
    if( processedSize != NULL ) *processedSize = size;
  }
  catch( ExceptionSP & e ){
    hr = HRESULT_FROM_WIN32(e->code());
  }
  return hr;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void LZMAFilter::Decoder::fiberExecute()
{
  while( !terminated_ ){
    uint8_t coderProperties[5];
    HRESULT hr = Read(coderProperties,sizeof(coderProperties),NULL);
    if( SUCCEEDED(hr) ){
      hr = SetDecoderProperties2(coderProperties,sizeof(coderProperties));
      if( SUCCEEDED(hr) ){
        hr = Code(this,this,NULL,NULL,NULL);
        assert( flush_ );
      }
    }
    err_ = HRESULT_CODE(hr);
    if( guest_ == NULL ){
      sem_.post();
    }
    else {
      guest_->event_.type_ = etDispatch;
      guest_->thread()->postEvent(&guest_->event_);
    }
    switchFiber(mainFiber());
  }
}
//------------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE LZMAFilter::Decoder::Read(void * data,UInt32 size,UInt32 * processedSize)
{
  if( processedSize != NULL ) *processedSize = 0;
  for(;;){
    UInt32 r = size > count_ ? UInt32(count_) : size;
    if( r > 0 ){
      memcpy(data,data_,r);
      data = (uint8_t *) data + r;
      data_ = (const uint8_t *) data_ + r;
      count_ -= r;
      size -= r;
      if( processedSize != NULL ) *processedSize += r;
    }
    if( size == 0 || flush_ ) break;
    if( count_ == 0 ){
      if( guest_ == NULL ){
        sem_.post();
      }
      else {
        guest_->event_.type_ = etDispatch;
        guest_->thread()->postEvent(&guest_->event_);
      }
      switchFiber(mainFiber());
    }
  }
  return S_OK;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE LZMAFilter::Decoder::Write(const void *data,UInt32 size,UInt32 * processedSize)
{
  HRESULT hr = S_OK;
  try {
    filter_->writeCompressedBuffer(data,size);
    if( processedSize != NULL ) *processedSize = size;
  }
  catch( ExceptionSP & e ){
    hr = HRESULT_FROM_WIN32(e->code());
  }
  return hr;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
LZMAFilter::~LZMAFilter()
{
  dropEncoder();
  dropDecoder();
}
//---------------------------------------------------------------------------
LZMAFilter::LZMAFilter()
{
  howCloseServer(HowCloseServer() & ~csWait);  
}
//---------------------------------------------------------------------------
LZMAFilter & LZMAFilter::dropEncoder()
{
  if( encoder_ != NULL ){
    encoder_->terminate();
    encoder_->event_.type_ = etDispatch;
    encoder_->thread()->postEvent(&encoder_->event_);
    while( !encoder_->finished() ) ksleep1();
    encoder_ = NULL;
  }
  return *this;
}
//---------------------------------------------------------------------------
LZMAFilter & LZMAFilter::dropDecoder()
{
  if( decoder_ != NULL ){
    decoder_->terminate();
    decoder_->event_.type_ = etDispatch;
    decoder_->thread()->postEvent(&decoder_->event_);
    while( !decoder_->finished() ) ksleep1();
    decoder_ = NULL;
  }
  return *this;
}
//---------------------------------------------------------------------------
StreamCompressionFilter & LZMAFilter::initializeCompression()
{
  dropEncoder();
  encoder_ = newObject<Encoder>();
  encoder_->filter_ = this;
  static const PROPID propIDs[] = {
    NCoderPropID::kNumFastBytes,
    NCoderPropID::kMatchFinderCycles,
    NCoderPropID::kAlgorithm,
    NCoderPropID::kMatchFinder,
    NCoderPropID::kDictionarySize,
    NCoderPropID::kPosStateBits,
    NCoderPropID::kLitPosBits,
    NCoderPropID::kLitContextBits,
    NCoderPropID::kEndMarker
  };
  PROPVARIANT properties[sizeof(propIDs) / sizeof(propIDs[0])] = {
    { VT_UI4, 0, 0, 0 /* 5 - 273 */ },
    { VT_UI4, 0, 0, 0 },
    { VT_UI4, 0, 0, 0 },
    { VT_BSTR, 0, 0, 0 /* BT2, BT3, BT4, HC4 */},
    { VT_UI4, 0, 0, 0 /* 1 - 1 << 30 */},
    { VT_UI4, 0, 0, 0 /* 0 - 4 */},
    { VT_UI4, 0, 0, 0 /* 0 - 4 */},
    { VT_UI4, 0, 0, 0 /* 0 - 8 */},
    { VT_BOOL, 0, 0, 0 /* VARIANT_FALSE, VARIANT_TRUE */}
  };
// this is default values
  V_UI4(properties + 0) = 32;               // NumFastBytes
  V_UI4(properties + 1) = 0;                // MatchFinderCycles
  V_UI4(properties + 2) = 1;                // Algorithm
  V_BSTR(properties + 3) = L"BT4";          // MatchFinder
  V_UI4(properties + 4) = 4194304;          // DictionarySize
  V_UI4(properties + 5) = 2;                // PosStateBits
  V_UI4(properties + 6) = 0;                // LitPosBits
  V_UI4(properties + 7) = 3;                // LitContextBits
  V_BOOL(properties + 8) = VARIANT_FALSE;   // EndMarker
// test values for fast
  V_UI4(properties + 2) = 0;                // Algorithm
  V_BSTR(properties + 3) = L"HC4";          // MatchFinder
  V_UI4(properties + 4) = 64 * 1024;      // DictionarySize
  V_BOOL(properties + 8) = VARIANT_TRUE;    // EndMarker

  if( FAILED(encoder_->SetCoderProperties(propIDs,properties,sizeof(propIDs) / sizeof(propIDs[0]))) )
    newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__);
  return *this;
}
//---------------------------------------------------------------------------
StreamCompressionFilter & LZMAFilter::compress(const void * buf,uintptr_t count)
{
  if( count > 0 ){
    encoder_->guest_ = currentFiber();
    encoder_->data_ = buf;
    encoder_->count_ = count;
    encoder_->flush_ = false;
    if( !encoder_->started() || encoder_->finished() ){
      if( encoder_->guest_ != NULL )
        encoder_->guest_->thread()->server()->attachFiber(encoder_);
      else
        attachFiber(encoder_);
    }
    else {
      encoder_->event_.type_ = etDispatch;
      encoder_->thread()->postEvent(&encoder_->event_);
    }
    if( encoder_->guest_ == NULL )
      encoder_->sem_.wait();
    else
      encoder_->guest_->switchFiber(encoder_->guest_->mainFiber());
    if( encoder_->err_ != 0 )
      newObjectV1C2<Exception>(encoder_->err_,__PRETTY_FUNCTION__);
  }
  return *this;
}
//---------------------------------------------------------------------------
StreamCompressionFilter & LZMAFilter::finishCompression()
{
  encoder_->guest_ = currentFiber();
  encoder_->data_ = NULL;
  encoder_->count_ = 0;
  encoder_->flush_ = true;
  if( !encoder_->started() || encoder_->finished() ){
    if( encoder_->guest_ != NULL )
      encoder_->guest_->thread()->server()->attachFiber(encoder_);
    else
      attachFiber(encoder_);
  }
  else {
    encoder_->event_.type_ = etDispatch;
    encoder_->thread()->postEvent(&encoder_->event_);
  }
  if( encoder_->guest_ == NULL )
    encoder_->sem_.wait();
  else
    encoder_->guest_->switchFiber(encoder_->guest_->mainFiber());
  if( encoder_->err_ != 0 )
    newObjectV1C2<Exception>(encoder_->err_,__PRETTY_FUNCTION__);
  dropEncoder();
  return *this;
}
//---------------------------------------------------------------------------
StreamCompressionFilter & LZMAFilter::initializeDecompression()
{
  dropDecoder();
  decoder_ = newObject<Decoder>();
  decoder_->filter_ = this;
  return *this;
}
//---------------------------------------------------------------------------
StreamCompressionFilter & LZMAFilter::decompress(void * buf,uintptr_t count)
{
  if( count > 0 ){
    decoder_->guest_ = currentFiber();
    decoder_->data_ = buf;
    decoder_->count_ = count;
    decoder_->flush_ = false;
    if( !decoder_->started() || decoder_->finished() ){
      if( decoder_->guest_ != NULL )
        decoder_->guest_->thread()->server()->attachFiber(decoder_);
      else
        attachFiber(decoder_);
    }
    else {
      decoder_->event_.type_ = etDispatch;
      decoder_->thread()->postEvent(&decoder_->event_);
    }
    if( decoder_->guest_ == NULL )
      decoder_->sem_.wait();
    else
      decoder_->guest_->switchFiber(decoder_->guest_->mainFiber());
    if( decoder_->err_ != 0 )
      newObjectV1C2<Exception>(encoder_->err_,__PRETTY_FUNCTION__);
  }
  return *this;
}
//---------------------------------------------------------------------------
StreamCompressionFilter & LZMAFilter::finishDecompression()
{
  decoder_->guest_ = currentFiber();
  decoder_->data_ = NULL;
  decoder_->count_ = 0;
  decoder_->flush_ = true;
  if( !decoder_->started() || decoder_->finished() ){
    if( decoder_->guest_ != NULL )
      decoder_->guest_->thread()->server()->attachFiber(decoder_);
    else
      attachFiber(decoder_);
  }
  else {
    decoder_->event_.type_ = etDispatch;
    decoder_->thread()->postEvent(&decoder_->event_);
  }
  if( decoder_->guest_ == NULL )
    decoder_->sem_.wait();
  else
    decoder_->guest_->switchFiber(decoder_->guest_->mainFiber());
  if( decoder_->err_ != 0 )
    newObjectV1C2<Exception>(decoder_->err_,__PRETTY_FUNCTION__);
  dropDecoder();
  return *this;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
StreamCompressionFilter & LZMAFileFilter::compressFile(const utf8::String & srcFileName,const utf8::String & dstFileName)
{
  srcFile_.fileName(srcFileName).readOnly(true).open();
  AutoFileClose autoSrcFileClose(srcFile_);
  dstFile_.fileName(dstFileName).createIfNotExist(true).open();
  AutoFileClose autoDstFileClose(dstFile_);
  dstFile_.resize(0);
  AutoPtr<uint8_t> buf;
  buf.alloc(getpagesize());
  initializeCompression();
  for(;;){
    int64_t r = srcFile_.read(buf,getpagesize());
    if( r <= 0 ) break;
    compress(buf,uintptr_t(r));
  }
  finishCompression();
  return *this;
}
//---------------------------------------------------------------------------
StreamCompressionFilter & LZMAFileFilter::decompressFile(const utf8::String & srcFileName,const utf8::String & dstFileName)
{
  srcFile_.fileName(srcFileName).readOnly(true).open();
  AutoFileClose autoSrcFileClose(srcFile_);
  dstFile_.fileName(dstFileName).createIfNotExist(true).open();
  AutoFileClose autoDstFileClose(dstFile_);
  dstFile_.resize(0);
  AutoPtr<uint8_t> buf;
  buf.alloc(getpagesize());
  initializeDecompression();
  for(;;){
    int64_t r = srcFile_.read(buf,getpagesize());
    if( r <= 0 ) break;
    decompress(buf,uintptr_t(r));
  }
  finishDecompression();
  return *this;
}
//---------------------------------------------------------------------------
StreamCompressionFilter & LZMAFileFilter::writeCompressedBuffer(const void * buf,uintptr_t count)
{
  dstFile_.writeBuffer(buf,count);
  return *this;
}
//---------------------------------------------------------------------------
StreamCompressionFilter & LZMAFileFilter::writeDecompressedBuffer(void * buf,uintptr_t count)
{
  dstFile_.writeBuffer(buf,count);
  return *this;
}
//---------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
