/*-
 * Copyright 2006-2007 Guram Dukashvili
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
#include <adicpp/adicpp.h>
#define _VERSION_C_AS_HEADER_
#include "version.c"
#undef _VERSION_C_AS_HEADER_
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
using namespace ksys;
//------------------------------------------------------------------------------
int main(int ac,char * av[])
{
  int errcode = 0;
  adicpp::AutoInitializer autoInitializer(ac,av);
  autoInitializer = autoInitializer;
  try {
    union {
      intptr_t i;
      uintptr_t u;
    };
    ksys::stdErr.fileName(SYSLOG_DIR("mskey/") + "mskey.log");
    ksys::Config::defaultFileName(SYSCONF_DIR("") + "mskey.conf");
    utf8::String expirationDate("00.00.0000");
    for( u = 1; u < argv().count(); u++ ){
      if( argv()[u].compare("--version") == 0 ){
        stdErr.debug(9,utf8::String::Stream() << mskey_version.tex_ << "\n");
        fprintf(stdout,"%s\n",mskey_version.tex_);
        continue;
      }
      if( argv()[u].compare("--expiration-date") == 0 && u + 1 < argv().count() ){
        expirationDate = argv()[++u];
        continue;
      }
      utf8::String keyAdd;
      if( gettimeofday() > timeFromTimeString("13.06.2008",false) ){
        Randomizer rnd;
        keyAdd = utf8::int2Str(rnd.random());
      }
      utf8::String key(getMachineCryptedUniqueKey(argv()[u] + keyAdd,expirationDate));
      fprintf(stdout,(const char *) key.getANSIString());
      copyStrToClipboard(key);
    }
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    errcode = e->code() >= errorOffset ? e->code() - errorOffset : e->code();
  }
  catch( ... ){
  }
  return errcode;
}
//------------------------------------------------------------------------------
