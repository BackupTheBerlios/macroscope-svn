include $(top_srcdir)/Makefile.am.inc

EXTRA_DIST = macroscope.h

SUB_DIRS = BCB6 VC60 VC70

sysconf_DATA = macroscope.conf

sbin_PROGRAMS = macroscope

macroscope_SOURCES = macroscope.cpp htmlrep.cpp

macroscope_LDADD = \
	../../src/libadicpp/libadicpp.la \
	../../src/libutf8/libutf8.la \
	../../src/libksys/libksys.la \
	../../src/libfbcpp/libfbcpp.la \
	../../src/libmycpp/libmycpp.la

macroscope_DEPENDENCIES = \
	../../src/libadicpp/libadicpp.la \
	../../src/libutf8/libutf8.la \
	../../src/libksys/libksys.la \
	../../src/libfbcpp/libfbcpp.la \
	../../src/libmycpp/libmycpp.la
