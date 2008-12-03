/*-
 * Copyright 2005-2008 Guram Dukashvili
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
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
uint8_t MemoryStream::nullContainer_[sizeof(MemoryStream::Container)];
//---------------------------------------------------------------------------
MemoryStream::Container::Container(uintptr_t count)
  : ptr_(kmalloc(count)),
    count_(count),
    mcount_(0),
    refCount_(0)
{
  if( count > 0 ) for( mcount_ = 1; mcount_ < count; mcount_ <<= 1 );
}
//---------------------------------------------------------------------------
void MemoryStream::initialize()
{
  new (&nullContainer_) Container(1, NULL, 0);
}
//---------------------------------------------------------------------------
void MemoryStream::cleanup()
{
  //  nullContainer().~Container();
}
//---------------------------------------------------------------------------
MemoryStream & MemoryStream::readBuffer(void * buffer, uintptr_t count)
{
  if( pos_ == container_->count_ )
    newObjectV1C2<EMemoryStreamEOF>(EIO, __PRETTY_FUNCTION__)->throwSP();
  if( pos_ + count > container_->count_ )
    newObjectV1C2<EMemoryStreamReadError>(EIO, __PRETTY_FUNCTION__)->throwSP();
  memcpy(buffer, container_->uptr_ + pos_, count);
  pos_ += count;
  return *this;
}
//---------------------------------------------------------------------------
MemoryStream & MemoryStream::operator >> (utf8::String & s)
{
  uintptr_t pos = pos_;
  while( pos < container_->count_ && container_->uptr_[pos] != '\0' ) pos++;
  if( pos == pos_ && pos == container_->count_ )
    newObjectV1C2<EMemoryStreamEOF>(EIO, __PRETTY_FUNCTION__)->throwSP();
  if( pos > pos_ && pos == container_->count_ )
    newObjectV1C2<EMemoryStreamReadError>(EIO, __PRETTY_FUNCTION__)->throwSP();
  s = utf8::plane(container_->sptr_ + pos_, pos - pos_);
  pos_ = pos;
  return *this;
}
//---------------------------------------------------------------------------
MemoryStream & MemoryStream::resize(uintptr_t newSize)
{
  if( newSize > 0 ){
    if( container_.ptr() == &nullContainer() ){
      container_ = newObjectV1<Container>(newSize);
    }
    else{
      if( newSize > container_->mcount_ || newSize < (container_->mcount_ << 1) ){
        uintptr_t a;
        for( a = 1; a < newSize; a <<= 1 );
        krealloc(container_->ptr_, a);
        container_->mcount_ = a;
      }
      container_->count_ = newSize;
    }
  }
  else{
    container_ = &nullContainer();
  }
  if( pos_ > container_->count_ ) pos_ = container_->count_;
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
EMemoryStreamEOF::EMemoryStreamEOF(int32_t code, const utf8::String & what)
  : Exception(code, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
EMemoryStreamReadError::EMemoryStreamReadError(int32_t code, const utf8::String & what)
  : Exception(code, what)
{
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
