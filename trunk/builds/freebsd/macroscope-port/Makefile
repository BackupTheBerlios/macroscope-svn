# New ports collection makefile for:	macroscope
# Date created:		2007-05-14
# Whom:			Dukashvili Guram <white_raven@users.berlios.de>
#
# $FreeBSD$
#

PORTNAME=		macroscope
PORTVERSION=		1.0.2108
CATEGORIES=		net-mgmt
MASTER_SITES=		ftp://ftp.berlios.de/pub/macroscope/:src0 \
			http://white-raven.pisem.net/:src1
DISTFILES=		macroscope-1.0.2108.tar.bz2:src0 \
			macroscope-1.0.2108.tar.bz2:src1

MAINTAINER=		white_raven@users.berlios.de
COMMENT=		User and IP traffic management with Web interface

BUILD_DEPENDS+=		cmake:${PORTSDIR}/devel/cmake

OPTIONS=		MYSQL		"With MySQL support" on \
			FIREBIRD	"With Firebird support" on

.include <bsd.port.pre.mk>

.if !defined(WITHOUT_MYSQL)
LIB_DEPENDS+=		mysqlclient_r:${PORTSDIR}/databases/mysql50-client
.endif

.if !defined(WITHOUT_FIREBIRD)
LIB_DEPENDS+=		fbclient:${PORTSDIR}/databases/firebird-client
.endif

LIB_DEPENDS+=		lzo2:${PORTSDIR}/archivers/lzo2
LIB_DEPENDS+=		odbc:${PORTSDIR}/databases/unixODBC
LIB_DEPENDS+=		pcap:${PORTSDIR}/net/libpcap

GNU_CONFIGURE=		no
USE_AUTOTOOLS=		no
USE_RC_SUBR=		macroscope
USE_BZIP2=		yes
USE_CMAKE=		yes
USE_DOS2UNIX=		yes

.include <bsd.port.post.mk>
