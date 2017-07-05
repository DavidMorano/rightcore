/* field_srvarg */

/* subroutine to field out (parse) a line into server arguments */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_TRAILWHITE	1		/* remove trailing whitespace? */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This code module was originally written in C language modeled (roughly)
	from a prior VAX assembly language version (circa 1980 perhaps).  This
	is why is looks so "ugly"!  This stuff comes from stuff dated back to
	almost the pre-dawm era of computer languages!  I wrote the original
	VAX assembly stuff also.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is very similar to 'field_sharg' in that it will parse
	a string of arguments similar to SHELL command line arguments.  It is
	different in that a comma character will terminate the list of
	argumuents (along with an EOL).  This function is needed for gathering
	up arguments to a key string in things like server table files.

	Note that generally, this function will skip over white space in order
	to get a group of items that are all considered one argument in the
	outer parsing context.

	Synopsis:

	int field_srvarg(fsbp,terms,abuf,alen)
	FIELD		*fsbp ;
	const uchar	terms[] ;
	char		abuf[] ;
	int		alen ;

	Arguments:

	fsbp		field status block pointer
	terms		bit array of terminating characters
	abuf		buffer to store result
	alen		length of buffer to hold result

	Returns:

	>0		length of retutned field
	==0		no additional fields

	The return status block outputs are:

	- length remaining in string
	- address of reminaing string
	- len of substring
	- address of substring
	- terminator character

	This subroutine will do quote interpretation like the SHELL when inside
	double or single quotes!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<baops.h>
#include	<char.h>
#include	<field.h>
#include	<localmisc.h>


/* local defines */

#define	CLEANCHAR(c)	((c) & 0xff)


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif /* CF_DEBUGS */

/* external variables */


/* local structures */


/* local variables */

/* 'double quote', 'back slash', 'pound', 'back accent', et cetera */
static const unsigned char	doubles[] = {
	0x00, 0x00, 0x00, 0x00,
	0x14, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x10,
	0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
} ;

/* default parse terminators for sever arguments! */
static const unsigned char	dterms[] = {
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int field_srvarg(FIELD *fsbp,const uchar *terms,char *abuf,int alen)
{
	uint		ch, nch ;
	uint		qe ;
	uint		term = '\0' ;
	int		ll ;
	int		fl = 0 ;
	const uchar	*lp ;
	uchar		*bp = (unsigned char *) abuf ;

	if ((fsbp == NULL) || (abuf == NULL)) return SR_FAULT ;

/* get the parameters */

	lp = (const uchar *) fsbp->lp ;
	ll = fsbp->ll ;

	if (terms == NULL) terms = dterms ;

#if	CF_DEBUGS
	debugprintf("field_srvarg: l=>%t<\n",lp,ll) ;
#endif

/* skip all initial white space */

	while ((ll > 0) && CHAR_ISWHITE(*lp)) {
	    lp += 1 ;
	    ll -= 1 ;
	}

	abuf[0] = '\0' ;

/* process the standard SHELL string */

	while (ll > 0) {

	    ch = CLEANCHAR(*lp) ;

#if	CF_DEBUGS
	debugprintf("field_srvarg: ch=>%c<\n",ch) ;
#endif

	    if (BATST(terms,ch))
		break ;

	    if (ch == '\"') {

	        if (alen > 0) {
	            *bp++ = ch ;
	            alen -= 1 ;
	        }

	        qe = ch ;
		lp += 1 ;
	        ll -= 1 ;
	        while (ll > 0) {

	            ch = CLEANCHAR(*lp) ;
		    if (ll > 1) nch = CLEANCHAR(lp[1]) ;

	            if ((ch == '\\') && (ll > 1) && BATST(doubles,nch)) {

	                if (alen > 0) {
	                    *bp++ = ch ;
	                    alen -= 1 ;
	                }

	                lp += 1 ;
	                ll -= 1 ;
	                ch = CLEANCHAR(*lp) ;
	                if (alen > 0) {
	                    *bp++ = ch ;
	                    alen -= 1 ;
	                }

	                lp += 1 ;
	                ll -= 1 ;

	            } else if (ch == qe) {

	                if (alen > 0) {
	                    *bp++ = ch ;
	                    alen -= 1 ;
	                }

	                lp += 1 ;
	                ll -= 1 ;
	                break ;

	            } else {

	                if (alen > 0) {
	                    *bp++ = ch ;
	                    alen -= 1 ;
	                }

	                lp += 1 ;
	                ll -= 1 ;

	            } /* end if */

	        } /* end while (processing the quoted portion) */

	    } else if (ch == '\'') {

	        if (alen > 0) {
	            *bp++ = ch ;
	            alen -= 1 ;
	        }

	        qe = ch ;
		lp += 1 ;
	        ll -= 1 ;
	        while (ll > 0) {

	            ch = CLEANCHAR(*lp) ;
	            if (ch == qe) {

	                if (alen > 0) {
	                    *bp++ = ch ;
	                    alen -= 1 ;
	                }

	                lp += 1 ;
	                ll -= 1 ;
	                break ;

	            } else {

	                if (alen > 0) {
	                    *bp++ = ch ;
	                    alen -= 1 ;
	                }

	                lp += 1 ;
	                ll -= 1 ;

	            } /* end if */

	        } /* end while (processing the quoted portion) */

	    } else if (ch == '<') {

	        qe = '>' ;
	        lp += 1 ;
	        ll -= 1 ;
	        while (ll > 0) {

	            ch = CLEANCHAR(*lp) ;
		    if (ll > 1) nch = CLEANCHAR(lp[1]) ;

	            if ((ch == '\\') && (ll > 1) && BATST(doubles,nch)) {

	                if (alen > 0) {
	                    *bp++ = ch ;
	                    alen -= 1 ;
	                }

	                lp += 1 ;
	                ll -= 1 ;
	                ch = CLEANCHAR(*lp) ;
	                if (alen > 0) {
	                    *bp++ = ch ;
	                    alen -= 1 ;
	                }

	                lp += 1 ;
	                ll -= 1 ;

	            } else if (ch == qe) {

	                lp += 1 ;
	                ll -= 1 ;
	                break ;

	            } else {

	                if (alen > 0) {
	                    *bp++ = ch ;
	                    alen -= 1 ;
	                }

	                lp += 1 ;
	                ll -= 1 ;

	            } /* end if */

	        } /* end while (processing the quoted portion) */

	    } else if ((ch == '\\') && (ll > 1)) {

	        if (alen > 0) {
	            *bp++ = ch ;
	            alen -= 1 ;
	        }

	        lp += 1 ;
	        ll -= 1 ;
	        ch = CLEANCHAR(*lp) ;
	        if (alen > 0) {
	            *bp++ = ch ;
	            alen -= 1 ;
	        }

	        lp += 1 ;
	        ll -= 1 ;

	    } else {

	        if (alen > 0) {
	            *bp++ = ch ;
	            alen -= 1 ;
	        }

	        lp += 1 ;
	        ll -= 1 ;

	    } /* end if */

	} /* end while */

#if	CF_DEBUGS
	debugprintf("field_srvarg: while-out fl=%d\n",(bp-((uchar*)abuf))) ;
#endif

/* do the terminator processing */

	if (ll > 0) {
	    term = ' ' ;
	    ch = CLEANCHAR(*lp) ;
	    if (BATST(terms,ch)) {
	        term = ch ;
	        lp += 1 ;
	        ll -= 1 ;
	    } /* end if (it was a terminator) */
	}

	while ((ll > 0) && CHAR_ISWHITE(*lp)) {
	    lp += 1 ;
	    ll -= 1 ;
	} /* end while */

/* remove whitespace from trailing end of acquired field */

	fl = (bp - ((uchar *) abuf)) ;

#if	CF_DEBUGS
	debugprintf("field_srvarg: mid10 fl=%d\n",fl) ;
#endif

#if	CF_TRAILWHITE
	while ((fl > 0) && CHAR_ISWHITE(abuf[fl - 1])) {
		fl -= 1 ;
	}
#endif

#if	CF_DEBUGS
	debugprintf("field_srvarg: mid11 fl=%d\n",fl) ;
#endif

/* update the return status block */

	fsbp->lp = (const uchar *) lp ;
	fsbp->ll = ll ;
	fsbp->fp = (fl >= 0) ? ((const uchar *) abuf) : NULL ;
	fsbp->fl = fl ;
	fsbp->term = term ;

#if	CF_DEBUGS
	debugprintf("field_srvarg: ret fl=%d term=>%c<\n",fl,term) ;
	debugprintf("field_srvarg: ret a=>%t<\n",abuf,fl) ;
#endif

	return fl ;
}
/* end subroutine (field_srvarg) */


