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
#ifndef _autoptr_H_
#define _autoptr_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
void * kmalloc(size_t size,bool noThrow);
void * krealloc(void * p,size_t size,bool noThrow);
void kfree(void * p);
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template< class T> class AutoPtr {
  public:
    AutoPtr(T * ptr = NULL);
    ~AutoPtr();

    T * operator -> ();
    T * operator -> () const;

#if !HAVE_INTPTR_T_AS_INT
    T &             operator [] (int i);
    const T &       operator [] (int i) const;
    T &             operator [] (unsigned int i);
    const T &       operator [] (unsigned int i) const;
#endif
    T &             operator [] (intptr_t i);
    const T &       operator [] (intptr_t i) const;
    T &             operator [] (uintptr_t i);
    const T &       operator [] (uintptr_t i) const;

    operator T * & ();
    operator T * const & () const;
    operator T & ();
    operator const T & () const;

    AutoPtr<T> & operator = (T * ptr);

    bool            operator ==(const T * ptr) const;
    bool            operator ==(const AutoPtr< T> & ptr) const;
    bool            operator !=(const T * ptr) const;
    bool            operator !=(const AutoPtr< T> & ptr) const;

    T * ptr(T * ptr) const;
    T * & ptr() const;
    AutoPtr<T> & xchg(AutoPtr<T> & ptr);

    AutoPtr< T> &   alloc(size_t size);
    AutoPtr< T> &   realloc(size_t size);
    T * realloc(size_t size,int);
    AutoPtr< T> &   free();

    AutoPtr< T> &   setBit(uintptr_t n);
    AutoPtr< T> &   resetBit(uintptr_t n);
    AutoPtr< T> &   invertBit(uintptr_t n);
    uintptr_t       bit(uintptr_t n) const;
#if !HAVE_INTPTR_T_AS_INTMAX_T
    AutoPtr< T> &   setBit(uintmax_t n);
    AutoPtr< T> &   resetBit(uintmax_t n);
    AutoPtr< T> &   invertBit(uintmax_t n);
    uintptr_t       bit(uintmax_t n) const;
#endif
    AutoPtr< T> &   setBitRange(uintptr_t n, uintptr_t c);
    AutoPtr< T> &   resetBitRange(uintptr_t n, uintptr_t c);
    AutoPtr< T> &   invertBitRange(uintptr_t n, uintptr_t c);
  protected:
  private:
    mutable T * ptr_;
};
//---------------------------------------------------------------------------
template< class T> inline AutoPtr< T>::~AutoPtr()
{
  deleteObject(ptr_);
}
//---------------------------------------------------------------------------
template< class T> inline AutoPtr< T>::AutoPtr(T * ptr) : ptr_(ptr)
{
}
//---------------------------------------------------------------------------
template< class T> inline
T * AutoPtr<T>::operator ->()
{
  return ptr_;
}
//---------------------------------------------------------------------------
template< class T> inline
T * AutoPtr<T>::operator ->() const
{
  return ptr_;
}
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT
//---------------------------------------------------------------------------
template< class T> inline
T & AutoPtr<T>::operator[](int i)
{
  return ptr_[i];
}
//---------------------------------------------------------------------------
template< class T> inline
const T & AutoPtr<T>::operator[](int i) const
{
  return ptr_[i];
}
//---------------------------------------------------------------------------
template< class T> inline
T & AutoPtr<T>::operator[](unsigned int i)
{
  return ptr_[i];
}
//---------------------------------------------------------------------------
template< class T> inline
const T & AutoPtr<T>::operator[](unsigned int i) const
{
  return ptr_[i];
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
template< class T> inline
T & AutoPtr<T>::operator[](intptr_t i)
{
  return ptr_[i];
}
//---------------------------------------------------------------------------
template< class T> inline
const T & AutoPtr<T>::operator[](intptr_t i) const
{
  return ptr_[i];
}
//---------------------------------------------------------------------------
template< class T> inline
T & AutoPtr<T>::operator[](uintptr_t i)
{
  return ptr_[i];
}
//---------------------------------------------------------------------------
template< class T> inline
const T & AutoPtr<T>::operator[](uintptr_t i) const
{
  return ptr_[i];
}
//---------------------------------------------------------------------------
template <class T> inline AutoPtr<T>::operator T * & ()
{
  return ptr_;
}
//---------------------------------------------------------------------------
template <class T> inline
AutoPtr<T>::operator T * const & () const
{
  return ptr_;
}
//---------------------------------------------------------------------------
template <class T> inline AutoPtr<T>::operator T & ()
{
  return *ptr_;
}
//---------------------------------------------------------------------------
template <class T> inline
AutoPtr<T>::operator const T & () const
{
  return *ptr_;
}
//---------------------------------------------------------------------------
template <class T> inline
bool AutoPtr<T>::operator == (const T * ptr) const
{
  return ptr_ == ptr;
}
//---------------------------------------------------------------------------
template< class T> inline
bool AutoPtr<T>::operator ==(const AutoPtr< T> & ptr) const
{
  return ptr_ == ptr.ptr_;
}
//---------------------------------------------------------------------------
template< class T> inline
bool AutoPtr<T>::operator !=(const T * ptr) const
{
  return ptr_ != ptr;
}
//---------------------------------------------------------------------------
template< class T> inline
bool AutoPtr<T>::operator !=(const AutoPtr< T> & ptr) const
{
  return ptr_ != ptr.ptr_;
}
//---------------------------------------------------------------------------
template< class T> inline
AutoPtr< T> & AutoPtr<T>::operator = (T * ptr)
{
  deleteObject(ptr_);
  ptr_ = ptr;
  return *this;
}
//---------------------------------------------------------------------------
template< class T> inline
T * AutoPtr<T>::ptr(T * ptr) const
{
  ksys::xchg(ptr_, ptr);
  return ptr;
}
//---------------------------------------------------------------------------
template< class T> inline
T * & AutoPtr< T>::ptr() const
{
  return ptr_;
}
//---------------------------------------------------------------------------
template <class T> inline
AutoPtr<T> & AutoPtr< T>::xchg(AutoPtr<T> & ptr)
{
  ksys::xchg(ptr_,ptr.ptr_);
  return *this;
}
//---------------------------------------------------------------------------
template< class T> inline
AutoPtr< T> & AutoPtr< T>::alloc(size_t size)
{
  assert(ptr_ == NULL);
  ptr_ = (T *) kmalloc(size);
  return *this;
}
//---------------------------------------------------------------------------
template< class T> inline
AutoPtr< T> & AutoPtr< T>::realloc(size_t size)
{
  ptr_ = (T *) krealloc(ptr_,size);
  return *this;
}
//---------------------------------------------------------------------------
template< class T> inline
T * AutoPtr<T>::realloc(size_t size,int)
{
  T * p = (T *) ::realloc(ptr_,size);
  if( p != NULL || size == 0 ) ptr_ = p;
  return p;
}
//---------------------------------------------------------------------------
template< class T> inline
AutoPtr< T> & AutoPtr< T>::free()
{
  kfree(ptr_);
  ptr_ = NULL;
  return *this;
}
//---------------------------------------------------------------------------
template< class T> inline
AutoPtr< T> & AutoPtr< T>::setBit(uintptr_t n)
{
  setBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
AutoPtr< T> & AutoPtr< T>::resetBit(uintptr_t n)
{
  resetBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
AutoPtr< T> & AutoPtr< T>::invertBit(uintptr_t n)
{
  invertBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
uintptr_t AutoPtr< T>::bit(uintptr_t n) const
{
  return bit(ptr_, n);
}
//-----------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INTMAX_T
//-----------------------------------------------------------------------------
template< class T> inline
AutoPtr< T> & AutoPtr< T>::setBit(uintmax_t n)
{
  setBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
AutoPtr< T> & AutoPtr< T>::resetBit(uintmax_t n)
{
  resetBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
AutoPtr< T> & AutoPtr< T>::invertBit(uintmax_t n)
{
  invertBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
uintptr_t AutoPtr< T>::bit(uintmax_t n) const
{
  return bit(ptr_, n);
}
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
template< class T> inline
AutoPtr< T> & AutoPtr< T>::setBitRange(uintptr_t n, uintptr_t c)
{
  ksys::setBitRange(ptr_, n, c);
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
AutoPtr< T> & AutoPtr< T>::resetBitRange(uintptr_t n, uintptr_t c)
{
  ksys::resetBitRange(ptr_, n, c);
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
AutoPtr< T> & AutoPtr< T>::invertBitRange(uintptr_t n, uintptr_t c)
{
  ksys::invertBitRange(ptr_, n, c);
  return *this;
}
//-----------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
