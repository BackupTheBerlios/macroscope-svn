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
#ifndef fbdbH
#define fbdbH
//---------------------------------------------------------------------------
namespace fbcpp {
//---------------------------------------------------------------------------
class Transaction;
class DSQLStatement;
class EventHandler;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DPB {
  friend class Database;
  public:
    ~DPB();
    DPB();

    const uintptr_t &     dpbLen() const;
    char *                dpb() const;
    DPB &                 clear();
    DPB &                 add(const utf8::String & name,const ksys::Mutant & value);
    const uintptr_t &     dialect();
    const uintptr_t &     pageSize();
    const int64_t &       fileLength();
    const utf8::String &  user() const;
    const utf8::String &  password() const;
    const utf8::String &  role() const;
    const utf8::String &  charset() const;
    const uintptr_t &     timeout() const;
  protected:
    DPB &                 injectCharset();
    DPB &                 injectTimeout();
  private:
#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#elif defined(__BCPLUSPLUS__)
#pragma option push -a1
#endif
    struct PACKED DPBParam {
      const char *  name_;
      char          number;
    };
#ifdef _MSC_VER
#pragma pack(pop)
#elif defined(__BCPLUSPLUS__)
#pragma option pop
#endif
    static DPBParam params[];
    char *          dpb_;
    uintptr_t       dpbLen_;
    uintptr_t       dialect_;
    uintptr_t       pageSize_;
    int64_t         fileLength_;
    utf8::String    user_;
    utf8::String    password_;
    utf8::String    role_;
    utf8::String    charset_;
    bool            charsetInDPB_;
    uintptr_t       timeout_;
    bool            timeoutInDPB_;

    DPB &     writeChar(uintptr_t code);
    DPB &     writeLong(uintptr_t a);
    intptr_t  writeISCCode(const utf8::String & name);
    DPB &     writeBuffer(const void * buf, uintptr_t size);
};
//---------------------------------------------------------------------------
inline const uintptr_t & DPB::dpbLen() const
{
  return dpbLen_;
}
//---------------------------------------------------------------------------
inline const uintptr_t & DPB::timeout() const
{
  return timeout_;
}
//---------------------------------------------------------------------------
inline char * DPB::dpb() const
{
  return dpb_;
}
//---------------------------------------------------------------------------
inline const uintptr_t & DPB::dialect()
{
  return dialect_;
}
//---------------------------------------------------------------------------
inline const uintptr_t & DPB::pageSize()
{
  return pageSize_;
}
//---------------------------------------------------------------------------
inline const int64_t & DPB::fileLength()
{
  return fileLength_;
}
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
inline const utf8::String & DPB::role() const
{
  return role_;
}
//---------------------------------------------------------------------------
inline const utf8::String & DPB::charset() const
{
  return charset_;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Database {
  friend class Transaction;
  friend class DSQLStatement;
  friend class EventHandler;
  friend class DSQLParamBlob;
  friend class DSQLValueBlob;
  public:
    virtual ~Database();
    Database();
    
    //void beforeDestruction() { detach(); }

    Database &            create(const utf8::String & name = utf8::String());
    Database &            drop();
    Database &            attach(const utf8::String & name = utf8::String());
    Database &            detach();

    // properties
    isc_db_handle &       handle();
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
    isc_db_handle                                       handle_;
    DPB                                                 dpb_;
    utf8::String                                        name_;

    ksys::HashedObjectList<utf8::String,Transaction>   transactions_;
    ksys::HashedObjectList<utf8::String,DSQLStatement> dsqlStatements_;
    ksys::HashedObjectList<utf8::String,EventHandler>  eventHandlers_;

    Database & detachHelper();
};
//---------------------------------------------------------------------------
inline isc_db_handle & Database::handle()
{
  return handle_;
}
//---------------------------------------------------------------------------
inline bool Database::attached()
{
  return handle_ != 0;
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
#if __GNUG__
typedef ksys::Array<Database *> DatabaseEnum;
#else
typedef ksys::HashedObjectList<utf8::String,Database> DatabaseEnum;
#endif
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
#endif /* fbdbH */
