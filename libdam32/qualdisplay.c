/* qualdisplay */

/* qualifit a display name with its domain-name */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1996-05-01, David A­D­ Morano 

	This subroutine was originally written.


*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is used to qualify an X window DISPLAY specification
	with the local hostname when :

	= it is not given at all!

	= it is an implied local name and the target is
	  in another domain!

	Arguments:

	- display	current display string
	- node		current nodename
	- domain	current domainname
	- disbuf	buffer to hold the resulting display value
	- dislen	length of user supplied display buffer

	Returns:

	>=0	length of the created DISPLAY string specification
	<0	**error**


*******************************************************************************/


#include	<string.h>

#include	<localmisc.h>


/* local defines */

#define	LENLEFT	(newdisplaylen + newdisplay - bp)


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */


/* exported subroutines */


int qualdisplay(olddisplay,node,domain,newdisplay,newdisplaylen)
const char	olddisplay[] ;
const char	node[] ;
const char	domain[] ;
char		newdisplay[] ;
int		newdisplaylen ;
{
	int	f_remote ;
	int	len = 0 ;
	int	f_done = FALSE ;

	char	*cp, *cp1 ;
	char	*bp = newdisplay ;


	newdisplay[0] = '\0' ;
	if ((olddisplay != NULL) && (olddisplay[0] == '\0'))
	    return BAD ;

#if	CF_DEBUGS
	debugprintf("qualdisplay: ent w/ h=%s display=%s dl=%d\n",
	    node,olddisplay,newdisplaylen) ;
#endif

/* handle the case where there is no hostname part at all ! */

	if (olddisplay[0] == ':') {

#if	CF_DEBUGS
	    debugprintf("qualdisplay: no hostname h=%s\n",node) ;
#endif

	    bp = newdisplay ;
#if	CF_DEBUGS
	    debugprintf("qualdisplay: h=%s new_so_far=%s\n",node,newdisplay) ;
#endif

	    bp = strwcpy(bp,node,LENLEFT) ;

#if	CF_DEBUGS
	    debugprintf("qualdisplay: h=%s new_so_far=%s\n",node,newdisplay) ;
#endif

	    if ((domain != NULL) && (domain[0] != '\0')) {

	        if (LENLEFT > 0)
	            *bp++ = '.' ;

	        bp = strwcpy(bp,domain,LENLEFT) ;

	    } /* end if */

#if	CF_DEBUGS
	    debugprintf("qualdisplay: new_so_far=%s  old=%s\n",
	        newdisplay,olddisplay) ;
#endif

	    bp = strwcpy(bp,olddisplay,LENLEFT) ;

	    len = bp - newdisplay ;
	    f_done = TRUE ;

#if	CF_DEBUGS
	    debugprintf("qualdisplay: new display=%s\n",newdisplay) ;
#endif

	} /* end if */

	f_remote = ((domain != NULL) && (domain[0] != '\0')) ;

/* handle the case where it is OK already */

	if (! f_done) {

#if	CF_DEBUGS
	    debugprintf("qualdisplay: checking if it is OK already\n") ;
	    debugprintf("qualdisplay: olddisplay=%s\n",olddisplay) ;
#endif

	    cp = strchr(olddisplay,':') ;

	    cp1 = NULL ;
	    if (f_remote)
	        cp1 = strchr(olddisplay,'.') ;

#if	CF_DEBUGS
	    debugprintf("qualdisplay: cp=%s\n",cp) ;
	    debugprintf("qualdisplay: cp1=%s\n",cp1) ;
#endif

	    if ((cp != NULL) && 
	        ((! f_remote) || ((cp1 != NULL) && (cp > cp1)))) {

#if	CF_DEBUGS
	        debugprintf("qualdisplay: it was OK already\n") ;
#endif

	        len = strlen(olddisplay) ;

	        bp = newdisplay ;
	        bp = strwcpy(bp,olddisplay,MIN(LENLEFT,len)) ;

	        len = bp - newdisplay ;
	        f_done = TRUE ;

	    } /* end if */

	} /* end if */

/* handle the case where there is no domain name part */

	if ((! f_done) && f_remote) {

#if	CF_DEBUGS
	    debugprintf("qualdisplay: checking for a partial hostname\n") ;
#endif

	    bp = newdisplay ;
	    bp = strwcpy(bp,olddisplay,MIN(cp - olddisplay,LENLEFT)) ;

	    if (LENLEFT > 0)
	        *bp++ = '.' ;

	    bp = strwcpy(bp,domain,LENLEFT) ;

	    bp = strwcpy(bp,cp,LENLEFT) ;

	    len = bp - newdisplay ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("qualdisplay: ret newdisplay=%s\n",newdisplay) ;
#endif

	return len ;
}
/* end subroutine (qualdisplay) */


