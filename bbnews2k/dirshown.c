/* dirshown */

/* module to handle the "shown" status of directories */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1995-05-01, David A­D­ Morano
        This code module was completely rewritten to replace any original
        garbage that was here before.

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1995,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module is a simple mechanism to track what articles have been read
        by the user.


*******************************************************************************/


#define	DIRSHOWN_MASTER		1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<string.h>

#include	<vechand.h>
#include	<localmisc.h>

#include	"mkdirlist.h"
#include	"dirshown.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local forward references */

static int	cmpdir(MKDIRLIST_ENT	*,MKDIRLIST_ENT *) ;


/* global variables */


/* local variables */


/* exported subroutines */


int dirshown_start(sdp)
DIRSHOWN	*sdp ;
{


	return vechand_start(sdp,100,0) ;
}
/* end subroutine (dirshown_start) */


int dirshown_finish(sdp)
DIRSHOWN	*sdp ;
{


	return vechand_finish(sdp) ;
}
/* end subroutine (dirshown_finish) */


/* add a directory to the list of those which have been shown already */
int dirshown_set(sdp,dsp)
DIRSHOWN	*sdp ;
MKDIRLIST_ENT	*dsp ;
{


	return vechand_add(sdp,dsp) ;
}
/* end subroutine (dirshown_set) */


/* returns the previous directory entry that matches this one */
int dirshown_already(sdp,dsp,rpp)
DIRSHOWN	*sdp ;
MKDIRLIST_ENT	*dsp ;
MKDIRLIST_ENT	**rpp ;
{
	int	rs ;
	int	i ;

	if (rpp == NULL) return SR_FAULT ;

	for (i = 0 ; (rs = vechand_get(sdp,i,rpp)) >= 0 ; i += 1) {
	    if (*rpp == NULL) continue ;

	    if (cmpdir(dsp,*rpp) == 0) break ;

	} /* end for */

	if ((rs < 0) && (rpp != NULL))
	    *rpp = NULL ;

	return rs ;
}
/* end subroutine (dirshown_already) */


/* local subroutines */


static int cmpdir(dsp1,dsp2)
MKDIRLIST_ENT	*dsp1, *dsp2 ;
{


	if (dsp1->dev < dsp2->dev)
	    return -1 ;

	if (dsp1->dev > dsp2->dev)
	    return 1 ;

	if (dsp1->ino < dsp2->ino)
	    return -1 ;

	if (dsp1->ino > dsp2->ino)
	    return 1 ;

	return 0 ;
}
/* end subroutine (cmpdir) */



