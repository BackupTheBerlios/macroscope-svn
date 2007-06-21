/*-
 * Copyright 2007 Guram Dukashvili
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
#ifndef _lzssk_H_
#define _lzssk_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class LZSSKFilter {
  public:
    ~LZSSKFilter();
    LZSSKFilter();
    
    LZSSKFilter & initializeCompression(uintptr_t dictSize);
    LZSSKFilter & compress(const void * buf,uintptr_t count);
    LZSSKFilter & finishCompression();
    LZSSKFilter & initializeDecompression();
    LZSSKFilter & decompress(const void * buf,uintptr_t count);

    AutoPtr<uint8_t> & output(){ return output_; }
    const uintptr_t & outputSize(){ return outputSize_; }
    const bool & eos(){ return eos_; }

    static void genStatisticTable(const Array<utf8::String> & fileNames);
  protected:
    class DictChar {
      public:
        ~DictChar() {}
        DictChar() {}

        static RBTreeNode & treeO2N(const DictChar & object,LZSSKFilter *){
          return object.treeNode_;
        }
	      static DictChar & treeN2O(const RBTreeNode & node,LZSSKFilter *){
	        DictChar * p = NULL;
	        return node.object<DictChar>(p->treeNode_);
	      }
	      static intptr_t treeCO(const DictChar & a0,const DictChar & a1,LZSSKFilter * filter){
          intptr_t i, c, s1 = &a0 - filter->dict_, s2 = &a1 - filter->dict_;
          filter->lastStrIndex_ = s1;
          for( i = 0; uintptr_t(i) < filter->aheadWindowSize_; i++ ){
            c = intptr_t(filter->dict_[s1].c_) - intptr_t(filter->dict_[s2].c_);
            if( c != 0 ) break;
            s1 = (s1 + 1) & filter->dictSizeMask_;
            s2 = (s2 + 1) & filter->dictSizeMask_;
          }
          filter->lastStrLen_ = i;
	        return c;
	      }
	      mutable RBTreeNode treeNode_;
        uintptr_t align_[3];
        uint8_t c_;
      protected:
      private:
    };
    friend class DictChar;
    typedef
      RBTree<
        DictChar,
        LZSSKFilter,
        DictChar::treeO2N,
        DictChar::treeN2O,
        DictChar::treeCO
    > DictTree;
    DictTree dictTree_;
    AutoPtr<uint8_t> buffer_;
    AutoPtr<uint8_t> output_;
    uintptr_t outputSize_;
    DictChar * dict_;
    uintptr_t dictSize_;
    uintptr_t dictSizeMask_;
    uintptr_t maxStrLen_;
    uintptr_t lastStrIndex_;
    uintptr_t lastStrLen_;
    uintptr_t dictPos_;
    uintptr_t aheadWindowSize_;
    uintptr_t abcFreq_[256];
    uint8_t abc2Idx_[256];
    uint8_t idx2abc_[256];
    bool finalize_;
    bool eos_;

    enum CodeType { ct6Char = 0, ct2Seq = 1, ct3Seq = 2, ct8Char = 3 };

    LZSSKFilter & updateABC(uintptr_t c);
  private:
};
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _lzssk_H_ */
//---------------------------------------------------------------------------
