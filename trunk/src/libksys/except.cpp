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
#include <adicpp/ksys.h>
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
Exception::~Exception()
{
}
//---------------------------------------------------------------------------
Exception::Exception(int32_t code, const utf8::String what) : refCount_(0)
{
  codes_.add(code);
  whats_.add(what);
}
//---------------------------------------------------------------------------
bool Exception::isFatalError() const
{
  return false;
}
//---------------------------------------------------------------------------
const Exception & Exception::writeStdError() const
{
  for( uintptr_t i = 0; i < whats_.count(); i++ ){
    if( codes_[0] == 0 ) continue;
    stdErr.debug(9,utf8::String::Stream() <<
      codes_[i] << " " << strError(codes_[i]) << " " << whats_[i] << "\n"
    );
  }
  return *this;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------

