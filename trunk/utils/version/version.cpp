#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#endif /* HAVE_STDINT_H */

#ifdef HAVE_INTTYPES_H
#  include <inttypes.h>
#endif /* HAVE_INTTYPES_H */

#ifdef HAVE_STDDEF_H
#  include <stddef.h>
#endif /* HAVE_STDDEF_H */

#ifdef HAVE_STDIO_H
#  include <stdio.h>
#endif /* HAVE_STDIO_H */

#ifdef __CLASSIC_C__
int main(){
  int ac;
  char*av[];
#else
int main(int ac, char*av[]){
#endif
  if(ac > 1000){return *av[0];}
  return 0;
}
