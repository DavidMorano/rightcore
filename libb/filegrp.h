/* filegrp */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

#ifndef	FILEGRP_INCLUDE
#define	FILEGRP_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vechand.h>
#include	<cq.h>
#include	<bfile.h>
#include	<localmisc.h>


#define	FILEGRP			struct filegrp_head
#define	FILEGRP_STATS		struct filegrp_s
#define	FILEGRP_ENT		struct filegrp_e

#define	FILEGRP_MAGIC		0x98643163
#define	FILEGRP_DEFENTRIES	10
#define	FILEGRP_DEFMAX		20	/* default maximum entries */
#define	FILEGRP_DEFTTL		600	/* default time-to-live */
#define	FILEGRP_MAXFREE		4


struct filegrp_e {
	bfile		fo ;			/* file object */
	const char	*fname ;		/* file-name */
} ;

struct filegrp_s {
	uint		nentries ;
	uint		total ;			/* access */
	uint		refreshes ;
	uint		phits, pmisses ;	/* positive */
	uint		nhits, nmisses ;	/* negative */
} ;

struct filegrp_head {
	uint		magic ;
	FILEGRP_STATS	s ;
	CQ		recsfree ;
	vechand		recs ;
	time_t		ti_check ;
	int		ttl ;		/* time-to-live in seconds */
	int		max ;		/* maximum entries */
} ;


#if	(! defined(FILEGRP_MASTER)) || (FILEGRP_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int filegrp_start(FILEGRP *,int,int) ;
extern int filegrp_add(FILEGRP *,gid_t,const char *) ;
extern int filegrp_lookname(FILEGRP *,FILEGRP_ENT *,const char *) ;
extern int filegrp_lookgid(FILEGRP *,FILEGRP_ENT *,gid_t) ;
extern int filegrp_check(FILEGRP *,time_t) ;
extern int filegrp_stats(FILEGRP *,FILEGRP_STATS *) ;
extern int filegrp_finish(FILEGRP *) ;

#ifdef	__cplusplus
}
#endif

#endif /* FILEGRP_MASTER */

#endif /* FILEGRP_INCLUDE */


