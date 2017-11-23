/* getuid_name */

/* get a UID by looking up the given name */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debugging */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */
#define	CF_GETUSER	0		/* compile in |getuser_uid()| */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This program was originally written.

	= 2017-11-22, David A­D­ Morano
	I finally (finally) removed (compiled out) the global function symbol
	|getuser_uid()|.  Good riddance.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will retrieve a UID by looking up a given name, assumed
        to be a user-name.

	Synopsis:

	int getuid_name(cchar *np,int nl)

	Arguments:

	np		user-name to lookup
	nl		length of name string

	Returns:

	<0		error
	>=0		retrieved UID


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vsystem.h>
#include	<getbufsize.h>
#include	<nulstr.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<cfdec.h>
#include	<localmisc.h>


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */


/* external subroutines */

extern int	hasalldig(cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getuid_name(cchar *np,int nl)
{
	NULSTR		n ;
	int		rs ;
	int		rs1 ;
	int		uid = 0 ;
	cchar		*name ;
	if (np == NULL) return SR_FAULT ;
	if (np[0] == '\0') return SR_INVALID ;
	if ((rs = nulstr_start(&n,np,nl,&name)) >= 0) {
	    struct passwd	pw ;
	    const int		pwlen = getbufsize(getbufsize_pw) ;
	    char		*pwbuf ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
		{
		    rs = GETPW_NAME(&pw,pwbuf,pwlen,name) ;
	            uid = pw.pw_uid ;
		}
	        rs1 = uc_free(pwbuf) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (memory-allocation) */
	    rs1 = nulstr_finish(&n) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (nulstr) */
	return (rs >= 0) ? uid : rs ;
}
/* end subroutine (getuid_name) */


int getuid_user(cchar *np,int nl)
{
	int		rs ;
	if (np == NULL) return SR_FAULT ;
	if (np[0] == '\0') return SR_INVALID ;
	if (hasalldig(np,nl)) {
	    int	v ;
	    if ((rs = cfdeci(np,nl,&v)) >= 0) {
		rs = v ;
	    }
	} else {
	    rs = getuid_name(np,nl) ;
	}
	return rs ;
}
/* end subroutine (getuid_user) */


#if	CF_GETUSER
int getuser_uid(cchar *np,int nl)
{
	return getuid_user(np,nl) ;
}
#endif /* CF_GETUSER */


