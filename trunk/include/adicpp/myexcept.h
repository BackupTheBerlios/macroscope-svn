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
#ifndef _myexcept_H_
#define _myexcept_H_
//---------------------------------------------------------------------------
namespace mycpp {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDBCreate : public EClientServer {
  public:
    EDBCreate(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDBCreate::EDBCreate(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDBDrop : public EClientServer {
  public:
    EDBDrop(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDBDrop::EDBDrop(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDBNotAttached : public EClientServer {
  public:
    EDBNotAttached(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDBNotAttached::EDBNotAttached(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDBAttach : public EClientServer {
  public:
    EDBAttach(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDBAttach::EDBAttach(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDBDetach : public EClientServer {
  public:
    EDBDetach(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDBDetach::EDBDetach(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStAttached : public EClientServer {
  public:
    EDSQLStAttached(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStAttached::EDSQLStAttached(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStNotAttached : public EClientServer {
  public:
    EDSQLStNotAttached(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStNotAttached::EDSQLStNotAttached(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStAllocate : public EClientServer {
  public:
    EDSQLStAllocate(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStAllocate::EDSQLStAllocate(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStFree : public EClientServer {
  public:
    EDSQLStFree(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStFree::EDSQLStFree(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStPrepare : public EClientServer {
  public:
    EDSQLStPrepare(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStPrepare::EDSQLStPrepare(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStExecute : public EClientServer {
  public:
    EDSQLStExecute(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStExecute::EDSQLStExecute(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStBindParam : public EClientServer {
  public:
    EDSQLStBindParam(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStBindParam::EDSQLStBindParam(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidValue : public EClientServer {
  public:
    EDSQLStInvalidValue(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidValue::EDSQLStInvalidValue(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStBindResult : public EClientServer {
  public:
    EDSQLStBindResult(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStBindResult::EDSQLStBindResult(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStResultMetadata : public EClientServer {
  public:
    EDSQLStResultMetadata(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStResultMetadata::EDSQLStResultMetadata(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStParamMetadata : public EClientServer {
  public:
    EDSQLStParamMetadata(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStParamMetadata::EDSQLStParamMetadata(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStFetch : public EClientServer {
  public:
    EDSQLStFetch(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStFetch::EDSQLStFetch(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStStoreResult : public EClientServer {
  public:
    EDSQLStStoreResult(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStStoreResult::EDSQLStStoreResult(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStFreeResult : public EClientServer {
  public:
    EDSQLStFreeResult(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStFreeResult::EDSQLStFreeResult(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidRowIndex : public EClientServer {
  public:
    EDSQLStInvalidRowIndex(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidRowIndex::EDSQLStInvalidRowIndex(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidValueIndex : public EClientServer {
  public:
    EDSQLStInvalidValueIndex(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidValueIndex::EDSQLStInvalidValueIndex(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidParamValue : public EClientServer {
  public:
    EDSQLStInvalidParamValue(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidParamValue::EDSQLStInvalidParamValue(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidParam : public EClientServer {
  public:
    EDSQLStInvalidParam(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidParam::EDSQLStInvalidParam(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidParamIndex : public EClientServer {
  public:
    EDSQLStInvalidParamIndex(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidParamIndex::EDSQLStInvalidParamIndex(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidParamName : public EClientServer {
  public:
    EDSQLStInvalidParamName(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidParamName::EDSQLStInvalidParamName(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrNotAttached : public EClientServer {
  public:
    ETrNotAttached(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrNotAttached::ETrNotAttached(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrNotActive : public EClientServer {
  public:
    ETrNotActive(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrNotActive::ETrNotActive(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrCommit : public EClientServer {
  public:
    ETrCommit(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrCommit::ETrCommit(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrRollback : public EClientServer {
  public:
    ETrRollback(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrRollback::ETrRollback(int32_t code,const utf8::String & what) :
  EClientServer(code,what)
{
}
//---------------------------------------------------------------------------
} // namespace mycpp
//---------------------------------------------------------------------------
#endif /* _myexcept_H_ */
//---------------------------------------------------------------------------
