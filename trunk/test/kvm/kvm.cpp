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
#define _VERSION_C_AS_HEADER_
#include "version.c"
#undef _VERSION_C_AS_HEADER_
#define ENABLE_GD_INTERFACE 1
//#define ENABLE_PCAP_INTERFACE 1
//#define ENABLE_ODBC_INTERFACE 1
#define ENABLE_MYSQL_INTERFACE 1
#define ENABLE_FIREBIRD_INTERFACE 1
#include <adicpp/adicpp.h>
#include "Parser.h"
#include "Scanner.h"
#include "CodeGenerator.h"
#include "SymbolTable.h"
#include "Compiler.h"
//------------------------------------------------------------------------------
using namespace ksys;
using namespace ksys::kvm;
using namespace adicpp;
//------------------------------------------------------------------------------
int main(int _argc,char * _argv[])
{
  //Sleep(15000);

  int errcode = EINVAL;
  adicpp::AutoInitializer autoInitializer(_argc,_argv);
  autoInitializer = autoInitializer;

  try {
    stdErr.fileName(includeTrailingPathDelimiter(SYSLOG_DIR(kvm_version.tag_)) + kvm_version.tag_ + ".log");
    Config::defaultFileName(SYSCONF_DIR("") + kvm_version.tag_ + ".conf");
    ConfigSP config(newObject<InterlockedConfig<FiberInterlockedMutex> >());
    Array<utf8::String> sources;
    for( uintptr_t i = 1; i < argv().count(); i++ ){
      if( argv()[i].strcmp("--version") == 0 ){
        stdErr.debug(9,utf8::String::Stream() << kvm_version.tex_ << "\n");
        fprintf(stdout,"%s\n",kvm_version.tex_);
        break;
      }
      else if( argv()[i].strcmp("--chdir") == 0 && i + 1 < argv().count() ){
        changeCurrentDir(argv()[++i]);
      }
      else if( argv()[i].strcmp("--log") == 0 && i + 1 < argv().count() ){
        stdErr.fileName(argv()[++i]);
      }
      else if( argv()[i].strcmp("-c") == 0 && i + 1 < argv().count() ){
        Config::defaultFileName(argv()[i + 1]);
        config->fileName(argv()[++i]);
      }
      else {
        sources.add(argv()[i]);
      }
    }
    config->silent(true).parse().override();
    stdErr.rotationThreshold(
      config->value("debug_file_rotate_threshold",1024 * 1024)
    );
    stdErr.rotatedFileCount(
      config->value("debug_file_rotate_count",10)
    );
    stdErr.setDebugLevels(
      config->value("debug_levels","+0,+1,+2,+3")
    );
    stdErr.fileName(
      config->value("log_file",stdErr.fileName())
    );
    stdErr.debug(0,
      utf8::String::Stream() << kvm_version.gnu_ << " started\n"
    );
    config->silent(false);
    utf8::String defaultConfigSectionName(config->text("default_config",kvm_version.gnu_));
    utf8::String defaultConnectionSectionName(config->textByPath(defaultConfigSectionName + ".connection","default_connection"));
    AutoPtr<Database> database(Database::newDatabase(&config->section(defaultConnectionSectionName)));
    
    for( uintptr_t i = 0; i < sources.count(); i++ ){
      AutoPtr<wchar_t> fileName(coco_string_create(sources[i].getUNICODEString()));
      AutoPtr<Scanner> scanner(newObjectV1<Scanner>(fileName.ptr()));
      AutoPtr<Parser> parser(newObjectV1<Parser>(scanner.ptr()));
      parser->gen = newObject<CodeGenerator>();
      parser->tab = newObjectV1<SymbolTable>(parser->gen.ptr());
	    parser->Parse();
	    if( parser->errors->count > 0 ){
        exit(EINVAL);
	    }
      Compiler compiler;
      compiler.detect(config);
      compiler.test(includeTrailingPathDelimiter(getPathFromPathName(sources[i])) + "config.h");
      parser->gen->generate(compiler,changeFileExt(sources[i],".cxx"));
    }
    stdErr.debug(0,
      utf8::String::Stream() << kvm_version.gnu_ << " stopped\n"
    );
    errcode = 0;
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
