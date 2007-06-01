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
#ifndef odbcdbH
#define odbcdbH
//---------------------------------------------------------------------------
namespace odbcpp {
//---------------------------------------------------------------------------
class Transaction;
class DSQLStatement;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Base : virtual ksys::Object {
  friend class EClientServer;
  public:
    virtual ~Base() {}
    Base() {}
  protected:
    virtual void exceptionHandler(ksys::Exception * e) = 0;
  private:
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DPB {
  public:
    ~DPB();
    DPB();

    DPB & clear();
    DPB & add(const utf8::String & name,const ksys::Mutant & value);
    const utf8::String & user() const;
    const utf8::String & password() const;
  protected:
  private:
    utf8::String user_;
    utf8::String password_;
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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EClientServer;
//---------------------------------------------------------------------------
class Database : virtual public Base {
  public:
    virtual ~Database();
    Database();
    
    void beforeDestruction() { detach(); }

    Database &            create(const utf8::String & name = utf8::String());
    Database &            drop();
    Database &            attach(const utf8::String & name = utf8::String());
    Database &            detach();

    // properties
    bool                  attached();
    DPB &                 params();
    Database &            name(const utf8::String & name);
    const utf8::String &  name();
    static bool           separateDBName(const utf8::String & name, utf8::String & hostName, utf8::String & dbName, uintptr_t & port);

    //  protected:
    void                  processingException(ksys::Exception * e);
    void                  staticExceptionHandler(ksys::Exception * e);
    void                  exceptionHandler(ksys::Exception * e);
  private:
    SQLHENV envHandle_; // Handle ODBC environment
    SQLHDBC handle_;    // Handle connection
    DPB dpb_;
    utf8::String name_;
    Transaction * transaction_;
    ksys::EmbeddedHash<utf8::String,DSQLStatement> statements_;
};
//---------------------------------------------------------------------------
inline bool Database::attached()
{
  return handle_ != NULL;
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
} // namespace odbcpp
//---------------------------------------------------------------------------
#endif // odbcdbH
//---------------------------------------------------------------------------
