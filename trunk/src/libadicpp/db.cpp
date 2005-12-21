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
#include <adicpp/adicpp.h>
//---------------------------------------------------------------------------
namespace adicpp {
//---------------------------------------------------------------------------
Database * Database::newDatabase(ksys::Config * config)
{
  ksys::Config lconfig;
  if( config == NULL ) config = &lconfig;
  config->parse().override();
  const ksys::ConfigSection & section =
    config->section(utf8::string("libadicpp")).section(utf8::string("default_connection"));
  utf8::String stype(section.value(utf8::string("server_type"),"FIREBIRD"));
  if( stype.strcasecmp(utf8::string("FIREBIRD")) == 0 ){
    FirebirdDatabase * p = new FirebirdDatabase;
    ksys::AutoPtr<FirebirdDatabase> fb(p);
    const ksys::ConfigSection & section2 = section.section(utf8::string("firebird"));
    p->name(section2.value(utf8::string("database")));
    p->params().add(utf8::string("user_name"),section2.value(utf8::string("user"),"sysdba"));
    p->params().add(utf8::string("password"),section2.value(utf8::string("password"),"masterkey"));
    p->params().add(utf8::string("role"),section2.value(utf8::string("role")));
    p->params().add(utf8::string("dialect"),section2.value(utf8::string("dialect")));
    p->params().add(utf8::string("page_size"),section2.value(utf8::string("page_size")));
    p->params().add(utf8::string("lc_ctype"),section2.value(utf8::string("default_charset")));
    p->params().add(utf8::string("lc_messages"),section2.value(utf8::string("messages_charset")));
    fb.ptr(NULL);
    return p;
  }
  if( stype.strcasecmp(utf8::string("MYSQL")) == 0 ){
    MYSQLDatabase * p = new MYSQLDatabase;
    ksys::AutoPtr<MYSQLDatabase> my(p);
    const ksys::ConfigSection & section2 = section.section(utf8::string("mysql"));
    p->name(section2.value(utf8::string("database")));
    p->params().add(utf8::string("user_name"),section2.value(utf8::string("user"),"root"));
    p->params().add(utf8::string("password"),section2.value(utf8::string("password")));
    p->params().add(utf8::string("protocol"),section2.value(utf8::string("protocol")));
    my.ptr(NULL);
    return p;
  }
  throw ksys::ExceptionSP(
    new ksys::Exception(-1,
    utf8::string("unknown or unsupported server type"))
  );
}
//---------------------------------------------------------------------------
} // namespace adicpp
//---------------------------------------------------------------------------
