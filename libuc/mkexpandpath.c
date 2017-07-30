/* mkexpandpath */

/* make an expanded path */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine creates a resolved filename path from the coded form.

	Synopsis:

	int mkexpandpath(rbuf,pp,pl)
	char		rbuf[] ;
	const char	*pp ;
	int		pl ;

	Arguments:

	rbuf		result buffer (should be MAXPATHLEN+1 long)
	pp		source path pointer
	pl		source path length

	Returns:

	<0		error
	==0		no expansion
	>0		expansion


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	mkvarpath(char *,const char *,int) ;
extern int	mkuserpath(char *,const char *,const char *,int) ;
extern int	mkcdpath(char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkexpandpath(char *rbuf,cchar *pp,int pl)
{
	int		rs ;

	if (rbuf == NULL) return SR_FAULT ;
	if (pp == NULL) return SR_FAULT ;

	if (pl < 0) pl = strlen(pp) ;

#if	CF_DEBUGS
	debugprintf("mkexpandpath: ent p=%t\n",pp,pl) ;
#endif

	rbuf[0] = '\0' ;
	if ((rs = mkuserpath(rbuf,NULL,pp,pl)) == 0) {
	    if ((rs = mkvarpath(rbuf,pp,pl)) == 0) {
	        rs = mkcdpath(rbuf,pp,pl) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("mkexpandpath: ret rs=%d\n",rs) ;
	debugprintf("mkexpandpath: ret rbuf=%s\n",rbuf) ;
#endif

	return rs ;
}
/* end subroutine (mkexpandpath) */


