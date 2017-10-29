/* getsystypenum */

/* get the 'ostype' and 'osnum' for the current system and release */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */


/* revision history:

	= 2001-04-11, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We find (or make) a system type-name from the combination of the system
        'sysname" and 'release'.

	Synopsis:

	int getsystypenum(rbuf,nbuf,sysname,release)
	char		tbuf[] ;
	char		nbuf[] ;
	const char	sysname[] ;
	const char	release[] ;

	Arguments:

	tbuf		buffer for 'ostype' result
	nbuf		buffer for 'osnum' result
	sysname		specified SYSNAME string
	release		specified RELEASE string

	Returns:

	<0		error (generally SR_NOTFOUND)
	>=0		OK


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;


/* local structures */


/* forward references */

static int	getfield(const char *,int,const char **) ;


/* local variables */

static cchar	*sysnames[] = {
	"SunOS",
	"Darwin",
	NULL
} ;

enum sysnames {
	sysname_sunos,
	sysname_darwin,
	sysname_overlast
} ;


/* exported subroutines */


int getsystypenum(char *tbuf,char *nbuf,cchar *sysname,cchar *release)
{
	const int	olen = USERNAMELEN ;
	int		rs = SR_OK ;
	int		si ;

	if (tbuf == NULL) return SR_FAULT ;
	if (nbuf == NULL) return SR_FAULT ;
	if (sysname == NULL) return SR_FAULT ;
	if (release == NULL) return SR_FAULT ;

	if (sysname[0] == '\0') return SR_INVALID ;
	if (release[0] == '\0') return SR_INVALID ;

	tbuf[0] = '\0' ;
	nbuf[0] = '\0' ;
	if ((si = matstr(sysnames,sysname,-1)) >= 0) {
	    int		cl ;
	    cchar	*ostype ;
	    cchar	*cp ;
	    rs = SR_NOTFOUND ;
	    switch (si) {
	    case sysname_sunos:
		ostype = "SYSV" ;
		if ((cl = getfield(release,1,&cp)) > 0) {
		    rs = snwcpy(nbuf,olen,cp,cl) ;
		}
		break ;
	    case sysname_darwin:
		ostype = "BSD" ;
		if ((cl = getfield(release,0,&cp)) > 0) {
		    rs = snwcpy(nbuf,olen,cp,cl) ;
		}
		break ;
	    } /* end switch */
	    if (rs >= 0) {
		rs = sncpy1(tbuf,olen,ostype) ;
	    }
	} else {
	    rs = SR_NOTFOUND ;
	}

#if	CF_DEBUGS
	debugprintf("getsystypenum: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (getsystypenum) */


/* local subroutines */


static int getfield(cchar *sp,int n,cchar **rpp)
{
	int		i = 0 ;
	int		cl = -1 ;
	const char	*tp ;
	const char	*cp = NULL ;

	while ((tp = strchr(sp,'.')) != NULL) {
	    if (i == n) {
		cp = sp ;
		cl = (tp-sp) ;
		break ;
	    }
	    sp = (tp+1) ;
	    i += 1 ;
	} /* end if */

	if ((cp == NULL) && sp[0] && (i == n)) {
	    cp = sp ;
	    cl = strlen(sp) ;
	}

	if (rpp != NULL)
	    *rpp = cp ;

	return cl ;
}
/* end subroutine (getfield) */


