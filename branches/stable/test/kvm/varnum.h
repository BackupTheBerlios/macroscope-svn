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
  friend class VarNumber;
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
#if !HAVE_INTPTR_T_AS_INT
    VarInteger(int v);
    VarInteger(unsigned int v);
#endif
    VarInteger(intptr_t v);
    VarInteger(uintptr_t v);
#if !HAVE_INTMAX_T_AS_INT && !HAVE_INTMAX_T_AS_INTPTR_T
    VarInteger(intmax_t v);
    VarInteger(uintmax_t v);
#endif

    void * operator new (size_t sz){ return alloc(sz); }
    void operator delete (void * p){ free(p); }

    VarInteger & operator = (const VarInteger & v);
#if !HAVE_INTPTR_T_AS_INT
    VarInteger & operator = (int v) { return operator = (intptr_t(v)); }
    VarInteger & operator = (unsigned int v) { return operator = (uintptr_t(v)); }
#endif
    VarInteger & operator = (intptr_t v);
    VarInteger & operator = (uintptr_t v);
#if !HAVE_INTMAX_T_AS_INT && !HAVE_INTMAX_T_AS_INTPTR_T
    VarInteger & operator = (intmax_t v);
    VarInteger & operator = (uintmax_t v);
#endif

#if !HAVE_INTPTR_T_AS_INT
    VarInteger operator << (int shift) const { return operator << (intptr_t(shift)); }
    VarInteger operator << (unsigned int shift) const { return operator << (uintptr_t(shift)); }
    VarInteger operator >> (int shift) const { return operator >> (intptr_t(shift)); }
    VarInteger operator >> (unsigned int shift) const { return operator >> (uintptr_t(shift)); }

    VarInteger & operator <<= (int shift){ return operator <<= (intptr_t(shift)); }
    VarInteger & operator <<= (unsigned int shift) { return operator <<= (uintptr_t(shift)); }
    VarInteger & operator >>= (int shift) { return operator >>= (intptr_t(shift)); }
    VarInteger & operator >>= (unsigned int shift) { return operator >>= (uintptr_t(shift)); }
#endif

    VarInteger operator << (intptr_t shift) const { return operator << (uintptr_t(shift)); }
    VarInteger operator << (uintptr_t shift) const;
    VarInteger operator >> (intptr_t shift) const { return operator >> (uintptr_t(shift)); }
    VarInteger operator >> (uintptr_t shift) const;

    VarInteger & operator <<= (intptr_t shift) { return operator <<= (uintptr_t(shift)); }
    VarInteger & operator <<= (uintptr_t shift) { return *this = operator << (shift); }
    VarInteger & operator >>= (intptr_t shift) { return operator >>= (uintptr_t(shift)); }
    VarInteger & operator >>= (uintptr_t shift) { return *this = operator >> (shift); }

    VarInteger & operator ++ (int); // postfix form
    VarInteger & operator ++ ();
    VarInteger & operator -- (int);
    VarInteger & operator -- ();

    VarInteger operator + (const VarInteger & v) const;
    VarInteger operator - (const VarInteger & v) const;
    VarInteger operator * (const VarInteger & v) const;
    VarInteger operator / (const VarInteger & v) const { return div(v); }
    VarInteger operator % (const VarInteger & v) const;

    VarInteger & operator += (const VarInteger & v) { return *this = operator + (v); }
    VarInteger & operator -= (const VarInteger & v) { return *this = operator - (v); }
    VarInteger & operator *= (const VarInteger & v) { return *this = operator * (v); }
    VarInteger & operator /= (const VarInteger & v) { return *this = operator / (v); }
    VarInteger & operator %= (const VarInteger & v) { return *this = operator % (v); }

    bool operator >  (const VarInteger & v) const { return compare(v) >  0; }
    bool operator >= (const VarInteger & v) const { return compare(v) >= 0; }
    bool operator <  (const VarInteger & v) const { return compare(v) <  0; }
    bool operator <= (const VarInteger & v) const { return compare(v) <= 0; }
    bool operator == (const VarInteger & v) const { return compare(v) == 0; }
    bool operator != (const VarInteger & v) const { return compare(v) != 0; }

    bool operator ! () const { return isZero(); }
    VarInteger operator - () const;

#if !HAVE_INTPTR_T_AS_INT
    int asInt() const { return (int) asIntPtrT(); }
    unsigned int asUInt() const { return (unsigned int) asUIntPtrT(); }
#endif
    intptr_t asIntPtrT() const;
    uintptr_t asUIntPtrT() const;
#if !HAVE_INTMAX_T_AS_INT && !HAVE_INTMAX_T_AS_INTPTR_T
    intmax_t asIntMaxT() const;
    uintmax_t asUIntMaxT() const;
#endif

    mint_t compare(const VarInteger & v) const;
    VarInteger nodNok(const VarInteger & divider,VarInteger * nok = NULL) const;
    VarInteger div(const VarInteger & divider,VarInteger * remainder = NULL) const;
    VarInteger abs() const;

    VarInteger pow(uintptr_t pow) const;
    VarInteger pow10(uintptr_t pow = 1) const;
    VarInteger mul10(uintptr_t pow = 1) const;

    uintptr_t print(char * s = NULL,uintptr_t pow = 10) const;

    mint_t sign() const { return mint_t(container_->data_[container_->count_ - 1]) >> (sizeof(mint_t) * 8 - 1); }
    bool isNeg() const { return sign() < 0; }
    bool isZero() const;
    bool isOne() const;

    struct Container {
        void cleanup()
        {
          VarInteger::free(data_);
          VarInteger::free(this);
        }

        static Container * initialize(void * data = NULL,uintptr_t count = 0,ilock_t ref = 0);

        void addRef(){ interlockedIncrement(ref_,1); }
        void remRef(){ if( interlockedIncrement(ref_,-1) == 1 ) cleanup(); }

        mutable mword_t * data_;
        mutable uintptr_t count_;
        mutable ilock_t ref_;
    };
    friend struct Container;
  protected:
    mutable SPRC<Container> container_;

    VarInteger(void * data,uintptr_t count);
    VarInteger(Container * container);

    mdint_t sign2() const { return sign(); }
    intptr_t getFirstSignificantBitIndex() const;

    static void * alloc(uintptr_t size) { return realloc(NULL,size); }
    static void * realloc(void * p,uintptr_t size);
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

    VarNumber(const VarInteger & v);
    VarNumber(const VarInteger & numerator,const VarInteger & denominator);
    VarNumber(const VarNumber & v);

    VarNumber & operator = (const VarInteger & v);
    VarNumber & operator = (const VarNumber & v);

    VarNumber operator + (const VarInteger & v) const;
    VarNumber operator + (const VarNumber & v) const;
    VarNumber operator - (const VarInteger & v) const;
    VarNumber operator - (const VarNumber & v) const;
    VarNumber operator * (const VarInteger & v) const;
    VarNumber operator * (const VarNumber & v) const;
    VarNumber operator / (const VarInteger & v) const;
    VarNumber operator / (const VarNumber & v) const;

    VarNumber & operator += (const VarInteger & v) { return *this = operator + (v); }
    VarNumber & operator += (const VarNumber & v) { return *this = operator + (v); }
    VarNumber & operator -= (const VarInteger & v) { return *this = operator - (v); }
    VarNumber & operator -= (const VarNumber & v) { return *this = operator - (v); }
    VarNumber & operator *= (const VarInteger & v) { return *this = operator * (v); }
    VarNumber & operator *= (const VarNumber & v) { return *this = operator * (v); }
    VarNumber & operator /= (const VarInteger & v) { return *this = operator / (v); }
    VarNumber & operator /= (const VarNumber & v) { return *this = operator / (v); }

    bool operator >  (const VarNumber & v) const { return compare(v) >  0; }
    bool operator >= (const VarNumber & v) const { return compare(v) >= 0; }
    bool operator <  (const VarNumber & v) const { return compare(v) <  0; }
    bool operator <= (const VarNumber & v) const { return compare(v) <= 0; }
    bool operator == (const VarNumber & v) const { return compare(v) == 0; }
    bool operator != (const VarNumber & v) const { return compare(v) != 0; }

    bool operator ! () const { return isZero(); }
    VarNumber operator - () const;

    VarInteger::mint_t compare(const VarNumber & v) const;
    VarNumber simplify() const;
    VarInteger::mint_t sign() const { return numerator_.sign(); }
    bool isNeg() const { return sign() < 0; }
    bool isZero() const { return numerator_.isZero() && denominator_.isOne(); }
    bool isOne() const { return numerator_.isOne() && denominator_.isOne(); }

    VarNumber sqrt(uintptr_t sqrtpow = 2,uintptr_t iter = 32) const;
    VarNumber abs() const;
    VarNumber pow(intptr_t pow) const;
    VarNumber sal(uintptr_t v) const;
    VarNumber sar(uintptr_t v) const;

    enum FormatMode {
      fmtLeadZeros = 1,
      fmtLeftJustifies = 2,
      fmtSign = 4,
      fmtEatRightZeros = 8,
      fmtEatFixedPoint = 16,
      fmtExponent = 32,
      fmtDefault = 0
    };

    uintptr_t print(char * s = NULL,uintptr_t width = 0,uintptr_t precision = 0,uintptr_t fmt = fmtDefault) const;
  protected:
    mutable VarInteger numerator_;     // числитель
    mutable VarInteger denominator_;   // знаменатель
  private:
};
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
