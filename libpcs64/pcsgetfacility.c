/* pcsgetfacility */

/* get the facility name for PCS */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEFPCS	1		/* try a default PCS program-root */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine retrieves the facility string for PCS.

	Synopsis:

	int pcsgetfacility(pr,rbuf,rlen)
	const char	pr[] ;
	char		rbuf[] ;
	int		rlen ;

	Arguments:

	pr		PCS system program root (if available)
	rbuf		buffer to hold result
	rlen		length of supplied result buffer

	Returns:

	>=0		OK
	<0		some error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<project.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<char.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<pcsgetnames.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARPRPCS
#define	VARPRPCS	"PCS"
#endif

#ifndef	PCSFACILITY
#define	PCSFACILITY	"Personal Communication Services" ;
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mknpath2(char *,int,const char *,const char *) ;
extern int	matkeystr(const char **,char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	getgecosname(const char *,int,const char **) ;
extern int	mkgecosname(char *,int,const char *) ;
extern int	mkrealname(char *,int,const char *,int) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	hasalldig(const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strdcpy1(char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const int	nots[] = {
	SR_DOM,
	SR_ACCESS,
	SR_NOTFOUND,
	SR_OVERFLOW,
	SR_NAMETOOLONG,
	SR_RANGE,
	0
} ;


/* exported subroutines */


int pcsgetfacility(cchar *pr,char *rbuf,int rlen)
{
	const int	nt = pcsnametype_fullname ;
	const int	ulen = USERNAMELEN ;
	int		rs ;
	const char	*un = VARPRPCS ;
	char		ubuf[USERNAMELEN+1] ;

	rbuf[0] = '\0' ;
	strwcpylc(ubuf,un,ulen) ; /* get lower-case */

	if ((rs = pcsgetnames(pr,rbuf,rlen,ubuf,nt)) >= 0) {
	    if (rs == 0) rs = strlen(rbuf) ;
	} else if (isOneOf(nots,rs)) {
	    const char	*facility = PCSFACILITY ;
	    rs = (strdcpy1(rbuf,rlen,facility) - rbuf) ;
	}

	return rs ;
}
/* end subroutine (pcsgetfacility) */


