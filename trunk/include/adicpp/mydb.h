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
#ifndef mydbH
#define mydbH
//---------------------------------------------------------------------------
namespace mycpp {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Base {
  friend class EClientServer;
  public:
    virtual ~Base() {}
  protected:
    virtual void exceptionHandler(ksys::Exception * e) = 0;
  private:
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DPB {
  private:
    utf8::String user_;
    utf8::String password_;
    utf8::String protocol_;
  public:
    DPB();
    ~DPB();

    DPB & clear();
    DPB & add(const utf8::String & name,const ksys::Mutant & value);
    const utf8::String & user() const;
    const utf8::String & password() const;
    const utf8::String & protocol() const;
};
//---------------------------------------------------------------------------
inline const utf8::String & DPB::user() const
{
  return user_;
}
//---------------------------------------------------------------------------
inline const utf8::String & DPB::password() const
{
  return password_;
}
//---------------------------------------------------------------------------
inline const utf8::String & DPB::protocol() const
{
  return protocol_;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EClientServer;
//---------------------------------------------------------------------------
class Database : virtual public Base {
  friend class Transaction;
  friend class DSQLValues;
  friend class DSQLStatement;
  public:
    virtual ~Database();
    Database();

    Database & create(const utf8::String & name = utf8::String());
    Database & drop();
    Database & attach(const utf8::String & name = utf8::String());
    Database & detach();

// properties
    bool attached();
    MYSQL * const & handle();
    DPB & params();
    Database & name(const utf8::String & name);
    const utf8::String & name();
    static bool separateDBName(
      const utf8::String & name,
      utf8::String & hostName,
      utf8::String & dbName,
      uintptr_t & port
    );

//  protected:
    void processingException(ksys::Exception * e);
    void staticExceptionHandler(ksys::Exception * e);
    void exceptionHandler(ksys::Exception * e);
  private:
    MYSQL * handle_;
    DPB dpb_;
    utf8::String name_;
    Transaction * transaction_;
    ksys::HashedObjectList<utf8::String,DSQLStatement> dsqlStatements_;

    Database & allocHandle(MYSQL * & handle);
    Database & freeHandle(MYSQL * & handle);
};
//---------------------------------------------------------------------------
inline bool Database::attached()
{
  return handle_ != NULL;
}
//---------------------------------------------------------------------------
inline MYSQL * const & Database::handle()
{
  return handle_;
}
//---------------------------------------------------------------------------
inline DPB & Database::params()
{
  return dpb_;
}
//---------------------------------------------------------------------------
inline const utf8::String & Database::name()
{
  return name_;
}
//---------------------------------------------------------------------------
} // namespace mycpp
//---------------------------------------------------------------------------
#endif // mydbH
