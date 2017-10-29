/* mkcleanline */

/* print a clean (cleaned up) line of text */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_CLEAN1	0		/* use CLEAN-1 */
#define	CF_CLEAN2	1		/* use CLEAN-2 */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine cleans up a line of text.  A mode paramter specifies
	how clean the resulting line gets.  To wit:

	mode	delete­or­sub	terminate­on­nul
	----------------------------------------
	0	delete		YES
	1	sub		YES
	2	delete		NO
	3	sub		NO

	Synopsis:

	int mkcleanline(lp,ll,m)
	char		lp[] ;
	int		ll ;
	int		m ;

	Arguments:

	lp		line-buffer to get operated on (must be writable)
	ll		length of line-buffer on input
	m		mode parameter (see above on use of mode)

	Returns:

	-		length of resulting line (not NL terminated)

	Important note:

	Resulting lines are nver NL (EOL) terminated - even if the input line
	was!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	isprintlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strnnlen(const char *,int,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */


/* external variables */


/* local structures */

struct mflags {
	uint	subnul:1 ;
	uint	subbad:1 ;
} ;


/* forward references */

#if	CF_CLEAN1
static int	clean1(struct mflags *,char *,int) ;
static int	isshift(int) ;
#else
static int	clean2(struct mflags *,char *,int) ;
#endif

static int	ischarok(int) ;
static int	isend(int) ;


/* module global variables */


/* local variables */


/* exported subroutines */


int mkcleanline(char lp[],int ll,int m)
{
	struct mflags	mf ;
	int		len = 0 ;

	if (lp == NULL) return SR_FAULT ;

	if (ll < 0)
	    ll = strlen(lp) ;

	while ((ll > 0) && isend(lp[ll - 1])) {
	    ll -= 1 ;
	}

#if	CF_DEBUGS
	debugprintf("mkcleanline: ll=%u lp=>%t<¬\n",ll,
	    lp,strlinelen(lp,ll,40)) ;
#endif 

	memset(&mf,0,sizeof(struct mflags)) ;
	mf.subnul = (m & 1) ;
	mf.subbad = (m & 2) ;

#if	CF_CLEAN1
	len = clean1(&mf,lp,ll) ;
#else
	len = clean2(&mf,lp,ll) ;
#endif

#if	CF_DEBUGS
	debugprintf("mkcleanline: ret len=^u\n",len) ;
#endif /* CF_DEBUGS */

	return len ;
}
/* end subroutine (mkcleanline) */


/* local subroutines */


#if	CF_CLEAN1

static int clean1(mfp,lp,ll)
struct mflags	*mfp ;
char		*lp ;
int		ll ;
{
	int		ili ;
	int		ch ;
	int		oli = 0 ;
	int		f_flipped = FALSE ;
	int		f_rem = FALSE ;
	int		f ;

	for (ili = 0 ; ili < ll ; ili += 1) {

	    if (! f_rem) {

	        ch = MKCHAR(lp[ili]) ;
	        if ((ch == 0) && (! mfp->subnul))
	            break ;

	        if (! isshift(ch)) {

	            f = isprintlatin(ch) || ischarok(ch) ;
	            if (f) {
	                if (f_flipped) {
	                    lp[oli++] = lp[ili] ;
	                } else {
	                    oli += 1 ;
			}
	            } else if (mfp->subbad) {
	                lp[oli++] = (char) '­' ;
	            } else
	                f_flipped = TRUE ;

	        } else
	            f_rem = TRUE ;

	    } else {
	        f_flipped = TRUE ;
	        f_rem = FALSE ;
	    }

	} /* end for */

	lp[oli] = '\0' ;
	return oli ;
}
/* end subroutine (clean1) */

tatic int isshift(int ch)
{
	int		f = FALSE ;
	f = f || (ch == CH_SS2) ;
	f = f || (ch == CH_SS3) ;
	return f ;
}
/* end subroutine (isshift) */

#endif /* CF_CLEAN1 */


#if	CF_CLEAN2

static int clean2(mfp,lp,ll)
struct mflags	*mfp ;
char		*lp ;
int		ll ;
{
	int		ili ;
	int		ch ;
	int		oli = 0 ;
	int		f_flipped = FALSE ;
	int		f ;

	for (ili = 0 ; ili < ll ; ili += 1) {

	    ch = MKCHAR(lp[ili]) ;
	    if ((ch == 0) && (! mfp->subnul))
	        break ;

	    f = isprintlatin(ch) || ischarok(ch) ;
	    if (f) {
	        if (f_flipped) {
	            lp[oli++] = lp[ili] ;
	        } else {
	            oli += 1 ;
		}
	    } else if (mfp->subbad) {
	        lp[oli++] = (char) '­' ;
	    } else
	        f_flipped = TRUE ;

	} /* end for */

	lp[oli] = '\0' ;
	return oli ;
}
/* end subroutine (clean2) */

#endif /* CF_CLEAN2 */


static int ischarok(int ch)
{
	return (ch == '\t') || (ch == '\n') ;
}
/* end subroutine (ischarok) */


static int isend(int ch)
{
	return (ch == '\n') || (ch == '\r') ;
}
/* end subroutine (isend) */


