/* optbool */

/* option-boolean */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Get (devine) a boolean option.

	Synopsis:

	int optbool(sp,sl)
	const char	*sp ;
	int		sl ;

	Arguments:

	sp		pointer to string
	sl		length of string

	Returns:

	<0		error
	0		false
	1		true


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	matocasestr(const char **,int,const char *,int) ;
extern int	matpcasestr(const char **,int,const char *,int) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	molc(int) ;
extern int	isdigitlatin(int) ;


/* local variables */

static const char	*hits[] = {
	"0",
	"1",
	"no",
	"yes",
	"false",
	"true",
	"-",
	"+",
	NULL
} ;


/* exported subroutines */


int optbool(cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		f = TRUE ; /* default when no value given */

	if (sp == NULL) return SR_FAULT ;

	if (sl < 0)
	    sl = strlen(sp) ;

	if (sl > 0) {
	    int	hi ;
	    if ((hi = matocasestr(hits,1,sp,sl)) >= 0) {
	        f = (hi&1) ;
	    } else {
	        int	ch = MKCHAR(sp[0]) ;
	        if (isdigitlatin(ch)) {
	            uint	v ;
	            rs = cfdecui(sp,sl,&v) ;
	            f = (v > 0) ;
	        } else {
	            rs = SR_INVALID ;
		}
	    } /* end if */
	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (optbool) */


