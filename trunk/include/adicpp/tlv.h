/*-
 * Copyright 2005-2007 Guram Dukashvili
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
namespace ksys {
//---------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option push -vGd- -vGt- -vGc- -r -k- -O2 -y- -vi
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T> class ThreadLocalVariable {
  public:
    ~ThreadLocalVariable();
    ThreadLocalVariable();
    ThreadLocalVariable(const T defValue);
    ThreadLocalVariable(const ThreadLocalVariable<T> & a);

    ThreadLocalVariable<T> & operator = (const T ptr);
    ThreadLocalVariable<T> & operator = (const ThreadLocalVariable<T> & a){ *this = (T) a; return *this; }
    
    operator T () const;

    bool operator == (const T ptr) const;
    bool operator != (const T ptr) const;
    bool operator <  (const T ptr) const;
    bool operator >  (const T ptr) const;
    bool operator <= (const T ptr) const;
    bool operator >= (const T ptr) const;

    ptrdiff_t operator - (const T ptr) const;
  private:
#if defined(__WIN32__) || defined(__WIN64__)
    DWORD key_;
#else
    pthread_key_t key_;
#endif
};
//---------------------------------------------------------------------------
template <typename T> inline ThreadLocalVariable<T>::~ThreadLocalVariable()
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
template <typename T> inline ThreadLocalVariable<T>::ThreadLocalVariable()
{
#if defined(__WIN32__) || defined(__WIN64__)
  key_ = TlsAlloc();
  assert( key_ != 0xFFFFFFFF );
#ifndef NDEBUG
  BOOL r =
#endif
  TlsSetValue(key_,0);
#ifndef NDEBUG
  assert( r != 0 );
#endif
#else
  int r = pthread_key_create(&key_,NULL);
  assert(r == 0);
  r = pthread_setspecific(key_,(const void *) NULL);
  assert(r == 0);
#endif
}
//---------------------------------------------------------------------------
template <typename T> inline ThreadLocalVariable<T>::ThreadLocalVariable(const T defValue)
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
  r = pthread_setspecific(key_,(const void *) defValue);
  assert(r == 0);
#endif
}
//---------------------------------------------------------------------------
template <typename T> inline ThreadLocalVariable<T>::ThreadLocalVariable(const ThreadLocalVariable<T> & a)
{
#if defined(__WIN32__) || defined(__WIN64__)
  key_ = TlsAlloc();
  assert( key_ != 0xFFFFFFFF );
#ifndef NDEBUG
  BOOL r =
#endif
  TlsSetValue(key_,(T) a);
#ifndef NDEBUG
  assert( r != 0 );
#endif
#else
  int r = pthread_key_create(&key_, NULL);
  assert(r == 0);
  r = pthread_setspecific(key_,(const void *) (T) a);
  assert(r == 0);
#endif
}
//---------------------------------------------------------------------------
template <typename T> inline
ThreadLocalVariable< T> & ThreadLocalVariable<T>::operator = (const T ptr)
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
  pthread_setspecific(key_,(const void *) ptr);
#ifndef NDEBUG
  assert(r == 0);
#endif
#endif
  return *this;
}
//---------------------------------------------------------------------------
template <typename T> inline ThreadLocalVariable<T>::operator T () const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T v = reinterpret_cast<T>(TlsGetValue(key_));
  assert( GetLastError() == NO_ERROR );
  return v;
#else
  return reinterpret_cast<T>(pthread_getspecific(key_));
#endif
}
//---------------------------------------------------------------------------
template <typename T> inline
bool ThreadLocalVariable<T>::operator ==(const T ptr) const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T v = reinterpret_cast<T>(TlsGetValue(key_));
  assert(GetLastError() == NO_ERROR);
  return v == ptr;
#else
  return reinterpret_cast<T>(pthread_getspecific(key_)) == ptr;
#endif
}
//---------------------------------------------------------------------------
template <typename T> inline
bool ThreadLocalVariable<T>::operator !=(const T ptr) const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T v = reinterpret_cast<T>(TlsGetValue(key_));
  assert(GetLastError() == NO_ERROR);
  return v != ptr;
#else
  return reinterpret_cast<T>(pthread_getspecific(key_)) != ptr;
#endif
}
//---------------------------------------------------------------------------
template <typename T> inline
bool ThreadLocalVariable<T>::operator >(const T ptr) const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T v = reinterpret_cast<T>(TlsGetValue(key_));
  assert(GetLastError() == NO_ERROR);
  return v > ptr;
#else
  return reinterpret_cast<T>(pthread_getspecific(key_)) > ptr;
#endif
}
//---------------------------------------------------------------------------
template <typename T> inline
bool ThreadLocalVariable<T>::operator <(const T ptr) const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T v = reinterpret_cast<T>(TlsGetValue(key_));
  assert(GetLastError() == NO_ERROR);
  return v < ptr;
#else
  return reinterpret_cast<T>(pthread_getspecific(key_)) < ptr;
#endif
}
//---------------------------------------------------------------------------
template <typename T> inline
bool ThreadLocalVariable<T>::operator >=(const T ptr) const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T v = reinterpret_cast<T>(TlsGetValue(key_));
  assert(GetLastError() == NO_ERROR);
  return v >= ptr;
#else
  return reinterpret_cast<T>(pthread_getspecific(key_)) >= ptr;
#endif
}
//---------------------------------------------------------------------------
template <typename T> inline
bool ThreadLocalVariable<T>::operator <=(const T ptr) const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T v = reinterpret_cast<T>(TlsGetValue(key_));
  assert(GetLastError() == NO_ERROR);
  return v <= ptr;
#else
  return reinterpret_cast<T>(pthread_getspecific(key_)) <= ptr;
#endif
}
//---------------------------------------------------------------------------
template <typename T> inline
ptrdiff_t ThreadLocalVariable<T>::operator -(const T ptr) const
{
#if defined(__WIN32__) || defined(__WIN64__)
  T v = reinterpret_cast<T>(TlsGetValue(key_));
  assert(GetLastError() == NO_ERROR);
  return v - ptr;
#else
  return reinterpret_cast<T>(pthread_getspecific(key_)) - ptr;
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
//---------------------------------------------------------------------------
