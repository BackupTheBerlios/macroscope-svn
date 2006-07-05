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
#ifndef _list_H
#define _list_H
//-----------------------------------------------------------------------------
namespace ksys {
//-----------------------------------------------------------------------------
template< class T> class ListNode {
  public:
    ListNode(ListNode< T> * prev = NULL, ListNode< T> * next = NULL);
    ListNode(const T & object, ListNode< T> * prev = NULL, ListNode< T> * next = NULL);

    ListNode< T> *& prev() const;
    ListNode< T> *& next() const;
    T & object() const;
  protected:
  private:
    mutable T               object_;
    mutable ListNode< T> *  prev_;
    mutable ListNode< T> *  next_;
};
//-----------------------------------------------------------------------------
template< class T> inline ListNode< T>::ListNode(ListNode< T> * prev, ListNode< T> * next)
  : prev_(prev), next_(next)
{
}
//-----------------------------------------------------------------------------
template< class T> inline ListNode< T>::ListNode(const T & object, ListNode< T> * prev, ListNode< T> * next)
  : object_(object), prev_(prev), next_(next)
{
}
//-----------------------------------------------------------------------------
template< class T> inline
ListNode< T> *& ListNode< T>::prev() const
{
  return prev_;
}
//-----------------------------------------------------------------------------
template< class T> inline
ListNode< T> *& ListNode< T>::next() const
{
  return next_;
}
//-----------------------------------------------------------------------------
template< class T> inline
T & ListNode< T>::object() const
{
  return object_;
}
//-----------------------------------------------------------------------------
template< class T> class List {
  public:
    ~List();
    List();

    List< T> &              replace(List< T> & s);
    List< T> &              assign(const List< T> & s);

    List< T> &              remove(ListNode< T> * node);
    ListNode< T> *          extract(ListNode< T> * node);
    List< T> &              clear();

    List< T> &              injectToHead(ListNode< T> * node);
    List< T> &              injectToTail(ListNode< T> * node);

    ListNode< T> &          insToHead();
    ListNode< T> &          insToHead(const T & object);
    List< T> &              insToHead(const List< T> & s);
    ListNode< T> &          insToTail();
    ListNode< T> &          insToTail(const T & object);
    List< T> &              insToTail(const List< T> & s);
    ListNode< T> &          insBefore(ListNode< T> * node);
    ListNode< T> &          insBefore(ListNode< T> * node, const T & object);
    ListNode< T> &          insAfter(ListNode< T> * node);
    ListNode< T> &          insAfter(ListNode< T> * node, const T & object);

    ListNode< T> * const &  first() const;
    ListNode< T> * const &  last() const;
    const uintptr_t &       count() const;
  protected:
    ListNode< T> *  first_;
    ListNode< T> *  last_;
    uintptr_t       count_;

    List< T> &              destroy();
    List< T> &              init();
    ListNode< T> &          insTo(ListNode< T> * node);
  private:
};
//-----------------------------------------------------------------------------
template< class T> inline
ListNode< T> * const & List< T>::first() const
{
  return first_;
}
//-----------------------------------------------------------------------------
template< class T> inline
ListNode< T> * const & List< T>::last() const
{
  return last_;
}
//-----------------------------------------------------------------------------
template< class T> inline
const uintptr_t & List< T>::count() const
{
  return count_;
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
List< T> & List< T>::assign(const List< T> & s)
{
  clear();
  for( ListNode< T> * p = s.first_; p != NULL; p = p->next() )
    insToTail(p->object());
  return *this;
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
List< T> & List< T>::insToHead(const List< T> & s)
{
  for( ListNode< T> * p = s.last_; p != NULL; p = p->prev() )
    insToHead(p->object());
  return *this;
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
List< T> & List< T>::insToTail(const List< T> & s)
{
  for( ListNode< T> * p = s.first_; p != NULL; p = p->next() )
    insToTail(p->object());
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline List< T>::~List()
{
  destroy();
}
//-----------------------------------------------------------------------------
template< class T> inline List< T>::List()
{
  init();
}
//-----------------------------------------------------------------------------
template< class T> inline
List< T> & List< T>::replace(List< T> & s)
{
  first_ = s.first_;
  last_ = s.last_;
  count_ = s.count_;
  return s.init();
}
//-----------------------------------------------------------------------------
template< class T> inline
List< T> & List< T>::clear()
{
  return destroy().init();
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
List< T> & List< T>::destroy()
{
  ListNode< T> *  node  = last_, * p;
  while( node != NULL ){
    p = node->prev();
    delete node;
    node = p;
  }
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
List< T> & List< T>::init()
{
  first_ = last_ = NULL;
  count_ = 0;
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
ListNode< T> & List< T>::insTo(ListNode< T> * node)
{
  if( node->prev() == NULL )
    first_ = node;
  else
    node->prev()->next() = node;
  if( node->next() == NULL )
    last_ = node;
  else
    node->next()->prev() = node;
  count_++;
  return *node;
}
//-----------------------------------------------------------------------------
template< class T> inline
ListNode< T> & List< T>::insToHead()
{
  return insTo(new ListNode< T>(NULL, first_));
}
//-----------------------------------------------------------------------------
template< class T> inline
ListNode< T> & List< T>::insToHead(const T & object)
{
  return insTo(new ListNode< T>(object, NULL, first_));
}
//-----------------------------------------------------------------------------
template< class T> inline
ListNode< T> & List< T>::insToTail()
{
  return insTo(new ListNode< T>(last_, NULL));
}
//-----------------------------------------------------------------------------
template< class T> inline
ListNode< T> & List< T>::insToTail(const T & object)
{
  return insTo(new ListNode< T>(object, last_, NULL));
}
//-----------------------------------------------------------------------------
template< class T> inline
ListNode< T> & List< T>::insBefore(ListNode< T> * node)
{
  return insTo(new ListNode< T>(node != NULL ? node->prev() : NULL, node));
}
//-----------------------------------------------------------------------------
template< class T> inline
ListNode< T> & List< T>::insBefore(ListNode< T> * node, const T & object)
{
  return insTo(new ListNode< T>(object, node != NULL ? node->prev() : NULL, node));
}
//-----------------------------------------------------------------------------
template< class T> inline
ListNode< T> & List< T>::insAfter(ListNode< T> * node)
{
  return insTo(new ListNode< T>(node, node != NULL ? node->next() : NULL));
}
//-----------------------------------------------------------------------------
template< class T> inline
ListNode< T> & List< T>::insAfter(ListNode< T> * node, const T & object)
{
  return insTo(new ListNode< T>(object, node, node != NULL ? node->next() : NULL));
}
//-----------------------------------------------------------------------------
template< class T> inline
List< T> & List< T>::injectToHead(ListNode< T> * node)
{
  node->prev() = NULL;
  node->next() = first_;
  insTo(node);
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
List< T> & List< T>::injectToTail(ListNode< T> * node)
{
  node->prev() = last_;
  node->next() = NULL;
  insTo(node);
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
ListNode< T> * List< T>::extract(ListNode< T> * node)
{
  if( node->prev() != NULL ){
    node->prev()->next() = node->next();
  }
  else{
    first_ = node->next();
  }
  if( node->next() != NULL ){
    node->next()->prev() = node->prev();
  }
  else{
    last_ = node->prev();
  }
  count_--;
  return node;
}
//-----------------------------------------------------------------------------
template< class T> inline
List< T> & List< T>::remove(ListNode< T> * node)
{
  extract(node);
  delete node;
  return *this;
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template< class T> class ObjectListNode {
  public:
                          ~ObjectListNode();
                          ObjectListNode(T * object = NULL, ObjectListNode< T> * prev = NULL, ObjectListNode< T> * next = NULL);

    ObjectListNode< T> *& prev() const;
    ObjectListNode< T> *& next() const;
    T *&                  object() const;
  protected:
  private:
    mutable T *                   object_;
    mutable ObjectListNode< T> *  prev_;
    mutable ObjectListNode< T> *  next_;
};
//-----------------------------------------------------------------------------
template< class T> inline ObjectListNode< T>::~ObjectListNode()
{
  delete object_;
}
//-----------------------------------------------------------------------------
template< class T> inline ObjectListNode< T>::ObjectListNode(T * object, ObjectListNode< T> * prev, ObjectListNode< T> * next)
  : object_(object),
    prev_(prev),
    next_(next)
{
}
//-----------------------------------------------------------------------------
template< class T> inline
ObjectListNode< T> *& ObjectListNode< T>::prev() const
{
  return prev_;
}
//-----------------------------------------------------------------------------
template< class T> inline
ObjectListNode< T> *& ObjectListNode< T>::next() const
{
  return next_;
}
//-----------------------------------------------------------------------------
template< class T> inline
T *& ObjectListNode< T>::object() const
{
  return object_;
}
//-----------------------------------------------------------------------------
template< class T> class ObjectList {
  public:
                                  ~ObjectList();
                                  ObjectList();

    ObjectList< T> &              replace(ObjectList< T> & s);
    ObjectList< T> &              assign(const ObjectList< T> & s);

    ObjectList< T> &              remove(ObjectListNode< T> * node);
    ObjectListNode< T> *          extract(ObjectListNode< T> * node);
    T *                           extractObject(ObjectListNode< T> * node);
    ObjectList< T> &              clear();

    ObjectList< T> &              injectToHead(ObjectListNode< T> * node);
    ObjectList< T> &              injectToTail(ObjectListNode< T> * node);

    ObjectListNode< T> *          insToHead(T * object);
    ObjectListNode< T> *          insToTail(T * object);
    ObjectListNode< T> *          insBefore(ObjectListNode< T> * node, T * object);
    ObjectListNode< T> *          insAfter(ObjectListNode< T> * node, T * object);

    ObjectListNode< T> * const &  first() const;
    ObjectListNode< T> * const &  last() const;
    const uintptr_t &             count() const;
  protected:
    ObjectList< T> &              destroy();
    ObjectList< T> &              init();
    ObjectListNode< T> *          insTo(ObjectListNode< T> * node);
  private:
    ObjectListNode< T> *  first_;
    ObjectListNode< T> *  last_;
    uintptr_t             count_;
};
//-----------------------------------------------------------------------------
template< class T> inline ObjectList< T>::~ObjectList()
{
  destroy();
}
//-----------------------------------------------------------------------------
template< class T> inline ObjectList< T>::ObjectList()
{
  init();
}
//-----------------------------------------------------------------------------
template< class T> inline
ObjectListNode< T> * const & ObjectList< T>::first() const
{
  return first_;
}
//-----------------------------------------------------------------------------
template< class T> inline
ObjectListNode< T> * const & ObjectList< T>::last() const
{
  return last_;
}
//-----------------------------------------------------------------------------
template< class T> inline
const uintptr_t & ObjectList< T>::count() const
{
  return count_;
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
ObjectList< T> & ObjectList< T>::assign(const ObjectList< T> & s)
{
  clear();
  for( ObjectListNode< T> * p = s.first_; p != NULL; p = p->next() )
    insToTail(new T(*p->object_));
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
ObjectList< T> & ObjectList< T>::replace(ObjectList< T> & s)
{
  first_ = s.first_;
  last_ = s.last_;
  count_ = s.count_;
  s.init();
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
ObjectList< T> & ObjectList< T>::clear()
{
  return destroy().init();
}
//-----------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
ObjectList< T> & ObjectList< T>::destroy()
{
  ObjectListNode< T> *  node  = last_, * p;
  while( node != NULL ){
    p = node->prev();
    delete node;
    node = p;
  }
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
ObjectList< T> & ObjectList< T>::init()
{
  first_ = last_ = NULL;
  count_ = 0;
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
ObjectListNode< T> * ObjectList< T>::insTo(ObjectListNode< T> * node)
{
  if( node->prev() == NULL )
    first_ = node;
  else
    node->prev()->next() = node;
  if( node->next() == NULL )
    last_ = node;
  else
    node->next()->prev() = node;
  count_++;
  return node;
}
//-----------------------------------------------------------------------------
template< class T> inline
ObjectListNode< T> * ObjectList< T>::insToHead(T * object)
{
  return insTo(new ObjectListNode< T>(object, NULL, first_));
}
//-----------------------------------------------------------------------------
template< class T> inline
ObjectListNode< T> * ObjectList< T>::insToTail(T * object)
{
  return insTo(new ObjectListNode< T>(object, last_, NULL));
}
//-----------------------------------------------------------------------------
template< class T> inline
ObjectListNode< T> * ObjectList< T>::insBefore(ObjectListNode< T> * node, T * object)
{
  return insTo(new ObjectListNode< T>(object, node != NULL ? node->prev() : NULL, node));
}
//-----------------------------------------------------------------------------
template< class T> inline
ObjectListNode< T> * ObjectList< T>::insAfter(ObjectListNode< T> * node, T * object)
{
  return insTo(new ObjectListNode< T>(object, node, node != NULL ? node->next() : NULL));
}
//-----------------------------------------------------------------------------
template< class T> inline
ObjectList< T> & ObjectList< T>::injectToHead(ObjectListNode< T> * node)
{
  node->prev_ = NULL;
  node->next_ = first_;
  insTo(node);
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
ObjectList< T> & ObjectList< T>::injectToTail(ObjectListNode< T> * node)
{
  node->prev_ = last_;
  node->next_ = NULL;
  insTo(node);
  return *this;
}
//-----------------------------------------------------------------------------
template< class T> inline
ObjectListNode< T> * ObjectList< T>::extract(ObjectListNode< T> * node)
{
  if( node->prev() != NULL ){
    node->prev()->next() = node->next();
  }
  else{
    first_ = node->next();
  }
  if( node->next() != NULL ){
    node->next()->prev() = node->prev();
  }
  else{
    last_ = node->prev();
  }
  count_--;
  return node;
}
//-----------------------------------------------------------------------------
template< class T> inline
T * ObjectList< T>::extractObject(ObjectListNode< T> * node)
{
  extract(node);
  T * object  = node->object();
  node->object() = NULL;
  delete node;
  return object;
}
//-----------------------------------------------------------------------------
template< class T> inline
ObjectList< T> & ObjectList< T>::remove(ObjectListNode< T> * node)
{
  extract(node);
  delete node;
  return *this;
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template<typename T> class EmbeddedListNode {
  public:
    ~EmbeddedListNode();
    EmbeddedListNode();

    EmbeddedListNode<T> *& prev() const;
    EmbeddedListNode<T> *& next() const;

    T & object(const EmbeddedListNode< T> & node) const;
  protected:
  private:
    mutable EmbeddedListNode<T> *  prev_;
    mutable EmbeddedListNode<T> *  next_;

    EmbeddedListNode(EmbeddedListNode< T> &){}
    void operator =(EmbeddedListNode< T> &){}
};
//-----------------------------------------------------------------------------
template< typename T> inline EmbeddedListNode< T>::~EmbeddedListNode()
{
  assert(prev_ == NULL && next_ == NULL);
}
//-----------------------------------------------------------------------------
template< typename T> inline EmbeddedListNode< T>::EmbeddedListNode()
  : prev_(NULL), next_(NULL)
{
}
//-----------------------------------------------------------------------------
template< typename T> inline
EmbeddedListNode< T> *& EmbeddedListNode< T>::prev() const
{
  return prev_;
}
//-----------------------------------------------------------------------------
template< typename T> inline
EmbeddedListNode< T> *& EmbeddedListNode< T>::next() const
{
  return next_;
}
//-----------------------------------------------------------------------------
template< typename T> inline
T & EmbeddedListNode<T>::object(const EmbeddedListNode<T> & node) const
{
  return *(T *) const_cast<uint8_t *>(
    (const uint8_t *) this - (uintptr_t) const_cast<EmbeddedListNode<T> *>(&node)
  );
}
//-----------------------------------------------------------------------------
template<
  typename T,
  EmbeddedListNode< T> & (*N) (const T &),
  T & (*O) (const EmbeddedListNode< T> &, T *)
>
class EmbeddedList {
  public:
    ~EmbeddedList();
    EmbeddedList();

    EmbeddedList< T,N,O> &          replace(EmbeddedList< T,N,O> & s);

    T & remove(const T & object);
    T & remove(const EmbeddedListNode< T> & node);
    EmbeddedList< T,N,O> &          clear();

    EmbeddedList< T,N,O> &          insToHead(const T & object);
    EmbeddedList< T,N,O> &          insToTail(const T & object);
    EmbeddedList< T,N,O> &          insBefore(const T & pos, const T & object);
    EmbeddedList< T,N,O> &          insAfter(const T & pos, const T & object);

    EmbeddedListNode< T> * const &  first() const;
    EmbeddedListNode< T> * const &  last() const;
    const uintptr_t &               count() const;

    bool                            nodeInserted(const EmbeddedListNode< T> & node) const;
    bool                            nodeInserted(const T & object) const;
  protected:
    EmbeddedList< T,N,O> &          insTo(EmbeddedListNode< T> * prev, EmbeddedListNode< T> * next, const T & object);
  private:
    EmbeddedListNode< T> *  first_;
    EmbeddedListNode< T> *  last_;
    uintptr_t               count_;
    EmbeddedList< T,N,O>(const EmbeddedList< T,N,O> &){}
    void operator =(const EmbeddedList< T,N,O> &){}
};
//-----------------------------------------------------------------------------
template< typename T,EmbeddedListNode< T> & (*N) (const T &),
          T & (*O) (const EmbeddedListNode< T> &, T *) > inline EmbeddedList< T,N,O>::~EmbeddedList()
{
}
//-----------------------------------------------------------------------------
template< typename T,EmbeddedListNode< T> & (*N) (const T &),
          T & (*O) (const EmbeddedListNode< T> &, T *) > inline EmbeddedList< T,N,O>::EmbeddedList()
{
  clear();
}
//-----------------------------------------------------------------------------
template< typename T,EmbeddedListNode< T> & (*N) (const T &),
          T & (*O) (const EmbeddedListNode< T> &, T *) > inline
EmbeddedListNode< T> * const & EmbeddedList< T,N,O>::first() const
{
  return first_;
}
//-----------------------------------------------------------------------------
template< typename T,EmbeddedListNode< T> & (*N) (const T &),
          T & (*O) (const EmbeddedListNode< T> &, T *) > inline
EmbeddedListNode< T> * const & EmbeddedList< T,N,O>::last() const
{
  return last_;
}
//-----------------------------------------------------------------------------
template< typename T,EmbeddedListNode< T> & (*N) (const T &),
          T & (*O) (const EmbeddedListNode< T> &, T *) > inline
const uintptr_t & EmbeddedList< T,N,O>::count() const
{
  return count_;
}
//-----------------------------------------------------------------------------
template< typename T,EmbeddedListNode< T> & (*N) (const T &),
          T & (*O) (const EmbeddedListNode< T> &, T *) > inline
EmbeddedList< T,N,O> & EmbeddedList< T,N,O>::replace(EmbeddedList< T,N,O> & s)
{
  first_ = s.first_;
  last_ = s.last_;
  count_ = s.count_;
  s.clear();
  return *this;
}
//-----------------------------------------------------------------------------
template< typename T,EmbeddedListNode< T> & (*N) (const T &),
          T & (*O) (const EmbeddedListNode< T> &, T *) > inline
EmbeddedList< T,N,O> & EmbeddedList< T,N,O>::clear()
{
  first_ = last_ = NULL;
  count_ = 0;
  return *this;
}
//-----------------------------------------------------------------------------
template< typename T,EmbeddedListNode< T> & (*N) (const T &),
          T & (*O) (const EmbeddedListNode< T> &, T *) > inline
EmbeddedList< T,N,O> & EmbeddedList< T,N,O>::insTo(EmbeddedListNode< T> * prev, EmbeddedListNode< T> * next, const T & object)
{
  assert( N(object).prev() == NULL && N(object).next() == NULL );
  N(object).prev() = prev;
  N(object).next() = next;
  if( N(object).prev() == NULL ){
    first_ = &N(object);
  }
  else{
    N(object).prev()->next() = &N(object);
  }
  if( N(object).next() == NULL ){
    last_ = &N(object);
  }
  else{
    N(object).next()->prev() = &N(object);
  }
  count_++;
  return *this;
}
//-----------------------------------------------------------------------------
template< typename T,EmbeddedListNode< T> & (*N) (const T &),
          T & (*O) (const EmbeddedListNode< T> &, T *) > inline
EmbeddedList< T,N,O> & EmbeddedList< T,N,O>::insToHead(const T & object)
{
  return insTo(NULL, first_, object);
}
//-----------------------------------------------------------------------------
template< typename T,EmbeddedListNode< T> & (*N) (const T &),
          T & (*O) (const EmbeddedListNode< T> &, T *) > inline
EmbeddedList< T,N,O> & EmbeddedList< T,N,O>::insToTail(const T & object)
{
  return insTo(last_, NULL, object);
}
//-----------------------------------------------------------------------------
template< typename T,EmbeddedListNode< T> & (*N) (const T &),
          T & (*O) (const EmbeddedListNode< T> &, T *) > inline
EmbeddedList< T,N,O> & EmbeddedList< T,N,O>::insBefore(const T & pos, const T & object)
{
  return insTo(N(pos).prev(), &pos, object);
}
//-----------------------------------------------------------------------------
template< typename T,EmbeddedListNode< T> & (*N) (const T &),
          T & (*O) (const EmbeddedListNode< T> &, T *) > inline
EmbeddedList< T,N,O> & EmbeddedList< T,N,O>::insAfter(const T & pos, const T & object)
{
  return insTo(&pos, N(pos).next(), object);
}
//-----------------------------------------------------------------------------
template<
  typename T,
  EmbeddedListNode< T> & (*N) (const T &),
  T & (*O) (const EmbeddedListNode< T> &, T *)
> inline T & EmbeddedList<T,N,O>::remove(const T & object)
{
  if( N(object).prev() != NULL ){
    N(object).prev()->next() = N(object).next();
  }
  else{
    first_ = N(object).next();
  }
  if( N(object).next() != NULL ){
    N(object).next()->prev() = N(object).prev();
  }
  else{
    last_ = N(object).prev();
  }
  N(object).prev() = NULL;
  N(object).next() = NULL;
  count_--;
  return *const_cast<T *>(&object);
}
//-----------------------------------------------------------------------------
template<
  typename T,
  EmbeddedListNode< T> & (*N) (const T &),
  T & (*O) (const EmbeddedListNode< T> &,
  T *)
> inline T & EmbeddedList<T,N,O>::remove(const EmbeddedListNode< T> & node)
{
  return remove(O(node, NULL));
}
//-----------------------------------------------------------------------------
template< typename T,EmbeddedListNode< T> & (*N) (const T &),
          T & (*O) (const EmbeddedListNode< T> &, T *) > inline
bool EmbeddedList< T,N,O>::nodeInserted(const EmbeddedListNode< T> & node) const
{
  return node.prev() != NULL || node.next() != NULL || first_ == &node || last_ == &node;
}
//-----------------------------------------------------------------------------
template< typename T,EmbeddedListNode< T> & (*N) (const T &),
          T & (*O) (const EmbeddedListNode< T> &, T *) > inline
bool EmbeddedList< T,N,O>::nodeInserted(const T & object) const
{
  return nodeInserted(N(object));
}
//-----------------------------------------------------------------------------
} // namespace ksys
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
