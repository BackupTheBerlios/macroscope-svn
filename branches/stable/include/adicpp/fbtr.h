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
#ifndef _fbtr_H_
#define _fbtr_H_
//---------------------------------------------------------------------------
namespace fbcpp {
//---------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#elif defined(__BCPLUSPLUS__)
#pragma option push -a1
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
struct PACKED ISC_TEB {
  isc_db_handle * db_ptr;
  uintptr_t       tpb_len;
  char *          tpb_ptr;
};
//---------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma pack(pop)
#elif defined(__BCPLUSPLUS__)
#pragma option pop
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class TPB {
  friend class Transaction;
  public:
    ~TPB();
    TPB();

    TPB & clear();
    TPB & add(const utf8::String & name);
    TPB & add(const utf8::String & name, const utf8::String & tableName);
  protected:
  private:
    TPB(const TPB &);
    void operator = (const TPB &);
#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#elif defined(__BCPLUSPLUS__)
#pragma option push -a1
#endif
    struct PACKED TPBParam {
      const char *  name_;
      char          number;
    };
#ifdef _MSC_VER
#pragma pack(pop)
#elif defined(__BCPLUSPLUS__)
#pragma option pop
#endif
    static TPBParam params[];
    char *          tpb_;
    uintptr_t       tpbLen_;

    TPB &     writeChar(uintptr_t code);
    intptr_t  writeISCCode(const utf8::String & name);
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Transaction : virtual public Base {
  friend class Database;
  friend class DSQLStatement;
  public:
    virtual ~Transaction();
    Transaction();
    
    Transaction &   attach(Database & database);
    Transaction &   detach(Database & database);
    Transaction &   detach();

    void beforeDestruction() { detach(); }

    Transaction &   isolation(const utf8::String & isolation);
    const utf8::String & isolation() const;

    Transaction &   start();
    Transaction &   prepare();
    Transaction &   commit(bool noThrow = false);
    Transaction &   commitRetaining(bool noThrow = false);
    Transaction &   rollback(bool noThrow = false);
    Transaction &   rollbackRetaining(bool noThrow = false);

    // properties
    isc_tr_handle & handle();
    bool            attached();
    TPB &           paramsByDatabase(Database & database);
    bool            active();

    //  protected:
    void            processingException(ksys::Exception * e);
    void            staticExceptionHandler(ksys::Exception * e);
    void            exceptionHandler(ksys::Exception * e);
  private:
    isc_tr_handle                                       handle_;
    uintptr_t                                           startCount_;
    utf8::String isolation_;

    DatabaseEnum                                        databases_;
    ksys::HashedObjectList< utf8::String,TPB>           tpbs_;
    ksys::HashedObjectList< utf8::String,DSQLStatement> dsqlStatements_;

    enum { lrtNone, lrtCommit, lrtRollback } lastRetainingTransaction_;
    Transaction & retainingHelper(bool noThrow = false);
};
//---------------------------------------------------------------------------
inline const utf8::String & Transaction::isolation() const
{
  return isolation_;
}
//---------------------------------------------------------------------------
inline isc_tr_handle & Transaction::handle()
{
  return handle_;
}
//---------------------------------------------------------------------------
inline bool Transaction::attached()
{
  return databases_.count() > 0;
}
//---------------------------------------------------------------------------
inline bool Transaction::active()
{
  return startCount_ > 0 || lastRetainingTransaction_ != lrtNone;
}
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
#endif /* _fbtr_H_ */
//---------------------------------------------------------------------------
