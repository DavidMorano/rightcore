/* last modified %G% version %I% */

/* DiSCOfunc (local SYSCAD SCHEMA CI functions for DiSCO) */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_PULLDOWN	1
#define	CF_ST		0
#define	CF_SUN		1
#define	CF_INIT		1


/* revision history:

	= 1992-12-01, David A­D­ Morano

	This code was originated in response to new CAD usability
	needs of the DiSCO project (an Area-11 research project).

*/

/* Copyright © 1992 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These SYSCAD SCHEMA CI functions are especially designed
	for use on the DiSCO project.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	"schema.h"


/* local defines */

/* level used for tagged "signal type" information */

#define	ST_LEVEL	"50"
#define	NFIELDS		5

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#define	OK		0
#define	BAD		-1

#define	TRUE		1
#define	FALSE		0


/* local structures */


/* forward references */

int	st() ;


/* local static data */

static DATA	dummy[1] ;


/* exported subroutines */


#if	CF_INIT

int _file_init()
{
	double		dm ;


	dm = s_getvar("designmode") ;


/* stuff for "signal type" support */

	if (dm > 0.5) {

	    s_command(dummy,"set level 100 2") ;

	    s_command(dummy,"set level 50 2") ;

#ifdef	COMMENT
	    s_makemenu("mine","my menu","quit,-here",0) ;
#endif

	    s_pulldown("signal type","signal type",
	        "tn,st tn",
	        "tc,st tc",
	        "en,st en",
	        "ec,st ec",
	        "es,st es",
	        "ed,st ed",
	        "pn,st pn",
	        "pc,st pc",
	        "ps,st ps",
	        "pd,st pd",
	        "vr,st vr",
	        "va,st va",
	        "vn,st vn",
	        "vp,st vp",
	        "eln,st eln",
	        "elc,st elc",
	        "els,st els",
	        "eld,st eld",
	        "acn,st acn",
	        "acc,st acc",
	        "btn,st btn",
	        "ben,st ben",
	        "batn,st batn",
	        "ttn,st ttn",
	        "ttc,st ttc",
	        NULL
	        ) ;

	} /* end if of CP design mode */


	return OK ;
}
/* end subroutine (_file_init) */

#endif CF_INIT


/* signal type */

#ifdef	COMMENT

static char	*st_names[] = {
	"tn",
	"tc",
	"en",
	"ec",
	"es",
	"ed",
	"pn",
	"pc",
	"ps",
	"pd",
	"vr",
	"va",
	"vn",
	"vp",
	"eln",
	"elc",
	"els",
	"eld",
	"acn",
	"acc",
	"btn",
	"btc",
	"ben",
	"bec",
	"batn",
	"bttn",
	"bttc",
	"ttn",
	"ttc",
	(char *) 0,
} ;

#endif


int st(argc,argv)
int	argc ;
char	*argv[] ;
{
	DATA	ap[1] ;

	int	i, f_err, kind ;
#ifdef	COMMENT
	int	f_w = FALSE ;
#endif

	char	*np, *cp ;
#ifdef	COMMENT
	char	*w = "7" ;
#endif
	char	curgrp[GRP_NM_SZ + 1] ;
	char	buf[SYMNAMELEN + 1] ;

	if (s_curgroup(curgrp) != 0) {

	    printf(NEED_PAG_SYM) ;

	    return BAD ;
	}

	s_resetid() ;

	np = "" ;
	if (argc > 1) np = argv[1] ;

	if ((np == ((char *) 0)) || (*np == '\0')) {

	    printf("\007null name given\n") ;

	    return BAD ;
	}


	if ((strlen(np) + 2) > SYMNAMELEN) {

	    printf("\007name is too long\n") ;

	    return BAD ;
	}

/* does the given have have any garbage */

	cp = np ;
	while ((*cp != '\0') && (! isdigit(*cp))) {

	    if ((*cp == '[') || (*cp == ']')) {

	        printf("\007cannot use compressed names\n") ;

	        return BAD ;
	    }

	    cp += 1 ;
	}

/* is the number correct (I forgot what this "signal width" thing is !) */

#ifdef	COMMENT
	if (isdigit(*cp)) {

	    switch ((int) *cp) {

	    case '6':
	    case '7':
	    case '8':
	        f_w = TRUE ;
	        w = cp ;
	        break ;

	    default:
	        printf("\007incorrect signal width specified\n") ;

	        return BAD ;
	    }
	}
#endif

/* is the base name valid */

#ifdef	COMMENT
	for (i = 0 ; (cp = st_names[i]), (cp != ((char *) 0)) ; i += 1) {

	    if (strncmp(np,cp,2) == 0) break ;

	}

	if (cp == ((char *) 0)) {

	    printf("\007bad signal type given\n") ;

	    return BAD ;
	}
#endif

/* is the tagged text string in the correct format ? */
/* find the number of colons in the text string */

	cp = np ;
	i = 0 ;
	while (*cp != '\0') if (*cp++ == ':') i += 1 ;

/* do we have the proper number of fields separated by colons ? */
/* currently there should be three fields separated by two colons */

	if (i < 2) {

	    strcpy(buf,np) ;

	    strcat(buf,":") ;

	    if (i == 0) strcat(buf,":") ;

	    np = buf ;
	}

/* OK, do it */

	s_savestate() ; /* snapshot off any accumulated changes */

	for (i = 0 ; i < 1000 ; i += 1) {

	    f_err = TRUE ;
	    while (f_err) {

	        s_id(ap,"ID a wire ") ;

	        if (s_inquire(ap,"kind",&kind) < 0) goto badinquire ;

	        switch (kind) {

	        case WIRE:
	        case SUBNET:
	            f_err = s_atmcmd(dummy,ap,
	                "tag text st %s plotted tlevel %s",
	                np,ST_LEVEL) ;

	            break ;

	        default:
	            printf(
	                "\007sorry, must ID a subnet ; try again\n",
	                argv[1]) ;

	        } /* end switch */

	    } /* end while */

	    s_savestate() ; /* snapshot off any accumulated changes */

	}

	return E_OK ;

badinquire:
	printf("%s: bad atom inquiry\n") ;

	return BAD ;
}
/* end subroutine (st) */


/* extract the "signal type" information from a schematic */

/***********************************************************************

	Arguments to this command :

		schema> extract_st [page [file]]

	where :
	page	- optional SCHEMA page group to edit
	file	- optional file name of a file to append write the results to


	OK, what is the purpose and strategy of this thing ?

	+ find all subnets with a "st" keyfield (and only one "st"
	  keyfield) on it

	+ write out the text string associated with the keyfield
		formatted output is :

		EXTRACT_ST:page:subnet_name:st:rt:rc

		where :

			EXTRACT_ST		line header
			page			SCHEMA page
			subnet_name		the subnet name
			st			susceptability threshold
			rt			rise time
			rc			routing code


***********************************************************************/

int st_extract(argc,argv)
int	argc ;
char	*argv[] ;
{
	FILE	*fp ;

	DATA	ap[1], gp[1], sap[1], wap[1] ;

	int	i, j, k ;
	int	kind ;
	int	f_file = FALSE ;
	int	f_name ;
	int	f_st ;
	int	f_error ;

	char	curgrp[GRP_NM_SZ + 1] ;
	char	wirename[SYMNAMELEN + 1] ;
	char	junk_string[SYMNAMELEN + 1] ;
	char	st_string[SYMNAMELEN + 1] ;
	char	*fmt_multi ;
	char	*fmt_name ;
	char	*fmt_good ;
	char	*cp ;


/* were we given a page to edit ? */

	if (argc >= 2) {

	    s_stracpy(curgrp,argv[1],2) ;

	    s_command(dummy,"edit %s",curgrp) ;

	} else {

	    if (s_curgroup(curgrp) != 0) {

	        printf(NEED_PAG_SYM) ;

	        return BAD ;
	    }
	}

/* were we given a file name to write out to ? */

	if (argc >= 3) {

	    fp = fopen(argv[2],"a") ;

	    if (fp == ((FILE *) 0)) {

	        printf("%s: could not open file \"%s\"\n",
	            argv[0],argv[2]) ;

	        return BAD ;
	    }

	    f_file = TRUE ;
	}

/* OK, let's go ; get the atom pointer to the current SCHEMA page group */

	if (s_getgroup(gp,curgrp,0) != 0) {

	    printf("%s: cannot access the current group \"%s\"\n",
	        argv[0],curgrp) ;

	    return BAD ;
	}

/* find the "dot" (if there is one) in the group name and delete it */

	cp = curgrp ;
	while (*cp != '\0') {

	    if (*cp == '.') {

	        for (i = 0 ; cp[i] != '\0' ; i += 1) cp[i] = cp[i + 1] ;

	    } else cp += 1 ;

	}

	fmt_multi = 
	    "ERROR-extract_st: multiple type - p %s, n %s, st \"%s\"\n" ;

	fmt_good = 
	    "EXTRACT_ST:%s:%s:%s\n" ;

	fmt_name = 
	    "ERROR-extract_st: no name - p %s, st \"%s\"\n" ;

/* loop through all of the atoms in this group for subnets */

	for (i = 1 ; s_getatom(sap,gp,i) == 0 ; i += 1) {

	    if ((s_inquire(sap,"kind",&kind) != 0) ||
	        (kind != SUBNET)) continue ;

	    f_st = FALSE ;
	    f_name = FALSE ;
	    for (k = 1 ; (! s_getatom(wap,sap,k)) ; k += 1) {

	        if (s_inquire(wap,"kind",&kind) || (kind != WIRE))
	            continue ;

	        if ((! f_name) && (s_inquire(wap,"NAME",wirename) == 0))
	            f_name = TRUE ;

	        if (s_inquire(wap,"ST",st_string) != 0)
	            continue ;

	        f_st = TRUE ;
	        k += 1 ;
	        break ;
	    }

/* note that the variable 'k' is ripe to be used from where it is already */

/* if we found a "signal type", then do some stuff, else continue looping */

	    if (f_st) {

/* audit if there is another "st" keyfield on another wire in the subnet */

	        f_error = FALSE ;
	        for ( ; (! s_getatom(ap,sap,k)) ; k += 1) {

	            if (s_inquire(ap,"kind",&kind) || (kind != WIRE))
	                continue ;

	            if ((! f_name) && (s_inquire(wap,"NAME",wirename) == 0))
	                f_name = TRUE ;

	            if (! f_error) {

	                if (s_inquire(ap,"ST",junk_string) != 0)
	                    continue ;

	                if (strcmp(st_string,junk_string) == 0) continue ;

	                f_error = TRUE ;
	            }

	            if (f_name && f_error) break ;

	        }

	        if (! f_name) wirename[0] = '\0' ;

	        if (f_error) {

/* we have an error, print it out */

	            if (f_file) {

	                fprintf(fp,fmt_multi,
	                    curgrp,wirename,st_string) ;

	            } else {

	                printf(fmt_multi,
	                    curgrp,wirename,st_string) ;

	            }

	        } else {

/* we got something, check for three (colon separated) fields */

	            j = 0 ;
	            cp = st_string ;
	            while (*cp != '\0') if (*cp++ == ':') j += 1 ;

	            for ( ; j < (NFIELDS - 1) ; j += 1) strcat(st_string,":") ;

/* we got the information, write it out */

	            if (f_name) {

	                if (f_file) {

	                    fprintf(fp,fmt_good,
	                        wirename,st_string,curgrp) ;

	                } else {

	                    printf(fmt_good,
	                        wirename,st_string,curgrp) ;

	                }

	            } else {

	                if (f_file) {

	                    fprintf(fp,fmt_name,
	                        curgrp,st_string) ;

	                } else {

	                    printf(fmt_name,
	                        curgrp,st_string) ;

	                }

	            } /* end if */

	        } /* end if */

/* end of having a "signal type" */

	    } else {

/* there may be no signal type, but did we find a name */
/* NOTE CAREFULLY HERE : the 'define' NFIELDS should be reflected here */

	        if (f_name) {

	            if (f_file)

	                fprintf(fp,"EXTRACT_ST:%s::::::%s\n",
	                    wirename,curgrp) ;

	            else

	                printf("EXTRACT_ST:%s::::::%s\n",
	                    wirename,curgrp) ;

	        }

	    } /* end if */

	} /* end outer-most for */

	return OK ;
}
/* end subroutine (st_extract) */


