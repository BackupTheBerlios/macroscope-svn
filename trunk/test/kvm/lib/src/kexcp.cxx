/*-
 * Copyright 2009 Guram Dukashvili
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
#include "kboot.h"
#include "kobject.h"
#include "kmalloc.h"
#include "kstring.h"
#include "karray.h"
#include "kexcp.h"
//---------------------------------------------------------------------------
namespace kvm {
//---------------------------------------------------------------------------
Exception::~Exception()
{
}
//---------------------------------------------------------------------------
Exception::Exception()
{
}
//---------------------------------------------------------------------------
int32_t Exception::mapWinErr2Std(int32_t e)
{
  switch( e ){
#if defined(__WIN32__) || defined(__WIN64__)
    case ERROR_ALREADY_EXISTS     : e = EEXIST; break;
    case ERROR_NOT_FOUND          : e = ENOENT; break;
    case ERROR_NOT_ENOUGH_MEMORY  : e = ENOMEM; break;
#endif
    default:;
  }
  return e;
}
//---------------------------------------------------------------------------
int32_t Exception::mapStdErr2Win(int32_t e)
{
  switch( e ){
#if defined(__WIN32__) || defined(__WIN64__)
    case EEXIST : e = ERROR_ALREADY_EXISTS;     break;
    case ENOENT : e = ERROR_NOT_FOUND;          break;
    case ENOMEM : e = ERROR_NOT_ENOUGH_MEMORY;  break;
#endif
    default:;
  }
  return e;
}
//---------------------------------------------------------------------------
Exception::Exception(int32_t code,const String & what,bool winError)
{
  errors_.add(Error(winError ? mapStdErr2Win(code) : code,what));
}
//---------------------------------------------------------------------------
int32_t & Exception::code(uintptr_t i) const
{
  assert( i < errors_.count() );
  return errors_[i].code_;
}
//---------------------------------------------------------------------------
const String & Exception::what(uintptr_t i) const
{
  assert( i < errors_.count() );
  return errors_[i].what_;
}
//---------------------------------------------------------------------------
} // namespace kvm
//---------------------------------------------------------------------------
