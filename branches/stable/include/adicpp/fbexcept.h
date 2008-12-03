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
    EDBCreate() {}
    EDBCreate(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDBCreate::EDBCreate(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDBDrop : public EClientServer {
  public:
    EDBDrop() {}
    EDBDrop(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDBDrop::EDBDrop(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDBNotAttached : public EClientServer {
  public:
    EDBNotAttached() {}
    EDBNotAttached(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDBNotAttached::EDBNotAttached(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDBAttach : public EClientServer {
  public:
    EDBAttach() {}
    EDBAttach(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDBAttach::EDBAttach(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDBDetach : public EClientServer {
  public:
    EDBDetach() {}
    EDBDetach(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDBDetach::EDBDetach(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStAttached : public EClientServer {
  public:
    EDSQLStAttached() {}
    EDSQLStAttached(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStAttached::EDSQLStAttached(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStNotAttached : public EClientServer {
  public:
    EDSQLStNotAttached() {}
    EDSQLStNotAttached(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStNotAttached::EDSQLStNotAttached(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStAllocate : public EClientServer {
  public:
    EDSQLStAllocate() {}
    EDSQLStAllocate(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStAllocate::EDSQLStAllocate(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStFree : public EClientServer {
  public:
    EDSQLStFree() {}
    EDSQLStFree(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStFree::EDSQLStFree(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStPrepare : public EClientServer {
  public:
    EDSQLStPrepare() {}
    EDSQLStPrepare(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStPrepare::EDSQLStPrepare(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStDescribe : public EClientServer {
  public:
    EDSQLStDescribe() {}
    EDSQLStDescribe(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStDescribe::EDSQLStDescribe(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStDescribeBind : public EClientServer {
  public:
    EDSQLStDescribeBind() {}
    EDSQLStDescribeBind(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStDescribeBind::EDSQLStDescribeBind(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStExecute : public EClientServer {
  public:
    EDSQLStExecute() {}
    EDSQLStExecute(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStExecute::EDSQLStExecute(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStCreateCursor : public EClientServer {
  public:
    EDSQLStCreateCursor() {}
    EDSQLStCreateCursor(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStCreateCursor::EDSQLStCreateCursor(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStDropCursor : public EClientServer {
  public:
    EDSQLStDropCursor() {}
    EDSQLStDropCursor(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStDropCursor::EDSQLStDropCursor(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStFetch : public EClientServer {
  public:
    EDSQLStFetch() {}
    EDSQLStFetch(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStFetch::EDSQLStFetch(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInfo : public EClientServer {
  public:
    EDSQLStInfo() {}
    EDSQLStInfo(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInfo::EDSQLStInfo(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidParamName : public EClientServer {
  public:
    EDSQLStInvalidParamName() {}
    EDSQLStInvalidParamName(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidParamName::EDSQLStInvalidParamName(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidParamIndex : public EClientServer {
  public:
    EDSQLStInvalidParamIndex() {}
    EDSQLStInvalidParamIndex(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidParamIndex::EDSQLStInvalidParamIndex(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidParamValue : public EClientServer {
  public:
    EDSQLStInvalidParamValue() {}
    EDSQLStInvalidParamValue(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidParamValue::EDSQLStInvalidParamValue(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidParam : public EClientServer {
  public:
    EDSQLStInvalidParam() {}
    EDSQLStInvalidParam(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidParam::EDSQLStInvalidParam(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidValueName : public EClientServer {
  public:
    EDSQLStInvalidValueName() {}
    EDSQLStInvalidValueName(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidValueName::EDSQLStInvalidValueName(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidValueIndex : public EClientServer {
  public:
    EDSQLStInvalidValueIndex() {}
    EDSQLStInvalidValueIndex(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidValueIndex::EDSQLStInvalidValueIndex(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidValue : public EClientServer {
  public:
    EDSQLStInvalidValue() {}
    EDSQLStInvalidValue(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidValue::EDSQLStInvalidValue(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidRowIndex : public EClientServer {
  public:
    EDSQLStInvalidRowIndex() {}
    EDSQLStInvalidRowIndex(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidRowIndex::EDSQLStInvalidRowIndex(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStArrayLookupBounds : public EClientServer {
  public:
    EDSQLStArrayLookupBounds() {}
    EDSQLStArrayLookupBounds(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStArrayLookupBounds::EDSQLStArrayLookupBounds(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStArrayPutSlice : public EClientServer {
  public:
    EDSQLStArrayPutSlice() {}
    EDSQLStArrayPutSlice(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStArrayPutSlice::EDSQLStArrayPutSlice(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStArrayGetSlice : public EClientServer {
  public:
    EDSQLStArrayGetSlice() {}
    EDSQLStArrayGetSlice(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStArrayGetSlice::EDSQLStArrayGetSlice(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStCreateBlob : public EClientServer {
  public:
    EDSQLStCreateBlob() {}
    EDSQLStCreateBlob(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStCreateBlob::EDSQLStCreateBlob(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStOpenBlob : public EClientServer {
  public:
    EDSQLStOpenBlob() {}
    EDSQLStOpenBlob(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStOpenBlob::EDSQLStOpenBlob(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStCancelBlob : public EClientServer {
  public:
    EDSQLStCancelBlob() {}
    EDSQLStCancelBlob(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStCancelBlob::EDSQLStCancelBlob(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStCloseBlob : public EClientServer {
  public:
    EDSQLStCloseBlob() {}
    EDSQLStCloseBlob(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStCloseBlob::EDSQLStCloseBlob(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStPutSegment : public EClientServer {
  public:
    EDSQLStPutSegment() {}
    EDSQLStPutSegment(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStPutSegment::EDSQLStPutSegment(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStGetSegment : public EClientServer {
  public:
    EDSQLStGetSegment() {}
    EDSQLStGetSegment(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStGetSegment::EDSQLStGetSegment(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EDSQLStInvalidArrayDim : public EClientServer {
  public:
    EDSQLStInvalidArrayDim() {}
    EDSQLStInvalidArrayDim(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EDSQLStInvalidArrayDim::EDSQLStInvalidArrayDim(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrActive : public EClientServer {
  public:
    ETrActive() {}
    ETrActive(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrActive::ETrActive(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrNotActive : public EClientServer {
  public:
    ETrNotActive() {}
    ETrNotActive(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrNotActive::ETrNotActive(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrNotAttached : public EClientServer {
  public:
    ETrNotAttached() {}
    ETrNotAttached(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrNotAttached::ETrNotAttached(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrStart : public EClientServer {
  public:
    ETrStart() {}
    ETrStart(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrStart::ETrStart(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrPrepare : public EClientServer {
  public:
    ETrPrepare() {}
    ETrPrepare(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrPrepare::ETrPrepare(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrCommit : public EClientServer {
  public:
    ETrCommit() {}
    ETrCommit(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrCommit::ETrCommit(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ETrRollback : public EClientServer {
  public:
    ETrRollback() {}
    ETrRollback(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline ETrRollback::ETrRollback(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EServiceAttach : public EClientServer {
  public:
    EServiceAttach() {}
    EServiceAttach(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EServiceAttach::EServiceAttach(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EServiceDetach : public EClientServer {
  public:
    EServiceDetach() {}
    EServiceDetach(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EServiceDetach::EServiceDetach(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EServiceStart : public EClientServer {
  public:
    EServiceStart() {}
    EServiceStart(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EServiceStart::EServiceStart(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EServiceQuery : public EClientServer {
  public:
    EServiceQuery() {}
    EServiceQuery(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EServiceQuery::EServiceQuery(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EEventQueue : public EClientServer {
  public:
    EEventQueue() {}
    EEventQueue(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EEventQueue::EEventQueue(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EEventCancel : public EClientServer {
  public:
    EEventCancel() {}
    EEventCancel(const ISC_STATUS_ARRAY status, const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EEventCancel::EEventCancel(const ISC_STATUS_ARRAY status, const utf8::String & what)
  : EClientServer(status, what)
{
}
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
#endif /* _fbexcept_H_ */
//---------------------------------------------------------------------------
