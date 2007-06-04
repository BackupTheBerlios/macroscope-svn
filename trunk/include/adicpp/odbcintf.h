/*-
 * Copyright 2007 Guram Dukashvili
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
#ifndef _odbcintf_H_
#define _odbcintf_H_
//---------------------------------------------------------------------------
namespace odbcpp {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
extern void initialize();
extern void cleanup();
//---------------------------------------------------------------------------
class API {
  friend void initialize();
  friend void cleanup();
  public:
    SQLRETURN (SQL_API * SQLAllocHandle)(SQLSMALLINT HandleType,SQLHANDLE InputHandle,SQLHANDLE * OutputHandle);	
    SQLRETURN (SQL_API * SQLSetEnvAttr)(SQLHENV EnvironmentHandle,SQLINTEGER Attribute,SQLPOINTER Value,SQLINTEGER StringLength);
    SQLRETURN (SQL_API * SQLSetConnectAttr)(SQLHDBC ConnectionHandle,SQLINTEGER Attribute, SQLPOINTER Value,SQLINTEGER StringLength);
    SQLRETURN (SQL_API * SQLBrowseConnectW)(SQLHDBC hdbc,SQLWCHAR * szConnStrIn,SQLSMALLINT cbConnStrIn,SQLWCHAR * szConnStrOut,SQLSMALLINT cbConnStrOutMax,SQLSMALLINT * pcbConnStrOut);
    SQLRETURN (SQL_API * SQLDisconnect)(SQLHDBC ConnectionHandle);
    SQLRETURN (SQL_API * SQLFreeHandle)(SQLSMALLINT HandleType,SQLHANDLE Handle);
    SQLRETURN (SQL_API * SQLBindParameter)(
      SQLHSTMT hstmt,
      SQLUSMALLINT ipar,
      SQLSMALLINT fParamType,
      SQLSMALLINT fCType,
      SQLSMALLINT fSqlType,
      SQLULEN cbColDef,
      SQLSMALLINT ibScale,
      SQLPOINTER rgbValue,
      SQLLEN cbValueMax,
      SQLLEN * pcbValue
    );
    SQLRETURN (SQL_API * SQLDescribeParam)(
      SQLHSTMT hstmt,
      SQLUSMALLINT ipar,
      SQLSMALLINT * pfSqlType,
      SQLULEN * pcbParamDef,
      SQLSMALLINT * pibScale,
      SQLSMALLINT * pfNullable
    );
    SQLRETURN (SQL_API * SQLNumParams)(SQLHSTMT hstmt,SQLSMALLINT * pcpar);
    SQLRETURN (SQL_API * SQLEndTran)(SQLSMALLINT HandleType,SQLHANDLE Handle,SQLSMALLINT CompletionType);
    SQLRETURN (SQL_API * SQLExecDirectW)(SQLHSTMT StatementHandle,SQLWCHAR * StatementText,SQLINTEGER TextLength);
    SQLRETURN (SQL_API * SQLExecute)(SQLHSTMT StatementHandle);
    SQLRETURN (SQL_API * SQLFetch)(SQLHSTMT StatementHandle);
    SQLRETURN (SQL_API * SQLGetDiagRecW)(SQLSMALLINT HandleType,SQLHANDLE Handle,SQLSMALLINT RecNumber,SQLWCHAR * Sqlstate,SQLINTEGER * NativeError,SQLWCHAR * MessageText,SQLSMALLINT BufferLength,SQLSMALLINT * TextLength);
    SQLRETURN (SQL_API * SQLPrepareW)(SQLHSTMT StatementHandle,SQLWCHAR * StatementText,SQLINTEGER TextLength);

    void open();
    void close();
    utf8::String clientLibrary();
    void clientLibrary(const utf8::String & lib);
  protected:
    uint8_t mutex_[sizeof(ksys::InterlockedMutex)];
    ksys::InterlockedMutex & mutex();
    utf8::String clientLibraryNL();
    uint8_t clientLibrary_[sizeof(utf8::String)];
    intptr_t count_;
#if defined(__WIN32__) || defined(__WIN64__)
    HINSTANCE handle_;
#else
    void * handle_;
#endif
    utf8::String tryOpen();
    static const char * const symbols_[];
  private:
    void initialize();
    void cleanup();
};
//---------------------------------------------------------------------------
extern API api;
//---------------------------------------------------------------------------
inline ksys::InterlockedMutex & API::mutex()
{
  return *reinterpret_cast<ksys::InterlockedMutex *>(mutex_);
}
//---------------------------------------------------------------------------
inline utf8::String API::clientLibrary()
{
  ksys::AutoLock<ksys::InterlockedMutex> lock(mutex());
  return *reinterpret_cast<utf8::String *>(clientLibrary_);
}
//---------------------------------------------------------------------------
inline utf8::String API::clientLibraryNL()
{
  return *reinterpret_cast<utf8::String *>(clientLibrary_);
}
//---------------------------------------------------------------------------
inline void API::clientLibrary(const utf8::String & lib)
{
  ksys::AutoLock<ksys::InterlockedMutex> lock(mutex());
  *reinterpret_cast<utf8::String *>(clientLibrary_) = lib;
}
//---------------------------------------------------------------------------
} // namespace odbcpp
//---------------------------------------------------------------------------
#endif /* _odbcintf_H_ */
//---------------------------------------------------------------------------
