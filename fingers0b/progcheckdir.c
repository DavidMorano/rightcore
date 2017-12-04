/* progcheckdir */

/* check that directories are present */
/* version %I% last modified %G% */


#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 2008-09-01, David A­D­ Morano
	This subroutine was adopted from the DWD program.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine checks for the existence of a directory and creates it
        if it is not present.

	Synopsis:

	int progcheckdir(pip,dname,dnamel,mode)
	struct proginfo	*pip ;
	char	dname[] ;
	int	dnamel ;
	int	mode ;

	Arguments:

	pip		program information pointer
	dname		specified buffer that has the directory path
	dnamel	length of passed directory path
	mode		mode of the directory if it needs to be created

	Returns:

	>0		number of directories that needed to be created
	==0		all directories existed
	<0		error

	Notes:

	This subroutine assumes that the buffer passed by the caller
	has one more byte in it (whatever its contents) than the
	length that is passed by the caller.


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

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkdirs(const char *,mode_t) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */


/* local variables */


/* exported subroutines */


int progcheckdir(pip,dnamep,dnamel,mode)
struct proginfo	*pip ;
const char	dnamep[] ;
int		dnamel ;
mode_t		mode ;
{
	int	rs = SR_OK ;

	const char	*fn ;

	char	fbuf[MAXPATHLEN + 1] ;


	fn = dnamep ;
	if ((dnamel >= 0) && (dnamep[dnamel] != '\0')) {
	    fn = fbuf ;
	    rs = mkpath1w(fbuf,dnamep,dnamel) ;
	}

	if (rs >= 0)
	    rs = mkdirs(fn,mode) ;

	return rs ;
}
/* end subroutine (progcheckdir) */


