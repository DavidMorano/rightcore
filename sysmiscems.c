/* sysmiscems */

/* manage the SYSMISC shared-memory region */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-12-18, David A­D­ Morano

	This little subroutine was put together so that for those
	situations where only the number of CPUs is desired (not often the
	case, but sometimes) we do not have to go through the process
	(hassle) of using the KINFO object directly (oh like that is a
	huge problem).

	= 2010-12-9, David A­D­ Morano

	I enhanced this subroutine to get the number of CPUs without
	using the KINFO object.  That KINFO object (as it is and has
	been) is NOT reentrant.  This is no fault of my own (since I
	wrote that KINFO code also) but rather from Sun-Solaris.  The
	KINFO object uses the underlying Solaris KSTAT facility --
	which is not reentrant (and therefore not thread-safe).  I
	needed a thread-safe way of getting the number of CPUs so I
	had to add some sort of mechanism to do that.  We have (basically)
	cheap and cheaper ways to do it.  I tried regular 'cheap' and
	got tired, so I switched to 'cheaper'.  The 'cheaper' version
	is the shared-memory thing I added below.  The regular 'cheap'
	one was to query the MSINFO or MSU facility.  The latter is
	left unfinished due to time constraints.  Also, it (naturally)
	took longer than desired to even do the 'cheaper' solution.


*/

/* Copyright © 1998,2010 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine returns the number of CPUs available from the
	current node.  We do note that the number of CPUs can change
	dynamically as some may be added or removed during the course of
	live machine operation.  We allow the number of CPUs returned to
	the caller to be zero (0) even though it is not clear how this
	might happen.  This sort of assumes that the caller understands
	(believes) that at least one CPU is available at any time --
	otherwise how would we be able to execute in the first place!


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
#include	<nulstr.h>
#include	<filebuf.h>
#include	<expcook.h>
#include	<getsysmisc.h>
#include	<ascii.h>
#include	<getax.h>
#include	<getxusername.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"sysmiscems.h"
#include	"sysmiscfh.h"


/* local defines */

#define	SYSMISCEMS_MAGIC	0x58261227
#define	SYSMISCEMS_SHMDBNAME	"sm"

#define	LOADINFO		struct loadinfo

#ifndef	ENDIANSTR
#ifdef	ENDIAN
#if	(ENDIAN == 0)
#define	ENDIANSTR	"0"
#else
#define	ENDIANSTR	"1"
#endif
#else
#define	ENDIANSTR	"1"
#endif
#endif

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

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,const char *,const char *,cchar *,cchar *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,cchar *,cchar *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	getnodedomain(char *,char *) ;
extern int	getuid_name(cchar *,int) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
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
	const char	*prbuf ;
	const char	*username ;
	const char	*rn ;
	struct loadinfo_flags	f ;
	EXPCOOK	cooks ;
	int		operms ;
} ;


/* forward references */

static int	sysmiscems_setbegin(SYSMISCEMS *,const char *) ;
static int	sysmiscems_setend(SYSMISCEMS *) ;
static int	sysmiscems_pr(SYSMISCEMS *) ;

static int	sysmiscems_shmcreate(SYSMISCEMS *,const char *,int) ;
static int	sysmiscems_shmcreater(SYSMISCEMS *,struct loadinfo *,
			int,const char *) ;
static int	sysmiscems_shmdestroy(SYSMISCEMS *) ;
static int	sysmiscems_shmwr(SYSMISCEMS *,int,int) ;

static int	sysmiscems_mapcreate(SYSMISCEMS *,int) ;
static int	sysmiscems_mapdestroy(SYSMISCEMS *) ;

static int	sysmiscems_shmproc(SYSMISCEMS *) ;
static int	sysmiscems_shmverify(SYSMISCEMS *,SYSMISCFH *) ;

static int	sysmiscems_shmupdate(SYSMISCEMS *) ;
static int	sysmiscems_shmchild(SYSMISCEMS *) ;
static int	sysmiscems_shmopenwait(SYSMISCEMS *,const char *,int) ;

static int	loadinfo_start(LOADINFO *,const char *,const char *,int) ;
static int	loadinfo_finish(LOADINFO *) ;
static int	loadinfo_expand(LOADINFO *,char *,const char *) ;
static int	loadinfo_cookcheck(LOADINFO *,const char *) ;
static int	loadinfo_rn(LOADINFO *) ;
static int	loadinfo_username(LOADINFO *) ;
static int	loadinfo_chown(LOADINFO *,int,int) ;

static int	filebuf_writefill(FILEBUF *,const char *,int) ;

#ifdef	COMMENT
static int	filebuf_writezero(FILEBUF *,int) ;
#endif

static int	getcookname(const char *,const char **) ;

static int	istermrs(int) ;


/* local variables */

static const char	*shmnames[] = {
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

static const char	*cookies[] = {
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

static const int	termrs[] = {
	SR_FAULT,
	SR_INVALID,
	SR_NOMEM,
	SR_NOANODE,
	SR_BADFMT,
	SR_NOSPC,
	SR_NOSR,
	SR_NOBUFS,
	SR_BADF,
	SR_OVERFLOW,
	SR_RANGE,
	0
} ;

static const char	zerobuf[4] = {
	0, 0, 0, 0 
} ;


/* exported variables */

SYSMISCEMS_OBJ	sysmiscems = {
	"sysmiscems",
	sizeof(SYSMISCEMS)
} ;


/* exported subroutines */


int sysmiscems_open(op,pr)
SYSMISCEMS	*op ;
const char	*pr ;
{
	int	rs ;
	int	operms = SHMPERMS ;

	const char	*dbname = SYSMISCEMS_SHMDBNAME ;


	if (op == NULL)
	    return SR_FAULT ;

	if (pr == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("sysmiscems_open: pr=%s\n",pr) ;
#endif

	memset(op,0,sizeof(SYSMISCEMS)) ;

	rs = sysmiscems_setbegin(op,pr) ;

#if	CF_DEBUGS
	debugprintf("sysmiscems_open: _init() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad0 ;

	rs = sysmiscems_shmcreate(op,dbname,operms) ;

#if	CF_DEBUGS
	debugprintf("sysmiscems_open: _shmcreate() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad1 ;

	op->magic = SYSMISCEMS_MAGIC ;

ret0:

#if	CF_DEBUGS
	debugprintf("sysmiscems_open: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad1:
	sysmiscems_setend(op) ;

bad0:
	goto ret0 ;
}
/* end subroutine (sysmiscems_open) */


int sysmiscems_close(op)
SYSMISCEMS	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SYSMISCEMS_MAGIC)
	    return SR_NOTOPEN ;

	rs1 = sysmiscems_shmdestroy(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = sysmiscems_setend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("sysmiscems_close: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sysmiscems_close) */


int sysmiscems_get(op,daytime,to,dp)
SYSMISCEMS	*op ;
time_t		daytime ;
int		to ;
SYSMISCEMS_DATA	*dp ;
{
	uint	*shmtable ;

	time_t	ti_update ;

	int	rs = SR_OK ;
	int	i ;
	int	n = 0 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SYSMISCEMS_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("sysmiscems_get: ent \n") ;
#endif

	if (to <= 0) to = 1 ;

	if (daytime > op->daytime) op->daytime = daytime ;

	shmtable = (uint *) (op->mapdata + SYSMISCFH_IDLEN) ;
	ti_update = shmtable[sysmiscfv_utime] ;

	op->ti_lastcheck = daytime ;
	if ((daytime - ti_update) >= to)
	    rs = sysmiscems_shmupdate(op) ;

	n = shmtable[sysmiscfv_ncpu] ;
	if (dp != NULL) {
	    if (rs >= 0) {
	        dp->intstale = shmtable[sysmiscfv_intstale] ;
	        dp->utime = shmtable[sysmiscfv_utime] ;
	        dp->btime = shmtable[sysmiscfv_btime] ;
	        dp->ncpu = shmtable[sysmiscfv_ncpu] ;
	        dp->nproc = shmtable[sysmiscfv_nproc] ;
		for (i = 0 ; i < 3 ; i += 1) {
	            dp->la[i] = shmtable[sysmiscfv_la + i] ;
		}
	    } else
	        memset(dp,0,sizeof(SYSMISCEMS_DATA)) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("sysmiscems_get: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (sysmiscems_get) */


/* local subroutines */


static int sysmiscems_setbegin(op,pr)
SYSMISCEMS	*op ;
const char	*pr ;
{


	op->pr = pr ;

	return SR_OK ;
}
/* end subroutine (sysmiscems_setbegin) */


static int sysmiscems_setend(op)
SYSMISCEMS	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op->shmname != NULL) {
	    rs1 = uc_free(op->shmname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->shmname = NULL ;
	}

	if (op->prbuf != NULL) {
	    rs1 = uc_free(op->prbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    op->prbuf = NULL ;
	}

	op->pr = NULL ;
	return rs ;
}
/* end subroutine (sysmiscems_setend) */


static int sysmiscems_pr(op)
SYSMISCEMS	*op ;
{
	int	rs = SR_OK ;
	int	prl = 0 ;


	if (op->pr == NULL) {
	    char	dn[MAXHOSTNAMELEN + 1] ;
	    char	prbuf[MAXPATHLEN + 1] ;
	    if ((rs = getnodedomain(NULL,dn)) >= 0) {
	        if ((rs = mkpr(prbuf,MAXPATHLEN,VARPR,dn)) >= 0) {
		    const char	*cp ;
		    prl = rs ;
		    if ((rs = uc_mallocstrw(prbuf,prl,&cp)) >= 0) {
		        op->prbuf = cp ;
	    	        op->pr = cp ;
		    }
	        }
	    }
	} else
	    prl = strlen(op->pr) ;

#if	CF_DEBUGS
	debugprintf("ncpu: rs=%d prbuf=%s\n",rs,op->prbuf) ;
#endif

	return rs ;
}
/* end subroutine (sysmiscems_pr) */


static int sysmiscems_shmcreate(op,dbname,operms)
SYSMISCEMS	*op ;
const char	dbname[] ;
int		operms ;
{
	struct loadinfo	li ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;

	char	shmname[SHMNAMELEN + 1] ;


	if ((rs = loadinfo_start(&li,op->pr,dbname,operms)) >= 0) {

#if	CF_DEBUGS
	debugprintf("sysmiscems_shmcreate: loadinfo_start() rs=%d\n",rs) ;
#endif

	for (i = 0 ; shmnames[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	debugprintf("sysmiscems_shmcreate: expand%i=>%s<\n",
		i,shmnames[i]) ;
#endif

	    rs1 = loadinfo_expand(&li,shmname,shmnames[i]) ;

#if	CF_DEBUGS
	debugprintf("sysmiscems_shmcreate: loadinfo_expand() rs=%d\n",rs) ;
#endif

	    if (rs1 >= 0) {

	        rs = sysmiscems_shmcreater(op,&li,i,shmname) ;

#if	CF_DEBUGS
	debugprintf("sysmiscems_shmcreate: _shmcreater() rs=%d\n",rs) ;
#endif

	        if ((rs >= 0) || istermrs(rs))
	            break ;

	    } else if (rs1 != SR_OVERFLOW)
	        rs = rs1 ;

	    if (rs < 0) break ;
	} /* end for */

	loadinfo_finish(&li) ;
	} /* end if (loadinfo) */

#if	CF_DEBUGS
	debugprintf("sysmiscems_shmcreate: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sysmiscems_shmcreate) */


static int sysmiscems_shmcreater(op,lip,shmi,shmname)
SYSMISCEMS	*op ;
struct loadinfo	*lip ;
int		shmi ;
const char	shmname[] ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	fd = -1 ;
	int	oflags ;
	int	operms = lip->operms ;
	int	f_needwr = FALSE ;
	int	f_needupdate = FALSE ;
	int	f_needchmod = FALSE ;
	int	f_created = FALSE ;


#if	CF_DEBUGS
	debugprintf("sysmiscems_shmcreate: shmname=%s\n",shmname) ;
#endif

	op->shmsize = 0 ;
	op->shmtable = NULL ;

	if (op->shmname != NULL) {
	    uc_free(op->shmname) ;
	    op->shmname = NULL ;
	}

	rs = uc_mallocstrw(shmname,-1,&op->shmname) ;
	if (rs < 0)
	    goto ret0 ;

	oflags = O_RDWR ;
	rs = uc_openshm(shmname,oflags,operms) ;
	fd = rs ;

#if	CF_DEBUGS
	debugprintf("sysmiscems_shmcreate: 1 uc_openshm() rs=%d\n",rs) ;
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
	    rs = uc_openshm(shmname,oflags,(operms & 0444)) ;
	    fd = rs ;

#if	CF_DEBUGS
	    debugprintf("sysmiscems_shmcreate: 2 uc_openshm() rs=%d\n",
	        rs) ;
#endif

	} /* end if */

	if ((rs >= 0) && f_needwr) {

	    if (rs >= 0) {
	        if (op->daytime == 0)
	            op->daytime = time(NULL) ;

	        f_needupdate = TRUE ;
	        f_needchmod = TRUE ;
	        rs = sysmiscems_shmwr(op,fd,operms) ;
	    }

	} /* end if */

	if ((rs == SR_ACCESS) || (rs == SR_EXIST)) {
	    op->shmsize = 0 ;
	    rs = sysmiscems_shmopenwait(op,shmname,operms) ;
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
	    rs = sysmiscems_mapcreate(op,fd) ;
	    if (rs >= 0) {
	        rs = sysmiscems_shmproc(op) ;
	        f_needupdate = f_needupdate || (rs > 0) ;

#if	CF_DEBUGS
	        debugprintf("sysmiscems_shmcreate: _shmproc() rs=%d\n",rs) ;
#endif

	        if (rs == SR_STALE) {
	            rs = SR_OK ;
	            f_needupdate = TRUE ;
	        }

	    }
	} /* end if */

	if ((rs >= 0) && f_needupdate) {
	    rs = sysmiscems_shmupdate(op) ;

#if	CF_DEBUGS
	    debugprintf("sysmiscems_shmcreate: _shmupdate() rs=%d\n",rs) ;
#endif

	} /* end if */

	if ((rs >= 0) && f_needchmod)
	    u_fchmod(fd,operms) ;

	if ((rs >= 0) && f_created)
	    rs = loadinfo_chown(lip,fd,shmi) ;

ret2:
	if (rs >= 0) {
	    op->f.shm = TRUE ;
	} else {
	    sysmiscems_mapdestroy(op) ;
	    op->shmtable = NULL ;
	}

/* close it (it stays mapped) */
ret1:
	if (fd >= 0)
	    u_close(fd) ;

ret0:

#if	CF_DEBUGS
	debugprintf("sysmiscems_shmcreate: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sysmiscems_shmcreater) */


static int sysmiscems_shmdestroy(op)
SYSMISCEMS	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op->mapdata != NULL) {
	    rs1 = sysmiscems_mapdestroy(op) ;
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
/* end subroutine (sysmiscems_shmdestroy) */


static int sysmiscems_shmwr(op,fd,operms)
SYSMISCEMS	*op ;
int		fd ;
int		operms ;
{
	SYSMISCFH	hdr ;

	FILEBUF	babyfile ;

	uint	fileoff = 0 ;

	int	rs = SR_OK ;
	int	size ;
	int	bl ;

	char	hdrbuf[HDRBUFLEN + 1] ;


	op->shmsize = 0 ;
	if (op->daytime == 0)
	    op->daytime = time(NULL) ;

	if (op->pagesize == 0)
	    op->pagesize = getpagesize() ;

	size = (op->pagesize * 4) ;
	rs = filebuf_start(&babyfile,fd,0,size,0) ;
	if (rs < 0)
	    goto ret1 ;

/* prepare the file-header */

	memset(&hdr,0,sizeof(SYSMISCFH)) ;

	hdr.vetu[0] = SYSMISCFH_VERSION ;
	hdr.vetu[1] = ENDIAN ;
	hdr.vetu[2] = 0 ;
	hdr.vetu[3] = 0 ;
	hdr.utime = (uint) op->daytime ;

/* create the file-header */

	rs = sysmiscfh(&hdr,0,hdrbuf,HDRBUFLEN) ;
	bl = rs ;
	if (rs < 0)
	    goto ret2 ;

/* write file-header */

	if (rs >= 0) {
	    rs = filebuf_writefill(&babyfile,hdrbuf,bl) ;
	    fileoff += rs ;
	}

/* write out the header -- again! */
ret2:
	filebuf_finish(&babyfile) ;

	if (rs >= 0) {

	    hdr.shmsize = fileoff ;
	    u_seek(fd,0L,SEEK_SET) ;

	    rs = sysmiscfh(&hdr,0,hdrbuf,HDRBUFLEN) ;
	    bl = rs ;
	    if (rs >= 0)
	        rs = u_write(fd,hdrbuf,bl) ;

	} /* end if */

/* set file permissions */

	if (rs >= 0) {
	    op->shmsize = fileoff ;
	    rs = u_fchmod(fd,operms) ;
	}

#ifdef	COMMENT /* not needed for shared memory on single system */
	if (rs >= 0)
	    rs = uc_fdatasync(fd) ;
#endif

/* we're out of here */
ret1:
ret0:
	return (rs >= 0) ? fileoff : rs ;
}
/* end subroutine (sysmiscems_shmwr) */


static int sysmiscems_shmopenwait(op,shmname,operms)
SYSMISCEMS	*op ;
const char	shmname[] ;
int		operms ;
{
	int		rs = SR_OK ;
	int		oflags = O_RDWR ;
	int		to = TO_SHMWAIT ;
	int		fd = -1 ;

	while (to-- > 0) {

	    rs = uc_openshm(shmname,oflags,operms) ;
	    fd = rs ;
	    if (rs >= 0)
	        break ;

	    if (rs != SR_ACCESS) break ;
	} /* end while */

	if ((rs < 0) && (to == 0)) {
	    rs = SR_TIMEDOUT ;
	}

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (sysmiscems_shmopenwait) */


static int sysmiscems_mapcreate(op,fd)
SYSMISCEMS	*op ;
int		fd ;
{
	size_t	msize ;

	int	rs = SR_OK ;
	int	mprot ;
	int	mflags ;

	void	*mdata ;


	if (fd < 0)
	    return SR_INVALID ;

	if (op->daytime == 0)
	    op->daytime = time(NULL) ;

	msize = op->shmsize ;
	mprot = PROT_READ | PROT_WRITE ;
	mflags = MAP_SHARED ;
	if ((rs = u_mmap(NULL,msize,mprot,mflags,fd,0L,&mdata)) >= 0) {
	    op->mapdata = mdata ;
	    op->mapsize = msize ;
	    op->ti_map = op->daytime ;
	}

	return rs ;
}
/* end subroutine (sysmiscems_mapcreate) */


static int sysmiscems_mapdestroy(op)
SYSMISCEMS	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


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
/* end subroutine (sysmiscems_mapdestroy) */


static int sysmiscems_shmproc(op)
SYSMISCEMS	*op ;
{
	SYSMISCFH	hdr ;

	uint	dtime = (uint) op->daytime ;
	uint	utime ;
	uint	intstale ;
	uint	*shmtable ;

	int	rs ;
	int	shmsize ;
	int	f_stale = FALSE ;


#if	CF_DEBUGS
	debugprintf("sysmiscems_shmproc: ent\n") ;
#endif

	rs = sysmiscfh(&hdr,1,op->mapdata,op->mapsize) ;

#if	CF_DEBUGS
	debugprintf("sysmiscems_shmproc: sysmiscfh() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	rs = sysmiscems_shmverify(op,&hdr) ;
	f_stale = (rs > 0) ;

#if	CF_DEBUGS
	debugprintf("sysmiscems_shmproc: _shmverify() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	shmtable = (uint *) (op->mapdata + SYSMISCFH_IDLEN) ;
	op->shmtable = shmtable ;

	utime = shmtable[sysmiscfv_utime] ;
	shmsize = shmtable[sysmiscfv_shmsize] ;
	intstale = shmtable[sysmiscfv_intstale] ;
	if (shmsize != op->shmsize) {
	    rs = SR_BADFMT ;
	    goto ret0 ;
	}

	if ((! f_stale) && (intstale > 0))
	    f_stale = ((dtime - utime) >= intstale) ;

	if (! f_stale)
	    f_stale = ((dtime - utime) >= TO_UPDATE) ;

ret0:
	return (rs >= 0) ? f_stale : rs ;
}
/* end subroutine (sysmiscems_shmproc) */


static int sysmiscems_shmverify(op,hp)
SYSMISCEMS	*op ;
SYSMISCFH	*hp ;
{
	int	rs = SR_OK ;
	int	f_stale = FALSE ;


	f_stale = (hp->shmsize != op->shmsize) ;

ret0:

#if	CF_DEBUGS
	debugprintf("sysmiscems_shmverify: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_stale : rs ;
}
/* end subroutine (sysmiscems_shmverify) */


static int sysmiscems_shmupdate(op)
SYSMISCEMS	*op ;
{
	pid_t	pid ;

	int	rs = SR_OK ;
	int	cs = 0 ;


	if (op->daytime == 0)
	    op->daytime = time(NULL) ;

	rs = uc_fork() ;
	pid = rs ;
	if (rs < 0)
	    goto ret0 ;

	if (pid == 0) {
	    int	ex ;
	    int	i ;

	    for (i = 0 ; i < 3 ; i += 1)
	        u_close(i) ;

	    rs = sysmiscems_shmchild(op) ;

	    ex = (rs >= 0) ? EX_OK : EX_DATAERR ;
	    uc_exit(ex) ;

	} /* end if */

	rs = 0 ;
	while (rs == 0) {
	    rs = u_waitpid(pid,&cs,0) ;
	    if (rs == SR_INTR) rs = 0 ;
	}

ret0:
	return rs ;
}
/* end subroutine (sysmiscems_shmupdate) */


static int sysmiscems_shmchild(op)
SYSMISCEMS	*op ;
{
	GETSYSMISC	kd ;

	uint	*shmtable = op->shmtable ;

	int	rs ;
	int	i ;


	if (op->shmtable == NULL) {
	    rs = SR_FAULT ;
	    goto ret0 ;
	}

	rs = getsysmisc(&kd,op->daytime) ;
	if (rs < 0)
	    goto ret0 ;

	shmtable[sysmiscfv_shmsize] = op->shmsize ;
	shmtable[sysmiscfv_utime] = op->daytime ; /* this should be last */
	shmtable[sysmiscfv_btime] = kd.btime ;
	shmtable[sysmiscfv_ncpu] = kd.ncpu ;
	shmtable[sysmiscfv_nproc] = kd.nproc ;
	for (i = 0 ; i < 3 ; i += 1)
	    shmtable[sysmiscfv_la + i] = kd.la[i] ;

ret0:
	return rs ;
}
/* end subroutine (sysmiscems_shmchild) */


static int loadinfo_start(lip,pr,dbname,operms)
struct loadinfo	*lip ;
const char	pr[] ;
const char	dbname[] ;
int		operms ;
{
	int	rs = SR_OK ;

	const char	*cn ;


	memset(lip,0,sizeof(struct loadinfo)) ;

	lip->pr = pr ;
	lip->dbname = dbname ;
	lip->operms = operms ;
	rs = expcook_start(&lip->cooks) ;
	if (rs < 0)
	    goto bad0 ;

	cn = cookies[cookie_n] ;
	rs = expcook_add(&lip->cooks,cn,dbname,-1) ;
	if (rs < 0)
	    goto bad1 ;

ret0:
	return rs ;

bad1:
	expcook_finish(&lip->cooks) ;

bad0:
	goto ret0 ;
}
/* end subroutine (loadinfo_start) */


static int loadinfo_finish(lip)
struct loadinfo	*lip ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (lip->username != NULL) {
	    uc_free(lip->username) ;
	    lip->username = NULL ;
	}

	if (lip->rn != NULL) {
	    uc_free(lip->rn) ;
	    lip->rn = NULL ;
	}

	if (lip->prbuf != NULL) {
	    uc_free(lip->prbuf) ;
	    lip->prbuf = NULL ;
	}

	rs1 = expcook_finish(&lip->cooks) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (loadinfo_finish) */


static int loadinfo_expand(lip,shmname,template)
struct loadinfo	*lip ;
char		shmname[] ;
const char	template[] ;
{
	int	rs ;


	rs = loadinfo_cookcheck(lip,template) ;

	if (rs >= 0)
	    rs = expcook_exp(&lip->cooks,0,
	        shmname,SHMNAMELEN,template,-1) ;

	return rs ;
}
/* end subroutine (loadinfo_expand) */


static int loadinfo_cookcheck(lip,template)
struct loadinfo	*lip ;
const char	template[] ;
{
	EXPCOOK	*ecp = &lip->cooks ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	cni ;
	int	cnl ;
	int	vl ;

	const char	*sp ;
	const char	*tp ;
	const char	*cnp ;
	const char	*vp ;

	char	cnbuf[CNBUFLEN + 1] ;


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

	    if (rs < 0)
	        break ;

	    sp = (tp + 1) ;

	} /* end while */

	return rs ;
}
/* end subroutine (loadinfo_cookcheck) */


static int loadinfo_rn(lip)
struct loadinfo	*lip ;
{
	int	rs = SR_OK ;
	int	bl ;

	const char	*bp ;


	if (lip->rn != NULL) {
	    bl = strlen(lip->rn) ;
	    goto ret0 ;
	}

	bl = sfbasename(lip->pr,-1,&bp) ;

	if (bl >= 0) {
	    const char	*cp ;
	    if (bl > SHMPREFIXLEN) bl = SHMPREFIXLEN ;
	    rs = uc_mallocstrw(bp,bl,&cp) ;
	    if (rs >= 0) lip->rn = cp ;
	} else
	    rs = SR_ISDIR ;

ret0:
	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (loadinfo_rn) */


static int loadinfo_username(lip)
struct loadinfo	*lip ;
{
	int		rs = SR_OK ;
	int		unl = 0 ;
	char		userbuf[USERNAMELEN + 1] ;

	if (lip->username == NULL) {
	    if ((rs = getusername(userbuf,USERNAMELEN,-1)) >= 0) {
	        cchar	*cp ;
		unl = rs ;
	        if (unl > SHMPREFIXLEN) unl = SHMPREFIXLEN ;
	        if ((rs = uc_mallocstrw(userbuf,unl,&cp)) >= 0) {
	            lip->username = cp ;
		}
	    } /* end if (getusername) */
	} else {
	    unl = strlen(lip->username) ;
	}

	return (rs >= 0) ? unl : rs ;
}
/* end subroutine (loadinfo_username) */


static int loadinfo_chown(lip,fd,shmi)
struct loadinfo	*lip ;
int		fd ;
int		shmi ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	cchar		*sysuser = "sys" ;

	switch (shmi) {
	case shmname_sys:
	    if ((rs = getuid_name(sysuser,-1)) >= 0) {
		uid_t	u = rs ;
		rs1 = u_fchown(fd,u,-1) ;
	    }
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (loadinfo_chown) */


#ifdef	COMMENT

static int filebuf_writezero(fp,size)
FILEBUF		*fp ;
int		size ;
{
	int	rs = SR_OK ;
	int	ml ;
	int	rlen = size ;
	int	wlen = 0 ;


	while ((rs >= 0) && (rlen > 0)) {

	    ml = MIN(rlen,4) ;
	    rs = filebuf_write(fp,zerobuf,ml) ;
	    rlen -= rs ;
	    wlen += rs ;

	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_writezero) */

#endif /* COMMENT */


static int filebuf_writefill(bp,buf,buflen)
FILEBUF		*bp ;
const char	buf[] ;
int		buflen ;
{
	int	rs ;
	int	r, nzero ;
	int	len ;
	int	asize = sizeof(uint) ;


	if (buflen < 0)
	    buflen = (strlen(buf) + 1) ;

	rs = filebuf_write(bp,buf,buflen) ;
	len = rs ;

	r = (buflen & (asize - 1)) ;
	if ((rs >= 0) && (r > 0)) {
	    nzero = (asize - r) ;
	    if (nzero > 0) {
	        rs = filebuf_write(bp,zerobuf,nzero) ;
	        len += rs ;
	    }
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (filebuf_writefill) */


static int getcookname(sp,rpp)
const char	*sp ;
const char	**rpp ;
{
	int	cnl = 1 ;

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


static int istermrs(rs)
int	rs ;
{
	int	i ;
	int	f = FALSE ;


	for (i = 0 ; termrs[i] != 0 ; i += 1) {
	    f = (rs == termrs[i]) ;
	    if (f)
	        break ;
	} /* end if */

	return f ;
}
/* end subroutine (istermrs) */


