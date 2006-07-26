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
  FILE * out = fopen(av[2],"w+b");
  if( out == NULL ) return errno;
  if( fseek(inp,0,SEEK_END) != 0 ) return errno;
  long inpSize = ftell(inp);
  if( fseek(inp,0,SEEK_SET) != 0 ) return errno;
  char * inpBuffer = (char *) malloc(inpSize + 1);
  if( inpBuffer == NULL ) return errno;
  if( fread(inpBuffer,inpSize,1,inp) != 1 ) return errno;
  inpBuffer[inpSize] = '\0';
  char versionString[256], target[256], upperTarget[256];
  unsigned int version, ver, rev, lev;
  if( sscanf(inpBuffer,"%s %s %u.%u.%u",versionString,target,&ver,&rev,&lev) != 5 ) return -1;
  for( int i = sizeof(target) - 1; i >= 0; i-- ) upperTarget[i] = toupper(target[i]);
  if( strcmp(av[1],av[2]) == 0 ){ // increase level
    if( ++lev >= (1u << 12) ){
      lev = 0;
      if( ++rev >= (1u << 10) ){
        rev = 0;
        ver++;
      }
    }
    if( fprintf(out,"%s %s %u.%u.%u\n",versionString,target,ver,rev,lev) == -1 ) return errno;
  }
  else {
    version = (ver << 22) | (rev << 12) | lev;
    if( fprintf(out,
      "#ifdef _VERSION_H_AS_HEADER_\n\n"
      "#ifndef _VERSION_H_\n"
      "#define _VERSION_H_\n\n"
      "#define %s_VERSION 0x%x\n\n"
      "typedef struct {\n"
      "  unsigned int v_hex;\n"
      "  const char * v_short;\n"
      "  const char * v_long;\n"
      "  const char * v_tex;\n"
      "  const char * v_gnu;\n"
      "  const char * v_web;\n"
      "  const char * v_sccs;\n"
      "  const char * v_rcs;\n"
      "} %s_version_t;\n\n"
      "extern %s_version_t %s_version;\n\n"
      "#endif /* _VERSION_H_ */\n\n"
      "#else /* _VERSION_H_AS_HEADER_ */\n\n"
      "#define _VERSION_H_AS_HEADER_\n"
      "#include \"version.h\"\n"
      "#undef  _VERSION_H_AS_HEADER_\n\n"
      "%s_version_t %s_version = {\n"
      "  0x%X,\n"
      "  \"%u.%u.%u\",\n"
      "  \"%u.%u.%u (21-Jul-2006)\",\n"
      "  \"This is %s, Version %u.%u.%u (21-Jul-2006)\",\n"
      "  \"%s %u.%u.%u (21-Jul-2006)\",\n"
      "  \"%s/%u.%u.%u\",\n"
      "  \"@(#)%s %u.%u.%u (21-Jul-2006)\",\n"
      "  \"$Id: %s %u.%u.%u (21-Jul-2006) $\"\n"
      "};\n\n"
      "#endif /* _VERSION_H_AS_HEADER_ */\n",
      upperTarget,
      version,
      target,
      target,
      target,
      target,
      target,
      version,
      ver,rev,lev,
      ver,rev,lev,
      target,
      ver,rev,lev,
      target,
      ver,rev,lev,
      target,
      ver,rev,lev,
      target,
      ver,rev,lev,
      target,
      ver,rev,lev
    ) == -1 ) return errno;
  }
  fflush(out);
  return 0;
}
