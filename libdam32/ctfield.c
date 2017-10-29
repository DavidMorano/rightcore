/* ctfield */

/* subroutines to parse a line into ctfields */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
        This code module was originally written in C language copied (roughly)
        from a prior VAX® assembly language version (circa 1980 perhaps). This
        is why is looks so "ugly"!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	The arguments to this routine are:

	- address of return status block
	- address of terminator block


	The function return is:
	<0	invalid ctfield block pointer was passwd
	>=0	length of ctfield just parsed out (length of CTFIELD!)


	The return status block outputs are:

	- length remaining in string
	- address of reminaing string
	- len of substring
	- address of substring
	- terminator character


*******************************************************************************/


#define	CTFIELD_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<baops.h>
#include	<char.h>
#include	<localmisc.h>

#include	"ctfield.h"


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


/* shell characters */

#ifdef	COMMENT

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


/* helper function to build an array of ctfield terminator characters */
int ctfieldterms(uchar *terms,int f_retain,cchar *s)
{
	int		i ;

	if (terms == NULL)
	    return SR_FAULT ;

	if (! f_retain)  {
	    for (i = 0 ; i < 32 ; i += 1)  {
	        terms[i] = '\0' ;
	    }
	}

	while (*s) {
	    BASET(terms,(*s & 0xFF)) ;
	    s += 1 ;
	} /* end while */

	return SR_OK ;
}
/* end subroutine (ctfieldterms) */


/* ACTUAL OBJECT */


/* initialize a CTFIELD status block with a buffer and buffer length */
int ctfield_start(CTFIELD *fsbp,cchar *lp,int ll)
{

	if (fsbp == NULL) return SR_FAULT ;

	fsbp->fp = NULL ;
	fsbp->flen = 0 ;
	fsbp->term = 0 ;		/* is this the best idea?? */
	if (lp == NULL) {
	    fsbp->lp = NULL ;
	    fsbp->rlen = -1 ;
	    return SR_FAULT ;
	}

	if (ll < 0)
	    ll = strlen(lp) ;

/* remove leading white space */

	while ((ll > 0) && isspace(*lp)) {
	    lp += 1 ;
	    ll -= 1 ;
	} /* end while */

	if (*lp == '\0') ll = 0 ;

	fsbp->lp = (char *) lp ;
	fsbp->rlen = ll ;
	return ll ;
}
/* end subroutine (ctfield_start) */


/* get the next CTFIELD in this buffer */
int ctfield_get(CTFIELD *fsbp,const uchar *terms)
{
	int		lr ;
	int		c ;
	int		qe, term ;
	int		flen = 0 ;
	unsigned char	*sp, *sap ;

	if (fsbp == NULL) return SR_FAULT ;

	if (fsbp->lp == NULL) return SR_NOTOPEN ;

/* get the parameters */

	lr = fsbp->rlen ;
	sp = (unsigned char *) fsbp->lp ;

	if (terms == NULL)
	    terms = (uchar *) dterms ;

/* skip all initial white space */

	while ((lr > 0) && CHAR_ISWHITE(*sp & 0xFF)) {
	    sp += 1 ;
	    lr -= 1 ;
	} /* end while */

	sap = sp ;
	flen = -1 ;
	term = 0 ;
	if (lr > 0) {

	flen = 0 ;

/* the character is not blank space -- is it a quote? */

	c = *sp & 0xFF ;
	if (BATST(quotes,c)) {

/* it is a quote character, so we prepare to extract it */

#if	CF_DEBUGS
	    debugprintf("ctfield_get: found initial quote c=>%c<\n",c) ;
#endif

	    qe = c ;			/* set default quote end */
	    sp += 1 ;
	    lr -= 1 ;			/* skip over quote character */
	    sap = sp ;	 		/* save ctfield address */

#if	CF_DEBUGS
	    debugprintf("ctfield_get: lr=%d qe=>%c<\n",lr,qe) ;
#endif

	    while ((lr > 0) && ((c = (*sp & 0xFF)) != qe)) {
	        sp += 1 ;
	        lr -= 1 ;
	    }

	    flen = sp - sap ;
	    sp += 1 ;
	    lr -= 1 ;			/* skip over quote character */

	} else if (! BATST(terms,c)) {

#if	CF_DEBUGS
	    debugprintf("ctfield_get: initial non-white c=>%c<\n",c) ;
#endif

	    sap = sp++ ;	 	/* save ctfield address */
	    lr -= 1 ;
	    while (lr > 0) {

	        c = *sp & 0xFF ;

#if	CF_DEBUGS
	        debugprintf("ctfield_get: non-white c=>%c<\n",c) ;
#endif

	        if (BATST(terms,c) || BATST(quotes,c))
	            break ;

	        if (CHAR_ISWHITE(c))
	            break ;

	        sp += 1 ;
	        lr -= 1 ;

	    } /* end while */

	    flen = sp - sap ;

#if	CF_DEBUGS
	    debugprintf("ctfield_get: non-white flen=%d\n",flen) ;
#endif

	} /* end if (processing a ctfield) */

	if ((lr > 0) && CHAR_ISWHITE(*sp)) {

	    term = ' ' ;
	    while ((lr > 0) && CHAR_ISWHITE(*sp)) {
	        sp += 1 ;
	        lr -= 1 ;
	    } /* end while */

	} /* end if */

	c = *sp & 0xFF ;

#if	CF_DEBUGS
	debugprintf("ctfield_get: possible term, rlen=%d c=>%c<\n",lr,c) ;
#endif

	if ((lr > 0) && BATST(terms,c) && (! BATST(quotes,c))) {

	    term = c ;
	    sp += 1 ;
	    lr -= 1 ;

	} /* end if */

	} /* end if (postiive) */

	fsbp->rlen = lr ;		/* load length remaining in string */
	fsbp->lp = (char *) sp ;	/* address of remaining string */
	fsbp->flen = flen ;		/* length of ctfield */
	fsbp->fp = (flen >= 0) ? ((char *) sap) : NULL ;
	fsbp->term = term ;		/* terminating character */

#if	CF_DEBUGS
	debugprintf("ctfield_get: ret rlen=%d flen=%d\n",lr,flen) ;
#endif

	return flen ;			/* return length of ctfield */
}
/* end subroutine (ctfield_get) */


/* get everything up to the next terminator */
int ctfield_term(CTFIELD *fsbp,const uchar *terms)
{
	int		lr, flen ;
	int		c ;
	int		term ;
	unsigned char	*sp, *sap ;

	if (fsbp == NULL) return SR_FAULT ;

	if (fsbp->lp == NULL) return SR_NOTOPEN ;

/* get the parameters */

	lr = fsbp->rlen ;
	sp = (unsigned char *) fsbp->lp ;

	sap = sp ;
	if (terms == NULL)
	    terms = (uchar *) dterms ;

	term = 0 ;
	flen = -1 ;
	if (lr > 0) {

	flen = 0 ;

	c = *sp & 255 ;
	if (! BATST(terms,c)) {

#if	CF_DEBUGS
	    debugprintf("ctfield_term: initial c=>%c<\n",c) ;
#endif

	    sap = sp++ ;	 		/* save ctfield address */
	    lr -= 1 ;
	    while (lr > 0) {

	        c = *sp & 0xFF ;

#if	CF_DEBUGS
	        debugprintf("ctfield_term: non-white c=>%c<\n",c) ;
#endif

	        if (BATST(terms,c))
	            break ;

	        sp += 1 ;
	        lr -= 1 ;

	    } /* end while */

	    flen = sp - sap ;

#if	CF_DEBUGS
	    debugprintf("ctfield_term: non-white flen=%d\n",flen) ;
#endif

	} /* end if (processing a ctfield) */

	c = *sp & 0xFF ;

#if	CF_DEBUGS
	debugprintf("ctfield_term: possible term, rlen=%d c=>%c<\n",lr,c) ;
#endif

	if ((lr > 0) && BATST(terms,c)) {

	    term = c ;
	    sp += 1 ;
	    lr -= 1 ;

	} /* end if */

	} /* end if (positive) */

	fsbp->rlen = lr ;		/* load length remaining in string */
	fsbp->lp = (char *) sp ;	/* address of remaining string */
	fsbp->flen = flen ;		/* length of ctfield */
	fsbp->fp = (flen >= 0) ? ((char *) sap) : NULL ;
	fsbp->term = term ;		/* terminating character */

#if	CF_DEBUGS
	debugprintf("ctfield_term: ret rlen=%d flen=%d\n",lr,flen) ;
#endif

	return flen ;			/* return length of ctfield */
}
/* end subroutine (ctfield_term) */


/* get the next SHELL argument ctfield in this buffer */

/*
	Arguments:
	fsbp		ctfield status block pointer
	terms		bit array of terminating characters
	buf		buffer to store result
	buflen		length of buffer to hold result

*/

int ctfield_sharg(CTFIELD *fsbp,const uchar *terms,char *buf,int buflen)
{
	int		lr, flen ;
	int		qe, term ;
	unsigned char	*cp ;
	unsigned char	*bp = (unsigned char *) buf ;

	if (fsbp == NULL) return SR_FAULT ;

	if (fsbp->lp == NULL) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("ctfield_sharg: ent, buflen=%d\n",buflen) ;
	debugprintf("ctfield_sharg: fbuf=>%t<\n",fsbp->lp,fsbp->rlen) ;
#endif

/* get the parameters */

	lr = fsbp->rlen ;
	cp = (unsigned char *) fsbp->lp ;

	if (terms == NULL)
	    terms = (uchar *) shterms ;

/* skip all initial white space */

	while ((lr > 0) && CHAR_ISWHITE(*cp)) {
	    lr -= 1 ;
	    cp += 1 ;
	}

	flen = -1 ;			/* end-of-arguments indicator */
	term = 0 ;
	if (lr > 0) {

/* process the standard SHELL string */

	while (lr > 0) {

	    if (BATST(terms,*cp) & (! BATST(quotes,*cp)))
	        break ;

	    if (CHAR_ISWHITE(*cp))
	        break ;

#if	CF_DEBUGS
	    debugprintf("ctfield_sharg: outer character =%c=\n",*cp) ;
#endif

	    if (*cp == '\"') {

	        qe = *cp++ ;
	        lr -= 1 ;
	        while (lr > 0) {

	            if ((*cp == '\\') && (lr > 1) && BATST(doubles,cp[1])) {

	                cp += 1 ;
	                lr -= 1 ;
	                if (buflen > 0) {
	                    *bp++ = *cp ;
	                    buflen -= 1 ;
	                }

	                cp += 1 ;
	                lr -= 1 ;

	            } else if (*cp == qe) {

	                cp += 1 ;
	                lr -= 1 ;
	                break ;

	            } else {

	                if (buflen > 0) {

	                    *bp++ = *cp ;
	                    buflen -= 1 ;
	                }

	                cp += 1 ;
	                lr -= 1 ;

	            } /* end if */

	        } /* end while (processing the quoted portion) */

	    } else if (*cp == '\'') {

	        qe = *cp++ ;
	        lr -= 1 ;
	        while (lr > 0) {

	            if (*cp == qe) {
	                cp += 1 ;
	                lr -= 1 ;
	                break ;
	            } else {
	                if (buflen > 0) {
	                    *bp++ = *cp ;
	                    buflen -= 1 ;
	                }
	                cp += 1 ;
	                lr -= 1 ;
	            }

	        } /* end while (processing the quoted portion) */

	    } else if ((*cp == '\\') && (lr > 1)) {

	        cp += 1 ;
	        lr -= 1 ;
	        if (buflen > 0) {
	            *bp++ = *cp ;
	            buflen -= 1 ;
	        }

	        cp += 1 ;
	        lr -= 1 ;

	    } else {

	        if (buflen > 0) {
	            *bp++ = *cp ;
	            buflen -= 1 ;
	        }

	        cp += 1 ;
	        lr -= 1 ;
	    }

	} /* end while (main loop) */

/* do the terminator processing */

	while ((lr > 0) && CHAR_ISWHITE(*cp)) {
	    cp += 1 ;
	    lr -= 1 ;
	} /* end while */

/* we are at the end */

	term = ' ' ;
	if (BATST(terms,*cp) && (! BATST(quotes,*cp))) {

	    term = *cp ;		/* save terminator */
	    lr -= 1 ;			/* skip over the terminator */
	    cp += 1 ;

	} /* end if */

	flen = (bp - ((unsigned char *) buf)) ;

	} /* end if (positive) */

	fsbp->rlen = lr ;		/* load length remaining in string */
	fsbp->lp = (char *) cp ;	/* address of remaining string */
	fsbp->flen = flen ;
	fsbp->fp = (char *) buf ;	/* ctfield sub-string address */
	fsbp->term = term ;		/* terminating character */

#if	CF_DEBUGS
	debugprintf("ctfield_sharg: done rlen=%d flen=%d term=%c\n",
	    lr,fsbp->flen,term) ;
#endif

	return flen ;			/* return length of ctfield */
}
/* end subroutine (ctfield_sharg) */


int ctfield_finish(CTFIELD *fsbp)
{

	if (fsbp == NULL) return SR_FAULT ;

	if (fsbp->lp == NULL) return SR_NOTOPEN ;

	return SR_OK ;
}
/* end subroutine (ctfield_finish) */


