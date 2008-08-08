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
#ifndef _CodeGeneratorH_
#define _CodeGeneratorH_
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
namespace kvm {
//------------------------------------------------------------------------------
class Symbol;
class CodeObjectOwner;
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class CodeObject : public Object { // base class only
  friend class SymbolTable;
  public:
    virtual ~CodeObject() {}
    CodeObject() {}

    CodeObjectOwner * parent() const;
    wchar_t * symbol() const;
  protected:
    Vector<Symbol> symbols_;
    mutable EmbeddedListNode<CodeObject> listNode_;

    static EmbeddedListNode<CodeObject> & listNode(const CodeObject & object){
      return object.listNode_;
    }
    static CodeObject & listNodeObject(const EmbeddedListNode<CodeObject> & node,CodeObject * p = NULL){
      return node.object(p->listNode_);
    }
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class CodeObjectOwner : virtual public CodeObject { // base class only
  friend class SymbolTable;
  public:
    virtual ~CodeObjectOwner() {}
    CodeObjectOwner() {}
  protected:
    Vector<CodeObject> childs_;

    class TypedList :
      public EmbeddedList<
        CodeObject,
        listNode,
        listNodeObject> {
      public:
        TypedList(const char * type = NULL) : type_(type) {}
        const char * type_;
    };

    static intptr_t compareByType(const TypedList & p1,const TypedList & p2){
      return strcmp(p1.type_,p2.type_);
    }

    Vector<TypedList> childsByType_;
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Class : public CodeObjectOwner {
  public:
    virtual ~Class() {}
    Class() {}

    class Member : virtual public CodeObject {
      public:
        virtual ~Member() {}
        Member() {}
    };

    class MemberFunc : public Member, public CodeObjectOwner {
      public:
        virtual ~MemberFunc() {}
        MemberFunc() {}

        class Param : public CodeObject {
          public:
            virtual ~Param() {}
            Param() {}
        };

        class CodeBlock : public CodeObjectOwner {
          public:
            virtual ~CodeBlock() {}
            CodeBlock() {}

            class Variable : public CodeObject {
              public:
                virtual ~Variable() {}
                Variable() {}
            };
        };
    };
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Expression : public CodeObjectOwner {
  public:
    virtual ~Expression();
    Expression();

    class Operator : public CodeObject { // base class only
      public:
        virtual ~Operator() {}
        Operator() {}
    };

    class Plus : public Operator {
      public:
        virtual ~Plus() {}
        Plus() {}
    };

    class Minus : public Operator {
      public:
        virtual ~Minus() {}
        Minus() {}
    };

    class Mul : public Operator {
      public:
        virtual ~Mul() {}
        Mul() {}
    };

    class Div : public Operator {
      public:
        virtual ~Div() {}
        Div() {}
    };

    Expression & add(CodeObject * object);
  protected:
    AutoPtr<CodeObject *> expression_;
    uintptr_t count_;
    uintptr_t max_;
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class CodeGenerator : public Object {
  friend class SymbolTable;
  public:
    virtual ~CodeGenerator();
    CodeGenerator();

    CodeGenerator & generate(const utf8::String & fileName);
  protected:
    Class root_;
  private:
    CodeGenerator(const CodeGenerator &);
    void operator = (const CodeGenerator &);
};
//------------------------------------------------------------------------------
} // namespace kvm
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
