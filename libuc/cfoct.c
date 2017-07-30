/* cfoct */

/* convert an octal digit string to its binary integer value */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-10-01, David A­D­ Morano
	This subroutine was written by being adapted from a previous version of
	the same, which itself was adapted from an original asembly-language
	version.

	= 2013-04-30, David A­D­ Morano
	I took the plunge and rewrote this using the LIBC subroutines
	'strtoXX(3c)'.  It is no longer stand-alone, like in the old days, but
	we have been mostly on UNIX®i for some time now (decades) and use in
	non-UNIX®i environments is now quite rare.  I hope that this is not a
	problem.  We will see.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Subroutines to convert digit strings to binary integers.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	TBUFLEN
#define	TBUFLEN		45		/* can hold int128_t in decimal */
#endif

#undef	OURBASE
#define	OURBASE		8


/* external subroutines */

extern int	snwcpyshrink(char *,int,cchar *,int) ;
extern int	checkbase(cchar *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

int		cfocti(cchar *,int,int *) ;


/* local variables */


/* exported subroutines */


int cfoct(cchar *sp,int sl,int *rp)
{

	return cfocti(sp,sl,rp) ;
}
/* end subroutine (cfoct) */


/* convert from an octal string to a signed integer */
int cfocti(cchar *sp,int sl,int *rp)
{
	int		rs = SR_OK ;
	char		tbuf[TBUFLEN+1] ;

	if ((sl >= 0) && (sp[sl] != '\0')) {
	    rs = snwcpyshrink(tbuf,TBUFLEN,sp,sl) ;
	    sl = rs ;
	    sp = tbuf ;
	}

	if (rs >= 0) {
	    if ((rs = checkbase(sp,sl,OURBASE)) >= 0) {
		if (rp != NULL) {
	    	    rs = uc_strtoi(sp,NULL,OURBASE,rp) ;
		}
	    }
	}

	return rs ;
}
/* end subroutine (cfocti) */


/* convert from an octal string to a unsigned integer */
int cfoctui(char *sp,int sl,uint *rp)
{
	int		rs = SR_OK ;
	char		tbuf[TBUFLEN+1] ;

	if ((sl >= 0) && (sp[sl] != '\0')) {
	    rs = snwcpyshrink(tbuf,TBUFLEN,sp,sl) ;
	    sl = rs ;
	    sp = tbuf ;
	}

	if (rs >= 0) {
	    if ((rs = checkbase(sp,sl,OURBASE)) >= 0) {
		if (rp != NULL) {
	    	    rs = uc_strtoui(sp,NULL,OURBASE,rp) ;
		}
	    }
	}

	return rs ;
}
/* end subroutine (cfoctui) */


/* convert from an octal string to a signed long integer */
int cfoctl(cchar *sp,int sl,long *rp)
{
	int		rs = SR_OK ;
	char		tbuf[TBUFLEN+1] ;

	if ((sl >= 0) && (sp[sl] != '\0')) {
	    rs = snwcpyshrink(tbuf,TBUFLEN,sp,sl) ;
	    sl = rs ;
	    sp = tbuf ;
	}

	if (rs >= 0) {
	    if ((rs = checkbase(sp,sl,OURBASE)) >= 0) {
		if (rp != NULL) {
	    	    rs = uc_strtol(sp,NULL,OURBASE,rp) ;
		}
	    }
	}

	return rs ;
}
/* end subroutine (cfoctl) */


/* convert from an octal string to a unsigned long integer */
int cfoctul(cchar *sp,int sl,ulong *rp)
{
	int		rs = SR_OK ;
	char		tbuf[TBUFLEN+1] ;

	if ((sl >= 0) && (sp[sl] != '\0')) {
	    rs = snwcpyshrink(tbuf,TBUFLEN,sp,sl) ;
	    sl = rs ;
	    sp = tbuf ;
	}

	if (rs >= 0) {
	    if ((rs = checkbase(sp,sl,OURBASE)) >= 0) {
		if (rp != NULL) {
	    	    rs = uc_strtoul(sp,NULL,OURBASE,rp) ;
		}
	    }
	}

	return rs ;
}
/* end subroutine (cfoctul) */


/* convert from an octal string to a 64-bit signed LONG integer */
int cfoctll(cchar *sp,int sl,longlong *rp)
{
	int		rs = SR_OK ;
	char		tbuf[TBUFLEN+1] ;

	if ((sl >= 0) && (sp[sl] != '\0')) {
	    rs = snwcpyshrink(tbuf,TBUFLEN,sp,sl) ;
	    sl = rs ;
	    sp = tbuf ;
	}

	if (rs >= 0) {
	    if ((rs = checkbase(sp,sl,OURBASE)) >= 0) {
		if (rp != NULL) {
	    	    rs = uc_strtoll(sp,NULL,OURBASE,rp) ;
		}
	    }
	}

	return rs ;
}
/* end subroutine (cfoctll) */


/* convert from an octal string to a 64-bit unsigned LONG integer */
int cfoctull(cchar *sp,int sl,ulonglong *rp)
{
	int		rs = SR_OK ;
	char		tbuf[TBUFLEN+1] ;

	if ((sl >= 0) && (sp[sl] != '\0')) {
	    rs = snwcpyshrink(tbuf,TBUFLEN,sp,sl) ;
	    sl = rs ;
	    sp = tbuf ;
	}

	if (rs >= 0) {
	    if ((rs = checkbase(sp,sl,OURBASE)) >= 0) {
		if (rp != NULL) {
	    	    rs = uc_strtoull(sp,NULL,OURBASE,rp) ;
		}
	    }
	}

	return rs ;
}
/* end subroutine (cfoctull) */


