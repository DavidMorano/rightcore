/* dw */

/* directory watch */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_FNAMECMP	1		/* ? */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was written for use in the 'rbbpost' daemon program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object is used to watch a directory path for additions or changes
        in the file objects under that path. Subdirectories are allowed. All
        files under the directory path (including those in subdirectories) are
        treated the same as an entry.


*******************************************************************************/


#define	DW_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<regexpr.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<vecstr.h>
#include	<fsdir.h>
#include	<localmisc.h>

#include	"dw.h"


/* local defines */

#define	IENTRY		struct dw_ientry

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	BUFLEN		(2 * MAXPATHLEN)

#define	MAXOPENTIME	300		/* maximum FD cache time */
#define	MINCHECKTIME	2		/* minumun check interval (seconds) */
#define	MAXIDLETIME	240		/* maximum allowable idle time */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;


/* external variables */


/* forward references */

static int	dw_scanfull(DW *) ;
static int	dw_scansub(DW *,cchar *,time_t) ;
static int	dw_findi(DW *,cchar *,IENTRY **) ;

static int	ientry_start(IENTRY *,DW *,cchar *,struct ustat *) ;
static int	ientry_finish(IENTRY *,DW *) ;

static int	entry_load(DW_ENT *,struct dw_ientry *) ;

static int	fnamecmp(struct dw_ientry **,struct dw_ientry **) ;


/* local variables */


/* exported subroutines */


int dw_start(DW *dwp,cchar *dirname)
{
	int		rs ;
	int		size ;
	int		opts ;

	if (dwp == NULL) return SR_FAULT ;
	if (dirname == NULL) return SR_FAULT ;

	if (dirname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("dw_start: ent dirname=%s\n",dirname) ;
#endif

	memset(dwp,0,sizeof(DW)) ;
	dwp->fd = -1 ;
	dwp->count_new = 0 ;
	dwp->count_checkable = 0 ;
	dwp->checkinterval = DW_DEFCHECKTIME ;
	dwp->f.subdirs = FALSE ;

/* initialize */

	size = sizeof(DW_ENT) ;
	opts = (VECOBJ_OSTATIONARY | VECOBJ_OCONSERVE) ;
	if ((rs = vecobj_start(&dwp->e,size,10,opts)) >= 0) {
	    const time_t	dt = time(NULL) ;
	    cchar	*cp ;
	    dwp->mtime = 0 ;
	    dwp->opentime = 0 ;
	    dwp->checktime = dt ;
	    dwp->removetime = dt ;
	    if ((rs = uc_mallocstrw(dirname,-1,&cp)) >= 0) {
	        if ((rs-1) <= MAXPATHLEN) {
	            dwp->dirname = cp ;
	            rs = dw_scanfull(dwp) ;
	            if (rs == SR_NOENT) rs = SR_OK ;
	            dwp->magic = DW_MAGIC ;
	        } else {
	            rs = SR_TOOBIG ;
	        }
	        if (rs < 0)
	            uc_free(cp) ;
	    } /* end if (m-a) */
	    if (rs < 0)
	        vecobj_finish(&dwp->e) ;
	} /* end if (vecobj-start) */

#if	CF_DEBUGS
	debugprintf("dw_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dw_start) */


/* free up the resources occupied by a DW object */
int dw_finish(DW *dwp)
{
	IENTRY		*iep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (dwp == NULL) return SR_FAULT ;

	if (dwp->magic != DW_MAGIC) return SR_NOTOPEN ;

/* directory entries */

	for (i = 0 ; vecobj_get(&dwp->e,i,&iep) >= 0 ; i += 1) {
	    if (iep != NULL) {
	        rs1 = ientry_finish(iep,dwp) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	rs1 = vecobj_finish(&dwp->e) ;
	if (rs >= 0) rs = rs1 ;

	if (dwp->dirname != NULL) {
	    rs1 = uc_free(dwp->dirname) ;
	    if (rs >= 0) rs = rs1 ;
	    dwp->dirname = NULL ;
	}

	if (dwp->fd >= 0) {
	    rs1 = u_close(dwp->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    dwp->fd = -1 ;
	}

	if (dwp->f.subdirs) {
	    dwp->f.subdirs = FALSE ;
	    rs1 = vecstr_finish(&dwp->subdirs) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

	dwp->fd = -1 ;
	dwp->magic = 0 ;
	return rs ;
}
/* end subroutine (dw_finish) */


int dw_curbegin(DW *dwp,DW_CUR *cp)
{

	if (dwp == NULL) return SR_FAULT ;

	if (dwp->magic != DW_MAGIC) return SR_NOTOPEN ;

	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (dw_curbegin) */


int dw_curend(DW *dwp,DW_CUR *cp)
{

	if (dwp == NULL) return SR_FAULT ;

	if (dwp->magic != DW_MAGIC) return SR_NOTOPEN ;

	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (dw_curend) */


/* enumerate the directory entries */
int dw_enum(DW *dwp,DW_CUR *cp,DW_ENT *dep)
{
	IENTRY		*iep ;
	int		rs ;
	int		i ;
	int		nlen ;

	if (dwp == NULL) return SR_FAULT ;

	if (dwp->magic != DW_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("dw_enum: ent\n") ;
#endif

/* get the next entry (any one) */

	if ((cp == NULL) || (cp->i < 0)) {
	    i = 0 ;
	} else {
	    i = cp->i + 1 ;
	}

	while ((rs = vecobj_get(&dwp->e,i,&iep)) >= 0) {
	    if (iep != NULL) break ;
	    i += 1 ;
	} /* end while */

	if (rs >= 0) {

	    if (dep != NULL) {
	        rs = entry_load(dep,iep) ;
	    }

	    nlen = rs ;
	    if ((rs >= 0) && (cp != NULL))
	        cp->i = i ;

	} /* end if (ok) */

	if ((cp != NULL) && (rs >= 0)) {
	    cp->i = i ;
	}

	return (rs >= 0) ? nlen : rs ;
}
/* end subroutine (dw_enum) */


/* delete the entry under the curosr */
int dw_del(DW *dwp,DW_CUR *cp)
{
	IENTRY		*iep = NULL ;
	int		rs ;

	if (dwp == NULL) return SR_FAULT ;
	if (cp == NULL) return SR_FAULT ;

	if (dwp->magic != DW_MAGIC) return SR_NOTOPEN ;

	if ((rs = vecobj_get(&dwp->e,cp->i,&iep)) >= 0) {
	    if (iep != NULL) {
	        ientry_finish(iep,dwp) ;
	        rs = vecobj_del(&dwp->e,cp->i) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (dw_del) */


/* search the directory file list for an entry */
int dw_find(DW *dwp,cchar *name,DW_ENT *dep)
{
	IENTRY		ie, *iep ;
	int		rs ;
	int		i = 0 ;

	if (dwp == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;

	if (dwp->magic != DW_MAGIC) return SR_NOTOPEN ;

	if (name[0] == '\0') return SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("dw_find: ent name=%s\n",name) ;
#endif

#if	CF_FNAMECMP
	ie.name = (char *) name ;
	rs = vecobj_search(&dwp->e,&ie,fnamecmp,&iep) ;
#else /* CF_FNAMECMP */
	for (i = 0 ; (rs = vecobj_get(&dwp->e,i,&iep)) >= 0 ; i += 1) {
	    if (iep != NULL) {

#if	CF_DEBUGS
	    debugprintf("dw_find: got entry=%s\n",iep->name) ;
#endif

	        if (strcmp(name,iep->name) == 0) break ;

	    }
	} /* end for (looping through entries) */
#endif /* CF_FNAMECMP */

	if ((rs >= 0) && (dep != NULL)) {
	    rs = entry_load(dep,iep) ;
	}

#if	CF_DEBUGS
	debugprintf("dw_find: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (dw_find) */


/* enumerate those entries that are "checkable" */
int dw_enumcheckable(DW *dwp,DW_CUR *cp,DW_ENT *dep)
{
	IENTRY		*iep ;
	int		rs ;
	int		i ;

	if (dwp == NULL) return SR_FAULT ;

	if (dwp->magic != DW_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("dw_enumcheckable: ent\n") ;
#endif

/* get the next entry with a checkable file */

	if ((cp == NULL) || (cp->i < 0)) {
	    i = 0 ;
	} else {
	    i = cp->i + 1 ;
	}

#if	CF_DEBUGS
	debugprintf("dw_enumcheckable: i=%u\n",i) ;
#endif

	while ((rs = vecobj_get(&dwp->e,i,&iep)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("dw_enumcheckable: vecobj_get() rs=%d i=%u\n",rs,i) ;
#endif

	    if ((iep != NULL) && (iep->state == DW_SCHECK)) break ;

	    i += 1 ;

	} /* end while */

	if (rs >= 0) {

	    if (dep != NULL) {
	        rs = entry_load(dep,iep) ;
	    }

	    if ((rs >= 0) && (cp != NULL))
	        cp->i = i ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("dw_enumcheckable: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (dw_enumcheckable) */


/* check if the directory (and any subdirectories) has changed */
int dw_check(DW *dwp,time_t daytime)
{
	struct ustat	sb ;
	IENTRY		*iep ;
	int		rs = SR_OK ;
	int		i ;
	int		n = 0 ;

#if	CF_DEBUGS
	debugprintf("dw_check: ent\n") ;
#endif

	if (dwp == NULL) return SR_FAULT ;

	if (dwp->magic != DW_MAGIC) return SR_NOTOPEN ;

	if (daytime <= 0) daytime = time(NULL) ;

/* should we even check? */

	if ((daytime - dwp->checktime) <= MINCHECKTIME)
	    goto ret0 ;

/* perform the directory time check */

#if	CF_DEBUGS
	debugprintf("dw_check: continuing w/ check\n") ;
#endif

	if (dwp->fd < 0) {

	    rs = u_open(dwp->dirname,O_RDONLY,0666) ;
	    dwp->fd = rs ;
	    if (rs >= 0) {
	        dwp->opentime = daytime ;
	        rs = uc_closeonexec(dwp->fd,TRUE) ;
	    }

	} /* end if (opened FD for caching) */
	if (rs < 0) goto ret0 ;

	if ((rs = u_fstat(dwp->fd,&sb)) < 0) {
	    u_close(dwp->fd) ;
	    dwp->fd = -1 ;
	    goto done ;
	}

	if ((sb.st_mtime > dwp->mtime) ||
	    ((daytime - dwp->checktime) > MAXIDLETIME)) {

#if	CF_DEBUGS
	    debugprintf("dw_check: scanning outer directory\n") ;
#endif

	    dwp->mtime = sb.st_mtime ;
	    rs = dw_scansub(dwp,"",daytime) ;
	    if (rs >= 0) n = rs ;

	} else if ((daytime - dwp->opentime) > MAXOPENTIME) {

	    u_close(dwp->fd) ;
	    dwp->fd = -1 ;

	} /* end if */

/* OK, look through the subdirectories and see if any of them need scanning */

	if (dwp->f.subdirs) {
	    const char	*dnp ;
	    char	dnamebuf[MAXPATHLEN + 1], *dbp ;


#if	CF_DEBUGS
	    debugprintf("dw_check: scanning subdirectories\n") ;
#endif

	    dbp = dnamebuf ;
	    dbp = strwcpy(dbp,dwp->dirname,-1) ;

	    *dbp++ = '/' ;
	    for (i = 0 ; vecstr_get(&dwp->subdirs,i,&dnp) >= 0 ; i += 1) {
	        if (dnp != NULL) {

	        {
	            int	rl = (MAXPATHLEN - (dbp-dnamebuf)) ;
	            strdcpy1(dbp,rl,dnp) ;
	        }

	        if ((u_stat(dnamebuf,&sb) >= 0) &&
	            ((sb.st_mtime > dwp->checktime) ||
	            ((daytime - dwp->checktime) > MAXIDLETIME))) {

#if	CF_DEBUGS
	            debugprintf("dw_check: scanning subdirectory=%s\n",
	                dnp) ;
#endif

	            rs = dw_scansub(dwp,dnp,daytime) ;
	            if (rs >= 0) n += 1 ;

	        } /* end if (scanning subdirectory) */

		}
	        if (rs < 0) break ;
	    } /* end for */

	} /* end if (subdirectories) */

/* OK, now check all files that are 'NEW' and see if they are older! */

	if ((rs >= 0) && (dwp->count_new > 0)) {
	    char	dnamebuf[MAXPATHLEN + 1], *dbp ;

#if	CF_DEBUGS
	    debugprintf("dw_check: checking for NEW entries\n") ;
#endif

	    dbp = dnamebuf ;
	    dbp = strwcpy(dbp,dwp->dirname,-1) ;

	    *dbp++ = '/' ;
	    for (i = 0 ; vecobj_get(&dwp->e,i,&iep) >= 0 ; i += 1) {
	        if (iep == NULL) continue ;

	        if (iep->state != DW_SNEW) continue ;

	        if ((daytime - iep->itime) > (dwp->checkinterval / 4)) {

	            {
	                int	rl = (MAXPATHLEN - (dbp-dnamebuf)) ;
	                strdcpy1(dbp,rl,iep->name) ;
	            }

	            if ((u_stat(dnamebuf,&sb) >= 0) &&
	                ((daytime - sb.st_mtime) > dwp->checkinterval)) {

#if	CF_DEBUGS
	                debugprintf("dw_check: checkable entry=%s\n",
	                    iep->name) ;
#endif

	                iep->state = DW_SCHECK ;
	                dwp->count_new -= 1 ;
	                dwp->count_checkable += 1 ;
	                n += 1 ;

	            } /* end if */

	        } /* end if (checkinterval) */

	    } /* end for */

	} /* end if */

/* OK, we are on a roll now !, check for files that have been removed ! */

	if ((rs >= 0) && ((daytime - dwp->removetime) > MAXIDLETIME) &&
	    (vecobj_count(&dwp->e) > 0)) {

	    char	dnamebuf[MAXPATHLEN + 1], *dbp ;

#if	CF_DEBUGS
	    debugprintf("dw_check: checking for removed files\n") ;
#endif

	    dwp->removetime = daytime ;
	    dbp = dnamebuf ;
	    dbp = strwcpy(dbp,dwp->dirname,-1) ;

	    *dbp++ = '/' ;
	    for (i = 0 ; vecobj_get(&dwp->e,i,&iep) >= 0 ; i += 1) {
	        if (iep == NULL) continue ;

	        if ((daytime - iep->itime) <= MAXIDLETIME)
	            continue ;

	        iep->itime = daytime ;
	        {
	            int	rl = (MAXPATHLEN - (dbp-dnamebuf)) ;
	            strdcpy1(dbp,rl,iep->name) ;
	        }

	        if (u_stat(dnamebuf,&sb) < 0) {

#if	CF_DEBUGS
	            debugprintf("dw_check: file removed? entry=%s\n",
	                iep->name) ;
#endif

	            ientry_finish(iep,dwp) ;

	            vecobj_del(&dwp->e,i--) ;

	        } /* end if (could not 'stat') */

	    } /* end for */

	} /* end if (checking for removed files) */

done:
	dwp->checktime = daytime ;

ret0:

#if	CF_DEBUGS
	debugprintf("dw_check: ret rs=%d n=%d\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (dw_check) */


int dw_callback(op,func,argp)
DW		*op ;
void		(*func)(DW_ENT *,int,void *) ;
void		*argp ;
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DW_MAGIC) return SR_NOTOPEN ;

	op->callback = func ;
	op->argp = argp ;
	return SR_OK ;
}
/* end subroutine (dw_callback) */


extern int dw_state(DW *op,int i,int state)
{
	IENTRY		*iep ;
	int		rs ;
	int		state_prev = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DW_MAGIC) return SR_NOTOPEN ;

	if (state < 0) return SR_INVALID ;

	if ((rs = vecobj_get(&op->e,i,&iep)) >= 0) {
	    state_prev = iep->state ;
	    iep->state = state ;
	}

	return (rs >= 0) ? state_prev : rs ;
}
/* end subroutine (dw_state) */


/* private subroutines */


/* perform a full (approximately) initial scan of the directory tree */
static int dw_scanfull(DW *dwp)
{
	struct ustat	sb ;
	IENTRY		ie, *iep ;
	fsdir		d ;
	fsdir_ent	ds ;
	time_t		daytime = time(NULL) ;
	int		rs ;
	int		nlen, i ;
	int		n = 0 ;
	char		dnamebuf[MAXPATHLEN + 1], *dbp ;

	if (dwp->dirname == NULL) return SR_FAULT ;

	if (dwp->dirname[0] == '\0') return SR_INVALID ;

/* "do" the outer directory */

	if ((rs = fsdir_open(&d,dwp->dirname)) >= 0) {

	    dbp = dnamebuf ;
	    dbp = strwcpy(dbp,dwp->dirname,-1) ;

	    *dbp++ = '/' ;
	    while ((nlen = fsdir_read(&d,&ds)) > 0) {
	        if (ds.name[0] == '.') continue ;

	        {
	            int	rl = (MAXPATHLEN - (dbp-dnamebuf)) ;
	            strdcpy1(dbp,rl,ds.name) ;
	        }

	        if (u_stat(dnamebuf,&sb) < 0) continue ;

	        if (S_ISDIR(sb.st_mode)) {

	            if (! dwp->f.subdirs) {
	                rs = vecstr_start(&dwp->subdirs,10,VECSTR_PNOHOLES) ;
	                dwp->f.subdirs = (rs >= 0) ;
	            }

	            if (rs >= 0) {
	                if (vecstr_find(&dwp->subdirs,dbp) == SR_NOTFOUND)
	                    rs = vecstr_add(&dwp->subdirs,dbp,nlen) ;
	            }

	        } else if (dw_findi(dwp,dbp,&iep) < 0) {

	            dwp->count_new += 1 ;
	            ientry_start(&ie,dwp,dbp,&sb) ;

	            rs = vecobj_add(&dwp->e,&ie) ;

	        } else if ((iep->state == DW_SNEW) &&
	            ((daytime - sb.st_mtime) > dwp->checkinterval)) {

	            iep->state = DW_SCHECK ;
	            dwp->count_new -= 1 ;
	            dwp->count_checkable += 1 ;
	            n += 1 ;

	        } /* end if */

	    } /* end while (reading directory entries) */

	    fsdir_close(&d) ;
	} /* end if (outer directory) */

/* do the subdirectories */

	if ((rs >= 0) && dwp->f.subdirs) {
	    const char	*dnp ;

	    for (i = 0 ; vecstr_get(&dwp->subdirs,i,&dnp) >= 0 ; i += 1) {
	        if (dnp != NULL) {
	            rs = dw_scansub(dwp,dnp,daytime) ;
	            n += rs ;
	        }
	        if (rs < 0) break ;
	    } /* end for */

	} /* end if (subdirectories) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (dw_scanfull) */


static int dw_scansub(DW *dwp,cchar *subdname,time_t daytime)
{
	struct ustat	sb ;
	IENTRY		ie, *iep ;
	fsdir		d ;
	fsdir_ent	ds ;
	int		rs ;
	int		rs1 ;
	int		nlen, dlen ;
	int		n = 0 ;
	char		dnamebuf[MAXPATHLEN + 1], *dbp ;
	char		*sdbp ;

#if	CF_DEBUGS
	debugprintf("dw_scansub: ent subdir=%s\n",subdir) ;
#endif

	if (dwp->dirname == NULL) return SR_FAULT ;

	if (dwp->dirname[0] == '\0') return SR_INVALID ;

/* put it together */

	dbp = dnamebuf ;
	dbp = strwcpy(dbp,dwp->dirname,-1) ;

	*dbp++ = '/' ;
	{
	    sdbp = dbp ;
	    if ((subdname != NULL) && (subdname[0] != '\0')) {
	        sdbp = strwcpy(sdbp,subdname,-1) ;
	        *sdbp++ = '/' ;
	    }
	    *sdbp = '\0' ;
	}

/* "do" the subdirectory */

#if	CF_DEBUGS
	debugprintf("dw_scansub: open dir=%s\n",dnamebuf) ;
#endif

	if ((rs = fsdir_open(&d,dnamebuf)) >= 0) {
	    const int	rsn = SR_NOENT ;

#if	CF_DEBUGS
	    debugprintf("dw_scansub: doing directory\n") ;
#endif

	    while ((nlen = fsdir_read(&d,&ds)) > 0) {
	        if (ds.name[0] == '.') continue ;

#if	CF_DEBUGS
	        debugprintf("dw_scansub: nlen=%d name=%s\n",
	            nlen,ds.name) ;
#endif

	        dlen = strwcpy(sdbp,ds.name,nlen) - dbp ;

#if	CF_DEBUGS
	        debugprintf("dw_scansub: got dlen=%d entry=%s\n",dlen,dnp) ;
#endif

	        if (u_stat(dnamebuf,&sb) < 0) continue ;

#if	CF_DEBUGS
	        debugprintf("dw_scansub: was statable\n") ;
#endif

	        if (S_ISDIR(sb.st_mode)) {

#if	CF_DEBUGS
	            debugprintf("dw_scansub: is subdirectory\n") ;
#endif

	            if (! dwp->f.subdirs) {
	                rs = vecstr_start(&dwp->subdirs,10,VECSTR_PNOHOLES) ;
	                dwp->f.subdirs = (rs >= 0) ;
	            }

	            if (rs >= 0) {
	                if ((rs = vecstr_find(&dwp->subdirs,dbp)) == rsn) {
	                    rs = vecstr_add(&dwp->subdirs,dbp,dlen) ;
			}
	            }

	        } else if (dw_findi(dwp,dbp,&iep) == SR_NOTFOUND) {

#if	CF_DEBUGS
	            debugprintf("dw_scansub: was not previously known\n") ;
#endif

	            dwp->count_new += 1 ;
	            ientry_start(&ie,dwp,dbp,&sb) ;

	            rs = vecobj_add(&dwp->e,&ie) ;

	        } else if (iep->state == DW_SNEW) {

#if	CF_DEBUGS
	            debugprintf("dw_scansub: is in NEW state already\n") ;
#endif

	            iep->itime = daytime ;
	            if ((daytime - sb.st_mtime) > dwp->checkinterval) {

#if	CF_DEBUGS
	                debugprintf("dw_scansub: is becoming checkable\n") ;
#endif

	                iep->state = DW_SCHECK ;
	                dwp->count_new -= 1 ;
	                dwp->count_checkable += 1 ;
	                n += 1 ;

	            } /* end if */

	        } else {
	            iep->itime = daytime ;
	        }

	    } /* end while (reading directory entries) */

#if	CF_DEBUGS
	    debugprintf("dw_scansub: closing directory\n") ;
#endif

	    rs1 = fsdir_close(&d) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subdirectory) */

#if	CF_DEBUGS
	debugprintf("dw_scansub: ret rs=%d n=%d\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (dw_scansub) */


#ifdef	COMMENT

static int dw_finddelete(dwp,name)
DW		*dwp ;
const char	name[] ;
{
	IENTRY		ie, *iep ;
	int		rs ;
	int		i ;

	ie.name = name ;
	rs = vecobj_search(&dwp->e,&ie,fnamecmp,&iep) ;
	i = rs ;
	if (rs >= 0)
	    rs = vecobj_del(&dwp->e,i) ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (dw_finddelete) */

#endif /* COMMENT */


static int dw_findi(DW *dwp,cchar *name,IENTRY **iepp)
{
	IENTRY		ie ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("dw_findi: ent name=%s\n",name) ;
#endif

	ie.name = (char *) name ;
	rs = vecobj_search(&dwp->e,&ie,fnamecmp,iepp) ;

#if	CF_DEBUGS
	debugprintf("dw_findi: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dw_findi) */


/* initialize an entry */
static int ientry_start(IENTRY *iep,DW *dwp,cchar *name,struct ustat *sbp)
{
	struct ustat	sb ;
	int		rs ;
	const char	*cp ;

#if	CF_DEBUGS
	debugprintf("ientry_start: ent name=%s\n",name) ;
#endif

	iep->state = DW_SNEW ;
	if ((rs = uc_mallocstrw(name,-1,&cp)) >= 0) {
	    iep->name = cp ;

/* do we need to get some status on the file? */

	    if (sbp == NULL) {
	        char	dnamebuf[MAXPATHLEN + 1] ;

	        if ((rs = mkpath2(dnamebuf,dwp->dirname,name)) >= 0) {
	            rs = u_stat(dnamebuf,&sb) ;
	            if (rs >= 0) sbp = &sb ;
	        }

	    } /* end if */

	    iep->itime = ((sbp != NULL) ? sbp->st_mtime : 0) ;

	} /* end if */

	return rs ;
}
/* end subroutine (ientry_start) */


/* free up an entry */
static int ientry_finish(IENTRY *iep,DW *dwp)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("ientry_finish: ent\n") ;
#endif

	if (iep->state == DW_SNEW) {
	    dwp->count_new -= 1 ;
	} else if (iep->state == DW_SCHECK) {
	    dwp->count_checkable -= 1 ;
	}

	if (iep->name != NULL) {
	    rs1 = uc_free(iep->name) ;
	    if (rs >= 0) rs = rs1 ;
	    iep->name = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("ientry_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ientry_finish) */


static int entry_load(DW_ENT *dep,IENTRY *iep)
{
	int		rs ;

	if (dep == NULL) return SR_FAULT ;

	dep->itime = iep->itime ;
	dep->mtime = iep->mtime ;
	dep->size = iep->size ;
	dep->state = iep->state ;
	rs = sncpy1(dep->name,MAXPATHLEN,iep->name) ;

	return rs ;
}
/* end subroutine (entry_load) */


static int fnamecmp(IENTRY **e1pp,IENTRY **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = strcmp((*e1pp)->name,(*e2pp)->name) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (fnamecmp) */


