/* msuclient (Memory-Status-Update client) */

/* manage the SYSMISC shared-memory region */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-12-18, David A­D­ Morano
        This little subroutine was put together so that for those situations
        where only the number of CPUs is desired (not often the case, but
        sometimes) we do not have to go through the process (hassle) of using
        the KINFO object directly (oh like that is a huge problem).

	= 2010-12-09, David A­D­ Morano
        I enhanced this subroutine to get the number of CPUs without using the
        KINFO object. That KINFO object (as it is and has been) is NOT
        reentrant. This is no fault of my own (since I wrote that KINFO code
        also) but rather from Sun-Solaris. The KINFO object uses the underlying
        Solaris KSTAT facility -- which is not reentrant (and therefore not
        thread-safe). I needed a thread-safe way of getting the number of CPUs
        so I had to add some sort of mechanism to do that. We have (basically)
        cheap and cheaper ways to do it. I tried regular 'cheap' and got tired,
        so I switched to 'cheaper'. The 'cheaper' version is the shared-memory
        thing I added below. The regular 'cheap' one was to query the MSINFO or
        MSU facility. The latter is left unfinished due to time constraints.
        Also, it (naturally) took longer than desired to even to the 'cheaper'
        solution.

*/

/* Copyright © 1998,2010 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine returns the number of CPUs available from the current
        node. We do note that the number of CPUs can change dynamically as some
        may be added or removed during the course of live machine operation. We
        allow the number of CPUs returned to the caller to be zero (0) even
        though it is not clear how this might happen. This sort of assumes that
        the caller understands (believes) that at least one CPU is available at
        any time -- otherwise how would we be able to execute in the first
        place!

	Notes:

	= Load-averages

        Although load-averages are available when retrieving SYSMISC
        (miscellaneous system) information from the kernel, we don't bother with
        it at all since the general introduction of the 'getloadavg(3c)'
        subroutine in the world. If that subroutine was not available,
        load-averages would have to be treated as being as difficult to retrieve
        as the number of CPUs is.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<estrings.h>
#include	<endian.h>
#include	<endianstr.h>
#include	<nulstr.h>
#include	<filebuf.h>
#include	<expcook.h>
#include	<getax.h>
#include	<getxusername.h>
#include	<ugetpw.h>
#include	<ascii.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"msuclient.h"
#include	"sysmiscfh.h"


/* local defines */

#define	MSUCLIENT_SHMDBNAME	"sm"

#define	LOADINFO		struct loadinfo
#define	LOADINFO_FL		struct loadinfo_flags

#ifndef	SHMNAMELEN
#define	SHMNAMELEN	14		/* shared-memory name length */
#endif

#ifndef	SHMPREFIXLEN
#define	SHMPREFIXLEN	8
#endif

#ifndef	SHMPOSTFIXLEN
#define	SHMPOSTFIXLEN	4
#endif

#define	LIBCNAME	"lib"
#define	SHMPERMS	0666
#define	CNBUFLEN	14		/* cookie-name buffer length */

#ifndef	VARLIBPATH
#define	VARLIBPATH	"LD_LIBRARY_PATH"
#endif

#ifndef	VARPR
#define	VARPR		"LOCAL"
#endif

#ifndef	VARNCPU
#define	VARNCPU		"NCPU"
#endif

#define	MODBNAME	"sysmiscs"	/* base-name (part of filename) */

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#define	HDRBUFLEN	(20 + sizeof(SYSMISCFH))

#define	TO_UPDATE	60
#define	TO_SHMWAIT	10


/* external subroutines */

extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;
extern int	filebuf_writefill(FILEBUF *,char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct loadinfo_flags {
	uint		username:1 ;
	uint		rn:1 ;
} ;

struct loadinfo {
	const char	*pr ;
	const char	*dbname ;
	const char	*username ;
	const char	*rn ;
	const char	*prbuf ;
	LOADINFO_FL	f ;
	EXPCOOK		cooks ;
	mode_t		om ;
} ;


/* forward references */

static int	msuclient_setbegin(MSUCLIENT *,const char *) ;
static int	msuclient_setend(MSUCLIENT *) ;

static int	msuclient_shmcreate(MSUCLIENT *,cchar *,mode_t) ;
static int	msuclient_shmcreater(MSUCLIENT *,LOADINFO *,int,cchar *) ;
static int	msuclient_shmdestroy(MSUCLIENT *) ;
static int	msuclient_shmwr(MSUCLIENT *,int,mode_t) ;

static int	msuclient_shmmapbegin(MSUCLIENT *,int) ;
static int	msuclient_shmmapend(MSUCLIENT *) ;

static int	msuclient_shmproc(MSUCLIENT *) ;
static int	msuclient_shmverify(MSUCLIENT *,SYSMISCFH *) ;

static int	msuclient_shmopenwait(MSUCLIENT *,cchar *,mode_t) ;

static int	loadinfo_start(LOADINFO *,cchar *,cchar *,mode_t) ;
static int	loadinfo_finish(LOADINFO *) ;
static int	loadinfo_expand(LOADINFO *,char *,const char *) ;
static int	loadinfo_cookcheck(LOADINFO *,const char *) ;
static int	loadinfo_rn(LOADINFO *) ;
static int	loadinfo_username(LOADINFO *) ;
static int	loadinfo_chown(LOADINFO *,int,int) ;

static int	getcookname(const char *,const char **) ;


/* local variables */

static cchar	*shmnames[] = {
	"/sys$%n",
	"/%{RN}$%n",
	"/%U¥%n",
	NULL
} ;

/* match 'shmnames' above */
enum shmnames {
	shmname_sys,
	shmname_pn,
	shmname_user,
	shmname_overlast
} ;

static cchar	*cookies[] = {
	"n",
	"RN",
	"U",
	NULL
} ;

/* match 'cookies' above */
enum cookies {
	cookie_n,
	cookie_rn,
	cookie_u,
	cookie_overlast
} ;


/* exported variables */

MSUCLIENT_OBJ	msuclient = {
	"msuclient",
	sizeof(MSUCLIENT)
} ;


/* exported subroutines */


int msuclient_open(MSUCLIENT *op,cchar *pr)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("msuclient_open: pr=%s\n",pr) ;
#endif

	memset(op,0,sizeof(MSUCLIENT)) ;

	if ((rs = msuclient_setbegin(op,pr)) >= 0) {
	    const mode_t	om = SHMPERMS ;
	    cchar		*dbname = MSUCLIENT_SHMDBNAME ;
	    if ((rs = msuclient_shmcreate(op,dbname,om)) >= 0) {
		op->magic = MSUCLIENT_MAGIC ;
	    }
	    if (rs < 0)
		msuclient_setend(op) ;
	} /* end if (msuclient_setbegin) */

#if	CF_DEBUGS
	debugprintf("msuclient_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msuclient_open) */


int msuclient_close(MSUCLIENT *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSUCLIENT_MAGIC) return SR_NOTOPEN ;

	rs1 = msuclient_shmdestroy(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = msuclient_setend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("msuclient_close: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msuclient_close) */


int msuclient_get(MSUCLIENT *op,time_t dt,int to,MSUCLIENT_DATA *dp)
{
	uint		*shmtable ;
	time_t		ti_update ;
	int		rs = SR_OK ;
	int		i ;
	int		n = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MSUCLIENT_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("msuclient_get: entered \n") ;
#endif

	if (to <= 0) to = 1 ;

	if (dt > op->dt) op->dt = dt ;

	shmtable = (uint *) (op->mapdata + SYSMISCFH_IDLEN) ;
	ti_update = shmtable[sysmiscfv_utime] ;

	op->ti_lastcheck = dt ;
	if ((dt - ti_update) >= to) {
	    rs = msuclient_shmupdate(op) ;
	}

	n = shmtable[sysmiscfv_ncpu] ;
	if (dp != NULL) {
	    if (rs >= 0) {
	        dp->utime = shmtable[sysmiscfv_utime] ;
	        dp->btime = shmtable[sysmiscfv_btime] ;
	        dp->ncpu = shmtable[sysmiscfv_ncpu] ;
	        dp->nproc = shmtable[sysmiscfv_nproc] ;
		for (i = 0 ; i < 3 ; i += 1) {
	            dp->la[i] = shmtable[sysmiscfv_la + i] ;
		}
	    } else {
	        memset(dp,0,sizeof(MSUCLIENT_DATA)) ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("msuclient_get: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (msuclient_get) */


/* local subroutines */


static int msuclient_setbegin(MSUCLIENT *op,cchar *pr)
{

	op->pr = pr ;
	return SR_OK ;
}
/* end subroutine (msuclient_setbegin) */


static int msuclient_setend(MSUCLIENT *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->shmname != NULL) {
	    rs1 = uc_free(op->shmname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->shmname = NULL ;
	}

	op->pr = NULL ;
	return rs ;
}
/* end subroutine (msuclient_setend) */


static int msuclient_shmcreate(MSUCLIENT *op,cchar *dbname,mode_t om)
{
	LOADINFO	li ;
	int		rs ;
	int		rs1 ;

	if ((rs = loadinfo_start(&li,op->pr,dbname,om)) >= 0) {
	    int		i ;
	    char	shmname[SHMNAMELEN + 1] ;

#if	CF_DEBUGS
	debugprintf("msuclient_shmcreate: loadinfo_start() rs=%d\n",rs) ;
#endif

	for (i = 0 ; shmnames[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	debugprintf("msuclient_shmcreate: expand%i=>%s<\n",
		i,shmnames[i]) ;
#endif

	    rs1 = loadinfo_expand(&li,shmname,shmnames[i]) ;

#if	CF_DEBUGS
	debugprintf("msuclient_shmcreate: loadinfo_expand() rs=%d\n",rs) ;
#endif

	    if (rs1 >= 0) {

	        rs = msuclient_shmcreater(op,&li,i,shmname) ;

#if	CF_DEBUGS
	debugprintf("msuclient_shmcreate: _shmcreater() rs=%d\n",rs) ;
#endif

	        if ((rs >= 0) || (! isNotPresent(rs))) break ;
	    } else if (rs1 != SR_OVERFLOW)
	        rs = rs1 ;

	    if (rs < 0) break ;
	} /* end for */

	loadinfo_finish(&li) ;
	} /* end if (loadinfo) */

#if	CF_DEBUGS
	debugprintf("msuclient_shmcreate: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msuclient_shmcreate) */


static int msuclient_shmcreater(MSUCLIENT *op,LOADINFO *lip,int shmi,
		cchar *shmname)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		fd = -1 ;
	int		oflags ;
	int		om = lip->om ;
	int		f_needwr = FALSE ;
	int		f_needupdate = FALSE ;
	int		f_needchmod = FALSE ;
	int		f_created = FALSE ;
	cchar		*cp ;

#if	CF_DEBUGS
	debugprintf("msuclient_shmcreate: shmname=%s\n",shmname) ;
#endif

	op->shmsize = 0 ;
	op->shmtable = NULL ;

	if (op->shmname != NULL) {
	    uc_free(op->shmname) ;
	    op->shmname = NULL ;
	}

	rs = uc_mallocstrw(shmname,-1,&cp) ;
	if (rs < 0) goto ret0 ;

	op->shmname = cp ;
	oflags = O_RDWR ;
	rs = uc_openshm(shmname,oflags,om) ;
	fd = rs ;

#if	CF_DEBUGS
	debugprintf("msuclient_shmcreate: 1 uc_openshm() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    rs = u_fstat(fd,&sb) ;
	    op->shmsize = sb.st_size ;
	    op->ti_shm = sb.st_mtime ;
	    f_needwr = (op->shmsize < SYSMISCFH_IDLEN) ;
	}

	if (rs == SR_NOENT) {

	    f_created = TRUE ;
	    f_needwr = TRUE ;
	    oflags = (O_RDWR | O_CREAT | O_EXCL) ;
	    rs = uc_openshm(shmname,oflags,(om & 0444)) ;
	    fd = rs ;

#if	CF_DEBUGS
	    debugprintf("msuclient_shmcreate: 2 uc_openshm() rs=%d\n",
	        rs) ;
#endif

	} /* end if */

	if ((rs >= 0) && f_needwr) {

	    if (rs >= 0) {
	        if (op->dt == 0)
	            op->dt = time(NULL) ;

	        f_needupdate = TRUE ;
	        f_needchmod = TRUE ;
	        rs = msuclient_shmwr(op,fd,om) ;
	    }

	} /* end if */

	if ((rs == SR_ACCESS) || (rs == SR_EXIST)) {
	    op->shmsize = 0 ;
	    rs = msuclient_shmopenwait(op,shmname,om) ;
	    fd = rs ;
	}

	if (rs < 0)
	    goto ret1 ;

/* map it */

	if ((rs >= 0) && (op->shmsize == 0)) {
	    rs = u_fstat(fd,&sb) ;
	    op->shmsize = sb.st_size ;
	    f_needupdate = f_needupdate || (op->shmsize == 0) ;
	}

	if (rs >= 0) {
	    rs = msuclient_shmmapbegin(op,fd) ;
	    if (rs >= 0) {
	        rs = msuclient_shmproc(op) ;
	        f_needupdate = f_needupdate || (rs > 0) ;

#if	CF_DEBUGS
	        debugprintf("msuclient_shmcreate: _shmproc() rs=%d\n",rs) ;
#endif

	        if (rs == SR_STALE) {
	            rs = SR_OK ;
	            f_needupdate = TRUE ;
	        }

	    }
	} /* end if */

	if ((rs >= 0) && f_needupdate) {
	    rs = msuclient_shmupdate(op) ;

#if	CF_DEBUGS
	    debugprintf("msuclient_shmcreate: _shmupdate() rs=%d\n",rs) ;
#endif

	} /* end if */

	if ((rs >= 0) && f_needchmod) {
	    u_fchmod(fd,om) ;
	}

	if ((rs >= 0) && f_created) {
	    rs = loadinfo_chown(lip,fd,shmi) ;
	}

ret2:
	if (rs >= 0) {
	    op->f.shm = TRUE ;
	} else {
	    msuclient_shmmapend(op) ;
	    op->shmtable = NULL ;
	}

/* close it (it stays mapped) */
ret1:
	if (fd >= 0)
	    u_close(fd) ;

ret0:

#if	CF_DEBUGS
	debugprintf("msuclient_shmcreate: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msuclient_shmcreater) */


static int msuclient_shmdestroy(MSUCLIENT *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->mapdata != NULL) {
	    rs1 = msuclient_shmmapend(op) ;
	    if (rs >= 0) rs = rs1 ;
	    op->shmtable = NULL ;
	}

	if (op->shmname != NULL) {
	    rs1 = uc_free(op->shmname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->shmname = NULL ;
	}

	return rs ;
}
/* end subroutine (msuclient_shmdestroy) */


static int msuclient_shmwr(MSUCLIENT *op,int fd,mode_t om)
{
	SYSMISCFH	hdr ;
	FILEBUF		babyfile ;
	uint		fileoff = 0 ;
	int		rs = SR_OK ;
	int		size ;
	int		bl ;
	char		hdrbuf[HDRBUFLEN + 1] ;

	op->shmsize = 0 ;
	if (op->dt == 0)
	    op->dt = time(NULL) ;

	if (op->pagesize == 0)
	    op->pagesize = getpagesize() ;

/* prepare the file-header */

	memset(&hdr,0,sizeof(SYSMISCFH)) ;
	hdr.vetu[0] = SYSMISCFH_VERSION ;
	hdr.vetu[1] = ENDIAN ;
	hdr.vetu[2] = 0 ;
	hdr.vetu[3] = 0 ;
	hdr.utime = (uint) op->dt ;

/* create the file-header */

	size = (op->pagesize * 4) ;
	if ((rs = filebuf_start(&babyfile,fd,0,size,0)) >= 0) {

	if ((rs = sysmiscfh(&hdr,0,hdrbuf,HDRBUFLEN)) >= 0) {
	    bl = rs ;
	    rs = filebuf_writefill(&babyfile,hdrbuf,bl) ;
	    fileoff += rs ;
	}

	filebuf_finish(&babyfile) ;
	} /* end if (filebuf) */

	if (rs >= 0) {
	    hdr.shmsize = fileoff ;
	    if ((rs = u_seek(fd,0L,SEEK_SET)) >= 0) {
	        if ((rs = sysmiscfh(&hdr,0,hdrbuf,HDRBUFLEN)) >= 0) {
	            bl = rs ;
	            if ((rs = u_write(fd,hdrbuf,bl)) >= 0) {
	    		op->shmsize = fileoff ;
	    		rs = u_fchmod(fd,om) ;
		    }
	        }
	    }
	} /* end if */

#ifdef	COMMENT /* not needed for shared memory on single system */
	if (rs >= 0)
	    rs = uc_fdatasync(fd) ;
#endif

	return (rs >= 0) ? fileoff : rs ;
}
/* end subroutine (msuclient_shmwr) */


static int msuclient_shmopenwait(MSUCLIENT *op,cchar *shmname,mode_t om)
{
	int		rs = SR_OK ;
	int		oflags = O_RDWR ;
	int		to = TO_SHMWAIT ;
	int		fd = -1 ;

	if (op == NULL) return SR_FAULT ;

	while (to-- > 0) {

	    rs = uc_openshm(shmname,oflags,om) ;
	    fd = rs ;
	    if (rs >= 0) break ;

	    if (rs != SR_ACCESS) break ;
	} /* end while */

	if ((rs < 0) && (to == 0)) {
	    rs = SR_TIMEDOUT ;
	}

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (msuclient_shmopenwait) */


static int msuclient_shmmapbegin(MSUCLIENT *op,int fd)
{
	size_t		msize ;
	int		rs = SR_OK ;
	int		mprot ;
	int		mflags ;
	char		*mp ;

	if (fd < 0) return SR_INVALID ;

	if (op->dt == 0)
	    op->dt = time(NULL) ;

	msize = op->shmsize ;
	mprot = PROT_READ | PROT_WRITE ;
	mflags = MAP_SHARED ;
	if ((rs = u_mmap(NULL,msize,mprot,mflags, fd,0L,&mp)) >= 0) {
	    op->mapdata = mp ;
	    op->mapsize = msize ;
	    op->ti_map = op->dt ;
	}

	return rs ;
}
/* end subroutine (msuclient_shmmapbegin) */


static int msuclient_shmmapend(MSUCLIENT *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->mapdata != NULL) {
	    rs1 = u_munmap(op->mapdata,op->mapsize) ;
	    if (rs >= 0) rs = rs1 ;
	    op->mapdata = NULL ;
	    op->mapsize = 0 ;
	    op->ti_map = 0 ;
	    if (op->f.shm && (op->shmtable != NULL)) {
	        op->f.shm = FALSE ;
	        op->shmtable = NULL ;
	    }
	}

	return rs ;
}
/* end subroutine (msuclient_shmmapend) */


static int msuclient_shmproc(MSUCLIENT *op)
{
	SYSMISCFH	hdr ;
	int		rs ;
	int		f_stale = FALSE ;

#if	CF_DEBUGS
	debugprintf("msuclient_shmproc: entered\n") ;
#endif

	if ((rs = sysmiscfh(&hdr,1,op->mapdata,op->mapsize)) >= 0) {
	    uint	dtime = (uint) op->dt ;
	    uint	utime ;
	    uint	intstale ;
	    uint	*shmtable ;
	    int		shmsize ;

#if	CF_DEBUGS
	debugprintf("msuclient_shmproc: sysmiscfh() rs=%d\n",rs) ;
#endif

	if ((rs = msuclient_shmverify(op,&hdr)) >= 0) {
	    f_stale = (rs > 0) ;

#if	CF_DEBUGS
	debugprintf("msuclient_shmproc: _shmverify() rs=%d\n",rs) ;
#endif

	shmtable = (uint *) (op->mapdata + SYSMISCFH_IDLEN) ;
	op->shmtable = shmtable ;

	utime = shmtable[sysmiscfv_utime] ;
	shmsize = shmtable[sysmiscfv_shmsize] ;
	intstale = shmtable[sysmiscfv_intstale] ;
	if (shmsize == op->shmsize) {
	if ((! f_stale) && (intstale > 0)) {
	    f_stale = ((dtime - utime) >= intstale) ;
	}
	if (! f_stale) {
	    f_stale = ((dtime - utime) >= TO_UPDATE) ;
	}
	} else {
	    rs = SR_BADFMT ;
	}

	} /* end if (msuclient_shmverify) */

	} /* end if (sysmiscfh) */

	return (rs >= 0) ? f_stale : rs ;
}
/* end subroutine (msuclient_shmproc) */


static int msuclient_shmverify(MSUCLIENT *op,SYSMISCFH *hp)
{
	int		rs = SR_OK ;
	int		f_stale = (hp->shmsize != op->shmsize) ;

#if	CF_DEBUGS
	debugprintf("msuclient_shmverify: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_stale : rs ;
}
/* end subroutine (msuclient_shmverify) */


static int msuclient_shmupdate(MSUCLIENT *op)
{
	pid_t		pid ;
	int		rs = SR_OK ;
	int		cs = 0 ;

	if (op->dt == 0) op->dt = time(NULL) ;

	if ((rs = uc_fork()) == 0) {
	    int	ex ;
	    int	i ;

	    for (i = 0 ; i < 3 ; i += 1)
	        u_close(i) ;

	    rs = msuclient_shmchild(op) ;

	    ex = (rs >= 0) ? EX_OK : EX_DATAERR ;
	    u_exit(ex) ;
	} else if (rs > 0) {
	    pid  = rs ;
	    rs = 0 ;
	    while (rs == 0) {
	        rs = u_waitpid(pid,&cs,0) ;
	        if (rs == SR_INTR) rs = 0 ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (msuclient_shmupdate) */


static int loadinfo_start(LOADINFO *lip,cchar *pr,cchar *dbname,mode_t om)
{
	int		rs ;

	memset(lip,0,sizeof(LOADINFO)) ;
	lip->pr = pr ;
	lip->dbname = dbname ;
	lip->om = om ;

	if ((rs = expcook_start(&lip->cooks)) >= 0) {
	    cchar	*cn = cookies[cookie_n] ;
	    rs = expcook_add(&lip->cooks,cn,dbname,-1) ;
	    if (rs < 0)
		expcook_finish(&lip->cooks) ;
	}

	return rs ;
}
/* end subroutine (loadinfo_start) */


static int loadinfo_finish(LOADINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip->username != NULL) {
	    rs1 = uc_free(lip->username) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->username = NULL ;
	}

	if (lip->rn != NULL) {
	    rs1 = uc_free(lip->rn) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->rn = NULL ;
	}

	if (lip->prbuf != NULL) {
	    rs1 = uc_free(lip->prbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->prbuf = NULL ;
	}

	rs1 = expcook_finish(&lip->cooks) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (loadinfo_finish) */


static int loadinfo_expand(LOADINFO *lip,char *shmname,cchar *template)
{
	int		rs ;

	if ((rs = loadinfo_cookcheck(lip,template)) >= 0) {
	    rs = expcook_exp(&lip->cooks,0,shmname,SHMNAMELEN,template,-1) ;
	}

	return rs ;
}
/* end subroutine (loadinfo_expand) */


static int loadinfo_cookcheck(LOADINFO *lip,cchar *template)
{
	EXPCOOK		*ecp = &lip->cooks ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		cni ;
	int		cnl ;
	int		vl ;
	const char	*sp ;
	const char	*tp ;
	const char	*cnp ;
	const char	*vp ;
	char		cnbuf[CNBUFLEN + 1] ;

	sp = template ;
	while ((tp = strchr(sp,'%')) != NULL) {

	    cnl = getcookname((tp+1),&cnp) ;

	    cni = matstr(cookies,cnp,cnl) ;
	    if (cni < 0) {
	        rs = SR_NOANODE ;
	        break ;
	    }

	    rs1 = expcook_findkey(ecp,cnp,cnl,NULL) ;

	    if (rs1 == SR_NOTFOUND) {

	        vp = NULL ;
	        vl = -1 ;
	        switch (cni) {

	        case cookie_n:
	            vp = lip->dbname ;
		    vl = strnlen(vp,SHMPOSTFIXLEN) ;
	            break ;

	        case cookie_rn:
	            rs = loadinfo_rn(lip) ;
		    vl = rs ;
	            if (rs >= 0)
	                vp = lip->rn ;
	            break ;

	        case cookie_u:
	            rs = loadinfo_username(lip) ;
		    vl = rs ;
	            if (rs >= 0)
	                vp = lip->username ;
	            break ;

	        } /* end switch */

	        if (rs >= 0)
	            rs = snwcpy(cnbuf,CNBUFLEN,cnp,cnl) ; /* make NUL termed */

	        if ((rs >= 0) && (vp != NULL))
	            rs = expcook_add(ecp,cnbuf,vp,vl) ;

	    } /* end if */

	    sp = (tp + 1) ;

	    if (rs < 0) break ;
	} /* end while */

	return rs ;
}
/* end subroutine (loadinfo_cookcheck) */


static int loadinfo_rn(lip)
LOADINFO	*lip ;
{
	int		rs = SR_OK ;
	int		bl ;
	const char	*cp ;
	const char	*bp ;

	if (lip->rn != NULL) {
	    bl = strlen(lip->rn) ;
	    goto ret0 ;
	}

	bl = sfbasename(lip->pr,-1,&bp) ;

	if (bl >= 0) {
	    if (bl > SHMPREFIXLEN) bl = SHMPREFIXLEN ;
	    rs = uc_mallocstrw(bp,bl,&cp) ;
	    if (rs >= 0)
	        lip->rn = cp ;
	} else
	    rs = SR_ISDIR ;

ret0:
	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (loadinfo_rn) */


static int loadinfo_username(lip)
LOADINFO	*lip ;
{
	int		rs = SR_OK ;
	int		unl = 0 ;
	const char	*cp ;
	char		userbuf[USERNAMELEN + 1] ;

	if (lip->username == NULL) {
	    if ((rs = getusername(userbuf,USERNAMELEN,-1)) >= 0) {
	        unl = rs ;
	        if (unl > SHMPREFIXLEN) unl = SHMPREFIXLEN ;
	        rs = uc_mallocstrw(userbuf,unl,&cp) ;
	        if (rs >= 0)
	            lip->username = cp ;
	    }
	} else {
	    unl = strlen(lip->username) ;
	}

	return (rs >= 0) ? unl : rs ;
}
/* end subroutine (loadinfo_username) */


/* ARGSUSED */
static int loadinfo_chown(lip,fd,shmi)
LOADINFO	*lip ;
int		fd ;
int		shmi ;
{
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs = SR_OK ;
	const char	*sysuser = "sys" ;
	char		*pwbuf ;

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    switch (shmi) {
	    case shmname_sys:
	        if ((rs = getpw_name(&pw,pwbuf,pwlen,sysuser)) >= 0) {
		    u_fchown(fd,pw.pw_uid,-1) ;
	        } else if (isNotPresent(rs)) {
		    rs = SR_OK ;
		}
	        break ;
	    } /* end switch */
	    uc_free(pwbuf) ;
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (loadinfo_chown) */


static int getcookname(sp,rpp)
const char	*sp ;
const char	**rpp ;
{
	int		cnl = 1 ;
	const char	*cnp ;
	const char	*tp ;

	if (sp[0] == CH_LBRACK) {
	    cnp = (sp + 1) ;
	    if ((tp = strchr(cnp,CH_RBRACK)) != NULL) {
	        cnl = (tp - cnp) ;
	    } else
	        cnl = strlen(cnp) ;
	} else
	    cnp = sp ;

	if (rpp != NULL)
	    *rpp = cnp ;

	return cnl ;
}
/* end subroutine (getcookname) */


