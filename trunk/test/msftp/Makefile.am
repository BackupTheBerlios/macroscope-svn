include $(top_srcdir)/Makefile.am.inc

EXTRA_DIST = nv.sh version.h

SUB_DIRS = BCB6 VC70

#sysconf_DATA = msftp.conf

noinst_PROGRAMS = msftp

msftp_SOURCES = main.cpp

msftp_LDADD = \
	../../src/libadicpp/libadicpp.la \
	../../src/libutf8/libutf8.la \
	../../src/libksys/libksys.la \
	../../src/libfbcpp/libfbcpp.la \
	../../src/libmycpp/libmycpp.la

msftp_DEPENDENCIES = \
	../../src/libadicpp/libadicpp.la \
	../../src/libutf8/libutf8.la \
	../../src/libksys/libksys.la \
	../../src/libfbcpp/libfbcpp.la \
	../../src/libmycpp/libmycpp.la
