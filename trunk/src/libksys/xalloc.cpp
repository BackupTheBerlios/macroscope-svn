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
    ~HeapManager();
    HeapManager();

    HeapManager & clear();

    void * malloc(uintptr_t size,bool lock = false);
    void * realloc(void * ptr,uintptr_t size,bool lock = false);
    void free(void * ptr);
    const uintptr_t & clusterSize() const { return clusterSize_; }
    const uintptr_t & allocatedSystemMemory() const { return allocatedSystemMemory_; }
    const uintptr_t & allocatedMemory() const { return allocatedMemory_; }
  protected:
    class Clusters;
    class Cluster {
      public:
        Cluster(Clusters * cs = NULL,void * memory = NULL) :
          cs_(cs),
          memory_((uint8_t *) memory),
          size_(0),
          fsize_(0),
          bsize_(0),
          index_(~uintptr_t(0)),
          count_(0),
          head_(NULL),
          next_(NULL),
          locked_(false) {}
	
        static RBTreeNode & treeO2N(const Cluster & object){
          return object.treeNode_;
        }
	      static Cluster & treeN2O(const RBTreeNode & node){
          Cluster * p = NULL;
          return node.object<Cluster>(p->treeNode_);
        }
        static intptr_t treeCO(const Cluster & a0,const Cluster & a1){
          return a0.memory_ < a1.memory_ ? -1 : a0.memory_ + a0.fsize_ >= a1.memory_ + a1.fsize_ ? 1 : 0;
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
          if( bsize_ == 1 ){
            if( j > 256 ) j = 256;
            for( i = 0; i < j; i++ )
              *(uint8_t *) (memory_ + i) = uint8_t(i + 1);
          }
          else if( bsize_ == 2 ){
            if( j > 65536 ) j = 65536;
            for( i = 0; i < j; i++ )
              *(uint16_t *) (memory_ + i * 2) = uint16_t(i + 1);
          }
          else if( bsize_ == 3 ){
            if( j > 16777216 ) j = 16777216;
            for( i = 0; i < j; i++ )
              *(uint32_t *) (memory_ + i * 3) = uint32_t(i + 1);
          }
          else if( bsize_ > ~uint32_t(0) - 1 ){
            for( i = 0; i < j; i++ )
              *(uintptr_t *) (memory_ + bsize_ * i) = i + 1;
          }
          else {
            for( i = 0; i < j; i++ )
              *(uint32_t *) (memory_ + bsize_ * i) = uint32_t(i + 1);
          }
          count_ = j;
          return *this;
	      }
      	Clusters * cs_;
	      uint8_t * memory_;
	      uintptr_t size_; // size of allocated memory
	      uintptr_t fsize_; // size of searching memory block
	      uintptr_t bsize_; // size of allocating memory block
	      uintptr_t index_; // first in free list block index
        uintptr_t count_; // count of allocated blocks
	      Cluster * head_; // head of allocated memory for small blocks (with size < 4)
	      Cluster * next_; // next memory for small blocks (with size < 4)
	      bool locked_;
    };
    class SizeDescriptor;
    class Clusters {
      public:
        Clusters(SizeDescriptor * sd = NULL,uintptr_t size = 0) : sd_(sd), size_(size) {}

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

        SizeDescriptor * sd_;
	      uintptr_t size_; // size of allocated memory
        bool locked_;
	      Cluster clusters_[1];
    };
    class SizeDescriptors;
    class SizeDescriptor {
      public:
        SizeDescriptor(SizeDescriptors * sds = NULL,uintptr_t size = 0) : sds_(sds), size_(size) {}

        SizeDescriptors * sds_;
	      uintptr_t size_; // size of allocating memory block
        EmbeddedList<Clusters,Clusters::listNode,Clusters::listObject> clusters_;
    };
    class SizeDescriptors {
      public:
        SizeDescriptors() {}

        static EmbeddedListNode<SizeDescriptors> & listNode(const SizeDescriptors & object){
	        return object.listNode_;
	      }
	      static SizeDescriptors & listObject(const EmbeddedListNode<SizeDescriptors> & node,SizeDescriptors * p = NULL){
	        return node.object(p->listNode_);
	      } 
	      mutable EmbeddedListNode<SizeDescriptors> listNode_;
  	
	      uintptr_t count() const { return (size_ - sizeof(SizeDescriptors) + sizeof(SizeDescriptor)) / sizeof(SizeDescriptor); }
	
	      uintptr_t size_; // size of allocated memory
        uintptr_t count_;
        bool locked_;
	      SizeDescriptor sizes_[1];
    };
    EmbeddedList<SizeDescriptors,SizeDescriptors::listNode,SizeDescriptors::listObject> sizes_;
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
    InterlockedMutex mutex_;
#endif

    void * sysalloc(uintptr_t size,bool lock,bool & locked,bool noThrow);
    HeapManager & sysfree(void * memory,uintptr_t size,bool locked);

    EmbeddedListNode<SizeDescriptors> * findSDSNode(uintptr_t size,uintptr_t & fsize,uintptr_t & lsize);
    EmbeddedListNode<Clusters> * findCSNode(SizeDescriptor * sd,uintptr_t size,bool & ffc);
  private:
};
//---------------------------------------------------------------------------
HeapManager::~HeapManager()
{
  clear();
}
//---------------------------------------------------------------------------
HeapManager::HeapManager() :
  clusterSize_(0), allocatedSystemMemory_(0), allocatedMemory_(0)
{
#if defined(__WIN32__) || defined(__WIN64__)
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  HANDLE hToken;
  BOOL r = OpenProcessToken(
    GetCurrentProcess(),
    TOKEN_ADJUST_PRIVILEGES,
    &hToken
  );
  if( r != 0 ){
    LUID luid;
    TOKEN_PRIVILEGES tp;
    /*r = LookupPrivilegeValue(
      NULL,
      SE_INC_BASE_PRIORITY_NAME,
      &luid
    );
    if( r != 0 ){
      tp.PrivilegeCount = 1;
      tp.Privileges[0].Luid = luid;
      tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
      r = AdjustTokenPrivileges(
        hToken,
        FALSE,
        &tp,
        sizeof(TOKEN_PRIVILEGES), 
        (PTOKEN_PRIVILEGES)NULL, 
        (PDWORD)NULL
      );
    }*/
    r = LookupPrivilegeValue(
      NULL,
      SE_LOCK_MEMORY_NAME,
      &luid
    );
    if( r != 0 ){
      tp.PrivilegeCount = 1;
      tp.Privileges[0].Luid = luid;
      tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
      r = AdjustTokenPrivileges(
        hToken,
        FALSE,
        &tp,
        sizeof(TOKEN_PRIVILEGES), 
        (PTOKEN_PRIVILEGES)NULL, 
        (PDWORD)NULL
      );
      if( r != 0 && GetLastError() != ERROR_NOT_ALL_ASSIGNED ){
        if( (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
            si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA32_ON_WIN64) &&
            !isWow64() ){
          if( GetLargePageMinimum() > si.dwAllocationGranularity ){
            clusterSize_ = GetLargePageMinimum();
            flags_ = MEM_LARGE_PAGES;
            bool locked;
            void * memory = sysalloc(clusterSize_,true,locked,true);
            if( memory == NULL ){
              clusterSize_ = 0;
            }
            else {
              sysfree(memory,clusterSize_,locked);
            }
          }
        }
      }
    }
    CloseHandle(hToken);
  }
  if( clusterSize_ < si.dwPageSize ){
    clusterSize_ = si.dwAllocationGranularity;
    flags_ = 0;
  }  
#else
  clusterSize_ = getpagesize();
#endif
  align_ = 1;
}
//---------------------------------------------------------------------------
HeapManager & HeapManager::clear()
{
  EmbeddedListNode<SizeDescriptors> * sdsNode = sizes_.first(), * sdsNode2;
  while( sdsNode != NULL ){
    SizeDescriptors * sds = &SizeDescriptors::listObject(*sdsNode);
    for( intptr_t i = sds->count() - 1; i >= 0; i-- ){
      SizeDescriptor * sd = sds->sizes_ + i;
      EmbeddedListNode<Clusters> * csNode = sd->clusters_.first(), * csNode2;
      while( csNode != NULL ){
        Clusters * cs = &Clusters::listObject(*csNode);
        EmbeddedListNode<Cluster> * cNode = cs->lru_.first();
        while( cNode != NULL ){
          Cluster * c = &Cluster::listObject(*cNode);
          if( c->head_ == NULL )
            sysfree(c->memory_,c->size_,c->locked_);
          cNode = cNode->next();
        }
        csNode2 = csNode->next();
        sysfree(cs,cs->size_,cs->locked_);
        csNode = csNode2;
      }
    }
    sdsNode2 = sdsNode->next();
    sysfree(sds,sds->size_,sds->locked_);
    sdsNode = sdsNode2;
  }
  sizes_.clear();
  clustersTree_.clear();
  allocatedSystemMemory_ = 0;
  allocatedMemory_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
EmbeddedListNode<HeapManager::SizeDescriptors> *
  HeapManager::findSDSNode(uintptr_t size,uintptr_t & fsize,uintptr_t & lsize)
{
  fsize = lsize = 0;
  EmbeddedListNode<SizeDescriptors> * sdsNode = sizes_.first();
  while( sdsNode != NULL ){
    SizeDescriptors * sds = &SizeDescriptors::listObject(*sdsNode);
    fsize = sds->sizes_[0].size_;
    lsize = sds->sizes_[sds->count() - 1].size_;
    if( size < fsize || size <= lsize ) break;
    sdsNode = sdsNode->next();
  }
  return sdsNode;
}
//---------------------------------------------------------------------------
EmbeddedListNode<HeapManager::Clusters> *
  HeapManager::findCSNode(SizeDescriptor * sd,uintptr_t size,bool & ffc)
{
  EmbeddedListNode<Clusters> * csNode = sd->clusters_.first();
  if( csNode != NULL ){
    Clusters * cs = &Clusters::listObject(*csNode);
    ffc = cs->lru_.count() > 0 && Cluster::listObject(*cs->lru_.first()).count_ > 0;
    if( !ffc ){
      uintptr_t a = 1;
      if( size == 3 ) a = clusterSize_ / 0x1000000u + (clusterSize_ < 0x1000000u);
      else
      if( size == 2 ) a = clusterSize_ / 0x10000u + (clusterSize_ < 0x10000u);
      else
      if( size == 1 ) a = clusterSize_ / 0x100u + (clusterSize_ < 0x100u);
      if( cs->free_.count() < a ) csNode = NULL;
    }
  }
  return csNode;
}
//---------------------------------------------------------------------------
void * HeapManager::malloc(uintptr_t size,bool lock)
{
  if( size == 0 ) return NULL;
  size += -intptr_t(size) & (align_ - 1);
  uintptr_t fsize, lsize;
  EmbeddedListNode<SizeDescriptors> * sdsNode = findSDSNode(size,fsize,lsize);
  SizeDescriptors * sds;
  bool locked;
  if( sdsNode == NULL || size < fsize ){
    sds = (SizeDescriptors *) sysalloc(clusterSize_,lock,locked,true);
    if( sds == NULL ) return NULL;
    new (sds) SizeDescriptors;
    sds->size_ = clusterSize_;
    sds->count_ = 0;
    sds->locked_ = locked;
    uintptr_t k = sds->count(), j = (size / k) * k + 1; // zero size not used
    for( intptr_t i = k - 1; i >= 0; i-- )
      new (sds->sizes_ + i) SizeDescriptor(sds,j + i);
    if( sdsNode == NULL ) sizes_.insToTail(*sds); else sizes_.insBefore(SizeDescriptors::listObject(*sdsNode),*sds);
    sdsNode = &SizeDescriptors::listNode(*sds);
  }
  sds = &SizeDescriptors::listObject(*sdsNode);
  SizeDescriptor * sd = &sds->sizes_[(size - sds->sizes_[0].size_) / align_];
  bool ffc = false;
  EmbeddedListNode<Clusters> * csNode = findCSNode(sd,size,ffc);
  Clusters * cs;
  if( csNode == NULL ){
    cs = (Clusters *) sysalloc(clusterSize_,lock,locked,true);
    if( cs == NULL ) return NULL;
    new (cs) Clusters(sd,clusterSize_);
    cs->locked_ = locked;
    for( intptr_t i = cs->count() - 1; i >= 0; i-- ){
      new (cs->clusters_ + i) Cluster(cs);
      cs->free_.insToHead(cs->clusters_[i]);
    }
    sd->clusters_.insToHead(*cs);
    csNode = &Clusters::listNode(*cs);
  }
  cs = &Clusters::listObject(*csNode);
  EmbeddedList<Cluster,Cluster::listNode,Cluster::listObject> & list = ffc ? cs->lru_ : cs->free_;
  uintptr_t cc = 1, bs = clusterSize_;
  if( size >= 4 ){
  }
  else if( size == 3 && clusterSize_ > 0x1000000u ){
    cc = clusterSize_ / 0x1000000u;
    bs = 0x1000000u;
  }
  else if( size == 2 && clusterSize_ > 0x10000u ){
    cc = clusterSize_ / 0x10000u;
    bs = 0x10000u;
  }
  else if( size == 1 && clusterSize_ > 0x100u ){
    cc = clusterSize_ / 0x100u;
    bs = 0x100u;
  }
  Cluster * c, * chead = NULL;
  for( uintptr_t i = 0; i < cc; i++ ){
    c = &Cluster::listObject(*list.first());
    if( c->memory_ != NULL ){
      chead = c;
      list.remove(*c);
      break;
    }
    if( chead == NULL ){
      c->size_ = size > clusterSize_ ? size + (-intptr_t(size) & (clusterSize_ - 1)) : clusterSize_;
      c->fsize_ = bs;
      c->bsize_ = size;
      c->memory_ = (uint8_t *) sysalloc(c->size_,lock,locked,true);
      if( c->memory_ == NULL ) return NULL;
      c->locked_ = locked;
      c->initialize();
      c->head_ = c;
      chead = c;
      list.remove(*c);
    }
    else {
      c->cs_ = chead->cs_;
      c->size_ = chead->size_;
      c->fsize_ = chead->fsize_;
      c->bsize_ = chead->bsize_;
      c->memory_ = chead->memory_ + i * bs;
      c->head_ = chead;
      c->next_ = chead->next_;
      chead->next_ = c;
      c->initialize();
      list.remove(*c);
      cs->lru_.insToHead(*c);
    }
    clustersTree_.insert(*c);
  }
  c = chead;
  void * p = c->memory_ + c->index_ * size;
  if( size == 1 ){
    c->index_ = *(uint8_t *) (c->memory_ + c->index_);
  }
  else if( size == 2 ){
    c->index_ = *(uint16_t *) (c->memory_ + c->index_ * 2);
  }
  else if( size == 3 ){
    c->index_ = *(uint32_t *) (c->memory_ + c->index_ * 3) & 0xFFFFFF;
  }
  else if( size > ~uint32_t(0) - 1 ){
    c->index_ = *(uintptr_t *) (c->memory_ + c->index_ * size);
  }
  else {
    c->index_ = *(uint32_t *) (c->memory_ + c->index_ * size);
  }
  c->count_--;
  if( c->count_ == 0 ) cs->lru_.insToTail(*c); else cs->lru_.insToHead(*c);
  sds->count_++;
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
  Cluster c(NULL,ptr), * cp = clustersTree_.find(c);
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
void HeapManager::free(void * ptr)
{
  if( ptr == NULL ) return;
  Cluster c(NULL,ptr), * cp = clustersTree_.find(c);
  if( cp == NULL || (uintptr_t(ptr) - uintptr_t(cp->memory_)) % cp->bsize_ != 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
    SetLastError(ERROR_INVALID_DATA);
#else
    errno = EINVAL;
#endif
    int32_t err = oserror() + errorOffset;
    Exception e(err,__PRETTY_FUNCTION__);
    e.writeStdError();
    return;
  }
  uintptr_t j = cp->size_ / cp->bsize_;
  if( cp->bsize_ == 1 && j > 256 ){
    j = 256;
  }
  else if( cp->bsize_ == 2 && j > 65536 ){
    j = 65536;
  }
  else if( cp->bsize_ == 3 && j > 16777216 ){
    j = 16777216;
  }
  if( cp->count_ >= j ){
#if defined(__WIN32__) || defined(__WIN64__)
    SetLastError(ERROR_INVALID_DATA);
#else
    errno = EINVAL;
#endif
    int32_t err = oserror() + errorOffset;
    Exception e(err,utf8::String("chunk already free ") + __PRETTY_FUNCTION__);
    e.writeStdError();
    return;
  }
  if( cp->count_ > 0 ){
    if( cp->bsize_ == 1 ){
      *(uint8_t *) ptr = uint8_t(cp->index_);
    }
    else if( cp->bsize_ == 2 ){
      *(uint16_t *) ptr = uint16_t(cp->index_);
    }
    else if( cp->bsize_ == 3 ){
      *(uint16_t *) ptr = uint16_t(cp->index_);
      *(uint8_t *) ptr = uint8_t(cp->index_ >> 16);
    }
    else if( cp->bsize_ > ~uint32_t(0) - 1 ){
      *(uintptr_t *) ptr = cp->index_;
    }
    else {
      *(uint32_t *) ptr = uint32_t(cp->index_);
    }
  }
  cp->index_ = ((uint8_t *) ptr - cp->memory_) / cp->bsize_;
  cp->count_++;
  cp->cs_->sd_->sds_->count_--;
  allocatedMemory_ -= cp->bsize_;
  if( cp->count_ == j ){
    Cluster * c = cp->head_;
    if( c != NULL ){
      for(;;){
        assert( c->count_ <= j );
        if( c->count_ != j ) return;
        c = c->next_;
        if( c == NULL ) break;
      }
      c = cp->head_;
      for(;;){
        c->cs_->lru_.remove(*c);
        if( c != c->head_ ){
          clustersTree_.remove(*c);
          c->memory_ = NULL;
          c->cs_->free_.insToHead(*c);
        }
        c = c->next_;
        if( c == NULL ) break;
      }
      cp = cp->head_;
    }
    else {
      cp->cs_->lru_.remove(*cp);
    }
    clustersTree_.remove(*cp);
    sysfree(cp->memory_,cp->size_,cp->locked_);
    cp->memory_ = NULL;
    cp->cs_->free_.insToHead(*cp);
    if( cp->cs_->free_.count() == cp->cs_->count() ){
      SizeDescriptor * sd = cp->cs_->sd_;
      sd->clusters_.remove(*cp->cs_);
      sysfree(cp->cs_,cp->cs_->size_,cp->cs_->locked_);
      if( sd->sds_->count_ == 0 ){
        sizes_.remove(*sd->sds_);
        sysfree(sd->sds_,sd->sds_->size_,sd->sds_->locked_);
      }
    }
  }
  else {
    cp->cs_->lru_.remove(*cp);
    cp->cs_->lru_.insToHead(*cp);
  }
}
//---------------------------------------------------------------------------
HeapManager & HeapManager::sysfree(void * memory,uintptr_t size,bool locked)
{
  assert( memory != NULL );
#if defined(__WIN32__) || defined(__WIN64__)
#ifndef NDEBUG
  BOOL r;
#endif
  if( locked ){
#ifndef NDEBUG
    r =
#endif
    VirtualUnlock(memory,size);
#ifndef NDEBUG
    assert( r != 0 );
#endif
  }
#ifndef NDEBUG
  r =
#endif
  VirtualFree(memory,size,MEM_DECOMMIT);
#ifndef NDEBUG
  assert( r != 0 );
#endif
#else
#ifndef NDEBUG
  int r;
#endif
  if( locked ){
    r =
    munlock(memory,size);
#ifndef NDEBUG
    assert( r == 0 );
#endif
  }
  errno = 0;
  ::free(memory);
  assert( errno == 0 );
#endif
  allocatedSystemMemory_ -= size;
  return *this;
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
static BOOL sysallocHelper(uintptr_t size)
{
  SIZE_T minimumWorkingSetSize, maximumWorkingSetSize;
  BOOL r;
  r = GetProcessWorkingSetSize(GetCurrentProcess(),&minimumWorkingSetSize,&maximumWorkingSetSize);
  if( r != 0 ){
    minimumWorkingSetSize += size;
    if( maximumWorkingSetSize < minimumWorkingSetSize ) maximumWorkingSetSize = minimumWorkingSetSize;
    r = SetProcessWorkingSetSize(GetCurrentProcess(),minimumWorkingSetSize,maximumWorkingSetSize);
  }
  return r;
}
#endif
//---------------------------------------------------------------------------
void * HeapManager::sysalloc(uintptr_t size,bool lock,bool & locked,bool noThrow)
{
  void * memory = NULL;
  locked = false;
#if defined(__WIN32__) || defined(__WIN64__)
  memory = VirtualAlloc(NULL,size,flags_ | MEM_COMMIT,PAGE_READWRITE);
  if( memory == NULL ){
    int32_t err = GetLastError() + errorOffset;
    if( !noThrow ) newObjectV1C2<EOutOfMemory>(err,__PRETTY_FUNCTION__)->throwSP();    
  }
  else {
l1: if( lock && !(locked = VirtualLock(memory,size) != 0) ){
      int32_t err = GetLastError();
      if( err == ERROR_WORKING_SET_QUOTA ){
        if( sysallocHelper(size) != 0 ) goto l1;
        err = GetLastError();
      }
      else if( err == ERROR_INVALID_PARAMETER ){
        VirtualFree(memory,size,MEM_DECOMMIT);
        SetLastError(err);
        return NULL;
      }
      Exception e(err + errorOffset,"VirtualLock(" + utf8::int2Str(size) + ") " + __PRETTY_FUNCTION__);
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
  assert( (uintptr_t(memory) & (clusterSize_ - 1)) == 0 ); // check address align
  return memory;
}
//---------------------------------------------------------------------------
void heapBenchmark()
{
  AutoPtr<Randomizer> rnd(newObject<Randomizer>());
  HeapManager hm;
  uintptr_t elCount = 4096 * 1024;
  Array<uintptr_t> sizes;
  Array<void *> ptrs;
  sizes.resize(elCount);
  ptrs.resize(elCount);
  for( uintptr_t i = 0; i < elCount; i++ ){
    sizes[i] = uintptr_t(rnd->random(64) + 1);
    ptrs[i] = NULL;
  }
  uint64_t t, seqMallocTime = 0, seqFreeTime = 0, rndTime, allocatedSystemMemory, allocatedMemory;
  t = gettimeofday();
  for( uintptr_t i = 0; i < elCount; i++ ){
    ptrs[i] = hm.malloc(sizes[i],false);
  }
  seqMallocTime += gettimeofday() - t;
  allocatedSystemMemory = hm.allocatedSystemMemory();
  allocatedMemory = hm.allocatedMemory();
  t = gettimeofday();
  for( uintptr_t i = 0; i < elCount; i++ ){
    hm.free(ptrs[i]);
    ptrs[i] = NULL;
  }
  seqFreeTime += gettimeofday() - t;
  fprintf(stderr,"seq mallocs: %8"PRIu64".%04"PRIu64" mps, ellapsed %s\n",
    uint64_t(elCount) * 1000000u / seqMallocTime,
    uint64_t(elCount) * 10000u * 1000000u / seqMallocTime -
    uint64_t(elCount) * 1000000u / seqMallocTime * 10000u,
    (const char *) utf8::elapsedTime2Str(seqMallocTime).getOEMString()
  );
  fprintf(stderr,"seq frees: %8"PRIu64".%04"PRIu64" fps, ellapsed %s\n",
    uint64_t(elCount) * 1000000u / seqFreeTime,
    uint64_t(elCount) * 10000u * 1000000u / seqFreeTime -
    uint64_t(elCount) * 1000000u / seqFreeTime * 10000u,
    (const char *) utf8::elapsedTime2Str(seqFreeTime).getOEMString()
  );
  fprintf(stderr,
    "memory used area: %s\n",
    (const char *) formatByteLength(
      allocatedMemory,
      allocatedSystemMemory,
      "P"
    ).getOEMString()
  );
  seqMallocTime = seqFreeTime = 0;
  t = gettimeofday();
  for( uintptr_t i = 0; i < elCount; i++ ){
    ptrs[i] = kmalloc(sizes[i]);
  }
  seqMallocTime += gettimeofday() - t;
  allocatedSystemMemory = hm.allocatedSystemMemory();
  allocatedMemory = hm.allocatedMemory();
  t = gettimeofday();
  for( uintptr_t i = 0; i < elCount; i++ ){
    kfree(ptrs[i]);
    ptrs[i] = NULL;
  }
  seqFreeTime += gettimeofday() - t;
  fprintf(stderr,"seq system mallocs: %8"PRIu64".%04"PRIu64" mps, ellapsed %s\n",
    uint64_t(elCount) * 1000000u / seqMallocTime,
    uint64_t(elCount) * 10000u * 1000000u / seqMallocTime -
    uint64_t(elCount) * 1000000u / seqMallocTime * 10000u,
    (const char *) utf8::elapsedTime2Str(seqMallocTime).getOEMString()
  );
  fprintf(stderr,"seq system frees: %8"PRIu64".%04"PRIu64" fps, ellapsed %s\n",
    uint64_t(elCount) * 1000000u / seqFreeTime,
    uint64_t(elCount) * 10000u * 1000000u / seqFreeTime -
    uint64_t(elCount) * 1000000u / seqFreeTime * 10000u,
    (const char *) utf8::elapsedTime2Str(seqFreeTime).getOEMString()
  );
// random test
  Array<uintptr_t> indices;
  indices.resize(elCount);
  for( uintptr_t i = 0; i < elCount; i++ ){
    indices[i] = uintptr_t(rnd->random(elCount));
  }
  rndTime = 0;
  t = gettimeofday();
  for( uintptr_t i = 0; i < elCount; i++ ){
    if( ptrs[indices[i]] == NULL ){
      ptrs[indices[i]] = hm.malloc(sizes[i]);
    }
    else {
      hm.free(ptrs[indices[i]]);
      ptrs[indices[i]] = NULL;
    }
  }
  rndTime += gettimeofday() - t;
  fprintf(stderr,"rnd mallocs - frees: %8"PRIu64".%04"PRIu64" mfps, ellapsed %s\n",
    uint64_t(elCount) * 1000000u / rndTime,
    uint64_t(elCount) * 10000u * 1000000u / rndTime -
    uint64_t(elCount) * 1000000u / rndTime * 10000u,
    (const char *) utf8::elapsedTime2Str(rndTime).getOEMString()
  );
  allocatedSystemMemory = hm.allocatedSystemMemory();
  allocatedMemory = hm.allocatedMemory();
  fprintf(stderr,
    "memory used area: %s\n",
    (const char *) formatByteLength(
      allocatedMemory,
      allocatedSystemMemory,
      "P"
    ).getOEMString()
  );
  for( uintptr_t i = 0; i < elCount; i++ ){
    hm.free(ptrs[i]);
    ptrs[i] = NULL;
  }
  rndTime = 0;
  t = gettimeofday();
  for( uintptr_t i = 0; i < elCount; i++ ){
    if( ptrs[indices[i]] == NULL ){
      ptrs[indices[i]] = kmalloc(sizes[i]);
    }
    else {
      kfree(ptrs[indices[i]]);
      ptrs[indices[i]] = NULL;
    }
  }
  rndTime += gettimeofday() - t;
  fprintf(stderr,"rnd system mallocs - frees: %8"PRIu64".%04"PRIu64" mfps, ellapsed %s\n",
    uint64_t(elCount) * 1000000u / rndTime,
    uint64_t(elCount) * 10000u * 1000000u / rndTime -
    uint64_t(elCount) * 1000000u / rndTime * 10000u,
    (const char *) utf8::elapsedTime2Str(rndTime).getOEMString()
  );
  for( uintptr_t i = 0; i < elCount; i++ ){
    kfree(ptrs[i]);
    ptrs[i] = NULL;
  }
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
