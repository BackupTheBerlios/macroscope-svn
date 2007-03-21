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
#ifndef cgiH
#define cgiH
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
enum CGIMethod { cgiNone, cgiPOST, cgiGET, cgiHEAD };
//---------------------------------------------------------------------------
class CGI {
  public:
    ~CGI();
    CGI();
    
    void initialize();
    CGI & print(const utf8::String & s);
    CGIMethod method();
    
    CGI & operator << (const utf8::String & s){ return print(s); }
    
    utf8::String paramAsString(const utf8::String & name);
    utf8::String paramAsString(uintptr_t i);
    Mutant paramAsMutant(const utf8::String & name);
    Mutant paramAsMutant(uintptr_t i);
    intptr_t paramIndex(const utf8::String & name);
    utf8::String paramName(uintptr_t i);
    uintptr_t paramCount();
    
    bool isCGI(){ return method() != cgiNone; }
  protected:
    utf8::String queryString_;
    CGIMethod method_;
    
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
