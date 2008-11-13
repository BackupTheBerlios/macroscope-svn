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
#ifndef _autoptr_H_
#define _autoptr_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T,class D = AutoPtrClassDestructor<T> > class AutoPtr {
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

    AutoPtr<T,D> & operator = (T * ptr);

    bool            operator ==(const T * ptr) const;
    bool            operator ==(const AutoPtr<T,D> & ptr) const;
    bool            operator !=(const T * ptr) const;
    bool            operator !=(const AutoPtr<T,D> & ptr) const;

    T * ptr(T * ptr) const;
    T * & ptr() const;
    AutoPtr<T,D> & xchg(AutoPtr<T,D> & ptr);

    AutoPtr<T,D> & alloc(size_t size);
    AutoPtr<T,D> & realloc(size_t size);
    AutoPtr<T,D> & reallocT(uintptr_t count){ return realloc(count * sizeof(T)); }
    T * realloc(size_t size,int);
    AutoPtr<T,D> & free();

    AutoPtr<T,D> & setBit(uintptr_t n);
    AutoPtr<T,D> & resetBit(uintptr_t n);
    AutoPtr<T,D> & invertBit(uintptr_t n);
    uintptr_t bit(uintptr_t n) const;
#if !HAVE_INTPTR_T_AS_INTMAX_T
    AutoPtr<T,D> & setBit(uintmax_t n);
    AutoPtr<T,D> & resetBit(uintmax_t n);
    AutoPtr<T,D> & invertBit(uintmax_t n);
    uintptr_t bit(uintmax_t n) const;
#endif
    AutoPtr<T,D> & setBitRange(uintptr_t n, uintptr_t c);
    AutoPtr<T,D> & resetBitRange(uintptr_t n, uintptr_t c);
    AutoPtr<T,D> & invertBitRange(uintptr_t n, uintptr_t c);
  protected:
  private:
    mutable T * ptr_;
};
//---------------------------------------------------------------------------
template <typename T,class D> inline AutoPtr<T,D>::~AutoPtr()
{
  D::destroyObject(ptr_);
}
//---------------------------------------------------------------------------
template <typename T,class D> inline AutoPtr<T,D>::AutoPtr(T * ptr) : ptr_(ptr)
{
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
T * AutoPtr<T,D>::operator ->()
{
  return ptr_;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
T * AutoPtr<T,D>::operator ->() const
{
  return ptr_;
}
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT
//---------------------------------------------------------------------------
template <typename T,class D> inline
T & AutoPtr<T,D>::operator[](int i)
{
  return ptr_[i];
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
const T & AutoPtr<T,D>::operator[](int i) const
{
  return ptr_[i];
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
T & AutoPtr<T,D>::operator[](unsigned int i)
{
  return ptr_[i];
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
const T & AutoPtr<T,D>::operator[](unsigned int i) const
{
  return ptr_[i];
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
template <typename T,class D> inline
T & AutoPtr<T,D>::operator[](intptr_t i)
{
  return ptr_[i];
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
const T & AutoPtr<T,D>::operator[](intptr_t i) const
{
  return ptr_[i];
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
T & AutoPtr<T,D>::operator[](uintptr_t i)
{
  return ptr_[i];
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
const T & AutoPtr<T,D>::operator[](uintptr_t i) const
{
  return ptr_[i];
}
//---------------------------------------------------------------------------
template <typename T,class D> inline AutoPtr<T,D>::operator T * & ()
{
  return ptr_;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
AutoPtr<T,D>::operator T * const & () const
{
  return ptr_;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline AutoPtr<T,D>::operator T & ()
{
  return *ptr_;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
AutoPtr<T,D>::operator const T & () const
{
  return *ptr_;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
bool AutoPtr<T,D>::operator == (const T * ptr) const
{
  return ptr_ == ptr;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
bool AutoPtr<T,D>::operator ==(const AutoPtr<T,D> & ptr) const
{
  return ptr_ == ptr.ptr_;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
bool AutoPtr<T,D>::operator !=(const T * ptr) const
{
  return ptr_ != ptr;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
bool AutoPtr<T,D>::operator !=(const AutoPtr<T,D> & ptr) const
{
  return ptr_ != ptr.ptr_;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
AutoPtr<T,D> & AutoPtr<T,D>::operator = (T * ptr)
{
  D::destroyObject(ptr_);
  ptr_ = ptr;
  return *this;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
T * AutoPtr<T,D>::ptr(T * ptr) const
{
  ksys::xchg(ptr_, ptr);
  return ptr;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
T * & AutoPtr<T,D>::ptr() const
{
  return ptr_;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
AutoPtr<T,D> & AutoPtr<T,D>::xchg(AutoPtr<T,D> & ptr)
{
  ksys::xchg(ptr_,ptr.ptr_);
  return *this;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
AutoPtr<T,D> & AutoPtr<T,D>::alloc(size_t size)
{
  assert(ptr_ == NULL);
  ptr_ = (T *) kmalloc(size);
  return *this;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
AutoPtr<T,D> & AutoPtr<T,D>::realloc(size_t size)
{
  ptr_ = (T *) krealloc(ptr_,size);
  return *this;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
T * AutoPtr<T,D>::realloc(size_t size,int)
{
  T * p = (T *) ::realloc(ptr_,size);
  if( p != NULL || size == 0 ) ptr_ = p;
  return p;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
AutoPtr<T,D> & AutoPtr<T,D>::free()
{
  kfree(ptr_);
  ptr_ = NULL;
  return *this;
}
//---------------------------------------------------------------------------
template <typename T,class D> inline
AutoPtr<T,D> & AutoPtr<T,D>::setBit(uintptr_t n)
{
  setBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class D> inline
AutoPtr<T,D> & AutoPtr<T,D>::resetBit(uintptr_t n)
{
  resetBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class D> inline
AutoPtr<T,D> & AutoPtr<T,D>::invertBit(uintptr_t n)
{
  invertBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class D> inline
uintptr_t AutoPtr<T,D>::bit(uintptr_t n) const
{
  return bit(ptr_, n);
}
//-----------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INTMAX_T
//-----------------------------------------------------------------------------
template <typename T,class D> inline
AutoPtr<T,D> & AutoPtr<T,D>::setBit(uintmax_t n)
{
  setBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class D> inline
AutoPtr<T,D> & AutoPtr<T,D>::resetBit(uintmax_t n)
{
  resetBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class D> inline
AutoPtr<T,D> & AutoPtr<T,D>::invertBit(uintmax_t n)
{
  invertBit(ptr_, n);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class D> inline
uintptr_t AutoPtr<T,D>::bit(uintmax_t n) const
{
  return bit(ptr_, n);
}
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
template <typename T,class D> inline
AutoPtr<T,D> & AutoPtr<T,D>::setBitRange(uintptr_t n, uintptr_t c)
{
  ksys::setBitRange(ptr_, n, c);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class D> inline
AutoPtr<T,D> & AutoPtr<T,D>::resetBitRange(uintptr_t n, uintptr_t c)
{
  ksys::resetBitRange(ptr_, n, c);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,class D> inline
AutoPtr<T,D> & AutoPtr<T,D>::invertBitRange(uintptr_t n, uintptr_t c)
{
  ksys::invertBitRange(ptr_, n, c);
  return *this;
}
//-----------------------------------------------------------------------------
typedef AutoPtr<uint8_t,AutoPtrMemoryDestructor> AutoPtrBuffer;
//-----------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
