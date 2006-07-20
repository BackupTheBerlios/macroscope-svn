include $(top_srcdir)/Makefile.am.inc

noinst_PROGRAMS = example1

example1_SOURCES = example1.cpp

example1_LDADD = \
	../src/libadicpp/libadicpp.la \
	../src/libksys/libksys.la \
	../src/libfbcpp/libfbcpp.la \
	../src/libmycpp/libmycpp.la \
	../src/libutf8/libutf8.la

example1_DEPENDENCIES = \
	../src/libadicpp/libadicpp.la \
	../src/libksys/libksys.la \
	../src/libfbcpp/libfbcpp.la \
	../src/libmycpp/libmycpp.la \
	../src/libutf8/libutf8.la
