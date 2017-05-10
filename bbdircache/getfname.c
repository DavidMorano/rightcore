/* getfname */

/* get a file name according to rules */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will form a file name according to some rules.

        This subroutine is used (among others) to find files that are specified
        in configuration files and are supposed to be rooted at one place or
        another in the file system.

        If a type of GETFNAME_TYPELOCAL is given as an argument, the file is
        searched locally before being searched in the program root area. If the
        type is given as GETFNAME_TYPEROOT, the file is searched for in the
        program root area first and then locally.

	Synopsis:

	int getfname(pr,inputname,type,fname)
	const char	pr[] ;
	const char	inputname[] ;
	int		type ;
	char		fname[] ;

	Arguments:

	pr		base directory path to check in
	inputname	the name of the input file to check for
	type		the type of the check to make
		0	search locally first
		1	search in the program root area first
	fname		the output file buffer (user supplied)

	Returns:

	>0		try file at new path
	0		try file as is


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	GETFNAME_TYPELOCAL
#define	GETFNAME_TYPELOCAL	0	/* search locally first */
#define	GETFNAME_TYPEROOT	1	/* search pr area first */
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getfname(pr,inputname,type,fname)
const char	pr[] ;
const char	inputname[] ;
char		fname[] ;
int		type ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	len = 0 ;
	int	f_local = FALSE ;


	if (inputname == NULL)
	    return SR_FAULT ;

	if (fname == NULL)
	    return SR_FAULT ;

/* first */

	if (inputname[0] == '/') {
	    len = 0 ;
	    goto ret0 ;
	}

/* second */

	if ((type == GETFNAME_TYPEROOT) && (pr != NULL)) {

	    rs = mkpath2(fname,pr,inputname) ;
	    len = rs ;
	    if (perm(fname,-1,-1,NULL,W_OK) >= 0)
	        goto ret0 ;

	} else {

	    f_local = TRUE ;
	    if ((perm(inputname,-1,-1,NULL,W_OK) >= 0) &&
	        (u_stat(inputname,&sb) >= 0) && (! S_ISDIR(sb.st_mode))) {
		goto ret0 ;
	    }

	} /* end if */

/* third */

	if ((type == GETFNAME_TYPELOCAL) && (pr != NULL)) {

	    rs = mkpath2(fname,pr,inputname) ;
	    len = rs ;
	    if (perm(fname,-1,-1,NULL,W_OK) >= 0)
	        goto ret0 ;

	} else if (! f_local) {

	    if ((perm(inputname,-1,-1,NULL,W_OK) >= 0) &&
	        (u_stat(inputname,&sb) >= 0) && (! S_ISDIR(sb.st_mode))) {
		len = 0 ;
		goto ret0 ;
	    }

	} /* end if */

/* forth */

	if (type != GETFNAME_TYPEROOT)
	    len = 0 ;

ret0:
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getfname) */



