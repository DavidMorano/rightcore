/* icalmk */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	ICALMK_INCLUDE
#define	ICALMK_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecobj.h>
#include	<localmisc.h>


#define	ICALMK_MAGIC	0x88773422
#define	ICALMK		struct icalmk_head
#define	ICALMK_OBJ	struct icalmk_obj
#define	ICALMK_ENT	struct icalmk_e
#define	ICALMK_LINE	struct icalmk_l
#define	ICALMK_FL	struct icalmk_flags


struct icalmk_obj {
	const char	*name ;
	uint		objsize ;
} ;

struct icalmk_l {
	uint		loff ;
	uint		llen ;
} ;

struct icalmk_e {
	struct icalmk_l	*lines ;
	uint		voff ;
	uint		vlen ;
	uint		hash ;
	uchar		nlines, m, d ;
} ;

struct icalmk_flags {
	uint		viopen:1 ;
	uint		notsorted:1 ;
	uint		tmpfile:1 ;
	uint		creat:1 ;
	uint		excl:1 ;
	uint		none:1 ;
	uint		inprogress:1 ;
	uint		created:1 ;
} ;

struct icalmk_head {
	uint		magic ;
	const char	*calname ;
	const char	*dbname ;
	const char	*idname ;
	const char 	*cyifname ;
	char		*nfname ;
	ICALMK_FL	f ;
	VECOBJ		verses ;
	VECOBJ		lines ;
	uint		pcitation ;
	int		nentries ;
	int		operms ;
	int		nfd ;
	int		year ;
} ;


#if	(! defined(ICALMK_MASTER)) || (ICALMK_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	icalmk_open(ICALMK *,cchar *,cchar *,int,int,int,int) ;
extern int	icalmk_add(ICALMK *,ICALMK_ENT *) ;
extern int	icalmk_close(ICALMK *) ;

#ifdef	__cplusplus
}
#endif

#endif /* ICALMK_MASTER */

#endif /* ICALMK_INCLUDE */


