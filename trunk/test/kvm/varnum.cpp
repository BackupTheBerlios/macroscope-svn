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
#include <adicpp/atomic.h>
#include <adicpp/bits.h>
#include <adicpp/autoptr.h>
#include <adicpp/sp.h>
#include "tlsf.h"
#include "varnum.h"
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
VarInteger::Container * VarInteger::Container::initialize(void * data,uintptr_t count,uintptr_t ref)
{
  Container * p = NULL;
  try {
    p = (Container *) VarInteger::alloc(sizeof(Container));
  }
  catch( ... ){
    VarInteger::free(data);
    throw;
  }
  p->data_ = (mword_t *) data;
  p->count_ = count;
  p->ref_ = ref;
  return p;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
//static void * memoryPool = NULL;
static VarInteger * varIntegerPool = NULL;
static VarInteger::Container * varIntegerContainerPool = NULL;
static void * varIntegerPools[sizeof(uintptr_t) * 8 - 2];
//------------------------------------------------------------------------------
void * VarInteger::realloc(void * p,uintptr_t size)
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
  return krealloc(p,size);
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
}
//------------------------------------------------------------------------------
static VarInteger::mword_t defaultContainerData = 0;
static VarInteger::Container defaultContainer = { &defaultContainerData, 1, 1 };
//------------------------------------------------------------------------------
VarInteger::VarInteger() : container_(&defaultContainer)
{
}
//------------------------------------------------------------------------------
VarInteger::VarInteger(void * data,uintptr_t count) :
  container_(Container::initialize(data,count))
{
}
//------------------------------------------------------------------------------
VarInteger::VarInteger(const VarInteger & v) : container_(v.container_)
{
}
//------------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT
VarInteger::VarInteger(int v) : container_(&defaultContainer)
{
  operator = (v);
}
//------------------------------------------------------------------------------
VarInteger::VarInteger(unsigned int v) : container_(&defaultContainer)
{
  operator = (v);
}
#endif
//------------------------------------------------------------------------------
VarInteger::VarInteger(intptr_t v) : container_(&defaultContainer)
{
  operator = (v);
}
//------------------------------------------------------------------------------
VarInteger::VarInteger(uintptr_t v) : container_(&defaultContainer)
{
  operator = (v);
}
//------------------------------------------------------------------------------
#if !HAVE_INTMAX_T_AS_INT && !HAVE_INTMAX_T_AS_INTPTR_T
VarInteger::VarInteger(intmax_t v) : container_(&defaultContainer)
{
  operator = (v);
}
//------------------------------------------------------------------------------
VarInteger::VarInteger(uintmax_t v) : container_(&defaultContainer)
{
  operator = (v);
}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
VarInteger & VarInteger::operator = (const VarInteger & v)
{
  container_ = v.container_;
  return *this;
}
//------------------------------------------------------------------------------
VarInteger & VarInteger::operator = (intptr_t v)
{
  container_ = Container::initialize(alloc(sizeof(v)),sizeof(v) / sizeof(mword_t));
  if( sizeof(v) > sizeof(mword_t) ){
    *(intptr_t *) container_->data_ = v;
  }
  else {
    container_->data_[0] = mint_t(v);
    memset(container_->data_ + 1,int(v >> (sizeof(v) * 8 - 1)),(container_->count_ - 1) * sizeof(mword_t));
  }
  return *this;
}
//------------------------------------------------------------------------------
VarInteger & VarInteger::operator = (uintptr_t v)
{
  container_ = Container::initialize(alloc(sizeof(v) * 2),sizeof(v) * 2 / sizeof(mword_t));
  *(intptr_t *) container_->data_ = v;
  memset(container_->data_ + sizeof(v) / sizeof(mword_t),0,container_->count_ / 2 * sizeof(mword_t));
  return *this;
}
//------------------------------------------------------------------------------
#if !HAVE_INTMAX_T_AS_INT && !HAVE_INTMAX_T_AS_INTPTR_T
VarInteger & VarInteger::operator = (intmax_t v)
{
  container_ = Container::initialize(alloc(sizeof(v)),sizeof(v) / sizeof(mword_t));
  if( sizeof(v) > sizeof(mword_t) ){
    *(intmax_t *) container_->data_ = v;
  }
  else {
    container_->data_[0] = mint_t(v);
    memset(container_->data_ + 1,int(v >> (sizeof(v) * 8 - 1)),(container_->count_ - 1) * sizeof(mword_t));
  }
  return *this;
}
//------------------------------------------------------------------------------
VarInteger & VarInteger::operator = (uintmax_t v)
{
  container_ = Container::initialize(alloc(sizeof(v) * 2),sizeof(v) * 2 / sizeof(mword_t));
  *(intmax_t *) container_->data_ = v;
  memset(container_->data_ + sizeof(v) / sizeof(mword_t),0,container_->count_ / 2 * sizeof(mword_t));
  return *this;
}
#endif
//------------------------------------------------------------------------------
VarInteger VarInteger::operator << (uintptr_t shift) const
{
  if( shift == 0 ) return *this;
  uintptr_t qsize = container_->count_ << 1;
  while( container_->count_ * sizeof(mword_t) * 8 + shift > qsize * sizeof(mword_t) * 8 ) qsize <<= 1;
  VarInteger q(alloc(qsize * sizeof(mword_t)),qsize);
  uintptr_t i, k, sb = shift & (sizeof(mword_t) * 8 - 1), sw = shift / (sizeof(mword_t) * 8);
  mword_t * data = container_->data_, * qdata = q.container_->data_;
  for( i = 0; i < sw; i++ ) qdata[i] = 0;
  if( sb == 0 ){
    for( i = 0, k = container_->count_; i < k; i++ ){
      qdata[i + sw] = data[i];
    }
  }
  else {
    qdata[i] = 0;
    for( i = 0, k = container_->count_; i < k; i++ ){
      qdata[i + sw] |= data[i] << sb;
      qdata[i + sw + 1] = mint_t(data[i]) >> (sizeof(mword_t) * 8 - sb);
    }
  }
  mint_t s = sign();
  for( i += sw + (sb != 0); i < qsize; i++ ) qdata[i] = s;
  return q;
}
//------------------------------------------------------------------------------
VarInteger VarInteger::operator >> (uintptr_t shift) const
{
  if( shift == 0 ) return *this;
  VarInteger q(alloc(container_->count_ * sizeof(mword_t)),container_->count_);
  uintptr_t i, k, sb = shift & (sizeof(mword_t) * 8 - 1), sw = shift / (sizeof(mword_t) * 8);
  mword_t * data = container_->data_, * qdata = q.container_->data_;
  for( i = 0, k = container_->count_; i < k; i++ ){
    qdata[i] = (data[i + sw] >> sb) | (data[i + sw + 1] << (sizeof(mword_t) * 8 - sb));
  }
  mint_t s = sign();
  for( ++i, k = q.container_->count_; i < k; i++ ) qdata[i] = s;
  return q;
}
//------------------------------------------------------------------------------
VarInteger VarInteger::operator + (const VarInteger & v) const
{
  uintptr_t qsize = tmax(container_->count_,v.container_->count_) << 1;
  qsize += (qsize == 0) * 2;
  VarInteger q(alloc(qsize * sizeof(mword_t)),qsize);
  mword_t * data = container_->data_, * vdata = v.container_->data_, * qdata = q.container_->data_;
  mword_t overflow = 0;
  uintptr_t i, k;
  for( i = 0, k = tmin(container_->count_,v.container_->count_); i < k; i++ ){
    mdword_t x = mdword_t(data[i]) + vdata[i];
    qdata[i] = mword_t(x) + overflow;
    overflow = mword_t(x >> (sizeof(mdword_t) * 8 / 2));
  }
  mword_t vs = v.sign();
  for( k = container_->count_; i < k; i++ ){
    mdword_t x = mdword_t(data[i]) + vs;
    qdata[i] = mword_t(x) + overflow;
    overflow = mword_t(x >> (sizeof(mdword_t) * 8 / 2));
  }
  mword_t s = sign();
  for( k = v.container_->count_; i < k; i++ ){
    mdword_t x = mdword_t(s) + vdata[i];
    qdata[i] = mword_t(x) + overflow;
    overflow = mword_t(x >> (sizeof(mdword_t) * 8 / 2));
  }
  mword_t qs = mint_t(qdata[i - 1]) >> (sizeof(mint_t) * 8 - 1);
  bool eq = true;
  for( k = qsize; i < k; i++ ){
    mdword_t x = mdword_t(s) + vs;
    eq = (qdata[i] = mword_t(x) + overflow) == qs && eq;
    overflow = mword_t(x >> (sizeof(mdword_t) * 8 / 2));
  }
  if( eq ){
    q.container_->data_ = (mword_t *) realloc(qdata,(q.container_->count_ >> 1) * sizeof(mword_t));
    q.container_->count_ >>= 1;
  }
  return q;
}
//------------------------------------------------------------------------------
VarInteger VarInteger::operator - (const VarInteger & v) const
{
  uintptr_t qsize = tmax(container_->count_,v.container_->count_) << 1;
  qsize += (qsize == 0) * 2;
  VarInteger q(alloc(qsize * sizeof(mword_t)),qsize);
  mword_t * data = container_->data_, * vdata = v.container_->data_, * qdata = q.container_->data_;
  mword_t overflow = 0;
  uintptr_t i, k;
  for( i = 0, k = tmin(container_->count_,v.container_->count_); i < k; i++ ){
    mdword_t x = mdword_t(data[i]) - vdata[i];
    qdata[i] = mword_t(x) + overflow;
    overflow = mword_t(x >> (sizeof(mdword_t) * 8 / 2));
  }
  mword_t vs = v.sign();
  for( k = container_->count_; i < k; i++ ){
    mdword_t x = mdword_t(data[i]) - vs;
    qdata[i] = mword_t(x) + overflow;
    overflow = mword_t(x >> (sizeof(mdword_t) * 8 / 2));
  }
  mword_t s = sign();
  for( k = v.container_->count_; i < k; i++ ){
    mdword_t x = mdword_t(s) - vdata[i];
    qdata[i] = mword_t(x) + overflow;
    overflow = mword_t(x >> (sizeof(mdword_t) * 8 / 2));
  }
  mword_t qs = mint_t(qdata[i - 1]) >> (sizeof(mint_t) * 8 - 1);
  bool eq = true;
  for( k = qsize; i < k; i++ ){
    mdword_t x = mdword_t(s) - vs;
    eq = (qdata[i] = mword_t(x) + overflow) == qs && eq;
    overflow = mword_t(x >> (sizeof(mdword_t) * 8 / 2));
  }
  if( eq ){
    q.container_->data_ = (mword_t *) realloc(qdata,(q.container_->count_ >> 1) * sizeof(mword_t));
    q.container_->count_ >>= 1;
  }
  return q;
}
//------------------------------------------------------------------------------
VarInteger VarInteger::operator * (const VarInteger & v) const
{
  VarInteger summ;
  VarInteger abst(abs());
  VarInteger absa(v.abs());
  mword_t * absaData = absa.container_->data_;
  for( uintptr_t k = absa.container_->count_ * sizeof(mword_t) * 8, i = 0, j = 0; i < k; ){
    mword_t a = absaData[i / (sizeof(mword_t) * 8)];
    if( a == 0 ){
      i += sizeof(mword_t) * 8;
      continue;
    }
    for( uintptr_t b = sizeof(mword_t) * 8; b > 0; b-- ){
      if( (a & (mword_t(1) << (i & (sizeof(mword_t) * 8 - 1)))) != 0 ){
        abst <<= i - j;
        summ += abst;
        j = i;
      }
      i++;
    }
  }
  if( isNeg() ^ v.isNeg() ) return -summ;
  return summ;
}
//------------------------------------------------------------------------------
VarInteger VarInteger::operator - () const
{
  uintptr_t qsize = container_->count_;
  VarInteger q(alloc(qsize * sizeof(mword_t)),qsize);
  mword_t * data = container_->data_, * qdata = q.container_->data_;
  mword_t overflow = 0;
  uintptr_t i;
  for( i = 0; i < qsize; i++ ){
    mdword_t x = mdword_t(0) - data[i];
    qdata[i] = mword_t(x) + overflow;
    overflow = mword_t(x >> (sizeof(mdword_t) * 8 / 2));
  }
  return q;
}
//------------------------------------------------------------------------------
intptr_t VarInteger::asIntPtrT() const
{
  intptr_t v;
  memcpy(&v,container_->data_,tmin(sizeof(v),container_->count_ * sizeof(mword_t)));
  return v;
}
//------------------------------------------------------------------------------
uintptr_t VarInteger::asUIntPtrT() const
{
  uintptr_t v;
  memcpy(&v,container_->data_,tmin(sizeof(v),container_->count_ * sizeof(mword_t)));
  return v;
}
//------------------------------------------------------------------------------
#if !HAVE_INTMAX_T_AS_INT && !HAVE_INTMAX_T_AS_INTPTR_T
intmax_t VarInteger::asIntMaxT() const
{
  intmax_t v;
  memcpy(&v,container_->data_,tmin(sizeof(v),container_->count_ * sizeof(mword_t)));
  return v;
}
//------------------------------------------------------------------------------
uintmax_t VarInteger::asUIntMaxT() const
{
  uintmax_t v;
  memcpy(&v,container_->data_,tmin(sizeof(v),container_->count_ * sizeof(mword_t)));
  return v;
}
#endif
//------------------------------------------------------------------------------
VarInteger::mint_t VarInteger::compare(const VarInteger & v) const
{
  mword_t * data = container_->data_, * vdata = v.container_->data_;
  mword_t overflow = 0, lv = 0;
  bool zero = true;
  uintptr_t i, k;
  for( i = 0, k = tmin(container_->count_,v.container_->count_); i < k; i++ ){
    mdword_t x = mdword_t(data[i]) - vdata[i];
    lv = mword_t(x) + overflow;
    zero = lv == 0 && zero;
    overflow = mword_t(x >> (sizeof(mdword_t) * 8 / 2));
  }
  mword_t vs = v.sign();
  for( k = container_->count_; i < k; i++ ){
    mdword_t x = mdword_t(data[i]) - vs;
    lv = mword_t(x) + overflow;
    zero = lv == 0 && zero;
    overflow = mword_t(x >> (sizeof(mdword_t) * 8 / 2));
  }
  mword_t s = sign();
  for( k = v.container_->count_; i < k; i++ ){
    mdword_t x = mdword_t(s) - vdata[i];
    lv = mword_t(x) + overflow;
    zero = lv == 0 && zero;
    overflow = mword_t(x >> (sizeof(mdword_t) * 8 / 2));
  }
  return zero ? 0 : lv == 0 ? 1 : lv;
}
//------------------------------------------------------------------------------
VarInteger VarInteger::div(const VarInteger & divider,VarInteger * remainder) const
{
  if( divider.isZero() ){ int zero = 0; zero /= zero; }
  VarInteger remainder2;
  VarInteger & mod = remainder == NULL ? remainder2 : *remainder;
  mod = abs();
  VarInteger summ;
  if( !isZero() ){
    VarInteger one(1);
    VarInteger divr(divider.abs());
    uintptr_t shift;

    if( divr.isOne() ){
      summ = mod;
    }
    else {
      while( mod >= divr ){
        intptr_t k = mod.getFirstSignificantBitIndex() - divr.getFirstSignificantBitIndex() - 1;
        for( shift = k <= 0 ? 1 : k; (divr << shift) <= mod; shift++ );
        summ += one << (shift - 1);
        mod -= divr << (shift - 1);
      }
    }             
    if( isNeg() ^ divider.isNeg() ){
      summ = -summ;
      mod = -mod;
    }
  }
  return summ;
}
//------------------------------------------------------------------------------
VarInteger VarInteger::abs() const
{
  return sign() >= 0 ? *this : -*this;
}
//------------------------------------------------------------------------------
uintptr_t VarInteger::print(char * s,uintptr_t pow) const
{
  char * p = s;
  uintptr_t l = 0;
  VarInteger m(*this), q;
  do {
    m = m.div(10,&q);
    if( s != NULL ) *s++ = char(q.abs().asUIntPtrT()) + '0';
    l++;
  } while( !m.isZero() );
  if( isNeg() ) if( s != NULL ){ *s++ = '-'; l++; }
  if( s != NULL ) reverseByteArray(p,l);
  return l;
}
//------------------------------------------------------------------------------
bool VarInteger::isZero() const
{
  mword_t * data = container_->data_;
  for( intptr_t i = container_->count_ - 1; i >= 0; i-- )
    if( data[i] != 0 ) return false;
  return true;
}
//------------------------------------------------------------------------------
bool VarInteger::isOne() const
{
  mword_t * data = container_->data_;
  if( data[0] != 1 ) return false;
  for( intptr_t i = container_->count_ - 1; i >= 1; i-- )
    if( data[i] != 0 ) return false;
  return true;
}
//------------------------------------------------------------------------------
intptr_t VarInteger::getFirstSignificantBitIndex() const
{
  mword_t * data = container_->data_, s = sign();
  intptr_t i, j;
  for( i = container_->count_ - 1; i >= 1 && data[i] == s; i-- );
  s = ~s & 1;
  mword_t q = data[i];
  for( j = sizeof(mword_t) * 8 - 1; j >= 0; j-- ){
    if( ((q >> j) & 1) == s ) break;
  }
  return i * sizeof(mword_t) * 8 + j;
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
