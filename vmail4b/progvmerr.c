/* progvmerr */

/* Program-Error */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */
#define	CF_DEBUG	0		/* switchable print-outs */


/* revision history:

	= 2008-09-01, David A­D­ Morano
	This subroutine was borrowed and modified from previous generic
	front-end 'main' subroutines!

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Here we do some multi-threaded error printing.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<format.h>
#include	<localmisc.h>

#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */

int	progvmerr_vprintf(PROGINFO *,cchar *,va_list) ;
int	progvmerr_print(PROGINFO *,cchar *,int) ;


/* local variables */


/* exported subroutines */


int progvmerr_printf(PROGINFO *pip,cchar *fmt,...)
{
	int		rs ;

	if (fmt == NULL) return SR_FAULT ;

	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = progvmerr_vprintf(pip,fmt,ap) ;
	    va_end(ap) ;
	}

	return rs ;
}
/* end subroutine (progvmerr_printf) */


/* vprintf-like thing */
int progvmerr_vprintf(PROGINFO *pip,cchar *fmt,va_list ap)
{
	int		rs ;
	int		wlen = 0 ;

	if (fmt == NULL) return SR_FAULT ;

	if (pip->efp != NULL) {
	    const int	flen = LINEBUFLEN ;
	    char	fbuf[LINEBUFLEN+1] ;
	    if ((rs = format(fbuf,flen,0x01,fmt,ap)) >= 0) {
	        rs = progvmerr_print(pip,fbuf,rs) ;
	        wlen = rs ;
	    }
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progvmerr_vprintf) */


int progvmerr_print(PROGINFO *pip,cchar *sp,int sl)
{
	int		rs ;
	if (pip == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (pip->efp != NULL) {
	    rs = bprintline(pip->efp,sp,sl) ;
	}
	return rs ;
}
/* end subroutine (progvmerr_print) */


