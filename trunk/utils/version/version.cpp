#include <adicpp/lconfig.h>

#ifdef __CLASSIC_C__
int main(){
  int ac;
  char*av[];
#else
int main(int ac, char*av[]){
#endif
  if(ac > 1000){return *av[0];}
  if( ac < 3 ) return EINVAL;
  FILE * inp = fopen(av[1],"rb");
  if( inp == NULL ) return errno;
  FILE * out = fopen(av[2],"r+b");
  if( out == NULL ) out = fopen(av[2],"w+b");
  if( out == NULL ) return errno;
  if( fseek(out,0,SEEK_SET) != 0 ) return errno;
  if( fseek(inp,0,SEEK_END) != 0 ) return errno;
  long inpSize = ftell(inp);
  if( fseek(inp,0,SEEK_SET) != 0 ) return errno;
  char * inpBuffer = (char *) malloc(inpSize + 1);
  if( inpBuffer == NULL ) return errno;
  if( fread(inpBuffer,inpSize,1,inp) != 1 ) return errno;
  fclose(inp);
  inpBuffer[inpSize] = '\0';
  char versionString[256], target[256], upperTarget[256], upperOutName[256];
  unsigned int version, ver, rev, lev;
  if( sscanf(inpBuffer,"%s %s %u.%u.%u",versionString,target,&ver,&rev,&lev) != 5 ) return -1;
  for( int i = sizeof(target) - 1; i >= 0; i-- ) upperTarget[i] = toupper(target[i]);
  for( unsigned u = 0; u < sizeof(upperOutName); u++ ){
    upperOutName[u] = toupper(av[2][u]);
    if( upperOutName[u] == '.' || isspace(upperOutName[u]) ) upperOutName[u] = '_';
    if( av[2][u] == '\0' ) break;
  }
  version = (ver << 22) + (rev << 12) + lev;
  version++;
  if( ++lev >= (1u << 12) ){
    lev = 0;
    if( ++rev >= (1u << 10) ){
      rev = 0;
      ver++;
    }
  }
  if( strcmp(av[1],av[2]) == 0 ){ // increase level
    if( fprintf(out,"%s %s %u.%u.%u\n",versionString,target,ver,rev,lev) == -1 ) return errno;
  }
  else {
    static const char machine[] =
#if __pentium__ || __pentium_mmx__
      "pentium"
#elif __pentiumpro__
      "pentiumpro"
#elif __k6__
      "k6"
#elif __k6_2__
      "k6_2"
#elif __k6_3__
      "k6_3"
#elif __athlon__
      "athlon"
#elif __k8__
      "k8"
#elif __pentium4__
      "pentium4"
#elif __amd64__
      "amd64"
#elif __x86_64__
      "x86_64"
#elif (_M_IX86 >= 300 && _M_IX86 < 400) || __i386__
      "i386"
#elif (_M_IX86 >= 400 && _M_IX86 < 500) || __i486__
      "i486"
#elif (_M_IX86 >= 500 && _M_IX86 < 600) || __i586__
      "i586"
#elif (_M_IX86 >= 600 && _M_IX86 < 700) || __i686__
      "i686"
#elif _M_IX86
      "x86"
#elif _M_X64
      "x86_64"
#endif
#if __MMX__ || __pentium_mmx__
      "+MMX"
#endif
#if __3dNOW__
      "+3dNOW"
#endif
#if __3dNOW_A__
      "+3dNOW_A"
#endif
#if _M_IX86_FP == 1 || __SSE__ || __athlon_sse__
      "+SSE"
#elif _M_IX86_FP == 2 || __SSE2__
      "+SSE2"
#elif _M_IX86_FP == 3 || __SSE3__
      "+SSE3"
#endif
    ;

#if HAVE__TZSET
    _tzset();
#elif HAVE_TZSET
    tzset();
#endif
    time_t t;
    time(&t);
    //Wed Jan 02 02:03:55 1980
    char * ts = ctime(&t);
    ts[7] = '\0';
    ts[10] = '\0';
    ts[19] = '\0';
    ts[24] = '\0';
    if( fprintf(out,
      "#ifdef _%s_AS_HEADER_\n\n"
      "#ifndef _%s_\n"
      "#define _%s_\n\n"
      "#define %s_VERSION 0x%x\n\n"
      "#ifdef __cplusplus\n"
      "extern \"C\" {\n"
      "#endif\n\n"
      "typedef struct {\n"
      "  unsigned int hex_;\n"
      "  unsigned short version_;\n"
      "  unsigned short revision_;\n"
      "  unsigned short level_;\n"
      "  const char * short_;\n"
      "  const char * long_;\n"
      "  const char * tex_;\n"
      "  const char * gnu_;\n"
      "  const char * web_;\n"
      "  const char * sccs_;\n"
      "  const char * rcs_;\n"
      "} %s_version_t;\n\n"
      "extern %s_version_t %s_version;\n\n"
      "#ifdef __cplusplus\n"
      "} // extern \"C\"\n"
      "#endif\n\n"
      "#endif /* _%s_ */\n\n"
      "#else /* _%s_AS_HEADER_ */\n\n"
      "#define _%s_AS_HEADER_\n"
      "#include \"%s\"\n"
      "#undef  _%s_AS_HEADER_\n\n"
      "#ifdef __cplusplus\n"
      "extern \"C\" {\n"
      "#endif\n\n"
      "%s_version_t %s_version = {\n"
      "  0x%X,\n"
      "  0x%X,\n"
      "  0x%X,\n"
      "  0x%X,\n"
      "  \"%u.%u.%u %s\",\n"
      "  \"%u.%u.%u %s (%s-%s-%s %s)\",\n"
      "  \"This is %s, Version %u.%u.%u %s (%s-%s-%s %s)\",\n"
      "  \"%s %u.%u.%u %s (%s-%s-%s %s)\",\n"
      "  \"%s/%u.%u.%u %s\",\n"
      "  \"@(#)%s %u.%u.%u %s (%s-%s-%s %s)\",\n"
      "  \"$Id: %s %u.%u.%u %s (%s-%s-%s %s) $\"\n"
      "};\n\n"
      "#ifdef __cplusplus\n"
      "} // extern \"C\"\n"
      "#endif\n\n"
      "#endif /* _%s_AS_HEADER_ */\n",
      upperOutName,
      upperOutName,
      upperOutName,
      upperTarget,
      version,
      target,
      target,
      target,
      upperOutName,
      upperOutName,
      upperOutName,
      av[2],
      upperOutName,
      target,
      target,
      version,
      ver,rev,lev,
      ver,rev,lev, machine,
      ver,rev,lev, machine,
      ts + 8,
      ts + 4,
      ts + 20,
      ts + 11,
      target,
      ver,rev,lev, machine,
      ts + 8,
      ts + 4,
      ts + 20,
      ts + 11,
      target,
      ver,rev,lev, machine,
      ts + 8,
      ts + 4,
      ts + 20,
      ts + 11,
      target,
      ver,rev,lev, machine,
      target,
      ver,rev,lev, machine,
      ts + 8,
      ts + 4,
      ts + 20,
      ts + 11,
      target,
      ver,rev,lev, machine,
      ts + 8,
      ts + 4,
      ts + 20,
      ts + 11,
      upperOutName
    ) == -1 ) return errno;
  }
  fflush(out);
#if HAVE__CHSIZE && HAVE__FILENO
  if( _chsize(_fileno(out),ftell(out)) != 0 ) return errno;
#elif HAVE__CHSIZE && HAVE_FILENO
  if( _chsize(fileno(out),ftell(out)) != 0 ) return errno;
#elif HAVE_CHSIZE && HAVE__FILENO
  if( chsize(_fileno(out),ftell(out)) != 0 ) return errno;
#elif HAVE_CHSIZE && HAVE_FILENO
  if( chsize(fileno(out),ftell(out)) != 0 ) return errno;
#elif HAVE_FTRUNCATE && HAVE__FILENO
  if( ftruncate(_fileno(out),ftell(out)) != 0 ) return errno;
#elif HAVE_FTRUNCATE && HAVE_FILENO
  if( ftruncate(fileno(out),ftell(out)) != 0 ) return errno;
#endif
  fclose(out);
#if HAVE_STAT
  struct stat st;
  if( stat(av[1],&st) != 0 ) return errno;
#elif HAVE__STAT
  struct _stat st;
  if( _stat(av[1],&st) != 0 ) return errno;
#endif
#if HAVE_UTIME
  struct utimbuf ut;
  ut.actime = st.st_atime;
  ut.modtime = st.st_mtime;
  if( utime(av[2],&ut) != 0 ) return errno;
#elif HAVE__UTIME
  struct _utimbuf ut;
  ut.actime = st.st_atime;
  ut.modtime = st.st_mtime;
  if( _utime(av[1],&ut) != 0 ) return errno;
#endif
  return 0;
}
