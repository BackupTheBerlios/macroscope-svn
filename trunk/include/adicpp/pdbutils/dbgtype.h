/**
  Utils to make debugging running application under Windows
  Copyright Roger Rene Kommer (kommer@artefaktur.com) 2003

  dbgutils The sources either can be in a shared library (.dll) or
  has to be linked directly to the executable. 
  Compiling dbgutils as static library (.lib) won't work!

  Requirements dbghelp.h .dll .lib from microsoft
*/
#ifndef DbgType_h
#define DbgType_h


#pragma warning (disable : 4231)
#include "DbgString.h"
#include "DbgVector.h"

#ifdef __BORLANDC__
#pragma option push -w-inl
#endif

namespace pdbutils {
/**
 @addtogroup public_classes
 */
/*@{*/


class DbgType;
typedef DbgVector<DbgType> DbgTypeArray;

/*DBGUTILS_EXPORT_TEMPLATE*/ template class /*DBGUTILS_EXPORT*/ DbgVector<DbgType>;

/**
  An FrameAddress is an  memory address with
  to elements: offset and address.
  For example the call pdbutils::getProgramCounters fills an array of FrameAddress
*/
struct /*DBGUTILS_EXPORT*/ FrameAddress
{
  intptr_t offset;
  intptr_t address;
};

/**
  Wrapper to types and values obtained
  by the debug api.
*/
class /*DBGUTILS_EXPORT*/ DbgType
{
public:
  /**
    Used internally for identifing the symbol type
  */
  enum TagEnum
  {
   SymTagNull             = 0x0000,
   SymTagExe              = 0x0001,
   SymTagCompiland        = 0x0002,
   SymTagCompilandDetails = 0x0003,
   SymTagCompilandEnv     = 0x0004,  
   SymTagFunction         = 0x0005,  
   SymTagBlock            = 0x0006,  
   SymTagData             = 0x0007,  
   SymTagAnnotation       = 0x0008,  
   SymTagLabel            = 0x0009,  
   SymTagPublicSymbol     = 0x000A,  
   SymTagUDT              = 0x000B,  
   SymTagEnum             = 0x000C,  
   SymTagFunctionType     = 0x000D,  
   SymTagPointerType      = 0x000E,  
   SymTagArrayType        = 0x000F,  
   SymTagBaseType         = 0x0010,  
   SymTagTypedef          = 0x0011,  
   SymTagBaseClass        = 0x0012,  
   SymTagFriend,
   SymTagFunctionArgType, 
   SymTagFuncDebugStart, 
   SymTagFuncDebugEnd,
   SymTagUsingNamespace, 
   SymTagVTableShape,
   SymTagVTable,
   SymTagCustom,
   SymTagThunk,
   SymTagCustomType,
   SymTagManagedType,
   SymTagDimension
  };
  
  /**
    type held by symbol, user normally doesn't
    need this enumeration.
    Stolen from CVCONST.H in the DIA 2.0 SDK
  */
  enum BasicType  
  {
    btNoType = 0,
    btVoid = 1,
    btChar = 2,
    btWChar = 3,
    btInt = 6,
    btUInt = 7,
    btFloat = 8,
    /** Basic type is a binary-coded decimal  */
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
  };
  /**
    The real type this DbgType held.
    @see DbgType::type
  */
  enum ValueType
  {
    /**
      the value was not requested
    */
    Unset,
    /**
      The value type of the variable is unknown
    */
    Unknown,
    /**
      The type is void
    */
    Void,
    /**
      The value  is a bool.
      Values::boolean is set
    */
    Boolean,
    /**
      The value  is a byte or char.
      Values::byte is set
    */
    Byte,
    /**
      The value  is a short (16bit integer).
      Values::shortInt is set.
    */
    Short,
    /**
      The value is a 32bit integer.
      Values::integer is set.
    */
    Integer,
    /**
      The value is a 64bit integer.
      Values::bigInt is set
    */
    Long, // 64 bit
    /**
      The value is a 32bit float.
      Values::smallFloat ist set.
    */
    Float,
    /**
      The value is 64bit float.
      Values::largeFloat is set.
    */
    Double,
    /**
      The value is (probably) a character stream.
      Values::c_string is set.
    */
    String,
    /**
      The value is an array.
      Values::typevec contains the elements.
    */
    Array,
    /**
      The value is a component type
      Values::typevec contains the elements.
    */
    User,
    /**
      The value is a pointer
      If pointer is valid (Values::flags & IsValidPointer)
      Values::typevec[0] contains the value behind the pointer
    */
    Pointer,
  };
  /**
    Additional informations
    @see DbgType::flags
  */
  enum Flags
  {
    NoFlags                     = 0x0000,
    /**
      Pointer points to a valid range of memory.
      This doesn't really means the pointer is valid,
      only the memory is in a readable range.
    */
    IsValidPointer              = 0x0001,
    /**
      The Pointer held by this DbgType is not
      expanded, because maximum recursion is reached
    */
    MaxPointerRecursionReached  = 0x0002,
    /**
      It is unknown, to which frame type this
      type 
      */
    STUnknown                   = 0x0000,
    /**
      Variable is a method parameter
    */
    STParam                     = 0x0100,
    /**
      Variable is a local variable
    */
    STLocal                     = 0x0200,
    /**
      Variable is global variable
    */
    STGlobal                    = 0x0400
  };
  /** the FrameAddress this DbgType owns */
  FrameAddress frameAddress;
  /** variable name of the DbgType */
  DbgString name;
  /** type name of this DbgType */
  DbgString typeName;
  /**
    actual type of the value
    @see ValueType
  */
  ValueType type;
  /**
    Combination of DbgType::Flags
    @see DbgType::Flags
  */
  intptr_t flags;

  /** 
    used if is pointer 
    the value is stored in typevec
  */
  void* dataAddress;
  /**
    filled by debug api.
    The size describe the size of the used memory.
    Not every time equaly to sizeof(T)
  */
  int64_t size;
  /**
    internal used
  */
  intptr_t typeIndex;
  /**
    internal used
  */
  __int64 modBase;
  union Values
  {
    bool boolean;
    unsigned char byte;
    short shortInt;
    int integer;
    __int64 bigInt;
    float smallFloat;
    double largeFloat;
    char* c_string;
    //void* pointer;
    //HRESULT hresult;
    DbgVector<DbgType>* typevec;
  };
  /** union holds the value */
  Values val;
  /**
    Default contstructor.
    The type will be Unset
  */
  DbgType()
  : type(Unset)
  , flags(NoFlags)
  , dataAddress(0)
  , size(0)
  , typeIndex(0)
  , modBase(0)
  {
    frameAddress.offset = 0;
    frameAddress.address = 0;
    val.bigInt = 0;
  }
  /**
    Copy constructor
    If this type contains childs, the childs
    will also be copied (deep copy)
  */
  DbgType(const DbgType& other)
  : type(other.type)
  , flags(other.flags)
  , name(other.name)
  , typeName(other.typeName)
  , dataAddress(other.dataAddress)
  , size(other.size)
  , typeIndex(other.typeIndex)
  , modBase(other.modBase)
  {
    if (other.typeWithChilds() && other.val.typevec != 0)
      val.typevec = new DbgVector<DbgType>(*other.val.typevec);
    else
      val.bigInt = other.val.bigInt;
  }
  /**
    Assignment operator, see DbgType()
  */
  DbgType& operator=(const DbgType& other)
  {
    if (this == &other)
      return *this;
    _reset();
    type = other.type;
    flags = other.flags;
    dataAddress = other.dataAddress;
    name = other.name;
    typeName = other.typeName;
    size = other.size;
    typeIndex = other.typeIndex;
    modBase = other.modBase;
    if (other.typeWithChilds() && other.val.typevec != 0)
      val.typevec = new DbgVector<DbgType>(*other.val.typevec);
    else
      val.bigInt = other.val.bigInt;
    return *this;
  }
  ~DbgType()
  {
    _reset();
  }
  void setInt(int i)
  {
    type = Integer;
    val.integer = i;
  }
  /**
    @return true if this type has child(s). 
            which is true if type is an Array, User (struct/classes) or Pointer
  */
  bool typeWithChilds() const { return type == Array || type == User || type == Pointer; }
  /**
    Add a child type to this type.
    If type is not set to Array, User or Pointer type will be set to User
  */
  void addSubType(const DbgType& typ)
  {
    if (type != Array && type != User && type != Pointer)
      type = User;
    if (val.typevec == 0)
      val.typevec = new DbgVector<DbgType>();
    val.typevec->push_back(typ);
  }
  /**
    Try to find a member by given name
    @param name member variable name
    @param type returned type
    @return true if a member with given name was found
  */
  bool getMember(const DbgString& name, DbgType& type) const
  {
    if (val.typevec == 0)
      return false;
    for (intptr_t i = 0; i < val.typevec->size(); ++i)
    {
      if ((*val.typevec)[i].name == name)
      {
        type = (*val.typevec)[i];
        return true;
      }
    }
    return false;
  }
  /**
    The the variable on which this type points
    @param ptype the returned type
    @return true if this type is a pointer and the pointer points to valid object
  */
  bool getPointer(DbgType& ptype) const
  {
    if (type != Pointer || val.typevec == 0 || val.typevec->size() != 1)
      return false;
    ptype = (*val.typevec)[0];
    return true;
  }
  /**
    Get the nth child element
    @param idx child index 
    @param ptype returned child
    @return true if this type has a child on idx position
  */
  bool getElement(intptr_t idx, DbgType& ptype) const
  {
    if (type != Array && type != User && type != Pointer)
      return false;
    if (val.typevec == 0 || val.typevec->size() < idx + 1)
      return false;
    ptype = (*val.typevec)[idx];
    return true;
  }
  bool getFirstElement(DbgType& ptype) const
  {
    return getElement(0, ptype);
  }
  /**
    Render this type to a string
    @param printflags combination of DbgGetFrameFlags
  */
  DbgString toString(intptr_t printflags = 10) const;
  /**
    Render this type to a string
    @param printflags combination of DbgGetFrameFlags
    @param indendent string contains spaces which will be used for indent
  */
  DbgString toString(intptr_t printflags, const DbgString& indendent) const
  {
    DbgStream oss; 
    toString(oss, printflags, indendent);
    return oss.str();
  }

  DbgStream& toString(DbgStream& out, intptr_t printflags, const DbgString& indendent = "") const;
  /**
    Try to find an variable on given address inside this type.
    It only will find this variable or parent (in case of classes) or 
    member variables.
    @param addr Address 
    @param type returned type
    @return true if an type is found
  */
  bool findElementForAddress(void* addr, DbgType& type) const
  {
    if (addr == dataAddress)
    {
      type = *this;
      return true;
    }
    if (typeWithChilds() == true && val.typevec != 0)
    {
      for (intptr_t i = 0; i < val.typevec->size(); ++i)
      {
        if ((*val.typevec)[i].findElementForAddress(addr, type) == true)
          return true;
      }
    }
    return false;
  }
protected:
  void _reset()
  {
    if (typeWithChilds() == false || val.typevec == 0)
      return;
    delete val.typevec;
    type = Unknown;
  }
};

/** @} */

} // namespace pdbutils {

#ifdef __BORLANDC__
#pragma option pop
#endif

#endif //DbgType_h