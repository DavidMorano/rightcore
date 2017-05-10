/* rtags */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	RTAGS_INCLUDE
#define	RTAGS_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<hdb.h>
#include	<vecobj.h>
#include	<ptm.h>
#include	<localmisc.h>


#define	RTAGS		struct rtags_head
#define	RTAGS_CUR	struct rtags_c
#define	RTAGS_TAG	struct rtags_tag
#define	RTAGS_TE	struct rtags_te
#define	RTAGS_MAGIC	0x99662223


struct rtags_tag {
	uint		hash ;
	uint		recoff ;
	uint		reclen ;
	char		fname[MAXPATHLEN + 1] ;
} ;

struct rtags_te {
	int		fi ;
	uint		recoff ;
	uint		reclen ;
} ;

struct rtags_head {
	uint		magic ;
	VECOBJ		fnames ;
	VECOBJ		tags ;
	HDB		fni ;
	PTM		m ;		/* mutex */
	int		nfiles ;
} ;

struct rtags_c {
	int		i ;
} ;


#if	(! defined(RTAGS_MASTER)) || (RTAGS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	rtags_start(RTAGS *,int) ;
extern int	rtags_add(RTAGS *,RTAGS_TAG *) ;
extern int	rtags_sort(RTAGS *,int (*)()) ;
extern int	rtags_curbegin(RTAGS *,RTAGS_CUR *) ;
extern int	rtags_curend(RTAGS *,RTAGS_CUR *) ;
extern int	rtags_curdump(RTAGS *,RTAGS_CUR *) ;
extern int	rtags_enum(RTAGS *,RTAGS_CUR *,RTAGS_TAG *) ;
extern int	rtags_count(RTAGS *) ;
extern int	rtags_finish(RTAGS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* RTAGS_MASTER */

#endif /* RTAGS_INCLUDE */


