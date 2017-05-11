/* li */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	LI_INCLUDE
#define	LI_INCLUDE	1


/* object defines */

#define	LI	struct longint


struct longint {
	long		u, l ;
} ;


#ifndef	LI_MASTER

#ifdef	__cplusplus
extern "C" {
#endif

extern int li_load(LI *,int,int) ;
extern int li_store(LI *,int *,int *) ;
extern int li_zero(LI *) ;
extern int li_add2(LI *,LI *) ;
extern int li_add3(LI *,LI *,LI *) ;

#ifdef	__cplusplus
}
#endif

#endif /* LI_MASTER */

#endif /* LI_INCLUDE */


