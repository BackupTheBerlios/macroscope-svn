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
#ifndef holH
#define holH
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template< typename T> class HashedObjectListKey {
  public:
    ~HashedObjectListKey();
    HashedObjectListKey();
    HashedObjectListKey(const T & key);
    HashedObjectListKey(const HashedObjectListKey< T> & key);

    HashedObjectListKey< T> & operator =(const T & key);
    HashedObjectListKey< T> & operator =(const HashedObjectListKey< T> & key);

    bool hashKeyEqu(const HashedObjectListKey< T> & key, bool) const;
    uintptr_t hash(bool) const;
  protected:
  private:
    T key_;
};
//---------------------------------------------------------------------------
template< typename T> inline HashedObjectListKey< T>::~HashedObjectListKey()
{
}
//---------------------------------------------------------------------------
template< typename T> inline HashedObjectListKey< T>::HashedObjectListKey()
{
}
//---------------------------------------------------------------------------
template< typename T> inline HashedObjectListKey< T>::HashedObjectListKey(const T & key) : key_(key)
{
}
//---------------------------------------------------------------------------
template< typename T> inline HashedObjectListKey< T>::HashedObjectListKey(const HashedObjectListKey< T> & key) : key_(key.key_)
{
}
//---------------------------------------------------------------------------
template< typename T> inline
HashedObjectListKey< T> & HashedObjectListKey<T>::operator =(const T & key)
{
  key_ = key;
  return *this;
}
//---------------------------------------------------------------------------
template< typename T> inline
HashedObjectListKey< T> & HashedObjectListKey<T>::operator =(const HashedObjectListKey< T> & key)
{
  key_ = key.key_;
  return *this;
}
//---------------------------------------------------------------------------
template< typename T> inline
bool HashedObjectListKey< T>::hashKeyEqu(const HashedObjectListKey< T> & key, bool) const
{
  return memcmp(&key_, &key.key_, sizeof(key_)) == 0;
}
//---------------------------------------------------------------------------
template< typename T> inline
uintptr_t HashedObjectListKey< T>::hash(bool) const
{
  return HF::hash((void *) &key_, sizeof(key_), 0);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
enum HashChainType { 
  hctKey, 
  hctObject, 
  hctIndex 
};
//---------------------------------------------------------------------------
template< class TKey,class TObj> class HashedObjectListItem {
  public:
    ~HashedObjectListItem();
    HashedObjectListItem(const TKey & key, TObj * object, uintptr_t index);

    HashedObjectListItem< TKey,TObj> ** next() const;
    TKey &                              key() const;
    HashedObjectListItem< TKey,TObj> *& keyNext() const;
    HashedObjectListItem< TKey,TObj> &  keyNext(HashedObjectListItem< TKey,TObj> * keyNextA);
    HashedObjectListItem< TKey,TObj> *& objectNext() const;
    HashedObjectListItem< TKey,TObj> &  objectNext(HashedObjectListItem< TKey,TObj> * objectNextA);
    HashedObjectListItem< TKey,TObj> *& indexNext() const;
    HashedObjectListItem< TKey,TObj> &  indexNext(HashedObjectListItem< TKey,TObj> * indexNextA);
    HashedObjectListItem< TKey,TObj> &  key(const TKey & keyA);
    TObj *&                             object() const;
    HashedObjectListItem< TKey,TObj> &  object(TObj * objectA);
    const uintptr_t &                   index() const;
    HashedObjectListItem< TKey,TObj> &  index(uintptr_t indexA);
  protected:
  private:
    HashedObjectListItem(const HashedObjectListItem< TKey,TObj> &){}
    void operator =(const HashedObjectListItem< TKey,TObj> &){}
#if _MSC_VER
#pragma warning(push,3)
#endif
    union {
        mutable HashedObjectListItem< TKey,TObj> *  next_[3];
        struct {
            mutable HashedObjectListItem< TKey,TObj> *  keyNext_;
            mutable HashedObjectListItem< TKey,TObj> *  objectNext_;
            mutable HashedObjectListItem< TKey,TObj> *  indexNext_;
        };
    };
#if _MSVC_VER
#pragma warning(pop)
#endif
    mutable TKey      key_;
    mutable TObj *    object_;
    mutable uintptr_t index_;
};
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline HashedObjectListItem< TKey,TObj>::~HashedObjectListItem()
{
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline HashedObjectListItem< TKey,TObj>::HashedObjectListItem(const TKey & key, TObj * object, uintptr_t index)
  : keyNext_(NULL),
    objectNext_(NULL),
    indexNext_(NULL),
    key_(key),
    object_(object),
    index_(index)
{
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectListItem< TKey,TObj> ** HashedObjectListItem< TKey,TObj>::next() const
{
  return next_;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectListItem< TKey,TObj> *& HashedObjectListItem< TKey,TObj>::keyNext() const
{
  return keyNext_;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectListItem< TKey,TObj> & HashedObjectListItem< TKey,TObj>::keyNext(HashedObjectListItem< TKey,TObj> * keyNextA)
{
  keyNext_ = keyNextA;
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectListItem< TKey,TObj> *& HashedObjectListItem< TKey,TObj>::objectNext() const
{
  return objectNext_;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectListItem< TKey,TObj> & HashedObjectListItem< TKey,TObj>::objectNext(HashedObjectListItem< TKey,TObj> * objectNextA)
{
  objectNext_ = objectNextA;
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectListItem< TKey,TObj> *& HashedObjectListItem< TKey,TObj>::indexNext() const
{
  return indexNext_;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectListItem< TKey,TObj> & HashedObjectListItem< TKey,TObj>::indexNext(HashedObjectListItem< TKey,TObj> * indexNextA)
{
  indexNext_ = indexNextA;
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
TKey & HashedObjectListItem< TKey,TObj>::key() const
{
  return key_;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectListItem< TKey,TObj> & HashedObjectListItem< TKey,TObj>::key(const TKey & keyA)
{
  key_ = keyA;
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
TObj *& HashedObjectListItem< TKey,TObj>::object() const
{
  return object_;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectListItem< TKey,TObj> & HashedObjectListItem< TKey,TObj>::object(TObj * objectA)
{
  object_ = objectA;
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
const uintptr_t & HashedObjectListItem< TKey,TObj>::index() const
{
  return index_;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectListItem< TKey,TObj> & HashedObjectListItem< TKey,TObj>::index(uintptr_t indexA)
{
  index_ = indexA;
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template< class TKey,class TObj> class HashedObjectListHash {
  public:
    ~HashedObjectListHash();
    HashedObjectListHash(uintptr_t slotCount = 1);

    HashedObjectListItem< TKey,TObj> **         find(const TKey & key, bool caseSensitive) const;
    HashedObjectListItem< TKey,TObj> **         find(const TObj * object) const;
    HashedObjectListItem< TKey,TObj> **         find(const uintptr_t index) const;

    uintptr_t                                   maxChainLength(HashChainType chain) const;
    uintptr_t                                   minChainLength(HashChainType chain) const;
    uintptr_t                                   avgChainLength(HashChainType chain) const;

    HashedObjectListHash< TKey,TObj> &          resize(uintptr_t slotCount);
    HashedObjectListItem< TKey,TObj> ** const & slots() const;
    HashedObjectListHash< TKey,TObj> &          slots(HashedObjectListItem< TKey,TObj> ** slotsA);
    const uintptr_t &                           slotCount() const;
    HashedObjectListHash< TKey,TObj> &          slotCount(uintptr_t slotCountA);
  protected:
  private:
    HashedObjectListHash(const HashedObjectListHash< TKey,TObj> &){}
    void operator =(const HashedObjectListHash< TKey,TObj> &){}
    HashedObjectListItem< TKey,TObj> ** slots_;
    uintptr_t                           slotCount_;
};
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline HashedObjectListHash< TKey,TObj>::~HashedObjectListHash()
{
  xfree(slots_);
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline HashedObjectListHash< TKey,TObj>::HashedObjectListHash(uintptr_t slotCount)
  : slots_(NULL),
    slotCount_(0)
{
  resize(slotCount);
}
//---------------------------------------------------------------------------
template< class TKey,class TObj>
#ifndef __BCPLUSPLUS__
 inline
#endif
uintptr_t HashedObjectListHash< TKey,TObj>::maxChainLength(HashChainType chain) const
{
  uintptr_t max = 0, m ;
  for( intptr_t i = slotCount_ - 1; i >= 0; i-- ){
    HashedObjectListItem< TKey,TObj> ** p;
    for( m = 0, p = slots_ + i; *p != NULL; p = &(*p)->next()[chain], m++ );
    if( m > max ) max = m;
  }
  return max;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj>
#ifndef __BCPLUSPLUS__
 inline
#endif
uintptr_t HashedObjectListHash< TKey,TObj>::minChainLength(HashChainType chain) const
{
  uintptr_t min = ~(uintptr_t) 0, m ;
  for( intptr_t i = slotCount_ - 1; i >= 0; i-- ){
    HashedObjectListItem< TKey,TObj> ** p;
    for( m = 0, p = slots_ + i; *p != NULL; p = &(*p)->next()[chain], m++ );
    if( m < min ) min = m;
  }
  return min;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj>
#ifndef __BCPLUSPLUS__
 inline
#endif
uintptr_t HashedObjectListHash< TKey,TObj>::avgChainLength(HashChainType chain) const
{
  uintptr_t avg = 0, m ;
  for( intptr_t i = slotCount_ - 1; i >= 0; i-- ){
    HashedObjectListItem< TKey,TObj> ** p;
    for( m = 0, p = slots_ + i; *p != NULL; p = &(*p)->next()[chain], m++ );
    avg += m;
  }
  return slotCount_ > 0 ? avg / slotCount_ : 0;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj>
#ifndef __BCPLUSPLUS__
 inline
#endif
HashedObjectListHash< TKey,TObj> & HashedObjectListHash< TKey,TObj>::resize(uintptr_t slotCount)
{
  xrealloc(slots_, sizeof(HashedObjectListItem< TKey,TObj>) * slotCount);
  while( slotCount_ < slotCount ) slots_[slotCount_++] = NULL;
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj>
#ifndef __BCPLUSPLUS__
 inline
#endif
HashedObjectListItem< TKey,TObj> ** HashedObjectListHash< TKey,TObj>::find(const TKey & key, bool caseSensitive) const
{
  HashedObjectListItem< TKey,TObj> ** pItem;
  pItem = slots_ + (key.hash(caseSensitive) & (slotCount_ - 1));
  while( *pItem != NULL ){
    if( key.hashKeyEqu((*pItem)->key(), caseSensitive) ) break;
    pItem = &(*pItem)->keyNext();
  }
  return pItem;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj>
#ifndef __BCPLUSPLUS__
 inline
#endif
HashedObjectListItem< TKey,TObj> ** HashedObjectListHash< TKey,TObj>::find(const TObj * object) const
{
  HashedObjectListItem< TKey,TObj> ** pItem = slots_ + (HF::hash(uintptr_t(object)) & (slotCount_ - 1));
  while( *pItem != NULL ){
    if( object == (*pItem)->object() )
      break;
    pItem = &(*pItem)->objectNext();
  }
  return pItem;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj>
#ifndef __BCPLUSPLUS__
 inline
#endif
HashedObjectListItem< TKey,TObj> ** HashedObjectListHash< TKey,TObj>::find(const uintptr_t index) const
{
  HashedObjectListItem< TKey,TObj> ** pItem = slots_ + (HF::hash(index) & (slotCount_ - 1));
  while( *pItem != NULL ){
    if( index == (*pItem)->index() )
      break;
    pItem = &(*pItem)->indexNext();
  }
  return pItem;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectListItem< TKey,TObj> ** const & HashedObjectListHash< TKey,TObj>::slots() const
{
  return slots_;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectListHash< TKey,TObj> & HashedObjectListHash< TKey,TObj>::slots(HashedObjectListItem< TKey,TObj> ** slotsA)
{
  slots_ = slotsA;
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
const uintptr_t & HashedObjectListHash< TKey,TObj>::slotCount() const
{
  return slotCount_;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectListHash< TKey,TObj> & HashedObjectListHash< TKey,TObj>::slotCount(uintptr_t slotCountA)
{
  slotCount_ = slotCountA;
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template< class TKey,class TObj> class HashedObjectList {
  public:
    ~HashedObjectList();
    HashedObjectList();

    TObj *                              operator[](intptr_t i) const;
    TObj *                              operator[](uintptr_t i) const;

    HashedObjectList< TKey,TObj> &      clear();
    HashedObjectList< TKey,TObj> &      assign(const HashedObjectList< TKey,TObj> & source);
    HashedObjectList< TKey,TObj> &      transplant(HashedObjectList< TKey,TObj> & donor);
    uintptr_t                           add(TObj * object, const TKey & key, HashedObjectListItem< TKey,TObj> ** pItem = NULL);
    HashedObjectList< TKey,TObj> &      removeByKey(const TKey & key);
    HashedObjectList< TKey,TObj> &      removeByObject(TObj * object);
    HashedObjectList< TKey,TObj> &      removeByIndex(uintptr_t index);
    TObj *                              extractByKey(const TKey & key);
    TObj *                              extractByObject(TObj * object);
    TObj *                              extractByIndex(uintptr_t index);

    uintptr_t                           indexOfObject(TObj * object) const;
    uintptr_t                           indexOfKey(const TKey & key) const;
    TKey                                keyOfObject(TObj * object) const;
    TKey                                keyOfIndex(uintptr_t Index) const;
    TObj *                              objectOfKey(const TKey & key) const;
    TObj *                              objectOfIndex(uintptr_t index) const;

    HashedObjectListItem< TKey,TObj> *  itemOfKey(const TKey & key) const;
    HashedObjectListItem< TKey,TObj> *  itemOfObject(TObj * key) const;
    HashedObjectListItem< TKey,TObj> *  itemOfIndex(uintptr_t index) const;

    HashedObjectList< TKey,TObj> &      changeKey(const TKey & oldKey, const TKey & newKey);
    HashedObjectList< TKey,TObj> &      changeObject(TObj * oldObject, TObj * newObject);
    HashedObjectList< TKey,TObj> &      changeIndex(uintptr_t oldIndex, uintptr_t newIndex);

    HashedObjectList< TKey,TObj> &      transplantByKey(HashedObjectList< TKey,TObj> & donor, const TKey & key);
    HashedObjectList< TKey,TObj> &      transplantByObject(HashedObjectList< TKey,TObj> & donor, TObj * object);
    HashedObjectList< TKey,TObj> &      transplantByIndex(HashedObjectList< TKey,TObj> & donor, uintptr_t index);

    const bool &                        ownsObjects() const;
    HashedObjectList< TKey,TObj> &      ownsObjects(bool ownsObjects);
    const bool &                        caseSensitive() const;
    HashedObjectList< TKey,TObj> &      caseSensitive(bool caseSensitive);
    const uintptr_t &                   count() const;

    uintptr_t                           maxChainLength(HashChainType chain) const;
    uintptr_t                           minChainLength(HashChainType chain) const;
    uintptr_t                           avgChainLength(HashChainType chain) const;
  protected:
    HashedObjectListHash< TKey,TObj>  keyHash_;
    HashedObjectListHash< TKey,TObj>  objectHash_;
    HashedObjectListHash< TKey,TObj>  indexHash_;

    uintptr_t                         count_;
    bool                              ownsObjects_;
    bool                              caseSensitive_;

    void                                clearHash();
    void                                reHash(uintptr_t slotCount);
    void                                renumeration(uintptr_t index);

    uintptr_t                           addHelper(HashedObjectListItem< TKey,TObj> * item);
  private:
    HashedObjectList(const HashedObjectList< TKey,TObj> &){}
    void operator =(const HashedObjectList< TKey,TObj> &){}
};
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline HashedObjectList< TKey,TObj>::~HashedObjectList()
{
  clear();
}
//---------------------------------------------------------------------------
template< class TKey,class TObj>
#ifndef __BCPLUSPLUS__
 inline
#endif
HashedObjectList< TKey,TObj>::HashedObjectList() : count_(0), ownsObjects_(true), caseSensitive_(true)
{
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
TObj * HashedObjectList<TKey,TObj>::operator[](intptr_t i) const
{
  assert((uintptr_t) i < count_);
  return objectOfIndex(i);
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
TObj * HashedObjectList<TKey,TObj>::operator[](uintptr_t i) const
{
  assert(i < count_);
  return objectOfIndex(i);
}
//---------------------------------------------------------------------------
template< class TKey,class TObj>
#ifndef __BCPLUSPLUS__
 inline
#endif
void HashedObjectList< TKey,TObj>::clearHash()
{
  for( intptr_t i = keyHash_.slotCount() - 1; i >= 0; i-- ){
    keyHash_.slots()[i] = NULL;
    objectHash_.slots()[i] = NULL;
    indexHash_.slots()[i] = NULL;
  }
}
//---------------------------------------------------------------------------
template< class TKey,class TObj>
#ifndef __BCPLUSPLUS__
 inline
#endif
void HashedObjectList< TKey,TObj>::reHash(uintptr_t slotCount)
{
  if( slotCount > 0 ){
    HashedObjectListItem< TKey,TObj> ** ppItem0 = keyHash_.slots(), ** ppItem1 = ppItem0 + keyHash_.slotCount();
    HashedObjectListItem< TKey,TObj> *  head    = NULL, * pItem;
    while( ppItem0 < ppItem1 ){
      pItem = *ppItem0;
      while( pItem != NULL ){
        pItem->indexNext(head);
        head = pItem;
        pItem = pItem->keyNext();
      }
      ppItem0++;
    }
    clearHash();
    uintptr_t oldSlotCount = keyHash_.slotCount();
    Exception * esp = NULL;
    try{
      keyHash_.resize(slotCount);
      try {
        objectHash_.resize(slotCount);
        try {
          indexHash_.resize(slotCount);
        }
        catch( ... ){
          objectHash_.slotCount(oldSlotCount);
          throw;
        }
      }
      catch( ... ){
        keyHash_.slotCount(oldSlotCount);
        throw;
      }
    }
    catch( ExceptionSP & e ){
      esp = e.ptr();
      esp->addRef();
    }
    catch( ... ){
      assert(0);
    }
    HashedObjectListItem< TKey,TObj> ** pKeyItem, ** pIndexItem, ** pObjectItem;
    while( head != NULL ){
      pItem = head->indexNext();
      pKeyItem = keyHash_.find(head->key(), caseSensitive_);
      pObjectItem = objectHash_.find(head->object());
      pIndexItem = indexHash_.find(head->index());
      head->keyNext() = NULL;
      head->objectNext() = NULL;
      head->indexNext() = NULL;
      *pKeyItem = head;
      *pObjectItem = head;
      *pIndexItem = head;
      head = pItem;
    }
    if( esp != NULL ) esp->throwSP();
  }
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectList< TKey,TObj> & HashedObjectList< TKey,TObj>::caseSensitive(bool caseSensitive)
{
  if( caseSensitive != caseSensitive_ ){
    if( caseSensitive && !caseSensitive_ && count_ > 0 )
      newObject<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    caseSensitive_ = caseSensitive;
  }
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj>
#ifndef __BCPLUSPLUS__
 inline
#endif
HashedObjectList< TKey,TObj> & HashedObjectList< TKey,TObj>::clear()
{
  if( count_ > 0 ){
    HashedObjectListItem< TKey,TObj> ** ppItem0 = keyHash_.slots(), ** ppItem1 = ppItem0 + keyHash_.slotCount();
    while( ppItem0 < ppItem1 ){
      HashedObjectListItem< TKey,TObj> *  pItem = *ppItem0;
      while( pItem != NULL ){
        if( ownsObjects_ )
          delete pItem->object();
        HashedObjectListItem< TKey,TObj> *  pItem1  = pItem->keyNext();
        delete pItem;
        pItem = pItem1;
      }
      ppItem0++;
    }
    clearHash();
    count_ = 0;
  }
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj>
#ifndef __BCPLUSPLUS__
 inline
#endif
HashedObjectList< TKey,TObj> & HashedObjectList< TKey,TObj>::assign(const HashedObjectList & source)
{
  clear();
  ownsObjects_ = false;
  HashedObjectListItem< TKey,TObj> ** ppItem0 = source.keyHash_.slots();
  HashedObjectListItem< TKey,TObj> ** ppItem1 = ppItem0 + source.keyHash_.slotCount();
  while( ppItem0 < ppItem1 ){
    HashedObjectListItem< TKey,TObj> *  pItem = *ppItem0;
    while( pItem != NULL ){
      addHelper(newObject<HashedObjectListItem<TKey,TObj> >(*pItem));
      pItem = pItem->keyNext();
    }
    ppItem0++;
  }
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj>
#ifndef __BCPLUSPLUS__
 inline
#endif
HashedObjectList< TKey,TObj> & HashedObjectList< TKey,TObj>::transplant(HashedObjectList & donor)
{
  HashedObjectListItem< TKey,TObj> ** ppItem0 = donor.keyHash_.slots();
  HashedObjectListItem< TKey,TObj> ** ppItem1 = ppItem0 + donor.keyHash_.slotCount();
  while( ppItem0 < ppItem1 ){
    HashedObjectListItem< TKey,TObj> *  pItem = *ppItem0;
    while( pItem != NULL ){
      HashedObjectListItem< TKey,TObj> ** ppKey0, ** ppObject0, ** ppIndex0;
      pItem->index() = count_++;
      ppKey0 = keyHash_.find(pItem->key(), caseSensitive_);
      ppObject0 = objectHash_.find(pItem->object());
      ppIndex0 = indexHash_.find(pItem->index());
      HashedObjectListItem< TKey,TObj> *  pItemNext = pItem->keyNext();
      pItem->keyNext() = NULL;
      pItem->objectNext() = NULL;
      pItem->indexNext() = NULL;
      *ppKey0 = pItem;
      *ppObject0 = pItem;
      *ppIndex0 = pItem;
      pItem = pItemNext;
    }
    ppItem0++;
  }
  donor.clearHash();
  donor.count_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
uintptr_t HashedObjectList< TKey,TObj>::addHelper(HashedObjectListItem< TKey,TObj> * item)
{
  HashedObjectListItem< TKey,TObj> ** pKeyItem, ** pObjectItem, ** pIndexItem;
  pKeyItem = keyHash_.find(item->key(), caseSensitive_);
  pObjectItem = objectHash_.find(item->object());
  pIndexItem = indexHash_.find(item->index());
  if( *pKeyItem != NULL || *pObjectItem != NULL || *pIndexItem != NULL )
    newObject<Exception>(EEXIST, __PRETTY_FUNCTION__)->throwSP();
  *pKeyItem = item;
  *pObjectItem = item;
  *pIndexItem = item;
  count_++;
  uintptr_t count = keyHash_.slotCount() << 4;
  if( count_ > count + (((count << 2) + count) >> 3) )
    reHash(keyHash_.slotCount() << 1);
  return count_ - 1;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectList< TKey,TObj> & HashedObjectList< TKey,TObj>::changeKey(const TKey & oldKey, const TKey & newKey)
{
  HashedObjectListItem< TKey,TObj> ** ppOldKeyItem, * pOldKeyItem;
  ppOldKeyItem = keyHash_.find(oldKey, caseSensitive_);
  if( *ppOldKeyItem == NULL )
    newObject<Exception>(ENOENT, __PRETTY_FUNCTION__)->throwSP();
  *ppOldKeyItem = (pOldKeyItem = *ppOldKeyItem)->keyNext();

  HashedObjectListItem< TKey,TObj> ** ppNewKeyItem;
  ppNewKeyItem = keyHash_.find(newKey, caseSensitive_);
  if( *ppNewKeyItem != NULL ){
    *ppOldKeyItem = pOldKeyItem;
    newObject<Exception>(EEXIST, __PRETTY_FUNCTION__)->throwSP();
  }
  pOldKeyItem->key() = newKey;
  *ppNewKeyItem = pOldKeyItem;
  pOldKeyItem->keyNext() = NULL;
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectList< TKey,TObj> & HashedObjectList< TKey,TObj>::changeObject(TObj * oldObject, TObj * newObject)
{
  HashedObjectListItem< TKey,TObj> ** ppOldObjectItem, * pOldObjectItem;
  ppOldObjectItem = objectHash_.find(oldObject);
  if( *ppOldObjectItem == NULL )
    Exception::newObject()->code(ENOENT).what(__PRETTY_FUNCTION__).throwSP();
  *ppOldObjectItem = (pOldObjectItem = *ppOldObjectItem)->objectNext();

  HashedObjectListItem< TKey,TObj> ** ppNewObjectItem;
  ppNewObjectItem = objectHash_.find(newObject);
  if( *ppNewObjectItem != NULL ){
    *ppOldObjectItem = pOldObjectItem;
    Exception::newObject()->code(ENOENT).what(__PRETTY_FUNCTION__).throwSP();
  }
  if( ownsObjects_ ) delete pOldObjectItem->object();
  pOldObjectItem->object() = newObject;
  *ppNewObjectItem = pOldObjectItem;
  pOldObjectItem->objectNext() = NULL;
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectList< TKey,TObj> & HashedObjectList< TKey,TObj>::changeIndex(uintptr_t oldIndex, uintptr_t newIndex)
{
  HashedObjectListItem< TKey,TObj> ** ppOldItem, ** ppNewItem;
  ppOldItem = indexHash_.find(oldIndex);
  ppNewItem = indexHash_.find(newIndex);
  assert(*ppOldItem != NULL && *ppNewItem != NULL);
  if( oldIndex != newIndex ){
    HashedObjectListItem< TKey,TObj> *  pOldItem, * pNewItem;
    *ppOldItem = (pOldItem = *ppOldItem)->indexNext();
    *ppNewItem = (pNewItem = *ppNewItem)->indexNext();

    pOldItem->indexNext(NULL);
    pNewItem->indexNext(NULL);

    pOldItem->index(newIndex);
    ppOldItem = indexHash_.find(pOldItem->index());
    assert(*ppOldItem == NULL);
    *ppOldItem = pOldItem;

    pNewItem->index(oldIndex);
    ppNewItem = indexHash_.find(pNewItem->index());
    assert(*ppNewItem == NULL);
    *ppNewItem = pNewItem;
  }
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
uintptr_t HashedObjectList< TKey,TObj>::add(TObj * object, const TKey & key, HashedObjectListItem< TKey,TObj> ** pItem)
{
  AutoPtr<HashedObjectListItem< TKey,TObj> > item(
    newObject<HashedObjectListItem<TKey,TObj> >(key, object, count_)
  );
  uintptr_t i = addHelper(item.ptr());
  if( pItem != NULL ) *pItem = item.ptr();
  item.ptr(NULL);
  return i;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectList< TKey,TObj> & HashedObjectList< TKey,TObj>::removeByKey(const TKey & key)
{
  TObj *  object  = extractByKey(key);
  if( ownsObjects_ ) delete object;
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectList< TKey,TObj> & HashedObjectList< TKey,TObj>::removeByObject(TObj * object)
{
  object = extractByObject(object);
  if( ownsObjects_ )
    delete object;
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectList< TKey,TObj> & HashedObjectList< TKey,TObj>::removeByIndex(uintptr_t index)
{
  TObj *  object  = extractByIndex(index);
  if( ownsObjects_ )
    delete object;
  return *this;
}
//---------------------------------------------------------------------------
#ifdef __BCPLUSPLUS__
#pragma option push -w-8013
#endif
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
TObj * HashedObjectList< TKey,TObj>::extractByKey(const TKey & key)
{
  HashedObjectListItem< TKey,TObj> ** pKeyItem, ** pObjectItem, ** pIndexItem;
  pKeyItem = keyHash_.find(key, caseSensitive_);
  pObjectItem = objectHash_.find((*pKeyItem)->object());
  pIndexItem = indexHash_.find((*pKeyItem)->index());
  if( *pKeyItem == NULL || *pObjectItem == NULL || *pIndexItem == NULL )
    newObject<Exception>(ENOENT, __PRETTY_FUNCTION__)->throwSP();
  HashedObjectListItem< TKey,TObj> *  pItem = *pKeyItem;
  *pKeyItem = (*pKeyItem)->keyNext();
  *pObjectItem = (*pObjectItem)->objectNext();
  *pIndexItem = (*pIndexItem)->indexNext();
  uintptr_t index   = pItem->index();
  TObj *    object  = pItem->object();
  delete pItem;
  renumeration(index);
  count_--;
  uintptr_t count = keyHash_.slotCount() << 4;
  if( count_ < count - (((count << 2) + count) >> 3) )
    reHash(keyHash_.slotCount() >> 1);
  return object;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
TObj * HashedObjectList< TKey,TObj>::extractByObject(TObj * object)
{
  if( object != NULL ){
    HashedObjectListItem< TKey,TObj> ** pKeyItem, ** pObjectItem, ** pIndexItem;
    pObjectItem = objectHash_.find(object);
    pKeyItem = keyHash_.find((*pObjectItem)->key(), caseSensitive_);
    pIndexItem = indexHash_.find((*pObjectItem)->index());
    if( *pObjectItem == NULL || *pKeyItem == NULL || *pIndexItem == NULL )
      newObject<Exception>(ENOENT, __PRETTY_FUNCTION__)->throwSP();
    HashedObjectListItem< TKey,TObj> *  pItem = *pKeyItem;
    *pKeyItem = (*pKeyItem)->keyNext();
    *pObjectItem = (*pObjectItem)->objectNext();
    *pIndexItem = (*pIndexItem)->indexNext();
    uintptr_t index = pItem->index();
    delete pItem;
    renumeration(index);
    count_--;
    uintptr_t count = keyHash_.slotCount() << 4;
    if( count_ < count - (((count << 2) + count) >> 3) )
      reHash(keyHash_.slotCount() >> 1);
  }
  return object;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
TObj * HashedObjectList< TKey,TObj>::extractByIndex(uintptr_t index)
{
  HashedObjectListItem< TKey,TObj> ** pKeyItem, ** pObjectItem, ** pIndexItem;
  pIndexItem = indexHash_.find(index);
  pObjectItem = objectHash_.find((*pIndexItem)->object());
  pKeyItem = keyHash_.find((*pIndexItem)->key(), caseSensitive_);
  if( *pIndexItem == NULL || *pObjectItem == NULL || *pKeyItem == NULL )
    newObject<Exception>(ENOENT, __PRETTY_FUNCTION__)->throwSP();
  HashedObjectListItem< TKey,TObj> *  pItem = *pKeyItem;
  *pKeyItem = (*pKeyItem)->keyNext();
  *pObjectItem = (*pObjectItem)->objectNext();
  *pIndexItem = (*pIndexItem)->indexNext();
  TObj *  object  = pItem->object();
  delete pItem;
  renumeration(index);
  count_--;
  uintptr_t count = keyHash_.slotCount() << 4;
  if( count_ < count - (((count << 2) + count) >> 3) )
    reHash(keyHash_.slotCount() >> 1);
  return object;
}
//---------------------------------------------------------------------------
#ifdef __BCPLUSPLUS__
#pragma option pop
#endif
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
void HashedObjectList< TKey,TObj>::renumeration(uintptr_t index)
{
  if( index < count_ - 1 ){
    HashedObjectListItem< TKey,TObj> ** pIndexItem, * pItem;
    pIndexItem = indexHash_.find(count_ - 1);
    pItem = *pIndexItem;
    assert(pItem != NULL);
    *pIndexItem = pItem->indexNext();
    pItem->indexNext(NULL);
    pItem->index(index);
    pIndexItem = indexHash_.find(index);
    *pIndexItem = pItem;
  }
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
uintptr_t HashedObjectList< TKey,TObj>::indexOfObject(TObj * object) const
{
  HashedObjectListItem< TKey,TObj> ** pItem = objectHash_.find(object);
  return *pItem == NULL ? -1 : (*pItem)->index();
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
uintptr_t HashedObjectList< TKey,TObj>::indexOfKey(const TKey & key) const
{
  HashedObjectListItem< TKey,TObj> ** pItem = keyHash_.find(key, caseSensitive_);
  return *pItem == NULL ? -1 : (*pItem)->index();
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
TKey HashedObjectList< TKey,TObj>::keyOfObject(TObj * object) const
{
  HashedObjectListItem< TKey,TObj> ** pItem = objectHash_.find(object);
  return *pItem == NULL ? TKey() : (*pItem)->key();
}
//---------------------------------------------------------------------------
template< class TKey,class TObj>
#ifndef __BCPLUSPLUS__
 inline
#endif
TKey HashedObjectList< TKey,TObj>::keyOfIndex(uintptr_t index) const
{
  HashedObjectListItem< TKey,TObj> ** pItem = indexHash_.find(index);
  return *pItem == NULL ? TKey() : (*pItem)->key();
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
TObj * HashedObjectList< TKey,TObj>::objectOfKey(const TKey & key) const
{
  HashedObjectListItem< TKey,TObj> ** pItem = keyHash_.find(key, caseSensitive_);
  return *pItem == NULL ? NULL : (*pItem)->object();
}                      
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
TObj * HashedObjectList< TKey,TObj>::objectOfIndex(uintptr_t index) const
{
  HashedObjectListItem< TKey,TObj> ** pItem = indexHash_.find(index);
  return *pItem == NULL ? NULL : (*pItem)->object();
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectListItem< TKey,TObj> * HashedObjectList< TKey,TObj>::itemOfKey(const TKey & key) const
{
  return *keyHash_.find(key, caseSensitive_);
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectListItem< TKey,TObj> * HashedObjectList< TKey,TObj>::itemOfObject(TObj * object) const
{
  return *objectHash_.find(object);
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectListItem< TKey,TObj> * HashedObjectList< TKey,TObj>::itemOfIndex(uintptr_t index) const
{
  return *indexHash_.find(index);
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectList< TKey,TObj> & HashedObjectList< TKey,TObj>::transplantByKey(HashedObjectList & donor, const TKey & key)
{
  HashedObjectListItem< TKey,TObj> ** ppKeyItem0, ** ppObjectItem0, ** ppIndexItem0;
  HashedObjectListItem< TKey,TObj> ** ppKeyItem1, ** ppObjectItem1, ** ppIndexItem1;

  ppKeyItem0 = donor.keyHash_.find(key, donor.caseSensitive_);
  ppObjectItem0 = donor.objectHash_.find((*ppKeyItem0)->object());
  ppIndexItem0 = donor.indexHash_.find((*ppKeyItem0)->index());
  if( *ppKeyItem0 == NULL || *ppObjectItem0 == NULL || *ppIndexItem0 == NULL || *ppKeyItem0 != *ppObjectItem0 || *ppKeyItem0 != *ppIndexItem0 )
    newObject<Exception>(ENOENT, __PRETTY_FUNCTION__)->throwSP();

  ppKeyItem1 = keyHash_.find(key, caseSensitive_);
  ppObjectItem1 = objectHash_.find((*ppKeyItem0)->object());
  if( *ppKeyItem1 != NULL || *ppObjectItem1 != NULL )
    newObject<Exception>(EEXIST, __PRETTY_FUNCTION__)->throwSP();

  HashedObjectListItem< TKey,TObj> *  pItem = *ppKeyItem0;

  *ppKeyItem0 = pItem->keyNext();
  *ppObjectItem0 = pItem->objectNext();
  *ppIndexItem0 = pItem->indexNext();
  donor.renumeration(pItem->index());
  donor.count_--;

  pItem->keyNext() = NULL;
  pItem->objectNext() = NULL;
  pItem->indexNext() = NULL;
  pItem->index() = count_;

  ppIndexItem1 = indexHash_.find(pItem->index());

  *ppKeyItem1 = pItem;
  *ppObjectItem1 = pItem;
  *ppIndexItem1 = pItem;
  count_++;
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectList< TKey,TObj> & HashedObjectList< TKey,TObj>::transplantByObject(HashedObjectList & donor, TObj * object)
{
  HashedObjectListItem< TKey,TObj> ** ppKeyItem0, ** ppObjectItem0, ** ppIndexItem0;
  HashedObjectListItem< TKey,TObj> ** ppKeyItem1, ** ppObjectItem1, ** ppIndexItem1;

  ppObjectItem0 = donor.objectHash_.find(object);
  ppKeyItem0 = donor.keyHash_.find((*ppObjectItem0)->key(), donor.caseSensitive_);
  ppIndexItem0 = donor.indexHash_.find((*ppObjectItem0)->index());
  if( *ppObjectItem0 == NULL || *ppKeyItem0 == NULL || *ppIndexItem0 == NULL || *ppObjectItem0 != *ppKeyItem0 || *ppObjectItem0 != *ppIndexItem0 )
    newObject<Exception>(ENOENT, __PRETTY_FUNCTION__)->throwSP();

  ppKeyItem1 = keyHash_.find((*ppKeyItem0)->key(), caseSensitive_);
  ppObjectItem1 = objectHash_.find((*ppKeyItem0)->object());
  if( *ppKeyItem1 != NULL || *ppObjectItem1 != NULL )
    newObject<Exception>(EEXIST, __PRETTY_FUNCTION__)->throwSP();

  HashedObjectListItem< TKey,TObj> *  pItem = *ppKeyItem0;

  *ppKeyItem0 = pItem->keyNext();
  *ppObjectItem0 = pItem->objectNext();
  *ppIndexItem0 = pItem->indexNext();
  donor.renumeration(pItem->index());
  donor.count_--;

  pItem->keyNext() = NULL;
  pItem->objectNext() = NULL;
  pItem->indexNext() = NULL;
  pItem->index() = count_;

  ppIndexItem1 = indexHash_.find(pItem->index());

  *ppKeyItem1 = pItem;
  *ppObjectItem1 = pItem;
  *ppIndexItem1 = pItem;
  count_++;
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectList< TKey,TObj> & HashedObjectList< TKey,TObj>::transplantByIndex(HashedObjectList & donor, uintptr_t index)
{
  HashedObjectListItem< TKey,TObj> ** ppKeyItem0, ** ppObjectItem0, ** ppIndexItem0;
  HashedObjectListItem< TKey,TObj> ** ppKeyItem1, ** ppObjectItem1, ** ppIndexItem1;

  ppIndexItem0 = donor.indexHash_.find(index);
  ppObjectItem0 = donor.objectHash_.find((*ppIndexItem0)->object());
  ppKeyItem0 = donor.keyHash_.find((*ppIndexItem0)->key(), donor.caseSensitive_);
  if( *ppIndexItem0 == NULL || *ppObjectItem0 == NULL || *ppKeyItem0 == NULL || *ppIndexItem0 != *ppObjectItem0 || *ppIndexItem0 != *ppKeyItem0 )
    newObject<Exception>(ENOENT, __PRETTY_FUNCTION__)->throwSP();

  ppKeyItem1 = keyHash_.find((*ppKeyItem0)->key(), caseSensitive_);
  ppObjectItem1 = objectHash_.find((*ppKeyItem0)->object());
  if( *ppKeyItem1 != NULL || *ppObjectItem1 != NULL )
    newObject<Exception>(EEXIST, __PRETTY_FUNCTION__)->throwSP();

  HashedObjectListItem< TKey,TObj> *  pItem = *ppKeyItem0;

  *ppKeyItem0 = pItem->keyNext();
  *ppObjectItem0 = pItem->objectNext();
  *ppIndexItem0 = pItem->indexNext();
  donor.renumeration(pItem->index());
  donor.count_--;

  pItem->keyNext() = NULL;
  pItem->objectNext() = NULL;
  pItem->indexNext() = NULL;
  pItem->index() = count_;

  ppIndexItem1 = indexHash_.find(pItem->index());

  *ppKeyItem1 = pItem;
  *ppObjectItem1 = pItem;
  *ppIndexItem1 = pItem;
  count_++;
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
const bool & HashedObjectList< TKey,TObj>::ownsObjects() const
{
  return ownsObjects_;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
HashedObjectList< TKey,TObj> & HashedObjectList< TKey,TObj>::ownsObjects(bool owns)
{
  ownsObjects_ = owns;
  return *this;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
const bool & HashedObjectList< TKey,TObj>::caseSensitive() const
{
  return caseSensitive_;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
const uintptr_t & HashedObjectList< TKey,TObj>::count() const
{
  return count_;
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
uintptr_t HashedObjectList< TKey,TObj>::maxChainLength(HashChainType chain) const
{
  return (&keyHash_ + chain)->maxChainLength(chain);
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
uintptr_t HashedObjectList< TKey,TObj>::minChainLength(HashChainType chain) const
{
  return (&keyHash_ + chain)->minChainLength(chain);
}
//---------------------------------------------------------------------------
template< class TKey,class TObj> inline
uintptr_t HashedObjectList< TKey,TObj>::avgChainLength(HashChainType chain) const
{
  return (&keyHash_ + chain)->avgChainLength(chain);
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

