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
#ifndef _SymbolTableH_
#define _SymbolTableH_
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
namespace kvm {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Symbol {
  friend class CodeObject;
  friend class SymbolTable;
  public:
    ~Symbol();
    Symbol(Symbol * parent = NULL,
           wchar_t * symbol = NULL,
           CodeObject * object = NULL);

    wchar_t * id() const;
    CodeObject * object() const { return object_; }

    static EmbeddedHashNode<Symbol,uintptr_t> & ehNLT(const uintptr_t & link,uintptr_t * &){
      return *reinterpret_cast<EmbeddedHashNode<Symbol,uintptr_t> *>(link);
    }
    static uintptr_t ehLTN(const EmbeddedHashNode<Symbol,uintptr_t> & node,uintptr_t * &){
      return reinterpret_cast<uintptr_t>(&node);
    }			    
    static EmbeddedHashNode<Symbol,uintptr_t> & keyNode(const Symbol & object){
      return object.keyNode_;
    }
    static Symbol & keyNodeObject(const EmbeddedHashNode<Symbol,uintptr_t> & node,Symbol * p){
      return node.object(p->keyNode_);
    }
    static uintptr_t keyNodeHash(const Symbol & object);
    static bool keyHashNodeEqu(const Symbol & object1,const Symbol & object2);
    
  protected:
    Symbol * parent_;
    wchar_t * symbol_;
    CodeObject * object_;

    mutable EmbeddedHashNode<Symbol,uintptr_t> keyNode_;
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class CodeGenerator;
//------------------------------------------------------------------------------
class SymbolTable : public Object {
  public:
    virtual ~SymbolTable();
    SymbolTable(CodeGenerator * codeGenerator = NULL);

    Symbol * newSymbol(CodeObjectOwner * parent,const wchar_t * symbol,CodeObject * object,bool pseudonym = false);
    Symbol * replaceSymbolObject(CodeObjectOwner * parent,const wchar_t * symbol,CodeObject * object);
    Symbol * findSymbol(CodeObjectOwner * parent,const wchar_t * symbol,bool noThrow = false);
  protected:
    CodeGenerator * codeGenerator_;
    typedef EmbeddedHash<
      Symbol,
      uintptr_t,
      uintptr_t *,
      Symbol::ehNLT,
      Symbol::ehLTN,
      Symbol::keyNode,
      Symbol::keyNodeObject,
      Symbol::keyNodeHash,
      Symbol::keyHashNodeEqu
    > Symbols;
    Symbols symbols_;
  private:
    SymbolTable(const SymbolTable &);
    void operator = (const SymbolTable &);
};
//------------------------------------------------------------------------------
} // namespace kvm
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
