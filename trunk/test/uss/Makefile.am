include $(top_srcdir)/Makefile.am.inc

EXTRA_DIST = FORMAT README

noinst_PROGRAMS = usstest

usstest_SOURCES = driver.cpp

usstest_LDADD = \
	../../src/libadicpp/libadicpp.la \
	../../src/libutf8/libutf8.la \
	../../src/libksys/libksys.la \
	../../src/libfbcpp/libfbcpp.la \
	../../src/libmycpp/libmycpp.la

usstest_DEPENDENCIES = \
	../../src/libadicpp/libadicpp.la \
	../../src/libutf8/libutf8.la \
	../../src/libksys/libksys.la \
	../../src/libfbcpp/libfbcpp.la \
	../../src/libmycpp/libmycpp.la
