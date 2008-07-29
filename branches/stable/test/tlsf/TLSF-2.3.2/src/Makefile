WARNINGS = -Wextra -Wall -Wwrite-strings -Wstrict-prototypes \
           -Wmissing-prototypes -Wno-long-long -Wstrict-aliasing=2
CFLAGS+=-g -O2 -I$(TLSF_PATH) $(WARNINGS) -DTLSF_USE_LOCKS=1
# CFLAGS+=-ftest-coverage -fprofile-arcs
CC=gcc

all: tlsf.o

tlsf.o: tlsf.h tlsf.c

clean:
	$(RM) -rf *.o *~ *.c.gcov *.gcda *.gcno
