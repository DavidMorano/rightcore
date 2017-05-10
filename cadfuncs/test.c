/* test */

/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/*
	= 1990-04-10, David A.D. Morano

*/

/* Copyright © 1990 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Local SYSCAD functions.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<string.h>

#include	"baops.h"
#include	"schema.h"


/* global initialization should be specified from SYSCAD home directory */

#define	SCHPROF_FILE	"cadlib/schema/schprof"

#define	MAXNARG		10

#ifndef	OK
#define	OK		0
#endif
#ifndef	BAD
#define	BAD		-1
#endif


/* externals not covered in 'schema.h' */

extern int	assign() ;


/* global forward references */

int		l_attexists(), l_highestname() ;


/* static forward references */

static int	i_tagtext() ;
static int	i_tagchange() ;


/* exported subroutines */


/* tag gencode */
int damtext(argc,argv)
int	argc ;
char	*argv[] ;
{

	return i_tagtext("damtext",1,argc,argv) ;
}


/* local subroutines */


static int i_tagtext(attp,level,argc,argv)
char	*attp ;
int	level ;
int	argc ;
char	*argv[] ;
{
	DATA	gp[1], lp[1], ap[1] ;

	COORD	defext[2], frame[2] ;

	int	i ;

	char	curgrp[GRP_NM_SZ + 1] ;
	char	*plottext ;


	if (s_getgroup(gp,NULL,0) != OK) {

	    printf(NEED_PAG_SYM) ;

	    return BAD ;
	}

	if (s_inquire(gp,"def_ext",defext) == E_OK)
		printf("defext %g %g %g %g\n",
		defext[0].x,defext[0].y,
		defext[1].x,defext[1].y) ;

	if (s_inquire(gp,"frame",frame) == E_OK)
		printf("frame %g %g %g %g\n",
		frame[0].x,frame[0].y,
		frame[1].x,frame[1].y) ;

	if (argc < 2) return BAD ;

	if (argc == 2) { /* only one argument was supplied by user */

	    if (argv[1] == NULL) return BAD ;

	    if (s_getlist(lp) == 0) { /* there was a selection already */

		s_graphicalUpdate(TRUE,YES) ;

	        for (i = 1 ; s_getatom(ap,lp,i) == 0 ; i += 1) {

	            i_showstuff(ap) ;

	        }

/* list out what is in the current selection list */

		for (i = 1 ; s_getatom(ap,lp,i) == 0 ; i += 1)
			s_selectobject(ap,SELECT_APPEND) ;

	    } else { /* there was not a selection list already */

	        if (s_id(ap,"ID for \"%s\" ",argv[1]) == 0) {

	            i_tagchange(ap,attp,argv[1],level) ;

	        }
	    }

	} else { /* handle the case of more than one argument */

	    for (i = 1 ; i < argc ; i += 1) {

	        s_id(ap,"ID for \"%s\" ",argv[i]) ;

	        i_tagchange(ap,attp,argv[i],level) ;

	    }
	}

	return OK ;
}
/* end subroutine (i_tagtext) */


static int i_tagchange(ap,attp,valuep,level)
DATA	ap[1] ;
char	*attp, *valuep ;
int	level ;
{
	int	rs ;

	char	buf[400] ;


	if (isSubnet(ap)) return E_OK ;

	if ((s_inquire(ap,attp,buf) != E_OK) || isText(ap)) {

	    if (level >= 0) {
	        rs = s_atmcmd(dummy,ap,
	            "tag text %s \"%s\" tlevel %d",
	            attp,valuep,level) ;

	    } else
	        rs = s_atmcmd(dummy,ap,
	            "tag text %s \"%s\" unplot",
	            attp,valuep) ;

	} else if (s_stracmp(attp,"parameters") == 0) {

	    rs = s_atmcmd(dummy,ap,
	        "del atom att %s",
	        attp) ;

	    if (rs >= 0) rs = s_atmcmd(dummy,ap,
	        "tag text %s \"%s\" tlevel %d",
	        attp,valuep,level) ;

	} else {

	    rs = s_atmcmd(dummy,ap,
	        "cha atom att %s \"%s\"",
	        attp,valuep) ;

	}

	if (rs < 0) {

	    printf(
	        "tt: tag or change failed with (%d)\n",
	        rs) ;

	}

	return E_OK ;
}
/* end subroutine (i_tagchange) */



