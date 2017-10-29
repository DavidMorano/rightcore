/* matenv */

/* manipulate an UNIX message envelope */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		0		/* "safe" mode */
#define	CF_SPACETAB	1		/* use space-tab as white space */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a small collection of subroutines that form a simple object for
	parsing and storing the elements of an UNIX message envelope.

	There are many (too many) subroutines out there that detect, parse, or
	otherwise process UNIX message envelope information.  I really wish
	that another set of subroutines was not needed.  But, no, we need to do
	just that simple little thing that nobody else currently does!  Isn't
	this the way?!

	Note: The following strings are considered critical to parsing and
	otherwise understanding the envelope format.  These are:

		"From"
		"remote from"
		"forwarded by"

	Currently, we allow any whitespace after the initial "From" but many
	other implementations only consider "From " to be a valid introducer.
	Producers of envelopes should consider always putting a SPACE character
	there rather than some other whitespace character.

	Synopsis:

	int matenv(mep,buf,buflen)
	MATENV	*mep ;
	char	buf[] ;
	int	buflen ;

	Arguments:

	mep		pointer to message environment information
	buf		buffer holding data to be checked
	buflen		length of buffer to check

	Returns:

	>=0		match and this is the length of the address
	<0		error or not match


******************************************************************************/


#define	MATENV_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>

#include	"matenv.h"


/* local defines */

#define	TWOCHARS(a,b)	(((a) << 8) + (b))
#define	SPACETAB(c)	(((c) == ' ') || ((c) == '\t'))

#if	CF_SPACETAB
#define	HDRWHITE(c)	SPACETAB(c)
#else
#define	HDRWHITE(c)	CHAR_ISWHITE(c)
#endif


/* external subroutines */

extern int	sisub(const char *,int,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	nextitem(const char *,int,char **) ;
static int	getday(const char *) ;
static int	getmonth(const char *) ;


/* local variables */


/* exported subroutines */


int matenv(MATENV *mep,cchar *ebuf,int elen)
{
	int		i, skip ;
	int		sl, cl ;
	int		f ;
	const char	*sp, *cp ;

#if	CF_SAFE
	if (mep == NULL)
	    return SR_FAULT ;

	if (ebuf == NULL)
	    return SR_FAULT ;
#endif /* CF_SAFE */

	if (elen < 0)
	    elen = strlen(ebuf) ;

#if	CF_DEBUGS
	debugprintf("matenv: ent> %t\n",ebuf,strnlen(ebuf,elen)) ;
#endif

	sp = (char *) ebuf ;
	sl = elen ;

	f = (*sp == '>') ;
	if (f) {
	    sp += 1 ;
	    sl -= 1 ;
	}

	if ((sl <= 0) || (*sp != 'F'))
	    return SR_INVALID ;

	if (strncmp(sp,"From",4) != 0)
	    return SR_INVALID ;

	sp += 4 ;
	sl -= 4 ;
	mep->f.start = (! f) ;

/* slip whitespace */

	f = FALSE ;
	while ((sl > 0) && HDRWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	    f = TRUE ;
	}

	if ((! f) || (sp[0] == ':'))
	    return SR_INVALID ;

/* get the address (or it might be the day-of-week) */

	cl = nextitem(sp,sl,&cp) ;

	if (cl <= 0)
	    return SR_INVALID ;

/* for now pretend that it is either an address or the start of the date */

	mep->address.p = cp ;
	mep->address.len = cl ;
	mep->date.p = cp ;

	sl -= ((cp + cl) - sp) ;
	sp = (cp + cl) ;

/* get the supposed day from the date */

	cl = nextitem(sp,sl,&cp) ;

	if (cl != 3)
	    return SR_INVALID ;

	if (getday(cp) >= 0) {

	    mep->date.p = cp ;
	    sl -= ((cp + cl) - sp) ;
	    sp = cp + cl ;
	    cl = nextitem(sp,sl,&cp) ;

	    if ((cl != 3) || (getmonth(cp) < 0))
	        return SR_INVALID ;

	} else if ((getmonth(cp) >= 0) && (getday(mep->date.p) >= 0)) {

	    mep->address.p = NULL ;
	    mep->address.len = 0 ;

	} else
	    return SR_INVALID ;

	sl -= ((cp + cl) - sp) ;
	sp = cp + cl ;

/* get a field that is supposed to be at least one digit (day-of-month)! */

	cl = nextitem(sp,sl,&cp) ;

	if ((cl <= 0) || (cl > 2) || (! isdigit(*cp)))
	    return SR_INVALID ;

/* scan for a colon character */

	cp = strnchr(sp,sl,':') ;

	if (cp == NULL)
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("matenv: colon at i=%d\n",(cp - buf)) ;
#endif

/* scan for "remote from" or "forwarded by" */

	skip = 11 ;
	i = sisub(sp,sl,"remote from") ;
	cp = (sp+i) ;

	if (i < 0) {
	    skip = 12 ;
	    i = sisub(sp,sl,"forwarded by") ;
	    cp = (sp+i) ;
	}

	if ((i > 0) && (i < 3))
	    return SR_INVALID ;

/* assume that the envelope is good at this point */

	if (i > 0) {

	    mep->date.len = cp - mep->date.p ;

	    mep->remote.len = sfshrink((cp + skip + 1), 
			((sp + sl) - (cp + skip + 1)),&mep->remote.p) ;

	} else {

	    mep->date.len = (sp + sl) - mep->date.p ;

	    mep->remote.p = NULL ;
	    mep->remote.len = 0 ;

	} /* end if */

	return mep->address.len ;
}
/* end subroutine (matenv) */


/* local subroutines */


static int nextitem(sp,sl,spp)
const char	*sp ;
int		sl ;
char		**spp ;
{
	int		f_len = (sl >= 0) ;

	while (((! f_len) || (sl > 0)) && HDRWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	*spp = (char *) sp ;

/* skip the non-white space */

	while ((((! f_len) && *sp) || (sl > 0)) && (! HDRWHITE(*sp))) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	return (sp - (*spp)) ;
}
/* end subroutine (nextitem) */


static int getday(s)
const char	s[] ;
{
	int		rs ;

	switch (TWOCHARS((s[0] & (~ 0x20)),(s[1] | 0x20))) {
	case TWOCHARS('S', 'u'):
	    rs = 0 ;
	    break ;
	case TWOCHARS('M', 'o'):
	    rs = 1 ;
	    break ;
	case TWOCHARS('T', 'u'):
	    rs = 2 ;
	    break ;
	case TWOCHARS('W', 'e'):
	    rs = 3 ;
	    break ;
	case TWOCHARS('T', 'h'):
	    rs = 4 ;
	    break ;
	case TWOCHARS('F', 'r'):
	    rs = 5 ;
	    break ;
	case TWOCHARS('S', 'a'):
	    rs = 6 ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (getday) */


static int getmonth(s)
const char	s[] ;
{
	int		rs = SR_INVALID ;

	switch (TWOCHARS((s[0] & (~ 0x20)),(s[1] | 0x20))) {
	case TWOCHARS('J', 'a'):
	    rs = 1 ;
	    break ;
	case TWOCHARS('F', 'e'):
	    rs = 2 ;
	    break ;
	case TWOCHARS('M', 'a'):		/* March - May */
	    rs = (((s[2] | 0x20) == 'r') ? 3 : 5) ;
	    break ;
	case TWOCHARS('A', 'p'):
	    rs = 4 ;
	    break ;
	case TWOCHARS('J', 'u'):		/* June - July */
	    rs = (((s[2] | 0x20) == 'n') ? 6 : 7) ;
	    break ;
	case TWOCHARS('A', 'u'):
	    rs = 8 ;
	    break ;
	case TWOCHARS('S', 'e'):
	    rs = 9 ;
	    break ;
	case TWOCHARS('O', 'c'):
	    rs = 10 ;
	    break ;
	case TWOCHARS('N', 'o'):
	    rs = 11 ;
	    break ;
	case TWOCHARS('D', 'e'):
	    rs = 12 ;
	    break ;
	default:
	    rs = SR_INVALID ;		/* bad month name */
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (getmonth) */


