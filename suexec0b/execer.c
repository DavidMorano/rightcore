/* execer */

/* subroutine to exec a program */
/* last modified %G% version %I% */


#define	CF_DEBUG	1		/* compile-time */


/* revision history:

	= 1999-03-01, Dave morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This subroutine 'exec(2)'s a program.


	Synopsis:

	int execer(pip,sbp,interpreter,shallarg,progfname,argc,argv) ;
	struct proginfo	*pip ;
	struct ustat	*sbp ;
	char	interpreter[] ;
	char	interpretarg[] ;
	char	progfname[] ;
	int	argc ;
	char	*argv[] ;

	Arguments:

	pip		program information pointer
	sbp		pointer to a stat(2) struct of PWD
	interpretpath	SHELL program file path
	interpretarg	SHELL argument (if any)
	progfname	program file path
	argc		number of arguments
	argv		array of arguments of character strings

	Returns:

	<0		exec(2) failed
	else		doesn't return


**************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	findfilepath(const char *,char *,const char *,int) ;


/* external variables */


/* exported subroutines */


int execer(pip,sbp,interpreter,interpretarg,progfname,argc,argv,envv)
struct proginfo	*pip ;
struct ustat	*sbp ;
char		interpreter[], interpretarg[] ;
char		progfname[] ;
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	int	rs ;
	int	i, j ;
	int	size ;

	char	interpretpath[MAXPATHLEN + 1] ;
	char	filebuf[MAXNAMELEN + 1] ;
	char	*ip ;
	char	**sargv ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("execer: interpreter=%s interpretarg=%s\n",
	        interpreter,interpretarg) ;
	    debugprintf("execer: progfname=%s argc=%d\n",
	        progfname,argc) ;
	}
#endif /* CF_DEBUG */

/* find the shell program file itself */

	rs = findfilepath(NULL,interpretpath,interpreter,X_OK) ;
	if (rs < 0)
	    return rs ;

	if ((rs == 0) && (interpretpath[0] == '\0'))
		strwcpy(interpretpath,interpreter,MAXPATHLEN) ;

	ip = interpretpath ;
	rs = currentdir(sbp,interpretpath,filebuf) ;

	if (rs > 0)
		ip = filebuf ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("execer: interpretpath=%s\n",
	        ip) ;
#endif

	size = (argc + 3) * sizeof(char *) ;
	rs = uc_malloc(size,&sargv) ;

	if (rs < 0)
	    return rs ;

	i = 0 ;
	sargv[i++] = argv[0] ;
	if ((interpretarg != NULL) && (interpretarg[0] != '\0'))
	    sargv[i++] = interpretarg ;

	sargv[i++] = progfname ;
	for (j = 1 ; j < argc ; j += 1)
	    sargv[i++] = argv[j] ;

	sargv[i] = NULL ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    for (j = 0 ; sargv[j] != NULL ; j += 1)
	        debugprintf("execer: sargv[%d]=%s\n",j,sargv[j]) ;
	}
#endif /* CF_DEBUG */

/* do it ! */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("execer: ultimate EUID=%d EGID=%d\n",
	        u_geteuid(),u_getegid()) ;
#endif

	rs = u_execve(ip,sargv,envv) ;

	return rs ;
}
/* end subroutine (execer) */


