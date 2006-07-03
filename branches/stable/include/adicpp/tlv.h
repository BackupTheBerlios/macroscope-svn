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
#ifndef _tlv_H_
#define _tlv_H_
//---------------------------------------------------------------------------
#ifdef __GNUG__
#define GNUG_NOTHROW __attribute__((nothrow))
#define GNUG_CDECL __attribute__((cdecl))
#define GNUG_NOTHROW_CDECL __attribute__((nothrow,cdecl))
#define GNUG_CONSTRUCTOR __attribute__((constructor))
#define GNUG_NOTHROW_CONSTRUCTOR __attribute__((nothrow,constructor))
#define GNUG_DESTRUCTOR __attribute__((destructor))
#define GNUG_NOTHROW_DESTRUCTOR __attribute__((nothrow,destructor))
#else
#define GNUG_NOTHROW
#define GNUG_CDECL
#define GNUG_NOTHROW_CDECL
#define GNUG_CONSTRUCTOR
#define GNUG_NOTHROW_CONSTRUCTOR
#define GNUG_DESTRUCTOR
#define GNUG_NOTHROW_DESTRUCTOR
#endif
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option push -vGd- -vGt- -vGc- -r -k- -O2 -y- -vi
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template<class T> class ThreadLocalVariable {
  public:
    ~ThreadLocalVariable();
    ThreadLocalVariable(T * defValue = NULL);

    ThreadLocalVariable<T> & operator = (T * ptr);
    T * operator -> () const;
    operator T * () const;
    operator T & () const;
    T * ptr() const;

    bool operator == (const T * const ptr) const;
    bool operator != (const T * const ptr) const;
    bool operator <  (const T * const ptr) const;
    bool operator >  (const T * const ptr) const;
    bool operator <= (const T * const ptr) const;
    bool operator >= (const T * const ptr) const;

    ptrdiff_t operator - (const T * const ptr) const;
  private:
#if defined(__WIN32__) || defined(__WIN64__)
    DWORD key_;
#else
    pthread_key_t key_;
#endif
};
//---------------------------------------------------------------------------
template<class T> inline ThreadLocalVariable<T>::~ThreadLocalVariable()
{
#if defined(__WIN32__) || defined(__WIN64__)
#ifndef NDEBUG
  BOOL r =
#endif
  TlsFree(key_);
#ifndef NDEBUG
  assert( r != 0 );
#endif
#else
#ifndef NDEBUG
  int r =
#endif
  pthread_key_delete(key_);
#ifndef NDEBUG
  assert(r == 0);
#endif
#endif
}
//---------------------------------------------------------------------------
template<class T> inline ThreadLocalVariable<T>::ThreadLocalVariable(T * defValue)
{
#if defined(__WIN32__) || defined(__WIN64__)
  key_ = TlsAlloc();
  assert( key_ != 0xFFFFFFFF );
#ifndef NDEBUG
  BOOL r =
#endif
  TlsSetValue(key_, defValue);
#ifndef NDEBUG
  assert( r != 0 );
#endif
#else
  int r = pthread_key_create(&key_, NULL);
  assert(r == 0);
  r = pthread_setspecific(key_, defValue);
  assert(r == 0);
#endif
}
//---------------------------------------------------------------------------
template< class T> inline
ThreadLocalVariable< T> & ThreadLocalVariable<T>::operator =(T * ptr)
{
#if defined(__WIN32__) || defined(__WIN64__)
#ifndef NDEBUG
  BOOL  r =
#endif
  TlsSetValue(key_, ptr);
#ifndef NDEBUG
  assert( r != 0 );
#endif
#else
#ifndef NDEBUG
  int r =
  #endif
  pthread_setspecific(key_, ptr);
#ifndef NDEBUG
  assert(r == 0);
#endif
#endif
  return *this;
}
//---------------------------------------------------------------------------
template< class T> inline
T * ThreadLocalVariable<T>::operator ->() const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T * v = reinterpret_cast< T *>(TlsGetValue(key_));
  assert( GetLastError() == NO_ERROR );
  return v;
#else
  return reinterpret_cast< T *>(pthread_getspecific(key_));
#endif
}
//---------------------------------------------------------------------------
template< class T> inline ThreadLocalVariable<T>::operator T *() const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T * v = reinterpret_cast< T *>(TlsGetValue(key_));
  assert( GetLastError() == NO_ERROR );
  return v;
#else
  return reinterpret_cast< T *>(pthread_getspecific(key_));
#endif
}
//---------------------------------------------------------------------------
template< class T> inline ThreadLocalVariable<T>::operator T & () const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T * v = reinterpret_cast< T *>(TlsGetValue(key_));
  assert( GetLastError() == NO_ERROR );
  return *v;
#else
  return *reinterpret_cast<T *>(pthread_getspecific(key_));
#endif
}
//---------------------------------------------------------------------------
template< class T> inline
T * ThreadLocalVariable< T>::ptr() const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T * v = reinterpret_cast< T *>(TlsGetValue(key_));
  assert( GetLastError() == NO_ERROR );
  return v;
#else
  return reinterpret_cast< T *>(pthread_getspecific(key_));
#endif
}
//---------------------------------------------------------------------------
template< class T> inline
bool ThreadLocalVariable<T>::operator ==(const T * const ptr) const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T * v = reinterpret_cast< T *>(TlsGetValue(key_));
  assert(GetLastError() == NO_ERROR);
  return v == ptr;
#else
  return reinterpret_cast< T *>(pthread_getspecific(key_)) == ptr;
#endif
}
//---------------------------------------------------------------------------
template< class T> inline
bool ThreadLocalVariable<T>::operator !=(const T * const ptr) const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T * v = reinterpret_cast< T *>(TlsGetValue(key_));
  assert(GetLastError() == NO_ERROR);
  return v != ptr;
#else
  return reinterpret_cast< T *>(pthread_getspecific(key_)) != ptr;
#endif
}
//---------------------------------------------------------------------------
template< class T> inline
bool ThreadLocalVariable<T>::operator >(const T * const ptr) const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T * v = reinterpret_cast< T *>(TlsGetValue(key_));
  assert(GetLastError() == NO_ERROR);
  return v > ptr;
#else
  return reinterpret_cast< T *>(pthread_getspecific(key_)) > ptr;
#endif
}
//---------------------------------------------------------------------------
template< class T> inline
bool ThreadLocalVariable<T>::operator <(const T * const ptr) const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T * v = reinterpret_cast< T *>(TlsGetValue(key_));
  assert(GetLastError() == NO_ERROR);
  return v < ptr;
#else
  return reinterpret_cast< T *>(pthread_getspecific(key_)) < ptr;
#endif
}
//---------------------------------------------------------------------------
template< class T> inline
bool ThreadLocalVariable<T>::operator >=(const T * const ptr) const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T * v = reinterpret_cast< T *>(TlsGetValue(key_));
  assert(GetLastError() == NO_ERROR);
  return v >= ptr;
#else
  return reinterpret_cast< T *>(pthread_getspecific(key_)) >= ptr;
#endif
}
//---------------------------------------------------------------------------
template< class T> inline
bool ThreadLocalVariable<T>::operator <=(const T * const ptr) const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T * v = reinterpret_cast< T *>(TlsGetValue(key_));
  assert(GetLastError() == NO_ERROR);
  return v <= ptr;
#else
  return reinterpret_cast< T *>(pthread_getspecific(key_)) <= ptr;
#endif
}
//---------------------------------------------------------------------------
template< class T> inline
ptrdiff_t ThreadLocalVariable<T>::operator -(const T * const ptr) const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T * v = reinterpret_cast< T *>(TlsGetValue(key_));
  assert(GetLastError() == NO_ERROR);
  return v - ptr;
#else
  return reinterpret_cast< T *>(pthread_getspecific(key_)) - ptr;
#endif
}
//---------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option pop
#endif
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _tlv_H_ */
