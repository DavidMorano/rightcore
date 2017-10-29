/* checkname */

/* check a file name */


#define	F_DEBUG 	1


/* revision history :

	= Dave Morano, March 1996
	This subroutine was originally written.

*/


/***********************************************************************

	Arguments :
	- name		directory entry
	- sbp		'stat' block pointer

	Returns :
	<0		exit directory walk altogether
	0		continue with directory walk as usual
	>0		skip this directory entry in directory walk


***********************************************************************/



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
#include	<sfstr.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINELEN
#define	LINELEN		(MAXPATHLEN + 10)
#endif


/* external subroutines */

extern int	rename() ;		/* UNIX 'rename(2)' */


/* external variables */


/* forward references */

static int	addsize() ;


/* exported subroutines */


int checkname(name,sbp,gp)
char		name[] ;
struct ustat	*sbp ;
struct global	*gp ;
{
	struct ustat	sb2 ;

	int	rs ;
	int	dirlen, len, nnlen ;
	int	f_changed = FALSE ;

	const char	*bnp ;

	char	newname[MAXPATHLEN + 1] ;
	char	*nnp ;


#if	F_DEBUG
	if (gp->debuglevel > 1)
	    eprintf("checkname: entered name=\"%s\"\n",name) ;
#endif

	if (sbp->st_ctime == 0) return 1 ;

	len = sfbasename(name,-1,&bnp) ;

	if ((len > 0) && (bnp[0] == '.')) return 0 ;

/* if this is a file link, see if it is a directory */

	rs = OK ;
	if (S_ISLNK(sbp->st_mode)) {

	    sbp = &sb2 ;
	    rs = u_stat(name,&sb2) ;

	}

/* if this is a directory or dangling link, skip it */

	    if ((rs < 0) || S_ISDIR(sbp->st_mode))
	        return 0 ;

#if	F_DEBUG
	if (gp->debuglevel > 1)
	    eprintf("checkname: name=\"%s\"\n",name) ;
#endif

/* does this file satisfy our name requirements if any */

/* check if it has a suffix already */

	if (gp->suffixlen > 0) {

		char	*cp ;


		if ((cp = strrchr(name,'.')) == NULL)
			return 0 ;

		if (strcmp(cp + 1,gp->suffix) != 0)
			return 0 ;

	}

	addsize(gp,sbp->st_size) ;


	return 0 ;
}
/* end subroutine (checkname) */



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



