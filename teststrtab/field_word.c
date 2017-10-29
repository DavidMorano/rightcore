/* field_word */

/* routine to parse a line into words (word field) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2002-05-01, David A­D­ Morano

	This was created (from 'field_get()') to deal with parsing
	keys that are supposed to be English words in prose.


*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine finds the next "word" in the buffer.  This is
	different than 'field_get()' since no default quoting is assumed.

	Synopsis:

	int field_word(fsbp,terms,fpp)
	FIELD		*fsbp ;
	const uchar	terms[] ;
	const char	*fpp ;

	Arguments:

	fsbp	address of return status block
	terms	address of terminator block
	fpp	pointer to result pointer

	Returns:

	>=0	length of field just parsed out
	<0	invalid field block pointer was passwd

	The return status block outputs are:

	- length remaining in string
	- address of reminaing string
	- len of substring
	- address of substring
	- terminator character


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<char.h>
#include	<field.h>
#include	<localmisc.h>


/* local defines */

#undef	COMMENT

#define	CLEANCHAR(c)	((c) & 0xff)


/* local variables */

static const unsigned char	wterms[] = {
	0x00, 0x1A, 0x00, 0x00,
	0x3F, 0x40, 0x00, 0x7C,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int field_word(FIELD *fsbp,const uchar terms[],cchar **fpp)
{
	int		ll, fl ;
	int		ch ;
	const uchar	*lp, *fp ;
	uchar		term = '\0' ;

	if (fsbp == NULL)
	    return SR_FAULT ;

/* get the parameters */

	lp = fsbp->lp ;
	ll = fsbp->ll ;

	if (terms == NULL)
	    terms = wterms ;

/* skip all initial white space */

	while ((ll > 0) && CHAR_ISWHITE(*lp)) {
	    lp += 1 ;
	    ll -= 1 ;
	} /* end while */

	fp = NULL ;
	fl = -1 ;
	if (ll <= 0)
	    goto done ;

	fl = 0 ;
	ch = CLEANCHAR(*lp) ;
	if (! BATST(terms,ch)) {

	    fp = lp ;	 		/* save field address */
	    while (ll > 0) {

		ch = CLEANCHAR(*lp) ;
	        if (BATST(terms,ch))
	            break ;

	        if (CHAR_ISWHITE(ch))
	            break ;

	        lp += 1 ;
	        ll -= 1 ;

	    } /* end while */

	    fl = (lp - fp) ;

	    if ((ll > 0) && CHAR_ISWHITE(*lp)) {

	        term = ' ' ;
	        while ((ll > 0) && CHAR_ISWHITE(*lp)) {
	            lp += 1 ;
	            ll -= 1 ;
	        } /* end while */

	    } /* end if */

	} /* end if (processing a field) */

	ch = CLEANCHAR(*lp) ;
	if ((ll > 0) && BATST(terms,ch)) {
	    term = ch ;
	    lp += 1 ;
	    ll -= 1 ;
	} /* end if */

/* update the return status block */
done:
	fsbp->lp = lp ;
	fsbp->ll = ll ;
	fsbp->fp = (fl >= 0) ? ((const uchar *) fp) : NULL ;
	fsbp->fl = fl ;
	fsbp->term = term ;

	if (fpp != NULL)
	    *fpp = (const char *) fsbp->fp ;

	return fl ;
}
/* end subroutine (field_word) */


