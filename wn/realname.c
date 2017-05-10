/* realname */

/* manipulate real names */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-08-13, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This modules manipulates "real names" (that is like the real name of a
        person) in various ways.


******************************************************************************/


#define	REALNAME_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<sbuf.h>
#include	<storeitem.h>
#include	<dstr.h>
#include	<localmisc.h>

#include	"realname.h"


/* local defines */

#define	NAMES	struct names

#define	NAMESTR	struct namestr


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	snwcpylc(char *,int,cchar *,int) ;
extern int	snwcpyuc(char *,int,cchar *,int) ;
extern int	sibreak(cchar *,int,cchar *) ;
extern int	tolc(int) ;
extern int	touc(int) ;
extern int	islowerlatin(int) ;
extern int	isupperlatin(int) ;


/* local structures */

struct names {
	STOREITEM	s ;
	cchar		*a[REALNAME_NNAMES] ;
	int		li ;
	int		i, count ;
	uchar		l[REALNAME_NNAMES] ;
	uchar		f_abv[REALNAME_NNAMES] ;
} ;

struct namestr {
	cchar		*s ;
	int		slen ;
} ;


/* forward references */

int		realname_startparse(REALNAME *,cchar *,int) ;

static int	names_start(NAMES *,char *) ;
static int	names_add(NAMES *,cchar *,int,int,int) ;
static int	names_finish(NAMES *) ;

static int	namestr_start(NAMESTR *,cchar *,int) ;
static int	namestr_next(NAMESTR *,cchar **,int *,int *) ;
static int	namestr_skipwhite() ;
static int	namestr_break(NAMESTR *,cchar *,cchar **) ;
static int	namestr_finish(NAMESTR *) ;

static int	isAbbr(int) ;


/* local variables */


/* exported subroutines */


int realname_start(REALNAME *rnp,cchar *sbuf,int slen)
{
	int		rs ;
	rs = realname_startparse(rnp,sbuf,slen) ;
	return rs ;
}
/* end subroutine (realname_start) */


int realname_startparts(REALNAME *rnp,DSTR *stp)
{
	STOREITEM	s ;
	const int	slen = REALNAME_STORELEN ;
	int		rs ;
	int		rs1 ;

	if (rnp == NULL) return SR_FAULT ;
	if (stp == NULL) return SR_FAULT ;

	memset(rnp,0,sizeof(REALNAME)) ;

	if ((rs = storeitem_start(&s,rnp->store,slen)) >= 0) {
	    int		cl ;
	    cchar	*cp ;

/* first */

	    if (stp[0].sbuf != NULL) {
	        cl = storeitem_strw(&s,stp[0].sbuf,stp[0].slen,&cp) ;
	        rnp->first = cp ;
	        rnp->len.first = (uchar) cl ;
	    }

/* middle-1 */

	    if (stp[1].sbuf != NULL) {
	        cl = storeitem_strw(&s,stp[1].sbuf,stp[1].slen,&cp) ;
	        rnp->m1 = cp ;
	        rnp->len.m1 = (uchar) cl ;
	    }

/* middle-2 */

	    if (stp[2].sbuf != NULL) {
	        cl = storeitem_strw(&s,stp[2].sbuf,stp[2].slen,&cp) ;
	        rnp->m2 = cp ;
	        rnp->len.m2 = (uchar) cl ;
	    }

/* middle-3 */

	    if (stp[3].sbuf != NULL) {
	        cl = storeitem_strw(&s,stp[3].sbuf,stp[3].slen,&cp) ;
	        rnp->m3 = cp ;
	        rnp->len.m3 = (uchar) cl ;
	    }

/* last */

	    if (stp[4].sbuf != NULL) {
	        cl = storeitem_strw(&s,stp[4].sbuf,stp[4].slen,&cp) ;
	        rnp->last = cp ;
	        rnp->len.last = (uchar) cl ;
	    }

/* done */

	    rs1 = storeitem_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (storeitem) */

	rnp->len.store = (uchar) rs ;
	return rs ;
}
/* end subroutine (realname_startparts) */


int realname_startpieces(REALNAME *rnp,cchar *sa[],int sn)
{
	STOREITEM	s ;
	const int	slen = REALNAME_STORELEN ;
	const int	nparts = REALNAME_NPARTS ;
	int		rs ;
	int		c = 0 ;
	cchar		*pieces[REALNAME_NPARTS] ;

	if (rnp == NULL) return SR_FAULT ;
	if (sa == NULL) return SR_FAULT ;

	if (sn < 0) sn = INT_MAX ;

	{
	    int	i ;
	    for (i = 0 ; ( i < sn) && sa[i] && (c < nparts) ; i += 1) {
	        if (sa[i][0] != '\0') {
	            pieces[c++] = sa[i] ;
	        }
	    } /* end for */
	}

	if (c == 0) return SR_INVALID ;

	memset(rnp,0,sizeof(REALNAME)) ;

	if ((rs = storeitem_start(&s,rnp->store,slen)) >= 0) {
	    cchar	*cp ;
	    int		len = 0 ;

	    if ((rs = storeitem_strw(&s,pieces[--c],-1,&cp)) >= 0) {
	        int	f_abv = FALSE ;
	        if (isAbbr(cp[rs-1])) {
	            f_abv = TRUE ;
	            rs -= 1 ;
	        }
	        rnp->last = cp ;
	        rnp->len.last = (uchar) rs ;
	        rnp->abv.last = f_abv ;
	        if (c > 0) {
	            int	i ;
	            int	n = 0 ;
	            for (i = 0 ; (rs >= 0) && (i < c) ; i += 1) {
	                f_abv = FALSE ;
	                if ((rs = storeitem_strw(&s,pieces[i],-1,&cp)) > 0) {
	                    if (isAbbr(cp[rs-1])) {
	                        f_abv = TRUE ;
	                        rs -= 1 ;
	                    }
	                    if (rs > 0) {
	                        switch (n++) {
	                        case 0:
	                            rnp->first = cp ;
	                            rnp->len.first = (uchar) rs ;
	                            rnp->abv.first = f_abv ;
	                            break ;
	                        case 1:
	                            rnp->m1 = cp ;
	                            rnp->len.m1 = (uchar) rs ;
	                            rnp->abv.m1 = f_abv ;
	                            break ;
	                        case 2:
	                            rnp->m2 = cp ;
	                            rnp->len.m2 = (uchar) rs ;
	                            rnp->abv.m2 = f_abv ;
	                            break ;
	                        case 3:
	                            rnp->m3 = cp ;
	                            rnp->len.m3 = (uchar) rs ;
	                            rnp->abv.m3 = f_abv ;
	                            break ;
	                        } /* end switch */
	                    } /* end if */
	                } /* end if */
	            } /* end for */
	        } /* end if */
	    } /* end if (last-name) */

	    len = storeitem_finish(&s) ;
	    if (rs >= 0) rs = len ;
	} /* end if (storeitem) */

	if (rs >= 0) rnp->len.store = (uchar) rs ;

	return rs ;
}
/* end subroutine (realname_startpieces) */


int realname_startparse(realname *rnp,cchar *sbuf,int slen)
{
	NAMES		n ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (rnp == NULL) return SR_FAULT ;
	if (sbuf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("realname_startparse: ent\n") ;
#endif

	memset(rnp,0,sizeof(struct realname_head)) ;

	if ((rs = names_start(&n,rnp->store)) >= 0) {
	    NAMESTR	ns ;
	    int		i, j ;
	    int		nl ;
	    int		f_abv, f_last ;
	    cchar	*np ;

/* start */

	    if ((rs = namestr_start(&ns,sbuf,slen)) >= 0) {

	        while ((nl = namestr_next(&ns,&np,&f_abv,&f_last)) >= 0) {

#if	CF_DEBUGS
	            debugprintf("realname_startparse: nl=%d part=%t\n",
	                nl,np,nl) ;
	            debugprintf("realname_startparse: abv=%d last=%d\n",
	                f_abv,f_last) ;
#endif

	            if (nl > 0) {
	                rs = names_add(&n,np,nl,f_abv,f_last) ;
	                if (rs < 0) break ;
	            }

	        } /* end while */

	        namestr_finish(&ns) ;
	    } /* end if (namestr) */

#if	CF_DEBUGS
	    {
	        debugprintf("realname_startparse: name parts\n") ;
	        i = (n.li + 1) % REALNAME_NNAMES ;
	        for (j = 0 ; j < REALNAME_NNAMES ; j += 1) {
	            if (n.a[i] != NULL) {
	                debugprintf("realname_startparse: %d part=%s\n",
	                    i,n.a[i]) ;
	            } else
	                debugprintf("realname_startparse: %d part=*\n") ;
	            i = (i + 1) % REALNAME_NNAMES ;
	        }
	    } /* end block */
#endif /* CF_DEBUGS */

	    if (rs >= 0) {

/* load up the information on the name parts, first the last name part */

	        if (n.li < 0) {
	            if (n.count >= REALNAME_NNAMES) {
	                i = n.i ;
	            } else
	                i = ((n.i + REALNAME_NNAMES - 1) % REALNAME_NNAMES) ;
	        } else
	            i = n.li ;

	        rnp->last = n.a[i] ;
	        rnp->len.last = n.l[i] ;
	        rnp->abv.last = n.f_abv[i] ;

	        n.a[i] = NULL ;

#if	CF_DEBUGS
	        debugprintf("realname_startparse: last=%s\n",
	            rnp->last) ;
#endif

	        i = (i + 1) % REALNAME_NNAMES ;

/* get the rest of the parts of this realname */

	        j = 0 ;
	        while (j < (n.count - 1)) {

	            if (n.a[i] != NULL) {
	                switch (j) {
	                case 0:
	                    rnp->first = n.a[i] ;
	                    rnp->len.first = n.l[i] ;
	                    rnp->abv.first = n.f_abv[i] ;
	                    break ;
	                case 1:
	                    rnp->m1 = n.a[i] ;
	                    rnp->len.m1 = n.l[i] ;
	                    rnp->abv.m1 = n.f_abv[i] ;
	                    break ;
	                case 2:
	                    rnp->m2 = n.a[i] ;
	                    rnp->len.m2 = n.l[i] ;
	                    rnp->abv.m2 = n.f_abv[i] ;
	                    break ;
	                case 3:
	                    rnp->m3 = n.a[i] ;
	                    rnp->len.m3 = n.l[i] ;
	                    rnp->abv.m3 = n.f_abv[i] ;
	                    break ;
	                } /* end switch */
	                j += 1 ;
	            } /* end if */

	            i = (i + 1) % REALNAME_NNAMES ;

	        } /* end while */

	    } /* end if (we had enough storage) */

	    rs1 = names_finish(&n) ;
	    if (rs >= 0) rs = rs1 ;
	    len = rs1 ;
	    rnp->len.store = (uchar) len ;
	} /* end if (names) */

#if	CF_DEBUGS
	debugprintf("realname_startparse: ret rs=%d last=%s storelen=%d\n", 
	    rs,rnp->last,rnp->len.store) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (realname_startparse) */


/* get the first */
int realname_getfirst(realname *rnp,cchar **rpp)
{
	if (rnp == NULL) return SR_FAULT ;
	if (rpp != NULL) *rpp = rnp->first ;
	return rnp->len.first ;
}
/* end subroutine (realname_getfirst) */


/* get m1 */
int realname_getm1(realname *rnp,cchar **rpp)
{
	if (rnp == NULL) return SR_FAULT ;
	if (rpp != NULL) *rpp = rnp->m1 ;
	return rnp->len.m1 ;
}
/* end subroutine (realname_getm1) */


/* get m2 */
int realname_getm2(realname *rnp,cchar **rpp)
{
	if (rnp == NULL) return SR_FAULT ;
	if (rpp != NULL) *rpp = rnp->m2 ;
	return rnp->len.m2 ;
}
/* end subroutine (realname_getm2) */


/* get m3 */
int realname_getm3(realname *rnp,cchar **rpp)
{
	if (rnp == NULL) return SR_FAULT ;
	if (rpp != NULL) *rpp = rnp->m3 ;
	return rnp->len.m3 ;
}
/* end subroutine (realname_getm3) */


/* get the last name */
int realname_getlast(realname *rnp,cchar **rpp)
{
	if (rnp == NULL) return SR_FAULT ;
	if (rpp != NULL) *rpp = rnp->last ;
	return rnp->len.last ;
}
/* end subroutine (realname_getlast) */


/* get parts last name */
int realname_getpieces(realname *rnp,cchar *parts[])
{
	int		c = 0 ;

	if (rnp == NULL) return SR_FAULT ;

	if (parts != NULL) {
	    int		i ;
	    cchar	*cp ;
	    for (i = 0 ; i < 5 ; i += 1) {
	        switch (i) {
	        case 0:
	            cp = rnp->first ;
	            break ;
	        case 1:
	            cp = rnp->m1 ;
	            break ;
	        case 2:
	            cp = rnp->m2 ;
	            break ;
	        case 3:
	            cp = rnp->m3 ;
	            break ;
	        case 4:
	            cp = rnp->last ;
	            break ;
	        } /* end switch */
	        if ((cp != NULL) && (cp[0] != '\0')) {
	            parts[c++] = cp ;
	        }
	    } /* end for */
	    parts[c] = NULL ;
	} /* end if */

	return c ;
}
/* end subroutine (realname_getpieces) */


/* create a "mailname" (used in the old days) */
int realname_mailname(realname *rnp,char *rbuf,int rlen)
{
	SBUF		s ;
	int		rs, rs1 ;

	if (rnp == NULL) return SR_FAULT ;

	if ((rs = sbuf_start(&s,rbuf,rlen)) >= 0) {
	    const int	nlen = REALNAME_STORELEN ;
	    int		len ;
	    int		nl ;
	    int		ch ;
	    int		f = FALSE ;
	    cchar	*np ;
	    char	nbuf[REALNAME_STORELEN + 1] ;

	    if ((rnp->first != NULL) && (rnp->first[0] != '\0')) {
	        f = TRUE ;
	        ch = tolc(rnp->first[0]) ;
	        sbuf_char(&s,ch) ;
	    }

	    if ((rnp->m1 != NULL) && (rnp->m1[0] != '\0')) {
	        sbuf_char(&s,'.') ;
	        ch = tolc(rnp->m1[0]) ;
	        sbuf_char(&s,ch) ;
	    }

	    if ((rnp->last != NULL) && (rnp->last[0] != '\0')) {
	        if (f) sbuf_char(&s,'.') ;
	        np = rnp->last ;
	        nl = rnp->len.last ;
	        if ((rs1 = snwcpylc(nbuf,nlen,np,nl)) > 0) {
	            np = nbuf ;
	            nl = rs1 ;
	        }
	        sbuf_strw(&s,np,nl) ;
	    } /* end if */

	    len = sbuf_finish(&s) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

	return rs ;
}
/* end subroutine (realname_mailname) */


/* create a "fullname" */
int realname_fullname(realname *rnp,char *rbuf,int rlen)
{
	SBUF		s ;
	int		rs ;

	if (rnp == NULL) return SR_FAULT ;

	if ((rs = sbuf_start(&s,rbuf,rlen)) >= 0) {
	    int	len ;
	    int	f = FALSE ;

	    if ((rnp->first != NULL) && (rnp->first[0] != '\0')) {
	        f = TRUE ;
	        sbuf_strw(&s,rnp->first,rnp->len.first) ;
	        if (rnp->abv.first) sbuf_char(&s,'.') ;
	    }

	    if ((rnp->m1 != NULL) && (rnp->m1[0] != '\0')) {
	        if (f) sbuf_char(&s,' ') ;
	        sbuf_strw(&s,rnp->m1,rnp->len.m1) ;
	        if (rnp->abv.m1) sbuf_char(&s,'.') ;
	    }

	    if ((rnp->m2 != NULL) && (rnp->m2[0] != '\0')) {
	        if (f) sbuf_char(&s,' ') ;
	        sbuf_strw(&s,rnp->m2,rnp->len.m2) ;
	        if (rnp->abv.m2) sbuf_char(&s,'.') ;
	    }

	    if ((rnp->m3 != NULL) && (rnp->m3[0] != '\0')) {
	        if (f) sbuf_char(&s,' ') ;
	        sbuf_strw(&s,rnp->m3,rnp->len.m3) ;
	        if (rnp->abv.m3) sbuf_char(&s,'.') ;
	    }

	    if ((rnp->last != NULL) && (rnp->last[0] != '\0')) {
	        if (f) sbuf_char(&s,' ') ;
	        sbuf_strw(&s,rnp->last,rnp->len.last) ;
	        if (rnp->abv.last) sbuf_char(&s,'.') ;
	    }

	    len = sbuf_finish(&s) ;
	    if (rs >= 0) rs = len ;
	} /* end if (buffer) */

	return rs ;
}
/* end subroutine (realname_fullname) */


/* create a "name" */

/****

	Oh boy!  We want to be careful with those foreign names that
	have funny article-preposition (whatever?) words in front of
	their last names.  Some examples, of these might be: a, da, 
	de, do, du, mc, mac, o, ou, von .

****/

int realname_name(realname *rnp,char *rbuf,int rlen)
{
	SBUF		s ;
	int		rs ;
	int		rs1 ;

	if (rnp == NULL) return SR_FAULT ;

	if ((rs = sbuf_start(&s,rbuf,rlen)) >= 0) {
	    const int	nlen = REALNAME_STORELEN ;
	    int		len ;
	    int		nl ;
	    int		ch, nch ;
	    int		f = FALSE ;
	    cchar	*np ;
	    char	nbuf[REALNAME_STORELEN + 1] ;

	    if ((rnp->first != NULL) && (rnp->first[0] != '\0')) {

#if	CF_DEBUGS
	        debugprintf("realname_name: first=%s\n",rnp->first) ;
#endif

	        f = TRUE ;
	        ch = touc(rnp->first[0]) ;

	        sbuf_char(&s,ch) ;

	        if (rnp->first[1] != '\0') {

	            np = (rnp->first + 1) ;
	            nl = (rnp->len.first - 1) ;
	            ch = MKCHAR(*np) ;
	            if (isupperlatin(ch)) {
	                if ((rs1 = snwcpylc(nbuf,nlen,np,nl)) > 0) {
	                    np = nbuf ;
	                    nl = rs1 ;
	                }
	            } /* end if */

	            sbuf_strw(&s,np,nl) ;

	        } /* end if */

	        if (rnp->abv.first)
	            sbuf_char(&s,'.') ;

	    } /* end if (first name) */

	    if ((rnp->m1 != NULL) && (rnp->m1[0] != '\0')) {

#if	CF_DEBUGS
	        debugprintf("realname_name: m1=%s\n",rnp->m1) ;
#endif

	        if (f)
	            sbuf_char(&s,' ') ;

	        f = TRUE ;
	        ch = MKCHAR(rnp->m1[0]) ;
	        nch = (rnp->m1[1] != '\0') ? touc(ch) : ch ;

	        sbuf_char(&s,nch) ;

	        if (rnp->abv.m1 || isupperlatin(ch))
	            sbuf_char(&s,'.') ;

	    } /* end if (middle name) */

	    if ((rnp->last != NULL) && (rnp->last[0] != '\0')) {

#if	CF_DEBUGS
	        debugprintf("realname_name: last=%s\n",rnp->last) ;
#endif

	        if (f)
	            sbuf_char(&s,' ') ;

	        ch = touc(rnp->last[0]) ;

	        sbuf_char(&s,ch) ;

	        if (rnp->last[1] != '\0') {

	            np = (rnp->last + 1) ;
	            nl = (rnp->len.last - 1) ;
	            ch = MKCHAR(*np) ;
	            if (f && isupperlatin(ch)) {
	                if ((rs1 = snwcpylc(nbuf,nlen,np,nl)) > 0) {
	                    np = nbuf ;
	                    nl = rs1 ;
	                }
	            } /* end if */

	            sbuf_strw(&s,np,nl) ;

	        } /* end if */

	        if (rnp->abv.last)
	            sbuf_char(&s,'.') ;

	    } /* end if (last name) */

	    len = sbuf_finish(&s) ;
	    if (rs >= 0) rs = len ;
	} /* end if (buffer) */

#if	CF_DEBUGS
	debugprintf("realname_name: ret rs=%d name=>%s<\n",rs,rbuf) ;
#endif

	return rs ;
}
/* end subroutine (realname_name) */


int realname_finish(realname *rnp)
{
	if (rnp == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (realname_finish) */


/* private subroutines */


static int namestr_start(NAMESTR *sp,cchar *sbuf,int slen)
{

#if	CF_DEBUGS
	debugprintf("realname/namestr_start: slen=%d sbuf=>%t<\n",
	    slen,sbuf,strnlen(sbuf,slen)) ;
#endif

	sp->s = sbuf ;

#ifdef	COMMENT
	sp->slen = (slen >= 0) ? slen : strlen(sbuf) ;
#else
	sp->slen = strnlen(sbuf,slen) ;
#endif

	return OK ;
}
/* end subroutine (namestr_start) */


static int namestr_finish(NAMESTR *sp)
{

	sp->slen = 0 ;
	return OK ;
}
/* end subroutine (namestr_finish) */


static int namestr_skipwhite(NAMESTR *sp)
{

	while ((sp->slen > 0) && sp->s[0] && CHAR_ISWHITE(sp->s[0])) {
	    sp->s += 1 ;
	    sp->slen -= 1 ;
	}

	return sp->slen ;
}
/* end subroutine (namestr_skipwhite) */


static int namestr_break(NAMESTR *nsp,cchar *bs,cchar **rpp)
{
	const int	si = sibreak(nsp->s,nsp->slen,bs) ;
	*rpp = (si >= 0) ? (nsp->s+si) : NULL ;
	return si ;
}
/* end subroutine (namestr_break) */


static int namestr_next(NAMESTR *nsp,cchar **npp,int *fap,int *flp)
{
	int		i ;
	int		nlen = 0 ;
	cchar		*np, *cp ;

	*fap = *flp = FALSE ;
	namestr_skipwhite(nsp) ;

	if (nsp->slen <= 0)
	    return SR_EOF ;

	if ((i = namestr_break(nsp," \t.,­",&cp)) >= 0) {

	    np = nsp->s ;
	    nlen = i ;
	    nsp->s += nlen ;
	    nsp->slen -= nlen ;

	    namestr_skipwhite(nsp) ;

	    if ((nsp->slen > 0) && ((nsp->s[0] == ',') || isAbbr(nsp->s[0]))) {

	        if (isAbbr(nsp->s[0])) {
	            *fap = TRUE ;
	        } else {
	            *flp = TRUE ;
		}

	        nsp->s += 1 ;
	        nsp->slen -= 1 ;

	    } /* end if */

	    namestr_skipwhite(nsp) ;

	    if ((! *flp) && (nsp->slen > 0) && (nsp->s[0] == ',')) {

	        *flp = TRUE ;
	        nsp->s += 1 ;
	        nsp->slen -= 1 ;

	    } /* end if */

	    namestr_skipwhite(nsp) ;

/* eat any weirdo characters that are here */

	    while ((nsp->slen > 0) && 
	        ((nsp->s[0] == ',') || (nsp->s[0] == '.'))) {
	        nsp->s += 1 ;
	        nsp->slen -= 1 ;
	    }

	} else {

	    np = nsp->s ;
	    nlen = (nsp->slen > 0) ? nsp->slen : SR_EOF ;
	    nsp->s += nlen ;
	    nsp->slen = 0 ;

	} /* end if */

	if (npp != NULL)
	    *npp = (nlen >= 0) ? np : NULL ;

	return nlen ;
}
/* end subroutine (namestr_next) */


static int names_start(NAMES *np,char *rbuf)
{
	const int	rlen = REALNAME_STORELEN ;
	int		rs ;

	if ((rs = storeitem_start(&np->s,rbuf,rlen)) >= 0) {
	    int	i ;
	    np->li = -1 ;
	    np->i = np->count = 0 ;
	    for (i = 0 ; i < REALNAME_NNAMES ; i += 1) {
	        np->a[i] = NULL ;
	        np->f_abv[i] = 0 ;
	    } /* end for */
	} /* end if */

	return rs ;
}
/* end subroutine (names_start) */


static int names_add(NAMES *np,cchar *nbuf,int nlen,int f_abv,int f_last)
{
	int		rs ;
	int		cl ;
	cchar		*cp ;

#if	CF_DEBUGS
	debugprintf("realname/names_add: nlen=%d\n",nlen) ;
#endif

	if (nlen <= 0)
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("realname/names_add: li=%d i=%d\n",np->li,np->i) ;
#endif

	if ((np->li >= 0) && (np->i == np->li))
	    return SR_INVALID ;

/* load this new one in */

	if (np->a[np->i] != NULL) {

#if	CF_DEBUGS
	    debugprintf("realname/names_add: already occupied\n") ;
#endif

	    np->count -= 1 ;

	}

#if	CF_DEBUGS
	debugprintf("realname/names_add: loading new one\n") ;
#endif

	if ((rs = storeitem_strw(&np->s,nbuf,nlen,&cp)) >= 0) {
	    cl = rs ;

	    np->a[np->i] = cp ;
	    np->l[np->i] = (uchar) cl ;

	    np->f_abv[np->i] = (f_abv)?1:0 ;
	    np->count += 1 ;
	    if (f_last)
	        np->li = np->i ;

/* saturate if we have not yet seen a last name */

	    if ((np->count < REALNAME_NNAMES) || (np->li >= 0)) {

#if	CF_DEBUGS
	        debugprintf("realname/names_add: c=%d incrementing index\n",
	            np->count) ;
#endif

	        np->i = (np->i + 1) % REALNAME_NNAMES ;

	    } /* end if */

	} /* end if (storeitem) */

	return (rs >= 0) ? np->count : rs ;
}
/* end subroutine (names_add) */


static int names_finish(NAMES *np)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = storeitem_finish(&np->s) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (names_finish) */


static int isAbbr(int ch)
{
	ch &= UCHAR_MAX ;
	return ((ch == '.') || (ch == MKCHAR('­'))) ;
}
/* end subroutine (isAbbr) */


