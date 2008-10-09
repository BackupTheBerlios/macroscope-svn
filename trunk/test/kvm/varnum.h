/*-
 * Copyright 2008 Guram Dukashvili
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
#ifndef _varnumH_
#define _varnumH_
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class VarInteger {
  public:
#if SIZEOF_INTMAX_T == 8
    typedef int32_t mint_t;
    typedef uint32_t mword_t;
    typedef int64_t mdint_t;
    typedef uint64_t mdword_t;
#elif SIZEOF_INTMAX_T == 16
    typedef int64_t mdint_t;
    typedef uint64_t mword_t;
    typedef intmax_t mdint_t;
    typedef uintmax_t mdword_t;
#endif

    virtual ~VarInteger();
    VarInteger();

    VarInteger(const VarInteger & v);
    VarInteger(int v);
    VarInteger(unsigned int v);
    VarInteger(intptr_t v);
    VarInteger(uintptr_t v);

    void * operator new (size_t sz){ return alloc(sz); }
    void operator delete (void * p){ free(p); }

    VarInteger & operator = (const VarInteger & v);
    VarInteger & operator = (int v);
    VarInteger & operator = (unsigned int v);
    VarInteger & operator = (intptr_t v);
    VarInteger & operator = (uintptr_t v);

    VarInteger operator << (intptr_t shift) const;
    VarInteger operator << (uintptr_t shift) const;
    VarInteger operator >> (intptr_t shift) const;
    VarInteger operator >> (uintptr_t shift) const;

    VarInteger operator + (const VarInteger & v) const;
    VarInteger operator - (const VarInteger & v) const;
    VarInteger operator * (const VarInteger & v) const;
    VarInteger operator / (const VarInteger & v) const;

    uintptr_t print(const char * s,uintptr_t pow = 10) const;

    mint_t sign() const { return count_ > 0 ? mint_t(data_[count_ - 1]) >> (sizeof(mint_t) * 8 - 1) : 0; }

  protected:
    mutable mword_t * data_;
    mutable uintptr_t count_;
    mutable uintptr_t ref_;

    VarInteger(mword_t * data,uintptr_t count);
    static mword_t * alloc(uintptr_t size) { return realloc(NULL,size); }
    static mword_t * realloc(mword_t * p,uintptr_t size);
    static void free(void * p);
  private:
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class VarNumber {
  public:
    virtual ~VarNumber();
    VarNumber();

    VarNumber(const VarNumber & v);
    VarNumber & operator = (const VarNumber & v);

  protected:
    mutable VarInteger numerator_; // числитель
    mutable VarInteger denominator_;   // знаменатель
  private:
};
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
