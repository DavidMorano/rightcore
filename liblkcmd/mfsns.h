/* mfsns */

/* last modified %G% version %I% */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code MFSERVE.

*/

/* Copyright © 2008,2017 David A­D­ Morano.  All rights reserved. */


#ifndef	MFSNS_INCLUDE
#define	MFSNS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<modload.h>
#include	<localmisc.h>


#define	MFSNS_MAGIC	0x99447244
#define	MFSNS		struct mfsns_head
#define	MFSNS_CUR	struct mfsns_c
#define	MFSNS_CALLS	struct mfsns_calls

/* query options */

#define	MFSNS_ONOSERV	(1<<0)		/* do not call the server */
#define	MFSNS_OPREFIX	(1<<1)		/* prefix match */


struct mfsns_c {
	uint	magic ;
	void	*scp ;		/* SO-cursor pointer */
} ;

struct mfsns_calls {
	int	(*open)(void *,cchar *) ;
	int	(*setopts)(void *,int) ;
	int	(*get)(void *,char *,int,cchar *,int) ;
	int	(*curbegin)(void *,void *) ;
	int	(*enumerate)(void *,void *,char *,int,int) ;
	int	(*curend)(void *,void *) ;
	int	(*audit)(void *) ;
	int	(*close)(void *) ;
} ;

struct mfsns_head {
	uint		magic ;
	MODLOAD		loader ;
	MFSNS_CALLS	call ;
	void		*obj ;		/* object pointer */
	int		objsize ;	/* object size */
	int		cursize ;	/* cursor size */
} ;


#if	(! defined(MFSNS_MASTER)) || (MFSNS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mfsns_open(MFSNS *,cchar *) ;
extern int mfsns_setopts(MFSNS *,int) ;
extern int mfsns_get(MFSNS *,char *,int,cchar *,int) ;
extern int mfsns_curbegin(MFSNS *,MFSNS_CUR *) ;
extern int mfsns_enum(MFSNS *,MFSNS_CUR *,char *,int,int) ;
extern int mfsns_curend(MFSNS *,MFSNS_CUR *) ;
extern int mfsns_audit(MFSNS *) ;
extern int mfsns_close(MFSNS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MFSNS_MASTER */

#endif /* MFSNS_INCLUDE */


