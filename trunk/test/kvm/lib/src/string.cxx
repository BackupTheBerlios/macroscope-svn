/*-
 * Copyright 2009 Guram Dukashvili
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
#include "boot.h"
#include "string.h"
//---------------------------------------------------------------------------
namespace kvm {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
uint8_t String::null_[] = {
#if BYTE_ORDER == LITTLE_ENDIAN
  0x01, 0x00, 0x00, 0x00,
  0xE9, 0xFD, // cpUtf8
#else
  0x00, 0x00, 0x00, 0x01,
  0xFD, 0xE9, // cpUtf8
#endif
  0, 0, 0, 0
};
//---------------------------------------------------------------------------
String::~String()
{
  data_->remRef();
}
//---------------------------------------------------------------------------
String::String() : data_(reinterpret_cast<Data *>(null_)->addRef())
{
}
//---------------------------------------------------------------------------
String::String(const String & s) : data_(s.data_->addRef())
{
}
//---------------------------------------------------------------------------
String::String(const char * s) : data_(reinterpret_cast<Data *>(null_)->addRef())
{
  uintptr_t l = strlen(s);
  Data * data = newData(cpUtf8,l + 1);
  memcpy(data->ss_,s,l + 1);
  data_->remRef();
  data_ = data;
}
//---------------------------------------------------------------------------
String::String(uintptr_t v) : data_(reinterpret_cast<Data *>(null_)->addRef())
{
  char zero[1];
#if HAVE_SNPRINTF
  int l = snprintf(zero,0,"%"PRIuPTR,v);
#elif HAVE__SNPRINTF
  int l = _snprintf(zero,0,"%"PRIuPTR,v);
#endif
  Data * data = newData(cpUtf8,l + 1);
#if HAVE_SNPRINTF
  snprintf(data->ss_,l + 1,"%"PRIuPTR,v);
#elif HAVE__SNPRINTF
  _snprintf(data->ss_,l + 1,"%"PRIuPTR,v);
#endif
  data_->remRef();
  data_ = data;
}
//---------------------------------------------------------------------------
String & String::operator = (const char * s)
{
  uintptr_t l = strlen(s);
  Data * data = newData(cpUtf8,l + 1);
  memcpy(data->ss_,s,l + 1);
  data_->remRef();
  data_ = data;
  return *this;
}
//---------------------------------------------------------------------------
String String::operator + (const char * s) const
{
  return *this + String(s);
}
//---------------------------------------------------------------------------
String String::operator + (const String & s) const
{
  Data * data = NULL;
  if( data_->codePage_ == s.data_->codePage_ ){
    uintptr_t l1 = size(), l2 = s.size(), csz = termCharSize();
    data = newData(data_->codePage_,l1 + l2 + csz);
    memcpy(data->ss_,data_->ss_,l1);
    memcpy(data->ss_ + l1,s.data_->ss_,l2);
    memset(data->ss_ + l1 + l2,0,csz);
  }
  else {
    assert( data_->codePage_ == s.data_->codePage_ );
  }
  return data;
}
//---------------------------------------------------------------------------
String & String::operator += (const char * s)
{
  return *this = *this + String(s);
}
//---------------------------------------------------------------------------
String & String::operator += (const String & s)
{
  return *this = *this + s;
}
//---------------------------------------------------------------------------
String::operator const char * () const
{
  return data_->ss_;
}
//---------------------------------------------------------------------------
String String::print(const char * format, ... )
{
  char zero[1];
  va_list list;
  va_start(list, format);
  int l = vsnprintf(zero,sizeof(zero),format,list);
  va_end(list);
  Data * data = newData(cpUtf8,l + 1);
  vsnprintf(data->ss_,l + 1,format,list);
  return data;
}
//---------------------------------------------------------------------------
uintptr_t String::size() const
{
  union {
    uint16_t * p16_;
    uint32_t * p32_;
  };
  switch( data_->codePage_ ){
    case cpUtf16  :
      for( p16_ = (uint16_t *) data_->ws_; *p16_ != 0; p16_++ );
      return p16_ - (uint16_t *) data_->ws_;
    case cpUtf32  :
      for( p32_ = (uint32_t *) data_->es_; *p32_ != 0; p32_++ );
      return p32_ - (uint32_t *) data_->es_;
    case cpAnsi   :
    case cpOem    :
    case cpMac    :
    case cpUtf7   :
    case cpUtf8   :
    case cpUnix   :
    default       :;
  }
  return strlen(data_->ss_);
}
//---------------------------------------------------------------------------
uintptr_t String::termCharSize() const
{
  switch( data_->codePage_ ){
    case cpUtf16  : return 2;
    case cpUtf32  : return 4;
    case cpAnsi   :
    case cpOem    :
    case cpMac    :
    case cpUtf7   :
    case cpUtf8   :
    case cpUnix   :
    default       :;
  }
  return 1;
}
//---------------------------------------------------------------------------
String::Data * String::newData(uintptr_t codePage,uintptr_t size,uilock_t refCount)
{
  Data * data = NULL;
  size += sizeof(data->refCount_) + sizeof(data->codePage_);
#ifndef NDEBUG
  //if( size < sizeof(Data) ) size = sizeof(Data);
#endif
  data = (Data *) gmalloc(size);
  data->refCount_ = refCount;
  data->codePage_ = uint16_t(codePage);
  return data;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
String operator + (const char * s1,const String & s2)
{
  return String(s1) + s2;
}
//---------------------------------------------------------------------------
} // namespace kvm
//---------------------------------------------------------------------------
