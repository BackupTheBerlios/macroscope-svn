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
template <typename IO>
class LZWFilter {
  public:
    ~LZWFilter();
    LZWFilter(IO * io = NULL);
    
    LZWFilter<IO> & compress(const void * buf,uintptr_t count);
    LZWFilter<IO> & decompress(const void * buf);
    LZWFilter<IO> & flush();
  protected:
    IO * io_;

    struct StringT {
      static EmbeddedHashNode<StringT> & keyNode(const StringT & object){
        return object.keyNode_;
      }
      static StringT & keyNodeObject(const EmbeddedHashNode<StringT> & node,StringT * p){
        return node.object(p->keyNode_);
      }
      static uintptr_t keyNodeHash(const StringT & object){
        return hash(object.s_,object.l);
      }
      static bool keyHashNodeEqu(const StringT & object1,const StringT & object2){
        return memncmp(object1.s_,object1.l_,object2.s_,object2.l_) == 0;
      }
														
      static EmbeddedListNode<StringT> & listNode(const StringT & object){
        return object.listNode_;
      }
      static StringT & listNodeObject(const EmbeddedListNode<StringT> & node,StringT * p){
        return node.object(p->listNode_);
      }

      static EmbeddedHashNode<StringT> & idNode(const StringT & object){
        return object.idNode_;
      }
      static StringT & idNodeObject(const EmbeddedHashNode<StringT> & node,StringT * p){
        return node.object(p->idNode_);
      }
      static uintptr_t idNodeHash(const StringT & object){
        return hash(object.id_);
      }
      static bool idHashNodeEqu(const StringT & object1,const StringT & object2){
        return object1.id_ == object2.id_;
      }

      mutable EmbeddedHashNode<StringT> keyNode_;
      mutable EmbeddedListNode<StringT> listNode_;
      mutable EmbeddedHashNode<StringT> idNode_;
      mutable uintptr_t id_;
      mutable uint16_t l_;
      mutable uint8_t s_[1];
    };
    typedef EmbeddedHash<
      StringT,
      StringT::keyNode,
      StringT::keyNodeObject,
      StringT::keyNodeHash,
      StringT::keyHashNodeEqu
    > TableHash;
    TableHash tableHash_;
    AutoHashDrop<TableHash> tableHashAutoDrop_;
    typedef EmbeddedList<
      StringT,
      StringT::listNode,
      StringT::listNodeObject,
    > TableList;
    TableList tableList_; // LRU
    typedef EmbeddedHash<
      StringT,
      StringT::idNode,
      StringT::idNodeObject,
      StringT::idNodeHash,
      StringT::idHashNodeEqu
    > TableIdHash;
    TableIdHash tableIdHash_;
    
    LZWFilter<IO> & initTable();    
    
    AutoPtr<uint8_t> out_;
    uintptr_t outCount_;
  private:
};
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilter<IO>::~LZWFilter()
{
}
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilter<IO>::LZWFilter(IO * io) :
  io_(io), tableHashAutoDrop_(tableHash_), outCount_(0)
{
}
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilter<IO> & LZWFilter<IO>::initTable()
{
  if( tableHash_.count() == 0 ){
    for( intptr_t i = 0; i < 256; i++ ){
      StringT * s = (StringT *) kmalloc(sizeof(StringT));
      s->id_ = tableHash_.count();
      s->l_ = 1;
      s->s_[0] = (uint8_t) i;
      tableHash_.insert(*s);
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilter<IO> & LZWFilter<IO>::compress(const void * buf,uintptr_t count)
{
  initTable();
  StringT * pStr, * pStr2;
  AutoPtr<StringT> s;
  s.malloc(sizeof(StringT) - 1);
  s->l_ = 0;
  while( count > 0 ){
    s.realloc(s->l_ + 1);
    s->s_[s->l_++] = *(const uint8_t *) buf;
    tableHash_.insert(s,false,false,&pStr);
    if( s == pStr ){
      s.ptr(NULL);
      pStr->id_ = tableHash_.count() - 1;
      pStr->l_--;
      pStr2 = tableHash_.find(*pStr);
      pStr->l_++;
      out_.realloc(outCount_ + sizeof(uint32_t));
      *(uint32_t *) (out_.ptr() + outCount_) = uint32_t(pStr->id_);
      outCount_ += sizeof(uint32_t);
      s.malloc(sizeof(StringT) - 1);
      s->l_ = 0;
    }
    buf = (const uint8_t *) buf + 1;
    count--;
  }
  return *this;
}
//---------------------------------------------------------------------------
template <typename IO> inline
LZWFilter<IO> & LZWFilter<IO>::decompress(const void * buf)
{
  initTable();
  uintptr_t count = *(const uint32_t *) buf;
  buf = (const uint32_t *) buf + 1;
  while( count > 0 ){
    StringT str, * pStr;
    str.id_ = *(const uint32_t *) buf;
    pStr = tableIdHash_.find(str);
    if( pStr != NULL ){
      // Строка = строка для предыдущего кода + первый символ строки для текущего кода
      // Добавление строки в словарь на первую свободную позицию
    }
    else {
      // Строка = строка для предыдущего кода + первый символ строки для предыдущего кода
      // Добавление строки в словарь на первую свободную позицию
    }
    buf = (const uint32_t *) buf + 1;
    count--;
  }
  return *this;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _lzw_H_ */
//---------------------------------------------------------------------------
