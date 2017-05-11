/* termcharset */

/* set the terminal character sets */


#define	CF_DEBUGS	0		/* compile-time */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This object package is finally finished!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

        This subroutine formulates a string that constitutes a terminal escape
        sequence to set the terminal character sets.

	Synopsis:

	int termcharset(dp,dl,setnum,f96,fontname)
	char		*dp ;
	int		dl ;
	int		setnum ;
	int		f96 ;
	const char	*fontname ;

	Arguments:

	dp		destination buffer pointer
	dl		destination buffer length
	setnum		number of set to set:
				0	G0
				1	G1
				2	G2
				3	G3
	f96		flag (TRUE or FALSE) on whether specified set is a
			96-character character set
	fontname	string representing font to have set in place

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#define	TERMCHARSET_MASTER	0


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

static const int	inter[4][2] = {
	{ '\050', 0 },			/* G0 */
	{ '\051', '-' },		/* G1 */
	{ '*', '.' },			/* G2 */
	{ '+', '/' }			/* G3 */
} ; /* intermediate characters */


/* exported subroutines */


int termcharset(char *dp,int dl,int setnum,int f96,cchar *fontname)
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (dp == NULL) return SR_FAULT ;
	if (fontname == NULL) return SR_FAULT ;

	if (dl < 0) dl = INT_MAX ;

	if ((setnum < 0) || (setnum >= 4)) return SR_INVALID ;

	if (f96 != 0) f96 = 1 ;		/* make safe as an array index */

/* setting a 96-character set to G0 is invalid */

	if ((setnum == 0) && f96) return SR_INVALID ;

/* construct escape sequence */

	if (rs >= 0) {
	    cchar	*sp = "\033" ;
	    rs = storebuf_strw(dp,dl,i,sp,1) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    int		ich = inter[setnum][f96] ;
	    rs = storebuf_char(dp,dl,i,ich) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(dp,dl,i,fontname,-1) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (termcharset) */


