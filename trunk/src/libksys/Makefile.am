include $(top_srcdir)/Makefile.am.inc

EXTRA_DIST = \
	sha256c.c \
	lzo1x_1.c \
	lzo1x_1k.c \
	lzo1x_1l.c \
	lzo1x_1o.c \
	lzo1x_9x.c \
	lzo1x_d1.c \
	lzo1x_d2.c \
	lzo1x_d3.c \
	lzo1x_o.c \
	lzo_init.c \
	lzo_crc.c \
	lzo_util.c

lib_LTLIBRARIES = libksys.la

libksys_la_SOURCES = \
	afd.cpp \
	aio.cpp \
	conf.cpp \
	except.cpp \
	fetch.cpp \
	fhc.cpp \
	fiber.cpp \
	log.cpp \
	lzo.cpp \
	mstream.cpp \
	mutant.cpp \
	mutex.cpp \
	perror.cpp \
	profiler.cpp \
	rnd.cpp \
	service.cpp \
	sha.cpp \
	shobjs.cpp \
	stat.cpp \
	sysutils.cpp \
	thread.cpp \
	utime.cpp \
	xalloc.cpp \
	xtime.cpp \
	address.cpp \
	scktintf.cpp \
	client.cpp \
	server.cpp \
	socket.cpp

if BUILD_LZO_LIB
libksys_la_SOURCES += \
	lzo1x_1.c \
	lzo1x_1k.c \
	lzo1x_1l.c \
	lzo1x_1o.c \
	lzo1x_9x.c \
	lzo1x_d1.c \
	lzo1x_d2.c \
	lzo1x_d3.c \
	lzo1x_o.c \
	lzo_init.c \
	lzo_crc.c \
	lzo_util.c
endif

if BUILD_SHA256_LIB
libksys_la_SOURCES += sha256c.c
endif
