/* termconseq */

/* Terminal Control Sequence */


#define	CF_DEBUGS	0		/* compile-time */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This object package is finally finished!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine formulates a simple string that constitutes a terminal
        command sequence (of characters).

	Synopsis:

	int termconseq(dp,dl,name,a1,a2,a3,a4)
	char		*dp ;
	int		dl ;
	int		name ;
	int		a1 ;
	int		a2 ;
	int		a3 ;
	int		a4 ;

	Arguments:

	dp		result buffer
	dl		result buffer length
	name		control sequence name
	a[1-4]		control sequence arguments

	Returns:

	<0		error
	>=0		length of resulting string


*******************************************************************************/


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


int termconseq(char *dp,int dl,int name,int a1,int a2,int a3,int a4)
{
	int		rs = SR_OK ;
	int		ai ;
	int		a ;
	int		c = 0 ;
	int		i = 0 ;

	if (dp == NULL) return SR_FAULT ;

	if (dl < 0) dl = INT_MAX ;

	if (name != 0) {

	    if (rs >= 0) {
	        const char *sp = "\033[" ;
	        rs = storebuf_strw(dp,dl,i,sp,2) ;
	        i += rs ;
	    }

	    for (ai = 1 ; (rs >= 0) && (ai <= 4) ; ai += 1) {
	        a = -1 ;
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
	            if ((rs >= 0) && (c > 0)) {
	                rs = storebuf_char(dp,dl,i,';') ;
	                i += rs ;
	            }
	            if (rs >= 0) {
	                rs = storebuf_deci(dp,dl,i,a) ;
	                i += rs ;
	            }
	            c += 1 ;
	        }
	    } /* end for */

	    if (rs >= 0) {
	        rs = storebuf_char(dp,dl,i,name) ;
	        i += rs ;
	    }

	} else
	    rs = SR_INVALID ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (termconseq) */


