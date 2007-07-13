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
#ifndef NDEBUG
//---------------------------------------------------------------------------
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
    err_ = 0;
    HRESULT hr = Code(this,this,NULL,NULL,NULL);
    assert( flush_ );
    err_ = HRESULT_CODE(hr);
    if( guest_ == NULL ){
      sem_.post();
    }
    else {
      guest_->event_.type_ = etDispatch;
      guest_->thread()->postEvent(&guest_->event_);
    }
    currentFiber()->switchFiber(currentFiber()->mainFiber());
  }
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE LZMAFilter::Encoder::Read(void * data,UInt32 size,UInt32 * processedSize)
{
  HRESULT hr = S_OK;
  try {
    if( processedSize != NULL ) *processedSize = 0;
    for(;;){
      UInt32 r = (UInt32) filter_->encoderRead(data,size);
      if( r > 0 ){
        data = (uint8_t *) data + r;
        size -= r;
        if( processedSize != NULL ) *processedSize += r;
      }
      if( size == 0 || flush_ ) break;
      if( r == 0 ){
        if( guest_ == NULL ){
          sem_.post();
        }
        else {
          guest_->event_.type_ = etDispatch;
          guest_->thread()->postEvent(&guest_->event_);
        }
        currentFiber()->switchFiber(currentFiber()->mainFiber());
      }
    }
  }
  catch( ExceptionSP & e ){
    hr = HRESULT_FROM_WIN32(e->code());
  }
  return hr;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE LZMAFilter::Encoder::Write(const void *data,UInt32 size,UInt32 * processedSize)
{
  HRESULT hr = S_OK;
  try {
    if( processedSize != NULL ) *processedSize = 0;
    while( size > 0 ){
      UInt32 w = (UInt32) filter_->encoderWrite(data,size);
      if( processedSize != NULL ) *processedSize += w;
      size -= w;
    }
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
    err_ = 0;
    HRESULT hr = Code(this,this,NULL,NULL,NULL);
    assert( flush_ );
    err_ = HRESULT_CODE(hr);
    if( guest_ == NULL ){
      sem_.post();
    }
    else {
      guest_->event_.type_ = etDispatch;
      guest_->thread()->postEvent(&guest_->event_);
    }
    currentFiber()->switchFiber(currentFiber()->mainFiber());
  }
}
//------------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE LZMAFilter::Decoder::Read(void * data,UInt32 size,UInt32 * processedSize)
{
  HRESULT hr = S_OK;
  try {
    if( processedSize != NULL ) *processedSize = 0;
    for(;;){
      UInt32 r = (UInt32) filter_->decoderRead(data,size);
      if( r > 0 ){
        data = (uint8_t *) data + r;
        size -= r;
        if( processedSize != NULL ) *processedSize += r;
      }
      if( size == 0 || flush_ ) break;
      if( r == 0 ){
        if( guest_ == NULL ){
          sem_.post();
        }
        else {
          guest_->event_.type_ = etDispatch;
          guest_->thread()->postEvent(&guest_->event_);
        }
        currentFiber()->switchFiber(currentFiber()->mainFiber());
      }
    }
  }
  catch( ExceptionSP & e ){
    hr = HRESULT_FROM_WIN32(e->code());
  }
  return hr;
}
//---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE LZMAFilter::Decoder::Write(const void *data,UInt32 size,UInt32 * processedSize)
{
  HRESULT hr = S_OK;
  try {
    if( processedSize != NULL ) *processedSize = 0;
    while( size > 0 ){
      UInt32 w = (UInt32) filter_->decoderWrite(data,size);
      if( processedSize != NULL ) *processedSize += w;
      size -= w;
    }
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

  HRESULT hr = encoder_->SetCoderProperties(propIDs,properties,sizeof(propIDs) / sizeof(propIDs[0]));
  if( SUCCEEDED(hr) ) hr = encoder_->WriteCoderProperties(encoder_);
  if( FAILED(hr) ) newObjectV1C2<Exception>(HRESULT_CODE(hr) + errorOffset,__PRETTY_FUNCTION__);
  return *this;
}
//---------------------------------------------------------------------------
StreamCompressionFilter & LZMAFilter::compress()
{
  encoder_->guest_ = currentFiber();
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
  return *this;
}
//---------------------------------------------------------------------------
StreamCompressionFilter & LZMAFilter::finishCompression()
{
  encoder_->guest_ = currentFiber();
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
  HRESULT hr = decoder_->Read(decoder_->coderProperties_,sizeof(decoder_->coderProperties_),NULL);
  if( SUCCEEDED(hr) )
    hr = decoder_->SetDecoderProperties2(decoder_->coderProperties_,sizeof(decoder_->coderProperties_));
  if( FAILED(hr) ) newObjectV1C2<Exception>(HRESULT_CODE(hr),__PRETTY_FUNCTION__);
  return *this;
}
//---------------------------------------------------------------------------
StreamCompressionFilter & LZMAFilter::decompress()
{
  decoder_->guest_ = currentFiber();
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
  return *this;
}
//---------------------------------------------------------------------------
StreamCompressionFilter & LZMAFilter::finishDecompression()
{
  decoder_->guest_ = currentFiber();
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
  initializeCompression();
  compress();
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
  initializeDecompression();
  decompress();
  finishDecompression();
  return *this;
}
//---------------------------------------------------------------------------
intptr_t LZMAFileFilter::encoderRead(void * buf,uintptr_t size)
{
  int64_t r = srcFile_.read(buf,size);
  if( r < 0 ){
    int32_t err = oserror() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__);
  }
  return intptr_t(r);
}
//---------------------------------------------------------------------------
intptr_t LZMAFileFilter::encoderWrite(const void * buf,uintptr_t size)
{
  dstFile_.writeBuffer(buf,size);
  return size;
}
//---------------------------------------------------------------------------
intptr_t LZMAFileFilter::decoderRead(void * buf,uintptr_t size)
{
  int64_t r = srcFile_.read(buf,size);
  if( r < 0 ){
    int32_t err = oserror() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__);
  }
  return intptr_t(r);
}
//---------------------------------------------------------------------------
intptr_t LZMAFileFilter::decoderWrite(const void * buf,uintptr_t size)
{
  dstFile_.writeBuffer(buf,size);
  return size;
}
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
intptr_t LZMADescriptorFilter::encoderRead(void * buf,uintptr_t size)
{
  intptr_t r = 0;
  while( size > 0 ){
    uintptr_t a = encodeBytes_ > size ? size : encodeBytes_;
    memcpy(buf,encodeBuffer_,a);
    buf = (uint8_t *) buf + a;
    encodeBuffer_ = (uint8_t *) encodeBuffer_ + a;
    encodeBytes_ -= a;
    size -= a;
    r += a;
    if( encodeBytes_ == 0 ){
      if( encoder_->guest_ == NULL ){
        encoder_->sem_.post();
      }
      else {
        encoder_->guest_->event_.type_ = etDispatch;
        encoder_->guest_->thread()->postEvent(&encoder_->guest_->event_);
      }
      currentFiber()->switchFiber(currentFiber()->mainFiber());
    }
  }
  return r;
}
//---------------------------------------------------------------------------
intptr_t LZMADescriptorFilter::encoderWrite(const void * buf,uintptr_t size)
{
  int64_t w = descriptor_->writeV(buf,size);
  if( w < 0 ){
    int32_t err = oserror() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__);
  }
  return intptr_t(w);
}
//---------------------------------------------------------------------------
intptr_t LZMADescriptorFilter::decoderRead(void * buf,uintptr_t size)
{
  int64_t r = descriptor_->readV(buf,size);
  if( r < 0 ){
    int32_t err = oserror() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__);
  }
  return intptr_t(r);
}
//---------------------------------------------------------------------------
intptr_t LZMADescriptorFilter::decoderWrite(const void * buf,uintptr_t size)
{
  intptr_t w = 0;
  while( size > 0 ){
    uintptr_t a = decodeBytes_ > size ? size : decodeBytes_;
    memcpy(decodeBuffer_,buf,a);
    buf = (uint8_t *) buf + a;
    decodeBuffer_ = (uint8_t *) decodeBuffer_ + a;
    decodeBytes_ -= a;
    size -= a;
    w += a;
    if( decodeBytes_ == 0 ){
      if( decoder_->guest_ == NULL ){
        decoder_->sem_.post();
      }
      else {
        decoder_->guest_->event_.type_ = etDispatch;
        decoder_->guest_->thread()->postEvent(&decoder_->guest_->event_);
      }
      currentFiber()->switchFiber(currentFiber()->mainFiber());
    }
  }
  return w;
}
//---------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
