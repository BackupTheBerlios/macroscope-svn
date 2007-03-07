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
    
    LZWFilterT<IO> & init();
    LZWFilterT<IO> & cleanup();
    
    LZWFilterT<IO> & compress(const void * buf,uintptr_t count,bool lastBlock);
    LZWFilterT<IO> & decompress(const void * buf,uintptr_t count,bool lastBlock);
    AutoPtr<uint8_t> & out();
    const uintptr_t & outSize();
  protected:
    NullIO nullIO_;
    IO * io_;

    class StringT {
      public:
        static EmbeddedHashNode<StringT> & keyNode(const StringT & object){
          return object.keyNode_;
        }
        static StringT & keyNodeObject(const EmbeddedHashNode<StringT> & node,StringT * p){
          return node.object(p->keyNode_);
        }
        static uintptr_t keyNodeHash(const StringT & object){
          return HF::hash(object.s_,object.l_);
        }
        static bool keyHashNodeEqu(const StringT & object1,const StringT & object2){
          return memncmp(object1.s_,object1.l_,object2.s_,object2.l_) == 0;
        }

        mutable EmbeddedHashNode<StringT> keyNode_;
        uint8_t * s_;
        uintptr_t l_;
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
    AutoPtr<StringT> table_;
    uintptr_t count_;
    AutoPtr<uint8_t> out_;
    uintptr_t outCount_;
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
  io_(io == NULL ? &nullIO_ : io), count_(0), outCount_(0), outSize_(0)
{
}
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilterT<IO> & LZWFilterT<IO>::init()
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
  if( count_ == 0 ){
    table_.resize(256);
    while( count_ < 256 ){
      StringT * pStr = &table_[count_];
      pStr->s_ = NULL;
      pStr->s_ = (StringT *) kmalloc(1);
      pStr->s_[0] = (uint8_t) count_;
      pStr->l_ = 1;
      tableHash_.insert(*pStr);
      count_++;
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilterT<IO> & LZWFilterT<IO>::cleanupTable()
{
  while( count_ > 0 ) kfree(table_[--count_].s_);
  table_.free();
  tableHash_.clear();
  return *this;
}
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilterT<IO> & LZWFilterT<IO>::cleanupOut()
{
  out_.free();
  outCount_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilterT<IO> & LZWFilterT<IO>::compress(const void * buf,uintptr_t count,bool lastBlock)
{
  uintptr_t ml;
  StringT * pStr, * pStr2;
  if( count_ == 256 ){
    table_.realloc(count_ + 1);
    table_[count_].s_ = (uint8_t *) kmalloc(ml = 64);
    table_[count_].l_ = 0;
    count_++;
  }
  else {
    ml = table_[count_ - 1].l_;
  }
  outSize_ = 0;
  while( count > 0 ){
    pStr2 = &table_[count_ - 1];
    if( pStr2->l_ == ml ){
      pStr2->s_ = (uint8_t *) krealloc(pStr2->s_,ml << 1);
      ml <<= 1;
    }
    pStr2->s_[pStr2->l_++] = *(const uint8_t *) buf;
    tableHash_.insert(*pStr2,false,false,&pStr);
    if( pStr == pStr2 || (count == 1 && lastBlock) ){
      pStr->s_ = (uint8_t *) krealloc(pStr->s_,pStr->l_);
      out_.realloc(outCount_ + sizeof(uint32_t));
      *(uint32_t *) (out_.ptr() + outCount_) = uint32_t(count_ - 1);
      outCount_ += sizeof(uint32_t);
      table_.realloc(count_ + 1);
      table_[count_].s_ = NULL;
      table_[count_].s_ = (uint8_t *) kmalloc(ml = 64);
      table_[count_].l_ = 1;
      table_[count_].s_[0] = pStr->s_[pStr->l_ - 1];
      count_++;
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
  while( count >= sizeof(uint32_t) ){
    if( *(const uint32_t *) buf < count_ ){ // Если строка найдена в таблице
      pStr = &table_[*(const uint32_t *) buf];
// Строка = строка для предыдущего кода + первый символ строки для текущего кода
      out_.realloc(outCount_ + prevStr->l_ + 1);
      memcpy(out_.ptr() + outCount_,prevStr->s_,prevStr->l_);
      outCount_ += prevStr->l_;
      out_[outCount_++] = pStr->s_[0];
    }
    else { // Если строка не найдена в таблице
// Строка = строка для предыдущего кода + первый символ строки для предыдущего кода
      out_.realloc(outCount_ + prevStr->l_ + 1);
      memcpy(out_.ptr() + outCount_,prevStr->s_,prevStr->l_);
      outCount_ += prevStr->l_;
      out_[outCount_++] = prevStr->s_[0];
    }
// Добавление строки в словарь на первую свободную позицию
    pStr2 = &table_.realloc(count_ + 1)[count];
    pStr2->s_ = NULL;
    pStr2->s_ = (uint8_t *) kmalloc(prevStr->l_ + 1);
    pStr2->l_ = prevStr->l_ + 1;
    memcpy(pStr2->s_,out_.ptr() + outCount_ - prevStr->l_ - 1,prevStr->l_ + 1);
    count_++;
    
    prevStr = pStr;
    buf = (const uint32_t *) buf + 1;
    count -= sizeof(uint32_t);
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
typedef LZWFilterT<NullIO> LZWFilter;
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _lzw_H_ */
//---------------------------------------------------------------------------
