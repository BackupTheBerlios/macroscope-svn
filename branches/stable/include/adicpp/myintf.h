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
#ifndef _myintf_H_
#define _myintf_H_
//---------------------------------------------------------------------------
#if MYSQL_STATIC_LIBRARY
extern "C" void my_end(int infoflag);
#endif
//---------------------------------------------------------------------------
namespace mycpp {

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
#if MYSQL_STATIC_LIBRARY
    unsigned int mysql_thread_safe()
    {
      return ::mysql_thread_safe();
    }
    my_bool mysql_thread_init()
    {
      return ::mysql_thread_init();
    }
    void mysql_thread_end()
    {
      ::mysql_thread_end();
    }
    my_bool my_init()
    {
      return ::my_init();
    }
    void my_end(int infoflag)
    {
      ::my_end(infoflag);
    }
    MYSQL * mysql_init(MYSQL * mysql)
    {
      return ::mysql_init(mysql);
    }
    MYSQL * mysql_real_connect(MYSQL * mysql, const char * host, const char * user, const char * passwd, const char * db, unsigned int port, const char * unix_socket, unsigned long clientflag)
    {
      return ::mysql_real_connect(mysql, host, user, passwd, db, port, unix_socket, clientflag);
    }
    void mysql_close(MYSQL * sock)
    {
      return ::mysql_close(sock);
    }
    int mysql_query(MYSQL * mysql, const char * q)
    {
      return ::mysql_query(mysql, q);
    }
    int mysql_real_query(MYSQL * mysql, const char * q, unsigned long length)
    {
      return ::mysql_real_query(mysql, q, length);
    }
    my_bool mysql_commit(MYSQL * mysql)
    {
      return ::mysql_commit(mysql);
    }
    my_bool mysql_rollback(MYSQL * mysql)
    {
      return ::mysql_rollback(mysql);
    }
    my_bool mysql_autocommit(MYSQL * mysql, my_bool auto_mode)
    {
      return ::mysql_autocommit(mysql, auto_mode);
    }
    unsigned int mysql_errno(MYSQL * mysql)
    {
      return ::mysql_errno(mysql);
    }
    const char * mysql_error(MYSQL * mysql)
    {
      return ::mysql_error(mysql);
    }
    MYSQL_STMT * mysql_stmt_init(MYSQL * mysql)
    {
      return ::mysql_stmt_init(mysql);
    }
    my_bool mysql_stmt_close(MYSQL_STMT * stmt)
    {
      return ::mysql_stmt_close(stmt);
    }
    int mysql_stmt_prepare(MYSQL_STMT * stmt, const char * query, unsigned long length)
    {
      return ::mysql_stmt_prepare(stmt, query, length);
    }
    my_bool mysql_stmt_bind_param(MYSQL_STMT * stmt, MYSQL_BIND * bnd)
    {
      return ::mysql_stmt_bind_param(stmt, bnd);
    }
    my_bool mysql_stmt_bind_result(MYSQL_STMT * stmt, MYSQL_BIND * bnd)
    {
      return ::mysql_stmt_bind_result(stmt, bnd);
    }
    unsigned long mysql_stmt_param_count(MYSQL_STMT * stmt)
    {
      return ::mysql_stmt_param_count(stmt);
    }
    unsigned int mysql_stmt_field_count(MYSQL_STMT * stmt)
    {
      return ::mysql_stmt_field_count(stmt);
    }
    int mysql_stmt_execute(MYSQL_STMT * stmt)
    {
      return ::mysql_stmt_execute(stmt);
    }
    int mysql_stmt_fetch(MYSQL_STMT * stmt)
    {
      return ::mysql_stmt_fetch(stmt);
    }
    int mysql_stmt_store_result(MYSQL_STMT * stmt)
    {
      return ::mysql_stmt_store_result(stmt);
    }
    MYSQL_RES * mysql_stmt_result_metadata(MYSQL_STMT * stmt)
    {
      return ::mysql_stmt_result_metadata(stmt);
    }
    MYSQL_RES * mysql_stmt_param_metadata(MYSQL_STMT * stmt)
    {
      return ::mysql_stmt_param_metadata(stmt);
    }
    my_bool mysql_stmt_free_result(MYSQL_STMT * stmt)
    {
      return ::mysql_stmt_free_result(stmt);
    }
    void mysql_free_result(MYSQL_RES * result)
    {
      ::mysql_free_result(result);
    }
    MYSQL_FIELD * mysql_fetch_fields(MYSQL_RES * res)
    {
      return ::mysql_fetch_fields(res);
    }
    my_ulonglong mysql_insert_id(MYSQL * mysql)
    {
      return ::mysql_insert_id(mysql);
    }
    int mysql_options(MYSQL * mysql, enum mysql_option option, const void * arg)
    {
      return ::mysql_options(mysql, option, (const char *) arg);
    }
#else
#if _MSC_VER
#pragma warning(push,3)
#endif
    union {
        struct {
            union {
                unsigned int (STDCALL * mysql_thread_safe)();
                void *  p_mysql_thread_safe;
            };
            union {
                my_bool (STDCALL * mysql_thread_init)();
                void *  p_mysql_thread_init;
            };
            union {
                void (STDCALL * mysql_thread_end)();
                void *  p_mysql_thread_end;
            };
            union {
                my_bool (* my_init)();
                void *  p_my_init;
            };
            union {
                void (* my_end)(int infoflag);
                void *  p_my_end;
            };
            union {
                MYSQL * (STDCALL * mysql_init)(MYSQL * mysql);
                void *  p_mysql_init;
            };
            union {
                MYSQL * (STDCALL * mysql_real_connect)(
                MYSQL * mysql, const char * host, const char * user,
                const char * passwd, const char * db, unsigned int port,
                const char * unix_socket, unsigned long clientflag);
                void *  p_mysql_real_connect;
            };
            union {
                void (STDCALL * mysql_close)(MYSQL * sock);
                void *  p_mysql_close;
            };
            union {
                int (STDCALL * mysql_query)(MYSQL * mysql, const char * q);
                void *  p_mysql_query;
            };
            union {
                int (STDCALL * mysql_real_query)(MYSQL * mysql,
                                                 const char * q,
                                                 unsigned long length);
                void *  p_mysql_real_query;
            };
            union {
                my_bool (STDCALL * mysql_commit)(MYSQL * mysql);
                void *  p_mysql_commit;
            };
            union {
                my_bool (STDCALL * mysql_rollback)(MYSQL * mysql);
                void *  p_mysql_rollback;
            };
            union {
                my_bool (STDCALL * mysql_autocommit)(MYSQL * mysql,
                                                     my_bool auto_mode);
                void *  p_mysql_autocommit;
            };
            union {
                unsigned int (STDCALL * mysql_errno)(MYSQL * mysql);
                void *  p_mysql_errno;
            };
            union {
                const char * (STDCALL * mysql_error)(MYSQL * mysql);
                void *  p_mysql_error;
            };
            union {
                MYSQL_STMT * (STDCALL * mysql_stmt_init)(MYSQL * mysql);
                void *  p_mysql_stmt_init;
            };
            union {
                my_bool (STDCALL * mysql_stmt_close)(MYSQL_STMT * stmt);
                void *  p_mysql_stmt_close;
            };
            union {
                int (STDCALL * mysql_stmt_prepare)(MYSQL_STMT * stmt,
                                                   const char * query,
                                                   unsigned long length);
                void *  p_mysql_stmt_prepare;
            };
            union {
                my_bool (STDCALL * mysql_stmt_bind_param)(MYSQL_STMT * stmt,
                                                          MYSQL_BIND * bnd);
                void *  p_mysql_stmt_bind_param;
            };
            union {
                my_bool (STDCALL * mysql_stmt_bind_result)(MYSQL_STMT * stmt,
                                                           MYSQL_BIND * bnd);
                void *  p_mysql_stmt_bind_result;
            };
            union {
                unsigned long (STDCALL * mysql_stmt_param_count)(MYSQL_STMT *
                                                                 stmt);
                void *  p_mysql_stmt_param_count;
            };
            union {
                unsigned int (STDCALL * mysql_stmt_field_count)(MYSQL_STMT *
                                                                stmt);
                void *  p_mysql_stmt_field_count;
            };
            union {
                int (STDCALL * mysql_stmt_execute)(MYSQL_STMT * stmt);
                void *  p_mysql_stmt_execute;
            };
            union {
                int (STDCALL * mysql_stmt_fetch)(MYSQL_STMT * stmt);
                void *  p_mysql_stmt_fetch;
            };
            union {
                int (STDCALL * mysql_stmt_store_result)(MYSQL_STMT * stmt);
                void *  p_mysql_stmt_store_result;
            };
            union {
                MYSQL_RES * (STDCALL * mysql_stmt_result_metadata)(MYSQL_STMT
                                                                 * stmt);
                void *  p_mysql_stmt_result_metadata;
            };
            union {
                MYSQL_RES * (STDCALL * mysql_stmt_param_metadata)(MYSQL_STMT *
                                                                  stmt);
                void *  p_mysql_stmt_param_metadata;
            };
            union {
                my_bool (STDCALL * mysql_stmt_free_result)(MYSQL_STMT * stmt);
                void *  p_mysql_stmt_free_result;
            };
            union {
                void (STDCALL * mysql_free_result)(MYSQL_RES * result);
                void *  p_mysql_free_result;
            };
            union {
                MYSQL_FIELD * (STDCALL * mysql_fetch_fields)(MYSQL_RES * res);
                void *  p_mysql_fetch_fields;
            };
            union {
                my_ulonglong (STDCALL * mysql_insert_id)(MYSQL * mysql);
                void *  p_mysql_insert_id;
            };
            union {
                int (STDCALL * mysql_options)(MYSQL * mysql,
                                              enum mysql_option option,
                                              const void * arg);
                void *  p_mysql_options;
            };
        };
    };
#if _MSC_VER
#pragma warning(pop)
#endif
#endif
    void  open();
    void  close();
  protected:
    uint8_t mutex_[sizeof(ksys::InterlockedMutex)];
    struct ThreadList {
        uintptr_t id_;
        uintptr_t count_;

        ThreadList(uintptr_t id = 0, uintptr_t count = 0)
          : id_(id),
            count_(count)
        {
        }
        ThreadList(const ThreadList & tl)
          : id_(tl.id_),
            count_(tl.count_)
        {
        }
        ThreadList & operator =(const ThreadList & tl)
        {
          id_ = tl.id_; count_ = tl.count_; return *this;
        }
        bool operator ==(const ThreadList & tl) const
        {
          return id_ == tl.id_;
        }
        bool operator !=(const ThreadList & tl) const
        {
          return id_ != tl.id_;
        }
        bool operator >(const ThreadList & tl) const
        {
          return id_ > tl.id_;
        }
        bool operator <(const ThreadList & tl) const
        {
          return id_ < tl.id_;
        }
    };
    uint8_t                   threadList_[sizeof(ksys::Array< ThreadList>)];

    ksys::InterlockedMutex &    mutex();
    ksys::Array< ThreadList> &  threadList();
    uintptr_t                 count_;
#if !MYSQL_STATIC_LIBRARY
#if defined(__WIN32__) || defined(__WIN64__)
    HINSTANCE                 handle_;
#else
    void *                    handle_;
#endif

    utf8::String                API::tryOpen();

    static const char * const symbols_[];
#endif
  private:
    static void                 afterThreadExecute(API * papi);

    void                        initialize();
    void                        cleanup();
};
//---------------------------------------------------------------------------
extern API  api;
//---------------------------------------------------------------------------
inline ksys::InterlockedMutex & API::mutex()
{
  return *reinterpret_cast< ksys::InterlockedMutex *>(mutex_);
}
//---------------------------------------------------------------------------
inline ksys::Array< API::ThreadList> & API::threadList()
{
  return *reinterpret_cast< ksys::Array< ThreadList> *>(threadList_);
}
//---------------------------------------------------------------------------
} // namespace mycpp
//---------------------------------------------------------------------------
#endif /* _myintf_H_ */
