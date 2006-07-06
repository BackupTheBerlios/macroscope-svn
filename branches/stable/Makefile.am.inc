# For non-gcc compilers, change the options to "no-dependencies"
#AUTOMAKE_OPTIONS = foreign dist-zip
#AUTOMAKE_OPTIONS = foreign dist-zip no-dependencies
# This option prevents autoreconf from overriding our COPYING and
# INSTALL targets:
#AUTOMAKE_OPTIONS = foreign

#AUTOMAKE_OPTIONS =

#DISTCLEANFILES = *~

INCLUDES = -I$(top_srcdir)/include
