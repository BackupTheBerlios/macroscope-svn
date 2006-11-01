/*-
 * Copyright 2006 Guram Dukashvili
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
#include "msupdater.h"
//------------------------------------------------------------------------------
#define _VERSION_C_AS_HEADER_
#include "version.c"
#undef _VERSION_C_AS_HEADER_
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
MSUpdateSetuper::~MSUpdateSetuper()
{
}
//------------------------------------------------------------------------------
MSUpdateSetuper::MSUpdateSetuper(const ConfigSP & config) : config_(config)
{
}
//------------------------------------------------------------------------------
void MSUpdateSetuper::fiberExecute()
{
  config_->parse().override();
  utf8::String localPath(includeTrailingPathDelimiter(
    config_->valueByPath("update.storage",getExecutablePath() + "updates")
  ));
  Config updatesSetup;
  updatesSetup.codePage(CP_UTF8);
  updatesSetup.silent(true).fileName(localPath + "setup.conf").parse();
  for( intptr_t i = updatesSetup.sectionCount() - 1; i >= 0; i-- ){
    if( (bool) updatesSetup.section(i).value("installed",false) ) continue;
    Config updateSetup;
    updateSetup.codePage(CP_UTF8);
    utf8::String updateLocalPath(includeTrailingPathDelimiter(
      localPath + updatesSetup.section(i).name()
    ));
    updateSetup.silent(true).fileName(updateLocalPath + "setup.conf").parse();
    for( intptr_t j = updateSetup.sectionCount() - 1; j >= 0; j-- ){
      if( updateSetup.section(j).name().strncasecmp("file",4) != 0 ) continue;
      utf8::String fileName(updateSetup.section(j).value(""));
      if( updateSetup.section(j).isSection("before") ){ // run before actions
      }
      if( updateSetup.section(j).isValue("install") ){
      }
      if( updateSetup.section(j).isValue("remove") ){
      }
      if( updateSetup.section(j).isSection("after") ){ // run after actions
      }
    }
  }
  updatesSetup.save();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
MSUpdateFetcher::~MSUpdateFetcher()
{
}
//------------------------------------------------------------------------------
MSUpdateFetcher::MSUpdateFetcher(const ConfigSP & config) : config_(config)
{
}
//------------------------------------------------------------------------------
void MSUpdateFetcher::fiberExecute()
{
  Fetcher fetch;
  uint64_t lastCheckUpdate = 0;
  while( !terminated_ ){
    config_->parse().override();
    uint64_t interval = config_->valueByPath("update.interval",180);
    interval *= 60000000u;
    if( getlocaltimeofday() - lastCheckUpdate >= interval ){
      fetch.url(config_->valueByPath("update.url"));
      fetch.proxy(config_->valueByPath("update.proxy"));
      fetch.resume(config_->valueByPath("update.resume",true));
      fetch.localPath(includeTrailingPathDelimiter(
        config_->valueByPath("update.storage",getExecutablePath() + "updates")
      ));
      stdErr.debug(3,utf8::String::Stream() << this <<
        "Check for new updates on " << fetch.url() << " ...\n"
      );
      try {
        Config updatesSetup;
        updatesSetup.codePage(CP_UTF8);
        updatesSetup.silent(true).fileName(fetch.localPath() + "setup.conf").parse();

        fetch.fetch("updates.lst");
        Array<utf8::String> updateURLs;
        if( fetch.fetched() || fetch.resumed() || fetch.modified() ){
          AsyncFile updateList(fetch.localPath() + "updates.lst");
          AsyncFile::LineGetBuffer buffer;
          buffer.codePage_ = CP_UTF8;
          buffer.removeNewLine_ = true;
          utf8::String s;
          while( !updateList.gets(s,&buffer) ) updateURLs.add(s);
        }
        if( (!fetch.fetched() && !fetch.resumed() && !fetch.modified()) || updateURLs.count() == 0 ){
          stdErr.debug(3,utf8::String::Stream() << this <<
            "New updates on " << fetch.url() << " semifresh.\n"
          );
        }
        else {
          for( intptr_t i = updateURLs.count() - 1; i >= 0; i-- ){
            uint64_t urlHash = updateURLs[i].hash_ll(true);
            utf8::String sectionName(base32Encode(&urlHash,sizeof(urlHash)));
            bool fetched = false;
            if( updatesSetup.isSection(sectionName) ){
              fetched = updatesSetup.section(sectionName).value("fetched",false);
            }
            if( !fetched ){
              fetch.url(updateURLs[i]);
              try {
                fetch.fetch();
                fetched = true;
              }
              catch( ExceptionSP & e ){
                e->writeStdError();
                stdErr.debug(4,utf8::String::Stream() << this <<
                  "Fetch for update on " << fetch.url() << " failed.\n"
                );
              }
            }
            updatesSetup.section(sectionName).setValue("fetched",fetched);
            updatesSetup.section(sectionName).setValue("url",fetch.url());
            bool extracted = updatesSetup.section(sectionName).value("extracted",false);
            if( fetched && !extracted ){
              Archive ar;
              try {
                ar.fileName(fetch.localPathName());
                ar.unpack(fetch.localPath() + sectionName);
                extracted = true;
              }
              catch( ExceptionSP & e ){
                e->writeStdError();
                stdErr.debug(5,utf8::String::Stream() << this <<
                  "Extracting update " << fetch.url() << " failed.\n"
                );
              }
            }
            updatesSetup.section(sectionName).setValue("extracted",extracted);
          }
          updatesSetup.save();
        }
      }
      catch( ExceptionSP & e ){
        e->writeStdError();
        stdErr.debug(3,utf8::String::Stream() << this <<
          "Check for new updates on " << fetch.url() << " failed.\n"
        );
      }
      lastCheckUpdate = getlocaltimeofday();
      thread()->server()->attachFiber(newObject<MSUpdateSetuper>(config_));
    }
    sleep(interval);
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
MSUpdaterService::MSUpdaterService() :
  config_(newObject<InterlockedConfig<FiberInterlockedMutex> >())
{
  serviceName_ = "msupdater";
  displayName_ = "Macroscope Update Service";
#if defined(__WIN32__) || defined(__WIN64__)
  serviceType_ = SERVICE_WIN32_OWN_PROCESS;
  startType_ = SERVICE_AUTO_START;
  errorControl_ = SERVICE_ERROR_IGNORE;
  binaryPathName_ = getExecutableName();
  serviceStatus_.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  serviceStatus_.dwWaitHint = 60000; // give me 60 seconds for start or stop 
#endif
}
//------------------------------------------------------------------------------
void MSUpdaterService::start()
{
  attachFiber(newObject<MSUpdateFetcher>(config_));
}
//------------------------------------------------------------------------------
void MSUpdaterService::stop()
{
  closeServer();
  stdErr.log(
    lmINFO,
    utf8::String::Stream() << msupdater_version.gnu_ << " stopped\n"
  );
}
//------------------------------------------------------------------------------
bool MSUpdaterService::active()
{
  return active();
}
//------------------------------------------------------------------------------
void MSUpdaterService::genUpdatePackage()
{

}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
int main(int ac,char * av[])
{
  int errcode = 0;
  adicpp::AutoInitializer autoInitializer;
  autoInitializer = autoInitializer;
  try {

    newObject<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();

    union {
      intptr_t i;
      uintptr_t u;
    };
    initializeArguments(ac,av);
    Config::defaultFileName(SYSCONF_DIR + "msupdater.conf");
    Services services(msupdater_version.gnu_);
    services.add(newObject<MSUpdaterService>());
#if defined(__WIN32__) || defined(__WIN64__)
    bool dispatch = true;
#else
    bool dispatch = false;
#endif
    for( u = 1; u < argv().count(); u++ ){
      if( argv()[u].strcmp("--version") == 0 ){
        stdErr.debug(9,utf8::String::Stream() << msupdater_version.tex_ << "\n");
        fprintf(stdout,"%s\n",msupdater_version.tex_);
        dispatch = false;
        continue;
      }
      if( argv()[u].strcmp("-c") == 0 && u + 1 < argv().count() ){
        Config::defaultFileName(argv()[u + 1]);
      }
      else if( argv()[u].strcmp("--log") == 0 && u + 1 < argv().count() ){
        stdErr.fileName(argv()[u + 1]);
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
        utf8::String b64(base64Encode(passwordSHA256.sha256(),passwordSHA256.size()));
        fprintf(stdout,"%s\n",b64.c_str());
        copyStrToClipboard(b64);
        dispatch = false;
      }
      else if( argv()[u].strcmp("--generate-update-package") == 0 && u + 1 < argv().count() ){
        MSUpdaterService::genUpdatePackage();
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
    if( dispatch ){
      bool daemon;
      {
        ConfigSP config(newObject<InterlockedConfig<FiberInterlockedMutex> >());
        daemon = config->value("daemon",false);
        checkMachineBinding(config->value("machine_key"));
      }
      services.startServiceCtrlDispatcher(daemon);
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

