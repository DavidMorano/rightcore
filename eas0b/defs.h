/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>


/* miscellaneous */

#define	BUFLEN		200
#define	MAXARG_SUBCKTS	200

/* circuit type descriptor */

struct type {
	struct type	*next ;
	char		*type ;
} ;


/* subcircuit data structures */

struct block {
	struct block	*next ;
	offset_t		start ;
	offset_t		len ;
} ;

struct circuit {
	struct circuit	*next ;
	struct block	*bp ;
	int		type ;
	int		sl ;
	int		lines ;
	char		*name ;
	char		nlen ;
} ;

/* global data structures */

struct flags {
	uint	verbose : 1 ;
	uint	separate : 1 ;
} ;

struct global {
	bfile		*efp ;
	bfile		*ofp ;
	bfile		*ifp ;
	struct flags	f ;
	struct circuit	*top ;
	struct circuit	*bottom ;
	int	debuglevel ;
	char	*progname ;
	char	*tmpdir ;
	char	*buf ;
	char	*suffix ;
} ;


#endif /* DEFS_INCLUDE */


