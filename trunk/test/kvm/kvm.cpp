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
class Encoder2 {
  public:
    struct Node;
    struct TreeParams {
      uintptr_t pos_;
      Node * bestMatchNode_;
      Node * worstMatchNode_;
    };
#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#elif defined(__BCPLUSPLUS__)
#pragma option push -a1
#endif
    struct PACKED Node {
      mutable RBTreeNode treeNode_;
      mutable uint8_t len_;
      mutable uint16_t idx_;
      mutable uint8_t data_[1];

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
        return a0.memncmp2(a1,params);
      }

      intptr_t memncmp2(const Node & n,TreeParams * params) const
      {
        intptr_t c, cc = 0;
        const uint8_t * s1 = data_ + params->pos_, * r1 = s1 + len_, * s2 = n.data_, * r2 = s2 + n.len_;
        while( s1 < r1 && s2 < r2 ){
          c = intptr_t(*s1) - *s2;
          if( c != 0 ) goto e1;
          cc++;
          s1++;
          s2++;
        }
        while( s1 < r1 ){
          c = intptr_t(*s1) - 0;
          if( c != 0 ) goto e1;
          cc++;
          s1++;
        }
        while( s2 < r2 ){
          c = intptr_t(0) - *s2;
          if( c != 0 ) break;
          cc++;
          s2++;
        }
        e1:
        if( cc == n.len_ ){
          if( params->bestMatchNode_ == NULL || len_ > params->bestMatchNode_->len_ ){
            params->bestMatchNode_ = const_cast<Node *>(&n);
          }
          if( params->worstMatchNode_ == NULL || len_ < params->worstMatchNode_->len_ ){
            params->worstMatchNode_ = const_cast<Node *>(&n);
          }
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
    Vector<Node> lru_;
    TreeParams params_;

    Node * newNode(uintptr_t len) const
    {
      Node * node = (Node *) kmalloc(sizeof(Node) + len - sizeof(node->data_));
      node->len_ = uint8_t(len);
      return node;
    }

    Node * newNode(uintptr_t len,const uint8_t * s) const
    {
      Node * node = newNode(len);
      memcpy(node->data_,s,len);
      return node;
    }

    Node * newNode(const Node & n,uintptr_t pos = 0,uintptr_t len = 0) const
    {
      uint8_t l = uint8_t(len == 0 ? n.len_ : len);
      Node * node = (Node *) kmalloc(sizeof(Node) + l - sizeof(node->data_));
      node->len_ = l;
      memcpy(node->data_,n.data_ + pos,node->len_);
      return node;
    }

    AutoPtr<Node> ahead_;
    uintptr_t apos_;
    uintptr_t alen_; // look ahead string length
    uintptr_t mlen_; // maximum look ahead string length
    uintptr_t mnct_; // maximum nodes count

    Encoder & initialize()
    {
      apos_ = 0;
      alen_ = 0;
      mlen_ = 8;
      mnct_ = 65536 - 1;
      ahead_ = newNode(mlen_ * 2);
      params_.pos_ = 0;
      params_.bestMatchNode_ = params_.worstMatchNode_ = NULL;
      tree_.param(&params_);
      lru_.clear();
      for( uintptr_t i = 0; i < 256; i++ ){
        uint8_t s[1];
        s[0] = uint8_t(i);
        Node * node = newNode(1,s);
        lru_.safeAdd(node);
        tree_.insert(*node,true,false);
        node->idx_ = uint16_t(lru_.count() - 1);
      }
      return *this;
    }

    Encoder & encode(AsyncFile::LineGetBuffer & lgb,AsyncFile & out)
    {
      Node & ahead = ahead_;
      intptr_t c = 0;
      for(;;){
        while( alen_ < mlen_ ){
          c = lgb.getc();
          if( lgb.eof(c) ) break;
          ahead.data_[apos_ + alen_] = uint8_t(c);
          alen_++;
        }
        if( alen_ == 0 ) break;
        ahead.len_ = uint8_t(alen_);
        params_.pos_ = apos_;
        params_.bestMatchNode_ = params_.worstMatchNode_ = NULL;
        Node * node = tree_.find(ahead);
        assert( params_.bestMatchNode_ != NULL );
        assert( params_.worstMatchNode_ != NULL );
        if( node == NULL ) node = params_.bestMatchNode_;
        out.writeBuffer(&node->idx_,sizeof(node->idx_));
        if( node->idx_ > 256 ){
          lru_.xchg(node->idx_,node->idx_ - 1);
          lru_[node->idx_].idx_ = node->idx_;
          node->idx_--;
        }
        params_.pos_ = 0;
        for( uintptr_t i = node->len_ + 1; i <= mlen_; i++ ){
          Node * p = newNode(ahead,apos_,i), * p2, * p3;
          params_.bestMatchNode_ = params_.worstMatchNode_ = NULL;
          tree_.insert(*p,false,true,&p2);
          p3 = tree_.find(*p);
          if( p == p2 ){
            p->idx_ = uint16_t(lru_.count());
            lru_.safeAdd(p);
            uint16_t idx = uint16_t(mnct_);
            out.writeBuffer(&idx,sizeof(idx));
            out.writeBuffer(&p->len_,sizeof(p->len_));
            out.writeBuffer(p->data_,p->len_);
          }
        }
        apos_ += node->len_;
        alen_ -= node->len_;
        if( apos_ >= mlen_ ){
          memcpy(ahead.data_,ahead.data_ + apos_,alen_);
          apos_ = 0;
        }
      }
      return *this;
    }
};
//------------------------------------------------------------------------------
class Encoder {
  public:
    struct Node;
    struct TreeParams {
      Encoder * encoder_;
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
        intptr_t c, cc = 0, l = params->mlen_;
        intptr_t s1 = &a0 - params->dict_, s2 = &a1 - params->dict_;
        do {
          c = intptr_t(params->dict_[s1].c_) - params->dict_[s2].c_;
          if( c != 0 ) break;
          s1 = (s1 + 1) & params->dmsk_;
          s2 = (s2 + 1) & params->dmsk_;
        } while( --l > 0 );
        l = params->mlen_ - l;
        if( l > params->bestMatchLen_ ){
          params->bestMatchNode_ = params->dict_ + s2;
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

    AutoPtr<Node> nodes_;
    Node * dict_;
    uintptr_t dpos_;
    uintptr_t dcnt_; // dict size
    uintptr_t dmsk_; // dict mask
    uintptr_t alen_; // look ahead string length
    uintptr_t mlen_; // maximum look ahead string length

    Encoder & initialize()
    {
      dpos_ = 0;
      alen_ = 0;
      mlen_ = 8;
      dcnt_ = 8192;
      dmsk_ = dcnt_ - 1;
      nodes_.reallocT(dcnt_);
      memset(dict_ = nodes_,0,dcnt_);
      params_.encoder_ = this;
      params_.dict_ = dict_;
      params_.dmsk_ = dmsk_;
      params_.mlen_ = mlen_;
      params_.bestMatchNode_ = NULL;
      params_.bestMatchLen_ = 0;
      tree_.param(&params_);
      return *this;
    }

    Encoder & encode(AsyncFile::LineGetBuffer & lgb,AsyncFile & out)
    {
      intptr_t c = 0;
      for(;;){
        while( alen_ < mlen_ ){
          c = lgb.getc();
          if( lgb.eof(c) ) break;
          dict_[dpos_].c_ = uint8_t(c);
          dpos_ = (dpos_ + 1) & dmsk_;
          alen_++;
        }
        if( alen_ == 0 ) break;
        params_.bestMatchNode_ = NULL;
        params_.bestMatchLen_ = 0;
        Node * p, * node = tree_.insert(dict_ + apos_,false,false,&p);
        assert( params_.bestMatchNode_ != NULL );

      }
      return *this;
    }
};
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

    AsyncFile out("C:/Korvin/trunk/test/kvm/test.out");
    out.createIfNotExist(true).open().resize(0);

    Encoder enc;
    enc.initialize();
    enc.encode(lgb,out);
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
