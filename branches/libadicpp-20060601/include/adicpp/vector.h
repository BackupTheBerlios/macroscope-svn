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
#ifndef _vector_H
#define _vector_H
//-----------------------------------------------------------------------------
namespace ksys {
//-----------------------------------------------------------------------------
template< class T> class Vector {
  private:
  protected:
    T **      ptr_;
    uintptr_t count_;
    uintptr_t max_;
  public:
                      Vector();
                      Vector(const Vector< T> & s);
                      ~Vector();

    Vector< T> &      operator =(const Vector< T> & s);
    T &               operator[](intptr_t i);
    const T &         operator[](intptr_t i) const;
    T &               operator[](uintptr_t i);
    const T &         operator[](uintptr_t i) const;
    Vector< T> &      operator <<(const T & val);
    Vector< T> &      operator <<(T * val);

    Vector< T> &      assign(const Vector< T> & s);
    const uintptr_t & count() const;
    Vector< T> &      clear();
    Vector< T> &      resize(uintptr_t asize);
    T ** const &      ptr() const;

    T &               add();
    T &               add(const T & val);
    T &               add(T * val);
    T &               safeAdd(T * val);
    T &               insert(uintptr_t i, const T & val);
    T &               insert(uintptr_t i, T * val);
    Vector< T> &      replace(uintptr_t i, T * val);
    Vector< T> &      remove(uintptr_t i);
    T *               cut(uintptr_t i);

    intptr_t          search(const T & element) const;
    intptr_t          bSearch(const T & element) const;
    uintptr_t         bSearch(const T & element, intptr_t & c) const;
    intptr_t          searchCase(const T & element) const;
    intptr_t          bSearchCase(const T & element) const;
    uintptr_t         bSearchCase(const T & element, intptr_t & c) const;
};
//---------------------------------------------------------------------------
template< class T> inline Vector< T>::Vector() : ptr_(NULL), count_(0), max_(0)
{
}
//---------------------------------------------------------------------------
template< class T> inline Vector< T>::Vector(const Vector< T> & s) : ptr_(NULL), count_(0), max_(0)
{
  assign(s);
}
//---------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
Vector< T>::~Vector()
{
  while( count_ > 0 ) delete ptr_[--count_];
  xfree(ptr_);
}
//---------------------------------------------------------------------------
template< class T> inline
Vector< T> & Vector<T>::operator =(const Vector< T> & s)
{
  return assign(s);
}
//---------------------------------------------------------------------------
template< class T> inline
const T & Vector<T>::operator[](intptr_t i) const
{
  assert((uintptr_t) i < count_);
  return *ptr_[i];
}
//---------------------------------------------------------------------------
template< class T> inline
T & Vector<T>::operator[](intptr_t i)
{
  assert((uintptr_t) i < count_);
  return *ptr_[i];
}
//---------------------------------------------------------------------------
template< class T> inline
const T & Vector<T>::operator[](uintptr_t i) const
{
  assert(i < count_);
  return *ptr_[i];
}
//---------------------------------------------------------------------------
template< class T> inline
T & Vector<T>::operator[](uintptr_t i)
{
  assert(i < count_);
  return *ptr_[i];
}
//---------------------------------------------------------------------------
template< class T> inline
Vector< T> & Vector<T>::operator <<(const T & val)
{
  add(val);
  return *this;
}
//---------------------------------------------------------------------------
template< class T> inline
Vector< T> & Vector<T>::operator <<(T * val)
{
  add(val);
  return *this;
}
//---------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
Vector< T> & Vector< T>::assign(const Vector< T> & s)
{
  resize(s.count_);
  for( intptr_t i = count_ - 1; i >= 0; i-- ) *ptr_[i] = *s.ptr_[i];
  return *this;
}
//---------------------------------------------------------------------------
template< class T> inline
const uintptr_t & Vector< T>::count() const
{
  return count_;
}
//---------------------------------------------------------------------------
template< class T> inline
Vector< T> & Vector< T>::clear()
{
  return resize(0);
}
//---------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
Vector< T> & Vector< T>::resize(uintptr_t asize)
{
  while( count_ > asize ) delete ptr_[--count_];
  uintptr_t max = 0;
  if( asize > 0 ) for( max = 1; max < asize; max <<= 1 );
  if( max != max_ ){
    xrealloc(ptr_, sizeof(T *) * max);
    max_ = max;
  }
  while( count_ < asize ) ptr_[count_++] = new T;
  return *this;
}
//---------------------------------------------------------------------------
template< class T> inline
T ** const & Vector< T>::ptr() const
{
  return ptr_;
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
T & Vector< T>::add(const T & val)
{
  uintptr_t amax  = max_;
  while( amax < count_ + 1 ) amax = (amax << 1) + (amax == 0);
  if( amax != max_ ){
    xrealloc(ptr_, sizeof(T *) * amax);
    max_ = amax;
  }
  return *(ptr_[count_++] = new T(val));
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
T & Vector< T>::add(T * val)
{
  uintptr_t amax  = max_;
  while( amax < count_ + 1 ) amax = (amax << 1) + (amax == 0);
  if( amax != max_ ){
    xrealloc(ptr_, sizeof(T *) * amax);
    max_ = amax;
  }
  return *(ptr_[count_++] = val);
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
T & Vector< T>::add()
{
  uintptr_t amax  = max_;
  while( amax < count_ + 1 ) amax = (amax << 1) + (amax == 0);
  if( amax != max_ ){
    xrealloc(ptr_, sizeof(T *) * amax);
    max_ = amax;
  }
  return *(ptr_[count_++] = new T);
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
T & Vector< T>::safeAdd(T * val)
{
  uintptr_t amax  = max_;
  while( amax < count_ + 1 ) amax = (amax << 1) + (amax == 0);
  if( amax != max_ ){
    try{
      xrealloc(ptr_, sizeof(T *) * amax);
      max_ = amax;
    }
    catch( ... ){
      delete val;
      throw;
    }
  }
  return *(ptr_[count_++] = val);
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
T & Vector< T>::insert(uintptr_t i, const T & val)
{
  assert(i <= count_);
  uintptr_t amax  = max_;
  while( amax < count_ + 1 ) amax = (amax << 1) + (amax == 0);
  if( amax != max_ ){
    xrealloc(ptr_, sizeof(T *) * amax);
    max_ = amax;
  }
  memmove(ptr_ + i + 1, ptr_ + i, sizeof(T *) * (count_ - i));
  ptr_[i] = new T(val);
  count_++;
  return *ptr_[i];
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
T & Vector< T>::insert(uintptr_t i, T * val)
{
  assert((uintptr_t) i <= count_);
  uintptr_t amax  = max_;
  while( amax < count_ + 1 ) amax = (amax << 1) + (amax == 0);
  if( amax != max_ ){
    try{
      xrealloc(ptr_, sizeof(T *) * amax);
      max_ = amax;
    }
    catch( ... ){
      delete val;
      throw;
    }
  }
  memmove(ptr_ + i + 1, ptr_ + i, sizeof(T *) * (count_ - i));
  ptr_[i] = val;
  count_++;
  return *ptr_[i];
}
//-----------------------------------------------------------------------------
template< class T> inline
Vector< T> & Vector< T>::replace(uintptr_t i, T * val)
{
  assert(i < count_);
  delete ptr_[i];
  ptr_[i] = val;
  return *this;
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
Vector< T> & Vector< T>::remove(uintptr_t i)
{
  assert(i < count_);
  delete ptr_[i];
  memcpy(ptr_ + i, ptr_ + i + 1, sizeof(T *) * (count_ - i - 1));
  count_--;
  uintptr_t amax  = max_;
  while( amax != 0 && (amax >> 1) >= count_ )
    amax = amax >> 1;
  if( amax != max_ ){
    xrealloc(ptr_, sizeof(T *) * amax);
    max_ = amax;
  }
  return *this;
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
T * Vector< T>::cut(uintptr_t i)
{
  assert(i < count_);
  T * object  = ptr_[i];
  memcpy(ptr_ + i, ptr_ + i + 1, sizeof(T *) * (count_ - i - 1));
  xrealloc(ptr_, sizeof(T *) * (count_ - 1));
  count_--;
  uintptr_t amax  = max_;
  while( amax != 0 && (amax >> 1) >= count_ ) amax = amax >> 1;
  if( amax != max_ ){
    xrealloc(ptr_, sizeof(T *) * amax);
    max_ = amax;
  }
  return object;
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
inline
#endif
intptr_t Vector<T>::search(const T & element) const
{
  intptr_t  i;
  for( i = count_ - 1; i >= 0; i-- ) if( *ptr_[i] == element ) break;
  return i;
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
inline
#endif
intptr_t Vector<T>::bSearch(const T & element) const
{
  intptr_t  low = 0, high = count_ - 1, pos ;

  while( low <= high ){
    pos = (low + high) / 2;
    if( element > *ptr_[pos] ){
      low = pos + 1;
    }
    else if( element < *ptr_[pos] ){
      high = pos - 1;
    }
    else
      return pos;
  }
  return -1;
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
inline
#endif
uintptr_t Vector<T>::bSearch(const T & element, intptr_t & c) const
{
  intptr_t  low = 0, high = count_ - 1, pos = -1;

  c = 1;
  while( low <= high ){
    pos = (low + high) / 2;
    if( element > *ptr_[pos] ){
      low = pos + 1;
      c = 1;
    }
    else if( element < *ptr_[pos] ){
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
intptr_t Vector<T>::searchCase(const T & element) const
{
  intptr_t  i;
  for( i = count_ - 1; i >= 0; i-- ) if( ptr_[i]->strcasecmp(element) == 0 ) break;
  return i;
}
//-----------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
intptr_t Vector<T>::bSearchCase(const T & element) const
{
  intptr_t low = 0, high = count_ - 1, pos, c;

  while( low <= high ){
    pos = (low + high) / 2;
    c = element.strcasecmp(*ptr_[pos]);
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
uintptr_t Vector<T>::bSearchCase(const T & element,intptr_t & c) const
{
  intptr_t low = 0, high = count_ - 1, pos = -1;

  c = 1;
  while( low <= high ){
    pos = (low + high) / 2;
    c = element.strcasecmp(*ptr_[pos]);
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
}
//-----------------------------------------------------------------------------
#endif
