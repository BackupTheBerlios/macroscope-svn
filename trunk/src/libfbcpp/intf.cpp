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
#include <adicpp/fbcpp.h>
//---------------------------------------------------------------------------
#if __BCPLUSPLUS__
using namespace ksys;
using namespace utf8;
#endif
//---------------------------------------------------------------------------
namespace fbcpp {
//---------------------------------------------------------------------------
API api;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if !FIREBIRD_STATIC_LIBRARY
const char * const  API::symbols_[] = {
  "isc_attach_database",
  "isc_detach_database",
  "isc_drop_database",
  "isc_start_multiple",
  "isc_rollback_retaining",
  "isc_rollback_transaction",
  "isc_commit_retaining",
  "isc_commit_transaction",
  "isc_prepare_transaction",
  "isc_event_counts",
  "isc_que_events",
  "isc_event_block",
  "isc_cancel_events",
  "isc_vax_integer",
  "isc_interprete",
  "isc_dsql_execute_immediate",
  "isc_dsql_allocate_statement",
  "isc_dsql_free_statement",
  "isc_dsql_describe",
  "isc_dsql_describe_bind",
  "isc_dsql_prepare",
  "isc_dsql_execute",
  "isc_dsql_set_cursor_name",
  "isc_dsql_fetch",
  "isc_dsql_sql_info",
  "isc_array_lookup_bounds",
  "isc_blob_lookup_desc",
  "isc_array_put_slice",
  "isc_array_get_slice",
  "isc_create_blob2",
  "isc_open_blob2",
  "isc_close_blob",
  "isc_cancel_blob",
  "isc_put_segment",
  "isc_get_segment",
  "isc_service_attach",
  "isc_service_detach",
  "isc_service_query",
  "isc_service_start",
  "isc_encode_timestamp",
  "isc_decode_timestamp",
  "fb_interpret"
};
#endif
//---------------------------------------------------------------------------
void API::initialize()
{
#if !FIREBIRD_STATIC_LIBRARY
  new (mutex_) ksys::InterlockedMutex;
  count_ = 0;
#endif
}
//---------------------------------------------------------------------------
void API::cleanup()
{
#if !FIREBIRD_STATIC_LIBRARY
  mutex().~InterlockedMutex();
#endif
}
//---------------------------------------------------------------------------
#if !FIREBIRD_STATIC_LIBRARY
utf8::String API::tryOpen()
{
  utf8::String  libFileName;
  if( handle_ == NULL ){
    try {
      ksys::Config config;
      config.silent(true).parse();
      static const char libKeyPath[] = "libadicpp.firebird.client_library";
#if defined(__WIN32__) || defined(__WIN64__)
      static const char libName[] = "fbclient.dll";
#else
      static const char libName[] = "libfbclient.so";
#endif
      libFileName = config.valueByPath(libKeyPath,libName);
#if defined(__WIN32__) || defined(__WIN64__)
      if( ksys::isWin9x() ){
        handle_ = LoadLibraryExA(ksys::anyPathName2HostPathName(libFileName).getANSIString(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
      }
      else{
        handle_ = LoadLibraryExW(ksys::anyPathName2HostPathName(libFileName).getUNICODEString(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
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
#if !FIREBIRD_STATIC_LIBRARY
  int32_t err;
  ksys::AutoLock<ksys::InterlockedMutex> lock(mutex());
  if( count_ == 0 ){
    utf8::String libFileName(tryOpen());
    if( handle_ == NULL ){
#if defined(__WIN32__) || defined(__WIN64__)
      err = GetLastError() + ksys::errorOffset;
#elif HAVE_DLFCN_H
      err = errno;
#endif
      ksys::stdErr.debug(
        9,
        utf8::String::Stream() << "Load " << libFileName << " failed\n"
      );
      newObjectV1C2<ksys::Exception>(err, __PRETTY_FUNCTION__)->throwSP();
    }
    for( uintptr_t i = 0; i < sizeof(symbols_) / sizeof(symbols_[0]); i++ ){
#if defined(__WIN32__) || defined(__WIN64__)
      (&p_isc_attach_database)[i] = GetProcAddress(handle_,symbols_[i]);
      if( strcmp(symbols_[i],"fb_interpret") != 0 ){
        if( (&p_isc_attach_database)[i] == NULL ){
          err = GetLastError() + ksys::errorOffset;
          FreeLibrary(handle_);
          handle_ = NULL;
          ksys::stdErr.debug(
            9,
            utf8::String::Stream() << "GetProcAddress(\"" << symbols_[i] << "\")\n"
          );
          newObjectV1C2<ksys::Exception>(err + ksys::errorOffset, __PRETTY_FUNCTION__)->throwSP();
        }
      }
#elif HAVE_DLFCN_H
      (&p_isc_attach_database)[i] = dlsym(handle_, symbols_[i]);
      if( strcmp(symbols_[i],"fb_interpret") != 0 ){
        if( (&p_isc_attach_database)[i] == NULL ){
          err = errno;
          dlclose(handle_);
          handle_ = NULL;
          ksys::stdErr.debug(9,
            utf8::String::Stream() << "dlsym(\"" << symbols_[i] << "\")\n"
          );
          newObjectV1C2<ksys::Exception>(err, __PRETTY_FUNCTION__)->throwSP();
        }
      }
#endif
    }
  }
  count_++;
#endif
}
//---------------------------------------------------------------------------
void API::close()
{
#if !FIREBIRD_STATIC_LIBRARY
  assert( count_ > 0 );
  ksys::AutoLock<ksys::InterlockedMutex> lock  (mutex());
  if( count_ == 1 ){
#if defined(__WIN32__) || defined(__WIN64__)
    FreeLibrary(handle_);
#elif HAVE_DLFCN_H
    dlclose(handle_);
#endif
    handle_ = NULL;
  }
  count_--;
#endif
}
//---------------------------------------------------------------------------
bool findISCCode(const ISC_STATUS * pStatus, ISC_LONG code)
{
  while( *pStatus != 0 ){
    if( pStatus[0] == 1 && pStatus[1] == code )
      return true;
    pStatus += 2;
  }
  return false;
}
//---------------------------------------------------------------------------
bool iscIsFatalError(const ISC_STATUS_ARRAY status)
{
  const ISC_STATUS *  pStatus = status;
  while( *pStatus != 0 ){
    if( *pStatus == 1 ){
      switch( pStatus[1] ){
        case isc_network_error          :
          //          case isc_net_connect_err        :
          //          case isc_net_connect_listen_err :
          //          case isc_net_event_connect_err  :
          //          case isc_net_event_listen_err   :
        case isc_net_read_err           :
        case isc_net_write_err          :
        case isc_lost_db_connection     :
          return true;
      }
    }
    pStatus += 2;
  }
  return false;
}
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
