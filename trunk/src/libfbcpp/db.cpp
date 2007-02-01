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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DPB::DPBParam DPB::params[] = {
  {             "version1",               isc_dpb_version1 }, 
  {             "pathname",           isc_dpb_cdd_pathname }, 
  {           "allocation",             isc_dpb_allocation }, 
  {              "journal",                isc_dpb_journal }, 
  {            "page_size",              isc_dpb_page_size }, 
  {          "num_buffers",            isc_dpb_num_buffers }, 
  {        "buffer_length",          isc_dpb_buffer_length }, 
  {                "debug",                  isc_dpb_debug }, 
  {      "garbage_collect",        isc_dpb_garbage_collect }, 
  {               "verify",                 isc_dpb_verify }, 
  {                "sweep",                  isc_dpb_sweep }, 
  {       "enable_journal",         isc_dpb_enable_journal }, 
  {      "disable_journal",        isc_dpb_disable_journal }, 
  {          "dbkey_scope",            isc_dpb_dbkey_scope }, 
  {      "number_of_users",        isc_dpb_number_of_users }, 
  {                "trace",                  isc_dpb_trace }, 
  {   "no_garbage_collect",     isc_dpb_no_garbage_collect }, 
  {              "damaged",                isc_dpb_damaged }, 
  {              "license",                isc_dpb_license }, 
  {        "sys_user_name",          isc_dpb_sys_user_name }, 
  {          "encrypt_key",            isc_dpb_encrypt_key }, 
  {      "activate_shadow",        isc_dpb_activate_shadow }, 
  {       "sweep_interval",         isc_dpb_sweep_interval }, 
  {        "delete_shadow",          isc_dpb_delete_shadow }, 
  {          "force_write",            isc_dpb_force_write }, 
  {            "begin_log",              isc_dpb_begin_log }, 
  {             "quit_log",               isc_dpb_quit_log }, 
  {           "no_reserve",             isc_dpb_no_reserve }, 
  {            "user_name",              isc_dpb_user_name }, 
  {             "password",               isc_dpb_password }, 
  {         "password_enc",           isc_dpb_password_enc }, 
  {    "sys_user_name_enc",      isc_dpb_sys_user_name_enc }, 
  {               "interp",                 isc_dpb_interp }, 
  {          "online_dump",            isc_dpb_online_dump }, 
  {        "old_file_size",          isc_dpb_old_file_size }, 
  {        "old_num_files",          isc_dpb_old_num_files }, 
  {             "old_file",               isc_dpb_old_file }, 
  {       "old_start_page",         isc_dpb_old_start_page }, 
  {      "old_start_seqno",        isc_dpb_old_start_seqno }, 
  {       "old_start_file",         isc_dpb_old_start_file }, 
  {         "drop_walfile",           isc_dpb_drop_walfile }, 
  {          "old_dump_id",            isc_dpb_old_dump_id }, 
  {       "wal_backup_dir",         isc_dpb_wal_backup_dir }, 
  {         "wal_chkptlen",           isc_dpb_wal_chkptlen }, 
  {          "wal_numbufs",            isc_dpb_wal_numbufs }, 
  {          "wal_bufsize",            isc_dpb_wal_bufsize }, 
  {     "wal_grp_cmt_wait",       isc_dpb_wal_grp_cmt_wait }, 
  {          "lc_messages",            isc_dpb_lc_messages }, 
  {             "lc_ctype",               isc_dpb_lc_ctype }, 
  {        "cache_manager",          isc_dpb_cache_manager }, 
  {             "shutdown",               isc_dpb_shutdown }, 
  {               "online",                 isc_dpb_online }, 
  {       "shutdown_delay",         isc_dpb_shutdown_delay }, 
  {             "reserved",               isc_dpb_reserved }, 
  {            "overwrite",              isc_dpb_overwrite }, 
  {           "sec_attach",             isc_dpb_sec_attach }, 
  {          "disable_wal",            isc_dpb_disable_wal }, 
  {      "connect_timeout",        isc_dpb_connect_timeout }, 
  { "dummy_packet_interval", isc_dpb_dummy_packet_interval }, 
  {           "gbak_attach",           isc_dpb_gbak_attach }, 
  {         "sql_role_name",         isc_dpb_sql_role_name }, 
  {      "set_page_buffers",      isc_dpb_set_page_buffers }, 
  {     "working_directory",     isc_dpb_working_directory }, 
  {           "sql_dialect",           isc_dpb_sql_dialect }, 
  {       "set_db_readonly",       isc_dpb_set_db_readonly }, 
  {    "set_db_sql_dialect",    isc_dpb_set_db_sql_dialect }, 
  {           "gfix_attach",           isc_dpb_gfix_attach }, 
  {          "gstat_attach",          isc_dpb_gstat_attach }, 
  {        "set_db_charset",        isc_dpb_set_db_charset }, 
  {           "file_length",                           127 }
};
//---------------------------------------------------------------------------
DPB::DPB() : dpb_(NULL)
{
  clear();
}
//---------------------------------------------------------------------------
DPB::~DPB()
{
  ksys::xfree(dpb_);
}
//---------------------------------------------------------------------------
DPB & DPB::clear()
{
  ksys::xfree(dpb_);
  dpb_ = NULL;
  dpbLen_ = 0;
  dialect_ = 3;
  pageSize_ = 0;
  fileLength_ = 0;
  user_.resize(0);
  password_.resize(0);
  role_.resize(0);
  charset_ = "UNICODE_FSS";
  charsetInDPB_ = false;
  writeChar(isc_dpb_version1);
  return *this;
}
//---------------------------------------------------------------------------
DPB & DPB::injectCharset()
{
  if( !charsetInDPB_ ){
    static const char charset[] = "UNICODE_FSS";
    writeISCCode("lc_ctype");
    writeChar(sizeof(charset) - 1).
    writeBuffer(charset, sizeof(charset) - 1).charsetInDPB_ = true;
  }
  return *this;
}
//---------------------------------------------------------------------------
DPB & DPB::writeChar(uintptr_t code)
{
  ksys::xrealloc(dpb_, dpbLen_ + 1);
  dpb_[dpbLen_++] = char(code);
  return *this;
}
//---------------------------------------------------------------------------
DPB & DPB::writeLong(uintptr_t a)
{
  ksys::xrealloc(dpb_, dpbLen_ + sizeof(int));
  *(int *) (dpb_ + dpbLen_) = (int32_t) a;
  dpbLen_ += sizeof(int32_t);
  return *this;
}
//---------------------------------------------------------------------------
intptr_t DPB::writeISCCode(const utf8::String & name)
{
  intptr_t  i;
  for( i = sizeof(params) / sizeof(params[0]) - 1; i >= 0; i-- ){
    if( name.strcasecmp(params[i].name_) == 0 ){
      ksys::xrealloc(dpb_, dpbLen_ + 1);
      dpb_[dpbLen_++] = params[i].number;
      return params[i].number;
    }
  }
  return i;
}
//---------------------------------------------------------------------------
DPB & DPB::writeBuffer(const void * buf, uintptr_t size)
{
  ksys::xrealloc(dpb_, dpbLen_ + size);
  memcpy(dpb_ + dpbLen_, buf, size);
  dpbLen_ += size;
  return *this;
}
//---------------------------------------------------------------------------
DPB & DPB::add(const utf8::String & name, const ksys::Mutant & value)
{
  uintptr_t     sweepInterval;
  utf8::String  sValue;
  switch( writeISCCode(name) ){
    case isc_dpb_sql_dialect     :
      dialect_ = value;
      dpbLen_--;
      break;
    case isc_dpb_page_size       :
      pageSize_ = value;
      dpbLen_--;
      break;
    case 127                     :
      fileLength_ = value;
      dpbLen_--;
      break;
    case isc_dpb_user_name       :
      user_ = value;
      goto l1;
    case isc_dpb_password        :
      password_ = value;
      goto l1;
    case isc_dpb_password_enc    :
    case isc_dpb_sys_user_name   :
    case isc_dpb_license         :
    case isc_dpb_encrypt_key     :
    case isc_dpb_lc_messages     :
      goto l1;
    case isc_dpb_lc_ctype        :
      charset_ = value;
      dpbLen_--;
      break;
    case isc_dpb_sql_role_name   :
      role_ = value;
      if( role_.strlen() == 0 ){
        dpbLen_--;
        break;
      }
l1:   sValue = value;
      writeChar(sValue.size()).writeBuffer(sValue.c_str(),sValue.size());
      break;
    case isc_dpb_num_buffers     :
    case isc_dpb_dbkey_scope     :
    case isc_dpb_force_write     :
    case isc_dpb_no_reserve      :
    case isc_dpb_damaged         :
    case isc_dpb_verify          :
      writeChar(1).writeChar(value);
      break;
    case isc_dpb_sweep           :
      writeChar(1).writeChar(isc_dpb_records);
      break;
    case isc_dpb_sweep_interval  :
      sweepInterval = (uintptr_t) value;
      api.open();
      sweepInterval = api.isc_vax_integer((char *) &sweepInterval, 4);
      api.close();
      writeChar(1).writeChar(sizeof(int32_t)).writeLong(sweepInterval);
      break;
    case isc_dpb_activate_shadow :
    case isc_dpb_delete_shadow   :
    case isc_dpb_begin_log       :
    case isc_dpb_quit_log        :
      writeChar(1).writeChar(0);
      break;
  }
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
Database::Database() : handle_(0)
{
  transactions_.ownsObjects(false);
  dsqlStatements_.ownsObjects(false);
  eventHandlers_.ownsObjects(false);
}
//---------------------------------------------------------------------------
Database::~Database()
{
  detachHelper();
  detach();
}
//---------------------------------------------------------------------------
bool Database::separateDBName(const utf8::String & name, utf8::String & hostName, utf8::String & dbName, uintptr_t & port)
{
  utf8::String::Iterator i1(name);
  while( !i1.eof() && i1.getChar() != ':' )
    i1.next();
  utf8::String::Iterator  i2  (i1 + 1);
  while( !i2.eof() && (i2.getChar() != ':' || (i2 + 1).getChar() == '\\') )
    i2.next();
  port = 0;
  if( i1.eof() && i2.eof() ){
    dbName = name;
  }
  else{
    intmax_t  prt = port;
    if( utf8::tryStr2Int(utf8::String(i1 + 1, i2), prt) && i2.eof() ){
      // dbName and port
      dbName = utf8::String(utf8::String::Iterator(name), i1);
    }
    else{
      utf8::tryStr2Int(utf8::String(i2 + 1, utf8::String::Iterator(name).last()), prt);
      hostName = utf8::String(utf8::String::Iterator(name), i1);
      dbName = utf8::String(i1 + 1, i2);
    }
    port = (uintptr_t) prt;
  }
  return true;
}
//---------------------------------------------------------------------------
Database & Database::create(const utf8::String & name)
{
  utf8::String createSQL("CREATE DATABASE '" + (name.strlen() > 0 ? name : name_) + "' ");
  if( dpb_.user().strlen() == 0 ){
    createSQL += "USER 'SYSDBA' ";
  }
  else {
    createSQL += "USER '" + dpb_.user() + "' ";
  }
  if( dpb_.password().strlen() == 0 ){
    createSQL += "PASSWORD 'masterkey' ";
  }
  else {
    createSQL += "PASSWORD '" + dpb_.password() + "' ";
  }
  if( dpb_.pageSize() <= 0 ){
    createSQL += "PAGE_SIZE=1024 ";
  }
  else {
    createSQL += "PAGE_SIZE=" + utf8::int2Str((intmax_t) dpb_.pageSize()) + " ";
  }
  if( dpb_.fileLength() > 0 ){
    createSQL += "LENGTH=" + utf8::int2Str(dpb_.fileLength() / dpb_.pageSize()) + " ";
  }
  createSQL += "DEFAULT CHARACTER SET " + dpb_.charset_;
  api.open();
  try {
    isc_db_handle newdb = 0;
    isc_tr_handle trans = 0;
    ISC_STATUS_ARRAY status;
    if( api.isc_dsql_execute_immediate(status,&newdb,&trans,0,(char *) createSQL.c_str(),(short) dpb_.dialect(), NULL) != 0 ){
      ksys::AutoPtr<EDBCreate> e(newObjectV1C2<EDBCreate>(status,__PRETTY_FUNCTION__));
      if( !e->searchCode(isc_db_or_file_exists) ) exceptionHandler(e.ptr(NULL));
    }
    else {
      api.isc_detach_database(status, &newdb);
    }
  }
  catch( ksys::ExceptionSP & ){
    api.close();
    throw;
  }
  api.close();
  return *this;
}
//---------------------------------------------------------------------------
Database & Database::detachHelper()
{
  while( eventHandlers_.count() > 0 ){
    eventHandlers_.objectOfIndex(0)->cancel();
    eventHandlers_.removeByIndex(0);
  }
  while( dsqlStatements_.count() > 0 ){
    DSQLStatement * dsqlStatement = dsqlStatements_.objectOfIndex(0);
    dsqlStatement->free();
    dsqlStatement->database_ = NULL;
    dsqlStatements_.removeByIndex(0);
  }
  while( transactions_.count() > 0 ){
    Transaction * transaction = transactions_.objectOfIndex(0);
    while( transaction->active() )
      transaction->rollback();
#if __GNUG__
#else
    transaction->databases_.removeByObject(this);
#endif
    transactions_.removeByIndex(0);
  }
  return *this;
}
//---------------------------------------------------------------------------
Database & Database::drop()
{
  if( !attached() )
    exceptionHandler(newObjectV1C2<EDBNotAttached>((ISC_STATUS *) NULL,__PRETTY_FUNCTION__));
  detach2();
  ISC_STATUS_ARRAY status;
  if( api.isc_drop_database(status,&handle_) != 0 )
    exceptionHandler(newObjectV1C2<EDBDrop>(status, __PRETTY_FUNCTION__));
  api.close();
  return *this;
}
//---------------------------------------------------------------------------
Database & Database::attach(const utf8::String & name)
{
  if( !attached() ){
    api.open();
    try{
      dpb_.injectCharset();
      ISC_STATUS_ARRAY  status;
      if( api.isc_attach_database(status, 0, (char *) (name.strlen() > 0 ? name.c_str() : name_.c_str()), &handle_, (short) dpb_.dpbLen(), dpb_.dpb()) != 0 )
        exceptionHandler(newObjectV1C2<EDBAttach>(status, __PRETTY_FUNCTION__));
    }
    catch( ksys::ExceptionSP & e ){
      if( !e->isFatalError() )
        api.close();
      throw;
    }
    if( name.strlen() > 0 )
      name_ = name;
  }
  return *this;
}
//---------------------------------------------------------------------------
Database & Database::detach2()
{
  if( attached() ){
    intptr_t  i;
    for( i = eventHandlers_.count() - 1; i >= 0; i-- )
      eventHandlers_.objectOfIndex(i)->cancel();
    for( i = dsqlStatements_.count() - 1; i >= 0; i-- )
      dsqlStatements_.objectOfIndex(i)->free();
    for( i = transactions_.count() - 1; i >= 0; i-- ){
      Transaction * transaction = transactions_.objectOfIndex(0);
      while( transaction->active() )
        transaction->rollback();
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
Database & Database::detach()
{
  if( attached() ){
    detach2();
    ISC_STATUS_ARRAY  status;
    if( api.isc_detach_database(status, &handle_) != 0 )
      exceptionHandler(newObjectV1C2<EDBDetach>(status, __PRETTY_FUNCTION__));
    api.close();
  }
  return *this;
}
//---------------------------------------------------------------------------
void Database::processingException(ksys::Exception * e)
{
  // handle fatal errors
  EClientServer * p = dynamic_cast< EClientServer *>(e);
  if( p != NULL ){
    if( p->isFatalError() ){
      ISC_STATUS_ARRAY  status;
      api.isc_detach_database(status, &handle_);
      detach2();
      api.close();
    }
  }
}
//---------------------------------------------------------------------------
void Database::staticExceptionHandler(ksys::Exception * e)
{
  for( intptr_t i = transactions_.count() - 1; i >= 0; i-- )
    transactions_[i]->processingException(e);
  processingException(e);
  e->throwSP();
}
//---------------------------------------------------------------------------
void Database::exceptionHandler(ksys::Exception * e)
{
  staticExceptionHandler(e);
}
//---------------------------------------------------------------------------
Database & Database::name(const utf8::String & name)
{
  detach();
  name_ = name;
  return *this;
}
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
