/* mailmsgviewer */

/* support low-overhead file bufferring requirements */


#define	CF_DEBUGS	0		/* debug print-outs */
#define	CF_SAFE		1		/* safe mode */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little object supports some buffered file operations for
        low-overhead buffered I/O requirements.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"mailmsgviewer.h"


/* local defines */

#define	MAILMSGVIEWER_LINE	struct mailmsgviewer_e

#define	BUFLEN		(2 * MAXPATHLEN)


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local typedefs */


/* local structures */

struct mailmsgviewer_e {
	const char	*lp ;
	int		ll ;
} ;


/* forward references */

static int	mailmsgviewer_findline(MAILMSGVIEWER *,int,const char **) ;


/* local variables */


/* exported subroutines */


int mailmsgviewer_open(MAILMSGVIEWER *op,cchar *fname)
{
	int		rs ;
	int		size ;
	int		opts ;
	int		n = 10 ;

	if (op == NULL) return SR_FAULT ;

	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mailmsgviewer_open: fname=%s\n",fname) ;
#endif

	if (fname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(MAILMSGVIEWER)) ;

	size = sizeof(MAILMSGVIEWER_LINE) ;
	opts = VECOBJ_OCOMPACT ;
	if ((rs = vecobj_start(&op->lines,size,n,opts)) >= 0) {
	    const int	of = O_RDONLY ;
	    if ((rs = uc_open(fname,of,0666)) >= 0) {
	        struct ustat	sb ;
	        int		fd = rs ;
	        if ((rs = u_fstat(fd,&sb)) >= 0) {
	            if (S_ISREG(sb.st_mode)) {
	                if (sb.st_size > 0) {
	                    const size_t	ms = (size_t) sb.st_size ;
	                    const int	mp = PROT_READ ;
	                    const int	mf = MAP_SHARED ;
	                    void	*md ;
	                    if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	                        op->mapbuf = md ;
	                        op->mapsize = ms ;
	                    }
	                } else {
	                    op->f.eof = TRUE ;
	                }
	            } else {
	                rs = SR_PROTO ;
	            }
	        } /* end if (stat) */
	        u_close(fd) ;
	    } /* end if (file) */
	    if (rs < 0)
	        vecobj_finish(&op->lines) ;
	} /* end if (vecobj_start) */

#if	CF_DEBUGS
	debugprintf("mailmsgviewer_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mailmsgviewer_open) */


int mailmsgviewer_close(MAILMSGVIEWER *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mailmsgviewer_close: ent\n") ;
#endif

	if ((op->mapbuf != NULL) && (op->mapsize > 0)) {
	    rs1 = u_munmap(op->mapbuf,op->mapsize) ;
	    if (rs >= 0) rs = rs1 ;
	    op->mapbuf = NULL ;
	    op->mapsize = 0 ;
	}

	rs1 = vecobj_finish(&op->lines) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (mailmsgviewer_close) */


int mailmsgviewer_getline(MAILMSGVIEWER *op,int ln,cchar **lpp)
{
	MAILMSGVIEWER_LINE	*ep ;
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (lpp == NULL) return SR_FAULT ;

	if (op->mapbuf == NULL) return SR_NOTOPEN ;
#endif /* CF_SAFE */

#if	CF_DEBUGS
	debugprintf("mailmsgviewer_getline: f_eof=%u ln=%d\n",op->f.eof,ln) ;
#endif

	if (ln < 0) return SR_INVALID ;

	if ((rs = vecobj_get(&op->lines,ln,&ep)) >= 0) {
	    *lpp = (const char *) ep->lp ;
	    rs = ep->ll ;
	} else if (rs == SR_NOTFOUND) {
	    rs = 0 ;
	    if ((! op->f.eof) && (op->mapbuf != NULL)) {
	        rs = mailmsgviewer_findline(op,ln,lpp) ;
#if	CF_DEBUGS
	        debugprintf("mailmsgviewer_getline: _findline() rs=%d\n",rs) ;
#endif
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("mailmsgviewer_getline: ret rs=%d \n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mailmsgviewer_getline) */


#ifdef	COMMENT

int mailmsgviewer_seek(op,off,w)
MAILMSGVIEWER	*op ;
offset_t	off ;
int		w ;
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->mapbuf == NULL) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	switch (w) {
	case SEEK_SET:
	    break ;
	case SEEK_END:
	    off = (op->mapsize + off) ;
	    break ;
	case SEEK_CUR:
	    off = ((op->bp - op->mapbuf) + off) ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */

	if (rs >= 0) {
	    if (off < 0) {
	        rs = SR_INVALID ;
	    } else if (off > op->mapsize) {
	        off = op->mapsize ;
	    }
	    op->bp = (op->mapbuf + off) ;
	} /* end if */

	return rs ;
}
/* end subroutine (mailmsgviewer_seek) */


int mailmsgviewer_tell(MAILMSGVIEWER *op,offset_t *offp)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->mapbuf == NULL) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (offp == NULL) return SR_FAULT ;

	*offp = (op->bp - op->mapbuf) ;
	return SR_OK ;
}
/* end subroutine (mailmsgviewer_tell) */


int mailmsgviewer_rewind(MAILMSGVIEWER *op)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->mapbuf == NULL) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	op->bp = op->mapbuf ;
	return SR_OK ;
}
/* end subroutine (mailmsgviewer_rewind) */

#endif /* COMMENT */


/* private subroutines */


static int mailmsgviewer_findline(MAILMSGVIEWER *op,int ln,cchar **lpp)
{
	int		rs = SR_OK ;
	int		ll = 0 ;

	*lpp = NULL ;
	if (! op->f.eof) {
	    if ((rs = vecobj_count(&op->lines)) >= 0) {
	        MAILMSGVIEWER_LINE	e, *ep ;
	        int		c = rs ;
	        int		bl ;
	        cchar		*tp ;
	        cchar		*bp ;

#if	CF_DEBUGS
	        debugprintf("mailmsgviewer_findline: lines=%u\n",c) ;
#endif

	        if (c > 0) {
	            if ((rs = vecobj_get(&op->lines,(c - 1),&ep)) >= 0) {
	                bp = (ep->lp + ep->ll) ;
	                bl = (op->mapsize - (bp - op->mapbuf)) ;
	            }
	        } else {
	            bp = op->mapbuf ;
	            bl = op->mapsize ;
	        } /* end if */

#if	CF_DEBUGS
	        debugprintf("mailmsgviewer_findline: mid rs=%d bl=%u\n",rs,bl) ;
	        debugprintf("mailmsgviewer_findline: bufline=>%t<\n",
	            bp,strlinelen(bp,bl,45)) ;
#endif

	        if (rs >= 0) {

	            while ((tp = strnchr(bp,bl,'\n')) != NULL) {

	                *lpp = bp ;
	                ll = ((tp + 1) - bp) ;

#if	CF_DEBUGS
	                debugprintf("mailmsgviewer_findline: line=>%t<\n",
	                    bp,strlinelen(bp,ll,45)) ;
#endif

	                e.lp = bp ;
	                e.ll = ((tp + 1) - bp) ;
	                rs = vecobj_add(&op->lines,&e) ;
	                if (rs < 0) break ;

	                if (c++ == ln) break ;

	                bl -= ((tp + 1) - bp) ;
	                bp = (tp + 1) ;

	            } /* end while */

	            if ((rs >= 0) && (tp == NULL)) {

	                op->f.eof = TRUE ;
	                if (bl > 0) {

	                    *lpp = bp ;
	                    ll = bl ;

	                    e.lp = bp ;
	                    e.ll = bl ;
	                    rs = vecobj_add(&op->lines,&e) ;

	                } /* end if */

	            } /* end if */

	        } /* end if (ok) */

	    } /* end if (vecobj_count) */
	} /* end if (not-EOF) */

#if	CF_DEBUGS
	debugprintf("mailmsgviewer_findline: ret rs=%d ll=%u\n",rs,ll) ;
	if (*lpp != NULL)
	    debugprintf("mailmsgviewer_findline: ret line=>%t<\n",
	        *lpp,strlinelen(*lpp,ll,45)) ;
#endif

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (mailmsgviewer_findline) */


