/* mkfingerquery */

/* make argument string for FINGER query */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates a string that reqpresent a FINGER query string.
	We handle the so-called "long" flag option and we also handle
	FINGER service-arguments if they are supplied.

	Synopsis:

	int mkfingerquery(qbuf,qlen,f_long,up,av)
	const char	abuf[] ;
	char		qbuf[] ;
	int		f_long ;
	const char	*up ;
	const char	**av ;

	Arguments:

	abuf		shell argument to be quoted
	alen		length of shell argument to be quoted
	f_long		the "long" flag
	up		user-part
	av		service arguments (if any)

	Returns:

	>=0		length of used destination buffer from conversion
	<0		destination buffer was not big enough or other problem


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<localmisc.h>


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sbuf_addquoted(SBUF *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */


/* forward references */


/* local variables */


/* exported subroutines */


int mkfingerquery(char *qbuf,int qlen,int f_long,cchar *up,cchar **av)
{
	SBUF		b ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (qbuf == NULL) return SR_FAULT ;
	if (up == NULL) return SR_FAULT ;

	if (qlen < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("mkfingerquery: u=>%s<\n",up) ;
#endif

	if ((rs = sbuf_start(&b,qbuf,qlen)) >= 0) {
	    rs = sbuf_strw(&b,up,-1) ;
	    if ((rs >= 0) && f_long) {
		sbuf_strw(&b," /W",3) ;
	    }
	    if (av != NULL) {
	        int	i ;
	        for (i = 0 ; (rs >= 0) && (av[i] != NULL) ; i += 1) {
#if	CF_DEBUGS
			debugprintf("mkfingerquery: a[%u]=>%s<\n",av[i]) ;
#endif
	            if ((rs = sbuf_char(&b,' ')) >= 0) {
			rs = sbuf_addquoted(&b,av[i],-1) ;
		    }
	        } /* end for */
	    } /* end if (argument-vector) */
	    if (rs >= 0)
	        sbuf_strw(&b,"\n\r",2) ;
	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

#if	CF_DEBUGS
	debugprintf("mkfingerquery: ret rs=%d len=%u\n",rs,len) ;
	debugprintf("mkfingerquery: qbuf=>%t<\n",qbuf,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkfingerquery) */


