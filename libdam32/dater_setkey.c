/* dater_setkey */

/* handle parsing a supplied date string using a key format identifier */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        Although there was probably something that could have done with parsing
        job (with a different argument syntax) I wrote this from scratch (sigh).
        Yes, we try and avoid writing whatever we can. This subroutine extends
        the DATE object to parse strings with the date type indicated by a key
        name.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine will parse out a date that has been specified
	with a syntax like:

		type=string

	where:

		type		is a name of a date string type
		string		is the string that is supposed to be the date


	Synopsis:

	int dater_setkey(dp,datestr,dlen,nowp,zn)
	DATER		*dp ;
	const char	datestr[] ;
	int		dlen ;
	struct timeb	*nowp ;
	const char	zn[] ;

	Arguments:

	dp		pointer to DATE object (already initialized)
	datestr		string containing the specified key=value pair
	dlen		length of supplied date string
	nowp		pointer to a timeb structure representing NOW
	zn		time zone name

	Returns:

	>=0		it all worked out OK
	<0		the date could not be parsed


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<dater.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	tolc(int) ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const char *datetypes[] = {
	"current",
	"now",
	"touch",
	"tt",
	"ttouch",
	"toucht",
	"log",
	"logz",
	"strdig",
	NULL
} ;

enum datetypes {
	datetype_current,
	datetype_now,
	datetype_touch,
	datetype_tt,
	datetype_ttouch,
	datetype_toucht,
	datetype_log,
	datetype_logz,
	datetype_strdig,
	datetype_overlast
} ;


/* exported subroutines */


int dater_setkey(dp,datestr,dlen,nowp,zn)
DATER		*dp ;
const char	datestr[] ;
int		dlen ;
struct timeb	*nowp ;
const char	zn[] ;
{
	int	rs = SR_OK ;
	int	tlen ;
	int	ti ;

	const char	*typename = NULL ;
	const char	*sp ;
	const char	*cp ;


	if (datestr == NULL)
	    return SR_FAULT ;

/* get the key name first (if it has one) */

	ti = -1 ;
	tlen = 0 ;
	if ((cp = strchr(datestr,'=')) != NULL) {

	    sp = cp + 1 ;
	    tlen = sfshrink(datestr,(cp - datestr),&typename) ;

#if	CF_DEBUGS
	    debugprintf("dater_setkey: explicit key=%W\n",typename,tlen) ;
#endif

	} else {
	    int	f = FALSE ;

	    sp = datestr ;
	    tlen = sfshrink(datestr,-1,&typename) ;

#if	CF_DEBUGS
	    debugprintf("dater_setkey: possible key=%W\n",typename,tlen) ;
#endif

	    if ((tolc(typename[0]) == 'c') ||
	        (tolc(typename[0]) == 'n'))
	        f = ((ti = matstr(datetypes,typename,tlen)) >= 0) ;

	    if (! f)
	        tlen = -1 ;

	} /* end if (getting possible typename) */

#if	CF_DEBUGS
	debugprintf("dater_setkey: ti=%d\n",ti) ;
	debugprintf("dater_setkey: tlen=%d typename=%W\n",tlen,typename,tlen) ;
#endif

/* if there was no type name, assume it was a TOUCHT type date string */

	if (ti < 0) {

	    if (tlen > 0) {

	        ti = matstr(datetypes,typename,tlen) ;
	        if (ti < 0) rs = SR_INVALID ;

	    } else
	        ti = datetype_toucht ;

	} /* end if (had to find the type index) */

#if	CF_DEBUGS
	debugprintf("dater_setkey: type[%d]=%s\n",ti,datetypes[ti]) ;
#endif

	if (rs >= 0) {
	    switch (ti) {

	    case datetype_touch:
	        rs = dater_settouch(dp,sp,-1) ;
	        break ;

	    case datetype_tt:
	    case datetype_ttouch:
	    case datetype_toucht:
	        rs = dater_settoucht(dp,sp,-1) ;
	        break ;

	    case datetype_log:
	    case datetype_logz:
	        rs = dater_setlogz(dp,sp,-1) ;
	        break ;

	    case datetype_strdig:
	        rs = dater_setstrdig(dp,sp,-1) ;
	        break ;

	    case datetype_current:
	    case datetype_now:
	        {
	            time_t	t = nowp->time ;
	            int	zoff = nowp->timezone ;
	            int	isdst = nowp->dstflag ;
	            rs = dater_settimezon(dp,t,zoff,zn,isdst) ;
	        }
	        break ;

	    default:
	        rs = SR_UNATCH ;
	        break ;

	    } /* end switch */
	} /* end if */

	return rs ;
}
/* end subroutine (dater_setkey) */



