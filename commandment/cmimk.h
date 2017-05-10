/* cmimk */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CMIMK_INCLUDE
#define	CMIMK_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<vecobj.h>
#include	<localmisc.h>


#define	CMIMK_MAGIC	0x88773425
#define	CMIMK		struct cmimk_head
#define	CMIMK_OBJ	struct cmimk_obj
#define	CMIMK_ENT	struct cmimk_e
#define	CMIMK_LINE	struct cmimk_l
#define	CMIMK_INFO	struct cmimk_i
#define	CMIMK_FL	struct cmimk_flags
#define	CMIMK_NE	4
#define	CMIMK_INTOPEN	(10*60)
#define	CMIMK_INTSTALE	(5*60)


/* this is the object description */
struct cmimk_obj {
	const char	*name ;
	uint		objsize ;
} ;

struct cmimk_i {
	uint		maxent ;
	uint		nents ;
} ;

struct cmimk_l {
	uint		loff ;
	uint		llen ;
} ;

struct cmimk_e {
	CMIMK_LINE	*lines ;
	uint		eoff ;
	uint		elen ;
	ushort		nlines, cn ;
} ;

struct cmimk_flags {
	uint		notsorted:1 ;
	uint		ofcreat:1 ;
	uint		ofexcl:1 ;
	uint		inprogress:1 ;
	uint		created:1 ;
	uint		abort:1 ;
} ;

struct cmimk_head {
	uint		magic ;
	cchar 		*dbname ;
	cchar		*idname ;
	char		*nidxfname ;
	CMIMK_FL	f ;
	VECOBJ		ents ;
	VECOBJ		lines ;
	mode_t		om ;
	time_t		ti_db ;
	size_t		size_db ;
	uint		pcn ;		/* previous command-number (PCN) */
	uint		maxent ;
	uint		nents ;
	int		nfd ;
} ;


#if	(! defined(CMIMK_MASTER)) || (CMIMK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	cmimk_open(CMIMK *,cchar *,int,mode_t) ;
extern int	cmimk_setdb(CMIMK *,size_t,time_t) ;
extern int	cmimk_add(CMIMK *,CMIMK_ENT *) ;
extern int	cmimk_abort(CMIMK *,int) ;
extern int	cmimk_info(CMIMK *,CMIMK_INFO *) ;
extern int	cmimk_close(CMIMK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CMIMK_MASTER */

#endif /* CMIMK_INCLUDE */


