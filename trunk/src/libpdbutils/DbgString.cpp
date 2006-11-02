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

const char* findrT(const char* it1, const char* end1, const char* it2, const char* end2)
  {
  const char* realend1 = end1;
  const char* find1 = realend1;
  const char* spe2 = end2;

    while (spe2 > it2 && end1 > it1) 
  {
    --spe2;
    --end1;
    
    if (*spe2 != *end1)
    {
      spe2 = end2;
      if (find1 != realend1)
      {
        end1 = find1;
        find1 = realend1;
      }
    } 
    else 
    {
      if (find1 == realend1)
        find1 = end1;
    }
  }
  if (spe2 != it2) 
    return realend1;
  return end1;
}

#if !defined(DBGUTILS_USE_STD_STRING) && !defined(DBGUTILS_USE_MFC_CSTRING)

namespace pdbutils {

intptr_t
DbgString::lastIndexOf(const char* text) const
{
  const char* end = _ptr + strlen(_ptr);
  const char* f = findrT(_ptr, end, text, text + strlen(text));
  if (f == end)
    return -1;
  return f - _ptr;
}
DbgString 
DbgString::substr(intptr_t offset, intptr_t length) const
{
  intptr_t len = size();
  if (length == -1 && offset > len)
    return *this;
  if (offset + length > len)
    return *this;
  if (length == -1)
    return DbgString(_ptr + offset);
  else
    return DbgString(_ptr + offset, length);
}
DbgString 
DbgString::operator+(const char* text) const
{
  intptr_t s1 = size();
  intptr_t s2 = strlen(text);
  intptr_t size = s1 + s2 + 1;
  char* nt = new char[size + 1];
  memcpy(nt, _ptr, s1);
  memcpy(nt + s1, text, s2);
  nt[s1 + s2] = 0;
  return nt;
}

void 
DbgString::_release()
{
  if (_owns == false)
  {
    _ptr = 0;
    return;
  }
  if (_ptr = 0)
    return;
  delete[] _ptr;
  _ptr = 0;
}

void 
DbgString::_assign(const char* ptr, intptr_t length)
{
  if (ptr == 0)
  {
    _ptr = 0;
    return;
  }
  intptr_t size;
  if (length == -1)
    size = strlen(ptr);
  else
    size = length;
  _ptr = new char[size + 1];
  memcpy(_ptr, ptr, size + 1);
  _ptr[size] = 0;
}

} // namespace pdbutils {

#else if defined(DBGUTILS_USE_MFC_CSTRING)

int DBGUTILS_EXPORT CString_ReverseFind(const CString& str, const char* text)
{
  const char* b = str;
  const char* e = b + str.GetLength();
  const char* f = findrT(b, e, text, text + strlen(text));
  if (f == e)
    return -1;
  return f - b;
}

#endif // #if defined(DBGUTILS_USE_STD_STRING)