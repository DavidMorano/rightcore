/* authtimeout */

/* authorization time-out */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-05-01, David Morano

	This was created along with the DATE object.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This small code piece provides for authoriation time-outs.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<estrings.h>
#include	<vecobj.h>
#include	<tmtime.h>
#include	<filebuf.h>
#include	<storebuf.h>
#include	<ptma.h>
#include	<ptm.h>
#include	<localmisc.h>


/* local defines */

#define	BABYCALCS_POSTFIXLEN	7
#define	BABYCALCS_ENTRY		struct babycalcs_e
#define	BABYCALCS_PERMS		0666

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
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

#define	HDRBUFLEN	(sizeof(BABIESFU) + MAXNAMELEN)

#ifndef	TO_WAITSHM
#define	TO_WAITSHM	20		/* seconds */
#endif

#define	TO_LASTCHECK	5		/* seconds */
#define	TO_DBWAIT	1		/* seconds */
#define	TO_DBPOLL	300		/* milliseconds */

#define	SHIFTINT	(6 * 60)	/* possible time-shift */


/* external subroutines */

extern ulong	ulceil(ulong,int) ;

extern uint	uceil(uint,int) ;

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,const char *,const char *,
			const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	strnnlen(const char *,int,int) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	iceil(int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;

#if	CF_DEBUGS
extern char	*timestr_log(time_t,char *) ;
#endif


/* exported variables */


/* local structures */

struct babycalcs_e {
	time_t		date ;
	uint		count ;
} ;


/* forward references */

static int	babycalcs_loadshm(BABYCALCS *,const char *,int) ;
static int	babycalcs_loadtxt(BABYCALCS *) ;

static int	babycalcs_mapbegin(BABYCALCS *,time_t,int) ;
static int	babycalcs_mapend(BABYCALCS *) ;

static int	babycalcs_proctxt(BABYCALCS *,vecobj *) ;
static int	babycalcs_proctxtline(BABYCALCS *,vecobj *,CVTDATER *,
			const char *,int) ;

static int	babycalcs_shmwr(BABYCALCS *,time_t,int,int) ;
static int	babycalcs_openshmwait(BABYCALCS *,const char *) ;
static int	babycalcs_mutexinit(BABYCALCS *) ;
static int	babycalcs_procmap(BABYCALCS *,time_t) ;
static int	babycalcs_verify(BABYCALCS *,time_t) ;

static int	babycalcs_lookshm(BABYCALCS *,time_t,time_t,uint *) ;
static int	babycalcs_lookproc(BABYCALCS *,time_t,uint *) ;
static int	babycalcs_lookinfo(BABYCALCS *,BABYCALCS_INFO *) ;
static int	babycalcs_calc(BABYCALCS *,int,time_t,uint *) ;
static int	babycalcs_dbcheck(BABYCALCS *,time_t) ;
static int	babycalcs_dbwait(BABYCALCS *,time_t,struct ustat *) ;
static int	babycalcs_reloadshm(BABYCALCS *,time_t,struct ustat *) ;
static int	babycalcs_reloadtxt(BABYCALCS *,time_t,struct ustat *) ;
static int	babycalcs_shmcheck(BABYCALCS *,struct ustat *) ;
static int	babycalcs_shmaccess(BABYCALCS *,time_t) ;
static int	babycalcs_shmupdate(BABYCALCS *,time_t,struct ustat *,int) ;
static int	babycalcs_shmaddwrite(BABYCALCS *,int) ;
static int	babycalcs_shminfo(BABYCALCS *,BABYCALCS_INFO *) ;

static int	filebuf_writefill(FILEBUF *,const char *,int) ;
static int	filebuf_writezero(FILEBUF *,int) ;

static int	mkshmname(char *,const char *,int,const char *,int) ;

static int	cmpentry(BABYCALCS_ENTRY **,BABYCALCS_ENTRY **) ;


/* local variables */

static BABYCALCS_ENTRY	defs[] = {
	{ 96526800, 0 },
	{ 1167627600, 47198810 },	/* from Guntmacker Institute */
	{ 0, 0 }
} ;

static const char	zerobuf[4] = {
	0, 0, 0, 0 
} ;


/* exported subroutines */


int authtimeout(op,pr,dbname)
BABYCALCS	*op ;
const char	pr[] ;
const char	dbname[] ;
{
	const int	operms = BABYCALCS_PERMS ;

	int	rs = SR_OK ;
	int	f ;

	char	dbcomp[MAXNAMELEN + 1] ;
	char	dbfname[MAXPATHLEN + 1] ;


	if (op == NULL)
	    return SR_FAULT ;

	if (pr == NULL)
	    return SR_FAULT ;

	if (pr[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("babycalcs_open: pr=%s dbname=%s\n",pr,dbname) ;
#endif

	if ((dbname == NULL) || (dbname[0] == '\0'))
	    dbname = BABYCALCS_DBNAME ;

	memset(op,0,sizeof(BABYCALCS)) ;

	rs = uc_mallocstrw(pr,-1,&op->pr) ;
	if (rs < 0)
	    goto bad0 ;

	rs = snsds(dbcomp,MAXNAMELEN,dbname,BABYCALCS_DBSUF) ;
	if (rs >= 0)
	    rs = mkpath3(dbfname,op->pr,BABYCALCS_DBDNAME,dbcomp) ;
	if (rs < 0)
	    goto bad1 ;

	rs = uc_mallocstrw(dbfname,-1,&op->dbfname) ;
	if (rs < 0)
	    goto bad1 ;

	rs = babycalcs_loadshm(op,dbname,operms) ;

#if	CF_DEBUGS
	debugprintf("babycalcs_open: babycalcs_loadshm() rs=%d\n",rs) ;
#endif

	f = ((rs == SR_NOENT) || (rs == SR_NOTSUP) || (rs == SR_NOSYS)) ;
	if (f && (op->table == NULL)) {

	    rs = babycalcs_loadtxt(op) ;

#if	CF_DEBUGS
	    debugprintf("babycalcs_open: babycalcs_loadtxt() rs=%d\n",rs) ;
#endif

	} /* end if */

	if (rs < 0)
	    goto bad2 ;

	op->magic = BABYCALCS_MAGIC ;

ret0:

#if	CF_DEBUGS
	debugprintf("babycalcs_open: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad2:
	if (op->f.txt && (op->table != NULL)) {
	    op->f.txt = FALSE ;
	    uc_free(op->table) ;
	    op->table = NULL ;
	}

	if (op->dbfname != NULL) {
	    uc_free(op->dbfname) ;
	    op->dbfname = NULL ;
	}

bad1:
	if (op->pr != NULL) {
	    uc_free(op->pr) ;
	    op->pr = NULL ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (babycalcs_open) */


int babycalcs_close(op)
BABYCALCS	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BABYCALCS_MAGIC)
	    return SR_NOTOPEN ;

	rs1 = babycalcs_mapend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->f.txt && (op->table != NULL)) {
	    op->f.txt = FALSE ;
	    rs1 = uc_free(op->table) ;
	    if (rs >= 0) rs = rs1 ;
	    op->table = NULL ;
	}

	if (op->shmname != NULL) {
	    rs1 = uc_free(op->shmname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->shmname = NULL ;
	}

	if (op->dbfname != NULL) {
	    rs1 = uc_free(op->dbfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbfname = NULL ;
	}

	if (op->pr != NULL) {
	    rs1 = uc_free(op->pr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pr = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (babycalcs_close) */


int babycalcs_check(op,daytime)
BABYCALCS	*op ;
time_t		daytime ;
{
	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BABYCALCS_MAGIC)
	    return SR_NOTOPEN ;

	rs = babycalcs_dbcheck(op,daytime) ;

	return rs ;
}
/* end subroutine (babycalcs_check) */


int babycalcs_lookup(op,datereq,rp)
BABYCALCS	*op ;
time_t		datereq ;
uint		*rp ;
{
	int	rs = SR_OK ;

	time_t	daytime = 0 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BABYCALCS_MAGIC)
	    return SR_NOTOPEN ;

	if (rp == NULL)
	    return SR_FAULT ;

	if (datereq == 0) {
	    if (daytime == 0) daytime = time(NULL) ;
	    datereq = daytime ;
	}

	rs = babycalcs_dbcheck(op,daytime) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("babycalcs_lookup: nentries=%u\n",op->nentries) ;
#endif

	if (op->f.shm) {
	    rs = babycalcs_lookshm(op,daytime,datereq,rp) ;

	} else
	    rs = babycalcs_lookproc(op,datereq,rp) ;

ret0:
	return rs ;
}
/* end subroutine (babycalcs_lookup) */


int babycalcs_info(op,bip)
BABYCALCS	*op ;
BABYCALCS_INFO	*bip ;
{
	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BABYCALCS_MAGIC)
	    return SR_NOTOPEN ;

	if (bip == NULL)
	    return SR_FAULT ;

	rs = babycalcs_dbcheck(op,0) ;
	if (rs < 0)
	    goto ret0 ;

	if (op->f.shm) {
	    rs = babycalcs_shminfo(op,bip) ;

	} else
	    memset(bip,0,sizeof(BABYCALCS_INFO)) ;

ret0:
	return rs ;
}
/* end subroutine (babycalcs_info) */


/* private subroutines */


static int babycalcs_loadshm(op,dbname,operms)
BABYCALCS	*op ;
const char	dbname[] ;
int		operms ;
{
	struct ustat	sb ;

	time_t	daytime = 0 ;

	int	rs = SR_OK ;
	int	fd = -1 ;
	int	oflags ;
	int	cl ;
	int	c = 0 ;
	int	f_needinit = FALSE ;

	const char	*postfix = BABYCALCS_SHMPOSTFIX ;
	const char	*cp ;

	char	shmname[MAXNAMELEN + 1] ;


	op->mapsize = 0 ;
	op->table = NULL ;
	if (op->pagesize == 0)
	    op->pagesize = getpagesize() ;

	cl = sfbasename(op->pr,-1,&cp) ;
	if (cl <= 0) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

	rs = mkshmname(shmname,cp,cl,postfix,-1) ;
	cl = rs ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("babycalcs_loadshm: shmname=%s\n",shmname) ;
#endif

	rs = uc_mallocstrw(shmname,cl,&op->shmname) ;
	if (rs < 0)
	    goto ret0 ;

	oflags = O_RDWR ;
	rs = uc_openshm(shmname,oflags,operms) ;
	fd = rs ;

#if	CF_DEBUGS
	debugprintf("babycalcs_loadshm: RDWR uc_openshm() rs=%d\n",rs) ;
#endif

	if (rs == SR_NOENT) {

	    oflags = (O_RDWR | O_CREAT | O_EXCL) ;
	    rs = uc_openshm(shmname,oflags,(operms & 0444)) ;
	    fd = rs ;

#if	CF_DEBUGS
	    debugprintf("babycalcs_loadshm: RDWR|CREAT uc_openshm() rs=%d\n",
	        rs) ;
#endif

	    if (rs >= 0) {
	        if (daytime == 0) daytime = time(NULL) ;
	        op->ti_lastcheck = daytime ;
	        rs = babycalcs_loadtxt(op) ;
	    }

	    if (rs >= 0) {
	        rs = babycalcs_shmwr(op,daytime,fd,operms) ;
	        f_needinit = (rs >= 0) ;
	    }

	} /* end if */

	if ((rs == SR_ACCESS) || (rs == SR_EXIST)) {
	    op->shmsize = 0 ;
	    rs = babycalcs_openshmwait(op,shmname) ;
	    fd = rs ;
	}

	if (rs < 0)
	    goto ret1 ;

/* map it */

	if ((rs >= 0) && (op->shmsize == 0)) {
	    rs = u_fstat(fd,&sb) ;
	    op->shmsize = sb.st_size ;
	}

	if (rs >= 0) {
	    rs = babycalcs_mapbegin(op,daytime,fd) ;
	    c = rs ;
	}

	if ((rs >= 0) && f_needinit) {
	    rs = babycalcs_mutexinit(op) ;

#if	CF_DEBUGS
	    debugprintf("babycalcs_loadshm: babycalcs_mutexinit() rs=%d\n",rs) ;
#endif

	    if (rs >= 0)
	        u_fchmod(fd,operms) ;

	} /* end if */

	if (rs >= 0) {
	    op->f.shm = TRUE ;
	} else
	    babycalcs_mapend(op) ;

/* close it (it stays mapped) */
ret1:
	if (fd >= 0)
	    u_close(fd) ;

	if (rs < 0) {
	    if (op->f.txt && (op->table != NULL)) {
	        uc_free(op->table) ;
	    }
	    op->table = NULL ;
	    if (op->shmname != NULL) {
	        uc_free(op->shmname) ;
	        op->shmname = NULL ;
	    }
	} /* end if */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (babycalcs_loadshm) */


static int babycalcs_mapbegin(op,daytime,fd)
BABYCALCS	*op ;
time_t		daytime ;
int		fd ;
{
	int	rs = SR_OK ;
	int	mprot ;
	int	mflags ;
	int	c = 0 ;


	if (fd < 0)
	    return SR_INVALID ;

	if (daytime == 0)
	    daytime = time(NULL) ;

	op->ti_map = daytime ;
	op->mapsize = op->shmsize ;

	mprot = PROT_READ | PROT_WRITE ;
	mflags = MAP_SHARED ;
	rs = u_mmap(NULL,(size_t) op->mapsize,mprot,mflags,
	    fd,0L,&op->mapdata) ;

#if	CF_DEBUGS
	debugprintf("babycalcs_mapbegin: u_mmap() rs=%d\n",rs) ;
	debugprintf("babycalcs_mapbegin: mapbuf(%p)\n",op->mapdata) ;
#endif

	if (rs < 0)
	    goto bad1 ;

	if (op->f.txt && (op->table != NULL)) {
	    op->f.txt = FALSE ;
	    uc_free(op->table) ;
	    op->table = NULL ;
	}

	rs = babycalcs_procmap(op,daytime) ;
	c = rs ;
	if (rs < 0)
	    goto bad2 ;

ret0:
	return (rs >= 0) ? c : rs ;

/* bad stuff */
bad2:
	op->table = NULL ;
	op->f.shm = FALSE ;
	u_munmap(op->mapdata,op->mapsize) ;

bad1:
	op->mapdata = NULL ;
	op->mapsize = 0 ;
	op->ti_map = 0 ;

bad0:
	goto ret0 ;
}
/* end subroutine (babycalcs_mapbegin) */


static int babycalcs_mapend(op)
BABYCALCS	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op->mapdata != NULL) {
	    rs1 = u_munmap(op->mapdata,op->mapsize) ;
	    if (rs >= 0) rs = rs1 ;
	    op->mapdata = NULL ;
	    op->mapsize = 0 ;
	    op->mp = NULL ;
	    op->ti_map = 0 ;
	    if (op->f.shm && (op->table != NULL)) {
	        op->f.shm = FALSE ;
	        op->table = NULL ;
	    }
	}

	return rs ;
}
/* end subroutine (babycalcs_mapend) */


static int babycalcs_procmap(op,daytime)
BABYCALCS	*op ;
time_t		daytime ;
{
	BABIESFU	*hfp ;

	int	rs = SR_OK ;
	int	c = 0 ;


#if	CF_DEBUGS
	debugprintf("babycalcs_procmap: entered\n") ;
#endif

	if (daytime == 0)
	    daytime = time(NULL) ;

	hfp = &op->hf ;
	rs = babiesfu(hfp,1,op->mapdata,op->mapsize) ;
	if (rs < 0)
	    goto ret0 ;

	rs = babycalcs_verify(op,daytime) ;
	if (rs < 0)
	    goto ret0 ;

	op->table = (BABYCALCS_ENTRY *) (op->mapdata + hfp->btoff) ;
	op->mp = (PTM *) (op->mapdata + hfp->muoff) ;
	op->nentries = hfp->btlen ;
	c = hfp->btlen ;

#if	CF_DEBUGS
	debugprintf("babycalcs_procmap: mp(%p)\n",op->mp) ;
	debugprintf("babycalcs_procmap: nentries=%u\n",op->nentries) ;
#endif

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (babycalcs_procmap) */


static int babycalcs_loadtxt(op)
BABYCALCS	*op ;
{
	BABYCALCS_ENTRY	*ep ;

	vecobj	entries ;

	int	rs ;
	int	size ;
	int	i ;
	int	n ;
	int	c = 0 ;


	op->table = NULL ;
	op->nentries = 0 ;
	size = sizeof(BABYCALCS_ENTRY) ;
	rs = vecobj_start(&entries,size,0,0) ;
	if (rs < 0)
	    goto ret0 ;

	rs = babycalcs_proctxt(op,&entries) ;
	n = rs ;

#if	CF_DEBUGS
	debugprintf("babycalcs_loadtxt: babycalcs_proctxt() rs=%d\n",rs) ;
#endif


	if ((rs == SR_NOENT) || (n == 0)) {
	    for (i = 0 ; defs[i].date > 0 ; i += 1) {
	        rs = vecobj_add(&entries,(defs + i)) ;
	        if (rs < 0)
	            break ;
	    }
	}

	if (rs < 0)
	    goto ret1 ;

	n = vecobj_count(&entries) ;

	size = (n + 1) * sizeof(BABYCALCS_ENTRY) ;
	rs = uc_malloc(size,&op->table) ;
	if (rs < 0) {
	    op->table = NULL ;
	    goto ret1 ;
	}

	for (i = 0 ; vecobj_get(&entries,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;

	    op->table[c++] = *ep ;

	} /* end for */

	op->table[c].date = 0 ;
	op->table[c].count = 0 ;
	op->nentries = c ;
	op->f.txt = TRUE ;

ret1:
	vecobj_finish(&entries) ;

ret0:

#if	CF_DEBUGS
	debugprintf("babycalcs_loadtxt: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (babycalcs_loadtxt) */


static int babycalcs_proctxt(op,tlp)
BABYCALCS	*op ;
vecobj		*tlp ;
{
	struct ustat	sb ;

	CVTDATER	cdater ;

	bfile		txtfile, *tfp = &txtfile ;

	const int	llen = LINEBUFLEN ;

	int	rs = SR_OK ;
	int	len ;
	int	ll ;
	int	c = 0 ;

	const char	*tp, *lp ;

	char	lbuf[LINEBUFLEN + 1] ;


	if (tlp == NULL)
	    return SR_FAULT ;

	if (op->dbfname == NULL)
	    return SR_BUGCHECK ;

#if	CF_DEBUGS
	debugprintf("babycalcs_proctxt: dbfname=%s\n",op->dbfname) ;
#endif

	op->f.sorted = TRUE ;
	if ((rs = cvtdater_start(&cdater,0)) >= 0) {

#if	CF_DEBUGS
	debugprintf("babycalcs_proctxt: cvtdater_start() rs=%d\n",rs) ;
#endif

	if ((rs = bopen(tfp,op->dbfname,"r",0666)) >= 0) {
	    if ((rs = bcontrol(&txtfile,BC_STAT,&sb)) >= 0) {

	op->ti_mdb = sb.st_mtime ;
	op->dbsize = sb.st_size ;
	while ((rs = breadline(&txtfile,lbuf,llen)) > 0) {
	    len = rs ;

	    if (lbuf[len - 1] == '\n') len -= 1 ;

	    lp = lbuf ;
	    ll = len ;
	    if ((tp = strnchr(lp,ll,'#')) != NULL)
	        ll = (tp - lbuf) ;

	    if ((ll == 0) || (lp[0] == '#')) continue ;

	    c += 1 ;
	    rs = babycalcs_proctxtline(op,tlp,&cdater,lp,ll) ;

#if	CF_DEBUGS
	    debugprintf("babycalcs_proctxt: "
		"babycalcs_proctxtline() rs=%d\n",
	        rs) ;
#endif

	    	    if (rs < 0) break ;
	        } /* end while */

	    } /* end if (bcontrol) */
	    bclose(&txtfile) ;
	} /* end if (file-open) */

	if ((rs >= 0) && (! op->f.sorted)) {
	    op->f.sorted = TRUE ;
	    if (c > 1)
	        vecobj_sort(tlp,cmpentry) ;	/* use heap-sort here */
	}

	cvtdater_finish(&cdater) ;
	} /* end if (cvtdater) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (babycalcs_proctxt) */


static int babycalcs_proctxtline(op,tlp,cdp,lbuf,linelen)
BABYCALCS	*op ;
vecobj		*tlp ;
CVTDATER	*cdp ;
const char	lbuf[] ;
int		linelen ;
{
	time_t	datereq ;

	uint	count ;

	int	rs = SR_OK ;
	int	ll ;
	int	cl ;
	int	c = 0 ;

	const char	*lp ;
	const char	*cp ;


	lp = lbuf ;
	ll = linelen ;
	cl = nextfield(lp,ll,&cp) ;

	if (cl <= 0)
	    goto ret0 ;

	rs = cvtdater_load(cdp,&datereq,cp,cl) ;

#if	CF_DEBUGS
	debugprintf("babycalcs_proctxtline: cvtdater_load() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	ll -= ((cp + cl) - lp) ;
	lp = (cp + cl) ;
	cl = nextfield(lp,ll,&cp) ;

	if (cl <= 0)
	    goto ret0 ;

	rs = cfdecui(cp,cl,&count) ;

#if	CF_DEBUGS
	debugprintf("babycalcs_proctxtline: cfdecui() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    BABYCALCS_ENTRY	e, *ep ;

	    int		rs1 ;
	    int		ei ;

	    c = 1 ;
	    e.date = datereq ;
	    e.count = count ;
	    rs = vecobj_add(tlp,&e) ;
	    if (rs > 0) {
	        ei = (rs - 1) ;
	        rs1 = vecobj_get(tlp,ei,&ep) ;
	        if (rs1 >= 0) {
	            if ((ep == NULL) || (e.date < ep->date))
	                op->f.sorted = FALSE ;
	        }
	    }

	} /* end if */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (babycalcs_proctxtline) */


static int babycalcs_shmwr(op,daytime,fd,operms)
BABYCALCS	*op ;
time_t		daytime ;
int		fd ;
int		operms ;
{
	BABIESFU	hf ;

	FILEBUF	babyfile ;

	uint	fileoff = 0 ;

	int	rs = SR_OK ;
	int	size ;
	int	bl ;

	char	hdrbuf[HDRBUFLEN + 1] ;


	op->shmsize = 0 ;
	if (daytime == 0)
	    daytime = time(NULL) ;

	if (op->pagesize == 0)
	    op->pagesize = getpagesize() ;

	size = (op->pagesize * 4) ;
	rs = filebuf_start(&babyfile,fd,0,size,0) ;
	if (rs < 0)
	    goto ret1 ;

/* prepare the file-header */

	memset(&hf,0,sizeof(BABIESFU)) ;

	hf.vetu[0] = BABIESFU_VERSION ;
	hf.vetu[1] = ENDIAN ;
	hf.vetu[2] = 0 ;
	hf.vetu[3] = 0 ;
	hf.dbsize = (uint) op->dbsize ;
	hf.dbtime = (uint) op->ti_mdb ;
	hf.wtime = (uint) daytime ;

/* create the file-header */

	rs = babiesfu(&hf,0,hdrbuf,HDRBUFLEN) ;
	bl = rs ;
	if (rs < 0)
	    goto ret2 ;

/* write file-header */

	if (rs >= 0) {
	    rs = filebuf_writefill(&babyfile,hdrbuf,bl) ;
	    fileoff += rs ;
	}

/* write the mutex (align up to the next 8-byte boundary) */

	if (rs >= 0) {
	    int	noff = ulceil(fileoff,8) ;
	    if (noff != fileoff) {
	        rs = filebuf_writezero(&babyfile,(noff - fileoff)) ;
	        fileoff += rs ;
	    }
	}

	hf.muoff = fileoff ;
	hf.musize = uceil(sizeof(PTM),sizeof(uint)) ;
	if (rs >= 0) {
	    rs = filebuf_writezero(&babyfile,hf.musize) ;
	    fileoff += rs ;
	}

/* write the table */

	hf.btoff = fileoff ;
	hf.btlen = op->nentries ;
	size = (op->nentries + 1) * sizeof(BABYCALCS_ENTRY) ;
	if (rs >= 0) {
	    rs = filebuf_write(&babyfile,op->table,size) ;
	    fileoff += rs ;
	}

/* write out the header -- again! */
ret2:
	filebuf_finish(&babyfile) ;

	if (rs >= 0) {

	    hf.shmsize = fileoff ;
	    u_seek(fd,0L,SEEK_SET) ;

	    rs = babiesfu(&hf,0,hdrbuf,HDRBUFLEN) ;
	    bl = rs ;
	    if (rs >= 0)
	        rs = u_write(fd,hdrbuf,bl) ;

	} /* end if */

/* set file permissions */

	if (rs >= 0) {
	    op->shmsize = fileoff ;
	    rs = u_fchmod(fd,operms) ;
	}

/* we're out of here */
ret1:
ret0:
	return (rs >= 0) ? fileoff : rs ;
}
/* end subroutine (babycalcs_shmwr) */


static int babycalcs_mutexinit(op)
BABYCALCS	*op ;
{
	BABIESFU	*hfp = &op->hf ;

	PTM	*mp ;

	PTMA	ma ;

	int	rs = SR_OK ;


#if	CF_DEBUGS
	debugprintf("babycalcs_mutexinit: muoff=%u\n",hfp->muoff) ;
#endif

	mp = (PTM *) (op->mapdata + hfp->muoff) ;

#if	CF_DEBUGS
	debugprintf("babycalcs_mutexinit: mp(%p)\n",op->mp) ;
#endif

	memset(mp,0,sizeof(PTM)) ;

	if ((rs = ptma_init(&ma)) >= 0) {

	    ptma_setpshared(&ma,PTHREAD_PROCESS_SHARED) ;

#if	CF_DEBUGS
	    debugprintf("babycalcs_mutexinit: ptm_init() size_ptm=%u\n",
	        sizeof(PTM)) ;
#endif

	    rs = ptm_init(mp,&ma) ;	/* we leave the MUTEX initialized */

	    ptma_destroy(&ma) ;
	} /* end if (mutex-lock attribute) */

	return rs ;
}
/* end subroutine (babycalcs_mutexinit) */


static int babycalcs_openshmwait(op,shmname)
BABYCALCS	*op ;
const char	shmname[] ;
{
	const int	operms = BABYCALCS_PERMS ;
	int		rs = SR_OK ;
	int		oflags = O_RDWR ;
	int		to = TO_WAITSHM ;
	int		fd = -1 ;

	while (to-- > 0) {

	    rs = uc_openshm(shmname,oflags,operms) ;
	    fd = rs ;
	    if (rs >= 0) break ;

	    if (rs != SR_ACCESS) break ;
	} /* end while */

	if ((rs < 0) && (to == 0))
	    rs = SR_TIMEDOUT ;

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (babycalcs_openshmwait) */


static int babycalcs_verify(op,daytime)
BABYCALCS	*op ;
time_t		daytime ;
{
	BABIESFU	*hfp = &op->hf ;

	uint	utime = (uint) daytime ;

	int	rs = SR_OK ;
	int	size ;
	int	f = TRUE ;


	f = f && (hfp->shmsize == op->mapsize) ;

#if	CF_DEBUGS
	debugprintf("babycalcs_verify: mapsize=%u hf.shmsize=%u f=%u\n",
	    op->mapsize,hfp->shmsize,f) ;
#endif

	if (hfp->wtime > 0)
	    f = f && (hfp->wtime <= (utime + SHIFTINT)) ;

#if	CF_DEBUGS
	{
	    char	timebuf1[TIMEBUFLEN + 1] ;
	    char	timebuf2[TIMEBUFLEN + 1] ;
	    debugprintf("babycalcs_verify: wtime=%s utime=%s f=%u\n",
	        timestr_log(((time_t) hfp->wtime),timebuf1),
	        timestr_log(((time_t) utime),timebuf2), f) ;
	}
#endif /* CF_DEBUGS */

	f = f && (hfp->muoff <= op->mapsize) ;
	size = hfp->musize ;
	if (size > 0) {
	    f = f && ((hfp->muoff + size) <= hfp->btoff) ;
	    f = f && ((hfp->muoff + size) <= op->mapsize) ;
	}

	f = f && (hfp->btoff <= op->mapsize) ;
	size = hfp->btlen * sizeof(BABYCALCS_ENTRY) ;
	f = f && ((hfp->btoff + size) <= op->mapsize) ;

/* get out */

	if (! f)
	    rs = SR_BADFMT ;

#if	CF_DEBUGS
	debugprintf("babycalcs_verify: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (babycalcs_verify) */


static int babycalcs_lookshm(op,daytime,datereq,rp)
BABYCALCS	*op ;
time_t		daytime ;
time_t		datereq ;
uint		*rp ;
{
	sigset_t	oldsigmask, newsigmask ;

	int	rs = SR_OK ;


	if (op->mapdata == NULL)
	    return SR_BUGCHECK ;

	if (op->mp == NULL)
	    return SR_BUGCHECK ;

#if	CF_DEBUGS
	debugprintf("babycalcs_lookshm: entered\n") ;
#endif

	uc_sigsetfill(&newsigmask) ;

	if ((rs = u_sigprocmask(SIG_BLOCK,&newsigmask,&oldsigmask)) >= 0) {

	    if ((rs = ptm_lock(op->mp)) >= 0) {

	        rs = babycalcs_shmaccess(op,daytime) ;

	        if (rs >= 0)
	            rs = babycalcs_lookproc(op,datereq,rp) ;

	        ptm_unlock(op->mp) ;
	    } /* end if (mutex lock) */

	    u_sigprocmask(SIG_SETMASK,&oldsigmask,NULL) ;
	} /* end if */

	return rs ;
}
/* end subroutine (babycalcs_lookshm) */


static int babycalcs_lookproc(op,datereq,rp)
BABYCALCS	*op ;
time_t		datereq ;
uint		*rp ;
{
	int	rs = SR_OK ;
	int	i ;


	if (datereq == 0)
	    datereq = time(NULL) ;

#if	CF_DEBUGS
	debugprintf("babycalcs_lookproc: nentries=%u\n",op->nentries) ;
#endif

	for (i = 0 ; i < op->nentries ; i += 1) {

	    if (datereq <= op->table[i].date)
	        break ;

	} /* end for */

	if ((i > 0) && (i >= op->nentries))
	    i -= 1 ;

#if	CF_DEBUGS
	debugprintf("babycalcs_lookproc: i=%u \n",i) ;
	debugprintf("babycalcs_lookproc: c=%u\n",op->table[i].count) ;
#endif

	babycalcs_calc(op,i,datereq,rp) ;

ret0:

#if	CF_DEBUGS
	debugprintf("babycalcs_lookproc: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (babycalcs_lookproc) */


static int babycalcs_calc(op,i,rd,rp)
BABYCALCS	*op ;
int		i ;
time_t		rd ;
uint		*rp ;
{
	time_t	bd ;

	double	x0, x1, dx ;
	double	y0, y1, dy ;
	double	xr, yr, yb ;

	uint	bc ;

	int	rs = SR_OK ;


	bd = (i > 0) ? op->table[i-1].date : 0 ;
	bc = (i > 0) ? op->table[i-1].count : 0 ;

	x0 = bd ;
	x1 = op->table[i].date ;
	dx = (x1 - x0) ;

	y0 = bc ;
	y1 = op->table[i].count ;
	dy = (y1 - y0) ;

	yb = bc ;
	xr = (rd - bd) ;
	yr = (xr * dy / dx) + yb ;

	*rp = yr ;
	return rs ;
}
/* end subroutine (babycalcs_calc) */


static int babycalcs_dbcheck(op,daytime)
BABYCALCS	*op ;
time_t		daytime ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	tint ;
	int	f = FALSE ;


#if	CF_DEBUGS
	debugprintf("babycalcs_dbcheck: entered\n") ;
#endif

	if (daytime == 0)
	    daytime = time(NULL) ;

	tint = (daytime - op->ti_lastcheck) ;
	if (tint < TO_LASTCHECK)
	    goto ret0 ;

	op->ti_lastcheck = daytime ;
	rs1 = u_stat(op->dbfname,&sb) ;
	if (rs1 < 0)
	    goto ret0 ;

	if (op->f.shm) {
	    f = (sb.st_mtime > op->hf.dbtime) ;
	    f = f || (sb.st_size != op->hf.dbsize) ;
	    if (f) {
	        rs = babycalcs_dbwait(op,daytime,&sb) ;
	        if (rs >= 0)
	            rs = babycalcs_reloadshm(op,daytime,&sb) ;
	    }
	} else {
	    f = (sb.st_mtime > op->ti_mdb) ;
	    f = f || (sb.st_size != op->dbsize) ;
	    if (f) {
	        rs = babycalcs_dbwait(op,daytime,&sb) ;
	        if (rs >= 0)
	            rs = babycalcs_reloadtxt(op,daytime,&sb) ;
	    }
	}

ret0:

#if	CF_DEBUGS
	debugprintf("babycalcs_dbcheck: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (babycalcs_dbcheck) */


static int babycalcs_shminfo(op,bip)
BABYCALCS	*op ;
BABYCALCS_INFO	*bip ;
{
	sigset_t	oldsigmask, newsigmask ;

	int	rs = SR_OK ;


#if	CF_DEBUGS
	debugprintf("babycalcs_shminfo: entered\n") ;
#endif

	if (op->mapdata == NULL)
	    return SR_BUGCHECK ;

	if (op->mp == NULL)
	    return SR_BUGCHECK ;

	uc_sigsetfill(&newsigmask) ;

	if ((rs = u_sigprocmask(SIG_BLOCK,&newsigmask,&oldsigmask)) >= 0) {

	    if ((rs = ptm_lock(op->mp)) >= 0) {

	        rs = babycalcs_lookinfo(op,bip) ;

	        ptm_unlock(op->mp) ;
	    } /* end if (mutex lock) */

	    u_sigprocmask(SIG_SETMASK,&oldsigmask,NULL) ;
	} /* end if */

	return rs ;
}
/* end subroutine (babycalcs_shminfo) */


static int babycalcs_lookinfo(op,bip)
BABYCALCS	*op ;
BABYCALCS_INFO	*bip ;
{
	uint	*hwp ;

	int	rs = SR_OK ;


	memset(bip,0,sizeof(BABYCALCS_INFO)) ;

	hwp = (uint *) (op->mapdata + BABIESFU_IDLEN) ;
	bip->wtime = hwp[babiesfuh_wtime] ;
	bip->atime = hwp[babiesfuh_atime] ;
	bip->acount = hwp[babiesfuh_acount] ;
	return rs ;
}
/* end subroutine (babycalcs_lookinfo) */


static int babycalcs_dbwait(op,daytime,sbp)
BABYCALCS	*op ;
time_t		daytime ;
struct ustat	*sbp ;
{
	struct ustat	nsb ;

	int	rs = SR_OK ;
	int	f ;


	f = ((daytime - sbp->st_mtime) >= TO_DBWAIT) ;
	if (f)
	    goto ret0 ;

	while (rs >= 0) {

	    msleep(TO_DBPOLL) ;

	    if ((rs = u_stat(op->dbfname,&nsb)) >= 0) {

	        f = (sbp->st_size == nsb.st_size) ;
	        f = f && (sbp->st_mtime == nsb.st_mtime) ;
	        f = f && ((daytime - nsb.st_mtime) >= TO_DBWAIT) ;
	        if (f)
	            break ;

	        *sbp = nsb ;
	        daytime = time(NULL) ;

	    } /* end if */

	} /* end while */

ret0:
	return rs ;
}
/* end subroutine (babycalcs_dbwait) */


static int babycalcs_reloadshm(op,daytime,sbp)
BABYCALCS	*op ;
time_t		daytime ;
struct ustat	*sbp ;
{
	sigset_t	oldsigmask, newsigmask ;

	const int	operms = BABYCALCS_PERMS ;

	int	rs = SR_OK ;
	int	fd = -1 ;
	int	oflags ;
	int	neo = op->nentries ;
	int	mapsize = op->mapsize ;
	int	mapextent ;
	int	c = 0 ;
	int	f = FALSE ;


#if	CF_DEBUGS
	debugprintf("babycalcs_reloadshm: entered\n") ;
#endif

	oflags = O_RDWR ;
	rs = uc_openshm(op->shmname,oflags,operms) ;
	fd = rs ;
	if (rs < 0)
	    goto ret0 ;

	uc_sigsetfill(&newsigmask) ;

	if ((rs = u_sigprocmask(SIG_BLOCK,&newsigmask,&oldsigmask)) >= 0) {

	    if ((rs = ptm_lock(op->mp)) >= 0) {

#if	CF_DEBUGS
	        debugprintf("babycalcs_reloadshm: _shmcheck()\n") ;
#endif

	        rs = babycalcs_shmcheck(op,sbp) ;
	        f = (rs > 0) ;
	        if ((rs >= 0) && f) {

	            rs = babycalcs_shmupdate(op,daytime,sbp,fd) ;

#if	CF_DEBUGS
	            debugprintf("babycalcs_reloadshm: _shmupdate() rs=%d\n",
			rs) ;
	            debugprintf("babycalcs_reloadshm: nentries=%d eno=%u\n",
	                op->nentries,neo) ;
#endif

	        }

	        ptm_unlock(op->mp) ;
	    } /* end if (mutex lock) */

	    u_sigprocmask(SIG_SETMASK,&oldsigmask,NULL) ;
	} /* end if (setprocmask) */

	if ((rs >= 0) && f)
	    uc_msync(op->mapdata,op->mapsize,MS_ASYNC) ;

	c = op->nentries ;
	if ((rs >= 0) && f && (c != neo)) {

	    mapextent = iceil(mapsize,op->pagesize) ;

	    if (op->shmsize > mapextent) {

#if	CF_DEBUGS
	        debugprintf("babycalcs_reloadshm: remap\n") ;
#endif

	        babycalcs_mapend(op) ;

	        rs = babycalcs_mapbegin(op,daytime,fd) ;
	        c = rs ;

	    } else {

	        op->mapsize = op->shmsize ;

	    } /* end if (SHM-segment exceeded the last page) */

	} /* end if */

	if (fd >= 0)
	    u_close(fd) ;

ret0:

#if	CF_DEBUGS
	debugprintf("babycalcs_reloadshm: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (babycalcs_reloadshm) */


static int babycalcs_shmupdate(op,daytime,sbp,fd)
BABYCALCS	*op ;
time_t		daytime ;
struct ustat	*sbp ;
int		fd ;
{
	BABYCALCS_ENTRY	*tblp = op->table ;

	uint	*hwp ;

	int	rs = SR_OK ;
	int	es = sizeof(BABYCALCS_ENTRY) ;
	int	nen ;
	int	neo = op->nentries ;
	int	tblsize ;
	int	shmsize = 0 ;
	int	f ;


#if	CF_DEBUGS
	debugprintf("babycalcs_shmupdate: entered\n") ;
#endif

	rs = babycalcs_loadtxt(op) ;
	nen = op->nentries ;
	if (rs < 0)
	    goto ret1 ;

	f = (nen != neo) ;
	if (f) {
	    tblsize = (nen * es) ;
	    f = (memcmp(tblp,op->table,tblsize) != 0) ;
	}
	if (f) {

	    rs = babycalcs_shmaddwrite(op,fd) ;
	    shmsize = rs ;
	    if (rs >= 0) {

#if	CF_DEBUGS
	        debugprintf("babycalcs_shmupdate: new shmsize=%u\n",shmsize) ;
#endif

	        op->shmsize = shmsize ;
	        hwp = (uint *) (op->mapdata + BABIESFU_IDLEN) ;
	        hwp[babiesfuh_shmsize] = shmsize ;
	        hwp[babiesfuh_dbsize] = (uint) sbp->st_size ;
	        hwp[babiesfuh_dbtime] = (uint) sbp->st_mtime ;
	        hwp[babiesfuh_wtime] = (uint) daytime ;
	        hwp[babiesfuh_btlen] = op->nentries ;

	        op->hf.shmsize = hwp[babiesfuh_shmsize] ;
	        op->hf.dbsize = hwp[babiesfuh_dbsize] ;
	        op->hf.dbtime = hwp[babiesfuh_dbtime] ;
	        op->hf.wtime = hwp[babiesfuh_wtime] ;
	        op->hf.btlen = hwp[babiesfuh_btlen] ;

	    } /* end if */

	} /* end if (update needed) */

	if (op->table != NULL) {
	    op->f.txt = FALSE ;
	    uc_free(op->table) ;
	    op->table = NULL ;
	}

ret1:
	op->table = tblp ;

ret0:

#if	CF_DEBUGS
	debugprintf("babycalcs_shmupdate: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (babycalcs_shmupdate) */


static int babycalcs_shmaddwrite(op,fd)
BABYCALCS	*op ;
int		fd ;
{
	offset_t	tbloff ;

	uint	*hwp ;

	int	rs = SR_OK ;
	int	es = sizeof(BABYCALCS_ENTRY) ;
	int	tblsize ;
	int	shmsize = 0 ;


#if	CF_DEBUGS
	debugprintf("babycalcs_shmaddwrite: nentries=%u\n",op->nentries) ;
#endif

	hwp = (uint *) (op->mapdata + BABIESFU_IDLEN) ;
	tbloff = hwp[babiesfuh_btoff] ;
	tblsize = (op->nentries + 1) * es ;

#if	CF_DEBUGS
	debugprintf("babycalcs_shmaddwrite: tbloff=%llu\n",tbloff) ;
#endif

	shmsize = tbloff ;
	rs = u_seek(fd,tbloff,SEEK_SET) ;

	if (rs >= 0) {
	    rs = u_write(fd,op->table,tblsize) ;
	    shmsize += rs ;
	}

	if (rs >= 0)
	    rs = uc_ftruncate(fd,shmsize) ;

ret0:
	return (rs >= 0) ? shmsize : rs ;
}
/* end subroutine (babycalcs_shmaddwrite) */


static int babycalcs_reloadtxt(op,daytime,sbp)
BABYCALCS	*op ;
time_t		daytime ;
struct ustat	*sbp ;
{
	int	rs = SR_OK ;


	if (op->f.txt && (op->table != NULL)) {
	    op->f.txt = FALSE ;
	    uc_free(op->table) ;
	    op->table = NULL ;
	}

	rs = babycalcs_loadtxt(op) ;

	return rs ;
}
/* end subroutine (babycalcs_reloadtxt) */


static int babycalcs_shmcheck(op,sbp)
BABYCALCS	*op ;
struct ustat	*sbp ;
{
	uint	*hwp ;

	int	rs = SR_OK ;
	int	f = FALSE ;


	hwp = (uint *) (op->mapdata + BABIESFU_IDLEN) ;
	f = (sbp->st_mtime > hwp[babiesfuh_dbtime]) ;
	f = f || (sbp->st_size != hwp[babiesfuh_dbsize]) ;
	f = f || (op->shmsize != hwp[babiesfuh_shmsize]) ;
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (babycalcs_shmcheck) */


static int babycalcs_shmaccess(op,daytime)
BABYCALCS	*op ;
time_t		daytime ;
{
	uint	*hwp ;

	int	rs = SR_OK ;


	if (op->mapdata == NULL)
	    return SR_NOANODE ;

	if (daytime == 0) daytime = time(NULL) ;

	hwp = (uint *) (op->mapdata + BABIESFU_IDLEN) ;
	hwp[babiesfuh_atime] = daytime ;
	hwp[babiesfuh_acount] += 1 ;
	return rs ;
}
/* end subroutine (babycalcs_shmaccess) */


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


static int mkshmname(shmbuf,fp,fl,dp,dl)
char		shmbuf[] ;
const char	*fp ;
const char	*dp ;
int		fl ;
int		dl ;
{
	const int	shmlen = SHMNAMELEN ;

	int	rs = SR_OK ;
	int	ml ;
	int	i = 0 ;


	if (rs >= 0) {
	    rs = storebuf_char(shmbuf,shmlen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    if (fp[0] == '/') {
		if (fl < 0) fl = strlen(fp) ;
		fp += 1 ;
	        fl -= 1 ;
	    }
	    ml = strnnlen(fp,fl,SHMPREFIXLEN) ;
	    rs = storebuf_strw(shmbuf,shmlen,i,fp,ml) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(shmbuf,shmlen,i,'$') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    ml = strnnlen(dp,dl,SHMPOSTFIXLEN) ;
	    rs = storebuf_strw(shmbuf,shmlen,i,dp,ml) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkshmname) */


static int cmpentry(e1pp,e2pp)
BABYCALCS_ENTRY	**e1pp ;
BABYCALCS_ENTRY	**e2pp ;
{
	BABYCALCS_ENTRY	*e1p, *e2p ;


	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return -1 ;

	if (*e2pp == NULL)
	    return +1 ;

	e1p = *e1pp ;
	e2p = *e2pp ;
	return (e1p->date - e2p->date) ;
}
/* end subroutine (cmpentry) */



