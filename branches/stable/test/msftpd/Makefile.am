include $(top_srcdir)/Makefile.am.inc

EXTRA_DIST = nv.sh version.h

SUB_DIRS = BCB6 VC70

sysconf_DATA = msftpd.conf

noinst_PROGRAMS = msftpd

msftpd_SOURCES = main.cpp

msftpd_LDADD = \
	../../src/libadicpp/libadicpp.la \
	../../src/libutf8/libutf8.la \
	../../src/libksys/libksys.la \
	../../src/libfbcpp/libfbcpp.la \
	../../src/libmycpp/libmycpp.la

msftpd_DEPENDENCIES = \
	../../src/libadicpp/libadicpp.la \
	../../src/libutf8/libutf8.la \
	../../src/libksys/libksys.la \
	../../src/libfbcpp/libfbcpp.la \
	../../src/libmycpp/libmycpp.la
