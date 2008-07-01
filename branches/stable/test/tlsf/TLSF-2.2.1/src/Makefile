CFLAGS= -O2 -Wall -fomit-frame-pointer
all: dep tlsf.o

tlsf.o: tlsf.h tlsf.c arch_dep.h

clean:
	$(RM) -rf *.o *~

dep:
	@if  uname -m | grep -q 64 ;  then \
		echo -e "TLSF is highly dependant on the processor word length"; \
		echo -e "It has been coded for 32 bit machines.\n"; \
		echo -e "Use at your own risk.\n"; \
	fi

.PHONY: dep