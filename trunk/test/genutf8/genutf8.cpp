#include <windows.h>
#include <sys\stat.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef __BCPLUSPLUS__
using namespace std;
#endif

#define EVENT_SIZE_BIT(a,b) ((a) * (b) / 8 + ((a) * (b) % 8 > 0))
#define EVENT_SIZE(a,b) EVENT_SIZE_BIT(a,b) + 4 - (EVENT_SIZE_BIT(a,b) & (sizeof(unsigned long) - 1))

unsigned char C1Table[EVENT_SIZE(65536u,9u)];
unsigned char C2Table[65536 / 2];
unsigned char C3Table[EVENT_SIZE(65536u,11u)];

struct UpperLower {
  unsigned short bl, br, c;
};

unsigned short UpperTable[65536 * 2];
int UpperCount;
unsigned short LowerTable[65536 * 2];
int LowerCount;

struct CPEnum {
  unsigned int cp;
  CPINFO info;
};

CPEnum * CPEnums;
int CPEnumCount;

void GenEmbed()
{
  struct stat st;
  char bfile[64];
  int i, j, k, g;
  FILE * file, * f;
  void * b;

  file = fopen("..\\..\\src\\libutf8\\utf8uct.cpp","wb");
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "#include <adicpp/utf8embd.h>\n"
    "//---------------------------------------------------------------------------\n"
    "namespace utf8 {\n"
    "//---------------------------------------------------------------------------\n"
  );
  k = UpperCount * 2;
  fprintf(file,
    "extern const unsigned short upperTable[%d] = {\n",k
  );
  for( i = 0; i < k; ){
    fprintf(file,"  ");
    for( j = 0; j < 8 && i < k; j++, i++ ){
      if( j > 0 ) fprintf(file," ");
      fprintf(file,"0x%04X",UpperTable[i]);
      if( i < k - 1 ) fprintf(file,",");
    }
    if( i < k ) fprintf(file,"\n");
  }
  fprintf(file,
    "\n};\n"
  );
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "} // namespace utf8\n"
    "//---------------------------------------------------------------------------\n"
  );
  fclose(file);
  file = fopen("..\\..\\src\\libutf8\\utf8lct.cpp","wb");
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "#include <adicpp/utf8embd.h>\n"
    "//---------------------------------------------------------------------------\n"
    "namespace utf8 {\n"
    "//---------------------------------------------------------------------------\n"
  );
  k = LowerCount * 2;
  fprintf(file,
    "extern const unsigned short lowerTable[%d] = {\n",k
  );
  for( i = 0; i < k; ){
    fprintf(file,"  ");
    for( j = 0; j < 8 && i < k; j++, i++ ){
      if( j > 0 ) fprintf(file," ");
      fprintf(file,"0x%04X",LowerTable[i]);
      if( i < k - 1 ) fprintf(file,",");
    }
    if( i < k ) fprintf(file,"\n");
  }
  fprintf(file,
    "\n};\n"
  );
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "} // namespace utf8\n"
    "//---------------------------------------------------------------------------\n"
  );
  fclose(file);
  file = fopen("..\\..\\src\\libutf8\\utf8c1.cpp","wb");
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "#include <adicpp/utf8embd.h>\n"
    "//---------------------------------------------------------------------------\n"
    "namespace utf8 {\n"
    "//---------------------------------------------------------------------------\n"
  );
  k = sizeof(C1Table) / sizeof(unsigned long);
  fprintf(file,
    "extern const unsigned int C1Table[%d] = {\n",k
  );
  for( i = 0; i < k; ){
    fprintf(file,"  ");
    for( j = 0; j < 4 && i < k; j++, i++ ){
      if( j > 0 ) fprintf(file," ");
      fprintf(file,"0x%08X",*((unsigned int *) C1Table + i));
      if( i < k - 1 ) fprintf(file,",");
    }
    if( i < k ) fprintf(file,"\n");
  }
  fprintf(file,
    "\n};\n"
  );
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "} // namespace utf8\n"
    "//---------------------------------------------------------------------------\n"
  );
  fclose(file);
  file = fopen("..\\..\\src\\libutf8\\utf8c2.cpp","wb");
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "#include <adicpp/utf8embd.h>\n"
    "//---------------------------------------------------------------------------\n"
    "namespace utf8 {\n"
    "//---------------------------------------------------------------------------\n"
  );
  k = sizeof(C2Table) / sizeof(unsigned long);
  fprintf(file,
    "extern const unsigned int C2Table[%d] = {\n",k
  );
  for( i = 0; i < k; ){
    fprintf(file,"  ");
    for( j = 0; j < 4 && i < k; j++, i++ ){
      if( j > 0 ) fprintf(file," ");
      fprintf(file,"0x%08X",*((unsigned int *) C2Table + i));
      if( i < k - 1 ) fprintf(file,",");
    }
    if( i < k ) fprintf(file,"\n");
  }
  fprintf(file,
    "\n};\n"
  );
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "} // namespace utf8\n"
    "//---------------------------------------------------------------------------\n"
  );
  fclose(file);
  file = fopen("..\\..\\src\\libutf8\\utf8c3.cpp","wb");
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "#include <adicpp/utf8embd.h>\n"
    "//---------------------------------------------------------------------------\n"
    "namespace utf8 {\n"
    "//---------------------------------------------------------------------------\n"
  );
  k = sizeof(C3Table) / sizeof(unsigned long);
  fprintf(file,
    "extern const unsigned int C3Table[%d] = {\n",k
  );
  for( i = 0; i < k; ){
    fprintf(file,"  ");
    for( j = 0; j < 4 && i < k; j++, i++ ){
      if( j > 0 ) fprintf(file," ");
      fprintf(file,"0x%08X",*((unsigned int *) C3Table + i));
      if( i < k - 1 ) fprintf(file,",");
    }
    if( i < k ) fprintf(file,"\n");
  }
  fprintf(file,
    "\n};\n"
  );
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "} // namespace utf8\n"
    "//---------------------------------------------------------------------------\n"
  );
  fclose(file);
  for( i = 0; i < CPEnumCount; i++ ){
    sprintf(bfile,"..\\..\\src\\libutf8\\cp%d.cpp",CPEnums[i].cp);
    file = fopen(bfile,"wb");
    fprintf(file,
      "//---------------------------------------------------------------------------\n"
      "#include <adicpp/utf8embd.h>\n"
      "//---------------------------------------------------------------------------\n"
      "namespace utf8 {\n"
      "//---------------------------------------------------------------------------\n"
    );
    sprintf(bfile,"..\\..\\src\\libutf8\\cp%dutf8.bin",CPEnums[i].cp);
    stat(bfile,&st);
    f = fopen(bfile,"rb");
    b = malloc(st.st_size);
    fread(b,st.st_size,1,f);
    fclose(f);
    k = st.st_size;
    fprintf(file,
      "#if defined(EMBED_CP%d_SUPPORT) || defined(EMBED_ALL_CP)\n"
      "extern const unsigned char cp%ds2utf8s[%d] = {\n",
      CPEnums[i].cp,CPEnums[i].cp,k
    );
    for( g = 0; g < k; ){
      fprintf(file,"  ");
      for( j = 0; j < 8 && g < k; j++, g++ ){
        if( j > 0 ) fprintf(file," ");
        fprintf(file,"0x%02X",*((unsigned char *) b + g));
        if( g < k - 1 ) fprintf(file,",");
      }
      if( g < k ) fprintf(file,"\n");
    }
    free(b);
    fprintf(file,
      "\n};\n"
      "//---------------------------------------------------------------------------\n"
    );
    sprintf(bfile,"..\\..\\src\\libutf8\\utf8cp%d.bin",CPEnums[i].cp);
    stat(bfile,&st);
    f = fopen(bfile,"rb");
    b = malloc(st.st_size);
    fread(b,st.st_size,1,f);
    fclose(f);
    k = st.st_size;
    fprintf(file,
      "extern const unsigned char utf8s2cp%ds[%d] = {\n",CPEnums[i].cp,k
    );
    for( g = 0; g < k; ){
      fprintf(file,"  ");
      for( j = 0; j < 8 && g < k; j++, g++ ){
        if( j > 0 ) fprintf(file," ");
        fprintf(file,"0x%02X",*((unsigned char *) b + g));
        if( g < k - 1 ) fprintf(file,",");
      }
      if( g < k ) fprintf(file,"\n");
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
  file = fopen("..\\..\\src\\libutf8\\utf8cp.cpp","wb");
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "#include <adicpp/utf8embd.h>\n"
    "//---------------------------------------------------------------------------\n"
    "namespace utf8 {\n"
    "//---------------------------------------------------------------------------\n"
  );
  for( i = 0; i < CPEnumCount; i++ ){
    fprintf(file,
      "extern const unsigned char utf8s2cp%ds[];\n"
      "extern const unsigned char cp%ds2utf8s[];\n",
      CPEnums[i].cp,CPEnums[i].cp
    );
  }
  fprintf(file,"extern const utf8cp utf8cps[] = {\n");
  for( i = 0; i < CPEnumCount; i++ ){
    if( i > 0 ) fprintf(file,",\n#endif\n");
    fprintf(file,
      "#if defined(EMBED_CP%d_SUPPORT) || defined(EMBED_ALL_CP)\n"
      "  { cp%ds2utf8s, utf8s2cp%ds }",
      CPEnums[i].cp,CPEnums[i].cp,CPEnums[i].cp
    );
  }
  fprintf(file,
    "\n"
    "#endif\n"
    "};\n"
  );
  fprintf(file,
    "//---------------------------------------------------------------------------\n"
    "extern const int utf8cpsCount = sizeof(utf8cps) / sizeof(utf8cps[0]);\n"
    "//---------------------------------------------------------------------------\n"
    "} // namespace utf8\n"
    "//---------------------------------------------------------------------------\n"
  );
  fclose(file);
}

struct Sequence {
  int Pos;
  int Len;
};

int sortCPSequences(const void * a,const void * b)
{
  return -(((Sequence *) a)->Len - ((Sequence *) b)->Len);
} 

int sortCPSequences2(const void * a,const void * b)
{
  return ((Sequence *) a)->Pos - ((Sequence *) b)->Pos;
} 

struct cp2ucsSeq {
  unsigned char c[6];
  unsigned short ucs;
};

struct cp2ucs {
  CPEnum * pcp;
  cp2ucsSeq * Seqs;
  int SeqCount;
};

#ifndef __BCPLUSPLUS__
inline
#endif
bool isLeadByte(const CPINFO & info,int byte)
{
  for( int k = 0; k < MAX_LEADBYTES; k += 2 ){
    if( info.LeadByte[k] == 0 && info.LeadByte[k + 1] == 0 ) break;
    if( byte >= (int) info.LeadByte[k] && byte <= (int) info.LeadByte[k + 1] )
      return true;
  }
  return false;
}

void findCPSequences(Sequence * & Seqs,int & SeqCount,int & mSeqCount,const cp2ucs * cp)
{
  Seqs = NULL;
  SeqCount = 0;
  mSeqCount = 0;
  int Pos = 0;
  while( Pos < cp->SeqCount ){
    if( !isLeadByte(cp->pcp->info,cp->Seqs[Pos].c[0]) ){
      Seqs = (Sequence *) realloc(Seqs,sizeof(Seqs[0]) * (SeqCount + 1));
      Seqs[SeqCount].Pos = Pos;
      Seqs[SeqCount].Len = 1;
      while( Pos + 1 < cp->SeqCount && !isLeadByte(cp->pcp->info,cp->Seqs[Pos + 1].c[0]) &&
        cp->Seqs[Pos + 1].c[0] - cp->Seqs[Pos].c[0] == 1 &&
        cp->Seqs[Pos + 1].ucs - cp->Seqs[Pos].ucs == 1
      ){
        Seqs[SeqCount].Len++;
        Pos++;
      }
      if( Seqs[SeqCount].Len > 1 ) mSeqCount++;
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
int memncmp(const void * m1,const void * m2,unsigned n)
{
  int c = 0;
  while( n-- > 0 ){
    c = *(const unsigned char *) m1 - *(const unsigned char *) m2;
    if( c != 0 ) break;
    *(const unsigned char **) m1 += 1;
    *(const unsigned char **) m2 += 1;
  }
  return c;
}

static
#ifndef __BCPLUSPLUS__
inline
#endif
int strlen(const unsigned char * s1)
{
  const unsigned char * s = s1;
  while( *s != '\0' ) s++;
  return s - s1;
}

static
#ifndef __BCPLUSPLUS__
inline
#endif
unsigned char * strchr(const unsigned char * s,unsigned c)
{
  while( *s != '\0' )
    if( (unsigned) *s == c ) return (unsigned char *) s; else s++;
  return NULL;
}

void findLBCPSequences(Sequence * & Seqs,int & SeqCount,int & mSeqCount,const cp2ucs * cp)
{
  Seqs = NULL;
  SeqCount = 0;
  mSeqCount = 0;
  int Pos = 0;
  while( Pos < cp->SeqCount ){
    if( isLeadByte(cp->pcp->info,cp->Seqs[Pos].c[0]) ){
      Seqs = (Sequence *) realloc(Seqs,sizeof(Seqs[0]) * (SeqCount + 1));
      Seqs[SeqCount].Pos = Pos;
      Seqs[SeqCount].Len = 1;
      while( Pos + 1 < cp->SeqCount && isLeadByte(cp->pcp->info,cp->Seqs[Pos + 1].c[0]) &&
        memncmp(cp->Seqs[Pos + 1].c,cp->Seqs[Pos].c,6) == 1 &&
        cp->Seqs[Pos + 1].ucs - cp->Seqs[Pos].ucs == 1
      ){
        Seqs[SeqCount].Len++;
        Pos++;
      }
      if( Seqs[SeqCount].Len > 1 ) mSeqCount++;
      SeqCount++;
    }
    Pos++;
  }
}

void findCPSequencesR(Sequence * & Seqs,int & SeqCount,const cp2ucs * cp)
{
  Seqs = NULL;
  SeqCount = 0;
  int Pos = 0;
  while( Pos < cp->SeqCount ){
    Seqs = (Sequence *) realloc(Seqs,sizeof(Seqs[0]) * (SeqCount + 1));
    Seqs[SeqCount].Pos = Pos;
    Seqs[SeqCount].Len = 1;
    while( Pos + 1 < cp->SeqCount &&
      cp->Seqs[Pos + 1].ucs - cp->Seqs[Pos].ucs == 1 &&
      memncmp(cp->Seqs[Pos + 1].c,cp->Seqs[Pos].c,6) == 1
    ){
      Seqs[SeqCount].Len++;
      Pos++;
    }
    SeqCount++;
    Pos++;
  }
}

void dumpCP2UCS()
{
  char bfile[64];
  FILE * file;
  int i, j, k, g, r;
  cp2ucs * cp2ucsSeqs = NULL;
  int cp2ucsSeqCount = 0;
  for( i = 0; i < CPEnumCount; i++ ){
    cp2ucsSeqs = (cp2ucs *) realloc(cp2ucsSeqs,sizeof(cp2ucs) * (cp2ucsSeqCount + 1));
    cp2ucs * Seq = cp2ucsSeqs + cp2ucsSeqCount;
    Seq->pcp = CPEnums + i;
    Seq->Seqs = NULL;
    Seq->SeqCount = 0;
    cp2ucsSeq * cSeq;
    for( j = 0; j < 256; j++ ){
      if( !isLeadByte(Seq->pcp->info,j) ){
        Seq->Seqs = (cp2ucsSeq *) realloc(Seq->Seqs,sizeof(cp2ucsSeq) * (Seq->SeqCount + 1));
        cSeq = Seq->Seqs + Seq->SeqCount;
        memset(cSeq->c,0,sizeof(cSeq->c));
        cSeq->c[0] = (char) j;
        r = MultiByteToWideChar(Seq->pcp->cp,0,(char *) cSeq->c,1,(wchar_t *)&cSeq->ucs,1);
        if( r == 1 ) Seq->SeqCount++;
      }
    }
    for( j = 0; j < 256; j++ ){
      if( isLeadByte(Seq->pcp->info,j) ){
        long long b = 1ll << ((Seq->pcp->info.MaxCharSize - 1) * 8);
        for( long long a = 0; a < b; a++ ){
          Seq->Seqs = (cp2ucsSeq *) realloc(Seq->Seqs,sizeof(cp2ucsSeq) * (Seq->SeqCount + 1));
          cSeq = Seq->Seqs + Seq->SeqCount;
          memset(cSeq->c,0,sizeof(cSeq->c));
          cSeq->c[0] = (char) j;
          *(unsigned int *) (cSeq->c + 1) = (unsigned int) a;
          r = MultiByteToWideChar(Seq->pcp->cp,0,(char *) cSeq->c,-1,(wchar_t *)&cSeq->ucs,1);
          if( r == 1 ) Seq->SeqCount++;
        }
      }
    }
    cp2ucsSeqCount++;
  }
  for( i = cp2ucsSeqCount - 1; i >= 0; i-- ){
    sprintf(bfile,"..\\..\\src\\libutf8\\cp%dutf8.bin",cp2ucsSeqs[i].pcp->cp);
    file = fopen(bfile,"wb");
    Sequence * Seqs;
    int SeqCount, mSeqCount, sSeqCount;
    findCPSequences(Seqs,SeqCount,mSeqCount,cp2ucsSeqs + i);
    fwrite(&cp2ucsSeqs[i].pcp->cp,sizeof(unsigned long),1,file);
    fwrite(&mSeqCount,sizeof(unsigned char),1,file);
    for( j = 0; j < SeqCount; j++ ){
      if( Seqs[j].Len < 2 ) continue;
      const unsigned char & c = cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c[0];
      fwrite(&c,sizeof(unsigned char),1,file);
      k = c + Seqs[j].Len - 1;
      fwrite(&k,sizeof(unsigned char),1,file);
      fwrite(&cp2ucsSeqs[i].Seqs[Seqs[j].Pos].ucs,sizeof(unsigned short),1,file);
    }
    sSeqCount = SeqCount - mSeqCount;
    fwrite(&sSeqCount,sizeof(unsigned char),1,file);
    for( j = 0; j < SeqCount; j++ ){
      if( Seqs[j].Len > 1 ) continue;
      const unsigned char & c = cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c[0];
      fwrite(&c,sizeof(unsigned char),1,file);
      fwrite(&cp2ucsSeqs[i].Seqs[Seqs[j].Pos].ucs,sizeof(unsigned short),1,file);
    }
    free(Seqs);
// Seqs with lead bytes
    findLBCPSequences(Seqs,SeqCount,mSeqCount,cp2ucsSeqs + i);
    if( SeqCount > 0 ){
      int lb = 0;
      j = 0;
      while( j < SeqCount ){
        if( lb != cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c[0] ){
          lb = cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c[0];
          k = 0;
          for( g = j; g < SeqCount && cp2ucsSeqs[i].Seqs[Seqs[g].Pos].c[0] == lb; g++ ){
            if( Seqs[g].Len > 1 ) k++;
          }
          fwrite(&lb,sizeof(unsigned char),1,file);
          fwrite(&k,sizeof(unsigned char),1,file);
          for( g = j; g < SeqCount && cp2ucsSeqs[i].Seqs[Seqs[g].Pos].c[0] == lb; g++ ){
            if( Seqs[g].Len > 1 ){
              unsigned int & a = *(unsigned int *) (cp2ucsSeqs[i].Seqs[Seqs[g].Pos].c + 1);
              unsigned int b = a + Seqs[g].Len - 1;
              fwrite(&a,cp2ucsSeqs[i].pcp->info.MaxCharSize - 1,1,file);
              fwrite(&b,cp2ucsSeqs[i].pcp->info.MaxCharSize - 1,1,file);
              fwrite(&cp2ucsSeqs[i].Seqs[Seqs[g].Pos].ucs,sizeof(unsigned short),1,file);
            }
          }
          k = 0;
          for( g = j; g < SeqCount && cp2ucsSeqs[i].Seqs[Seqs[g].Pos].c[0] == lb; g++ ){
            if( Seqs[g].Len < 2 ) k++;
          }
          fwrite(&k,sizeof(unsigned char),1,file);
          for( g = j; g < SeqCount && cp2ucsSeqs[i].Seqs[Seqs[g].Pos].c[0] == lb; g++ ){
            if( Seqs[g].Len < 2 ){
              unsigned int & a = *(unsigned int *) (cp2ucsSeqs[i].Seqs[Seqs[g].Pos].c + 1);
              fwrite(&a,cp2ucsSeqs[i].pcp->info.MaxCharSize - 1,1,file);
              fwrite(&cp2ucsSeqs[i].Seqs[Seqs[g].Pos].ucs,sizeof(unsigned short),1,file);
            }
          }
        }
        j++;
      }
    }
    free(Seqs);
    k = 0;
    fwrite(&k,sizeof(unsigned char),1,file);
    fclose(file);
    free(cp2ucsSeqs[i].Seqs);
  }
  free(cp2ucsSeqs);

  cp2ucsSeqs = NULL;
  cp2ucsSeqCount = 0;
  for( i = 0; i < CPEnumCount; i++ ){
    cp2ucsSeqs = (cp2ucs *) realloc(cp2ucsSeqs,sizeof(cp2ucs) * (cp2ucsSeqCount + 1));
    cp2ucs * Seq = cp2ucsSeqs + cp2ucsSeqCount;
    Seq->pcp = CPEnums + i;
    Seq->Seqs = NULL;
    Seq->SeqCount = 0;
    cp2ucsSeq * cSeq;
    for( j = 0; j < 65536; j++ ){
      Seq->Seqs = (cp2ucsSeq *) realloc(Seq->Seqs,sizeof(cp2ucsSeq) * (Seq->SeqCount + 1));
      cSeq = Seq->Seqs + Seq->SeqCount;
      memset(cSeq->c,0,sizeof(cSeq->c));
      cSeq->ucs = (unsigned short) j;
      r = WideCharToMultiByte(Seq->pcp->cp,0/*WC_COMPOSITECHECK | WC_DISCARDNS*/,(wchar_t *)&cSeq->ucs,1,(char *) cSeq->c,5,NULL,NULL);
      for( k = MAX_DEFAULTCHAR - 1; k >= 0; k-- )
        if( Seq->pcp->info.DefaultChar[k] != 0 && cSeq->ucs != Seq->pcp->info.DefaultChar[k] )
          if( strchr(cSeq->c,Seq->pcp->info.DefaultChar[k]) != NULL ) break;
      if( r > 0 && k < 0 ) Seq->SeqCount++;
    }
    cp2ucsSeqCount++;
  }
  for( i = cp2ucsSeqCount - 1; i >= 0; i-- ){
    sprintf(bfile,"..\\..\\src\\libutf8\\utf8cp%d.bin",cp2ucsSeqs[i].pcp->cp);
    file = fopen(bfile,"wb");
    Sequence * Seqs;
    int SeqCount, mSeqCount, sSeqCount;
    findCPSequencesR(Seqs,SeqCount,cp2ucsSeqs + i);
//    fwrite(&cp2ucsSeqs[i].pcp->cp,sizeof(unsigned long),1,file);
    fwrite(&cp2ucsSeqs[i].pcp->info.MaxCharSize,sizeof(unsigned char),1,file);
    fwrite(&cp2ucsSeqs[i].pcp->info.DefaultChar,sizeof(unsigned char),1,file);
    for( k = 1; k <= (int) cp2ucsSeqs[i].pcp->info.MaxCharSize; k++ ){
      mSeqCount = 0;
      for( j = 0; j < SeqCount; j++ ){
        if( Seqs[j].Len < 2 ) continue;
        g = strlen(cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c);
        if( g == k || g == 0 ) mSeqCount++;
      }
      fwrite(&mSeqCount,sizeof(unsigned char),1,file);
      fwrite(&k,sizeof(unsigned char),1,file);
      for( j = 0; j < SeqCount; j++ ){
        if( Seqs[j].Len < 2 ) continue;
        g = strlen(cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c);
        if( g != k && g != 0 ) continue;
        const unsigned short & c = cp2ucsSeqs[i].Seqs[Seqs[j].Pos].ucs;
        fwrite(&c,sizeof(unsigned short),1,file);
        r = c + Seqs[j].Len - 1;
        fwrite(&r,sizeof(unsigned short),1,file);
        fwrite(&cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c,k,1,file);
      }
    }
    k = -1;
    fwrite(&k,sizeof(unsigned char),1,file);
    fwrite(&k,sizeof(unsigned char),1,file);
    for( k = 1; k <= (int) cp2ucsSeqs[i].pcp->info.MaxCharSize; k++ ){
      sSeqCount = 0;
      for( j = 0; j < SeqCount; j++ ){
        if( Seqs[j].Len > 1 ) continue;
        g = strlen(cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c);
        if( g == k || g == 0 ) sSeqCount++;
      }
      fwrite(&sSeqCount,sizeof(unsigned short),1,file);
      fwrite(&k,sizeof(unsigned char),1,file);
      for( j = 0; j < SeqCount; j++ ){
        if( Seqs[j].Len > 1 ) continue;
        g = strlen(cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c);
        if( g != k && g != 0 ) continue;
        const unsigned short & c = cp2ucsSeqs[i].Seqs[Seqs[j].Pos].ucs;
        fwrite(&c,sizeof(unsigned short),1,file);
        fwrite(cp2ucsSeqs[i].Seqs[Seqs[j].Pos].c,k,1,file);
      }
    }
    k = -1;
    fwrite(&k,sizeof(unsigned short),1,file);
    fwrite(&k,sizeof(unsigned char),1,file);
    free(Seqs);
    fclose(file);
    free(cp2ucsSeqs[i].Seqs);
  }
  free(cp2ucsSeqs);
//
  GenEmbed();
// make one indexed big monster file
  FILE * monster = fopen("..\\..\\src\\libutf8\\utf8mod.bin","wb");
// write index
  fwrite(&CPEnumCount,sizeof(unsigned long),1,monster);
  unsigned int offset = sizeof(unsigned long);
  unsigned int SizeOfC1Table = sizeof(C1Table);
  fwrite(&SizeOfC1Table,sizeof(unsigned int),1,monster);
  offset += sizeof(unsigned int);
  unsigned int SizeOfC2Table = sizeof(C2Table);
  fwrite(&SizeOfC2Table,sizeof(unsigned int),1,monster);
  offset += sizeof(unsigned int);
  unsigned int SizeOfC3Table = sizeof(C3Table);
  fwrite(&SizeOfC3Table,sizeof(unsigned int),1,monster);
  offset += sizeof(unsigned long);

  offset += SizeOfC1Table;
  offset += SizeOfC2Table;
  offset += SizeOfC3Table;

  for( i = 0; i < CPEnumCount; i++ ){
    fwrite(&CPEnums[i].cp,sizeof(unsigned long),1,monster);
    struct stat st;
    sprintf(bfile,"cp%dutf8.bin",CPEnums[i].cp);
    stat(bfile,&st);
    fwrite(&offset,sizeof(unsigned long),1,monster);
    offset += st.st_size;
    sprintf(bfile,"utf8cp%d.bin",CPEnums[i].cp);
    stat(bfile,&st);
    fwrite(&offset,sizeof(unsigned long),1,monster);
    offset += st.st_size;
  }
  fwrite(&C1Table,SizeOfC1Table,1,monster);
  fwrite(&C2Table,SizeOfC2Table,1,monster);
  fwrite(&C3Table,SizeOfC3Table,1,monster);
  for( i = 0; i < CPEnumCount; i++ ){
    void * b;
    struct stat st;
    sprintf(bfile,"..\\..\\src\\libutf8\\cp%dutf8.bin",CPEnums[i].cp);
    stat(bfile,&st);
    file = fopen(bfile,"rb");
    b = malloc(st.st_size);
    fread(b,st.st_size,1,file);
    fclose(file);
    remove(bfile);
    fwrite(b,st.st_size,1,monster);
    free(b);
    offset += st.st_size;
    sprintf(bfile,"..\\..\\src\\libutf8\\utf8cp%d.bin",CPEnums[i].cp);
    stat(bfile,&st);
    file = fopen(bfile,"rb");
    b = malloc(st.st_size);
    fread(b,st.st_size,1,file);
    fclose(file);
    remove(bfile);
    fwrite(b,st.st_size,1,monster);
    free(b);
  }
  fclose(monster);
  remove("..\\..\\src\\libutf8\\utf8mod.bin");
}

BOOL CALLBACK EnumCodePagesProc(LPTSTR lpCodePageString)
{
  CPEnums = (CPEnum *) realloc(CPEnums,sizeof(CPEnum) * (CPEnumCount + 1));
  CPEnums[CPEnumCount].cp = atol(lpCodePageString);
  GetCPInfo(CPEnums[CPEnumCount].cp,&CPEnums[CPEnumCount].info);
  if( CPEnums[CPEnumCount].cp != 65000 && CPEnums[CPEnumCount].cp != 65001 )
    CPEnumCount++;
  return TRUE;
}	

int sortCodePages(const void * a,const void * b)
{
  return *(unsigned int *) a - *(unsigned int *) b;
}

int main(int /*argc*/,char ** /*argv*/)
{
  unsigned i;
  for( i = 0; i < 65536; i++ ){
    unsigned short a1, a2, a3;
    DWORD le1, le2, le3;
    GetStringTypeW(CT_CTYPE1,(wchar_t *) &i,1,&a1);
    le1 = GetLastError();
    GetStringTypeW(CT_CTYPE2,(wchar_t *) &i,1,&a2);
    le2 = GetLastError();
    GetStringTypeW(CT_CTYPE3,(wchar_t *) &i,1,&a3);
    le3 = GetLastError();
//    printf("0x%04X ",i);
    if( a1 != 0 || a2 != 0 || a3 != 0 ){
      i = i;
    }
    if( le1 != 0 ){
      *(unsigned int *) (C1Table + i * 9 / 8) |= (a1 & ((1u << 9) - 1)) << i * 9 % 8;
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
    if( le2 != 0 ){
      *(unsigned int *) (C2Table + i * 4 / 8) |= (a2 & ((1u << 4) - 1)) << i * 4 % 8;
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
    if( le3 != 0 ){
      *(unsigned int *) (C3Table + i * 11 / 8) |= (a3 & ((1u << 11) - 1)) << i * 11 % 8;
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

  EnumSystemCodePages((CODEPAGE_ENUMPROC) EnumCodePagesProc,CP_SUPPORTED);
  qsort(CPEnums,CPEnumCount,sizeof(CPEnums[0]),sortCodePages);

  dumpCP2UCS();

  free(CPEnums);
  return 0;
}

