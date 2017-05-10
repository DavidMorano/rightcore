/* last modified %G% version %I% */

/*
	David A.D. Morano
	April 1990
*/

/**********************************************************************

	Local SYSCAD functions.


***********************************************************************/



#define		F_DEBUG		0



#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<string.h>

#include	"baops.h"
#include	"schema.h"



#ifndef	MAXPATHLEN
#define	MAXPATHLEN	1024
#endif


#define		PATHLEN		MAXPATHLEN
#define		LINELEN		(PATHLEN * 2)

#define		OK		0
#define		BAD		-1



/* externals not covered in 'schema.h' */


/* global forward references */

int		l_attexists(), l_highestname() ;


/* static forward references */

static int	i_showstuff() ;



/* tag gencode */
int test_show(argc,argv)
int	argc ;
char	*argv[] ;
{
	DATA	gp[1], lp[1], ap[1] ;

	COORD	defext[2], frame[2] ;

	int	i ;

	char	curgrp[GRP_NM_SZ + 1] ;


	if (s_getgroup(gp,NULL,0) != OK) {

	    printf(NEED_PAG_SYM) ;

	    return BAD ;
	}

#ifdef	COMMENT
	if (s_inquire(gp,"def_ext",defext) == E_OK)
		printf("defext %g %g %g %g\n",
		defext[0].x,defext[0].y,
		defext[1].x,defext[1].y) ;

	if (s_inquire(gp,"frame",frame) == E_OK)
		printf("frame %g %g %g %g\n",
		frame[0].x,frame[0].y,
		frame[1].x,frame[1].y) ;
#endif

	    if (s_getlist(lp) == 0) { /* there was a selection already */

	        for (i = 1 ; s_getatom(ap,lp,i) == 0 ; i += 1) {

	            i_showstuff(ap) ;

	        }

/* list out what is in the current selection list */

		for (i = 1 ; s_getatom(ap,lp,i) == 0 ; i += 1)
			s_selectobject(ap,SELECT_APPEND) ;

	    } else { /* there was not a selection list already */

	        if (s_id(ap,"ID the thing ") == 0) {

	            i_showstuff(ap) ;

	        }

	    } /* end if */

	return OK ;
}
/* end subroutine (test_show) */


static int i_showstuff(ap)
DATA	ap[1] ;
{
	COORD	plaext[2], defext[2], frame[2] ;
	COORD	defoff[2], plaoff[2] ;

	int	rs, kind ;

	char	buf[GRP_NM_SZ + 1] ;


	if (isSubnet(ap)) return E_OK ;

	if (s_inquire(ap,"def_group_name",buf) == E_OK)
		printf("symbol %s\n",
		buf) ;

	if (s_inquire(ap,"kind",&kind) == E_OK)
		printf("kind %d\n",
		kind) ;

	if (s_inquire(ap,"frame",frame) == E_OK)
		printf("frame %g %g %g %g\n",
		frame[0].x,frame[0].y,
		frame[1].x,frame[1].y) ;

	if (s_inquire(ap,"def_ext",defext) == E_OK)
		printf("defext %g %g %g %g\n",
		defext[0].x,defext[0].y,
		defext[1].x,defext[1].y) ;

	if (s_inquire(ap,"pla_ext",plaext) == E_OK)
		printf("plaext %g %g %g %g\n",
		plaext[0].x,plaext[0].y,
		plaext[1].x,plaext[1].y) ;

	if (s_inquire(ap,"def_off",defoff) == E_OK)
		printf("defoff %g %g \n",
		defoff[0].x,defoff[0].y) ;

	if (s_inquire(ap,"pla_off",plaoff) == E_OK)
		printf("plaoff %g %g \n",
		plaoff[0].x,plaoff[0].y) ;

	return E_OK ;
}
/* end subroutine (i_showstuff) */


