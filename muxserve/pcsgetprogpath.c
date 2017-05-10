/* pcsgetprogpath */

/* get the path to a program that is used within the PCS system */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used to find a PCS related program and to verify that
        it is executable.

	Important:

        This subroutine is different from 'pcsgetprog(3pcs)' in that this will
        return a full path of the found program whenever it is different than
        what was supplied. In contrast, the 'pcsgetprog(3pcs)' subroutine only
        returns the full path of the found program when it is not absolute and
        it is found in the PCS distribution.

	Synopsis:

	int pcsgetprogpath(pcsroot,programpath,name)
	const char	pcsroot[] ;
	char		programpath[] ;
	const char	name[] ;

	Arguments:

	pcsroot		PCS program root path
	programpath	returned file path if not the same as input
	name		program to find

	Returns:

	>0		found the program path and this is the length
	==0		program was found w/o any path prefix
	<0		program was not found

	programpath	returned file path if it was not in the PWD


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<ids.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	prgetprogpath(const char *,char *,const char *,int) ;

#if	CF_DEBUGS
extern int	strnnlen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int pcsgetprogpath(cchar *pr,char *rbuf,cchar *np)
{
	return prgetprogpath(pr,rbuf,np,-1) ;
}
/* end subroutine (pcsgetprogpath) */


