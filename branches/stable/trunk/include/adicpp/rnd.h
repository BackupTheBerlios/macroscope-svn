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
#ifndef RandomizerH
#define RandomizerH
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/*

    C++ TEMPLATE VERSION OF Robert J. Jenkins Jr.'s
    ISAAC Random Number Generator.

    Ported from vanilla C to to template C++ class
    by Quinn Tyler Jackson on 16-23 July 1998.

        quinn@qtj.net

    The function for the expected period of this
    random number generator, according to Jenkins is:

        f(a,b) = 2**((a+b*(3+2^^a)-1)

        (where a is ALPHA and b is bitwidth)

    So, for a bitwidth of 32 and an ALPHA of 8,
    the expected period of ISAAC is:

        2^^(8+32*(3+2^^8)-1) = 2^^8295

    Jackson has been able to run implementations
    with an ALPHA as high as 16, or

        2^^2097263

*/

template <uintptr_t ALPHA = 8, class T = uint64_t> class QTIsaac {
  public:
    enum { N = (1<<ALPHA) };
    struct randctx {
       T randcnt;
       T randrsl[N];
       T randmem[N];
       T randa;
       T randb;
       T randc;

//       randctx()/* : randrsl(N), randmem(N)*/ {}
//       ~randctx(){}
    };
    randctx m_rc;

    void isaac(randctx* ctx);
    T ind(T* mm, T x);
    void rngstep(T mix, T& a, T& b, T*& mm, T*& m, T*& m2, T*& r, T& x, T& y);
    void shuffle(T& a, T& b, T& c, T& d, T& e, T& f, T& g, T& h);

    QTIsaac(T a = 0, T b = 0, T c = 0);
    ~QTIsaac(void);

    T rand(void);
    void randinit(randctx* ctx, bool bUseSeed);
    void srand(T a = 0, T b = 0, T c = 0, T* s = NULL);
};


template<uintptr_t ALPHA, class T> inline QTIsaac<ALPHA,T>::QTIsaac(T a, T b, T c)
{
  srand(a, b, c);
}

template<uintptr_t ALPHA, class T> inline QTIsaac<ALPHA,T>::~QTIsaac(void)
{
  /* DO NOTHING */
}

template<uintptr_t ALPHA, class T>
#ifndef __BCPLUSPLUS__
inline
#endif
void QTIsaac<ALPHA,T>::srand(T a, T b, T c, T* s)
{
  for( intptr_t i = 0; i < N; i++ ) m_rc.randrsl[i] = s != NULL ? s[i] : 0;
  m_rc.randa = a;
  m_rc.randb = b;
  m_rc.randc = c;
  randinit(&m_rc, true);
}

template<uintptr_t ALPHA, class T> inline T QTIsaac<ALPHA,T>::rand(void)
{
  return (
    !m_rc.randcnt-- ?
      (isaac(&m_rc),
       m_rc.randcnt = N - 1,
       m_rc.randrsl[(unsigned) m_rc.randcnt]
      ) :
      m_rc.randrsl[(unsigned) m_rc.randcnt]
  );
}

template<uintptr_t ALPHA, class T>
#ifndef __BCPLUSPLUS__
inline
#endif
void QTIsaac<ALPHA,T>::randinit(randctx * ctx, bool bUseSeed)
{
  T a,b,c,d,e,f,g,h;

  a = b = c = d = e = f = g = h = UINT64_C(0x9e3779b97f4a7c13);

  T * m = (ctx->randmem);
  T * r = (ctx->randrsl);

  if( !bUseSeed ){
     ctx->randa = 0;
     ctx->randb = 0;
     ctx->randc = 0;
  }
  intptr_t i;
// scramble it
  for( i=0; i < 4; ++i ){
    shuffle(a,b,c,d,e,f,g,h);
  }
  if( bUseSeed ){
    // initialize using the contents of r[] as the seed

     for( i = 0; i < N; i += 8 ){
        a+=r[i  ]; b+=r[i+1]; c+=r[i+2]; d+=r[i+3];
        e+=r[i+4]; f+=r[i+5]; g+=r[i+6]; h+=r[i+7];

        shuffle(a,b,c,d,e,f,g,h);

        m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
        m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
     }

    //do a second pass to make all of the seed affect all of m

     for( i = 0; i < N; i += 8){
        a+=m[i  ]; b+=m[i+1]; c+=m[i+2]; d+=m[i+3];
        e+=m[i+4]; f+=m[i+5]; g+=m[i+6]; h+=m[i+7];

        shuffle(a,b,c,d,e,f,g,h);

        m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
        m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
     }
  }
  else {
    // fill in mm[] with messy stuff

     shuffle(a,b,c,d,e,f,g,h);

     m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
     m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
  }

  isaac(ctx);         // fill in the first set of results
  ctx->randcnt = N;   // prepare to use the first set of results
}

template<uintptr_t ALPHA, class T> inline T QTIsaac<ALPHA,T>::ind(T * mm, T x)
{
  return *(T *) ( (unsigned char *) mm + unsigned(x & ((N - 1) << 3)) );
}

template<uintptr_t ALPHA, class T> inline
void QTIsaac<ALPHA,T>::rngstep(T mix, T& a, T& b, T*& mm, T*& m, T*& m2, T*& r, T& x, T& y)
{
  x = *m;
  a = (a^(mix)) + *(m2++);
  *(m++) = y = ind(mm,x) + a + b;
  *(r++) = b = ind(mm,y>>ALPHA) + x;
}

template<uintptr_t ALPHA, class T> inline
void QTIsaac<ALPHA,T>::shuffle(T& a, T& b, T& c, T& d, T& e, T& f, T& g, T& h)
{
  a-=e; f^=h>>9;  h+=a;
  b-=f; g^=a<<9;  a+=b;
  c-=g; h^=b>>23; b+=c;
  d-=h; a^=c<<15; c+=d;
  e-=a; b^=d>>14; d+=e;
  f-=b; c^=e<<20; e+=f;
  g-=c; d^=f>>17; f+=g;
  h-=d; e^=g<<14; g+=h;
}

template<uintptr_t ALPHA, class T>
#ifndef __BCPLUSPLUS__
inline
#endif
void QTIsaac<ALPHA,T>::isaac(randctx* ctx)
{
  T x,y;

  T* mm = ctx->randmem;
  T* r  = ctx->randrsl;

  T a = (ctx->randa);
  T b = (ctx->randb + (++ctx->randc));

  T* m    = mm;
  T* m2   = (m+(N/2));
  T* mend = m2;

  for(; m<mend; ){
    rngstep(~(a^(a<<21)), a, b, mm, m, m2, r, x, y);
    rngstep(  a^(a>>5)  , a, b, mm, m, m2, r, x, y);
    rngstep(  a^(a<<12) , a, b, mm, m, m2, r, x, y);
    rngstep(  a^(a>>33) , a, b, mm, m, m2, r, x, y);
  }
  m2 = mm;
  for(; m2<mend; ){
    rngstep(~(a^(a<<21)), a, b, mm, m, m2, r, x, y);
    rngstep(  a^(a>>5)  , a, b, mm, m, m2, r, x, y);
    rngstep(  a^(a<<12) , a, b, mm, m, m2, r, x, y);
    rngstep(  a^(a>>33) , a, b, mm, m, m2, r, x, y);
  }
  ctx->randb = b;
  ctx->randa = a;
}

//---------------------------------------------------------------------------
class Randomizer : public QTIsaac<> {
  typedef QTIsaac<> Inherited;
  private:
  public:
    ~Randomizer();
    Randomizer();

    Randomizer & randomize();
    Randomizer & srand(uint64_t a);
    uint64_t random(uint64_t m = ~UINT64_C(0));
    uint64_t random2(uint64_t m = UINT64_C(0));
    uint64_t zrandom();
};
//---------------------------------------------------------------------------
inline Randomizer::~Randomizer()
{
}
//---------------------------------------------------------------------------
inline Randomizer::Randomizer()
{
}
//---------------------------------------------------------------------------
inline uint64_t Randomizer::random2(uint64_t m)
{ 
  return rand() & (m - 1); 
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif

