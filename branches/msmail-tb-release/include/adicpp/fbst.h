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
#ifndef _fbst_H_
#define _fbst_H_
//---------------------------------------------------------------------------
namespace fbcpp {

//---------------------------------------------------------------------------
int64_t         iscTimeStamp2Time(const ISC_TIMESTAMP & stamp);
struct timeval  iscTimeStamp2Timeval(const ISC_TIMESTAMP & stamp);
struct tm       iscTimeStamp2tm(const ISC_TIMESTAMP & stamp);
ISC_TIMESTAMP   time2IscTimeStamp(int64_t stamp);
ISC_TIMESTAMP   timeval2IscTimeStamp(const struct timeval & stamp);
ISC_TIMESTAMP   tm2IscTimeStamp(struct tm stamp);
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class XSQLDAHolder {
  private:
    XSQLDA *  sqlda_;
  public:
                          XSQLDAHolder();
                          ~XSQLDAHolder();

    XSQLDAHolder &        resize(long n);
    // properties
    XSQLDA *              sqlda();
    const XSQLDA * const  sqlda() const;
    uintptr_t             count();
};
//---------------------------------------------------------------------------
inline XSQLDA * XSQLDAHolder::sqlda()
{
  return sqlda_;
}
//---------------------------------------------------------------------------
inline const XSQLDA * const XSQLDAHolder::sqlda() const
{
  return sqlda_;
}
//---------------------------------------------------------------------------
inline uintptr_t XSQLDAHolder::count()
{
  return sqlda_->sqln;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DSQLParam {
    friend class DSQLParams;
  private:
  protected:
    short sqlind_;
    short sqltype_;
    char  sqlscale_;
    bool  changed_;

    virtual ksys::Mutant  getMutant();
    virtual DSQLParam &   setMutant(const ksys::Mutant & value);
    virtual utf8::String  getString();
    virtual DSQLParam &   setString(const utf8::String & value);
  public:
                          DSQLParam();
    virtual               ~DSQLParam();
};
//---------------------------------------------------------------------------
inline DSQLParam::DSQLParam()
  : sqlscale_(0),
    changed_(false)
{
}
//---------------------------------------------------------------------------
inline DSQLParam::~DSQLParam()
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DSQLParamScalar : public DSQLParam {
    friend class DSQLParams;
  private:
  protected:
    union {
        int64_t       bigInt_;
        float         float_;
        double        double_;
        ISC_TIMESTAMP timeStamp_;
    };

    ksys::Mutant  getMutant();
    DSQLParam &   setMutant(const ksys::Mutant & value);
    utf8::String  getString();
    DSQLParam &   setString(const utf8::String & value);
  public:
                  DSQLParamScalar();
    virtual       ~DSQLParamScalar();
};
//---------------------------------------------------------------------------
class DSQLParamText : public DSQLParam {
    friend class DSQLParams;
  private:
  protected:
    utf8::String  text_;

    ksys::Mutant  getMutant();
    DSQLParam &   setMutant(const ksys::Mutant & value);
    utf8::String  getString();
    DSQLParam &   setString(const utf8::String & Value);
  public:
                  DSQLParamText();
    virtual       ~DSQLParamText();
};
//---------------------------------------------------------------------------
class DSQLParamArray : public DSQLParam {
  friend class DSQLParams;
  private:
    DSQLParamArray(const DSQLParamArray &){}
    void operator = (const DSQLParamArray &){}
  protected:
    DSQLStatement   * statement_;
    ISC_QUAD        id_;
    ISC_ARRAY_DESC  desc_;
    void *          data_;
    ISC_LONG        dataSize_;
    ISC_LONG        elementSize_;
    uint16_t        dimElements_[sizeof(ISC_ARRAY_DESC().array_desc_bounds) / sizeof(ISC_ARRAY_DESC().array_desc_bounds[0])];

    DSQLParamArray &        clear();
    DSQLParamArray &        checkData();
    DSQLParamArray &        checkDim(bool inRange);
    bool                    isDimInBound(intptr_t dim, intptr_t i);
    DSQLParamArray &        setDataFromMutant(uintptr_t absIndex, const ksys::Mutant & value);
    ksys::Mutant            getMutantFromArray(uintptr_t absIndex);
    DSQLParamArray &        putSlice();
  public:
    DSQLParamArray(DSQLStatement & statement);
    virtual                 ~DSQLParamArray();

    DSQLParamArray &        putElement(intptr_t i, const ksys::Mutant & value);
    DSQLParamArray &        putElement(intptr_t i, intptr_t j, const ksys::Mutant & value);
    DSQLParamArray &        putElement(intptr_t a, intptr_t i, intptr_t j, const ksys::Mutant & value);
    DSQLParamArray &        putElement(intptr_t a, intptr_t b, intptr_t i, intptr_t j, const ksys::Mutant & value);
    DSQLParamArray &        putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t i, intptr_t j, const ksys::Mutant & value);
    DSQLParamArray &        putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t i, intptr_t j, const ksys::Mutant & value);
    DSQLParamArray &        putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t i, intptr_t j, const ksys::Mutant & value);
    DSQLParamArray &        putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t i, intptr_t j, const ksys::Mutant & value);
    DSQLParamArray &        putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t i, intptr_t j, const ksys::Mutant & value);
    DSQLParamArray &        putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t i, intptr_t j, const ksys::Mutant & value);
    DSQLParamArray &        putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t i, intptr_t j, const ksys::Mutant & value);
    DSQLParamArray &        putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t i, intptr_t j, const ksys::Mutant & value);
    DSQLParamArray &        putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t i, intptr_t j, const ksys::Mutant & value);
    DSQLParamArray &        putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t i, intptr_t j, const ksys::Mutant & value);
    DSQLParamArray &        putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t o, intptr_t i, intptr_t j, const ksys::Mutant & value);
    DSQLParamArray &        putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t o, intptr_t p, intptr_t i, intptr_t j, const ksys::Mutant & value);

    ksys::Mutant            getElement(intptr_t i);
    ksys::Mutant            getElement(intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t o, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t o, intptr_t p, intptr_t i, intptr_t j);

    DSQLParamArray &        putElByAbsIndex(uintptr_t i, const ksys::Mutant & value);
    ksys::Mutant            getElByAbsIndex(uintptr_t i);

    const ISC_ARRAY_DESC &  desc() const;
};
//---------------------------------------------------------------------------
inline const ISC_ARRAY_DESC & DSQLParamArray::desc() const
{
  return desc_;
}
//---------------------------------------------------------------------------
inline bool DSQLParamArray::isDimInBound(intptr_t dim, intptr_t i)
{
  return i >= desc_.array_desc_bounds[dim].array_bound_lower && i <= desc_.array_desc_bounds[dim].array_bound_upper;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElByAbsIndex(uintptr_t i, const ksys::Mutant & value)
{
  setDataFromMutant(i, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElement(intptr_t i, const ksys::Mutant & value)
{
  checkDim(isDimInBound(0, i)).setDataFromMutant(i, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElement(intptr_t i, intptr_t j, const ksys::Mutant & value)
{
  checkDim(isDimInBound(0, i) && isDimInBound(1, j));
  setDataFromMutant(i * dimElements_[0] + j, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElement(intptr_t a, intptr_t i, intptr_t j, const ksys::Mutant & value)
{
  checkDim(isDimInBound(0, a) && isDimInBound(1, i) && isDimInBound(2, j));
  setDataFromMutant(a * dimElements_[0] + i * dimElements_[1] + j, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElement(intptr_t a, intptr_t b, intptr_t i, intptr_t j, const ksys::Mutant & value)
{
  checkDim(isDimInBound(0, a) && isDimInBound(1, b) && isDimInBound(2, i) && isDimInBound(3, j));
  setDataFromMutant(a * dimElements_[0] + b * dimElements_[1] + i * dimElements_[2] + j, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t i, intptr_t j, const ksys::Mutant & value)
{
  checkDim(isDimInBound(0, a) && isDimInBound(1, b) && isDimInBound(2, c) && isDimInBound(3, i) && isDimInBound(4, j));
  setDataFromMutant(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + i * dimElements_[4] + j, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t i, intptr_t j, const ksys::Mutant & value)
{
  checkDim(isDimInBound(0, a) && isDimInBound(1, b) && isDimInBound(2, c) && isDimInBound(3, d) && isDimInBound(4, i) && isDimInBound(5, j));
  setDataFromMutant(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + i * dimElements_[5] + j, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t i, intptr_t j, const ksys::Mutant & value)
{
  checkDim(isDimInBound(0, a) && isDimInBound(1, b) && isDimInBound(2, c) && isDimInBound(3, d) && isDimInBound(4, e) && isDimInBound(5, i) && isDimInBound(6, j));
  setDataFromMutant(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + i * dimElements_[5] + j, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t i, intptr_t j, const ksys::Mutant & value)
{
  checkDim(isDimInBound(0, a) && isDimInBound(1, b) && isDimInBound(2, c) && isDimInBound(3, d) && isDimInBound(4, e) && isDimInBound(5, f) && isDimInBound(6, i) && isDimInBound(7, j));
  setDataFromMutant(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + i * dimElements_[7] + j, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t i, intptr_t j, const ksys::Mutant & value)
{
  checkDim(isDimInBound(0, a) && isDimInBound(1, b) && isDimInBound(2, c) && isDimInBound(3, d) && isDimInBound(4, e) && isDimInBound(5, f) && isDimInBound(6, g) && isDimInBound(7, i) && isDimInBound(8, j));
  setDataFromMutant(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + i * dimElements_[8] + j, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t i, intptr_t j, const ksys::Mutant & value)
{
  checkDim(isDimInBound(0, a) && isDimInBound(1, b) && isDimInBound(2, c) && isDimInBound(3, d) && isDimInBound(4, e) && isDimInBound(5, f) && isDimInBound(6, g) && isDimInBound(7, h) && isDimInBound(8, i) && isDimInBound(9, j));
  setDataFromMutant(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + i * dimElements_[9] + j, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t i, intptr_t j, const ksys::Mutant & value)
{
  checkDim(isDimInBound(0, a) && isDimInBound(1, b) && isDimInBound(2, c) && isDimInBound(3, d) && isDimInBound(4, e) && isDimInBound(5, f) && isDimInBound(6, g) && isDimInBound(7, h) && isDimInBound(8, k) && isDimInBound(9, i) && isDimInBound(10, j));
  setDataFromMutant(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + i * dimElements_[10] + j, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t i, intptr_t j, const ksys::Mutant & value)
{
  checkDim(isDimInBound(0, a) && isDimInBound(1, b) && isDimInBound(2, c) && isDimInBound(3, d) && isDimInBound(4, e) && isDimInBound(5, f) && isDimInBound(6, g) && isDimInBound(7, h) && isDimInBound(8, k) && isDimInBound(9, l) && isDimInBound(10, i) && isDimInBound(11, j));
  setDataFromMutant(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + l * dimElements_[10] + i * dimElements_[11] + j, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t i, intptr_t j, const ksys::Mutant & value)
{
  checkDim(isDimInBound(0, a) && isDimInBound(1, b) && isDimInBound(2, c) && isDimInBound(3, d) && isDimInBound(4, e) && isDimInBound(5, f) && isDimInBound(6, g) && isDimInBound(7, h) && isDimInBound(8, k) && isDimInBound(9, l) && isDimInBound(10, m) && isDimInBound(11, i) && isDimInBound(12, j));
  setDataFromMutant(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + l * dimElements_[10] + m * dimElements_[11] + i * dimElements_[12] + j, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t i, intptr_t j, const ksys::Mutant & value)
{
  checkDim(isDimInBound(0, a) && isDimInBound(1, b) && isDimInBound(2, c) && isDimInBound(3, d) && isDimInBound(4, e) && isDimInBound(5, f) && isDimInBound(6, g) && isDimInBound(7, h) && isDimInBound(8, k) && isDimInBound(9, l) && isDimInBound(10, m) && isDimInBound(11, n) && isDimInBound(12, i) && isDimInBound(13, j));
  setDataFromMutant(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + l * dimElements_[10] + m * dimElements_[11] + n * dimElements_[12] + i * dimElements_[13] + j, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t o, intptr_t i, intptr_t j, const ksys::Mutant & value)
{
  checkDim(isDimInBound(0, a) && isDimInBound(1, b) && isDimInBound(2, c) && isDimInBound(3, d) && isDimInBound(4, e) && isDimInBound(5, f) && isDimInBound(6, g) && isDimInBound(7, h) && isDimInBound(8, k) && isDimInBound(9, l) && isDimInBound(10, m) && isDimInBound(11, n) && isDimInBound(12, o) && isDimInBound(13, i) && isDimInBound(14, j));
  setDataFromMutant(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + l * dimElements_[10] + m * dimElements_[11] + n * dimElements_[12] + o * dimElements_[13] + i * dimElements_[14] + j, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLParamArray::putElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t o, intptr_t p, intptr_t i, intptr_t j, const ksys::Mutant & value)
{
  checkDim(isDimInBound(0, a) && isDimInBound(1, b) && isDimInBound(2, c) && isDimInBound(3, d) && isDimInBound(4, e) && isDimInBound(5, f) && isDimInBound(6, g) && isDimInBound(7, h) && isDimInBound(8, k) && isDimInBound(9, l) && isDimInBound(10, m) && isDimInBound(11, n) && isDimInBound(12, o) && isDimInBound(13, p) && isDimInBound(14, i) && isDimInBound(15, j));
  setDataFromMutant(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + l * dimElements_[10] + m * dimElements_[11] + n * dimElements_[12] + o * dimElements_[13] + p * dimElements_[14] + i * dimElements_[15] + j, value);
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElement(intptr_t i)
{
  checkDim(isDimInBound(0, i));
  return getMutantFromArray(i);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElement(intptr_t i, intptr_t j)
{
  checkDim(isDimInBound(0, i) && isDimInBound(1, j));
  return getMutantFromArray(i * dimElements_[0] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElement(intptr_t a, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + i * dimElements_[1] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElement(intptr_t a, intptr_t b, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + i * dimElements_[2] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + i * dimElements_[4] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + i * dimElements_[5] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + i * dimElements_[6] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + i * dimElements_[7] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + i * dimElements_[8] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + i * dimElements_[9] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + i * dimElements_[10] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + l * dimElements_[10] + i * dimElements_[11] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + l * dimElements_[10] + m * dimElements_[11] + i * dimElements_[12] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + l * dimElements_[10] + m * dimElements_[11] + n * dimElements_[12] + i * dimElements_[13] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t o, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + l * dimElements_[10] + m * dimElements_[11] + n * dimElements_[12] + o * dimElements_[13] + i * dimElements_[14] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t o, intptr_t p, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + l * dimElements_[10] + m * dimElements_[11] + n * dimElements_[12] + o * dimElements_[13] + p * dimElements_[14] + i * dimElements_[15] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLParamArray::getElByAbsIndex(uintptr_t i)
{
  return getMutantFromArray(i);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DSQLParamBlob : public DSQLParam {
  friend class DSQLParams;
  private:
    DSQLParamBlob(const DSQLParamBlob &){}
    void operator = (const DSQLParamBlob &){}
  protected:
    DSQLStatement   * statement_;
    ISC_QUAD        id_;
    isc_blob_handle handle_;
    ISC_BLOB_DESC   desc_;

    DSQLParam &     setString(const utf8::String & value);
  public:
    DSQLParamBlob(DSQLStatement & statement);
    virtual         ~DSQLParamBlob();

    DSQLParamBlob & createBlob();
    DSQLParamBlob & closeBlob();
    DSQLParamBlob & cancelBlob();
    DSQLParamBlob & writeBuffer(const void * buf, uintptr_t size);
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DSQLParams {
  friend class DSQLStatement;
  public:
    DSQLParams(DSQLStatement & statement);
    ~DSQLParams();

    uintptr_t         count();
    // access methods
    bool              isNull(uintptr_t i);
    DSQLParams &      setNull(uintptr_t i);
    bool              isNull(const utf8::String & paramName);
    DSQLParams &      setNull(const utf8::String & paramName);

    ksys::Mutant      asMutant(uintptr_t i);
    DSQLParams &      asMutant(uintptr_t i, const ksys::Mutant & value);
    utf8::String      asString(uintptr_t i);
    DSQLParams &      asString(uintptr_t i, const utf8::String & value);
    DSQLParamBlob &   asBlob(uintptr_t i);
    DSQLParamBlob &   asBlob(const utf8::String & name);
    DSQLParamArray &  asArray(uintptr_t i);
    DSQLParamArray &  asArray(const utf8::String & name);

    ksys::Mutant      asMutant(const utf8::String & paramName);
    DSQLParams &      asMutant(const utf8::String & paramName, const ksys::Mutant & value);
    utf8::String      asString(const utf8::String & paramName);
    DSQLParams &      asString(const utf8::String & paramName, const utf8::String & value);
  protected:
  private:
    DSQLParams(const DSQLParams &){}
    void operator = (const DSQLParams &){}

    DSQLStatement                                                       * statement_;
    XSQLDAHolder                                                        sqlda_;
    ksys::HashedObjectList< utf8::String,DSQLParam>                     params_;
    ksys::Array< ksys::HashedObjectListItem< utf8::String,DSQLParam> *> indexToParam_;
    utf8::String                                                        tempString_;
    ISC_TIMESTAMP                                                       tempStamp_;
    bool                                                                changed_;

    enum BindType { preBind, postBind, preBindAfterExecute };

    DSQLParams &                                          bind(BindType bindType);
    DSQLParams &                                          resetChanged();
    DSQLParams &                                          removeUnchanged();
    ksys::HashedObjectListItem< utf8::String,DSQLParam> * add(const utf8::String & paramName);
    DSQLParams &                                          checkParamIndex(uintptr_t i);
    DSQLParam *                                           checkParamName(const utf8::String & paramName);
    DSQLParamText *                                       castParamToText(DSQLParam * param);
    DSQLParamScalar *                                     castParamToScalar(DSQLParam * param);
    DSQLParamArray *                                      castParamToArray(DSQLParam * param, XSQLVAR & v);
    DSQLParamBlob *                                       castParamToBlob(DSQLParam * param);
};
//---------------------------------------------------------------------------
inline uintptr_t DSQLParams::count()
{
  return params_.count();
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DSQLValue {
    friend class DSQLValues;
  private:
  protected:
    short sqltype_;
    short sqlind_;

    virtual ksys::Mutant  getMutant();
    virtual utf8::String  getString();
  public:
                          DSQLValue();
    virtual               ~DSQLValue();
};
//---------------------------------------------------------------------------
inline DSQLValue::DSQLValue()
{
}
//---------------------------------------------------------------------------
inline DSQLValue::~DSQLValue()
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DSQLValueText : public DSQLValue {
    friend class DSQLValues;
  private:
  protected:
    utf8::String  text_;

    ksys::Mutant  getMutant();
    utf8::String  getString();
  public:
                  DSQLValueText();
    virtual       ~DSQLValueText();
};
//---------------------------------------------------------------------------
class DSQLValueScalar : public DSQLValue {
    friend class DSQLValues;
  private:
  protected:
    union {
        int64_t       bigInt_;
        float         float_;
        double        double_;
        ISC_TIMESTAMP timeStamp_;
    };
    char  sqlscale_;

    ksys::Mutant  getMutant();
    utf8::String  getString();
  public:
                  DSQLValueScalar();
    virtual       ~DSQLValueScalar();
};
//---------------------------------------------------------------------------
class DSQLValueArray : public DSQLValue {
  friend class DSQLValues;
  private:
    DSQLValueArray(const DSQLValueArray &){}
    void operator = (const DSQLValueArray &){}
  protected:
    DSQLStatement   * statement_;
    ISC_QUAD        id_;
    ISC_ARRAY_DESC  desc_;
    void *          data_;
    ISC_LONG        dataSize_;
    ISC_LONG        elementSize_;
    uintptr_t       dimElements_[sizeof(ISC_ARRAY_DESC().array_desc_bounds) / sizeof(ISC_ARRAY_DESC().array_desc_bounds[0])];

    DSQLValueArray &        clear();
    DSQLValueArray &        checkData();
    DSQLValueArray &        checkDim(bool inRange);
    bool                    isDimInBound(intptr_t dim, intptr_t i);
    ksys::Mutant            getMutantFromArray(uintptr_t absIndex);
    DSQLValueArray &        getSlice();
  public:
    DSQLValueArray(DSQLStatement & statement);
    virtual                 ~DSQLValueArray();

    ksys::Mutant            getElement(intptr_t i);
    ksys::Mutant            getElement(intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t o, intptr_t i, intptr_t j);
    ksys::Mutant            getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t o, intptr_t p, intptr_t i, intptr_t j);

    ksys::Mutant            getElByAbsIndex(uintptr_t i);

    const ISC_ARRAY_DESC &  desc() const;
};
//---------------------------------------------------------------------------
inline const ISC_ARRAY_DESC & DSQLValueArray::desc() const
{
  return desc_;
}
//---------------------------------------------------------------------------
inline bool DSQLValueArray::isDimInBound(intptr_t dim, intptr_t i)
{
  return i >= desc_.array_desc_bounds[dim].array_bound_lower && i <= desc_.array_desc_bounds[dim].array_bound_upper;
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElement(intptr_t i)
{
  checkDim(isDimInBound(0, i));
  return getMutantFromArray(i);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElement(intptr_t i, intptr_t j)
{
  checkDim(isDimInBound(0, i) && isDimInBound(1, j));
  return getMutantFromArray(i * dimElements_[0] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElement(intptr_t a, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + i * dimElements_[1] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElement(intptr_t a, intptr_t b, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + i * dimElements_[2] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + i * dimElements_[4] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + i * dimElements_[5] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + i * dimElements_[6] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + i * dimElements_[7] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + i * dimElements_[8] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + i * dimElements_[9] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + i * dimElements_[10] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + l * dimElements_[10] + i * dimElements_[11] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + l * dimElements_[10] + m * dimElements_[11] + i * dimElements_[12] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + l * dimElements_[10] + m * dimElements_[11] + n * dimElements_[12] + i * dimElements_[13] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t o, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + l * dimElements_[10] + m * dimElements_[11] + n * dimElements_[12] + o * dimElements_[13] + i * dimElements_[14] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElement(intptr_t a, intptr_t b, intptr_t c, intptr_t d, intptr_t e, intptr_t f, intptr_t g, intptr_t h, intptr_t k, intptr_t l, intptr_t m, intptr_t n, intptr_t o, intptr_t p, intptr_t i, intptr_t j)
{
  return getMutantFromArray(a * dimElements_[0] + b * dimElements_[1] + c * dimElements_[3] + d * dimElements_[4] + e * dimElements_[5] + f * dimElements_[6] + g * dimElements_[7] + h * dimElements_[8] + k * dimElements_[9] + l * dimElements_[10] + m * dimElements_[11] + n * dimElements_[12] + o * dimElements_[13] + p * dimElements_[14] + i * dimElements_[15] + j);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValueArray::getElByAbsIndex(uintptr_t i)
{
  return getMutantFromArray(i);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DSQLValueBlob : public DSQLValue {
  friend class DSQLValues;
  private:
    DSQLValueBlob(const DSQLValueBlob &){}
    void operator = (const DSQLValueBlob &){}
  protected:
    DSQLStatement   * statement_;
    ISC_QUAD        id_;
    isc_blob_handle handle_;
    ISC_BLOB_DESC   desc_;

    utf8::String          getString();
  public:
    DSQLValueBlob(DSQLStatement & statement);
    virtual               ~DSQLValueBlob();

    DSQLValueBlob &       openBlob();
    DSQLValueBlob &       closeBlob();
    intptr_t              readBuffer(void * buf, uintptr_t size);

    const ISC_BLOB_DESC & desc();
};
//---------------------------------------------------------------------------
inline const ISC_BLOB_DESC & DSQLValueBlob::desc()
{
  return desc_;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DSQLValues {
  friend class DSQLStatement;
  private:
    DSQLValues(const DSQLValues &){}
    void operator = (const DSQLValues &){}

    DSQLStatement                                   * statement_;
    XSQLDAHolder                                    sqlda_;
    ksys::Vector< ksys::Vector< DSQLValue> >        rows_;
    intptr_t                                        row_;
    ksys::HashedObjectList< utf8::String,DSQLValue> valuesIndex_;

    DSQLValues &                clear();
    ksys::Vector< DSQLValue> *  bind();
    DSQLValues &                fillRow(ksys::Vector< DSQLValue> * row);
    void *                      getArrayPrepareMutant(DSQLValueArray * paramArray, ksys::Mutant & value, utf8::String & tempString, ISC_TIMESTAMP & tempStamp, ISC_LONG & sizeOfData);
    DSQLValues &                getArrayPosMutant(DSQLValueArray * paramArray, ksys::Mutant & value, utf8::String & tempString, ISC_TIMESTAMP & tempStamp);
  public:
                                DSQLValues(DSQLStatement & statement);
                                ~DSQLValues();

    uintptr_t                   rowCount();
    intptr_t                    rowIndex();
    bool                        fetch();
    DSQLValues &                fetchAll();
    DSQLValues &                selectRow(uintptr_t i);
    DSQLValues &                selectFirst();
    DSQLValues &                selectLast();

    //    bool Bof();
    //    bool Eof();
    //    DSQLValues & first();
    //    DSQLValues & last();
    //    DSQLValues & prev();
    //    DSQLValues & next();

    uintptr_t                   count();
    const XSQLDAHolder &        sqlda();

    uintptr_t                   indexOfName(const utf8::String & name);
    const utf8::String          nameOfIndex(uintptr_t i);
    uintptr_t                   checkValueIndex(uintptr_t i);

    // access methods
    bool                        isNull(uintptr_t i);
    bool                        isNull(const utf8::String & valueName);

    ksys::Mutant                asMutant(uintptr_t i);
    ksys::Mutant                asMutant(const utf8::String & valueName);
    utf8::String                asString(uintptr_t i);
    utf8::String                asString(const utf8::String & valueName);
    DSQLValueBlob &             asBlob(uintptr_t i);
    DSQLValueBlob &             asBlob(const utf8::String & name);
    DSQLValueArray &            asArray(uintptr_t i);
    DSQLValueArray &            asArray(const utf8::String & name);
};
//---------------------------------------------------------------------------
inline DSQLValues & DSQLValues::clear()
{
  rows_.clear();
  row_ = -1;
  return *this;
}
//---------------------------------------------------------------------------
inline uintptr_t DSQLValues::rowCount()
{
  return rows_.count();
}
//---------------------------------------------------------------------------
inline intptr_t DSQLValues::rowIndex()
{
  return row_;
}
//---------------------------------------------------------------------------
inline DSQLValues & DSQLValues::selectFirst()
{
  return selectRow(0);
}
//---------------------------------------------------------------------------
inline DSQLValues & DSQLValues::selectLast()
{
  return selectRow(rowCount() - 1);
}
//---------------------------------------------------------------------------
inline uintptr_t DSQLValues::count()
{
  return sqlda_.count();
}
//---------------------------------------------------------------------------
inline const XSQLDAHolder & DSQLValues::sqlda()
{
  return sqlda_;
}
//---------------------------------------------------------------------------
inline uintptr_t DSQLValues::indexOfName(const utf8::String & name)
{
  return valuesIndex_.indexOfKey(name);
}
//---------------------------------------------------------------------------
inline const utf8::String DSQLValues::nameOfIndex(uintptr_t i)
{
  return valuesIndex_.keyOfIndex(i);
}
//---------------------------------------------------------------------------
inline bool DSQLValues::isNull(uintptr_t i)
{
  return rows_[row_][checkValueIndex(i)].sqlind_ < 0 ? true : false;
}
//---------------------------------------------------------------------------
inline bool DSQLValues::isNull(const utf8::String & valueName)
{
  return rows_[row_][checkValueIndex(valuesIndex_.indexOfKey(valueName))].sqlind_ < 0 ? true : false;
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValues::asMutant(uintptr_t i)
{
  return rows_[row_][checkValueIndex(i)].getMutant();
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValues::asMutant(const utf8::String & valueName)
{
  return rows_[row_][checkValueIndex(valuesIndex_.indexOfKey(valueName))].getMutant();
}
//---------------------------------------------------------------------------
inline utf8::String DSQLValues::asString(uintptr_t i)
{
  return rows_[row_][checkValueIndex(i)].getString();
}
//---------------------------------------------------------------------------
inline utf8::String DSQLValues::asString(const utf8::String & valueName)
{
  return rows_[row_][checkValueIndex(valuesIndex_.indexOfKey(valueName))].getString();
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
enum Stmt { stmtUnknown = 0, stmtSelect = 1, stmtInsert = 2, stmtUpdate = 3, stmtDelete = 4, stmtDdl = 5, stmtGetSegment = 6, stmtPutSegment = 7, stmtExecProcedure = 8, stmtStartTrans = 9, stmtCommit = 10, stmtRollback = 11, stmtSelectForUpd = 12, stmtSetGenerator = 13, stmtSavepoint = 14 };
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DSQLStatement {
    friend class Database;
    friend class Transaction;
    friend class DSQLParams;
    friend class DSQLValues;
    friend class DSQLParamBlob;
    friend class DSQLValueBlob;
    friend class DSQLParamArray;
    friend class DSQLValueArray;
  public:
                      ~DSQLStatement();
                      DSQLStatement();

    DSQLStatement &   attach(Database & database, Transaction & transaction);
    DSQLStatement &   detach();

    DSQLStatement &   prepare();
    DSQLStatement &   unprepare();
    DSQLStatement &   createCursor(const utf8::String & name);
    DSQLStatement &   dropCursor();
    DSQLStatement &   execute();
    DSQLStatement &   execute(const utf8::String & sqlTextImm);

    ksys::Mutant      paramAsMutant(uintptr_t i);
    ksys::Mutant      paramAsMutant(const utf8::String & name);
    utf8::String      paramAsString(uintptr_t i);
    utf8::String      paramAsString(const utf8::String & name);
    DSQLStatement &   paramAsMutant(uintptr_t i, const ksys::Mutant & value);
    DSQLStatement &   paramAsMutant(const utf8::String & name, const ksys::Mutant & value);
    DSQLStatement &   paramAsString(uintptr_t i, const utf8::String & Value);
    DSQLStatement &   paramAsString(const utf8::String & name, const utf8::String & Value);
    DSQLParamBlob &   paramAsBlob(uintptr_t i);
    DSQLParamBlob &   paramAsBlob(const utf8::String & name);
    DSQLParamArray &  paramAsArray(uintptr_t i);
    DSQLParamArray &  paramAsArray(const utf8::String & name);

    ksys::Mutant      valueAsMutant(uintptr_t i);
    ksys::Mutant      valueAsMutant(const utf8::String & name);
    utf8::String      valueAsString(uintptr_t i);
    utf8::String      valueAsString(const utf8::String & name);
    DSQLValueBlob &   valueAsBlob(uintptr_t i);
    DSQLValueBlob &   valueAsBlob(const utf8::String & name);
    DSQLValueArray &  valueAsArray(uintptr_t i);
    DSQLValueArray &  valueAsArray(const utf8::String & name);

    // properties
    isc_stmt_handle & handle();
    bool              attached();
    bool              allocated();
    utf8::String      sqlText();
    DSQLStatement &   sqlText(const utf8::String & sqlText);
    DSQLParams &      params();
    DSQLValues &      values();
  protected:
  private:
    isc_stmt_handle handle_;
    Database *      database_;
    Transaction *   transaction_;
    utf8::String    sqlText_;
    bool            sqlTextChanged_;
    bool            prepared_;
    bool            executed_;
    DSQLParams      params_;
    DSQLValues      values_;
    Stmt            stmtType;

    bool              isSQLTextDDL() const;
    DSQLStatement &   allocate();
    DSQLStatement &   free();
    DSQLStatement &   describe(XSQLDAHolder & sqlda);
    DSQLStatement &   describeBind(XSQLDAHolder & sqlda);
    ISC_STATUS        fetch(ISC_STATUS_ARRAY status);
    DSQLStatement &   info();
    DSQLStatement &   arrayLookupBounds(char * tableName, char * arrayColumnName, ISC_ARRAY_DESC & desc);
    DSQLStatement &   arrayPutSlice(ISC_QUAD & arrayId, ISC_ARRAY_DESC & desc, void * data, ISC_LONG & count);
    DSQLStatement &   arrayGetSlice(ISC_QUAD & arrayId, ISC_ARRAY_DESC & desc, void * data, ISC_LONG & count);
    DSQLStatement &   createBlob(isc_blob_handle & blobHandle, ISC_QUAD & blobId, short bpbLength, char * bpbAddress);
    DSQLStatement &   openBlob(isc_blob_handle & blobHandle, ISC_QUAD & blobId, ISC_USHORT bpbLength, ISC_UCHAR * bpbAddress);
    DSQLStatement &   closeBlob(isc_blob_handle & blobHandle);
    DSQLStatement &   cancelBlob(isc_blob_handle & blobHandle);
    DSQLStatement &   blobLookupDesc(char * tableName, char * columnName, ISC_BLOB_DESC & desc, char * globalColumnName);
    utf8::String      compileSQLParameters();
};
//---------------------------------------------------------------------------
inline isc_stmt_handle & DSQLStatement::handle()
{
  return handle_;
}
//---------------------------------------------------------------------------
inline bool DSQLStatement::attached()
{
  return database_ != NULL;
}
//---------------------------------------------------------------------------
inline bool DSQLStatement::allocated()
{
  return handle_ != 0;
}
//---------------------------------------------------------------------------
inline utf8::String DSQLStatement::sqlText()
{
  return sqlText_;
}
//---------------------------------------------------------------------------
inline DSQLParams & DSQLStatement::params()
{
  return params_;
}
//---------------------------------------------------------------------------
inline DSQLValues & DSQLStatement::values()
{
  return values_;
}
//---------------------------------------------------------------------------
inline DSQLStatement & DSQLStatement::execute(const utf8::String & sqlTextImm)
{
  return sqlText(sqlTextImm).execute();
}
//---------------------------------------------------------------------------
inline DSQLStatement & DSQLStatement::unprepare()
{
  prepared_ = false;
  return *this;
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLStatement::paramAsMutant(uintptr_t i)
{
  return params_.asMutant(i);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLStatement::paramAsMutant(const utf8::String & name)
{
  return params_.asMutant(name);
}
//---------------------------------------------------------------------------
inline utf8::String DSQLStatement::paramAsString(uintptr_t i)
{
  return params_.asString(i);
}
//---------------------------------------------------------------------------
inline utf8::String DSQLStatement::paramAsString(const utf8::String & name)
{
  return params_.asString(name);
}
//---------------------------------------------------------------------------
inline DSQLStatement & DSQLStatement::paramAsMutant(uintptr_t i, const ksys::Mutant & value)
{
  params_.asMutant(i, value);
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLStatement & DSQLStatement::paramAsMutant(const utf8::String & name, const ksys::Mutant & value)
{
  params_.asMutant(name, value);
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLStatement & DSQLStatement::paramAsString(uintptr_t i, const utf8::String & value)
{
  params_.asString(i, value);
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLStatement & DSQLStatement::paramAsString(const utf8::String & name, const utf8::String & value)
{
  params_.asString(name, value);
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLParamBlob & DSQLStatement::paramAsBlob(uintptr_t i)
{
  return params_.asBlob(i);
}
//---------------------------------------------------------------------------
inline DSQLParamBlob & DSQLStatement::paramAsBlob(const utf8::String & name)
{
  return params_.asBlob(name);
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLStatement::paramAsArray(uintptr_t i)
{
  return params_.asArray(i);
}
//---------------------------------------------------------------------------
inline DSQLParamArray & DSQLStatement::paramAsArray(const utf8::String & name)
{
  return params_.asArray(name);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLStatement::valueAsMutant(uintptr_t i)
{
  return values_.asMutant(i);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLStatement::valueAsMutant(const utf8::String & name)
{
  return values_.asMutant(name);
}
//---------------------------------------------------------------------------
inline utf8::String DSQLStatement::valueAsString(uintptr_t i)
{
  return values_.asString(i);
}
//---------------------------------------------------------------------------
inline utf8::String DSQLStatement::valueAsString(const utf8::String & name)
{
  return values_.asString(name);
}
//---------------------------------------------------------------------------
inline DSQLValueBlob & DSQLStatement::valueAsBlob(uintptr_t i)
{
  return values_.asBlob(i);
}
//---------------------------------------------------------------------------
inline DSQLValueBlob & DSQLStatement::valueAsBlob(const utf8::String & name)
{
  return values_.asBlob(name);
}
//---------------------------------------------------------------------------
inline DSQLValueArray & DSQLStatement::valueAsArray(uintptr_t i)
{
  return values_.asArray(i);
}
//---------------------------------------------------------------------------
inline DSQLValueArray & DSQLStatement::valueAsArray(const utf8::String & name)
{
  return values_.asArray(name);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
#endif /* _fbst_H_ */