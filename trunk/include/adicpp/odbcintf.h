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
#if defined(WIN32) && !defined(SQL_API)
#define SQL_API  __stdcall
#else
#define SQL_API
#endif
#if defined(_WCHAR_T_DEFINED) || SIZEOF_WCHAR_T > 0
typedef wchar_t SQLWCHAR;
#else
typedef uint16_t SQLWCHAR;
#endif
#if SIZEOF_LONG > 4
#define SQL_INTEGER_T int32_t
#define SQL_UINTEGER_T uint32_t
#else
#define SQL_INTEGER_T long
#define SQL_UINTEGER_T unsigned long
#endif
//---------------------------------------------------------------------------
class API {
  friend void initialize();
  friend void cleanup();
  public:
    int16_t (SQL_API * SQLAllocHandle)(int16_t HandleType,void * InputHandle,void ** OutputHandle);
    int16_t (SQL_API * SQLSetEnvAttr)(void * EnvironmentHandle,SQL_INTEGER_T Attribute,void * Value,SQL_INTEGER_T StringLength);
    int16_t (SQL_API * SQLSetConnectAttr)(void * ConnectionHandle,SQL_INTEGER_T Attribute, void * Value,SQL_INTEGER_T StringLength);
    int16_t (SQL_API * SQLBrowseConnectW)(void * hdbc,SQLWCHAR * szConnStrIn,int16_t cbConnStrIn,SQLWCHAR * szConnStrOut,int16_t cbConnStrOutMax,int16_t * pcbConnStrOut);
    int16_t (SQL_API * SQLDisconnect)(void * ConnectionHandle);
    int16_t (SQL_API * SQLFreeHandle)(int16_t HandleType,void * Handle);
    int16_t (SQL_API * SQLBindParameter)(
      void * hstmt,
      uint16_t ipar,
      int16_t fParamType,
      int16_t fCType,
      int16_t fSqlType,
      uintptr_t cbColDef,
      int16_t ibScale,
      void * rgbValue,
      intptr_t cbValueMax,
      intptr_t * pcbValue
    );
    int16_t (SQL_API * SQLDescribeParam)(
      void * hstmt,
      uint16_t ipar,
      int16_t * pfSqlType,
      SQL_UINTEGER_T * pcbParamDef,
      int16_t * pibScale,
      int16_t * pfNullable
    );
    int16_t (SQL_API * SQLNumParams)(void * hstmt,int16_t * pcpar);
    int16_t (SQL_API * SQLNumResultCols)(void * StatementHandle,int16_t * ColumnCount);
    int16_t (SQL_API * SQLEndTran)(int16_t HandleType,void * Handle,int16_t CompletionType);
    int16_t (SQL_API * SQLExecDirectW)(void * StatementHandle,SQLWCHAR * StatementText,SQL_INTEGER_T TextLength);
    int16_t (SQL_API * SQLExecute)(void * StatementHandle);
    int16_t (SQL_API * SQLFetch)(void * StatementHandle);
    int16_t (SQL_API * SQLGetDiagRecW)(int16_t HandleType,void * Handle,int16_t RecNumber,SQLWCHAR * Sqlstate,SQL_INTEGER_T * NativeError,SQLWCHAR * MessageText,int16_t BufferLength,int16_t * TextLength);
    int16_t (SQL_API * SQLPrepareW)(void * StatementHandle,SQLWCHAR * StatementText,SQL_INTEGER_T TextLength);
    int16_t (SQL_API * SQLSetStmtAttr)(void * StatementHandle,SQL_INTEGER_T Attribute,void * Value,SQL_INTEGER_T StringLength);
    int16_t (SQL_API * SQLGetInfo)(void * ConnectionHandle,uint16_t InfoType,void * InfoValue,int16_t BufferLength,int16_t * StringLength);
    int16_t (SQL_API * SQLDriverConnectW)(
      void * ConnectionHandle,
      void * WindowHandle,
      SQLWCHAR * InConnectionString,
      int16_t StringLength1,
      SQLWCHAR * OutConnectionString,
      int16_t BufferLength,
      int16_t * StringLength2Ptr,
      uint16_t DriverCompletion
    );

    void open();
    void close();
    utf8::String clientLibrary();
    void clientLibrary(const utf8::String & lib);
  protected:
    uint8_t mutex_[sizeof(ksys::WriteLock)];
    ksys::WriteLock & mutex();
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
inline ksys::WriteLock & API::mutex()
{
  return *reinterpret_cast<ksys::WriteLock *>(mutex_);
}
//---------------------------------------------------------------------------
inline utf8::String API::clientLibrary()
{
  ksys::AutoLock<ksys::WriteLock> lock(mutex());
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
  ksys::AutoLock<ksys::WriteLock> lock(mutex());
  *reinterpret_cast<utf8::String *>(clientLibrary_) = lib;
}
//---------------------------------------------------------------------------
} // namespace odbcpp
//---------------------------------------------------------------------------
#endif /* _odbcintf_H_ */
//---------------------------------------------------------------------------
