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
#include "CodeGenerator.h"
#include "SymbolTable.h"
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
namespace kvm {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
CodeObject::~CodeObject()
{
}
//------------------------------------------------------------------------------
CodeObject::CodeObject() : childIndex_(-1)
{
}
//------------------------------------------------------------------------------
CodeObjectOwner * CodeObject::parent() const
{
  return dynamic_cast<CodeObjectOwner *>(symbols_[0].parent_->object_);
}
//------------------------------------------------------------------------------
wchar_t * CodeObject::symbol() const
{
  return symbols_[0].symbol_;
}
//------------------------------------------------------------------------------
utf8::String CodeObject::getCxxSymbol() const
{
  return symbol();
}
//------------------------------------------------------------------------------
utf8::String CodeObject::getCxxSymbols(const utf8::String & delimiter) const
{
  utf8::String s;
  for( uintptr_t i = 0; i < symbols_.count(); i++ ){
    if( i > 0 ) s += delimiter;
    s += symbols_[i].symbol_;
  }
  return s;
}
//------------------------------------------------------------------------------
utf8::String CodeObject::getMangledCxxSymbol() const
{
  const wchar_t * sym = symbol(), * p = sym;
  for(;;){
    wchar_t c = *p++;
    if( c == L'\0' ) return sym;
    if( (c >= L'0' && c <= L'9') ||
        (c >= L'A' && c <= L'Z') ||
        (c >= L'a' && c <= L'z') ||
        c == L'@' || c == L'_' ) ; else break;
  }
  utf8::String s(sym);
  s = base64Encode(s.c_str(),s.size());
  utf8::String::Iterator i(s);
  if( i.isDigit() ) i = (s = "@" + s);
  while( !i.eos() ){
    uintptr_t c = i.getChar();
    if( c == '+' ) *i.c_str() = '_';
    else
    if( c == '/' ) *i.c_str() = '@';
    i.next();
  }
  return s;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
CodeObjectOwner::~CodeObjectOwner()
{
}
//------------------------------------------------------------------------------
CodeObjectOwner::CodeObjectOwner()
{
}
//------------------------------------------------------------------------------
CodeObjectOwner::TypedList * CodeObjectOwner::getChildsByType(const char * type) const
{
  TypedList list(type);
  intptr_t c, i = childsByType_.bSearch(list,c,compareByType);
  return c == 0 ? &childsByType_[i] : NULL;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Class::~Class()
{
}
//------------------------------------------------------------------------------
Class::Class()
{
}
//------------------------------------------------------------------------------
void Class::generateCode(CodeGenerator & codeGenerator,AsyncFile & file,const utf8::String & margin)
{
  if( this != codeGenerator.root() ){
    file <<
      "\n" << margin << "class " << getMangledCxxSymbol() << " { // " <<
      getCxxSymbols() << "\n" <<
      margin << "  public:\n"
    ;
  }
  else {
    file << "using ksys::Mutant;\n";
  }
  for( uintptr_t i = 0; i < childs_.count(); i++ )
    childs_[i].generateCode(codeGenerator,file,margin + (this != codeGenerator.root() ? "    " : ""));
  if( this != codeGenerator.root() ) file << margin << "};\n";
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Class::Member::~Member()
{
}
//------------------------------------------------------------------------------
Class::Member::Member()
{
}
//------------------------------------------------------------------------------
void Class::Member::generateCode(CodeGenerator & codeGenerator,AsyncFile & file,const utf8::String & margin)
{
  file << margin << "Mutant " << getMangledCxxSymbol() << ";\n";
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Class::MemberFunc::~MemberFunc()
{
}
//------------------------------------------------------------------------------
Class::MemberFunc::MemberFunc()
{
}
//------------------------------------------------------------------------------
void Class::MemberFunc::generateCode(CodeGenerator & codeGenerator,AsyncFile & file,const utf8::String & margin)
{
  file << margin << "Mutant " << getMangledCxxSymbol() << "(\n";
  TypedList * list = getChildsByType(typeid(Class::MemberFunc::Param).raw_name());
  if( list != NULL ){
    for( EmbeddedListNode<CodeObject> * node = list->first(); node != NULL; node = node->next() ){
      file <<
        margin << "  Mutant " << CodeObject::listNodeObject(*node).getMangledCxxSymbol() <<
        (node != list->last() ? ",\n" : "\n")
      ;
    }
  }
  file << margin << ")\n";
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
CodeGenerator::~CodeGenerator()
{
}
//------------------------------------------------------------------------------
CodeGenerator::CodeGenerator()
{
}
//------------------------------------------------------------------------------
CodeGenerator & CodeGenerator::generate(const utf8::String & fileName)
{
  AsyncFile file(fileName);
  file.createIfNotExist(true).open().resize(0);
  root_.generateCode(*this,file,utf8::String());
  return *this;
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Expression::~Expression()
{
}
//------------------------------------------------------------------------------
Expression::Expression() : count_(0), max_(0)
{
}
//------------------------------------------------------------------------------
Expression & Expression::add(CodeObject * object)
{
  if( count_ >= max_ ){
    expression_.reallocT((max_ << 1) + ((max_ == 0) << 5));
    max_ = (max_ << 1) + ((max_ == 0) << 5);
  }
  expression_[count_++] = object;
  return *this;
}
//------------------------------------------------------------------------------
} // namespace kvm
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
