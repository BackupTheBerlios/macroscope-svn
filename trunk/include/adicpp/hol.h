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
template <typename T>
class HashedObjectListKey {
  public:
    ~HashedObjectListKey();
    HashedObjectListKey();
    HashedObjectListKey(const T & key);
    HashedObjectListKey(const HashedObjectListKey<T> & key);

    HashedObjectListKey<T> & operator = (const T & key);
    HashedObjectListKey<T> & operator = (const HashedObjectListKey<T> & key);
    
    bool hashKeyEqu(const HashedObjectListKey<T> & key,bool) const;
    uintptr_t hash(bool) const;
  protected:
  private:
    T key_;
};
//---------------------------------------------------------------------------
template <typename T>
inline HashedObjectListKey<T>::~HashedObjectListKey()
{
}
//---------------------------------------------------------------------------
template <typename T>
inline HashedObjectListKey<T>::HashedObjectListKey()
{
}
//---------------------------------------------------------------------------
template <typename T>
inline HashedObjectListKey<T>::HashedObjectListKey(const T & key) : key_(key)
{
}
//---------------------------------------------------------------------------
template <typename T>
inline HashedObjectListKey<T>::HashedObjectListKey(const HashedObjectListKey<T> & key) : key_(key.key_)
{
}
//---------------------------------------------------------------------------
template <typename T>
inline HashedObjectListKey<T> & HashedObjectListKey<T>::operator = (const T & key)
{
  key_ = key;
  return *this;
}
//---------------------------------------------------------------------------
template <typename T>
inline HashedObjectListKey<T> & HashedObjectListKey<T>::operator = (const HashedObjectListKey<T> & key)
{
  key_ = key.key_;
  return *this;
}
//---------------------------------------------------------------------------
template <typename T>
inline bool HashedObjectListKey<T>::hashKeyEqu(const HashedObjectListKey<T> & key,bool) const
{
  return memcmp(&key_,&key.key_,sizeof(key_)) == 0;
}
//---------------------------------------------------------------------------
template <typename T>
inline uintptr_t HashedObjectListKey<T>::hash(bool) const
{
  return HF::hash(&key_,sizeof(key_));
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <class TKey,class TObj>
class EHashedObjectList : public Exception {
  public:
    EHashedObjectList(long code,const utf8::String & what);
};
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
EHashedObjectList<TKey,TObj>::EHashedObjectList(long code,const utf8::String & what) : Exception(code,what)
{
}  
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <class TKey,class TObj> class HashedObjectList;
template <class TKey,class TObj> class HashedObjectListHash;
//---------------------------------------------------------------------------
enum HashChainType {
  hctKey,
  hctObject,
  hctIndex
};
//---------------------------------------------------------------------------
template <class TKey,class TObj>
class HashedObjectListItem {
  friend class HashedObjectList<TKey,TObj>;
  friend class HashedObjectListHash<TKey,TObj>;
  public:
    TKey key_;
    TObj * object_;
    uintptr_t index_;

    ~HashedObjectListItem();
    HashedObjectListItem(const HashedObjectListItem<TKey,TObj> & src);
    HashedObjectListItem(const TKey & key,TObj * object,uintptr_t index);
  protected:
    union {
      HashedObjectListItem<TKey,TObj> * next_[3];
      struct {
        HashedObjectListItem<TKey,TObj> * keyNext_;
        HashedObjectListItem<TKey,TObj> * objectNext_;
        HashedObjectListItem<TKey,TObj> * indexNext_;
      };
    };
  private:
};
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectListItem<TKey,TObj>::~HashedObjectListItem()
{
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectListItem<TKey,TObj>::HashedObjectListItem(const HashedObjectListItem<TKey,TObj> & src)
{
  keyNext_ = objectNext_ = indexNext_ = NULL;
  key_ = src.key_;
  object_ = src.object_;
  index_ = src.index_;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectListItem<TKey,TObj>::HashedObjectListItem(const TKey & key,TObj * object,uintptr_t index)
{
  keyNext_ = objectNext_ = indexNext_ = NULL;
  key_ = key;
  object_ = object;
  index_ = index;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <class TKey,class TObj>
class HashedObjectListHash {
  friend class HashedObjectList<TKey,TObj>;
  private:
  protected:
    HashedObjectListItem<TKey,TObj> ** slots_;
    uintptr_t slotCount_;
  public:
    HashedObjectListHash(uintptr_t slotCount = 1);
    ~HashedObjectListHash();

    HashedObjectListItem<TKey,TObj> ** find(const TKey & key,bool caseSensitive) const;
    HashedObjectListItem<TKey,TObj> ** find(TObj * object) const;
    HashedObjectListItem<TKey,TObj> ** find(uintptr_t index) const;

    uintptr_t maxChainLength(HashChainType chain) const;
    uintptr_t minChainLength(HashChainType chain) const;
    uintptr_t avgChainLength(HashChainType chain) const;
};
//---------------------------------------------------------------------------
template <class TKey,class TObj>
#ifndef __BCPLUSPLUS__
inline
#endif
uintptr_t HashedObjectListHash<TKey,TObj>::maxChainLength(HashChainType chain) const
{
  uintptr_t max = 0, m;
  for( intptr_t i = slotCount_ - 1; i >= 0; i-- ){
    HashedObjectListItem<TKey,TObj> ** p;
    for( m = 0, p = slots_ + i; *p != NULL; p = &(*p)->next_[chain], m++ );
    if( m > max ) max = m;
  }
  return max;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj>
#ifndef __BCPLUSPLUS__
inline
#endif
uintptr_t HashedObjectListHash<TKey,TObj>::minChainLength(HashChainType chain) const
{
  uintptr_t min = ~(uintptr_t) 0, m;
  for( intptr_t i = slotCount_ - 1; i >= 0; i-- ){
    HashedObjectListItem<TKey,TObj> ** p;
    for( m = 0, p = slots_ + i; *p != NULL; p = &(*p)->next_[chain], m++ );
    if( m < min ) min = m;
  }
  return min;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj>
#ifndef __BCPLUSPLUS__
inline
#endif
uintptr_t HashedObjectListHash<TKey,TObj>::avgChainLength(HashChainType chain) const
{
  uintptr_t avg = 0, m;
  for( intptr_t i = slotCount_ - 1; i >= 0; i-- ){
    HashedObjectListItem<TKey,TObj> ** p;
    for( m = 0, p = slots_ + i; *p != NULL; p = &(*p)->next_[chain], m++ );
    avg += m;
  }
  return slotCount_ > 0 ? avg / slotCount_ : 0;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj>
#ifndef __BCPLUSPLUS__
inline
#endif
HashedObjectListHash<TKey,TObj>::HashedObjectListHash(uintptr_t slotCount)
{
  xmalloc(slots_,sizeof(HashedObjectListItem<TKey,TObj>) * slotCount);
  slotCount_ = slotCount;
  while( slotCount > 0 ) slots_[--slotCount] = NULL;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectListHash<TKey,TObj>::~HashedObjectListHash()
{
  xfree(slots_);
}
//---------------------------------------------------------------------------
template <class TKey,class TObj>
#ifndef __BCPLUSPLUS__
inline
#endif
HashedObjectListItem<TKey,TObj> ** HashedObjectListHash<TKey,TObj>::find(const TKey & key,bool caseSensitive) const
{
  HashedObjectListItem<TKey,TObj> ** pItem;
  pItem = slots_ + (key.hash(caseSensitive) & (slotCount_ - 1));
  while( *pItem != NULL ){
    if( key.hashKeyEqu((*pItem)->key_,caseSensitive) ) break;
    pItem = &(*pItem)->keyNext_;
  }
  return pItem;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj>
#ifndef __BCPLUSPLUS__
inline
#endif
HashedObjectListItem<TKey,TObj> ** HashedObjectListHash<TKey,TObj>::find(TObj * object) const
{
  HashedObjectListItem<TKey,TObj> ** pItem = slots_ + (HF::hash(object) & (slotCount_ - 1));
  while( *pItem != NULL ){
    if( object == (*pItem)->object_ ) break;
    pItem = &(*pItem)->objectNext_;
  }
  return pItem;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj>
#ifndef __BCPLUSPLUS__
inline
#endif
HashedObjectListItem<TKey,TObj> ** HashedObjectListHash<TKey,TObj>::find(uintptr_t index) const
{
  HashedObjectListItem<TKey,TObj> ** pItem = slots_ + (HF::hash(index) & (slotCount_ - 1));
  while( *pItem != NULL ){
    if( index == (*pItem)->index_ ) break;
    pItem = &(*pItem)->indexNext_;
  }
  return pItem;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <class TKey,class TObj>
class HashedObjectList {
  public:
    ~HashedObjectList();
    HashedObjectList();

    TObj * operator [] (intptr_t i) const;
    TObj * operator [] (uintptr_t i) const;

    HashedObjectList<TKey,TObj> & clear();
    HashedObjectList<TKey,TObj> & assign(const HashedObjectList<TKey,TObj> & source);
    HashedObjectList<TKey,TObj> & transplant(HashedObjectList<TKey,TObj> & donor);
    uintptr_t add(TObj * object,const TKey & key,HashedObjectListItem<TKey,TObj> ** pItem = NULL);
    HashedObjectList<TKey,TObj> & removeByKey(const TKey & key);
    HashedObjectList<TKey,TObj> & removeByObject(TObj * object);
    HashedObjectList<TKey,TObj> & removeByIndex(uintptr_t index);
    TObj * extractByKey(const TKey & key);
    TObj * extractByObject(TObj * object);
    TObj * extractByIndex(uintptr_t index);

    uintptr_t indexOfObject(TObj * object) const;
    uintptr_t indexOfKey(const TKey & key) const;
    TKey keyOfObject(TObj * object) const;
    TKey keyOfIndex(uintptr_t Index) const;
    TObj * objectOfKey(const TKey & key) const;
    TObj * objectOfIndex(uintptr_t index) const;

    HashedObjectListItem<TKey,TObj> * itemOfKey(const TKey & key) const;
    HashedObjectListItem<TKey,TObj> * itemOfObject(TObj * key) const;
    HashedObjectListItem<TKey,TObj> * itemOfIndex(uintptr_t index) const;

    HashedObjectList<TKey,TObj> & changeKey(const TKey & oldKey,const TKey & newKey);
    HashedObjectList<TKey,TObj> & changeObject(TObj * oldObject,TObj * newObject);
    HashedObjectList<TKey,TObj> & changeIndex(uintptr_t oldIndex,uintptr_t newIndex);

    HashedObjectList<TKey,TObj> & transplantByKey(HashedObjectList<TKey,TObj> & donor,const TKey & key);
    HashedObjectList<TKey,TObj> & transplantByObject(HashedObjectList<TKey,TObj> & donor,TObj * object);
    HashedObjectList<TKey,TObj> & transplantByIndex(HashedObjectList<TKey,TObj> & donor,uintptr_t index);

    uintptr_t reHashThreshold() const;
    HashedObjectList<TKey,TObj> & reHashThreshold(uintptr_t reHashThreshold);
    const bool & ownsObjects() const;
    HashedObjectList<TKey,TObj> & ownsObjects(bool ownsObjects);
    const bool & caseSensitive() const;
    HashedObjectList<TKey,TObj> & caseSensitive(bool caseSensitive);
    const uintptr_t & count() const;

    uintptr_t maxChainLength(HashChainType chain) const;
    uintptr_t minChainLength(HashChainType chain) const;
    uintptr_t avgChainLength(HashChainType chain) const;
  protected:
    HashedObjectListHash<TKey,TObj> keyHash_;
    HashedObjectListHash<TKey,TObj> objectHash_;
    HashedObjectListHash<TKey,TObj> indexHash_;

    uintptr_t reHashThreshold_;
    uintptr_t count_;
    bool ownsObjects_;
    bool caseSensitive_;

    void reHash(uintptr_t slotCount,uintptr_t reHashThreshold);
    void renumeration(uintptr_t index);

    uintptr_t addHelper(HashedObjectListItem<TKey,TObj> * item);
  private:
};
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectList<TKey,TObj>::~HashedObjectList()
{
  clear();
}
//---------------------------------------------------------------------------
template <class TKey,class TObj>
#ifndef __BCPLUSPLUS__
inline
#endif
HashedObjectList<TKey,TObj>::HashedObjectList() :
  reHashThreshold_(4), count_(0), ownsObjects_(true), caseSensitive_(true)
{
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
TObj * HashedObjectList<TKey,TObj>::operator [] (intptr_t i) const
{
  assert( (uintptr_t) i < count_ );
  return objectOfIndex(i);
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
TObj * HashedObjectList<TKey,TObj>::operator [] (uintptr_t i) const
{
  assert( i < count_ );
  return objectOfIndex(i);
}
//---------------------------------------------------------------------------
template <class TKey,class TObj>
#ifndef __BCPLUSPLUS__
inline
#endif
void HashedObjectList<TKey,TObj>::reHash(uintptr_t slotCount,uintptr_t reHashThreshold)
{
  HashedObjectListHash<TKey,TObj> aKeyHash(slotCount);
  HashedObjectListHash<TKey,TObj> aObjectHash(slotCount);
  HashedObjectListHash<TKey,TObj> aIndexHash(slotCount);

  HashedObjectListItem<TKey,TObj> ** ppItem0 = keyHash_.slots_, ** ppItem1 = ppItem0 + keyHash_.slotCount_;
  HashedObjectListItem<TKey,TObj> * head = NULL, * pItem;
  while( ppItem0 < ppItem1 ){
    pItem = *ppItem0;
    while( pItem != NULL ){
      pItem->indexNext_ = head;
      head = pItem;
      pItem = pItem->keyNext_;
    }
    ppItem0++;
  }
  HashedObjectListItem<TKey,TObj> ** pKeyItem, ** pIndexItem, ** pObjectItem;

  xfree(keyHash_.slots_);
  keyHash_.slots_ = aKeyHash.slots_;
  keyHash_.slotCount_ = aKeyHash.slotCount_;
  aKeyHash.slots_ = NULL;
  xfree(objectHash_.slots_);
  objectHash_.slots_ = aObjectHash.slots_;
  objectHash_.slotCount_ = aObjectHash.slotCount_;
  aObjectHash.slots_ = NULL;
  xfree(indexHash_.slots_);
  indexHash_.slots_ = aIndexHash.slots_;
  indexHash_.slotCount_ = aIndexHash.slotCount_;
  aIndexHash.slots_ = NULL;
  reHashThreshold_ = reHashThreshold;
  while( head != NULL ){
    pItem = head->indexNext_;
    pKeyItem = keyHash_.find(head->key_,caseSensitive_);
    pObjectItem = objectHash_.find(head->object_);
    pIndexItem = indexHash_.find(head->index_);
    head->keyNext_ = NULL;
    head->objectNext_ = NULL;
    head->indexNext_ = NULL;
    *pKeyItem = head;
    *pObjectItem = head;
    *pIndexItem = head;
    head = pItem;
  }
}
//---------------------------------------------------------------------------
template <class TKey,class TObj>
#ifndef __BCPLUSPLUS__
inline
#endif
HashedObjectList<TKey,TObj> & HashedObjectList<TKey,TObj>::reHashThreshold(uintptr_t reHashThreshold)
{
  if( reHashThreshold > 0 && reHashThreshold != reHashThreshold_ ){
    uintptr_t newThreshold;
    for( newThreshold = 1; (uintptr_t(1) << newThreshold) < reHashThreshold; newThreshold++ );
    reHash(keyHash_.slotCount_,newThreshold);
  }
  return *this;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectList<TKey,TObj> & HashedObjectList<TKey,TObj>::caseSensitive(bool caseSensitive)
{
  if( caseSensitive != caseSensitive_ ){
    if( caseSensitive && !caseSensitive_ && count_ > 0 )
      throw ExceptionSP(new EHashedObjectList<TKey,TObj>(-1,utf8::string(__PRETTY_FUNCTION__)));
    caseSensitive_ = caseSensitive;
  }
  return *this;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj>
#ifndef __BCPLUSPLUS__
inline
#endif
HashedObjectList<TKey,TObj> & HashedObjectList<TKey,TObj>::clear(void)
{
  if( count_ > 0 ){
    HashedObjectListItem<TKey,TObj> ** ppItem0 = keyHash_.slots_, ** ppItem1 = ppItem0 + keyHash_.slotCount_;
    while( ppItem0 < ppItem1 ){
      HashedObjectListItem<TKey,TObj> * pItem = *ppItem0;
      while( pItem != NULL ){
        if( ownsObjects_ ) delete pItem->object_;
        HashedObjectListItem<TKey,TObj> * pItem1 = pItem->keyNext_;
        delete pItem;
        pItem = pItem1;
      }
      ppItem0++;
    }
    memset(keyHash_.slots_,0,sizeof(HashedObjectListItem<TKey,TObj> *) * keyHash_.slotCount_);
    memset(objectHash_.slots_,0,sizeof(HashedObjectListItem<TKey,TObj> *) * objectHash_.slotCount_);
    memset(indexHash_.slots_,0,sizeof(HashedObjectListItem<TKey,TObj> *) * indexHash_.slotCount_);
    count_ = 0;
  }
  return *this;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj>
#ifndef __BCPLUSPLUS__
inline
#endif
HashedObjectList<TKey,TObj> & HashedObjectList<TKey,TObj>::assign(const HashedObjectList & source)
{
  clear();
  ownsObjects_ = false;
  HashedObjectListItem<TKey,TObj> ** ppItem0 = source.keyHash_.slots_;
  HashedObjectListItem<TKey,TObj> ** ppItem1 = ppItem0 + source.keyHash_.slotCount_;
  while( ppItem0 < ppItem1 ){
    HashedObjectListItem<TKey,TObj> * pItem = *ppItem0;
    while( pItem != NULL ){
      addHelper(new HashedObjectListItem<TKey,TObj>(*pItem));
      pItem = pItem->keyNext_;
    }
    ppItem0++;
  }
  return *this;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj>
#ifndef __BCPLUSPLUS__
inline
#endif
HashedObjectList<TKey,TObj> & HashedObjectList<TKey,TObj>::transplant(HashedObjectList & donor)
{
  HashedObjectListItem<TKey,TObj> ** ppItem0 = donor.keyHash_.slots_;
  HashedObjectListItem<TKey,TObj> ** ppItem1 = ppItem0 + donor.keyHash_.slotCount_;
  while( ppItem0 < ppItem1 ){
    HashedObjectListItem<TKey,TObj> * pItem = *ppItem0;
    while( pItem != NULL ){
      HashedObjectListItem<TKey,TObj> ** ppKey0, ** ppObject0, ** ppIndex0;
      pItem->index_ = count_++;
      ppKey0 = keyHash_.find(pItem->key_,caseSensitive_);
      ppObject0 = objectHash_.find(pItem->object_);
      ppIndex0 = indexHash_.find(pItem->index_);
      HashedObjectListItem<TKey,TObj> * pItemNext = pItem->keyNext_;
      pItem->keyNext_ = NULL;
      pItem->objectNext_ = NULL;
      pItem->indexNext_ = NULL;
      *ppKey0 = pItem;
      *ppObject0 = pItem;
      *ppIndex0 = pItem;
      pItem = pItemNext;
    }
    ppItem0++;
  }
  memset(donor.keyHash_.slots_,0,sizeof(HashedObjectListItem<TKey,TObj> *) * donor.keyHash_.slotCount_);
  memset(donor.objectHash_.slots_,0,sizeof(HashedObjectListItem<TKey,TObj> *) * donor.objectHash_.slotCount_);
  memset(donor.indexHash_.slots_,0,sizeof(HashedObjectListItem<TKey,TObj> *) * donor.indexHash_.slotCount_);
  donor.count_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
uintptr_t HashedObjectList<TKey,TObj>::addHelper(HashedObjectListItem<TKey,TObj> * item)
{
  HashedObjectListItem<TKey,TObj> ** pKeyItem, ** pObjectItem, ** pIndexItem;
  pKeyItem = keyHash_.find(item->key_,caseSensitive_);
  pObjectItem = objectHash_.find(item->object_);
  pIndexItem = indexHash_.find(item->index_);
  if( *pKeyItem != NULL || *pObjectItem != NULL || *pIndexItem != NULL )
    throw ExceptionSP(
      new EHashedObjectList<TKey,TObj>(
        -1,utf8::string(__PRETTY_FUNCTION__)
      )
    );
  *pKeyItem = item;
  *pObjectItem = item;
  *pIndexItem = item;
  count_++;
  if( (count_ >> reHashThreshold_) >= keyHash_.slotCount_ )
    reHash(keyHash_.slotCount_ << 1,reHashThreshold_);
  return count_ - 1;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectList<TKey,TObj> & HashedObjectList<TKey,TObj>::changeKey(const TKey & oldKey,const TKey & newKey)
{
  HashedObjectListItem<TKey,TObj> ** ppOldKeyItem, * pOldKeyItem;
  ppOldKeyItem = keyHash_.find(oldKey,caseSensitive_);
  if( *ppOldKeyItem == NULL ) throw ExceptionSP(new EHashedObjectList<TKey,TObj>(-1,utf8::string(__PRETTY_FUNCTION__)));
  *ppOldKeyItem = (pOldKeyItem = *ppOldKeyItem)->keyNext_;
  
  HashedObjectListItem<TKey,TObj> ** ppNewKeyItem;
  ppNewKeyItem = keyHash_.find(newKey,caseSensitive_);
  if( *ppNewKeyItem != NULL ){
    *ppOldKeyItem = pOldKeyItem;
    throw ExceptionSP(new EHashedObjectList<TKey,TObj>(-1,utf8::string(__PRETTY_FUNCTION__)));
  }
  pOldKeyItem->key_ = newKey;
  *ppNewKeyItem = pOldKeyItem;
  pOldKeyItem->keyNext_ = NULL;
  return *this;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectList<TKey,TObj> & HashedObjectList<TKey,TObj>::changeObject(TObj * oldObject,TObj * newObject)
{
  HashedObjectListItem<TKey,TObj> ** ppOldObjectItem, * pOldObjectItem;
  ppOldObjectItem = objectHash_.find(oldObject);
  if( *ppOldObjectItem == NULL ) throw ExceptionSP(new EHashedObjectList<TKey,TObj>(-1,utf8::string(__PRETTY_FUNCTION__)));
  *ppOldObjectItem = (pOldObjectItem = *ppOldObjectItem)->objectNext_;
  
  HashedObjectListItem<TKey,TObj> ** ppNewObjectItem;
  ppNewObjectItem = objectHash_.find(newObject);
  if( *ppNewObjectItem != NULL ){
    *ppOldObjectItem = pOldObjectItem;
    throw ExceptionSP(new EHashedObjectList<TKey,TObj>(-1,utf8::string(__PRETTY_FUNCTION__)));
  }
  if( ownsObjects_ ) delete pOldObjectItem->object_;
  pOldObjectItem->object_ = newObject;
  *ppNewObjectItem = pOldObjectItem;
  pOldObjectItem->objectNext_ = NULL;
  return *this;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectList<TKey,TObj> & HashedObjectList<TKey,TObj>::changeIndex(uintptr_t oldIndex,uintptr_t newIndex)
{
  HashedObjectListItem<TKey,TObj> ** ppOldItem, ** ppNewItem;
  ppOldItem = indexHash_.find(oldIndex);
  ppNewItem = indexHash_.find(newIndex);
  assert( *ppOldItem != NULL && *ppNewItem != NULL );
  if( oldIndex != newIndex ){
    HashedObjectListItem<TKey,TObj> * pOldItem, * pNewItem;
    *ppOldItem = (pOldItem = *ppOldItem)->indexNext_;
    *ppNewItem = (pNewItem = *ppNewItem)->indexNext_;

    pOldItem->indexNext_ = NULL;
    pNewItem->indexNext_ = NULL;

    ppOldItem = indexHash_.find(pOldItem->index_ = newIndex);
    assert( *ppOldItem == NULL );
    *ppOldItem = pOldItem;

    ppNewItem = indexHash_.find(pNewItem->index_ = oldIndex);
    assert( *ppNewItem == NULL );
    *ppNewItem = pNewItem;
  }
  return *this;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
uintptr_t HashedObjectList<TKey,TObj>::add(TObj * object,const TKey & key,HashedObjectListItem<TKey,TObj> ** pItem)
{
  AutoPtr<HashedObjectListItem<TKey,TObj> > item(new HashedObjectListItem<TKey,TObj>(key,object,count_));
  uintptr_t i = addHelper(item.ptr());
  if( pItem != NULL ) *pItem = item.ptr();
  item.ptr(NULL);
  return i;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectList<TKey,TObj> & HashedObjectList<TKey,TObj>::removeByKey(const TKey & key)
{
  TObj * object = extractByKey(key);
  if( ownsObjects_ ) delete object;
  return *this;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectList<TKey,TObj> & HashedObjectList<TKey,TObj>::removeByObject(TObj * object)
{
  object = extractByObject(object);
  if( ownsObjects_ ) delete object;
  return *this;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectList<TKey,TObj> & HashedObjectList<TKey,TObj>::removeByIndex(uintptr_t index)
{
  TObj * object = extractByIndex(index);
  if( ownsObjects_ ) delete object;
  return *this;
}
//---------------------------------------------------------------------------
#ifdef __BCPLUSPLUS__
#pragma option push -w-8013
#endif
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
TObj * HashedObjectList<TKey,TObj>::extractByKey(const TKey & key)
{
  HashedObjectListItem<TKey,TObj> ** pKeyItem, ** pObjectItem, ** pIndexItem;
  pKeyItem = keyHash_.find(key,caseSensitive_);
  pObjectItem = objectHash_.find((*pKeyItem)->object_);
  pIndexItem = indexHash_.find((*pKeyItem)->index_);
  if( *pKeyItem == NULL || *pObjectItem == NULL || *pIndexItem == NULL )
    throw ExceptionSP(new EHashedObjectList<TKey,TObj>(-1,utf8::string(__PRETTY_FUNCTION__)));
  HashedObjectListItem<TKey,TObj> * pItem = *pKeyItem;
  *pKeyItem = (*pKeyItem)->keyNext_;
  *pObjectItem = (*pObjectItem)->objectNext_;
  *pIndexItem = (*pIndexItem)->indexNext_;
  uintptr_t index = pItem->index_;
  TObj * object = pItem->object_;
  delete pItem;
  renumeration(index);
  count_--;
  if( (count_ >> reHashThreshold_) <= keyHash_.slotCount_ && keyHash_.slotCount_ > 1 )
    reHash(keyHash_.slotCount_ >> 1,reHashThreshold_);
  return object;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
TObj * HashedObjectList<TKey,TObj>::extractByObject(TObj * object)
{
  if( object != NULL ){
    HashedObjectListItem<TKey,TObj> ** pKeyItem, ** pObjectItem, ** pIndexItem;
    pObjectItem = objectHash_.find(object);
    pKeyItem = keyHash_.find((*pObjectItem)->key_,caseSensitive_);
    pIndexItem = indexHash_.find((*pObjectItem)->index_);
    if( *pObjectItem == NULL || *pKeyItem == NULL || *pIndexItem == NULL )
      throw ExceptionSP(
        new EHashedObjectList<TKey,TObj>(-1,utf8::string(__PRETTY_FUNCTION__))
      );
    HashedObjectListItem<TKey,TObj> * pItem = *pKeyItem;
    *pKeyItem = (*pKeyItem)->keyNext_;
    *pObjectItem = (*pObjectItem)->objectNext_;
    *pIndexItem = (*pIndexItem)->indexNext_;
    uintptr_t index = pItem->index_;
    delete pItem;
    renumeration(index);
    count_--;
    if( (count_ >> reHashThreshold_) <= keyHash_.slotCount_ && keyHash_.slotCount_ > 1 )
      reHash(keyHash_.slotCount_ >> 1,reHashThreshold_);
  }
  return object;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
TObj * HashedObjectList<TKey,TObj>::extractByIndex(uintptr_t index)
{
  HashedObjectListItem<TKey,TObj> ** pKeyItem, ** pObjectItem, ** pIndexItem;
  pIndexItem = indexHash_.find(index);
  pObjectItem = objectHash_.find((*pIndexItem)->object_);
  pKeyItem = keyHash_.find((*pIndexItem)->key_,caseSensitive_);
  if( *pIndexItem == NULL || *pObjectItem == NULL || *pKeyItem == NULL )
    throw ExceptionSP(new EHashedObjectList<TKey,TObj>(-1,utf8::string(__PRETTY_FUNCTION__)));
  HashedObjectListItem<TKey,TObj> * pItem = *pKeyItem;
  *pKeyItem = (*pKeyItem)->keyNext_;
  *pObjectItem = (*pObjectItem)->objectNext_;
  *pIndexItem = (*pIndexItem)->indexNext_;
  TObj * object = pItem->object_;
  delete pItem;
  renumeration(index);
  count_--;
  if( (count_ >> reHashThreshold_) <= keyHash_.slotCount_ && keyHash_.slotCount_ > 1 )
    reHash(keyHash_.slotCount_ >> 1,reHashThreshold_);
  return object;
}
//---------------------------------------------------------------------------
#ifdef __BCPLUSPLUS__
#pragma option pop
#endif
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
void HashedObjectList<TKey,TObj>::renumeration(uintptr_t index)
{
  if( index < count_ - 1 ){
    HashedObjectListItem<TKey,TObj> ** pIndexItem, * pItem;
    pIndexItem = indexHash_.find(count_ - 1);
    pItem = *pIndexItem;
    assert( pItem != NULL );
    *pIndexItem = pItem->indexNext_;
    pItem->indexNext_ = NULL;
    pItem->index_ = index;
    pIndexItem = indexHash_.find(index);
    *pIndexItem = pItem;
  }
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
uintptr_t HashedObjectList<TKey,TObj>::indexOfObject(TObj * object) const
{
  HashedObjectListItem<TKey,TObj> ** pItem = objectHash_.find(object);
  return *pItem == NULL ? -1 : (*pItem)->index_;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
uintptr_t HashedObjectList<TKey,TObj>::indexOfKey(const TKey & key) const
{
  HashedObjectListItem<TKey,TObj> ** pItem = keyHash_.find(key,caseSensitive_);
  return *pItem == NULL ? -1 : (*pItem)->index_;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
TKey HashedObjectList<TKey,TObj>::keyOfObject(TObj * object) const
{
  HashedObjectListItem<TKey,TObj> ** pItem = objectHash_.find(object);
  return *pItem == NULL ? TKey() : (*pItem)->key_;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj>
#ifndef __BCPLUSPLUS__
inline
#endif
TKey HashedObjectList<TKey,TObj>::keyOfIndex(uintptr_t index) const
{
  HashedObjectListItem<TKey,TObj> ** pItem = indexHash_.find(index);
  return *pItem == NULL ? TKey() : (*pItem)->key_;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
TObj * HashedObjectList<TKey,TObj>::objectOfKey(const TKey & key) const
{
  HashedObjectListItem<TKey,TObj> ** pItem = keyHash_.find(key,caseSensitive_);
  return *pItem == NULL ? NULL : (*pItem)->object_;
}                      
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
TObj * HashedObjectList<TKey,TObj>::objectOfIndex(uintptr_t index) const
{
  HashedObjectListItem<TKey,TObj> ** pItem = indexHash_.find(index);
  return *pItem == NULL ? NULL : (*pItem)->object_;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectListItem<TKey,TObj> * HashedObjectList<TKey,TObj>::itemOfKey(const TKey & key) const
{
  return *keyHash_.find(key,caseSensitive_);
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectListItem<TKey,TObj> * HashedObjectList<TKey,TObj>::itemOfObject(TObj * object) const
{
  return *objectHash_.find(object);
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectListItem<TKey,TObj> * HashedObjectList<TKey,TObj>::itemOfIndex(uintptr_t index) const
{
  return *indexHash_.find(index);
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectList<TKey,TObj> & HashedObjectList<TKey,TObj>::transplantByKey(HashedObjectList & donor,const TKey & key)
{
  HashedObjectListItem<TKey,TObj> ** ppKeyItem0, ** ppObjectItem0, ** ppIndexItem0;
  HashedObjectListItem<TKey,TObj> ** ppKeyItem1, ** ppObjectItem1, ** ppIndexItem1;

  ppKeyItem0 = donor.keyHash_.find(key,donor.caseSensitive_);
  ppObjectItem0 = donor.objectHash_.find((*ppKeyItem0)->object_);
  ppIndexItem0 = donor.indexHash_.find((*ppKeyItem0)->index_);
  if( *ppKeyItem0 == NULL || *ppObjectItem0 == NULL || *ppIndexItem0 == NULL ||
      *ppKeyItem0 != *ppObjectItem0 || *ppKeyItem0 != *ppIndexItem0 )
    throw ExceptionSP(new EHashedObjectList<TKey,TObj>(-1,utf8::string(__PRETTY_FUNCTION__)));

  ppKeyItem1 = keyHash_.find(key,caseSensitive_);
  ppObjectItem1 = objectHash_.find((*ppKeyItem0)->object_);
  if( *ppKeyItem1 != NULL || *ppObjectItem1 != NULL )
    throw ExceptionSP(new EHashedObjectList<TKey,TObj>(-1,utf8::string(__PRETTY_FUNCTION__)));

  HashedObjectListItem<TKey,TObj> * pItem = *ppKeyItem0;

  *ppKeyItem0 = pItem->keyNext_;
  *ppObjectItem0 = pItem->objectNext_;
  *ppIndexItem0 = pItem->indexNext_;
  donor.renumeration(pItem->index_);
  donor.count_--;

  pItem->keyNext_ = NULL;
  pItem->objectNext_ = NULL;
  pItem->indexNext_ = NULL;
  pItem->index_ = count_;

  ppIndexItem1 = indexHash_.find(pItem->index_);

  *ppKeyItem1 = pItem;
  *ppObjectItem1 = pItem;
  *ppIndexItem1 = pItem;
  count_++;
  return *this;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectList<TKey,TObj> & HashedObjectList<TKey,TObj>::transplantByObject(HashedObjectList & donor,TObj * object)
{
  HashedObjectListItem<TKey,TObj> ** ppKeyItem0, ** ppObjectItem0, ** ppIndexItem0;
  HashedObjectListItem<TKey,TObj> ** ppKeyItem1, ** ppObjectItem1, ** ppIndexItem1;

  ppObjectItem0 = donor.objectHash_.find(object);
  ppKeyItem0 = donor.keyHash_.find((*ppObjectItem0)->key_,donor.caseSensitive_);
  ppIndexItem0 = donor.indexHash_.find((*ppObjectItem0)->index_);
  if( *ppObjectItem0 == NULL || *ppKeyItem0 == NULL || *ppIndexItem0 == NULL ||
      *ppObjectItem0 != *ppKeyItem0 || *ppObjectItem0 != *ppIndexItem0 )
    throw ExceptionSP(new EHashedObjectList<TKey,TObj>(-1,utf8::string(__PRETTY_FUNCTION__)));

  ppKeyItem1 = keyHash_.find((*ppKeyItem0)->key_,caseSensitive_);
  ppObjectItem1 = objectHash_.find((*ppKeyItem0)->object_);
  if( *ppKeyItem1 != NULL || *ppObjectItem1 != NULL )
    throw ExceptionSP(new EHashedObjectList<TKey,TObj>(-1,utf8::string(__PRETTY_FUNCTION__)));

  HashedObjectListItem<TKey,TObj> * pItem = *ppKeyItem0;

  *ppKeyItem0 = pItem->keyNext_;
  *ppObjectItem0 = pItem->objectNext_;
  *ppIndexItem0 = pItem->indexNext_;
  donor.renumeration(pItem->index_);
  donor.count_--;

  pItem->keyNext_ = NULL;
  pItem->objectNext_ = NULL;
  pItem->indexNext_ = NULL;
  pItem->index_ = count_;

  ppIndexItem1 = indexHash_.find(pItem->index_);

  *ppKeyItem1 = pItem;
  *ppObjectItem1 = pItem;
  *ppIndexItem1 = pItem;
  count_++;
  return *this;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectList<TKey,TObj> & HashedObjectList<TKey,TObj>::transplantByIndex(HashedObjectList & donor,uintptr_t index)
{
  HashedObjectListItem<TKey,TObj> ** ppKeyItem0, ** ppObjectItem0, ** ppIndexItem0;
  HashedObjectListItem<TKey,TObj> ** ppKeyItem1, ** ppObjectItem1, ** ppIndexItem1;

  ppIndexItem0 = donor.indexHash_.find(index);
  ppObjectItem0 = donor.objectHash_.find((*ppIndexItem0)->object_);
  ppKeyItem0 = donor.keyHash_.find((*ppIndexItem0)->key_,donor.caseSensitive_);
  if( *ppIndexItem0 == NULL || *ppObjectItem0 == NULL || *ppKeyItem0 == NULL ||
      *ppIndexItem0 != *ppObjectItem0 || *ppIndexItem0 != *ppKeyItem0 )
    throw ExceptionSP(new EHashedObjectList<TKey,TObj>(-1,utf8::string(__PRETTY_FUNCTION__)));

  ppKeyItem1 = keyHash_.find((*ppKeyItem0)->key_,caseSensitive_);
  ppObjectItem1 = objectHash_.find((*ppKeyItem0)->object_);
  if( *ppKeyItem1 != NULL || *ppObjectItem1 != NULL )
    throw ExceptionSP(new EHashedObjectList<TKey,TObj>(-1,utf8::string(__PRETTY_FUNCTION__)));

  HashedObjectListItem<TKey,TObj> * pItem = *ppKeyItem0;

  *ppKeyItem0 = pItem->keyNext_;
  *ppObjectItem0 = pItem->objectNext_;
  *ppIndexItem0 = pItem->indexNext_;
  donor.renumeration(pItem->index_);
  donor.count_--;

  pItem->keyNext_ = NULL;
  pItem->objectNext_ = NULL;
  pItem->indexNext_ = NULL;
  pItem->index_ = count_;

  ppIndexItem1 = indexHash_.find(pItem->index_);

  *ppKeyItem1 = pItem;
  *ppObjectItem1 = pItem;
  *ppIndexItem1 = pItem;
  count_++;
  return *this;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
uintptr_t HashedObjectList<TKey,TObj>::reHashThreshold() const
{
  return uintptr_t(1) << reHashThreshold_;
}  
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
const bool & HashedObjectList<TKey,TObj>::ownsObjects() const
{
  return ownsObjects_;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
HashedObjectList<TKey,TObj> & HashedObjectList<TKey,TObj>::ownsObjects(bool owns)
{
  ownsObjects_ = owns;
  return *this;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
const bool & HashedObjectList<TKey,TObj>::caseSensitive() const
{
  return caseSensitive_;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
const uintptr_t & HashedObjectList<TKey,TObj>::count() const
{
  return count_;
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
uintptr_t HashedObjectList<TKey,TObj>::maxChainLength(HashChainType chain) const
{
  return (&keyHash_ + chain)->maxChainLength(chain);
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
uintptr_t HashedObjectList<TKey,TObj>::minChainLength(HashChainType chain) const
{
  return (&keyHash_ + chain)->minChainLength(chain);
}
//---------------------------------------------------------------------------
template <class TKey,class TObj> inline
uintptr_t HashedObjectList<TKey,TObj>::avgChainLength(HashChainType chain) const
{
  return (&keyHash_ + chain)->avgChainLength(chain);
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

