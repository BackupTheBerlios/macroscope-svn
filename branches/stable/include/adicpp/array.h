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
#ifndef _array_H
#define _array_H
//-----------------------------------------------------------------------------
namespace ksys {
//-----------------------------------------------------------------------------
//template <typename T> class Vector;
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template <typename T> class Array {
  public:
    virtual ~Array();
    Array(T * ptr = NULL);
    Array(const T & element);
    Array(const Array<T> & array);
    //Array(const Vector<T> & vector);

    Array<T> & operator = (const Array<T> & array);
    //Array<T> & operator = (const Vector<T> & vector);
    Array<T> & operator = (const T & element);
#if !HAVE_INTPTR_T_AS_INT
    T &        operator [] (int i);
    const T &  operator [] (int i) const;
    T &        operator [] (unsigned int i);
    const T &  operator [] (unsigned int i) const;
#endif
    T &        operator [] (intptr_t i);
    const T &  operator [] (intptr_t i) const;
    T &        operator [] (uintptr_t i);
    const T &  operator [] (uintptr_t i) const;

    operator T * (){ return ptr_; }
    operator const T * () const { return ptr_; }

    Array<T> & operator << (const T & element){ return add(element); }

    const uintptr_t & count() const;
    const uintptr_t & mcount() const;
    Array<T> &       clear();
    T * &             ptr() const;
    Array<T> &       resize(uintptr_t newSize);
    Array<T> &       reserve(uintptr_t newMaxSize);
    Array<T> &       add();
    Array<T> &       add(int,int);
    Array<T> &       add(const T & element);
    Array<T> &       insert(uintptr_t i, const T & element);

    intptr_t          search(const T & element) const;
    intptr_t          bSearch(const T & element) const;
    uintptr_t         bSearch(const T & element, intptr_t & c) const;
    intptr_t          searchCase(const T & element) const;
    intptr_t          bSearchCase(const T & element) const;
    uintptr_t         bSearchCase(const T & element, intptr_t & c) const;
    Array<T> &       remove(uintptr_t i);

    Array<T> &       setBit(uintptr_t n);
    Array<T> &       resetBit(uintptr_t n);
    Array<T> &       invertBit(uintptr_t n);
    uintptr_t         bit(uintptr_t n) const;
#if !HAVE_INTPTR_T_AS_INTMAX_T
    Array<T> &       setBit(uintmax_t n);
    Array<T> &       resetBit(uintmax_t n);
    Array<T> &       invertBit(uintmax_t n);
    uintptr_t         bit(uintmax_t n) const;
#endif
    Array<T> &       setBitRange(uintptr_t n, uintptr_t c);
    Array<T> &       resetBitRange(uintptr_t n, uintptr_t c);
    Array<T> &       invertBitRange(uintptr_t n, uintptr_t c);
    Array<T> & replace(Array<T> & array);
  protected:
    mutable T *       ptr_;
    mutable uintptr_t count_;
    mutable uintptr_t mcount_;
  private:
};
//-----------------------------------------------------------------------------
template <typename T> inline
Array<T> & Array<T>::replace(Array<T> & array)
{
  xchg(ptr_,array.ptr_);
  xchg(count_,array.count_);
  xchg(mcount_,array.mcount_);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline Array<T>::~Array()
{
  clear();
}
//-----------------------------------------------------------------------------
template <typename T> inline Array<T>::Array(T * ptr) : ptr_(ptr), count_(0), mcount_(0)
{
}
//-----------------------------------------------------------------------------
template <typename T> inline Array<T>::Array(const T & element) : ptr_(NULL), count_(0), mcount_(0)
{
  resize(1).ptr_[0] = element;
}
//-----------------------------------------------------------------------------
template <typename T> inline Array<T>::Array(const Array<T> & array) : ptr_(NULL), count_(0), mcount_(0)
{
  *this = array;
}
//-----------------------------------------------------------------------------
//template <typename T> inline Array<T>::Array(const Vector<T> & vector) : ptr_(NULL), count_(0), mcount_(0)
//{
//  *this = vector;
//}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::operator = (const Array<T> & array)
{
  Array<T> newArray;
  newArray.ptr_ = (T *) kmalloc(sizeof(T) * array.mcount_);
  while( newArray.count_ < array.count_ ){
    new (newArray.ptr_ + newArray.count_) T(array.ptr_[newArray.count_]);
    newArray.count_++;
  }
  newArray.mcount_ = newArray.count_;
  while( newArray.mcount_ < array.mcount_ ){
    new (newArray.ptr_ + newArray.mcount_) T();
    newArray.mcount_++;
  }
  return replace(newArray);
}
//-----------------------------------------------------------------------------
//template <typename T>
//#ifndef __BCPLUSPLUS__
//inline
//#endif
//Array<T> & Array<T>::operator = (const Vector<T> & vector)
//{
//  Array<T> newArray;
//  newArray.ptr_ = (T *) kmalloc(sizeof(T) * vector.mcount());
//  while( newArray.count_ < vector.count() ){
//    if( vector[newArray.count_] != NULL )
//      new (newArray.ptr_ + newArray.count_) T(vector[newArray.count_]);
//    else
//      new (newArray.ptr_ + newArray.count_) T;
//    newArray.count_++;
//  }
//  newArray.mcount_ = newArray.count_;
//  while( newArray.mcount_ < vector.mcount() ){
//    new (newArray.ptr_ + newArray.mcount_) T();
//    newArray.mcount_++;
//  }
//  return replace(newArray);
//}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::operator = (const T & element)
{
  for( intptr_t i = count_ - 1; i >= 0; i-- ) ptr_[i] = element;
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline
T & Array<T>::operator[](intptr_t i)
{
  assert( (uintptr_t) i < mcount_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <typename T> inline
const T & Array<T>::operator[](intptr_t i) const
{
  assert( (uintptr_t) i < mcount_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <typename T> inline
T & Array<T>::operator[](uintptr_t i)
{
  assert( i < mcount_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <typename T> inline
const T & Array<T>::operator[](uintptr_t i) const
{
  assert( i < mcount_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT
//-----------------------------------------------------------------------------
template <typename T> inline
T & Array<T>::operator [] (int i)
{
  assert( (uintptr_t) i < mcount_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <typename T> inline
const T & Array<T>::operator [] (int i) const
{
  assert( (uintptr_t) i < mcount_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <typename T> inline
T & Array<T>::operator [] (unsigned int i)
{
  assert( i < mcount_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <typename T> inline
const T & Array<T>::operator [] (unsigned int i) const
{
  assert( i < mcount_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
template <typename T> inline
const uintptr_t & Array<T>::count() const
{
  return count_;
}
//-----------------------------------------------------------------------------
template <typename T> inline
const uintptr_t & Array<T>::mcount() const
{
  return mcount_;
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::clear()
{
  while( mcount_ > 0 ){
    mcount_--;
    (ptr_ + mcount_)->~T();
  }
  count_ = 0;
  kfree(ptr_);
  ptr_ = NULL;
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline
T * & Array<T>::ptr() const
{
  return ptr_;
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::resize(uintptr_t newSize)
{
  uintptr_t mcount = mcount_;
  while( newSize < mcount / 2u ) mcount /= 2;
  while( newSize > mcount ) mcount = (mcount * 2u) + (mcount == 0);
  if( mcount != mcount_ ){
    Array<T> newArray;
    newArray.ptr_ = (T *) kmalloc(sizeof(T) * mcount);
    while( newArray.count_ < mcount ){
      if( newArray.count_ < count_ )
        new (newArray.ptr_ + newArray.count_) T(ptr_[newArray.count_]);
      else
        new (newArray.ptr_ + newArray.count_) T;
      newArray.count_++;
      newArray.mcount_++;
    }
    replace(newArray);
  }
  count_ = newSize;
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline
Array<T> & Array<T>::reserve(uintptr_t newMaxSize)
{
  uintptr_t count = count_;
  resize(newMaxSize);
  count_ = count > mcount_ ? mcount_ : count;
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::add(int,int)
{
  count_++;
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::add()
{
  if( count_ == mcount_ ){
    uintptr_t mcount = mcount_ * 2u + (mcount_ == 0);
    Array<T> newArray;
    newArray.ptr_ = (T *) kmalloc(sizeof(T) * mcount);
    while( newArray.count_ < count_ ){
      new (newArray.ptr_ + newArray.count_) T(ptr_[newArray.count_]);
      newArray.count_++;
      newArray.mcount_++;
    }
    while( newArray.mcount_ < mcount ){
      new (newArray.ptr_ + newArray.mcount_) T;
      newArray.mcount_++;
    }
    replace(newArray);
  }
  count_++;
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::add(const T & element)
{
  if( count_ == mcount_ ){
    uintptr_t mcount = mcount_ * 2u + (mcount_ == 0);
    Array<T> newArray;
    newArray.ptr_ = (T *) kmalloc(sizeof(T) * mcount);
    while( newArray.count_ < count_ ){
      new (newArray.ptr_ + newArray.count_) T(ptr_[newArray.count_]);
      newArray.count_++;
      newArray.mcount_++;
    }
    while( newArray.mcount_ < mcount ){
      new (newArray.ptr_ + newArray.mcount_) T;
      newArray.mcount_++;
    }
    replace(newArray);
  }
  ptr_[count_++] = element;
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::insert(uintptr_t i,const T & element)
{
  assert( i < count_ );
  uintptr_t mcount;
  if( count_ == mcount_ ){
    mcount = mcount_ * 2u + (mcount_ == 0);
    Array<T> newArray;
    newArray.ptr_ = (T *) kmalloc(sizeof(T) * mcount);
    while( newArray.count_ < i ){
      new (newArray.ptr_ + newArray.count_) T(ptr_[newArray.count_]);
      newArray.count_++;
      newArray.mcount_++;
    }
    new (newArray.ptr_ + newArray.count_) T(element);
    newArray.count_++;
    newArray.mcount_++;
    while( newArray.count_ - 1 < count_ ){
      new (newArray.ptr_ + newArray.count_) T(ptr_[newArray.count_ - 1]);
      newArray.count_++;
      newArray.mcount_++;
    }
    while( newArray.mcount_ < mcount ){
      new (newArray.ptr_ + newArray.mcount_) T;
      newArray.mcount_++;
    }
    replace(newArray);
  }
  else {
    for( mcount = count_; mcount > i; mcount-- ) ptr_[mcount] = ptr_[mcount - 1];
    ptr_[count_++] = element;
  }
  return *this;
}
//-----------------------------------------------------------------------------
template<typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::remove(uintptr_t i)
{
  assert( i < count_ );
  uintptr_t mcount;
  if( count_ - 1 == mcount_ / 2u ){
    mcount = mcount_ / 2u + (mcount_ == 0);
    Array<T> newArray;
    newArray.ptr_ = (T *) kmalloc(sizeof(T) * mcount);
    while( newArray.count_ < i ){
      new (newArray.ptr_ + newArray.count_) T(ptr_[newArray.count_]);
      newArray.count_++;
      newArray.mcount_++;
    }
    while( newArray.count_ + 1 < count_ ){
      new (newArray.ptr_ + newArray.count_) T(ptr_[newArray.count_ + 1]);
      newArray.count_++;
      newArray.mcount_++;
    }
    replace(newArray);
  }
  else {
    for( mcount = i; mcount < count_ - 1; mcount++ ) ptr_[mcount] = ptr_[mcount + 1];
    count_--;
  }
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
intptr_t Array<T>::search(const T & element) const
{
  intptr_t i;
  for( i = count_ - 1; i >= 0; i-- ) if( ptr_[i] == element ) break;
  return i;
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
intptr_t Array<T>::bSearch(const T & element) const
{
  intptr_t  low = 0, high = count_ - 1, pos ;

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
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
uintptr_t Array<T>::bSearch(const T & element, intptr_t & c) const
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
    else{
      c = 0;
      break;
    }
  }
  return pos;
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
intptr_t Array<T>::searchCase(const T & element) const
{
  intptr_t  i;
  for( i = count_ - 1; i >= 0; i-- ) if( ptr_[i].casecompare(element) == 0 ) break;
  return i;
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
intptr_t Array<T>::bSearchCase(const T & element) const
{
  intptr_t low = 0, high = count_ - 1, pos, c;

  while( low <= high ){
    pos = (low + high) / 2;
    c = element.casecompare(ptr_[pos]);
    if( c > 0 ){
      low = pos + 1;
    }
    else if( c < 0 ){
      high = pos - 1;
    }
    else
      return pos;
  }
  return -1;
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
uintptr_t Array<T>::bSearchCase(const T & element,intptr_t & c) const
{
  intptr_t low = 0, high = count_ - 1, pos = -1;

  c = 1;
  while( low <= high ){
    pos = (low + high) / 2;
    c = element.casecompare(ptr_[pos]);
    if( c > 0 ){
      low = pos + 1;
    }
    else if( c < 0 ){
      high = pos - 1;
    }
    else 
      break;
  }
  return pos;
}
//-----------------------------------------------------------------------------
template <typename T> inline
Array<T> & Array<T>::setBit(uintptr_t n)
{
  assert(n < count_ * 8);
  setBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline
Array<T> & Array<T>::resetBit(uintptr_t n)
{
  assert(n < count_ * 8);
  resetBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline
Array<T> & Array<T>::invertBit(uintptr_t n)
{
  assert(n < count_ * 8);
  invertBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline
uintptr_t Array<T>::bit(uintptr_t n) const
{
  assert(n < count_ * 8);
  return bit(ptr_, n);
}
//-----------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INTMAX_T
//-----------------------------------------------------------------------------
template <typename T> inline
Array<T> & Array<T>::setBit(uintmax_t n)
{
  assert(n < count_ * 8);
  setBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline
Array<T> & Array<T>::resetBit(uintmax_t n)
{
  assert(n < count_ * 8);
  resetBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline
Array<T> & Array<T>::invertBit(uintmax_t n)
{
  assert(n < count_ * 8);
  invertBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline
uintptr_t Array<T>::bit(uintmax_t n) const
{
  assert(n < count_ * 8);
  return bit(ptr_, n);
}
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
template <typename T> inline
Array<T> & Array<T>::setBitRange(uintptr_t n, uintptr_t c)
{
  assert(n < count_ * 8 && n + c <= count_ * 8);
  setBitRange(ptr_, n, c);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline
Array<T> & Array<T>::resetBitRange(uintptr_t n, uintptr_t c)
{
  assert(n < count_ * 8 && n + c <= count_ * 8);
  resetBitRange(ptr_, n, c);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline
Array<T> & Array<T>::invertBitRange(uintptr_t n, uintptr_t c)
{
  assert(n < count_ * 8 && n + c <= count_ * 8);
  invertBitRange(ptr_, n, c);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,typename ST>
ST & operator << (ST & stream,const Array<T> & array)
{
  uint64_t u = array.count();
  stream << u;
  while( u-- > 0 ) stream << array[(uintptr_t) u];
  return stream;
}
//-----------------------------------------------------------------------------
template <typename T,typename ST>
ST & operator >> (ST & stream,Array<T> & array)
{
  Array<T> t;
  uint64_t u;
  stream >> u;
  t.resize((uintptr_t) u);
  while( u > 0 ) stream >> t[(uintptr_t) --u];
  array.replace(t);
  return stream;
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
/*template <typename T> class SparseArray {
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
template <typename T> inline
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
template <typename T>
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
template <typename T> inline
SparseArray<T>::~SparseArray()
{
  clear();
}
//-----------------------------------------------------------------------------
template <typename T> inline
SparseArray<T>::SparseArray(T * ptr) : ptr_(ptr), count_(0)
{
}
//-----------------------------------------------------------------------------
template <typename T> inline
SparseArray<T>::SparseArray(const SparseArray<T> & array) : ptr_(NULL), count_(0)
{
  *this = array;
}
//-----------------------------------------------------------------------------
template <typename T>
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
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
SparseArray<T> & SparseArray<T>::operator = (const T & element)
{
  for( long i = count_ - 1; i >= 0; i-- ) ptr_[i] = element;
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline
T & SparseArray<T>::operator [] (long i)
{
  assert( i >= 0 && i < count_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <typename T> inline
const T & SparseArray<T>::operator [] (long i) const
{
  assert( i >= 0 && i < count_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <typename T> inline
const long & SparseArray<T>::count() const
{
  return count_;
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
SparseArray<T> & SparseArray<T>::clear()
{
  while( count_ > 0 ){
    count_--;
    ptr_[count_].~T();
  }
  kfree(ptr_);
  ptr_ = NULL;
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline
T * & SparseArray<T>::ptr()
{
  return ptr_;
}
//-----------------------------------------------------------------------------
template <typename T> inline
T * const & SparseArray<T>::ptr() const
{
  return ptr_;
}
//-----------------------------------------------------------------------------
template <typename T>
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
template <typename T>
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
template <typename T>
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
template <typename T>
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
template <typename T>
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
template <typename T>
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
template <typename T>
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
