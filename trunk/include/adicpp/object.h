/*-
 * Copyright 2007 Guram Dukashvili
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
#ifndef objectH
#define objectH
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
template <typename T> class ObjectActionsT;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T>
class ObjectT {
  friend class ObjectActionsT<T>;
  public:
    virtual ~ObjectT();
    ObjectT();
    
// actions
    virtual void afterConstruction() {}
    virtual void beforeDestruction() {}
    
// methods
    virtual ObjectT<T> * newObject() { return NULL; }
    virtual ObjectT<T> * cloneObject() { return NULL; }
    virtual T className() const { return "Object"; }
    virtual T classId() const; // see body in sysutils.h
    virtual T objectName() const { return ""; }
    virtual T objectId() const { return ""; }
  protected:
    ObjectT(const ObjectT<T> &);
    ObjectT<T> & operator = (const ObjectT<T> & a);
    
    struct {
      uint8_t heap_ : 1; // 1 if memory for object allocated in heap instead of stack or BSS
      uint8_t constructor_ : 1; // 1 if construction of object in progress
      uint8_t destructor_ : 1; // 1 if destruction of object in progress
      uint8_t constructed_ : 1; // 1 if construction of object is done
      uint8_t destructed_ : 1; // 1 if destruction of object is done
      uint8_t afterConstruction_ : 1; // 1 if afterConstruction method in progress
      uint8_t beforeDestruction_ : 1; // 1 if beforeDestruction method in progress
    };
  private:
};
//---------------------------------------------------------------------------
template <typename T> inline
ObjectT<T>::~ObjectT()
{
}
//---------------------------------------------------------------------------
template <typename T> inline
ObjectT<T>::ObjectT()
{
}
//---------------------------------------------------------------------------
template <typename T> inline
ObjectT<T>::ObjectT(const ObjectT<T> &)
{
}
//---------------------------------------------------------------------------
template <typename T> inline
ObjectT<T> & ObjectT<T>::operator = (const ObjectT<T> &)
{
  return *this;
}
//---------------------------------------------------------------------------
#define KsysObject ksys::ObjectT<utf8::String>
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T>
class ObjectActionsT {
  public:
    static void beforeConstructor(void *) {}
    static void afterConstructor(void *) {}
    static void beforeDestructor(void *) {}
    static void afterDestructor(void *) {}
    static void afterConstruction(void *) {}
    static void beforeDestruction(void *) {}

    static void beforeConstructor(ObjectT<T> * object);
    static void afterConstructor(ObjectT<T> * object);
    static void beforeDestructor(ObjectT<T> * object);
    static void afterDestructor(ObjectT<T> * object);
    static void afterConstruction(ObjectT<T> * object);
    static void beforeDestruction(ObjectT<T> * object);
};
//---------------------------------------------------------------------------
template <typename T> inline
void ObjectActionsT<T>::beforeConstructor(ObjectT<T> * object)
{
  memset(object,0,sizeof(ObjectT<T>));
  object->heap_ = true;
  object->constructor_ = true;
}
//---------------------------------------------------------------------------
template <typename T> inline
void ObjectActionsT<T>::afterConstructor(ObjectT<T> * object)
{
  object->constructor_ = false;
  object->constructed_ = true;
}
//---------------------------------------------------------------------------
template <typename T> inline
void ObjectActionsT<T>::beforeDestructor(ObjectT<T> * object)
{
  object->destructor_ = true;
}
//---------------------------------------------------------------------------
template <typename T> inline
void ObjectActionsT<T>::afterDestructor(ObjectT<T> * object)
{
  object->destructor_ = false;
  object->destructed_ = true;
}
//---------------------------------------------------------------------------
template <typename T> inline
void ObjectActionsT<T>::afterConstruction(ObjectT<T> * object)
{
  object->afterConstruction_ = true;
  object->afterConstruction();
  object->afterConstruction_ = false;
}
//---------------------------------------------------------------------------
template <typename T> inline
void ObjectActionsT<T>::beforeDestruction(ObjectT<T> * object)
{
  object->beforeDestruction_ = true;
  object->beforeDestruction();
  object->beforeDestruction_ = false;
}
//---------------------------------------------------------------------------
#define KsysObjectActions ksys::ObjectActionsT<utf8::String>
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
