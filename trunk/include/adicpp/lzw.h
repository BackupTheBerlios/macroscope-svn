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
#ifndef _lzw_H_
#define _lzw_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class NullIO {
  public:
    void readBuffer(void *,uintptr_t) {}
    void writeBuffer(const void *,uintptr_t) {}
  protected:
  private:
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename IO>
class LZWFilterT {
  public:
    ~LZWFilterT();
    LZWFilterT(IO * io = NULL);
    
    LZWFilterT<IO> & initialize();
    LZWFilterT<IO> & cleanup();
    
    LZWFilterT<IO> & compress(const void * buf,uintptr_t count,bool lastBlock);
    LZWFilterT<IO> & decompress(const void * buf,uintptr_t count,bool lastBlock);
    AutoPtr<uint8_t> & out();
    const uintptr_t & outSize();

    static void genStatisticTable(const Array<utf8::String> & fileNames);
  protected:
    NullIO nullIO_;
    IO * io_;

    class StringT {
      public:
        ~StringT() {}
        StringT() {}

        static EmbeddedHashNode<StringT> & keyNode(const StringT & object){
          return object.keyNode_;
        }
        static StringT & keyNodeObject(const EmbeddedHashNode<StringT> & node,StringT * p){
          return node.object(p->keyNode_);
        }
        static uintptr_t keyNodeHash(const StringT & object){
          return HF::hash(object.s_.ptr(),object.l_);
        }
        static bool keyHashNodeEqu(const StringT & object1,const StringT & object2){
          return memncmp(object1.s_,object1.l_,object2.s_,object2.l_) == 0;
        }

        mutable EmbeddedHashNode<StringT> keyNode_;
        AutoPtr<uint8_t> s_;
        uintptr_t l_;
        uintptr_t i_;
      protected:
      private:
    };
    typedef EmbeddedHash<
      StringT,
      StringT::keyNode,
      StringT::keyNodeObject,
      StringT::keyNodeHash,
      StringT::keyHashNodeEqu
    > TableHash;
    TableHash tableHash_;
    Vector<StringT> table_;
    AutoPtr<uint8_t> out_;
    uintptr_t outCount_;
    uintptr_t outMax_;
    uintptr_t outSize_;

    LZWFilterT<IO> & initTable();    
    LZWFilterT<IO> & cleanupTable();
    LZWFilterT<IO> & cleanupOut();
  private:
};
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilterT<IO>::~LZWFilterT()
{
  cleanupTable();
}
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilterT<IO>::LZWFilterT(IO * io) :
  io_(io == NULL ? &nullIO_ : io), outCount_(0), outMax_(0), outSize_(0)
{
}
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilterT<IO> & LZWFilterT<IO>::initialize()
{
  cleanup().initTable();
  return *this;
}
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilterT<IO> & LZWFilterT<IO>::cleanup()
{
  cleanupTable().cleanupOut();
  return *this;
}
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilterT<IO> & LZWFilterT<IO>::initTable()
{
  while( table_.count() < 256 ){
    StringT * pStr = &table_.add();
    pStr->s_.realloc(1);
    pStr->s_[0] = (uint8_t) (pStr->i_ = table_.count() - 1);
    pStr->l_ = 1;
    tableHash_.insert(*pStr);
  }
  return *this;
}
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilterT<IO> & LZWFilterT<IO>::cleanupTable()
{
  table_.clear();
  tableHash_.clear();
  return *this;
}
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilterT<IO> & LZWFilterT<IO>::cleanupOut()
{
  out_.free();
  outCount_ = outMax_ = outSize_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilterT<IO> & LZWFilterT<IO>::compress(const void * buf,uintptr_t count,bool lastBlock)
{
  uintptr_t ml;
  StringT * pStr, * pStr2;
  if( table_.count() == 256 ){
    pStr = &table_.add();
    pStr->s_.realloc(ml = 64);
    pStr->l_ = 0;
    pStr->i_ = table_.count() - 1;
  }
  else {
    ml = table_[table_.count() - 1].l_;
  }
  outSize_ = 0;
  if( outMax_ == 0 ){
    out_.realloc(getpagesize());
    outMax_ = getpagesize();
  }
  while( count > 0 ){
    pStr2 = &table_[table_.count() - 1];
    if( pStr2->l_ == ml ){
      pStr2->s_.realloc(ml << 1);
      ml <<= 1;
    }
    pStr2->s_[pStr2->l_++] = *(const uint8_t *) buf;
    tableHash_.insert(*pStr2,false,false,&pStr);
    if( pStr == pStr2 || (count == 1 && lastBlock) ){
      if( ml > pStr2->l_ ) pStr2->s_.realloc(pStr2->l_);
      uint8_t fStr[sizeof(StringT)];
      pStr = (StringT *) fStr;
      pStr->l_ = pStr2->l_ - 1;
      pStr->s_.ptr(pStr2->s_);
      uint32_t i = uint32_t(tableHash_.find(*pStr)->i_);
      if( i < (1u << 6) ){
        if( outCount_ + sizeof(uint8_t) >= outMax_ ){
          out_.realloc(outMax_ << 1);
          outMax_ = outMax_ << 1;
        }
        *(uint8_t *) (out_.ptr() + outCount_) = uint8_t(i << 2);
        outCount_ += sizeof(uint8_t);
      }
      else if( i < (1u << 14) ){
        if( outCount_ + sizeof(uint16_t) >= outMax_ ){
          out_.realloc(outMax_ << 1);
          outMax_ = outMax_ << 1;
        }
        be16enc(out_.ptr() + outCount_,uint16_t((i << 2) | 0x1));
        outCount_ += sizeof(uint16_t);
      }
      else if( i < (1u << 22) ){
        if( outCount_ + sizeof(uint16_t) + sizeof(uint8_t) >= outMax_ ){
          out_.realloc(outMax_ << 1);
          outMax_ = outMax_ << 1;
        }
        be32enc(out_.ptr() + outCount_,uint32_t((i << 2) | 0x2));
        outCount_ += sizeof(uint16_t) + sizeof(uint8_t);
      }
      else if( i < (1u << 30) ){
        if( outCount_ + sizeof(uint32_t) >= outMax_ ){
          out_.realloc(outMax_ << 1);
          outMax_ = outMax_ << 1;
        }
        be32enc(out_.ptr() + outCount_,uint32_t((i << 2) | 0x3));
        outCount_ += sizeof(uint32_t);
      }
      else {
        assert( 0 );
      }
      pStr2 = &table_.add();
      pStr2->s_.realloc(ml = 64);
      pStr2->l_ = 1;
      pStr2->i_ = table_.count() - 1;
      pStr2->s_[0] = *(const uint8_t *) buf;
    }
    buf = (const uint8_t *) buf + 1;
    count--;
  }
  io_->writeBuffer(out_,outSize_ = outCount_);
  outCount_ = 0;
  if( lastBlock ) cleanupTable();
  return *this;
}
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilterT<IO> & LZWFilterT<IO>::decompress(const void * buf,uintptr_t count,bool lastBlock)
{
  StringT str, * prevStr = &str, * pStr, * pStr2;
  str.l_ = 0;
  outSize_ = 0;
  if( outMax_ == 0 ){
    out_.realloc(getpagesize());
    outMax_ = getpagesize();
  }
  while( count >= 0 ){
    uint32_t i, j;
    switch( *(const uint8_t *) buf & 0x3 ){
      case 0 :
        i = *(const uint8_t *) buf >> 2;
        j = 1;
        break;
      case 1 :
        i = be16dec(buf) >> 2;
        j = 2;
        break;
      case 2 :
        i = 0;
        memcpy(&i,buf,3);
        i = be32dec(&i) >> 2;
        j = 3;
        break;
      case 3 :
        i = be32dec(buf) >> 2;
        j = 4;
        break;
      default :
        assert(0);
        abort();
    }
    if( i < table_.count() ){ // Если строка найдена в таблице
      pStr = &table_[i];
// Строка = строка для предыдущего кода + первый символ строки для текущего кода
      if( outCount_ + pStr->l_ >= outMax_ ){
        out_.realloc(outMax_ << 1);
        outMax_ = outMax_ << 1;
      }
      memcpy(out_.ptr() + outCount_,pStr->s_,pStr->l_);
      outCount_ += pStr->l_;

      pStr2 = &table_.add();
      pStr2->s_ = (uint8_t *) kmalloc(prevStr->l_ + 1);
      pStr2->l_ = prevStr->l_ + 1;
      pStr2->i_ = table_.count() - 1;
      memcpy(pStr2->s_,prevStr->s_,prevStr->l_);
      pStr2->s_[pStr2->l_ - 1] = pStr->s_[0];
    }
    else { // Если строка не найдена в таблице
// Строка = строка для предыдущего кода + первый символ строки для предыдущего кода
      if( outCount_ + prevStr->l_ + 1 >= outMax_ ){
        out_.realloc(outMax_ << 1);
        outMax_ = outMax_ << 1;
      }
      memcpy(out_.ptr() + outCount_,prevStr->s_,prevStr->l_);
      outCount_ += prevStr->l_;
      out_[outCount_++] = prevStr->s_[0];

      pStr2 = &table_.add();
      pStr2->s_ = (uint8_t *) kmalloc(prevStr->l_ + 1);
      pStr2->l_ = prevStr->l_ + 1;
      pStr2->i_ = table_.count() - 1;
      memcpy(pStr2->s_,out_.ptr() + outCount_ - prevStr->l_ - 1,prevStr->l_ + 1);
    }
// Добавление строки в словарь на первую свободную позицию
    
    prevStr = pStr;
    buf = (const uint8_t *) buf + j;
    count -= j;
  }
  io_->writeBuffer(out_,outSize_ = outCount_);
  outCount_ = 0;
  if( lastBlock ) cleanupTable();
  return *this;
}
//---------------------------------------------------------------------------
template <typename IO> inline
AutoPtr<uint8_t> & LZWFilterT<IO>::out()
{
  return out_;
}
//---------------------------------------------------------------------------
template <typename IO> inline
const uintptr_t & LZWFilterT<IO>::outSize()
{
  return outSize_;
}
//---------------------------------------------------------------------------
template <typename IO> inline
void LZWFilterT<IO>::genStatisticTable(const Array<utf8::String> & fileNames)
{
  class TEntry {
    public:
      uintmax_t freq_;
      uint8_t i_;

      bool operator == (const TEntry & e) const { return freq_ == e.freq_; }
      bool operator < (const TEntry & e) const { return freq_ > e.freq_; }
      bool operator > (const TEntry & e) const { return freq_ < e.freq_; }
  };
  Array<TEntry> table;
  table.resize(256);
  uintptr_t i, j;
  for( i = 0; i < 256; i++ ){
    table[i].freq_ = 0;
    table[i].i_ = (uint8_t) i;
  }
  AutoPtr<uint8_t> b;
  b.alloc(getpagesize() * 16);
  AsyncFile f;
  for( i = 0; i < fileNames.count(); i++ ){
    f.fileName(fileNames[i]).readOnly(true).open();
    for(;;){
      int64_t r = f.read(b,getpagesize() * 16);
      if( r <= 0 ) break;
      while( --r >= 0 ) table[(uintptr_t) b[(uintptr_t) r]].freq_++;
    }
    f.close();
  }
  f.fileName("stdout").open();
  qSort(table.ptr(),0,table.count() - 1);
  f <<
    "const uint8_t table[256] = {\n"
  ;
  for( i = 0; i < 256; i += 8 ){
    for( j = 0; j < 8; j++ )
      f <<
        utf8::String(j == 0 ? "  " : "") +
        "0x" + utf8::int2HexStr(table[i + j].i_,2) +
        (i + j < 255 ? ", " : "") + (j < 7 ? "" : "\n");
  }
  f <<
    "};\n"
  ;
}
//---------------------------------------------------------------------------
typedef LZWFilterT<NullIO> LZWFilter;
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _lzw_H_ */
//---------------------------------------------------------------------------
