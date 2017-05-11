/* cyimk */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CYIMK_INCLUDE
#define	CYIMK_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecobj.h>
#include	<localmisc.h>


#define	CYIMK_MAGIC	0x88773422
#define	CYIMK		struct cyimk_head
#define	CYIMK_OBJ	struct cyimk_obj
#define	CYIMK_ENT	struct cyimk_e
#define	CYIMK_LINE	struct cyimk_l
#define	CYIMK_FL	struct cyimk_flags
#define	CYIMK_INTOPEN	(10*60)
#define	CYIMK_INTSTALE	(5*60)


/* this is the shared-object (SO) description */
struct cyimk_obj {
	const char	*name ;
	uint		objsize ;
} ;

struct cyimk_l {
	uint		loff ;
	uint		llen ;
} ;

struct cyimk_e {
	struct cyimk_l	*lines ;
	uint		voff ;
	uint		vlen ;
	uint		hash ;
	uchar		nlines, m, d ;
} ;

struct cyimk_flags {
	uint		notsorted:1 ;
	uint		ofcreat:1 ;
	uint		ofexcl:1 ;
	uint		none:1 ;
	uint		inprogress:1 ;
	uint		created:1 ;
	uint		abort:1 ;
} ;

struct cyimk_head {
	uint		magic ;
	const char	*idname ;
	const char	*cname ;
	char		*nidxfname ;
	CYIMK_FL	f ;
	VECOBJ		verses ;
	VECOBJ		lines ;
	mode_t		om ;
	gid_t		gid ;
	uid_t		uid ;
	uint		pcitation ;
	int		nentries ;
	int		nfd ;
	int		year ;
} ;


#if	(! defined(CYIMK_MASTER)) || (CYIMK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	cyimk_open(CYIMK *,int,cchar *,cchar *,int,mode_t) ;
extern int	cyimk_add(CYIMK *,CYIMK_ENT *) ;
extern int	cyimk_abort(CYIMK *,int) ;
extern int	cyimk_close(CYIMK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CYIMK_MASTER */

#endif /* CYIMK_INCLUDE */


