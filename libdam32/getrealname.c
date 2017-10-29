/* getrealname */

/* try to find the real-name of a user (given a username) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 2003-10-01, David A­D­ Morano
	This was written to consolidate this type of code that was previously
	done separately in different places.  This code only makes sense if
	access to the user PASSWD record is *only* needed to get the GECOS
	"name" field.  If more general access of the user PASSWD record is
	needed, something other than this should be used.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will try to find a real-name given a username.  If just
	queries the PASSWD-GECOS information.

	Synopsis:

	int getrealname(rbuf,rlen,un)
	char		rbuf[] ;
	int		rlen ;
	const char	un[] ;

	Arguments:

	rbuf		supplied buffer to receive the program-root
	rlen		length of supplied buffer
	un		username

	Returns:

	>=0		length of result
	<0		couldn't find and program-root


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<getxusername.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<localmisc.h>


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */
#undef	COMMENT

#ifndef	REALNAMELEN
#define	REALNAMELEN	100		/* real name length */
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mknpath1(char *,int,const char *) ;
extern int	mknpath2(char *,int,const char *,const char *) ;
extern int	mknpath3(char *,int,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	getgecosname(const char *,int,const char **) ;
extern int	mkrealname(char *,int,const char *,int) ;
extern int	haslc(const char *,int) ;
extern int	hasuc(const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int getpwname(struct passwd *,char *,int,const char *) ;


/* local variables */


/* exported subroutines */


int getrealname(char *rbuf,int rlen,cchar *un)
{
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	char		*pwbuf ;

	if (rbuf == NULL) return SR_FAULT ;

	if (rlen < 0) rlen = REALNAMELEN ;

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    if ((rs = getpwname(&pw,pwbuf,pwlen,un)) >= 0) {
	        const char	*gp ;
	        if ((rs = getgecosname(pw.pw_gecos,-1,&gp)) > 0) {
		    rs = mkrealname(rbuf,rlen,gp,rs) ;
	        } /* end if (getgecosname) */
	    } /* end if (getpwname) */
	    uc_free(pwbuf) ;
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (getrealname) */


/* local subroutines */


static int getpwname(struct passwd *pwp,char *pwbuf,int pwlen,const char *un)
{
	int	rs ;
	if ((un != NULL) && (un[0] != '\0') && (un[0] != '-')) {
	    rs = GETPW_NAME(pwp,pwbuf,pwlen,un) ;
	} else {
	    rs = getpwusername(pwp,pwbuf,pwlen,-1) ;
	}
	return rs ;
}
/* end if (getpwname) */


