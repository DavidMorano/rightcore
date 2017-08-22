/* densitydbe */

/* density DB entry */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	DENSITYDBE_INCLUDE
#define	DENSITYDBE_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


#define	DENSITYDBE_ALL		struct densitydbe_all
#define	DENSITYDBE_UPD		struct densitydbe_upd

/* entry field lengths */
#define	DENSITYDBE_LCOUNT	4
#define	DENSITYDBE_LUTIME	4	/* entry update */

/* entry field offsets */
/* do this carefully! */
/* there is no good automatic way to do this in C language (sigh) */
/* the C language does not have all of the advantages of assembly language */

#define	DENSITYDBE_OCOUNT	0
#define	DENSITYDBE_OUTIME	(DENSITYDBE_OCOUNT + DENSITYDBE_LCOUNT)
#define	DENSITYDBE_SIZE		(DENSITYDBE_OUTIME + DENSITYDBE_LUTIME)


struct densitydbe_all {
	uint	count ;			/* count */
	uint	utime ;			/* update time */
} ;

struct densitydbe_upd {
	uint	count ;			/* count */
	uint	utime ;			/* update time */
} ;


#if	(! defined(DENSITYDBE_MASTER)) || (DENSITYDBE_MASTER == 0)

extern int densitydbe_all(char *,int,int,DENSITYDBE_ALL *) ;
extern int densitydbe_update(char *,int,int,DENSITYDBE_UPD *) ;

#endif /* DENSITYDBE_MASTER */

#endif /* DENSITYDBE_INCLUDE */


