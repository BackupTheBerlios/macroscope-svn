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
#ifndef arrayH
#define arrayH
//-----------------------------------------------------------------------------
namespace kvm {
//-----------------------------------------------------------------------------
template <typename T> class Vector;
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template <typename T> class Array {
  public:
    ~Array();
    Array();
    Array(const T & element);
    Array(const Array<T> & array);
    Array(const Vector<T> & vector);

    Array<T> & operator = (const Array<T> & array);
    Array<T> & operator = (const Vector<T> & vector);
    Array<T> & operator = (const T & element);
#if !HAVE_INTPTR_T_AS_INT_T
    T & operator [] (int i);
    const T &  operator [] (int i) const;
    T & operator [] (unsigned int i);
    const T & operator [] (unsigned int i) const;
#endif
    T & operator [] (intptr_t i);
    const T & operator [] (intptr_t i) const;
    T & operator [] (uintptr_t i);
    const T & operator [] (uintptr_t i) const;

    Array<T> & operator << (const T & element){ return add(element); }

    const uintptr_t & count() const;
    Array<T> & clear();
    const T * ptr() const;
    Array<T> & resize(uintptr_t newSize);
    Array<T> & add(const T & element);
    Array<T> & insert(uintptr_t i, const T & element);

    intptr_t search(const T & element) const;
    intptr_t bSearch(const T & element) const;
    uintptr_t bSearch(const T & element, intptr_t & c) const;
    intptr_t searchCase(const T & element) const;
    intptr_t bSearchCase(const T & element) const;
    uintptr_t bSearchCase(const T & element, intptr_t & c) const;
    Array<T> & remove(uintptr_t i);
    Array<T> & replace(Array<T> & array);
  protected:
    T * ptr_;
    uintptr_t count_;
  private:
};
//-----------------------------------------------------------------------------
template <typename T> inline
Array<T> & Array<T>::replace(Array<T> & array)
{
  kxchg(ptr_,array.ptr_);
  kxchg(count_,array.count_);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline Array<T>::~Array()
{
  clear();
}
//-----------------------------------------------------------------------------
template <typename T> inline Array<T>::Array() : ptr_(NULL), count_(0)
{
}
//-----------------------------------------------------------------------------
template <typename T> inline Array<T>::Array(const T & element) : ptr_(NULL), count_(0)
{
  resize(1).ptr_[0] = element;
}
//-----------------------------------------------------------------------------
template <typename T> inline Array<T>::Array(const Array<T> & array) : ptr_(NULL), count_(0)
{
  *this = array;
}
//-----------------------------------------------------------------------------
template <class T> inline Array<T>::Array(const Vector<T> & vector) : ptr_(NULL), count_(0)
{
  *this = vector;
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::operator = (const Array<T> & array)
{
  Array<T> newArray;
  newArray.ptr_ = (T *) kmalloc(sizeof(T) * array.count_);
  while( newArray.count_ < array.count_ ){
    new (newArray.ptr_ + newArray.count_) T(array.ptr_[newArray.count_]);
    newArray.count_++;
  }
  return replace(newArray);
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::operator = (const Vector<T> & vector)
{
  Array<T> newArray;
  newArray.ptr_ = (T *) kmalloc(sizeof(T) * vector.count());
  while( newArray.count_ < vector.count() ){
    new (newArray.ptr_ + newArray.count_) T(vector[newArray.count_]);
    newArray.count_++;
  }
  return replace(newArray);
}
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
T & Array<T>::operator [] (intptr_t i)
{
  assert((uintptr_t) i < count_);
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <typename T> inline
const T & Array<T>::operator [] (intptr_t i) const
{
  assert((uintptr_t) i < count_);
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <typename T> inline
T & Array<T>::operator [] (uintptr_t i)
{
  assert(i < count_);
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <class T> inline
const T & Array<T>::operator [] (uintptr_t i) const
{
  assert(i < count_);
  return ptr_[i];
}
//-----------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT_T
//-----------------------------------------------------------------------------
template <class T> inline
T & Array<T>::operator [] (int i)
{
  assert( (uintptr_t) i < count_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <class T> inline
const T & Array<T>::operator [] (int i) const
{
  assert( (uintptr_t) i < count_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <class T> inline
T & Array<T>::operator [] (unsigned int i)
{
  assert( i < count_ );
  return ptr_[i];
}
//-----------------------------------------------------------------------------
template <class T> inline
const T & Array<T>::operator [] (unsigned int i) const
{
  assert( i < count_ );
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
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::clear()
{
  while( count_ > 0 ){
    count_--;
    (ptr_ + count_)->~T();
  }
  kmfree(ptr_);
  ptr_ = NULL;
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline
const T * Array<T>::ptr() const
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
  if( newSize == count_ ) return *this;
  Array<T> newArray;
  newArray.ptr_ = (T *) kmalloc(sizeof(T) * newSize);
  while( newArray.count_ < newSize ){
    if( newArray.count_ < count_ )
      new (newArray.ptr_ + newArray.count_) T(ptr_[newArray.count_]);
    else
      new (newArray.ptr_ + newArray.count_) T;
    newArray.count_++;
  }
  return replace(newArray);
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::add(const T & element)
{
  Array<T> newArray;
  newArray.ptr_ = (T *) kmalloc(sizeof(T) * (count_ + 1));
  while( newArray.count_ < count_ ){
    new (newArray.ptr_ + newArray.count_) T(ptr_[newArray.count_]);
    newArray.count_++;
  }
  new (newArray.ptr_ + newArray.count_++) T(element);
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
  Array<T> newArray;
  newArray.ptr_ = (T *) kmalloc(sizeof(T) * (count_ + 1));
  while( newArray.count_ < i ){
    new (newArray.ptr_ + newArray.count_) T(ptr_[newArray.count_]);
    newArray.count_++;
  }
  new (newArray.ptr_ + newArray.count_++) T(element);
  while( newArray.count_ - 1 < count_ ){
    new (newArray.ptr_ + newArray.count_) T(ptr_[newArray.count_ - 1]);
    newArray.count_++;
  }
  return replace(newArray);
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
intptr_t Array<T>::search(const T & element) const
{
  intptr_t  i;
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
  intptr_t  low = 0, high = count_ - 1, pos = -1;

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
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
intptr_t Array<T>::searchCase(const T & element) const
{
  intptr_t  i;
  for( i = count_ - 1; i >= 0; i-- ) if( ptr_[i].strcasecmp(element) == 0 ) break;
  return i;
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
intptr_t Array<T>::bSearchCase(const T & element) const
{
  intptr_t low = 0, high = count_ - 1, pos, c;

  while( low <= high ){
    pos = (low + high) / 2;
    c = element.casecmp(ptr_[pos]);
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
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
uintptr_t Array<T>::bSearchCase(const T & element,intptr_t & c) const
{
  intptr_t low = 0, high = count_ - 1, pos = -1;

  c = 1;
  while( low <= high ){
    pos = (low + high) / 2;
    c = element.casecmp(ptr_[pos]);
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
template<class T>
#ifndef __BCPLUSPLUS__
inline
#endif
Array<T> & Array<T>::remove(uintptr_t i)
{
  assert(i < count_);
  Array<T> newArray;
  newArray.ptr_ = (T *) kmalloc(sizeof(T) * (count_ - 1));
  while( newArray.count_ < i ){
    new (newArray.ptr_ + newArray.count_) T(ptr_[newArray.count_]);
    newArray.count_++;
  }
  while( newArray.count_ + 1 < count_ ){
    new (newArray.ptr_ + newArray.count_) T(ptr_[newArray.count_ + 1]);
    newArray.count_++;
  }
  return replace(newArray);
}
//-----------------------------------------------------------------------------
} // namespace kvm
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
