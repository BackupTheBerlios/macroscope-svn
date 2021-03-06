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
class CodeObject;
class CodeObjectOwner;
class CodeGenerator;
class Compiler;
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class CodeGeneratorParameters {
  public:
    CodeGeneratorParameters(const CodeGeneratorParameters & copy,const utf8::String & margin) :
      codeGenerator_(copy.codeGenerator_), file_(copy.file_), margin_(margin), caller_(NULL) {}
    CodeGeneratorParameters(CodeGenerator & codeGenerator,AsyncFile & file,const utf8::String & margin) :
      codeGenerator_(codeGenerator), file_(file), margin_(margin), caller_(NULL) {}

    CodeGenerator & codeGenerator_;
    AsyncFile & file_;
    utf8::String margin_;
    utf8::String config_;
    CodeObject * caller_;
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class CodeObject : virtual public Object { // base class only
  friend class CodeGenerator;
  friend class SymbolTable;
  public:
    virtual ~CodeObject();
    CodeObject();

    CodeObjectOwner * parent() const;
    wchar_t * symbol() const;

    virtual void generateCode(const CodeGeneratorParameters &) {}

    static EmbeddedListNode<CodeObject> & listNode(const CodeObject & object){
      return object.listNode_;
    }
    static CodeObject & listNodeObject(const EmbeddedListNode<CodeObject> & node,CodeObject * p = NULL){
      return node.object(p->listNode_);
    }

    utf8::String getCxxSymbol() const;
    utf8::String getCxxSymbols(uintptr_t startFrom = 0,const utf8::String & delimiter = ", ") const;
    utf8::String getMangledCxxSymbol() const;
  protected:
    Vector<Symbol> symbols_;
    mutable EmbeddedListNode<CodeObject> listNode_;
    intptr_t childIndex_;
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class CodeObjectOwner : virtual public CodeObject { // base class only
  friend class CodeGenerator;
  friend class SymbolTable;
  public:
    virtual ~CodeObjectOwner();
    CodeObjectOwner();
  protected:
    Vector<CodeObject> childs_;

    class TypedList :
      public EmbeddedList<
        CodeObject,
        listNode,
        listNodeObject> {
      public:
        virtual ~TypedList() {}
        TypedList(const char * type = NULL) : type_(type) {}
        const char * type_;
    };

    static intptr_t compareByType(const TypedList & p1,const TypedList & p2){
      return strcmp(p1.type_,p2.type_);
    }

    TypedList * getChildsByType(const char * type) const;

    mutable Vector<TypedList> childsByType_;
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Class : public CodeObjectOwner {
  public:
    virtual ~Class();
    Class();

    void generateCode(const CodeGeneratorParameters & p);

    class Member : virtual public CodeObject {
      public:
        virtual ~Member();
        Member();

        void generateCode(const CodeGeneratorParameters & p);
    };

    class MemberFunc : public CodeObjectOwner, public Member {
      public:
        virtual ~MemberFunc();
        MemberFunc();

        void generateCode(const CodeGeneratorParameters & p);

        class Param : public CodeObject {
          public:
            virtual ~Param();
            Param();

            void generateCode(const CodeGeneratorParameters & p);
        };

        class CodeBlock : public CodeObjectOwner {
          public:
            virtual ~CodeBlock();
            CodeBlock();

            void generateCode(const CodeGeneratorParameters & p);
    
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

    void generateCode(const CodeGeneratorParameters & p);

    class Operator : public CodeObject { // base class only
      public:
        virtual ~Operator() {}
        Operator() {}
    };

    class Return : public CodeObject {
      public:
        virtual ~Return();
        Return();

        void generateCode(const CodeGeneratorParameters & p);
    };

    class Equ : public Operator {
      public:
        virtual ~Equ();
        Equ();

        void generateCode(const CodeGeneratorParameters & p);
    };

    class Plus : public Operator {
      public:
        virtual ~Plus();
        Plus();

        void generateCode(const CodeGeneratorParameters & p);
    };

    class Minus : public Operator {
      public:
        virtual ~Minus();
        Minus();

        void generateCode(const CodeGeneratorParameters & p);
    };

    class Mul : public Operator {
      public:
        virtual ~Mul();
        Mul();

        void generateCode(const CodeGeneratorParameters & p);
    };

    class Div : public Operator {
      public:
        virtual ~Div();
        Div();

        void generateCode(const CodeGeneratorParameters & p);
    };

    Expression & add(CodeObject * object);
  protected:
    Array<CodeObject *> expression_;
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

    CodeGenerator & generate(Compiler & compiler,const utf8::String & config,const utf8::String & fileName);
    const CodeObject * const root() const { return &root_; }
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
