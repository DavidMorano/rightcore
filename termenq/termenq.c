/* termenq */

/* manage reading or writing of the TERMENQ database */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_SAFE		0		/* safer */


/* revision history:

	= 2000-07-19, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This code is used to access the TERMENQ database. This database holds
        information that connects terminal lines with the terminal devices.


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

#include	"termenq.h"


/* local defines */

#define	TERMENQ_INTOPEN		30	/* seconds */
#define	TERMENQ_INTCHECK	3	/* seconds */
#define	TERMENQ_NENTS		32
#define	TERMENQ_MAPSIZE		(2 * 1024 * 1024)


/* external subroutines */

extern uint	ufloor(uint,int) ;
extern uint	uceil(uint,int) ;

extern char	*strwcpy(char *,char *,int) ;


/* forward references */

int		termenq_close(TERMENQ *) ;

static int	termenq_filesize(TERMENQ *,time_t) ;
static int	termenq_fileopen(TERMENQ *,time_t) ;
static int	termenq_fileclose(TERMENQ *) ;
static int	termenq_mapents(TERMENQ *,int,TERMENT **) ;
static int	termenq_mapper(TERMENQ *,int,uint,uint) ;

static int	isproctype(int) ;


/* local variables */

static const int	proctypes[] = {
	TERMENT_TLOGINPROC,
	TERMENT_TUSERPROC,
	TERMENT_TDEADPROC,
	-1
} ;


/* exported subroutines */


int termenq_open(TERMENQ *op,cchar *dbfname,int oflags)
{
	const time_t	dt = time(NULL) ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("termenq_open: ent dbfname=%s\n",dbfname) ;
	debugprintf("termenq_open: entry size=%d\n",
	    sizeof(TERMENT)) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (dbfname == NULL) return SR_FAULT ;

	if (dbfname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(TERMENQ)) ;
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
	        if ((rs = termenq_fileopen(op,dt)) >= 0) {
		    USTAT	sb ;
		    op->ti_check = dt ;
		    if ((rs = u_fstat(op->fd,&sb)) >= 0) {
	    	        op->fsize = (size_t) (sb.st_size & INT_MAX) ;
	    	        op->ti_mod = sb.st_mtime ;
		        op->magic = TERMENQ_MAGIC ;
		    } /* end if (stat) */
		    if (rs < 0) {
		        termenq_fileclose(op) ;
		    }
	        } /* end if (termenq-fileopen) */
	        if (rs < 0) {
		    uc_free(op->fname) ;
		    op->fname = NULL ;
	        }
	    } /* end if (memory-allocation) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("termenq_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (termenq_open) */


/* close this termenq data structure */
int termenq_close(TERMENQ *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMENQ_MAGIC) return SR_NOTOPEN ;

	if ((op->mapdata != NULL) && (op->mapsize > 0)) {
	    rs1 = u_munmap(op->mapdata,op->mapsize) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->fd >= 0) {
	    rs1 = termenq_fileclose(op) ;
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
/* end subroutine (termenq_close) */


/* write an entry */
int termenq_write(TERMENQ *op,int ei,TERMENT *ep)
{
	int		rs = SR_OK ;
	int		am ;

	if (op == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;

	if (op->magic != TERMENQ_MAGIC) return SR_NOTOPEN ;

/* proceed with operation */

	am = (op->oflags & O_ACCMODE) ;
	if ((am == SR_WRONLY) || (am == O_RDWR)) {
	    if (op->fd < 0) {
	        rs = termenq_fileopen(op,0L) ;
	    }
	    if (rs >= 0) {
	        offset_t	poff ;
	        const int	esize = sizeof(TERMENT) ;
	        poff = (offset_t) (ei * esize) ;
	        rs = u_pwrite(op->fd,ep,esize,poff) ;
	    }
	} else {
	    rs = SR_BADF ;
	}

	return rs ;
}
/* end subroutine (termenq_write) */


/* check up on the object */
int termenq_check(TERMENQ *op,time_t daytime)
{
	int		rs = SR_OK ;
	int		f_close = FALSE ;
	int		f_changed = FALSE ;
	int		f ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMENQ_MAGIC) return SR_NOTOPEN ;

	if (op->ncursors > 0)
	    goto ret0 ;

	if (op->fd < 0)
	    goto ret0 ;

	if (daytime == 0)
	    daytime = time(NULL) ;

	f = ((daytime - op->ti_check) < TERMENQ_INTCHECK) ;
	op->ti_check = daytime ;
	if (f)
	    goto ret0 ;

	f_close = ((daytime - op->ti_open) >= TERMENQ_INTOPEN) ;

	if (! f_close) {
	    struct ustat	sb ;
	    if ((rs = u_fstat(op->fd,&sb)) >= 0) {
	        f_changed = (sb.st_size != op->fsize) ;
	        f_changed = f_changed || (sb.st_mtime > op->ti_mod) ;
	        op->fsize = sb.st_size ;
	        op->ti_mod = sb.st_mtime ;
	    } else
	        f_close = TRUE ;
	} /* end if */

	if (f_close) {
	    u_close(op->fd) ;
	    op->fd = -1 ;
	}

ret0:
	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (termenq_check) */


/* initialize a cursor for enumeration */
int termenq_curbegin(TERMENQ *op,TERMENQ_CUR *cp)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (cp == NULL) return SR_FAULT ;

	if (op->magic != TERMENQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	cp->i = -1 ;
	if (op->ncursors == 0) {
	    if (op->fd < 0) {
	        time_t	daytime = time(NULL) ;
	        rs = termenq_filesize(op,daytime) ;
	    } /* end if (opened the file) */
	}

	if (rs >= 0)
	    op->ncursors += 1 ;

	return rs ;
}
/* end subroutine (termenq_curbegin) */


/* free up a cursor */
int termenq_curend(TERMENQ *op,TERMENQ_CUR *cp)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (cp == NULL) return SR_FAULT ;

	if (op->magic != TERMENQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (op->ncursors > 0)
	    op->ncursors -= 1 ;

	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (termenq_curend) */


/* enumerate entries */
int termenq_enum(TERMENQ *op,TERMENQ_CUR *curp,TERMENT *ep)
{
	TERMENT		*bp ;
	int		rs ;
	int		ei ;
	int		n ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != TERMENQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (curp == NULL)
	    return SR_FAULT ;

#ifdef	COMMENT
	if (ep == NULL)
	    return SR_FAULT ;
#endif

	ei = (curp->i < 0) ? 0 : (curp->i + 1) ;

#if	CF_DEBUGS
	debugprintf("termenq_enum: ei=%d\n",ei) ;
#endif

	rs = termenq_mapents(op,ei,&bp) ;
	n = rs ;
	if ((rs >= 0) && (n > 0) && (bp != NULL)) {
	    if (ep != NULL) {
	        memcpy(ep,bp,sizeof(TERMENT)) ;
	    }
	    curp->i = ei ;
	} else {
	    rs = SR_EOF ;
	}

#if	CF_DEBUGS
	debugprintf("termenq_enum: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (termenq_enum) */


/* fetch an entry by a PID (usually also a session ID) */
int termenq_fetchsid(TERMENQ *op,TERMENT *ep,pid_t pid)
{
	int		rs = SR_OK ;
	int		ei = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMENQ_MAGIC) return SR_NOTOPEN ;
#endif

#if	CF_DEBUGS
	debugprintf("termenq_fetchsid: ent pid=%d\n",pid) ;
#endif

	if (op->ncursors == 0) {
	    rs = termenq_filesize(op,0L) ;
	}

	if (rs >= 0) {
	    TERMENT	*up ;
	    int		n, i ;
	    int		f = FALSE ;
	    while ((rs = termenq_mapents(op,ei,&up)) > 0) {
	        n = rs ;
	        for (i = 0 ; i < n ; i += 1) {
	            f = (up->type == TERMENT_TUSERPROC) ;
	            f = f && (up->sid == pid) ;
	            if (f)
	                break ;
	            ei += 1 ;
	            up += 1 ;
	        } /* end for */
	        if (f) break ;
	    } /* end while */
	    if ((rs >= 0) && f && (ep != NULL)) {
	        memcpy(ep,up,sizeof(TERMENT)) ;
	    }
	    if ((rs == SR_OK) && (! f)) {
	        rs = SR_SEARCH ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("termenq_fetchsid: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (termenq_fetchsid) */


/* fetch all entries by user name */
int termenq_fetchline(TERMENQ *op,TERMENQ_CUR *curp,TERMENT *ep,cchar *name)
{
	int		rs = SR_OK ;
	int		ei = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMENQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (name == NULL) return SR_FAULT ;

	if (curp != NULL) {
	    ei = (curp->i < 0) ? 0 : (curp->i + 1) ;
	}

#if	CF_DEBUGS
	debugprintf("termenq_fetchline: name=%t ei=%d\n",
	    name,strnlen(name,TERMENT_LLINE), ei) ;
#endif

	if (op->ncursors == 0) {
	    rs = termenq_filesize(op,0L) ;
	}

	if (rs >= 0) {
	    TERMENT	*up ;
	    const int	esize = sizeof(TERMENT) ;
	    int		n, i ;
	    int		f = FALSE ;

	    while ((rs = termenq_mapents(op,ei,&up)) > 0) {
	        n = rs ;
	        for (i = 0 ; i < n ; i += 1) {
	            f = isproctype(up->type) ;
	            f = f && (strncmp(name,up->line,TERMENT_LLINE) == 0) ;
	            if (f) break ;
	            ei += 1 ;
	            up += 1 ;
	        } /* end for */
	        if (f) break ;
	    } /* end while */

#if	CF_DEBUGS
	    debugprintf("termenq_fetchline: while-end rs=%d ei=%u f=%u\n",
		rs,ei,f) ;
#endif

	    if ((rs >= 0) && f && (ep != NULL)) {
	        memcpy(ep,up,esize) ;
	    }

	    if ((rs == SR_OK) && (! f)) {
	        rs = SR_NOTFOUND ;
	    }

	    if ((rs >= 0) && (curp != NULL)) {
	        curp->i = ei ;
	    }

	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("termenq_fetchline: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (termenq_fetchline) */


/* read an entry */
int termenq_read(TERMENQ *op,int ei,TERMENT *ep)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMENQ_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAGE */

#if	CF_DEBUGS
	debugprintf("termenq_read: ei=%u\n",ei) ;
#endif

	if (ei < 0) return SR_INVALID ;

	if (op->ncursors == 0) {
	    rs = termenq_filesize(op,0L) ;
	}

	if (rs >= 0) {
	    TERMENT	*up ;
	    const int	esize = sizeof(TERMENT) ;
	    if ((rs = termenq_mapents(op,ei,&up)) > 0) {
		if (up != NULL) {
		    f = TRUE ;
	            if (ep != NULL)
	                memcpy(ep,up,esize) ;
		}
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("termenq_read: ret rs=%d ei=%u\n",rs,ei) ;
	if (rs >= 0) {
	    debugprintf("termenq_read: line=%t\n",
	        ep->ut_user,strnlen(ep->line,TERMENT_LUSER)) ;
	}
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (termenq_read) */


int termenq_nactive(TERMENQ *op)
{
	int		rs = SR_OK ;
	int		n = 0 ;

	if (op->ncursors == 0) {
	    rs = termenq_filesize(op,0L) ;
	}

	if (rs >= 0) {
	    TERMENT	*ep ;
	    int		n, i ;
	    int		ei = 0 ;
	    int		f ;
	    while ((rs = termenq_mapents(op,ei,&ep)) > 0) {
	        n = rs ;
	        for (i = 0 ; i < n ; i += 1) {
	            f = (ep->type == TERMENT_TUSERPROC) ;
	            f = f && (ep->line[0] != '.') ;
	            if (f) n += 1 ;
	            ep += 1 ;
	        } /* end for */
	        ei += i ;
	    } /* end while */
	} /* end if (ok) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (termenq_nactive) */


/* private subroutines */


static int termenq_filesize(TERMENQ *op,time_t daytime)
{
	int		rs = SR_OK ;

	if (op->fd < 0) {
	    rs = termenq_fileopen(op,daytime) ;
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
/* end subroutine (termenq_filesize) */


static int termenq_fileopen(TERMENQ *op,time_t daytime)
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
/* end subroutine (termenq_fileopen) */


static int termenq_fileclose(TERMENQ *op)
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
/* end subroutine (termenq_fileclose) */


static int termenq_mapents(TERMENQ *op,int ei,TERMENT **rpp)
{
	const int	esize = sizeof(TERMENT) ;
	int		en ;
	int		rs = SR_OK ;
	int		n = 0 ;

#if	CF_DEBUGS
	debugprintf("termenq_mapents: ei=%d\n",ei) ;
#endif

	en = MIN(((op->fsize / esize) + 1),TERMENQ_NENTS) ;

	if (en != 0) {

	if (op->mapdata != NULL) {

	    if ((ei >= op->mapei) && (ei < (op->mapei + op->mapen))) {

	        n = ((op->mapei + op->mapen) - ei) ;

#if	CF_DEBUGS
	        debugprintf("termenq_mapents: mapei=%d mapen=%d\n",
	            op->mapei,op->mapen) ;
	        debugprintf("termenq_mapents: n=%u\n",n) ;
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
	    debugprintf("termenq_mapents: new n=%u\n",n) ;
#endif

	    if (n > 0) {
	        uint	woff, wext, wsize ;

	        woff = ufloor(eoff,op->pagesize) ;
	        wext = uceil(eext,op->pagesize) ;
	        wsize = (wext - woff) ;

	        if (wsize > 0) {
	            int	f = (woff < op->mapoff) ;

#if	CF_DEBUGS
	            debugprintf("termenq_mapents: mapoff=%u mapsize=%u\n",
	                op->mapoff,op->mapsize) ;
	            debugprintf("termenq_mapents: woff=%u wsize=%u\n",
			woff,wsize) ;
#endif

	            f = f || (woff >= (op->mapoff + op->mapsize)) ;
	            if (! f) {
	                f = (((op->mapoff + op->mapsize) - eoff) < esize) ;
	            }

	            if (f) {
	                rs = termenq_mapper(op,ei,woff,wsize) ;
	                n = rs ;
	            } /* end if */

	        } else {
	            n = 0 ;
		}

	    } /* end if (need a map) */

	} /* end if (new map needed) */

	if (rpp != NULL) {
	    caddr_t	ep = NULL ;
	    if ((rs >= 0) && (n > 0)) {
	        ep = (((ei * esize) - op->mapoff) + op->mapdata) ;
	    }
	    *rpp = (TERMENT *) ep ;
	}

	} /* end if (non-equal-zero) */

#if	CF_DEBUGS
	debugprintf("termenq_mapents: ret rs=%d n=%u\n",rs,n) ;
	if (rpp != NULL)
	    debugprintf("termenq_mapents: ret ep{%p}\n",*rpp) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (termenq_mapents) */


static int termenq_mapper(TERMENQ *op,int ei,uint woff,uint wsize)
{
	offset_t	mo ;
	size_t		ms ;
	uint		eoff ;
	uint		eext ;
	uint		e ;
	const int	esize = sizeof(TERMENT) ;
	int		rs = SR_OK ;
	int		fd ;
	int		mp ;
	int		mf ;
	int		n = 0 ;
	void		*md ;

#if	CF_DEBUGS
	debugprintf("termenq_mapper: ei=%d woff=%u wsize=%u\n",
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
	debugprintf("termenq_mapper: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (termenq_mapper) */


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


