#if defined(CHECK_EMPTY_ARRAY_SIZE)

#ifdef __CLASSIC_C__
int main(){
  int ac;
  char*av[];
#else
int main(int ac, char*av[]){
#endif
  char emptyArray[CHECK_EMPTY_ARRAY_SIZE];
  if(ac > 1000){return *av[0];}
  return 0;
}

#else  /* CHECK_EMPTY_ARRAY_SIZE */

#  error "CHECK_EMPTY_ARRAY_SIZE has to specify the variable"

#endif /* CHECK_EMPTY_ARRAY_SIZE */
