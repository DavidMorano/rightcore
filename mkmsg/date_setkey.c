/* handledate */

/* handle parsing a supplied date string using a key format identifier */


#define	F_DEBUGS	0		/* non-switchable debug print-outs */


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
#include	<sys/stat.h>
#include	<sys/utsname.h>
#include	<sys/timeb.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<field.h>
#include	<localmisc.h>
#include	<date.h>


/* local defines */


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	matstr(char * const *,char *,int) ;
extern int	sfshrink(const char *,int,char **) ;

extern char	*strbasename(char *), *strshrink(char *) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */

#define	A	(__STDC__ != 0) 
#define	B	defined(_POSIX_C_SOURCE) 
#define	C	defined(_XOPEN_SOURCE)

#if	(A != 0) || (B != 0) || (C != 0)
extern long	altzone ;
#endif


/* local structures */


/* forward references */


/* global variables */


/* local variables */

static char *const datetypes[] = {
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

#define	DATETYPE_CURRENT	0
#define	DATETYPE_NOW		1
#define	DATETYPE_TOUCH		2
#define	DATETYPE_TT		3
#define	DATETYPE_TTOUCH		4
#define	DATETYPE_TOUCHT		5
#define	DATETYPE_LOG		6
#define	DATETYPE_LOGZ		7
#define	DATETYPE_STRDIG		8







int date_setkey(dp,datestr,dlen,nowp,zn)
DATE		*dp ;
char		datestr[] ;
int		dlen ;
struct timeb	*nowp ;
char		zn[] ;
{
	int	rs ;
	int	sl, cl ;
	int	tlen ;
	int	ti ;

	const char	*sp, *cp ;
	const char	*typename = NULL ;


	if (datestr == NULL)
	    return SR_FAULT ;

/* get the key name first (if it has one) */

	ti = -1 ;
	tlen = 0 ;
	if ((cp = strchr(datestr,'=')) != NULL) {

	    sp = cp + 1 ;
	    tlen = sfshrink(datestr,(cp - datestr),&typename) ;

#if	F_DEBUGS
	    eprintf("date_setkey: explicit key=%W\n",typename,tlen) ;
#endif

	} else {

	    int	f = FALSE ;


	    sp = datestr ;
	    tlen = sfshrink(datestr,-1,&typename) ;

#if	F_DEBUGS
	    eprintf("date_setkey: possible key=%W\n",typename,tlen) ;
#endif

	    if ((tolower(typename[0]) == 'c') ||
	        (tolower(typename[0]) == 'n'))
	        f = ((ti = matstr(datetypes,typename,tlen)) >= 0) ;

	    if (! f)
	        tlen = -1 ;

	} /* end if (getting possible typename) */

#if	F_DEBUGS
	eprintf("date_setkey: ti=%d\n",ti) ;
	eprintf("date_setkey: tlen=%d typename=%W\n",tlen,typename,tlen) ;
#endif

/* if there was no type name, assume it was a TOUCHT type date string */

	if (ti < 0) {

	    if (tlen > 0) {

	        ti = matstr(datetypes,typename,tlen) ;

	        if (ti < 0)
	            return SR_INVALID ;

	    } else
	        ti = DATETYPE_TOUCHT ;

	} /* end if (had to find the type index) */

#if	F_DEBUGS
	eprintf("date_setkey: type[%d]=%s\n",ti,datetypes[ti]) ;
#endif

	switch (ti) {

	case DATETYPE_TOUCH:
	    rs = date_settouch(dp,sp,-1) ;

	    break ;

	case DATETYPE_TT:
	case DATETYPE_TTOUCH:
	case DATETYPE_TOUCHT:
	    rs = date_settoucht(dp,sp,-1) ;

	    break ;

	case DATETYPE_LOG:
	case DATETYPE_LOGZ:
	    rs = date_setlogz(dp,sp,-1) ;

	    break ;

	case DATETYPE_STRDIG:
	    rs = date_setstrdig(dp,sp,-1) ;

	    break ;

	case DATETYPE_CURRENT:
	case DATETYPE_NOW:
	    rs = date_settimezn(dp,nowp->time,zn,-1) ;

	    break ;

	} /* end switch */


	return rs ;
}
/* end subroutine (date_setkey) */



