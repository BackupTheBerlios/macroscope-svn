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
#include <adicpp/adicpp.h>
//---------------------------------------------------------------------------
namespace adicpp {
//---------------------------------------------------------------------------
#if defined(_MSC_VER) // for avoid linker warning
int __ST_CPP_AVOID_MSC_VER_LINKER_WARN__;
#endif
//---------------------------------------------------------------------------
ksys::Mutant Statement::sum(uintptr_t fieldNum,uintptr_t sRowNum,uintptr_t eRowNum)
{
  intptr_t srow = rowIndex();
  uintptr_t row, k;
  
  ksys::Mutant summa = 0;
  if( srow >= 0 ){
    if( sRowNum > eRowNum ) ksys::xchg(sRowNum,eRowNum);
    k = rowCount();
    if( k >= eRowNum ) k = eRowNum + 1;
    for( row = sRowNum; row < k; row++ ){
      selectRow(row);
      if( summa.type() == ksys::mtFloat || fieldType(fieldNum) == ftFloat || fieldType(fieldNum) == ftDouble )
        summa = (ldouble) summa + (ldouble) valueAsMutant(fieldNum);
      else
        summa = intmax_t(summa) + intmax_t(valueAsMutant(fieldNum));
    }
    selectRow(srow);
  }
  return summa;
}
//---------------------------------------------------------------------------
ksys::Mutant Statement::sum(const utf8::String & fieldName,uintptr_t sRowNum,uintptr_t eRowNum)
{
  uintptr_t row, k;
  intptr_t srow = rowIndex(), fieldNum = fieldIndex(fieldName);
  ksys::Mutant summa = 0;
  if( srow >= 0 && fieldNum >= 0 ){
    if( sRowNum > eRowNum ) ksys::xchg(sRowNum,eRowNum);
    k = rowCount();
    if( k >= eRowNum ) k = eRowNum + 1;
    for( row = sRowNum; row < k; row++ ){
      selectRow(row);
      if( summa.type() == ksys::mtFloat || fieldType(fieldNum) == ftFloat || fieldType(fieldNum) == ftDouble )
        summa = (ldouble) summa + (ldouble) valueAsMutant(fieldNum);
      else
        summa = intmax_t(summa) + intmax_t(valueAsMutant(fieldNum));
    }
    selectRow(srow);
  }
  return summa;
}
//---------------------------------------------------------------------------
} // namespace adicpp
//---------------------------------------------------------------------------
