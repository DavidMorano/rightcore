/* nextqtoken */

/* find the next quoted string token */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2009-04-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine finds the next quoted string token in a given string.

	Synopsis:

	int nextqtoken(sp,sl,rpp)
	const char	*sp ;
	int		sl ;
	const char	**rpp ;

	Arguments:

	sp		pointer to source string
	sl		length of given source string
	rpp		pointer to result pointer

	Returns:

	==0		no more token strings were found
	>0		lenght of found string token


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	sidquote(const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int nextqtoken(sp,sl,rpp)
const char	*sp ;
int		sl ;
const char	**rpp ;
{
	int	si ;
	int	len ;

	if (sl < 0) sl = strlen(sp) ;

/* skip over whitespace */

	while (sl && CHAR_ISWHITE(sp[0])) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	if (rpp != NULL) *rpp = sp ;

/* skip over the non-whitespace */

	len = sl ;
	while (sl && sp[0] && (! CHAR_ISWHITE(sp[0]))) {
	    if (sp[0] == CH_DQUOTE) {
	        sp += 1 ;
	        sl -= 1 ;
	        si = sidquote(sp,sl) ;
	    } else {
	        si = 1 ;
	    } /* end if */
	    sp += si ;
	    sl -= si ;
	} /* end while */

/* done */

	len = (len-sl) ;

#if	CF_DEBUGS
	debugprintf("nextqtoken: ret len=%u\n",len) ;
#endif

	return len ;
}
/* end subroutine (nextqtoken) */


