/*-
 * Copyright 2005 Guram Dukashvili
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
#include <adicpp/mycpp.h>
//---------------------------------------------------------------------------
namespace mycpp {
//---------------------------------------------------------------------------
extern const MYSQLErrorDesc mysqlErrors[] = {
#include <adicpp/myapi/mysqld_ername.h>
};
//---------------------------------------------------------------------------
utf8::String strErrorHandler(int32_t err)
{
  MYSQLErrorDesc  bs;
  bs.code_ = err;
  intptr_t  c = sizeof(mysqlErrors) / sizeof(mysqlErrors[0]);
  uintptr_t i = ksys::bSearch<MYSQLErrorDesc>(mysqlErrors, bs, c);
  if( c == 0 ) return mysqlErrors[i].error_ + 3;
  return utf8::String();
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
EClientServer::~EClientServer()
{
}
//---------------------------------------------------------------------------
EClientServer::EClientServer(int32_t code, const utf8::String what)
  : ksys::Exception(code, what)
{
//  utf8::String se(strErrorHandler(code));
//  if( se.strlen() > 0 ) whats_[0] += ", " + se;
}
//---------------------------------------------------------------------------
bool EClientServer::isFatalError() const
{
  for( intptr_t i = codes_.count() - 1; i >= 0; i-- )
    switch( codes_[i] ){
      case ER_MASTER_NET_READ :
      case ER_MASTER_NET_WRITE :
        return true;
    }
  return false;
}
//---------------------------------------------------------------------------
} // namespace mycpp
//---------------------------------------------------------------------------

