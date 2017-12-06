/* ncpu */

/* find the number of processes on the system */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SHM		1		/* use SHM */
#define	CF_SHMLOCAL	0		/* use local SHM */
#define	CF_OTHER	0		/* use SYSMISCS */
#define	CF_KINFO	0		/* use KINFO */


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

	Notes:

	= Load-averages

	Although load-averages are available when retrieving SYSMISC
	(miscellaneous system) information from the kernel, we don't
	bother with it at all since the general introduction of the
	'getloadavg(3c)' subroutine in the world.  If that subroutine
	was not available, load-averages would have to be treated as
	being as difficult to retrieve as the number of CPUs is.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<dlfcn.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<storebuf.h>
#include	<nulstr.h>
#include	<filebuf.h>
#include	<kinfo.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"modload.h"
#include	"sysmiscem.h"
#include	"sysmiscfh.h"
#include	"getsysmisc.h"


/* local defines */

#define	NCPU_PREFIXLEN		5
#define	NCPU_POSTFIXLEN		7

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

#define	LIBCNAME	"lib"
#define	SHMPERMS	0666
#define	SHMDBNAME	"sysmisc"

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
#define	MODSYMNAME	"ncpu"		/* symbol-name */

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#define	HDRBUFLEN	(20 + sizeof(SYSMISCFH))

#define	TO_UPDATE	60
#define	TO_SHMWAIT	10


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,const char *,const char *,const char *,
			const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,const char *,
			const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;
extern int	getsysmisc(GETSYSMISC *,time_t) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct subinfo_flags {
	uint		shm:1 ;
} ;

struct subinfo {
	const char	*pr ;
	const char	*prbuf ;
	const char	*shmname ;
	char		*mapdata ;
	uint		*shmtable ;
	struct subinfo_flags	f ;
	time_t		daytime ;
	time_t		ti_map ;
	int		pagesize ;
	int		mapsize ;
	int		shmsize ;
} ;


/* forward references */

static int subinfo_start(struct subinfo *,const char *) ;
static int subinfo_finish(struct subinfo *) ;
static int subinfo_pr(struct subinfo *) ;

static int ncpu_env(struct subinfo *) ;
static int ncpu_nprocessors(struct subinfo *) ;
static int ncpu_shm(struct subinfo *) ;
static int ncpu_kinfo(struct subinfo *) ;
static int ncpu_default(struct subinfo *) ;

#if	CF_OTHER
static int ncpu_other(struct subinfo *) ;
#endif

static int	ncpu_shmcreate(struct subinfo *,const char *,mode_t) ;
static int	ncpu_shmdestroy(struct subinfo *) ;
static int	ncpu_shmwr(struct subinfo *,int,mode_t) ;

static int	ncpu_shmmapbegin(struct subinfo *,int) ;
static int	ncpu_shmmapend(struct subinfo *) ;

static int	ncpu_shmproc(struct subinfo *) ;
static int	ncpu_shmverify(struct subinfo *,SYSMISCFH *) ;

static int	ncpu_shmupdate(struct subinfo *) ;
static int	ncpu_shmchild(struct subinfo *) ;
static int	ncpu_shmopenwait(struct subinfo *,mode_t) ;

static int	filebuf_writefill(FILEBUF *,const char *,int) ;
static int	filebuf_writezero(FILEBUF *,int) ;

static int	istermrs(int) ;


/* local variables */

static int	(*tries[])(struct subinfo *) = {
	ncpu_env,
	ncpu_nprocessors,
	ncpu_shm,
	ncpu_default,
	NULL
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


/* exported subroutines */


int ncpu(pr)
const char	*pr ;
{
	struct subinfo	si ;

	int	rs ;
	int	i ;
	int	n = 0 ;


	if ((rs = subinfo_start(&si,pr)) >= 0) {

	    for (i = 0 ; tries[i] != NULL ; i += 1) {

	        rs = (*tries[i])(&si) ;
	        n = rs ;
	        if (rs == SR_NOENT) rs = SR_OK ;
	        if (rs == SR_NOSYS) rs = SR_OK ;

	        if ((rs > 0) || istermrs(rs))
	            break ;

	    } /* end for */

#if	CF_DEBUGS
	    debugprintf("ncpu: subinfo_finish()\n") ;
#endif

	    rs1 = subinfo_finish(&si) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

	if ((rs < 0) && (! istermrs(rs))) {
	    rs = SR_OK ;
	    n = 0 ;
	}

#if	CF_DEBUGS
	debugprintf("ncpu: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (ncpu) */


/* local subroutines */


static int subinfo_start(sip,pr)
struct subinfo	*sip ;
const char	*pr ;
{

	memset(sip,0,sizeof(struct subinfo)) ;
	sip->pr = pr ;

	return SR_OK ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(sip)
struct subinfo	*sip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->prbuf != NULL) {
	    rs1 = uc_free(sip->prbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->prbuf = NULL ;
	}

	sip->pr = NULL ;
	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_pr(sip)
struct subinfo	*sip ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	prl ;

	const char	*cp ;

	char	domainname[MAXHOSTNAMELEN + 1] ;
	char	prbuf[MAXPATHLEN + 1] ;


	if (sip->pr != NULL)
	    goto ret0 ;

	rs1 = getnodedomain(NULL,domainname) ;
	if (rs1 < 0)
	    domainname[0] = '\0' ;

	rs1 = mkpr(prbuf,MAXPATHLEN,VARPR,domainname) ;
	prl = rs1 ;
	if (rs1 < 0)
	    prbuf[0] = '\0' ;

	rs = uc_mallocstrw(prbuf,prl,&cp) ;
	if (rs >= 0) {
	    sip->prbuf = cp ;
	    sip->pr = cp ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("ncpu: rs=%d prbuf=%s\n",rs,sip->prbuf) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_pr) */


static int ncpu_env(sip)
struct subinfo	*sip ;
{
	int	rs = SR_NOSYS ;
	int	rs1 ;
	int	n = 0 ;

	const char	*vn = VARNCPU ;
	const char	*cp ;


	if ((cp = getenv(vn)) != NULL) {

	    rs = cfdeci(cp,-1,&n) ;
	    if ((rs == SR_INVALID) || (rs == SR_DOM)) rs = SR_NOSYS ;

	} /* end if (environment) */

	return (n >= 0) ? n : rs ;
}
/* end subroutine (ncpu_env) */


static int ncpu_nprocessors(sip)
struct subinfo	*sip ;
{
	long	rn = 0 ;

	int	rs = SR_NOSYS ;
	int	n = 0 ;


#ifdef	_SC_NPROCESSORS_ONLN
	rs = uc_sysconf(_SC_NPROCESSORS_ONLN,&rn) ;
	n = (rn & INT_MAX) ;
	if (rs == SR_INVALID) rs = SR_NOSYS ;
#endif /* _SC_NPROCESSORS_ONLN */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (ncpu_nprocessors) */


#if	CF_KINFO

int ncpu_kinfo(sip)
struct subinfo	*sip ;
{
	GETSYSMISC	kd ;

	int	rs ;

	if (sip->daytime == 0)
	    sip->daytime = time(NULL) ;

	rs = getsysmisc(&kd,sip->daytime) ;

	return rs ;
}
/* end subroutine (ncpu_kinfo) */

#endif /* CF_KINFO */


#if	CF_SHM

#if	CF_SHMLOCAL

static int ncpu_shm(sip)
struct subinfo	*sip ;
{
	uint	*shmtable ;

	mode_t	operms = SHMPERMS ;

	int	rs = SR_OK ;
	int	n = 0 ;

	const char	*dbname = SHMDBNAME ;


	if ((rs = ncpu_shmcreate(sip,dbname,operms)) >= 0) {

	    shmtable = (uint *) (sip->mapdata + SYSMISCFH_IDLEN) ;
	    n = shmtable[sysmiscfv_ncpu] ;

#if	CF_DEBUGS
	    debugprintf("ncpu_shm: ncpu=%d\n", shmtable[sysmiscfv_ncpu]) ;
	    debugprintf("ncpu_shm: nproc=%d\n", shmtable[sysmiscfv_nproc]) ;
	    debugprintf("ncpu_shm: bt=%u\n", shmtable[sysmiscfv_btime]) ;
#endif

	    ncpu_shmdestroy(sip) ;
	} /* end if */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (ncpu_shm) */


static int ncpu_shmcreate(sip,dbname,operms)
struct subinfo	*sip ;
const char	dbname[] ;
mode_t		operms ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	fd = -1 ;
	int	oflags ;
	int	cl ;
	int	f_needwr = FALSE ;
	int	f_needupdate = FALSE ;
	int	f_needchmod = FALSE ;

	const char	*cp ;

	char	shmname[MAXNAMELEN + 1] ;
	char	prefix[NCPU_PREFIXLEN + 1] ;
	char	postfix[NCPU_POSTFIXLEN + 1] ;


	sip->shmsize = 0 ;
	sip->shmtable = NULL ;

	cl = sfbasename(sip->pr,-1,&cp) ;
	if (cl <= 0) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

	strwcpy(prefix,cp,MIN(cl,NCPU_PREFIXLEN)) ;
	strwcpy(postfix,dbname,NCPU_POSTFIXLEN) ;

	rs = sncpy4(shmname,MAXNAMELEN,"/",prefix,"$",postfix) ;
	cl = rs ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("ncpu_shmcreate: shmname=%s\n",shmname) ;
#endif

	rs = uc_mallocstrw(shmname,cl,&cp) ;
	sip->shmname = cp ;
	if (rs < 0)
	    goto ret0 ;

	oflags = O_RDWR ;
	rs = uc_openshm(shmname,oflags,operms) ;
	fd = rs ;

#if	CF_DEBUGS
	debugprintf("ncpu_shmcreate: RDWR uc_openshm() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    rs = u_fstat(fd,&sb) ;
	    sip->shmsize = sb.st_size ;
	    f_needwr = (sip->shmsize < SYSMISCFH_IDLEN) ;
	}

	if (rs == SR_NOENT) {

	    f_needwr = TRUE ;
	    oflags = (O_RDWR | O_CREAT | O_EXCL) ;
	    rs = uc_openshm(shmname,oflags,(operms & 0444)) ;
	    fd = rs ;

#if	CF_DEBUGS
	    debugprintf("ncpu_shmcreate: RDWR|CREAT uc_openshm() rs=%d\n",
	        rs) ;
#endif

	} /* end if */

	if ((rs >= 0) && f_needwr) {

	    if (rs >= 0) {
	        if (sip->daytime == 0)
	            sip->daytime = time(NULL) ;

	        f_needupdate = TRUE ;
	        f_needchmod = TRUE ;
	        rs = ncpu_shmwr(sip,fd,operms) ;
	    }

	} /* end if */

	if ((rs == SR_ACCESS) || (rs == SR_EXIST)) {
	    sip->shmsize = 0 ;
	    rs = ncpu_shmopenwait(sip,operms) ;
	    fd = rs ;
	}

	if (rs < 0)
	    goto ret1 ;

/* map it */

	if ((rs >= 0) && (sip->shmsize == 0)) {
	    rs = u_fstat(fd,&sb) ;
	    sip->shmsize = sb.st_size ;
	    f_needupdate = f_needupdate || (sip->shmsize == 0) ;
	}

	if (rs >= 0) {
	    rs = ncpu_shmmapbegin(sip,fd) ;
	    if (rs >= 0) {
	        rs = ncpu_shmproc(sip) ;
	        f_needupdate = f_needupdate || (rs > 0) ;
	    }
	}

	if ((rs >= 0) && f_needupdate) {
	    rs = ncpu_shmupdate(sip) ;

#if	CF_DEBUGS
	    debugprintf("ncpu_shmcreate: ncpu_shmupdate() rs=%d\n",rs) ;
#endif

	} /* end if */

	if ((rs >= 0) && f_needchmod)
	    u_fchmod(fd,operms) ;

ret2:
	if (rs >= 0) {
	    sip->f.shm = TRUE ;
	} else {
	    ncpu_shmmapend(sip) ;
	    sip->shmtable = NULL ;
	}

/* close it (it stays mapped) */
ret1:
	if (fd >= 0)
	    u_close(fd) ;

ret0:
	return rs ;
}
/* end subroutine (ncpu_shmcreate) */


static int ncpu_shmdestroy(sip)
struct subinfo	*sip ;
{
	int	rs = SR_OK ;


	if (sip->mapdata != NULL) {
	    rs = ncpu_shmmapend(sip) ;
	    sip->shmtable = NULL ;
	}

	return rs ;
}
/* end subroutine (ncpu_shmdestroy) */


static int ncpu_shmwr(sip,fd,operms)
struct subinfo	*sip ;
int		fd ;
mode_t		operms ;
{
	SYSMISCFH	hdr ;

	FILEBUF	babyfile ;

	uint	fileoff = 0 ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	size ;
	int	bl ;

	char	hdrbuf[HDRBUFLEN + 1] ;


	sip->shmsize = 0 ;
	if (sip->daytime == 0)
	    sip->daytime = time(NULL) ;

	if (sip->pagesize == 0)
	    sip->pagesize = getpagesize() ;

	size = (sip->pagesize * 4) ;

/* prepare the file-header */

	memset(&hdr,0,sizeof(SYSMISCFH)) ;
	hdr.vetu[0] = SYSMISCFH_VERSION ;
	hdr.vetu[1] = ENDIAN ;
	hdr.vetu[2] = 0 ;
	hdr.vetu[3] = 0 ;
	hdr.utime = (uint) sip->daytime ;

/* create the file-header */

	rs = sysmiscfh(&hdr,0,hdrbuf,HDRBUFLEN) ;
	bl = rs ;

/* write file-header */

	if (rs >= 0) {
	    if ((rs = filebuf_start(&babyfile,fd,0L,size,0)) >= 0) {

	        rs = filebuf_writefill(&babyfile,hdrbuf,bl) ;
	        fileoff += rs ;

	        rs1 = filebuf_finish(&babyfile) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (file) */
	} /* end if */

/* write out the header -- again! */

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
	    sip->shmsize = fileoff ;
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
/* end subroutine (ncpu_shmwr) */


static int ncpu_shmopenwait(sip,operms)
struct subinfo	*sip ;
mode_t		operms ;
{
	const int	to = TO_SHMWAIT ;
	int		rs = SR_OK ;
	int		oflags = O_RDWR ;
	int		fd = -1 ;
	const char	*shmname = sip->shmname ;

	while (to-- > 0) {

	    rs = uc_openshm(shmname,oflags,operms) ;
	    fd = rs ;
	    if (rs >= 0)
	        break ;

	    if (rs != SR_ACCESS)
	        break ;

	} /* end while */

	if ((rs < 0) && (to == 0))
	    rs = SR_TIMEDOUT ;

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (ncpu_shmopenwait) */


static int ncpu_shmmapbegin(sip,fd)
struct subinfo	*sip ;
int		fd ;
{
	size_t	msize ;

	int	rs = SR_OK ;
	int	mprot ;
	int	mflags ;

	char	*mp ;


	if (fd < 0)
	    return SR_INVALID ;

	if (sip->daytime == 0)
	    sip->daytime = time(NULL) ;

	msize = sip->shmsize ;
	mprot = PROT_READ | PROT_WRITE ;
	mflags = MAP_SHARED ;
	rs = u_mmap(NULL,msize,mprot,mflags, fd,0L,&mp) ;

#if	CF_DEBUGS
	debugprintf("ncpu_mapinit: u_mmap() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad1 ;

	sip->mapdata = mp ;
	sip->mapsize = msize ;
	sip->ti_map = sip->daytime ;

ret0:
	return rs ;

/* bad stuff */
bad1:
	sip->mapdata = NULL ;
	sip->mapsize = 0 ;
	sip->ti_map = 0 ;

bad0:
	goto ret0 ;
}
/* end subroutine (ncpu_shmmapbegin) */


static int ncpu_shmmapend(sip)
struct subinfo	*sip ;
{
	int	rs = SR_OK ;


	if (sip->mapdata != NULL) {
	    u_munmap(sip->mapdata,sip->mapsize) ;
	    sip->mapdata = NULL ;
	    sip->mapsize = 0 ;
	    sip->ti_map = 0 ;
	    if (sip->f.shm && (sip->shmtable != NULL)) {
	        sip->f.shm = FALSE ;
	        sip->shmtable = NULL ;
	    }
	}

	return rs ;
}
/* end subroutine (ncpu_shmmapend) */


static int ncpu_shmproc(sip)
struct subinfo	*sip ;
{
	SYSMISCFH	hdr ;

	uint	dtime = (uint) sip->daytime ;
	uint	utime ;
	uint	intstale ;
	uint	*shmtable ;

	int	rs ;
	int	shmsize ;
	int	f_stale = FALSE ;


#if	CF_DEBUGS
	debugprintf("ncpu_shmproc: ent\n") ;
#endif

	rs = sysmiscfh(&hdr,1,sip->mapdata,sip->mapsize) ;
	if (rs < 0)
	    goto ret0 ;

	rs = ncpu_shmverify(sip,&hdr) ;
	f_stale = (rs > 0) ;
	if (rs < 0)
	    goto ret0 ;

	shmtable = (uint *) (sip->mapdata + SYSMISCFH_IDLEN) ;
	sip->shmtable = shmtable ;

	utime = shmtable[sysmiscfv_utime] ;
	shmsize = shmtable[sysmiscfv_shmsize] ;
	intstale = shmtable[sysmiscfv_intstale] ;
	if (shmsize != sip->shmsize) {
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
/* end subroutine (ncpu_shmproc) */


static int ncpu_shmverify(sip,hp)
struct subinfo	*sip ;
SYSMISCFH	*hp ;
{
	int	rs = SR_OK ;
	int	f_stale = FALSE ;


	f_stale = (hp->shmsize != sip->shmsize) ;

#if	CF_DEBUGS
	debugprintf("ncpu_shmverify: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_stale : rs ;
}
/* end subroutine (ncpu_shmverify) */


static int ncpu_shmupdate(sip)
struct subinfo	*sip ;
{
	pid_t	pid ;

	int	rs ;
	int	cs = 0 ;


	if (sip->daytime == 0)
	    sip->daytime = time(NULL) ;

	rs = uc_fork() ;
	pid = rs ;
	if (rs < 0)
	    goto ret0 ;

	if (pid == 0) {
	    int	ex ;
	    int	i ;

	    u_setsid() ; /* failure is ok */
	    for (i = 0 ; i < 3 ; i += 1)
	        u_close(i) ;

	    rs = ncpu_shmchild(sip) ;

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
/* end subroutine (ncpu_shmupdate) */


static int ncpu_shmchild(sip)
struct subinfo	*sip ;
{
	GETSYSMISC	kd ;

	uint	*shmtable = sip->shmtable ;

	int	rs ;


	if (sip->shmtable == NULL) {
	    rs = SR_FAULT ;
	    goto ret0 ;
	}

	rs = getsysmisc(&kd,sip->daytime) ;
	if (rs < 0)
	    goto ret0 ;

	shmtable[sysmiscfv_shmsize] = sip->shmsize ;
	shmtable[sysmiscfv_ncpu] = kd.ncpu ;
	shmtable[sysmiscfv_nproc] = kd.nproc ;
	shmtable[sysmiscfv_btime] = kd.btime ;
	shmtable[sysmiscfv_utime] = sip->daytime ; /* this should be last */

ret0:
	return rs ;
}
/* end subroutine (ncpu_shmchild) */


#else /* CF_SHMLOCAL */


static int ncpu_shm(sip)
struct subinfo	*sip ;
{
	SYSMISCEM	sm ;
	SYSMISCEM_DATA	sd ;

	const int	to = TO_UPDATE ;

	int	rs ;
	int	n = 0 ;


#if	CF_DEBUGS
	debugprintf("ncpu_shm: ent pr=%s\n",sip->pr) ;
#endif

	rs = subinfo_pr(sip) ;
	if (rs < 0)
	    goto ret0 ;

	if ((rs = sysmiscem_open(&sm,sip->pr)) >= 0) {

	    if (sip->daytime == 0)
	        sip->daytime = time(NULL) ;

#if	CF_DEBUGS
	    debugprintf("ncpu_shm: sysmiscem_get()\n") ;
#endif

	    rs = sysmiscem_get(&sm,sip->daytime,to,&sd) ;
	    n = rs ;

#if	CF_DEBUGS
	    debugprintf("ncpu_shm: sysmiscem_get() rs=%d\n",rs) ;
#endif

	    sysmiscem_close(&sm) ;
	} /* end if (SYSMISCEM) */

ret0:

#if	CF_DEBUGS
	debugprintf("ncpu_shm: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (ncpu_shm) */

#endif /* CF_SHMLOCAL */


#endif /* CF_SHM */


#if	CF_OTHER

static int ncpu_other(sip)
struct subinfo	*sip ;
{
	MODLOAD	loader, *lp = &loader ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	opts ;
	int	n = 0 ;

	const void	*cvp ;

	const char	*syms[2] = { "ncpu", NULL } ;
	const char	*modbname ;
	const char	*modname ;
	const char	*modsymname ;


	rs = subinfo_pr(sip) ;
	if (rs < 0)
	    goto ret0 ;

	modbname = MODBNAME ;
	modname = modbname ;
	opts = (MODLOAD_OLIBVAR | MODLOAD_OPRS | MODLOAD_OSDIRS) ;
	rs = modload_open(lp,sip->prbuf,modbname,modname,opts,syms) ;
	if (rs >= 0) {

	    modsymname = MODSYMNAME ;
	    rs1 = modload_getsym(lp,modsymname,&cvp) ;

	    if ((rs1 >= 0) && (cvp != NULL) && (sip->pr != NULL)) {
	        int (*callp)(const char *) = (int (*)(const char *)) cvp ;
	        rs = (*callp)(sip->pr) ;
	        if (rs >= 0)
	            n = rs ;
	    } /* end if */

	    modload_close(lp) ;
	} /* end if (modload) */

ret0:
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (ncpu_other) */

#endif /* CF_OTHER */


static int ncpu_default(sip)
struct subinfo	*sip ;
{
	int	rs = SR_OK ;
	int	n = 1 ;


	return (rs >= 0) ? n : rs ;
}
/* end subroutine (ncpu_default) */


static int filebuf_writezero(fp,size)
FILEBUF		*fp ;
int		size ;
{
	int		rs = SR_OK ;
	int		ml ;
	int		rlen = size ;
	int		wlen = 0 ;

	while ((rs >= 0) && (rlen > 0)) {

	    ml = MIN(rlen,4) ;
	    rs = filebuf_write(fp,zerobuf,ml) ;
	    rlen -= rs ;
	    wlen += rs ;

	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_writezero) */


static int filebuf_writefill(bp,buf,buflen)
FILEBUF		*bp ;
const char	buf[] ;
int		buflen ;
{
	int		rs ;
	int		r, nzero ;
	int		len ;
	int		asize = sizeof(uint) ;

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


static int istermrs(int rs)
{
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; termrs[i] != 0 ; i += 1) {
	    f = (rs == termrs[i]) ;
	    if (f) break ;
	} /* end if */

	return f ;
}
/* end subroutine (istermrs) */


