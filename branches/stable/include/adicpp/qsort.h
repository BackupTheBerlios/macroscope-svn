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
#ifndef QSortBSearchH
#define QSortBSearchH
//-------------------------------------------------------------------------------
namespace ksys {

//-------------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
void qSort(T * ptr, intptr_t lb, intptr_t ub)
{
  intptr_t  l, r, m;
  if( lb < ub ){
    if( ub - lb < 12 ){
      for( r = ub; r > lb; r-- ){
        for( m = r, l = m - 1; l >= lb; l-- )
          if( ptr[m] < ptr[l] )
            m = l;
        T t = ptr[m];
        ptr[m] = ptr[r];
        ptr[r] = t;
      }
    }
    else{
      m = lb + ((ub - lb) >> 1);
      l = lb, r = ub;
      for(;;){
        while( l < r && ptr[m] > ptr[l] ) l++;
        while( r >= l && ptr[m] < ptr[r] ) r--;
        if( l >= r )
          break;
        T t = ptr[l];
        ptr[l] = ptr[r];
        ptr[r] = t;
        if( r == m )
          m = l;
        else if( l == m )
          m = r;
        l++;
        r--;
      }
      qSort< T>(ptr, lb, r);
      qSort< T>(ptr, r + 1, ub);
    }
  }
}
//-------------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
void qSort(T * const ptr, intptr_t lb, intptr_t ub, intptr_t(*const f)(const T & p1, const T & p2))
{
  intptr_t  l, r, m;
  if( lb < ub ){
    if( ub - lb < 12 ){
      for( r = ub; r > lb; r-- ){
        for( m = r, l = m - 1; l >= lb; l-- )
          if( f(ptr[m], ptr[l]) < 0 )
            m = l;
        T t = ptr[m];
        ptr[m] = ptr[r];
        ptr[r] = t;
      }
    }
    else{
      m = lb + ((ub - lb) >> 1);
      l = lb, r = ub;
      for(;;){
        while( l < r && f(ptr[m], ptr[l]) > 0 ) l++;
        while( r >= l && f(ptr[m], ptr[r]) < 0 ) r--;
        if( l >= r ) break;
        T t = ptr[l];
        ptr[l] = ptr[r];
        ptr[r] = t;
        if( r == m )
          m = l;
        else if( l == m )
          m = r;
        l++;
        r--;
      }
      qSort< T>(ptr, lb, r, f);
      qSort< T>(ptr, r + 1, ub, f);
    }
  }
}
//-------------------------------------------------------------------------------
template< class T,intptr_t(*const f)(const T & p1, const T & p2)>
#ifndef __BCPLUSPLUS__
 inline
#endif
void qSort(T * const ptr, intptr_t lb, intptr_t ub)
{
  intptr_t  l, r, m;
  if( lb < ub ){
    if( ub - lb < 12 ){
      for( r = ub; r > lb; r-- ){
        for( m = r, l = m - 1; l >= lb; l-- )
          if( f(ptr[m], ptr[l]) < 0 )
            m = l;
        T t = ptr[m];
        ptr[m] = ptr[r];
        ptr[r] = t;
      }
    }
    else{
      m = lb + ((ub - lb) >> 1);
      l = lb, r = ub;
      for(;;){
        while( l < r && f(ptr[m], ptr[l]) > 0 )
          l++;
        while( r >= l && f(ptr[m], ptr[r]) < 0 )
          r--;
        if( l >= r )
          break;
        T t = ptr[l];
        ptr[l] = ptr[r];
        ptr[r] = t;
        if( r == m )
          m = l;
        else if( l == m )
          m = r;
        l++;
        r--;
      }
      qSort< T,f>(ptr, lb, r);
      qSort< T,f>(ptr, r + 1, ub);
    }
  }
}
//-------------------------------------------------------------------------------
template< class T,class PT>
#ifndef __BCPLUSPLUS__
 inline
#endif
void qSort(T * const ptr, intptr_t lb, intptr_t ub, intptr_t(*const f)(const T & p1, const T & p2, const PT & param), const PT & param)
{
  intptr_t  l, r, m;
  if( lb < ub ){
    if( ub - lb < 12 ){
      for( r = ub; r > lb; r-- ){
        for( m = r, l = m - 1; l >= lb; l-- )
          if( f(ptr[m], ptr[l], param) < 0 )
            m = l;
        T t = ptr[m];
        ptr[m] = ptr[r];
        ptr[r] = t;
      }
    }
    else{
      m = lb + ((ub - lb) >> 1);
      l = lb, r = ub;
      for(;;){
        while( l < r && f(ptr[m], ptr[l], param) > 0 )
          l++;
        while( r >= l && f(ptr[m], ptr[r], param) < 0 )
          r--;
        if( l >= r )
          break;
        T t = ptr[l];
        ptr[l] = ptr[r];
        ptr[r] = t;
        if( r == m )
          m = l;
        else if( l == m )
          m = r;
        l++;
        r--;
      }
      qSort< T>(ptr, lb, r, f, param);
      qSort< T>(ptr, r + 1, ub, f, param);
    }
  }
}
//-------------------------------------------------------------------------------
template< class T,class PT,
          intptr_t(*const f)(const T & p1, const T & p2, const PT & param)>
#ifndef __BCPLUSPLUS__
 inline
#endif
void qSort(T * const ptr, intptr_t lb, intptr_t ub, const PT & param)
{
  intptr_t  l, r, m;
  if( lb < ub ){
    if( ub - lb < 12 ){
      for( r = ub; r > lb; r-- ){
        for( m = r, l = m - 1; l >= lb; l-- )
          if( f(ptr[m], ptr[l], param) < 0 )
            m = l;
        T t = ptr[m];
        ptr[m] = ptr[r];
        ptr[r] = t;
      }
    }
    else{
      m = lb + ((ub - lb) >> 1);
      l = lb, r = ub;
      for(;;){
        while( l < r && f(ptr[m], ptr[l], param) > 0 )
          l++;
        while( r >= l && f(ptr[m], ptr[r], param) < 0 )
          r--;
        if( l >= r )
          break;
        T t = ptr[l];
        ptr[l] = ptr[r];
        ptr[r] = t;
        if( r == m )
          m = l;
        else if( l == m )
          m = r;
        l++;
        r--;
      }
      qSort< T>(ptr, lb, r, f, param);
      qSort< T>(ptr, r + 1, ub, f, param);
    }
  }
}
//-------------------------------------------------------------------------------
/*template <
  class T,
  class PT,
  long (*f)(const T & p1,const T & p2,PT param),
  void (*e)(T * ptr,long p1,long p2,PT param)
>
#ifndef __BCPLUSPLUS__
inline
#endif
void qSort(T * ptr,long lb,long ub,PT param)
{
  long l, r, m;
  if( lb < ub ){
    if( ub - lb < 12 ){
      for( r = ub; r > lb; r-- ){
        for( m = r, l = m - 1; l >= lb; l-- ) if( f(ptr[m],ptr[l]) < 0 ) m = l;
        e(ptr,m,r,param);
      }
    }
    else {
      m = lb + ((ub - lb) >> 1);
      l = lb, r = ub;
      for(;;){
        while( l < r && f(ptr[m],ptr[l]) > 0 ) l++;
        while( r >= l && f(ptr[m],ptr[r]) < 0 ) r--;
        if( l >= r ) break;
        e(ptr,l,r,param);
        if( r == m ) m = l; else if( l == m ) m = r;
        l++;
        r--;
      }
      qSort<T,PT,f,e>(ptr,    lb,  r, param);
      qSort<T,PT,f,e>(ptr, r + 1, ub, param);
    }
  }
}*/
//-------------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
intptr_t bSearch(const T * bsa, const T & item, intptr_t & c)
{
  intptr_t  low = 0, high = c - 1, pos = -1;

  c = 1;
  while( low <= high ){
    pos = (low + high) / 2;
    c = item > bsa[pos] ? 1 : item < bsa[pos] ? -1 : 0;
    if( c > 0 ){
      low = pos + 1;
    }
    else if( c < 0 ){
      high = pos - 1;
    }
    else
      break;
  }
  return pos;
}
//-------------------------------------------------------------------------------
template< class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
intptr_t bSearch(const T * bsa, const T & item, intptr_t & c, intptr_t(*const F)(const T &, const T &))
{
  intptr_t  low = 0, high = c - 1, pos = -1;

  c = 1;
  while( low <= high ){
    pos = (low + high) / 2;
    c = F(item, bsa[pos]);
    if( c > 0 ){
      low = pos + 1;
    }
    else if( c < 0 ){
      high = pos - 1;
    }
    else
      break;
  }
  return pos;
}
//-------------------------------------------------------------------------------
template< class T,intptr_t(*F)(const T &, const T &)>
#ifndef __BCPLUSPLUS__
 inline
#endif
intptr_t bSearch(const T * bsa, const T & item, intptr_t & c)
{
  intptr_t  low = 0, high = c - 1, pos = -1;

  c = 1;
  while( low <= high ){
    pos = (low + high) / 2;
    c = F(item, bsa[pos]);
    if( c > 0 ){
      low = pos + 1;
    }
    else if( c < 0 ){
      high = pos - 1;
    }
    else
      break;
  }
  return pos;
}
//-------------------------------------------------------------------------------
}
//-------------------------------------------------------------------------------
#endif
