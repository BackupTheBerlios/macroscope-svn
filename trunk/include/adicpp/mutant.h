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
#ifndef _mutant_H_
#define _mutant_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
enum MutantType { 
  mtNull, 
  mtInt, 
  mtFloat, 
  mtTime, 
  mtCStr, 
  mtWStr, 
  mtStr, 
  mtString, 
  mtBinary
};
//---------------------------------------------------------------------------
class Mutant {
  friend void         initialize();
  friend void         cleanup();
  public:
    ~Mutant();
    Mutant();

    Mutant(bool a);
    Mutant(char a);
    Mutant(unsigned char a);
    Mutant(short a);
    Mutant(unsigned short a);
#if !HAVE_INTPTR_T_AS_INT
    Mutant(int a);
    Mutant(unsigned a);
#endif
    Mutant(long a);
    Mutant(unsigned long a);
    Mutant(intptr_t a);
    Mutant(uintptr_t a);
#if !HAVE_INTPTR_T_AS_INT64_T
    Mutant(intmax_t a);
    Mutant(uintmax_t a);
#endif
    Mutant(float a);
    Mutant(double a);
#if HAVE_LONG_DOUBLE
    Mutant(long double a);
#endif
    Mutant(const struct timeval & a);
    Mutant(const struct tm & a);
    Mutant(const char * cStr);
    Mutant(const wchar_t * wStr);
    Mutant(const char * str, int); // plane utf8
    Mutant(const utf8::String & string);
    Mutant(const MemoryStream & stream);

    Mutant(const Mutant & m);

    Mutant &            operator =(bool a);
    Mutant &            operator =(char a);
    Mutant &            operator =(unsigned char a);
    Mutant &            operator =(short a);
    Mutant &            operator =(unsigned short a);
#if !HAVE_INTPTR_T_AS_INT
    Mutant &            operator =(int a);
    Mutant &            operator =(unsigned a);
#endif
    Mutant &            operator =(long a);
    Mutant &            operator =(unsigned long a);
    Mutant &            operator =(intptr_t a);
    Mutant &            operator =(uintptr_t a);
#if !HAVE_INTPTR_T_AS_INT64_T
    Mutant &            operator =(intmax_t a);
    Mutant &            operator =(uintmax_t a);
#endif
    Mutant &            operator =(float a);
    Mutant &            operator =(double a);
#if HAVE_LONG_DOUBLE
    Mutant &            operator =(long double a);
#endif
    Mutant &            operator =(const struct timeval & a);
    Mutant &            operator =(const struct tm & a);
    Mutant &            operator =(const char * cStr);
    Mutant &            operator =(const wchar_t * wStr);
    Mutant &            operator =(const utf8::String & string);
    Mutant &            operator =(const MemoryStream & stream);

    Mutant &            operator =(const Mutant & m);

                        operator bool() const;
                        operator char() const;
    operator            unsigned char() const;
                        operator short() const;
    operator            unsigned short() const;
#if !HAVE_INTPTR_T_AS_INT
                        operator int() const;
    operator            unsigned() const;
#endif
                        operator long() const;
    operator            unsigned long() const;
    operator intptr_t() const;
    operator uintptr_t() const;
#if !HAVE_INTPTR_T_AS_INT64_T
    operator intmax_t() const;
    operator uintmax_t() const;
#endif
    operator float() const;
    operator double() const;
#if HAVE_LONG_DOUBLE
    operator long double() const;
#endif
    operator struct timeval() const;
    operator struct tm() const;
    operator utf8::String() const;
    operator MemoryStream() const;

    const MutantType &  type() const;
    Mutant              unique() const;
    Mutant &            clear();
    Mutant &            changeType(MutantType newType);
    Mutant &            changeType(MutantType newType, const Mutant & m);

    void *              raw();
    char *              plane();

    static bool         isBooleanString(const utf8::String & string, bool & value);
    static bool         isBooleanString(const utf8::String & string, intmax_t * i = NULL);
    static bool         isIntegerString(const utf8::String & string, intmax_t & value);
    static bool         isIntegerString(const utf8::String & string, intmax_t * i = NULL);
  protected:
#if _MSC_VER
#pragma warning(push,3)
#endif
    struct {
        union {
#if _MSC_VER
#pragma warning(disable:4200)
#endif
            char            raw_[];
#if _MSC_VER
#pragma warning(default:4200)
#endif
            intmax_t        int_;
            intptr_t        pint_;
            long            lint_;
#if HAVE_LONG_DOUBLE
            long
            #endif
            double float_;
            const char *    cStr_;
            const wchar_t * wStr_;
            const char *    str_;
            char            stringPlaceHolder_[sizeof(utf8::String)];
            char            streamPlaceHolder_[sizeof(MemoryStream)];
        };
        MutantType  type_;
    };
#if _MSC_VER
#pragma warning(pop)
#endif
    utf8::String &        string();
    const utf8::String &  string() const;
    MemoryStream &        stream();
    const MemoryStream &  stream() const;
  private:
    static void           initialize();
    static void           cleanup();
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline utf8::String & Mutant::string()
{
  return *reinterpret_cast< utf8::String *>(raw_);
}
//---------------------------------------------------------------------------
inline const utf8::String & Mutant::string() const
{
  return *reinterpret_cast< const utf8::String *>(raw_);
}
//---------------------------------------------------------------------------
inline MemoryStream & Mutant::stream()
{
  return *reinterpret_cast< MemoryStream *>(raw_);
}
//---------------------------------------------------------------------------
inline const MemoryStream & Mutant::stream() const
{
  return *reinterpret_cast< const MemoryStream *>(raw_);
}
//---------------------------------------------------------------------------
inline Mutant::~Mutant()
{
  clear();
}
//---------------------------------------------------------------------------
inline Mutant::Mutant() : type_(mtNull)
{
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(bool a) : int_(a), type_(mtInt)
{
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(char a) : int_(a), type_(mtInt)
{
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(unsigned char a) : int_(a), type_(mtInt)
{
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(short a) : int_(a), type_(mtInt)
{
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(unsigned short a) : int_(a), type_(mtInt)
{
}
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT
//---------------------------------------------------------------------------
inline Mutant::Mutant(int a) : int_(a), type_(mtInt)
{
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(unsigned a) : int_(a), type_(mtInt)
{
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
inline Mutant::Mutant(long a) : int_(a), type_(mtInt)
{
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(unsigned long a) : int_(a), type_(mtInt)
{
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(intptr_t a) : int_(a), type_(mtInt)
{
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(uintptr_t a) : int_(a), type_(mtInt)
{
}
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT64_T
inline Mutant::Mutant(intmax_t a) : int_(a), type_(mtInt)
{
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(uintmax_t a) : int_(a), type_(mtInt)
{
}
#endif
//---------------------------------------------------------------------------
inline Mutant::Mutant(float a) : float_(a), type_(mtFloat)
{
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(double a) : float_(a), type_(mtFloat)
{
}
//---------------------------------------------------------------------------
#if HAVE_LONG_DOUBLE
inline Mutant::Mutant(long double a) : float_(a), type_(mtFloat)
{
}
#endif
//---------------------------------------------------------------------------
inline Mutant::Mutant(const struct timeval & a) : int_(timeval2Time(a)), type_(mtTime)
{
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(const struct tm & a) : int_(tm2Time(a)), type_(mtTime)
{
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(const char * cStr) : cStr_(cStr), type_(mtCStr)
{
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(const wchar_t * wStr) : wStr_(wStr), type_(mtWStr)
{
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(const char * str, int) : str_(str), type_(mtStr)
{
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(const utf8::String & string) : type_(mtNull)
{
  new (raw_) utf8::String(string);
  type_ = mtString;
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(const MemoryStream & stream) : type_(mtNull)
{
  new (raw_) MemoryStream(stream);
  type_ = mtBinary;
}
//---------------------------------------------------------------------------
inline Mutant::Mutant(const Mutant & m) : type_(mtNull)
{
  *this = m;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(bool a)
{
  clear().int_ = a;
  type_ = mtInt;
  return *this;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(char a)
{
  clear().int_ = a;
  type_ = mtInt;
  return *this;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(unsigned char a)
{
  clear().int_ = a;
  type_ = mtInt;
  return *this;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(short a)
{
  clear().int_ = a;
  type_ = mtInt;
  return *this;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(unsigned short a)
{
  clear().int_ = a;
  type_ = mtInt;
  return *this;
}
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(int a)
{
  clear().int_ = a;
  type_ = mtInt;
  return *this;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(unsigned a)
{
  clear().int_ = a;
  type_ = mtInt;
  return *this;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(long a)
{
  clear().int_ = a;
  type_ = mtInt;
  return *this;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(unsigned long a)
{
  clear().int_ = a;
  type_ = mtInt;
  return *this;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(intptr_t a)
{
  clear().int_ = a;
  type_ = mtInt;
  return *this;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(uintptr_t a)
{
  clear().int_ = a;
  type_ = mtInt;
  return *this;
}
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT64_T
inline Mutant & Mutant::operator =(intmax_t a)
{
  clear().int_ = a;
  type_ = mtInt;
  return *this;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(uintmax_t a)
{
  clear().int_ = a;
  type_ = mtInt;
  return *this;
}
#endif
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(float a)
{
  clear().float_ = a;
  type_ = mtFloat;
  return *this;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(double a)
{
  clear().float_ = a;
  type_ = mtFloat;
  return *this;
}
//---------------------------------------------------------------------------
#if HAVE_LONG_DOUBLE
inline Mutant & Mutant::operator =(long double a)
{
  clear().float_ = a;
  type_ = mtFloat;
  return *this;
}
#endif
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(const struct timeval & a)
{
  clear().int_ = timeval2Time(a);
  type_ = mtTime;
  return *this;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(const struct tm & a)
{
  clear().int_ = tm2Time(a);
  type_ = mtTime;
  return *this;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(const char * cStr)
{
  clear().cStr_ = cStr;
  type_ = mtCStr;
  return *this;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(const wchar_t * wStr)
{
  clear().wStr_ = wStr;
  type_ = mtWStr;
  return *this;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(const utf8::String & string)
{
  clear().type_ = mtNull;
  new (raw_) utf8::String(string);
  type_ = mtString;
  return *this;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(const MemoryStream & stream)
{
  clear().type_ = mtNull;
  new (raw_) MemoryStream(stream);
  type_ = mtBinary;
  return *this;
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::changeType(MutantType newType)
{
  return changeType(newType, *this);
}
//---------------------------------------------------------------------------
inline Mutant & Mutant::operator =(const Mutant & m)
{
  return changeType(m.type_, m);
}
//---------------------------------------------------------------------------
inline const MutantType & Mutant::type() const
{
  return type_;
}
//---------------------------------------------------------------------------
inline void * Mutant::raw()
{
  return raw_;
}
//---------------------------------------------------------------------------
inline char * Mutant::plane()
{
  return (char *) str_;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EMutant : public Exception {
  public:
    EMutant(int32_t code, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EMutant::EMutant(int32_t code, const utf8::String & what) : Exception(code, what)
{
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _mutant_H_ */
//---------------------------------------------------------------------------

