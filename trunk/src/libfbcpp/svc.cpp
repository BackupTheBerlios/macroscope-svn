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
namespace fbcpp {
//---------------------------------------------------------------------------
#if _MSC_VER
#pragma warning(disable:4305)
#pragma warning(disable:4309)
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
SPB::SPBParam SPB::params[] = {
  {                  "version1",                           isc_spb_version1 },
  {           "current_version",                    isc_spb_current_version },
  {                   "version",                            isc_spb_version },
  {                 "user_name",                          isc_spb_user_name },
  {             "sys_user_name",                      isc_spb_sys_user_name },
  {         "sys_user_name_enc",                  isc_spb_sys_user_name_enc },
  {                  "password",                           isc_spb_password },
  {              "password_enc",                       isc_spb_password_enc },
  {              "command_line",                       isc_spb_command_line },
  {                    "dbname",                             isc_spb_dbname },
  {                   "verbose",                            isc_spb_verbose },
  {                   "options",                            isc_spb_options },
  {           "connect_timeout",                    isc_spb_connect_timeout },
  {     "dummy_packet_interval",              isc_spb_dummy_packet_interval },
  {             "sql_role_name",                      isc_spb_sql_role_name },
  {                "sec_userid",                         isc_spb_sec_userid },
  {               "sec_groupid",                        isc_spb_sec_groupid },
  {              "sec_username",                       isc_spb_sec_username },
  {              "sec_password",                       isc_spb_sec_password },
  {             "sec_groupname",                      isc_spb_sec_groupname },
  {             "sec_firstname",                      isc_spb_sec_firstname },
  {            "sec_middlename",                     isc_spb_sec_middlename },
  {              "sec_lastname",                       isc_spb_sec_lastname },
  {                   "lic_key",                            isc_spb_lic_key },
  {                    "lic_id",                             isc_spb_lic_id },
  {                  "lic_desc",                           isc_spb_lic_desc },
  {                  "bkp_file",                  isc_spb_bkp_file | 0x8000 },
  {                "bkp_factor",                isc_spb_bkp_factor | 0x8000 },
  {                "bkp_length",                isc_spb_bkp_length | 0x8000 },
  {                "bkp_expand",                         isc_spb_bkp_expand },
  {          "prp_page_buffers",          isc_spb_prp_page_buffers | 0x4000 },
  {        "prp_sweep_interval",        isc_spb_prp_sweep_interval | 0x4000 },
  {           "prp_shutdown_db",           isc_spb_prp_shutdown_db | 0x4000 },
  {  "prp_deny_new_attachments",  isc_spb_prp_deny_new_attachments | 0x4000 },
  { "prp_deny_new_transactions", isc_spb_prp_deny_new_transactions | 0x4000 },
  {         "prp_reserve_space",         isc_spb_prp_reserve_space | 0x4000 },
  {            "prp_write_mode",            isc_spb_prp_write_mode | 0x4000 },
  {           "prp_access_mode",           isc_spb_prp_access_mode | 0x4000 },
  {       "prp_set_sql_dialect",       isc_spb_prp_set_sql_dialect | 0x4000 },
  {          "rpr_commit_trans",                   isc_spb_rpr_commit_trans },
  {        "rpr_rollback_trans",                 isc_spb_rpr_rollback_trans },
  {     "rpr_recover_two_phase",              isc_spb_rpr_recover_two_phase },
  {      "rpr_list_limbo_trans",               isc_spb_rpr_list_limbo_trans },
  {                    "tra_id",                             isc_spb_tra_id },
  {             "single_tra_id",                      isc_spb_single_tra_id },
  {              "multi_tra_id",                       isc_spb_multi_tra_id },
  {                 "tra_state",                          isc_spb_tra_state },
  {           "tra_state_limbo",                    isc_spb_tra_state_limbo },
  {          "tra_state_commit",                   isc_spb_tra_state_commit },
  {        "tra_state_rollback",                 isc_spb_tra_state_rollback },
  {         "tra_state_unknown",                  isc_spb_tra_state_unknown },
  {             "tra_host_site",                      isc_spb_tra_host_site },
  {           "tra_remote_site",                    isc_spb_tra_remote_site },
  {               "tra_db_path",                        isc_spb_tra_db_path },
  {                "tra_advise",                         isc_spb_tra_advise },
  {         "tra_advise_commit",                  isc_spb_tra_advise_commit },
  {       "tra_advise_rollback",                isc_spb_tra_advise_rollback },
  {        "tra_advise_unknown",                 isc_spb_tra_advise_unknown },
  {               "res_buffers",               isc_spb_res_buffers | 0x8000 },
  {             "res_page_size",             isc_spb_res_page_size | 0x8000 },
  {                "res_length",                isc_spb_res_length | 0x8000 },
  {           "res_access_mode",           isc_spb_res_access_mode | 0x8000 },
  {           "res_am_readonly",                    isc_spb_res_am_readonly },
  {          "res_am_readwrite",                   isc_spb_res_am_readwrite },
  {                   "num_att",                            isc_spb_num_att },
  {                    "num_db",                             isc_spb_num_db },
  {       "sts_record_versions",                isc_spb_sts_record_versions },
  {         "isc_spb_sts_table",                          isc_spb_sts_table }
};
//---------------------------------------------------------------------------
SPB::SPB() : spb_(NULL), spbLen_(0)
{
  writeChar(isc_spb_version).writeChar(isc_spb_current_version);
}
//---------------------------------------------------------------------------
SPB::~SPB()
{
  ksys::xfree(spb_);
}
//---------------------------------------------------------------------------
SPB & SPB::clear()
{
  ksys::xfree(spb_);
  spb_ = NULL;
  spbLen_ = 0;
  writeChar(isc_spb_version).writeChar(isc_spb_current_version);
  return *this;
}
//---------------------------------------------------------------------------
SPB & SPB::writeChar(uintptr_t code)
{
  ksys::xrealloc(spb_, spbLen_ + 1);
  spb_[spbLen_++] = char(code);
  return *this;
}
//---------------------------------------------------------------------------
SPB & SPB::writeLong(uintptr_t a)
{
  ksys::xrealloc(spb_, spbLen_ + sizeof(ISC_LONG));
  *(ISC_LONG *) (spb_ + spbLen_) = (ISC_LONG) a;
  spbLen_ += sizeof(ISC_LONG);
  return *this;
}
//---------------------------------------------------------------------------
intptr_t SPB::writeISCCode(const utf8::String & name)
{
  intptr_t  i;
  for( i = sizeof(params) / sizeof(params[0]) - 1; i >= 0; i-- ){
    if( name.strcasecmp(params[i].name_) == 0 ){
      ksys::xrealloc(spb_, spbLen_ + 1);
      spb_[spbLen_++] = char(params[i].number & ~0xC000);
      return params[i].number;
    }
  }
  return i;
}
//---------------------------------------------------------------------------
SPB & SPB::writeBuffer(const void * buf, uintptr_t size)
{
  ksys::xrealloc(spb_, spbLen_ + size);
  memcpy(spb_ + spbLen_, buf, size);
  spbLen_ += size;
  return *this;
}
//---------------------------------------------------------------------------
SPB & SPB::add(const utf8::String & name, const ksys::Mutant & value)
{
  utf8::String  sValue;
  switch( writeISCCode(name) ){
    case isc_spb_user_name :
    case isc_spb_sys_user_name :
    case isc_spb_sys_user_name_enc :
    case isc_spb_password :
    case isc_spb_password_enc :
      //    case isc_spb_command_line :
      //    case isc_spb_dbname :
      //    case isc_spb_options :
      //    case isc_spb_connect_timeout :
      //    case isc_spb_dummy_packet_interval :
    case isc_spb_sql_role_name :
      sValue = value;
      writeChar(sValue.size()).writeBuffer(sValue.c_str(), sValue.size());
      break;
      //    case isc_spb_verbose :
      //      break;
  }
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
ServiceRequest::RequestParam  ServiceRequest::params[]  = {
  {         "action_svc_backup",         isc_action_svc_backup },
  {        "action_svc_restore",        isc_action_svc_restore },
  {         "action_svc_repair",         isc_action_svc_repair },
  {       "action_svc_add_user",       isc_action_svc_add_user },
  {    "action_svc_delete_user",    isc_action_svc_delete_user },
  {    "action_svc_modify_user",    isc_action_svc_modify_user },
  {   "action_svc_display_user",   isc_action_svc_display_user },
  {     "action_svc_properties",     isc_action_svc_properties },
  {    "action_svc_add_license",    isc_action_svc_add_license },
  { "action_svc_remove_license", isc_action_svc_remove_license },
  {       "action_svc_db_stats",       isc_action_svc_db_stats },
  {     "action_svc_get_ib_log",     isc_action_svc_get_ib_log }
};
//---------------------------------------------------------------------------
ServiceRequest::ServiceRequest() : request_(NULL), requestLen_(0)
{
}
//---------------------------------------------------------------------------
ServiceRequest::~ServiceRequest()
{
  ksys::xfree(request_);
}
//---------------------------------------------------------------------------
ServiceRequest & ServiceRequest::clear()
{
  ksys::xfree(request_);
  request_ = NULL;
  requestLen_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
ServiceRequest & ServiceRequest::writeChar(uintptr_t code)
{
  ksys::xrealloc(request_, requestLen_ + 1);
  request_[requestLen_++] = char(code);
  return *this;
}
//---------------------------------------------------------------------------
ServiceRequest & ServiceRequest::writeShort(short a)
{
  ksys::xrealloc(request_, requestLen_ + sizeof(short));
  *(short *) (request_ + requestLen_) = a;
  requestLen_ += sizeof(short);
  return *this;
}
//---------------------------------------------------------------------------
ServiceRequest & ServiceRequest::writeLong(uintptr_t a)
{
  ksys::xrealloc(request_, requestLen_ + sizeof(ISC_LONG));
  *(ISC_LONG *) (request_ + requestLen_) = (ISC_LONG) a;
  requestLen_ += sizeof(ISC_LONG);
  return *this;
}
//---------------------------------------------------------------------------
intptr_t ServiceRequest::writeISCCode(const utf8::String & name)
{
  intptr_t  i;
  for( i = sizeof(SPB::params) / sizeof(SPB::params[0]) - 1; i >= 0; i-- ){
    if( name.strcasecmp(SPB::params[i].name_) == 0 ){
      ksys::xrealloc(request_, requestLen_ + 1);
      request_[requestLen_++] = char(SPB::params[i].number & ~0xC000);
      return SPB::params[i].number;
    }
  }
  if( i < 0 ){
    for( i = sizeof(params) / sizeof(params[0]) - 1; i >= 0; i-- ){
      if( name.strcasecmp(params[i].name_) == 0 ){
        ksys::xrealloc(request_, requestLen_ + 1);
        request_[requestLen_++] = params[i].number;
        return params[i].number;
      }
    }
  }
  return i;
}
//---------------------------------------------------------------------------
ServiceRequest & ServiceRequest::writeBuffer(const void * buf, uintptr_t size)
{
  ksys::xrealloc(request_, requestLen_ + size);
  memcpy(request_ + requestLen_, buf, size);
  requestLen_ += size;
  return *this;
}
//---------------------------------------------------------------------------
ServiceRequest & ServiceRequest::add(const utf8::String & name, const ksys::Mutant & value)
{
  intptr_t      a;
  utf8::String  sValue;
  switch( (a = writeISCCode(name)) ){
    case isc_action_svc_backup                      :
    case isc_action_svc_restore                     :
    case isc_action_svc_repair                      :
    case isc_action_svc_add_user                    :
    case isc_action_svc_delete_user                 :
    case isc_action_svc_modify_user                 :
    case isc_action_svc_display_user                :
    case isc_action_svc_properties                  :
    case isc_action_svc_add_license                 :
    case isc_action_svc_remove_license              :
    case isc_action_svc_db_stats                    :
    case isc_action_svc_get_ib_log                  :
      requestAction_ = a;
      break;
    case isc_spb_verbose                            :
    case isc_spb_rpr_commit_trans                   :
    case isc_spb_rpr_rollback_trans                 :
    case isc_spb_rpr_recover_two_phase              :
      break;
    case isc_spb_dbname                             :
    case isc_spb_bkp_file | 0x8000                  :
    case isc_spb_bkp_length | 0x8000                :
      sValue = value;
      writeShort((short) sValue.size()).writeBuffer(sValue.c_str(),sValue.size());
      break;
    case isc_spb_bkp_factor | 0x8000                :
    case isc_spb_options                            :
    case isc_spb_res_length | 0x8000                :
    case isc_spb_res_buffers | 0x8000               :
    case isc_spb_res_page_size | 0x8000             :
    case isc_spb_prp_page_buffers | 0x4000          :
    case isc_spb_prp_sweep_interval | 0x4000        :
    case isc_spb_prp_shutdown_db | 0x4000           :
    case isc_spb_prp_deny_new_transactions | 0x4000 :
    case isc_spb_prp_deny_new_attachments  | 0x4000 :
    case isc_spb_prp_set_sql_dialect | 0x4000       :
    case isc_spb_tra_id                             :
      writeLong(value);
      break;
    case isc_spb_res_access_mode | 0x8000           :
    case isc_spb_prp_reserve_space | 0x4000         :
    case isc_spb_prp_write_mode | 0x4000            :
    case isc_spb_prp_access_mode | 0x4000           :
      writeChar(value);
      break;
  }
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
Service::Service() : handle_(0), queryResponse_(NULL), queryResponseLen_(0)
{
}
//---------------------------------------------------------------------------
Service::~Service()
{
  detach();
  ksys::xfree(queryResponse_);
}
//---------------------------------------------------------------------------
Service & Service::attach(const utf8::String & name)
{
  if( !attached() ){
    api.open();
    ISC_STATUS_ARRAY  status;
    if( api.isc_service_attach(status, 0, (char *) name.c_str(), &handle_, (short) spb_.spbLen_, spb_.spb_) != 0 ){
      ksys::AutoPtr<ksys::Exception> e(
        newObjectV1C2<EServiceAttach>(status, __PRETTY_FUNCTION__)
      );
      api.close();
      exceptionHandler(e.ptr(NULL));
    }
    name_ = name;
  }
  return *this;
}
//---------------------------------------------------------------------------
Service & Service::detach()
{
  if( attached() ){
    ISC_STATUS_ARRAY  status;
    if( api.isc_service_detach(status, &handle_) != 0 && !iscIsFatalError(status) )
      exceptionHandler(newObjectV1C2<EServiceDetach>(status, __PRETTY_FUNCTION__));
    handle_ = 0;
    api.close();
  }
  return *this;
}
//---------------------------------------------------------------------------
Service & Service::invoke()
{
  if( attached() ){
    ISC_STATUS ret;
    ISC_STATUS_ARRAY status;
    switch( request_.requestAction_ ){
      case isc_action_svc_backup         :
      case isc_action_svc_restore        :
      case isc_action_svc_repair         :
      case isc_action_svc_add_user       :
      case isc_action_svc_delete_user    :
      case isc_action_svc_modify_user    :
      case isc_action_svc_display_user   :
      case isc_action_svc_properties     :
      case isc_action_svc_add_license    :
      case isc_action_svc_remove_license :
        for(;;){
          ret = api.isc_service_start(status,&handle_,NULL,(short) request_.requestLen_,request_.request_);
          if( ret == 0 ) break;
          if( status[1] != isc_svc_in_use )
            exceptionHandler(newObjectV1C2<EServiceStart>(status, __PRETTY_FUNCTION__));
          ksys::ksleep1();
        }
        break;
      case isc_action_svc_db_stats       :
      case isc_action_svc_get_ib_log     :
        ksys::xrealloc(queryResponse_, getpagesize());
        queryResponseLen_ = getpagesize();
        for( ; ; ){
          if( api.isc_service_query(status, &handle_, NULL, (short) spb_.spbLen_, spb_.spb_, (short) queryResponseLen_, queryResponse_, (short) request_.requestLen_, request_.request_) != 0 )
            exceptionHandler(newObjectV1C2<EServiceQuery>(status,__PRETTY_FUNCTION__));
          if( *queryResponse_ != isc_info_truncated ) break;
          ksys::xrealloc(queryResponse_, queryResponseLen_ << 1);
          queryResponseLen_ <<= 1;
        }
        break;
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
void Service::staticExceptionHandler(ksys::Exception * e)
{
  EClientServer * p = dynamic_cast<EClientServer *>(e);
  if( p != NULL && p->isFatalError() && attached() ){
    ISC_STATUS_ARRAY status;
    api.isc_service_detach(status,&handle_);
    handle_ = 0;
    api.close();
  }
  e->throwSP();
}
//---------------------------------------------------------------------------
void Service::exceptionHandler(ksys::Exception * e)
{
  staticExceptionHandler(e);
}
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
