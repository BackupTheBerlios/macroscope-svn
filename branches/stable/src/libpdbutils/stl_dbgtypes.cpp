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

bool get_std_vector_symbolInfo(DbgType& dbgType, bool& /*discarge*/)
{
  if (DBGSTRING_STARTSWITH(dbgType.typeName, "std::vector<") == false)
    return false;
  
  DbgType begin;
  if (dbgType.getMember("_First", begin) == false)
    return false;
  DbgType end;
  if (dbgType.getMember("_Last", end) == false)
    return false;
  DbgType capacity;
  if (dbgType.getMember("_End", capacity) == false)
    return false;
  DbgType eltype;
  if (begin.getPointer(eltype) == false)
    return false;
  dbgType.typeName = "!std::vector<T>";
  DbgType newtype;
  newtype.name = dbgType.name;
  newtype.typeName = dbgType.typeName;
  newtype.type = dbgType.type;
  char* beginPtr = (char*)begin.dataAddress;
  char* endPtr = (char*)end.dataAddress;
  char* capacityPtr = (char*)capacity.dataAddress;
  char* itPtr = beginPtr;
  int64_t size = 0;
  int64_t icapacity = 0;
  if (eltype.size > 0)
  {
    size = (endPtr - beginPtr) / eltype.size;
    icapacity = (capacityPtr - beginPtr) / eltype.size;
  }
  DbgType sizeType;
  sizeType.name = DBGSTRING_CONSTSTR("size");
  sizeType.typeName = DBGSTRING_CONSTSTR("int");
  sizeType.type = DbgType::Integer;
  sizeType.val.bigInt = size;
  newtype.addSubType(sizeType);

  sizeType.name = DBGSTRING_CONSTSTR("capacity");
  sizeType.val.bigInt = icapacity;
  newtype.addSubType(sizeType);

  DbgType arrayType;
  arrayType.name = DBGSTRING_CONSTSTR("data");
  arrayType.typeName = DBGSTRING_CONSTSTR("T");
  arrayType.type = DbgType::Array;

  for (; eltype.size > 0 && itPtr < endPtr; itPtr += (uintptr_t) eltype.size)
  {
    DbgType subType;
    if (querySymbolType(dbgType.modBase, begin.typeIndex, (intptr_t)&itPtr, eltype.size, subType, DbgFrameGetAll) == true)
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
RegisterDebugPostTypeHandler _register_std_vectorHandler(get_std_vector_symbolInfo);


bool get_std_string_symbolInfo(DbgType& dbgType, bool& /*discarge*/)
{
   if (DBGSTRING_STARTSWITH(dbgType.typeName, "std::basic_string<char") == false)
    return false;
   DbgType begin;
  if (dbgType.getMember("_Ptr", begin) == false)
    return false;
  DbgType length;
  if (dbgType.getMember("_Len", length) == false)
    return false;
  DbgType capacity;
  if (dbgType.getMember("_Res", capacity) == false)
    return false;
  DbgType newtype;
  newtype.name = dbgType.name;
  newtype.size = dbgType.size;
  newtype.dataAddress = dbgType.dataAddress;
  newtype.typeName = "!std::string";
  if (IsBadStringPtr((char*)begin.dataAddress, length.val.integer) == FALSE)
  {
    newtype.type = DbgType::String;
    newtype.val.c_string = (char*)begin.dataAddress;
  }
  else
  {
    newtype.type = DbgType::Void;
  }
  dbgType = newtype;
  return true;
}

RegisterDebugPostTypeHandler _register_std_string_Handler(get_std_string_symbolInfo);