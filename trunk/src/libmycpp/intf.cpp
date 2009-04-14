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
#include <adicpp/mycpp.h>
//---------------------------------------------------------------------------
using namespace ksys;
using namespace utf8;
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
  "mysql_library_init",
  "mysql_library_end",
  "mysql_thread_init",
  "mysql_thread_end",
  "my_thread_init",
  "my_thread_end",
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
  "mysql_options",
  "mysql_ping"
};
#endif
//---------------------------------------------------------------------------
void API::initialize()
{
  new (mutex_) WriteLock;
  new (clientLibrary_) utf8::String;
  new (threadCount_) ThreadLocalVariable<intptr_t>;
#if !MYSQL_STATIC_LIBRARY  
  count_ = 0;
#endif
  //Thread::afterExecuteActions().add(Thread::Action((void *) afterThreadExecute,this));
}
//---------------------------------------------------------------------------
void API::cleanup()
{
  //Thread::afterExecuteActions().remove(Thread::afterExecuteActions().search(Thread::Action((void *) afterThreadExecute,this)));
  threadCount().~ThreadLocalVariable<intptr_t>();
  using namespace utf8;
  reinterpret_cast<utf8::String *>(clientLibrary_)->~String();
  mutex().~WriteLock();
}
//---------------------------------------------------------------------------
#if !MYSQL_STATIC_LIBRARY
utf8::String API::tryOpen()
{
#if defined(__WIN32__) || defined(__WIN64__)
  static const char libName[] = "libmysql.dll";
#else
  static const char libName[] = "libmysqlclient_r.so";
#endif
  utf8::String libFileName(clientLibraryNL());
  if( libFileName.isNull() ) libFileName = libName;
  if( handle_ == NULL ){
    try {
#if defined(__WIN32__) || defined(__WIN64__)
      if( isWin9x() ){
        handle_ = LoadLibraryExA(anyPathName2HostPathName(libFileName).getANSIString(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
      }
      else{
        handle_ = LoadLibraryExW(anyPathName2HostPathName(libFileName).getUNICODEString(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
      }
#elif HAVE_DLFCN_H
      handle_ = dlopen(anyPathName2HostPathName(libFileName).getANSIString(),
#ifdef __linux__
        RTLD_GLOBAL | RTLD_NOW
#else
        0
#endif
      );
#endif
    }
    catch( ExceptionSP & ){
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
  AutoLock<WriteLock> lock(mutex());
  if( count_ == 0 ){
#if !MYSQL_STATIC_LIBRARY
    int32_t       err;
    utf8::String  libFileName(tryOpen());
    if( handle_ == NULL ){
#if defined(__WIN32__) || defined(__WIN64__)
      err = GetLastError() + errorOffset;
#elif HAVE_DLFCN_H
      err = errno;
#endif
      stdErr.debug(
        9,
        utf8::String::Stream() << "Load " << libFileName << " failed\n"
      );
      newObjectV1C2<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
    }
    for( uintptr_t i = 0; i < sizeof(symbols_) / sizeof(symbols_[0]); i++ ){
      void * & func = *(void **) (&p_mysql_thread_safe + i);
#if defined(__WIN32__) || defined(__WIN64__)
      func = (void *) GetProcAddress(handle_,symbols_[i]);
      if( func == NULL ){
        if( &func == &p_mysql_library_init ) func = (void *) GetProcAddress(handle_,"mysql_server_init");
        else
	if( &func == &p_mysql_library_end ) func = (void *) GetProcAddress(handle_,"mysql_server_end");
        else
        if( &func == &p_my_thread_init ) continue; //func = p_mysql_thread_init;
        else
        if( &func == &p_my_thread_end ) continue; //func = p_mysql_thread_end;
      }
      if( func == NULL ){
        err = GetLastError() + errorOffset;
        FreeLibrary(handle_);
        handle_ = NULL;
        stdErr.debug(
          9,
          utf8::String::Stream() << "GetProcAddress(\"" << symbols_[i] << "\")\n"
        );
        newObjectV1C2<Exception>(err + errorOffset, __PRETTY_FUNCTION__)->throwSP();
      }
#elif HAVE_DLFCN_H
      func = dlsym(handle_,symbols_[i]);
      if( func == NULL ){
        if( &func == &p_mysql_library_init ) func = dlsym(handle_,"mysql_server_init");
        else
        if( &func == &p_mysql_library_end ) func = dlsym(handle_,"mysql_server_end");
      }
      if( func == NULL ){
        err = errno;
        dlclose(handle_);
        handle_ = NULL;
        stdErr.debug(9,
	        utf8::String::Stream() << "dlsym(\"" << symbols_[i] << "\")\n"
	      );
        newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
      }
#endif
    }
#endif
    char * groups = NULL;
    if( mysql_library_init != NULL && mysql_library_init(0,NULL,&groups) != 0 ){
#if !MYSQL_STATIC_LIBRARY    
#if defined(__WIN32__) || defined(__WIN64__)
      FreeLibrary(handle_);
#elif HAVE_DLFCN_H
      dlclose(handle_);
#endif
      handle_ = NULL;
#endif
      stdErr.debug(9,
        utf8::String::Stream() << "mysql_library_init couldn't initialize environment\n"
      );
      newObjectV1C2<Exception>(EINVAL, "mysql_library_init couldn't initialize environment")->throwSP();
    }
  }
  if( (intptr_t) threadCount() == 0 ){
    //if( count_ > 0 ){
      my_bool r = my_thread_init != NULL ? my_thread_init() : mysql_thread_init();
      if( r != 0 ){
#if !MYSQL_STATIC_LIBRARY
        if( count_ == 0 ){
          if( mysql_library_end != NULL ) mysql_library_end();
#if defined(__WIN32__) || defined(__WIN64__)
          FreeLibrary(handle_);
#elif HAVE_DLFCN_H
          dlclose(handle_);
#endif
          handle_ = NULL;
       }
#endif
        stdErr.debug(9,
          utf8::String::Stream() << "mysql_thread_init couldn't initialize environment\n"
        );
        newObjectV1C2<Exception>(EINVAL, "mysql_thread_init couldn't initialize environment")->throwSP();
      }
    //}
  }  
  threadCount() = (intptr_t) threadCount() + 1;
  count_++;
}
//---------------------------------------------------------------------------
void API::close()
{
  AutoLock<WriteLock> lock(mutex());
  assert( count_ > 0 );
  assert( (intptr_t) threadCount() > 0 );
  if( (intptr_t) threadCount() == 1 ){
    if( my_thread_init != NULL ) my_thread_end(); else mysql_thread_end();
  }
  threadCount() = (intptr_t) threadCount() - 1;
  if( count_ == 1 ){
#if !MYSQL_STATIC_LIBRARY    
    if( mysql_library_end != NULL ) mysql_library_end();
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
  //assert( (intptr_t) papi->threadCount() == 0 );
}
//---------------------------------------------------------------------------
} // namespace mycpp
//---------------------------------------------------------------------------
