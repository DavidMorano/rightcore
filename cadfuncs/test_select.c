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



/* global initialization should be specified from SYSCAD home directory */

#define		SCHPROF_FILE		"cadlib/schema/schprof"


#define		PATHLEN		200
#define		LINELEN		(PATHLEN * 2)
#define		NAMELEN		100
#define		MAXNARG		10

#define		OK		0
#define		BAD		-1



/* externals not covered in 'schema.h' */

extern int	assign() ;


/* global forward references */

int		l_attexists(), l_highestname() ;


/* static forward references */

static int	i_tagtext() ;
static int	i_tagchange() ;



/* tag gencode */
int damtext(argc,argv)
int	argc ;
char	*argv[] ;
{

	return i_tagtext("damtext",1,argc,argv) ;
}



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

	if (argc < 2) return BAD ;

	if (argc == 2) { /* only one argument was supplied by user */

	    if (argv[1] == NULL) return BAD ;

	    if (s_getlist(lp) == 0) { /* there was a selection already */

#ifdef	COMMENT
		s_graphicalUpdate(TRUE,YES) ;
#endif

	        for (i = 1 ; s_getatom(ap,lp,i) == 0 ; i += 1) {

	            i_tagchange(ap,attp,argv[1],level) ;

	        }

/* list out what is in the current selection list */

		for (i = 1 ; s_getatom(ap,lp,i) == 0 ; i += 1) {

			if (s_inquire(ap,"def_group_name",curgrp) == E_OK)
			printf("damtext: current selection %s\n",curgrp) ;

#ifdef	COMMENT
			s_highlight(ap) ;
#else
			s_selectobject(ap,SELECT_APPEND) ;
#endif

		}

/* get a new list and list out what is in the current selection list */

	    if (s_getlist(lp) == 0) { /* there was a selection already */

		for (i = 1 ; s_getatom(ap,lp,i) == 0 ; i += 1) {

			if (s_inquire(ap,"def_group_name",curgrp) == E_OK)
			printf("damtext: new selection %s\n",curgrp) ;

		}

		} else
			printf("damtext: there was no new list\n") ;

#ifdef	COMMENT
		s_graphicalUpdate(TRUE,YES) ;
#endif

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

	    if (level >= 0)
	        rs = s_atmcmd(dummy,ap,
	            "tag text %s \"%s\" tlevel %d",
	            attp,valuep,level) ;

	    else
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


