/* field_wordphrase */

/* subroutine to parse a line into word-fields */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
        This code module was originally written in C language but modeled
        (roughly) from a prior VAX assembly language version of mine (circa 1980
        perhaps). This is why is looks so "ugly"!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int field_wordphrase(fsbp,terms,fbuf,flen)
	FIELD		*fsbp ;
	uchar		*terms ;
	char		fbuf[] ;
	int		flen ;

	Arguments:

	fsbp		field status block pointer
	terms		bit array of terminating characters
	fbuf		buffer to store result
	flen		length of buffer to hold result

	Returns:

	>=0	length of field just parsed out (length of FIELD!)
	<0	invalid field block pointer was passwd

	The return status block outputs are:

	- length remaining in string
	- address of reminaing string
	- len of substring
	- address of substring
	- terminator character


*******************************************************************************/


#define	FIELD_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<char.h>
#include	<field.h>
#include	<localmisc.h>


/* local defines */

#define	CLEANCHAR(c)	((c) & 0xff)


/* local variables */

/* quote character '\"' */
static const unsigned char	dquote[] = {
	0x00, 0x00, 0x00, 0x00,
	0x04, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

/* 'double quote', 'back slash', 'pound', 'back accent', et cetera */
static const unsigned char	doubles[] = {
	0x00, 0x00, 
	0x00, 0x00,
	0x14, 0x00, 
	0x00, 0x00,
	0x00, 0x00, 
	0x00, 0x10,
	0x01, 0x00, 
	0x00, 0x00,
	0x00, 0x00, 
	0x00, 0x00,
	0x00, 0x00, 
	0x00, 0x00,
	0x00, 0x00, 
	0x00, 0x00,
	0x00, 0x00, 
	0x00, 0x00
} ;

static const unsigned char	shterms[] = {
	0x00, 0x00, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int field_wordphrase(FIELD *fsbp,const uchar *terms,char *fbuf,int flen)
{
	int		ll, fl ;
	int		qe ;
	int		ch = 0 ;
	int		nch ;
	int		term = '\0' ;
	const uchar	*lp ;
	uchar		*bp = (unsigned char *) fbuf ;

	if (fsbp == NULL) return SR_FAULT ;
	if (fbuf == NULL) return SR_FAULT ;

	if (fsbp->lp == NULL) return SR_NOTOPEN ;

/* get the parameters */

	lp = fsbp->lp ;
	ll = fsbp->ll ;

	if (terms == NULL)
	    terms = shterms ;

/* skip all initial white space */

	while ((ll > 0) && CHAR_ISWHITE(*lp)) {
	    lp += 1 ;
	    ll -= 1 ;
	}

	fl = -1 ;			/* end-of-arguments indicator */
	if (ll > 0)  {

/* process the standard SHELL string */

	while (ll > 0) {

	    ch = CLEANCHAR(*lp) ;
	    if (BATST(terms,ch) & (! BATST(dquote,ch))) 
		break ;

	    if (CHAR_ISWHITE(ch)) 
		break ;

	    if (ch == '\"') {

	        qe = ch ;
		lp += 1 ;
	        ll -= 1 ;
	        while (ll > 0) {

		    ch = CLEANCHAR(*lp) ;
		    if (ll > 1)
			nch = CLEANCHAR(lp[1]) ;

	            if ((ch == '\\') && (ll > 1) && BATST(doubles,nch)) {

	                lp += 1 ;
	                ll -= 1 ;
		        ch = CLEANCHAR(*lp) ;
	                if (flen > 0) {
	                    *bp++ = ch ;
	                    flen -= 1 ;
	                }

	                lp += 1 ;
	                ll -= 1 ;

	            } else if (ch == qe) {

	                lp += 1 ;
	                ll -= 1 ;
	                break ;

	            } else {

	                if (flen > 0) {
	                    *bp++ = ch ;
	                    flen -= 1 ;
	                }

	                lp += 1 ;
	                ll -= 1 ;

	            } /* end if */

	        } /* end while (processing the quoted portion) */

	    } else if (ch == '\'') {

		qe = ch ;
		lp += 1 ;
	        ll -= 1 ;
	        while (ll > 0) {

		    ch = CLEANCHAR(*lp) ;
	            if (ch == qe) {

	                lp += 1 ;
	                ll -= 1 ;
	                break ;

	            } else {

	                if (flen > 0) {
	                    *bp++ = ch ;
	                    flen -= 1 ;
	                }

	                lp += 1 ;
	                ll -= 1 ;

	            } /* end if */

	        } /* end while (processing the quoted portion) */

	    } else if ((ch == '\\') && (ll > 1)) {

	        lp += 1 ;
	        ll -= 1 ;
		ch = CLEANCHAR(*lp) ;
	        if (flen > 0) {
	            *bp++ = ch ;
	            flen -= 1 ;
	        }

	        lp += 1 ;
	        ll -= 1 ;

	    } else {

	        if (flen > 0) {
	            *bp++ = ch ;
	            flen -= 1 ;
	        }

	        lp += 1 ;
	        ll -= 1 ;

	    } /* end if */

	} /* end while (main loop) */

/* do the terminator processing */

	while ((ll > 0) && CHAR_ISWHITE(*lp)) {
	    lp += 1 ;
	    ll -= 1 ;
	} /* end while */

/* we are at the end */

	term = ' ' ;
	ch = CLEANCHAR(*lp) ;
	if (BATST(terms,ch) && (! BATST(dquote,ch))) {
	    term = ch ;			/* save terminator */
	    lp += 1 ;
	    ll -= 1 ;			/* skip over the terminator */
	} /* end if */

	fl = (bp - ((unsigned char *) fbuf)) ;

	} /* end if (positive) */

/* we are out of here! */

	fsbp->ll = ll ;
	fsbp->lp = lp ;
	fsbp->fl = fl ;
	fsbp->fp = (const uchar *) fbuf ;
	fsbp->term = term ;

	return fl ;			/* return length of field */
}
/* end subroutine (field_wordphrase) */


