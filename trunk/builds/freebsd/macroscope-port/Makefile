# New ports collection makefile for:	macroscope
# Date created:		2007-05-14
# Whom:			Dukashvili Guram <white_raven@users.berlios.de>
#
# $FreeBSD$
#

PORTNAME=		macroscope
PORTVERSION=		1.0.1614
CATEGORIES=		net-mgmt
MASTER_SITES=		http://white-raven.pisem.net/

MAINTAINER=		white_raven@users.berlios.de
COMMENT=		Squid, sendmail, bpft log analizer, IP sniffer and HTML report generator

BUILD_DEPENDS+=		cmake:${PORTSDIR}/devel/cmake
LIB_DEPENDS+=		lzo2:${PORTSDIR}/archivers/lzo2
LIB_DEPENDS+=		mysqlclient_r:${PORTSDIR}/databases/mysql[5]+-client
LIB_DEPENDS+=		fbclient:${PORTSDIR}/databases/firebird+-client
LIB_DEPENDS+=		pcap:${PORTSDIR}/net/libpcap

GNU_CONFIGURE=		no
USE_AUTOTOOLS=		no
USE_RC_SUBR=		macroscope
USE_BZIP2=		yes
USE_CMAKE=		yes
USE_DOS2UNIX=		yes

.include <bsd.port.mk>
