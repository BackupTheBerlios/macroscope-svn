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
#if __BCPLUSPLUS__ || _MSC_VER
using namespace ksys;
using namespace utf8;
#endif
//---------------------------------------------------------------------------
namespace mycpp {
//---------------------------------------------------------------------------
API api;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if !MYSQL_STATIC_LIBRARY
const char * const  API::symbols_[] = {
  "mysql_thread_safe",
  "mysql_thread_init",
  "mysql_thread_end",
  "my_init",
  "my_end",
  "mysql_init",
  "mysql_real_connect",
  "mysql_close",
  "mysql_query",
  "mysql_real_query",
  "mysql_commit",
  "mysql_rollback",
  "mysql_autocommit",
  "mysql_errno",
  "mysql_error",
  "mysql_stmt_init",
  "mysql_stmt_close",
  "mysql_stmt_prepare",
  "mysql_stmt_bind_param",
  "mysql_stmt_bind_result",
  "mysql_stmt_param_count",
  "mysql_stmt_field_count",
  "mysql_stmt_execute",
  "mysql_stmt_fetch",
  "mysql_stmt_store_result",
  "mysql_stmt_result_metadata",
  "mysql_stmt_param_metadata",
  "mysql_stmt_free_result",
  "mysql_free_result",
  "mysql_fetch_fields",
  "mysql_insert_id",
  "mysql_options"
};
#endif
//---------------------------------------------------------------------------
void API::initialize()
{
  new (mutex_) ksys::InterlockedMutex;
  new (threadList_) ksys::Array< ThreadList>;
#if !MYSQL_STATIC_LIBRARY  
  count_ = 0;
#endif
  ksys::Thread::afterExecuteActions().add(ksys::Thread::Action((void *) afterThreadExecute, this));
}
//---------------------------------------------------------------------------
void API::cleanup()
{
  ksys::Thread::afterExecuteActions().remove(ksys::Thread::afterExecuteActions().search(ksys::Thread::Action((void *) afterThreadExecute, this)));
  threadList().~Array< ThreadList>();
  mutex().~InterlockedMutex();
}
//---------------------------------------------------------------------------
#if !MYSQL_STATIC_LIBRARY
utf8::String API::tryOpen()
{
  utf8::String  libFileName;
  if( handle_ == NULL ){
    try{
      ksys::Config  config;
      config.parse();
      static const char libKey[] = "libadicpp.mysql.client_library";
#if defined(__WIN32__) || defined(__WIN64__)
      libFileName = config.valueByPath(libKey,"libmysql.dll");
#else
      libFileName = config.valueByPath(libKey,"libmysqlclient_r.so");
#endif
#if defined(__WIN32__) || defined(__WIN64__)
      if( ksys::isWin9x() ){
        handle_ = LoadLibraryExA(anyPathName2HostPathName(libFileName).getANSIString(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
      }
      else{
        handle_ = LoadLibraryExW(anyPathName2HostPathName(libFileName).getUNICODEString(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
      }
#elif HAVE_DLFCN_H
      handle_ = dlopen(ksys::anyPathName2HostPathName(libFileName).getANSIString(),
#ifdef __linux__
        RTLD_GLOBAL | RTLD_NOW
#else
        0
#endif
      );
#endif
    }
    catch( ksys::ExceptionSP & ){
      if( handle_ != NULL ){
#if defined(__WIN32__) || defined(__WIN64__)
        FreeLibrary(handle_);
#elif HAVE_DLFCN_H
        dlclose(handle_);
#endif
        handle_ = NULL;
      }
    }
  }
  return libFileName;
}
#endif
//---------------------------------------------------------------------------
void API::open()
{
  ksys::AutoLock< ksys::InterlockedMutex> lock  (mutex());
  if( count_ == 0 ){
#if !MYSQL_STATIC_LIBRARY
    int32_t       err;
    utf8::String  libFileName (tryOpen());
    if( handle_ == NULL ){
#if defined(__WIN32__) || defined(__WIN64__)
      err = GetLastError() + errorOffset;
#elif HAVE_DLFCN_H
      err = errno;
#endif
      ksys::stdErr.debug(
        9,
        utf8::String::Stream() << "Load " << libFileName << " failed\n"
      );
      newObject<ksys::Exception>(err, __PRETTY_FUNCTION__)->throwSP();
    }
    for( uintptr_t i = 0; i < sizeof(symbols_) / sizeof(symbols_[0]); i++ ){
#if defined(__WIN32__) || defined(__WIN64__)
      (&p_mysql_thread_safe)[i] = GetProcAddress(handle_, symbols_[i]);
      if( (&p_mysql_thread_safe)[i] == NULL ){
        err = GetLastError() + errorOffset;
        FreeLibrary(handle_);
        handle_ = NULL;
        ksys::stdErr.debug(
          9,
          utf8::String::Stream() << "GetProcAddress(\"" << symbols_[i] << "\")\n"
        );
        newObject<ksys::Exception>(err + ksys::errorOffset, __PRETTY_FUNCTION__)->throwSP();
      }
#elif HAVE_DLFCN_H
      (&p_mysql_thread_safe)[i] = dlsym(handle_, symbols_[i]);
      if( (&p_mysql_thread_safe)[i] == NULL ){
        err = errno;
        dlclose(handle_);
        handle_ = NULL;
        ksys::stdErr.debug(9,
	  utf8::String::Stream() << "dlsym(\"" << symbols_[i] << "\")\n"
	);
        newObject<ksys::Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
#endif
    }
#endif
    if( my_init() ){
#if !MYSQL_STATIC_LIBRARY    
#if defined(__WIN32__) || defined(__WIN64__)
      FreeLibrary(handle_);
#elif HAVE_DLFCN_H
      dlclose(handle_);
#endif
      handle_ = NULL;
#endif
      ksys::stdErr.debug(9,
        utf8::String::Stream() << "my_init couldn't initialize environment\n"
      );
      newObject<ksys::Exception>(EINVAL, "my_init couldn't initialize environment")->throwSP();
    }
  }
  count_++;
  intptr_t c, i;
  API::ThreadList tl(gettid());
  i = threadList().bSearch(tl, c);
  if( c != 0 ){
    threadList().insert(i += (c > 0), tl);
    api.mysql_thread_init();
  }
  threadList()[i].count_++;
}
//---------------------------------------------------------------------------
void API::close()
{
  ksys::AutoLock<ksys::InterlockedMutex> lock(mutex());
  API::ThreadList tl(gettid());
  intptr_t i = threadList().bSearch(tl);
  if( i >= 0 && --threadList()[i].count_ == 0 ){
    threadList().remove(i);
    api.mysql_thread_end();
  }
  if( count_ == 1 ){
    my_end(0);
#if !MYSQL_STATIC_LIBRARY    
#if defined(__WIN32__) || defined(__WIN64__)
    FreeLibrary(handle_);
#elif HAVE_DLFCN_H
    dlclose(handle_);
#endif
    handle_ = NULL;
#endif
  }
  count_--;
}
//---------------------------------------------------------------------------
void API::afterThreadExecute(API * /*papi*/)
{
  //  ENTER_MUTEX_SECTION(mutex());
  //  API::ThreadList tl(gettid());
  //  intptr_t i = threadList().bSearch(tl);
  //  if( i >= 0 && threadList()[i].count_ > 0 ){
  //    threadList().remove(i);
  //    api.mysql_thread_end();
  //  }
  //  LEAVE_MUTEX_SECTION;
}
//---------------------------------------------------------------------------
} // namespace mycpp
//---------------------------------------------------------------------------
