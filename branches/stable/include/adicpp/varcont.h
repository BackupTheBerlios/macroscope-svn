/*-
 * Copyright 2006 Guram Dukashvili
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
#ifndef varcontH
#define varcontH
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
class VARIANTContainer : public VARIANT {
  public:
    ~VARIANTContainer();
    VARIANTContainer();
    VARIANTContainer(const VARIANTContainer & v);
    VARIANTContainer(const VARIANT & v);
    VARIANTContainer & operator = (const VARIANTContainer & v);
    VARIANTContainer & operator = (const VARIANT & v);
    VARIANTContainer & changeType(VARTYPE vt,HRESULT * pRes = NULL);
    VARIANTContainer & changeType(
      VARIANT * pVarSrc, 
      LCID lcid,
      unsigned short wFlags, 
      VARTYPE vt,
      HRESULT * pRes = NULL
    );
    VARIANTContainer & changeType(
      const VARIANT & pVarSrc, 
      LCID lcid,
      unsigned short wFlags, 
      VARTYPE vt,
      HRESULT * pRes = NULL
    );
    VARIANTContainer & changeType(
      const VARIANTContainer & pVarSrc,
      LCID lcid,
      unsigned short wFlags,
      VARTYPE vt,
      HRESULT * pRes = NULL
    );
  protected:
  private:
};
//---------------------------------------------------------------------------
inline VARIANTContainer::~VARIANTContainer()
{
  VariantClear(this);
}
//---------------------------------------------------------------------------
inline VARIANTContainer::VARIANTContainer()
{
  VariantInit(this);
}
//---------------------------------------------------------------------------
inline VARIANTContainer::VARIANTContainer(const VARIANTContainer & v)
{
  VariantInit(this);
  changeType(v,0,0,V_VT(const_cast<VARIANT *>((const VARIANT *) &v)));
}
//---------------------------------------------------------------------------
inline VARIANTContainer::VARIANTContainer(const VARIANT & v)
{
  VariantInit(this);
  changeType(v,0,0,V_VT(const_cast<VARIANT *>((const VARIANT *) &v)));
}
//---------------------------------------------------------------------------
inline VARIANTContainer & VARIANTContainer::operator = (const VARIANTContainer & v)
{
  changeType(v,0,0,V_VT(const_cast<VARIANT *>((const VARIANT *) &v)));
}
//---------------------------------------------------------------------------
inline VARIANTContainer & VARIANTContainer::operator = (const VARIANT & v)
{
  changeType(const_cast<VARIANT *>(&v),0,0,V_VT(const_cast<VARIANT *>((const VARIANT *) &v)));
}
//---------------------------------------------------------------------------
inline VARIANTContainer & VARIANTContainer::changeType(VARTYPE vt,HRESULT * pRes)
{
  HRESULT hRes;
  hRes = VariantChangeTypeEx(this,this,0,0,V_VT(this));
  if( pRes != NULL ){
    *pRes = hRes;
  }
  else if( FAILED(hRes) ){
    newObjectV1C2<Exception>(HRESULT_CODE(hRes) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  }
  return *this;
}
//---------------------------------------------------------------------------
inline VARIANTContainer & VARIANTContainer::changeType(
  VARIANT * pVarSrc,
  LCID lcid,
  unsigned short wFlags,
  VARTYPE vt,
  HRESULT * pRes)
{
  HRESULT hRes;
  hRes = VariantChangeTypeEx(this,pVarSrc,lcid,wFlags,vt);
  if( pRes != NULL ){
    *pRes = hRes;
  }
  else if( FAILED(hRes) ){
    newObjectV1C2<Exception>(HRESULT_CODE(hRes) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  }
  return *this;
}
//---------------------------------------------------------------------------
inline VARIANTContainer & VARIANTContainer::changeType(
  const VARIANT & pVarSrc,
  LCID lcid,
  unsigned short wFlags,
  VARTYPE vt,
  HRESULT * pRes)
{
  HRESULT hRes;
  hRes = VariantChangeTypeEx(this,const_cast<VARIANT *>((const VARIANT *) &pVarSrc),lcid,wFlags,vt);
  if( pRes != NULL ){
    *pRes = hRes;
  }
  else if( FAILED(hRes) ){
    newObjectV1C2<Exception>(HRESULT_CODE(hRes) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  }
  return *this;
}
//---------------------------------------------------------------------------
inline VARIANTContainer & VARIANTContainer::changeType(
  const VARIANTContainer & pVarSrc,
  LCID lcid,
  unsigned short wFlags,
  VARTYPE vt,
  HRESULT * pRes)
{
  HRESULT hRes;
  hRes = VariantChangeTypeEx(this,const_cast<VARIANT *>((const VARIANT *) &pVarSrc),lcid,wFlags,vt);
  if( pRes != NULL ){
    *pRes = hRes;
  }
  else if( FAILED(hRes) ){
    newObjectV1C2<Exception>(HRESULT_CODE(hRes) + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  }
  return *this;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
