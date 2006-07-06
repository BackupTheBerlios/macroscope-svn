/*
**  version.h -- Version Information for msftpd (syntax: C/C++)
**  [automatically generated and maintained by GNU shtool]
*/

#ifdef _VERSION_H_AS_HEADER_

#ifndef _VERSION_H_
#define _VERSION_H_

#define MSFTPD_VERSION 0x100200

typedef struct {
    const int   v_hex;
    const char *v_short;
    const char *v_long;
    const char *v_tex;
    const char *v_gnu;
    const char *v_web;
    const char *v_sccs;
    const char *v_rcs;
} msftpd_version_t;

extern msftpd_version_t msftpd_version;

#endif /* _VERSION_H_ */

#else /* _VERSION_H_AS_HEADER_ */

#define _VERSION_H_AS_HEADER_
#include "version.h"
#undef  _VERSION_H_AS_HEADER_

msftpd_version_t msftpd_version = {
    0x100200,
    "1.0.0",
    "1.0.0 (18-Aug-2005)",
    "This is msftpd, Version 1.0.0 (18-Aug-2005)",
    "msftpd 1.0.0 (18-Aug-2005)",
    "msftpd/1.0.0",
    "@(#)msftpd 1.0.0 (18-Aug-2005)",
    "$Id: version.h,v 1.1.1.1 2005/08/24 12:51:04 korvin Exp $"
};

#endif /* _VERSION_H_AS_HEADER_ */

