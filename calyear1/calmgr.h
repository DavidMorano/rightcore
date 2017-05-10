/* calmgr */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	CALMGR_INCLUDE
#define	CALMGR_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vechand.h>
#include	<vecobj.h>
#include	<localmisc.h>

#include	"calyears.h"
#include	"calent.h"
#include	"cyi.h"
#include	"calcite.h"


#define	CALMGR		struct calmgr_head
#define	CALMGR_Q	CALCITE
#define	CALMGR_CUR	struct calmgr_c
#define	CALMGR_FL	struct calmgr_flags


struct calmgr_c {
	uint		magic ;
	void		*results ;
	uint		nresults ;
	int		i ;
} ;

struct calmgr_flags {
	uint		idxes:1 ;		/* IDXES container open */
} ;

struct calmgr_head {
	cchar		*dn ;			/* DB directory-name */
	cchar 		*cn ;			/* DB calendar-name */
	cchar		*a ;			/* memory-allocations */
	cchar		*idxdname ;
	void		*calyears ;
	cchar		*mapdata ;		/* DB memory-map address */
	vechand		idxes ;			/* indices */
	CALMGR_FL	f ;
	time_t		ti_db ;			/* DB file modification */
	time_t		ti_map ;		/* DB map */
	time_t		ti_lastcheck ;		/* DB last check */
	size_t		filesize ;		/* DB file size */
	size_t		mapsize ;		/* DB map length */
	int		nentries ;		/* DB entries */
	int		cidx ;			/* parent index (ordinal) */
} ;


#if	(! defined(CALMGR_MASTER)) || (CALMGR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int calmgr_start(CALMGR *,CALYEARS *,int,cchar *,cchar *) ;
extern int calmgr_finish(CALMGR *) ;
extern int calmgr_lookup(CALMGR *,vecobj *,CALCITE *) ;
extern int calmgr_gethash(CALMGR *,CALENT *,uint *) ;
extern int calmgr_getci(CALMGR *) ;
extern int calmgr_getbase(CALMGR *,cchar **) ;
extern int calmgr_loadbuf(CALMGR *,char *,int,CALENT *) ;
extern int calmgr_audit(CALMGR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CALMGR_MASTER */

#endif /* CALMGR_INCLUDE */


