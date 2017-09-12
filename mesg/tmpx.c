/* tmpx */

/* manage reading or writing of the [UW]TMP database */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_SAFE		0		/* safer */
#define	CF_DYNENTS	1		/* dynamic entries per read */


/* revision history:

	= 2000-05-14, David A­D­ Morano
        This subroutine module was adopted for use from some previous code that
        performed similar sorts of functions.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This code is used to access the UTMP (or WTMP) database. Those databases
        constitute the connect-time accounting information for the system.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"tmpx.h"


/* local defines */

#define	TMPX_INTOPEN	30	/* seconds */
#define	TMPX_INTCHECK	3	/* seconds */
#define	TMPX_NENTS	32	/* default n-entries (most systems "small") */
#define	TMPX_MAPSIZE	(2 * 1024 * 1024)


/* external subroutines */

extern uint	ufloor(uint,int) ;
extern uint	uceil(uint,int) ;

extern char	*strwcpy(char *,char *,int) ;


/* forward references */

int		tmpx_close(TMPX *) ;

static int	tmpx_filesize(TMPX *,time_t) ;
static int	tmpx_fileopen(TMPX *,time_t) ;
static int	tmpx_fileclose(TMPX *) ;
static int	tmpx_mapents(TMPX *,int,int,TMPX_ENT **) ;
static int	tmpx_mapper(TMPX *,int,uint,uint) ;

static int	isproctype(int) ;


/* local variables */

static const int	proctypes[] = {
	TMPX_TINITPROC,
	TMPX_TLOGINPROC,
	TMPX_TUSERPROC,
	TMPX_TDEADPROC,
	-1
} ;


/* exported subroutines */


int tmpx_open(TMPX *op,cchar dbfname[],int oflags)
{
	const time_t	dt = time(NULL) ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("tmpx_open: ent dbfname=%s\n",dbfname) ;
	debugprintf("tmpx_open: entry size=%d\n",
	    sizeof(TMPX_ENT)) ;
#endif

	if (op == NULL) return SR_FAULT ;

/* establish default UTMP-file as necessary */

	if ((dbfname == NULL) || (dbfname[0] == '\0') || (dbfname[0] == '-'))
	    dbfname = TMPX_DEFUTMP ;

	memset(op,0,sizeof(TMPX)) ;
	op->pagesize = getpagesize() ;
	op->oflags = oflags ;
	op->fd = -1 ;
	op->fname = NULL ;

/* make sure that any file access mode includes reading */

	{
	    int	amode = (oflags & O_ACCMODE) ;
	    switch (amode) {
	    case O_RDONLY:
	        break ;
	    case O_WRONLY:
	        amode = O_RDWR ;
	        oflags = ((oflags & (~ O_ACCMODE)) | amode) ;
		break ;
	    case O_RDWR:
	        break ;
	    default:
	        rs = SR_INVALID ;
	        break ;
	    } /* end switch */
	    op->f.writable = ((amode == O_WRONLY) || (amode == O_RDWR)) ;
	}

	if (rs >= 0) {
	    cchar	*cp ;
	    if ((rs = uc_mallocstrw(dbfname,-1,&cp)) >= 0) {
	        op->fname = cp ;
	        if ((rs = tmpx_fileopen(op,dt)) >= 0) {
		    struct ustat	sb ;
		    op->ti_check = dt ;
		    if ((rs = u_fstat(op->fd,&sb)) >= 0) {
	    	        op->fsize = (size_t) (sb.st_size & SIZE_MAX) ;
	    	        op->ti_mod = sb.st_mtime ;
		        op->magic = TMPX_MAGIC ;
		    } /* end if (stat) */
		    if (rs < 0) {
		        tmpx_fileclose(op) ;
		    }
	        } /* end if (tmpx-fileopen) */
	        if (rs < 0) {
		    uc_free(op->fname) ;
		    op->fname = NULL ;
	        }
	    } /* end if (memory-allocation) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("tmpx_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (tmpx_open) */


/* close this tmpx data structure */
int tmpx_close(TMPX *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TMPX_MAGIC) return SR_NOTOPEN ;

	if ((op->mapdata != NULL) && (op->mapsize > 0)) {
	    rs1 = u_munmap(op->mapdata,op->mapsize) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->fd >= 0) {
	    rs1 = tmpx_fileclose(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (tmpx_close) */


/* write an entry */
int tmpx_write(TMPX *op,int ei,TMPX_ENT *ep)
{
	int		rs = SR_OK ;
	int		am ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;

	if (op->magic != TMPX_MAGIC) return SR_NOTOPEN ;

/* proceed with operation */

	am = (op->oflags & O_ACCMODE) ;
	if ((am == SR_WRONLY) || (am == O_RDWR)) {
	    if (op->fd < 0) {
	        rs = tmpx_fileopen(op,0L) ;
	    }
	    if (rs >= 0) {
	        offset_t	poff ;
	        const int	esize = TMPX_ENTSIZE ;
	        poff = (offset_t) (ei * esize) ;
	        rs = u_pwrite(op->fd,ep,esize,poff) ;
	    }
	} else {
	    rs = SR_BADF ;
	}

	return rs ;
}
/* end subroutine (tmpx_write) */


/* check up on the object */
int tmpx_check(TMPX *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f_changed = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TMPX_MAGIC) return SR_NOTOPEN ;

	if ((op->ncursors == 0) && (op->fd >= 0)) {
	    if (dt == 0) dt = time(NULL) ;
	    if ((dt - op->ti_check) >= TMPX_INTCHECK) {
	        op->ti_check = dt ;
	        if ((dt - op->ti_open) < TMPX_INTOPEN) {
	            struct ustat	sb ;
	            if ((rs = u_fstat(op->fd,&sb)) >= 0) {
	                f_changed = (sb.st_size != op->fsize) ;
	                f_changed = f_changed || (sb.st_mtime > op->ti_mod) ;
	                op->fsize = sb.st_size ;
	                op->ti_mod = sb.st_mtime ;
	            } else {
	                u_close(op->fd) ;
	                op->fd = -1 ;
	            }
	        } /* end if */
	    } /* end if (check time-out) */
	} /* end if (possible) */

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (tmpx_check) */


/* initialize a cursor for enumeration */
int tmpx_curbegin(TMPX *op,TMPX_CUR *cp)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (cp == NULL) return SR_FAULT ;

	if (op->magic != TMPX_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	cp->i = -1 ;
	if (op->ncursors == 0) {
	    if (op->fd < 0) {
	        time_t	daytime = time(NULL) ;
	        rs = tmpx_filesize(op,daytime) ;
	    } /* end if (opened the file) */
	}

	if (rs >= 0)
	    op->ncursors += 1 ;

	return rs ;
}
/* end subroutine (tmpx_curbegin) */


/* free up a cursor */
int tmpx_curend(TMPX *op,TMPX_CUR *cp)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (cp == NULL) return SR_FAULT ;

	if (op->magic != TMPX_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (op->ncursors > 0)
	    op->ncursors -= 1 ;

	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (tmpx_curend) */


/* enumerate entries */
int tmpx_enum(TMPX *op,TMPX_CUR *curp,TMPX_ENT *ep)
{
	TMPX_ENT	*bp ;
	int		rs ;
	int		ei ;
	int		en = TMPX_NENTS ;
	int		n ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != TMPX_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (curp == NULL)
	    return SR_FAULT ;

#ifdef	COMMENT
	if (ep == NULL)
	    return SR_FAULT ;
#endif

	ei = (curp->i < 0) ? 0 : (curp->i + 1) ;

#if	CF_DEBUGS
	debugprintf("tmpx_enum: ei=%d\n",ei) ;
#endif

	rs = tmpx_mapents(op,ei,en,&bp) ;
	n = rs ;
	if ((rs >= 0) && (n > 0) && (bp != NULL)) {
	    if (ep != NULL) {
	        memcpy(ep,bp,TMPX_ENTSIZE) ;
	    }
	    curp->i = ei ;
	} else {
	    rs = SR_EOF ;
	}

#if	CF_DEBUGS
	debugprintf("tmpx_enum: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (tmpx_enum) */


/* fetch an entry by a PID (usually also a session ID) */
int tmpx_fetchpid(TMPX *op,TMPX_ENT *ep,pid_t pid)
{
	int		rs = SR_OK ;
	int		ei = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != TMPX_MAGIC) return SR_NOTOPEN ;
#endif

#if	CF_DEBUGS
	debugprintf("tmpx_fetchpid: ent pid=%d\n",pid) ;
#endif

	if (op->ncursors == 0) {
	    rs = tmpx_filesize(op,0L) ;
	}

	if (rs >= 0) {
	    TMPX_ENT	*up ;
	    const int	en = TMPX_NENTS ;
	    int		n, i ;
	    int		f = FALSE ;
	    while ((rs = tmpx_mapents(op,ei,en,&up)) > 0) {
	        n = rs ;
	        for (i = 0 ; i < n ; i += 1) {
	            f = (up->ut_type == TMPX_TUSERPROC) ;
	            f = f && (up->ut_pid == pid) ;
	            if (f)
	                break ;
	            ei += 1 ;
	            up += 1 ;
	        } /* end for */
	        if (f) break ;
	    } /* end while */
	    if ((rs >= 0) && f && (ep != NULL)) {
	        memcpy(ep,up,TMPX_ENTSIZE) ;
	    }
	    if ((rs == SR_OK) && (! f)) {
	        rs = SR_SEARCH ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("tmpx_fetchpid: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (tmpx_fetchpid) */


/* fetch all entries by user name */
int tmpx_fetchuser(TMPX *op,TMPX_CUR *curp,TMPX_ENT *ep,cchar *name)
{
	int		rs = SR_OK ;
	int		ei = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != TMPX_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (name == NULL) return SR_FAULT ;

	if (curp != NULL) {
	    ei = (curp->i < 0) ? 0 : (curp->i + 1) ;
	}

#if	CF_DEBUGS
	debugprintf("tmpx_fetchuser: name=%t ei=%d\n",
	    name,strnlen(name,TMPX_LUSER), ei) ;
#endif

	if (op->ncursors == 0) {
	    rs = tmpx_filesize(op,0L) ;
	}

	if (rs >= 0) {
	    TMPX_ENT	*up ;
	    const int	esize = TMPX_ENTSIZE ;
	    const int	en = TMPX_NENTS ;
	    int		n, i ;
	    int		f = FALSE ;

	    while ((rs = tmpx_mapents(op,ei,en,&up)) > 0) {
	        n = rs ;
	        for (i = 0 ; i < n ; i += 1) {
	            f = isproctype(up->ut_type) ;
	            f = f && (strncmp(name,up->ut_user,TMPX_LUSER) == 0) ;
	            if (f) break ;
	            ei += 1 ;
	            up += 1 ;
	        } /* end for */
	        if (f) break ;
	    } /* end while */

#if	CF_DEBUGS
	    debugprintf("tmpx_fetchuser: while-end rs=%d ei=%u f=%u\n",
		rs,ei,f) ;
#endif

	    if ((rs >= 0) && f && (ep != NULL))
	        memcpy(ep,up,esize) ;

	    if ((rs == SR_OK) && (! f))
	        rs = SR_NOTFOUND ;

	    if ((rs >= 0) && (curp != NULL))
	        curp->i = ei ;

	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("tmpx_fetchuser: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (tmpx_fetchuser) */


/* read an entry */
int tmpx_read(TMPX *op,int ei,TMPX_ENT *ep)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != TMPX_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAGE */

#if	CF_DEBUGS
	debugprintf("tmpx_read: ei=%u\n",ei) ;
#endif

	if (ei < 0) return SR_INVALID ;

	if (op->ncursors == 0) {
	    rs = tmpx_filesize(op,0L) ;
	}

	if (rs >= 0) {
	    TMPX_ENT	*up ;
	    const int	en = TMPX_NENTS ;
	    const int	esize = TMPX_ENTSIZE ;
	    if ((rs = tmpx_mapents(op,ei,en,&up)) > 0) {
		if (up != NULL) {
		    f = TRUE ;
	            if (ep != NULL)
	                memcpy(ep,up,esize) ;
		}
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("tmpx_read: ret rs=%d ei=%u\n",rs,ei) ;
	if (rs >= 0) {
	    debugprintf("tmpx_read: ut_user=%t\n",
	        ep->ut_user, strnlen(ep->ut_user,TMPX_LUSER)) ;
	}
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (tmpx_read) */


int tmpx_nusers(TMPX *op)
{
	int		rs = SR_OK ;
	int		en ;
	int		nusers = 0 ;

#if	CF_DYNENTS
	{
	    const int	esize = TMPX_ENTSIZE ;
	    en = ((op->fsize / esize) + 1) ;
	}
#else
	en = TMPX_NENTS ;
#endif /* CF_DYNENTS */

	if (op->ncursors == 0) {
	    rs = tmpx_filesize(op,0L) ;
	}

	if (rs >= 0) {
	    TMPX_ENT	*ep ;
	    int		n, i ;
	    int		ei = 0 ;
	    int		f ;
	    while ((rs = tmpx_mapents(op,ei,en,&ep)) > 0) {
	        n = rs ;
	        for (i = 0 ; i < n ; i += 1) {
	            f = (ep->ut_type == TMPX_TUSERPROC) ;
	            f = f && (ep->ut_user[0] != '.') ;
	            if (f) nusers += 1 ;
	            ep += 1 ;
	        } /* end for */
	        ei += i ;
	    } /* end while */
	} /* end if (ok) */

	return (rs >= 0) ? nusers : rs ;
}
/* end subroutine (tmpx_nusers) */


/* private subroutines */


static int tmpx_filesize(TMPX *op,time_t daytime)
{
	int		rs = SR_OK ;

	if (op->fd < 0) {
	    rs = tmpx_fileopen(op,daytime) ;
	}

	if (rs >= 0) {
	    struct ustat	sb ;
	    if ((rs = u_fstat(op->fd,&sb)) >= 0) {
	        op->ti_mod = sb.st_mtime ;
	        op->fsize = sb.st_size ;
	    }
	}

	return rs ;
}
/* end subroutine (tmpx_filesize) */


static int tmpx_fileopen(TMPX *op,time_t daytime)
{
	int		rs = SR_OK ;

	if (op->fd < 0) {
	    if ((rs = u_open(op->fname,op->oflags,0660)) >= 0) {
	        op->fd = rs ;
	        if ((rs = uc_closeonexec(op->fd,TRUE)) >= 0) {
	            if (daytime == 0) daytime = time(NULL) ;
	            op->ti_open = daytime ;
	        }
		if (rs < 0) {
		    u_close(op->fd) ;
		    op->fd = -1 ;
		}
	    }
	} /* end if (open) */

	return rs ;
}
/* end subroutine (tmpx_fileopen) */


static int tmpx_fileclose(TMPX *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->fd >= 0) {
	    rs1 = u_close(op->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fd = -1 ;
	} /* end if (was open) */

	return rs ;
}
/* end subroutine (tmpx_fileclose) */


static int tmpx_mapents(TMPX *op,int ei,int en,TMPX_ENT **rpp)
{
	const int	esize = TMPX_ENTSIZE ;
	int		rs = SR_OK ;
	int		n = 0 ;

#if	CF_DEBUGS
	debugprintf("tmpx_mapents: ei=%d en=%d\n",ei,en) ;
#endif

	if (en != 0) {

	if (en < 0) en = TMPX_NENTS ;

	if (op->mapdata != NULL) {

	    if ((ei >= op->mapei) && (ei < (op->mapei + op->mapen))) {

	        n = ((op->mapei + op->mapen) - ei) ;

#if	CF_DEBUGS
	        debugprintf("tmpx_mapents: mapei=%d mapen=%d\n",
	            op->mapei,op->mapen) ;
	        debugprintf("tmpx_mapents: n=%u\n",n) ;
#endif

	    } /* end if */

	} /* end if (old map was sufficient) */

	if ((rs >= 0) && (n == 0)) {
	    uint	eoff = (ei * esize) ;
	    uint	elen = (en * esize) ;
	    uint	eext = (eoff + elen) ;

	    if (eext > op->fsize) eext = op->fsize ;

	    n = (eext - eoff) / esize ;

#if	CF_DEBUGS
	    debugprintf("tmpx_mapents: new n=%u\n",n) ;
#endif

	    if (n > 0) {
	        uint	woff, wext, wsize ;

	        woff = ufloor(eoff,op->pagesize) ;
	        wext = uceil(eext,op->pagesize) ;
	        wsize = (wext - woff) ;

	        if (wsize > 0) {
	            int	f ;

#if	CF_DEBUGS
	            debugprintf("tmpx_mapents: mapoff=%u mapsize=%u\n",
	                op->mapoff,op->mapsize) ;
	            debugprintf("tmpx_mapents: woff=%u wsize=%u\n",
			woff,wsize) ;
#endif

	            f = (woff < op->mapoff) ;
	            f = f || (woff >= (op->mapoff + op->mapsize)) ;
	            if (! f) {
	                f = (((op->mapoff + op->mapsize) - eoff) < esize) ;
	            }

	            if (f) {
	                rs = tmpx_mapper(op,ei,woff,wsize) ;
	                n = rs ;
	            } /* end if */

	        } else {
	            n = 0 ;
		}

	    } /* end if (need a map) */

	} /* end if (new map needed) */

	if (rpp != NULL) {
	    caddr_t	ep = NULL ;
	    if ((rs >= 0) && (n > 0))
	        ep = (((ei * esize) - op->mapoff) + op->mapdata) ;
	    *rpp = (TMPX_ENT *) ep ;
	}

	} /* end if (non-equal-zero) */

#if	CF_DEBUGS
	debugprintf("tmpx_mapents: ret rs=%d n=%u\n",rs,n) ;
	if (rpp != NULL)
	    debugprintf("tmpx_mapents: ret ep{%p}\n",*rpp) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (tmpx_mapents) */


static int tmpx_mapper(TMPX *op,int ei,uint woff,uint wsize)
{
	offset_t	mo ;
	size_t		ms ;
	uint		eoff ;
	uint		eext ;
	uint		e ;
	const int	esize = TMPX_ENTSIZE ;
	int		rs = SR_OK ;
	int		fd ;
	int		mp ;
	int		mf ;
	int		n = 0 ;
	void		*md ;

#if	CF_DEBUGS
	debugprintf("tmpx_mapper: ei=%d woff=%u wsize=%u\n",
	    ei,woff,wsize) ;
#endif

	if (op->mapdata != NULL) {
	    u_munmap(op->mapdata,op->mapsize) ;
	    op->mapdata = NULL ;
	    op->mapsize = 0 ;
	    op->mapen = 0 ;
	}

	fd = op->fd ;
	mo = woff ;
	ms = wsize ;
	mp = PROT_READ ;
	mf = MAP_SHARED ;
	if ((rs = u_mmap(NULL,ms,mp,mf,fd,mo,&md)) >= 0) {
	    const int		madv = MADV_SEQUENTIAL ;
	    const caddr_t	ma = md ;
	    if ((rs = uc_madvise(ma,ms,madv)) >= 0) {

	        op->mapdata = (caddr_t) md ;
	        op->mapsize = ms ;
	        op->mapoff = (uint) mo ;

	        eoff = uceil(woff,esize) ;
	        e = MIN((woff + wsize),op->fsize) ;
	        eext = ufloor(e,esize) ;

	        op->mapei = eoff / esize ;
	        op->mapen = (eext - eoff) / esize ;

	        if (ei >= op->mapei) {
	            n = ((op->mapei + op->mapen) - ei) ;
		}

	    } /* end if (madvise) */
	} /* end if (mapped) */

#if	CF_DEBUGS
	debugprintf("tmpx_mapper: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (tmpx_mapper) */


static int isproctype(int type)
{
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; proctypes[i] >= 0 ; i += 1) {
	    f = (type == proctypes[i]) ;
	    if (f) break ;
	} /* end for */

	return f ;
}
/* end subroutine (isproctype) */


