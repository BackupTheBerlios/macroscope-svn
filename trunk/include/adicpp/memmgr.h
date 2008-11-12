/*-
 * Copyright 2007-2008 Guram Dukashvili
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
#ifndef _memmgr_H_
#define _memmgr_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
void initialize(int,char **);
void cleanup();
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// Memory manager /////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class MemoryManager {
  friend void initialize(int,char **);
  friend void cleanup();
  public:
    virtual ~MemoryManager();
    MemoryManager();

    static MemoryManager & globalMemoryManager();
    MemoryManager & clear();

    void * sysalloc(uintptr_t size,bool lock,bool & locked,bool noThrow);
    MemoryManager & sysfree(void * memory,uintptr_t size,bool locked,bool noThrow);

    void * malloc(uintptr_t size,bool lock,bool & locked);
    void * realloc(void * ptr,uintptr_t size,bool lock,bool & locked);
    void * malloc(uintptr_t size){ bool locked; return malloc(size,lock_,locked); }
    void * realloc(void * ptr,uintptr_t size){ bool locked; return realloc(ptr,size,lock_,locked); }
    void free(void * ptr);

    const uintptr_t & clusterSize() const { return clusterSize_; }
    const uintptr_t & allocatedSystemMemory() const { return allocatedSystemMemory_; }
    const uintptr_t & allocatedMemory() const { return allocatedMemory_; }
    const bool & lock() const { return lock_; }
    MemoryManager & lock(bool a){ lock_ = a; return *this; }
  protected:
    class Clusters;
    class Cluster {
      public:
        virtual ~Cluster() {}
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
	
        static RBTreeNode & treeO2N(const Cluster & object,uintptr_t *){
          return object.treeNode_;
        }
	      static Cluster & treeN2O(const RBTreeNode & node,uintptr_t *){
          Cluster * p = NULL;
          return node.object<Cluster>(p->treeNode_);
        }
        static intptr_t treeCO(const Cluster & a0,const Cluster & a1,uintptr_t *){
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
      uintptr_t,
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
    FiberInterlockedMutex mutex_;
    bool lock_;
    bool interlocked_;
    volatile ilock_t initMutex_; // used only in global
    volatile ilock_t initCount_; // used only in global


    EmbeddedListNode<SizeDescriptors> * findSDSNode(uintptr_t size,uintptr_t & fsize,uintptr_t & lsize);
    EmbeddedListNode<Clusters> * findCSNode(SizeDescriptor * sd,uintptr_t size,bool & ffc);
  private:
    MemoryManager(const MemoryManager &);
    void operator = (const MemoryManager &);

    static uint8_t globalMemoryManagerHolder_[];

    static void initialize();
    static void cleanup();
};
//---------------------------------------------------------------------------
inline MemoryManager & MemoryManager::globalMemoryManager()
{
  return *reinterpret_cast<MemoryManager *>(globalMemoryManagerHolder_);
}
//---------------------------------------------------------------------------
void heapBenchmark();
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _memmgr_H_ */
//---------------------------------------------------------------------------
