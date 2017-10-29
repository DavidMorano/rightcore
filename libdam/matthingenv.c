/* matthingenv */

/* match on message boundary */


#define	CF_DEBUGS	0
#define	CF_SHORTENV	0		/* early return? */
#define	CF_SPACETAB	1		/* header whitespace is space-tab */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This file is a collection of some subroutines that were individually
	written in the past.  I think that I added the 'matmsgstart()'
	subroutine for more general completeness.  Existing applications figure
	out whether it is a message-start on their own right now.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a small collection of subroutines that match up with the start
	of a mail message (its envelope line) or to match up to a mail message
	header line.

	Does the supplied string contain an UNIX mail envelope?  Return 0 if
	there is no match, else we return the character position of the
	envelope string after the initial "From " part.  The match is case
	sensitive!

	NOTE:	The return values for this subroutine are historical
		and are supposed to double as a TRUE/FALSE return.

	Oh, by the way, a typical CORRECT UNIX envelope is supposed to look
	like this :

		From dam Tue Apr 06 20:40 EDT 1994

	or

		From dam Tue Apr 06 20:40:32 EDT 1994

	Synopsis:

	int matthingenv(s,slen)
	const char	s[] ;
	int		slen ;

	Arguments:

	s		possible envelope string (buffer)
	len		len

	Returns:

	>0		the string contained an envelope
	==0		the string did not contain an envelope


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<mailmsgmatenv.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */

#define	MAXHEADERLEN	1025
#define	SPACETAB(c)	(((c) == ' ') || ((c) == '\t'))

#if	CF_SPACETAB
#define	HDRWHITE(c)	SPACETAB(c)
#else
#define	HDRWHITE(c)	CHAR_ISWHITE(c)
#endif


/* external subroutines */

extern int	matcasestr(const char **,const char *,int) ;
extern int	sibreak(const char *,int,const char *) ;
extern int	isdigitlatin(int) ;


/* external variables */


/* local structures */


/* forward references */

int		matthingenv(const char *,int) ;

static int	nexttoken(const char *,int,const char **) ;


/* local variables */

static const char	*months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	NULL
} ;

static const char	*days[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", NULL
} ;


/* exported subroutines */


int matthingenv(cchar *s,int slen)
{
	int		fromlen ;
	int		i, ll, fromindex = 0 ;
	int		f_len, f_esc ;
	int		ch ;
	const char	*cp, *cp2 ;
	const char	*from, *day, *month ;

#if	CF_DEBUGS
	    debugprintf("matthingenv: ent> %t\n",s,strnlen(s,len)) ;
#endif

	f_esc = (s[0] == '>') ;
	if ((! f_esc) && (s[0] != 'F'))
		return 0 ;

	cp = (char *) s ;
	if (f_esc) {
		cp += 1 ;
		slen -= 1 ;
	}

	if (strncmp(cp,"From ",5) != 0)
		return 0 ;

	f_len = (slen >= 0) ;
	cp += 5 ;
	slen -= 5 ;

#if	CF_DEBUGS
	debugprintf("matthingenv: match so far\n") ;
#endif

/* skip the white space after the leader string */

	while (((! f_len) || (slen > 0)) && HDRWHITE(*cp)) {
	    cp += 1 ;
	    slen -= 1 ;
	} /* end while */

	if (cp[0] == ':')
	    return 0 ;

	from = cp ;
	fromindex = cp - s ;

/* skip the envelope address */

	while ((((! f_len) && (*cp != '\0')) || (slen > 0)) && 
	    (! HDRWHITE(*cp))) {
	    cp += 1 ;
	    slen -= 1 ;
	} /* end while */

	fromlen = cp - from ;

/* handle the case of the old fashioned "short" envelopes! */

#if	CF_SHORTENV

/****
	Short envelopes only had the leading "From " portion
	followed by the envelope address, which was always
	just a simple user name (no non-alphanumerics).
****/

	if ((f_len && (slen <= 0)) || (*cp == '\0')) {

/* check if the address looks OK */

	    if (fromlen <= 0)
		return 0 ;

#ifdef	COMMENT
	    if (sibreak(from,(cp - from),":,;/") >= 0)
	        return 0 ;

	    if (sibreak(from,(cp - from),"!@%") >= 0)
	        return fromindex ;
#endif /* COMMENT */

	    for (i = 0 ; i < (cp - from) ; i += 1) {

	        if (! isalpha(from[i]))
	            return 0 ;

	    } /* end for */

	    return fromindex ;

	} /* end if (possible early return) */

#else /* CF_SHORTENV */

	if (fromlen <= 0)
		return 0 ;

#endif /* CF_SHORTENV */

#if	CF_DEBUGS
	debugprintf("matthingenv: long haul\n") ;
#endif

/* get the field that is supposed to have the day of the week */

	if ((ll = nexttoken(cp,slen,&day)) != 3)
	    return 0 ;

	slen -= (day + ll - cp) ;
	cp = day + ll ;
	if (matcasestr(days,day,3) < 0)
	    return 0 ;

/* get the field that is supposed to have the month in it */

	if ((ll = nexttoken(cp,slen,&month)) != 3)
	    return 0 ;

	slen -= (month + ll - cp) ;
	cp = month + ll ;
	if (matcasestr(months,month,3) < 0)
	    return 0 ;

/* does the next field contain only digits? */

	if ((ll = nexttoken(cp,slen,&cp2)) > 2)
	    return 0 ;

	for (i = 0 ; i < ll ; i += 1) {
	    ch = MKCHAR(cp2[i]) ;
	    if (! isdigitlatin(ch)) return 0 ;
	}

/* the next field should have one or more colons in it (the rest digits) */

	slen -= (cp2 + ll - cp) ;
	cp = cp2 + ll ;
	if ((ll = nexttoken(cp,slen,&cp2)) < 4)
	    return 0 ;

	ch = MKCHAR(cp2[0]) ;
	if ((! isdigitlatin(ch)) || (strchr(cp2,':') == NULL))
		return 0 ;

/* if we have gotten this far OK, then we assume this IS an envelope! */

	return fromindex ;
}
/* end subroutine (matthingenv) */


/* find the start of a mesage by the existence of an UNIX envelope */
int matmsgstart(cchar *buf,int len)
{
	int		rc = 0 ;

	if (buf[0] == 'F') {
	    MAILMSGMATENV	e ;
	    rc = mailmsgmatenv(&e,buf,len) ;
	}

	return rc ;
}
/* end subroutine (matmsgstart) */


/* local subroutines */


static int nexttoken(cchar *buf,int len,cchar **spp)
{
	int		f_len = (len >= 0) ;
	const char	*cp = buf ;

/* skip leading white space */

	while (((! f_len) || (len > 0)) && HDRWHITE(*cp)) {
	    cp += 1 ;
	    len -= 1 ;
	} /* end while */

	*spp = cp ;

/* skip the non-white space */

	while ((((! f_len) && (*cp != '\0')) || (len > 0)) && 
	    (! HDRWHITE(*cp))) {

	    cp += 1 ;
	    len -= 1 ;

	} /* end while */

	return (cp - (*spp)) ;
}
/* end subroutine (nexttoken) */


