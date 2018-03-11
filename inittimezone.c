/* inittimezone */

/* get the default timezone ('TZ') that 'init(1m)' uses */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2002-05-16, David A­D­ Morano
	This subroutine was originally written for use in the PCS facility.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine retrieves (if it exists) the timezone variable value
        (value name is 'TZ') that the 'init(1m)' program sets for its children.

	Synopsis:

	int inittimezone(rbuf,rlen,fname)
	char		rbuf[] ;
	int		rlen ;
	const char	fname[] ;

	Arguments:

	rbuf		buffer to receive result
	rlen		length of supplied buffer to receive result
	fname		the 'init(1m)' configuration filename

	Returns:

	>=0		length of returned data
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* local defines */

#ifndef	DEFINITFNAME
#define	DEFINITFNAME	"/etc/default/init"
#endif

#ifndef	VARTZ
#define	VARTZ		"TZ"
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;

extern int	vecstr_envfile(vecstr *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int inittimezone(char *rbuf,int rlen,cchar *fname)
{
	vecstr		defs ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (rbuf == NULL) return SR_FAULT ;

	if ((rlen >= 0) && (rlen < 1))
	    return SR_OVERFLOW ;

	if (fname == NULL)
	    fname = DEFINITFNAME ;

	rbuf[0] = '\0' ;
	if ((rs = vecstr_start(&defs,20,0)) >= 0) {
	    if ((rs = vecstr_envfile(&defs,fname)) >= 0) {
		cchar	*var = VARTZ ;
		cchar	*sp ;

	        if ((rs = vecstr_finder(&defs,var,vstrkeycmp,&sp)) >= 0) {
	            if ((sp != NULL) && (sp[0] != '\0')) {
			cchar	*tp ;
	                if ((tp = strchr(sp,'=')) != NULL) {
	                    rs = sncpy1(rbuf,rlen,(tp+1)) ;
			    len = rs ;
			}
	            } else
	                rs = SR_NOTFOUND ;
	        } /* end if (found our key-name) */

	    } /* end if (got some variables) */
	    rs1 = vecstr_finish(&defs) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (inittimezone) */


