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
#include <adicpp/adicpp.h>
#include "uss.h"
//---------------------------------------------------------------------------
namespace uss {
//---------------------------------------------------------------------------
void initialize()
{
  new (&FileDriver::extentMap()) FileDriver::ExtentMap;
  new (&FileDriver::cache()) FileDriver::Cache;
}
//---------------------------------------------------------------------------
void cleanup()
{
  FileDriver::cache().~Cache();
  FileDriver::extentMap().~ExtentMap();
}
//---------------------------------------------------------------------------
extern const uint8_t ussFileSignature_[16] = {
  0x75, 0xF6, 0x8E, 0xC3, 0x9F, 0x18, 0x47, 0xBD,
  0xBF, 0x6D, 0x38, 0x12, 0xFC, 0x84, 0x49, 0x89
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
ussaddr_t FileParams::nearestPowerOfTwo(ussaddr_t a)
{
  ussaddr_t q = ussaddr_t(1) << 63;
  while( q > a ) q >>= 1;
  return q;
}
//---------------------------------------------------------------------------
uintptr_t FileParams::bitSize(ussaddr_t a)
{
  uintptr_t q = 0;
  while( (ussaddr_t(1) << q) != a ) q++;
  return q;
}
//---------------------------------------------------------------------------
void FileParams::calc(
  uintptr_t addressBitSize,
  uintptr_t sectorBitSize,
  uintptr_t clusterBitSize,
  uintptr_t extentBitSize)
{
  if( addressBitSize < 17 ) addressBitSize = 40;
  if( sectorBitSize < 5 || sectorBitSize > 10 ) sectorBitSize = 5;
  if( addressBitSize + sectorBitSize > sizeof(ussaddr_t) * 8 ){
    calc(addressBitSize >> 1,sectorBitSize,clusterBitSize,extentBitSize);
    return;
  }
  addressBitSize_ = addressBitSize;
  sectorBitSize_ = sectorBitSize;
  clusterBitSize_ = clusterBitSize;
  extentBitSize_ = extentBitSize;
  addressSize_ = nearest(nearest(addressBitSize_,8) / 8,4);
  sectorSize_ = powerOfTwo(sectorBitSize_);
  clusterSize_ = powerOfTwo(clusterBitSize_);
  extentSize_ = powerOfTwo(extentBitSize_);
  if( sectorSize_ * 8 * sectorSize_ > clusterSize_ ){
    clusterSize_ = sectorSize_ * 8 * sectorSize_;
    clusterBitSize_ = bitSize(clusterSize_);
  }
  if( sectorSize_ * 8 * clusterSize_ > extentSize_ ){
    extentSize_ = sectorSize_ * 8 * clusterSize_;
    extentBitSize_ = bitSize(extentSize_);
  }
  if( addressBitSize_ + sectorBitSize_ > sizeof(ussaddr_t) * 8 ){
    calc(addressBitSize_ - 1,sectorBitSize_,clusterBitSize_,extentBitSize_);
    return;
  }

  sectorBitMask_ = powerOfTwo(sectorBitSize_) - 1;
  sectorInvertedBitMask_ = ~sectorBitMask_;
  
  clusterBitShift_ = sectorBitSize_;
  clusterBitMask_ = powerOfTwo(clusterBitSize_) - 1;
  clusterInvertedBitMask_ = ~clusterBitMask_;
  clusterShiftedBitMask_ = clusterBitMask_ << sectorBitSize_;
  clusterInvertedShiftedBitMask_ = ~clusterShiftedBitMask_;
  
  extentBitShift_ = sectorBitSize_ + clusterBitSize_;
  extentBitMask_ = powerOfTwo(sectorBitSize_ + clusterBitSize_) - 1;
  extentInvertedBitMask_ = ~extentBitMask_;
  extentShiftedBitMask_ = extentBitMask_ << (sectorBitSize_ + clusterBitSize_);
  extentInvertedShiftedBitMask_ = ~extentShiftedBitMask_;

  clusterAndSectorShiftedBitMask_ = sectorBitMask_ | clusterShiftedBitMask_;
  clusterAndSectorInvertedShiftedBitMask_ = ~clusterAndSectorShiftedBitMask_;

  extentAndClusterShiftedBitMask_ = extentShiftedBitMask_ | clusterShiftedBitMask_;
  extentAndClusterInvertedShiftedBitMask_ = ~extentAndClusterShiftedBitMask_;

  sectorsInCluster_ = clusterSize_ >> sectorBitSize_;
  sectorsInClusterBit_ = bitSize(sectorsInCluster_);
  sectorsInClusterBitMask_ = sectorsInCluster_ - 1;
  clustersInExtent_ = extentSize_ >> clusterBitSize_;
  clustersInExtentBit_ = bitSize(clustersInExtent_);
  clustersInExtentBitMask_ = clustersInExtent_ - 1;

  headerSize_ = nearest(sizeof(FileHeader),sectorSize_);
  
  mapTable_.extents_ = powerOfTwo(addressBitSize_ + sectorBitSize_ - extentBitSize_);
  mapTable_.tableSize_ = mapTable_.extents_ * sizeof(ussaddr_t);
  mapTable_.pageSize_ = clusterSize_;
  mapTable_.pageBitSize_ = bitSize(mapTable_.pageSize_);
  mapTable_.extentsInPage_ = mapTable_.pageSize_ / sizeof(ussaddr_t);
  mapTable_.extentsInPageBit_ = bitSize(mapTable_.extentsInPage_);
  mapTable_.extentsInPageBitMask_ = mapTable_.extentsInPage_ - 1;
  mapTable_.pagesInCatalog_ = mapTable_.tableSize_ / clusterSize_;
  mapTable_.catalogSize_ = mapTable_.pagesInCatalog_ * sizeof(ussaddr_t);
  mapTable_.catalogPageSize_ = clusterSize_;
  mapTable_.catalogPageBitSize_ = bitSize(mapTable_.catalogPageSize_);
  mapTable_.pagesInCatalogPage_ = mapTable_.catalogPageSize_ / sizeof(ussaddr_t);
  mapTable_.pagesInCatalogPageBit_ = bitSize(mapTable_.pagesInCatalogPage_);
  mapTable_.pagesInCatalogPageBitMask_ = mapTable_.pagesInCatalogPage_ - 1;
  if( mapTable_.catalogSize_ > extentSize_ - clusterSize_ ){
    calc(addressBitSize_,sectorBitSize_,clusterBitSize_,extentBitSize_ + 1);
    return;
  }
  
  clusterBitmap_.bitSize_ = bitSize((clusterSize_ >> sectorBitSize_) / 8);
  clusterBitmap_.sectorSize_ = clusterSize_ >> sectorBitSize_;
  clusterBitmap_.size_ = (clusterSize_ >> sectorBitSize_) / 8;

  extentBitmap_.bitSize_ = bitSize((extentSize_ >> clusterBitSize_) / 8);
  extentBitmap_.sectorSize_ = extentSize_ >> clusterBitSize_;
  extentBitmap_.size_ = (extentSize_ >> clusterBitSize_) / 8;
  if( extentBitmap_.size_ > clusterSize_ - clusterBitmap_.size_ ){
    calc(addressBitSize_,sectorBitSize_,clusterBitSize_ + 1,extentBitSize_);
    return;
  }  

  clusterBitmap_.first_.realloc(clusterBitmap_.size_);
  memset(clusterBitmap_.first_,0,clusterBitmap_.size_);
  clusterBitmap_.first_.setBitRange(
    0,clusterBitmap_.bitSize_ + extentBitmap_.bitSize_
  );
  clusterBitmap_.regular_.realloc(clusterBitmap_.size_);
  memset(clusterBitmap_.regular_,0,clusterBitmap_.size_);
  clusterBitmap_.regular_.setBitRange(0,clusterBitmap_.bitSize_);
  
//  extentBitmap_.first_.realloc(extentBitmap_.size_);
//  memset(extentBitmap_.first_,0,extentBitmap_.size_);
  extentBitmap_.regular_.realloc(extentBitmap_.size_);
  memset(extentBitmap_.regular_,0,extentBitmap_.size_);
  clusterBitmap_.firstOffset_ = headerSize_ >> sectorBitSize_;
  clusterBitmap_.regularOffset_ = 0;
  extentBitmap_.firstOffset_ = (headerSize_ + clusterBitmap_.size_) >> sectorBitSize_;
  extentBitmap_.regularOffset_ = clusterBitmap_.size_ >> sectorBitSize_;
}
//---------------------------------------------------------------------------
void FileParams::create(const utf8::String & name)
{
  ksys::FileHandleContainer file(name);
  file.open();
  file.resize(0);
  ksys::AutoPtr<uint8_t> cluster;
  cluster.alloc(clusterSize_);
  memset(cluster,0,clusterSize_);
  FileHeader * pHdr = reinterpret_cast<FileHeader *>(cluster.ptr());
  memcpy(pHdr->signature_,ussFileSignature_,sizeof(pHdr->signature_));
  pHdr->version_.release_ = 0;
  pHdr->version_.revision_ = 0;
  pHdr->version_.level_ = 0;
  pHdr->sizes_.addressBitSize_ = uint8_t(addressBitSize_);
  pHdr->sizes_.sectorBitSize_ = uint8_t(sectorBitSize_);
  pHdr->sizes_.clusterBitSize_ = uint8_t(clusterBitSize_);
  pHdr->sizes_.extentBitSize_ = uint8_t(extentBitSize_);
  pHdr->catalogAddress_ = clusterSize_ >> sectorBitSize_;
  ksys::sha256(pHdr,sizeof(FileHeader),pHdr->checksum_);
// set first cluster bitmap
  cluster.setBitRange(
    headerSize_ * 8,
    (headerSize_ + clusterBitmap_.size_ + extentBitmap_.size_) >> sectorBitSize_
  );
// set first extent bitmap
  cluster.setBitRange(
    (headerSize_ + clusterBitmap_.size_) * 8 + 1,
    mapTable_.catalogSize_ >> clusterBitSize_
  );
  file.write(cluster,clusterSize_);
  memset(cluster,0,clusterSize_);
  for( ussaddr_t i = 0; i < (mapTable_.catalogSize_ >> clusterBitSize_); i++ )
    file.write(cluster,clusterSize_);
}
//---------------------------------------------------------------------------
void FileParams::fmtStr(char * fmt,const char * s1,const char * s2,const char * s3)
{
  strcat(fmt,s1);
  strcat(fmt,s2);
  strcat(fmt,s3);
}
//---------------------------------------------------------------------------
void FileParams::print(const utf8::String & name)
{
  ksys::FileHandleContainer file(name);
  file.open();
  FileHeader hdr;
  file.read(&hdr,sizeof(hdr));
  if( memcmp(hdr.signature_,ussFileSignature_,sizeof(hdr.signature_)) != 0 ){
    fprintf(
      stderr,"%s may be corrupted or not uss file, invalid signature\n",
      (const char *) name.getANSIString()
    );
  }
  else {
    union {
      char fmt[1024];
      uint8_t sha[32];
    };
    memcpy(sha,hdr.checksum_,sizeof(sha));
    memset(hdr.checksum_,0,sizeof(hdr.checksum_));
    ksys::sha256(&hdr,sizeof(hdr),hdr.checksum_);
    if( memcmp(hdr.checksum_,sha,sizeof(sha)) != 0 ){
      fprintf(
        stderr,
	"%s header corrupted or not uss file, invalid header checksum\n",
        (const char *) name.getANSIString()
      );
    }
    else {
      FileParams calc;
      calc.calc(
        hdr.sizes_.addressBitSize_,
        hdr.sizes_.sectorBitSize_,
	hdr.sizes_.clusterBitSize_,
	hdr.sizes_.extentBitSize_
      );
      strcpy(fmt,
	"Universal System Storage file\n"
	"                           structure version: %u.%u.%u\n"
	"                            address bit size: %u\n"
	"                           address byte size: %"PRIuPTR"\n"
      );
      fmtStr(fmt,"                                 sector size: %",PRIu64,"\n");
      fmtStr(fmt,"                                cluster size: %",PRIu64,"\n");
      fmtStr(fmt,"                                 extent size: %",PRIu64,"\n");
      fmtStr(fmt,"                         cluster bitmap size: %",PRIu64,"\n");
      fmtStr(fmt,"                          extent bitmap size: %",PRIu64,"\n");
      fmtStr(fmt,"                    max extents in map table: %",PRIu64,"\n");
      fmtStr(fmt,"                      extents map table size: %",PRIu64,"\n");
      fmtStr(fmt,"                   extents in map table page: %",PRIu64,"\n");
      fmtStr(fmt,"                        map table page space: %",PRIu64,"");
      fmtStr(fmt," %",PRIu64,"K");
      fmtStr(fmt," %",PRIu64,"M");
      fmtStr(fmt," %",PRIu64,"G");
      fmtStr(fmt," %",PRIu64,"T\n");
      fmtStr(fmt,"                         map table page size: %",PRIu64,"\n");
      fmtStr(fmt,"  map table pages in map table pages catalog: %",PRIu64,"\n");
      fmtStr(fmt,"                map table pages catalog size: %",PRIu64,"\n");
      fmtStr(fmt,"            map table pages catalog clusters: %",PRIu64,"\n");
      fmtStr(fmt,"          map table pages catalog page space: %",PRIu64,"");
      fmtStr(fmt," %",PRIu64,"K");
      fmtStr(fmt," %",PRIu64,"M");
      fmtStr(fmt," %",PRIu64,"G");
      fmtStr(fmt," %",PRIu64,"T\n");
//      fprintf(stderr,"%u\n",strlen(fmt));
      fprintf(
        stderr,
        fmt,
	hdr.version_.release_,
	hdr.version_.revision_,
	hdr.version_.level_,
	hdr.sizes_.addressBitSize_,
        calc.addressSize_,
	powerOfTwo(hdr.sizes_.sectorBitSize_),
	powerOfTwo(hdr.sizes_.clusterBitSize_),
	powerOfTwo(hdr.sizes_.extentBitSize_),
        calc.clusterBitmap_.size_,
        calc.extentBitmap_.size_,
        calc.mapTable_.extents_,
        calc.mapTable_.tableSize_,
        calc.mapTable_.extentsInPage_,
        calc.mapTable_.extentsInPage_ * calc.extentSize_,
        calc.mapTable_.extentsInPage_ * calc.extentSize_ / 1024u,
        calc.mapTable_.extentsInPage_ * calc.extentSize_ / 1024u / 1024u,
        calc.mapTable_.extentsInPage_ * calc.extentSize_ / 1024u / 1024u / 1024u,
        calc.mapTable_.extentsInPage_ * calc.extentSize_ / 1024u / 1024u / 1024u / 1024u,
        calc.mapTable_.pageSize_,
        calc.mapTable_.pagesInCatalog_,
        calc.mapTable_.catalogSize_,
        calc.mapTable_.catalogSize_ >> calc.clusterBitSize_,
	calc.mapTable_.pagesInCatalog_ * calc.mapTable_.extentsInPage_ *
	  calc.extentSize_,
	calc.mapTable_.pagesInCatalog_ * calc.mapTable_.extentsInPage_ *
	  calc.extentSize_ / 1024u,
	calc.mapTable_.pagesInCatalog_ * calc.mapTable_.extentsInPage_ *
	  calc.extentSize_ / 1024u / 1024u,
	calc.mapTable_.pagesInCatalog_ * calc.mapTable_.extentsInPage_ *
	  calc.extentSize_ / 1024u / 1024u / 1024u,
	calc.mapTable_.pagesInCatalog_ * calc.mapTable_.extentsInPage_ *
	  calc.extentSize_ / 1024u / 1024u / 1024u / 1024u
      );
    }
  }
}
//---------------------------------------------------------------------------
void FileParams::initialize(const FileHeader & hdr)
{
  catalogAddress_ = hdr.catalogAddress_;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
uint8_t FileDriver::extentMapHolder_[sizeof(ExtentMap)];
uint8_t FileDriver::cacheHolder_[sizeof(Cache)];
//---------------------------------------------------------------------------
void FileDriver::print(const utf8::String & name)
{
  FileParams::print(name);
}
//---------------------------------------------------------------------------
void FileDriver::create(const utf8::String & name)
{
  FileParams params;
  params.calc();
  params.create(name);
}
//---------------------------------------------------------------------------
FileDriver & FileDriver::open(const utf8::String & name)
{
  file_.fileName(name);
  fileNameHash_ = name.hash_ll(true);
  file_.open();
  FileHeader hdr;
  file_.read(&hdr,sizeof(hdr));
  if( memcmp(hdr.signature_,ussFileSignature_,sizeof(hdr.signature_)) == 0 ){
    uint8_t sha[32];
    memcpy(sha,hdr.checksum_,sizeof(sha));
    memset(hdr.checksum_,0,sizeof(hdr.checksum_));
    ksys::sha256(&hdr,sizeof(hdr),hdr.checksum_);
    if( memcmp(hdr.checksum_,sha,sizeof(sha)) == 0 ){
      if( params_ != NULL ){
        params_->calc(
          hdr.sizes_.addressBitSize_,
	  hdr.sizes_.sectorBitSize_,
	  hdr.sizes_.clusterBitSize_,
	  hdr.sizes_.extentBitSize_
        );
        params_->initialize(hdr);
      }
      return *this;
    }
  }
  throw ksys::ExceptionSP(new ksys::Exception(EINVAL,__PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
FileDriver & FileDriver::close()
{
  file_.close();
  return *this;
}
//---------------------------------------------------------------------------
ussaddr_t FileParams::getClusterBitmapAddress(ussaddr_t inExtentAddress,ussaddr_t cluster) const
{
  ussaddr_t clusterBitmapAddress = cluster << clusterBitShift_;
  clusterBitmapAddress += inExtentAddress & extentInvertedShiftedBitMask_;
  clusterBitmapAddress += clusterBitmap_.offsets_[clusterBitmapAddress != 0];
  return clusterBitmapAddress;
}
//---------------------------------------------------------------------------
ussaddr_t FileParams::getExtentBitmapAddress(ussaddr_t inExtentAddress) const
{
  ussaddr_t extentBitmapAddress = inExtentAddress & extentInvertedShiftedBitMask_;
  extentBitmapAddress += extentBitmap_.offsets_[extentBitmapAddress != 0];
  return extentBitmapAddress;
}
//---------------------------------------------------------------------------
FileDriver & FileDriver::alloc(ussaddr_t count,ussaddr_t & address,ussint_t & allocated)
{
  assert( count > 0 );
  uintptr_t i, j, k;
  Cache::Line * line, * line2;
  uint8_t * bitmap, * bitmap2;
  ussaddr_t clusterBitmapAddress, extentBitmapAddress, bound;
  extentBitmapAddress = params_->extentBitmap_.firstOffset_;
  address = 0;
  allocated = 0;
  if( count > params_->sectorsInCluster_ )
    count += params_->sectorsInCluster_ - (count & params_->sectorsInClusterBitMask_);
  for(;;){
    bound = (extentBitmapAddress >> params_->extentBitSize_) + 1;
    bound <<= params_->extentBitSize_ + params_->sectorBitSize_;
    if( file_.size() < bound ){
      line = pin(0,1,O_EXLOCK);
      try {
        file_.resize(bound);
      }
      catch( ksys::ExceptionSP & e ){
        unpin(line);
        throw;
      }
      unpin(line);
    }
    line = pin(extentBitmapAddress,1,O_EXLOCK);
    bitmap = line->buffer_.ptr() +
      ((extentBitmapAddress & params_->sectorBitMask_) << params_->sectorBitSize_);
    for( i = 0; i < params_->extentBitmap_.bitSize_; i++ ){
      if( ksys::bit(bitmap,i) != 0 ) continue;
      if( count < params_->sectorsInCluster_ ){
        clusterBitmapAddress = params_->getClusterBitmapAddress(extentBitmapAddress,i);
        line2 = line;
        if( (clusterBitmapAddress & params_->sectorInvertedBitMask_) != line->virtualAddress_ ){
	  try {
	    bound = (clusterBitmapAddress >> params_->clusterBitShift_) + 1;
	    bound <<= params_->clusterBitShift_ + params_->sectorBitSize_;
	    if( file_.size() < bound ) file_.resize(bound);
	    line2 = pin(clusterBitmapAddress,1,O_EXLOCK);
	  }
	  catch( ksys::ExceptionSP & e ){
            unpin(line);
	    throw;
	  }
	}
        bitmap2 = line2->buffer_.ptr() +
          ((clusterBitmapAddress & params_->sectorBitMask_) << params_->sectorBitSize_);
        for( j = 0; j < params_->clusterBitmap_.bitSize_; j++ )
          if( ksys::bit(bitmap2,j) == 0 ) break;
        for( k = j; k < params_->clusterBitmap_.bitSize_ && k - i < count; k++ )
          if( ksys::bit(bitmap2,k) != 0 ) break;
	address = j;
	address += clusterBitmapAddress & params_->sectorInvertedBitMask_;
	allocated = k;
	allocated -= j;
	if( allocated > 0 ){
	  ksys::setBitRange(bitmap2,j,k - j);
	  setDirty(line2);
	}
        if( line2 != line ) unpin(line2);
        if( allocated == 0 ){
	  ksys::setBit(bitmap,i);
	  setDirty(line);
	}
      }
      break;
    }
    if( count > params_->sectorsInCluster_ ){
      for( j = i; j < params_->extentBitmap_.bitSize_; j++ )
        if( ksys::bit(bitmap,j) != 0 ) break;
      address = i;
      address <<= params_->sectorBitSize_;
      address += extentBitmapAddress & params_->clusterAndSectorInvertedShiftedBitMask_;
      allocated = j;
      allocated -= i;
      allocated <<= params_->sectorsInClusterBit_;
      if( allocated == count ){
        ksys::setBitRange(bitmap2,i,j - i);
        setDirty(line);
      }
      else {
        address = 0;
        allocated = 0;
      }
    }
    unpin(line);
    if( allocated > 0 ) break;
    extentBitmapAddress >>= params_->extentBitShift_;
    extentBitmapAddress++;
    extentBitmapAddress <<= params_->extentBitShift_;
    extentBitmapAddress += params_->extentBitmap_.regularOffset_;
  }
  return *this;
}
//---------------------------------------------------------------------------
FileDriver & FileDriver::free(ussaddr_t address,ussint_t count)
{
  assert( address > 0 && count > 0 );
  ussaddr_t extentBitmapAddress = params_->getExtentBitmapAddress(address);
  if( count > params_->sectorsInCluster_ )
    count += params_->sectorsInCluster_ - (count & params_->sectorsInClusterBitMask_);
  Cache::Line * line = pin(extentBitmapAddress,1,O_EXLOCK);
  uint8_t * bitmap = line->buffer_.ptr() +
    ((extentBitmapAddress & params_->sectorBitMask_) << params_->sectorBitSize_);
  ussint_t clusterNo = (address & params_->clusterShiftedBitMask_) >> params_->clusterBitShift_;
  if( count < params_->sectorsInCluster_ ){
    ussaddr_t clusterBitmapAddress = params_->getClusterBitmapAddress(
      extentBitmapAddress,clusterNo
    );
    Cache::Line * line2 = line;
    if( (clusterBitmapAddress & params_->sectorInvertedBitMask_) != line->virtualAddress_ ){
      try {
        line2 = pin(clusterBitmapAddress,1,O_EXLOCK);
      }
      catch( ksys::ExceptionSP & e ){
        unpin(line);
        throw;
      }
    }
    uint8_t * bitmap2 = line2->buffer_.ptr() +
      ((clusterBitmapAddress & params_->sectorBitMask_) << params_->sectorBitSize_);
    ksys::resetBitRange(bitmap2,address & params_->sectorBitMask_,count);
    setDirty(line2);
    if( line2 != line ) unpin(line2);
    if( ksys::bit(bitmap,clusterNo) ){
      ksys::resetBit(bitmap,clusterNo);
      setDirty(line);
    }
  }
  else {
    ksys::resetBitRange(bitmap,clusterNo,count);
    setDirty(line);
  }
  unpin(line);
  return *this;
}
//---------------------------------------------------------------------------
FileDriver & FileDriver::read(void * buf,ussaddr_t address,ussint_t count)
{
  assert( count > 0 && (address & params_->sectorBitMask_) + count <= params_->sectorsInCluster_ );
  Cache::Line * line = pin(address,1,O_SHLOCK);
  memcpy(
    buf,
    line->buffer_.ptr() + ((address & params_->sectorBitMask_) << params_->sectorBitSize_),
    count << params_->sectorBitSize_
  );
  unpin(line);
  return *this;
}
//---------------------------------------------------------------------------
FileDriver & FileDriver::write(const void * buf,ussaddr_t address,ussint_t count)
{
  assert( count > 0 && (address & params_->sectorBitMask_) + count <= params_->sectorsInCluster_ );
  Cache::Line * line = pin(address,1,O_EXLOCK);
  memcpy(
    line->buffer_.ptr() + ((address & params_->sectorBitMask_) << params_->sectorBitSize_),
    buf,
    count << params_->sectorBitSize_
  );
  setDirty(line).unpin(line);
  return *this;
}
//---------------------------------------------------------------------------
FileDriver & FileDriver::pack()
{
  return *this;
}
//---------------------------------------------------------------------------
ussaddr_t FileParams::getCatalogPageAddress(ussaddr_t virtualAddress) const
{
  ussaddr_t extent = virtualAddress >> extentBitShift_;
  ussaddr_t page = extent >> mapTable_.extentsInPageBit_;
  ussaddr_t catalogPage = page >> mapTable_.pagesInCatalogPageBit_;
  return (catalogPage << mapTable_.catalogPageBitSize_) >> sectorsInClusterBit_;
}
//---------------------------------------------------------------------------
ussaddr_t FileParams::getPageIndexInCatalogPage(ussaddr_t virtualAddress) const
{
  ussaddr_t extent = virtualAddress >> extentBitShift_;
  ussaddr_t page = extent >> mapTable_.extentsInPageBit_;
  return page & mapTable_.pagesInCatalogPageBitMask_;
}
//---------------------------------------------------------------------------
ussaddr_t FileParams::getExtentIndexInPage(ussaddr_t virtualAddress) const
{
  ussaddr_t extent = virtualAddress >> extentBitShift_;
  return extent & mapTable_.extentsInPageBitMask_;
}
//---------------------------------------------------------------------------
FileDriver & FileDriver::map(ussaddr_t virtualAddress,ussaddr_t & physicalAddress)
{
  ussaddr_t address = virtualAddress & params_->extentAndClusterInvertedShiftedBitMask_;
  physicalAddress = 0;
  extentMap().acquire();
  while( extentMap().map_.count() > 0 ){
    if( extentMap().curSize_ >= extentMap().minSize_ &&
        extentMap().curSize_ <= extentMap().maxSize_ ) break;
    ExtentMap::EMA & object = ExtentMap::EMA::nodeObject(*extentMap().lru_.last());
    extentMap().hash_.remove(object);
    extentMap().lru_.remove(object);
    extentMap().map_.remove(object.mapNode_);
    extentMap().curSize_ -= sizeof(ExtentMap::EMA);
  }
  uint8_t e[sizeof(ExtentMap::EMA)];
  ExtentMap::EMA * ema = reinterpret_cast<ExtentMap::EMA *>(e);
  ema->fileNameHash_ = fileNameHash_;
  ema->virtualAddress_ = address;
  ema = extentMap().hash_.find(*ema);
  if( ema == NULL ){
    extentMap().release();
    Cache::Line * line = pin(
      params_->catalogAddress_ + params_->getCatalogPageAddress(address),
      params_->mapTable_.catalogPageSize_ >> params_->clusterBitSize_,
      O_DIRECT | O_SHLOCK
    );
    ussaddr_t mappedAddress =
      *((ussaddr_t *) line->buffer_.ptr() +
        params_->getPageIndexInCatalogPage(address));
    unpin(line);
    if( mappedAddress != 0 ){
      line = pin(
        mappedAddress + params_->getCatalogPageAddress(address),
        params_->mapTable_.pageSize_ >> params_->clusterBitSize_,
        O_DIRECT | O_SHLOCK
      );
      mappedAddress =
        *((ussaddr_t *) line->buffer_.ptr() +
           params_->getExtentIndexInPage(address));
      unpin(line);
      if( mappedAddress != 0 )
        physicalAddress = mappedAddress +
	  (virtualAddress & (params_->clusterShiftedBitMask_ | params_->sectorBitMask_));
    }
    extentMap().acquire();
    ema = extentMap().hash_.find(*reinterpret_cast<ExtentMap::EMA *>(e));
    if( ema == NULL ){
      ema = &extentMap().map_.insToTail().object();
      ema->mapNode_ = extentMap().map_.last();
      extentMap().hash_.insert(*ema);
    }
  }
  else {
    physicalAddress = ema->physicalAddress_;
    extentMap().lru_.remove(*ema);
  }
  extentMap().lru_.insToHead(*ema);
  extentMap().curSize_ += sizeof(ExtentMap::EMA);
  extentMap().release();
  if( physicalAddress == 0 ) physicalAddress = virtualAddress;
  return *this;
}
//---------------------------------------------------------------------------
FileDriver::Cache::Line & FileDriver::Cache::Line::releaseQueue()
{
  for( intptr_t i = queueLength_ - 1; i >= 0; i-- ) queue_.post();
  return *this;
}
//---------------------------------------------------------------------------
FileDriver & FileDriver::flushDirtyCacheLines()
{
  Cache::Line * line;
  ksys::EmbeddedListNode<Cache::Line> * lineNode;
  int64_t t = getlocaltimeofday();
  lineNode = cache().dirty_.first();
  while( lineNode != NULL ){
    line = &Cache::Line::dirtyObject(*lineNode);
    lineNode = lineNode->next();
    if( line->flushTime_ + cache().flushDelay_ >= t ) break;
    if( line->state_ == Cache::Line::stReady && line->tryRDLock() ){
      bool flushed = false;
      assert( file_.size() >= (line->physicalAddress_ << params_->sectorBitSize_) + line->size_ );
      line->state_ = Cache::Line::stWriting;
      cache().dirty_.remove(*line);
      cache().release();
      try {
        file_.write(
	  line->physicalAddress_ << params_->sectorBitSize_,
	  line->buffer_,
	  line->size_
	);
        flushed = true;
      }
      catch( ksys::ExceptionSP & e ){
        e->writeStdError();
      }
      catch( ... ){}
      cache().acquire();
      if( flushed ){
        line->state_ = Cache::Line::stReady;
        line->dirty_ = false;
        line->flushTime_ = getlocaltimeofday();
      }
      else {
        cache().dirty_.insToHead(*line);
      }
      line->unlock();
      line->releaseQueue();
      lineNode = cache().dirty_.first();
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
FileDriver & FileDriver::removeLRUCacheLines()
{
  Cache::Line * line;
  ksys::EmbeddedListNode<Cache::Line> * lineNode;
  while( cache().lru_.count() > 0 ){
    if( cache().curSize_ >= cache().minSize_ &&
        cache().curSize_ <= cache().maxSize_ ) break;
    lineNode = cache().lru_.last();
    while( lineNode != NULL ){
      line = &Cache::Line::lruObject(*lineNode);
      lineNode = lineNode->prev();
      assert(
	line->state_ == Cache::Line::stReady ||
	line->state_ == Cache::Line::stWriting
      );
      cache().lru_.remove(*line);
      cache().vaHash_.remove(*line);
      cache().dead_.insToTail(*line);
      cache().curSize_ -= line->size_ + sizeof(Cache::Line);
      line->releaseQueue();
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
FileDriver & FileDriver::removeDeadCacheLines()
{
  Cache::Line * line;
  ksys::EmbeddedListNode<Cache::Line> * lineNode;
  lineNode = cache().dead_.first();
  while( lineNode != NULL ){
    line = &Cache::Line::deadObject(*lineNode);
    lineNode = lineNode->next();
    assert( line->queueLength_ == 0 );
    if( line->tryWRLock() ){
      cache().dead_.remove(*line);
      line->unlock();
      cache().cache_.remove(line->cacheNode_);
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
FileDriver::Cache::Line * FileDriver::pin(ussaddr_t firstClusterSectorAddress,ussint_t clusterCount,uintptr_t flags)
{
  firstClusterSectorAddress &= params_->sectorInvertedBitMask_;
  assert(
    firstClusterSectorAddress != 0 &&
    clusterCount > 0 && clusterCount < params_->clustersInExtent_ - 1 &&
    ((flags & O_SHLOCK) != 0 || (flags & O_EXLOCK) != 0)
  );
  Cache::Line * line, l(fileNameHash_,firstClusterSectorAddress);
  do {
    cache().acquire();
    flushDirtyCacheLines();
    removeLRUCacheLines();
    removeDeadCacheLines();
    uint8_t l[sizeof(Cache::Line)];
    Cache::Line * line = reinterpret_cast<Cache::Line *>(l);
    line->fileNameHash_ = fileNameHash_;
    line->virtualAddress_ = firstClusterSectorAddress;
    line = cache().vaHash_.find(*line);
    if( line == NULL ){
      try {
        line = &cache().cache_.insToTail().object();
	line->cacheNode_ = cache().cache_.last();
        line->size_ = clusterCount * params_->clusterSize_;
        line->buffer_.alloc(line->size_);
      }
      catch( ksys::ExceptionSP & e ){
        if( line != NULL ) cache().cache_.remove(line->cacheNode_);
        e->writeStdError();
        if( e->code() != ENOMEM ){
          cache().release();
          throw;
        }
      }
      line->fileNameHash_ = fileNameHash_;
      line->virtualAddress_ = firstClusterSectorAddress;
      line->physicalAddress_ = 0;
      line->flushTime_ = 0;
      line->dirty_ = false;
      line->state_ = Cache::Line::stReading;
      line->wrLock();
      cache().vaHash_.insert(*line);
      cache().release();
      try {
        if( flags & O_DIRECT ){
	  line->physicalAddress_ = firstClusterSectorAddress;
	}
	else {
	  map(line->virtualAddress_,line->physicalAddress_);
	}
        file_.read(
          line->physicalAddress_ << params_->sectorBitSize_,
 	  line->buffer_,
	  line->size_
	);
      }
      catch( ... ){
        cache().acquire();
        line->releaseQueue();
	cache().vaHash_.remove(*line);
	cache().dead_.insToTail(*line);
	throw;
      }
      line->state_ = Cache::Line::stReady;
      line->unlock();
      cache().acquire();
      line->releaseQueue();
      cache().curSize_ += line->size_ + sizeof(Cache::Line);
    }
    else {
      cache().lru_.remove(*line);
    }
    cache().lru_.insToHead(*line);
    bool locked;
    if( flags & O_SHLOCK ){
      locked = line->tryRDLock();
    }
    else if( flags & O_EXLOCK ){
      locked = line->tryWRLock();
    }
    if( !locked && (flags & O_NONBLOCK) == 0 ){
      line->queueLength_++;
      cache().release();
      line->queue_.wait();
      cache().acquire();
      line->queueLength_--;
    }
    cache().release();
  } while( (flags & O_NONBLOCK) == 0 && line == NULL );
  return line;
}
//---------------------------------------------------------------------------
FileDriver & FileDriver::unpin(Cache::Line * line)
{
  assert( line != NULL );
  line->unlock();
  return *this;
}
//---------------------------------------------------------------------------
FileDriver & FileDriver::setDirty(Cache::Line * line)
{
  assert( line != NULL );
  cache().acquire();
  if( !line->dirty_ ){
    line->dirty_ = true;
    line->flushTime_ = getlocaltimeofday();
    cache().dirty_.insToTail(*line);
  }
  cache().release();
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
StorageDriver & StorageDriver::attach(const utf8::String & name)
{
  if( files_.count() > 0 )
    throw ksys::ExceptionSP(new ksys::Exception(EALREADY,__PRETTY_FUNCTION__));
  try {
    File & file = files_.add();
    FileDriver & driver = file.handles_.insToTail().object();
    driver.params_ = &file.params_;
    driver.open(name);
  }
  catch( ... ){
    files_.clear();
    throw;
  }
  return *this;
}
//---------------------------------------------------------------------------
StorageDriver & StorageDriver::detach()
{
  if( files_.count() > 0 ){
  }
  return *this;
}
//---------------------------------------------------------------------------
} // namespace uss
//---------------------------------------------------------------------------
#ifndef NDEBUG
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class TestFiber : public ksys::BaseFiber {
  public:
    virtual ~TestFiber();
    TestFiber();
  protected:
  private:
    void execute();
};
//---------------------------------------------------------------------------
TestFiber::~TestFiber()
{
}
//---------------------------------------------------------------------------
TestFiber::TestFiber()
{
}
//---------------------------------------------------------------------------
void TestFiber::execute()
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Test : public ksys::BaseServer {
  public:
    virtual ~Test();
    Test();
  protected:
    ksys::BaseFiber * newFiber();
  private:
    void execute();
};
//---------------------------------------------------------------------------
Test::~Test()
{
}
//---------------------------------------------------------------------------
Test::Test()
{
}
//---------------------------------------------------------------------------
ksys::BaseFiber * Test::newFiber()
{
  return new TestFiber;
}
//---------------------------------------------------------------------------
void Test::execute()
{
}
//---------------------------------------------------------------------------
int main(int argc,char ** argv)
{
  int errcode = 0;
  adicpp::initialize();
  uss::initialize();
  try {
    ksys::initializeArguments(argc,argv);
    uss::FileDriver ussFile;
    ussFile.create("ussfile.uss");
    ussFile.print("ussfile.uss");
  }
  catch( ksys::ExceptionSP & e ){
    e->writeStdError();
    errcode = e->codes()[0];
  }
  catch( ... ){
    fprintf(stderr,"Unexpected exception catched\n");
  }
  uss::cleanup();
  adicpp::cleanup();
  return errcode;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
