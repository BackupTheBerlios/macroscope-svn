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
//---------------------------------------------------------------------------
#ifndef cgiH
#define cgiH
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
enum CGIMethod { cgiInit, cgiNone, cgiPOST, cgiGET, cgiHEAD };
//---------------------------------------------------------------------------
class CGI {
  public:
    virtual ~CGI();
    CGI();
    
    void initialize();
    CGI & print(const utf8::String & s);
    CGIMethod method() const;
    
    CGI & operator << (const utf8::String & s);
    CGI & writeBuffer(const void * buf,uint64_t size);
    
    utf8::String paramAsString(const utf8::String & name,const utf8::String & defValue = utf8::String());
    utf8::String paramAsString(uintptr_t i,const utf8::String & defValue = utf8::String());
    Mutant paramAsMutant(const utf8::String & name,const Mutant & defValue = Mutant());
    Mutant paramAsMutant(uintptr_t i,const Mutant & defValue = Mutant());
    intptr_t paramIndex(const utf8::String & name,bool noThrow = true);
    utf8::String paramName(uintptr_t i);
    uintptr_t paramCount();

    utf8::String contentType() const { return contentType_; }
    CGI & contentType(const utf8::String & type){ contentType_ = type; return *this; }
    
    static utf8::String uuencode(const utf8::String & string);
    static utf8::String uudecode(const utf8::String & string);
    
    bool isCGI() const { return method() != cgiNone; }
  protected:
    AsyncFile out_;
    mutable utf8::String queryString_;
    mutable CGIMethod method_;
    
    class Param {
      friend class CGI;
      public:
        virtual ~Param() {}
	      Param(const utf8::String & name = utf8::String(),const utf8::String & value = utf8::String()) :
	        name_(name), value_(value) {}
      protected:
        static EmbeddedHashNode<Param,uintptr_t> & ehNLT(const uintptr_t & link,Array<Param> * & param){
          return keyNode((*param)[link - 1]);
        }
        static uintptr_t ehLTN(const EmbeddedHashNode<Param,uintptr_t> & node,Array<Param> * & param){
          return &keyNodeObject(node,NULL) - &(*param)[0] + 1;
        }
        static EmbeddedHashNode<Param,uintptr_t> & keyNode(const Param & object){
          return object.keyNode_;
        }
      	static Param & keyNodeObject(const EmbeddedHashNode<Param,uintptr_t> & node,Param * p){
          return node.object(p->keyNode_);
        }
        static uintptr_t keyNodeHash(const Param & object){
          return object.name_.hash(false);
        }
        static bool keyHashNodeEqu(const Param & object1,const Param & object2){
          return object1.name_.strcasecmp(object2.name_) == 0;
        }
        mutable EmbeddedHashNode<Param,uintptr_t> keyNode_;
	
	      utf8::String name_;
	      utf8::String value_;
      private:
    };
    typedef EmbeddedHash<
      Param,
      uintptr_t,
      Array<Param> *,
      Param::ehNLT,
      Param::ehLTN,
      Param::keyNode,
      Param::keyNodeObject,
      Param::keyNodeHash,
      Param::keyHashNodeEqu
    > Params;
    Params paramsHash_;
    Array<Param> params_;
    utf8::String contentType_;
    bool contentTypePrinted_;

    void initalizeByMethodPOST();
    void initalizeByMethodGET();
  private:
    CGI(const CGI &);
    void operator = (const CGI &);
};
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
