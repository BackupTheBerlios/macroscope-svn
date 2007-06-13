/**
  Utils to make debugging running application under Windows
  Copyright Roger Rene Kommer (kommer@artefaktur.com) 2003

  dbgutils The sources either can be in a shared library (.dll) or
  has to be linked directly to the executable. 
  Compiling dbgutils as static library (.lib) won't work!

  Requirements dbghelp.h .dll .lib from microsoft
*/
#include <adicpp/lconfig.h>
#include <adicpp/pdbutils/pdbutils.h>

using namespace pdbutils;

bool get_core_vector_symbolInfo(DbgType& dbgType, bool& discarge)
{
  if (DBGSTRING_STARTSWITH(dbgType.typeName, "core_vector<") == false)
    return false;

  DbgType begin;
  if (dbgType.getMember("_begin", begin) == false)
    return false;
  DbgType end;
  if (dbgType.getMember("_end", end) == false)
    return false;
  DbgType capacity;
  if (dbgType.getMember("_capacity", capacity) == false)
    return false;
  DbgType eltype;
  if (begin.getPointer(eltype) == false)
    return false;
  DbgType newtype;
  newtype.name = dbgType.name;
  newtype.typeName = dbgType.typeName;
  newtype.type = dbgType.type;
  char* beginPtr = (char*)begin.dataAddress;
  char* endPtr = (char*)end.dataAddress;

  char* itPtr = beginPtr;
  int64_t size = (endPtr - beginPtr) / eltype.size;
  DbgType sizeType;
  sizeType.name = DBGSTRING_CONSTSTR("size");
  sizeType.typeName = DBGSTRING_CONSTSTR("int");
  sizeType.type = DbgType::Integer;
  sizeType.val.bigInt = size;
  newtype.addSubType(sizeType);
  newtype.addSubType(capacity);
  DbgType arrayType;
  arrayType.name = DBGSTRING_CONSTSTR("data");
  arrayType.typeName = DBGSTRING_CONSTSTR("T");
  arrayType.type = DbgType::Array;
  for (; itPtr < endPtr; itPtr += eltype.size)
  {
    DbgType subType;
    if (querySymbolType(dbgType.modBase, begin.typeIndex, (intptr_t) &itPtr, eltype.size, subType, DbgFrameGetAll) == true)
    {
      DbgType s2;
      if (subType.getPointer(s2) == true)
        arrayType.addSubType(s2);
      else
      {
        arrayType.addSubType(subType);
      }
    }

  }
  newtype.addSubType(arrayType);
  dbgType = newtype;
  return true;
}
RegisterDebugPostTypeHandler _register_core_vectorHandler(get_core_vector_symbolInfo);
