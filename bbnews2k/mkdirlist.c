/* mkdirlist */

/* create a list of the newsgroup directories */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1994-04-13, David A­D­ Morano

	This is new so that we can track directory visits for duplicates.

	= 2014-11-25, David A­D­ Morano

	This object was enhanced to include much of the functionality from the
	old 'get_bds' function.

*/

/* Copyright © 1994,2014 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Create a list of pathnames for each newsgroup in the spool area.  This
	routine is called with the path to the spool area directory.  The
	routine returns two open file pointers (Basic I/O).  These file
	pointers must be eventually closed by the calling routine or somebody!

	One returned file pointer is to a file of the path names.  The other
	file pointer is a file of an array of 'stat(2)' structures
	corresponding to the directory specified by the path in the other
	file.

	Synopsis:

	int mkdirlist(basedir,sfp,nfp,uf)
	bfile	*sfp, *nfp ;
	char	*basedir ;
	int	(*uf)() ;

	Arguments:

	basedir		directory at top of tree
	nfp		Name File Pointer
	sfp		Stat File Pointer

	Returns:

	<0		error
	>=0		count


*******************************************************************************/


#define	MKDIRLIST_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<filebuf.h>
#include	<fsdirtree.h>
#include	<localmisc.h>

#include	"mkdirlist.h"


/* local defines */

#define	MKDIRLIST_DIRCACHE	".dircache"
#define	MKDIRLIST_DCMAGIC	"DIRCACHE"

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 10),2048)
#endif

#define	DS_SIZE		sizeof(MKDIRLIST)

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	pathadd(char *,int,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	strwcmp(const char *,const char *,int) ;

extern int	openpcsdircache(const char *,const char *,int,mode_t,int) ;
extern int	isNotPresent(int) ;

extern int	bbcmp(const char *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */

static int mkdirlist_procdircache(MKDIRLIST *,const char *,int) ;
static int mkdirlist_procnewsdir(MKDIRLIST *,const char *) ;
static int mkdirlist_newentry(MKDIRLIST *,struct ustat *,const char *,int) ;
static int mkdirlist_entfins(MKDIRLIST *) ;

static int entry_start(MKDIRLIST_ENT *,struct ustat *,const char *,int) ;
static int entry_finish(MKDIRLIST_ENT *) ;
static int entry_showdef(MKDIRLIST_ENT *) ;
static int entry_show(MKDIRLIST_ENT *,const char *,int) ;
static int entry_matung(MKDIRLIST_ENT *,const char *,time_t,int,int) ;

static int ordercmp(MKDIRLIST_ENT **,MKDIRLIST_ENT **) ;


/* local variables */


/* exported subroutines */


int mkdirlist_start(MKDIRLIST *op,const char *pr,const char *newsdname)
{
	int	rs ;
	int	c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;
	if (newsdname == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;
	if (newsdname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(MKDIRLIST)) ;

	if ((rs = vechand_start(&op->dirs,20,0)) >= 0) {
	    const mode_t	om = 0666 ;
	    const int		of = O_RDONLY ;
	    if ((rs = openpcsdircache(pr,newsdname,of,om,-1)) >= 0) {
	        int	fd = rs ;
#if	CF_DEBUGS
	        debugprintf("mkdirlist_start: dir-cache\n") ;
#endif
	        rs = mkdirlist_procdircache(op,newsdname,fd) ;
	        c = rs ;
#if	CF_DEBUGS
	        debugprintf("mkdirlist_start: _procdircache() rs=%d\n",rs) ;
#endif

	        u_close(fd) ;
	    } else if (isNotPresent(rs)) {
#if	CF_DEBUGS
	        debugprintf("mkdirlist_start: read-dir \n") ;
#endif
	        rs = mkdirlist_procnewsdir(op,newsdname) ;
	        c = rs ;
	    }
	    if (rs >= 0) op->magic = MKDIRLIST_MAGIC ;
	    if (rs < 0)
	        mkdirlist_entfins(op) ;
	    if (rs < 0)
	        vechand_finish(&op->dirs) ;
	} /* end if (vechand) */

#if	CF_DEBUGS
	debugprintf("mkdirlist_start: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mkdirlist_start) */


int mkdirlist_finish(MKDIRLIST *op)
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MKDIRLIST_MAGIC) return SR_NOTOPEN ;

	rs1 = mkdirlist_entfins(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&op->dirs) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (mkdirlist_finish) */


int mkdirlist_get(MKDIRLIST *op,int i,MKDIRLIST_ENT **epp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (op->magic != MKDIRLIST_MAGIC) return SR_NOTOPEN ;

	rs = vechand_get(&op->dirs,i,epp) ;

	return rs ;
}
/* end subroutine (mkdirlist_get) */


int mkdirlist_link(MKDIRLIST *op)
{
	MKDIRLIST_ENT	*pep, *ep ;
	vechand		*dlp ;
	int		rs = SR_OK ;
	int		i, j ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MKDIRLIST_MAGIC) return SR_NOTOPEN ;

	dlp = &op->dirs ;
	for (i = 0 ; vechand_get(dlp,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;

	    if (! ep->f.link) {
	        pep = ep ;

	        for (j = (i+1) ; vechand_get(dlp,j,&ep) >= 0 ; j += 1) {
	            if (ep == NULL) continue ;

	            if ((! ep->f.link) && (bbcmp(pep->name,ep->name) == 0)) {

	                pep->link = ep ;
	                ep->f.link = TRUE ;

#if	CF_DEBUGS
	                debugprintf("mkdirlist_link: linked a NG=%s\n",
	                    pep->name) ;
#endif

	                pep = ep ;
	            } /* end if (board match) */

	        } /* end for (inner) */

	    } /* end if (entry not linked) */

	} /* end for (linking like entries) */

	return rs ;
}
/* end subroutine (mkdirlist_link) */


int mkdirlist_showdef(MKDIRLIST *op)
{
	MKDIRLIST_ENT	*ep ;
	int	rs = SR_OK ;
	int	i ;
	int	c = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MKDIRLIST_MAGIC) return SR_NOTOPEN ;

	for (i = 0 ; vechand_get(&op->dirs,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;
	    rs = entry_showdef(ep) ;
	    c += rs ;
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mkdirlist_showdef) */


int mkdirlist_show(MKDIRLIST *op,const char *ng,int order)
{
	MKDIRLIST_ENT	*ep ;
	int	rs = SR_OK ;
	int	i ;
	int	c = 0 ;

#if	CF_DEBUGS
	debugprintf("mkdirlist_show: ng=%s\n",ng) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (ng == NULL) return SR_FAULT ;

	if (op->magic != MKDIRLIST_MAGIC) return SR_NOTOPEN ;
	if (ng[0] == '\0') return SR_INVALID ;

	for (i = 0 ; vechand_get(&op->dirs,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;
#if	CF_DEBUGS
	    debugprintf("mkdirlist_show: name=%s\n",ep->name) ;
#endif
	    rs = entry_show(ep,ng,order) ;
	    c += rs ;
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("mkdirlist_show: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mkdirlist_show) */


int mkdirlist_ung(op,ung,utime,f_sub,order)
MKDIRLIST	*op ;
const char	ung[] ;
time_t		utime ;
int		f_sub ;
int		order ;
{
	MKDIRLIST_ENT	*ep ;
	VECHAND		*dlp ;
	int	rs = SR_OK ;
	int	i ;
	int	c = 0 ;

#if	CF_DEBUGS
	debugprintf("mkdirlist_ung: entered ung=%s\n",ung) ;
	debugprintf("mkdirlist_ung: f_sub=%u\n",f_sub) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (ung == NULL) return SR_FAULT ;

	if (op->magic != MKDIRLIST_MAGIC) return SR_NOTOPEN ;
	if (ung[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("mkdirlist_ung: loop\n") ;
#endif

	dlp = &op->dirs ;
	for (i = 0 ; vechand_get(dlp,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;
	    rs = entry_matung(ep,ung,utime,f_sub,order) ;
	    c += rs ;

#if	CF_DEBUGS
	    debugprintf("mkdirlist_ung: entry_matung() rs=%d\n",rs) ;
#endif

	    if (rs < 0) break ;
	} /* end for (looping through entrires) */

#if	CF_DEBUGS
	debugprintf("mkdirlist_ung: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mkdirlist_ung) */


int mkdirlist_sort(MKDIRLIST *op)
{
	int	rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MKDIRLIST_MAGIC) return SR_NOTOPEN ;

	rs = vechand_sort(&op->dirs,ordercmp) ;

	return rs ;
}
/* end subroutine (mkdirlist_sort) */


int mkdirlist_audit(op)
MKDIRLIST	*op ;
{
	VECHAND		*dlp ;
	int	rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("mkdirlist_audit: entered\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MKDIRLIST_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("mkdirlist_audit: continue\n") ;
#endif

	dlp = &op->dirs ;
	rs = vechand_audit(dlp) ;

#if	CF_DEBUGS
	debugprintf("mkdirlist_audit: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mkdirlist_audit) */


/* private subroutines */


static int mkdirlist_procdircache(MKDIRLIST *op,const char *newsdname,int fd)
{
	int	rs ;
	int	c = 0 ;

	const char	*dcm = MKDIRLIST_DCMAGIC ;

	char	dbuf[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	debugprintf("mkdirlist_procdircache: nd=%s\n",newsdname) ;
#endif

	if ((rs = mkpath1(dbuf,newsdname)) >= 0) {
	    FILEBUF	b ;
	    int	dlen = rs ;

	    if ((rs = filebuf_start(&b,fd,0L,0,0)) >= 0) {
	        struct ustat	sb ;
	        const int	nlen = MAXPATHLEN ;
	        int	line = 0 ;
	        int	f_bol = TRUE ;
	        int	f_eol ;
	        char	nbuf[MAXPATHLEN+1] ;
	        while ((rs = filebuf_readline(&b,nbuf,nlen,-1)) > 0) {
	            int	len = rs ;
	            f_eol = (len && (nbuf[len-1] == '\n')) ;
	            if (f_eol) nbuf[--len] = '\0' ;
	            if (f_bol) {

#if	CF_DEBUGS
	debugprintf("mkdirlist_procdircache: n=>%t<\n",nbuf,len) ;
#endif
	                if (line++ == 0) {
	                    if (strwcmp(dcm,nbuf,len) != 0) {
	                        rs = SR_BADFMT ;
	                    }
	                } else {
	                    if ((rs = pathadd(dbuf,dlen,nbuf)) >= 0) {
#if	CF_DEBUGS
	debugprintf("mkdirlist_procdircache: dbuf=>%s<\n",dbuf) ;
#endif
	                        if (u_stat(dbuf,&sb) >= 0) {
#if	CF_DEBUGS
	debugprintf("mkdirlist_procdircache: new-entry\n") ;
#endif
	                            rs = mkdirlist_newentry(op,&sb,nbuf,len) ;
	                            c += rs ;
	                        }
	                    }
	                }
	            } /* end if (BOL) */
	            f_bol = f_eol ;
	            if (rs < 0) break ;
	        } /* end while */
	        filebuf_finish(&b) ;
	    } /* end if */

	} /* end if (mkpath) */

#if	CF_DEBUGS
	debugprintf("mkdirlist_procdircache: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mkdirlist_procdircache) */


static int mkdirlist_procnewsdir(op,newsdname)
MKDIRLIST	*op ;
const char	newsdname[] ;
{
	FSDIRTREE	dir ;

	int	rs ;
	int	opts = 0 ;
	int	c = 0 ;

	opts = (FSDIRTREE_MFOLLOW | FSDIRTREE_MDIR) ;
	if ((rs = fsdirtree_open(&dir,newsdname,opts)) >= 0) {
	    struct ustat	sb ;
	    const int	nglen = MAXPATHLEN ;
	    char	ngdname[MAXPATHLEN+1] ;

	    while ((rs = fsdirtree_read(&dir,&sb,ngdname,nglen)) > 0) {
	        int	ngl = rs ;

#if	CF_DEBUGS
	        debugprintf("progdname: name=%t\n",ngdname,ngl) ;
#endif

	        if (ngdname[0] != '.') {
	            rs = mkdirlist_newentry(op,&sb,ngdname,ngl) ;
	            c += rs ;
	        }

	        if (rs < 0) break ;
	    } /* end while */

	    fsdirtree_close(&dir) ;
	} /* end if (fsdirtree) */

#if	CF_DEBUGS
	debugprintf("progdname: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mkdirlist_procnewsdir) */


static int mkdirlist_newentry(op,sbp,nbuf,nlen)
MKDIRLIST	*op ;
struct ustat	*sbp ;
const char	nbuf[] ;
int		nlen ;
{
	const int	esize = sizeof(MKDIRLIST_ENT) ;

	int	rs ;
	int	c = 0 ;

	void	*p ;

#if	CF_DEBUGS
	debugprintf("mkdirlist_newentry: n=%t\n",nbuf,nlen) ;
#endif
	if ((rs = uc_malloc(esize,&p)) >= 0) {
	    MKDIRLIST_ENT	*ep = (MKDIRLIST_ENT *) p ;
	    if ((rs = entry_start(ep,sbp,nbuf,nlen)) > 0) { /* rs>0 */
	        c = rs ;
	        rs = vechand_add(&op->dirs,ep) ;
	        if (rs < 0)
	            entry_finish(ep) ;
	    } /* end if (entry) */
	    if ((rs < 0) || (c == 0))
	        uc_free(p) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("mkdirlist_newentry: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mkdirlist_newentry) */


static int mkdirlist_entfins(MKDIRLIST *op)
{
	VECHAND		*dlp = &op->dirs ;
	MKDIRLIST_ENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vechand_get(dlp,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;
	    rs1 = entry_finish(ep) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(ep) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end for */

	return rs ;
}
/* end subroutine (mkdirlist_entfins) */


static int entry_start(ep,sbp,dbuf,dlen)
MKDIRLIST_ENT	*ep ;
struct ustat	*sbp ;
const char	*dbuf ;
int		dlen ;
{
	int		rs ;
	int		c = 0 ;
	const char	*cp ;

	memset(ep,0,sizeof(MKDIRLIST_ENT)) ;

	if ((rs = uc_mallocstrw(dbuf,dlen,&cp)) >= 0) {
	    const int		nlen = (rs-1) ;
	    ep->name = cp ;
	    c += 1 ;
	    ep->nlen = nlen ;
	    ep->mode = sbp->st_mode ;
	    ep->mtime = sbp->st_mtime ;
	    ep->ino = sbp->st_ino ;
	    ep->dev = sbp->st_dev ;
	} /* end if (memory-allocation) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(MKDIRLIST_ENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep->name != NULL) {
	    rs1 = uc_free(ep->name) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->name = NULL ;
	}

	return rs ;
}
/* end subroutine (entry_finish) */


static int entry_matung(ep,ung,utime,f_sub,order)
MKDIRLIST_ENT	*ep ;
const char	ung[] ;
time_t		utime ;
int		f_sub ;
int		order ;
{
	int		rs = SR_OK ;

	if (! ep->f.link) {
	    if (bbcmp(ung,ep->name) == 0) {
	        rs = 1 ;
	        ep->f.seen = TRUE ;
	        ep->f.subscribe = f_sub ;
	        ep->utime = utime ;
		ep->order = order ;
	    } /* end if (name match) */
	} /* end if (not a linked entry) */

	return rs ;
}
/* end subroutine (entry_matung) */


static int entry_showdef(MKDIRLIST_ENT *ep)
{
	int		rs = SR_OK ;

	if (!ep->f.link) {
	    ep->f.show = ep->f.subscribe ;
	    if (ep->f.show) rs = 1 ;
	}

#if	CF_DEBUGS
	debugprintf("mkdirlist/entry_showdef: ng=%s rs=%d\n",ep->name,rs) ;
#endif
	return rs ;
}
/* end subroutine (entry_showdef) */


static int entry_show(MKDIRLIST_ENT *ep,const char *ng,int order)
{
	int		rs = SR_OK ;
#if	CF_DEBUGS
	debugprintf("mkdirlist/entry_show: ng=%s\n",ng) ;
	debugprintf("mkdirlist/entry_show: name=%s\n",ep->name) ;
	debugprintf("mkdirlist/entry_show: f_link=%u\n",ep->f.link) ;
#endif
	if (! ep->f.link) {
	    if (bbcmp(ng,ep->name) == 0) {
		ep->order = order ;
	        ep->f.show = TRUE ;
	        rs = 1 ;
	    } /* end if (name match) */
	} /* end if (not a linked entry) */
#if	CF_DEBUGS
	debugprintf("mkdirlist/entry_show: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (entry_show) */


static int ordercmp(MKDIRLIST_ENT **e1pp,MKDIRLIST_ENT **e2pp)
{
	MKDIRLIST_ENT	*e1p ;
	MKDIRLIST_ENT	*e2p ;
	int		rc = 0 ;

	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
		    rc = ((*e1pp)->order - (*e2pp)->order) ;
		} else
		    rc = -1 ;
	    } else
		rc = 1 ;
	}

	return rc ;
}
/* end subroutine (ordercmp) */


