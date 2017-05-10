/* gettable */


#define	CF_DEBUGS	0		/* compile-time debugging */


/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
 *		T.S.Kennedy						
 *		J.Mukerji						
 *									

*									
*	FUNCTIONAL DESCRIPTION:						
*	'Gettable' reads a name translation table and places the	
*	in the structure 'names'.					
*									
*	PARAMETERS:							
*	namelist	file names containing data (separated by	
*			colons) taken from global variable.		
*	sysname		system name					
*									
*	RETURNED VALUE:							
*									
*	SUBROUTINES CALLED:						
*									

*	NOTE:								
*	The lines beginning with 'jm' comment were added by J.Mukerji	
*	on 12/22/83.  These lines check for array bounds while copying	
*	one string into another and in general, prevents coredumps!	
*									

************************************************************************/


#include	<envstandards.h>

#include	<string.h>
#include	<pwd.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"config.h"
#include	"smail.h"


/* external subroutines */


/* external variables */

extern struct global	g ;

extern struct table	*nmalloc() ;


/* local variables */

static char sccsid[] = "@(#)gettable.c	PCS 3.0" ;


/* exported subroutines */


void gettable(sysname)
char	*sysname ;
{
	FILE *fp ;

	struct table	*tname ;

	int	lnmin1, syslmin1 ;
	int	type ;

	char	line[BUFSIZE], *l ;
	char *file,files[BUFSIZE] ;
	char *realname ;
	char *address ;
	char *c ;


	if (table_is_in) return ;	/* no need to read it again */

/* read each table one at a time */

	lnmin1 = LENNAME-1 ;
	syslmin1 = SYSLEN-1 ;
	file = strtok(namelist,":") ;

	while (file != NULL) {

/* determine table type, is this a user table or a system table ? */

	    type = TT_USER ;
	    if (substring(file,strlen(file),PCS) >= 0) {

	        type = TT_SYSTEM ;
#if	CF_DEBUGS
	        errprintf("got a system translation\n") ;
#endif

	    }

/* open translation file */

	    if ((fp = fopen(file, "r")) == NULL) {

	        file = strtok(0,":") ;

	        continue ;
	    }

/* save rest of file names */

	    file = strtok(0,"") ;

	    if (file == NULL) files[0] = '\0' ;

	    else strcpy(files, file ) ;

/* The 100 below used to be BUFSIZE. I changed it to 100 to make
*  sure that by chance realname and address strings never come out to be
*  greater than 100 in length. Of course, this can cause truncation
*  of an address if it is longer than 60 characters in length. Oh well..
*  can't win 'em all! However, absence of coredumps is preferable to
*  truncation of an address or two!   ..jm
*/
/*jm*/

/*
	I do not know what Jishnu was talking about above but
	the use of the 'strncpy's below seem to preclude any
	core dumps alone.  This may be some sort of software
	programmer braindamage confusion (oh, if the EEs did the
	programming again) but I am not sure.
	DAM 01/06/94
*/

	    while (fgetline(fp,line,BUFSIZE) > 0) {

	        l = line ;

/* empty line */

	        if (*l == '\n') continue ;

/* skip over any leading white space */

	        while ((*l == ' ') || (*l == '\t')) l += 1 ;

/* skip over comment lines in the table */

	        if (*l == '#') continue ;

/* get the real name */

	        realname = l ;
	        if ((l = strpbrk(l,": \t\n")) == ((char *) 0)) continue ;

	        *l++ = '\0' ;
	        if (*realname == '\0') continue ;

/* skip over white space -- or a colon -- (new feature for unknown purpose) */

	        while ((*l == ' ') || (*l == '\t') || (*l == ':'))
	            l += 1 ;

/* get email address */

	        address = l ;
	        if ((l = strpbrk(l,",\\ \t\n")) == ((char *) 0)) continue ;

	        *l++ = '\0' ;
	        if (*address == '\0') continue ;

/* if name database overflows then drop out */

	        if ((tname = nmalloc()) == NULL) break ;

/* add to table */

	        strncpy(tname->realname,realname,lnmin1) ;

	        tname->realname[lnmin1] = '\0' ;
	        tname->mail = 0 ;

/* strip off local system name */

	        if (*address == '!') address++ ;

	        for (c = address ; (*c != '!') && *c ; c += 1) ;

	        if (*c) {

	            *c++ = '\0' ;
	            if(strcmp(address,sysname) == 0) {

	                strncpy(tname->mailaddress,c,syslmin1) ;

	                tname->mailaddress[syslmin1] = '\0' ;

	            } else {

	                *(--c) = '!' ;

/*jm*/
	                strncpy(tname->mailaddress,address,syslmin1) ;
/*jm*/
	                tname->mailaddress[syslmin1] = '\0' ;
	            }

	        } else {

/*jm*/
	            strncpy(tname->mailaddress,address,syslmin1) ;
/*jm*/
	            tname->mailaddress[syslmin1] = '\0' ;
	        }

	        tname->type = type ;

	    } /* end while */

	    fclose(fp) ;

	    file = strtok(files,":") ;

	}  /* end while */

	if (tname == NULL) {

#if CF_DEBUGS
	    fprintf(errlog,"%s	%s	%s\n",
	        getpwuid(getuid())->pw_name, datetime,
	        "gettable: translation table overflow") ;
#endif
	    fprintf(stderr,"warning: translation table truncated\n") ;

	}

	table_is_in = 1 ;

}
/* end subroutine (gettable) */


/* eventually we will check if any of the tables have changed since */
/* they were read last. For now this is a NOOP			    */

int table_has_changed()
{


	return 0 ;
}


regettable()
{
	table_is_in = 0 ;
	tablelen = 0 ;
	gettable(g.nodename) ;

}


