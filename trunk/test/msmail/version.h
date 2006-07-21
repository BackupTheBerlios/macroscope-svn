/*
**  version.h -- Version Information for msftp (syntax: C/C++)
**  [automatically generated and maintained by GNU shtool]
*/

#ifdef _VERSION_H_AS_HEADER_

#ifndef _VERSION_H_
#define _VERSION_H_

#define MSFTP_VERSION 0x1002D3

typedef struct {
    const int   v_hex;
    const char *v_short;
    const char *v_long;
    const char *v_tex;
    const char *v_gnu;
    const char *v_web;
    const char *v_sccs;
    const char *v_rcs;
} msftp_version_t;

extern msftp_version_t msftp_version;

#endif /* _VERSION_H_ */

#else /* _VERSION_H_AS_HEADER_ */

#define _VERSION_H_AS_HEADER_
#include "version.h"
#undef  _VERSION_H_AS_HEADER_

msftp_version_t msftp_version = {
    0x1002D3,
    "1.0.211",
    "1.0.211 (21-Jul-2006)",
    "This is msftp, Version 1.0.211 (21-Jul-2006)",
    "msftp 1.0.211 (21-Jul-2006)",
    "msftp/1.0.211",
    "@(#)msftp 1.0.211 (21-Jul-2006)",
    "$Id: msftp 1.0.211 (21-Jul-2006) $"
};

#endif /* _VERSION_H_AS_HEADER_ */

