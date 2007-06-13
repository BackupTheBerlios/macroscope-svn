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
#ifndef _sp_H
#define _sp_H
//-----------------------------------------------------------------------------
namespace ksys {
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template< typename T> class RefCounted {
  protected:
    static T * takeOwnership(T * ptr)
    {
      ptr->addRef();
      return ptr;
    }

    static T * returnOwnership(T * ptr)
    {
      ptr->remRef();
      return ptr;
    }
};
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template< typename T> class NoRefCounted {
  protected:
    static T * takeOwnership(T * ptr)
    {
      return ptr;
    }

    static T * returnOwnership(T * ptr)
    {
      return ptr;
    }
};
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template< typename T> class AssertCheck {
  protected:
    static T * validate(T * ptr)
    {
      assert(ptr != NULL);
      return ptr;
    }
};
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template< typename T> class NoCheck {
  protected:
    static T * validate(T * ptr)
    {
      return ptr;
    }
};
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template< class T> class DirectAccess {
  public:
    ~DirectAccess()
    {
    }

    T * operator ->() const
    {
      return ptr_;
    }

    DirectAccess(T * ptr)
      : ptr_(ptr)
    {
    }
  protected:
  private:
    T * ptr_;
};
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template< typename T> class InterlockedAccess {
  public:
    ~InterlockedAccess()
    {
      ptr_->release();
    }

    T * operator ->() const
    {
      return ptr_;
    }

    InterlockedAccess(T * ptr)
      : ptr_(ptr)
    {
      ptr_->acquire();
    }
  protected:
  private:
    T * ptr_;
};
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template< typename T> class DefaultStorage {
  public:
    T * const & getStorage() const
    {
      return ptr_;
    }

    DefaultStorage< T> & setStorage(T * ptr)
    {
      ptr_ = ptr;
      return *this;
    }

    DefaultStorage< T>(T * ptr)
      : ptr_(ptr)
    {
    }
  protected:
  private:
    T * ptr_;
};
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template< typename T,class OwnershipPolicy = RefCounted< T>,
          class CheckingPolicy = NoCheck< T>,
          class AccessStrategy = DirectAccess< T>,
          class StoragePolicy = DefaultStorage< T> > class SP : public OwnershipPolicy, public CheckingPolicy {
    typedef SP< T,OwnershipPolicy,CheckingPolicy,AccessStrategy,StoragePolicy>  SP_;
  public:
    ~SP()
    {
      returnOwnership(storage_.getStorage());
    }

    SP(T * ptr = NULL)
      : storage_(takeOwnership(validate(ptr)))
    {
    }

    SP_ & operator =(const SP_ & ptr)
    {
      takeOwnership(ptr.storage_.getStorage());
      returnOwnership(storage_.getStorage());
      storage_.setStorage(ptr.storage_.getStorage());
      return *this;
    }

    SP_ & operator =(T * ptr)
    {
      takeOwnership(validate(ptr));
      returnOwnership(storage_.getStorage());
      storage_.setStorage(ptr);
      return *this;
    }

    AccessStrategy operator ->() const
    {
      return AccessStrategy(storage_.getStorage());
    }
  protected:
  private:
    StoragePolicy storage_;
};
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template <class T> class SPRC {
  public:
    ~SPRC();
    SPRC(T * ptr);
    SPRC(const SPRC< T> & ptr);

    T * operator ->() const;
    SPRC< T> & operator =(T * ptr);
    SPRC< T> & operator =(const SPRC< T> & ptr);

    bool operator == (const SPRC<T> & ptr) const;
    bool operator != (const SPRC<T> & ptr) const;

    T * ptr(T * ptr);
    T * const & ptr() const;
  protected:
    T * ptr_;
  private:
};
//-----------------------------------------------------------------------------
template <class T> inline SPRC<T>::SPRC(T * ptr) : ptr_(ptr)
{
  ptr->addRef();
}
//-----------------------------------------------------------------------------
template <class T> inline SPRC<T>::SPRC(const SPRC<T> & ptr) : ptr_(ptr.ptr_)
{
  ptr->addRef();
}
//-----------------------------------------------------------------------------
template <class T> inline SPRC<T>::~SPRC()
{
  ptr_->remRef();
}
//-----------------------------------------------------------------------------
template <class T> inline
T * SPRC<T>::operator -> () const
{
  return ptr_;
}
//-----------------------------------------------------------------------------
template <class T> inline
SPRC<T> & SPRC<T>::operator =(T * ptr)
{
  ptr->addRef();
  ptr_->remRef();
  ptr_ = ptr;
  return *this;
}
//-----------------------------------------------------------------------------
template <class T> inline
SPRC<T> & SPRC<T>::operator =(const SPRC<T> & ptr)
{
  ptr.ptr_->addRef();
  ptr_->remRef();
  ptr_ = ptr.ptr_;
  return *this;
}
//-----------------------------------------------------------------------------
template <class T> inline
bool SPRC<T>::operator ==(const SPRC< T> & ptr) const
{
  return ptr_ == ptr.ptr_;
}
//-----------------------------------------------------------------------------
template <class T> inline
bool SPRC<T>::operator !=(const SPRC< T> & ptr) const
{
  return ptr_ != ptr.ptr_;
}
//-----------------------------------------------------------------------------
template <class T> inline
T * SPRC<T>::ptr(T * ptr)
{
  xchg(ptr_, ptr);
  return ptr;
}
//-----------------------------------------------------------------------------
template <class T> inline
T * const & SPRC<T>::ptr() const
{
  return ptr_;
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template <class T> class SPRCZS {
  public:
    ~SPRCZS();
    SPRCZS(T * ptr);
    SPRCZS(const SPRCZS< T> & ptr);

    T * operator ->() const;
    SPRCZS< T> &  operator =(T * ptr);
    SPRCZS< T> &  operator =(const SPRCZS< T> & ptr);

    bool          operator ==(const SPRCZS< T> & ptr) const;
    bool          operator !=(const SPRCZS< T> & ptr) const;

    T *           ptr(T * ptr);
    T * const &   ptr() const;
  protected:
    T * ptr_;
  private:
};
//-----------------------------------------------------------------------------
template< class T> inline SPRCZS< T>::SPRCZS(T * ptr) : ptr_(ptr)
{
  if( ptr != NULL ) ptr->addRef();
}
//-----------------------------------------------------------------------------
template< class T> inline SPRCZS< T>::SPRCZS(const SPRCZS< T> & ptr) : ptr_(ptr.ptr_)
{
  if( ptr_ != NULL ) ptr->addRef();
}
//-----------------------------------------------------------------------------
template< class T> inline SPRCZS< T>::~SPRCZS()
{
  if( ptr_ != NULL ) ptr_->remRef();
}
//-----------------------------------------------------------------------------
template< class T> inline
T * SPRCZS<T>::operator ->() const
{
  return ptr_;
}
//-----------------------------------------------------------------------------
template< class T> inline
SPRCZS< T> & SPRCZS<T>::operator =(T * ptr)
{
  if( ptr != NULL ) ptr->addRef();
  if( ptr_ != NULL ) ptr_->remRef();
  ptr_ = ptr;
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
SPRCZS<T> & SPRCZS<T>::operator =(const SPRCZS< T> & ptr)
{
  if( ptr.ptr_ != NULL ) ptr.ptr_->addRef();
  if( ptr_ != NULL ) ptr_->remRef();
  ptr_ = ptr.ptr_;
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
bool SPRCZS<T>::operator ==(const SPRCZS< T> & ptr) const
{
  return ptr_ == ptr.ptr_;
}
//-----------------------------------------------------------------------------
template< class T> inline
bool SPRCZS<T>::operator !=(const SPRCZS< T> & ptr) const
{
  return ptr_ != ptr.ptr_;
}
//-----------------------------------------------------------------------------
template< class T> inline
T * SPRCZS< T>::ptr(T * ptr)
{
  xchg(ptr_, ptr);
  return ptr;
}
//-----------------------------------------------------------------------------
template< class T> inline
T * const & SPRCZS< T>::ptr() const
{
  return ptr_;
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template<typename T,typename M> class LockedAction {
  public:
    ~LockedAction();
    LockedAction(T & object,M & mutex);
    LockedAction(const LockedAction<T,M> & a);

    T * operator -> () const;
  protected:
  private:
    void operator = (const LockedAction<T,M> &){}
    T & object_;
    M & mutex_;
};
//-----------------------------------------------------------------------------
template <typename T,typename M> inline LockedAction<T,M>::~LockedAction()
{
  mutex_.release();
}
//-----------------------------------------------------------------------------
template <typename T,typename M> inline
LockedAction<T,M>::LockedAction(T & object,M & mutex) : object_(object), mutex_(mutex)
{
  mutex_.acquire();
}
//-----------------------------------------------------------------------------
template <typename T,typename M> inline
LockedAction<T,M>::LockedAction(const LockedAction<T,M> & a) : object_(a.object_), mutex_(a.mutex_)
{
}
//-----------------------------------------------------------------------------
/*template <typename T,typename M> inline LockedAction<T,M> & operator = (const LockedAction<T,M> & a)
{
  return *this;
}*/
//-----------------------------------------------------------------------------
template <typename T,typename M> inline
T * LockedAction<T,M>::operator -> () const
{
  return &object_;
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template<class T> class SPIA {
  public:
    ~SPIA();
    SPIA(T * ptr);

    SPIA<T> & operator = (const SPIA<T> & ptr);
    LockedAction<T,T> operator -> () const;

    bool operator == (const SPIA< T> & ptr) const;
    bool operator != (const SPIA< T> & ptr) const;

    T * ptr(T * ptr);
    T * const & ptr() const;
  protected:
  private:
    SPIA(const SPIA<T> &){}
    void operator = (T *){}

    T * ptr_;
};
//-----------------------------------------------------------------------------
template<class T> inline SPIA< T>::~SPIA()
{
  deleteObject(ptr_);
}
//-----------------------------------------------------------------------------
template<class T> inline SPIA< T>::SPIA(T * ptr) : ptr_(ptr)
{
}
//-----------------------------------------------------------------------------
template< class T> inline
LockedAction<T,T> SPIA<T>::operator ->() const
{
  return LockedAction<T,T>(*ptr_,*ptr_);
}
//-----------------------------------------------------------------------------
template< class T> inline
bool SPIA<T>::operator ==(const SPIA< T> & ptr) const
{
  return ptr_ == ptr.ptr_;
}
//-----------------------------------------------------------------------------
template< class T> inline
bool SPIA<T>::operator !=(const SPIA< T> & ptr) const
{
  return ptr_ != ptr.ptr_;
}
//-----------------------------------------------------------------------------
template< class T> inline
T * SPIA< T>::ptr(T * ptr)
{
  xchg(ptr_,ptr);
  return ptr;
}
//-----------------------------------------------------------------------------
template< class T> inline
T * const & SPIA< T>::ptr() const
{
  return ptr_;
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template <typename T,typename M> class SPEIA {
  public:
    ~SPEIA();
    SPEIA(T * object,M & mutex);

    SPEIA<T,M> & operator = (T * object);
    LockedAction<T,M> operator -> () const;

    bool operator == (const SPEIA<T,M> & object) const;
    bool operator != (const SPEIA<T,M> & object) const;

    T * ptr(T * object);
    T * const & ptr() const;
  protected:
  private:
    SPEIA(const SPEIA<T,M> &){}
    void operator = (const SPEIA<T,M> &){}

    T * object_;
    M & mutex_;
};
//-----------------------------------------------------------------------------
template <typename T,typename M> inline SPEIA<T,M>::~SPEIA()
{
  deleteObject(object_);
}
//-----------------------------------------------------------------------------
template <typename T,typename M> inline
SPEIA<T,M>::SPEIA(T * object,M & mutex) : object_(object), mutex_(mutex)
{
}
//-----------------------------------------------------------------------------
template <typename T,typename M> inline
SPEIA<T,M> & SPEIA<T,M>::operator = (T * object)
{
  mutex_.acquire();
  deleteObject(object_);
  object_ = object;
  mutex_.release();
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T,typename M> inline
LockedAction<T,M> SPEIA<T,M>::operator -> () const
{
  return LockedAction<T,M>(*object_,mutex_);
}
//-----------------------------------------------------------------------------
template <typename T,typename M> inline
bool SPEIA<T,M>::operator == (const SPEIA<T,M> & object) const
{
  return object_ == object.object_;
}
//-----------------------------------------------------------------------------
template <typename T,typename M> inline
bool SPEIA<T,M>::operator != (const SPEIA<T,M> & object) const
{
  return object_ != object.object_;
}
//-----------------------------------------------------------------------------
template <typename T,typename M> inline
T * SPEIA<T,M>::ptr(T * object)
{
  xchg(object_,object);
  return object;
}
//-----------------------------------------------------------------------------
template <typename T,typename M> inline
T * const & SPEIA<T,M>::ptr() const
{
  return object_;
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template<typename T> class SPIARC : public SPRC<T> {
  public:
    ~SPIARC();
    SPIARC(T * ptr);
    SPIARC(const SPIARC<T> & ptr);

    //SPIARC<T> & operator = (T * ptr);
    //SPIARC<T> & operator = (const SPIARC<T> & ptr);
    LockedAction<T,T> operator ->() const;
  protected:
  private:
};
//-----------------------------------------------------------------------------
template<typename T> inline SPIARC< T>::~SPIARC()
{
}
//-----------------------------------------------------------------------------
template< typename T> inline SPIARC< T>::SPIARC(T * ptr) : SPRC< T>(ptr)
{
}
//-----------------------------------------------------------------------------
template< typename T> inline
SPIARC< T>::SPIARC(const SPIARC< T> & ptr) : SPRC< T>(ptr)
{
}
//-----------------------------------------------------------------------------
/*template <typename T> inline
SPIARC<T> & SPIARC<T>::SPIARC(T * ptr)
{
  SPRC<T>::operator = (ptr);
  return *this;
}
//-----------------------------------------------------------------------------
template <typename T> inline
SPIARC<T> & SPIARC<T>::SPIARC(const SPIARC<T> & ptr)
{
  SPRC<T>::operator = (ptr);
  return *this;
}*/
//-----------------------------------------------------------------------------
template< typename T> inline
LockedAction<T,T> SPIARC<T>::operator -> () const
{
  return LockedAction<T,T>(*SPRC<T>::ptr_,*SPRC<T>::ptr_);
}
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
#endif
