/*-
 * Copyright (C) 2005-2006 Guram Dukashvili. All rights reserved.
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
#include <adicpp/adicpp.h>
#include "msmail.h"
#include "version.h"
//------------------------------------------------------------------------------
int main(int _argc,char * _argv[])
{
//  sleep(15000000);
//  _set_amblksiz(1024);

  int errcode = 0;
  adicpp::AutoInitializer autoInitializer;
  autoInitializer = autoInitializer;
  try {
    union {
      intptr_t i;
      uintptr_t u;
    };
    initializeArguments(_argc,_argv);
    Config::defaultFileName(SYSCONF_DIR + "msmail.conf");
    Services services(msmail_version.v_gnu);
    AutoPtr<msmail::Service> serviceAP(newObject<msmail::Service>());
    services.add(serviceAP);
    msmail::Service * service = serviceAP.ptr(NULL);
#if defined(__WIN32__) || defined(__WIN64__)
    bool dispatch = true;
#else
    bool dispatch = false;
#endif
    for( u = 1; u < argv().count(); u++ ){
      if( argv()[u].strcmp("-c") == 0 && u + 1 < argv().count() ){
        Config::defaultFileName(argv()[u + 1]);
      }
      else if( argv()[u].strcmp("--install") == 0 ){
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
        stdErr.enableDebugLevel(0).enableDebugLevel(1).enableDebugLevel(2).enableDebugLevel(3).
          enableDebugLevel(4).enableDebugLevel(5).enableDebugLevel(6).enableDebugLevel(7).
          enableDebugLevel(8).enableDebugLevel(9);
      }
      else if( argv()[u].strcmp("--start-disp") == 0 ){
        dispatch = true;
      }
      else if( argv()[u].strcmp("--stop-disp") == 0 ){
        services.stopServiceCtrlDispatcher();
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
    }
    if( dispatch ){
      service->msmailConfig()->parse().override();
      stdErr.rotationThreshold(
        service->msmailConfig()->value("debug_file_rotate_threshold",1024 * 1024)
      );
      stdErr.rotatedFileCount(
        service->msmailConfig()->value("debug_file_rotate_count",10)
      );
      stdErr.setDebugLevels(
        service->msmailConfig()->value("debug_levels","+0,+1,+2,+3")
      );
      services.startServiceCtrlDispatcher();
    }
  }
  catch( ExceptionSP & e ){
    e->writeStdError();
    errcode = e->code();
  }
  catch( ... ){
  }
  return errcode;
}
//------------------------------------------------------------------------------

