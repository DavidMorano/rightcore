/* calyears */

/* CALYEARS object implementation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGCUR	0		/* debug cursor operation */
#define	CF_SAFE		1		/* normal safety */
#define	CF_SAFE2	1		/* extra safety */
#define	CF_EMPTYTERM	1		/* terminate entry on empty line */
#define	CF_TMPPRNAME	1		/* put under a PRNAME in /tmp */
#define	CF_SAMECITE	0		/* same entry citation? */
#define	CF_ALREADY	1		/* do not allow duplicate results */
#define	CF_TRANSHOL	1		/* translate holidays */
#define	CF_MKDNAME	0		/* |mkdname()| */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module manages access to the various CALENDAR databases either in
	the distribution or specified by the caller.

	Implementation notes:

	= parsing a calendar file

	There are several valid forms for the date (month-day) part of
	a calendar entry.  These are:
		mm/dd		most common
		name[±ii]	name plus-or-minus increment in days
	The subroutine 'subinfo_havestart()' parses this out.


*******************************************************************************/


#define	CALYEARS_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<estrings.h>
#include	<vecobj.h>
#include	<sbuf.h>
#include	<char.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<tmtime.h>
#include	<holidays.h>
#include	<getxusername.h>
#include	<fsdir.h>
#include	<endianstr.h>
#include	<localmisc.h>

#include	"calyears.h"
#include	"cyi.h"
#include	"cyimk.h"
#include	"dayofmonth.h"


/* local defines */

#define	CALYEARS_CAL	struct calyears_cal
#define	CALYEARS_ENT	struct calyears_e
#define	CALYEARS_ELINE	struct calyears_eline
#define	CALYEARS_EFL	struct calyears_eflags
#define	CALYEARS_CFL	struct calyears_calflags
#define	CALYEARS_DBSUF	"calendar"
#define	CALYEARS_NLE	1	/* default number line entries */
#define	CALYEARS_DMODE	0777
#define	CALYEARS_DBDIR	"share/calendar"

#define	WORDER		struct worder

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#define	IDXDNAME	".calyears"
#define	IDXSUF		"cyi"

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	HOLBUFLEN	100

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	TMPVARDNAME
#define	TMPVARDNAME	"/var/tmp"
#endif

#ifndef	VCNAME
#define	VCNAME		"var"
#endif

#undef	NLINES
#define	NLINES		20

#define	CEBUFLEN	(NLINES * 3 * sizeof(int))

#define	TO_FILEMOD	(60 * 24 * 3600)
#define	TO_MKWAIT	(5 * 50)
#define	TO_CHECK	4

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern uint	hashelf(const char *,int) ;

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sibreak(const char *,int,const char *) ;
extern int	siskipwhite(const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	matocasestr(const char **,int,const char *,int) ;
extern int	matpcasestr(const char **,int,const char *,int) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	mktmpuserdir(char *,const char *,const char *,mode_t) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_log(time_t,char *) ;


/* local structures */

struct subinfo_flags {
	uint		id:1 ;
	uint		defdirs:1 ;
	uint		dom:1 ;
	uint		hols:1 ;
} ;

struct subinfo {
	IDS		id ;
	DAYOFMONTH	dom ;
	HOLIDAYS	hols ;
	vecstr		defdirs ;
	SUBINFO_FL	init, f ;
	CALYEARS	*op ;
	const char	*tmpdname ;
	const char	*tudname ;
	const char	*userhome ;
	const char	**dirnames ;
	time_t		dt ;
	int		year ;
	int		isdst ;
	int		gmtoff ;
	char		username[USERNAMELEN + 1] ;
} ;

struct calyears_calflags {
	uint		vind:1 ;		/* open-flag */
} ;

struct calyears_cal {
	const char	*dirname ;
	const char 	*calname ;		/* DB file-name */
	const char	*mapdata ;		/* DB memory-map address */
	CALYEARS_CFL	f ;
	CYI		vind ;			/* verse-index */
	time_t		ti_db ;			/* DB file modification */
	fime_t		ti_map ;		/* DB map */
	time_t		ti_lastcheck ;		/* DB last check */
	time_t		ti_vind ;		/* verse-index */
	size_t		filesize ;		/* DB file size */
	size_t		mapsize ;		/* DB map length */
	int		nentries ;		/* DB entries */
	int		cidx ;			/* ordinal */
} ;

struct calyears_eline {
	uint		loff ;
	uint		llen ;
} ;

struct calyears_eflags {
	uint		hash:1 ;
} ;

struct calyears_e {
	CALYEARS_ELINE	*lines ;
	CALYEARS_EFL	f ;
	uint		voff ;
	uint		vlen ;
	uint		hash ;
	int		e, i ;
	int		cidx ;
	uchar		m, d ;
} ;

struct calyears_citer {
	int		i, e ;
} ;

struct worder {
	CALYEARS_ELINE	*lines ;
	const char	*mp ;
	const char	*sp ;
	int		sl ;
	int		i ;
	int		nlines ;
} ;


/* forward references */

static int	calyears_dirnamescreate(CALYEARS *,const char **) ;
static int	calyears_dirnamesdestroy(CALYEARS *) ;

static int	calyears_loadbuf(CALYEARS *,char *,int,CALYEARS_ENT *) ;
static int	calyears_tmpdels(CALYEARS *) ;
static int	calyears_calscreate(CALYEARS *,SUBINFO *,cchar **) ;
static int	calyears_calscreater(CALYEARS *,SUBINFO *,cchar *,cchar **) ;
static int	calyears_calsdestroy(CALYEARS *) ;
static int	calyears_calcreate(CALYEARS *,SUBINFO *,cchar *,cchar *) ;
static int	calyears_caldestroy(CALYEARS *,CALYEARS_CAL *) ;

#ifdef	COMMENT
static int	calyears_checkupdate(CALYEARS *,time_t) ;
static int	calyears_mksysvarsi(CALYEARS *,const char *) ;
#endif

static int	calyears_resultfins(CALYEARS *,CALYEARS_CUR *) ;
static int	calyears_calcite(CALYEARS *,vecobj *,CALYEARS_CAL *,
			CALYEARS_Q *) ;
static int	calyears_mkresults(CALYEARS *,vecobj *,CALYEARS_CUR *) ;
static int	calyears_already(CALYEARS *,vecobj *,CALYEARS_ENT *) ;

#if	CF_DEBUGS && CF_DEBUGCUR
static int	calyears_debugcur(CALYEARS *,vecobj *,const char *) ;
#endif

static int	cal_open(CALYEARS_CAL *,SUBINFO *,int,cchar *,cchar *) ;
static int	cal_close(CALYEARS_CAL *) ;
static int	cal_dbloadbegin(CALYEARS_CAL *,SUBINFO *) ;
static int	cal_dbloadend(CALYEARS_CAL *) ;
static int	cal_indopen(CALYEARS_CAL *,SUBINFO *) ;
static int	cal_dbmapcreate(CALYEARS_CAL *,time_t) ;
static int	cal_dbmapdestroy(CALYEARS_CAL *) ;
static int	cal_indopenperm(CALYEARS_CAL *,SUBINFO *) ;
static int	cal_indopentmp(CALYEARS_CAL *,SUBINFO *) ;

static int	cal_idxget(CALYEARS_CAL *) ;
static int	cal_indopencheck(CALYEARS_CAL *,const char *,int,int) ;
static int	cal_mkdirs(CALYEARS_CAL *,const char *,mode_t) ;
static int	cal_audit(CALYEARS_CAL *) ;

static int	cal_indmk(CALYEARS_CAL *,SUBINFO *,cchar *,time_t) ;
static int	cal_indmkdata(CALYEARS_CAL *,SUBINFO *,cchar *,mode_t) ;
static int	cal_indclose(CALYEARS_CAL *) ;

static int	cal_loadbuf(CALYEARS_CAL *,char *,int,CALYEARS_ENT *) ;
static int	cal_mapdata(CALYEARS_CAL *,cchar **) ;

#ifdef	COMMENT
static int	cal_idxset(CALYEARS_CAL *,int) ;
#endif

static int	subinfo_start(SUBINFO *,CALYEARS *,time_t) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_ids(SUBINFO *) ;
static int	subinfo_username(SUBINFO *) ;
static int	subinfo_mkdirnames(SUBINFO *) ;
static int	subinfo_havedir(SUBINFO *,char *) ;
static int	subinfo_loadnames(SUBINFO *,vecstr *,const char *) ;
static int	subinfo_havestart(SUBINFO *,CALYEARS_QUERY *,cchar *,int) ;
static int	subinfo_year(SUBINFO *) ;
static int	subinfo_mkday(SUBINFO *,int,const char *,int) ;
static int	subinfo_transhol(SUBINFO *,CALYEARS_CITE *,cchar *,int) ;
static int	subinfo_checkdname(SUBINFO *,const char *) ;
static int	subinfo_regacc(SUBINFO *,cchar *,int) ;

#ifdef	COMMENT
static int	subinfo_tmpuserdir(SUBINFO *) ;
#endif

static int	entry_start(CALYEARS_ENT *,CALYEARS_CITE *,uint,int) ;
static int	entry_setidx(CALYEARS_ENT *,int) ;
static int	entry_add(CALYEARS_ENT *,uint,int) ;
static int	entry_finish(CALYEARS_ENT *) ;
static int	entry_mkhash(CALYEARS_ENT *,CALYEARS *) ;
static int	entry_sethash(CALYEARS_ENT *,uint) ;
static int	entry_samehash(CALYEARS_ENT *,CALYEARS *,CALYEARS_ENT *) ;
static int	entry_same(CALYEARS_ENT *,CALYEARS *,CALYEARS_ENT *) ;
static int	entry_loadbuf(CALYEARS_ENT *,char *,int,cchar *) ;

#if	CF_SAMECITE
static int	entry_samecite(CALYEARS_ENT *,CALYEARS *,CALYEARS_ENT *) ;
#endif

static int	mkbve_start(CYIMK_ENT *,SUBINFO *,CALYEARS_ENT *) ;
static int	mkbve_finish(CYIMK_ENT *) ;

static int	worder_start(WORDER *,CALYEARS *,CALYEARS_ENT *) ;
static int	worder_finish(WORDER *) ;
static int	worder_get(WORDER *,const char **) ;

static int	isempty(const char *,int) ;

static int	mkmonth(const char *,int) ;
static int	dayofmonth_mkday(DAYOFMONTH *,uint,const char *,int) ;

#if	CF_MKDNAME
static int	mkdname(cchar *,mode_t) ;
#endif

static int	vrcmp(const void *,const void *) ;


/* exported variables */

CALYEARS_OBJ	calyears = {
	"calyears",
	sizeof(CALYEARS),
	sizeof(CALYEARS_CUR)
} ;


/* local variables */

static const char	*days[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", NULL
} ;

static const char	*daytypes[] = {
	"First", "Second", "Third", "Fourth", "Fifth", "Last", NULL
} ;

enum wdays {
	wday_sunday,
	wday_monday,
	wday_tuesday,
	wday_wednesday,
	wday_thursday,
	wday_friday,
	wday_saturday,
	wday_overlast
} ;

enum tdays {
	tday_first,
	tday_second,
	tday_third,
	tday_fourth,
	tday_fifth,
	tday_last,
	tday_overlast
} ;


/* exported subroutines */


int calyears_open(CALYEARS *op,cchar pr[],cchar *dirnames[],cchar *calnames[])
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif
	if (pr == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("calyears_open: pr=%s\n",pr) ;
#endif

	if (pr[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(CALYEARS)) ;
	op->pr = pr ;

	if (op->tmpdname == NULL) op->tmpdname = getenv(VARTMPDNAME) ;
	if (op->tmpdname == NULL) op->tmpdname = TMPDNAME ;

	if ((rs = calyears_dirnamescreate(op,dirnames)) >= 0) {
	    if ((rs = vecstr_start(&op->tmpfiles,2,0)) >= 0) {
	        const int	vo = VECHAND_OSTATIONARY ;
	        if ((rs = vechand_start(&op->cals,2,vo)) >= 0) {
	            SUBINFO	si, *sip = &si ;
	            if ((rs = subinfo_start(sip,op,0)) >= 0) {

	                if ((rs = calyears_calscreate(op,sip,calnames)) >= 0) {
	                    c = rs ;
	                    op->nentries = c ;
	                    op->magic = CALYEARS_MAGIC ;
	                }

	                rs1 = subinfo_finish(sip) ;
			if (rs >= 0) rs = rs1 ;
	            } /* end if */
	            if (rs < 0) {
	                calyears_calsdestroy(op) ;
	                vechand_finish(&op->cals) ;
	            }
	        } /* end if (cals) */
	        if (rs < 0) {
	            calyears_tmpdels(op) ;
	            vecstr_finish(&op->tmpfiles) ;
	        }
	    } /* end if (tmpfiles) */
	    if (rs < 0)
	        calyears_dirnamesdestroy(op) ;
	} /* end if (dirnames) */

#if	CF_DEBUGS
	debugprintf("calyears_open: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (calyears_open) */


/* free up the entire vector string data structure object */
int calyears_close(CALYEARS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != CALYEARS_MAGIC) return SR_NOTOPEN ;
#endif

	rs1 = calyears_calsdestroy(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&op->cals) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = calyears_tmpdels(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecstr_finish(&op->tmpfiles) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = calyears_dirnamesdestroy(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("calyears_close: ret rs=%d\n",rs) ;
#endif

	op->nentries = 0 ;
	op->magic = 0 ;
	return rs ;
}
/* end subroutine (calyears_close) */


int calyears_count(CALYEARS *op)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != CALYEARS_MAGIC) return SR_NOTOPEN ;
#endif

	rs = op->nentries ;

	return rs ;
}
/* end subroutine (calyears_count) */


int calyears_audit(CALYEARS *op)
{
	CALYEARS_CAL	*calp ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != CALYEARS_MAGIC) return SR_NOTOPEN ;
#endif

	for (i = 0 ; vechand_get(&op->cals,i,&calp) >= 0 ; i += 1) {
	    if (calp == NULL) continue ;
	    c += 1 ;
	    rs = cal_audit(calp) ;
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (calyears_audit) */


int calyears_curbegin(CALYEARS *op,CALYEARS_CUR *curp)
{

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != CALYEARS_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;

	memset(curp,0,sizeof(CALYEARS_CUR)) ;

	op->ncursors += 1 ;

	curp->i = -1 ;
	curp->magic = CALYEARS_MAGIC ;
	return SR_OK ;
}
/* end subroutine (calyears_curbegin) */


int calyears_curend(CALYEARS *op,CALYEARS_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != CALYEARS_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;

	if (curp->magic != CALYEARS_MAGIC) return SR_NOTOPEN ;

	if (curp->results != NULL) {
	    rs1 = calyears_resultfins(op,curp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(curp->results) ;
	    if (rs >= 0) rs = rs1 ;
	    curp->results = NULL ;
	}

	if (op->ncursors > 0)
	    op->ncursors -= 1 ;

	curp->magic = 0 ;
	return rs ;
}
/* end subroutine (calyears_curend) */


static int calyears_resultfins(CALYEARS *op,CALYEARS_CUR *curp)
{
	CALYEARS_ENT	*ep = (CALYEARS_ENT *) curp->results ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep != NULL) {
	    int	i ;
	    for (i = 0 ; i < curp->nresults ; i += 1) {
	        rs1 = entry_finish(ep + i) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end for */
	} /* end if (non-null) */

	return rs ;
}
/* end subroutine (calyears_resultfins) */


int calyears_lookcite(CALYEARS *op,CALYEARS_CUR *curp,CALYEARS_QUERY *qvp)
{
	CALYEARS_CAL	*calp ;
	vecobj		res ;
	int		rs ;
	int		opts ;
	int		size ;
	int		c = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != CALYEARS_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;
	if (qvp == NULL) return SR_FAULT ;

	if (curp->magic != CALYEARS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("calyears_lookcite: q=(%d,%d,%d)\n",
	    qvp->y,qvp->m,qvp->d) ;
#endif

	if (curp->results != NULL) {
	    calyears_resultfins(op,curp) ;
	    uc_free(curp->results) ;
	    curp->results = NULL ;
	}

	opts = 0 ;
	opts |= VECOBJ_OORDERED ;
	opts |= VECOBJ_OSTATIONARY ;
	size = sizeof(CALYEARS_ENT) ;
	if ((rs = vecobj_start(&res,size,0,opts)) >= 0) {
	    int	i ;
	    for (i = 0 ; vechand_get(&op->cals,i,&calp) >= 0 ; i += 1) {
	        if (calp == NULL) continue ;
	        rs = calyears_calcite(op,&res,calp,qvp) ;
	        c += rs ;
#if	CF_DEBUGS
	        debugprintf("calyears_lookcite: i=%u "
	            "calyears_calcite() rs=%d\n",
	            i,rs) ;
#endif
	        if (rs < 0) break ;
	    } /* end for */
	    if (rs >= 0) {
	        rs = calyears_mkresults(op,&res,curp) ;
#if	CF_DEBUGS
	        debugprintf("calyears_lookcite: calyears_mkresults() rs=%d\n",
	            rs) ;
#endif
	    }
	    if ((rs < 0) || (c > 0)) {
	        CALYEARS_ENT	*ep ;
	        for (i = 0 ; vecobj_get(&res,i,&ep) >= 0 ; i += 1) {
	            if (ep == NULL) continue ;
	            entry_finish(ep) ;
	        }
	    }
	    vecobj_finish(&res) ;
	} /* end if (res) */

#if	CF_DEBUGS
	debugprintf("calyears_lookcite: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (calyears_lookcite) */


int calyears_read(CALYEARS *op,CALYEARS_CUR *curp,CALYEARS_CITE *qvp,
		char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		len = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != CALYEARS_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;
	if (qvp == NULL) return SR_FAULT ;

	if (curp->magic != CALYEARS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("calyears_read: ent\n") ;
#endif

	if (curp->results != NULL) {
	    const int	i = curp->i ;
#if	CF_DEBUGS
	    debugprintf("calyears_read: c_i=%d\n",i) ;
#endif
	    if ((i >= 0) && (i < curp->nresults)) {
	        CALYEARS_ENT	*ep = (CALYEARS_ENT *) curp->results ;
	        qvp->m = ep->m ;
	        qvp->d = ep->d ;
	        if (rbuf != NULL) {
#if	CF_DEBUGS
	            debugprintf("calyears_read: calyears_loadbuf()\n") ;
#endif
	            rs = calyears_loadbuf(op,rbuf,rlen,(ep+1)) ;
	            len = rs ;
#if	CF_DEBUGS
	            debugprintf("calyears_read: calyears_loadbuf() rs=%d\n",
	                rs) ;
#endif
	        } /* end if */
	        if (rs >= 0)
	            curp->i = (i + 1) ;
	    } else
	        rs = SR_NOTFOUND ;
	} else
	    rs = SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("calyears_read: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (calyears_read) */


/* ARGSUSED */
int calyears_check(CALYEARS *op,time_t dt)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != CALYEARS_MAGIC) return SR_NOTOPEN ;
#endif

#ifdef	COMMENT
	rs = calyears_checkupdate(op,dt) ;
#endif

	return rs ;
}
/* end subroutine (calyears_check) */


/* private subroutines */


static int calyears_dirnamescreate(CALYEARS *op,cchar **dirnames)
{
	int		rs = SR_OK ;

	if (dirnames != NULL) {
	    int		i ;
	    int		strsize = 1 ;
	    int		size ;
	    void	*p ;
	    for (i = 0 ; dirnames[i] != NULL ; i += 1) {
	        strsize += (strlen(dirnames[i]) + 1) ;
	    } /* end if */
	    size = (i + 1) * sizeof(char *) ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        op->dirnames = (char **) p ;
	        if ((rs = uc_malloc(strsize,&p)) >= 0) {
	            char	*bp = p ;
	            op->dirstrtab = p ;
	            *bp++ = '\0' ;
	            for (i = 0 ; dirnames[i] != NULL ; i += 1) {
	                op->dirnames[i] = bp ;
	                bp = strwcpy(bp,dirnames[i],-1) + 1 ;
	            } /* end for */
	            op->dirnames[i] = NULL ;
	        } /* end if (memory-allocation) */
	        if (rs < 0) {
	            uc_free(op->dirnames) ;
	            op->dirnames = NULL ;
	        }
	    } /* end if (memory-allocation) */
	} /* end if (non-null) */

	return rs ;
}
/* end subroutine (calyears_dirnamescreate) */


static int calyears_dirnamesdestroy(CALYEARS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->dirnames != NULL) {
	    rs1 = uc_free(op->dirnames) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dirnames = NULL ;
	}

	if (op->dirstrtab != NULL) {
	    rs1 = uc_free(op->dirstrtab) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dirstrtab = NULL ;
	}

	return rs ;
}
/* end subroutine (calyears_dirnamesdestroy) */


static int calyears_calcite(CALYEARS *op,vecobj *rlp,CALYEARS_CAL *calp,
		CALYEARS_QUERY *qvp)
{
	CYI		*cip = &calp->vind ;
	CYI_CUR		ccur ;
	CYI_QUERY	cq ;
	CYI_ENT		ce ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = cal_idxget(calp)) >= 0) {
	    int	cidx = rs ;
#if	CF_DEBUGS
	    debugprintf("calyears_calcite: cidx=%d\n",cidx) ;
#endif
	    memset(&cq,0,sizeof(CYI_QUERY)) ;
	    cq.y = qvp->y ;
	    cq.m = qvp->m ;
	    cq.d = qvp->d ;
	    if ((rs = cyi_curbegin(cip,&ccur)) >= 0) {
	        if ((rs = cyi_lookcite(cip,&ccur,&cq)) >= 0) {
	            CALYEARS_ENT	e ;
	            uint		loff ;
	            const int		celen = CEBUFLEN ;
	            int			llen ;
	            int			f_ent = FALSE ;
	            int			f_already = FALSE ;
	            char		cebuf[CEBUFLEN + 1] ;

	            while (rs >= 0) {

#if	CF_DEBUGS && CF_DEBUGCUR
	                debugprintf("calyears_calcite: cyi_read() c=%u\n",c) ;
	                calyears_debugcur(op,rlp,"before cyi_read") ;
#endif

	                rs1 = cyi_read(cip,&ccur,&ce,cebuf,celen) ;

#if	CF_DEBUGS && CF_DEBUGCUR
	                debugprintf("calyears_calcite: cyi_read() rs1=%d\n",
	                    rs1) ;
	                calyears_debugcur(op,rlp,"after cyi_read") ;
#endif

	                if ((rs1 == SR_NOTFOUND) || (rs1 == 0)) break ;
	                rs = rs1 ;
	                if (rs < 0) break ;

	                if (rs1 > 0) {
	                    int	n = 0 ;
	                    int	i ;

	                    for (i = 0 ; i < ce.nlines ; i += 1) {
	                        loff = ce.lines[i].loff ;
	                        llen = (int) ce.lines[i].llen ;

#if	CF_DEBUGS
	                        debugprintf("calyears_calcite: "
	                            "i=%u loff=%u llen=%u\n",
	                            i,loff,llen) ;
#endif

	                        n += 1 ;
	                        if (! f_ent) {
	                            uint	lo = loff ;
	                            int		ll = llen ;
	                            if ((rs = entry_start(&e,qvp,lo,ll)) >= 0) {
	                                f_ent = TRUE ;
	                                entry_sethash(&e,ce.hash) ;
	                                rs = entry_setidx(&e,cidx) ;
	                            }
	                        } else {
	                            rs = entry_add(&e,loff,llen) ;
	                        }
	                    } /* end for */

	                    if ((rs >= 0) && (n > 0) && f_ent) {
	                        c += 1 ;

#if	CF_ALREADY
	                        rs = calyears_already(op,rlp,&e) ;
	                        f_already = (rs > 0) ;
#endif

	                        f_ent = FALSE ;
	                        if ((rs >= 0) && (! f_already)) {
	                            rs = vecobj_add(rlp,&e) ;
	                        } else
	                            entry_finish(&e) ;
	                    }

	                } /* end if */

	            } /* end while */

	            if (f_ent) {
	                f_ent = FALSE ;
	                entry_finish(&e) ;
	            }

	        } else if (rs == SR_NOTFOUND) {
	            rs = SR_OK ;
		}
	        cyi_curend(cip,&ccur) ;
	    } /* end if (cursor) */
	} /* end if (cal_idxget) */

#if	CF_DEBUGS
	debugprintf("calyears_calcite: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (calyears_calcite) */


static int calyears_already(CALYEARS *op,vecobj *rlp,CALYEARS_ENT *ep)
{
	CALYEARS_ENT	*oep ;
	int		rs = SR_OK ;
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; vecobj_get(rlp,i,&oep) >= 0 ; i += 1) {
	    if (oep != NULL) {
	        if ((rs = entry_samehash(ep,op,oep)) > 0) {
	            rs = entry_same(ep,op,oep) ;
	    	    f = (rs > 0) ; /* same? */
	    	    if (f) break ;
		}
	    }
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (calyears_already) */


static int calyears_mkresults(CALYEARS *op,vecobj *rlp,CALYEARS_CUR *curp)
{
	int		rs = SR_OK ;
	int		n ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("calyears_mkresults: ent\n") ;
#endif

	vecobj_sort(rlp,vrcmp) ; /* sort results in ascending order */

	if ((n = vecobj_count(rlp)) > 0) {
	    CALYEARS_ENT	*rp ;
	    CALYEARS_ENT	*ep ;
	    const int		size = (n * sizeof(CALYEARS_ENT)) ;
	    if ((rs = uc_malloc(size,&rp)) >= 0) {
	        int	i ;
		for (i = 0 ; vecobj_get(rlp,i,&ep) >= 0 ; i += 1) {
	    	    if (ep == NULL) continue ;
#if	CF_DEBUGS
	            {
	                CALYEARS_ELINE	*lines = ep->lines ;
	                int	j ;
	                if (lines != NULL) {
	                    for (j = 0 ; j < ep->i ; j += 1) {
	                        debugprintf("calyears_mkresults: "
	                            "i=%u j=%u loff=%u llen=%u\n",
	                            i,j,lines[j].loff,lines[j].llen) ;
	                    }
	                }
	            }
#endif /* CF_DEBUGS */
	            rp[c++] = *ep ;
	            vecobj_del(rlp,i) ; /* entries are stationary */
	        } /* end for */
	        if (rs >= 0) {
	            curp->results = rp ;
	            curp->nresults = c ;
	            curp->i = 0 ;
	        } else
	            uc_free(rp) ;
	    } /* end if (m-a) */
	} /* end if (greater-than-zero) */

#if	CF_DEBUGS
	debugprintf("calyears_mkresults: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (calyears_mkresults) */


static int calyears_tmpdels(CALYEARS *op)
{
	int		i ;
	cchar		*sp ;

	for (i = 0 ; vecstr_get(&op->tmpfiles,i,&sp) >= 0 ; i += 1) {
	    if (sp != NULL) {
	        if (sp[0] != '\0') u_unlink(sp) ;
	    }
	} /* end for */

	return SR_OK ;
}
/* end subroutine (calyears_tmpdels) */


static int calyears_calscreate(CALYEARS *op,SUBINFO *sip,cchar *calnames[])
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	**dirnames = sip->dirnames ;

	if (dirnames != NULL) {
	    int		i ;
	    for (i = 0 ; dirnames[i] != NULL ; i += 1) {
		cchar	*dn = dirnames[i] ;
	        if (dn[0] != '\0') {
	            rs = calyears_calscreater(op,sip,dn,calnames) ;
	            c += rs ;
		}
	        if (rs < 0) break ;
	    } /* end for (dirnames) */
	} /* end if (dirnames) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (calyears_calscreate) */


static int calyears_calscreater(CALYEARS *op,SUBINFO *sip,cchar *dirname,
		cchar *calnames[])
{
	vecstr		cals ;
	int		rs = SR_OK ;
	int		c = 0 ;
	int		f_search = FALSE ;
	const char	**names = NULL ;

#if	CF_DEBUGS
	debugprintf("calyears_calscreater: dirname=%s\n",dirname) ;
#endif

	if (calnames == NULL) {
	    if ((rs = vecstr_start(&cals,1,0)) >= 0) {
	        f_search = TRUE ;
	        if ((rs = subinfo_loadnames(sip,&cals,dirname)) > 0) {
		    cchar	**npp ;
	            if ((rs = vecstr_getvec(&cals,&npp)) >= 0) {
	                names = npp ;
		    }
	        } /* end if (subinfo_loadnames) */
	    } /* end if (vecstr_start) */
	} else {
	    names = calnames ;
	}

#if	CF_DEBUGS
	debugprintf("calyears_calscreater: mid rs=%d n=%d\n",rs,n) ;
#endif

	if (rs >= 0)
	    rs = subinfo_ids(sip) ;

	if ((rs >= 0) && (names != NULL)) {
	    int		j ;
	    for (j = 0 ; names[j] != NULL ; j += 1) {
	        if (names[j][0] != '\0') {
	            rs = calyears_calcreate(op,sip,dirname,names[j]) ;
	            c += rs ;
		}
	        if (rs < 0) break ;
	    } /* end for (calnames) */
	} /* end if */

	if (f_search)
	    vecstr_finish(&cals) ;

#if	CF_DEBUGS
	debugprintf("calyears_calscreater: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (calyears_calscreater) */


static int calyears_calsdestroy(CALYEARS *op)
{
	CALYEARS_CAL	*calp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vechand_get(&op->cals,i,&calp) >= 0 ; i += 1) {
	    if (calp != NULL) {
	        rs1 = calyears_caldestroy(op,calp) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (calyears_calsdestroy) */


static int calyears_calcreate(CALYEARS *op,SUBINFO *sip,cchar *dn,cchar *cn)
{
	const int	nlen = MAXNAMELEN ;
	int		rs ;
	int		f = FALSE ;
	const char	*suf = CALYEARS_DBSUF ;
	char		nbuf[MAXNAMELEN + 1] ;

	if ((rs = snsds(nbuf,nlen,cn,suf)) >= 0) {
	    char	tbuf[MAXPATHLEN + 1] ;
	    if ((rs = mkpath2(tbuf,dn,nbuf)) >= 0) {
		if ((rs = subinfo_regacc(sip,tbuf,R_OK)) > 0) {
		    CALYEARS_CAL	*calp ;
		    const int		size = sizeof(CALYEARS_CAL) ;
		    f = TRUE ;
	    	    if ((rs = uc_malloc(size,&calp)) >= 0) {
			vechand	*clp = &op->cals ;
	        	if ((rs = vechand_add(clp,calp)) >= 0) {
	            	    const int	cidx = rs ;
	            	    if ((rs = cal_open(calp,sip,cidx,dn,cn)) >= 0) {
	                	f = TRUE ;
	            	    }
	            	    if (rs < 0)
	 			vechand_del(clp,cidx) ;
	        	} /* end if */
	        	if (rs < 0)
		            uc_free(calp) ;
	    	    } /* end if (m-a) */
		} /* end if (subinfo_regacc) */
	    } /* end if (mkpath) */
	} /* end if (snsds) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (calyears_calcreate) */


static int calyears_caldestroy(CALYEARS *op,CALYEARS_CAL *calp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = cal_close(calp) ;
	if (rs >= 0) rs = rs1 ;

	if ((rs1 = vechand_ent(&op->cals,calp)) >= 0) {
	    rs1 = vechand_del(&op->cals,rs1) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = uc_free(calp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (calyears_caldestroy) */


#ifdef	COMMENT
static int calyears_checkupdate(op,dt)
CALYEARS	*op ;
time_t		dt ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		to = TO_CHECK ;
	int		f = FALSE ;

	if (op->ncursors > 0)
	    goto ret0 ;

	if (dt <= 0)
	    dt = time(NULL) ;

	if ((dt - op->ti_lastcheck) >= to) {
	    struct ustat	sb ;
	    op->ti_lastcheck = dt ;
	    if ((rs1 = u_stat(op->dbfname,&sb)) >= 0) {
	        if ((sb.st_mtime > op->ti_db) || (sb.st_mtime > op->ti_map)) {
	            SUBINFO	si ;

	            f = TRUE ;
	            calyears_dbloadend(op) ;

	            if ((rs = subinfo_start(&si,op,0)) >= 0) {

	                rs = calyears_dbloadbegin(op,&si) ;

	                subinfo_finish(&si) ;
	            } /* end if */

	        } /* end if (update) */
	    } /* end if (stat) */
	} /* end if (time-out) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (calyears_checkupdate) */
#endif /* COMMENT */


static int calyears_loadbuf(CALYEARS *op,char rbuf[],int rlen,CALYEARS_ENT *ep)
{
	CALYEARS_CAL	*calp ;
	int		rs ;
	int		cidx = ep->cidx ;

#if	CF_DEBUGS
	debugprintf("calyears_loadbuf: cidx=%d\n",cidx) ;
#endif

	if ((rs = vechand_get(&op->cals,cidx,&calp)) >= 0) {
	    rs = cal_loadbuf(calp,rbuf,rlen,ep) ;
	}

#if	CF_DEBUGS
	debugprintf("calyears_loadbuf: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (calyears_loadbuf) */


static int cal_open(calp,sip,cidx,dirname,calname)
CALYEARS_CAL	*calp ;
SUBINFO		*sip ;
int		cidx ;
const char	dirname[] ;
const char	calname[] ;
{
	int		rs ;
	const char	*cp ;

	memset(calp,0,sizeof(CALYEARS_CAL)) ;
	calp->cidx = cidx ;

	if ((rs = uc_mallocstrw(dirname,-1,&cp)) >= 0) {
	    calp->dirname = cp ;
	    if ((rs = uc_mallocstrw(calname,-1,&cp)) >= 0) {
	        calp->calname = cp ;
	        rs = cal_dbloadbegin(calp,sip) ;
	        if (rs < 0) {
	            uc_free(calp->calname) ;
	            calp->calname = NULL ;
	        }
	    }
	    if (rs < 0) {
	        uc_free(calp->dirname) ;
	        calp->dirname = NULL ;
	    }
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("cal_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cal_open) */


static int cal_close(calp)
CALYEARS_CAL	*calp ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = cal_dbloadend(calp) ;
	if (rs >= 0) rs = rs1 ;

	if (calp->calname != NULL) {
	    rs1 = uc_free(calp->calname) ;
	    if (rs >= 0) rs = rs1 ;
	    calp->calname = NULL ;
	}

	if (calp->dirname != NULL) {
	    rs1 = uc_free(calp->dirname) ;
	    if (rs >= 0) rs = rs1 ;
	    calp->dirname = NULL ;
	}

	return rs ;
}
/* end subroutine (cal_close) */


static int cal_dbloadbegin(CALYEARS_CAL *calp,SUBINFO *sip)
{
	int		rs ;

	if ((rs = cal_dbmapcreate(calp,sip->dt)) >= 0) {
	    rs = cal_indopen(calp,sip) ;
	    if (rs < 0)
	        cal_dbmapdestroy(calp) ;
	} /* end if (cal_dbmapcreate) */

	return rs ;
}
/* end subroutine (cal_dbloadbegin) */


static int cal_dbloadend(CALYEARS_CAL *calp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = cal_indclose(calp) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = cal_dbmapdestroy(calp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (cal_dbloadend) */


static int cal_dbmapcreate(CALYEARS_CAL *calp,time_t dt)
{
	const int	nlen = MAXNAMELEN ;
	int		rs ;
	const char	*suf = CALYEARS_DBSUF ;
	char		nbuf[MAXNAMELEN + 1] ;

	if ((rs = snsds(nbuf,nlen,calp->calname,suf)) >= 0) {
	    char	dbfname[MAXPATHLEN + 1] ;
	    if ((rs = mkpath2(dbfname,calp->dirname,nbuf)) >= 0) {
	        if ((rs = u_open(dbfname,O_RDONLY,0666)) >= 0) {
	            struct ustat	sb ;
	            const int		fd = rs ;
	            if ((rs = u_fstat(fd,&sb)) >= 0) {
	                if (S_ISREG(sb.st_mode)) {
	                    calp->filesize = sb.st_size ;
	                    calp->ti_db = sb.st_mtime ;
	                    if (sb.st_size <= INT_MAX) {
	                        size_t	ms = (size_t) calp->filesize ;
	                        int	mp = PROT_READ ;
	                        int	mf = MAP_SHARED ;
				void	*n = NULL ;
	                        void	*md ;
	    			if ((rs = u_mmap(n,ms,mp,mf,fd,0L,&md)) >= 0) {
	        		    calp->mapdata = md ;
	        		    calp->mapsize = calp->filesize ;
	        		    calp->ti_map = dt ;
	        		    calp->ti_lastcheck = dt ;
	    			} /* end if (u_mmap) */
	                    } else
	                        rs = SR_TOOBIG ;
	                } else
	                    rs = SR_NOTSUP ;
	            } /* end if (stat) */
	            u_close(fd) ;
	        } /* end if (file) */
	    } /* end if (mkpath) */
	} /* end if (snsds) */

	return rs ;
}
/* end subroutine (cal_dbmapcreate) */


static int cal_dbmapdestroy(calp)
CALYEARS_CAL	*calp ;
{
	int		rs = SR_OK ;

	if (calp->mapdata != NULL) {
	    rs = u_munmap(calp->mapdata,calp->mapsize) ;
	    calp->mapdata = NULL ;
	    calp->mapsize = 0 ;
	}

	return rs ;
}
/* end subroutine (cal_dbmapdestroy) */


static int cal_indopen(CALYEARS_CAL *calp,SUBINFO *sip)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("cal_indopen: ent\n") ;
#endif

	rs = subinfo_year(sip) ; /* the current year is needed later */
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("cal_indopen: year=%u\n",sip->year) ;
#endif

	rs = cal_indopenperm(calp,sip) ;

#if	CF_DEBUGS
	debugprintf("cal_indopen: cal_indopenperm() rs=%d\n",rs) ;
#endif

	{
	    int	f = FALSE ;
	    f = f || (rs == SR_ACCESS) ;
	    f = f || (rs == SR_STALE) ;
	    f = f || (rs == SR_NOTDIR) ;
	    f = f || (rs == SR_ROFS) ;
	    if (f) {
	        rs = cal_indopentmp(calp,sip) ;
#if	CF_DEBUGS
	        debugprintf("cal_indopen: cal_indopentmp() rs=%d\n",rs) ;
#endif
	    }
	} /* end block */

ret0:

#if	CF_DEBUGS
	debugprintf("cal_indopen: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cal_indopen) */


static int cal_indopenperm(calp,sip)
CALYEARS_CAL	*calp ;
SUBINFO		*sip ;
{
	int		rs ;
	int		year = sip->year ;
	int		f_search = FALSE ;
	int		f_mkind = FALSE ;
	const char	*idxdname = IDXDNAME ;
	char		idname[MAXNAMELEN + 1] ;

	rs = mkpath2(idname,calp->dirname,idxdname) ;
	if (rs < 0)
	    goto ret0 ;

try:

#if	CF_DEBUGS
	debugprintf("cal_indopenperm: try idname=%s\n",idname) ;
#endif

	rs = cal_indopencheck(calp,idname,year,f_search) ;

#if	CF_DEBUGS
	debugprintf("cal_indopenperm: 1 cal_indopencheck() rs=%d\n",rs) ;
#endif

	if (rs == SR_NOTDIR) {
	    f_mkind = TRUE ;
	    rs = cal_mkdirs(calp,idname,CALYEARS_DMODE) ;

#if	CF_DEBUGS
	    debugprintf("cal_indopenperm: cal_mkdirs() rs=%d\n",rs) ;
#endif

	}

	f_mkind = f_mkind || (rs == SR_NOENT) ;
	f_mkind = f_mkind || (rs == SR_STALE) ;
	f_mkind = f_mkind || (rs == SR_NOCSI) ; /* zero sized file */
	if (f_mkind) {
	    rs = cal_indmk(calp,sip,idname,sip->dt) ;

#if	CF_DEBUGS
	    debugprintf("cal_indopenperm: cal_indmk() rs=%d\n",rs) ;
#endif

	    if (rs >= 0)
	        rs = cal_indopencheck(calp,idname,year,f_search) ;

#if	CF_DEBUGS
	    debugprintf("cal_indopenperm: 2 cal_indopencheck() rs=%d\n",rs) ;
#endif

	} /* end if */

ret0:

#if	CF_DEBUGS
	debugprintf("cal_indopenperm: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_mkind : rs ;
}
/* end subroutine (cal_indopenperm) */


static int cal_indopentmp(calp,sip)
CALYEARS_CAL	*calp ;
SUBINFO		*sip ;
{
	CALYEARS	*op = sip->op ;
	int		rs ;
	int		year = sip->year ;
	int		f_search = TRUE ;
	int		f_mkind = FALSE ;
	const char	*idxdname = IDXDNAME ;
	char		idname[MAXPATHLEN + 1] ;

#if	CF_TMPPRNAME
	{
	    const char	*prname ;
	    if ((rs = sfbasename(op->pr,-1,&prname)) > 0) {
	        rs = mkpath3(idname,op->tmpdname,prname,idxdname) ;
	    }
	}
#else /* CF_TMPPRNAME */
	rs = mkpath2(idname,op->tmpdname,idxdname) ;
#endif /* CF_TMPPRNAME */

	if (rs < 0)
	    goto ret0 ;

try:
	rs = cal_indopencheck(calp,idname,year,f_search) ;

#if	CF_DEBUGS
	debugprintf("cal_indopentmp: 1 cal_indopencheck() rs=%d\n",rs) ;
#endif

	if (rs == SR_NOTDIR) {
	    f_mkind = TRUE ;
	    rs = cal_mkdirs(calp,idname,CALYEARS_DMODE) ;

#if	CF_DEBUGS
	    debugprintf("cal_indopentmp: cal_mkdirs() rs=%d\n",rs) ;
#endif

	}

	f_mkind = f_mkind || (rs == SR_NOENT) ;
	f_mkind = f_mkind || (rs == SR_STALE) ;
	f_mkind = f_mkind || (rs == SR_NOCSI) ; /* zero sized file */
	if (f_mkind) {
	    rs = cal_indmk(calp,sip,idname,sip->dt) ;

#if	CF_DEBUGS
	    debugprintf("cal_indopentmp: cal_indmk() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

	        rs = cal_indopencheck(calp,idname,year,f_search) ;

#if	CF_DEBUGS
	        debugprintf("cal_indopentmp: 2 cal_indopencheck() rs=%d\n",rs) ;
#endif

	    }

	} /* end if */

ret0:

#if	CF_DEBUGS
	debugprintf("cal_indopentmp: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_mkind : rs ;
}
/* end subroutine (cal_indopentmp) */


static int cal_mkdirs(calp,dname,dm)
CALYEARS_CAL	*calp ;
const char	dname[] ;
mode_t		dm ;
{
	int		rs ;

	dm &= S_IAMB ;
	if ((rs = mkdirs(dname,dm)) >= 0) {
	    struct ustat	sb ;
	    if ((rs = u_stat(dname,&sb)) >= 0) {
		if (((sb.st_mode & dm) != dm)) {
	            rs = uc_minmod(dname,dm) ;
		}
	    }
	} /* end if (mkdirs) */

	return rs ;
}
/* end subroutine (cal_mkdirs) */


static int cal_indopencheck(calp,dname,year,f_search)
CALYEARS_CAL	*calp ;
const char	dname[] ;
int		year ;
int		f_search ;
{
	CYI_INFO	binfo ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	{
	    char	timebuf[TIMEBUFLEN+1] ;
	    debugprintf("cal_indopencheck: dname=%s\n",dname) ;
	    debugprintf("cal_indopencheck: year=%d f_search=%u\n",
	        year,f_search) ;
	    debugprintf("cal_indopencheck: calname=%s\n",calp->calname) ;
	    debugprintf("cal_indopencheck: ti_db=%s\n",
	        timestr_log(calp->ti_db,timebuf)) ;
	}
#endif /* CF_DEBUGS */

	if ((rs = cyi_open(&calp->vind,dname,calp->calname,f_search)) >= 0) {
	    rs = cyi_info(&calp->vind,&binfo) ;

#if	CF_DEBUGS
	    {
	        char	timebuf[TIMEBUFLEN+1] ;
	        debugprintf("cal_indopencheck: cyi_info() rs=%d\n",rs) ;
	        debugprintf("cal_indopencheck: year=%u\n", binfo.year) ;
	        debugprintf("cal_indopencheck: mtime=%s\n",
	            timestr_log(binfo.mtime,timebuf)) ;
	        debugprintf("cal_indopencheck: ctime=%s\n",
	            timestr_log(binfo.ctime,timebuf)) ;
	    }
#endif /* CF_DEBUGS */

	    if (rs >= 0) {
	        int	f = FALSE ;
	        f = f || (binfo.ctime < calp->ti_db) ;
	        f = f || (binfo.mtime < calp->ti_db) ;
	        f = f || (binfo.year < year) ;
	        if (f) {
	            rs = SR_STALE ;
	            cyi_close(&calp->vind) ;
	        }
	    } /* end if (cyi_open) */
	} /* end if (cyi_open) */

	calp->f.vind = (rs >= 0) ;

#if	CF_DEBUGS
	debugprintf("cal_indopencheck: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (calyears_indopencheck) */


#ifdef	COMMENT

static int calyears_indopens(op,sip,of)
CALYEARS	*op ;
SUBINFO		*sip ;
int		of ;
{
	int		rs = SR_NOENT ;
	int		i ;

	for (i = 0 ; indopens[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("calyears_indopens: i=%u\n",i) ;
#endif

	    rs = (*indopens[i])(op,sip,of) ;

	    if ((rs == SR_BADFMT) || (rs == SR_NOMEM))
	        break ;

	    if (rs >= 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (calyears_indopens) */


static int calyears_indopenpr(op,sip,of)
CALYEARS	*op ;
SUBINFO		*sip ;
int		of ;
{
	int		rs ;
	char		idname[MAXPATHLEN + 1] ;

	if ((rs = mkpath3(idname,op->pr,VCNAME,IDXDNAME)) >= 0) {
	    rs = calyears_indopendname(op,sip,idname,of) ;
	}

	return rs ;
}
/* end subroutine (calyears_indopenpr) */


static int calyears_indopentmp(op,sip,of)
CALYEARS	*op ;
SUBINFO		*sip ;
int		of ;
{
	int		rs = SR_OK ;
	const char	*idxdname = IDXDNAME ;
	char		idname[MAXPATHLEN + 1] ;

#if	CF_TMPPRNAME
	{
	    const char	*prname ;

	    rs = sfbasename(op->pr,-1,&prname) ;
	    if (rs >= 0)
	        rs = mkpath3(idname,op->tmpdname,prname,idxdname) ;

	}
#else /* CF_TMPPRNAME */

	rs = mkpath2(idname,op->tmpdname,idxdname) ;

#endif /* CF_TMPPRNAME */

	if (rs >= 0)
	    rs = calyears_indopendname(op,sip,idname,of) ;

ret0:
	return rs ;
}
/* end subroutine (calyears_indopentmp) */


static int calyears_indopendname(op,sip,dname,of)
CALYEARS	*op ;
SUBINFO		*sip ;
const char	dname[] ;
int		of ;
{
	int		rs ;
	int		f_ok = FALSE ;
	int		f_mk = FALSE ;
	char		indname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("calyears_indopendname: dname=%s of=%05x\n",
	    dname,of) ;
#endif

	rs = mkpath2(indname,dname,op->dbname) ;
	if (rs < 0)
	    goto ret0 ;

	if (of & O_CREAT) {

	    rs = calyears_indcheck(op,indname,sip->dt) ;
	    f_ok = (rs > 0) ;

#if	CF_DEBUGS
	    debugprintf("calyears_indopendname: "
	        "calyears_indcheck() rs=%d f_ok=%u\n",
	        rs,f_ok) ;
#endif

	    if (rs < 0)
	        goto ret0 ;

#ifdef	COMMENT
	    if ((rs < 0) || (! f_ok)) {
	        rs = calyears_mksysvarsi(op,dname) ;
	        if (rs >= 0) {
	            f_mk = TRUE ;
	            rs = calyears_indcheck(op,indname,sip->dt) ;
	            f_ok = (rs > 0) ;
	        }
	    }
#endif /* COMMENT */

	    if ((rs < 0) || (! f_ok)) {
	        f_mk = TRUE ;
	        rs = calyears_indmk(op,sip,dname,sip->dt) ;

#if	CF_DEBUGS
	        debugprintf("calyears_indopendname: "
	            "calyears_indmk() rs=%d\n", rs) ;
#endif

	    } /* end if */

	    if (rs >= 0) {
	        rs = cyi_open(&op->vind,indname) ;
	        op->f.vind = (rs >= 0) ;

#if	CF_DEBUGS
	        debugprintf("calyears_indopendname: "
	            "1 cyi_open() rs=%d\n",rs) ;
#endif

	    } /* end if */

	    if ((rs < 0) && (rs != SR_BADFMT) && (! f_mk)) {
	        rs = calyears_indmk(op,sip,dname,sip->dt) ;
	        if (rs >= 0) {
	            rs = cyi_open(&op->vind,indname) ;
	            op->f.vind = (rs >= 0) ;

#if	CF_DEBUGS
	            debugprintf("calyears_indopendname: "
	                "2 cyi_open() rs=%d\n",rs) ;
#endif

	        }
	    }

	} else {

	    rs = cyi_open(&op->vind,indname) ;
	    op->f.vind = (rs >= 0) ;

#if	CF_DEBUGS
	    debugprintf("calyears_indopendname: "
	        "0 cyi_open() rs=%d\n",rs) ;
#endif

	} /* end if (open-only or open-create) */

ret0:

#if	CF_DEBUGS
	debugprintf("calyears_indopendname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (calyears_indopendname) */


static int calyears_indcheck(op,indname,dt)
CALYEARS	*op ;
const char	indname[] ;
time_t		dt ;
{
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;
	char		indfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("calyears_indcheck: indname=%s\n",indname) ;
#endif

	if ((rs = mkfnamesuf2(indfname,indname,IDXSUF,ENDIANSTR)) >= 0) {
	    struct ustat	sb ;
	    time-t		ti_ind ;
	    rs1 = u_stat(indfname,&sb) ;
	    ti_ind = sb.st_mtime ;
	    if ((rs1 >= 0) && (op->ti_db > ti_ind))
	        rs1 = SR_TIMEDOUT ;
	    if ((rs1 >= 0) && ((dt - ti_ind) >= TO_FILEMOD))
	        rs1 = SR_TIMEDOUT ;
	    f = (rs1 >= 0) ;
	} /* end if (mkfnamesuf) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (calyears_indcheck) */

#endif /* COMMENT */


#if	CF_DEBUGS && CF_DEBUGCUR
static int calyears_debugcur(op,rlp,s)
CALYEARS	*op ;
vecobj		*rlp ;
const char	s[] ;
{
	CALYEARS_ELINE	*lines ;
	CALYEARS_ENT	*ep ;
	int		rs = SR_OK ;
	int		n ;
	int		i, j ;
	n = vecobj_count(rlp) ;
	debugprintf("calyears_debugcur: %s n=%u\n",s,n) ;
	for (i = 0 ; vecobj_get(rlp,i,&ep) >= 0 ; i += 1) {
	    lines = ep->lines ;
	    for (j = 0 ; j < ep->i ; j += 1) {
	        debugprintf("calyears_debugcur: i=%u loff[%u]=%u\n",
	            i,j,lines[j].loff) ;
	        debugprintf("calyears_debugcur: i=%u llen[%u]=%u\n",
	            i,j,lines[j].llen) ;
	    }
	} /* end for */
	return rs ;
}
/* end subroutine (calyears_debugcur) */
#endif /* CF_DEBUGS */


static int cal_indmk(CALYEARS_CAL *calp,SUBINFO *sip,cchar *dname,time_t dt)
{
	const mode_t	om = 0664 ;
	int		rs ;
	int		c = 0 ;

/* check the given directory for writability */

	rs = subinfo_checkdname(sip,dname) ;

	if (rs == SR_NOENT)
	    rs = mkdirs(dname,0777) ; /* will fail if parent is not writable */

	if (rs >= 0) {
	    if ((rs = cal_indmkdata(calp,sip,dname,om)) >= 0) {
	        c += rs ;
	        calp->ti_vind = dt ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("calyears_indmk: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (cal_indmk) */


static int cal_indmkdata(CALYEARS_CAL *calp,SUBINFO *sip,cchar *dname,mode_t om)
{
	CALYEARS_ENT	e ;
	CALYEARS_QUERY	q ;
	CYIMK		cyind ;
	CYIMK_ENT	bve ;
	uint		fileoff = 0 ;
	int		rs = SR_NOANODE ;
	int		rs1 ;
	int		of ;
	int		ml, ll ;
	int		si ;
	int		len ;
	int		cidx ;
	int		year ;
	int		to ;
	int		c = 0 ;
	int		f_ent = FALSE ;
	int		f ;
	const char	*cn ;
	const char	*tp, *mp, *lp ;

	if (calp->mapdata == NULL)
	    goto ret0 ;

	rs = subinfo_year(sip) ;
	if (rs < 0)
	    goto ret0 ;

	rs = cal_idxget(calp) ;
	cidx = rs ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("calyears_indmkdata: cidx=%d\n",cidx) ;
#endif

	cn = calp->calname ;
	of = 0 ;
	year = sip->year ;
	rs = cyimk_open(&cyind,year,dname,cn,of,om) ;

#if	CF_DEBUGS
	debugprintf("calyears_indmkdata: cyimk_open() rs=%d\n",rs) ;
#endif

	if (rs == SR_INPROGRESS)
	    goto retinprogress ;

	if (rs < 0)
	    goto ret0 ;

mkgo:
	mp = calp->mapdata ;
	ml = calp->mapsize ;

#if	CF_DEBUGS
	debugprintf("calyears_indmkdata: mp=%p ml=%d\n",mp,ml) ;
#endif

	while ((tp = strnchr(mp,ml,'\n')) != NULL) {

	    len = ((tp + 1) - mp) ;
	    lp = mp ;
	    ll = (len - 1) ;

	    if (! isempty(lp,ll)) {

#if	CF_DEBUGS
	        debugprintf("calyears_indmkdata: line=>%t<\n",
	            lp,strnlen(lp,MIN(ll,40))) ;
	        if (ll > 40)
	            debugprintf("calyears_indmkdata: cont=>%t<\n",
	                (lp+40),strnlen((lp+40),MIN((ll-40),40))) ;
#endif

	        rs1 = subinfo_havestart(sip,&q,lp,ll) ;
	        si = rs1 ;

#if	CF_DEBUGS
	        debugprintf("calyears_indmkdata: subinfo_havestart() rs1=%d\n",
	            rs1) ;
	        debugprintf("calyears_indmkdata: q=(%u:%u)\n",q.m,q.d) ;
#endif

	        if (rs1 > 0) { /* start (proper) */

	            if (f_ent) {
	                c += 1 ;
	                if ((rs = mkbve_start(&bve,sip,&e)) >= 0) {
	                    rs = cyimk_add(&cyind,&bve) ;
	                    mkbve_finish(&bve) ;
	                }
	                f_ent = FALSE ;
	                entry_finish(&e) ;
	            }

	            if (rs >= 0) {
	                rs = entry_start(&e,&q,(fileoff + si),(ll - si)) ;
	                if (rs >= 0) {
	                    f_ent = TRUE ;
	                    rs = entry_setidx(&e,cidx) ;
	                }
	            }

	        } else if (rs1 == 0) { /* continuation */

	            if (f_ent)
	                rs = entry_add(&e,fileoff,ll) ;

	        } else { /* bad */

	            f = FALSE ;
	            f = f || (rs1 == SR_NOENT) || (rs == SR_NOTFOUND) ;
	            f = f || (rs1 == SR_ILSEQ) ;
	            f = f || (rs1 == SR_INVALID) ;
	            f = f || (rs1 == SR_NOTSUP) ;

	            if (f && f_ent) {
	                c += 1 ;
	                if ((rs = mkbve_start(&bve,sip,&e)) >= 0) {
	                    rs = cyimk_add(&cyind,&bve) ;
	                    mkbve_finish(&bve) ;
	                }
	                f_ent = FALSE ;
	                entry_finish(&e) ;
	            }

	        } /* end if (entry start of add) */

	    } else {

#if	CF_EMPTYTERM
	        if (f_ent) {
	            c += 1 ;
	            if ((rs = mkbve_start(&bve,sip,&e)) >= 0) {
	                rs = cyimk_add(&cyind,&bve) ;
	                mkbve_finish(&bve) ;
	            }
	            f_ent = FALSE ;
	            entry_finish(&e) ;
	        }
#else
	        rs = SR_OK ;
#endif /* CF_EMPTYTERM */

	    } /* end if (not empty) */

	    fileoff += len ;
	    ml -= len ;
	    mp += len ;

	    if (rs < 0) break ;
	} /* end while (readling lines) */

	if ((rs >= 0) && f_ent) {
	    c += 1 ;
	    if ((rs = mkbve_start(&bve,sip,&e)) >= 0) {
	        rs = cyimk_add(&cyind,&bve) ;
	        mkbve_finish(&bve) ;
	    }
	    f_ent = FALSE ;
	    entry_finish(&e) ;
	}

	if (f_ent) {
	    f_ent = FALSE ;
	    entry_finish(&e) ;
	}

ret1:
	rs1 = cyimk_close(&cyind) ;
	if (rs >= 0) rs = rs1 ;

ret0:

#if	CF_DEBUGS
	debugprintf("calyears_indmkdata: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;

retinprogress:
	rs1 = SR_EXIST ;
	cn = calp->calname ;
	of = (O_CREAT | O_EXCL) ;
	for (to = 0 ; to < TO_MKWAIT ; to += 1) {
	    sleep(1) ;
	    rs1 = cyimk_open(&cyind,year,dname,cn,of,om) ;
	    if ((rs1 >= 0) || (rs1 == SR_EXIST)) break ;
	} /* end while */

	if (to < TO_MKWAIT) {
	    if (rs1 >= 0) {
	        rs = SR_OK ;
	        goto ret1 ;
	    } else if (rs1 == SR_EXIST)
	        rs1 = SR_OK ;
	    rs = rs1 ;
	} else
	    rs = SR_TIMEDOUT ;

	goto ret0 ;
}
/* end subroutine (cal_indmkdata) */


static int cal_indclose(calp)
CALYEARS_CAL	*calp ;
{
	int		rs = SR_OK ;

	if (calp->f.vind) {
	    calp->f.vind = FALSE ;
	    rs = cyi_close(&calp->vind) ;
	}

	return rs ;
}
/* end subroutine (cal_indclose) */


#ifdef	COMMENT
static int cal_idxset(calp,cidx)
CALYEARS_CAL	*calp ;
int		cidx ;
{

	calp->cidx = cidx ;
	return SR_OK ;
}
/* end subroutine (cal_idxset) */
#endif /* COMMENT */


static int cal_idxget(calp)
CALYEARS_CAL	*calp ;
{
	int		cidx = calp->cidx ;

	return cidx ;
}
/* end subroutine (cal_idxget) */


static int cal_audit(calp)
CALYEARS_CAL	*calp ;
{
	int		rs ;

	rs = cyi_audit(&calp->vind) ;

	return rs ;
}
/* end subroutine (cal_audit) */


static int cal_loadbuf(CALYEARS_CAL *calp,char rbuf[],int rlen,CALYEARS_ENT *ep)
{
	int		rs ;
	const char	*mp ;

	if ((rs = cal_mapdata(calp,&mp)) >= 0) {
	    rs = entry_loadbuf(ep,rbuf,rlen,mp) ;
	}

#if	CF_DEBUGS
	debugprintf("calyears/cal_loadbuf: entry_loadbuf() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cal_loadbuf) */


static int cal_mapdata(calp,mpp)
CALYEARS_CAL	*calp ;
const char	**mpp ;
{
	int		rs ;

	if (mpp != NULL) {
	    *mpp = calp->mapdata ;
	}

	rs = calp->mapsize ;
	return rs ;
}
/* end subroutine (cal_mapdata) */


static int subinfo_start(sip,op,dt)
SUBINFO		*sip ;
CALYEARS	*op ;
time_t		dt ;
{
	int		rs = SR_OK ;

	if (dt == 0)
	    dt = time(NULL) ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->op = op ;
	sip->dt = dt ;

	sip->tmpdname = getenv(VARTMPDNAME) ;
	if (sip->tmpdname == NULL) sip->tmpdname = TMPDNAME ;

	if ((op->dirnames == NULL) || (op->dirnames[0] == NULL)) {
	    rs = subinfo_mkdirnames(sip) ;
	} else
	    sip->dirnames = (const char **) op->dirnames ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(sip)
SUBINFO		*sip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->f.hols) {
	    sip->f.hols = FALSE ;
	    rs1 = holidays_close(&sip->hols) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sip->f.dom) {
	    sip->f.dom = FALSE ;
	    rs1 = dayofmonth_finish(&sip->dom) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sip->f.defdirs) {
	    sip->f.defdirs = FALSE ;
	    rs1 = vecstr_finish(&sip->defdirs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sip->tudname != NULL) {
	    rs1 = uc_free(sip->tudname) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->tudname = NULL ;
	}

	if (sip->userhome != NULL) {
	    rs1 = uc_free(sip->userhome) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->userhome = NULL ;
	}

	if (sip->f.id) {
	    rs1 = ids_release(&sip->id) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->f.id = FALSE ;
	}

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_mkdirnames(sip)
SUBINFO		*sip ;
{
	CALYEARS	*op = sip->op ;
	int		rs = SR_OK ;
	int		tl ;
	int		c = 0 ;

	if ((rs = subinfo_username(sip)) >= 0) {
	    vecstr	*dlp = &sip->defdirs ;
	    if ((rs = vecstr_start(dlp,0,0)) >= 0) {
	        cchar	*sharedname = CALYEARS_DBDIR ;
	        char	tbuf[MAXPATHLEN + 1] ;
	        sip->f.defdirs = TRUE ;

/* user-home area */

		if (rs >= 0) {
	            cchar	*un = sip->username ;
	            if ((rs = mkpath2(tbuf,un,sharedname)) >= 0) {
	                tl = rs ;
	                if ((rs = subinfo_havedir(sip,tbuf)) > 0) {
	                    c += 1 ;
	                    rs = vecstr_add(dlp,tbuf,tl) ;
	                }
	            }
		} /* end if (ok) */

/* system area */

	        if (rs >= 0) {
	            if ((rs = mkpath2(tbuf,op->pr,sharedname)) >= 0) {
	                tl = rs ;
	                if ((rs = subinfo_havedir(sip,tbuf)) > 0) {
	                    c += 1 ;
	                    rs = vecstr_add(dlp,tbuf,tl) ;
	                }
	            }
	        } /* end if (ok) */

/* finish */

	        if (rs >= 0) {
	            const char	**dap ;
	            rs = vecstr_getvec(dlp,&dap) ;
	            if (rs >= 0)
	                sip->dirnames = (const char **) dap ;
	        }

	        if (rs < 0) {
	            if (sip->f.defdirs) {
	                sip->f.defdirs = FALSE ;
	                vecstr_finish(dlp) ;
	            }
	        }
	    } /* end if (vecstr) */
	} /* end if (username) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_mkdirnames) */


static int subinfo_havedir(sip,tmpdname)
SUBINFO		*sip ;
char		tmpdname[] ;
{
	struct ustat	sb ;
	int		rs ;
	int		f = FALSE ;

	if ((rs = u_stat(tmpdname,&sb)) >= 0) {
	    f = S_ISDIR(sb.st_mode) ? 1 : 0 ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_havedir) */


static int subinfo_ids(sip)
SUBINFO		*sip ;
{
	int		rs = SR_OK ;

	if (! sip->f.id) {
	    sip->f.id = TRUE ;
	    rs = ids_load(&sip->id) ;
	}

	return rs ;
}
/* end subroutine (subinfo_ids) */


static int subinfo_username(sip)
SUBINFO		*sip ;
{
	int		rs = SR_OK ;

	if (sip->username[0] == '\0') {
	    struct passwd	pw ;
	    const int		pwlen = getbufsize(getbufsize_pw) ;
	    char		*pwbuf ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	        if ((rs = getpwusername(&pw,pwbuf,pwlen,-1)) >= 0) {
	            const char	*cp ;
	            strwcpy(sip->username,pw.pw_name,USERNAMELEN) ;
	            if ((rs = uc_mallocstrw(pw.pw_dir,-1,&cp)) >= 0) {
	                sip->userhome = cp ;
		    }
		}
		uc_free(pwbuf) ;
	    } /* end if (m-a) */
	} /* end if (needed) */

	return rs ;
}
/* end subroutine (subinfo_username) */


#ifdef	COMMENT
static int subinfo_tmpuserdir(sip)
SUBINFO		*sip ;
{
	const mode_t	dmode = 0775 ;
	int		rs ;

	if ((rs = subinfo_username(sip)) >= 0) {
	    if (sip->tudname == NULL) {
	        const char	*un = sip->username ;
	        char		tmpdname[MAXPATHLEN + 1] ;
	        if ((rs = mktmpuserdir(tmpdname,un,IDXDNAME,dmode)) >= 0) {
	            int		dl = rs ;
	            const char	*dp ;
	            if ((rs = uc_mallocstrw(tmpdname,dl,&dp)) >= 0) {
	                sip->tudname = dp ;
	            }
	        }
	    } /* end if */
	} /* end if (username) */

	return rs ;
}
/* end subroutine (subinfo_tmpuserdir) */
#endif /* COMMENT */


static int subinfo_loadnames(SUBINFO *sip,vecstr *nlp,cchar dirname[])
{
	FSDIR		dir ;
	FSDIR_ENT	ds ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("calyears_loadnames: dirname=%s\n",dirname) ;
#endif

	if ((rs = fsdir_open(&dir,dirname)) >= 0) {
	    struct ustat	sb ;
	    int			nl ;
	    const char		*calsuf = CALYEARS_DBSUF ;
	    const char		*tp ;
	    const char		*np ;
	    char		tbuf[MAXPATHLEN + 1] ;

	    while (fsdir_read(&dir,&ds) > 0) {
	        if (ds.name[0] == '.') continue ;

#if	CF_DEBUGS
	        debugprintf("calyears_loadnames: name=%s\n",ds.name) ;
#endif

	        if ((tp = strrchr(ds.name,'.')) != NULL) {
	            if ((rs = mkpath2(tbuf,dirname,ds.name)) >= 0) {
	                if ((rs = u_stat(tbuf,&sb)) >= 0) {
	            	    if (S_ISREG(sb.st_mode)) {
	                	if (strcmp((tp+1),calsuf) == 0) {
	                    	    np = ds.name ;
	                    	    nl = (tp - ds.name) ;
	                            c += 1 ;
	                            rs = vecstr_add(nlp,np,nl) ;
				}
	            	    } /* end if (regular file) */
			} else if (isNotPresent(rs)) {
			    rs = SR_OK ;
	                } /* end if (correct file extension) */
	            } /* end if (mkpath) */
	        } /* end if (candidate) */

	        if (rs < 0) break ;
	    } /* end while */

	    rs1 = fsdir_close(&dir) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (fsdir) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_loadnames) */


static int subinfo_havestart(SUBINFO *sip,CALYEARS_CITE *qp,cchar *lp,int ll)
{
	int		rs1 = SR_OK ;
	int		cl ;
	int		si = 0 ;
	int		f ;
	const char	*tp, *cp ;

#if	CF_DEBUGS
	debugprintf("calyears/subinfo_havestart: >%t<\n",
	    lp,strlinelen(lp,ll,40)) ;
#endif

	if (CHAR_ISWHITE(lp[0])) /* continuation */
	    goto ret0 ;

	si = sibreak(lp,ll," \t") ;
	if (si < 3) {
	    rs1 = SR_ILSEQ ;
	    goto ret1 ;
	}

	if (isdigitlatin(lp[0])) {

#if	CF_DEBUGS
	    debugprintf("calyears/subinfo_havestart: digitlatin\n") ;
#endif

	    if ((tp = strnchr(lp,ll,'/')) != NULL) {

	        rs1 = mkmonth(lp,(tp - lp)) ;
	        qp->m = (rs1 & UCHAR_MAX) ;

#if	CF_DEBUGS
	        debugprintf("calyears/subinfo_havestart: mkmonth() rs1=%d\n",
	            rs1) ;
#endif

	        if (rs1 >= 0) {
	            cp = (tp + 1) ;
	            cl = ((lp + ll) - cp) ;
	            if ((tp = strnpbrk(cp,cl," \t")) != NULL)
	                cl = (tp - cp) ;

	            rs1 = subinfo_mkday(sip,qp->m,cp,cl) ;
	            qp->d = (rs1 & UCHAR_MAX) ;

#if	CF_DEBUGS
	            debugprintf("calyears/subinfo_havestart: mkday() rs1=%d\n",
	                rs1) ;
#endif

	        } else {

	            f = FALSE ;
	            f = f || (rs1 == SR_INVALID) ;
	            if (f)
	                rs1 = SR_ILSEQ ;

	        } /* end if */

	    } else
	        rs1 = SR_ILSEQ ;

	} else {

#if	CF_DEBUGS
	    debugprintf("calyears/subinfo_havestart: !digitlatin\n") ;
	    debugprintf("calyears/subinfo_havestart: name=>%t<\n",lp,si) ;
#endif

#if	CF_TRANSHOL
	    rs1 = subinfo_transhol(sip,qp,lp,si) ;
#else
	    rs1 = SR_NOTSUP ;
#endif

	} /* end if */

#if	CF_DEBUGS
	debugprintf("calyears/subinfo_havestart: mid rs=%d si=%u\n",rs1,si) ;
#endif

	if (rs1 >= 0)
	    si += siskipwhite((lp+si),(ll-si)) ;

ret1:
ret0:

#if	CF_DEBUGS
	debugprintf("calyears/subinfo_havestart: ret rs=%d si=%u\n",rs1,si) ;
#endif

	return (rs1 >= 0) ? si : rs1 ;
}
/* end subroutine (subinfo_havestart) */


static int subinfo_year(SUBINFO *sip)
{
	int		rs = SR_OK ;

	if (sip->year == 0) {
	    TMTIME	tm ;
	    rs = tmtime_localtime(&tm,sip->dt) ;
	    sip->year = (tm.year + TM_YEAR_BASE) ;
	    sip->isdst = tm.isdst ;
	    sip->gmtoff = tm.gmtoff ;
	}

	return rs ;
}
/* end subroutine (subinfo_year) */


static int subinfo_mkday(SUBINFO *sip,int m,cchar *cp,int cl)
{
	DAYOFMONTH	*dmp = &sip->dom ;
	int		rs = SR_OK ;

/* open the DAYOFMONTH database (manager?) if it is not already open */

	if (! sip->f.dom) {
	    rs = dayofmonth_start(dmp,sip->year) ;
	    sip->f.dom = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = dayofmonth_mkday(dmp,m,cp,cl) ;
	}

#if	CF_DEBUGS
	debugprintf("calyears/subinfo_mkday: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_mkday) */


static int subinfo_transhol(SUBINFO *sip,CALYEARS_CITE *qp,cchar *sp,int sl)
{
	HOLIDAYS_CUR	hcur ;
	HOLIDAYS	*holp ;
	CALYEARS	*op ;
	TMTIME		tm ;
	int		rs = SR_OK ;
	int		nl ;
	int		f_negative = FALSE ;
	int		f_inityear = FALSE ;
	int		f_found = FALSE ;
	int		f ;
	const char	*tp ;
	const char	*np ;

#if	CF_DEBUGS
	debugprintf("calyears/subinfo_transhol: >%t<\n",sp,sl) ;
#endif

	op = sip->op ;

	qp->m = 0 ;
	qp->d = 0 ;

	np = NULL ;
	nl = 0 ;
	if ((tp = strnpbrk(sp,sl,"+-")) != NULL) {
	    np = (tp + 1) ;
	    nl = (sl - ((tp + 1) - sp)) ;
	    sl = (tp - sp) ;
	    f_negative = (tp[0] == '-') ;
	}

	if (sl == 0) {
	    rs = SR_ILSEQ ;
	    goto ret0 ;
	}

#if	CF_DEBUGS
	if (np != NULL) {
	    debugprintf("calyears/subinfo_transhol: n=>%t<\n",np,nl) ;
	    debugprintf("calyears/subinfo_transhol: f_neg=%u\n",f_negative) ;
	} else
	    debugprintf("calyears/subinfo_transhol: *no_number*\n") ;
#endif

	holp = &sip->hols ;

/* open the HOLIDAYS database if it is not already open */

	if (! sip->init.hols) {
	    sip->init.hols = TRUE ;
	    f_inityear = TRUE ;
	    if ((rs = subinfo_year(sip)) >= 0) {
	        rs = holidays_open(holp,op->pr,sip->year,NULL) ;
	        sip->f.hols = (rs >= 0) ;

#if	CF_DEBUGS
	        debugprintf("calyears/subinfo_transhol: "
	            "holidays_open() rs=%d\n",rs) ;
#endif

	        f = FALSE ;
	        f = f || (rs == SR_BADFMT) ;
	        f = f || (rs == SR_NOMSG) ;
	        if (f)
	            rs = SR_NOENT ;
	    }
	} /* end if (open database as necessary) */

	if ((rs >= 0) && sip->f.hols) {
	    HOLIDAYS_CITE	hc ;
	    const int	hlen = HOLBUFLEN ;
	    char	hbuf[HOLBUFLEN + 1] ;

#if	CF_DEBUGS
	    debugprintf("calyears/subinfo_transhol: fq=>%t<\n",sp,sl) ;
#endif

	    if ((rs = holidays_curbegin(holp,&hcur)) >= 0) {

	        rs = holidays_fetchname(holp,sp,sl,&hcur,&hc,hbuf,hlen) ;
	        if (rs >= 0) {
	            f_found = TRUE ;
	            qp->m = hc.m ;
	            qp->d = hc.d ;
	        }

	        holidays_curend(holp,&hcur) ;
	    } /* end if (holidays-cur) */

#if	CF_DEBUGS
	    debugprintf("calyears/subinfo_transhol: "
	        "holidays_fetchname() rs=%d\n",rs) ;
	    debugprintf("calyears/subinfo_transhol: un q=(%u:%u)\n",
	        qp->m,qp->d) ;
#endif

	} /* end if */

	if ((rs >= 0) && f_found && (nl > 0)) {

	    if (! f_inityear)
	        rs = subinfo_year(sip) ;

	    if (rs >= 0) {
	        int	odays ;
	        if ((rs = cfdeci(np,nl,&odays)) >= 0) {
	            time_t	t ;

	            if (f_negative) odays = (- odays) ;

	            memset(&tm,0,sizeof(TMTIME)) ;
	            tm.isdst = sip->isdst ;
	            tm.gmtoff = sip->gmtoff ;
	            tm.year = (sip->year - TM_YEAR_BASE) ;
	            tm.mon = qp->m ;
	            tm.mday = (qp->d + odays) ;
	            if ((rs = tmtime_adjtime(&tm,&t)) >= 0) {
	                qp->m = tm.mon ;
	                qp->d = tm.mday ;
	            }

#if	CF_DEBUGS
	            debugprintf("calyears/subinfo_transhol: "
	                "adjusted q=(%u:%u)\n",qp->m,qp->d) ;
#endif

	        } /* end if (odays) */
	    } /* end if (got year) */

	} /* end if (day offset required) */

ret0:

#if	CF_DEBUGS
	debugprintf("calyears/subinfo_transhol: ret rs=%d f_found=%u\n",
	    rs,f_found) ;
#endif

	return (rs >= 0) ? f_found : rs ;
}
/* end subroutine (subinfo_transhol) */


static int subinfo_checkdname(SUBINFO *sip,cchar dname[])
{
	int		rs = SR_OK ;

	if (dname[0] == '/') {
	    struct ustat	sb ;
	    if ((rs = u_stat(dname,&sb)) >= 0) {
	        if ((rs >= 0) && (! S_ISDIR(sb.st_mode)))
	            rs = SR_NOTDIR ;
	        if (rs >= 0) {
	            rs = subinfo_ids(sip) ;
	            if (rs >= 0)
	                rs = sperm(&sip->id,&sb,W_OK) ;
	        }
	    }
	} else
	    rs = SR_INVALID ;

	return rs ;
}
/* end subroutine (subinfo_checkdname) */


static int subinfo_regacc(SUBINFO *sip,cchar *fn,int am)
{
	struct ustat	sb ;
	int		rs ;
	int		f = FALSE ;
	if ((rs = u_stat(fn,&sb)) >= 0) {
	    if (S_ISREG(sb.st_mode)) {
	        if ((rs = sperm(&sip->id,&sb,am)) >= 0) {
		    f = TRUE ;
	        } else if (isNotAccess(rs)) {
		    rs = SR_OK ;
	        }
	    }
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_regacc) */


static int entry_start(CALYEARS_ENT *ep,CALYEARS_CITE *qp,int loff,int llen)
{
	CALYEARS_ELINE	*elp ;
	const int	ne = CALYEARS_NLE ;
	int		rs ;
	int		size ;

	if (ep == NULL) return SR_FAULT ;

	memset(ep,0,sizeof(CALYEARS_ENT)) ;
	ep->cidx = -1 ;
	ep->m = qp->m ;
	ep->d = qp->d ;
	ep->voff = loff ;
	ep->vlen = llen ;

	size = ne * sizeof(CALYEARS_ELINE) ;
	if ((rs = uc_malloc(size,&elp)) >= 0) {
	    ep->lines = elp ;
	    ep->e = ne ;
	    elp->loff = loff ;
	    elp->llen = llen ;
	    ep->i += 1 ;
	}

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(CALYEARS_ENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->e <= 0) return SR_NOTOPEN ;

	if ((ep->i < 0) || (ep->i > ep->e)) return SR_BADFMT ;

	if (ep->lines != NULL) {
	    rs1 = uc_free(ep->lines) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->lines = NULL ;
	}

	ep->i = 0 ;
	ep->e = 0 ;
	return rs ;
}
/* end subroutine (entry_finish) */


static int entry_setidx(CALYEARS_ENT *ep,int cidx)
{

	if (ep == NULL) return SR_FAULT ;

	ep->cidx = cidx ;
	return SR_OK ;
}
/* end subroutine (entry_setidx) */


static int entry_add(CALYEARS_ENT *ep,uint loff,int llen)
{
	CALYEARS_ELINE	*elp ;
	int		rs = SR_OK ;
	int		ne ;
	int		size ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->e <= 0) return SR_NOTOPEN ;

	if ((ep->i < 0) || (ep->i > ep->e)) return SR_BADFMT ;

	if (ep->i == ep->e) {
	    ne = (ep->e * 2) + CALYEARS_NLE ;
	    size = ne * sizeof(CALYEARS_ELINE) ;
	    if ((rs = uc_realloc(ep->lines,size,&elp)) >= 0) {
	        ep->e = ne ;
	        ep->lines = elp ;
	    }
	}

	if (rs >= 0) {
	    ep->vlen = ((loff + llen) - ep->voff) ;
	    elp = (ep->lines + ep->i) ;
	    elp->loff = loff ;
	    elp->llen = llen ;
	    ep->i += 1 ;
	}

	return rs ;
}
/* end subroutine (entry_add) */


#if	CF_SAMECITE
static int entry_samecite(ep,oep)
CALYEARS_ENT	*ep ;
CALYEARS_ENT	*oep ;
{
	int		rs1 = SR_OK ;

	if ((ep->m == oep->m) && (ep->d == oep->d)) {
	    rs1 = 1 ;
	}

	return rs1 ;
}
/* end subroutine (entry_samecite) */
#endif /* CF_SAMECITE */


static int entry_samehash(CALYEARS_ENT *ep,CALYEARS *op,CALYEARS_ENT *oep)
{
	int		rs = SR_OK ;

/* the following checks (code) are not needed in the present implementation! */

	if ((rs >= 0) && (! ep->f.hash)) {
	    rs = entry_mkhash(ep,op) ;
	}

	if ((rs >= 0) && (! oep->f.hash)) {
	    rs = entry_mkhash(oep,op) ;
	}

/* we continue with the real (needed) work here */

	if (rs >= 0) {
	    rs = (ep->hash == oep->hash) ? 1 : 0 ;
	}

	return rs ;
}
/* end subroutine (entry_samehash) */


static int entry_mkhash(CALYEARS_ENT *ep,CALYEARS *op)
{
	CALYEARS_CAL	*calp ;
	int		rs ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->e <= 0) return SR_NOTOPEN ;

	if (ep->lines == NULL) return SR_NOTOPEN ;

	if ((rs = vechand_get(&op->cals,ep->cidx,&calp)) >= 0) {
	    const char	*mp ;
	    if ((rs = cal_mapdata(calp,&mp)) >= 0) {
	        CALYEARS_ELINE	*elp = ep->lines ;
	        uint		hash = 0 ;
	        int		i ;
	        int		sl, cl ;
	        const char	*sp, *cp ;
	        for (i = 0 ; i < ep->i ; i += 1) {
	            sp = (mp + elp[i].loff) ;
	            sl = elp[i].llen ;
	            while ((cl = nextfield(sp,sl,&cp)) > 0) {
	                hash += hashelf(cp,cl) ;
	                sl -= ((cp + cl) - sp) ;
	                sp = (cp + cl) ;
	            } /* end while */
	        } /* end for */
	        ep->hash = hash ;
	        ep->f.hash = TRUE ;
	    } /* end if (cal_mapdata) */
	} /* end if (vechand_get) */

	return rs ;
}
/* end subroutine (entry_mkhash) */


static int entry_sethash(CALYEARS_ENT *ep,uint hash)
{

	ep->hash = hash ;
	ep->f.hash = TRUE ;
	return SR_OK ;
}
/* end subroutine (entry_sethash) */


static int entry_same(CALYEARS_ENT *ep,CALYEARS *op,CALYEARS_ENT *oep)
{
	WORDER		w1, w2 ;
	int		rs ;
	int		c1l, c2l ;
	int		f = FALSE ;
	const char	*c1p, *c2p ;

	if ((rs = worder_start(&w1,op,ep)) >= 0) {

	    if ((rs = worder_start(&w2,op,oep)) >= 0) {

	        while ((rs >= 0) && (! f)) {

	            c1l = worder_get(&w1,&c1p) ;

	            c2l = worder_get(&w2,&c2p) ;

	            if (c1l != c2l)
	                break ;

	            if ((c1l == 0) && (c2l == 0)) {
	                f = TRUE ;
	                break ;
	            }

	            if (c1p[0] != c2p[0])
	                break ;

	            if (strncmp(c1p,c2p,c1l) != 0)
	                break ;

	        } /* end while */

	        worder_finish(&w2) ;
	    } /* end if (w2) */

	    worder_finish(&w1) ;
	} /* end if (w1) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (entry_same) */


static int entry_loadbuf(CALYEARS_ENT *ep,char rbuf[],int rlen,cchar *mp)
{
	SBUF		b ;
	int		rs ;
	int		len = 0 ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    CALYEARS_ELINE	*lines = ep->lines ;
	    int			nlines = ep->i ; /* number of line elements */
	    int			i ;
	    int			ll ;
	    const char		*lp ;

	    for (i = 0 ; i < nlines ; i += 1) {

	        if (i > 0) sbuf_char(&b,' ') ;

	        lp = (mp + lines[i].loff) ;
	        ll = lines[i].llen ;

#if	CF_DEBUGS
	        debugprintf("calyears/entry_loadbuf: i=%u loff=%u llen=%u\n",
	            i,lines[i].loff,lines[i].llen) ;
#endif

	        rs = sbuf_strw(&b,lp,ll) ;

	        if (rs < 0) break ;
	    } /* end for */

	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (entry_loadbuf) */


static int mkbve_start(CYIMK_ENT *bvep,SUBINFO *sip,CALYEARS_ENT *ep)
{
	int		rs ;
	int		nlines = 0 ;

	if (ep == NULL) return SR_FAULT ;

	if ((rs = entry_mkhash(ep,sip->op)) >= 0) {
	    bvep->m = ep->m ;
	    bvep->d = ep->d ;
	    bvep->voff = ep->voff ;
	    bvep->vlen = ep->vlen ;
	    bvep->hash = ep->hash ;
	    bvep->lines = NULL ;
	    nlines = ep->i ;
	    if (nlines <= UCHAR_MAX) {
	        CYIMK_LINE	*lines ;
	        const int	size = (nlines + 1) * sizeof(CYIMK_LINE) ;
	        bvep->nlines = nlines ;
	        if ((rs = uc_malloc(size,&lines)) >= 0) {
	            int	i ;
	            bvep->lines = lines ;
	            for (i = 0 ; i < nlines ; i += 1) {
	                lines[i].loff = ep->lines[i].loff ;
	                lines[i].llen = ep->lines[i].llen ;
	            }
	            lines[i].loff = 0 ;
	            lines[i].llen = 0 ;
	        } /* end if (memory-allocation) */
	    } else
	        rs = SR_TOOBIG ;
	} /* end if (entry_mkhash) */

	return (rs >= 0) ? nlines : rs ;
}
/* end subroutine (mkbve_start) */


static int mkbve_finish(CYIMK_ENT *bvep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (bvep == NULL) return SR_FAULT ;

	if (bvep->lines != NULL) {
	    rs1 = uc_free(bvep->lines) ;
	    if (rs >= 0) rs = rs1 ;
	    bvep->lines = NULL ;
	}

	return rs ;
}
/* end subroutine (mkbve_finish) */


int worder_start(WORDER *wp,CALYEARS *op,CALYEARS_ENT *ep)
{
	CALYEARS_CAL	*calp ;
	int		rs ;

	if ((rs = vechand_get(&op->cals,ep->cidx,&calp)) >= 0) {
	    const char	*mp ;
	    if ((rs = cal_mapdata(calp,&mp)) >= 0) {
	        CALYEARS_ELINE	*lines = ep->lines ;
	        wp->i = 0 ;
	        wp->nlines = ep->i ;
	        wp->lines = ep->lines ;
	        wp->mp = mp ;
	        if (lines != NULL) {
	            wp->sp = (mp + lines[0].loff) ;
	            wp->sl = (lines[0].llen) ;
	        }
	    } /* end if (cal_mapdata) */
	} /* end if (vechand_get) */

	return rs ;
}
/* end subroutine (worder_start) */


/* ARGSUSED */
int worder_finish(WORDER *wp)
{
	if (wp == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (worder_finish) */


int worder_get(WORDER *wp,cchar **rpp)
{
	int		cl = 0 ;
	const char	*cp = NULL ; /* ¥ GCC is stupid! */

	while (wp->i < wp->nlines) {

	    if ((cl = nextfield(wp->sp,wp->sl,&cp)) > 0) {
	        wp->sl -= ((cp + cl) - wp->sp) ;
	        wp->sp = (cp + cl) ;
	        break ;
	    }

	    wp->i += 1 ;
	    if (wp->i < wp->nlines) {
	        wp->sp = (wp->mp + wp->lines[wp->i].loff) ;
	        wp->sl = (wp->lines[wp->i].llen) ;
	    }

	} /* end while */

	if (rpp != NULL)
	    *rpp = cp ;

	return cl ;
}
/* end subroutine (worder_get) */


static int dayofmonth_mkday(DAYOFMONTH *dmp,uint m,cchar *cp,int cl)
{
	uint		ch ;
	int		rs = SR_NOTFOUND ;
	int		wday ;
	int		oday ;
	int		mday = 0 ;

	if (cl <= 0) {
	    rs = SR_NOTFOUND ;
	    goto ret0 ;
	}

	if (cp[cl-1] == '*')
	    cl -= 1 ;

	if (cl <= 0) {
	    rs = SR_NOTFOUND ;
	    goto ret0 ;
	}

#if	CF_DEBUGS
	debugprintf("calyears/dayofmonth_mkday: m=%u >%t<\n",m,cp,cl) ;
#endif

	ch = (cp[0] & 0xff) ;

	if (isdigitlatin(ch)) {
	    rs = cfdeci(cp,cl,&mday) ;
#if	CF_DEBUGS
	    debugprintf("calyears/dayofmonth_mkday: digit_day rs=%d\n",
	        rs) ;
#endif
	} else if (cl >= 3) {
	    if ((wday = matcasestr(days,cp,3)) >= 0) {
	        cp += 3 ;
	        cl -= 3 ;
	        oday = matocasestr(daytypes,2,cp,cl) ;
#if	CF_DEBUGS
	        debugprintf("calyears/dayofmonth_mkday: "
	            "matpcasestr() oday=%d\n",oday) ;
#endif
	        if (oday >= 0) {
	            rs = dayofmonth_lookup(dmp,m,wday,oday) ;
	            mday = rs ;
	        }
#if	CF_DEBUGS
	        debugprintf("calyears/dayofmonth_mkday: "
	            "dayofmonth_lookup() rs=%d\n", rs) ;
#endif
	    } else
	        rs = SR_ILSEQ ;
	} else
	    rs = SR_ILSEQ ;

ret0:

#if	CF_DEBUGS
	debugprintf("calyears/dayofmonth_mkday: ret rs=%d mday=%u\n",rs,mday) ;
#endif

	return (rs >= 0) ? mday : rs ;
}
/* end subroutine (dayofmonth_mkday) */


#if	CF_MKDNAME
static int mkdname(cchar *dname,mode_t dm)
{
	const int	nrs = SR_NOENT ;
	int		rs ;
	if ((rs = checkdname(dname)) == nrs) {
	    rs = mkdirs(dname,dm) ;
	}
	return rs ;
}
/* end subroutine (mkdname) */
#endif /* CF_MKDNAME */


static int isempty(cchar *lp,int ll)
{
	int		f = FALSE ;

	f = f || (ll == 0) ;
	f = f || (lp[0] == '#') ;
	if ((! f) && CHAR_ISWHITE(*lp)) {
	    int		cl ;
	    const char	*cp ;
	    cl = sfskipwhite(lp,ll,&cp) ;
	    f = ((cl == 0) || (cp[0] == '#')) ;
	}

	return f ;
}
/* end subroutine (isempty) */


static int mkmonth(cchar *cp,int cl)
{
	int		rs ;
	int		v ;

	rs = cfdeci(cp,cl,&v) ;
	v -= 1 ;

	return (rs >= 0) ? v : rs ;
}
/* end subroutine (mkmonth) */


/* for use with 'vecobj_sort(3dam)' or similar */
static int vrcmp(const void *v1p,const void *v2p)
{
	CALYEARS_ENT	*e1p, **e1pp = (CALYEARS_ENT **) v1p ;
	CALYEARS_ENT	*e2p, **e2pp = (CALYEARS_ENT **) v2p ;
	int		rc = 0 ;

	if (*e1pp != NULL) {
	    if (*e2pp != NULL) {
	        e1p = *e1pp ;
	        e2p = *e2pp ;
	        if ((rc = (e1p->m - e2p->m)) == 0) {
	            rc = (e1p->d - e2p->d) ;
	        }
	    } else
	        rc = -1 ;
	} else
	    rc = 1 ;

	return rc ;
}
/* end subroutine (vrcmp) */


