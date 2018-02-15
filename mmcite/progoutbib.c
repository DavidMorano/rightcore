/* progoutbib */

/* write out a bibliographical reference */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1996-02-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine processes the temporary text file along with the
        accumulated citation and bibliographical information into the final text
        output.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<sbuf.h>
#include	<realname.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"citedb.h"
#include	"bdb.h"
#include	"keytracker.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif

#ifndef	LINEFOLDLEN
#define	LINEFOLDLEN	76
#endif


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	bprintlns(bfile *,int,const char *,int) ;
extern int	touc(int) ;

extern int	progwritebib(PROGINFO *,bfile *,BDB_ENT *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

static int	mkname(PROGINFO *,char *,int,cchar *) ;
static int	matkey(const char *(*)[2],const char *) ;


/* external variables */


/* local variables */


/* exported subroutines */


int progoutbib(PROGINFO *pip,bfile *ofp,BDB_ENT *bep)
{
	KEYTRACKER	bibkeys ;
	const int	lflen = LINEFOLDLEN ;
	int		rs ;
	int		rs1 ;
	int		i, ji ;
	int		ll, cl ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/writebib: keyvals¬\n") ;
	    for (i = 0 ; bep->keyvals[i][0] != NULL ; i += 1) {
	        debugprintf("main/writebib: kv[i][0]=%s\n",
	            bep->keyvals[i][0]) ;
	    } /* end for */
	    debugprintf("main/writebib: num=%u\n",i) ;
	}
#endif /* CF_DEBUG */

	if ((rs = keytracker_start(&bibkeys,bep->keyvals)) >= 0) {
	    const int	nlen = BUFLEN ;
	    int		nl ;
	    int		n = 0 ;
	    int		c = 0 ;
	    cchar	*cp ;
	    char	nbuf[BUFLEN + 1] ;

/* process authors */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main/writebib: authors\n") ;
#endif

	    for (i = 0 ; bep->keyvals[i][0] != NULL ; i += 1) {
	        if (strcmp(bep->keyvals[i][0],"A") == 0) {
	            n += 1 ;
	        }
	    } /* end for */

	    ll = 0 ;
	    for (i = 0 ; bep->keyvals[i][0] != NULL ; i += 1) {
	        if (strcmp(bep->keyvals[i][0],"A") != 0) continue ;

	        keytracker_done(&bibkeys,i) ;

	        cp = nbuf ;
	        if ((cl = mkname(pip,nbuf,nlen,bep->keyvals[i][1])) > 0) {

	                if ((ll + (cl + 4)) >= lflen) {
	                    ll = 0 ;
	                    rs = bprintf(ofp,",\n") ;
	                    wlen += rs ;
	                    ll += rs ;
	                } else {
	                    rs = bprintf(ofp,", ") ;
	                    wlen += rs ;
	                    ll += rs ;
	                }

	            c += 1 ;
	            if ((n > 1) && (c == n)) {
	                if (rs >= 0) {
	                    rs = bwrite(ofp,"and ",4) ;
	                    wlen += rs ;
	                }
	            }

	            if (rs >= 0) {
	                rs = bwrite(ofp,cp,cl) ;
	                wlen += rs ;
	                ll += rs ;
	            }

	        } /* end if (created the author name) */

	        if (rs < 0) break ;
	    } /* end for (authors) */

	    if ((rs >= 0) && (c > 0)) {
	        rs = bprintf(ofp,",\n") ;
	        wlen += rs ;
	    }

/* is this in a journal or a book */

	    ji = matkey(bep->keyvals,"J") ;

/* OK, print out the title (knowing whether we have a journal or not) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main/writebib: title\n") ;
#endif

	    i = matkey(bep->keyvals,"T") ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progout/writebib: title=>%t<\n",
	            bep->keyvals[i][1],
	            strnlen(bep->keyvals[i][1],60)) ;
#endif

	    if ((rs >= 0) && (i >= 0)) {
	        SBUF	tb ;

	        keytracker_done(&bibkeys,i) ;

	        if ((rs = sbuf_start(&tb,nbuf,nlen)) >= 0) {
	            cchar	*keys = "J,I,C,V,N,D,DD,MM,YY,P" ;

	            if (ji >= 0) {
	                sbuf_strw(&tb,"\"",-1) ;
	            } else {
	                sbuf_strw(&tb,"\\fI",-1) ;
		    }

	            sbuf_strw(&tb,bep->keyvals[i][1],-1) ;

	            if (keytracker_more(&bibkeys,keys) > 0) {

	                if (ji >= 0) {
	                    sbuf_strw(&tb,",\"",-1) ;
	                } else {
	                    sbuf_strw(&tb,"\\fP,",-1) ;
			}

	            } else {

	                if (ji >= 0) {
	                    sbuf_strw(&tb,".\"",-1) ;
	                } else {
	                    sbuf_strw(&tb,"\\fP.",-1) ;
			}

	            } /* end if */

	            nl = sbuf_finish(&tb) ;
	            if (rs >= 0) rs = nl ;
	        } /* end if (buffer management) */

	        if (rs >= 0) {
	            rs = bprintlns(ofp,lflen,nbuf,nl) ;
	            wlen += rs ;
	        }

	    } /* end if (titles) */

/* print out the journal if we have one */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progout/writebib: journal\n") ;
#endif

	    if ((rs >= 0) && (ji >= 0)) {
	        SBUF	tb ;

	        keytracker_done(&bibkeys,ji) ;

	        if ((rs = sbuf_start(&tb,nbuf,nlen)) >= 0) {
	            cchar	*keys = "I,C,V,N,D,DD,MM,YY,P" ;

	            sbuf_strw(&tb,"\\fI",-1) ;

	            sbuf_strw(&tb,bep->keyvals[ji][1],-1) ;

	            if (keytracker_more(&bibkeys,keys) > 0) {
	                sbuf_strw(&tb,"\\fP,",-1) ;
	            } else {
	                sbuf_strw(&tb,"\\fP.",-1) ;
	            }

	            nl = sbuf_finish(&tb) ;
	            if (rs >= 0) rs = nl ;
	        } /* end if (buffer management) */

	        if (rs >= 0) {
	            rs = bprintlns(ofp,lflen,nbuf,nl) ;
	            wlen += rs ;
	        }

	    } /* end if (journal) */

/* publisher */

	    i = matkey(bep->keyvals,"I") ;

	    if ((rs >= 0) && (i >= 0)) {
	        SBUF	tb ;

	        keytracker_done(&bibkeys,i) ;

	        if ((rs = sbuf_start(&tb,nbuf,nlen)) >= 0) {
	            cchar	*keys = "C,V,N,D,DD,MM,YY,P" ;

	            sbuf_strw(&tb,bep->keyvals[i][1],-1) ;

	            if (keytracker_more(&bibkeys,keys) > 0) {
	                sbuf_strw(&tb,",",-1) ;
	            } else {
	                sbuf_strw(&tb,".",-1) ;
	            }

	            nl = sbuf_finish(&tb) ;
	            if (rs >= 0) rs = nl ;
	        } /* end if (buffer management) */

	        if (rs >= 0) {
	            rs = bprintlns(ofp,lflen,nbuf,nl) ;
	            wlen += rs ;
	        }

	    } /* end if (publisher) */

/* publisher address */

	    i = matkey(bep->keyvals,"C") ;

	    if ((rs >= 0) && (i >= 0)) {
	        SBUF	tb ;

	        keytracker_done(&bibkeys,i) ;

	        if ((rs = sbuf_start(&tb,nbuf,nlen)) >= 0) {
	            cchar	*keys = "V,N,D,DD,MM,YY,P" ;

	            sbuf_strw(&tb,bep->keyvals[i][1],-1) ;

	            if (keytracker_more(&bibkeys,keys) > 0) {
	                sbuf_strw(&tb,",",-1) ;
	            } else {
	                sbuf_strw(&tb,".",-1) ;
	            }

	            nl = sbuf_finish(&tb) ;
	            if (rs >= 0) rs = nl ;
	        } /* end if (buffer management) */

	        if (rs >= 0) {
	            rs = bprintlns(ofp,lflen,nbuf,nl) ;
	            wlen += rs ;
	        }

	    } /* end if (publisher address) */

/* volume and number */

	    i = matkey(bep->keyvals,"V") ;

	    ji = matkey(bep->keyvals,"N") ;

	    if ((i >= 0) && (ji >= 0)) {
	        SBUF	tb ;

	        keytracker_done(&bibkeys,i) ;

	        keytracker_done(&bibkeys,ji) ;

	        if ((rs = sbuf_start(&tb,nbuf,nlen)) >= 0) {
	            cchar	*keys = "D,DD,MM,YY,P" ;

	            sbuf_strw(&tb,bep->keyvals[i][1],-1) ;

	            sbuf_char(&tb,CH_LPAREN) ;

	            sbuf_strw(&tb,bep->keyvals[ji][1],-1) ;

	            sbuf_char(&tb,CH_RPAREN) ;

	            if (keytracker_more(&bibkeys,keys) > 0) {
	                sbuf_strw(&tb,",",-1) ;
	            } else {
	                sbuf_strw(&tb,".",-1) ;
	            }

	            nl = sbuf_finish(&tb) ;
	            if (rs >= 0) rs = nl ;
	        } /* end if (buffer management) */

	        if (rs >= 0) {
	            rs = bprintlns(ofp,lflen,nbuf,nl) ;
	            wlen += rs ;
	        }

	    } else if (i >= 0) {
	        SBUF	tb ;

	        keytracker_done(&bibkeys,i) ;

	        if ((rs = sbuf_start(&tb,nbuf,nlen)) >= 0) {
	            cchar	*keys = "D,DD,MM,YY,P" ;

	            sbuf_strw(&tb,"v. ",3) ;

	            sbuf_strw(&tb,bep->keyvals[i][1],-1) ;

	            if (keytracker_more(&bibkeys,keys) > 0) {
	                sbuf_strw(&tb,",",-1) ;
	            } else {
	                sbuf_strw(&tb,".",-1) ;
	            }

	            nl = sbuf_finish(&tb) ;
	            if (rs >= 0) rs = nl ;
	        } /* end if (buffer management) */

	        if (rs >= 0) {
	            rs = bprintlns(ofp,lflen,nbuf,nl) ;
	            wlen += rs ;
	        }

	    } /* end if (volume and number) */

/* date */

	    i = matkey(bep->keyvals,"D") ;

	    if ((rs >= 0) && (i >= 0)) {
	        SBUF	tb ;

	        keytracker_done(&bibkeys,i) ;

	        if ((rs = sbuf_start(&tb,nbuf,nlen)) >= 0) {

	            sbuf_strw(&tb,bep->keyvals[i][1],-1) ;

	            if (keytracker_more(&bibkeys,"P") > 0) {
	                sbuf_strw(&tb,",",-1) ;
	            } else {
	                sbuf_strw(&tb,".",-1) ;
	            }

	            nl = sbuf_finish(&tb) ;
	            if (rs >= 0) rs = nl ;
	        } /* end if (buffer management) */

	        if (rs >= 0) {
	            rs = bprintlns(ofp,lflen,nbuf,nl) ;
	            wlen += rs ;
	        }

	    } /* end if (date) */

/* pages */

	    i = matkey(bep->keyvals,"P") ;

	    if ((rs >= 0) && (i >= 0)) {
	        SBUF	tb ;

	        keytracker_done(&bibkeys,i) ;

	        if ((rs = sbuf_start(&tb,nbuf,nlen)) >= 0) {

	            sbuf_strw(&tb,"pp. ",-1) ;

	            sbuf_strw(&tb,bep->keyvals[i][1],-1) ;

	            sbuf_strw(&tb,".",-1) ;

	            nl = sbuf_finish(&tb) ;
	            if (rs >= 0) rs = nl ;
	        } /* end if (buffer management) */

	        if (rs >= 0) {
	            rs = bprintlns(ofp,lflen,nbuf,nl) ;
	            wlen += rs ;
	        }

	    } /* end if (pages) */

	    rs1 = keytracker_finish(&bibkeys) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (keytracker) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progout/writebib: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progoutbib) */


/* local subroutines */


/* make a presentable name out of the author field */
static int mkname(PROGINFO *pip,char *rbuf,int rlen,cchar *nameval)
{
	SBUF		ab ;
	int		rs, rs1 ;
	int		len = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (nameval == NULL) return SR_FAULT ;

	if (nameval[0] == '\0') return SR_INVALID ;

	if ((rs = sbuf_start(&ab,rbuf,rlen)) >= 0) {
	    REALNAME	a ;
	    if ((rs = realname_start(&a,nameval,-1)) >= 0) {
		int	c = 0 ;
		int	cl ;
		cchar	*cp ;

	        rs1 = realname_getfirst(&a,&cp) ;
	        if (rs1 > 0) {
	            c += 1 ;
	            sbuf_char(&ab,touc(cp[0])) ;
	            sbuf_char(&ab,'.') ;
	        }

	        rs1 = realname_getm1(&a,&cp) ;
	        if (rs1 > 0) {
	            c += 1 ;
	            sbuf_char(&ab,touc(cp[0])) ;
	            sbuf_char(&ab,'.') ;
	        }

	        if (c > 0)
	            sbuf_char(&ab,' ') ;

	        rs = realname_getlast(&a,&cp) ;
	        cl = rs ;
	        if ((rs >= 0) && (cl > 0)) {
	            sbuf_char(&ab,cp[0]) ;
	            if (cl > 1)
	                sbuf_strw(&ab,(cp + 1),(cl - 1)) ;
	        }

	        realname_finish(&a) ;
	    } /* end if */
	    len = sbuf_finish(&ab) ;
	    if (rs >= 0) rs = len ;
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkname) */


static int matkey(cchar *(*keyvals)[2],cchar *keyname)
{
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; keyvals[i][0] != NULL ; i += 1) {
	    f = (strcmp(keyvals[i][0],keyname) == 0) ;
	    f = f && (keyvals[i][1] != NULL) ;
	    if (f) break ;
	} /* end for */

	return (f) ? i : -1 ;
}
/* end subroutine (matkey) */


