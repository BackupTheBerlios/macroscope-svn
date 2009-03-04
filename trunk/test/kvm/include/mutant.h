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
//------------------------------------------------------------------------------
#ifndef mutantH
#define mutantH
//------------------------------------------------------------------------------
namespace kvm {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class Mutant {
  public:

    enum Type {
      mtNull,
      mtInt,
      mtFloat,
      mtTime,
      mtString,
      mtBinary
    };

    ~Mutant() { clear(); }
    Mutant() : type_(mtNull) {}

    Mutant(const Mutant & v) : type_(mtNull) { operator = (v); }


    Mutant operator = (const Mutant & v) const;

    Mutant operator + (const Mutant & v) const;
    Mutant operator - (const Mutant & v) const;
    Mutant operator * (const Mutant & v) const;
    Mutant operator / (const Mutant & v) const;

    Mutant & clear();

  protected:

    union {
      char raw_[1];
      intmax_t int_;
      ldouble float_;
      const char * str_;
    };

    Type type_;

  private:
};
//------------------------------------------------------------------------------
} // namespace kvm
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
