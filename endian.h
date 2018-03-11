/* endian */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef ENDIAN_H
#define ENDIAN_H	1


#ifndef	SHA_BYTE_ORDER
#define SHA_BYTE_ORDER 4321
#endif

#ifndef	SHA_VERSION
#define SHA_VERSION 1
#endif

#ifndef	ENDIAN
#ifdef	_LITTLE_ENDIAN
#define	ENDIAN		0
#endif
#ifdef	_BIG_ENDIAN
#define	ENDIAN		1
#endif
#ifndef	ENDIAN
#if	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0) && defined(__sparc)
#define	ENDIAN		1
#else
#error	"could not determine endianness of this machine"
#endif
#endif
#endif /* ENDIAN */


#endif /* ENDIAN_H */


