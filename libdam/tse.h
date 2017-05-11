/* tse */

/* machine status entry */


/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

#ifndef	TSE_INCLUDE
#define	TSE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<localmisc.h>


/* object defines */

#define	TSE_ALL		struct tse_all
#define	TSE_UPDATE	struct tse_update


/* entry field lengths */
#define	TSE_LCOUNT		4
#define	TSE_LUTIME		4	/* entry update */
#define	TSE_LCTIME		4	/* entry creation */
#define	TSE_LHASH		4
#define	TSE_LKEYNAME		32

/* entry field offsets */
/* do this carefully! */
/* there is no good automatic way to do this in C language (sigh) */
/* the C language does not have all of the advantages of assembly language! */

#define	TSE_OCOUNT		0
#define	TSE_OUTIME		(TSE_OCOUNT + TSE_LCOUNT)
#define	TSE_OCTIME		(TSE_OUTIME + TSE_LUTIME)
#define	TSE_OHASH		(TSE_OCTIME + TSE_LCTIME)
#define	TSE_OKEYNAME		(TSE_OHASH + TSE_LHASH)

#define	TSE_SIZE		(TSE_OKEYNAME + TSE_LKEYNAME)


struct tse_all {
	uint		count ;		/* count */
	uint		utime ;		/* update time-stamp */
	uint		ctime ;		/* creation time-stamp */
	uint		hash ;
	char		keyname[TSE_LKEYNAME+ 1] ;
} ;

struct tse_update {
	uint		count ;
	uint		utime ;
} ;


#if	(! defined(TSE_MASTER)) || (TSE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int tse_all(struct tse_all *,int,char *,int) ;
extern int tse_update(struct tse_update *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* TSE_MASTER */

#endif /* TSE_INCLUDE */


