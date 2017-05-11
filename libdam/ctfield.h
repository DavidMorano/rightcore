/* ctfield */

/* structure definition for ctfield extraction calls */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	File was originally written.
	
*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CTFIELD_INCLUDE
#define	CTFIELD_INCLUDE		1


#if	(! defined(__EXTENSIONS__)) && (! defined(_KERNEL))
#if	defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE)

#ifndef	TYPEDEFS_UNSIGNED
#define	TYPEDEFS_UNSIGNED	1

#ifndef	TYPEDEF_USHORT
#define	TYPEDEF_USHORT	1

typedef unsigned short	ushort ;

#endif

#ifndef	TYPEDEF_UINT
#define	TYPEDEF_UINT	1

typedef unsigned int	uint ;

#endif

#ifndef	TYPEDEF_ULONG
#define	TYPEDEF_ULONG	1

typedef unsigned long	ulong ;

#endif

#endif /* TYPEDEFS_UNSIGNED */

#endif
#endif


#ifndef	TYPEDEF_UCHAR
#define	TYPEDEF_UCHAR	1

typedef unsigned char	uchar ;

#endif /* TYPEDEF_UCHAR */


struct ctfield {
	char	*lp ;			/* line pointer */
	char	*fp ;			/* ctfield pointer */
	int	rlen ;			/* line length remaining */
	int	flen ;			/* ctfield length */
	int	term ;			/* terminating character */
} ;


#define	CTFIELD		struct ctfield


#if	(! defined(CTFIELD_MASTER)) || (CTFIELD_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ctfield_start(CTFIELD *,const char *,int) ;
extern int	ctfield_get(CTFIELD *,const unsigned char *) ;
extern int	ctfield_term(CTFIELD *,const unsigned char *) ;
extern int	ctfield_sharg(CTFIELD *,const unsigned char *,char *,int) ;
extern int	ctfield_finish(CTFIELD *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(CTFIELD_MASTER)) || (CTFIELD_MASTER == 0) */

#endif /* CTFIELD_INCLUDE */


