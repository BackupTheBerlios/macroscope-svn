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
namespace utf8 {
class String;
}
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
void initialize(int,char **);
void cleanup();
class ObjectFabric;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Object {
  friend void initialize(int,char **);
  friend void cleanup();
  friend class ObjectActions;
  public:
    virtual ~Object() {}
    Object() {}
    
// actions
    virtual void afterConstruction() {}
    virtual void beforeDestruction() {}
    
// methods
    virtual Object * createObject() { return NULL; }
    virtual Object * cloneObject() { return NULL; }
    virtual utf8::String className() const;
    virtual utf8::String classId() const;
    virtual utf8::String objectName() const;
    virtual utf8::String objectId() const;

    static ObjectFabric & fabric();
  protected:
    Object(const Object &) {}
    Object & operator = (const Object &) { return *this; }
    
    struct {
      uint8_t heap_ : 1; // 1 if memory for object allocated in heap instead of stack or BSS
      uint8_t template_ : 1; // 1 if object instance placed in objectFabric
      uint8_t clone_ : 1; // 1 if object copied via cloneObject() function
      uint8_t constructor_ : 1; // 1 if construction of object in progress
      uint8_t destructor_ : 1; // 1 if destruction of object in progress
      uint8_t constructed_ : 1; // 1 if construction of object is done
      uint8_t destructed_ : 1; // 1 if destruction of object is done
      uint8_t afterConstruction_ : 1; // 1 if afterConstruction method in progress
      uint8_t beforeDestruction_ : 1; // 1 if beforeDestruction method in progress
    } object_;
    static uint8_t fabric_[];
  private:
    static void initialize();
    static void cleanup();
};
//---------------------------------------------------------------------------
inline ObjectFabric & Object::fabric()
{
  return *reinterpret_cast<ObjectFabric *>(fabric_);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ObjectActions {
  public:
    static void beforeConstructor(void *) {}
    static void afterConstructor(void *) {}
    static void beforeDestructor(void *) {}
    static void afterDestructor(void *) {}
    static void afterConstruction(void *) {}
    static void beforeDestruction(void *) {}

    static void beforeConstructor(Object * object);
    static void afterConstructor(Object * object);
    static void beforeDestructor(Object * object);
    static void afterDestructor(Object * object);
    static void afterConstruction(Object * object);
    static void beforeDestruction(Object * object);
};
//---------------------------------------------------------------------------
inline void ObjectActions::beforeConstructor(Object * object)
{
  memset(&object->object_,0,sizeof(object->object_));
  object->object_.heap_ = true;
  object->object_.constructor_ = true;
}
//---------------------------------------------------------------------------
inline void ObjectActions::afterConstructor(Object * object)
{
  object->object_.constructor_ = false;
  object->object_.constructed_ = true;
}
//---------------------------------------------------------------------------
inline void ObjectActions::beforeDestructor(Object * object)
{
  object->object_.destructor_ = true;
}
//---------------------------------------------------------------------------
inline void ObjectActions::afterDestructor(Object * object)
{
  object->object_.destructor_ = false;
  object->object_.destructed_ = true;
}
//---------------------------------------------------------------------------
inline void ObjectActions::afterConstruction(Object * object)
{
  object->object_.afterConstruction_ = true;
  object->afterConstruction();
  object->object_.afterConstruction_ = false;
}
//---------------------------------------------------------------------------
inline void ObjectActions::beforeDestruction(Object * object)
{
  object->object_.beforeDestruction_ = true;
  object->beforeDestruction();
  object->object_.beforeDestruction_ = false;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ObjectFabric : public Object {
  public:
    virtual ~ObjectFabric() {}
    ObjectFabric() {}
    
// methods
    utf8::String className() const;
    utf8::String classId() const;

    void registerClass(Object * object);
    void unregisterClass(const utf8::String & className);
  protected:
  private:
    ObjectFabric(const ObjectFabric &);
    void operator = (const ObjectFabric &);
};
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
