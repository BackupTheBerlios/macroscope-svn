/*-
 * Copyright 2008 Guram Dukashvili
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
//------------------------------------------------------------------------------
#define _VERSION_C_AS_HEADER_
#include "version.c"
#undef _VERSION_C_AS_HEADER_
#define ENABLE_GD_INTERFACE 1
//#define ENABLE_PCAP_INTERFACE 1
//#define ENABLE_ODBC_INTERFACE 1
#define ENABLE_MYSQL_INTERFACE 1
#define ENABLE_FIREBIRD_INTERFACE 1
#include <adicpp/adicpp.h>
#include "Parser.h"
#include "Scanner.h"
#include "CodeGenerator.h"
#include "SymbolTable.h"
#include "Compiler.h"
#include "varnum.h"
//------------------------------------------------------------------------------
using namespace ksys;
using namespace ksys::kvm;
using namespace adicpp;
//------------------------------------------------------------------------------
class RangeCoderFilter32 {
  public:
    RangeCoderFilter32 & initializeEncoder();
    RangeCoderFilter32 & encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL);
    RangeCoderFilter32 & flush(void * out,uintptr_t * wb);
    RangeCoderFilter32 & encode(AsyncFile & inp,AsyncFile & out);

    RangeCoderFilter32 & initializeDecoder();
    RangeCoderFilter32 & decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL);
    RangeCoderFilter32 & decode(AsyncFile & inp,AsyncFile & out);
  private:
	  static const uint32_t top_ = uint32_t(1) << 24;
    static const uint32_t bottom_ = uint32_t(1) << 16;
	  static const uint32_t maxRange_ = bottom_;

    // encoder stateful members
    uint32_t eFreq_[257];
    uint32_t eLow_;
    uint32_t eRange_;
    uintptr_t ec_;

    // decoder stateful members
    uint32_t dFreq_[257];
    uint32_t code_;
    uint32_t dLow_;
    uint32_t dRange_;
    uintptr_t dc_;
    uintptr_t di_;

    enum { stInit, stInp, stOut } eState_, dState_;

    void rescale(uint32_t * freq)
    {
	    for( uintptr_t i = 1; i <= 256; i++ ){
		    freq[i] /= 2;
		    if( freq[i] <= freq[i - 1]) freq[i] = freq[i - 1] + 1;
	    }
    }

    void update(uintptr_t c,uint32_t * freq)
    {
      for( uintptr_t j = c + 1; j < 257; j++ ) freq[j]++;
		  if( freq[256] >= maxRange_ ) rescale(freq);
    }
};
//------------------------------------------------------------------------------
RangeCoderFilter32 & RangeCoderFilter32::initializeEncoder()
{
  eLow_ = 0;
  eRange_ = int32_t(-1);
	for( uintptr_t i = 0; i < sizeof(eFreq_) / sizeof(eFreq_[0]); i++ ) eFreq_[i] = uint32_t(i);
  eState_ = stInit;
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter32 & RangeCoderFilter32::encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb,uintptr_t * wb)
{
  if( eState_ == stOut ) goto out;
  for(;;){
    if( inpSize == 0 ){
      eState_ = stInp;
      return *this;
    }
    ec_ = *(const uint8_t *) inp;
    inp = (const uint8_t *) inp + sizeof(uint8_t);
    inpSize -= sizeof(uint8_t);
    if( rb != NULL ) *rb += sizeof(uint8_t);

    uint32_t symbolLow = eFreq_[ec_], symbolHigh = eFreq_[ec_ + 1], totalRange = eFreq_[256];
    eLow_ += symbolLow * (eRange_ /= totalRange);
    eRange_ *= symbolHigh - symbolLow;

    while( (eLow_ ^ (eLow_ + eRange_)) < top_ || eRange_ < bottom_ && ((eRange_ = -int32_t(eLow_) & (bottom_ - 1)),1) ){
out:  if( outSize == 0 ){
        eState_ = stOut;
        return *this;
      }	     
      *(uint8_t *) out = uint8_t(eLow_ >> 24);
      out = (uint8_t *) out + sizeof(uint8_t);
      outSize -= sizeof(uint8_t);
      if( wb != NULL ) *wb += sizeof(uint8_t);

      eRange_ <<= 8;
      eLow_ <<= 8;
    }
    update(ec_,eFreq_);
	}
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter32 & RangeCoderFilter32::flush(void * out,uintptr_t * wb)
{
	for( uintptr_t i = 0; i < 4; i++ ){
    *(uint8_t *) out = uint8_t(eLow_ >> 24);
    out = (uint8_t *) out + sizeof(uint8_t);
    if( wb != NULL ) *wb += sizeof(uint8_t);
		eLow_ <<= 8;
	}
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter32 & RangeCoderFilter32::encode(AsyncFile & inp,AsyncFile & out)
{
  uintptr_t rbs = getpagesize() * 16;
  uintptr_t wbs = getpagesize() * 16;
  AutoPtr<uint8_t> inpBuffer((uint8_t *) kmalloc(rbs));
  AutoPtr<uint8_t> outBuffer((uint8_t *) kmalloc(wbs));
  uintptr_t rb = 0, wb = 0;
  int64_t r;
  while( (r = inp.read(inpBuffer,rbs)) > 0 ){
    while( rb < uintptr_t(r) ){
      encodeBuffer(&inpBuffer[rb],uintptr_t(r) - rb,&outBuffer[wb],wbs - wb,&rb,&wb);
      if( wb == wbs ){
        out.writeBuffer(outBuffer,wbs);
        wb = 0;
      }
    }
    rb = 0;
  }
  out.writeBuffer(outBuffer,wb);
  wb = 0;
  flush(outBuffer,&wb);
  out.writeBuffer(outBuffer,wb);
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter32 & RangeCoderFilter32::initializeDecoder()
{
  eLow_ = 0;
  eRange_ = int32_t(-1);
	for( uintptr_t i = 0; i < sizeof(dFreq_) / sizeof(dFreq_[0]); i++ ) dFreq_[i] = uint32_t(i);
	code_ = 0;
  di_ = 0;
  dState_ = stInit;
  return *this;
}
//------------------------------------------------------------------------------
RangeCoderFilter32 & RangeCoderFilter32::decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb,uintptr_t * wb)
{
  if( eState_ == stInp ) goto inp;
  if( eState_ == stOut ) goto out;
  if( dState_ == stInit ){
	  while( di_ < 4 ){
      if( inpSize == 0 ) return *this;
		  code_ = (code_ << 8) | *(const uint8_t *) inp;
      inp = (const uint8_t *) inp + sizeof(uint8_t);
      inpSize -= sizeof(uint8_t);
      if( rb != NULL ) *rb += sizeof(uint8_t);
      di_++;
	  }
  }
  for(;;){
    uint32_t totalRange = dFreq_[256];
    uint32_t count = (code_ - dLow_) / (dRange_ /= totalRange);
  	for( dc_ = 255; dFreq_[dc_] > count; dc_-- );
out:
    if( outSize == 0 ){
      dState_ = stOut;
      return *this;
    }	     
    *(uint8_t *) out = uint8_t(dc_);
    out = (uint8_t *) out + sizeof(uint8_t);
    outSize -= sizeof(uint8_t);
    if( wb != NULL ) *wb += sizeof(uint8_t);

    uint32_t symbolLow = dFreq_[dc_], symbolHigh = dFreq_[dc_ + 1];
    dLow_ += symbolLow * dRange_;
    dRange_ *= symbolHigh - symbolLow;

    while( (dLow_ ^ dLow_ + dRange_) < top_ || dRange_ < bottom_ && ((dRange_ = -int32_t(dLow_) & bottom_ - 1),1) ){
inp:  if( inpSize == 0 ){
        dState_ = stInp;
        return *this;
      }
      code_ = code_ << 8 | *(const uint8_t *) inp;
      inp = (const uint8_t *) inp + sizeof(uint8_t);
      inpSize -= sizeof(uint8_t);
      if( rb != NULL ) *rb += sizeof(uint8_t);

      dRange_ <<= 8;
      dLow_ <<= 8;
    }

    update(dc_,dFreq_);
  }
  return *this;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class LZKFilter {
  public:
    LZKFilter & initializeEncoder();
    LZKFilter & encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL);
    LZKFilter & flush(void * out,uintptr_t * wb = NULL);
    LZKFilter & encode(AsyncFile & inp,AsyncFile & out);
    LZKFilter & initializeDecoder();
    LZKFilter & decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb = NULL,uintptr_t * wb = NULL);
    LZKFilter & decode(AsyncFile & inp,AsyncFile & out);
  protected:
    struct Node;
    struct TreeParams {
      LZKFilter * filter_;
      Node * dict_;
      uintptr_t dmsk_;
      uintptr_t mlen_;
      Node * bestMatchNode_;
      uintptr_t bestMatchLen_;
    };
#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#elif defined(__BCPLUSPLUS__)
#pragma option push -a1
#endif
    struct PACKED Node {
      mutable RBTreeNode treeNode_;
      mutable uint8_t c_;
      mutable uint8_t alignment_[(sizeof(RBTreeNode) < 16 ? 16 : 32) - sizeof(RBTreeNode) - 1];

      Node & operator = (const Node & )
      {
        assert(0);
        return *this;
      }

      static RBTreeNode & treeO2N(const Node & object,TreeParams *){
        return object.treeNode_;
      }

      static Node & treeN2O(const RBTreeNode & node,TreeParams *){
        Node * p = NULL;
        return node.object<Node>(p->treeNode_);
      }

      static intptr_t treeCO(const Node & a0,const Node & a1,TreeParams * params){
        intptr_t c, s1 = &a0 - params->dict_, s2 = &a1 - params->dict_;
        uintptr_t l = params->mlen_;
        do {
          c = intptr_t(params->dict_[s1].c_) - params->dict_[s2].c_;
          if( c != 0 ) break;
          s1 = (s1 + 1) & params->dmsk_;
          s2 = (s2 + 1) & params->dmsk_;
        } while( --l > 0 );
        l = params->mlen_ - l;
        if( l > params->bestMatchLen_ ){
          params->bestMatchNode_ = params->dict_ + s2 - l;
          params->bestMatchLen_ = l;
        }
        return c;
      }
    };
#ifdef _MSC_VER
#pragma pack(pop)
#elif defined(__BCPLUSPLUS__)
#pragma option pop
#endif

    typedef
      RBTree<
        Node,
        TreeParams,
        Node::treeO2N,
        Node::treeN2O,
        Node::treeCO
    > NodeTree;
    NodeTree tree_;
    TreeParams params_;

    // encoder stateful variables
    AutoPtr<Node> nodes_;
    Node * dict_;
    uintptr_t dpos_;
    uintptr_t dcnt_; // dict size
    uintptr_t dmsk_; // dict mask
    uintptr_t alen_; // look ahead string length
    uintptr_t mlen_; // maximum look ahead string length
    intptr_t dlen_;
    uintmax_t lstat_[8];

    // decoder stateful variables
    AutoPtr<uint8_t> ddict_;
    uintptr_t ddpos_;
    uintptr_t dcpos_;
    uintptr_t ddcnt_; // decode dict size
    uintptr_t ddmsk_; // decode dict mask
    uintptr_t ddlen_;

    enum { stInit, stInp, stOut } eState_, dState_;
  private:
};
//------------------------------------------------------------------------------
LZKFilter & LZKFilter::initializeEncoder()
{
  dpos_ = 0;
  alen_ = 0;
  mlen_ = 8;
  dlen_ = 0;
  eState_ = stInit;
  dcnt_ = 8192;
  dmsk_ = dcnt_ - 1;
  nodes_.reallocT(dcnt_);
  memset(dict_ = nodes_,0,dcnt_ * sizeof(Node));
  memset(lstat_,0,sizeof(lstat_));
  params_.filter_ = this;
  params_.dict_ = dict_;
  params_.dmsk_ = dmsk_;
  params_.mlen_ = mlen_;
  params_.bestMatchNode_ = NULL;
  params_.bestMatchLen_ = 0;
  tree_.param(&params_);
  return *this;
}
//------------------------------------------------------------------------------
LZKFilter & LZKFilter::encodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb,uintptr_t * wb)
{
  if( eState_ == stInit ){
    while( alen_ < mlen_ ){
      if( inpSize == 0 ){
        eState_ = stInp;
        return *this;
      }
      dict_[(dpos_ + alen_) & dmsk_].c_ = *(const uint8_t *) inp;
      alen_ += sizeof(uint8_t);
      inp = (const uint8_t *) inp + sizeof(uint8_t);
      inpSize -= sizeof(uint8_t);
      if( rb != NULL ) *rb += sizeof(uint8_t);
    }
    goto init;
  }
  if( eState_ == stInp ) goto inp;
  if( eState_ == stOut ) goto out;
  for(;;){
    do {
inp:  Node * p = dict_ + ((dpos_ + mlen_) & dmsk_);
      if( p->treeNode_.parent_ != NULL ) tree_.remove(*p);
      if( inpSize == 0 ){
        eState_ = stInp;
        return *this;
      }
      p->c_ = *(const uint8_t *) inp;
      inp = (const uint8_t *) inp + sizeof(uint8_t);
      inpSize -= sizeof(uint8_t);
      if( rb != NULL ) *rb += sizeof(uint8_t);
      dpos_ = (dpos_ + sizeof(uint8_t)) & dmsk_;
init: params_.bestMatchNode_ = dict_ + dict_[dpos_].c_;
      params_.bestMatchLen_ = 1;
      tree_.insert(dict_[dpos_],false,false);
    } while( --dlen_ >= 0 );
    dlen_ = params_.bestMatchLen_ - 1;
out:
    if( outSize < sizeof(uint16_t) ){
      eState_ = stOut;
      return *this;
    }
    lstat_[dlen_]++;
    *(uint16_t *) out = uint16_t(dlen_ | ((params_.bestMatchNode_ - dict_) << 3));
    out = (uint8_t *) out + sizeof(uint16_t);
    outSize -= sizeof(uint16_t);
    if( wb != NULL ) *wb += sizeof(uint16_t);
  }
  return *this;
}
//------------------------------------------------------------------------------
LZKFilter & LZKFilter::flush(void * out,uintptr_t * wb)
{
  if( eState_ == stInit ) return *this;
  if( eState_ == stOut ) goto out;
  for(;;){
    do {
      dict_[(dpos_ + mlen_) & dmsk_].c_ = 0;
      dpos_ = (dpos_ + 1) & dmsk_;
      if( --alen_ == 0 ){
        eState_ = stInit;
        return *this;
      }
    } while( --dlen_ >= 0 );
    params_.bestMatchNode_ = dict_ + dict_[dpos_].c_;
    params_.bestMatchLen_ = 1;
    tree_.find(dict_[dpos_]);
    dlen_ = tmin(alen_,params_.bestMatchLen_) - 1;
out:
    lstat_[dlen_]++;
    *(uint16_t *) out = uint16_t(dlen_ | ((params_.bestMatchNode_ - dict_) << 3));
    out = (uint8_t *) out + sizeof(uint16_t);
    if( wb != NULL ) *wb += sizeof(uint16_t);
  }
  return *this;
}
//------------------------------------------------------------------------------
LZKFilter & LZKFilter::encode(AsyncFile & inp,AsyncFile & out)
{
  uintptr_t rbs = getpagesize() * 16;
  uintptr_t wbs = getpagesize() * 16;
  AutoPtr<uint8_t> inpBuffer((uint8_t *) kmalloc(rbs));
  AutoPtr<uint8_t> outBuffer((uint8_t *) kmalloc(wbs));
  uintptr_t rb = 0, wb = 0;
  int64_t r;
  while( (r = inp.read(inpBuffer,rbs)) > 0 ){
    while( rb < uintptr_t(r) ){
      encodeBuffer(&inpBuffer[rb],uintptr_t(r) - rb,&outBuffer[wb],wbs - wb,&rb,&wb);
      if( wb == wbs ){
        out.writeBuffer(outBuffer,wbs);
        wb = 0;
      }
    }
    rb = 0;
  }
  out.writeBuffer(outBuffer,wb);
  wb = 0;
  flush(outBuffer,&wb);
  out.writeBuffer(outBuffer,wb);
  return *this;
}
//------------------------------------------------------------------------------
LZKFilter & LZKFilter::initializeDecoder()
{
  ddpos_ = 0;
  ddcnt_ = 8192;
  ddmsk_ = dcnt_ - 1;
  dState_ = stInit;
  ddict_.reallocT(ddcnt_);
  memset(ddict_,0,ddcnt_);
  return *this;
}
//------------------------------------------------------------------------------
LZKFilter & LZKFilter::decodeBuffer(const void * inp,uintptr_t inpSize,void * out,uintptr_t outSize,uintptr_t * rb,uintptr_t * wb)
{
  uint8_t * dict = ddict_;
  if( dState_ == stOut ) goto out;
  for(;;){
    if( inpSize == 0 ){
      dState_ = stInp;
      return *this;
    }
    uint16_t code = *(const uint16_t *) inp;
    inp = (const uint8_t *) inp + sizeof(uint16_t);
    inpSize -= sizeof(uint16_t);
    if( rb != NULL ) *rb += sizeof(uint16_t);
    ddlen_ = (code & 7) + 1;
    if( ddlen_ == 1 ){
      dict[dcpos_ = ddpos_] = uint8_t(code >> 3);
      goto out;
    }
    dcpos_ = code >> 3;
    do {
out:  if( outSize == 0 ){
        dState_ = stOut;
        return *this;
      }
      *(uint8_t *) out = dict[ddpos_] = dict[dcpos_];
      out = (uint8_t *) out + sizeof(uint8_t);
      outSize -= sizeof(uint8_t);
      if( wb != NULL ) *wb += sizeof(uint8_t);
      ddpos_ = (ddpos_ + 1) & ddmsk_;
      dcpos_ = (dcpos_ + 1) & ddmsk_;
    } while( --ddlen_ > 0 );
  }
  return *this;
}
//------------------------------------------------------------------------------
LZKFilter & LZKFilter::decode(AsyncFile & inp,AsyncFile & out)
{
  uintptr_t rbs = getpagesize() * 16;
  uintptr_t wbs = getpagesize() * 16;
  AutoPtr<uint8_t> inpBuffer((uint8_t *) kmalloc(rbs));
  AutoPtr<uint8_t> outBuffer((uint8_t *) kmalloc(wbs));
  uintptr_t rb = 0, wb = 0;
  int64_t r;
  while( (r = inp.read(inpBuffer,rbs)) > 0 ){
    while( rb < uintptr_t(r) ){
      decodeBuffer(&inpBuffer[rb],uintptr_t(r) - rb,&outBuffer[wb],wbs - wb,&rb,&wb);
      if( wb == wbs ){
        out.writeBuffer(outBuffer,wbs);
        wb = 0;
      }
    }
    rb = 0;
  }
  out.writeBuffer(outBuffer,wb);
  return *this;
}
//------------------------------------------------------------------------------
int main(int _argc,char * _argv[])
{
  //{
    //intmax_t aa = 0x10000000i64, bb = 0x10000001i64, cc;
    //cc = aa * bb;
  //  intmax_t aa = 0, bb = -1, cc;
  //  cc = aa - bb;

  //  VarInteger a, b, c;

  //  for( uintptr_t i = 33; i < 300; i++ ){
  //    a = 0xFFFFFFFFFFFFFF00ui64;
  //    a <<= i;
  //  }

  //  a = 6ui64;
  //  b = 10;
  //  c = a - b;


  //  a = 0xFFFFFFFFFFFFFFFFi64;
  //  b = 0xFFFFFFFFFFFFFFFFi64;
  //  c = a + b;
  //  c = c + -2;
  //  b = -b;
  //  a = 3;
  //  b = 5;
  //  c = a * b;
  //  AutoPtr<char> s((char *) kmalloc(c.print() + 1));
  //  c.print(s);
  //  a = 0x100000000ui64;
  //  b = 3;
  //  c = a * b;
  //  AutoPtr<char> s2((char *) kmalloc(c.print(NULL,16) + 1));
  //  c.print(s2,16);
  //  c = c;
  //  ldouble r = sqrt(ldouble(2));
  //  VarNumber sqrt(2);
  //  sqrt = sqrt.sqrt();
  //  AutoPtr<char> s3((char *) kmalloc(sqrt.print() + 1));
  //  sqrt.print(s3);
  //  sqrt = sqrt;
  //}

  ////Sleep(15000);
  //ldouble freqs[256] = {
  //  1,
  //  2,
  //  3,
  //  4,
  //  5
  //};
  //ldouble weights[256] = {
  //  1,
  //  1,
  //  1,
  //  1,
  //  1
  //};
  //ldouble weight = 5;
  //ldouble range, freq, low = 0, high = 1;
  //range = high - low;
  //high  = low + range / freqs[3] * weights[3] / weight;
  //low   = low + range * cumFreq[1];


  int errcode = EINVAL;
  adicpp::AutoInitializer autoInitializer(_argc,_argv);
  autoInitializer = autoInitializer;

  try {
// tests
    AsyncFile file("C:/Korvin/trunk/test/kvm/test.txt");
    AsyncFile::LineGetBuffer lgb(file);
    file.open();

    AsyncFile encFile("C:/Korvin/trunk/test/kvm/test.enc");
    encFile.createIfNotExist(true).open().resize(0);

    AsyncFile decFile("C:/Korvin/trunk/test/kvm/test.dec");
    decFile.createIfNotExist(true).open().resize(0);

    uint64_t ellapsed;

    RangeCoderFilter32 filter2;
    filter2.initializeEncoder();
    ellapsed = gettimeofday();
    filter2.encode(file,encFile);
    ellapsed = gettimeofday() - ellapsed;
    fprintf(stderr,"encode: ellapsed %s, %lf kbps\n",
      (const char *) utf8::elapsedTime2Str(ellapsed).getOEMString(),
      (file.size() * 1000000. / ellapsed) / 1024
    );

    LZKFilter filter;
    filter.initializeEncoder();
    ellapsed = gettimeofday();
    filter.encode(file.seek(0),encFile);
    ellapsed = gettimeofday() - ellapsed;
    fprintf(stderr,"encode: ellapsed %s, %lf kbps\n",
      (const char *) utf8::elapsedTime2Str(ellapsed).getOEMString(),
      (file.size() * 1000000. / ellapsed) / 1024
    );
    encFile.seek(0);
    filter.initializeDecoder();
    ellapsed = gettimeofday();
    filter.decode(encFile,decFile);
    ellapsed = gettimeofday() - ellapsed;
    fprintf(stderr,"decode: ellapsed %s, %lf kbps\n",
      (const char *) utf8::elapsedTime2Str(ellapsed).getOEMString(),
      (decFile.size() * 1000000. / (ellapsed + (ellapsed == 0))) / 1024
    );
    return 0;
//

    stdErr.fileName(includeTrailingPathDelimiter(SYSLOG_DIR(kvm_version.tag_)) + kvm_version.tag_ + ".log");
    Config::defaultFileName(SYSCONF_DIR("") + kvm_version.tag_ + ".conf");
    ConfigSP config(newObject<InterlockedConfig<FiberInterlockedMutex> >());
    Array<utf8::String> sources;
    for( uintptr_t i = 1; i < argv().count(); i++ ){
      if( argv()[i].strcmp("--version") == 0 ){
        stdErr.debug(9,utf8::String::Stream() << kvm_version.tex_ << "\n");
        fprintf(stdout,"%s\n",kvm_version.tex_);
        break;
      }
      else if( argv()[i].strcmp("--chdir") == 0 && i + 1 < argv().count() ){
        changeCurrentDir(argv()[++i]);
      }
      else if( argv()[i].strcmp("--log") == 0 && i + 1 < argv().count() ){
        stdErr.fileName(argv()[++i]);
      }
      else if( argv()[i].strcmp("-c") == 0 && i + 1 < argv().count() ){
        Config::defaultFileName(argv()[i + 1]);
        config->fileName(argv()[++i]);
      }
      else {
        sources.add(argv()[i]);
      }
    }
    config->silent(true).parse().override();
    stdErr.rotationThreshold(
      config->value("debug_file_rotate_threshold",1024 * 1024)
    );
    stdErr.rotatedFileCount(
      config->value("debug_file_rotate_count",10)
    );
    stdErr.setDebugLevels(
      config->value("debug_levels","+0,+1,+2,+3")
    );
    stdErr.fileName(
      config->value("log_file",stdErr.fileName())
    );
    stdErr.debug(0,
      utf8::String::Stream() << kvm_version.gnu_ << " started\n"
    );
    config->silent(false);
    utf8::String defaultConfigSectionName(config->text("default_config",kvm_version.gnu_));
    utf8::String defaultConnectionSectionName(config->textByPath(defaultConfigSectionName + ".connection","default_connection"));
    AutoPtr<Database> database(Database::newDatabase(&config->section(defaultConnectionSectionName)));
    
    for( uintptr_t i = 0; i < sources.count(); i++ ){
      AutoPtr<wchar_t> fileName(coco_string_create(sources[i].getUNICODEString()));
      AutoPtr<Scanner> scanner(newObjectV1<Scanner>(fileName.ptr()));
      AutoPtr<Parser> parser(newObjectV1<Parser>(scanner.ptr()));
      parser->gen = newObject<CodeGenerator>();
      parser->tab = newObjectV1<SymbolTable>(parser->gen.ptr());
	    parser->Parse();
	    if( parser->errors->count > 0 ){
        exit(EINVAL);
	    }
      Compiler compiler;
      compiler.detect(config);
      compiler.test(includeTrailingPathDelimiter(getPathFromPathName(sources[i])) + "config.h");
      parser->gen->generate(compiler,changeFileExt(sources[i],".cxx"));
    }
    stdErr.debug(0,
      utf8::String::Stream() << kvm_version.gnu_ << " stopped\n"
    );
    errcode = 0;
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    errcode = e->code() >= errorOffset ? e->code() - errorOffset : e->code();
  }
  catch( ... ){
  }
  return errcode;
}
//------------------------------------------------------------------------------
