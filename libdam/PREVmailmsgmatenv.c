/* mailmsgmatenv */

/* manipulate an UNIX® message envelope */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		0		/* "safe" mode */
#define	CF_SPACETAB	1		/* use space-tab as white space */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine detects if a mail-message "environment" line is present
        in the caller-supplied bufer.

        Note: The following strings are considered critical to parsing and
        otherwise understanding the envelope format. These are:

		"From"
		"remote from"
		"forwarded by"

        Currently, we allow any whitespace after the initial "From" but many
        other implementations only consider "From " to be a valid introducer.
        Producers of envelopes should consider always putting a SPACE character
        there rather than some other whitespace character.

	What is a proper representative envelope format?

	From rightcore.com!dam Wed Dec 8 11:44:30 EDT 1993 -0400

	What are the optional parts?

	From [rightcore.com!dam] [Wed] Dec 8 11:44[:30] [EDT] [[19]93] [-0400]


	Synopsis:

	int mailmsgmatenv(mep,mbuf,mlen)
	MAILMSGMATENV	*mep ;
	char		mbuf[] ;
	int		mlen ;

	Arguments:

	mep		pointer to message environment information
	mbuf		buffer holding data to be checked
	mlen		length of buffer to check

	Returns:

	>=0		match and this is the length of the address-part
	<0		error or no match


*******************************************************************************/


#define	MAILMSGMATENV_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<tmstrs.h>
#include	<localmisc.h>

#include	"mailmsgmatenv.h"


/* local defines */

#define	TWOCHARS(a,b)	(((a) << 8) + (b))
#define	SPACETAB(c)	(((c) == ' ') || ((c) == '\t'))

#if	CF_SPACETAB
#define	HEADERWHITE(c)	SPACETAB(c)
#else
#define	HEADERWHITE(c)	CHAR_ISWHITE(c)
#endif


/* external subroutines */

extern int	sisub(const char *,int,const char *) ;
extern int	sicasesub(const char *,int,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	nextitem(const char *,int,const char **) ;


/* local variables */


/* exported subroutines */


int mailmsgmatenv(MAILMSGMATENV *mep,cchar *sbuf,int slen)
{
	int		i, skip ;
	int		sl, cl ;
	int		ch = 0 ; /* ¥ GCC false complaint */
	int		f ;
	const char	*sp, *cp ;

#if	CF_SAFE
	if (mep == NULL) return SR_FAULT ;
	if (buf == NULL) return SR_FAULT ;
#endif /* CF_SAFE */

	if (slen < 0)
	    slen = strlen(sbuf) ;

#if	CF_DEBUGS
	debugprintf("mailmsgmatenv: ent> %t\n",sbuf,strnlen(sbuf,slen)) ;
#endif

	sp = sbuf ;
	sl = slen ;

	f = (*sp == '>') ;
	if (f) {
	    sp += 1 ;
	    sl -= 1 ;
	}

	if ((sl <= 0) || (*sp != 'F'))
	    return SR_NOTFOUND ;

	if (strncmp(sp,"From",4) != 0)
	    return SR_NOTFOUND ;

	sp += 4 ;
	sl -= 4 ;
	mep->f.start = (! f) ;

/* slip whitespace */

	f = FALSE ;
	while ((sl > 0) && HEADERWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	    f = TRUE ;
	}

	if ((! f) || (sp[0] == ':'))
	    return SR_NOTFOUND ;

/* get the address (or it might be the day-of-week) */

	cl = nextitem(sp,sl,&cp) ;

	if (cl <= 0)
	    return SR_NOTFOUND ;

/* for now pretend that it is either an address or the date */

	mep->a.ep = cp ;
	mep->a.el = cl ;
	mep->d.ep = cp ;

	sl -= ((cp + cl) - sp) ;
	sp = (cp + cl) ;

/* get the supposed day from the date */

	cl = nextitem(sp,sl,&cp) ;

	if (cl != 3)
	    return SR_NOTFOUND ;

	if (tmstrsday(cp,cl) >= 0) {

	    mep->d.ep = cp ;
	    sl -= ((cp + cl) - sp) ;
	    sp = cp + cl ;
	    cl = nextitem(sp,sl,&cp) ;

	    if ((cl != 3) || (tmstrsmonth(cp,cl) < 0))
	        return SR_NOTFOUND ;

	} else if ((tmstrsmonth(cp,cl) >= 0) && 
		(tmstrsday(mep->d.ep,cl) >= 0)) {

	    mep->a.ep = NULL ;
	    mep->a.el = 0 ;

	} else
	    return SR_NOTFOUND ;

	sl -= ((cp + cl) - sp) ;
	sp = cp + cl ;

/* get a field that is supposed to be at least one digit (day-of-month)! */

	cl = nextitem(sp,sl,&cp) ;

	if (cl > 0) ch = cp[0] ;
	if ((cl <= 0) || (cl > 2) || (! isdigitlatin(ch)))
	    return SR_NOTFOUND ;

/* scan for a colon character */

	cp = strnchr(sp,sl,':') ;

	if (cp == NULL)
	    return SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("mailmsgmatenv: colon at i=%d\n",(cp - sbuf)) ;
#endif

/* scan for "remote from" or "forwarded by" */

	skip = 11 ;
	i = sicasesub(sp,sl,"remote from") ;

	if (i < 0) {
	    skip = 12 ;
	    i = sicasesub(sp,sl,"forwarded by") ;
	}

	if ((i > 0) && (i < 3))
	    return SR_NOTFOUND ;

/* assume that the envelope is good at this point */

	if (i > 0) {
	    const char	*ep ;
	    int		el ;

	    cp = (sp+i) ;
	    mep->d.el = (cp - mep->d.ep) ;

	    el = sfshrink((cp + skip + 1), ((sp + sl) - (cp + skip + 1)),&ep) ;
	    mep->r.ep = ep ;
	    mep->r.el = el ;

	} else {

	    mep->d.el = (sp + sl) - mep->d.ep ;
	    mep->r.ep = NULL ;
	    mep->r.el = 0 ;

	} /* end if */

#if	CF_DEBUGS
	{
	    const char	*ep = mep->a.ep ;
	    int		el = mep->a.el ;
	    debugprintf("mailmsgmatenv: a=>%t<\n", ep,strlinelen(ep,el,50)) ;
	    ep = mep->d.ep ;
	    el = mep->d.el ;
	    debugprintf("mailmsgmatenv: d=>%t<\n", ep,strlinelen(ep,el,50)) ;
	    ep = mep->r.ep ;
	    el = mep->r.el ;
	    debugprintf("mailmsgmatenv: r=>%t<\n", ep,strlinelen(ep,el,50)) ;
	}
#endif

	return mep->a.el ;
}
/* end subroutine (mailmsgmatenv) */


/* local subroutines */


int nextitem(cchar *sp,int sl,cchar **spp)
{
	int		f_len = (sl >= 0) ;

/* skip leading white space */

	while (((! f_len) || (sl > 0)) && HEADERWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	*spp = sp ;

/* skip the non-white space */

	while ((((! f_len) && *sp) || (sl > 0)) && (! HEADERWHITE(*sp))) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	return (sp - (*spp)) ;
}
/* end subroutine (nextitem) */


