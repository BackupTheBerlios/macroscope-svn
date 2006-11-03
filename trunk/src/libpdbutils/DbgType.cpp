#include <adicpp/lconfig.h>
#include <adicpp/pdbutils/pdbutils.h>

#include <sstream>
#include <iostream>

namespace pdbutils {

#define NL "\n"

extern HANDLE _currentProcess;

//#define LOCAL_DEBUG

#if defined(LOCAL_DEBUG)
# define DOUT(msg) do { std::cout << msg; } while (false)
# define DOUTNL(msg) do { std::cout << msg << NL; } while (false)
#else
# define DOUT(msg)
# define DOUTNL(msg) 
#endif

#define ELOGNL(msg)  do { DbgStream out; out << msg << NL; OutputDebugString(out.str().c_str()); } while (false)

std::ostream&
operator<<(std::ostream& out, const DbgString& str)
{
  out << DBGSTRING2CHARPTR(str);
  return out;
}

#define HANDLE_FALSE_CALL(Call, falseblock) \
if ((Call) == FALSE) \
{ \
  ELOGNL(#Call " failed:"); \
  printLastError(); \
  falseblock \
}
#define HANDLE_FALSE_CALL_OK(Call, falseblock) \
if ((Call) == FALSE) \
{ \
  falseblock \
}

void printLastError()
{
  LPVOID lpMsgBuf;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |  FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPTSTR) &lpMsgBuf, 0, NULL  );
  ELOGNL((const char*)lpMsgBuf);
  LocalFree(lpMsgBuf);
}

static intptr_t CurrentPointerRecursion = 0;


DbgVector<DebugTypeHandler>&
getDebugTypePostHandlerArray()
{
  static DbgVector<DebugTypeHandler> _handler;
  return _handler;
}
DbgVector<DebugTypeHandler>&
getDebugTypePreHandlerArray()
{
  static DbgVector<DebugTypeHandler> _handler;
  return _handler;
}
void
registerDebugTypePostHandler(DebugTypeHandler handler)
{
  getDebugTypePostHandlerArray().push_back(handler);
}
void 
registerDebugTypePreHandler(DebugTypeHandler handler)
{
  getDebugTypePreHandlerArray().push_back(handler);
}

intptr_t MaxPointerRecursion = 0;

void DbgFrame::toString(DbgStream& ss, intptr_t printflags) const
{
//  ss << "------------------------------------------------------------------" NL;
  if (printflags & DbgFrameGetLibary)
  {
    if (DBGSTRING_SIZE(libraryName) > 0)
      ss << libraryName << ": " << NL;
  }
  if (printflags & DbgFrameGetSourcePos)
  {
    ss << sourceFileName << "(" << sourceFileLine << "): ";
  }
  //return sourceFileName;// + "(" + sourceFileLine + "): " + functionName;
  ss << functionName << "(";
  intptr_t i;
  for (i = 0; i < params.size(); ++i)
  {
    if (i >= 1)
      ss << ", ";
    params[i].toString(ss, printflags);
  }
  ss << ")";
  if (printflags & DbgFrameGetLocals)
    ss << NL "{";
  else if ((printflags & DbgFrameGetGlobals) == 0 || globals.size() == 0)
  {
    ss << ";" NL;
    return;
  }

  for (i = 0; i < locals.size(); ++i)
  {
    ss << NL;
    locals[i].toString(ss, printflags, "  ");
  }
  ss << NL "}" NL;

  for (i = 0; i < globals.size(); ++i)
  {
    ss << NL;
    globals[i].toString(ss, printflags, "  ");
  }
  //return ss.str().c_str();
}

DbgString 
DbgType::toString(intptr_t printflags) const
{
  return toString(printflags, "");
}

DbgStream& 
DbgType::toString(DbgStream& ss, intptr_t printflags, const DbgString& indendent) const
{
  if ((printflags & DbgPrintContinueLine) == 0)
    ss << DBGSTRING2CHARPTR(indendent);
  if (((printflags & DbgPrintInPointerRef) == 0) && type != Pointer && type != Array)
  {
    ss << typeName;
    ss << " " << name;
    if (type != Unset && (printflags & DbgFrameGetValues))
      ss << " = ";
  }
  printflags &= ~DbgPrintInPointerRef;
  printflags &= ~DbgPrintContinueLine;
  if (type == Unset)
    return ss;

  const intptr_t max_string_buffer = 4096;
 
    switch(type)
    {
    case Void:
      ss << "<void>";
      break;
    case Boolean:
      ss << val.boolean;
      break;
    case Byte:
      if (typeName == "char")
      {
        char buf[2]; buf[0] = val.byte; buf[1] = 0;
        ss << "'" << buf << "' (" << (short)val.byte << ")";
      }
      else
        ss << (short)val.byte;
      break;
    case Integer:
      ss << val.integer;
      break;
    case Short:
      ss << val.shortInt;
      break;
    case Long:
      ss << (int)val.bigInt;
      break;
    
    case Pointer:
      if (flags & IsValidPointer && val.typevec != 0 && val.typevec->size() > 0)
      {
        ss << (*val.typevec)[0].typeName << "* " << name;
      }
      else
      {
        ss << "<Unknown>* " << name;
      }
      if (printflags & DbgFrameGetValues)
      {
        ss << "(0x" << (void*)dataAddress << ") ";
        if (flags & MaxPointerRecursionReached)
        {
          ss << "<max pointer recursion reached>";
        }
        else if (flags & IsValidPointer)
        {
          ss << " *= ";
          if (val.typevec != 0 && val.typevec->size() == 1)
          {
             (*val.typevec)[0].toString(ss, printflags | DbgPrintInPointerRef | DbgPrintContinueLine, indendent + "  ");
          }
          else
            ss << "<unset pointer>";
        }
        else
          ss << "<invalid pointer>";
      }
      break;
    case Array:
    case User:
    {
      if ((printflags & DbgFrameGetValues) == 0)
        return ss;

      if (type == Array)
      {
        if (val.typevec != 0 && val.typevec->size() > 0)
        {
          /* handle char[] as string?
          if ((*val.typevec)[0].typeName == "char")
          {
            intptr_t j = 0;
            for (j = 0; j < val.typevec->size(); ++j)
            {
              if ((*val.typevec)[j].val.byte == 0)
              {
                ss << (*val.typevec)[0].typeName << " " << name << "[" << val.typevec->size() << "] = "
                    << "\"" << (const char*)(*val.typevec)[0].dataAddress << "\"";
                return ss;
              }
            }
          }*/
          ss << (*val.typevec)[0].typeName << " " << name << "[" << val.typevec->size() << "]";
        }
        ss << " = {" NL;
      }
      else
        ss << "{" NL;
      if (val.typevec != 0)
      {
        intptr_t j = 0;
        for (j = 0; j < val.typevec->size(); ++j)
        {
          if (type == Array)
          {
            ss << DBGSTRING2CHARPTR(indendent) << "  [" << j << "] ";
            (*val.typevec)[j].toString(ss, printflags | DbgPrintContinueLine, "");
            ss << ";" NL;
          }
          else
          {
            (*val.typevec)[j].toString(ss, printflags, indendent + "  " ) << ";" NL;
          }
        }
      }
      ss << DBGSTRING2CHARPTR(indendent) << "}";
      break;
    }
    case String:
      if ((printflags & DbgFrameGetValues) == 0)
        return ss;
      
      if (IsBadReadPtr(val.c_string, max_string_buffer) == FALSE)
      {
        char buffer[max_string_buffer + 2];
        memcpy(buffer, val.c_string, max_string_buffer);
        buffer[max_string_buffer] = 0;
        ss << "(0x" << (void*)dataAddress << ") \"" << buffer<< "\"";
      }
      else 
      {
        ss << "(0x" << (void*)dataAddress << ") <invalid character pointer>";
      }
      break;
    default:
      ss << "<Unhandled data type>: " << type;
      //DOUTNL("Unhandled data type: " << type);
      break;
    }
  return ss;  
}


struct BasicType2String
{
  DbgType::BasicType type;
  const char* descr;
};

BasicType2String basictype2string[] =
{
  { DbgType::btNoType, "NoType" },
  { DbgType::btVoid, "void" },
  { DbgType::btChar, "char" },
  { DbgType::btWChar, "wchar" },
  { DbgType::btInt, "int" },
  { DbgType::btUInt, "unsigned int" },
  { DbgType::btFloat, "float" },
  { DbgType::btBCD, "bcd" },
  { DbgType::btBool, "bool" },
  { DbgType::btLong, "long" },
  { DbgType::btULong, "unsigned long" },
  { DbgType::btCurrency, "currency" },
  { DbgType::btDate, "date" },
  { DbgType::btVariant, "variant" },
  { DbgType::btComplex, "complex" },
  { DbgType::btBit, "bit" },
  { DbgType::btBSTR, "BSTR" },
  { DbgType::btHresult, "HRESULT" },
  { DbgType::btNoType, 0 }
};

const char* basicType2String(DbgType::BasicType type)
{
  for (intptr_t i = 0; basictype2string[i].descr != 0; ++i)
    if (basictype2string[i].type == type)
      return basictype2string[i].descr;
  return "<unknown type>";
}

#define DWORD_PTR intptr_t

DbgType::BasicType
queryBasicType( DWORD typeIndex, DWORD64 modBase )
{
  DbgType::BasicType basicType;
  if ( SymGetTypeInfo( _currentProcess, modBase, typeIndex, TI_GET_BASETYPE, &basicType))
  {
    //ELOGNL("qbt 1: " << basicType);
    DOUT(basicType2String(basicType));
    return basicType;
  }

  // Get the real "TypeId" of the child.  We need this for the
  // SymGetTypeInfo( TI_GET_TYPEID ) call below.
  DWORD typeId = 0;
  if (SymGetTypeInfo(_currentProcess, modBase, typeIndex, TI_GET_TYPEID, &typeId))
  {
    if ( SymGetTypeInfo( _currentProcess, modBase, typeId, TI_GET_BASETYPE, &basicType ) )
    {
      DOUT(basicType2String(basicType));
      //ELOGNL("qbt 2: " << basicType);
      return basicType;
    }
  }
  DOUT(basicType2String(DbgType::btNoType));
  //ELOGNL("qbt 3: " << (int)DbgType::btNoType);
  return DbgType::btNoType;
}

void
setBasicType(DbgType::BasicType basicType, DWORD64 length, PVOID pAddress, DbgType& dbgType, int queryFlags)
{
  switch (basicType)
  {
  case DbgType::btNoType:
    break;
  case DbgType::btVoid:
    dbgType.typeName = DBGSTRING_CONSTSTR("void");
    dbgType.type = DbgType::Void; 
    if (length == 0)
      return;
    break;
  case DbgType::btBool:
    
    dbgType.typeName = DBGSTRING_CONSTSTR("bool");
    if (queryFlags & DbgFrameGetValues)
    {
      dbgType.type = DbgType::Boolean; 
      dbgType.val.boolean = *(bool*)pAddress;
    }
    return;
  case  DbgType::btChar:
    dbgType.typeName = DBGSTRING_CONSTSTR("char");
    if (queryFlags & DbgFrameGetValues)
    {
      dbgType.type = DbgType::Byte; 
      dbgType.val.byte = *(char*)pAddress;
    }
    DOUTNL(dbgType.val.byte);
    return;
  case DbgType::btWChar:
    dbgType.typeName = DBGSTRING_CONSTSTR("wchar");
    if (queryFlags & DbgFrameGetValues)
    {
      if (length == 2)
      {
        dbgType.type = DbgType::Short; 
        dbgType.val.shortInt = *(short*)pAddress;
      }
      else
      {
        dbgType.type = DbgType::Integer; 
        dbgType.val.integer = *(int*)pAddress;
      }
    }
    return;
  case DbgType::btULong:
  case DbgType::btLong:
  case DbgType::btUInt:
  case DbgType::btInt:
    if (length == 1)
    {
      dbgType.typeName = DBGSTRING_CONSTSTR("byte");
      if (queryFlags & DbgFrameGetValues)
      {
        dbgType.type = DbgType::Byte;
        dbgType.val.byte = *(char*)pAddress != 0;
      }
    }
    else if (length == 2)
    {
      dbgType.typeName = DBGSTRING_CONSTSTR("short");
      if (queryFlags & DbgFrameGetValues)
      {
        dbgType.type = DbgType::Short;
      
        dbgType.val.shortInt = *(short*)pAddress;
      }
    } 
    else if (length == 4)
    {
      dbgType.typeName = DBGSTRING_CONSTSTR("int");
      if (queryFlags & DbgFrameGetValues)
      {
        dbgType.type = DbgType::Integer;
        dbgType.val.integer = *(int*)pAddress;
      }
    }
    else
    {
      dbgType.typeName = DBGSTRING_CONSTSTR("int64");
      if (queryFlags & DbgFrameGetValues)
      {
        dbgType.type = DbgType::Long;
        dbgType.val.bigInt = *(__int64*)pAddress;
      }
    }
    return;
  case DbgType::btFloat:
    if (length == 8)
    {
      dbgType.typeName = DBGSTRING_CONSTSTR("double");
      if (queryFlags & DbgFrameGetValues)
      {
        dbgType.type = DbgType::Double;
        dbgType.val.largeFloat = *(double*)pAddress;
      }
    }
    else
    {
      dbgType.typeName = DBGSTRING_CONSTSTR("float");
      if (queryFlags & DbgFrameGetValues)
      {
        dbgType.type = DbgType::Float;
        dbgType.val.smallFloat = *(float*)pAddress;
      }
    }
    return;
  default:
    ELOGNL("unhandled basic data type: " << basicType << "; Length=" << (int)length << " Address=" << (void*)pAddress);
    break;
  /*
    btBCD = 9,
    btBool = 10,
    btLong = 13,
    btULong = 14,
    btCurrency = 25,
    btDate = 26,
    btVariant = 27,
    btComplex = 28,
    btBit = 29,
    btBSTR = 30,
    btHresult = 31
    */
  }
  
  //dbgType.type = DbgType::Unknown;
  if (DBGSTRING_SIZE(dbgType.typeName) == 0)
    dbgType.typeName = DBGSTRING_CONSTSTR("<unknown type with 0 size>");

  if (length == 1)
  {
    dbgType.val.byte = *(PBYTE)pAddress;
    if (dbgType.type == DbgType::Unknown)
      dbgType.type = DbgType::Byte;
    DOUTNL(" byte: " << dbgType.val.byte);
  }
  else if ( length == 2 )
  {
    dbgType.val.shortInt = *(PWORD)pAddress;
    if (dbgType.type == DbgType::Unknown)
      dbgType.type = DbgType::Short;
    DOUTNL(" short: " << dbgType.val.shortInt);
  }
  else if (length == 4)
  {
   dbgType.val.integer = *(PDWORD)pAddress;
   if (dbgType.type == DbgType::Unknown)
    dbgType.type = DbgType::Integer;
   DOUTNL(" integer: " << dbgType.val.integer);
  }
  else if ( length == 8 )
  {
    dbgType.val.bigInt = *(DWORD64*)pAddress;
    if (dbgType.type == DbgType::Unknown)
      dbgType.type = DbgType::Long;
    DOUTNL(" int64: " << (int)dbgType.val.bigInt);
  }
}




bool
queryArrayType(DWORD64 modBase, uintptr_t dwTypeIndex, DWORD_PTR address, DbgType& dbgType, intptr_t queryFlags)
{
  DWORD realTypeId = 0;
  DOUTNL("queryArrayType");
  HANDLE_FALSE_CALL(
    SymGetTypeInfo(_currentProcess, modBase, dwTypeIndex, TI_GET_TYPEID, &realTypeId),
    ;
  )
  DWORD elcount = 0;
  HANDLE_FALSE_CALL(
    SymGetTypeInfo(_currentProcess, modBase, dwTypeIndex, TI_GET_COUNT, &elcount),
    ;
  )

  
  DbgType::BasicType basicType = queryBasicType(dwTypeIndex, modBase);

  ULONG64 length;
  HANDLE_FALSE_CALL(
    SymGetTypeInfo(_currentProcess, modBase, dwTypeIndex, TI_GET_LENGTH, &length),
    ;
  )

  ULONG64 elementLength;
  HANDLE_FALSE_CALL(
    SymGetTypeInfo(_currentProcess, modBase, realTypeId, TI_GET_LENGTH, &elementLength),
    ;
  )

  dbgType.type = DbgType::Array;
  dbgType.typeName = DBGSTRING_CONSTSTR("[]");
  
  
  int64_t elsize = 0;
  if (elcount != 0) elsize = length / elcount;
  DOUT("Array: count=[" << elcount << "]; elsize=[" << elsize << "]; ");
  for (uintptr_t i = 0; i < elcount; ++i)
  {
    void* eladdr = *(void**)(address + (elsize * i));
    DbgType subtype;
    if (querySymbolType(modBase, realTypeId, (intptr_t)&eladdr, elsize, subtype, queryFlags) == true)
    {
      dbgType.addSubType(subtype);
    }
  }
  return true;
  /*
  if (basicType == DbgType::btInt)
  {
    dbgType.typeName = "int";
    int size = length / sizeof(int);
    DOUT("Array (" << size << "): [");
    
    int* iptr = (int*)address;
    for (int i = 0; i < size; ++i)
    {
      DbgType tmp;
      tmp.setInt(iptr[i]);
      dbgType.addSubType(tmp);
      DOUT(iptr[i] << ", ");
    }
    DOUT("] ");
    return true;
  }
  return false;
  */
}


bool
isBadStringPtr(const char* ptr, int64_t size)
{
  if (intptr_t(ptr) < 1024 || intptr_t(ptr) > 0xFFFFF000 || uintptr_t(ptr) == 0xcdcdcdcd || uintptr_t(ptr) == 0xCCCCCCCC)
    return true;
  return IsBadStringPtr(ptr, size) == TRUE;
}

bool
isBadReadPtr(const void* ptr, int64_t size)
{
  if (intptr_t(ptr) < 1024 || intptr_t(ptr) > 0xFFFFF000  || uintptr_t(ptr) == 0xcdcdcdcd)
    return true;
  return IsBadReadPtr(ptr, size) == TRUE;
}

bool
queryPoinerType(DWORD64 modBase, DWORD dwTypeIndex, DWORD_PTR address, DbgType& dbgType, intptr_t queryFlags)
{
  //ELOGNL("queryPoinerType: " << dbgType.name);
  DOUTNL("queryPoinerType");
  DbgType::BasicType bt = queryBasicType(dwTypeIndex, modBase);
  dbgType.type = DbgType::Pointer;
  DWORD realTypeId = 0;
  HANDLE_FALSE_CALL(
    SymGetTypeInfo(_currentProcess, modBase, dwTypeIndex, TI_GET_TYPEID, &realTypeId),
    ;
  )
  dwTypeIndex = realTypeId;
  ULONG64 length;
  HANDLE_FALSE_CALL(
    SymGetTypeInfo(_currentProcess, modBase, dwTypeIndex, TI_GET_LENGTH, &length),
    ;
  )
  dbgType.dataAddress = *(void**)address;
  if (bt == DbgType::btChar)
  {
    if (isBadStringPtr((char*)*(void**)address, 1024) == false)
    {
      dbgType.type = DbgType::String;
      dbgType.typeName = DBGSTRING_CONSTSTR("char*");
      dbgType.val.c_string = (char*)*(void**)address;
      return true;
    }
  }
  if (isBadReadPtr((void*)dbgType.dataAddress, length) == false)
  {
    if (MaxPointerRecursion < CurrentPointerRecursion)
    {
      dbgType.flags  |= DbgType::MaxPointerRecursionReached;
      return true;
    }
    ++CurrentPointerRecursion;
    dbgType.flags |= DbgType::IsValidPointer;
    DbgType subtype;
    if (querySymbolType(modBase, dwTypeIndex, (intptr_t)*(void**)address, length, subtype, queryFlags) == true)
      dbgType.addSubType(subtype);
    --CurrentPointerRecursion;

  }
  return true;
}

struct SymbolInfo
{
  DbgType dbgType;
  DWORD64 modBase;
  DWORD typeIndex;
  int address;
};

bool handleDbgType(DbgType& dbgType, bool& discarge)
{
  if (DBGSTRING_STARTSWITH(dbgType.typeName, "DbgType") == true)
  {
    discarge = true;
    return true;
  }
  return false;
}
RegisterDebugPreTypeHandler _register_dbgTypeHandler(handleDbgType);

bool handleSysType(DbgType& dbgType, bool& discarge)
{
  if (DBGSTRING_STARTSWITH(dbgType.name, "__") == true)
  {
    discarge = true;
    return true;
  }
  /*
  if (DBGSTRING_STARTSWITH(dbgType.name, "std::") == true)
  {
    discarge = true;
    return true;
  }*/
  return false;
}
RegisterDebugPreTypeHandler _register_sysTypeHandler(handleSysType);






bool 
querySymbolType(__int64 modBase, uintptr_t dwTypeIndex, DWORD_PTR offset, int64_t size, DbgType& dbgType, intptr_t queryFlags)
{
  intptr_t mb1 = (intptr_t)modBase;
  intptr_t mb2 = modBase >> 32;
  DOUTNL("querySymbolType: << modbase: " << mb1 << ":"<< mb2 << "; dwTypeIndex: " << dwTypeIndex
         << "; offset: " << offset << "; size=" << size);
  dbgType.size = size;
  dbgType.typeIndex = dwTypeIndex;
  dbgType.modBase = modBase;
  dbgType.dataAddress = (void*)offset;
  DWORD symbTag = 0;

  HANDLE_FALSE_CALL(
    SymGetTypeInfo( _currentProcess, modBase, (ULONG) dwTypeIndex, TI_GET_SYMTAG, &symbTag ),
      symbTag = -1;
    )
  
  DbgType::TagEnum tag = (DbgType::TagEnum)symbTag;
  WCHAR * pwszTypeName;
  if (SymGetTypeInfo(_currentProcess, modBase, (ULONG) dwTypeIndex, TI_GET_SYMNAME, &pwszTypeName))
  {
    char tbuf[2048];
    sprintf(tbuf, "%ls", pwszTypeName);
    dbgType.typeName = tbuf;
    DOUT(" typeName: " << dbgType.typeName);
    //pszCurrBuffer += sprintf( pszCurrBuffer, " %ls", pwszTypeName );
    LocalFree( pwszTypeName );
  }
  else
  {
    DOUT(" name: <unknown>");
  }
  /*
  if ((queryFlags & DbgFrameGetValues) == 0)
    return true;
  */
  int i;
  DbgVector<DebugTypeHandler>& prehandlers = getDebugTypePreHandlerArray();
  bool discarge = false;
  for (i = 0; i < prehandlers.size(); ++i)
    if (prehandlers[i](dbgType, discarge) == true)
      return discarge == false;
retry:
  switch (tag)
  {
  case -1:
  {
    DWORD dwChildrenCount = 0;
    SymGetTypeInfo(_currentProcess, modBase, (ULONG) dwTypeIndex, TI_GET_CHILDRENCOUNT, &dwChildrenCount);
    if (dwChildrenCount != 0)
    {
      tag = DbgType::SymTagBaseType;
      goto retry;
    }
    // try UDT
    break;    
  }
  case DbgType::SymTagFunctionType:
    return false;
  case DbgType::SymTagUDT:
    // continue below
    break;
  case DbgType::SymTagArrayType:
    return queryArrayType(modBase, dwTypeIndex, offset, dbgType, queryFlags);

  case DbgType::SymTagPointerType:
    //ELOGNL("DbgType::SymTagPointerType: " << DbgType::SymTagPointerType);
    return queryPoinerType(modBase, dwTypeIndex, offset, dbgType, queryFlags);
  case DbgType::SymTagEnum:
  {

    //break;
  }
  case DbgType::SymTagNull:
    DOUTNL("DbgType::SymTagNull");
    //return false;
  case DbgType::SymTagBaseType:
  {
    DbgType::BasicType basicType = queryBasicType(dwTypeIndex, modBase);
    //DOUT(" " << pSym->Name << ": ");
    //ELOGNL("1: " << basicType);
    setBasicType(basicType, size, (PVOID)offset, dbgType, queryFlags);
    return true;
  }
  case DbgType::SymTagTypedef:
    DOUTNL("DbgType::SymTagTypedef");
    return false;
  case DbgType::SymTagFunction:
    DOUTNL("DbgType::SymTagFunction");
    return false;
  
  default:
    ELOGNL("Unhandled DbgType tag:" << tag);
    break;
    //return false;
  }
  if ((queryFlags & DbgFrameGetValues) == 0)
    return true;
  
  DWORD dwChildrenCount = 0;
  SymGetTypeInfo(_currentProcess, modBase, dwTypeIndex, TI_GET_CHILDRENCOUNT, &dwChildrenCount);
    
  if (dwChildrenCount == 0)
    return false;

 
  struct FindChildren
  : TI_FINDCHILDREN_PARAMS
  {
    ULONG   _childs[1024];
    FindChildren()
    {
      memset(_childs, 0, sizeof(_childs));
      Count = sizeof(_childs) / sizeof(_childs[0]);
    }
  } ;
  FindChildren children;
  children.Count = dwChildrenCount;
  children.Start= 0;

  // Get the array of TypeIds, one for each child type
  if ( !SymGetTypeInfo(_currentProcess, modBase, dwTypeIndex, TI_FINDCHILDREN, &children))
      return false;
  dbgType.type = DbgType::User;
  DOUTNL("Component type:");
  // Iterate through each of the children
  for (i = 0; i < int(dwChildrenCount); i++ )
  {

    if (children.ChildId[i] == 0)
      break;
    if (tag == DbgType::SymTagEnum)
    {
      WCHAR * pwszTypeName = 0;
      
      if (SymGetTypeInfo(_currentProcess, modBase, children.ChildId[i], TI_GET_SYMNAME, &pwszTypeName))
      {
        char tbuf[2048];
        sprintf(tbuf, "%ls", pwszTypeName);
        LocalFree( pwszTypeName );
        // ## todo make something with it
      }
      DWORD typeId = 0;
      if (SymGetTypeInfo( _currentProcess, modBase, children.ChildId[i], TI_GET_TYPEID, &typeId ) == FALSE)
        continue;
      DbgType::BasicType basicType = queryBasicType(typeId, modBase);
      
      continue;
    }
    DWORD typeId = 0;
    HANDLE_FALSE_CALL_OK(
      SymGetTypeInfo(_currentProcess, modBase, children.ChildId[i], TI_GET_TYPEID, &typeId),
      continue;
    )
    DWORD symbTag = 0;
    HANDLE_FALSE_CALL_OK(
      SymGetTypeInfo( _currentProcess, modBase, typeId, TI_GET_SYMTAG, &symbTag),
      continue;
    )

    DbgType::TagEnum tag = (DbgType::TagEnum)symbTag;
    
    DbgType subType;
    
    DWORD dwMemberOffset = 0;
    HANDLE_FALSE_CALL_OK(
      SymGetTypeInfo( _currentProcess, modBase, children.ChildId[i], TI_GET_OFFSET, &dwMemberOffset),
      continue;
    )
   
    ULONG64 length = 0;
    HANDLE_FALSE_CALL(
      SymGetTypeInfo(_currentProcess, modBase, typeId, TI_GET_LENGTH, &length),
      ;
    )

    DWORD_PTR dwFinalOffset = offset + dwMemberOffset;
    subType.modBase = modBase;
    subType.typeIndex = typeId;

    if (DbgType::SymTagFunction != tag && DbgType::SymTagFunctionType != tag)
    {
      WCHAR * pwszTypeName = 0;
      if (SymGetTypeInfo(_currentProcess, modBase, children.ChildId[i], TI_GET_SYMNAME, &pwszTypeName))
      {
        char tbuf[2048];
        sprintf(tbuf, "%ls", pwszTypeName);
        subType.name = tbuf;
        DOUT(" name: " << subType.name);
        //pszCurrBuffer += sprintf( pszCurrBuffer, " %ls", pwszTypeName );
        LocalFree( pwszTypeName );
      }
    }
    switch (tag)
    {
    case DbgType::SymTagBaseType:
    {
      
      DbgType::BasicType basicType = queryBasicType(typeId, modBase);
      //ELOGNL("Tag: " << tag << " Basic type: " << basicType);
      if (basicType != DbgType::btNoType)
      {
        //ELOGNL("2: " << basicType);
        setBasicType(basicType, length, (PVOID)dwFinalOffset, subType, queryFlags);
        dbgType.addSubType(subType);
      }
      break;
    }
    case DbgType::SymTagArrayType:
      queryArrayType(modBase, typeId, dwFinalOffset, subType, queryFlags);
      dbgType.addSubType(subType);
      break;
      /*
    case DbgType::SymTagData:
    {
      DbgType::BasicType basicType = queryBasicType(children.ChildId[i], modBase);
      if (basicType != DbgType::btNoType)
      {
        setBasicType(basicType, length, (PVOID)dwFinalOffset, subType);
        dbgType.addSubType(subType);
        break;
      }
      break;
    }*/
    case DbgType::SymTagPointerType:
    {
      if (queryPoinerType(modBase, typeId, dwFinalOffset, subType, queryFlags) == true)
        dbgType.addSubType(subType);
      break;
    }
    case DbgType::SymTagUDT:
    {
      if (querySymbolType(modBase, typeId, dwFinalOffset, length, subType, queryFlags) == true)
      {
        dbgType.addSubType(subType);
      }
      break;
    }
    
      break;
    case DbgType::SymTagTypedef:
    case DbgType::SymTagFunctionType:
    case DbgType::SymTagFunction:
      break;
    case DbgType::SymTagEnum:
      //ELOGNL("1 Tag: " << tag << " Basic type: " << DbgType::btInt);    
      //ELOGNL("3: " << (int)DbgType::btInt);
      setBasicType(DbgType::btInt, length, (PVOID)dwFinalOffset, subType, queryFlags);
      dbgType.addSubType(subType);
      break;
    default:
      ELOGNL("Unhandled DbgType tag:" << tag);
      break;
    }
  }

  DbgVector<DebugTypeHandler>& vec = getDebugTypePostHandlerArray();
  for (i = 0; i < vec.size(); ++i)
    if (vec[i](dbgType, discarge) == true)
      return discarge == false;
  return true;
}



bool querySymbolValue(PSYMBOL_INFO pSym, DbgFrame& frame)
{
  DbgType::Flags symtype = DbgType::STUnknown;
  // Indicate if the variable is a local or parameter
  if (pSym->Flags & SYMFLAG_PARAMETER /*IMAGEHLP_SYMBOL_INFO_PARAMETER*/)
  {
    if ((frame.queryFlags & DbgFrameGetArguments) == 0)
      return false;
    symtype = DbgType::STParam;
    DOUT("  Param: ");
  }
  else if ( pSym->Flags & SYMFLAG_LOCAL/*IMAGEHLP_SYMBOL_INFO_LOCAL*/ )
  {
    if ((frame.queryFlags & DbgFrameGetLocals) == 0)
      return false;
    symtype = DbgType::STLocal;
    DOUT("  Local: ");
  }
  if ( pSym->Tag == DbgType::SymTagFunction)
    return false;
    
  DbgType dbgType;
  dbgType.flags |= symtype;
  dbgType.name =  pSym->Name;


  DWORD_PTR pVariable = 0;
  if (pSym->Flags & IMAGEHLP_SYMBOL_INFO_REGRELATIVE)
  {
    pVariable = frame.frame.offset;
    pVariable += (DWORD_PTR)pSym->Address;
  }
  else if ( pSym->Flags & IMAGEHLP_SYMBOL_INFO_REGISTER )
  {
    return false;   // Don't try to report register variable
  }
  else
  {
    pVariable = (DWORD_PTR)pSym->Address;   // It must be a global variable
    symtype = DbgType::STGlobal;
    dbgType.flags |= symtype;
    DOUT("  Global: ");
  }

    // Determine if the variable is a user defined type (UDT).  IF so, bHandled
    // will return true.
  bool bHandled = querySymbolType(pSym->ModBase, pSym->TypeIndex, pVariable, pSym->Size, dbgType, frame.queryFlags); 
  if (bHandled == false)
    return true;
  switch (symtype)
  {
  case DbgType::STParam:
    frame.params.push_back(dbgType);
    break;
  case DbgType::STLocal:
    frame.locals.push_back(dbgType);
    break;
  case DbgType::STGlobal:
    frame.globals.push_back(dbgType);
    break;
  }
  return true;
}

} // namespace pdbutils {