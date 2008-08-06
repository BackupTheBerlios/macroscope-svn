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
#include <adicpp/adicpp.h>
#include "Scanner.h"
#include "CodeGenerator.h"
#include "SymbolTable.h"
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
namespace kvm {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
SymbolTable::~SymbolTable()
{
}
//------------------------------------------------------------------------------
SymbolTable::SymbolTable(CodeGenerator * codeGenerator) : codeGenerator_(codeGenerator)
{
  if( codeGenerator_ != NULL ) newSymbol(NULL,L"",&codeGenerator_->root_,true);
}
//------------------------------------------------------------------------------
Symbol * SymbolTable::newSymbol(CodeObject * parent,const wchar_t * symbol,CodeObject * object,bool pseudonym)
{
  AutoPtr<CodeObject> safe(pseudonym ? NULL : object);
  if( parent == NULL ) parent = &codeGenerator_->root_;
  Symbol * parentSymbol = parent->symbols_.count() > 0 ? &parent->symbols_[0] : NULL;
  ksys::AutoPtr<Symbol> sym(newObjectV1V2V3<Symbol>(parentSymbol,coco_string_create(symbol),object));
  if( !pseudonym ) parent->childs_.add(object);
  safe.ptr(NULL);
  symbols_.insert(sym,true,false);
  object->symbols_.add(sym.ptr());
  return sym.ptr(NULL);
}
//------------------------------------------------------------------------------
Symbol * SymbolTable::replaceSymbolObject(CodeObject * parent,const wchar_t * symbol,CodeObject * object)
{
  AutoPtr<CodeObject> safe(object);
  Symbol * sym = findSymbol(parent,symbol);
  AutoPtr<CodeObject> oldObject(sym->object_);
  object->symbols_.xchg(oldObject->symbols_);
  sym->object_ = object;
  safe.ptr(NULL);
  return sym;
}
//------------------------------------------------------------------------------
Symbol * SymbolTable::findSymbol(CodeObject * parent,const wchar_t * symbol,bool noThrow)
{
  if( parent == NULL ) parent = &codeGenerator_->root_;
  Symbol sym(&parent->symbols_[0],coco_string_create(symbol)), * p = symbols_.find(sym);
  if( p == NULL && !noThrow )
    newObjectV1C2<Exception>(ENOENT,utf8::String(symbol) + " " + __PRETTY_FUNCTION__)->throwSP();
  return p;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Symbol::~Symbol()
{
  coco_string_delete(symbol_);
}
//------------------------------------------------------------------------------
Symbol::Symbol(Symbol * parent,
       wchar_t * symbol,
       CodeObject * object) :
  parent_(parent),
  symbol_(symbol),
  object_(object)
{
}
//------------------------------------------------------------------------------
wchar_t * Symbol::id() const
{
  wchar_t * symbol = NULL, * s;
  for( Symbol * p = parent_; p != NULL; p = p->parent_ ){
    s = coco_string_create_append(p->symbol_,L".");
    coco_string_merge(s,symbol);
    coco_string_delete(symbol);
    symbol = s;
  }
  if( symbol == NULL || symbol[0] == '\0' ){
    s = coco_string_create(symbol_);
  }
  else {
    s = coco_string_create_append(symbol,L".");
    coco_string_merge(s,symbol_);
    coco_string_delete(symbol);
  }
  return s;
}
//------------------------------------------------------------------------------
uintptr_t Symbol::keyNodeHash(const Symbol & object)
{
  uintptr_t h = 0;
  for( Symbol * p = object.parent_; p != NULL; p = p->parent_ )
    h = HF::hash(p->symbol_,h);
  return HF::hash(object.symbol_,h);
}
//------------------------------------------------------------------------------
bool Symbol::keyHashNodeEqu(const Symbol & object1,const Symbol & object2)
{
  Symbol * p1 = object1.parent_;
  Symbol * p2 = object2.parent_;
  for(;;){
    if( p1 == NULL && p2 == NULL ) break;
    if( p1 == NULL || p2 == NULL || !coco_string_equal(p1->symbol_,p2->symbol_) != 0 ) return false;
    p1 = p1->parent_;
    p2 = p2->parent_;
  }
  return coco_string_equal(object1.symbol_,object2.symbol_) == 0;
}
//------------------------------------------------------------------------------
} // namespace kvm
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
