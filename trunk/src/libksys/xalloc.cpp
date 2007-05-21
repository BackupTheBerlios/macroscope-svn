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
/*class HeapManager {
  public:
    void * malloc(uintptr_t size);
    void * realloc(uintptr_t size);
    void free(const void * ptr);
  protected:
    class Cluster {
      public:
        Cluster() : memory_(NULL), refCount_(0), locked_(false) {}
	
        static EmbeddedHashNode<Cluster,uintptr_t> & ehNLT(const uintptr_t & link,uintptr_t &){
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
        mutable EmbeddedHashNode<Cluster,uintptr_t> keyNode_;

        static EmbeddedListNode<Cluster> & listNode(const Cluster & object){
	  return object.listNode_;
	}
	static Cluster & listNodeObject(const EmbeddedListNode<Cluster> & node,Cluster * p = NULL){
	  return node.object(p->listNode_);
	} 
	mutable EmbeddedListNode<Cluster> listNode_;
	
	void * memory_;
	uintptr_t index_; // first in free list block index
	ilock_t refCount_;
	bool locked_;
    };
    class Clusters {
      public:
        EmbeddedList<Cluster,Cluster::listNode,Cluster::listNodeObject> clusters_;
	uintptr_t size_;
    };
    EmbeddedList<Cluster,Cluster::listNode,Cluster::listNodeObject> clusters_;
    uintptr_t clusterSize_;
    uintptr_t usedMemory_;
  private:
};
//---------------------------------------------------------------------------
HeapManager::~HeapManager()
{
}
//---------------------------------------------------------------------------
HeapManager::HeapManager()
{
#if defined(__WIN32__) || defined(__WIN64__)
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  clusterSize_ = GetLargePageMinimum();
  if( clusterSize_ < si.dwPageSize ) clusterSize_ = si.dwPageSize;
#else
  clusterSize_ = getpagesize();
#endif
}
//---------------------------------------------------------------------------
HeapManager & HeapManager::clear()
{
  deleteCluster(clusters_);
  return *this;
}
//---------------------------------------------------------------------------
Cluster * HeapManager::findClusterBySize(uintptr_t size)
{
  Cluster * cluster = *Cluster::listNodeObject(*clusters_.first());
  uintptr_t fsize = 0, lsize = 0;
  while( cluster != NULL ){
    Clusters * clusters = *((Clusters *) cluster->memory_;
    fsize = Cluster::listNodeObject(*clusters->clusters_.first())->size_;
    lsize = Cluster::listNodeObject(*clusters->clusters_.last())->size_;
    if( size < fsize || size <= lsize ) break;
    cluster = &Cluster::listNode(cluster->next());
  }
  if( cluster == NULL || size < fsize ){
    newCluster(clusters_);
    for( intptr_t i = clusterSize_ - sizeof(Clusters) / sizeof(Clusters) - 1; i >= 0; i-- ){
      Clusters * clusters = (Clusters *) clusters_.memory_ + i;
      new (clusters) Clusters;
      clusters_->size_ = i;
    }
  }
}
//---------------------------------------------------------------------------
void * HeapManager::malloc(uintptr_t size)
{
  Cluster * cluster = findClusterBySize(size);
}
//---------------------------------------------------------------------------
HeapManager & HeapManager::dropCluster(Cluster & cluster)
{
  if( cluster.memory_ != NULL ){
#if defined(__WIN32__) || defined(__WIN64__)
    VirtualUnlock(cluster.memory_,clusterSize_);
    VirtualFree(cluster.memory_,clusterSize_);
#else
    munlock(cluster.memory_,clusterSize_);
    ::free(cluster.memory_);
#endif
    cluster.memory_ = NULL;
    usedMemory_ -= clusterSize_;
  }
  return *this;
}
//---------------------------------------------------------------------------
HeapManager & HeapManager::newCluster(Cluster & cluster)
{
#if defined(__WIN32__) || defined(__WIN64__)
  assert( cluster.memory_ == NULL );
  cluster.memory_ = VirtualAlloc(NULL,clusterSize_,MEM_COMMIT | MEM_LARGE_PAGES,0);
  if( cluster.memory_ == NULL ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<EOutOfMemory>(err,__PRETTY_FUNCTION__)->throwSP();    
  }
  if( VirtualLock(cluster.memory_,clusterSize_) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    Exception e(err,__PRETTY_FUNCTION__);
    e.writeStdError();
  }
  else {
    cluster.locked_ = true;
  }
  usedMemory_ += clusterSize_;
#else
  cluster.memory_ = ::malloc(clusterSize_);
  if( cluster.memory_ == NULL ){
    int32_t err = errno;
    newObjectV1C2<EOutOfMemory>(err,__PRETTY_FUNCTION__)->throwSP();    
  }
  if( mlock(cluster.memory_,clusterSize_) != 0 ){  
    int32_t err = errno;
    Exception e(err,__PRETTY_FUNCTION__);
    e.writeStdError();
  }
  else {
    cluster.locked_ = true;
  }
#endif
  assert( (cluster.memory_ & (clusterSize_ - 1)) == 0 );
  return *this;
}*/
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
