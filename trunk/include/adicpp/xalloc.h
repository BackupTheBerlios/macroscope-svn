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
#ifndef _XAlloc_H_
#define _XAlloc_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
void * kmalloc(size_t size);
void * krealloc(void * p, size_t size);
void kfree(void * p);
//---------------------------------------------------------------------------
template <class T> inline T * xmalloc(T *& lp, size_t size)
{
  lp = NULL;
  return lp = (T *) kmalloc(size);
}
//---------------------------------------------------------------------------
template <typename T> inline T * xrealloc(T *& lp, size_t size)
{
  return lp = (T *) krealloc(lp, size);
}
//---------------------------------------------------------------------------
template <class T> inline void xfree(T * lp)
{
  kfree(lp);
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
inline void * operator new(size_t size)
{
  return ksys::kmalloc(size);
}
//---------------------------------------------------------------------------
inline void * operator new[](size_t size)
{
  return ksys::kmalloc(size);
}
//---------------------------------------------------------------------------
#ifdef __BCPLUSPLUS__
//---------------------------------------------------------------------------
inline void * operator new(size_t size, void * p)
{
  return p;
}
//---------------------------------------------------------------------------
inline void * operator new[](size_t size, void * p)
{
  return p;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
inline void operator delete(void * ptr)
{
  ksys::kfree(ptr);
}
//---------------------------------------------------------------------------
inline void operator delete[](void * ptr)
{
  ksys::kfree(ptr);
}
//---------------------------------------------------------------------------
template <typename T> inline T * newObject()
{
  ksys::AutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  new (safe) T;
  return (T *) safe.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1
> inline
T * newObjectV(Param1 & p1)
{
  ksys::AutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  new (safe) T(p1);
  return (T *) safe.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1
> inline
T * newObject(const Param1 & p1)
{
  ksys::AutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  new (safe) T(p1);
  return (T *) safe.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObjectV(Param1 & p1,Param2 & p2)
{
  ksys::AutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  new (safe) T(p1,p2);
  return (T *) safe.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObjectV1(Param1 & p1,const Param2 & p2)
{
  ksys::AutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  new (safe) T(p1,p2);
  return (T *) safe.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObjectV2(const Param1 & p1,Param2 & p2)
{
  ksys::AutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  new (safe) T(p1,p2);
  return (T *) safe.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObject(const Param1 & p1,const Param2 & p2)
{
  ksys::AutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  new (safe) T(p1,p2);
  return (T *) safe.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3
> inline T * newObjectV(Param1 & p1,Param2 & p2,Param3 & p3)
{
  ksys::AutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  new (safe) T(p1,p2,p3);
  return (T *) safe.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3
> inline T * newObject(const Param1 & p1,const Param2 & p2,const Param3 & p3)
{
  ksys::AutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  new (safe) T(p1,p2,p3);
  return (T *) safe.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3
> inline T * newObjectV1(Param1 & p1,const Param2 & p2,const Param3 & p3)
{
  ksys::AutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  new (safe) T(p1,p2,p3);
  return (T *) safe.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3,
  typename Param4
> inline T * newObjectV(Param1 & p1,Param2 & p2,Param3 & p3,Param4 & p4)
{
  ksys::AutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  new (safe) T(p1,p2,p3,p4);
  return (T *) safe.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3,
  typename Param4
> inline T * newObjectV1(Param1 & p1,const Param2 & p2,const Param3 & p3,const Param4 & p4)
{
  ksys::AutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  new (safe) T(p1,p2,p3,p4);
  return (T *) safe.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3,
  typename Param4
> inline T * newObject(const Param1 & p1,const Param2 & p2,const Param3 & p3,const Param4 & p4)
{
  ksys::AutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  new (safe) T(p1,p2,p3,p4);
  return (T *) safe.ptr(NULL);
}
//---------------------------------------------------------------------------
#endif /* _XAlloc_H_ */
//---------------------------------------------------------------------------
