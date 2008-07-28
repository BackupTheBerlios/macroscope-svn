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
#ifndef _uss_H
#define _uss_H
//-----------------------------------------------------------------------------
namespace uss {
//-----------------------------------------------------------------------------
typedef uint64_t ussaddr_t;
typedef uint64_t ussint_t;
extern const uint8_t ussFileSignature_[];
//-----------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
struct FileHeader {
  uint8_t signature_[16];
  union {
    struct {
      uint8_t release_;
      uint8_t revision_;
      uint16_t level_;
    };
    uint32_t version_;
    uint8_t align_[16];
  } version_;
  union {
    struct {
      uint8_t addressBitSize_;
      uint8_t sectorBitSize_;
      uint8_t clusterBitSize_;
      uint8_t extentBitSize_;
    };
    uint8_t align_[16];
  } sizes_;
  uint64_t catalogAddress_; // in sectors
  uint8_t checksum_[32]; // sha256
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class FileParams {
  friend class FileDriver;
  public:
    ~FileParams();
    FileParams();
  protected:
  private:
// input parameters
    uintptr_t addressBitSize_;
    uintptr_t sectorBitSize_; // in bits
    uintptr_t clusterBitSize_; // in bits
    uintptr_t extentBitSize_; // in bits
// output parameters
    uintptr_t addressSize_; // in bytes
    ussaddr_t sectorSize_; // in bytes
    ussaddr_t clusterSize_; // in bytes
    ussaddr_t extentSize_; // in bytes

    ussaddr_t sectorBitMask_;
    ussaddr_t sectorInvertedBitMask_;
    uintptr_t clusterBitShift_;
    ussaddr_t clusterBitMask_;
    ussaddr_t clusterInvertedBitMask_;
    ussaddr_t clusterShiftedBitMask_;
    ussaddr_t clusterInvertedShiftedBitMask_;
    uintptr_t extentBitShift_;
    ussaddr_t extentBitMask_;
    ussaddr_t extentInvertedBitMask_;
    ussaddr_t extentShiftedBitMask_;
    ussaddr_t extentInvertedShiftedBitMask_;

    ussaddr_t clusterAndSectorShiftedBitMask_;
    ussaddr_t clusterAndSectorInvertedShiftedBitMask_;

    ussaddr_t extentAndClusterShiftedBitMask_;
    ussaddr_t extentAndClusterInvertedShiftedBitMask_;

    uintptr_t sectorsInCluster_;
    uintptr_t sectorsInClusterBit_;
    uintptr_t sectorsInClusterBitMask_;
    uintptr_t clustersInExtent_;
    uintptr_t clustersInExtentBit_;
    uintptr_t clustersInExtentBitMask_;

    uintptr_t headerSize_; // in bytes, aligned on sector boundary
    struct ExtentsMapTable {
      ussaddr_t extents_; // number of all extents in table = (1 << addressSize_) / extentSize_
      ussaddr_t tableSize_; // in bytes
      ussaddr_t extentsInPage_; // number of extents in single page
      ussaddr_t extentsInPageBit_; // number of extents in single page, in bits
      ussaddr_t extentsInPageBitMask_;
      ussaddr_t pageSize_; // in bytes, must be greater or equal cluster size
      ussaddr_t pageBitSize_; // in bits
      ussaddr_t pagesInCatalog_;
      ussaddr_t catalogSize_; // in bytes
      ussaddr_t catalogPageSize_; // in bytes
      ussaddr_t catalogPageBitSize_; // in bits
      ussaddr_t pagesInCatalogPage_;
      ussaddr_t pagesInCatalogPageBit_; // in bits
      ussaddr_t pagesInCatalogPageBitMask_; // in bits
    } mapTable_;
    struct Bitmap {
      ksys::AutoPtr<uint8_t> first_;
      ksys::AutoPtr<uint8_t> regular_;
      uintptr_t bitSize_; // in bits
      uintptr_t sectorSize_; // in sectors
      ussaddr_t size_; // in bytes
      union {
        struct {
          ussaddr_t firstOffset_; // in sectors from cluster begining
          ussaddr_t regularOffset_; // in sectors from cluster begining
        };
        ussaddr_t offsets_[2];
      };
    } clusterBitmap_, extentBitmap_;
    ussaddr_t catalogAddress_; // in sectors

    static ussaddr_t nearest(ussaddr_t a,ussaddr_t sz);
    static ussaddr_t powerOfTwo(uintptr_t pow);
    static ussaddr_t nearestPowerOfTwo(ussaddr_t a);
    static uintptr_t bitSize(ussaddr_t a);
    static void fmtStr(char * fmt,const char * s1,const char * s2,const char * s3);
    void calc(
      uintptr_t addressBitSize = 0,
      uintptr_t sectorBitSize = 0,
      uintptr_t clusterBitSize = 0,
      uintptr_t extentBitSize = 0
    );
    static void print(const utf8::String & name);
    void create(const utf8::String & name);
    void initialize(const FileHeader & hdr);

    ussaddr_t getClusterBitmapAddress(ussaddr_t inExtentAddress,ussaddr_t cluster) const;
    ussaddr_t getExtentBitmapAddress(ussaddr_t inExtentAddress) const;
    ussaddr_t getCatalogPageAddress(ussaddr_t virtualAddress) const;
    ussaddr_t getPageIndexInCatalogPage(ussaddr_t virtualAddress) const;
    ussaddr_t getExtentIndexInPage(ussaddr_t virtualAddress) const;
};
//---------------------------------------------------------------------------
inline FileParams::~FileParams()
{
}
//---------------------------------------------------------------------------
inline FileParams::FileParams()
{
}
//---------------------------------------------------------------------------
ussaddr_t FileParams::nearest(ussaddr_t a,ussaddr_t sz)
{
  return (a / sz + (a % sz != 0)) * sz;
}
//---------------------------------------------------------------------------
ussaddr_t FileParams::powerOfTwo(uintptr_t pow)
{
  return ussaddr_t(1) << pow;
}
//---------------------------------------------------------------------------
class FileDriver { // sectors level driver
  friend void initialize();
  friend void cleanup();
  friend class StorageDriver;
  public:
    ~FileDriver();
    FileDriver();
    
    static void print(const utf8::String & name);
    void create(const utf8::String & name);
    FileDriver & open(const utf8::String & name);
    FileDriver & close();
    FileDriver & alloc(ussaddr_t count,ussaddr_t & address,ussint_t & allocated);
    FileDriver & free(ussaddr_t address,ussint_t count);
// flags may be or'ed values: O_DIRECT, O_SHLOCK, O_EXLOCK
    FileDriver & read(void * buf,ussaddr_t address,ussint_t count);
    FileDriver & write(const void * buf,ussaddr_t address,ussint_t count);
    FileDriver & pack();
  protected:
  private:
    FileParams * params_;
    ksys::AsyncFile file_;
    uint64_t fileNameHash_;
    class ExtentMap : public ksys::FiberInterlockedMutex {
      friend class FileDriver;
      public:
        ~ExtentMap();
        ExtentMap();
      protected:
      private:
        class EMA { // extent mapped addresses
          friend class FileDriver;
	  friend class ExtentMap;
          public:
            ~EMA();
            EMA(uint64_t fileNameHash = 0,ussaddr_t virtualAddress = 0,ussaddr_t physicalAddress = 0);
          protected:
          private:
	    ksys::ListNode<EMA> * mapNode_;
	    mutable ksys::EmbeddedHashNode<EMA> hashNode_;
	    static ksys::EmbeddedHashNode<EMA> & hashNode(const EMA & object){ return object.hashNode_; }
	    static EMA & hashObject(const ksys::EmbeddedHashNode<EMA> & node,EMA * p = NULL){ return node.object(p->hashNode_); }
	    static uintptr_t hash(const EMA & object);
	    static bool equ(const EMA & o1,const EMA & o2);
	    mutable ksys::EmbeddedListNode<EMA> node_;
	    static ksys::EmbeddedListNode<EMA> & node(const EMA & object){ return object.node_; }
	    static EMA & nodeObject(const ksys::EmbeddedListNode<EMA> & node,EMA * p = NULL){ return node.object(p->node_); }
  	    uint64_t fileNameHash_;
            ussaddr_t virtualAddress_;
            ussaddr_t physicalAddress_;
	};
        ksys::List<EMA> map_;
        ksys::EmbeddedHash<EMA,EMA::hashNode,EMA::hashObject,EMA::hash,EMA::equ> hash_;
        ksys::EmbeddedList<EMA,EMA::node,EMA::nodeObject> lru_;
	uintptr_t minSize_; // in bytes
	uintptr_t maxSize_; // in bytes
	uintptr_t curSize_; // in bytes
    };
    static uint8_t extentMapHolder_[];
    static ExtentMap & extentMap();
    class Cache : public ksys::FiberInterlockedMutex {
      friend class FileDriver;
      public:
        ~Cache();
	Cache();
      protected:
      private:
        class Line : public ksys::FiberMutex {
          friend class FileDriver;
	  friend class Cache;
          public:
            ~Line();
            Line(uint64_t fileNameHash = 0,ussaddr_t virtualAddress = 0);
          protected:
          private:
	    ksys::ListNode<Line> * cacheNode_;
            ksys::FiberSemaphore queue_;
   	    uintptr_t queueLength_;
	    mutable ksys::EmbeddedHashNode<Line> vaHashNode_;
	    static ksys::EmbeddedHashNode<Line> & vaHashNode(const Line & object){ return object.vaHashNode_; }
	    static Line & vaHashObject(const ksys::EmbeddedHashNode<Line> & node,Line * p = NULL){ return node.object(p->vaHashNode_); }
	    static uintptr_t vaHash(const Line & object);
	    static bool vaEqu(const Line & o1,const Line & o2);
	    mutable ksys::EmbeddedListNode<Line> lruNode_;
	    static ksys::EmbeddedListNode<Line> & lruNode(const Line & object){ return object.lruNode_; }
	    static Line & lruObject(const ksys::EmbeddedListNode<Line> & node,Line * p = NULL){ return node.object(p->lruNode_); }
	    mutable ksys::EmbeddedListNode<Line> dirtyNode_;
	    static ksys::EmbeddedListNode<Line> & dirtyNode(const Line & object){ return object.dirtyNode_; }
	    static Line & dirtyObject(const ksys::EmbeddedListNode<Line> & node,Line * p = NULL){ return node.object(p->dirtyNode_); }
	    mutable ksys::EmbeddedListNode<Line> deadNode_;
	    static ksys::EmbeddedListNode<Line> & deadNode(const Line & object){ return object.deadNode_; }
	    static Line & deadObject(const ksys::EmbeddedListNode<Line> & node,Line * p = NULL){ return node.object(p->deadNode_); }
  	    uint64_t fileNameHash_;
            ussaddr_t virtualAddress_;
            ussaddr_t physicalAddress_;
	    ksys::AutoPtr<uint8_t> buffer_;
	    uintptr_t size_; // buffer size in bytes
	    int64_t flushTime_;
	    bool dirty_;
	    enum State { stReady, stReading, stWriting };
	    State state_;
	    Line & releaseQueue();
        };
        ksys::List<Line> cache_;
        ksys::EmbeddedHash<Line,Line::vaHashNode,Line::vaHashObject,Line::vaHash,Line::vaEqu> vaHash_;
        ksys::EmbeddedList<Line,Line::lruNode,Line::lruObject> lru_;
        ksys::EmbeddedList<Line,Line::dirtyNode,Line::dirtyObject> dirty_;
        ksys::EmbeddedList<Line,Line::deadNode,Line::deadObject> dead_;
	uintptr_t minSize_; // in bytes
	uintptr_t maxSize_; // in bytes
	uintptr_t curSize_; // in bytes
	int64_t flushDelay_;
    };
    static uint8_t cacheHolder_[];
    static Cache & cache();

    FileDriver & map(ussaddr_t virtualAddress,ussaddr_t & physicalAddress);
    FileDriver & flushDirtyCacheLines();
    FileDriver & removeLRUCacheLines();
    FileDriver & removeDeadCacheLines();
    Cache::Line * pin(ussaddr_t firstClusterSectorAddress,ussint_t clusterCount,uintptr_t flags);
    FileDriver & unpin(Cache::Line * line);
    FileDriver & setDirty(Cache::Line * line);
};
//---------------------------------------------------------------------------
inline FileDriver::~FileDriver()
{
}
//---------------------------------------------------------------------------
inline FileDriver::FileDriver() : params_(NULL)
{
}
//---------------------------------------------------------------------------
inline FileDriver::ExtentMap & FileDriver::extentMap()
{
  return *reinterpret_cast<ExtentMap *>(extentMapHolder_);
}
//---------------------------------------------------------------------------
inline FileDriver::Cache & FileDriver::cache()
{
  return *reinterpret_cast<Cache *>(cacheHolder_);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline FileDriver::ExtentMap::~ExtentMap()
{
}
//---------------------------------------------------------------------------
inline FileDriver::ExtentMap::ExtentMap() :
  minSize_(0), maxSize_(getpagesize()), curSize_(0)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline FileDriver::ExtentMap::EMA::~EMA()
{
}
//---------------------------------------------------------------------------
inline FileDriver::ExtentMap::EMA::EMA(
  uint64_t fileNameHash,ussaddr_t virtualAddress,ussaddr_t physicalAddress) :
    mapNode_(NULL),
    fileNameHash_(fileNameHash),
    virtualAddress_(virtualAddress),
    physicalAddress_(physicalAddress)
{
}
//---------------------------------------------------------------------------
inline uintptr_t FileDriver::ExtentMap::EMA::hash(const EMA & object)
{
  return ksys::HF::hash(
    &object.fileNameHash_,
    sizeof(object.fileNameHash_) + sizeof(object.virtualAddress_),
    0
  ); 
}
//---------------------------------------------------------------------------
inline bool FileDriver::ExtentMap::EMA::equ(const EMA & o1,const EMA & o2)
{
  return o1.fileNameHash_ == o2.fileNameHash_ && o1.virtualAddress_ == o2.virtualAddress_;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline FileDriver::Cache::~Cache()
{
}
//---------------------------------------------------------------------------
inline FileDriver::Cache::Cache() :
  minSize_(0), maxSize_(512 * 1024), curSize_(0), flushDelay_(1000000)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline FileDriver::Cache::Line::~Line()
{
}
//---------------------------------------------------------------------------
inline FileDriver::Cache::Line::Line(uint64_t fileNameHash,ussaddr_t virtualAddress) :
  cacheNode_(NULL),
  queueLength_(0),
  fileNameHash_(fileNameHash),
  virtualAddress_(virtualAddress),
  state_(stReady)
{
}
//---------------------------------------------------------------------------
inline uintptr_t FileDriver::Cache::Line::vaHash(const Line & object)
{
  return ksys::HF::hash(
    &object.fileNameHash_,
    sizeof(object.fileNameHash_) + sizeof(object.virtualAddress_)
  ); 
}
//---------------------------------------------------------------------------
inline bool FileDriver::Cache::Line::vaEqu(const Line & o1,const Line & o2)
{
  return o1.fileNameHash_ == o2.fileNameHash_ && o1.virtualAddress_ == o2.virtualAddress_;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class RecordDescriptor {
  public:
    ~RecordDescriptor();
    RecordDescriptor();
    
    RecordDescriptor & seek(ussaddr_t pos);
    ussaddr_t tell();
    intptr_t read(void * buf,uintptr_t len);
    intptr_t write(const void * buf,uintptr_t len);
    intptr_t read(void * buf,ussaddr_t pos,uintptr_t len);
    intptr_t write(const void * buf,ussaddr_t pos,uintptr_t len);
  protected:
  private:
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class StorageDriver { // database level driver
  public:
    ~StorageDriver();
    StorageDriver();
    
    StorageDriver & attach(const utf8::String & name);
    StorageDriver & detach();
  protected:
  private:
    ksys::FiberInterlockedMutex mutex_;
    class File {
      friend class StorageDriver;
      public:
        ~File();
        File();
      protected:
      private:
        FileParams params_;
        ksys::List<FileDriver> handles_;
    };
    ksys::Vector<File> files_;
    ksys::List<RecordDescriptor> records_;
};
//---------------------------------------------------------------------------
inline StorageDriver::~StorageDriver()
{
}
//---------------------------------------------------------------------------
inline StorageDriver::StorageDriver()
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline StorageDriver::File::~File()
{
}
//---------------------------------------------------------------------------
inline StorageDriver::File::File()
{
}
//---------------------------------------------------------------------------
} // namespace uss
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
