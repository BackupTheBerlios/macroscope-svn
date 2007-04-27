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
#ifndef _fbintf_H_
#define _fbintf_H_
//---------------------------------------------------------------------------
namespace fbcpp {
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
#if FIREBIRD_STATIC_LIBRARY
    ISC_STATUS isc_attach_database(ISC_STATUS * p1, short p2, char * p3, isc_db_handle * p4, short p5, char * p6)
    {
      return ::isc_attach_database(p1, p2, p3, p4, p5, p6);
    }
    ISC_STATUS isc_detach_database(ISC_STATUS * p1, isc_db_handle * p2)
    {
      return ::isc_detach_database(p1, p2);
    }
    ISC_STATUS isc_drop_database(ISC_STATUS * p1, isc_db_handle * p2)
    {
      return ::isc_drop_database(p1, p2);
    }
    ISC_STATUS isc_start_multiple(ISC_STATUS * p1, isc_tr_handle * p2, short p3, void * p4)
    {
      return ::isc_start_multiple(p1, p2, p3, p4);
    }
    ISC_STATUS isc_rollback_retaining(ISC_STATUS * p1, isc_tr_handle * p2)
    {
      return ::isc_rollback_retaining(p1, p2);
    }
    ISC_STATUS isc_rollback_transaction(ISC_STATUS * p1, isc_tr_handle * p2)
    {
      return ::isc_rollback_transaction(p1, p2);
    }
    ISC_STATUS isc_commit_retaining(ISC_STATUS * p1, isc_tr_handle * p2)
    {
      return ::isc_commit_retaining(p1, p2);
    }
    ISC_STATUS isc_commit_transaction(ISC_STATUS * p1, isc_tr_handle * p2)
    {
      return ::isc_commit_transaction(p1, p2);
    }
    ISC_STATUS isc_prepare_transaction(ISC_STATUS * p1, isc_tr_handle * p2)
    {
      return ::isc_prepare_transaction(p1, p2);
    }
    void isc_event_counts(ISC_LONG * p1, short p2, char * p3, char * p4)
    {
      ::isc_event_counts(p1, p2, p3, p4);
    }
    ISC_STATUS isc_que_events(ISC_STATUS * p1, isc_db_handle * p2, ISC_LONG * p3, short p4, char * p5, isc_callback p6, void * p7)
    {
      return ::isc_que_events(p1, p2, p3, p4, p5, p6, p7);
    }
    ISC_LONG isc_event_block(char ** p1, char ** p2, unsigned short p3, const char * n0 = NULL, const char * n1 = NULL, const char * n2 = NULL, const char * n3 = NULL, const char * n4 = NULL, const char * n5 = NULL, const char * n6 = NULL, const char * n7 = NULL, const char * n8 = NULL, const char * n9 = NULL, const char * n10 = NULL, const char * n11 = NULL, const char * n12 = NULL, const char * n13 = NULL, const char * n14 = NULL)
    {
      return ::isc_event_block(p1, p2, p3, n0, n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14);
    }
    ISC_STATUS isc_cancel_events(ISC_STATUS * p1, isc_db_handle * p2, ISC_LONG * p3)
    {
      return ::isc_cancel_events(p1, p2, p3);
    }
    ISC_LONG isc_vax_integer(char * p1, short p2)
    {
      return ::isc_vax_integer(p1, p2);
    }
    ISC_STATUS isc_interprete(char * p1, ISC_STATUS ** p2)
    {
      return ::isc_interprete(p1, p2);
    }
    ISC_STATUS isc_dsql_execute_immediate(ISC_STATUS * p1,isc_db_handle * p2,isc_tr_handle * p3,unsigned short p4,const ISC_SCHAR * p5,unsigned short p6,XSQLDA * p7)
    {
      return ::isc_dsql_execute_immediate(p1, p2, p3, p4, p5, p6, p7);
    }
    ISC_STATUS isc_dsql_allocate_statement(ISC_STATUS * p1, isc_db_handle * p2, isc_stmt_handle * p3)
    {
      return ::isc_dsql_allocate_statement(p1, p2, p3);
    }
    ISC_STATUS isc_dsql_free_statement(ISC_STATUS * p1, isc_stmt_handle * p2, unsigned short p3)
    {
      return ::isc_dsql_free_statement(p1, p2, p3);
    }
    ISC_STATUS isc_dsql_describe(ISC_STATUS * p1, isc_stmt_handle * p2, unsigned short p3, XSQLDA * p4)
    {
      return ::isc_dsql_describe(p1, p2, p3, p4);
    }
    ISC_STATUS isc_dsql_describe_bind(ISC_STATUS * p1, isc_stmt_handle * p2, unsigned short p3, XSQLDA * p4)
    {
      return ::isc_dsql_describe_bind(p1, p2, p3, p4);
    }
    ISC_STATUS isc_dsql_prepare(ISC_STATUS * p1, isc_tr_handle * p2, isc_stmt_handle * p3, unsigned short p4, char * p5, unsigned short p6, XSQLDA * p7)
    {
      return ::isc_dsql_prepare(p1, p2, p3, p4, p5, p6, p7);
    }
    ISC_STATUS isc_dsql_execute(ISC_STATUS * p1, isc_tr_handle * p2, isc_stmt_handle * p3, unsigned short p4, XSQLDA * p5)
    {
      return ::isc_dsql_execute(p1, p2, p3, p4, p5);
    }
    ISC_STATUS isc_dsql_set_cursor_name(ISC_STATUS * p1, isc_stmt_handle * p2, char * p3, unsigned short p4)
    {
      return ::isc_dsql_set_cursor_name(p1, p2, p3, p4);
    }
    ISC_STATUS isc_dsql_fetch(ISC_STATUS * p1, isc_stmt_handle * p2, unsigned short p3, XSQLDA * p4)
    {
      return ::isc_dsql_fetch(p1, p2, p3, p4);
    }
    ISC_STATUS isc_dsql_sql_info(ISC_STATUS * p1, isc_stmt_handle * p2, short p3, const char * p4, short p5, char * p6)
    {
      return ::isc_dsql_sql_info(p1, p2, p3, p4, p5, p6);
    }
    ISC_STATUS isc_array_lookup_bounds(ISC_STATUS * p1, isc_db_handle * p2, isc_tr_handle * p3, char * p4, char * p5, ISC_ARRAY_DESC * p6)
    {
      return ::isc_array_lookup_bounds(p1, p2, p3, p4, p5, p6);
    }
    ISC_STATUS isc_blob_lookup_desc(ISC_STATUS * p1, isc_db_handle * p2, isc_tr_handle * p3, char * p4, char * p5, ISC_BLOB_DESC * p6, char * p7)
    {
      return ::isc_blob_lookup_desc(p1, p2, p3, p4, p5, p6, p7);
    }
    ISC_STATUS isc_array_put_slice(ISC_STATUS * p1, isc_db_handle * p2, isc_tr_handle * p3, ISC_QUAD * p4, ISC_ARRAY_DESC * p5, void * p6, ISC_LONG * p7)
    {
      return ::isc_array_put_slice(p1, p2, p3, p4, p5, p6, p7);
    }
    ISC_STATUS isc_array_get_slice(ISC_STATUS * p1, isc_db_handle * p2, isc_tr_handle * p3, ISC_QUAD * p4, ISC_ARRAY_DESC * p5, void * p6, ISC_LONG * p7)
    {
      return ::isc_array_get_slice(p1, p2, p3, p4, p5, p6, p7);
    }
    ISC_STATUS isc_create_blob2(ISC_STATUS * p1, isc_db_handle * p2, isc_tr_handle * p3, isc_blob_handle * p4, ISC_QUAD * p5, short p6, char * p7)
    {
      return ::isc_create_blob2(p1, p2, p3, p4, p5, p6, p7);
    }
    ISC_STATUS isc_open_blob2(ISC_STATUS * p1, isc_db_handle * p2, isc_tr_handle * p3, isc_blob_handle * p4, ISC_QUAD * p5, ISC_USHORT p6, ISC_UCHAR * p7)
    {
      return ::isc_open_blob2(p1, p2, p3, p4, p5, p6, p7);
    }
    ISC_STATUS isc_close_blob(ISC_STATUS * p1, isc_blob_handle * p2)
    {
      return ::isc_close_blob(p1, p2);
    }
    ISC_STATUS isc_cancel_blob(ISC_STATUS * p1, isc_blob_handle * p2)
    {
      return ::isc_cancel_blob(p1, p2);
    }
    ISC_STATUS isc_put_segment(ISC_STATUS * p1, isc_blob_handle * p2, unsigned short p3, char * p4)
    {
      return ::isc_put_segment(p1, p2, p3, p4);
    }
    ISC_STATUS isc_get_segment(ISC_STATUS * p1, isc_blob_handle * p2, unsigned short * p3, unsigned short p4, char * p5)
    {
      return ::isc_get_segment(p1, p2, p3, p4, p5);
    }
    ISC_STATUS isc_service_attach(ISC_STATUS * p1, unsigned short p2, char * p3, isc_svc_handle * p4, unsigned short p5, char * p6)
    {
      return ::isc_service_attach(p1, p2, p3, p4, p5, p6);
    }
    ISC_STATUS isc_service_detach(ISC_STATUS * p1, isc_svc_handle * p2)
    {
      return ::isc_service_detach(p1, p2);
    }
    ISC_STATUS isc_service_query(ISC_STATUS * p1, isc_svc_handle * p2, isc_resv_handle * p3, unsigned short p4, char * p5, unsigned short p6, char * p7, unsigned short p8, char * p9)
    {
      return ::isc_service_query(p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }
    ISC_STATUS isc_service_start(ISC_STATUS * p1, isc_svc_handle * p2, isc_resv_handle * p3, unsigned short p4, char * p5)
    {
      return ::isc_service_start(p1, p2, p3, p4, p5);
    }
    void isc_encode_timestamp(const struct tm * p1, ISC_TIMESTAMP * p2)
    {
      ::isc_encode_timestamp(p1, p2);
    }
    void isc_decode_timestamp(const ISC_TIMESTAMP * p1, struct tm * p2)
    {
      isc_decode_timestamp(p1, p2);
    }
    ISC_LONG ISC_EXPORT fb_interpret(ISC_SCHAR * p1,int p2,const ISC_STATUS ** p3)
    {
      return ::fb_interpret(p1,p2,p3);
    }
#else
#if _MSC_VER
#pragma warning(push,3)
#endif
    union {
        struct {
            union {
                ISC_STATUS (ISC_EXPORT * isc_attach_database)(ISC_STATUS *,
                                                              short, char *,
                                                              isc_db_handle *,
                                                              short, char *);
                void *  p_isc_attach_database;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_detach_database)(ISC_STATUS *,
                                                              isc_db_handle *);
                void *  p_isc_detach_database;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_drop_database)(ISC_STATUS *,
                                                            isc_db_handle *);
                void *  p_isc_drop_database;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_start_multiple)(ISC_STATUS *,
                                                             isc_tr_handle *,
                                                             short, void *);
                void *  p_isc_start_multiple;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_rollback_retaining)(
                ISC_STATUS *, isc_tr_handle *);
                void *  p_isc_rollback_retaining;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_rollback_transaction)(
                ISC_STATUS *, isc_tr_handle *);
                void *  p_isc_rollback_transaction;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_commit_retaining)(ISC_STATUS *,
                                                               isc_tr_handle *);
                void *  p_isc_commit_retaining;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_commit_transaction)(
                ISC_STATUS *, isc_tr_handle *);
                void *  p_isc_commit_transaction;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_prepare_transaction)(
                ISC_STATUS *, isc_tr_handle *);
                void *  p_isc_prepare_transaction;
            };
            union {
                void (ISC_EXPORT * isc_event_counts)(ISC_LONG *, short,
                                                     char *, char *);
                void *  p_isc_event_counts;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_que_events)(ISC_STATUS *,
                                                         isc_db_handle *,
                                                         ISC_LONG *, short,
                                                         char *, isc_callback,
                                                         void *);
                void *  p_isc_que_events;
            };
            union {
                ISC_LONG (ISC_EXPORT_VARARG * isc_event_block)(char **,
                                                               char **,
                                                               unsigned short,
                                                               ...);
                void *  p_isc_event_block;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_cancel_events)(ISC_STATUS *,
                                                            isc_db_handle *,
                                                            ISC_LONG *);
                void *  p_isc_cancel_events;
            };
            union {
                ISC_LONG (ISC_EXPORT * isc_vax_integer)(char *, short);
                void *  p_isc_vax_integer;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_interprete)(char *, ISC_STATUS **);
                void *  p_isc_interprete;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_dsql_execute_immediate)(ISC_STATUS * p1,isc_db_handle * p2,isc_tr_handle * p3,unsigned short p4,const ISC_SCHAR * p5,unsigned short p6,XSQLDA * p7);
                void * p_isc_dsql_execute_immediate;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_dsql_allocate_statement)(
                ISC_STATUS *, isc_db_handle *, isc_stmt_handle *);
                void *  p_isc_dsql_allocate_statement;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_dsql_free_statement)(
                ISC_STATUS *, isc_stmt_handle *, unsigned short);
                void *  p_isc_dsql_free_statement;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_dsql_describe)(ISC_STATUS *,
                                                            isc_stmt_handle *,
                                                            unsigned short,
                                                            XSQLDA *);
                void *  p_isc_dsql_describe;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_dsql_describe_bind)(
                ISC_STATUS *, isc_stmt_handle *, unsigned short, XSQLDA *);
                void *  p_isc_dsql_describe_bind;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_dsql_prepare)(ISC_STATUS *,
                                                           isc_tr_handle *,
                                                           isc_stmt_handle *,
                                                           unsigned short,
                                                           char *,
                                                           unsigned short,
                                                           XSQLDA *);
                void *  p_isc_dsql_prepare;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_dsql_execute)(ISC_STATUS *,
                                                           isc_tr_handle *,
                                                           isc_stmt_handle *,
                                                           unsigned short,
                                                           XSQLDA *);
                void *  p_isc_dsql_execute;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_dsql_set_cursor_name)(
                ISC_STATUS *, isc_stmt_handle *, char *, unsigned short);
                void *  p_isc_dsql_set_cursor_name;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_dsql_fetch)(ISC_STATUS *,
                                                         isc_stmt_handle *,
                                                         unsigned short,
                                                         XSQLDA *);
                void *  p_isc_dsql_fetch;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_dsql_sql_info)(ISC_STATUS *,
                                                            isc_stmt_handle *,
                                                            short,
                                                            const char *,
                                                            short, char *);
                void *  p_isc_dsql_sql_info;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_array_lookup_bounds)(
                ISC_STATUS *, isc_db_handle *, isc_tr_handle *, char *,
                char *, ISC_ARRAY_DESC *);
                void *  p_isc_array_lookup_bounds;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_blob_lookup_desc)(
                ISC_STATUS *,
                isc_db_handle *, isc_tr_handle *, char *, char *,
                ISC_BLOB_DESC *,
                char *);
                void *  p_isc_blob_lookup_desc;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_array_put_slice)(
                ISC_STATUS *, isc_db_handle *, isc_tr_handle *, ISC_QUAD *,
                ISC_ARRAY_DESC *, void *, ISC_LONG *);
                void *  p_isc_array_put_slice;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_array_get_slice)(
                ISC_STATUS *, isc_db_handle *, isc_tr_handle *, ISC_QUAD *,
                ISC_ARRAY_DESC *, void *, ISC_LONG *);
                void *  p_isc_array_get_slice;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_create_blob2)(ISC_STATUS *,
                                                           isc_db_handle *,
                                                           isc_tr_handle *,
                                                           isc_blob_handle *,
                                                           ISC_QUAD *, short,
                                                           char *);
                void *  p_isc_create_blob2;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_open_blob2)(ISC_STATUS *,
                                                         isc_db_handle *,
                                                         isc_tr_handle *,
                                                         isc_blob_handle *,
                                                         ISC_QUAD *,
                                                         ISC_USHORT,
                                                         ISC_UCHAR *);
                void *  p_isc_open_blob2;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_close_blob)(ISC_STATUS *,
                                                         isc_blob_handle *);
                void *  p_isc_close_blob;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_cancel_blob)(ISC_STATUS *,
                                                          isc_blob_handle *);
                void *  p_isc_cancel_blob;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_put_segment)(ISC_STATUS *,
                                                          isc_blob_handle *,
                                                          unsigned short,
                                                          char *);
                void *  p_isc_put_segment;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_get_segment)(ISC_STATUS *,
                                                          isc_blob_handle *,
                                                          unsigned short *,
                                                          unsigned short,
                                                          char *);
                void *  p_isc_get_segment;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_service_attach)(ISC_STATUS *,
                                                             unsigned short,
                                                             char *,
                                                             isc_svc_handle *,
                                                             unsigned short,
                                                             char *);
                void *  p_isc_service_attach;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_service_detach)(ISC_STATUS *,
                                                             isc_svc_handle *);
                void *  p_isc_service_detach;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_service_query)(ISC_STATUS *,
                                                            isc_svc_handle *,
                                                            isc_resv_handle *,
                                                            unsigned short,
                                                            char *,
                                                            unsigned short,
                                                            char *,
                                                            unsigned short,
                                                            char *);
                void *  p_isc_service_query;
            };
            union {
                ISC_STATUS (ISC_EXPORT * isc_service_start)(ISC_STATUS *,
                                                            isc_svc_handle *,
                                                            isc_resv_handle *,
                                                            unsigned short,
                                                            char *);
                void *  p_isc_service_start;
            };
            union {
                void (ISC_EXPORT * isc_encode_timestamp)(const struct tm *,
                                                         ISC_TIMESTAMP *);
                void *  p_isc_encode_timestamp;
            };
            union {
                void (ISC_EXPORT * isc_decode_timestamp)(
                const ISC_TIMESTAMP *, struct tm *);
                void *  p_isc_decode_timestamp;
            };
            union {
                ISC_LONG (ISC_EXPORT * fb_interpret)(ISC_SCHAR * p1,int p2,const ISC_STATUS ** p3);
                void *  p_fb_interpret;
            };
        };
    };
#if _MSC_VER
#pragma warning(pop)
#endif
#endif   
    void open();
    void close();
    utf8::String clientLibrary();
    void clientLibrary(const utf8::String & lib);
  protected:
    utf8::String clientLibraryNL();
#if !FIREBIRD_STATIC_LIBRARY
#if defined(__WIN32__) || defined(__WIN64__)
    HINSTANCE                 handle_;
#else
    void *                    handle_;
#endif
    uint8_t mutex_[sizeof(ksys::InterlockedMutex)];
    ksys::InterlockedMutex & mutex();
    uint8_t clientLibrary_[sizeof(utf8::String)];

    intptr_t                 count_;

    utf8::String              tryOpen();

    static const char * const symbols_[];
#endif
  private:
    void                      initialize();
    void                      cleanup();
};
//---------------------------------------------------------------------------
extern API  api;
//---------------------------------------------------------------------------
#if !FIREBIRD_STATIC_LIBRARY
inline ksys::InterlockedMutex & API::mutex()
{
  return *reinterpret_cast<ksys::InterlockedMutex *>(mutex_);
}
#endif
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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
bool  findISCCode(const ISC_STATUS * pStatus, ISC_LONG code);
bool  iscIsFatalError(const ISC_STATUS_ARRAY status);
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
#endif /* _fbintf_H_ */
//---------------------------------------------------------------------------
