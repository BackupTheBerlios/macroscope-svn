/*-
 * Copyright (C) 2005-2007 Guram Dukashvili. All rights reserved.
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
#include "msmail.h"
//------------------------------------------------------------------------------
#define _VERSION_C_AS_HEADER_
#include "version.c"
#undef _VERSION_C_AS_HEADER_
//------------------------------------------------------------------------------
int main(int _argc,char * _argv[])
{
//   Sleep(15000);
//  _set_amblksiz(1024);
  int errcode = 0;
  adicpp::AutoInitializer autoInitializer(_argc,_argv);
  autoInitializer = autoInitializer;
  try {
    uintptr_t u;
    stdErr.fileName(SYSLOG_DIR("msmail/") + "msmail.log");
    Config::defaultFileName(SYSCONF_DIR("") + "msmail.conf");
    Services services(msmail_version.gnu_);
    AutoPtr<msmail::Service> serviceAP(newObject<msmail::Service>());
    services.add(serviceAP);
    msmail::Service * service = serviceAP.ptr(NULL);
#if defined(__WIN32__) || defined(__WIN64__)
    bool dispatch = true;
#else
    bool dispatch = false;
#endif
    service->msmailConfig()->silent(true);
    for( u = 1; u < argv().count(); u++ ){
      if( argv()[u].strcmp("--chdir") == 0 && u + 1 < argv().count() ){
        changeCurrentDir(argv()[u + 1]);
      }
      else if( argv()[u].strcmp("-c") == 0 && u + 1 < argv().count() ){
        Config::defaultFileName(argv()[u + 1]);
        service->msmailConfig()->fileName(argv()[u + 1]);
      }
      else if( argv()[u].strcmp("--log") == 0 && u + 1 < argv().count() ){
        stdErr.fileName(argv()[u + 1]);
      }
    }
    for( u = 1; u < argv().count(); u++ ){
      if( argv()[u].strcmp("--version") == 0 ){
        stdErr.debug(9,utf8::String::Stream() << msmail_version.tex_ << "\n");
        fprintf(stdout,"%s\n",msmail_version.tex_);
        dispatch = false;
        continue;
      }
      if( argv()[u].strcmp("--install") == 0 ){
        for( uintptr_t j = u + 1; j < argv().count(); j++ )
          if( argv()[j].isSpace() )
            service->args(service->args() + " \"" + argv()[j] + "\"");
          else
            service->args(service->args() + " " + argv()[j]);
        services.install();
        dispatch = false;
      }
      else if( argv()[u].strcmp("--uninstall") == 0 ){
        services.uninstall();
        dispatch = false;
      }
      else if( argv()[u].strcmp("--start") == 0 && u + 1 < argv().count() ){
        services.start(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].strcmp("--stop") == 0 && u + 1 < argv().count() ){
        services.stop(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].strcmp("--suspend") == 0 && u + 1 < argv().count() ){
        services.suspend(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].strcmp("--resume") == 0 && u + 1 < argv().count() ){
        services.resume(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].strcmp("--query") == 0 && u + 1 < argv().count() ){
        services.query(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].strcmp("--debug-levels") == 0 && u + 1 < argv().count() ){
        stdErr.setDebugLevels(argv()[u + 1]);
      }
      else if( argv()[u].strcmp("--debug") == 0 && u + 1 < argv().count() ){
        stdErr.setAllDebugLevels(1);
      }
      else if( argv()[u].strcmp("--start-disp") == 0 ){
        dispatch = true;
      }
      else if( argv()[u].strcmp("--stop-disp") == 0 ){
        services.stopServiceCtrlDispatcher();
        dispatch = false;
      }
      else if( argv()[u].strcmp("--benchmark") == 0 ){
        RBTreeBenchmarkTree tree;
        tree.benchmark(10000000,3);
        dispatch = false;
      }
      else if( argv()[u].strcmp("--sha256") == 0 && u + 1 < argv().count() ){
        SHA256 passwordSHA256;
        passwordSHA256.make(argv()[u + 1].c_str(),argv()[u + 1].size());
        utf8::String b64(base64Encode(passwordSHA256.sha256(),32));
        fprintf(stdout,"%s\n",b64.c_str());
        copyStrToClipboard(b64);
        dispatch = false;
      }
#if PRIVATE_RELEASE
      else if( argv()[u].strcmp("--machine-key") == 0 ){
        utf8::String key(getMachineCleanUniqueKey());
        fprintf(stdout,"%s\n",key.c_str());
        copyStrToClipboard(key);
        dispatch = false;
      }
#endif
    }
//    if( dispatch ){
      service->msmailConfig()->parse().override();
      stdErr.bufferDataTTA(
        (uint64_t) service->msmailConfig()->value("debug_file_max_collection_time",60) * 1000000u
      );
      stdErr.rotationThreshold(
        service->msmailConfig()->value("debug_file_rotate_threshold",1024 * 1024)
      );
      stdErr.rotatedFileCount(
        service->msmailConfig()->value("debug_file_rotate_count",10)
      );
      stdErr.setDebugLevels(
        service->msmailConfig()->value("debug_levels","+0,+1,+2,+3")
      );
      stdErr.fileName(
        service->msmailConfig()->value("log_file",stdErr.fileName())
      );
      stackBackTrace = service->msmailConfig()->value("stack_back_trace",false);
      service->msmailConfig()->silent(false);

      const ConfigSection & section = service->msmailConfig()->section("reverse_dns_resolve_overrides");
      for( u = 0; u < section.valueCount(); u++ ){
        utf8::String key, value = section.value(u,&key);
        ksock::SockAddr::reverseResolveOverrideAdd(ksock::SockAddr().resolveName(key),value);
      }
      ksock::SockAddr addr;
      addr.resolveName("pleh");
      utf8::String host(addr.resolveAddr(~uintptr_t(0)));

      services.startServiceCtrlDispatcher();
  //  }
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
