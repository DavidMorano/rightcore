/* varsub */

/* module to handle variable substitution in strings */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was originally written.

	= 1999-07-12, David A­D­ Morano
        Believe it or not I did not like the treatment that zero length values
        were getting! I modified the 'varsub_add' subroutine to allow
        zero-length values in the default case.

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module performs substitutions on strings that have variable
        substitution escapes of some sort in them. The variable substitution
        escapes look like environment variable use within (for example) the
        Bourne and Korn shells.


*******************************************************************************/


#define	VARSUB_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<ascii.h>
#include	<buffer.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"varsub.h"


/* local defines */

#define	VARSUB_DEFENT	10
#define	VARSUB_NLINES	10
#define	VARSUB_SUB	struct varsub_s

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	MKCHAR
#define	MKCHAR(c)	((c) & 0xff)
#endif

#define	BUFLEN		(2 * LINEBUFLEN)


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	strwcmp(cchar *,cchar *,int) ;
extern int	strnkeycmp(cchar *,cchar *,int) ;
extern int	strnncmp(cchar *,int,cchar *,int) ;
extern int	isprintlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;


/* external variables */


/* local structures */

struct varsub_s {
	int		kl, vl ;
	const char	*kp, *vp ;
} ;


/* forward references */

int		varsub_expandbuf(VARSUB *,BUFFER *,const char *,int) ;

static int	varsub_setopts(VARSUB *,int) ;
static int	varsub_iadd(VARSUB *,cchar *,int,cchar *,int) ;
static int	varsub_iaddquick(VARSUB *,cchar *,int,cchar *,int) ;
static int	varsub_sort(VARSUB *) ;
static int	varsub_procvalue(VARSUB *,BUFFER *,cchar *,int) ;
static int	varsub_procsub(VARSUB *,BUFFER *,cchar *,int) ;
static int	varsub_getval(VARSUB *,cchar *,int,cchar **) ;
static int	varsub_writebuf(VARSUB *,bfile *,BUFFER *) ;
static int	varsub_entfins(VARSUB *) ;

static int	entry_start(VARSUB_SUB *,cchar *,int,cchar *,int) ;
static int	entry_keycmp(VARSUB_SUB *,VARSUB_SUB *) ;
static int	entry_finish(VARSUB_SUB *) ;
static int	entry_tmp(VARSUB_SUB *,cchar *,int,cchar *,int) ;
static int	entry_valcmp(VARSUB_SUB *,VARSUB_SUB *) ;

#ifdef	COMMENT
static int	entry_cmp(VARSUB_SUB *,VARSUB_SUB *) ;
#endif

static int	getkey(const char *,int,int [][2]) ;
static int	ventcmp(const void **,const void **) ;


/* local variables */


/* exported subroutines */


/* variable substitution object initialization */
int varsub_start(VARSUB *op,int aopts)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(VARSUB)) ;
	op->i = 0 ;
	op->n = VARSUB_DEFENT ;

/* aopts */

	if ((rs = varsub_setopts(op,aopts)) >= 0) {
	    const int	vopts = VECHAND_OORDERED ;
	    if ((rs = vechand_start(&op->subs,op->n,vopts)) >= 0) {
	        op->magic = VARSUB_MAGIC ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("varsub_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (varsub_start) */


/* free up this object */
int varsub_finish(VARSUB *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	rs1 = varsub_entfins(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&op->subs) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (varsub_finish) */


/* add a variable to the substitution object array */
int varsub_add(VARSUB *op,cchar *k,int klen,cchar *v,int vlen)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (k == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	if (k[0] == '\0') return SR_INVALID ;

	rs = varsub_iadd(op,k,klen,v,vlen) ;

	return rs ;
}
/* end subroutine (varsub_add) */


/* load from key-value string pairs */
int varsub_addva(VARSUB *op,cchar **envv)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (envv == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	for (i = 0 ; envv[i] != NULL ; i += 1) {
	    cchar	*esp = envv[i] ;
	    cchar	*tp ;
	    if ((tp = strchr(esp,'=')) != NULL) {
	        const int	kch = MKCHAR(esp[0]) ;
	        cchar		*vp = (tp + 1) ;
	        if (isprintlatin(kch)) {
	            const int	vch = MKCHAR(vp[0]) ;
	            if ((vch == '\0') || isprintlatin(vch)) {
	                rs = varsub_iadd(op,esp,(tp-esp),vp,-1) ;
	                if (rs < INT_MAX)  c += 1 ;
	            }
	        } /* end if (adding) */
	    } /* end if (valid construction) */
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (varsub_addva) */


/* add a variable to the substitution object array */
int varsub_addquick(VARSUB *op,cchar *k,int klen,cchar *v,int vlen)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (k == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	if (k[0] == '\0') return SR_INVALID ;

	rs = varsub_iaddquick(op,k,klen,v,vlen) ;

	return rs ;
}
/* end subroutine (varsub_addquick) */


/* load from key-value string pairs */
int varsub_addvaquick(VARSUB *op,cchar **envv)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (envv == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	for (i = 0 ; envv[i] != NULL ; i += 1) {
	    cchar	*esp = envv[i] ;
	    cchar	*tp ;
	    if ((tp = strchr(esp,'=')) != NULL) {
	        const int	kch = MKCHAR(esp[0]) ;
	        cchar		*vp = (tp + 1) ;
	        if (isprintlatin(kch)) {
	            const int	vch = MKCHAR(vp[0]) ;
	            if ((vch == '\0') || isprintlatin(vch)) {
	                rs = varsub_iaddquick(op,esp,(tp-esp),vp,-1) ;
	                if (rs < INT_MAX)  c += 1 ;
	            }
	        } /* end if (adding) */
	    } /* end if (valid construction) */
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (varsub_addvaquick) */


/* delete a substitution string from the DB */
int varsub_del(VARSUB *op,cchar *k,int klen)
{
	VECHAND		*elp ;
	int		rs ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (k == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	if (klen < 0) klen = strlen(k) ;

	elp = &op->subs ;
	if (klen > 0) {
	    VARSUB_SUB	te, *ep ;
	    te.kp = k ;
	    te.kl = klen ;
	    if ((rs = vechand_search(elp,&te,ventcmp,&ep)) >= 0) {
	        rs1 = vechand_del(elp,rs) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = entry_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = uc_free(ep) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (search) */
	} else
	    rs = SR_DOM ;

	return rs ;
}
/* end subroutine (varsub_del) */


int varsub_find(VARSUB *op,cchar *k,int klen,cchar **vpp,int *vlenp)
{
	int		rs ;
	int		vl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (k == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	if (klen < 0) klen = strlen(k) ;

	if ((rs = varsub_sort(op)) >= 0) {
	    rs = varsub_getval(op,k,klen,vpp) ;
	    vl = rs ;
	}

	if (vlenp != NULL) *vlenp = (rs >= 0) ? vl : 0 ;

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (varsub_find) */


int varsub_fetch(VARSUB *op,cchar *k,int klen,cchar **vpp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (k == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	if (klen < 0) klen = strlen(k) ;

	if ((rs = varsub_sort(op)) >= 0) {
	    rs = varsub_getval(op,k,klen,vpp) ;
	}

	return rs ;
}
/* end subroutine (varsub_fetch) */


int varsub_curbegin(VARSUB *op,VARSUB_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(VARSUB_CUR)) ;
	curp->i = -1 ;

	return SR_OK ;
}
/* end subroutine (varsub_curbegin) */


int varsub_curend(VARSUB *op,VARSUB_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(VARSUB_CUR)) ;
	curp->i = -1 ;

	return SR_OK ;
}
/* end subroutine (varsub_curend) */


int varsub_enum(VARSUB *op,VARSUB_CUR *curp,cchar **kpp,cchar **vpp)
{
	VARSUB_SUB	*ep = NULL ;
	int		rs = SR_OK ;
	int		i ;
	int		vl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	i = (curp->i >= 0) ? (curp->i + 1) : 0 ;

	while ((rs >= 0) && (ep == NULL)) {
	    if ((rs = vechand_get(&op->subs,i,&ep)) >= 0) {
	        if (ep == NULL) i += 1 ;
	    }
	} /* end while */

#if	CF_DEBUGS
	debugprintf("varsub_enum: vechand_get() rs=%d\n",rs) ;
	if (rs >= 0) {
	    debugprintf("varsub_enum: ep={%p}\n",ep) ;
	    debugprintf("varsub_enum: k=%t\n",ep->kp,ep->kl) ;
	}
#endif

	if (kpp != NULL) {
	    *kpp = (rs >= 0) ? ep->kp : NULL ;
	}

	if (vpp != NULL) {
	    *vpp = (rs >= 0) ? ep->vp : NULL ;
	}

	if (rs >= 0) {
	    vl = ep->vl ;
	    curp->i = i ;
	}

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (varsub_enum) */


/* substitute a whole file */
int varsub_expandfile(VARSUB *op,bfile *ifp,bfile *ofp)
{
	int		rs ;
	int		rs1 ;
	int		nlines = 0 ;
	int		wlen = 0 ;

	if (op == NULL) return SR_FAULT ;
	if ((ifp == NULL) || (ofp == NULL)) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	if ((rs = varsub_sort(op)) >= 0) {
	    BUFFER	b ;
	    const int	startlen = (VARSUB_NLINES * LINEBUFLEN) ;
	    op->badline = -1 ;
	    if ((rs = buffer_start(&b,startlen)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len-1] == '\n') nlines += 1 ;

#if	CF_DEBUGS
	            debugprintf("varsub_subfile: line=>%t<\n",
	                lbuf,strlinelen(lbuf,len,60)) ;
#endif

	            rs = varsub_expandbuf(op,&b,lbuf,len) ;

#if	CF_DEBUGS
	            debugprintf("varsub_subfile: _expandbuf() rs=%d\n",rs) ;
#endif

/* write the (possibly modified) line to the output */

	            if ((rs >= 0) && (nlines >= VARSUB_NLINES)) {
	                nlines = 0 ;
	                rs = varsub_writebuf(op,ofp,&b) ;
	            } /* end if (flush) */

	            if (rs < 0) break ;
	        } /* end while (reading file lines) */

	        if ((rs >= 0) && (nlines > 0)) {
	            rs = varsub_writebuf(op,ofp,&b) ;
	        }

	        rs1 = buffer_finish(&b) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (buffer) */
	} /* end if (varsub_sort) */

#if	CF_DEBUGS
	debugprintf("varsub_subfile: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (varsub_expandfile) */


/* perform substitutions on a buffer */
int varsub_expand(VARSUB *op,char *dbuf,int dlen,cchar *linein,int linelen)
{
	int		rs ;
	int		rs1 ;
	int		bl = 0 ;
	const char	*bp ;

	if (op == NULL) return SR_FAULT ;
	if (linein == NULL) return SR_FAULT ;
	if (dbuf == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	if (linelen < 0) linelen = strlen(linein) ;

	if (dlen < 0) dlen = LINEBUFLEN ;

/* early errors */

	op->badline = -1 ;
	if (dlen < linelen) return SR_TOOBIG ;

/* do it */

	if ((rs = varsub_sort(op)) >= 0) {
	    BUFFER	b ;
	    if ((rs = buffer_start(&b,LINEBUFLEN)) >= 0) {

	        if ((rs = varsub_procvalue(op,&b,linein,linelen)) >= 0) {
	            if ((rs = buffer_get(&b,&bp)) >= 0) {
	                bl = rs ;
	                rs = snwcpy(dbuf,dlen,bp,bl) ;
	            }
	        }

	        rs1 = buffer_finish(&b) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (buffer) */
	} /* end if (varsub_sort) */

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (varsub_expand) */


int varsub_expandbuf(VARSUB *op,BUFFER *bufp,cchar *linein,int linelen)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (bufp == NULL) return SR_FAULT ;
	if (linein == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	if (linelen < 0) linelen = strlen(linein) ;

/* do it */

	if ((rs = varsub_sort(op)) >= 0) {
	    rs = varsub_procvalue(op,bufp,linein,linelen) ;
	}

	return rs ;
}
/* end subroutine (varsub_expandbuf) */


/* private subroutines */


static int varsub_setopts(VARSUB *op,int aopts)
{

	op->f.noblank = FALSE ;
	op->f.badnokey = FALSE ;
	op->f.brace = FALSE ;
	op->f.paren = FALSE ;
	if (aopts & VARSUB_MNOBLANK) op->f.noblank = TRUE ;

#if	CF_DEBUGS
	debugprintf("varsub_setopts: f_noblank=%u\n",op->f.noblank) ;
#endif

	if (aopts & VARSUB_MBADNOKEY) op->f.badnokey = TRUE ;

	if (aopts & (VARSUB_MPAREN | VARSUB_MBRACE)) {
	    if (aopts & VARSUB_MBRACE) op->f.brace = TRUE ;
	    if (aopts & VARSUB_MPAREN) op->f.paren = TRUE ;
	} else {
	    op->f.brace = op->f.paren = TRUE ;
	}

	return SR_OK ;
}
/* end subroutine (varsub_setopts) */


static int varsub_procvalue(VARSUB *op,BUFFER *bufp,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		sses[3][2] ;
	int		kl, cl ;
	int		i = 0 ;
	int		len = 0 ;
	const char	ssb[] = { CH_LBRACE, CH_RBRACE, 0 } ;
	const char	ssp[] = { CH_LPAREN, CH_RPAREN, 0 } ;
	const char	*tp ;
	const char	*cp ;
	const char	*kp ;

	if (op->f.brace) {
	    sses[i][0] = ssb[0] ;
	    sses[i][1] = ssb[1] ;
	    i += 1 ;
	}
	if (op->f.paren) {
	    sses[i][0] = ssp[0] ;
	    sses[i][1] = ssp[1] ;
	    i += 1 ;
	}
	sses[i][0] = 0 ;
	sses[i][1] = 0 ;

	while ((tp = strnchr(sp,sl,'$')) != NULL) {

	    cp = sp ;
	    cl = (tp - sp) ;

#if	CF_DEBUGS
	    debugprintf("varsub_procvalue: il=%u leader=>%t<\n",cl,cp,cl) ;
#endif

	    if (cl > 0) {
	        rs = buffer_strw(bufp,cp,cl) ;
	        len += rs ;
#if	CF_DEBUGS
	        debugprintf("varsub_procvalue: buffer_strw() rs=%d\n",rs) ;
	        debugprintf("varsub_procvalue: len=%u\n",len) ;
#endif
	    }

	    if (rs >= 0) {
	        kp = (tp + 2) ;
	        kl = getkey(tp,((sp + sl) - tp),sses) ;

#if	CF_DEBUGS
	        debugprintf("varsub_procvalue: getkey() rs=%d\n",kl) ;
#endif

	        if (kl > 0) {
	            rs = varsub_procsub(op,bufp,kp,kl) ;
	            len += rs ;

#if	CF_DEBUGS
	            debugprintf("varsub_procvalue: _procsub() rs=%d\n",rs) ;
	            debugprintf("varsub_procvalue: len=%u\n",len) ;
#endif

	        }
	        if (rs >= 0) {
	            if (kl >= 0) {
	                sl -= ((kp + kl + 1) - sp) ;
	                sp = (kp + kl + 1) ;
	            } else {
	                sl -= ((tp + 1) - sp) ;
	                sp = (tp + 1) ;
	                rs = buffer_char(bufp,'$') ;
	                len += rs ;
	            }
	        }
	    } /* end if (ok) */

	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (sl > 0)) {
#if	CF_DEBUGS
	    debugprintf("varsub_procvalue: il=%u trailer=>%t<\n",sl,sp,sl) ;
#endif
	    rs = buffer_strw(bufp,sp,sl) ;
	    len += rs ;
#if	CF_DEBUGS
	    debugprintf("varsub_procvalue: buffer_strw() rs=%d\n",rs) ;
	    debugprintf("varsub_procvalue: len=%u\n",len) ;
#endif
	} /* end if */

#if	CF_DEBUGS
	debugprintf("varsub_procvalue: ret rs=%d vl=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (varsub_procvalue) */


static int varsub_procsub(VARSUB *op,BUFFER *bufp,cchar *kp,int kl)
{
	int		rs = SR_OK ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("varsub_procsub: ent k=%t\n",kp,kl) ;
#endif

	if (kl > 0) {
	    int		al = 0 ;
	    cchar	*tp ;
	    cchar	*cp ;
	    cchar	*ap = NULL ;

	    if ((tp = strnchr(kp,kl,'=')) != NULL) {
	        ap = (tp + 1) ;
	        al = (kp + kl) - (tp + 1) ;
	        kl = (tp - kp) ;
	    }

/* lookup the environment key-name that we have */

#if	CF_DEBUGS
	    debugprintf("varsub_procsub: ext al=%d k=%t\n",al,kp,kl) ;
	    if ((ap != NULL) && (al > 0))
	        debugprintf("varsub_procsub: as=%t\n",ap,al) ;
#endif

	    if ((rs = varsub_getval(op,kp,kl,&cp)) >= 0) {
	        rs = buffer_strw(bufp,cp,rs) ;
	        len += rs ;
	    } else if (rs == SR_NOTFOUND) {
	        if (al > 0) {
	            rs = buffer_strw(bufp,ap,al) ;
	            len += rs ;
		} else if (op->f.noblank) {
#if	CF_DEBUGS
		    debugprintf("varsub_procsub: noblank\n") ;
#endif
		    if ((rs = buffer_char(bufp,'*')) >= 0) {
	            	len += rs ;
	                if ((rs = buffer_strw(bufp,kp,kl)) >= 0) {
	            	    len += rs ;
		    	    rs = buffer_char(bufp,'*') ;
	            	    len += rs ;
			}
		    }
	        } else if (op->f.badnokey) {
#if	CF_DEBUGS
		    debugprintf("varsub_procsub: nobadkey\n") ;
#endif
	            rs = SR_NOTFOUND ;
	        } else {
		    rs = SR_OK ;
		}
	    } /* end if (not-found) */

	} /* end if (positive) */

#if	CF_DEBUGS
	debugprintf("varsub_procsub: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (varsub_procsub) */


static int varsub_iadd(VARSUB *op,cchar *k,int klen,cchar *v,int vlen)
{
	VARSUB_SUB	tmp ;
	int		rs ;

	if (klen < 0) klen = strlen(k) ;
	if (vlen < 0) vlen = (v != NULL) ? strlen(v) : 0 ;

#if	CF_DEBUGS
	debugprintf("varsub_iadd: ent k=%t\n",k,klen) ;
#endif

	if ((rs = entry_tmp(&tmp,k,klen,v,vlen)) >= 0) {
	    VECHAND	*elp = &op->subs ;
	    VARSUB_SUB	*dep ;
	    int		rs1 ;

	    if ((rs1 = vechand_search(elp,&tmp,ventcmp,&dep)) >= 0) {
	        if (entry_valcmp(dep,&tmp) != 0) {
	            vechand_del(elp,rs1) ;
	            entry_finish(dep) ;
	            uc_free(dep) ;
	            rs1 = SR_NOTFOUND ;
	        } else {
	            rs = INT_MAX ;
		}
	    } /* end if (entry search-by-key) */

#if	CF_DEBUGS
	    debugprintf("varsub_iadd: mid rs=%d rs1=%d\n",rs,rs1) ;
#endif

	    if (rs1 == SR_NOTFOUND) {
	        VARSUB_SUB	*ep ;
	        const int	msize = sizeof(VARSUB_SUB) ;

#if	CF_DEBUGS
	        debugprintf("varsub_iadd: adding\n") ;
#endif

	        if ((rs = uc_malloc(msize,&ep)) >= 0) {
	            if ((rs = entry_start(ep,k,klen,v,vlen)) >= 0) {
	                op->f.sorted = FALSE ;
	                rs = vechand_add(elp,ep) ;
	                if (rs < 0)
	                    entry_finish(ep) ;
	            } /* end if (entry-start) */
	            if (rs < 0)
	                uc_free(ep) ;
	        } /* end if (memory-allocation) */

	    } else {
	        rs = rs1 ;
	    }

	} /* end if (entry_tmp) */

#if	CF_DEBUGS
	debugprintf("varsub_iadd: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (varsub_iadd) */


static int varsub_iaddquick(VARSUB *op,cchar *k,int klen,cchar *v,int vlen)
{
	int		rs = SR_INVALID ;

	if (klen < 0) klen = strlen(k) ;
	if (vlen < 0) vlen = (v != NULL) ? strlen(v) : 0 ;

#if	CF_DEBUGS
	debugprintf("varsub_iaddquick: ent k=%t\n",k,klen) ;
#endif

	if (klen > 0) {
	    VECHAND	*elp = &op->subs ;
	    VARSUB_SUB	*ep ;
	    const int	msize = sizeof(VARSUB_SUB) ;

#if	CF_DEBUGS
	    debugprintf("varsub_iadd: adding\n") ;
#endif

	    if ((rs = uc_malloc(msize,&ep)) >= 0) {
	        if ((rs = entry_start(ep,k,klen,v,vlen)) >= 0) {
	            op->f.sorted = FALSE ;
	            rs = vechand_add(elp,ep) ;
	            if (rs < 0)
	                entry_finish(ep) ;
	        } /* end if (entry-start) */
	        if (rs < 0)
	            uc_free(ep) ;
	    } /* end if (memory-allocation) */

	} /* end if (positive) */

#if	CF_DEBUGS
	debugprintf("varsub_iaddquick: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (varsub_iaddquick) */


static int varsub_sort(VARSUB *op)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (! op->f.sorted) {
	    f = TRUE ;
	    op->f.sorted = TRUE ;
	    rs = vechand_sort(&op->subs,ventcmp) ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (varsub_sort) */


static int varsub_getval(VARSUB *op,cchar *kp,int kl,cchar **vpp)
{
	VARSUB_SUB	te, *ep ;
	int		rs ;
	int		vl = 0 ;

	if (kl < 0) kl = strlen(kp) ;

#if	CF_DEBUGS
	debugprintf("varsub_getval: ent k=%t\n",kp,kl) ;
#endif

	if (kl > 0) {
	    te.kp = kp ;
	    te.kl = kl ;
	    if ((rs = vechand_search(&op->subs,&te,ventcmp,&ep)) >= 0) {
	        vl = ep->vl ;
	    }
#if	CF_DEBUGS
	    debugprintf("varsub_getval: vechand-search() rs=%d\n",rs) ;
#endif
	} else {
	    rs = SR_DOM ;
	}

	if (vpp != NULL) {
	    *vpp = (rs >= 0) ? ep->vp : NULL ;
	}

#if	CF_DEBUGS
	debugprintf("varsub_getval: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (varsub_getval) */


static int varsub_writebuf(VARSUB *op,bfile *ofp,BUFFER *bufp)
{
	int		rs ;
	int		wlen = 0 ;
	const char	*bp ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = buffer_get(bufp,&bp)) > 0) {
	    if ((rs = bwrite(ofp,bp,rs)) >= 0) {
	        wlen += rs ;
	        buffer_reset(bufp) ;
	    }
	} /* end if (buffer_get) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (varsub_writebuf) */


static int varsub_entfins(VARSUB *op)
{
	VARSUB_SUB	*ep ;
	VECHAND		*elp = &op->subs ; /* loop invariant */
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vechand_get(elp,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs1 = entry_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = uc_free(ep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (varsub_entfins) */


static int entry_start(VARSUB_SUB *ep,cchar *kp,int kl,cchar *vp,int vl)
{
	int		rs ;

	if (kl < 0)
	    kl = strlen(kp) ;

	if (vl < 0) {
	    vl = (vp != NULL) ? strlen(vp) : 0 ;
	}

/* allocate buffers for the key and its value respectively */

	if (kl > 0) {
	    const int	size = (kl + 1) + (vl + 1) ;
	    char	*bp ;
	    if ((rs = uc_malloc(size,&bp)) >= 0) {
	        ep->kp = bp ;
	        ep->kl = kl ;
	        bp = (strwcpy(bp,kp,kl) + 1) ;
	        ep->vp = bp ;
	        ep->vl = vl ;
	        bp = (strwcpy(bp,vp,vl) + 1) ;
	    } /* end if (memory-allocation) */
	} else {
	    rs = SR_DOM ;
	}

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(VARSUB_SUB *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep->kp != NULL) {
	    rs1 = uc_free(ep->kp) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->kp = NULL ;
	}
	ep->vp = NULL ;

	return rs ;
}
/* end subroutine (entry_finish) */


static int entry_keycmp(VARSUB_SUB *ep,VARSUB_SUB *eap)
{
	return strnncmp(ep->kp,ep->kl,eap->kp,eap->kl) ;
}
/* end subroutine (entry_keycmp) */


static int entry_tmp(VARSUB_SUB *ep,cchar *kp,int kl,cchar *vp,int vl)
{
	int		rs = SR_OK ;

	if (kl < 0) kl = strlen(kp) ;

	if (kl > 0) {
	    ep->kp = kp ;
	    ep->kl = kl ;
	    ep->vp = vp ;
	    ep->vl = (vp != NULL) ? vl : 0 ;
	} else {
	    rs = SR_DOM ;
	}

	return rs ;
}
/* end subroutine (entry_tmp) */


static int entry_valcmp(VARSUB_SUB *ep,VARSUB_SUB *e2p)
{
	int		rc = 0 ;

	if (ep == NULL) return SR_FAULT ;
	if (e2p == NULL) return SR_FAULT ;

	if ((ep->vl > 0) || (e2p->vl > 0)) {
	    if (ep->vl == 0) rc = -1 ;
	    if ((rc == 0) && (e2p->vl == 0)) rc = 1 ;
	    if (rc == 0)
	        rc = (ep->vp[0] - e2p->vp[0]) ;
	    if (rc == 0)
	        rc = strwcmp(ep->vp,e2p->vp,e2p->vl) ;
	    if (rc == 0)
	        rc = (ep->vl - e2p->vl) ;
	} /* end if (try harder) */

	return rc ;
}
/* end subroutine (entry_valcmp) */


static int getkey(const char *sp,int sl,int sses[][2])
{
	int		kl = -1 ;

	if (sl > 1) {
	    int	ch ;
	    int	i ;
	    int	f = FALSE ;

	    sp += 1 ;
	    sl -= 1 ;
	    for (i = 0 ; sses[i][0] != 0 ; i += 1) {
	        ch = (*sp & 0xff) ;
	        f = (ch == sses[i][0]) ;
	        if (f) break ;
	    } /* end for */

	    if (f) {
	        cchar	*tp ;
	        tp = strnchr(sp,sl,sses[i][1]) ;
	        if (tp != NULL) {
	            kl = (tp - (sp+1)) ;
	        }
	    }

	} /* end if (greater than length one) */

	return kl ;
}
/* end subroutine (getkey) */


static int ventcmp(const void **ve1pp,const void **ve2pp)
{
	VARSUB_SUB	**e1pp = (VARSUB_SUB **) ve1pp ;
	VARSUB_SUB	**e2pp = (VARSUB_SUB **) ve2pp ;
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            VARSUB_SUB	*e1p = *e1pp ;
	            VARSUB_SUB	*e2p = *e2pp ;
#if	CF_DEBUGS
		    debugprintf("ventcmp: k1=%t k2=%s\n",
			e1p->kp,e1p->kl,e2p->kp) ;
#endif
		    rc = entry_keycmp(e1p,e2p) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
#if	CF_DEBUGS
	debugprintf("ventcmp: ret rc=%d\n",rc) ;
#endif
	return rc ;
}
/* end subroutine (ventcmp) */


