/* pcs-adj(unct) */

/* last modified %G% version %I% */


/* revision history:

	= 2004-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	PCSADJ_INCLUDE
#define	PCSADJ_INCLUDE	1


#ifdef	__cplusplus
extern "C" {
#endif

extern int	pcsadj_begin(PROGINFO *) ;
extern int	pcsadj_end(PROGINFO *) ;
extern int	pcsadj_req(PROGINFO *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* PCSADJ_INCLUDE */


