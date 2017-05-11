/* dialprog */

/* connect to a local program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_ENVSORT	0		/* sort the environment? */
#define	CF_MKVARPATH	0		/* somehow use 'mkvarpath(3dam)' */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a dialer to connect to a local program.

	Synopsis:

	int dialprog(fname,of,argv,envv,fd2p)
	const char	fname[] ;
	int		of ;
	char		*argv[] ;
	char		*envv[] ;
	int		*fd2p ;

	Arguments:

	fname		program to execute
	of		open-flags
	argv		arguments to program
	envv		environment to program
	fd2p		pointer to integer to receive STDERR descriptor

	Returns:

	>=0		file descriptor to program STDIN and STDOUT
	<0		error

	fd2p		if it was supplied, the pointed-to integer
			received a file descriptor to the STDERR


	Importand note on debugging:

	One some (maybe many) OS systems, turning on any debugging in this
	subroutine can cause hangs after the 'fork(2)'.  This is due to the
	famous (infamous) fork-safety problem on many UNIX®i®.  One UNIX® OS
	that has fork-safe lib-C subroutines (for the greater most part) is
	Solaris®.  They (the Solaris® people) seem to be among the only ones
	who took fork-safety seriously in their OS.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	uc_openprogerr(cchar *,int,cchar **,cchar **,int *) ;

#if	CF_DEBUGS
extern int	nprintf(const char *,const char *,...) ;
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* forward reference */


/* local variables */


/* exported subroutines */


int dialprog(cchar *fname,int of,cchar **argv,cchar **envv,int *fd2p)
{
	return uc_openprogerr(fname,of,argv,envv,fd2p) ;
}
/* end subroutine (dialprog) */


