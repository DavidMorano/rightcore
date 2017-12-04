/* checkdirs */

/* check that directories are present */
/* version %I% last modified %G% */


#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 1991-09-01, David A­D­ Morano
	This subroutine was adopted from the DWD program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine checks for the existence of a directory and creates it
        if it is not present.

	Synopsis:

	int checkdirs(pip,dname,dnamelen,mode)
	struct proginfo	*pip ;
	char	dname[] ;
	int	dnamelen ;
	int	mode ;

	Arguments:

	pip		program information pointer
	dname		specified buffer that has the directory path
	dnamelen	length of passed directory path
	mode		mode of the directory if it needs to be created

	Returns:

	>0		number of directories that needed to be created
	==0		all directories existed
	<0		error

	Notes:

        This subroutine assumes that the buffer passed by the caller has one
        more byte in it (whatever its contents) than the length that is passed
        by the caller.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	IPCDIRMODE	0777


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkdirs(const char *,mode_t) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */


/* exported subroutines */


/* check if a directory exists and has the correct permissions */
int checkdirs(pip,dname,dnamelen,om)
struct proginfo	*pip ;
char		dname[] ;
int		dnamelen ;
mode_t		om ;
{
	int	rs ;
	int	bl ;

	char	buf[MAXPATHLEN + 1], *fn = dname ;


	if ((dnamelen >= 0) && (dname[dnamelen] != '\0')) {
		fn = buf ;
		bl = MIN(dnamelen,(MAXPATHLEN - 1)) ;
		strwcpy(buf,dname,bl) ;
	}

	rs = mkdirs(fn,om) ;

	return rs ;
}
/* end subroutine (checkdirs) */



