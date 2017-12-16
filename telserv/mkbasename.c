/* mkbasename */

/* make the filename for a shared library (shared object) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time print-outs */


/* revision history:

	= 2008-07-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine formulates (makes) the base-name part of a file-name
	into the result buffer.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	mkpath3w(char *,const char *,const char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int mkbasename(char *rbuf,cchar *pnp,int pnl)
{
	int		cl ;
	int		len ;
	cchar		*cp ;

	if ((cl = sfbasename(pnp,pnl,&cp)) > 0) {
	    const int	ml = MIN(MAXNAMELEN,cl) ;
	    len = strwcpy(rbuf,cp,ml) - rbuf ;
	}

	return len ;
}
/* end subroutine (mkbasename) */


