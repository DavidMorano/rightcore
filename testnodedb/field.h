/* field */

/* structure definition for field extraction calls */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	File was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a classic, ported forward from the old VAX-11/70 days. This was
        translated from VAX assembly language.


*******************************************************************************/

#ifndef	FIELD_INCLUDE
#define	FIELD_INCLUDE	1


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


#define	FIELD		struct field


struct field {
	const uchar	*lp ;		/* line pointer */
	const uchar	*fp ;		/* field pointer */
	int		ll ;		/* line length remaining */
	int		fl ;		/* field length */
	int		term ;		/* terminating character */
} ;


#if	(! defined(FIELD_MASTER)) || (FIELD_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	field_start(FIELD *,const char *,int) ;
extern int	field_get(FIELD *,const unsigned char *,const char **) ;
extern int	field_term(FIELD *,const unsigned char *,const char **) ;
extern int	field_sharg(FIELD *,const unsigned char *,char *,int) ;
extern int	field_remaining(FIELD *,const char **) ;
extern int	field_finish(FIELD *) ;

#ifdef	__cplusplus
}
#endif

/* helper function (like that makes it OK!) */

#ifdef	__cplusplus
extern "C" {
#endif

extern int fieldterms(uchar *,int,char *) ;

#ifdef	__cplusplus
}
#endif

#endif /* FIELD_MASTER */

#endif /* FIELD_INCLUDE */


