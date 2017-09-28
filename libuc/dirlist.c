/* dirlist */

/* directory list manager */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_NULPATH	1		/* specify NUL-paths */


/* revision history:

	= 2000-09-10, David A­D­ Morano
        I created this modeled after something similar that was used for some of
        my PCS programs.

	= 2017-09-11, David A­D­ Morano
        Whew! Enhanced this module (subroutine |dirlist_add()| in particular) to
        make it more robust in the face of a bug in KSH. The KSH shell adds
        stuff to LD_LIBRARY_PATH to suit what it thinks are its own needs. It
        adds stuff at the front of that path.  But KSH is broken and adds an
	invalid path to the front of LD_LIBRARY_PATH.  We now try to ignore as
	much as possible invalid library-search paths.

*/

/* Copyright © 2000,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages directory lists by:

	+ ensuring unique entries by name
	+ ensuring unique entries by dev-inode pair


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"dirlist.h"


/* local defines */

#define	DIRLIST_ENT	struct dirlist_e
#define	DIRLIST_NDEF	10


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* local structures */

struct dirlist_e {
	const char	*np ;
	uino_t		ino ;
	dev_t		dev ;
	int		nl ;
} ;


/* typesets */

typedef int (*vcmp_t)(DIRLIST_ENT **,DIRLIST_ENT **) ;


/* forward references */

int		dirlist_add(DIRLIST *,cchar *,int) ;

static int	vcmpname(DIRLIST_ENT **,DIRLIST_ENT **) ;
static int	vcmpdevino(DIRLIST_ENT **,DIRLIST_ENT **) ;

static int	entry_start(DIRLIST_ENT *,cchar *,int,dev_t,uino_t) ;
static int	entry_finish(DIRLIST_ENT *) ;


/* local variables */


/* exported subroutines */


int dirlist_start(DIRLIST *op)
{
	const int	size = sizeof(DIRLIST_ENT) ;
	const int	n = DIRLIST_NDEF ;
	const int	vo = VECOBJ_OORDERED ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(DIRLIST)) ;

	if ((rs = vecobj_start(&op->db,size,n,vo)) >= 0) {
	    op->magic = DIRLIST_MAGIC ;
	}

#if	CF_DEBUGS
	debugprintf("dirlist_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dirlist_start) */


int dirlist_finish(DIRLIST *op)
{
	DIRLIST_ENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DIRLIST_MAGIC) return SR_NOTOPEN ;

	for (i = 0 ; vecobj_get(&op->db,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs1 = entry_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	rs1 = vecobj_finish(&op->db) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (dirlist_finish) */


int dirlist_semi(DIRLIST *op)
{
	DIRLIST_ENT	e ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DIRLIST_MAGIC) return SR_NOTOPEN ;

	if ((rs = entry_start(&e,";",1,0L,0LL)) >= 0) {
	    op->tlen += (rs+1) ;
	    rs = vecobj_add(&op->db,&e) ;
	    if (rs < 0)
	        entry_finish(&e) ;
	}

	return rs ;
}
/* end subroutine (dirlist_semi) */


int dirlist_adds(DIRLIST *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		cl ;
	int		c = 0 ;
	const char	*tp, *cp ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	while ((tp = strnpbrk(sp,sl,":; \t,")) != NULL) {

	    cp = sp ;
	    cl = (tp - sp) ;

	    if (rs >= 0) {
	        rs = dirlist_add(op,cp,cl) ;
	        c += rs ;
	    }

	    if ((rs >= 0) && (tp[0] == ';')) {
	        rs = dirlist_semi(op) ;
	    }

	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;

	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (sl > 0)) {
	    rs = dirlist_add(op,sp,sl) ;
	    c += rs ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("dirlist_adds: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (dirlist_adds) */


int dirlist_add(DIRLIST *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		plen ;
	int		f_added = FALSE ;
	char		pbuf[MAXPATHLEN + 1] ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DIRLIST_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("dirlist_add: ent >%t<\n",
	    sp,strlinelen(sp,sl,50)) ;
#endif

	if (sl < 0) sl = strlen(sp) ;

	if ((sl == 0) || ((sl == 1) && (sp[0] == '.'))) {
	    pbuf[0] = '.' ;
	    pbuf[1] = '\0' ;
	    plen = 1 ;
	} else {
	    rs = pathclean(pbuf,sp,sl) ;
	    plen = rs ;
	}

#if	CF_DEBUGS
	debugprintf("dirlist_add: pathclean() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    DIRLIST_ENT	e, *ep ;
	    const int	rsn = SR_NOTFOUND ;

/* any NUL (blank) paths need to be converted to "." */

	    if (plen == 0) {
	        pbuf[0] = '.' ;
	        pbuf[1] = '\0' ;
	        plen = 1 ;
	    }

#if	CF_DEBUGS
	    debugprintf("dirlist_add: apath=>%t<\n",
	        pbuf,strlinelen(pbuf,plen,50)) ;
#endif

/* now see if it is already in the list by NAME */

	    e.np = pbuf ;
	    e.nl = plen ;
	    if ((rs = vecobj_search(&op->db,&e,vcmpname,&ep)) == rsn) {
	        USTAT	sb ;
/* now see if it is already in the list by DEV-INO */
	        if ((rs = u_stat(pbuf,&sb)) >= 0) {
	            if (S_ISDIR(sb.st_mode)) {
	                vcmp_t vc = vcmpdevino ;
	                e.dev = sb.st_dev ;
	                e.ino = sb.st_ino ;
	                if ((rs = vecobj_search(&op->db,&e,vc,&ep)) == rsn) {
	                    dev_t	d = sb.st_dev ;
	                    uino_t	i = sb.st_ino ;
	                    f_added = TRUE ;
	                    if ((rs = entry_start(&e,pbuf,plen,d,i)) >= 0) {
	                        op->tlen += (rs+1) ;
	                        rs = vecobj_add(&op->db,&e) ;
	                        if (rs < 0)
	                            entry_finish(&e) ;
	                    } /* end if (entry_start) */
	                } /* end if (was not found) */
	            } else {
	                rs = SR_NOENT ;
	            } /* end if (isDir) */
		} else if (isNotPresent(rs)) {
		    rs = SR_OK ;
	        } /* end if (stat) */
	    } /* end if (was not found) */

	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("dirlist_add: ret rs=%d f_added=%u\n",rs,f_added) ;
#endif

	return (rs >= 0) ? f_added : rs ;
}
/* end subroutine (dirlist_add) */


int dirlist_strsize(DIRLIST *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DIRLIST_MAGIC) return SR_NOTOPEN ;

	rs = op->tlen ;
	return rs ;
}
/* end subroutine (dirlist_strsize) */


int dirlist_curbegin(DIRLIST *op,DIRLIST_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != DIRLIST_MAGIC) return SR_NOTOPEN ;

#ifdef	OPTIONAL
	memset(curp,0,sizeof(DIRLIST_CUR)) ;
#endif
	curp->i = -1 ;

	return SR_OK ;
}
/* end subroutine (dirlist_curbegin) */


int dirlist_curend(DIRLIST *op,DIRLIST_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != DIRLIST_MAGIC) return SR_NOTOPEN ;

#ifdef	OPTIONAL
	memset(curp,0,sizeof(DIRLIST_CUR)) ;
#endif
	curp->i = -1 ;

	return SR_OK ;
}
/* end subroutine (dirlist_curend) */


int dirlist_enum(DIRLIST *op,DIRLIST_CUR *curp,char *rbuf,int rlen)
{
	DIRLIST_ENT	*ep ;
	int		rs ;
	int		i ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != DIRLIST_MAGIC) return SR_NOTOPEN ;

	i = (curp->i >= 0) ? (curp->i+1) : 0 ;

	while ((rs = vecobj_get(&op->db,i,&ep)) >= 0) {
	    if (ep != NULL) break ;
	    i += 1 ;
	} /* end while */

	if (rs >= 0) {
	    if ((rs = sncpy1w(rbuf,rlen,ep->np,ep->nl)) >= 0) {
	        curp->i = i ;
	    }
	}

	return rs ;
}
/* end subroutine (dirlist_enum) */


int dirlist_get(DIRLIST *op,DIRLIST_CUR *curp,cchar **rpp)
{
	DIRLIST_ENT	*ep ;
	int		rs ;
	int		i ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != DIRLIST_MAGIC) return SR_NOTOPEN ;

	i = (curp->i >= 0) ? (curp->i+1) : 0 ;

	while ((rs = vecobj_get(&op->db,i,&ep)) >= 0) {
	    if (ep != NULL) break ;
	    i += 1 ;
	} /* end while */

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? ep->np : NULL ;
	}

	if (rs >= 0) {
	    curp->i = i ;
	    rs = ep->nl ;
	}

	return rs ;
}
/* end subroutine (dirlist_get) */


int dirlist_joinsize(DIRLIST *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DIRLIST_MAGIC) return SR_NOTOPEN ;

	rs = op->tlen ;
	return rs ;
}
/* end subroutine (dirlist_joinsize) */


int dirlist_joinmk(DIRLIST *op,char *jbuf,int jlen)
{
	int		rs = SR_OK ;
	int		djlen ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (jbuf == NULL) return SR_FAULT ;

	if (op->magic != DIRLIST_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("dirlist_joinmk: jlen=%d\n",jlen) ;
#endif

	djlen = op->tlen ;
	if (jlen >= djlen) {
	    DIRLIST_ENT	*ep ;
	    char	*bp = jbuf ;
	    const char	*dp ;
	    int		dl ;
	    int		i ;
	    int		f_semi = FALSE ;

	    for (i = 0 ; vecobj_get(&op->db,i,&ep) >= 0 ; i += 1) {
	        if (ep == NULL) break ;
	        dp = ep->np ;
	        dl = ep->nl ;

#if	CF_DEBUGS
	        debugprintf("dirlist_joinmk: dl=%d e=>%t<\n",
	            dl,dp,strlinelen(dp,dl,50)) ;
#endif

	        if (dp[0] != ';') {
	            if (c++ > 0) {
	                if (f_semi) {
	                    f_semi = FALSE ;
	                    *bp++ = ';' ;
	                } else
	                    *bp++ = ':' ;
	            }

#if	CF_DEBUGS
	            debugprintf("dirlist_joinmk: pc=>%t<\n",dp,dl) ;
#endif

#if	CF_NULPATH
	            if ((dl > 0) && (! ((dp[0] == '.') && (dl == 1)))) {
	                bp = strwcpy(bp,dp,dl) ;
	            }
#else
	            if (dl > 0) {
	                bp = strwcpy(bp,dp,dl) ;
	            }
#endif
	            c += 1 ;
	        } else
	            f_semi = TRUE ;
	    } /* end for */
	    rs = (bp-jbuf) ;

	} else
	    rs = SR_OVERFLOW ;

#if	CF_DEBUGS
	debugprintf("dirlist_joinmk: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dirlist_joinmk) */


/* private subroutines */


static int entry_start(ep,np,nl,dev,ino)
DIRLIST_ENT	*ep ;
const char	np[] ;
int		nl ;
dev_t		dev ;
uino_t		ino ;
{
	int		rs ;
	const char	*cp ;

	memset(ep,0,sizeof(DIRLIST_ENT)) ;
	ep->dev = dev ;
	ep->ino = ino ;
	ep->nl = nl ;

	rs = uc_mallocstrw(np,nl,&cp) ;
	if (rs >= 0) ep->np = cp ;

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(DIRLIST_ENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep->np != NULL) {
	    rs1 = uc_free(ep->np) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->np = NULL ;
	    ep->nl = 0 ;
	}

	return rs ;
}
/* end subroutine (entry_finish) */


static int vcmpname(DIRLIST_ENT **e1pp,DIRLIST_ENT **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp == NULL) rc = 1 ;
	    else if (*e2pp == NULL) rc = -1 ;
	    if (rc == 0) {
	        rc = strcmp((*e1pp)->np,(*e2pp)->np) ;
	    }
	}
	return rc ;
}
/* end subroutine (vcmpname) */


static int vcmpdevino(DIRLIST_ENT **e1pp,DIRLIST_ENT **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp == NULL) rc = 1 ;
	    else if (*e2pp == NULL) rc = -1 ;
	    if (rc == 0) {
	        if ((rc = ((*e1pp)->ino - (*e2pp)->ino)) == 0) {
	            rc = (*e1pp)->dev - (*e2pp)->dev ;
	        }
	    }
	}
	return rc ;
}
/* end subroutine (vcmpdevino) */


