/*-
 * Copyright 2005-2009 Guram Dukashvili
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
#ifndef mycppH
#define mycppH

#include <adicpp/ksys.h>

#if HAVE_MYSQL_MYSQL_H
#include <mysql/mysql.h>
#if MYSQL_VERSION_ID >= 60000
#include <mysql/my_config.h>
#undef HAVE_FENV_H
#include <mysql/my_global.h>
#endif
#include <mysql/mysqld_error.h>
#include <mysql/my_pthread.h>
#include <mysql/errmsg.h>
#elif HAVE_MYSQL_H
#include <mysql.h>
#if MYSQL_VERSION_ID >= 60000
#include <my_config.h>
#undef HAVE_FENV_H
#include <my_global.h>
#endif
#include <mysqld_error.h>
#include <my_pthread.h>
#include <errmsg.h>
#else
#include <adicpp/myapi/mysql.h>
#include <adicpp/myapi/mysqld_error.h>
#include <adicpp/myapi/errmsg.h>
//#include <adicpp/myapi/my_global.h>
//#include <adicpp/myapi/my_sys.h>
#endif

#ifndef ER_DUP_ENTRY_WITH_KEY_NAME
#define ER_DUP_ENTRY_WITH_KEY_NAME 1582
#endif

#include <adicpp/myintf.h>
#include <adicpp/mydb.h>
#include <adicpp/myexcpt.h>
#include <adicpp/myst.h>
#include <adicpp/mytr.h>
#include <adicpp/myexcept.h>

namespace mycpp {

extern void initialize();
extern void cleanup();

}

#endif /* mycppH */
