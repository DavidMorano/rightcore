/* getutmpterm */

/* get the name of the controlling terminal for the current session */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1999-01-10, David A­D­ Morano
        This subroutine was originally written. It was prompted by the failure
        of other system subroutines from finding the proper controlling terminal
        (on Solaris)! Maybe they fixed their stuff -- and maybe they have not
        yet!

	= 2007-10-11, David A­D­ Morano
        I added a little comparison of the result with the device directory
        name. Some other systems (no names mentioned here) sometimes put the
        line-terminal name into the UTMP entry with the device directory name
        prefixed to the line-terminal name. The world is a complicated place!

	= 2010-02-22, David A­D­ Morano
        I added a crap-hack for Darwin (Apple MacOS kernel). Darwin based
        systems -- and many other BSD (cough) systems -- are very screwed up in
        the way that they do things.

*/

/* Copyright © 1998,2007,2010 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will find and return the name of the controlling
        terminal for the given session ID.

	Synopsis:

	int getutmpterm(rbuf,rlen,sid)
	char		rbuf[] ;
	int		rlen ;
	pid_t		sid ;

	Arguments:

	rbuf		user buffer to receive name of controlling terminal
	rlen		length of user supplied buffer
	sid		session ID to find controlling terminal for

	Returns:

	>=0	length of name of controlling terminal
	<0	error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getutmpent.h>
#include	<localmisc.h>


/* local defines */

#ifndef	DEVDNAME
#define	DEVDNAME	"/dev"
#endif

#define	DARWINPREFIX	"tty"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mknpath1(char *,int,const char *) ;
extern int	mknpath2(char *,int,const char *,const char *) ;

extern char	*strwcpy(char *,char *,int) ;


/* forward references */


/* local variables */


/* exported subroutines */


int getutmpterm(char *rbuf,int rlen,pid_t sid)
{
	GETUTMPENT	ute ;
	int		rs ;
	const char	*devdname = DEVDNAME ;

	if (rbuf == NULL) return SR_FAULT ;

	if ((rs = getutmpent(&ute,sid)) >= 0) {

#if	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)
	    {
	        if (strncmp(ute.line,devdname,sizeof(DEVDNAME)) == 0) {
	            rs = mknpath1(rbuf,rlen,ute.line) ;
	        } else if (strchr(ute.line,'/') == NULL) {
	            const char	*dp = DARWINPREFIX ;
	            rs = sncpy4(rbuf,rlen,devdname,"/",dp,ute.line) ;
	        } else {
	            rs = mknpath2(rbuf,rlen,devdname,ute.line) ;
		}
	    }
#else /* Darwin */
	    {
	        if (strncmp(ute.line,devdname,sizeof(DEVDNAME)) == 0) {
	            rs = mknpath1(rbuf,rlen,ute.line) ;
	        } else {
	            rs = mknpath2(rbuf,rlen,devdname,ute.line) ;
		}
	    }
#endif /* Darwin */

	} /* end if (getutmpent) */

	return rs ;
}
/* end subroutine (getutmpterm) */


