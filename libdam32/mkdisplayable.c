/* mkdisplayable */

/* clean up some text string for printing to terminal */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-01-20, David A­D­ Morano
	I couldn't find something like this already so I popped one.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will scan the source data looking for displayable (like
        on a terminal) characters. Displayable characters are transferred to the
        result buffer as space permits.

	Synopsis:

	int mkdisplayable(rbuf,rlen,sbuf,slen)
	const char	sbuf[] ;
	char		rbuf[] ;
	int		rlen, slen ;

	Arguments:

	rbuf	result buffer
	rlen	result buffer length
	sbuf	source buffer
	slen	source buffer length

	Returns:

	>=0	length of transfered characters
	<0	error code


        Implementation note: Remeber that all signed 'char's get promoted to
        stupid signed values on subroutine calls (whether the subroutine
        argument is signed or unsigned)!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<string.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */

#ifdef	LINEBUFLEN
#define	RLEN	LINEBUFLEN
#else
#define	RLEN	2048
#endif


/* external subroutines */

extern int	isprintlatin(int) ;

#if	CF_DEBUGS
extern int	strlinelen(const char *,int,int) ;
#endif


/* forward references */


/* local variables */


/* exported subroutines */


int mkdisplayable(char *rbuf,int rlen,cchar *sbuf,int slen)
{
	uint		pch = 0 ;
	int		f_shift = FALSE ;
	const char	*sp = sbuf, *slast ;
	char		*rp = rbuf, *rlast ;

	if (slen < 0)
	    slen = strlen(sbuf) ;

	if (rlen < 0)
	    rlen = RLEN ;

#if	CF_DEBUGS
	debugprintf("mkdisplayable: s=>%t<\n",
		sbuf,strlinelen(sbuf,slen,40)) ;
#endif

	slast = sbuf + slen ;
	rlast = rbuf + rlen ;
	while ((sp < slast) && *sp && (rp < rlast)) {
	    const uint	ch = MKCHAR(*sp++) ;
	    switch (ch) {
	    case CH_SI:
	    case CH_SO:
	    case CH_SS2:
	    case CH_SS3:
		f_shift = TRUE ;
	        pch = ch ;
		break ;
	    default:
	        if (isprintlatin(ch)) {
		    if (f_shift) {
			f_shift = FALSE ;
	    		*rp++ = pch ;
		    }
	            *rp++ = ch ;
	        } else {
	            *rp++ = ' ' ;
		}
		break ;
	    } /* end switch */
	} /* end while */

	*rp = '\0' ;
	return (rp - rbuf) ;
}
/* end subroutine (mkdisplayable) */


