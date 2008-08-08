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
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class CodeObject : public Object { // base class
  friend class SymbolTable;
  public:
    virtual ~CodeObject() {}
    CodeObject() {}

    CodeObject * parent() const;
    wchar_t * symbol() const;
  protected:
    Vector<Symbol> symbols_;
    Vector<CodeObject> childs_;

    static EmbeddedListNode<CodeObject> & listNode(const CodeObject & object){
      return object.listNode_;
    }
    static CodeObject & listNodeObject(const EmbeddedListNode<CodeObject> & node,CodeObject * p = NULL){
      return node.object(p->listNode_);
    }

    class TypedList :
      public EmbeddedList<
        CodeObject,
        listNode,
        listNodeObject> {
      public:
        TypedList(CodeObject * object = NULL) : object_(object) {}
        CodeObject * object_;
    };

    static intptr_t compareByType(const TypedList & p1,const TypedList & p2){
      return strcmp(Object::getClassName(p1.object_),Object::getClassName(p2.object_));
    }

    Vector<TypedList> childsByType_;
  private:
    mutable EmbeddedListNode<CodeObject> listNode_;
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Class : public CodeObject {
  public:
    virtual ~Class() {}
    Class() {}

    class Member : public CodeObject {
      public:
        virtual ~Member() {}
        Member() {}
    };

    class MemberFunc : public Member {
      public:
        virtual ~MemberFunc() {}
        MemberFunc() {}

        class FuncObject : public CodeObject { // base class only
          protected:
            virtual ~FuncObject() {}
            FuncObject() {}
        };

        class Param : public FuncObject {
          public:
            virtual ~Param() {}
            Param() {}
        };

        class CodeBlock : public FuncObject {
          public:
            virtual ~CodeBlock() {}
            CodeBlock() {}

            class Variable : public FuncObject {
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
class Expression : public CodeObject {
  public:
    virtual ~Expression() {}
    Expression() {}

    class Operator : public CodeObject { // base class only
      protected:
        virtual ~Operator() {}
        Operator() {}
    };

    class Plus : public Operator {
      protected:
        virtual ~Plus() {}
        Plus() {}
    };

    class Minus : public Operator {
      protected:
        virtual ~Minus() {}
        Minus() {}
    };

    class Mul : public Operator {
      protected:
        virtual ~Mul() {}
        Mul() {}
    };

    class Div : public Operator {
      protected:
        virtual ~Div() {}
        Div() {}
    };
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class CodeGenerator : public Object {
  friend class SymbolTable;
  public:
    virtual ~CodeGenerator();
    CodeGenerator();
  protected:
    CodeObject root_;
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
