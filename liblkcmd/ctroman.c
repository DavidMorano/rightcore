/* ctroman */

/* subroutines to convert an integer to a Roman-Numeral representation */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

        These subroutines convert an integer (signed or unsigned) into a Roman
	Numeral representation.

	Synopsis:

	int ctromanXX(rbuf,rlen,v)
	char		rbuf[] ;
	int		rlen ;
	int		v ;

	Arguments:

	rbuf		caller supplied buffer
	rlen		caller supplied buffer length
	v		integer value to be converted

	Returns:

	>=0		length of buffer used by the conversion
	<0		error in the conversion


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<localmisc.h>


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif


/* local structures */


/* forward references */

int		ctromani(char *,int,int) ;

static int	ictroman(char *,int,ulonglong) ;


/* local variables */

static cchar	*hundreds[] = {
	"",
	"C",
	"CC",
	"CCC",
	"CD",
	"D",
	"DC",
	"DCC",
	"DCCC",
	"CM"
} ;

static cchar	*tens[] = {
	"",
	"X",
	"XX",
	"XXX",
	"XL",
	"L",
	"LX",
	"LXX",
	"LXXX",
	"XC"
} ;

static cchar	*ones[] = {
	"",
	"I",
	"II",
	"III",
	"IV",
	"V",
	"VI",
	"VII",
	"VIII",
	"IX"
} ;


/* exported subroutines */


int ctroman(char *dbuf,int dlen,int v)
{
	return ctromani(dbuf,dlen,v) ;
}
/* end subroutine (ctroman) */


int ctromani(char *dbuf,int dlen,int v)
{
	ulonglong	ulv = (ulonglong) v ;
	return ictroman(dbuf,dlen,ulv) ;
}
/* end subroutine (ctromani) */


int ctromanl(char *dbuf,int dlen,long v)
{
	ulonglong	ulv = (ulonglong) v ;
	return ictroman(dbuf,dlen,ulv) ;
}
/* end subroutine (ctromanl) */


int ctromanll(char *dbuf,int dlen,longlong v)
{
	ulonglong	ulv = (longlong) v ;
	return ictroman(dbuf,dlen,ulv) ;
}
/* end subroutine (ctromanll) */


/* unsigned */
int ctromanui(char *dbuf,int dlen,uint v)
{
	ulonglong	ulv = (ulonglong) v ;
	return ictroman(dbuf,dlen,ulv) ;
}
/* end subroutine (ctromanui) */


/* unsigned */
int ctromanul(char *dbuf,int dlen,ulong v)
{
	ulonglong	ulv = (ulonglong) v ;
	return ictroman(dbuf,dlen,ulv) ;
}
/* end subroutine (ctromanul) */


/* unsigned */
int ctromanull(char *dbuf,int dlen,ulonglong v)
{
	ulonglong	ulv = (longlong) v ;
	return ictroman(dbuf,dlen,ulv) ;
}
/* end subroutine (ctromanull) */


/* local subroutines */


static int ictroman(char *dbuf,int dlen,ulonglong v)
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;
	cchar		**tabs[] = { hundreds, tens, ones } ;
	if ((rs = sbuf_start(&b,dbuf,dlen)) >= 0) {
	    const int	ntabs = nelem(tabs) ;
	    int		n = 1000 ;
	    int		r ;
	    if (v >= n) {
	        const int	i = (v/n) ;
	        rs = sbuf_nchar(&b,i,'M') ;
	        v = (v%n) ;
	    }
	    n /= 10 ;
	    for (r = 0 ; (rs >= 0) && (r < ntabs) ; r += 1) {
	        if (v >= n) {
	            const int	i = (v/n) ;
	            rs = sbuf_strw(&b,tabs[r][i],-1) ;
	            v = (v%n) ;
	        }
	        n /= 10 ;
	    } /* end for */
	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */
	return rs ;
}
/* end subroutine (ictroman) */


