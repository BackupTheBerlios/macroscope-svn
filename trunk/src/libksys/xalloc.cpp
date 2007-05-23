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
#include <adicpp/ksys.h>
//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------------
#if !HAVE__MALLOC_OPTIONS
const char * _malloc_options;
#endif
//---------------------------------------------------------------------------
#ifdef __cplusplus
};
#endif
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
#define NODEBUGMEM 1
//---------------------------------------------------------------------------
void * kmalloc(size_t size,bool noThrow)
{
  void * p = NULL;
  if( size > 0 ){
#if !defined(NDEBUG) && !defined(NODEBUGMEM)
    p = malloc(size + sizeof(uintptr_t));
#else
    p = malloc(size);
#endif
    if( p == NULL && !noThrow ){
      newObjectV1C2<EOutOfMemory>(
#if defined(__WIN32__) || defined(__WIN64__)
        ERROR_NOT_ENOUGH_MEMORY + errorOffset,
#else
        ENOMEM,
#endif
        __PRETTY_FUNCTION__ + utf8::String(" ") + utf8::int2Str(size)
      )->throwSP();
    }
#if !defined(NDEBUG) && !defined(NODEBUGMEM)
    *(uintptr_t *) p = size;
    memset((uintptr_t *) p + 1,0xFF,size);
    p = (uintptr_t *) p + 1;
#endif
  }
  return p;
}
//---------------------------------------------------------------------------
void * krealloc(void * p,size_t size,bool noThrow)
{
  if( p == NULL ) return kmalloc(size,noThrow);
  void * a;
  if( size == 0 ){
    kfree(p);
    a = NULL;
  }
  else {
#if !defined(NDEBUG) && !defined(NODEBUGMEM)
    a = realloc((uintptr_t *) p - 1,size + sizeof(uintptr_t));
#else
    a = realloc(p,size);
#endif
    if( a == NULL && !noThrow )
      newObjectV1C2<EOutOfMemory>(
#if defined(__WIN32__) || defined(__WIN64__)
        ERROR_NOT_ENOUGH_MEMORY + errorOffset,
#else
        ENOMEM,
#endif
        utf8::String(__PRETTY_FUNCTION__) + " " + utf8::int2Str(size)
      )->throwSP();
#if !defined(NDEBUG) && !defined(NODEBUGMEM)
    if( size > *(uintptr_t *) a )
      memset((uint8_t *) ((uintptr_t *) a + 1) + *(uintptr_t *) a,0xFF,size - *(uintptr_t *) a);
    *(uintptr_t *) a = size;
    a = (uintptr_t *) a + 1;
#endif
  }
  return a;
}
//---------------------------------------------------------------------------
void kfree(void * p)
{
  if( p != NULL ){
#if !defined(NDEBUG) && !defined(NODEBUGMEM)
    memset((uintptr_t *) p - 1,0xFF,*((uintptr_t *) p - 1) + sizeof(uintptr_t));
    p = (uintptr_t *) p - 1;
#endif
    free(p);
  }
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// heap manager /////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class HeapManager {
  public:
    void * malloc(uintptr_t size,bool lock);
    void * realloc(void * ptr,uintptr_t size,bool lock);
    void free(void * ptr);
  protected:
    class Cluster {
      public:
        Cluster(void * memory = NULL) : memory_(memory), size_(0), bsize_(0), index_(~uintptr_t(0)), head_(NULL), next_(NULL), locked_(false) {}
	
        /*static EmbeddedHashNode<Cluster,uintptr_t> & ehNLT(const uintptr_t & link,uintptr_t &){
          return *reinterpret_cast<EmbeddedHashNode<Cluster,uintptr_t> *>(link);
	}
	static uintptr_t ehLTN(const EmbeddedHashNode<Cluster,uintptr_t> & node,uintptr_t &){
	  return reinterpret_cast<uintptr_t>(&node);
	}
	static EmbeddedHashNode<Cluster,uintptr_t> & keyNode(const Cluster & object){
	  return object.keyNode_;
	}
	static Cluster & keyNodeObject(const EmbeddedHashNode<Cluster,uintptr_t> & node,Cluster * p){
	  return node.object(p->keyNode_);
	}
	static uintptr_t keyNodeHash(const Cluster & object){
	  return HF::hash(&object.memory_);
	}
        static bool keyHashNodeEqu(const Cluster & object1,const Cluster & object2){
	  return object1.memory_ == object2.memory_;
        }
        mutable EmbeddedHashNode<Cluster,uintptr_t> keyNode_;*/

        static RBTreeNode & treeO2N(const Cluster & object){
          return object.treeNode_;
        }
	static Cluster & treeN2O(const RBTreeNode & node){
          Cluster * p = NULL;
          return node.object<Cluster>(p->treeNode_);
        }
        static intptr_t treeCO(const Cluster & a0,const Cluster & a1){
          return a0.memory_ < a1.memory_ ? -1 : a0.memory_ + a0.size_ >= a1.memory_ + a1.size_ ? 1 : 0;
        }
        mutable RBTreeNode treeNode_;
																								
        static EmbeddedListNode<Cluster> & listNode(const Cluster & object){
	  return object.listNode_;
	}
	static Cluster & listObject(const EmbeddedListNode<Cluster> & node,Cluster * p = NULL){
	  return node.object(p->listNode_);
	} 
	mutable EmbeddedListNode<Cluster> listNode_;

        Cluster & initialize(){
          index_ = 0;
          uintptr_t i, j = size_ / bsize_;
          if( size == 1 ){
            if( j > 255 ) j = 255;
            for( i = 0; i < j; i++ )
              *(uint8_t *) (memory_ + i) = uint8_t(i + 1);
            *(uint8_t *) (memory_ + i) = ~uint8_t(0);
          }
          else if( size == 2 ){
            if( j > 65535 ) j = 65535;
            for( i = 0; i < j; i++ )
              *(uint16_t *) (memory_ + i * 2) = uint16_t(i + 1);
            *(uint16_t *) (memory_ + i * 2) = ~uint16_t(0);
          }
          else if( size == 3 ){
            if( j > 65535 ) j = 16777215;
            for( i = 0; i < j; i++ )
              *(uint32_t *) (memory_ + i * 3) = uint32_t(i + 1);
            *(uint32_t *) (memory_ + i * 3) = ~uint32_t(0);
          }
          else if( size > ~uint32_t(0) - 1 ){
            for( i = 0; i < j; i++ )
              *(uintptr_t *) (memory_ + bsize_ * i) = i + 1;
            *(uintptr_t *) (memory_ + bsize_ * i) = ~uintptr_t(0);
          }
          else {
            for( i = 0; i < j; i++ )
              *(uint32_t *) (memory_ + bsize_ * i) = uint32_t(i + 1);
            *(uint32_t *) (memory_ + bsize_ * i) = ~uint32_t(0);
          }
          return *this;
	}
	
	uint8_t * memory_;
	uintptr_t size_; // size of allocated memory
	uintptr_t bsize_; // size of allocating memory block
	uintptr_t index_; // first in free list block index
	Cluster * head_; // head of allocated memory for small blocks (with size < 4)
	Cluster * next_; // next memory for small blocks (with size < 4)
	bool locked_;
    };
    class Clusters {
      public:
        static EmbeddedListNode<Clusters> & listNode(const Clusters & object){
	  return object.listNode_;
	}
	static Clusters & listObject(const EmbeddedListNode<Clusters> & node,Clusters * p = NULL){
	  return node.object(p->listNode_);
	} 
	mutable EmbeddedListNode<Clusters> listNode_;

        EmbeddedList<Cluster,Cluster::listNode,Cluster::listObject> free_; // free slots
        EmbeddedList<Cluster,Cluster::listNode,Cluster::listObject> lru_; // for fast allocating

	uintptr_t count() const { return (size_ - sizeof(Clusters) + sizeof(Cluster)) / sizeof(Cluster); }

	uintptr_t size_; // size of allocated memory
	Cluster clusters_[1];
    };
    class SizeDescriptor {
      public:
	uintptr_t size_; // size of allocating memory block
        EmbeddedList<Clusters,Clusters::listNode,Clusters::listObject> clusters_;
    };
    class SizeDescriptors {
      public:
        static EmbeddedListNode<SizeDescriptors> & listNode(const SizeDescriptors & object){
	  return object.listNode_;
	}
	static SizeDescriptors & listObject(const EmbeddedListNode<SizeDescriptors> & node,SizeDescriptors * p = NULL){
	  return node.object(p->listNode_);
	} 
	mutable EmbeddedListNode<SizeDescriptors> listNode_;
	
	uintptr_t count() const { return (size_ - sizeof(SizeDescriptors) + sizeof(SizeDescriptor)) / sizeof(SizeDescriptor); }
	
	uintptr_t size_; // size of allocated memory
	SizeDescriptor sizes_[1];
    };
    EmbeddedList<SizeDescriptors,SizeDescriptors::listNode,SizeDescriptors::listObject> sizes_;
    EmbeddedList<Clusters,Clusters::listNode,Clusters::listObject> clusters_;
    /*typedef EmbeddedHash<
      Cluster,
      uintptr_t,
      uintptr_t,
      Cluster::ehNLT,
      Cluster::ehLTN,
      Cluster::keyNode,
      Cluster::keyNodeObject,
      Cluster::keyNodeHash,
      Cluster::keyHashNodeEqu
    > ClustersHash;
    ClustersHash clustersHash_;*/
    typedef RBTree<
      Cluster,
      Cluster::treeO2N,
      Cluster::treeN2O,
      Cluster::treeCO
    > ClustersTree;
    ClustersTree clustersTree_;
    uintptr_t clusterSize_;
    uintptr_t align_;
    uintptr_t allocatedSystemMemory_;
    uintptr_t allocatedMemory_;
#if defined(__WIN32__) || defined(__WIN64__)
    DWORD flags_;
#endif

    void * sysalloc(uintptr_t size,bool lock,bool & locked,bool noThrow);
    HeapManager & sysfree(const void * memory,uintptr_t size);
  private:
};
//---------------------------------------------------------------------------
HeapManager::~HeapManager()
{
  clear();
}
//---------------------------------------------------------------------------
HeapManager::HeapManager()
{
#if defined(__WIN32__) || defined(__WIN64__)
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  clusterSize_ = GetLargePageMinimum();
  flags_ = MEM_COMMIT | MEM_LARGE_PAGES;
  if( clusterSize_ < si.dwPageSize ){
    clusterSize_ = si.dwPageSize;
    flags_ = MEM_COMMIT;
  }
#else
  clusterSize_ = getpagesize();
#endif
  align_ = 1;
}
//---------------------------------------------------------------------------
HeapManager & HeapManager::clear()
{
  return *this;
}
//---------------------------------------------------------------------------
EmbeddedListNode<SizeDescriptors> * HeapManager::findSDSNode(uintptr_t size,uintptr_t & fsize,uintptr_t & lsize)
{
  fsize = lsize = 0;
  EmbeddedListNode<SizeDescriptors> * sdsNode = sizes_.first();
  while( sdsNode != NULL ){
    SizeDescriptors * sds = SizeDescriptors::listObject(*sdsNode);
    fsize = sds->sizes_[0].size_;
    lsize = sds->sizes_[sds->count() - 1].size_;
    if( size < fsize || size <= lsize ) break;
    sdsNode = sdsNode->next();
  }
  return sdsNode;
}
//---------------------------------------------------------------------------
EmbeddedListNode<Clusters> * HeapManager::findCSNode(SizeDescriptor * sd,uintptr_t size)
{
  EmbeddedListNode<Clusters> * csNode = sd->clusters_.first();
  Clusters * cs;
  while( csNode != NULL ){
    cs = &Clusters::listObject(*csNode);
    if( (cs->lru_.count() > 0 && Cluster::listObject((*cs->lru_.first()).index_ != ~uintptr_t(0)) ||
        (size > 3 && cs->free_.count() > 0) ||
	(size == 3 && cs->free_.count() > (clusterSize_ / 0x1000000u) - (clusterSize_ % 0x1000000u == 0)) ||
	(size == 2 && cs->free_.count() > (clusterSize_ / 0x10000u) - (clusterSize_ % 0x10000u == 0)) ||
	(size == 1 && cs->free_.count() > (clusterSize_ / 0x100u) - (clusterSize_ % 0x100u == 0))
    ) break;
    csNode = csNode->next();
  }
}
//---------------------------------------------------------------------------
void * HeapManager::malloc(uintptr_t size,bool lock)
{
  if( size == 0 ) return NULL;
  size += -intptr_t(size) & (align_ - 1);
  uintptr_t fsize, lsize;
  EmbeddedListNode<SizeDescriptors> * sdsNode = findSDSNode(size,fsize,lsize);
  SizeDescriptors * sds;
  if( sdsNode == NULL || size < fsize ){
    bool locked;
    sds = (SizeDescriptors *) sysalloc(clusterSize_,lock,locked,true);
    if( sds == NULL ) return NULL;
    new (sds) SizeDescriptors;
    sds->size_ = clusterSize;
    uintptr_t k = sds->count(), j = (size / k) * k + 1; // zero size not used
    for( intptr_t i = k - 1; i >= 0; i-- ){
      new (sds->sizes_ + i) SizeDescriptor;
      sds->sizes_[i].size_ = j + i;
    }
    if( sdsNode == NULL ) sizes_.insToTail(*sds); else sizes_.insBefore(*sdsNode,*sds);
    sdsNode = &SizeDescriptors::listNode(*sds);
  }
  sds = &SizeDescriptors::listObject(*sdsNode);
  SizeDescriptor * sd = sds->sizes_[(size - sds->sizes_[0].size_) / align_];
  EmbeddedListNode<Clusters> * csNode = findCSNode(sd,size);
  Clusters * cs;
  if( csNode == NULL ){
    bool locked;
    cs = (Clusters *) sysalloc(clusterSize_,lock,locked,true);
    if( cs == NULL ) return NULL;
    new (cs) Clusters;
    cs->size_ = clusterSize;
    for( intptr_t i = cs->count() - 1; i >= 0; i-- ){
      new (cs->clusters_ + i) Cluster;
      cs->free_.insToHead(cs->clusters_[i]);
    }
    sd.clusters_.insToTail(*sds);
    csNode = &Clusters::listNode(*cs);
  }
  cs = Clusters::listObject(*csNode);
  EmbeddedList<Cluster,Cluster::listNode,Cluster::listObject> & list = cs->lru_.count() > 0 ? cs->lru_ : cs->free_;
  uintptr_t cc = 1, bs = 0;
  if( size >= 4 ){
  }
  else if( size == 3 && clusterSize_ > 0x1000000u ){
    cc = clusterSize_ / 0x1000000u + (clusterSize_ % 0x1000000u == 0);
    bs = 0x1000000u;
  }
  else if( size == 2 && clusterSize_ > 0x10000u ){
    cc = clusterSize_ / 0x10000u + (clusterSize_ % 0x10000u == 0);
    bs = 0x10000u;
  }
  else if( size == 1 && clusterSize_ > 0x100u ){
    cc = clusterSize_ / 0x100u + (clusterSize_ % 0x100u == 0);
    bs = 0x100u;
  }
  Clusters * c, * chead = NULL;
  for( uintptr_t i = 0; i < cc; i++ ){
    c = &Cluster::listObject((*list.first());
    if( c->memory_ == NULL ){
      if( chead == NULL ){
        bool locked;
        c->size_ = size > clusterSize_ ? size + (-intptr_t(size) & (clusterSize_ - 1)) : clusterSize_;
	c->bsize_ = size;
        c->memory_ = (uint8_t *) sysalloc(c->size_,lock,locked,true);
	if( c->memory_ == NULL ) return NULL;
        c->locked_ = locked;
        c->initialize(size);
        chead = c;
        list.remove(*c);
      }
      else {
        c->size_ = chead->size_;
        c->bsize_ = chead->bsize_;
        c->memory_ = chead->memory_ + i * bs;
	c->head_ = chead;
	c->next_ = chead->next_;
	chead->next_ = c;
        c->initialize(size);
        cs->free_.remove(*c);
	cs->lru_.insToHead(*c);
      }
      clustersTree_.insert(*c);
    }
    else {
      chead = c;
      list.remove(*c);
    }
  }
  c = chead;
  void * p = c->memory_ + c->index_ * size;
  uintptr_t next;
  if( size == 1 ){
    next = *(uint8_t *) (c->memory_ + c->index_);
    if( next == 0xFF ) next = ~uintptr_t(0);
  }
  else if( size == 2 ){
    next = *(uint16_t *) (c->memory_ + c->index_ * 2);
    if( next == 0xFFFF ) next = ~uintptr_t(0);
  }
  else if( size == 3 ){
    next = *(uint32_t *) (c->memory_ + c->index_ * 3) & 0xFFFFFF;
    if( next == 0xFFFFFF ) next = ~uintptr_t(0);
  }
  else if( size > ~uint32_t(0) - 1 ){
    next = *(uintptr_t *) (c->memory_ + c->index_ * size);
  }
  else {
    next = *(uint32_t *) (c->memory_ + c->index_ * size);
    if( next == 0xFFFFFFFF ) next = ~uintptr_t(0);
  }
  c->index_ = next;
  if( next == ~uintptr_t(0) ) cs->lru_.insToTail(*c); else cs->lru_.insToHead(*c);
  allocatedMemory_ += size;
  return p;
}
//---------------------------------------------------------------------------
void * HeapManager::realloc(void * ptr,uintptr_t size,bool lock)
{
  if( size == 0 ){
    if( ptr != NULL ) free(ptr);
    return NULL;
  }
  size += -intptr_t(size) & (align_ - 1);
  Cluster c(ptr), * cp = clustersTree_.find(c);
  if( cp == NULL ){
#if defined(__WIN32__) || defined(__WIN64__)
    SetLastError(ERROR_INVALID_DATA);
#else
    errno = EINVAL;
#endif
    int32_t err = oserror() + errorOffset;
    Exception e(err,__PRETTY_FUNCTION__);
    e.writeStdError();
    return NULL;
  }
  if( cp->bsize_ >= size ) return ptr;
  void * p = malloc(size,lock);
  if( p != NULL ){
    memcpy(p,ptr,cp->bsize_);
    free(ptr);
  }
  return p;
}
//---------------------------------------------------------------------------
void HeapManager::free(const void * ptr)
{
  Cluster c(ptr), * cp = clustersTree_.find(c);
  if( cp == NULL ){
#if defined(__WIN32__) || defined(__WIN64__)
    SetLastError(ERROR_INVALID_DATA);
#else
    errno = EINVAL;
#endif
    int32_t err = oserror() + errorOffset;
    Exception e(err,__PRETTY_FUNCTION__);
    e.writeStdError();
    return NULL;
  }
  
}
//---------------------------------------------------------------------------
HeapManager & HeapManager::sysfree(const void * memory,uintptr_t size)
{
  if( memory != NULL ){
#if defined(__WIN32__) || defined(__WIN64__)
    VirtualUnlock(memory,size);
    VirtualFree(memory,size);
#else
    munlock(memory,size);
    ::free(memory,size);
#endif
    usedMemory_ -= size;
  }
  return *this;
}
//---------------------------------------------------------------------------
void * HeapManager::sysalloc(uintptr_t size,bool lock,bool & locked,bool noThrow)
{
  void * memory = NULL;
#if defined(__WIN32__) || defined(__WIN64__)
  memory = VirtualAlloc(NULL,size,flags_,0);
  if( memory == NULL ){
    int32_t err = GetLastError() + errorOffset;
    if( !noThrow ) newObjectV1C2<EOutOfMemory>(err,__PRETTY_FUNCTION__)->throwSP();    
  }
  else {
    if( lock && !(locked = VirtualLock(memory,size) != 0) ){
      int32_t err = GetLastError() + errorOffset;
      Exception e(err,__PRETTY_FUNCTION__);
      e.writeStdError();
    }
    allocatedSystemMemory_ += size;
  }
#else
  memory = ::malloc(size);
  if( memory == NULL ){
    int32_t err = errno;
    if( !noThrow ) newObjectV1C2<EOutOfMemory>(err,__PRETTY_FUNCTION__)->throwSP();    
  }
  else {
    if( lock && !(locked = mlock(memory,size) == 0) ){
      int32_t err = errno;
      Exception e(err,__PRETTY_FUNCTION__);
      e.writeStdError();
    }
    allocatedSystemMemory_ += size;
  }
#endif
  assert( (memory & (clusterSize_ - 1)) == 0 ); // check address align
  return memory;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
