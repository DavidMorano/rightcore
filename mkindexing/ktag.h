/* ktag */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	KTAG_INCLUDE
#define	KTAG_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<txtindexmk.h>
#include	<eigendb.h>
#include	<localmisc.h>


#define	KTAG		struct ktag_head
#define	KTAG_PARAMS	struct ktag_params
#define	KTAG_KEY	TXTINDEXMK_KEY


struct ktag_params {
	EIGENDB		*edbp ;
	uchar		*wterms ;
	int		minwlen ;
	int		f_eigen ;
} ;

struct ktag_head {
	KTAG_PARAMS	*kap ;
	TXTINDEXMK_KEY	*tkeys ;	/* storage for TXTMKINDEXMK_ADDTAGS */
	const char	*fname ;
	vecobj		keys ;
	vecstr		store ;
	ulong		recoff ;
	ulong		reclen ;
	int		f_store ;
} ;


#if	(! defined(KTAG_MASTER)) || (KTAG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ktag_start(KTAG *,KTAG_PARAMS *,uint,const char *,int) ;
extern int	ktag_add(KTAG *,const char *,int) ;
extern int	ktag_procline(KTAG *,const char *,int) ;
extern int	ktag_procword(KTAG *,const char *,int) ;
extern int	ktag_mktag(KTAG *,uint,TXTINDEXMK_TAG *) ;
extern int	ktag_storelc(KTAG *,const char **,const char *,int) ;
extern int	ktag_finish(KTAG *) ;

#ifdef	__cplusplus
}
#endif

#endif /* KTAG_MASTER */

#endif /* KTAG_INCLUDE */


