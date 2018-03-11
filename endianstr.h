/* endianstr */


/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

#ifndef	ENDIANSTR_INCLUDE
#define	ENDIANSTR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>		/* for '_XXX_ENDIAN' */
#include	<localmisc.h>


#ifndef	ENDIANSTR
#ifdef	_LITTLE_ENDIAN
#define	ENDIANSTR	"0"
#else
#ifdef	_BIG_ENDIAN
#define	ENDIANSTR	"1"
#else
#if	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0) && defined(__sparc)
#define	ENDIANSTR	"1"
#else
#error	"could not determine endianness of this machine"
#endif /* sparc */
#endif /* _BIG_ENDIAN */
#endif /* _LITTLE_ENDIAN */
#endif /* ENDIANSTR */


#endif /* ENDIANSTR_INCLUDE */


