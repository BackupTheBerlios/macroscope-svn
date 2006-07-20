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
  ksys::Config  lconfig;
  if( config == NULL ) config = &lconfig;
  config->parse().override();
  const ksys::ConfigSection & section = config->section("libadicpp").section("default_connection");
  utf8::String stype(section.value("server_type", "FIREBIRD"));
  if( stype.strcasecmp("FIREBIRD") == 0 ){
    FirebirdDatabase * p = newObject<FirebirdDatabase>();
    ksys::AutoPtr< FirebirdDatabase> fb(p);
    const ksys::ConfigSection & section2  = section.section("firebird");
    p->name(ksys::unScreenString(section2.value("database")));
    p->params().add("user_name", section2.value("user", "sysdba"));
    p->params().add("password", section2.value("password", "masterkey"));
    p->params().add("role", section2.value("role"));
    p->params().add("dialect", section2.value("dialect"));
    p->params().add("page_size", section2.value("page_size"));
    p->params().add("lc_ctype", section2.value("default_charset"));
    p->params().add("lc_messages", section2.value("messages_charset"));
    fb.ptr(NULL);
    return p;
  }
  if( stype.strcasecmp("MYSQL") == 0 ){
    MYSQLDatabase * p = newObject<MYSQLDatabase>();
    ksys::AutoPtr< MYSQLDatabase> my(p);
    const ksys::ConfigSection & section2  = section.section("mysql");
    p->name(ksys::unScreenString(section2.value("database")));
    p->params().add("user_name", section2.value("user", "root"));
    p->params().add("password", section2.value("password"));
    p->params().add("protocol", section2.value("protocol"));
    my.ptr(NULL);
    return p;
  }
  throw ksys::ExceptionSP(ksys::Exception::newException(EINVAL, "unknown or unsupported server type"));
}
//---------------------------------------------------------------------------
} // namespace adicpp
//---------------------------------------------------------------------------
