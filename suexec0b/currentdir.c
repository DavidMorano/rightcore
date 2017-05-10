/* currentdir */

/* does the current path contain the current working directory */


#define	CF_DEBUGS	0


/* revision history:

	= 97/05/01, David A­D­ Morano

	This subroutine was originally written.


	= 98/05/20, David A­D­ Morano

	I modified this subroutine to treat an empty path string to be
	the current directory.  If it matches one of the directories
	that has the specified file, then the expanded directory string
	is added to the list (if a list is requested).


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Description :

	This subroutine checks to see if the given file path contains
	the current working directory.

	We actually use the present working directory (PWD) instead
	because that is what most people expect now-a-days with
	all of these virtual reality paths floating around !


	Synopsis:

	int currentdir(sbp,filepath,file)
	struct ustat	*sbp ;
	char	filepath[] ;
	char	file[] ;


	Arguments:

	sbp		pointer to 'stat(5)' structure of 
			the current working directory
	filepath	input file path to check
	file		basename of file to be searched for


	Returns:

	>=0	the length of the revised file
	<0	some error



******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sfdirname(const char *,int,const char **) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* forward references */


/* exported subroutines */


int currentdir(sbp,filepath,file)
struct ustat	*sbp ;
char		filepath[] ;
char		file[] ;
{
	struct ustat	sb1, sb2 ;

	int	rs, len ;
	int	dirlen ;

	char	dirbuf[MAXPATHLEN + 1] ;
	char	*cp ;


	if (filepath == NULL)
	    return SR_FAULT ;

	if (sbp == NULL) {

#if	CF_DEBUGS
	    debugprintf("currentdir: need a stat()\n") ;
#endif

	    if ((rs = u_stat(".",&sb2)) < 0)
	        return rs ;

	    sbp = &sb2 ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("currentdir: PWD dev=%d inode=%d\n",
	    sbp->st_dev,sbp->st_ino) ;
#endif

	if ((dirlen = sfdirname(filepath,-1,&cp)) <= 0)
	    return SR_NOTDIR ;

	strwcpy(dirbuf,cp,dirlen) ;

	if ((rs = u_stat(dirbuf,&sb1)) < 0)
	    return rs ;

#if	CF_DEBUGS
	debugprintf("currentdir: filepath dev=%d inode=%d\n",
	    sb1.st_dev,sb1.st_ino) ;
#endif

	if ((sb1.st_dev != sbp->st_dev) || (sb1.st_ino != sbp->st_ino))
	    return 0 ;

	cp = filepath + dirlen ;
	while (*cp && (*cp == '/'))
	    cp += 1 ;

	if (file != NULL)
	    strcpy(file,cp) ;

	rs = strlen(cp) ;

	return rs ;
}
/* end subroutine (currentdir) */



