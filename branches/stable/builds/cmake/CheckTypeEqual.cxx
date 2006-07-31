#if defined(CHECK_TYPE_EQUAL1) && defined(CHECK_TYPE_EQUAL2)

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

#ifdef __CLASSIC_C__
int main(){
  int ac;
  char*av[];
#else
int main(int ac, char*av[]){
#endif
  if(ac > 1000){return *av[0];}
  struct intptr_t_check {
    void f(CHECK_TYPE_EQUAL1){}
    void f(CHECK_TYPE_EQUAL2){}
  };
  return 0;
}

#else  /* CHECK_TYPE_EQUAL */

#  error "CHECK_TYPE_EQUAL has to specify the variable"

#endif /* CHECK_TYPE_EQUAL */
