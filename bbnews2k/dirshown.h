/* dirshown */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DIRSHOWN_INCLUDE
#define	DIRSHOWN_INCLUDE	1


#include	<envstandards.h>
#include	<vechand.h>

#include	"mkdirlist.h"


#define	DIRSHOWN	VECHAND


#ifdef	__cplusplus
extern "C" {
#endif

extern int dirshown_start(DIRSHOWN *) ;
extern int dirshown_finish(DIRSHOWN *) ;
extern int dirshown_set(DIRSHOWN *,MKDIRLIST_ENT *) ;
extern int dirshown_already(DIRSHOWN *,MKDIRLIST_ENT *,MKDIRLIST_ENT **) ;

#ifdef	__cplusplus
}
#endif


#endif /* DIRSHOWN_INCLUDE */


