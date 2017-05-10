/* calent */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	CALENT_INCLUDE
#define	CALENT_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>
#include	"calcite.h"


#define	CALENT		struct calent_head
#define	CALENT_Q	CALCITE
#define	CALENT_LINE	struct calent_line
#define	CALENT_FL	struct calent_flags
#define	CALENT_NLE	4		/* starting number of lines */


struct calent_line {
	uint		loff ;
	int		llen ;
} ;

struct calent_flags {
	uint		hash:1 ;
} ;

struct calent_head {
	uint		magic ;
	CALENT_LINE	*lines ;
	CALENT_FL	f ;
	CALENT_Q	q ;
	uint		voff ;
	uint		vlen ;
	uint		hash ;
	int		e ;		/* (lines) extent */
	int		i ;		/* (lines) index */
	int		cidx ;
} ;


#if	(! defined(CALENT_MASTER)) || (CALENT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	calent_start(CALENT *,CALENT_Q *,uint,int) ;
extern int	calent_setidx(CALENT *,int) ;
extern int	calent_add(CALENT *,uint,int) ;
extern int	calent_finish(CALENT *) ;
extern int	calent_getci(CALENT *) ;
extern int	calent_mkhash(CALENT *,cchar *) ;
extern int	calent_sethash(CALENT *,uint) ;
extern int	calent_gethash(CALENT *,uint *) ;
extern int	calent_loadbuf(CALENT *,char *,int,cchar *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CALENT_MASTER */

#endif /* CALENT_INCLUDE */


