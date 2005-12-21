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
#ifndef ehashH
#define ehashH
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T>
class EmbeddedHashNode {
  public:
    ~EmbeddedHashNode();
    EmbeddedHashNode();

    EmbeddedHashNode<T> * & next() const;
    T * & object() const;
  protected:
  private:
    mutable EmbeddedHashNode<T> * next_;
    mutable T * object_;
    EmbeddedHashNode(const EmbeddedHashNode<T> &){}
    EmbeddedHashNode<T> & operator = (const EmbeddedHashNode<T> &){ return *this; }
};
//---------------------------------------------------------------------------
template <typename T> inline
EmbeddedHashNode<T>::~EmbeddedHashNode()
{
}
//---------------------------------------------------------------------------
template <typename T> inline
EmbeddedHashNode<T>::EmbeddedHashNode() : next_(NULL), object_(NULL)
{
}
//---------------------------------------------------------------------------
template <typename T> inline
EmbeddedHashNode<T> * & EmbeddedHashNode<T>::next() const
{
  return next_;
}
//---------------------------------------------------------------------------
template <typename T> inline
T * & EmbeddedHashNode<T>::object() const
{
  return object_;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N)(const T &), // must return node embedded in object
  uintptr_t (*H)(const T &),             // must return computed hash of object key
  bool (*E)(const T &,const T &)         // must return true if objects keys equals
>
class EmbeddedHash {
  public:
    ~EmbeddedHash();
    EmbeddedHash();
    
    EmbeddedHash<T,N,H,E> & clear();
    EmbeddedHash<T,N,H,E> & insert(const T & object);
    const T & remove(const T & object);
    T & search(const T & object) const;
    T * find(const T & object) const;
  protected:
  private:
    AutoPtr<EmbeddedHashNode<T> *> hash_;
    uintptr_t size_;
    uintptr_t count_;
    EmbeddedHashNode<T> ** internalFind(const T & object,bool throwIfExist = false,bool throwIfNotExist = false) const;
    EmbeddedHash<T,N,H,E> & optimize();
    enum OptType { optInc, optDec };
};
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N)(const T &),
  uintptr_t (*H)(const T &),
  bool (*E)(const T &,const T &)
> inline EmbeddedHash<T,N,H,E>::~EmbeddedHash()
{
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N)(const T &),
  uintptr_t (*H)(const T &),
  bool (*E)(const T &,const T &)
> inline EmbeddedHash<T,N,H,E>::EmbeddedHash() : size_(0), count_(0)
{
  clear();
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N)(const T &),
  uintptr_t (*H)(const T &),
  bool (*E)(const T &,const T &)
> inline EmbeddedHash<T,N,H,E> & EmbeddedHash<T,N,H,E>::clear()
{
  hash_.realloc(1);
  hash_[0] = NULL;
  size_ = 1;
  return *this;
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N)(const T &),
  uintptr_t (*H)(const T &),
  bool (*E)(const T &,const T &)
>
#ifndef __BCPLUSPLUS__
inline
#endif
EmbeddedHash<T,N,H,E> & EmbeddedHash<T,N,H,E>::insert(const T & object)
{
  EmbeddedHashNode<T> ** head = internalFind(object,true);
  *head = &N(object);
  N(object).object() = const_cast<T *>(&object);
  count_++;
  return optimize();
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N)(const T &),
  uintptr_t (*H)(const T &),
  bool (*E)(const T &,const T &)
> inline const T & EmbeddedHash<T,N,H,E>::remove(const T & object)
{
  EmbeddedHashNode<T> ** head = internalFind(object,false,true);
  *head = (*head)->next();
  N(object).next() = NULL;
  N(object).object() = NULL;
  count_--;
  optimize();
  return object;
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N)(const T &),
  uintptr_t (*H)(const T &),
  bool (*E)(const T &,const T &)
> inline T & EmbeddedHash<T,N,H,E>::search(const T & object) const
{
  return *internalFind(object,true)->object();
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N)(const T &),
  uintptr_t (*H)(const T &),
  bool (*E)(const T &,const T &)
> inline T * EmbeddedHash<T,N,H,E>::find(const T & object) const
{
  EmbeddedHashNode<T> ** p = internalFind(object);
  return *p == NULL ? NULL : (*p)->object();
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N)(const T &),
  uintptr_t (*H)(const T &),
  bool (*E)(const T &,const T &)
>
EmbeddedHashNode<T> ** EmbeddedHash<T,N,H,E>::internalFind(
  const T & object,bool throwIfExist,bool throwIfNotExist) const
{
  EmbeddedHashNode<T> ** head = hash_.ptr() + (H(object) & (size_ - 1));
  while( *head != NULL ){
    if( E(*(*head)->object(),object) ) break;
    head = &(*head)->next();
  }
  int32_t err = 0;
  if( *head != NULL && throwIfExist ){
    err = EEXIST;
  }
  else if( *head == NULL && throwIfNotExist ){
    err = ENOENT;
  }
  if( err != 0 ) throw ExceptionSP(
    new Exception(err,utf8::string(__PRETTY_FUNCTION__))
  );
  return head;
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N)(const T &),
  uintptr_t (*H)(const T &),
  bool (*E)(const T &,const T &)
> EmbeddedHash<T,N,H,E> & EmbeddedHash<T,N,H,E>::optimize()
{
// using golden section == 5/8 as optimization threshold
  EmbeddedHashNode<T> * head = NULL, ** p0, ** p1, * a0, * a1;
  union {
    uintptr_t i;
    OptType o;
  };
  if( (size_ << 4) + ((count_ << 2 + count_) >> 3) > count_ ){
    o = optInc;
  }
  else if( (size_ << 4) - ((count_ << 2 + count_) >> 3) < count_ ){
    o = optDec;
  }
  else {
    goto l1;
  }
  p0 = hash_;
  p1 = p0 + size_;
  while( p0 < p1 ){
    a0 = *p0;
    while( a0 != NULL ){
      a1 = a0->next();
      a0->next() = head;
      head = a0;
      a0 = a1;
    }
    *p0 = NULL;
    p0++;
  }
  try {
    switch( o ){
      case optInc :
        hash_.realloc(size_ << 1);
        for( i = size_, size_ <<= 1; i < size_; i++ ) hash_[i] = NULL;
        break;
      case optDec :
        if( size_ > 1 ){
          hash_.realloc(size_ >> 1);
  	  size_ >>= 1;
        }
        break;
    }
  }
  catch( ExceptionSP & ){} // catch ENOMEM
  while( head != NULL ){
    a0 = head->next();
    p0 = internalFind(*head->object());
    assert( p0 != NULL && *p0 == NULL );
    *p0 = head;
    head->next() = NULL;
    head = a0;
  }
l1:
  return *this;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
