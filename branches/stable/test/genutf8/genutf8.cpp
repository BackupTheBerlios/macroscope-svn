/*-
 * Copyright 2005-2007 Guram Dukashvili
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
#include <adicpp/bootconf.h>
#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <sys\stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef __INTEL_COMPILER
#pragma warning disable 268 810
#endif

const char lic[] =
"/*-\n"
" * Copyright 2005 Guram Dukashvili\n"
" * All rights reserved.\n"
" *\n"
" * Redistribution and use in source and binary forms, with or without\n"
" * modification, are permitted provided that the following conditions\n"
" * are met:\n"
" * 1. Redistributions of source code must retain the above copyright\n"
" *    notice, this list of conditions and the following disclaimer.\n"
" * 2. Redistributions in binary form must reproduce the above copyright\n"
" *    notice, this list of conditions and the following disclaimer in the\n"
" *    documentation and/or other materials provided with the distribution.\n"
" *\n"
" * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND\n"
" * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n"
" * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n"
" * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE\n"
" * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n"
" * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS\n"
" * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n"
" * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\n"
" * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\n"
" * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\n"
" * SUCH DAMAGE.\n"
" */\n"
"//---------------------------------------------------------------------------\n"
"// WARNING: MACHINE GENERATED, PLEASE DO NOT EDIT\n"
"//---------------------------------------------------------------------------\n"
"#include <adicpp/lconfig.h>\n"
"#include <adicpp/utf8embd.h>\n"
"//---------------------------------------------------------------------------\n"
"namespace utf8 {\n"
"//---------------------------------------------------------------------------\n"
;

#ifdef __BCPLUSPLUS__
using namespace std;
#endif

#if _MSC_VER
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#endif

#define EVEN_SIZE_BIT(a,b) ((a) * (b) / 8 + ((a) * (b) % 8 > 0))
#define EVEN_SIZE(a,b) EVEN_SIZE_BIT(a,b) + 4 - (EVEN_SIZE_BIT(a,b) & (sizeof(uint32_t) - 1))

uint8_t C1Table[EVEN_SIZE(65536u,9u)];
uint8_t C2Table[65536 / 2];
uint8_t C3Table[EVEN_SIZE(65536u,11u)];

struct UpperLower {
    uint16_t  bl, br, c;
};

uint16_t  UpperTable[65536 * 2];
intptr_t  UpperCount;
uint16_t  LowerTable[65536 * 2];
intptr_t  LowerCount;

struct CPEnum {
    uint32_t  cp;
    CPINFOEX  info;
};

CPEnum *  CPEnums;
intptr_t  CPEnumCount;

#define DST_DIR "..\\..\\..\\src\\libutf8"

static void GenEmbed()
{
  struct stat st;
  char        bfile[64];
  intptr_t    i, j, k, g;
  FILE *      file, * f;
  void *      b;

  file = fopen(DST_DIR "\\utf8uct.cpp", "wb");
  assert( file != NULL );
  fprintf(file,"%s",lic);
  k = UpperCount * 2;
  fprintf(file, "extern const uint16_t upperTable["PRIdPTR"] = {\n", k);
  for( i = 0; i < k; ){
    fprintf(file, "  ");
    for( j = 0; j < 8 && i < k; j++, i++ ){
      if( j > 0 )
        fprintf(file, " ");
      fprintf(file, "0x%04X", UpperTable[i]);
      if( i < k - 1 )
        fprintf(file, ",");
    }
    if( i < k )
      fprintf(file, "\n");
  }
  fprintf(file, "\n};\n");
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "} // namespace utf8\n"
    "//---------------------------------------------------------------------------\n"
  );
  fclose(file);
  file = fopen(DST_DIR "\\utf8lct.cpp", "wb");
  assert( file != NULL );
  fprintf(file,"%s",lic);
  k = LowerCount * 2;
  fprintf(file, "extern const uint16_t lowerTable["PRIdPTR"] = {\n", k);
  for( i = 0; i < k; ){
    fprintf(file, "  ");
    for( j = 0; j < 8 && i < k; j++, i++ ){
      if( j > 0 )
        fprintf(file, " ");
      fprintf(file, "0x%04X", LowerTable[i]);
      if( i < k - 1 )
        fprintf(file, ",");
    }
    if( i < k )
      fprintf(file, "\n");
  }
  fprintf(file, "\n};\n");
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "} // namespace utf8\n"
    "//---------------------------------------------------------------------------\n"
  );
  fclose(file);
  file = fopen(DST_DIR "\\utf8c1.cpp", "wb");
  assert( file != NULL );
  fprintf(file,"%s",lic);
  k = sizeof(C1Table) / sizeof(uint32_t);
  fprintf(file, "extern const uint32_t C1Table["PRIdPTR"] = {\n", k);
  for( i = 0; i < k; ){
    fprintf(file, "  ");
    for( j = 0; j < 4 && i < k; j++, i++ ){
      if( j > 0 )
        fprintf(file, " ");
      fprintf(file, "0x%08X", *((uint32_t *) C1Table + i));
      if( i < k - 1 )
        fprintf(file, ",");
    }
    if( i < k )
      fprintf(file, "\n");
  }
  fprintf(file, "\n};\n");
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "uintptr_t getC1Type(uintptr_t c)\n"
    "{\n"
    "  uintptr_t i = c * 9u / 8u;\n"
    "  if( i >= sizeof(C1Table) / sizeof(C1Table[0]) ) return 0;\n"
    "  return *(uint16_t *) ((uint8_t *) C1Table + i) >> (c * 9u % 8u);\n"
    "}\n"
    "//---------------------------------------------------------------------------\n"
    "} // namespace utf8\n"
    "//---------------------------------------------------------------------------\n"
  );
  fclose(file);
  file = fopen(DST_DIR "\\utf8c2.cpp", "wb");
  assert( file != NULL );
  fprintf(file,"%s",lic);
  k = sizeof(C2Table) / sizeof(uint32_t);
  fprintf(file, "extern const uint32_t C2Table["PRIdPTR"] = {\n", k);
  for( i = 0; i < k; ){
    fprintf(file, "  ");
    for( j = 0; j < 4 && i < k; j++, i++ ){
      if( j > 0 )
        fprintf(file, " ");
      fprintf(file, "0x%08X", *((uint32_t *) C2Table + i));
      if( i < k - 1 )
        fprintf(file, ",");
    }
    if( i < k )
      fprintf(file, "\n");
  }
  fprintf(file, "\n};\n");
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "uintptr_t getC2Type(uintptr_t c)\n"
    "{\n"
    "  uintptr_t i = c * 4u / 8u;\n"
    "  if( i >= sizeof(C2Table) / sizeof(C2Table[0]) ) return 0;\n"
    "  return *((uint8_t *) C2Table + i) >> (c * 4u % 8u);\n"
    "}\n"
    "//---------------------------------------------------------------------------\n"
    "} // namespace utf8\n"
    "//---------------------------------------------------------------------------\n"
  );
  fclose(file);
  file = fopen(DST_DIR "\\utf8c3.cpp", "wb");
  assert( file != NULL );
  fprintf(file,"%s",lic);
  k = sizeof(C3Table) / sizeof(uint32_t);
  fprintf(file, "extern const uint32_t C3Table["PRIdPTR"] = {\n", k);
  for( i = 0; i < k; ){
    fprintf(file, "  ");
    for( j = 0; j < 4 && i < k; j++, i++ ){
      if( j > 0 )
        fprintf(file, " ");
      fprintf(file, "0x%08X", *((uint32_t *) C3Table + i));
      if( i < k - 1 )
        fprintf(file, ",");
    }
    if( i < k )
      fprintf(file, "\n");
  }
  fprintf(file, "\n};\n");
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "uintptr_t getC3Type(uintptr_t c)\n"
    "{\n"
    "  uintptr_t i = imul11(c) / 8u;\n"
    "  if( i >= sizeof(C3Table) / sizeof(C3Table[0]) ) return 0;\n"
    "  return *(uint16_t *) ((uint8_t *) C3Table + i) >> (imul11(c) % 8u);\n"
    "}\n"
    "} // namespace utf8\n"
    "//---------------------------------------------------------------------------\n"
  );
  fclose(file);
  for( i = 0; i < CPEnumCount; i++ ){
    sprintf(bfile, DST_DIR "\\cp"PRIu32".cpp", CPEnums[i].cp);
    file = fopen(bfile, "wb");
    assert( file != NULL );
    fprintf(file,"%s",lic);
    sprintf(bfile, DST_DIR "\\cp"PRIu32"utf8.bin", CPEnums[i].cp);
    stat(bfile, &st);
    f = fopen(bfile, "rb");
    assert( file != NULL );
    b = malloc(st.st_size);
    fread(b, st.st_size, 1, f);
    fclose(f);
    k = st.st_size;
    fprintf(file,
      "#if defined(EMBED_CP"PRIu32"_SUPPORT) || defined(EMBED_ALL_CP)\n"
      "extern const uint8_t cp"PRIu32"s2utf8s["PRIdPTR"] = {\n",
      CPEnums[i].cp,
      CPEnums[i].cp,
      k
    );
    for( g = 0; g < k; ){
      fprintf(file, "  ");
      for( j = 0; j < 8 && g < k; j++, g++ ){
        if( j > 0 )
          fprintf(file, " ");
        fprintf(file, "0x%02X", *((uint8_t *) b + g));
        if( g < k - 1 )
          fprintf(file, ",");
      }
      if( g < k )
        fprintf(file, "\n");
    }
    free(b);
    fprintf(file,
      "\n};\n"
      "//---------------------------------------------------------------------------\n"
    );
    sprintf(bfile,DST_DIR "\\utf8cp"PRIu32".bin", CPEnums[i].cp);
    stat(bfile, &st);
    f = fopen(bfile, "rb");
    assert( file != NULL );
    b = malloc(st.st_size);
    fread(b, st.st_size, 1, f);
    fclose(f);
    k = st.st_size;
    fprintf(file, "extern const uint8_t utf8s2cp"PRIu32"s["PRIdPTR"] = {\n", CPEnums[i].cp, k);
    for( g = 0; g < k; ){
      fprintf(file, "  ");
      for( j = 0; j < 8 && g < k; j++, g++ ){
        if( j > 0 ) fprintf(file, " ");
        fprintf(file, "0x%02X", *((uint8_t *) b + g));
        if( g < k - 1 ) fprintf(file, ",");
      }
      if( g < k ) fprintf(file, "\n");
    }
    free(b);
    fprintf(file,
      "\n};\n"
      "#endif\n"
      "//---------------------------------------------------------------------------\n"
      "} // namespace utf8\n"
      "//---------------------------------------------------------------------------\n"
    );
    fclose(file);
  }
  file = fopen(DST_DIR "\\utf8cp.cpp", "wb");
  assert( file != NULL );
  fprintf(file,"%s",lic);
  for( i = 0; i < CPEnumCount; i++ ){
    fprintf(file,
      "extern const uint8_t utf8s2cp"PRIu32"s[];\n"
      "extern const uint8_t cp"PRIu32"s2utf8s[];\n",
      CPEnums[i].cp,
      CPEnums[i].cp
    );
  }
  fprintf(file, "extern const utf8cp utf8cps[] = {\n");
  for( i = 0; i < CPEnumCount; i++ ){
    if( i > 0 )
      fprintf(file, ",\n#endif\n");
    fprintf(file,
      "#if defined(EMBED_CP"PRIu32"_SUPPORT) || defined(EMBED_ALL_CP)\n"
      "  { cp"PRIu32"s2utf8s, utf8s2cp"PRIu32"s }",
      CPEnums[i].cp,
      CPEnums[i].cp,
      CPEnums[i].cp
    );
  }
  fprintf(file,
    "\n"
    "#endif\n"
    "};\n"
  );
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "extern const uintptr_t utf8cpsCount = sizeof(utf8cps) / sizeof(utf8cps[0]);\n"
    "//---------------------------------------------------------------------------\n"
    "} // namespace utf8\n"
    "//---------------------------------------------------------------------------\n");
  fclose(file);
}

struct Sequence {
    intptr_t Pos;
    intptr_t Len;
};

static intptr_t sortCPSequences(const void * a, const void * b)
{
  return -(((Sequence *) a)->Len - ((Sequence *) b)->Len);
} 

static intptr_t sortCPSequences2(const void * a, const void * b)
{
  return ((Sequence *) a)->Pos - ((Sequence *) b)->Pos;
} 

struct cp2ucsSeq {
    uint8_t   c[6];
    uint16_t  ucs;
};

struct cp2ucs {
    CPEnum *    pcp;
    cp2ucsSeq * Seqs;
    int         SeqCount;
};

static
#ifndef __BCPLUSPLUS__
inline
#endif
bool isLeadByte(const CPINFOEX & info, intptr_t byte)
{
  for( intptr_t k = 0; k < MAX_LEADBYTES; k += 2 ){
    if( info.LeadByte[k] == 0 && info.LeadByte[k + 1] == 0 )
      break;
    if( byte >= (intptr_t) info.LeadByte[k] && byte <= (intptr_t) info.LeadByte[k + 1] )
      return true;
  }
  return false;
}

static void findCPSequences(Sequence *& Seqs, intptr_t & SeqCount, intptr_t & mSeqCount, const cp2ucs * cp)
{
  Seqs = NULL;
  SeqCount = 0;
  mSeqCount = 0;
  intptr_t Pos = 0;
  while( Pos < cp->SeqCount ){
    if( !isLeadByte(cp->pcp->info, cp->Seqs[Pos].c[0]) ){
      Seqs = (Sequence *) realloc(Seqs, sizeof(Seqs[0]) * (SeqCount + 1));
      Seqs[SeqCount].Pos = Pos;
      Seqs[SeqCount].Len = 1;
      while( Pos + 1 < cp->SeqCount && !isLeadByte(cp->pcp->info, cp->Seqs[Pos + 1].c[0]) && cp->Seqs[Pos + 1].c[0] - cp->Seqs[Pos].c[0] == 1 && cp->Seqs[Pos + 1].ucs - cp->Seqs[Pos].ucs == 1 ){
        Seqs[SeqCount].Len++;
        Pos++;
      }
      if( Seqs[SeqCount].Len > 1 )
        mSeqCount++;
      SeqCount++;
    }
    Pos++;
  }
  //  qsort(Seqs,SeqCount,sizeof(Sequence),sortCPSequences);
}

static
#ifndef __BCPLUSPLUS__
inline
#endif
intptr_t memncmp(const void * m1, const void * m2, unsigned n)
{
  intptr_t c = 0;
  while( n-- > 0 ){
    c = *(const unsigned char *) m1 - *(const unsigned char *) m2;
    if( c != 0 )
      break;
    *(const unsigned char **) m1 += 1;
    *(const unsigned char **) m2 += 1;
  }
  return c;
}

static
#ifndef __BCPLUSPLUS__
inline
#endif
intptr_t strlen(const uint8_t * s1)
{
  const unsigned char * s = s1;
  while( *s != '\0' )
    s++;
  return s - s1;
}

static
#ifndef __BCPLUSPLUS__
inline
#endif
uint8_t * strchr(const uint8_t * s, uintptr_t c)
{
  while( *s != '\0' )
    if( (uintptr_t) *s == c )
      return (uint8_t *) s;
    else
      s++;
  return NULL;
}

static void findLBCPSequences(Sequence *& Seqs, intptr_t & SeqCount, intptr_t & mSeqCount, const cp2ucs * cp)
{
  Seqs = NULL;
  SeqCount = 0;
  mSeqCount = 0;
  intptr_t Pos = 0;
  while( Pos < cp->SeqCount ){
    if( isLeadByte(cp->pcp->info, cp->Seqs[Pos].c[0]) ){
      Seqs = (Sequence *) realloc(Seqs, sizeof(Seqs[0]) * (SeqCount + 1));
      Seqs[SeqCount].Pos = Pos;
      Seqs[SeqCount].Len = 1;
      while( Pos + 1 < cp->SeqCount && isLeadByte(cp->pcp->info, cp->Seqs[Pos + 1].c[0]) && memncmp(cp->Seqs[Pos + 1].c, cp->Seqs[Pos].c, 6) == 1 && cp->Seqs[Pos + 1].ucs - cp->Seqs[Pos].ucs == 1 ){
        Seqs[SeqCount].Len++;
        Pos++;
      }
      if( Seqs[SeqCount].Len > 1 )
        mSeqCount++;
      SeqCount++;
    }
    Pos++;
  }
}

static void findCPSequencesR(Sequence *& Seqs, intptr_t & SeqCount, const cp2ucs * cp)
{
  Seqs = NULL;
  SeqCount = 0;
  intptr_t Pos = 0;
  while( Pos < cp->SeqCount ){
    Seqs = (Sequence *) realloc(Seqs, sizeof(Seqs[0]) * (SeqCount + 1));
    Seqs[SeqCount].Pos = Pos;
    Seqs[SeqCount].Len = 1;
    while( Pos + 1 < cp->SeqCount && cp->Seqs[Pos + 1].ucs - cp->Seqs[Pos].ucs == 1 && memncmp(cp->Seqs[Pos + 1].c, cp->Seqs[Pos].c, 6) == 1 ){
      Seqs[SeqCount].Len++;
      Pos++;
    }
    SeqCount++;
    Pos++;
  }
}

static void dumpCP2UCS()
{
  char      bfile[256];
  FILE *    file;
  intptr_t  i, j, k, g, r;
  cp2ucs *  cp2ucsSeqs      = NULL;
  intptr_t  cp2ucsSeqCount  = 0;
  for( i = 0; i < CPEnumCount; i++ ){
    cp2ucsSeqs = (cp2ucs *) realloc(cp2ucsSeqs, sizeof(cp2ucs) * (cp2ucsSeqCount + 1));
    cp2ucs *  Seq = cp2ucsSeqs + cp2ucsSeqCount;
    Seq->pcp = CPEnums + i;
    Seq->Seqs = NULL;
    Seq->SeqCount = 0;
    cp2ucsSeq * cSeq;
    for( j = 0; j < 256; j++ ){
      if( !isLeadByte(Seq->pcp->info, j) ){
        Seq->Seqs = (cp2ucsSeq *) realloc(Seq->Seqs, sizeof(cp2ucsSeq) * (Seq->SeqCount + 1));
        cSeq = Seq->Seqs + Seq->SeqCount;
        memset(cSeq->c, 0, sizeof(cSeq->c));
        cSeq->c[0] = (char) j;
        r = MultiByteToWideChar(Seq->pcp->cp, 0, (char *) cSeq->c, 1, (wchar_t *) &cSeq->ucs, 1);
        if( r == 1 )
          Seq->SeqCount++;
      }
    }
    for( j = 0; j < 256; j++ ){
      if( isLeadByte(Seq->pcp->info, j) ){
        long long b = 1ll << ((Seq->pcp->info.MaxCharSize - 1) * 8);
        for( long long a = 0; a < b; a++ ){
          Seq->Seqs = (cp2ucsSeq *) realloc(Seq->Seqs, sizeof(cp2ucsSeq) * (Seq->SeqCount + 1));
          cSeq = Seq->Seqs + Seq->SeqCount;
          memset(cSeq->c, 0, sizeof(cSeq->c));
          cSeq->c[0] = (char) j;
          *(uint32_t *) (cSeq->c + 1) = (uint32_t) a;
          r = MultiByteToWideChar(Seq->pcp->cp, 0, (char *) cSeq->c, -1, (wchar_t *) &cSeq->ucs, 1);
          if( r == 1 )
            Seq->SeqCount++;
        }
      }
    }
    cp2ucsSeqCount++;
  }
  for( i = cp2ucsSeqCount - 1; i >= 0; i-- ){
    sprintf(bfile,DST_DIR "\\cp%dutf8.bin", cp2ucsSeqs[i].pcp->cp);
    file = fopen(bfile, "wb");
    assert( file != NULL );
    Sequence *  Seqs;
    intptr_t SeqCount, mSeqCount, sSeqCount;
    findCPSequences(Seqs, SeqCount, mSeqCount, cp2ucsSeqs + i);
    fwrite(&cp2ucsSeqs[i].pcp->cp, sizeof(uint32_t), 1, file);
    fwrite(&mSeqCount, sizeof(uint8_t), 1, file);
    for( j = 0; j < SeqCount; j++ ){
      if( Seqs[j].Len < 2 ) continue;
      const uint8_t & c = cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c[0];
      fwrite(&c, sizeof(uint8_t), 1, file);
      k = c + Seqs[j].Len - 1;
      fwrite(&k, sizeof(uint8_t), 1, file);
      fwrite(&cp2ucsSeqs[i].Seqs[Seqs[j].Pos].ucs, sizeof(uint16_t), 1, file);
    }
    sSeqCount = SeqCount - mSeqCount;
    fwrite(&sSeqCount, sizeof(uint8_t), 1, file);
    for( j = 0; j < SeqCount; j++ ){
      if( Seqs[j].Len > 1 ) continue;
      const uint8_t & c = cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c[0];
      fwrite(&c, sizeof(uint8_t), 1, file);
      fwrite(&cp2ucsSeqs[i].Seqs[Seqs[j].Pos].ucs, sizeof(uint16_t), 1, file);
    }
    free(Seqs);
    // Seqs with lead bytes
    findLBCPSequences(Seqs, SeqCount, mSeqCount, cp2ucsSeqs + i);
    if( SeqCount > 0 ){
      intptr_t lb  = 0;
      j = 0;
      while( j < SeqCount ){
        if( lb != cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c[0] ){
          lb = cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c[0];
          k = 0;
          for( g = j; g < SeqCount && cp2ucsSeqs[i].Seqs[Seqs[g].Pos].c[0] == lb; g++ ){
            if( Seqs[g].Len > 1 )
              k++;
          }
          fwrite(&lb, sizeof(uint8_t), 1, file);
          fwrite(&k, sizeof(uint8_t), 1, file);
          for( g = j; g < SeqCount && cp2ucsSeqs[i].Seqs[Seqs[g].Pos].c[0] == lb; g++ ){
            if( Seqs[g].Len > 1 ){
              uint32_t &  a = *(uint32_t *) (cp2ucsSeqs[i].Seqs[Seqs[g].Pos].c + 1);
              uint32_t    b = uint32_t(a + Seqs[g].Len - 1);
              fwrite(&a, cp2ucsSeqs[i].pcp->info.MaxCharSize - 1, 1, file);
              fwrite(&b, cp2ucsSeqs[i].pcp->info.MaxCharSize - 1, 1, file);
              fwrite(&cp2ucsSeqs[i].Seqs[Seqs[g].Pos].ucs, sizeof(uint16_t), 1, file);
            }
          }
          k = 0;
          for( g = j; g < SeqCount && cp2ucsSeqs[i].Seqs[Seqs[g].Pos].c[0] == lb; g++ ){
            if( Seqs[g].Len < 2 )
              k++;
          }
          fwrite(&k, sizeof(uint8_t), 1, file);
          for( g = j; g < SeqCount && cp2ucsSeqs[i].Seqs[Seqs[g].Pos].c[0] == lb; g++ ){
            if( Seqs[g].Len < 2 ){
              uint32_t &  a = *(uint32_t *) (cp2ucsSeqs[i].Seqs[Seqs[g].Pos].c + 1);
              fwrite(&a, cp2ucsSeqs[i].pcp->info.MaxCharSize - 1, 1, file);
              fwrite(&cp2ucsSeqs[i].Seqs[Seqs[g].Pos].ucs, sizeof(uint16_t), 1, file);
            }
          }
        }
        j++;
      }
    }
    free(Seqs);
    k = 0;
    fwrite(&k, sizeof(uint8_t), 1, file);
    fclose(file);
    free(cp2ucsSeqs[i].Seqs);
  }
  free(cp2ucsSeqs);

  cp2ucsSeqs = NULL;
  cp2ucsSeqCount = 0;
  for( i = 0; i < CPEnumCount; i++ ){
    cp2ucsSeqs = (cp2ucs *) realloc(cp2ucsSeqs, sizeof(cp2ucs) * (cp2ucsSeqCount + 1));
    cp2ucs *  Seq = cp2ucsSeqs + cp2ucsSeqCount;
    Seq->pcp = CPEnums + i;
    Seq->Seqs = NULL;
    Seq->SeqCount = 0;
    cp2ucsSeq * cSeq;
    for( j = 0; j < 65536; j++ ){
      Seq->Seqs = (cp2ucsSeq *) realloc(Seq->Seqs, sizeof(cp2ucsSeq) * (Seq->SeqCount + 1));
      cSeq = Seq->Seqs + Seq->SeqCount;
      memset(cSeq->c, 0, sizeof(cSeq->c));
      cSeq->ucs = (uint16_t) j;
      r = WideCharToMultiByte(Seq->pcp->cp, 0/*WC_COMPOSITECHECK | WC_DISCARDNS*/, (wchar_t *) &cSeq->ucs, 1, (char *) cSeq->c, 5, NULL, NULL);
      for( k = MAX_DEFAULTCHAR - 1; k >= 0; k-- )
        if( Seq->pcp->info.DefaultChar[k] != 0 && cSeq->ucs != Seq->pcp->info.DefaultChar[k] )
          if( strchr(cSeq->c, Seq->pcp->info.DefaultChar[k]) != NULL )
            break;
      if( r > 0 && k < 0 )
        Seq->SeqCount++;
    }
    cp2ucsSeqCount++;
  }
  for( i = cp2ucsSeqCount - 1; i >= 0; i-- ){
    sprintf(bfile,DST_DIR "\\utf8cp%d.bin", cp2ucsSeqs[i].pcp->cp);
    file = fopen(bfile, "wb");
    assert( file != NULL );
    Sequence *  Seqs;
    intptr_t SeqCount, mSeqCount, sSeqCount;
    findCPSequencesR(Seqs, SeqCount, cp2ucsSeqs + i);
    //    fwrite(&cp2ucsSeqs[i].pcp->cp,sizeof(unsigned long),1,file);
    fwrite(&cp2ucsSeqs[i].pcp->info.MaxCharSize, sizeof(uint8_t), 1, file);
    fwrite(&cp2ucsSeqs[i].pcp->info.DefaultChar, sizeof(uint8_t), 1, file);
    for( k = 1; k <= (intptr_t) cp2ucsSeqs[i].pcp->info.MaxCharSize; k++ ){
      mSeqCount = 0;
      for( j = 0; j < SeqCount; j++ ){
        if( Seqs[j].Len < 2 )
          continue;
        g = strlen(cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c);
        if( g == k || g == 0 )
          mSeqCount++;
      }
      fwrite(&mSeqCount, sizeof(uint8_t), 1, file);
      fwrite(&k, sizeof(uint8_t), 1, file);
      for( j = 0; j < SeqCount; j++ ){
        if( Seqs[j].Len < 2 ) continue;
        g = strlen(cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c);
        if( g != k && g != 0 ) continue;
        const uint16_t &  c = cp2ucsSeqs[i].Seqs[Seqs[j].Pos].ucs;
        fwrite(&c, sizeof(uint16_t), 1, file);
        r = c + Seqs[j].Len - 1;
        fwrite(&r, sizeof(uint16_t), 1, file);
        fwrite(&cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c, k, 1, file);
      }
    }
    k = -1;
    fwrite(&k, sizeof(uint8_t), 1, file);
    fwrite(&k, sizeof(uint8_t), 1, file);
    for( k = 1; k <= (intptr_t) cp2ucsSeqs[i].pcp->info.MaxCharSize; k++ ){
      sSeqCount = 0;
      for( j = 0; j < SeqCount; j++ ){
        if( Seqs[j].Len > 1 ) continue;
        g = strlen(cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c);
        if( g == k || g == 0 )
          sSeqCount++;
      }
      fwrite(&sSeqCount, sizeof(uint16_t), 1, file);
      fwrite(&k, sizeof(uint8_t), 1, file);
      for( j = 0; j < SeqCount; j++ ){
        if( Seqs[j].Len > 1 ) continue;
        g = strlen(cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c);
        if( g != k && g != 0 ) continue;
        const uint16_t &  c = cp2ucsSeqs[i].Seqs[Seqs[j].Pos].ucs;
        fwrite(&c, sizeof(uint16_t), 1, file);
        fwrite(cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c, k, 1, file);
      }
    }
    k = -1;
    fwrite(&k, sizeof(uint16_t), 1, file);
    fwrite(&k, sizeof(uint8_t), 1, file);
    free(Seqs);
    fclose(file);
    free(cp2ucsSeqs[i].Seqs);
  }
  free(cp2ucsSeqs);
  //
  GenEmbed();
  // make one indexed big monster file
  FILE *  monster = fopen(DST_DIR "\\utf8mod.bin", "wb");
  assert( monster != NULL );
  // write index
  fwrite(&CPEnumCount, sizeof(uint32_t), 1, monster);
  uint32_t  offset        = sizeof(uint32_t);
  uint32_t  SizeOfC1Table = sizeof(C1Table);
  fwrite(&SizeOfC1Table, sizeof(uint32_t), 1, monster);
  offset += sizeof(uint32_t);
  uint32_t  SizeOfC2Table = sizeof(C2Table);
  fwrite(&SizeOfC2Table, sizeof(uint32_t), 1, monster);
  offset += sizeof(uint32_t);
  uint32_t  SizeOfC3Table = sizeof(C3Table);
  fwrite(&SizeOfC3Table, sizeof(uint32_t), 1, monster);
  offset += sizeof(uint32_t);

  offset += SizeOfC1Table;
  offset += SizeOfC2Table;
  offset += SizeOfC3Table;

  for( i = 0; i < CPEnumCount; i++ ){
    fwrite(&CPEnums[i].cp, sizeof(uint32_t), 1, monster);
    struct stat st;
    sprintf(bfile, "cp%dutf8.bin", CPEnums[i].cp);
    stat(bfile, &st);
    fwrite(&offset, sizeof(uint32_t), 1, monster);
    offset += st.st_size;
    sprintf(bfile, "utf8cp%d.bin", CPEnums[i].cp);
    stat(bfile, &st);
    fwrite(&offset, sizeof(uint32_t), 1, monster);
    offset += st.st_size;
  }
  fwrite(&C1Table, SizeOfC1Table, 1, monster);
  fwrite(&C2Table, SizeOfC2Table, 1, monster);
  fwrite(&C3Table, SizeOfC3Table, 1, monster);
  for( i = 0; i < CPEnumCount; i++ ){
    void *      b;
    struct stat st;
    sprintf(bfile,DST_DIR "\\cp%dutf8.bin", CPEnums[i].cp);
    stat(bfile, &st);
    file = fopen(bfile, "rb");
    assert( file != NULL );
    b = malloc(st.st_size);
    fread(b, st.st_size, 1, file);
    fclose(file);
    remove(bfile);
    fwrite(b, st.st_size, 1, monster);
    free(b);
    offset += st.st_size;
    sprintf(bfile,DST_DIR "\\utf8cp%d.bin", CPEnums[i].cp);
    stat(bfile, &st);
    file = fopen(bfile, "rb");
    assert( file != NULL );
    b = malloc(st.st_size);
    fread(b, st.st_size, 1, file);
    fclose(file);
    remove(bfile);
    fwrite(b, st.st_size, 1, monster);
    free(b);
  }
  fclose(monster);
  //remove(DST_DIR "\\utf8mod.bin");
}

static BOOL CALLBACK EnumCodePagesProc(LPTSTR lpCodePageString)
{
  CPEnums = (CPEnum *) realloc(CPEnums, sizeof(CPEnum) * (CPEnumCount + 1));
  CPEnums[CPEnumCount].cp = _ttol(lpCodePageString);
  SetLastError(0);
  GetCPInfoEx(CPEnums[CPEnumCount].cp,0,&CPEnums[CPEnumCount].info);
  DWORD err = GetLastError();
  if( err == 0 && CPEnums[CPEnumCount].cp != CP_UTF7 && CPEnums[CPEnumCount].cp != CP_UTF8 &&
      CPEnums[CPEnumCount].cp != CP_SYMBOL )
    CPEnumCount++;
  return TRUE;
} 

static int sortCodePages(const void * a, const void * b)
{
  return ((const CPEnum *) a)->cp > ((const CPEnum *) b)->cp ? 1 : ((const CPEnum *) a)->cp < ((const CPEnum *) b)->cp ? -1 : 0;
}

int main(int /*argc*/, char ** /*argv*/)
{
//  TCHAR cd[1024];
//  GetCurrentDirectory(sizeof(cd) / sizeof(cd[0]),cd);

  uintptr_t i;
  for( i = 0; i < 65536; i++ ){
    uint16_t  a1, a2, a3;
    DWORD     le1, le2, le3;
    SetLastError(0);
    GetStringTypeW(CT_CTYPE1, (wchar_t *) &i, 1, &a1);
    le1 = GetLastError();
    SetLastError(0);
    GetStringTypeW(CT_CTYPE2, (wchar_t *) &i, 1, &a2);
    le2 = GetLastError();
    SetLastError(0);
    GetStringTypeW(CT_CTYPE3, (wchar_t *) &i, 1, &a3);
    le3 = GetLastError();
    if( le1 == 0 ){
      *(uint32_t *) (C1Table + i * 9 / 8) |= (a1 & ((1u << 9) - 1)) << i * 9 % 8;
      if( (a1 & C1_UPPER) != 0 ){
        //        printf("C1_UPPER "); // 0x0001 Uppercase
      }
      if( (a1 & C1_LOWER) != 0 ){
        //        printf("C1_LOWER "); // 0x0002 Lowercase
      }
      if( (a1 & C1_DIGIT) != 0 ){
        //        printf("C1_DIGIT "); // 0x0004 Decimal digits
      }
      if( (a1 & C1_SPACE) != 0 ){
        //        printf("C1_SPACE "); // 0x0008 Space characters
      }
      if( (a1 & C1_PUNCT) != 0 ){
        //        printf("C1_PUNCT "); // 0x0010 Punctuation
      }
      if( (a1 & C1_CNTRL) != 0 ){
        //        printf("C1_CNTRL "); // 0x0020 Control characters
      }
      if( (a1 & C1_BLANK) != 0 ){
        //        printf("C1_BLANK "); // 0x0040 Blank characters
      }
      if( (a1 & C1_XDIGIT) != 0 ){
        //        printf("C1_XDIGIT "); // 0x0080 Hexadecimal digits
      }
      if( (a1 & C1_ALPHA) != 0 ){
        //        printf("C1_ALPHA "); // 0x0100 Any linguistic character: alphabetic, syllabary, or ideographic
      }
    }
    if( le2 == 0 ){
      *(uint32_t *) (C2Table + i * 4 / 8) |= (a2 & ((1u << 4) - 1)) << i * 4 % 8;
      // Strong:
      //      if( a2 == C2_LEFTTORIGHT ) printf("C2_LEFTTORIGHT "); // 0x1 Left to right
      //      if( a2 == C2_RIGHTTOLEFT ) printf("C2_RIGHTTOLEFT "); // 0x2 Right to left
      // Weak:
      //      if( a2 == C2_EUROPENUMBER ) printf("C2_EUROPENUMBER "); // 0x3 European number, European digit
      //      if( a2 == C2_EUROPESEPARATOR ) printf("C2_EUROPESEPARATOR "); // 0x4 European numeric separator
      //      if( a2 == C2_EUROPETERMINATOR ) printf("C2_EUROPETERMINATOR "); // 0x5 European numeric terminator
      //      if( a2 == C2_ARABICNUMBER ) printf("C2_ARABICNUMBER "); // 0x6 Arabic number
      //      if( a2 == C2_COMMONSEPARATOR ) printf("C2_COMMONSEPARATOR "); // 0x7 Common numeric separator
      // Neutral:
      //      if( a2 == C2_BLOCKSEPARATOR ) printf("C2_BLOCKSEPARATOR "); // 0x8 Block separator
      //      if( a2 == C2_SEGMENTSEPARATOR ) printf("C2_SEGMENTSEPARATOR "); // 0x9 Segment separator
      //      if( a2 == C2_WHITESPACE ) printf("C2_WHITESPACE "); // 0xA White space
      //      if( a2 == C2_OTHERNEUTRAL ) printf("C2_OTHERNEUTRAL "); // 0xB Other neutrals
      // Not applicable:
      // C2_NOTAPPLICABLE 0x0 No implicit directionality (for example, control codes)
    }
    if( le3 == 0 ){
      *(uint32_t *) (C3Table + i * 11 / 8) |= (a3 & ((1u << 11) - 1)) << i * 11 % 8;
      //      if( (a3 & C3_NONSPACING) != 0 ) printf("C3_NONSPACING "); // 0x1 Nonspacing mark
      //      if( (a3 & C3_DIACRITIC) != 0 ) printf("C3_DIACRITIC "); // 0x2 Diacritic nonspacing mark
      //      if( (a3 & C3_VOWELMARK) != 0 ) printf("C3_VOWELMARK "); // 0x4 Vowel nonspacing mark
      //      if( (a3 & C3_SYMBOL) != 0 ) printf("C3_SYMBOL "); // 0x8 Symbol
      //      if( (a3 & C3_KATAKANA) != 0 ) printf("C3_KATAKANA "); // 0x10 Katakana character
      //      if( (a3 & C3_HIRAGANA) != 0 ) printf("C3_HIRAGANA "); // 0x20 Hiragana character
      //      if( (a3 & C3_HALFWIDTH) != 0 ) printf("C3_HALFWIDTH "); // 0x40 Half-width character
      //      if( (a3 & C3_FULLWIDTH) != 0 ) printf("C3_FULLWIDTH "); // 0x80 Full-width character
      //      if( (a3 & C3_IDEOGRAPH) != 0 ) printf("C3_IDEOGRAPH "); // 0x100 Ideographic character
      //      if( (a3 & C3_KASHIDA) != 0 ) printf("C3_KASHIDA "); // 0x200 Arabic Kashida character
      //      if( (a3 & C3_ALPHA) != 0 ) printf("C3_ALPHA "); // 0x8000 All linguistic characters (alphabetic, syllabary, and ideographic)
      // Not applicable:
      // C3_NOTAPPLICABLE 0x0 Not applicable
    }
    UpperTable[UpperCount * 2 + 1] = (unsigned short) CharUpperW((LPWSTR) i);
    if( (unsigned) UpperTable[UpperCount * 2 + 1] != i ){
      UpperTable[UpperCount * 2] = (unsigned short) i;
      UpperCount++;
    }
    LowerTable[LowerCount * 2 + 1] = (unsigned short) CharLowerW((LPWSTR) i);
    if( (unsigned) LowerTable[LowerCount * 2 + 1] != i ){
      LowerTable[LowerCount * 2] = (unsigned short) i;
      LowerCount++;
    }
  }

  EnumSystemCodePages((CODEPAGE_ENUMPROC) EnumCodePagesProc, CP_SUPPORTED);
  qsort(CPEnums, CPEnumCount, sizeof(CPEnums[0]), sortCodePages);

  dumpCP2UCS();

  free(CPEnums);
  return 0;
}

