/* field */

/* routine to parse a line into fields */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano

        This code module was originally written in C language but modeled
        somewhat from a prior VAX assembly language version of mine (circa 1980
        perhaps). This is why it looks so ugly!


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Notes:

        This is a classic, ported forward from the old VAX-11/70 days. This was
        translated from VAX assembly language.

	The arguments to this routine are:

	- address of return status block
	- address of terminator block

	The function return is:

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
#include	<localmisc.h>

#include	"field.h"


/* local defines */


/* local variables */

/* quote characters '\"' and '\'' */
static const unsigned char	quotes[] = {
	0x00, 0x00, 0x00, 0x00,
	0x84, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

#ifdef	COMMENT

/* shell characters */
static const unsigned char	metas[] = {
	0x00, 0x02, 0x00, 0x00,
	0x41, 0x03, 0x00, 0x58,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x10,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

#endif /* COMMENT */

/* 'double quote', 'back slash', 'pound', 'back accent', et cetera */
static const unsigned char	doubles[] = {
	0x00, 0x00, 0x00, 0x00,
	0x14, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x10,
	0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

/* default parse terminators */
static const unsigned char	dterms[] = {
	0x00, 0x04, 0x00, 0x00,
	0x7A, 0x10, 0x00, 0x7C,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
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


/* helper function to build an array of field terminator characters */
int fieldterms(uchar *terms,int f_retain,cchar *s)
{
	int		i ;
	int		c = 0 ;

	if (terms == NULL) return SR_FAULT ;

	if (! f_retain) {
	    for (i = 0 ; i < 32 ; i += 1) {
		terms[i] = '\0' ;
	    }
	} /* end if */

	while (*s) {
	    BASET(terms,MKCHAR(*s)) ;
	    s += 1 ;
	    c += 1 ;
	} /* end while */

	return c ;
}
/* end subroutine (fieldterms) */


/* featured object */


/* initialize a field status block with a buffer and buffer length */
int field_start(FIELD *fsbp,cchar *lp,int ll)
{

	if (fsbp == NULL) return SR_FAULT ;

	fsbp->fp = NULL ;
	fsbp->fl = 0 ;
	fsbp->term = '\0' ;
	if (lp == NULL) {
	    fsbp->lp = NULL ;
	    fsbp->ll = -1 ;
	    return SR_FAULT ;
	}

	if (ll < 0)
	    ll = strlen(lp) ;

/* remove leading white space */

	while ((ll > 0) && CHAR_ISWHITE(*lp)) {
	    lp += 1 ;
	    ll -= 1 ;
	} /* end while */

	if (*lp == '\0') 
	    ll = 0 ;

	fsbp->lp = (const uchar *) lp ;
	fsbp->ll = ll ;
	return ll ;
}
/* end subroutine (field_start) */


int field_finish(FIELD *fsbp)
{

	if (fsbp == NULL) return SR_FAULT ;

	if (fsbp->lp == NULL) return SR_NOTOPEN ;

	fsbp->ll = 0 ;
	return SR_OK ;
}
/* end subroutine (field_finish) */


/* get the next field in this buffer */
int field_get(FIELD *fsbp,const uchar *terms,cchar **fpp)
{
	uint		ch ;
	uint		qe ;
	int		ll, fl ;
	int		term = '\0' ;
	const uchar	*lp, *fp ;

	if (fsbp == NULL) return SR_FAULT ;

	if (fsbp->lp == NULL) return SR_NOTOPEN ;

/* get the parameters */

	lp = fsbp->lp ;
	ll = fsbp->ll ;

	if (terms == NULL)
	    terms = dterms ;

/* skip all initial white space */

	while ((ll > 0) && CHAR_ISWHITE(*lp)) {
	    lp += 1 ;
	    ll -= 1 ;
	} /* end while */

	fp = lp ;
	fl = -1 ;
	if (ll <= 0)
	    goto done ;

	fl = 0 ;

/* the character is not blank space -- is it a quote? */

	ch = MKCHAR(*lp) ;
	if (BATST(quotes,ch)) {

/* it is a quote character, so we prepare to extract it */

	    qe = ch ;			/* set default quote end */
	    lp += 1 ;
	    ll -= 1 ;			/* skip over quote character */
	    fp = lp ;	 		/* save field address */

	    while ((ll > 0) && ((ch = MKCHAR(*lp)) != qe)) {
	        lp += 1 ;
	        ll -= 1 ;
	    }

	    fl = (lp - fp) ;
	    lp += 1 ;
	    ll -= 1 ;			/* skip over quote character */

	} else if (! BATST(terms,ch)) {

	    fp = lp ;	 		/* save field address */
	    lp += 1 ;
	    ll -= 1 ;
	    while (ll > 0) {

	        ch = MKCHAR(*lp) ;
	        if (BATST(terms,ch) || BATST(quotes,ch))
	            break ;

	        if (CHAR_ISWHITE(ch))
	            break ;

	        lp += 1 ;
	        ll -= 1 ;

	    } /* end while */

	    fl = (lp - fp) ;

	} /* end if (processing a field) */

	if ((ll > 0) && CHAR_ISWHITE(*lp)) {

	    term = ' ' ;
	    while ((ll > 0) && CHAR_ISWHITE(*lp)) {
	        lp += 1 ;
	        ll -= 1 ;
	    } /* end while */

	} /* end if */

	ch = MKCHAR(*lp) ;
	if ((ll > 0) && BATST(terms,ch) && (! BATST(quotes,ch))) {

	    term = ch ;
	    lp += 1 ;
	    ll -= 1 ;

	} /* end if */

done:
	fsbp->ll = ll ;
	fsbp->lp = lp ;
	fsbp->fl = fl ;
	fsbp->fp = (fl >= 0) ? fp : NULL ;
	fsbp->term = term ;

	if (fpp != NULL)
	    *fpp = (const char *) fsbp->fp ;

	return fl ;			/* return length of field */
}
/* end subroutine (field_get) */


/* get everything up to the next terminator */
int field_term(FIELD *fsbp,const uchar *terms,cchar **fpp)
{
	uint		ch ;
	int		ll, fl ;
	int		term = '\0' ;
	const uchar	*lp, *fp ;

	if (fsbp == NULL) return SR_FAULT ;

	if (fsbp->lp == NULL) return SR_NOTOPEN ;

/* get the parameters */

	lp = fsbp->lp ;
	ll = fsbp->ll ;

	fp = lp ;
	if (terms == NULL)
	    terms = dterms ;

	fl = -1 ;
	if (ll <= 0)
	    goto done ;

	fl = 0 ;

	ch = MKCHAR(*lp) ;
	if (! BATST(terms,ch)) {

	    fp = lp ;	 		/* save field address */
	    lp += 1 ;
	    ll -= 1 ;
	    while (ll > 0) {

	        ch = MKCHAR(*lp) ;
	        if (BATST(terms,ch))
	            break ;

	        lp += 1 ;
	        ll -= 1 ;

	    } /* end while */

	    fl = (lp - fp) ;

	} /* end if (processing a field) */

	ch = MKCHAR(*lp) ;
	if ((ll > 0) && BATST(terms,ch)) {
	    term = ch ;
	    lp += 1 ;
	    ll -= 1 ;
	} /* end if */

done:
	fsbp->ll = ll ;
	fsbp->lp = lp ;
	fsbp->fl = fl ;
	fsbp->fp = (fl >= 0) ? fp : NULL ;
	fsbp->term = term ;

	if (fpp != NULL)
	    *fpp = (const char *) fsbp->fp ;

	return fl ;			/* return length of field */
}
/* end subroutine (field_term) */


/* get the next SHELL argument field in this buffer */

/****

	Arguments:

	fsbp		field status block pointer
	terms		bit array of terminating characters
	fbuf		buffer to store result
	flen		length of buffer to hold result

****/

int field_sharg(FIELD *fsbp,const uchar *terms,char *fbuf,int flen)
{
	uint		ch ;
	int		fl = SR_NOENT ; /* end-of-arguments */
	int		ll ;
	int		qe ;
	int		nch ;
	int		term = '\0' ;
	const uchar	*lp ;
	uchar		*bp = (uchar *) fbuf ;

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

	if (ll <= 0) 
	    goto done ;

/* process the standard SHELL string */

	while (ll > 0) {

	    ch = MKCHAR(*lp) ;
	    if (BATST(terms,ch) & (! BATST(quotes,ch))) 
		break ;

	    if (CHAR_ISWHITE(ch)) 
		break ;

	    if (ch == '\"') {

	        qe = ch ;
		lp += 1 ;
	        ll -= 1 ;
	        while (ll > 0) {

		    ch = MKCHAR(*lp) ;
		    if (ll > 1)
			nch = MKCHAR(lp[1]) ;

	            if ((ch == '\\') && (ll > 1) && BATST(doubles,nch)) {

	                lp += 1 ;
	                ll -= 1 ;
		        ch = MKCHAR(*lp) ;
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

		    ch = MKCHAR(*lp) ;
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
		ch = MKCHAR(*lp) ;
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
	ch = MKCHAR(*lp) ;
	if (BATST(terms,ch) && (! BATST(quotes,ch))) {
	    term = ch ;			/* save terminator */
	    lp += 1 ;
	    ll -= 1 ;			/* skip over the terminator */
	} /* end if */

	fl = (bp - ((unsigned char *) fbuf)) ;

/* we are out of here! */
done:
	*bp = '\0' ;

	fsbp->ll = ll ;
	fsbp->lp = lp ;
	fsbp->fl = fl ;
	fsbp->fp = (const uchar *) fbuf ;
	fsbp->term = term ;

#if	CF_DEBUGS
	debugprintf("field_sharg: ret rs=%d\n",fl) ;
#endif

	return fl ;
}
/* end subroutine (field_sharg) */


int field_remaining(FIELD *fsbp,cchar **lpp)
{

	if (fsbp == NULL) return SR_FAULT ;

	if (fsbp->lp == NULL) return SR_NOTOPEN ;

	if (lpp != NULL) {
	    *lpp = (const char *) fsbp->lp ;
	}

	return fsbp->ll ;
}
/* end subroutine (field_remaining) */


