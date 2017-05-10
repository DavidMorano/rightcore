/* checkname */

/* check a file name */


#define	CF_DEBUG 	0


/* revision history:

	= 1996-03-01, David A­D­ Morano

	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/***********************************************************************

	This module first checks to see if the file meets the
	specified requirements to be considered in the file size sum.
	If a file is supposed to be included, the size of the file
	is added to the running sum.

	Arguments:

	- name		directory entry
	- sbp		'stat' block pointer
	- gp		pointer to program's global data
	- pp		pointer to the suffixes to consider

	Returns:

	<0		exit directory walk altogether
	0		continue with directory walk as usual
	>0		skip this directory entry in directory walk


***********************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<dirent.h>
#include	<limits.h>
#include	<ctype.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<paramopt.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 10),2048)
#endif


/* external subroutines */

extern int	sfbasename(const char *,int,const char **) ;


/* external variables */


/* forward references */

static int	addsize() ;


/* exported subroutines */


int checkname(name,sbp,ckp)
const char	name[] ;
struct ustat	*sbp ;
struct checkparams	*ckp ;
{
	struct ustat	sb2 ;

	struct global	*gp = ckp->gp ;

	PARAMOPT	*pp = ckp->pp ;

	int	rs ;
	int	dirlen, len, nnlen ;
	int	f_changed = FALSE ;

	char	newname[MAXPATHLEN + 1] ;
	char	*nnp ;
	char	*bnp ;


#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("checkname: entered name=\"%s\"\n",name) ;
#endif

	if (sbp->st_ctime == 0)
		return 1 ;

	len = sfbasename(name,-1,&bnp) ;

	if ((len > 0) && (bnp[0] == '.'))
		return 0 ;

/* if this is a file link, see if it is a directory */

	rs = OK ;
	if (S_ISLNK(sbp->st_mode)) {

	    sbp = &sb2 ;
	    rs = u_stat(name,&sb2) ;

	}

/* if this is a directory or dangling link, skip it */

	    if ((rs < 0) || S_ISDIR(sbp->st_mode))
	        return 0 ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("checkname: name=\"%s\"\n",name) ;
#endif

/* check if it has a suffix already */

	if (gp->f.suffix) {

		char	*cp ;


		if ((cp = strrchr(name,'.')) == NULL)
			return 0 ;

		if (paramopt_findvalue(pp,SUFFIX,(cp + 1),-1,NULL) < 0)
			return 0 ;

	} /* end if (restricting to specified suffixes) */


	addsize(gp,sbp->st_size) ;


	return 0 ;
}
/* end subroutine (checkname) */


/* add up the size of this file to the total */
static int addsize(gp,size)
struct global	*gp ;
size_t		size ;
{
	long	bytes ;


	bytes = gp->bytes ;
	bytes += (size % MEGABYTE) ;
	gp->bytes = (bytes % MEGABYTE) ;

	gp->megabytes += (bytes / MEGABYTE) ;
	gp->megabytes += (size / MEGABYTE) ;


}
/* end subroutine (addsize) */



