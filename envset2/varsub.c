/* varsub */

/* module to handle variable substitution in strings */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_ENTCMP	1		/* compare new entries */


/* revision history:

	= 1998-12-01, David A­D­ Morano

	This subroutine was originally written.


	= 1999-07-12, David A­D­ Morano

	Believe it or not I did not like the treatment that zero length
	values were getting!  I modified the 'varsub_add' subroutine
	to allow zero-length values in the default case.


*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module performs substitutions on strings that have variables
	substitution escapes of some sort in them.  The variable
	substitution escapes look like environment variable use within
	(for example) the Bourne and Korn shells.


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
#include	<ascii.h>
#include	<buffer.h>
#include	<bfile.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"varsub.h"


/* local defines */

#define	VARSUB_DEFENT		10
#define	VARSUB_NLINES		10
#define	VARSUB_SUB		struct varsub_s

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

extern int	snwcpy(char *,int,const char *,int) ;
extern int	strwcmp(const char *,const char *,int) ;
extern int	isprintlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */

struct varsub_s {
	int		kl, vl ;
	const char	*kp, *vp ;
} ;


/* forward references */

int		varsub_expandbuf(VARSUB *,BUFFER *,const char *,int) ;

static int	varsub_setopts(VARSUB *,int) ;
static int	varsub_iadd(VARSUB *,const char *,int,const char *,int) ;
static int	varsub_sort(VARSUB *) ;
static int	varsub_procvalue(VARSUB *,BUFFER *,const char *,int) ;
static int	varsub_procsub(VARSUB *,BUFFER *,const char *,int) ;
static int	varsub_getval(VARSUB *,const char *,int,const char **) ;
static int	varsub_writebuf(VARSUB *,bfile *,BUFFER *) ;
static int	varsub_entfins(VARSUB *) ;

static int	entry_start(VARSUB_SUB *,const char *,int,const char *,int) ;
static int	entry_finish(VARSUB_SUB *) ;
#if	CF_ENTCMP
static int	entry_tmp(VARSUB_SUB *,const char *,int,const char *,int) ;
static int	entry_cmpval(VARSUB_SUB *,VARSUB_SUB *) ;
#endif /* CF_ENTCMP */

#ifdef	COMMENT
static int	entry_cmp(VARSUB_SUB *,VARSUB_SUB *) ;
#endif

static int	getkey(const char *,int,int [][2]) ;
static int	vcmpkey(const void **,const void **) ;


/* local variables */


/* exported subroutines */


/* variable substitution object initialization */
int varsub_start(op,aopts)
VARSUB		*op ;
int		aopts ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(VARSUB)) ;
	op->i = 0 ;
	op->n = VARSUB_DEFENT ;

/* aopts */

	if ((rs = varsub_setopts(op,aopts)) >= 0) {
	    const int	vopts = VECHAND_OORDERED ;
	    if ((rs = vechand_start(&op->subs,op->n,vopts)) >= 0)
		op->magic = VARSUB_MAGIC ;
	}

#if	CF_DEBUGS
	debugprintf("varsub_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (varsub_start) */


/* free up this object */
int varsub_finish(op)
VARSUB		*op ;
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
int varsub_add(op,k,klen,v,vlen)
VARSUB		*op ;
const char	k[] ;
const char	v[] ;
int		klen, vlen ;
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
int varsub_loadenv(op,envv)
VARSUB		*op ;
const char	*envv[] ;
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (envv == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	for (i = 0 ; envv[i] != NULL ; i += 1) {
	    const char	*esp = envv[i] ;
	    const char	*tp ;
	    if ((tp = strchr(esp,'=')) != NULL) {
		const char	*vp = (tp + 1) ;
		const int	kch = MKCHAR(esp[0]) ;
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
/* end subroutine (varsub_loadenv) */


/* delete a substitution string from the DB */
int varsub_del(op,k,klen)
VARSUB		*op ;
const char	k[] ;
int		klen ;
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
	    if ((rs = vechand_search(elp,&te,vcmpkey,&ep)) >= 0) {
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


int varsub_find(op,k,klen,vpp,vlenp)
VARSUB		*op ;
const char	k[] ;
int		klen ;
const char	**vpp ;
int		*vlenp ;
{
	int		rs = SR_OK ;
	int		vl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (k == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	if (klen < 0) klen = strlen(k) ;

	if (! op->f.sorted) rs = varsub_sort(op) ;

	if (rs >= 0) {
	    rs = varsub_getval(op,k,klen,vpp) ;
	    vl = rs ;
	}

	if (vlenp != NULL) *vlenp = (rs >= 0) ? vl : 0 ;

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (varsub_find) */


int varsub_fetch(op,k,klen,vpp)
VARSUB		*op ;
const char	k[] ;
int		klen ;
const char	**vpp ;
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (k == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	if (klen < 0) klen = strlen(k) ;

	if (! op->f.sorted) rs = varsub_sort(op) ;

	if (rs >= 0)
	    rs = varsub_getval(op,k,klen,vpp) ;

	return rs ;
}
/* end subroutine (varsub_fetch) */


int varsub_curbegin(op,curp)
VARSUB		*op ;
VARSUB_CUR	*curp ;
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(VARSUB_CUR)) ;
	curp->i = -1 ;

	return SR_OK ;
}
/* end subroutine (varsub_curbegin) */


int varsub_curend(op,curp)
VARSUB		*op ;
VARSUB_CUR	*curp ;
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(VARSUB_CUR)) ;
	curp->i = -1 ;

	return SR_OK ;
}
/* end subroutine (varsub_curend) */


int varsub_enum(op,curp,kpp,vpp)
VARSUB		*op ;
VARSUB_CUR	*curp ;
const char	**kpp ;
const char	**vpp ;
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

	if (kpp != NULL)
	    *kpp = (rs >= 0) ? ep->kp : NULL ;

	if (vpp != NULL)
	    *vpp = (rs >= 0) ? ep->vp : NULL ;

	if (rs >= 0) {
	    vl = ep->vl ;
	    curp->i = i ;
	}

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (varsub_enum) */


/* substitute a whole file */
int varsub_file(op,ifp,ofp)
VARSUB		*op ;
bfile		*ifp ;
bfile		*ofp ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		nlines = 0 ;
	int		wlen = 0 ;

	if (op == NULL) return SR_FAULT ;
	if ((ifp == NULL) || (ofp == NULL)) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	if (! op->f.sorted) rs = varsub_sort(op) ;

	if (rs >= 0) {
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
	            debugprintf("varsub_file: line=>%t<\n",
	                lbuf,strlinelen(lbuf,len,60)) ;
#endif

	            rs = varsub_expandbuf(op,&b,lbuf,len) ;

#if	CF_DEBUGS
	            debugprintf("varsub_file: _expandbuf() rs=%d\n",rs) ;
#endif

/* write the (possibly modified) line to the output */

	            if ((rs >= 0) && (nlines >= VARSUB_NLINES)) {
	                nlines = 0 ;
	                rs = varsub_writebuf(op,ofp,&b) ;
	            } /* end if (flush) */

	            if (rs < 0) break ;
	        } /* end while (reading file lines) */

	        if ((rs >= 0) && (nlines > 0))
	            rs = varsub_writebuf(op,ofp,&b) ;

	        rs1 = buffer_finish(&b) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (buffer) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("varsub_file: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (varsub_file) */


/* perform substitutions on a buffer */
int varsub_expand(op,dbuf,dlen,linein,linelen)
VARSUB		*op ;
char		dbuf[] ;
int		dlen ;
const char	linein[] ;
int		linelen ;
{
	int		rs = SR_OK ;
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
	if (dlen < linelen)
	    return SR_TOOBIG ;

/* do it */

	if (! op->f.sorted)
	    rs = varsub_sort(op) ;

	if (rs >= 0) {
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
	} /* end if (ok) */

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (varsub_expand) */


int varsub_expandbuf(op,bufp,linein,linelen)
VARSUB		*op ;
BUFFER		*bufp ;
const char	linein[] ;
int		linelen ;
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (bufp == NULL) return SR_FAULT ;
	if (linein == NULL) return SR_FAULT ;

	if (op->magic != VARSUB_MAGIC) return SR_NOTOPEN ;

	if (linelen < 0) linelen = strlen(linein) ;

/* do it */

	if (! op->f.sorted)
	    rs = varsub_sort(op) ;

	if (rs >= 0)
	    rs = varsub_procvalue(op,bufp,linein,linelen) ;

	return rs ;
}
/* end subroutine (varsub_expandbuf) */


/* private subroutines */


static int varsub_setopts(op,aopts)
VARSUB		*op ;
int		aopts ;
{

	op->f.noblank = FALSE ;
	op->f.badnokey = FALSE ;
	op->f.brace = FALSE ;
	op->f.paren = FALSE ;
	if (aopts & VARSUB_MNOBLANK)
	    op->f.noblank = TRUE ;

	if (aopts & VARSUB_MBADNOKEY)
	    op->f.badnokey = TRUE ;

	if (aopts & (VARSUB_MPAREN | VARSUB_MBRACE)) {
	    if (aopts & VARSUB_MBRACE)
	        op->f.brace = TRUE ;
	    if (aopts & VARSUB_MPAREN)
	        op->f.paren = TRUE ;
	} else
	    op->f.brace = op->f.paren = TRUE ;

	return SR_OK ;
}
/* end subroutine (varsub_setopts) */


static int varsub_procvalue(op,bufp,sp,sl)
VARSUB		*op ;
BUFFER		*bufp ;
const char	*sp ;
int		sl ;
{
	int		rs = SR_OK ;
	int		sses[3][2] ;
	int		kl, cl ;
	int		i ;
	int		len = 0 ;
	const char	ssb[] = { CH_LBRACE, CH_RBRACE, 0 } ;
	const char	ssp[] = { CH_LPAREN, CH_RPAREN, 0 } ;
	const char	*tp ;
	const char	*cp ;
	const char	*kp ;

	i = 0 ;
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
	    }

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

	}

#if	CF_DEBUGS
	debugprintf("varsub_procvalue: ret rs=%d vl=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (varsub_procvalue) */


static int varsub_procsub(op,bufp,kp,kl)
VARSUB		*op ;
BUFFER		*bufp ;
const char	*kp ;
int		kl ;
{
	int		rs = SR_OK ;
	int		cl, al ;
	int		len = 0 ;
	const char	*tp ;
	const char	*ap ; /* alternate value (if supplied) */
	const char	*cp ;

#if	CF_DEBUGS
	debugprintf("varsub_procsub: k=%t\n",kp,kl) ;
#endif

	if (kl > 0) {

	ap = NULL ;
	al = 0 ;
	if ((tp = strnchr(kp,kl,'=')) != NULL) {
	    ap = (tp + 1) ;
	    al = (kp + kl) - (tp + 1) ;
	    kl = (tp - kp) ;
	}

/* lookup the environment key-name that we have */

#if	CF_DEBUGS
	debugprintf("varsub_procsub: modified k=%t\n",kp,kl) ;
	if ((ap != NULL) && (al > 0))
	    debugprintf("varsub_procsub: as=%t\n",ap,al) ;
#endif

	cl = varsub_getval(op,kp,kl,&cp) ;

#if	CF_DEBUGS
	debugprintf("varsub_procsub: _getval() rs=%d\n",cl) ;
	if (cl >= 0)
	    debugprintf("varsub_procsub: _getval() v=>%t<\n",cp,cl) ;
#endif

/* perform any appropriate substitution */

	if (cl >= 0) {
	    rs = buffer_strw(bufp,cp,cl) ;
	    len += rs ;
	} else if (al > 0) {
	    rs = buffer_strw(bufp,ap,al) ;
	    len += rs ;
	} else if (op->f.badnokey)
	    rs = SR_NOTFOUND ;

	} /* end if (positive) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (varsub_procsub) */


#if	CF_ENTCMP

static int varsub_iadd(op,k,klen,v,vlen)
VARSUB		*op ;
const char	k[] ;
const char	v[] ;
int		klen, vlen ;
{
	VARSUB_SUB	tmp ;
	int		rs ;

	if (klen < 0) klen = strlen(k) ;
	if (vlen < 0) vlen = (v != NULL) ? strlen(v) : 0 ;

	if ((rs = entry_tmp(&tmp,k,klen,v,vlen)) >= 0) {
	    VECHAND	*elp = &op->subs ;
	    VARSUB_SUB	*dep ;
	    int		rs1 ;

	    if ((rs1 = vechand_search(elp,&tmp,vcmpkey,&dep)) >= 0) {
	        if (entry_cmpval(dep,&tmp) != 0) {
		    vechand_del(elp,rs1) ;
		    entry_finish(dep) ;
		    uc_free(dep) ;
		    rs1 = SR_NOTFOUND ;
	        } else
		    rs = INT_MAX ;
	    } /* end if (entry search-by-key) */

#if	CF_DEBUGS
	    debugprintf("varsub_iadd: mid rs=%d rs1=%d\n",rs,rs1) ;
#endif

	    if (rs1 == SR_NOTFOUND) {
	        VARSUB_SUB	*ep ;
	        const int	msize = sizeof(VARSUB_SUB) ;
    
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

	    } else
	        rs = rs1 ;

	} /* end if (entry-validated) */

	return rs ;
}
/* end subroutine (varsub_iadd) */

#else /* CF_ENTCMP */

static int varsub_iadd(op,k,klen,v,vlen)
VARSUB		*op ;
const char	k[] ;
const char	v[] ;
int		klen, vlen ;
{
	VARSUB_SUB	*ep ;
	const int	msize = sizeof(VARSUB_SUB) ;
	int		rs ;
	int		rs1 ;

	if (klen < 0) klen = strlen(k) ;

	if ((rs = uc_malloc(msize,&ep)) >= 0) {
	    if ((rs = entry_start(ep,k,klen,v,vlen)) >= 0) {
		VECHAND		*elp = &op->subs ;
		VARSUB_SUB	*dep ;
	        if ((rs1 = vechand_search(elp,ep,vcmpkey,&dep)) >= 0) {
		    vechand_del(elp,rs1) ;
		    entry_finish(dep) ;
		    uc_free(dep) ;
		} else if (rs1 != SR_NOTFOUND)
		    rs = rs1 ;
		if (rs >= 0) {
	    	    op->f.sorted = FALSE ;
		    rs = vechand_add(elp,ep) ;
		}
		if (rs < 0)
	    	    entry_finish(ep) ;
	    } /* end if (entry-start) */
	    if (rs < 0)
		uc_free(ep) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (varsub_iadd) */

#endif /* CF_ENTCMP */


static int varsub_sort(op)
VARSUB		*op ;
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (! op->f.sorted) {
	    f = TRUE ;
	    op->f.sorted = TRUE ;
	    rs = vechand_sort(&op->subs,vcmpkey) ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (varsub_sort) */


static int varsub_getval(op,kp,kl,vpp)
VARSUB		*op ;
const char	kp[] ;
int		kl ;
const char	**vpp ;
{
	VARSUB_SUB	te, *ep ;
	int		rs ;
	int		vl = 0 ;

	if (kl < 0) kl = strlen(kp) ;

	if (kl > 0) {
	    te.kp = kp ;
	    te.kl = kl ;
	    if ((rs = vechand_search(&op->subs,&te,vcmpkey,&ep)) >= 0)
	        vl = ep->vl ;
	} else
	    rs = SR_DOM ;

	if (vpp != NULL) *vpp = (rs >= 0) ? ep->vp : NULL ;

#if	CF_DEBUGS
	debugprintf("varsub_getval: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (varsub_getval) */


static int varsub_writebuf(op,ofp,bufp)
VARSUB		*op ;
bfile		*ofp ;
BUFFER		*bufp ;
{
	int		rs ;
	int		wlen = 0 ;
	const char	*bp ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = buffer_get(bufp,&bp)) > 0) {
	    rs = bwrite(ofp,bp,rs) ;
	    wlen += rs ;
	    if (rs >= 0) buffer_reset(bufp) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (varsub_writebuf) */


static int varsub_entfins(op)
VARSUB		*op ;
{
	VARSUB_SUB	*ep ;
	VECHAND		*elp = &op->subs ; /* loop invariant */
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vechand_get(elp,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;
	    rs1 = entry_finish(ep) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(ep) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end for */

	return rs ;
}
/* end subroutine (varsub_entfins) */


static int entry_start(ep,kp,kl,vp,vl)
VARSUB_SUB	*ep ;
const char	*kp ;
const char	*vp ;
int		kl, vl ;
{
	int		rs ;

	if (kl < 0)
	    kl = strlen(kp) ;

	if (vl < 0)
	    vl = (vp != NULL) ? strlen(vp) : 0 ;

/* allocate buffers for the key and its value respectively */

	if (kl > 0) {
	    const int	size = (kl + 1) + (vl + 1) ;
	    char	*bp ;
	    if ((rs = uc_malloc(size,&bp)) >= 0) {
	        ep->kp = bp ;
	        ep->kl = kl ;
	        bp = strwcpy(bp,kp,kl) + 1 ;
	        ep->vp = bp ;
	        ep->vl = vl ;
	        bp = strwcpy(bp,vp,vl) + 1 ;
	    } /* end if (memory-allocation) */
	} else
	    rs = SR_DOM ;

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(ep)
VARSUB_SUB	*ep ;
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


#if	CF_ENTCMP

static int entry_tmp(ep,kp,kl,vp,vl)
VARSUB_SUB	*ep ;
const char	*kp ;
const char	*vp ;
int		kl, vl ;
{
	int		rs = SR_OK ;

	if (kl < 0) kl = strlen(kp) ;

	if (kl > 0) {
	    ep->kp = kp ;
	    ep->kl = kl ;
	    ep->vp = vp ;
	    ep->vl = (vp != NULL) ? vl : 0 ;
	} else
	    rs = SR_DOM ;

	return rs ;
}
/* end subroutine (entry_tmp) */


static int entry_cmpval(ep,e2p)
VARSUB_SUB	*ep ;
VARSUB_SUB	*e2p ;
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
/* end subroutine (entry_cmpval) */

#endif /* CF_ENTCMP */


#ifdef	COMMENT
static int entry_cmp(ep,e2p)
VARSUB_SUB	*ep ;
VARSUB_SUB	*e2p ;
{
	int		rc = 0 ;

	if (ep == NULL) return SR_FAULT ;
	if (e2p == NULL) return SR_FAULT ;

	if (rc == 0)
	    rc = (ep->kp[0] - e2p->kp[0]) ;
	if (rc == 0)
	    rc = strcmp(ep->kp,e2p->kp) ;
	if (rc == 0) {
	    if (ep->vl == 0) rc = -1 ;
	    if ((rc == 0) && (e2p->vl == 0)) rc = 1 ;
	    if (rc == 0)
	        rc = (ep->vp[0] - e2p->vp[0]) ;
	    if (rc == 0)
	        rc = strcmp(ep->vp,e2p->vp) ;
	}

	return rc ;
}
/* end subroutine (entry_cmp) */
#endif /* COMMENT */


static int vcmpkey(const void **ve1pp,const void **ve2pp)
{
	VARSUB_SUB	**e1pp = (VARSUB_SUB **) ve1pp ;
	VARSUB_SUB	**e2pp = (VARSUB_SUB **) ve2pp ;
	VARSUB_SUB	*e1p ;
	VARSUB_SUB	*e2p ;
	int		rc = 0 ;

	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

	e1p = *e1pp ;
	e2p = *e2pp ;
	rc = (e1p->kp[0] - e2p->kp[0]) ;
	if (rc == 0) {
	    rc = strncmp(e1p->kp,e2p->kp,e2p->kl) ;
	    if (rc == 0)
		rc = (e1p->kl - e2p->kl) ;
	}

	return rc ;
}
/* end subroutine (vcmpkey) */


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
	        const char	*tp ;
	        tp = strnchr(sp,sl,sses[i][1]) ;
	        if (tp != NULL) {
	            kl = (tp - (sp+1)) ;
	        }
	    }

	} /* end if (greater than length one) */

	return kl ;
}
/* end subroutine (getkey) */


