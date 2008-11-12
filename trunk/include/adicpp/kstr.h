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
#ifndef _kstrH_
#define _kstrH_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
void initialize(int,char **);
void cleanup();
//---------------------------------------------------------------------------
class String {
  public:
    ~String();
    String();
    String(const String & string);

    String & operator = (const String & string);
  protected:
  private:
    class SubString {
      friend class String;
      public:
      protected:
      private:
        static RBTreeNode & treeO2N(const SubString & object,uintptr_t *){
          return object.node_;
	      }
	      static PacketGroup & treeN2O(const SubString & node,uintptr_t *){
	        SubString * p = NULL;
	        return node.object<SubString>(p->node_);
	      }
	      static intptr_t treeCO(const SubString & a0,const SubString & a1,uintptr_t *){
          return ::strcmp(a0.string_,a1.string_);
	      }

        volatile ilock_t refCount_;
        RBTreeNode node_;
        uint8_t string_[1];
    };
    SubString * string_;
    typedef RBTree<
      SubString,
      uintptr_t,
      SubString::treeO2N,
      SubString::treeN2O,
      SubString::treeCO
    > StringTree;
    static uint8_t treeHolder_[];
    StringTree & tree() const { return *reinterpret_cast<StringTree *>(treeHolder_); }
    static uint8_t treeMutexHolder_[];
    InterlockedMutex & treeMutex() const { return *reinterpret_cast<InterlockedMutex *>(treeMutexHolder_); }
    static SubString nullString_;
    static void addRef(SubString * string);
    static void release(SubString * string);
};
//---------------------------------------------------------------------------
inline String::~String()
{
  release(string_);
}
//---------------------------------------------------------------------------
inline String::String()
{
  addRef(string_ = &nullString_);
}
//---------------------------------------------------------------------------
inline String::String(const String & string)
{
  addRef(string.string_);
}
//---------------------------------------------------------------------------
inline String & String::operator = (const String & string)
{
  addRef(string_ = string.string_);
}
//---------------------------------------------------------------------------
inline void String::addRef(SubString * string)
{
  interlockedIncrement(string->refCount_,1);
}
//---------------------------------------------------------------------------
inline void String::release(SubString * string)
{
  if( interlockedIncrement(string->refCount_,-1) == 0 ) tree().drop(*string);
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
