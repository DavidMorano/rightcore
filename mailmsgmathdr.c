/* mailmsgmathdr */

/* match on a message header (returns the key-name) */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SPACETAB	1		/* header whitespace is space-tab */
#define	CF_ALT1		0		/* use alternative-1 */


/* revision history:

	= 1998-08-15, David A­D­ Morano
	I copied this from something that I wrote previously.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine tests whether a MSG-header is in the suppled test
	string.  If a MSG-header is present, then the subroutine returns the
	length of the MSG-header key, otherwise it returns zero (0).

	Synopsis:

	int mailmsgmathdr(ts,tslen,ip)
	const char	ts[] ;
	int		tslen ;
	int		*ip ;

	Arguments:

	ts		string to test for header
	tslen		length of input string
	ip		pointer to string-index of header-value

	Returns:

	>0		yes-matched: length of found header-key
	==0		no-match: did not get a match
	<0		error


	Design notes:

	The 'ALT1' compile-time switch (above) was an experiment to try to find
	a faster way to execute this subroutine.  Profiling shows that this
	subroutine (as we might expect already) is significant in the total
	performance of reading large mailboxes (the only really performance
	challenge this whole program has).  So far, no significant way to
	increase performance has been found (so we leave the compile-switch
	OFF).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */

#define	SPACETAB(c)	(((c) == ' ') || ((c) == '\t'))

#if	CF_SPACETAB
#define	HEADERWHITE(c)	SPACETAB(c)
#else
#define	HEADERWHITE(c)	CHAR_ISWHITE(c)
#endif


/* external subroutines */

extern int	matcasestr(cchar **,cchar *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mailmsgmathdr(cchar *ts,int tslen,int *ip)
{
	int		tl ;
	int		kl = 0 ;
	cchar		*tp ;

#if	CF_DEBUGS
	debugprintf("mailmsgmathdr: ent tl=%d t=>%t<\n",
		tslen,ts,strlinelen(ts,tslen,40)) ;
#endif

	if (ts == NULL) return SR_FAULT ;

	if (ip != NULL)
	    *ip = NULL ;

	if (tslen < 0)
	    tslen = strlen(ts) ;

	tp = ts ;
	tl = tslen ;

#if	CF_ALT1
	{
	    int		ch ;
	    int		f = FALSE ;
	    while (tl) {
	        ch = *tp ;
		f = (ch == 0) ;
	        f = f || HEADERWHITE(ch) ;
	        f = f || (ch == ':') ;
		if (f) break ;
	        tp += 1 ;
	        tl -= 1 ;
	    }
	}
#else
	while (tl && *tp && (! HEADERWHITE(*tp)) && (*tp != ':')) {
	    tp += 1 ;
	    tl -= 1 ;
	} /* end while */
#endif /* CF_ALT1 */

	kl = (tp - ts) ;
	while (tl && HEADERWHITE(*tp)) {
	    tp += 1 ;
	    tl -= 1 ;
	}

	if (ip != NULL)
	    *ip = ((tp + 1) - ts) ;

#if	CF_DEBUGS
	debugprintf("mailmsgmathdr: ret f=%u kl=%u i=%i\n",
		(tl && (*tp == ':')),kl,((ip != NULL) ? *ip : 0)) ;
#endif

	return (tl && (*tp == ':')) ? kl : 0 ;
}
/* end subroutine (mailmsgmathdr) */


