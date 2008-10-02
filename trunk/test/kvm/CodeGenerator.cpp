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
utf8::String CodeObject::getCxxSymbols(uintptr_t startFrom,const utf8::String & delimiter) const
{
  utf8::String s;
  for( uintptr_t i = startFrom >= symbols_.count() ? startFrom - 1 : startFrom; i < symbols_.count(); i++ ){
    if( !s.isNull() ) s += delimiter;
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
void Class::generateCode(const CodeGeneratorParameters & p)
{
  if( this != p.codeGenerator_.root() ){
    p.file_ <<
      "\n" << p.margin_<< "class " << getMangledCxxSymbol() << " {";
    if( getCxxSymbol() != getCxxSymbols() || getCxxSymbol() != getMangledCxxSymbol() )
      p.file_ << " // " << getCxxSymbols();
    p.file_ << "\n" << p.margin_<< "  public:\n";
  }
  else {
    p.file_ << "using ksys::Mutant;\n";
  }
  CodeGeneratorParameters p2(p,p.margin_+ (this != p.codeGenerator_.root() ? "    " : ""));
  for( uintptr_t i = 0; i < childs_.count(); i++ ) childs_[i].generateCode(p2);
  if( this != p.codeGenerator_.root() ) p.file_ << p.margin_<< "};\n";
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
void Class::Member::generateCode(const CodeGeneratorParameters & p)
{
  if( p.caller_ == NULL ){
    p.file_ << p.margin_<< "Mutant " << getMangledCxxSymbol() << ";";
    if( getCxxSymbol() != getCxxSymbols() || getCxxSymbol() != getMangledCxxSymbol() )
      p.file_ << " // " << getCxxSymbols();
    p.file_ << "\n";
  }
  else {
    p.file_ << getMangledCxxSymbol();
  }
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
void Class::MemberFunc::generateCode(const CodeGeneratorParameters & p)
{
  if( p.caller_ == NULL ){
    p.file_ << p.margin_<< "Mutant " << getMangledCxxSymbol() << "(";
    if( getCxxSymbol() != getCxxSymbols() || getCxxSymbol() != getMangledCxxSymbol() )
      p.file_ << " // " << getCxxSymbols();
    p.file_ << "\n";
    TypedList * list = getChildsByType(typeid(Class::MemberFunc::Param).raw_name());
    if( list != NULL ){
      for( EmbeddedListNode<CodeObject> * node = list->first(); node != NULL; node = node->next() ){
        CodeObject & object = CodeObject::listNodeObject(*node);
        p.file_ <<
          p.margin_<< "  Mutant " << object.getMangledCxxSymbol() <<
          (node != list->last() ? "," : "")
        ;
        if( object.getCxxSymbol() != object.getCxxSymbols() || object.getCxxSymbol() != object.getMangledCxxSymbol() )
          p.file_ << " // " << object.getCxxSymbols();
        if( node != list->last() ) p.file_ << "\n";
      }
    }
    list = getChildsByType(typeid(Class::MemberFunc::CodeBlock).raw_name());
    if( list != NULL && list->count() > 0 ){
      p.file_ << ")\n";
      for( EmbeddedListNode<CodeObject> * node = list->first(); node != NULL; node = node->next() ){
        CodeObject & object = CodeObject::listNodeObject(*node);
        object.generateCode(p);
      }
    }
    else {
      p.file_ << ");\n";
    }
  }
  else {
    p.file_ << getMangledCxxSymbol();
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Class::MemberFunc::Param::~Param()
{
}
//------------------------------------------------------------------------------
Class::MemberFunc::Param::Param()
{
}
//------------------------------------------------------------------------------
void Class::MemberFunc::Param::generateCode(const CodeGeneratorParameters & p)
{
  p.file_ << getMangledCxxSymbol();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Class::MemberFunc::CodeBlock::~CodeBlock()
{
}
//------------------------------------------------------------------------------
Class::MemberFunc::CodeBlock::CodeBlock()
{
}
//------------------------------------------------------------------------------
void Class::MemberFunc::CodeBlock::generateCode(const CodeGeneratorParameters & p)
{
  p.file_ << p.margin_ << "{\n";
  CodeGeneratorParameters p2(p,p.margin_+ "  ");
  TypedList * list = getChildsByType(typeid(Class::MemberFunc::CodeBlock::Variable).raw_name());
  if( list != NULL ){
    for( EmbeddedListNode<CodeObject> * node = list->first(); node != NULL; node = node->next() ){
      CodeObject & object = CodeObject::listNodeObject(*node);
      object.generateCode(p2);
    }
  }
  list = getChildsByType(typeid(Expression).raw_name());
  if( list != NULL ){
    for( EmbeddedListNode<CodeObject> * node = list->first(); node != NULL; node = node->next() ){
      CodeObject & object = CodeObject::listNodeObject(*node);
      object.generateCode(p2);
    }
  }
  p.file_ << p.margin_ << "}\n";
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
void Expression::generateCode(const CodeGeneratorParameters & p)
{
  if( p.caller_ == NULL ) p.file_ << p.margin_;
  CodeGeneratorParameters p2(p);
  p2.caller_ = this;
  for( uintptr_t i = 0; i < count_; i++ ){
    if( expression_[i] == NULL ) continue;
    Expression * e = dynamic_cast<Expression *>(expression_[i]);
    if( e != NULL ) p.file_ << "(";
    expression_[i]->generateCode(p2);
    if( e != NULL ) p.file_ << ")";
  }
  if( p.caller_ == NULL ) p.file_ << ";\n";
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Expression::Equ::~Equ()
{
}
//------------------------------------------------------------------------------
Expression::Equ::Equ()
{
}
//------------------------------------------------------------------------------
void Expression::Equ::generateCode(const CodeGeneratorParameters & p)
{
  p.file_ << " = ";
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Expression::Plus::~Plus()
{
}
//------------------------------------------------------------------------------
Expression::Plus::Plus()
{
}
//------------------------------------------------------------------------------
void Expression::Plus::generateCode(const CodeGeneratorParameters & p)
{
  p.file_ << " + ";
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Expression::Minus::~Minus()
{
}
//------------------------------------------------------------------------------
Expression::Minus::Minus()
{
}
//------------------------------------------------------------------------------
void Expression::Minus::generateCode(const CodeGeneratorParameters & p)
{
  p.file_ << " - ";
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Expression::Mul::~Mul()
{
}
//------------------------------------------------------------------------------
Expression::Mul::Mul()
{
}
//------------------------------------------------------------------------------
void Expression::Mul::generateCode(const CodeGeneratorParameters & p)
{
  p.file_ << " * ";
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Expression::Div::~Div()
{
}
//------------------------------------------------------------------------------
Expression::Div::Div()
{
}
//------------------------------------------------------------------------------
void Expression::Div::generateCode(const CodeGeneratorParameters & p)
{
  p.file_ << " / ";
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Expression::Return::~Return()
{
}
//------------------------------------------------------------------------------
Expression::Return::Return()
{
}
//------------------------------------------------------------------------------
void Expression::Return::generateCode(const CodeGeneratorParameters & p)
{
  p.file_ << "return ";
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
#ifndef NDEBUG
  file.codePage(CP_OEMCP);
#endif
  root_.generateCode(CodeGeneratorParameters(*this,file,utf8::String()));
  return *this;
}
//------------------------------------------------------------------------------
} // namespace kvm
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
