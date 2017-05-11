/* msgide */

/* machine status entry */


/* revision history:

	= 2003-03-04, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

#ifndef	MSGIDE_INCLUDE
#define	MSGIDE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<netdb.h>

#include	<localmisc.h>


/* object defines */

#define	MSGIDE_ALL		struct msgide_all
#define	MSGIDE_UPDATE		struct msgide_update


/* entry field lengths */
#define	MSGIDE_LCOUNT		4
#define	MSGIDE_LUTIME		4	/* entry update */
#define	MSGIDE_LCTIME		4	/* entry creation */
#define	MSGIDE_LMTIME		4	/* message */
#define	MSGIDE_LHASH		4
#define	MSGIDE_LRECIPIENT	32
#define	MSGIDE_LMESSAGEID	(2 * MAXHOSTNAMELEN)
#define	MSGIDE_LFROM		(2 * MAXHOSTNAMELEN)

/* entry field offsets */
/* do this carefully! */
/* there is no good automatic way to do this in C language (sigh) */
/* the C language does not have all of the advantages of assembly language */

#define	MSGIDE_OCOUNT		0
#define	MSGIDE_OUTIME		(MSGIDE_OCOUNT + MSGIDE_LCOUNT)
#define	MSGIDE_OCTIME		(MSGIDE_OUTIME + MSGIDE_LUTIME)
#define	MSGIDE_OMTIME		(MSGIDE_OCTIME + MSGIDE_LCTIME)
#define	MSGIDE_OHASH		(MSGIDE_OMTIME + MSGIDE_LMTIME)
#define	MSGIDE_ORECIPIENT	(MSGIDE_OHASH + MSGIDE_LHASH)
#define	MSGIDE_OMESSAGEID	(MSGIDE_ORECIPIENT + MSGIDE_LRECIPIENT)
#define	MSGIDE_OFROM		(MSGIDE_OMESSAGEID + MSGIDE_LMESSAGEID)

#define	MSGIDE_SIZE		(MSGIDE_OFROM + MSGIDE_LFROM)


struct msgide_all {
	uint		count ;		/* count */
	uint		utime ;		/* update time-stamp */
	uint		ctime ;		/* creation time-stamp */
	uint		mtime ;		/* message time-stamp */
	uint		hash ;
	char		recipient[MSGIDE_LRECIPIENT + 1] ;
	char		messageid[MSGIDE_LMESSAGEID + 1] ;
	char		from[MSGIDE_LFROM + 1] ;
} ;

struct msgide_update {
	uint		count ;
	uint		utime ;
} ;


#if	(! defined(MSGIDE_MASTER)) || (MSGIDE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int msgide_all(struct msgide_all *,int,char *,int) ;
extern int msgide_update(struct msgide_update *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* MSGIDE_MASTER */

#endif /* MSGIDE_INCLUDE */


