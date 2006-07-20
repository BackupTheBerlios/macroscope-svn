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
#ifndef _fbexcept_H_
#define _fbexcept_H_
//---------------------------------------------------------------------------
namespace fbcpp {

//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDBCreate : public EClientServer {
  public:
    EDBCreate(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDBCreate::EDBCreate(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDBDrop : public EClientServer {
  public:
    EDBDrop(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDBDrop::EDBDrop(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDBNotAttached : public EClientServer {
  public:
    EDBNotAttached(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDBNotAttached::EDBNotAttached(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDBAttach : public EClientServer {
  public:
    EDBAttach(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDBAttach::EDBAttach(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDBDetach : public EClientServer {
  public:
    EDBDetach(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDBDetach::EDBDetach(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStAttached : public EClientServer {
  public:
    EDSQLStAttached(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStAttached::EDSQLStAttached(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStNotAttached : public EClientServer {
  public:
    EDSQLStNotAttached(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStNotAttached::EDSQLStNotAttached(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStAllocate : public EClientServer {
  public:
    EDSQLStAllocate(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStAllocate::EDSQLStAllocate(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStFree : public EClientServer {
  public:
    EDSQLStFree(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStFree::EDSQLStFree(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStPrepare : public EClientServer {
  public:
    EDSQLStPrepare(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStPrepare::EDSQLStPrepare(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStDescribe : public EClientServer {
  public:
    EDSQLStDescribe(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStDescribe::EDSQLStDescribe(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStDescribeBind : public EClientServer {
  public:
    EDSQLStDescribeBind(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStDescribeBind::EDSQLStDescribeBind(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStExecute : public EClientServer {
  public:
    EDSQLStExecute(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStExecute::EDSQLStExecute(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStCreateCursor : public EClientServer {
  public:
    EDSQLStCreateCursor(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStCreateCursor::EDSQLStCreateCursor(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStDropCursor : public EClientServer {
  public:
    EDSQLStDropCursor(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStDropCursor::EDSQLStDropCursor(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStFetch : public EClientServer {
  public:
    EDSQLStFetch(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStFetch::EDSQLStFetch(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInfo : public EClientServer {
  public:
    EDSQLStInfo(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInfo::EDSQLStInfo(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidParamName : public EClientServer {
  public:
    EDSQLStInvalidParamName(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidParamName::EDSQLStInvalidParamName(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidParamIndex : public EClientServer {
  public:
    EDSQLStInvalidParamIndex(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidParamIndex::EDSQLStInvalidParamIndex(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidParamValue : public EClientServer {
  public:
    EDSQLStInvalidParamValue(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidParamValue::EDSQLStInvalidParamValue(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidParam : public EClientServer {
  public:
    EDSQLStInvalidParam(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidParam::EDSQLStInvalidParam(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidValueName : public EClientServer {
  public:
    EDSQLStInvalidValueName(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidValueName::EDSQLStInvalidValueName(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidValueIndex : public EClientServer {
  public:
    EDSQLStInvalidValueIndex(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidValueIndex::EDSQLStInvalidValueIndex(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidValue : public EClientServer {
  public:
    EDSQLStInvalidValue(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidValue::EDSQLStInvalidValue(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidRowIndex : public EClientServer {
  public:
    EDSQLStInvalidRowIndex(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidRowIndex::EDSQLStInvalidRowIndex(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStArrayLookupBounds : public EClientServer {
  public:
    EDSQLStArrayLookupBounds(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStArrayLookupBounds::EDSQLStArrayLookupBounds(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStArrayPutSlice : public EClientServer {
  public:
    EDSQLStArrayPutSlice(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStArrayPutSlice::EDSQLStArrayPutSlice(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStArrayGetSlice : public EClientServer {
  public:
    EDSQLStArrayGetSlice(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStArrayGetSlice::EDSQLStArrayGetSlice(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStCreateBlob : public EClientServer {
  public:
    EDSQLStCreateBlob(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStCreateBlob::EDSQLStCreateBlob(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStOpenBlob : public EClientServer {
  public:
    EDSQLStOpenBlob(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStOpenBlob::EDSQLStOpenBlob(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStCancelBlob : public EClientServer {
  public:
    EDSQLStCancelBlob(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStCancelBlob::EDSQLStCancelBlob(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStCloseBlob : public EClientServer {
  public:
    EDSQLStCloseBlob(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStCloseBlob::EDSQLStCloseBlob(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStPutSegment : public EClientServer {
  public:
    EDSQLStPutSegment(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStPutSegment::EDSQLStPutSegment(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStGetSegment : public EClientServer {
  public:
    EDSQLStGetSegment(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStGetSegment::EDSQLStGetSegment(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidArrayDim : public EClientServer {
  public:
    EDSQLStInvalidArrayDim(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidArrayDim::EDSQLStInvalidArrayDim(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrActive : public EClientServer {
  public:
    ETrActive(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrActive::ETrActive(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrNotActive : public EClientServer {
  public:
    ETrNotActive(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrNotActive::ETrNotActive(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrNotAttached : public EClientServer {
  public:
    ETrNotAttached(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrNotAttached::ETrNotAttached(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrStart : public EClientServer {
  public:
    ETrStart(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrStart::ETrStart(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrPrepare : public EClientServer {
  public:
    ETrPrepare(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrPrepare::ETrPrepare(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrCommit : public EClientServer {
  public:
    ETrCommit(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrCommit::ETrCommit(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrRollback : public EClientServer {
  public:
    ETrRollback(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrRollback::ETrRollback(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EServiceAttach : public EClientServer {
  public:
    EServiceAttach(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EServiceAttach::EServiceAttach(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EServiceDetach : public EClientServer {
  public:
    EServiceDetach(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EServiceDetach::EServiceDetach(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EServiceStart : public EClientServer {
  public:
    EServiceStart(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EServiceStart::EServiceStart(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EServiceQuery : public EClientServer {
  public:
    EServiceQuery(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EServiceQuery::EServiceQuery(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EEventQueue : public EClientServer {
  public:
    EEventQueue(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EEventQueue::EEventQueue(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EEventCancel : public EClientServer {
  public:
    EEventCancel(ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EEventCancel::EEventCancel(ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
#endif /* _fbexcept_H_ */
//---------------------------------------------------------------------------
