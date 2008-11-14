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
template <typename T,typename LT> class EmbeddedHashNode {
  public:
    ~EmbeddedHashNode();
    EmbeddedHashNode();

    LT & next() const;
    T & object(const EmbeddedHashNode<T,LT> & node) const;
  protected:
    mutable LT next_;
  private:
};
//---------------------------------------------------------------------------
template <typename T,typename LT> inline
EmbeddedHashNode<T,LT>::~EmbeddedHashNode()
{
}
//---------------------------------------------------------------------------
template <typename T,typename LT> inline
EmbeddedHashNode<T,LT>::EmbeddedHashNode()
{
}
//---------------------------------------------------------------------------
template <typename T,typename LT> inline
LT & EmbeddedHashNode<T,LT>::next() const
{
  return next_;
}
//---------------------------------------------------------------------------
template <typename T,typename LT> inline
T & EmbeddedHashNode<T,LT>::object(const EmbeddedHashNode<T,LT> & node) const
{
  return *(T *) const_cast<uint8_t *>((const uint8_t *) this - (uintptr_t) const_cast<EmbeddedHashNode<T,LT> *>(&node));
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <
  typename T,
  typename LT,
  typename LPT,
  EmbeddedHashNode<T,LT> & NLT(const LT & link,LPT & param), // must return node reference by node link type
  LT LTN(const EmbeddedHashNode<T,LT> & node,LPT & param),   // must link value from node reference
  EmbeddedHashNode<T,LT> & (*N) (const T &),                 // must return node embedded in object
  T & (*O) (const EmbeddedHashNode<T,LT> &,T *),             // must return object of node embedded
  uintptr_t (*H)(const T &),                                 // must return computed hash of object key
  bool (*E) (const T &, const T &),                          // must return true if objects keys equals
  class D = AutoPtrClassDestructor<T>
>
class EmbeddedHash {
  // member methods declared inside class for borland (bugland) compiler compatibility
  public:
    virtual ~EmbeddedHash() { clear(); }
    EmbeddedHash() :
      hash_(staticHash_),
      size_(staticHashCount()),
      count_(0),
      estimatedChainLength_(16),
      thresholdNumerator_(10),
      thresholdDenominator_(18)
    {
      clearStaticHash();
    }

    EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> & xchg(const EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> & a) const
    {
      LT * hash = hash_;
      if( a.hash_ != a.staticHash_ )
        hash_ = a.hash_; else hash_ = staticHash_;
      if( hash != staticHash_ )
        a.hash_ = hash; else a.hash_ = a.staticHash_;
      xchgStaticHash(a);
      ksys::xchg(size_,a.size_);
      ksys::xchg(count_,a.count_);
      ksys::xchg(estimatedChainLength_,a.estimatedChainLength_);
      ksys::xchg(thresholdNumerator_,a.thresholdNumerator_);
      ksys::xchg(thresholdDenominator_,a.thresholdDenominator_);
      ksys::xchg(param_,a.param_);
      return *const_cast<EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> *>(this);
    }

    EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> & clear() const
    {
      if( size_ > staticHashCount() ) kfree(hash_);
      hash_ = staticHash_;
      clearStaticHash();
      size_ = staticHashCount();
      count_ = 0;
      return *const_cast<EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> *>(this);
    }

    EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> & insert(const T & object,bool throwIfExist = true,bool deleteIfExist = true,T ** pObject = NULL) const
    {
      LT * head = internalFind(object,throwIfExist,false,deleteIfExist,false);
      if( *head == (LT) NULL ){
        if( pObject != NULL ) *pObject = const_cast<T *>(&object);
        *head = LTN(N(object),param_);
        N(object).next() = LT(NULL);
        optimize(optInc);
      }
      else if( pObject != NULL ){
        *pObject = &O(NLT(*head,param_),NULL);
      }
      return *const_cast<EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> *>(this);
    }

    T & remove(const T & object,bool throwIfNotExist = true,bool deleteIfNotExist = true) const
    {
      LT * head = internalFind(object,false,throwIfNotExist,false,deleteIfNotExist), node;
      if( *head != (LT) NULL ){
        node = *head;
        *head = NLT(node,param_).next();
        NLT(node,param_).next() = LT(NULL);
        optimize(optDec);
        return O(NLT(node,param_),NULL);
      }
      return *const_cast<T *>(&object);
    }

    T & remove() const
    {
      if( count_ > 0 ){
        LT * p1 = hash_, * p2 = p1 + size_, node;
        while( p1 < p2 && *p1 == NULL ) p1++;
        node = *p1;
        *p1 = NLT(node,param_).next();
        NLT(node,param_).next() = LT(NULL);
        optimize(optDec);
        return O(NLT(node),NULL);
      }
      return *(T *) NULL;
    }

    T & first() const
    {
      assert( count_ > 0 );
      LT * p1 = hash_, * p2 = p1 + size_;
      while( p1 < p2 && *p1 == NULL ) p1++;
      return O(NLT(*p1,param_),NULL);
    }

    EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> & drop() const
    {
      LT * head = hash_, * tail = head + size_, walk, w;
      while( head < tail ){
        walk = *head;
        while( walk != (LT) NULL ){
          w = NLT(walk,param_).next();
          D::destroyObject(&O(NLT(walk,param_),NULL));
          walk = w;
        }
        head++;
      }
      return clear();
    }

    EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> & drop(T & object,bool throwIfNotExist = true) const
    {
      D::destroyObject(&remove(object,throwIfNotExist));
      return *const_cast<EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> *>(this);
    }

    T & search(const T & object,bool deleteIfExist = false,bool deleteIfNotExist = false) const
    {
      return NLT(*internalFind(object,false,true,deleteIfExist,deleteIfNotExist),param_)->object();
    }

    T * find(const T & object,bool deleteIfExist = false,bool deleteIfNotExist = false) const
    {
      LT * p = internalFind(object,false,false,deleteIfExist,deleteIfNotExist);
      return *p == (LT) NULL ? (T *) NULL : &O(NLT(*p,param_),NULL);
    }

    EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> & list(Array<T *> & l) const
    {
      l.resize(count_);
      intptr_t i = 0;
      LT * head = hash_, * tail = head + size_, walk;
      while( head < tail ){
        walk = *head;
        while( walk != (LT) NULL ){
          l[i++] = &O(NLT(walk,param_),NULL);
          walk = NLT(walk,param_).next();
        }
        head++;
      }
      return *const_cast<EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> *>(this);
    }

    const uintptr_t & count() const { return count_; }
    LPT & param() const { return param_; }

    const uintptr_t & estimatedChainLength() const { return estimatedChainLength_; }
    const uintptr_t & thresholdNumerator() const { return thresholdNumerator_; }
    const uintptr_t & thresholdDenominator() const { return thresholdDenominator_; }

    EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> & estimatedChainLength(uintptr_t a) const
    {
      estimatedChainLength_ = a;
      return *const_cast<EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> *>(this);
    }

    EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> & thresholdNumerator(uintptr_t a) const
    {
      thresholdNumerator_ = a;
      return *const_cast<EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> *>(this);
    }

    EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> & thresholdDenominator(uintptr_t a) const
    {
      thresholdDenominator_ = a;
      return *const_cast<EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> *>(this);
    }

    uintptr_t maxChainLength() const
    {
      uintptr_t max = 0, m;
      for( intptr_t i = size_ - 1; i >= 0; i-- ){
        LT * p;
        for( m = 0, p = hash_ + i; *p != NULL; p = &NLT(*p,param_).next(), m++ );
        if( m > max ) max = m;
      }
      return max;
    }

    uintptr_t minChainLength() const
    {
      uintptr_t min = ~(uintptr_t) 0, m;
      for( intptr_t i = size_ - 1; i >= 0; i-- ){
        LT * p;
        for( m = 0, p = hash_ + i; *p != NULL; p = &NLT(*p,param_).next(), m++ );
        if( m < min ) min = m;
      }
      return min;
    }

    uintptr_t avgChainLength() const
    {
      uintptr_t avg = 0, m;
      for( intptr_t i = size_ - 1; i >= 0; i-- ){
        LT * p;
        for( m = 0, p = hash_ + i; *p != NULL; p = &NLT(*p,param_).next(), m++ );
        avg += m;
      }
      return size_ > 0 ? avg / size_ : 0;
    }

    template <typename ST> ST & put(ST & stream) const
    {
      Array<T *> lst;
      list(lst);
      uint64_t u = lst.count();
      stream << u;
      while( u-- > 0 ) stream << *lst[(uintptr_t) u];
      return stream;
    }

    template <typename ST> ST & get(ST & stream) const
    {
      EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> t;
      uint64_t u;
      stream >> u;
      while( u-- > 0 ){
        AutoPtr<T> p(newObject<T>());
        stream >> *p.ptr();
        t.insert(*p.ptr());
        p.ptr(NULL);
      }
      drop();
      xchg(t);
      return stream;
    }
  protected:
  private:
    mutable LT * hash_;
    mutable LT staticHash_[16];
    mutable uintptr_t size_;
    mutable uintptr_t count_;
    mutable uintptr_t estimatedChainLength_;
    mutable uintptr_t thresholdNumerator_;
    mutable uintptr_t thresholdDenominator_;
    mutable LPT param_;

    EmbeddedHash(const EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> &);
    void operator = (const EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> &);

    uintptr_t staticHashCount() const
    {
      return sizeof(staticHash_) / sizeof(staticHash_[0]);
    }

    void clearStaticHash() const
    {
      for( intptr_t i = staticHashCount() - 1; i >= 0; i-- ) staticHash_[i] = (LT) NULL;
    }

    void xchgStaticHash(const EmbeddedHash<T,LT,LPT,NLT,LTN,N,O,H,E,D> & a) const
    {
      for( intptr_t i = staticHashCount() - 1; i >= 0; i-- ) ksys::xchg(staticHash_[i],a.staticHash_[i]);
    }

    LT * internalFind(const T & object,bool throwIfExist,bool throwIfNotExist,bool deleteIfExist,bool deleteIfNotExist) const
    {
      LT * head = hash_ + (H(object) & (size_ - 1));
      while( *head != (LT) NULL ){
        if( E(O(NLT(*head,param_),NULL),object) ) break;
        head = &NLT(*head,param_).next();
      }
      int32_t err = 0;
      if( *head != (LT) NULL ){
        if( deleteIfExist ) D::destroyObject(&object);
        if( throwIfExist ){
#if defined(__WIN32__) || defined(__WIN64__)
          err = ERROR_ALREADY_EXISTS;
#else
          err = EEXIST;
#endif
        }
      }
      else if( *head == (LT) NULL ){
        if( deleteIfNotExist ) D::destroyObject(&object);
        if( throwIfNotExist ){
#if defined(__WIN32__) || defined(__WIN64__)
          err = ERROR_NOT_FOUND;
#else
          err = ENOENT;
#endif
        }
      }
      if( err != 0 )
        newObjectV1C2<Exception>(err + errorOffset, __PRETTY_FUNCTION__)->throwSP();
      return head;
    }

    enum OptType { optInc, optDec };

    LT getChain() const
    {
      LT head = (LT) NULL, * p0, * p1, a0, a1;
      p0 = hash_;
      p1 = p0 + size_;
      while( p0 < p1 ){
        a0 = *p0;
        while( a0 != (LT) NULL ){
          a1 = NLT(a0,param_).next();
          NLT(a0,param_).next() = head;
          head = a0;
          a0 = a1;
        }
        *p0++ = LT(NULL);
      }
      return head;
    }
    
    void putChain(LT head) const
    {
      LT * p0, a0;
      while( head != (LT) NULL ){
        a0 = NLT(head,param_).next();
        p0 = internalFind(O(NLT(head,param_),NULL),false,false,false,false);
        assert( p0 != NULL && *p0 == (LT) NULL );
        *p0 = head;
        NLT(head,param_).next() = LT(NULL);
        head = a0;
      }
    }

    void optimize(OptType o) const
    {
      LT head, * a;
// using golden section == 5/8 as optimization threshold
//  uintptr_t count = size_ << 4, c = ((count << 2) + count) >> 3, i;
      uintptr_t count = size_ * estimatedChainLength_, c = count * thresholdNumerator_ / thresholdDenominator_, i;
      switch( o ){
        case optInc :
          if( ++count_ < count + c ) return;
          assert( size_ >= staticHashCount() );
          i = size_ << 1;
          if( size_ > staticHashCount() ){
            a = (LT *) krealloc(hash_,i * sizeof(LT),true);
          }
          else {
            a = (LT *) kmalloc(i * sizeof(LT),true);
            memcpy(a,staticHash_,sizeof(staticHash_));
          }
          if( a != NULL ){
            hash_ = a;
            head = getChain();
            while( size_ < i ) a[size_++] = LT(NULL);
            putChain(head);
          }
          break;
        case optDec :
          if( --count_ > count - c ) return;
          i = size_ >> 1;
          if( i >= staticHashCount() ){
            head = getChain();
            if( i > staticHashCount() ){
              a = (LT *) krealloc(hash_,i * sizeof(LT),true);
              if( a == NULL ) i <<= 1; else hash_ = a;
            }
            else {
              kfree(hash_);
              hash_ = staticHash_;
              clearStaticHash();
            }
            size_ = i;
            putChain(head);
          }
          break;
      }
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
template <typename KeyT,typename ValueT,bool caseV = true>
class EmbeddedHashKey {
  typedef EmbeddedHashKey<KeyT,ValueT,caseV> ClassT;
  public:
    virtual ~EmbeddedHashKey(){}
    EmbeddedHashKey(){}
    EmbeddedHashKey(const ClassT & a) : key_(a.key_), value_(a.value_) {}
    EmbeddedHashKey(const KeyT & key,const ValueT & value = ValueT()) : key_(key), value_(value) {}
    ClassT & operator = (const ClassT & a){ key_ = a.key_; value_ = a.value_; return *this; }
    ClassT & operator = (const ValueT & a){ value_ = a.value_; return *this; }
    bool operator == (const ClassT & a) const { return key_ == a.key_; }
    bool operator != (const ClassT & a) const { return key_ != a.key_; }
    bool operator >= (const ClassT & a) const { return key_ >= a.key_; }
    bool operator >  (const ClassT & a) const { return key_ >  a.key_; }
    bool operator <= (const ClassT & a) const { return key_ <= a.key_; }
    bool operator <  (const ClassT & a) const { return key_ <  a.key_; }

    KeyT key_;
    ValueT value_;

    static EmbeddedHashNode<ClassT,uintptr_t> & ehNLT(const uintptr_t & link,uintptr_t * &){
      return *reinterpret_cast<EmbeddedHashNode<ClassT,uintptr_t> *>(link);
    }
    static uintptr_t ehLTN(const EmbeddedHashNode<ClassT,uintptr_t> & node,uintptr_t * &){
      return reinterpret_cast<uintptr_t>(&node);
    }
    static EmbeddedHashNode<ClassT,uintptr_t> & keyNode(const ClassT & object){
      return object.keyNode_;
    }
    static ClassT & keyNodeObject(const EmbeddedHashNode<ClassT,uintptr_t> & node,ClassT * p){
      return node.object(p->keyNode_);
    }
    static uintptr_t keyNodeHash(const ClassT & object){
      return hash(object.key_,caseV);
    }
    static bool keyHashNodeEqu(const ClassT & object1,const ClassT & object2){
      return compareObjects(object1.key_,object2.key_,caseV) == 0;
    }
  protected:
  private:
    mutable EmbeddedHashNode<ClassT,uintptr_t> keyNode_;
};
//---------------------------------------------------------------------------
#define typedefEmbeddedHashKey(KeyT,ValueT,caseV) \
typedef EmbeddedHashKey<KeyT,ValueT,caseV>
#define typedefEmbeddedHashKeys(KeyT,ValueT,caseV) \
typedef EmbeddedHash<\
  EmbeddedHashKey<KeyT,ValueT,caseV>,\
  uintptr_t,\
  uintptr_t *,\
  EmbeddedHashKey<KeyT,ValueT,caseV>::ehNLT,\
  EmbeddedHashKey<KeyT,ValueT,caseV>::ehLTN,\
  EmbeddedHashKey<KeyT,ValueT,caseV>::keyNode,\
  EmbeddedHashKey<KeyT,ValueT,caseV>::keyNodeObject,\
  EmbeddedHashKey<KeyT,ValueT,caseV>::keyNodeHash,\
  EmbeddedHashKey<KeyT,ValueT,caseV>::keyHashNodeEqu>
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
inline intptr_t hash(const int8_t & a,bool /*caseSensitive*/ = true)
{
  return ksys::HF::hash(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline intptr_t hash(const uint8_t & a,bool /*caseSensitive*/ = true)
{
  return ksys::HF::hash(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline intptr_t hash(const int16_t & a,bool /*caseSensitive*/ = true)
{
  return ksys::HF::hash(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline intptr_t hash(const uint16_t & a,bool /*caseSensitive*/ = true)
{
  return ksys::HF::hash(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline intptr_t hash(const int32_t & a,bool /*caseSensitive*/ = true)
{
  return ksys::HF::hash(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline intptr_t hash(const uint32_t & a,bool /*caseSensitive*/ = true)
{
  return ksys::HF::hash(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline intptr_t hash(const int64_t & a,bool /*caseSensitive*/ = true)
{
  return ksys::HF::hash(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline intptr_t hash(const uint64_t & a,bool /*caseSensitive*/ = true)
{
  return ksys::HF::hash(&a,sizeof(a));
}
//---------------------------------------------------------------------------
inline intptr_t compareObjects(const int8_t & a1,const int8_t & a2,bool /*caseSensitive*/ = true)
{
  return a1 - a2;
}
//---------------------------------------------------------------------------
inline intptr_t compareObjects(const uint8_t & a1,const uint8_t & a2,bool /*caseSensitive*/ = true)
{
  return intptr_t(a1) - intptr_t(a2);
}
//---------------------------------------------------------------------------
inline intptr_t compareObjects(const int16_t & a1,const int16_t & a2,bool /*caseSensitive*/ = true)
{
  return a1 - a2;
}
//---------------------------------------------------------------------------
inline intptr_t compareObjects(const uint16_t & a1,const uint16_t & a2,bool /*caseSensitive*/ = true)
{
  return intptr_t(a1) - intptr_t(a2);
}
//---------------------------------------------------------------------------
inline intptr_t compareObjects(const int32_t & a1,const int32_t & a2,bool /*caseSensitive*/ = true)
{
  return a1 - a2;
}
//---------------------------------------------------------------------------
inline intptr_t compareObjects(const uint32_t & a1,const uint32_t & a2,bool /*caseSensitive*/ = true)
{
  return a1 > a2 ? 1 : a1 < a2 ? -1 : 0;
}
//---------------------------------------------------------------------------
inline intptr_t compareObjects(const int64_t & a1,const int64_t & a2,bool /*caseSensitive*/ = true)
{
  return a1 > a2 ? 1 : a1 < a2 ? -1 : 0;
}
//---------------------------------------------------------------------------
inline intptr_t compareObjects(const uint64_t & a1,const uint64_t & a2,bool /*caseSensitive*/ = true)
{
  return a1 > a2 ? 1 : a1 < a2 ? -1 : 0;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
