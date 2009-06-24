/*-
 * Copyright 2008-2009 Guram Dukashvili
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
#ifndef _compilerH_
#define _compilerH_
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
namespace kvm {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Compiler : public Object {
  public:
    virtual ~Compiler();
    Compiler();

    Compiler & detect(const ConfigSP config);
    Compiler & test(const utf8::String & config);
    Compiler & compile(const utf8::String & config,const utf8::String & source,const utf8::String & object);
    Compiler & link(const utf8::String & module,const utf8::String & objects,bool dlm = true);

    Compiler & includeDirectories(const utf8::String & v){ includeDirectories_ = v; return *this; }
    const utf8::String & includeDirectories() const { return includeDirectories_; }
    Compiler & libDirectories(const utf8::String & v){ libDirectories_ = v; return *this; }
    const utf8::String & libDirectories() const { return libDirectories_; }
    Compiler & libraries(const utf8::String & v){ libraries_ = v; return *this; }
    const utf8::String & libraries() const { return libraries_; }
  protected:
    utf8::String type_;
    utf8::String compiler_;
    utf8::String compilerVersion_;
    utf8::String compilerArgs_;
    Array<utf8::String> compilerEnv_;
    utf8::String linker_;
    utf8::String linkerArgs_;
    utf8::String linkerDlArgs_;
    utf8::String includeDirectories_;
    utf8::String libDirectories_;
    utf8::String libraries_;
    bool keepStderr_;

    bool testCxx(const utf8::String & config,const utf8::String & test,const utf8::String & tmpCxx);
    bool testLinkCxx(
      const utf8::String & config,
      const utf8::String & test,
      const utf8::String & tmpCxx,
      const utf8::String & libraries = utf8::String());
    intptr_t testRunCxx(
      const utf8::String & config,
      const utf8::String & test,
      const utf8::String & tmpCxx,
      const utf8::String & libraries = utf8::String());

    utf8::String testCxxHeaderHelper(
      const utf8::String & config,
      const utf8::String & header,
      const utf8::String & tmpCxx,
      const utf8::String & headers = utf8::String());

    utf8::String testCxxTypeHelper(
      const utf8::String & config,
      const utf8::String & type,
      const utf8::String & member,
      const utf8::String & tmpCxx,
      const utf8::String & header,
      const utf8::String & body);

    utf8::String testCxxTypeEqualCheck(
      const utf8::String & config,
      const utf8::String & type1,
      const utf8::String & type2,
      const utf8::String & tmpCxx,
      const utf8::String & header);

    utf8::String testCxxFuncExists(
      const utf8::String & config,
      const utf8::String & func,
      const utf8::String & tmpCxx,
      const utf8::String & header);

    utf8::String testCxxSymbolExists(
      const utf8::String & config,
      const utf8::String & symbol,
      const utf8::String & tmpCxx,
      const utf8::String & header);

    utf8::String testCxxLibExists(
      const utf8::String & library,
      const utf8::String & symbol,
      const utf8::String & mod,
      const utf8::String & tmpCxx,
      utf8::String * existsLibraries = NULL);

    intptr_t testCxxCode(
      const utf8::String & config,
      const utf8::String & tmpCxx,
      const utf8::String & header,
      const utf8::String & body);
  private:
    Compiler(const Compiler &);
    void operator = (const Compiler &);
};
//------------------------------------------------------------------------------
} // namespace kvm
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
