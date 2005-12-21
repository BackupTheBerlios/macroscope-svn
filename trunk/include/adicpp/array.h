/*-
 * Copyright 2005 Guram Dukashvili
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
#ifndef _array_H
#define _array_H
//-----------------------------------------------------------------------------
namespace ksys {
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template <class T> class Array {
  public:
    ~Array();
    Array(T * ptr = NULL);
    Array(const T & element);
    Array(const Array<T> & array);
    
    Array<T> & operator = (const Array<T> & array);
    Array<T> & operator = (const T & element);
    T & operator [] (intptr_t i);
    const T & operator [] (intptr_t i) const;
    T & operator [] (uintptr_t i);
    const T & operator [] (uintptr_t i) const;

    const uintptr_t & count() const;
    Array<T> & clear();
    T * & ptr();
    T * const & ptr() const;
    Array<T> & resize(uintptr_t newSize);
    Array<T> & add(const T & element);
    Array<T> & insert(uintptr_t i,const T & element);

    intptr_t search(const T & element) const;
    intptr_t bSearch(const T & element) const;
    uintptr_t bSearch(const T & element,intptr_t & c) const;
    Array<T> & remove(uintptr_t i);

    Array<T> & setBit(uintptr_t n);
    Array<T> & setBit(uintmax_t n);
    Array<T> & resetBit(uintptr_t n);
    Array<T> & resetBit(uintmax_t n);
    Array<T> & invertBit(uintptr_t n);
    Array<T> & invertBit(uintmax_t n);
    uintptr_t bit(uintptr_t n) const;
    uintptr_t bit(uintmax_t n) const;
    Array<T> & setBitRange(uintptr_t n,uintptr_t c);
    Array<T> & resetBitRange(uintptr_t n,uintptr_t c);
    Array<T> & invertBitRange(uintptr_t n,uintptr_t c);
  protected:
    T * ptr_;
    uintptr_t count_;

    Array(uintptr_t count);
    Array<T> & replace(Array<T> & array);
  private:
};
//-----------------------------------------------------------------------------
template <class T> inline
Array<T> & Array<T>::replace(Array<T> & array)
{
  clear();
  ptr_ = array.ptr_;
  count_ = array.count_;
  array.ptr_ = NULL;
  array.count_ = 0;
  return *this;
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T>::Array(uintptr_t count) : /*ptr_(),*/ count_(0)
{
  xmalloc(ptr_,sizeof(T) * count);
  while( count_ < count ){
    new (ptr_ + count_) T;
    count_++;
  }
}
//-----------------------------------------------------------------------------
template <class T> inline
Array<T>::~Array()
{
  clear();
}
//-----------------------------------------------------------------------------
template <class T> inline
Array<T>::Array(T * ptr) : ptr_(ptr), count_(0)
{
}
//-----------------------------------------------------------------------------
template <class T> inline
Array<T>::Array(const T & element) : ptr_(NULL), count_(0)
{
  resize(1) = element;
}
//-----------------------------------------------------------------------------
template <class T> inline
Array<T>::Array(const Array<T> & array) : ptr_(NULL), count_(0)
{
  *this = array;
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::operator = (const Array<T> & array)
{
  Array<T> newArray(array.count_);
  for( intptr_t i = array.count_ - 1; i >= 0; i-- ) newArray.ptr_[i] = array.ptr_[i];
  return replace(newArray);
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::operator = (const T & element)
{
  for( intptr_t i = count_ - 1; i >= 0; i-- ) ptr_[i] = element;
  return *this;
}
//-----------------------------------------------------------------------------
template <class T> inline
T & Array<T>::operator [] (intptr_t i)
{
  assert( (uintptr_t) i < count_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <class T> inline
const T & Array<T>::operator [] (intptr_t i) const
{
  assert( (uintptr_t) i < count_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <class T> inline
T & Array<T>::operator [] (uintptr_t i)
{
  assert( i < count_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <class T> inline
const T & Array<T>::operator [] (uintptr_t i) const
{
  assert( i < count_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <class T> inline
const uintptr_t & Array<T>::count() const
{
  return count_;
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::clear()
{
  while( count_ > 0 ){
    count_--;
    ptr_[count_].~T();
  }
  xfree(ptr_);
  ptr_ = NULL;
  return *this;
}
//-----------------------------------------------------------------------------
template <class T> inline
T * & Array<T>::ptr()
{
  return ptr_;
}
//-----------------------------------------------------------------------------
template <class T> inline
T * const & Array<T>::ptr() const
{
  return ptr_;
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::resize(uintptr_t newSize)
{
  Array<T> newArray(newSize);
  for( intptr_t i = (newSize > count_ ? count_ : newSize) - 1; i >= 0; i-- )
    newArray.ptr_[i] = ptr_[i];
  return replace(newArray);
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::add(const T & element)
{
  Array<T> newArray(count_ + 1);
  for( intptr_t i = count_ - 1; i >= 0; i-- ) newArray.ptr_[i] = ptr_[i];
  newArray.ptr_[count_] = element;
  return replace(newArray);
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::insert(uintptr_t i,const T & element)
{
  assert( (uintptr_t) i <= count_ );
  Array<T> newArray(count_ + 1);
  uintptr_t j;
  for( j = 0; j < i; j++ ) newArray.ptr_[j] = ptr_[j];
  newArray.ptr_[j] = element;
  while( j < count_ ){
    newArray.ptr_[j + 1] = ptr_[j];
    j++;
  }
  return replace(newArray);
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
intptr_t Array<T>::search(const T & element) const
{
  intptr_t i;
  for( i = count_ - 1; i >= 0; i-- )
    if( ptr_[i] == element ) break;
  return i;
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
intptr_t Array<T>::bSearch(const T & element) const
{
  intptr_t low = 0, high = count_ - 1, pos;

  while( low <= high ){
    pos = (low + high) / 2;
    if( element > ptr_[pos] ){
      low = pos + 1;
    }
    else if( element < ptr_[pos] ){
      high = pos - 1;
    }
    else
     return pos;
  }
  return -1;
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
uintptr_t Array<T>::bSearch(const T & element,intptr_t & c) const
{
  intptr_t low = 0, high = count_ - 1, pos = -1;

  c = 1;
  while( low <= high ){
    pos = (low + high) / 2;
    if( element > ptr_[pos] ){
      low = pos + 1;
      c = 1;
    }
    else if( element < ptr_[pos] ){
      high = pos - 1;
      c = -1;
    }
    else {
      c = 0;
      break;
    }
  }
  return pos;
}

//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::remove(uintptr_t i)
{
  assert( i < count_ );
  Array<T> newArray(count_ - 1);
  uintptr_t j;
  for( j = 0; j < i; j++ ) newArray.ptr_[j] = ptr_[j];
  for( j = i + 1; j < count_; j++ ) newArray.ptr_[j - 1] = ptr_[j];
  return replace(newArray);
}
//-----------------------------------------------------------------------------
template <class T> inline
Array<T> & Array<T>::setBit(uintptr_t n)
{
  assert( n < count_ * 8 );
  setBit(ptr_,n);
  return *this;
}
//-----------------------------------------------------------------------------
template <class T> inline
Array<T> & Array<T>::setBit(uintmax_t n)
{
  assert( n < count_ * 8 );
  setBit(ptr_,n);
  return *this;
}
//-----------------------------------------------------------------------------
template <class T> inline
Array<T> & Array<T>::resetBit(uintptr_t n)
{
  assert( n < count_ * 8 );
  resetBit(ptr_,n);
  return *this;
}
//-----------------------------------------------------------------------------
template <class T> inline
Array<T> & Array<T>::resetBit(uintmax_t n)
{
  assert( n < count_ * 8 );
  resetBit(ptr_,n);
  return *this;
}
//-----------------------------------------------------------------------------
template <class T> inline
Array<T> & Array<T>::invertBit(uintptr_t n)
{
  assert( n < count_ * 8 );
  invertBit(ptr_,n);
  return *this;
}
//-----------------------------------------------------------------------------
template <class T> inline
Array<T> & Array<T>::invertBit(uintmax_t n)
{
  assert( n < count_ * 8 );
  invertBit(ptr_,n);
  return *this;
}
//-----------------------------------------------------------------------------
template <class T> inline
uintptr_t Array<T>::bit(uintptr_t n) const
{
  assert( n < count_ * 8 );
  return bit(ptr_,n);
}
//-----------------------------------------------------------------------------
template <class T> inline
uintptr_t Array<T>::bit(uintmax_t n) const
{
  assert( n < count_ * 8 );
  return bit(ptr_,n);
}
//-----------------------------------------------------------------------------
template <class T> inline
Array<T> & Array<T>::setBitRange(uintptr_t n,uintptr_t c)
{
  assert( n < count_ * 8 && n + c <= count_ * 8 );
  setBitRange(ptr_,n,c);
  return *this;
}
//-----------------------------------------------------------------------------
template <class T> inline
Array<T> & Array<T>::resetBitRange(uintptr_t n,uintptr_t c)
{
  assert( n < count_ * 8 && n + c <= count_ * 8 );
  resetBitRange(ptr_,n,c);
  return *this;
}
//-----------------------------------------------------------------------------
template <class T> inline
Array<T> & Array<T>::invertBitRange(uintptr_t n,uintptr_t c)
{
  assert( n < count_ * 8 && n + c <= count_ * 8 );
  invertBitRange(ptr_,n,c);
  return *this;
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
/*template <class T> class SparseArray {
  private:
  protected:
    T * ptr_;
    long count_;

    SparseArray(long count);
    SparseArray<T> & replace(SparseArray<T> & array);
  public:
    ~SparseArray();
    SparseArray(T * ptr = NULL);
    SparseArray(const SparseArray<T> & array);
    
    SparseArray<T> & operator = (const SparseArray<T> & array);
    SparseArray<T> & operator = (const T & element);
    T & operator [] (long i);
    const T & operator [] (long i) const;

    const long & count() const;
    SparseArray<T> & clear();
    T * & ptr();
    T * const & ptr() const;
    SparseArray<T> & resize(long newSize);
    SparseArray<T> & add(const T & element);
    SparseArray<T> & insert(long i,const T & element);

    long search(const T & element) const;
    long bSearch(const T & element) const;
    long bSearch(const T & element,long & c) const;
    SparseArray<T> & remove(long i);
};
//-----------------------------------------------------------------------------
template <class T> inline
SparseArray<T> & SparseArray<T>::replace(SparseArray<T> & array)
{
  clear();
  ptr_ = array.ptr_;
  count_ = array.count_;
  array.ptr_ = NULL;
  array.count_ = 0;
  return *this;
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
SparseArray<T>::SparseArray(long count) : count_(0)
{
  xmalloc(ptr_,sizeof(T) * count);
  while( count_ < count ){
    new (ptr_ + count_) T;
    count_++;
  }
}
//-----------------------------------------------------------------------------
template <class T> inline
SparseArray<T>::~SparseArray()
{
  clear();
}
//-----------------------------------------------------------------------------
template <class T> inline
SparseArray<T>::SparseArray(T * ptr) : ptr_(ptr), count_(0)
{
}
//-----------------------------------------------------------------------------
template <class T> inline
SparseArray<T>::SparseArray(const SparseArray<T> & array) : ptr_(NULL), count_(0)
{
  *this = array;
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
SparseArray<T> & SparseArray<T>::operator = (const SparseArray<T> & array)
{
  SparseArray<T> newArray(array.count_);
  for( long i = array.count_ - 1; i >= 0; i-- ) newArray.ptr_[i] = array.ptr_[i];
  return replace(newArray);
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
SparseArray<T> & SparseArray<T>::operator = (const T & element)
{
  for( long i = count_ - 1; i >= 0; i-- ) ptr_[i] = element;
  return *this;
}
//-----------------------------------------------------------------------------
template <class T> inline
T & SparseArray<T>::operator [] (long i)
{
  assert( i >= 0 && i < count_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <class T> inline
const T & SparseArray<T>::operator [] (long i) const
{
  assert( i >= 0 && i < count_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <class T> inline
const long & SparseArray<T>::count() const
{
  return count_;
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
SparseArray<T> & SparseArray<T>::clear()
{
  while( count_ > 0 ){
    count_--;
    ptr_[count_].~T();
  }
  xfree(ptr_);
  ptr_ = NULL;
  return *this;
}
//-----------------------------------------------------------------------------
template <class T> inline
T * & SparseArray<T>::ptr()
{
  return ptr_;
}
//-----------------------------------------------------------------------------
template <class T> inline
T * const & SparseArray<T>::ptr() const
{
  return ptr_;
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
SparseArray<T> & SparseArray<T>::resize(long newSize)
{
  SparseArray<T> newArray(newSize);
  for( long i = (newSize > count_ ? count_ : newSize) - 1; i >= 0; i-- )
    newArray.ptr_[i] = ptr_[i];
  return replace(newArray);
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
SparseArray<T> & SparseArray<T>::add(const T & element)
{
  SparseArray<T> newArray(count_ + 1);
  for( long i = count_ - 1; i >= 0; i-- ) newArray.ptr_[i] = ptr_[i];
  newArray.ptr_[count_] = element;
  return replace(newArray);
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
SparseArray<T> & SparseArray<T>::insert(long i,const T & element)
{
  assert( i >= 0 && i <= count_ );
  SparseArray<T> newArray(count_ + 1);
  long j;
  for( j = 0; j < i; j++ ) newArray.ptr_[j] = ptr_[j];
  newArray.ptr_[j] = element;
  while( j < count_ ){
    newArray.ptr_[j + 1] = ptr_[j];
    j++;
  }
  return replace(newArray);
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
long SparseArray<T>::search(const T & element) const
{
  long i;
  for( i = count_ - 1; i >= 0; i-- )
    if( ptr_[i] == element ) break;
  return i;
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
long SparseArray<T>::bSearch(const T & element) const
{
  long low = 0, high = count_ - 1, pos;

  while( low <= high ){
    pos = (low + high) / 2;
    if( element > ptr_[pos] ){
      low = pos + 1;
    }
    else if( element < ptr_[pos] ){
      high = pos - 1;
    }
    else
     return pos;
  }
  return -1;
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
long SparseArray<T>::bSearch(const T & element,long & c) const
{
  long low = 0, high = count_ - 1, pos = -1;

  c = 1;
  while( low <= high ){
    pos = (low + high) / 2;
    if( element > ptr_[pos] ){
      low = pos + 1;
      c = 1;
    }
    else if( element < ptr_[pos] ){
      high = pos - 1;
      c = -1;
    }
    else {
      c = 0;
      break;
    }
  }
  return pos;
}

//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
SparseArray<T> & SparseArray<T>::remove(long i)
{
  assert( i >= 0 && i < count_ );
  SparseArray<T> newArray(count_ - 1);
  long j;
  for( j = 0; j < i; j++ ) newArray.ptr_[j] = ptr_[j];
  for( j = i + 1; j < count_; j++ ) newArray.ptr_[j - 1] = ptr_[j];
  return replace(newArray);
}*/
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
#endif
