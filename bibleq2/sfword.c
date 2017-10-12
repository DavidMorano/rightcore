/* sfword */

/* find an English word within the given string */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_ALLOWSMODE	0		/* faster? */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	The subroutine was written from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine finds an English language word within the given string.
	Leading white space is removed along with leading or trailing
	single-quote (apostrophe) characters.  Trailing single quotes are not
	removed if they appear to be making the embodied English word
	possessive.  Also, leading and trailing hypen (minus-sign) characters
	are also removed (and will be removed even when intermixed with regular
	white-space characters).

	Synopsis:

	int sfword(sp,sl,rpp)
	const char	*sp ;
	int		sl ;
	const char	**rpp ;

	Arguments:

	sp		source string
	sl		length of supplied source string
	rpp		pointer to pointer to get result (an English word)

	Return:

	-		length of resulting word

	Notes:

        = CF_ALLOWSMORE: I don't know but this was a (vain?) attempt at speeding
        up this subroutine. Profiles show that time is being spent in here (how
        much compared with everything else?). There is not much to improve upon,
        without removing nice old faithful subroutine calls (like
        'sfshrink(3dam)'). Instead we play around (elsewhere) with removing the
        string conversion to lower-case before an array comparison. You be the
        judge.


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<string.h>

#include	<ascii.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;

extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* external variables */


/* forward references */

static int	sfshrinkmore(const char *,int,const char **) ;
static int	iswhitemore(int) ;


/* local variables */

static cchar	*allows[] = {
	"'s",
	"'t",
	"s'",
#if	CF_ALLOWSMODE
	"'S",
	"'T",
	"S'",
#endif /* CF_ALLOWSMODE */
	NULL
} ;


/* exported subroutines */


int sfword(cchar *sp,int sl,cchar **rpp)
{
	int		cl ;
	cchar		*tp ;
	cchar		*cp ;

	if ((cl = sfshrinkmore(sp,sl,&cp)) > 0) {
	    int	f = FALSE ;

	    if (cp[0] == CH_SQUOTE) {
		cp += 1 ;
		cl -= 1 ;
	    }

	    if (cl > 2) {

		if ((tp = strnrchr(cp,cl,CH_SQUOTE)) != NULL) {
		    f = ((cl - ((tp + 1) - cp)) > 1) ;
		}

		if (! f) {
	    	    int	i = (cl - 2) ;

#if	CF_ALLOWSMORE
		    {
			int 	j ;
			cchar	*cc = (cp+i) ;
	                for (j = 0 ; allows[j] != NULL ; j += 1) {
			    f = (allows[j][0] == cc[0]) ;
	                    f = f && (strncmp(allows[j],cc,2) == 0) ;
			    if (f) break ;
	                } /* end for */
		    }
#else /* CF_ALLOWSMORE */
		    {
			int	j ;
			char	lowbuf[4] ;
			cchar	*cc = lowbuf ;
			lowbuf[0] = CHAR_TOLC((cp+i)[0]) ;
			lowbuf[1] = CHAR_TOLC((cp+i)[1]) ;
	                for (j = 0 ; allows[j] != NULL ; j += 1) {
			    f = (allows[j][0] == cc[0]) ;
	                    f = f && (strncmp(allows[j],cc,2) == 0) ;
			    if (f) break ;
	                } /* end for */
		    }
#endif /* CF_ALLOWSMORE */

		} /* end if */

	    } /* end if (more than one character) */

	    if ((! f) && (cp[cl - 1] == CH_SQUOTE)) {
	        cl -= 1 ;
	    }

	} /* end if (non-zero) */

	if (rpp != NULL)
	    *rpp = cp ;

	return cl ;
}
/* end subroutine (sfword) */


/* local subroutines */


static int sfshrinkmore(cchar *sp,int sl,cchar **rpp)
{
	const char	*cp ;
	int		cl = 0 ;

	if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	    while (cl && iswhitemore(cp[0])) {
		cp += 1 ;
		cl -= 1 ;
	    }
	    *rpp = cp ;
	    while (cl && iswhitemore(cp[cl-1])) {
		cl -= 1 ;
	    }
	}

	return cl ;
}
/* end subroutine (sfshrinkmore) */


static int iswhitemore(int ch)
{
	return CHAR_ISWHITE(ch) || (ch == '-') ;
}
/* end subroutine (iswhitemore) */


