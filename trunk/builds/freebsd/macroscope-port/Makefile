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
DISTFILES=		${PORTNAME}-${PORTVERSION}.tar.bz2:src0 \
			${PORTNAME}-${PORTVERSION}.tar.bz2:src1

MAINTAINER=		white_raven@users.berlios.de
COMMENT=		User and IP traffic management with Web interface

OPTIONS=		MYSQL		"With MySQL support" on \
			FIREBIRD	"With Firebird support" off

GNU_CONFIGURE=		no
USE_AUTOTOOLS=		no
USE_RC_SUBR=		macroscope
USE_BZIP2=		yes
USE_CMAKE=		yes
USE_DOS2UNIX=		yes

BUILD_DEPENDS+=		cmake:${PORTSDIR}/devel/cmake
LIB_DEPENDS+=		lzo2:${PORTSDIR}/archivers/lzo2
LIB_DEPENDS+=		odbc:${PORTSDIR}/databases/unixODBC
LIB_DEPENDS+=		pcap:${PORTSDIR}/net/libpcap

CMAKE_ARGS=		-DCMAKE_BUILD_TYPE:STRING=Release \
			-DCMAKE_C_COMPILER:STRING="${CC}" \
			-DCMAKE_C_FLAGS:STRING="${CFLAGS}" \
			-DCMAKE_CXX_COMPILER:STRING="${CXX}" \
			-DCMAKE_CXX_FLAGS:STRING="${CXXFLAGS}" \
			-DCMAKE_INSTALL_PREFIX=${PREFIX}

.include <bsd.port.pre.mk>

.if defined(NOPORTDOCS)
CMAKE_ARGS+=		-DCMAKE_NOT_INSTALL_DOC:BOOL=ON
.endif

.if !defined(WITHOUT_MYSQL)
LIB_DEPENDS+=		mysqlclient_r:${PORTSDIR}/databases/mysql50-client
.endif

.if !defined(WITHOUT_FIREBIRD)
LIB_DEPENDS+=		fbclient:${PORTSDIR}/databases/firebird-client
.endif

do-configure:
	@(cd ${WRKSRC}; \
	${LOCALBASE}/bin/cmake ${CMAKE_ARGS} .)

.if !defined(NOPORTDOCS)
post-install:
	@${ECHO_CMD} share/doc/macroscope/mysql-user >> ${TMPPLIST}
	@${ECHO_CMD} share/doc/macroscope/AUTHORS >> ${TMPPLIST}
	@${ECHO_CMD} share/doc/macroscope/README >> ${TMPPLIST}
	@${ECHO_CMD} share/doc/macroscope/INSTALL >> ${TMPPLIST}
	@${ECHO_CMD} share/doc/macroscope/NEWS >> ${TMPPLIST}
	@${ECHO_CMD} @dirrm share/doc/macroscope >> ${TMPPLIST}
	@${STRIP_CMD} ${PREFIX}/sbin/macroscope
.endif

.include <bsd.port.post.mk>
