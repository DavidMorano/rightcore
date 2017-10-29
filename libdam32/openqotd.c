/* openqotd */

/* open a channel (file-descriptor) to the quote-of-the-day (QOTD) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* special debugging */
#define	CF_OPENMASK	0		/* use |openmask()| */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	We open a QOTD quote-file and return a file-descriptor to it (which
	must be eventually closed).  The quote-file is identified by its
	Modified-Julian-Day (MJD).

	Synopsis:

	int openqotd(pr,mjd,of,to)
	cchar		*pr ;
	int		mjd ;
	int		of ;
	int		to ;

	Arguments:

	pr		program-root
	mjd		modified-julian-day
	of		open-flags
	to		time-out

	Returns:

	<0		error
	>=0		FD of QOTD

	Notes:

	- open flags:
		O_NOCTTY (delete)
		O_EXCL (expiration)
		O_SYNC
		O_NDELAY
		O_NONBLOCK

	= Implementation Notes:

	Q. Why does this subroutine have to be multi-thread-safe?
	A. Because all code needs to be multi-thread-safe today!

	Q. Why is this code not already naturally multi-thread-safe?
	A. Because this code uses file-record-locks; file-record-locks
	are inherently not multi-thread-safe!  Yes, Virginia, there are
	semi-secret pieces of system-supplied code all over the place
	that are not natually multi-thread-safe.

	Q. How do we fix the problem with using file-record-locks to make
	this subroutine mutli-thread-safe?
	A. We add a mutual-exclusion lock around the code that creates
	file-record-locks so that multiple threads can come into the code
	and not foul up due to the file-record-locks.

	Q. Since creating a new QOTD locally is such a rare event, why
	can we not call our own |_fini()| subroutine after we have
	completed that task?
	A. Beucase currently there is no way to un-register our
	|atexit()| handler from the system; currently this is only done
	when the code module containing us (this subroutine) is
	unloaded from the process memory address space.  Alternately
	there is (indeed) a way to un-register an |atfork()| handler --
	because I created that capability myself -- but we have not
	(yet) created the capability to un-register |atexit()|
	handlers.  

	Q. Why has not an un-register function been created to un-register
	an |atexit()| handler that had been previously registered?
	A. After some research into the matter, there is no *easy* way
	to create an |atexit()| un-register capability given the
	current X/Open® interface and implementation citcumstances.
	The reason is because the private data (locks and memory) that
	implement the ATEXIT facility are kept privately scoped inside
	the LIBC library (actally inside the |atexit()| subroutine code
	translation unit itself).  There is no way to access it in
	order to sneak in a little un-register function.  It would have
	been easy to implement had access to the private data
	structures been available, but they are not!  Thanks to the
	X/Open® and the previous UNIX® folks!  Got to love them!

	Q. How did we create a mutual-exclusion lock around the
	|lockfile()| subroutine?
	A. We created our own mutual-exclusion lock with our implementation
	consisting of |openqotd_capbegin()| and |openqotd_capend()|.

	Q. Why didn't we just use |ptm_lock()| and |ptm_unloc()| as
	our mutual-exclusion lock around |lockfile()|?
	A. Because:
	a. it is not a good idea to put POSIX® mutexes around a large
	piece of code (that calls unknown subroutines)
	b. the code inside the exclusion zone calls |uc_fork(3uc)| and
	that would create a deadlock (because of the "fork"-related pre-lock
	and post-unlock operations)

	Q. This is ridiculously complicated.  Were there not much easier
	ways to do this?
	A. Yes.  There are other mutual exlusion mechanisms available for
	file-system operations.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<ptm.h>
#include	<ptc.h>
#include	<tmtime.h>
#include	<storebuf.h>
#include	<vecpstr.h>
#include	<fsdir.h>
#include	<localmisc.h>

#include	"openqotd.h"


/* local defines */

#ifndef	USERNAMELEN
#ifndef	LOGNAME_MAX
#define	USERNAMELEN	LOGNAME_MAX
#else
#define	USERNAMELEN	32
#endif
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	VTMPDNAME	"/var/tmp"
#define	QCNAME		"qotd"

#define	OPENQOTD	struct openqotd_head
#define	OPENQOTD_SUB	struct openqotd_sub

#define	TTL_EXPIRE	(30*24*3600)		/* default 30 days */

#define	NFNAME		"qotd.ndeb"


/* external subroutines */

extern int	snopenflags(char *,int,int) ;
extern int	snsd(char *,int,cchar *,uint) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mkpath2w(char *,cchar *,cchar *,int) ;
extern int	pathadd(char *,int,cchar *) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	getaf(cchar *,int) ;
extern int	getmjd(int,int,int) ;
extern int	lockfile(int,int,offset_t,offset_t,int) ;
extern int	msleep(int) ;
extern int	prmktmpdir(cchar *,char *,cchar *,cchar *,mode_t) ;

extern int	maintqotd(cchar *,int,int,int) ;

extern int	dialudp(cchar *,char *,int,int,int) ;
extern int	dialtcp(cchar *,cchar *,int,int,int) ;
extern int	dialtcpnls(cchar *,cchar *,int,cchar *,int,int) ;
extern int	dialtcpmux(cchar *,cchar *,int,cchar *,cchar **,int,int) ;
extern int	dialuss(cchar *,int,int) ;
extern int	dialussnls(cchar *,cchar *,int,int) ;
extern int	dialussmux(cchar *,cchar *,cchar **,int,int) ;
extern int	dialticotsord(cchar *,int,int,int) ;
extern int	dialticotsordnls(cchar *,int,cchar *,int,int) ;
extern int	dialpass(cchar *,int,int) ;

extern int	ofWritable(int) ;
extern int	isprintlatin(int) ;
extern int	hasNotDots(cchar *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif /* CF_DEBUGS */

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strwcpylc(char *,cchar *,int) ;
extern char	*strwcpyuc(char *,cchar *,int) ;
extern char	*strcpylc(char *,cchar *) ;
extern char	*strcpyuc(char *,cchar *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */

struct openqotd_head {
	PTM		m ;		/* data mutex */
	PTC		c ;		/* condition variable */
	volatile int	waiters ;
	volatile int	f_capture ;	/* capture flag */
	volatile int	f_init ;	/* race-condition, blah, blah */
	volatile int	f_initdone ;
} ;

struct openqotd_sub {
	cchar		*pr ;
	cchar		*vtmpdname ;
	cchar		*qcname ;
	cchar		*qfname ;
	mode_t		dm ;
	int		of ;
	int		ttl ;
	int		mjd ;
} ;


/* forward references */

int		openqotd_init() ;
void		openqotd_fini() ;

static int	openqotd_capbegin(int) ;
static int	openqotd_capend() ;
static int	openqotd_open(OPENQOTD_SUB *) ;

static int	qotdexpire(cchar *,cchar *,int,cchar *,time_t,int) ;
static int	qotdexpireload(vecpstr *,char *,time_t,int) ;

static int	qotdfetch(cchar *,int,int,int,cchar *) ;

#if	CF_OPENMASK
static int	openmask(cchar *,int,mode_t) ;
#endif /* CF_OPENMASK */

static int	loadchown(cchar *,int) ;

static int	getdefmjd(time_t) ;

static int	mkqdname(char *,cchar *, cchar *,int,cchar *) ;
static int	mkqfname(char *,cchar *, cchar *,int,cchar *,int) ;

static void	openqotd_atforkbefore() ;
static void	openqotd_atforkafter() ;


/* local variables */

static OPENQOTD		openqotd_data ; /* zero-initialized */


/* exported subroutines */


int openqotd_init()
{
	OPENQOTD	*uip = &openqotd_data ;
	int		rs = 1 ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        if ((rs = ptc_create(&uip->c,NULL)) >= 0) {
	    	    void	(*b)() = openqotd_atforkbefore ;
	    	    void	(*a)() = openqotd_atforkafter ;
	            if ((rs = uc_atfork(b,a,a)) >= 0) {
	                if ((rs = uc_atexit(openqotd_fini)) >= 0) {
	                    rs = 0 ;
	                    uip->f_initdone = TRUE ;
	                }
	                if (rs < 0)
	                    uc_atforkrelease(b,a,a) ;
	            }
	            if (rs < 0)
	                ptc_destroy(&uip->c) ;
	        } /* end if (ptc_create) */
	        if (rs < 0)
	            ptm_destroy(&uip->m) ;
	    } /* end if (ptm_create) */
	} else {
	    while (! uip->f_initdone) msleep(1) ;
	}
	return rs ;
}
/* end subroutine (openqotd_init) */


void openqotd_fini()
{
	OPENQOTD	*uip = &openqotd_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    {
	        void	(*b)() = openqotd_atforkbefore ;
	        void	(*a)() = openqotd_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptc_destroy(&uip->c) ;
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(OPENQOTD)) ;
	} /* end if (was initialized) */
}
/* end subroutine (openqotd_fini) */


int openqotd(cchar *pr,int mjd,int of,int to)
{
	time_t		dt = 0 ;
	mode_t		dm = 0777 ;
	int		rs = SR_OK ;
	int		fd = -1 ;
	cchar		*vtmpdname = VTMPDNAME ;
	cchar		*qcname = QCNAME ;
	char		qfname[MAXPATHLEN+1] ;

	if (pr == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	{
	    char	obuf[100+1] ;
	    snopenflags(obuf,100,of) ;
	    debugprintf("openqotd: of=%s\n",obuf) ;
	}
#endif /* CF_DEBUGS */

	if (mjd <= 0) {
	    if (dt == 0) dt = time(NULL) ;
	    rs = getdefmjd(dt) ;
	    mjd = rs ;
	}

#if	CF_DEBUGS
	debugprintf("openqotd: mjd=%u\n",mjd) ;
	debugprintf("openqotd: vtmpdname=%s\n",vtmpdname) ;
	debugprintf("openqotd: qcname=%s\n",qcname) ;
#endif /* CF_DEBUGS */


	if (rs >= 0) {
	    int		rnl ;
	    cchar	*rnp ;
	    if ((rnl = sfbasename(pr,-1,&rnp)) > 0) {
	        cchar	*vtd = vtmpdname ;
	        cchar	*cn = qcname ;

	        if (of & O_EXCL) {
	            if (dt == 0) dt = time(NULL) ;
	            rs = qotdexpire(vtd,rnp,rnl,cn,dt,to) ;
	        }

	        if (rs >= 0) {
	            if ((rs = mkqfname(qfname,vtd,rnp,rnl,cn,mjd)) >= 0) {

	                {
	                    OPENQOTD_SUB	qs ;
	                    memset(&qs,0,sizeof(OPENQOTD_SUB)) ;
	                    qs.pr = pr ;
	                    qs.qfname = qfname ;
	                    qs.vtmpdname = vtmpdname ;
	                    qs.qcname = qcname ;
	                    qs.ttl = to ;
	                    qs.of = of ;
	                    qs.dm = dm ;
	                    qs.mjd = mjd ;
	                    if ((rs = openqotd_open(&qs)) >= 0) {
	                        fd = rs ;
	                    }
	                }

	                if ((rs >= 0) && (of & O_NOCTTY)) {
	                    u_unlink(qfname) ;
	                }

	                if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	            } /* end if (mkqfname) */
	        } /* end if */
	    } else
	        rs = SR_NOTDIR ;
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("openqotd: ret rs=%d fd=%u\n",rs,fd) ;
#endif /* CF_DEBUGS */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (openqotd) */


/* local subroutines */


static int openqotd_capbegin(int to)
{
	OPENQOTD	*uip = &openqotd_data ;
	int		rs ;
	int		rs1 ;

	if ((rs = ptm_lockto(&uip->m,to)) >= 0) {
	    uip->waiters += 1 ;

	    while ((rs >= 0) && uip->f_capture) { /* busy */
	        rs = ptc_waiter(&uip->c,&uip->m,to) ;
	    } /* end while */

	    if (rs >= 0) {
	        uip->f_capture = TRUE ;
	    }

	    uip->waiters -= 1 ;
	    rs1 = ptm_unlock(&uip->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return rs ;
}
/* end subroutine (openqotd_capbegin) */


static int openqotd_capend()
{
	OPENQOTD	*uip = &openqotd_data ;
	int		rs ;
	int		rs1 ;

	if ((rs = ptm_lock(&uip->m)) >= 0) {

	    uip->f_capture = FALSE ;
	    if (uip->waiters > 0) {
	        rs = ptc_signal(&uip->c) ;
	    }

	    rs1 = ptm_unlock(&uip->m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

	return rs ;
}
/* end subroutine (openqotd_capend) */


static void openqotd_atforkbefore()
{
	OPENQOTD	*uip = &openqotd_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (openqotd_atforkbefore) */


static void openqotd_atforkafter()
{
	OPENQOTD	*uip = &openqotd_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (openqotd_atforkafter) */


static int openqotd_open(OPENQOTD_SUB *sip)
{
	const mode_t	om = 0666 ;
	int		rs ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("openqotd_open: ent qfname=%s\n",sip->qfname) ;
#endif /* CF_DEBUGS */

	if ((rs = u_open(sip->qfname,sip->of,om)) >= 0) {
	    fd = rs ;
	} else if (rs == SR_NOENT) {
	    const mode_t	dm = sip->dm ;
	    cchar		*pr = sip->pr ;
	    cchar		*qfname = sip->qfname ;
	    cchar		*vtmpdname = sip->vtmpdname ;
	    cchar		*qcname = sip->qcname ;
	    char		qdname[MAXPATHLEN+1] ;
	    if ((rs = prmktmpdir(pr,qdname,vtmpdname,qcname,dm)) >= 0) {
	        int	mjd = sip->mjd ;
	        int	of = sip->of ;
	        int	ttl = sip->ttl ;
	        rs = qotdfetch(pr,mjd,of,ttl,qfname) ;
	        fd = rs ;
	    }
	} /* end if (NOENT) */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (openqotd_open) */


static int qotdexpire(vtd,rnp,rnl,cn,dt,to)
cchar		*vtd ;
cchar		*rnp ;
int		rnl ;
cchar		*cn ;
time_t		dt ;
int		to ;
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	char		qdname[MAXPATHLEN+1] ;

	if (to <= 0) to = TTL_EXPIRE ;

#if	CF_DEBUGS
	{
	    char	tbuf[TIMEBUFLEN+1] ;
	    debugprintf("qotdexpire: ent to=%u\n",to) ;
	    timestr_log(dt,tbuf) ;
	    debugprintf("qotdexpire: et=%s\n",tbuf) ;
	}
#endif

	if ((rs = mkqdname(qdname,vtd,rnp,rnl,cn)) >= 0) {
	    struct ustat	sb ;
#if	CF_DEBUGS
	    debugprintf("qotdexpire: qd=%s\n",qdname) ;
#endif
	    if (u_stat(qdname,&sb) >= 0) {
	        if (S_ISDIR(sb.st_mode)) {
	            const int	n = (sb.st_size / 10) ;
	            const int	cs = (sb.st_size / 4) ;
	            VECPSTR	ds ;
	            if ((rs = vecpstr_start(&ds,n,cs,0)) >= 0) {
	                if ((rs = qotdexpireload(&ds,qdname,dt,to)) > 0) {
	                    int		i ;
	                    cchar	*fn ;
	                    for (i = 0 ; vecpstr_get(&ds,i,&fn) >= 0 ; i += 1) {
	                        if (fn == NULL) continue ;
#if	CF_DEBUGS
	                        debugprintf("qotdexpire: del=%s\n",
	                            fn) ;
#endif
	                        rs1 = u_unlink(fn) ;
	                        if (rs1 >= 0) c += 1 ;
	                    } /* end for */
	                } /* end if (qotdexpireload) */
	                vecpstr_finish(&ds) ;
	            } /* end if (vecpstr) */
	        } else
	            rs = SR_NOTDIR ;
	    } /* end if (stat) */
	} /* end if (mkqdname) */

#if	CF_DEBUGS
	debugprintf("qotdexpire: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (qotdexpire) */


static int qotdexpireload(vecpstr *dsp,char *qfname,time_t dt,int to)
{
	FSDIR		d ;
	FSDIR_ENT	de ;
	int		rs ;
	int		c = 0 ;

	if ((rs = fsdir_open(&d,qfname)) >= 0) {
	    struct ustat	sb ;
	    const int		dlen = strlen(qfname) ;
	    while ((rs = fsdir_read(&d,&de)) > 0) {
	        const int	el = rs ;
	        cchar		*ep = de.name ;
	        if (hasNotDots(ep,el)) {
	            if ((rs = pathadd(qfname,dlen,ep)) >= 0) {
	                const int	fl = rs ;
#if	CF_DEBUGS
	                debugprintf("qotdexpire: qf=%s\n",qfname) ;
#endif
	                if (u_stat(qfname,&sb) >= 0) {
	                    if (S_ISREG(sb.st_mode)) {

#if	CF_DEBUGS
	                        {
	                            char	tbuf[TIMEBUFLEN+1] ;
	                            timestr_log(sb.st_mtime,tbuf) ;
	                            debugprintf("qotdexpire: mt=%s\n",tbuf) ;
	                        }
#endif
	                        if ((dt-sb.st_mtime) >= to) {
#if	CF_DEBUGS
	                            debugprintf("qotdexpire: "
	                                "adding=%s\n", de.name) ;
#endif
	                            c += 1 ;
	                            rs = vecpstr_add(dsp,qfname,fl) ;
	                        } /* end if (expired) */
	                    } /* end if (regular file) */
	                } /* end if (stat) */
	            } /* end if (pathadd) */
	        } /* end if (hasNotDots) */
	        if (rs < 0) break ;
	    } /* end while (fsdir_read) */
	    qfname[dlen] = '\0' ;
	    fsdir_close(&d) ;
	} /* end if (fsdir) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (qotdexpireload) */


/* ARGSUSED */
static int qotdfetch(pr,mjd,of,ttl,qfname)
cchar		*pr ;
int		mjd ;
int		of ;
int		ttl ;
cchar		*qfname ;
{
	const mode_t	om = 0664 ;
	int		rs ;
	int		rs1 ;
	int		lof = of ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("openqotd/qotdfetch: ent mjd=%u\n",mjd) ;
#endif

	lof &= (~ O_ACCMODE) ;
	lof &= (~ O_TRUNC) ;
	lof &= (~ O_EXCL) ;
	lof |= (O_CREAT | O_RDWR) ;

	if ((rs = u_open(qfname,lof,om)) >= 0) {
	    fd = rs ;
	    if ((rs = uc_fminmod(fd,om)) >= 0) {
	        if ((rs = openqotd_init()) >= 0) {
	            const int	to = (5*60) ;
	            if ((rs = openqotd_capbegin(to)) >= 0) {
	                const int	cmd = F_WLOCK ;
	                if ((rs = lockfile(fd,cmd,0L,0L,to)) >= 0) {
	                    if ((rs = uc_fsize(fd)) == 0) {
	                        if ((rs = maintqotd(pr,mjd,of,to)) >= 0) {
	                            const int	s = rs ;
	                            if ((rs = uc_writedesc(fd,s,-1)) >= 0) {
	                                if ((rs = u_rewind(fd)) >= 0) {
	                                    rs = loadchown(pr,fd) ;
					}
	                            }
	                            u_close(s) ;
	                        } /* end if (maintqotd) */
	                    } /* end if (file-size-is-zero) */
	                    rs1 = uc_lockf(fd,F_ULOCK,0L) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (file-lock) */
	                rs1 = openqotd_capend() ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (capture-exclusion) */
	        } /* end if (openqotd_init) */
	    } /* end if (uc_fminmod) */
	    if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	} /* end if (open-) */

#if	CF_DEBUGS
	debugprintf("openqotd/qotdfetch: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (qotdfetch) */


#if	CF_OPENMASK
static int openmask(cchar *qfname,int of,mode_t om)
{
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;
	if ((rs = umaskset(0)) >= 0) {
	    mode_t	pm = rs ;
	    rs = u_open(qfname,of,om) ;
	    fd = rs ;
	    rs1 = umaskset(pm) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */
	if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	return (rs >= 0) ? fd : rs ;
}
/* end if (openmask) */
#endif /* CF_OPENMASK */


static int loadchown(cchar *pr,int fd)
{
	struct ustat	sb ;
	int		rs ;

	if ((rs = u_stat(pr,&sb)) >= 0) {
	    uid_t	euid = geteuid() ;
	    if (euid != sb.st_uid) {
	        u_fchown(fd,sb.st_uid,sb.st_gid) ;
	    }
	}

	return rs ;
}
/* end subroutine (loadchown) */


static int getdefmjd(time_t dt)
{
	TMTIME		ct ;
	int		rs ;
	if (dt == 0) dt = time(NULL) ;
	if ((rs = tmtime_localtime(&ct,dt)) >= 0) {
	    int	y = (ct.year + TM_YEAR_BASE) ;
	    int	m = ct.mon ;
	    int	d = ct.mday ;
	    rs = getmjd(y,m,d) ;
	}
	return rs ;
}
/* end subroutine (getdefmjd) */


static int mkqdname(rbuf,vtmpdname,rnp,rnl,qcname)
char		rbuf[] ;
cchar		vtmpdname[] ;
cchar		qcname[] ;
cchar		rnp[] ;
int		rnl ;
{
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,vtmpdname,-1) ;
	    i += rs ;
	}

	if ((rs >= 0) && (i > 0) && (rbuf[i-1] != '/')) {
	    rs = storebuf_char(rbuf,rlen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,rnp,rnl) ;
	    i += rs ;
	}

	if ((rs >= 0) && (i > 0) && (rbuf[i-1] != '/')) {
	    rs = storebuf_char(rbuf,rlen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,qcname,-1) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkqdname) */


static int mkqfname(rbuf,vtmpdname,rnp,rnl,qcname,mjd)
char		rbuf[] ;
cchar		vtmpdname[] ;
cchar		qcname[] ;
cchar		rnp[] ;
int		rnl ;
int		mjd ;
{
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i = 0 ;

	if (rs >= 0) {
	    rs = mkqdname(rbuf,vtmpdname,rnp,rnl,qcname) ;
	    i += rs ;
	}

	if ((rs >= 0) && (i > 0) && (rbuf[i-1] != '/')) {
	    rs = storebuf_char(rbuf,rlen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(rbuf,rlen,i,'q') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_deci(rbuf,rlen,i,mjd) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkqfname) */


#ifdef	COMMENT
extern int ofWritable(int of)
{
	int	f = FALSE ;
	f = f || ((of & O_ACCMODE) == O_WRONLY) ;
	f = f || ((of & O_ACCMODE) == O_RDWR) ;
	return f ;
}
/* end subroutine (ofWritable) */
#endif /* COMMENT */


