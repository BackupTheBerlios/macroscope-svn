/*
**  version.h -- Version Information for libadicpp (syntax: C/C++)
**  [automatically generated and maintained by GNU shtool]
*/

#ifdef _VERSION_H_AS_HEADER_

#ifndef _VERSION_H_
#define _VERSION_H_

#define LIBADICPP_VERSION 0x101234

typedef struct {
    const int   v_hex;
    const char *v_short;
    const char *v_long;
    const char *v_tex;
    const char *v_gnu;
    const char *v_web;
    const char *v_sccs;
    const char *v_rcs;
} libadicpp_version_t;

extern libadicpp_version_t libadicpp_version;

#endif /* _VERSION_H_ */

#else /* _VERSION_H_AS_HEADER_ */

#define _VERSION_H_AS_HEADER_
#include "version.h"
#undef  _VERSION_H_AS_HEADER_

libadicpp_version_t libadicpp_version = {
    0x101234,
    "1.1.52",
    "1.1.52 (28-Feb-2006)",
    "This is libadicpp, Version 1.1.52 (28-Feb-2006)",
    "libadicpp 1.1.52 (28-Feb-2006)",
    "libadicpp/1.1.52",
    "@(#)libadicpp 1.1.52 (28-Feb-2006)",
    "$Id: libadicpp 1.1.52 (28-Feb-2006) $"
};

#endif /* _VERSION_H_AS_HEADER_ */

