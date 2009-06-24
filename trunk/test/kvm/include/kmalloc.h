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
#ifndef kmallocH
#define kmallocH
//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option push -w-inl
#endif
//---------------------------------------------------------------------------
namespace kvm {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
void * kmalloc(size_t size,bool noThrow = false);
void * kmrealloc(void * p,size_t size,bool noThrow = false);
void kmfree(void * p);
//---------------------------------------------------------------------------
} // namespace kvm
//---------------------------------------------------------------------------
inline void * operator new(size_t size)
{
  return kvm::kmalloc(size);
}
//---------------------------------------------------------------------------
inline void * operator new[](size_t size)
{
  return kvm::kmalloc(size);
}
//---------------------------------------------------------------------------
inline void operator delete(void * ptr)
{
  kvm::kmfree(ptr);
}
//---------------------------------------------------------------------------
inline void operator delete[](void * ptr)
{
  kvm::kmfree(ptr);
}
//---------------------------------------------------------------------------
namespace kvm {
//---------------------------------------------------------------------------
template <typename T> inline void deleteObject(T * object)
{
  if( object != NULL ){
    kvm::Object * obj = dynamic_cast<kvm::Object *>(object);
    if( obj != NULL ){
      obj->beforeDestruction();
    }
    delete object;
  }
}
//---------------------------------------------------------------------------
template <typename T> inline void deleteObject(const T * object)
{
  deleteObject(const_cast<T *>(object));
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AutoPtrClassDestructor {
  public:
    template <typename T> static void destroyObject(T * object)
    {
      deleteObject(object);
    }

    template <typename T> static void destroyObject(const T * object)
    {
      deleteObject(object);
    }

    template <typename T> static kvm::Object * dynamicCastToObject(T * object)
    {
      return dynamic_cast<kvm::Object *>(object);
    }
};
//---------------------------------------------------------------------------
class AutoPtrNonVirtualClassDestructor {
  public:
    template <typename T> static void destroyObject(T * object)
    {
      if( object != NULL ){
        object->~T();
        kvm::kmfree(object);
      }
    }

    template <typename T> static void destroyObject(const T * object)
    {
      if( object != NULL ){
        const_cast<T *>(object)->~T();
        kvm::kmfree(const_cast<T *>(object));
      }
    }

    template <typename T> static kvm::Object * dynamicCastToObject(T *)
    {
      return reinterpret_cast<kvm::Object *>(NULL);
    }
};
//---------------------------------------------------------------------------
class AutoPtrMemoryDestructor {
  public:
    static void destroyObject(void * object)
    {
      kvm::kmfree(object);
    }

    static void destroyObject(const void * object)
    {
      kvm::kmfree(const_cast<void *>(object));
    }

    template <typename T> static kvm::Object * dynamicCastToObject(T *)
    {
      return reinterpret_cast<kvm::Object *>(NULL);
    }
};
//---------------------------------------------------------------------------
class AutoPtrNullDestructor {
  public:
    static void destroyObject(void *) {}
    static void destroyObject(const void *) {}
    static kvm::Object * dynamicCastToObject(void *)
    {
      return reinterpret_cast<kvm::Object *>(NULL);
    }
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T> class AutoDrop {
  public:
    ~AutoDrop();
    AutoDrop(T & object);
  protected:
  private:
    T * object_;

    AutoDrop(const AutoDrop<T> &);
    void operator = (const AutoDrop<T> &);
};
//---------------------------------------------------------------------------
template <typename T> inline AutoDrop<T>::~AutoDrop()
{
  object_->drop();
}
//---------------------------------------------------------------------------
template <typename T> inline
AutoDrop<T>::AutoDrop(T & object) : object_(&object)
{
  assert( object_ != NULL );
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T,class D = AutoPtrClassDestructor>
class AutoPtr {
  public:
    ~AutoPtr() { D::destroyObject(ptr_); }
    AutoPtr(T * ptr = NULL) : ptr_(ptr) {}

    AutoPtr<T,D> & operator = (T * ptr) {
      D::destroyObject(ptr_);
      ptr_ = ptr;
      return *this;
    }

    AutoPtr<T,D> & reallocT(uintptr_t count){ ptr_ = (T *) kmrealloc(ptr_,count * sizeof(T)); return *this; }

    T * operator -> () const { return ptr_; }

    T & operator [] (intptr_t i) const { return ptr_[i]; }
    T & operator [] (uintptr_t i) const { return ptr_[i]; }

    T * ptr() const { return ptr_; }
    T * ptr(T * ptr) const { kvm::kxchg(ptr_,ptr); return ptr; }
  protected:
  private:
    mutable T * ptr_;
};
//---------------------------------------------------------------------------
typedef AutoPtr<uint8_t,AutoPtrMemoryDestructor> AutoPtrBuffer;
//---------------------------------------------------------------------------
template <typename T> inline T * newObject()
{
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T;
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1
> inline
T * newObjectV1(Param1 p1)
{
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1
> inline
T * newObjectR1(Param1 & p1)
{
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1
> inline
T * newObjectC1(const Param1 & p1)
{
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObjectV1V2(Param1 p1,Param2 p2)
{
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename D
> inline T * newObjectV1V2(Param1 p1,Param2 p2)
{
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2);
  AutoPtr<T,D> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = D::dynamicCastToObject(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObjectR1V2(Param1 & p1,Param2 p2)
{
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObjectR1R2(Param1 & p1,Param2 & p2)
{
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObjectV1C2(Param1 p1,const Param2 & p2)
{
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObjectR1C2(Param1 & p1,const Param2 & p2)
{
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2
> inline T * newObjectC1R2(const Param1 & p1,Param2 & p2)
{
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
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
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
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
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2,p3);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
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
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2,p3);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
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
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2,p3);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
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
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2,p3);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
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
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2,p3);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
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
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2,p3);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  typename Param1,
  typename Param2,
  typename Param3
> inline T * newObjectV1C2V3(Param1 p1,const Param2 & p2,Param3 p3)
{
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2,p3);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
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
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2,p3,p4);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
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
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2,p3,p4);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
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
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2,p3,p4);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
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
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2,p3,p4);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
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
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2,p3,p4);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
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
  AutoPtrBuffer safe((uint8_t *) kvm::kmalloc(sizeof(T)));
  new (safe.ptr()) T(p1,p2,p3,p4);
  AutoPtr<T> safe2((T *) safe.ptr(NULL));
  kvm::Object * object = dynamic_cast<kvm::Object *>(safe2.ptr());
  if( object != NULL ) object->afterConstruction();
  return safe2.ptr(NULL);
}
//---------------------------------------------------------------------------
} // namespace kvm
//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option pop
#endif
//---------------------------------------------------------------------------
#endif /* kmallocH */
//---------------------------------------------------------------------------
