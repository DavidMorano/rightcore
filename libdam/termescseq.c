/* termescseq */

/* Terminal Control Sequence (make them) */


#define	CF_DEBUGS	0		/* compile-time */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This object package is finally finished!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine formulates a simple string that constitutes a terminal
        command sequence (of characters).


*******************************************************************************/


#define	TD_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	ctdecui(char *,int,uint) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int termescseq(char *dp,int dl,int final,int a1,int a2,int a3,int a4)
{
	int		rs = SR_OK ;
	int		ai ;
	int		a ;
	int		i = 0 ;

	if (dp == NULL) return SR_FAULT ;

	if (dl < 0) dl = INT_MAX ;

#if	CF_DEBUGS
	debugprintf("termescseq: f=%c(%02x) a1=%c(%02x)\n",
		final,final,a1,a1) ;
#endif

	if (final > 0) {

	if (rs >= 0) {
	    rs = storebuf_char(dp,dl,i,'\033') ;
	    i += rs ;
	}

	for (ai = 1 ; (rs >= 0) && (ai <= 4) ; ai += 1) {
	    a = -1 ; /* superfluous! */
	    switch (ai) {
	    case 1: 
	        a = a1 ; 
	        break ;
	    case 2: 
	        a = a2 ; 
	        break ;
	    case 3: 
	        a = a3 ; 
	        break ;
	    case 4: 
	        a = a4 ; 
	        break ;
	    } /* end switch */
	    if (a >= 0) {
	        if ((a >= 0x20) && (a < 0x2f)) {
		    rs = storebuf_char(dp,dl,i,a) ;
		    i += rs ;
	        } else
		    rs = SR_ILSEQ ;
	    }
	} /* end for */

	if (rs >= 0) {
	    if ((final >= 0x30) && (final <= 0x7e)) {
	        rs = storebuf_char(dp,dl,i,final) ;
	        i += rs ;
	    } else
		rs = SR_ILSEQ ;
	} /* end if */

	} else
	    rs = SR_ILSEQ ;

#if	CF_DEBUGS
	debugprintf("termescseq: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (termescseq) */



