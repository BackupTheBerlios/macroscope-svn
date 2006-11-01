
#ifndef DbgVector_h
#define DbgVector_h


namespace pdbutils {
/**
  @addtogroup helper_classes
  @{
*/
/**
  
  Minimal vector class, following roughly the std::vector
  to enable freestanding implementation
  @author Roger Rene Kommer (kommer@artefaktur.com)
*/
template <typename T>
class DbgVector
{
  T* _begin;
  T* _end;
  T* _bufEnd;
public:
  typedef T* iterator;
  DbgVector() 
  : _begin(0)
  , _end(0)
  , _bufEnd(0)
  {
    //_ensureCap(10);
  }
  DbgVector(int capacity)
  : _begin(0)
  , _end(0)
  , _bufEnd(0)
  {
    _ensureCap(capacity);
  }
  DbgVector(const DbgVector<T>& other)
  : _begin(0)
  , _end(0)
  , _bufEnd(0)
  {
    _ensureCap(other.capacity());
    intptr_t osize = other.size();
    for (intptr_t i = 0; i < osize; ++i)
      push_back(other[i]);
  }
  ~DbgVector()
  {
    char* oldptr = (char*)_begin;
    delete[] oldptr;
  }
  intptr_t size() const { return _end - _begin; }
  intptr_t capacity() const { return _bufEnd - _begin; }
  void push_back(const T& t) 
  {
    _ensureCap(size() + 1);
    new ((void*)_end)T(t);
    ++_end;
  }

  T& operator[](intptr_t idx) { return _begin[idx]; }
  const T& operator[](intptr_t idx) const { return _begin[idx]; }
  void ensureCapacity(intptr_t cap)
  {
    _ensureCap(cap);
  }
  iterator begin() { return _begin; }
  iterator end() { return _end; }
  iterator bufend() { return _bufEnd; }
  void setEnd(iterator it) { _end = it; }
protected:
  void _ensureCap(intptr_t cap)
  {
    intptr_t oc = capacity();
    if (oc >= cap) return;
    intptr_t newcap = oc * 2;
    if (newcap < cap) newcap = cap;
    if (newcap < 10) newcap = 10;
    intptr_t s = size();
    char* newBuf = new char[sizeof(T) * newcap];
    memcpy(newBuf, _begin, s * sizeof(T));

    char* oldptr = (char*)_begin;
    _begin = (T*)newBuf;
    _end = _begin + s;
    _bufEnd = _begin + newcap;
    if (oldptr != 0) delete oldptr;
  }
};
/** @} */

} // namespace pdbutils 

#endif //dbgutils_h