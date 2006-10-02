include $(top_srcdir)/Makefile.am.inc

CXX = @CXX@
DEFS = @DEFS@
#DEFAULT_INCLUDES = @DEFAULT_INCLUDES@
#INCLUDES = @INCLUDES@
#AM_CPPFLAGS = @AM_CPPFLAGS@
CPPFLAGS = @CPPFLAGS@
#AM_CXXFLAGS = @AM_CXXFLAGS@
CXXFLAGS = @CXXFLAGS@

CXXCOMPILE = $(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)


#bin_SCRIPTS = gch
#gch: ksys.h.gch adicpp.h.gch

#adicpp.h.gch: adicpp.h
#	$(CXXCOMPILE) -c adicpp.h

#ksys.h.gch: ksys.h
#	$(CXXCOMPILE) -c ksys.h

SUBDIRS = fbapi lzo myapi sha

pkginclude_HEADERS = \
	adicpp.h \
	adidb.h \
	adiexcpt.h \
	adist.h \
	afd.h \
	archive.h \
	array.h \
	async.h \
	autoptr.h \
	bits.h \
	conf.h \
	config.h.borland \
	config.h.mingw \
	config.h.msvc \
	ehash.h \
	endian.h \
	except.h \
	fbcpp.h \
	fbdb.h \
	fbevent.h \
	fbexcept.h \
	fbexcpt.h \
	fbintf.h \
	fbst.h \
	fbsvc.h \
	fbtr.h \
	fhc.h \
	fiber.h \
	hash.h \
	hol.h \
	ksys.h \
	lconfig.h \
	list.h \
	log.h \
	lzo.h \
	mstream.h \
	mutant.h \
	mutex.h \
	mycpp.h \
	mydb.h \
	myexcept.h \
	myexcpt.h \
	myintf.h \
	myst.h \
	mytr.h \
	profiler.h \
	pwerrs.h \
	qsort.h \
	rnd.h \
	scktintf.h \
	service.h \
	sha.h \
	shmem.h \
	shsem.h \
	sockaddr.h \
	socket.h \
	sp.h \
	sysutils.h \
	table.h \
	thread.h \
	tlv.h \
	tree.h \
	utf8embd.h \
	utf8str.h \
	vector.h \
	version.h \
	xalloc.h \
	xtime.h
