/*-
 * Copyright 2005-2007 Guram Dukashvili
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
  if( config == NULL ) config = &lconfig.silent(true);
  config->parse().override();
  utf8::String stype0(config->valueByPath("libadicpp.default_connection.server_type","MYSQL"));
  utf8::String stype(stype0.lower());
  utf8::String section("libadicpp.default_connection." + stype);
  if( stype.strcasecmp("firebird") == 0 ){
    ksys::AutoPtr<FirebirdDatabase> p(newObject<FirebirdDatabase>());
    p->name(config->valueByPath(section + ".database"));
    p->params().add("user_name",config->valueByPath(section + ".user","sysdba"));
    p->params().add("password",config->valueByPath(section + ".password","masterkey"));
    p->params().add("role",config->valueByPath(section + ".role"));
    p->params().add("dialect",config->valueByPath(section + ".dialect"));
    p->params().add("page_size",config->valueByPath(section + ".page_size"));
    p->params().add("lc_ctype",config->valueByPath(section + ".default_charset"));
    p->params().add("lc_messages",config->valueByPath(section + ".messages_charset"));
    return p.ptr(NULL);
  }
  if( stype.strcasecmp("MYSQL") == 0 ){
    ksys::AutoPtr<MYSQLDatabase> p(newObject<MYSQLDatabase>());
    p->name(config->valueByPath(section + ".database"));
    p->params().add("user_name",config->valueByPath(section + ".user","root"));
    p->params().add("password",config->valueByPath(section + ".password"));
    p->params().add("protocol",config->valueByPath(section + ".protocol"));
    p->params().add("connect_timeout",config->valueByPath(section + ".connect_timeout",0));
    p->params().add("read_timeout",config->valueByPath(section + ".read_timeout",0));
    p->params().add("write_timeout",config->valueByPath(section + ".write_timeout",0));
    p->params().add("reconnect",config->valueByPath(section + ".reconnect",false));
    return p.ptr(NULL);
  }
  newObjectV1C2<ksys::Exception>(EINVAL,"unknown or unsupported server type: " + stype0)->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
} // namespace adicpp
//---------------------------------------------------------------------------
