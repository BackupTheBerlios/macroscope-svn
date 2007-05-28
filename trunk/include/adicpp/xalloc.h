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
#ifndef _XAlloc_H_
#define _XAlloc_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
void * kmalloc(size_t size,bool noThrow = false);
void * krealloc(void * p,size_t size,bool noThrow = false);
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
void heapBenchmark();
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
template <typename T> inline void deleteObject(T * object)
{
  if( object != NULL ){
    ksys::ObjectActions::beforeDestruction(object);
    ksys::ObjectActions::beforeDestructor(object);
    object->~T();
    ksys::ObjectActions::afterDestructor(object);
    ksys::kfree(object);
  }
}
//---------------------------------------------------------------------------
template <typename T> inline void deleteObject(const T * object)
{
  if( object != NULL ){
    ksys::ObjectActions::beforeDestruction(const_cast<T *>(object));
    ksys::ObjectActions::beforeDestructor(const_cast<T *>(object));
    const_cast<T *>(object)->~T();
    ksys::ObjectActions::afterDestructor(const_cast<T *>(object));
    ksys::kfree(const_cast<T *>(object));
  }
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T>
class XAutoPtr {
  public:
    ~XAutoPtr() { deleteObject(ptr_); }
    XAutoPtr(T * ptr) : ptr_(ptr) {}

    T * operator ->() const { return ptr_; }
    
    T * ptr() const { return ptr_; }
    T * ptr(T * ptr) const { ksys::xchg(ptr_,ptr); return ptr; }
  protected:
  private:
    mutable T * ptr_;
};
//---------------------------------------------------------------------------
template <typename T> inline T * newObject()
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T;
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1
> inline
T * newObjectV1(Param1 p1)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1
> inline
T * newObjectR1(Param1 & p1)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1
> inline
T * newObjectC1(const Param1 & p1)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObjectV1V2(Param1 p1,Param2 p2)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObjectR1V2(Param1 & p1,Param2 p2)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObjectR1R2(Param1 & p1,Param2 & p2)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObjectV1C2(Param1 p1,const Param2 & p2)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObjectR1C2(Param1 & p1,const Param2 & p2)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObjectC1R2(const Param1 & p1,Param2 & p2)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
#if _MSC_VER
#pragma warning (push)
#pragma warning (disable:4700)
#endif
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObjectC1C2(const Param1 & p1,const Param2 & p2)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
#if _MSC_VER
#pragma warning (pop)
#endif
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3
> inline T * newObjectR1R2R3(Param1 & p1,Param2 & p2,Param3 & p3)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2,p3);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3
> inline T * newObjectC1C2C3(const Param1 & p1,const Param2 & p2,const Param3 & p3)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2,p3);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3
> inline T * newObjectC1V2V3(const Param1 & p1,Param2 p2,Param3 p3)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2,p3);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3
> inline T * newObjectR1C2C3(Param1 & p1,const Param2 & p2,const Param3 & p3)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2,p3);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3
> inline T * newObjectC1C2R3(const Param1 & p1,const Param2 & p2,Param3 & p3)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2,p3);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3
> inline T * newObjectV1V2V3(Param1 p1,Param2 p2,Param3 p3)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2,p3);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3,
  typename Param4
> inline T * newObjectR1R2R3R4(Param1 & p1,Param2 & p2,Param3 & p3,Param4 & p4)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2,p3,p4);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3,
  typename Param4
> inline T * newObjectV1C2C3C4(Param1 p1,const Param2 & p2,const Param3 & p3,const Param4 & p4)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2,p3,p4);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3,
  typename Param4
> inline T * newObjectR1C2C3C4(Param1 & p1,const Param2 & p2,const Param3 & p3,const Param4 & p4)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2,p3,p4);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3,
  typename Param4
> inline T * newObjectC1C2C3C4(const Param1 & p1,const Param2 & p2,const Param3 & p3,const Param4 & p4)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2,p3,p4);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3,
  typename Param4
> inline T * newObjectC1C2C3V4(const Param1 & p1,const Param2 & p2,const Param3 & p3,Param4 p4)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2,p3,p4);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3,
  typename Param4
> inline T * newObjectV1V2V3V4(Param1 p1,Param2 p2,Param3 p3,Param4 p4)
{
  XAutoPtr<uint8_t> safe((uint8_t *) ksys::kmalloc(sizeof(T)));
  const T st;
#if __GNUG__
  if( sizeof(T) >= sizeof(void *) * 2 ) memcpy(safe.ptr(),&st,sizeof(void *) * 2);
#else
  memcpy(safe.ptr(),&st,sizeof(T));
#endif
  ksys::ObjectActions::beforeConstructor((T *) safe.ptr());
  new (safe.ptr()) T(p1,p2,p3,p4);
  XAutoPtr<T> safe2((T *) safe.ptr(NULL));
  ksys::ObjectActions::afterConstructor(safe2.ptr());
  ksys::ObjectActions::afterConstruction(safe2.ptr());
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
#endif /* _XAlloc_H_ */
//---------------------------------------------------------------------------
