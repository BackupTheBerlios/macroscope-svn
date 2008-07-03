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
#ifndef _odbctr_H_
#define _odbctr_H_
//---------------------------------------------------------------------------
namespace odbcpp {
//---------------------------------------------------------------------------
class Database;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Transaction : virtual public Base {
  friend class Database;
  friend class DSQLStatement;
  public:
    virtual ~Transaction();
    Transaction();

    void beforeDestruction() { detach(); }

    Transaction &         attach(Database & database);
    Transaction &         detach();

    Transaction &         start();
    Transaction &         commit(bool noThrow = false);
    Transaction &         rollback(bool noThrow = false);

    // properties
    bool                  attached();
    bool                  active();
    Transaction &         isolation(const utf8::String & isolation);
    const utf8::String &  isolation() const;
    //  protected:
    void                  processingException(ksys::Exception * e);
    void                  staticExceptionHandler(ksys::Exception * e);
    void                  exceptionHandler(ksys::Exception * e);
  private:
    Database *    database_;
    uintptr_t     startCount_;
    utf8::String  isolation_;
};
//---------------------------------------------------------------------------
inline bool Transaction::active()
{
  return startCount_ > 0;
}
//---------------------------------------------------------------------------
inline bool Transaction::attached()
{
  return database_ != NULL;
}
//---------------------------------------------------------------------------
inline Transaction & Transaction::isolation(const utf8::String & isolation)
{
  isolation_ = isolation;
  return *this;
}
//---------------------------------------------------------------------------
inline const utf8::String & Transaction::isolation() const
{
  return isolation_;
}
//---------------------------------------------------------------------------
} // namespace odbcpp
//---------------------------------------------------------------------------
#endif // _odbctr_H_
//---------------------------------------------------------------------------
