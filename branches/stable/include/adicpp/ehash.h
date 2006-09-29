/*-
 * Copyright 2006 Guram Dukashvili
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
template <typename T> class EmbeddedHashNode {
  public:
    ~EmbeddedHashNode();
    EmbeddedHashNode();

    EmbeddedHashNode<T> * & next() const;
    T & object(const EmbeddedHashNode<T> & node) const;
  protected:
    mutable EmbeddedHashNode<T> * next_;
  private:
    EmbeddedHashNode(const EmbeddedHashNode<T> &){}
    void operator = (const EmbeddedHashNode<T> &){}
};
//---------------------------------------------------------------------------
template <typename T> inline
EmbeddedHashNode<T>::~EmbeddedHashNode()
{
}
//---------------------------------------------------------------------------
template <typename T> inline
EmbeddedHashNode<T>::EmbeddedHashNode() : next_(NULL)
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
T & EmbeddedHashNode<T>::object(const EmbeddedHashNode<T> & node) const
{
  return *(T *) const_cast<uint8_t *>((const uint8_t *) this - (uintptr_t) const_cast<EmbeddedHashNode<T> *>(&node));
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  // must return node embedded in object
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  // must return object of node embedded
  uintptr_t (*H)(const T &),// must return computed hash of object key
  bool (*E) (const T &, const T &)         // must return true if objects keys equals
>
class EmbeddedHash {
  public:
    ~EmbeddedHash();
    EmbeddedHash();
    EmbeddedHash(const EmbeddedHash<T,N,O,H,E> & a);

    EmbeddedHash<T,N,O,H,E> & operator = (const EmbeddedHash<T,N,O,H,E> & a);

    EmbeddedHash<T,N,O,H,E> & clear();
    EmbeddedHash<T,N,O,H,E> & insert(const T & object,bool throwIfExist = true);
    T & remove(const T & object,bool throwIfNotExist = true);
    EmbeddedHash<T,N,O,H,E> & drop();
    EmbeddedHash<T,N,O,H,E> & drop(T & object);
    T & search(const T & object) const;
    T * find(const T & object) const;
    EmbeddedHash<T,N,O,H,E> & list(Array<T *> & l) const;
    const uintptr_t & count() const;

    template <typename ST> ST & put(ST & stream) const
    {
      Array<T *> lst;
      list(lst);
      uint64_t u = lst.count();
      stream << u;
      while( u-- > 0 ) stream << *lst[(uintptr_t) u];
      return stream;
    }
    template <typename ST> ST & get(ST & stream)
    {
      EmbeddedHash<T,N,O,H,E> t;
      uint64_t u;
      stream >> u;
      while( u-- > 0 ){
        AutoPtr<T> p(newObject<T>());
        stream >> *p.ptr();
        t.insert(*p.ptr(NULL));
      }
      drop();
      hash_.ptr(t.hash_.ptr(NULL));
      size_ = t.size_;
      t.size_ = 0;
      count_ = t.count_;
      t.count_ = 0;
      return stream;
    }
  protected:
  private:
    mutable AutoPtr<EmbeddedHashNode<T> *> hash_;
    mutable uintptr_t size_;
    uintptr_t count_;
    EmbeddedHashNode<T> ** internalFind(const T & object,bool throwIfExist = false,bool throwIfNotExist = false) const;
    enum OptType { optInc, optDec };
    EmbeddedHashNode<T> * getChain();
    void putChain(EmbeddedHashNode<T> * head);
    void optimize(OptType o);
};
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t(*H)(const T &),
  bool (*E) (const T &, const T &)
> inline
EmbeddedHash<T,N,O,H,E>::~EmbeddedHash()
{
  clear();
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t (*H)(const T &),
  bool (*E) (const T &, const T &)
> inline
EmbeddedHash<T,N,O,H,E>::EmbeddedHash() : size_(0), count_(0)
{
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t (*H)(const T &),
  bool (*E) (const T &, const T &)
> inline
EmbeddedHash<T,N,O,H,E>::EmbeddedHash(const EmbeddedHash<T,N,O,H,E> & a) : size_(0), count_(0)
{
  operator = (a);
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t (*H)(const T &),
  bool (*E) (const T &, const T &)
>
EmbeddedHash<T,N,O,H,E> & EmbeddedHash<T,N,O,H,E>::operator = (const EmbeddedHash<T,N,O,H,E> & a)
{
  EmbeddedHash<T,N,O,H,E> t;
  EmbeddedHashNode<T> ** head = a.hash_, ** tail = head + a.size_, * walk;
  while( head < tail ){
    walk = *head;
    while( walk != NULL ){
      t.insert(*newObject<T>(O(*walk,NULL)));
      walk = walk->next();
    }
    head++;
  }
  drop();
  hash_.ptr(t.hash_.ptr(NULL));
  size_ = t.size_;
  t.size_ = 0;
  count_ = t.count_;
  t.count_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t (*H)(const T &),
  bool (*E) (const T &, const T &)
> inline
EmbeddedHash<T,N,O,H,E> & EmbeddedHash<T,N,O,H,E>::clear()
{
  if( count_ > 0 ){
    hash_.free();
    size_ = 0;
    count_ = 0;
  }
  return *this;
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t (*H)(const T &),
  bool (*E) (const T &, const T &)
>
#ifndef __BCPLUSPLUS__
inline
#endif
EmbeddedHash<T,N,O,H,E> & EmbeddedHash<T,N,O,H,E>::drop()
{
  EmbeddedHashNode<T> ** head = hash_, ** tail = head + size_, * walk, * w;
  while( head < tail ){
    walk = *head;
    while( walk != NULL ){
      w = walk->next();
      delete &O(*walk,NULL);
      walk = w;
    }
    head++;
  }
  if( size_ > 0 ){
    hash_.free();
    size_ = 0;
  }
  count_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t (*H)(const T &),
  bool (*E) (const T &, const T &)
>
EmbeddedHash<T,N,O,H,E> & EmbeddedHash<T,N,O,H,E>::list(Array<T *> & l) const
{
  l.resize(count_);
  intptr_t i = 0;
  EmbeddedHashNode<T> ** head = hash_.ptr(), ** tail = head + size_, * walk;
  while( head < tail ){
    walk = *head;
    while( walk != NULL ){
      l[i++] = &O(*walk,NULL);
      walk = walk->next();
    }
    head++;
  }
  return *const_cast<EmbeddedHash<T,N,O,H,E> *>(this);
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t (*H)(const T &),
  bool (*E) (const T &, const T &)
>
#ifndef __BCPLUSPLUS__
inline
#endif
EmbeddedHash<T,N,O,H,E> & EmbeddedHash<T,N,O,H,E>::insert(const T & object,bool throwIfExist)
{
  EmbeddedHashNode<T> ** head = internalFind(object,throwIfExist);
  if( head == NULL || *head == NULL ){
    if( size_ == 0 ){
      hash_.realloc(sizeof(EmbeddedHashNode<T> *) * 1);
      hash_[0] = NULL;
      size_ = 1;
      head = &hash_[0];
      *head = &N(object);
    }
    else if( *head == NULL ){
      *head = &N(object);
    }
    N(object).next() = NULL;
    optimize(optInc);
  }
  return *this;
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t (*H)(const T &),
  bool (*E) (const T &, const T &)
> inline
T & EmbeddedHash<T,N,O,H,E>::remove(const T & object,bool throwIfNotExist)
{
  EmbeddedHashNode<T> ** head = internalFind(object,false,throwIfNotExist);
  if( head != NULL && *head != NULL ){
    *head = (*head)->next();
    N(object).next() = NULL;
    optimize(optDec);
  }
  return *const_cast<T *>(&object);
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t (*H)(const T &),
  bool (*E) (const T &, const T &)
> inline
EmbeddedHash<T,N,O,H,E> & EmbeddedHash<T,N,O,H,E>::drop(T & object)
{
  EmbeddedHashNode<T> ** head = internalFind(object,false,true);
  *head = (*head)->next();
  N(object).next() = NULL;
  optimize(optDec);
  delete &object;
  return *this;
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t (*H)(const T &),
  bool (*E) (const T &, const T &)
> inline
T & EmbeddedHash<T,N,O,H,E>::search(const T & object) const
{
  return *internalFind(object,true)->object();
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t (*H)(const T &),
  bool (*E) (const T &, const T &)
> inline
T * EmbeddedHash<T,N,O,H,E>::find(const T & object) const
{
  EmbeddedHashNode<T> ** p = internalFind(object);
  return p == NULL || *p == NULL ? NULL : &O(**p,NULL);
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t(*H)(const T &),
  bool (*E) (const T &, const T &)
>
EmbeddedHashNode<T> ** EmbeddedHash<T,N,O,H,E>::internalFind(const T & object, bool throwIfExist, bool throwIfNotExist) const
{
  EmbeddedHashNode<T> ** head = NULL;
  if( size_ > 0 ){
    head = hash_.ptr() + (H(object) & (size_ - 1));
    while( *head != NULL ){
      if( E(O(**head,NULL), object) ) break;
      head = &(*head)->next();
    }
  }
  int32_t err = 0;
  if( head != NULL && *head != NULL && throwIfExist ){
#if defined(__WIN32__) || defined(__WIN64__)
    err = ERROR_ALREADY_EXISTS;
#else
    err = EEXIST;
#endif
  }
  else if( (head == NULL || *head == NULL) && throwIfNotExist ){
#if defined(__WIN32__) || defined(__WIN64__)
    err = ERROR_NOT_FOUND;
#else
    err = ENOENT;
#endif
  }
  if( err != 0 )
    Exception::throwSP(err + errorOffset, __PRETTY_FUNCTION__);
  return head;
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t(*H)(const T &),
  bool (*E) (const T &, const T &)
>
EmbeddedHashNode<T> * EmbeddedHash<T,N,O,H,E>::getChain()
{
  EmbeddedHashNode<T> * head = NULL, ** p0, ** p1, * a0, * a1;
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
  return head;
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t(*H)(const T &),
  bool (*E) (const T &, const T &)
>
void EmbeddedHash<T,N,O,H,E>::putChain(EmbeddedHashNode<T> * head)
{
  EmbeddedHashNode<T> ** p0, * a0;
  while( head != NULL ){
    a0 = head->next();
    p0 = internalFind(O(*head,NULL));
    assert( p0 != NULL && *p0 == NULL );
    *p0 = head;
    head->next() = NULL;
    head = a0;
  }
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t(*H)(const T &),
  bool (*E) (const T &, const T &)
>
#ifndef __BCPLUSPLUS__
inline
#endif
void EmbeddedHash<T,N,O,H,E>::optimize(OptType o)
{
  EmbeddedHashNode<T> * head = NULL;
// using golden section == 5/8 as optimization threshold
  uintptr_t count = size_ << 4, c = ((count << 2) + count) >> 3, i;
  switch( o ){
    case optInc :
      if( ++count_ < count + c ) return;
      head = getChain();
      hash_.realloc((i = (size_ << 1) + (size_ == 0)) * sizeof(EmbeddedHashNode<T> *));
      while( size_ < i ){
        hash_[size_] = NULL;
        size_++;
      }
      putChain(head);
      break;
    case optDec :
      if( --count_ > count - c ) return;
      if( size_ > 1 || count_ == 0 ){
        head = getChain();
        hash_.realloc((size_ >> 1) * sizeof(EmbeddedHashNode<T> *));
        size_ >>= 1;
        putChain(head);
      }
      break;
  }
}
//---------------------------------------------------------------------------
template <
  typename T,
  EmbeddedHashNode<T> & (*N) (const T &),
  T & (*O) (const EmbeddedHashNode<T> &, T *),
  uintptr_t(*H)(const T &),
  bool (*E) (const T &, const T &)
> inline
const uintptr_t & EmbeddedHash<T,N,O,H,E>::count() const
{
  return count_;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T> class AutoHashDrop {
  public:
    ~AutoHashDrop();
    AutoHashDrop(T & hash);
  protected:
  private:
    T * hash_;

    AutoHashDrop(const AutoHashDrop<T> &){}
    void operator =(const AutoHashDrop<T> &){}
};
//---------------------------------------------------------------------------
template <typename T> inline AutoHashDrop<T>::~AutoHashDrop()
{
  hash_->drop();
}
//---------------------------------------------------------------------------
template <typename T> inline
AutoHashDrop<T>::AutoHashDrop(T & hash) : hash_(&hash)
{
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
