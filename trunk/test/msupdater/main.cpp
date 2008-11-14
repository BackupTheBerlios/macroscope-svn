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
MSUpdateSetuper::MSUpdateSetuper(MSUpdateFetcher & fetcher) : fetcher_(&fetcher)
{
}
//------------------------------------------------------------------------------
void MSUpdateSetuper::executeAction(const utf8::String & name,const ConfigSection & section)
{
  int32_t result = 0;
  utf8::String image(section.value());
  utf8::String args(section.value("args"));
  if( !name.isNull() ){
    result = execute(image,args,NULL,true);
    utf8::String::Stream s;
    s << name <<
      " before action: '" << image << " " << args << "'";
    if( result == 0 ){
      s << " executed successfuly\n";
    }
    else {
      utf8::String iec(section.value("ignore_exit_code"));
      intptr_t i;
      for( i = enumStringParts(iec) - 1; i >= 0; i-- )
        if( (int32_t) Mutant(stringPartByNo(iec,i)) == result ) break;
      s << " failed with code " << result;
      if( i >= 0 ){
        s << ", but error will be ignored\n";
        result = 0;
      }
    }
    stdErr.debug(5,s);
  }
  if( result != 0 ) newObjectV1C2<Exception>(result,__PRETTY_FUNCTION__)->throwSP();
}
//------------------------------------------------------------------------------
void MSUpdateSetuper::fiberExecute()
{
  try {
    fetcher_->updater_->config_->parse().override();
    utf8::String localPath(includeTrailingPathDelimiter(
      fetcher_->updater_->config_->valueByPath("update.storage",getExecutablePath() + "updates")
    ));
    Config updatesSetup;
    updatesSetup.codePage(CP_UTF8);
    updatesSetup.silent(true).fileName(localPath + "setup.conf").parse();
    for( uintptr_t i = 0; i < updatesSetup.sectionCount(); i++ ){
      if( (bool) updatesSetup.section(i).value("installed",false) ) continue;
      stdErr.debug(4,utf8::String::Stream() <<
        "Try to install update: " << updatesSetup.section(i).name() << "\n"
      );
      Config updateSetup;
      updateSetup.codePage(CP_UTF8);
      utf8::String updateLocalPath(includeTrailingPathDelimiter(
        localPath + updatesSetup.section(i).name()
      ));
      updateSetup.silent(true).fileName(updateLocalPath + "setup.conf").parse();
      for( uintptr_t j = 0; j < updateSetup.sectionCount(); j++ ){
        if( updateSetup.section(j).name().ncasecompare("file",4) != 0 ) continue;
        if( (bool) updateSetup.section(j).value("installed",false) ) continue;
        utf8::String fileName(updateSetup.section(j).value(""));
        if( updateSetup.section(j).isSection("before") ){ // run before actions
          if( updateSetup.section(j).section("before").isSection("execute") )
            executeAction(
              updateSetup.section(j).name(),
              updateSetup.section(j).section("before").section("execute")
            );
        }
        if( updateSetup.section(j).isSection("install") ){
          if( updateSetup.section(j).section("install").isSection("copy") ){
            const ConfigSection & section = updateSetup.section(j).section("install").section("copy");
            try {
              copy(section.value("destination"),section.value("source"));
            }
            catch( ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
              if( e->code() != ERROR_ACCESS_DENIED + errorOffset &&
                  e->code() != ERROR_SHARING_VIOLATION  + errorOffset &&
                  e->code() != ERROR_LOCK_VIOLATION  + errorOffset ) throw;
#endif
            }
          }
        }
        if( updateSetup.section(j).isSection("remove") ){
          remove(updateSetup.section(j).section("remove").value());
        }
        if( updateSetup.section(j).isSection("after") ){ // run after actions
          if( updateSetup.section(j).section("after").isSection("execute") )
            executeAction(
              updateSetup.section(j).name(),
              updateSetup.section(j).section("after").section("execute")
            );
        }
        updateSetup.section(j).setValue("installed",true);
        updateSetup.save();
      }
      updatesSetup.section(i).setValue("installed",true);
    }
    updatesSetup.save();
  }
  catch( ... ){
    fetcher_->setupSem_.post();
    throw;
  }
  fetcher_->setupSem_.post();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
MSUpdateFetcher::~MSUpdateFetcher()
{
}
//------------------------------------------------------------------------------
MSUpdateFetcher::MSUpdateFetcher(MSUpdaterService & updater) : updater_(&updater)
{
}
//------------------------------------------------------------------------------
void MSUpdateFetcher::fiberExecute()
{
  setupSem_.post();
  Fetcher fetch;
  uint64_t lastCheckUpdate = 0;
  while( !terminated_ ){
    updater_->config_->parse().override();
    uint64_t interval = updater_->config_->valueByPath("update.interval",180);
    interval *= 60000000u;
    bool setupEnded = setupSem_.tryWait();
    if( getlocaltimeofday() - lastCheckUpdate >= interval && setupEnded ){
      fetch.url(updater_->config_->valueByPath("update.url"));
      fetch.proxy(updater_->config_->valueByPath("update.proxy"));
      fetch.resume(updater_->config_->valueByPath("update.resume",true));
      fetch.localPath(includeTrailingPathDelimiter(
        updater_->config_->valueByPath("update.storage",getExecutablePath() + "updates")
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
          AsyncFile::LineGetBuffer buffer(updateList);
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
      thread()->server()->attachFiber(newObjectR1<MSUpdateSetuper>(*this));
    }
    if( setupEnded ) ksleep(interval); else ksleep(1000000u);
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
MSUpdaterService::MSUpdaterService() :
  config_(newObject<InterlockedConfig<FiberWriteLock> >())
{
  serviceName_ = "msupdater";
  displayName_ = "Macroscope Update Service";
#if defined(__WIN32__) || defined(__WIN64__)
  serviceType_ = SERVICE_WIN32_OWN_PROCESS;
  startType_ = SERVICE_AUTO_START;
  errorControl_ = SERVICE_ERROR_IGNORE;
  binaryPathName_ = getExecutableName();
  serviceStatus_.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
  serviceStatus_.dwWaitHint = 60000; // give me 60 seconds for start or stop 
#endif
}
//------------------------------------------------------------------------------
void MSUpdaterService::start()
{
  attachFiber(newObjectR1<MSUpdateFetcher>(*this));
  stdErr.debug(0,utf8::String::Stream() << msupdater_version.gnu_ << " started\n");
}
//------------------------------------------------------------------------------
void MSUpdaterService::stop()
{
  closeServer();
  stdErr.debug(0,utf8::String::Stream() << msupdater_version.gnu_ << " stopped\n");
}
//------------------------------------------------------------------------------
bool MSUpdaterService::active()
{
  return active();
}
//------------------------------------------------------------------------------
void MSUpdaterService::genUpdatePackage(const utf8::String & setupConfigFile)
{
  Config updatesSetup;
  updatesSetup.codePage(CP_UTF8);
  updatesSetup.silent(true).fileName(setupConfigFile).parse();
  Vector<utf8::String> fileList;
  for( uintptr_t i = 0; i < updatesSetup.sectionCount(); i++ ){
    if( updatesSetup.section(i).name().ncasecompare("file",4) != 0 ) continue;
    fileList.add(updatesSetup.section(i).value("file_name"));
  }
  fileList.add(updatesSetup.fileName());
  Archive ar;
  ar.fileName(updatesSetup.value("file_name"));
  ar.pack(fileList);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
int main(int ac,char * av[])
{
  int errcode = 0;
  adicpp::AutoInitializer autoInitializer(ac,av);
  autoInitializer = autoInitializer;
  try {
    uintptr_t u;
    stdErr.fileName(SYSLOG_DIR("msupdater/") + "msupdater.log");
    Config::defaultFileName(SYSCONF_DIR("") + "msupdater.conf");
    Services services(msupdater_version.gnu_);
    services.add(newObject<MSUpdaterService>());
#if defined(__WIN32__) || defined(__WIN64__)
    bool dispatch = true;
#else
    bool dispatch = false;
#endif
    for( u = 1; u < argv().count(); u++ ){
      if( argv()[u].compare("--version") == 0 ){
        stdErr.debug(9,utf8::String::Stream() << msupdater_version.tex_ << "\n");
        fprintf(stdout,"%s\n",msupdater_version.tex_);
        dispatch = false;
        continue;
      }
      if( argv()[u].compare("-c") == 0 && u + 1 < argv().count() ){
        Config::defaultFileName(argv()[u + 1]);
      }
      else if( argv()[u].compare("--log") == 0 && u + 1 < argv().count() ){
        stdErr.fileName(argv()[u + 1]);
      }
      else if( argv()[u].compare("--install") == 0 ){
        services.install();
        dispatch = false;
      }
      else if( argv()[u].compare("--uninstall") == 0 ){
        services.uninstall();
        dispatch = false;
      }
      else if( argv()[u].compare("--start") == 0 && u + 1 < argv().count() ){
        services.start(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].compare("--stop") == 0 && u + 1 < argv().count() ){
        services.stop(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].compare("--suspend") == 0 && u + 1 < argv().count() ){
        services.suspend(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].compare("--resume") == 0 && u + 1 < argv().count() ){
        services.resume(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].compare("--query") == 0 && u + 1 < argv().count() ){
        services.query(argv()[u + 1]);
        dispatch = false;
      }
      else if( argv()[u].compare("--start-disp") == 0 ){
        dispatch = true;
      }
      else if( argv()[u].compare("--stop-disp") == 0 ){
        services.stopServiceCtrlDispatcher();
        dispatch = false;
      }
      else if( argv()[u].compare("--sha256") == 0 && u + 1 < argv().count() ){
        SHA256 passwordSHA256;
        passwordSHA256.make(argv()[u + 1].c_str(),argv()[u + 1].size());
        utf8::String b64(base64Encode(passwordSHA256.sha256(),passwordSHA256.size()));
        fprintf(stdout,"%s\n",b64.c_str());
        copyStrToClipboard(b64);
        dispatch = false;
      }
      else if( argv()[u].compare("--generate-update-package") == 0 && u + 1 < argv().count() ){
        MSUpdaterService::genUpdatePackage(argv()[u + 1]);
      }
    }
    if( dispatch ){
      bool daemon;
      {
        ConfigSP config(newObject<InterlockedConfig<FiberWriteLock> >());
        daemon = config->value("daemon",false);
      }
      services.startServiceCtrlDispatcher(daemon);
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

