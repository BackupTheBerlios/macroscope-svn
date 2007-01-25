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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T> class ObjectT {
  public:
    virtual ~ObjectT();
    ObjectT();
    
// actions
    virtual void oaAfterConstruction() {}
    virtual void oaBeforeDestruction() {}
    
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
    static void afterConstruction(void *) {}
    static void beforeDestruction(void *) {}
    static void afterConstruction(ObjectT<T> * object);
    static void beforeDestruction(ObjectT<T> * object);
};
//---------------------------------------------------------------------------
template <typename T> inline
void ObjectActionsT<T>::afterConstruction(ObjectT<T> * object)
{
  object->oaAfterConstruction();
}
//---------------------------------------------------------------------------
template <typename T> inline
void ObjectActionsT<T>::beforeDestruction(ObjectT<T> * object)
{
  object->oaBeforeDestruction();
}
//---------------------------------------------------------------------------
#define KsysObjectActions ksys::ObjectActionsT<utf8::String>
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
