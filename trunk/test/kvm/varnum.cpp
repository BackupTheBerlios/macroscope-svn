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
#include <adicpp/lconfig.h>
#include <adicpp/endian.h>
#include <adicpp/object.h>
#include <adicpp/xalloc.h>
#include "tlsf.h"
#include "varnum.h"
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
void * memoryPool = NULL;
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
VarInteger::mword_t * VarInteger::realloc(mword_t * p,uintptr_t size)
{
  //if( memoryPool == NULL ){
  //  size_t mps = 0;
  //  DWORD flags = 0;

  //  SYSTEM_INFO si;
  //  GetSystemInfo(&si);
  //  HANDLE hToken;
  //  BOOL r = OpenProcessToken(
  //    GetCurrentProcess(),
  //    TOKEN_ADJUST_PRIVILEGES,
  //    &hToken
  //  );
  //  if( r != 0 ){
  //    LUID luid;
  //    TOKEN_PRIVILEGES tp;
  //    r = LookupPrivilegeValue(
  //      NULL,
  //      SE_LOCK_MEMORY_NAME,
  //      &luid
  //    );
  //    if( r != 0 ){
  //      tp.PrivilegeCount = 1;
  //      tp.Privileges[0].Luid = luid;
  //      tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  //      r = AdjustTokenPrivileges(
  //        hToken,
  //        FALSE,
  //        &tp,
  //        sizeof(TOKEN_PRIVILEGES), 
  //        (PTOKEN_PRIVILEGES)NULL, 
  //        (PDWORD)NULL
  //      );
  //      if( r != 0 && GetLastError() != ERROR_NOT_ALL_ASSIGNED ){
  //        if( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL ||
  //            si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
  //            si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA32_ON_WIN64 ){
  //          union {
  //            SIZE_T (WINAPI * pGetLargePageMinimum)();
  //            FARPROC ph;
  //          };
  //          ph = GetProcAddress(GetModuleHandleA("KERNEL32.DLL"),"GetLargePageMinimum");
  //          if( pGetLargePageMinimum != NULL && pGetLargePageMinimum() > si.dwAllocationGranularity ){
  //            mps = pGetLargePageMinimum();
  //            flags = MEM_LARGE_PAGES;
  //          }
  //        }
  //      }
  //    }
  //    CloseHandle(hToken);
  //  }
  //  flags = MEM_LARGE_PAGES;
  //  mps = tmax(mps,SIZE_T(1024u * 1024u * 1024u));
  //  memoryPool = VirtualAllocEx(GetCurrentProcess(),NULL,mps,MEM_COMMIT | MEM_RESERVE | flags,PAGE_READWRITE);
  //  if( memoryPool == NULL )
  //    memoryPool = VirtualAllocEx(GetCurrentProcess(),NULL,mps,MEM_COMMIT | MEM_RESERVE,PAGE_READWRITE);
  //  init_memory_pool(mps,memoryPool);
  //};
  //return (mword_t *) tlsf_realloc(p,size);
  return (mword_t *) krealloc(p,size);
}
//------------------------------------------------------------------------------
void VarInteger::free(void * p)
{
  //tlsf_free(p);
  kfree(p);
}
//------------------------------------------------------------------------------
VarInteger::~VarInteger()
{
  free(data_);
}
//------------------------------------------------------------------------------
VarInteger::VarInteger() : data_(NULL), count_(0), ref_(0)
{
}
//------------------------------------------------------------------------------
VarInteger::VarInteger(mword_t * data,uintptr_t count) :
  data_(data), count_(count), ref_(0)
{
}
//------------------------------------------------------------------------------
VarInteger::VarInteger(const VarInteger & v) : data_(NULL), count_(0), ref_(0)
{
  operator = (v);
}
//------------------------------------------------------------------------------
VarInteger::VarInteger(int v) : data_(NULL), count_(0), ref_(0)
{
  operator = (v);
}
//------------------------------------------------------------------------------
VarInteger::VarInteger(unsigned int v) : data_(NULL), count_(0), ref_(0)
{
  operator = (v);
}
//------------------------------------------------------------------------------
VarInteger::VarInteger(intptr_t v) : data_(NULL), count_(0), ref_(0)
{
  operator = (v);
}
//------------------------------------------------------------------------------
VarInteger::VarInteger(uintptr_t v) : data_(NULL), count_(0), ref_(0)
{
  operator = (v);
}
//------------------------------------------------------------------------------
VarInteger & VarInteger::operator = (const VarInteger & v)
{
  if( v.ref_ > 0 ){ // temp object
    free(data_);
    data_ = v.data_;
    v.data_ = NULL;
    count_ = v.count_;
    v.count_ = 0;
    ref_ = v.ref_ - 1;
    v.ref_ = 0;
  }
  else {
    mword_t * data = alloc(v.count_ * sizeof(mword_t));
    free(data_);
    memcpy(data_ = data,v.data_,count_ = v.count_);
  }
  return *this;
}
//------------------------------------------------------------------------------
VarInteger & VarInteger::operator = (int v)
{
  if( count_ != 2 ){
    data_ = realloc(data_,2 * sizeof(mword_t));
    count_ = 2;
  }
  data_[0] = mint_t(v);
  data_[1] = mint_t(v) >> (sizeof(mint_t) * 8 - 1);
  return *this;
}
//------------------------------------------------------------------------------
VarInteger & VarInteger::operator = (unsigned int v)
{
  if( count_ != 2 ){
    data_ = realloc(data_,2 * sizeof(mword_t));
    count_ = 2;
  }
  data_[0] = v;
  data_[1] = 0;
  return *this;
}
//------------------------------------------------------------------------------
VarInteger & VarInteger::operator = (intptr_t v)
{
  if( count_ * sizeof(mword_t) != sizeof(v) ){
    data_ = realloc(data_,sizeof(v));
    count_ = sizeof(v) / sizeof(mword_t);
  }
  if( sizeof(v) > sizeof(mint_t) ){
    *(mdint_t *) data_ = v;
  }
  else {
    data_[0] = mint_t(v);
    data_[1] = mint_t(v >> (sizeof(v) * 8 - 1));
  }
  return *this;
}
//------------------------------------------------------------------------------
VarInteger & VarInteger::operator = (uintptr_t v)
{
  if( count_ * sizeof(mword_t) != sizeof(v) * 2 ){
    data_ = realloc(data_,sizeof(v) * 2);
    count_ = sizeof(v) * 2 / sizeof(mword_t);
  }
  memset(data_,0,count_ * sizeof(mword_t));
  memcpy(data_,&v,sizeof(v));
  return *this;
}
//------------------------------------------------------------------------------
VarInteger VarInteger::operator << (intptr_t shift) const
{
  VarInteger q(operator << (uintptr_t(shift)));
  q.ref_ = 2;
  return q;  
}
//------------------------------------------------------------------------------
VarInteger VarInteger::operator << (uintptr_t shift) const
{
  uintptr_t qsize = count_ << 1;
  while( count_ * sizeof(mword_t) * 8 + shift > qsize * sizeof(mword_t) * 8 ) qsize <<= 1;
  qsize += (qsize == 0) * 2;
  VarInteger q(alloc(qsize * sizeof(mword_t)),qsize);
  uintptr_t i, sb = shift & (sizeof(mword_t) * 8 - 1), sw = shift / (sizeof(mword_t) * 8);
  for( i = 0; i <= sw; i++ ) q.data_[i] = 0;
  for( i = 0; i < count_; i++ ){
    q.data_[i + sw] |= data_[i] << sb;
    q.data_[i + sw + 1] = mint_t(data_[i]) >> (sizeof(mword_t) * 8 - sb);
  }
  mint_t s = sign();
  for( i += sw + 1; i < qsize; i++ ) q.data_[i] = s;
  q.ref_ = 2;
  return q;
}
//------------------------------------------------------------------------------
VarInteger VarInteger::operator >> (intptr_t shift) const
{
  VarInteger q(operator >> (uintptr_t(shift)));
  q.ref_ = 2;
  return q;  
}
//------------------------------------------------------------------------------
VarInteger VarInteger::operator >> (uintptr_t shift) const
{
  VarInteger q(alloc(count_ * sizeof(mword_t)),count_);
  uintptr_t i, sb = shift & (sizeof(mword_t) * 8 - 1), sw = shift / (sizeof(mword_t) * 8);
  for( i = 0; i < count_; i++ ){
    q.data_[i] = (data_[i + sw] >> sb) |
                 (data_[i + sw + 1] << (sizeof(mword_t) * 8 - sb));
  }
  mint_t s = sign();
  for( ++i; i < qsize; i++ ) q.data_[i] = s;
  q.ref_ = 2;
  return q;
}
//------------------------------------------------------------------------------
VarInteger VarInteger::operator + (const VarInteger & v) const
{
  uintptr_t qsize = tmax(count_,v.count_) << 1;
  qsize += (qsize == 0) * 2;
  VarInteger q(alloc(qsize * sizeof(mword_t)),qsize);
  mdword_t overflow = 0;
  uintptr_t i, k;
  for( i = 0, k = tmin(count_,v.count_); i < k; i++ )
    q.data_[i] = mword_t(overflow = (overflow >> (sizeof(mdword_t) * 8 / 2)) + data_[i] + v.data_[i]);
  mword_t vs = v.sign();
  for( k = count_; i < k; i++ )
    q.data_[i] = mword_t(overflow = (overflow >> (sizeof(mdword_t) * 8 / 2)) + data_[i] + vs);
  mword_t s = sign();
  for( k = v.count_; i < k; i++ )
    q.data_[i] = mword_t(overflow = (overflow >> (sizeof(mdword_t) * 8 / 2)) + s + v.data_[i]);
  mword_t qs = mint_t(q.data_[i - 1]) >> (sizeof(mint_t) * 8 - 1);
  bool eq = true;
  for( k = qsize; i < k; i++ )
    eq = (q.data_[i] = mword_t(overflow = (overflow >> (sizeof(mdword_t) * 8 / 2)) + s + vs)) == qs && eq;
  if( eq ){
    q.data_ = realloc(q.data_,q.count_ >> 1);
    q.count_ >>= 1;
  }
  q.ref_ = 2;
  return q;
}
//------------------------------------------------------------------------------
VarInteger VarInteger::operator - (const VarInteger & v) const
{
  uintptr_t qsize = tmax(count_,v.count_) << 1;
  qsize += (qsize == 0) * 2;
  VarInteger q(alloc(qsize * sizeof(mword_t)),qsize);
  mdword_t overflow = 0;
  uintptr_t i, k;
  for( i = 0, k = tmin(count_,v.count_); i < k; i++ )
    q.data_[i] = mword_t(overflow = data_[i] - v.data_[i] - (overflow >> (sizeof(mdword_t) * 8 / 2)));
  mword_t vs = v.sign();
  for( k = count_; i < k; i++ )
    q.data_[i] = mword_t(overflow = data_[i] - vs - (overflow >> (sizeof(mdword_t) * 8 / 2)));
  mword_t s = sign();
  for( k = v.count_; i < k; i++ )
    q.data_[i] = mword_t(overflow = s - v.data_[i] - (overflow >> (sizeof(mdword_t) * 8 / 2)));
  mword_t qs = mint_t(q.data_[i - 1]) >> (sizeof(mint_t) * 8 - 1);
  bool eq = true;
  for( k = qsize; i < k; i++ )
    eq = (q.data_[i] = mword_t(overflow = s - vs - (overflow >> (sizeof(mdword_t) * 8 / 2)))) == qs && eq;
  if( eq ){
    q.data_ = realloc(q.data_,q.count_ >> 1);
    q.count_ >>= 1;
  }
  q.ref_ = 2;
  return q;
}
//------------------------------------------------------------------------------
VarInteger VarInteger::operator * (const VarInteger & v) const
{
  uintptr_t qsize = tmax(count_,v.count_) << 2;
  qsize += (qsize == 0) * 2;
  VarInteger q(alloc(qsize * sizeof(mword_t)),qsize);

  q.ref_ = 2;
  return q;
}
//------------------------------------------------------------------------------
uintptr_t VarInteger::print(const char * s,uintptr_t pow) const
{
  return 0;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
VarNumber::~VarNumber()
{
}
//------------------------------------------------------------------------------
VarNumber::VarNumber()
{
}
//------------------------------------------------------------------------------
VarNumber::VarNumber(const VarNumber & v) :
  numerator_(v.numerator_),
  denominator_(v.denominator_)
{
}
//------------------------------------------------------------------------------
VarNumber & VarNumber::operator = (const VarNumber & v)
{
  numerator_ = v.numerator_;
  denominator_ = v.denominator_;
  return *this;
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
