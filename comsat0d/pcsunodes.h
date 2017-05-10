/* pcsunodes */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	PCSUNODES_INCLUDE
#define	PCSUNODES_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


#define	PCSUNODES_MAGIC	0x99447245
#define	PCSUNODES	struct pcsunodes_head
#define	PCSUNODES_CUR	struct pcsunodes_cur


struct pcsunodes_cur {
	int		i ;
} ;

struct pcsunodes_head {
	uint		magic ;
	int		n ;
	cchar		**unodes ;
} ;


#if	(! defined(PCSUNODES_MASTER)) || (PCSUNODES_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pcsunodes_start(PCSUNODES *,cchar *) ;
extern int pcsunodes_get(PCSUNODES *,int,cchar **) ;
extern int pcsunodes_mat(PCSUNODES *,cchar *,int) ;
extern int pcsunodes_curbegin(PCSUNODES *,PCSUNODES_CUR *) ;
extern int pcsunodes_enum(PCSUNODES *,PCSUNODES_CUR *,char *,int) ;
extern int pcsunodes_curend(PCSUNODES *,PCSUNODES_CUR *) ;
extern int pcsunodes_audit(PCSUNODES *) ;
extern int pcsunodes_finish(PCSUNODES *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PCSUNODES_MASTER */

#endif /* PCSUNODES_INCLUDE */


