/* caldays */

/* CALDAYS object implementation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGCUR	0		/* debug cursor operation */
#define	CF_SAFE		1		/* normal safety */
#define	CF_SAFE2	1		/* extra safety */
#define	CF_EMPTYTERM	1		/* terminate entry on empty line */
#define	CF_TMPPRNAME	1		/* put under a PRNAME in /tmp */
#define	CF_SAMECITE	0		/* same entry citation? */
#define	CF_ALREADY	1		/* do not allow duplicate results */
#define	CF_TRANSHOL	1		/* translate holidays */


/* revision history:

	- 2008-10-01, David A­D­ Morano

	This object module was originally written.


*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module manages access to the various CALENDAR databases
	either in the distribution or specified by the caller.

	Implementation notes:

	= parsing a calendar file

	There are several valid forms for the date (month-day) part of
	a calendar entry.  These are:
		mm/dd		most common
		name[±ii]	name plus-or-minus increment in days
	The subroutine 'subinfo_havestart()' parses this out.


*******************************************************************************/


#define	CALDAYS_MASTER	1


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

#include	"caldays.h"
#include	"cyi.h"
#include	"cyimk.h"
#include	"dayofmonth.h"


/* local defines */

#define	CALDAYS_DBSUF		"calendar"
#define	CALDAYS_CAL		struct caldays_cal
#define	CALDAYS_ENT		struct caldays_e
#define	CALDAYS_NLE		1	/* default number line entries */
#define	CALDAYS_DMODE		0777
#define	CALDAYS_DBDIR		"share/calendar"

#undef	WORDER
#define	WORDER			struct worder

#define	SUBINFO			struct subinfo
#define	SUBINFO_FL		struct subinfo_flags

#define	IDXDNAME	".caldays"
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
	CALDAYS		*op ;
	const char	*tmpdname ;
	const char	*tudname ;
	const char	*userhome ;
	const char	**dirnames ;
	time_t		daytime ;
	int		year ;
	int		isdst ;
	int		gmtoff ;
	char		username[USERNAMELEN + 1] ;
} ;

struct caldays_calflags {
	uint		vind:1 ;
} ;

struct caldays_cal {
	const char	*dirname ;
	const char 	*calname ;		/* DB file-name */
	const char	*mapdata ;		/* DB memory-map address */
	struct caldays_calflags	f ;
	CYI		vind ;			/* verse-index */
	time_t		ti_db ;			/* DB file modification */
	time_t		ti_map ;		/* DB map */
	time_t		ti_lastcheck ;		/* DB last check */
	time_t		ti_vind ;		/* verse-index */
	size_t		filesize ;		/* DB file size */
	size_t		mapsize ;		/* DB map length */
	int		nentries ;		/* DB entries */
	int		cidx ;			/* ordinal */
} ;

struct caldays_eline {
	uint		loff ;
	uint		llen ;
} ;

struct caldays_eflags {
	uint		hash:1 ;
} ;

struct caldays_e {
	struct caldays_eline	*lines ;
	struct caldays_eflags	f ;
	uint		voff ;
	uint		vlen ;
	uint		hash ;
	int		e, i ;
	int		cidx ;
	uchar		m, d ;
} ;

struct caldays_citer {
	int		i, e ;
} ;

struct worder {
	struct caldays_eline	*lines ;
	const char	*mp ;
	const char	*sp ;
	int		sl ;
	int		i ;
	int		nlines ;
} ;


/* forward references */

static int	caldays_dirnamescreate(CALDAYS *,const char **) ;
static int	caldays_dirnamesdestroy(CALDAYS *) ;

static int	caldays_loadbuf(CALDAYS *,CALDAYS_ENT *,
			char *,int) ;
static int	caldays_tmpdels(CALDAYS *) ;
static int	caldays_calscreate(CALDAYS *,struct subinfo *,
			const char **) ;
static int	caldays_calscreater(CALDAYS *,struct subinfo *,
			const char *,const char **) ;
static int	caldays_calsdestroy(CALDAYS *) ;
static int	caldays_calcreate(CALDAYS *,struct subinfo *,
			const char *,const char *) ;
static int	caldays_caldestroy(CALDAYS *,CALDAYS_CAL *) ;

#ifdef	COMMENT
static int	caldays_checkupdate(CALDAYS *,time_t) ;
static int	caldays_mksysvarsi(CALDAYS *,const char *) ;
#endif

static int	caldays_resultfins(CALDAYS *,CALDAYS_CUR *) ;
static int	caldays_calcite(CALDAYS *,vecobj *,CALDAYS_CAL *,
			CALDAYS_QUERY *) ;
static int	caldays_mkresults(CALDAYS *,vecobj *,CALDAYS_CUR *) ;
static int	caldays_already(CALDAYS *,vecobj *,
			CALDAYS_ENT *) ;

#if	CF_DEBUGS && CF_DEBUGCUR
static int	caldays_debugcur(CALDAYS *,vecobj *,const char *) ;
#endif

static int	cal_open(CALDAYS_CAL *,struct subinfo *,int,
			const char *,const char *) ;
static int	cal_close(CALDAYS_CAL *) ;
static int	cal_dbloadbegin(CALDAYS_CAL *,struct subinfo *) ;
static int	cal_dbloadend(CALDAYS_CAL *) ;
static int	cal_indopen(CALDAYS_CAL *,struct subinfo *) ;
static int	cal_dbmapcreate(CALDAYS_CAL *,time_t) ;
static int	cal_dbmapdestroy(CALDAYS_CAL *) ;
static int	cal_indopenperm(CALDAYS_CAL *,struct subinfo *) ;
static int	cal_indopentmp(CALDAYS_CAL *,struct subinfo *) ;

#ifdef	COMMENT
static int	cal_idxset(CALDAYS_CAL *,int) ;
#endif

static int	cal_idxget(CALDAYS_CAL *) ;
static int	cal_indopencheck(CALDAYS_CAL *,const char *,int,int) ;
static int	cal_mkdirs(CALDAYS_CAL *,const char *,mode_t) ;
static int	cal_audit(CALDAYS_CAL *) ;

static int	cal_indmk(CALDAYS_CAL *,struct subinfo *,const char *,
			int,time_t) ;
static int	cal_indmkdata(CALDAYS_CAL *,struct subinfo *,const char *,
			mode_t,int) ;
static int	cal_indclose(CALDAYS_CAL *) ;

static int	cal_loadbuf(CALDAYS_CAL *,CALDAYS_ENT *,char *,int) ;
static int	cal_mapdata(CALDAYS_CAL *,const char **) ;

static int	subinfo_start(struct subinfo *,CALDAYS *,time_t) ;
static int	subinfo_ids(struct subinfo *) ;
static int	subinfo_finish(struct subinfo *) ;
static int	subinfo_username(struct subinfo *) ;
static int	subinfo_mkdirnames(struct subinfo *) ;
static int	subinfo_havedir(struct subinfo *,char *) ;
static int	subinfo_loadnames(struct subinfo *,vecstr *,const char *) ;
static int	subinfo_havestart(struct subinfo *,
			CALDAYS_QUERY *,const char *,int) ;
static int	subinfo_year(struct subinfo *) ;
static int	subinfo_mkday(struct subinfo *,int,const char *,int) ;
static int	subinfo_transhol(struct subinfo *,CALDAYS_CITE *,
			const char *,int) ;
static int	subinfo_checkdname(struct subinfo *,const char *) ;

#ifdef	COMMENT
static int	subinfo_tmpuserdir(struct subinfo *) ;
#endif

static int	entry_start(CALDAYS_ENT *,CALDAYS_CITE *,int,int) ;
static int	entry_setidx(CALDAYS_ENT *,int) ;
static int	entry_add(CALDAYS_ENT *,uint,uint) ;
static int	entry_finish(CALDAYS_ENT *) ;
static int	entry_mkhash(CALDAYS_ENT *,CALDAYS *) ;
static int	entry_sethash(CALDAYS_ENT *,uint) ;
static int	entry_samehash(CALDAYS_ENT *,CALDAYS *,CALDAYS_ENT *) ;
static int	entry_same(CALDAYS_ENT *,CALDAYS *,CALDAYS_ENT *) ;
static int	entry_loadbuf(CALDAYS_ENT *,const char *,char *,int) ;

#if	CF_SAMECITE
static int	entry_samecite(CALDAYS_ENT *,CALDAYS *,CALDAYS_ENT *) ;
#endif

static int	mkbve_start(CYIMK_ENT *,struct subinfo *,CALDAYS_ENT *) ;
static int	mkbve_finish(CYIMK_ENT *) ;

static int	worder_start(WORDER *,CALDAYS *,CALDAYS_ENT *) ;
static int	worder_finish(WORDER *) ;
static int	worder_get(WORDER *,const char **) ;

static int	isempty(const char *,int) ;

static int	mkmonth(const char *,int) ;
static int	dayofmonth_mkday(DAYOFMONTH *,uint,const char *,int) ;

static int	vrcmp(const void *,const void *) ;


/* exported variables */

CALDAYS_OBJ	caldays = {
	"caldays",
	sizeof(CALDAYS),
	sizeof(CALDAYS_CUR)
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


int caldays_open(op,pr,dirnames,calnames)
CALDAYS		*op ;
const char	pr[] ;
const char	*dirnames[] ;
const char	*calnames[] ;
{
	int	rs ;
	int	c = 0 ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;
#endif

#if	CF_DEBUGS
	debugprintf("caldays_open: pr=%s\n",pr) ;
#endif

	if (pr == NULL)
	    return SR_FAULT ;

	if (pr[0] == '\0')
	    return SR_INVALID ;

	memset(op,0,sizeof(CALDAYS)) ;
	op->pr = pr ;

	if (op->tmpdname == NULL) op->tmpdname = getenv(VARTMPDNAME) ;
	if (op->tmpdname == NULL) op->tmpdname = TMPDNAME ;

	if ((rs = caldays_dirnamescreate(op,dirnames)) >= 0) {
	    if ((rs = vecstr_start(&op->tmpfiles,2,0)) >= 0) {
	        const int	opts = VECHAND_OSTATIONARY ;
	        if ((rs = vechand_start(&op->cals,2,opts)) >= 0) {
	            struct subinfo	si, *sip = &si ;
	            if ((rs = subinfo_start(sip,op,0)) >= 0) {

	                if ((rs = caldays_calscreate(op,sip,calnames)) >= 0) {
	                    c = rs ;
	                    op->nentries = c ;
	                    op->magic = CALDAYS_MAGIC ;
	                }

	                subinfo_finish(sip) ;
	            } /* end if */
	            if (rs < 0) {
	                caldays_calsdestroy(op) ;
	                vechand_finish(&op->cals) ;
	            }
	        } /* end if (cals) */
	        if (rs < 0) {
	            caldays_tmpdels(op) ;
	            vecstr_finish(&op->tmpfiles) ;
	        }
	    } /* end if (tmpfiles) */
	    if (rs < 0)
	        caldays_dirnamesdestroy(op) ;
	} /* end if (dirnames) */

#if	CF_DEBUGS
	debugprintf("caldays_open: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (caldays_open) */


/* free up the entire vector string data structure object */
int caldays_close(op)
CALDAYS	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != CALDAYS_MAGIC)
	    return SR_NOTOPEN ;
#endif

	rs1 = caldays_calsdestroy(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&op->cals) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = caldays_tmpdels(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecstr_finish(&op->tmpfiles) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = caldays_dirnamesdestroy(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("caldays_close: ret rs=%d\n",rs) ;
#endif

	op->nentries = 0 ;
	op->magic = 0 ;
	return rs ;
}
/* end subroutine (caldays_close) */


int caldays_count(op)
CALDAYS	*op ;
{
	int	rs ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != CALDAYS_MAGIC)
	    return SR_NOTOPEN ;
#endif

	rs = op->nentries ;

	return rs ;
}
/* end subroutine (caldays_count) */


int caldays_audit(op)
CALDAYS	*op ;
{
	CALDAYS_CAL	*calp ;

	int	rs = SR_OK ;
	int	i ;
	int	c = 0 ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != CALDAYS_MAGIC)
	    return SR_NOTOPEN ;
#endif

	for (i = 0 ; vechand_get(&op->cals,i,&calp) >= 0 ; i += 1) {
	    if (calp == NULL) continue ;
	    c += 1 ;
	    rs = cal_audit(calp) ;
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (caldays_audit) */


int caldays_curbegin(op,curp)
CALDAYS	*op ;
CALDAYS_CUR	*curp ;
{


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != CALDAYS_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (curp == NULL)
	    return SR_FAULT ;

	memset(curp,0,sizeof(CALDAYS_CUR)) ;

	op->ncursors += 1 ;

	curp->i = -1 ;
	curp->magic = CALDAYS_MAGIC ;
	return SR_OK ;
}
/* end subroutine (caldays_curbegin) */


int caldays_curend(op,curp)
CALDAYS	*op ;
CALDAYS_CUR	*curp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != CALDAYS_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (curp == NULL)
	    return SR_FAULT ;

	if (curp->magic != CALDAYS_MAGIC)
	    return SR_NOTOPEN ;

	if (curp->results != NULL) {
	    rs1 = caldays_resultfins(op,curp) ;
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
/* end subroutine (caldays_curend) */


static int caldays_resultfins(op,curp)
CALDAYS	*op ;
CALDAYS_CUR	*curp ;
{
	CALDAYS_ENT	*ep = (CALDAYS_ENT *) curp->results ;

	int	rs = SR_OK ;
	int	rs1 ;


	if (ep != NULL) {
	    int	i ;
	    for (i = 0 ; i < curp->nresults ; i += 1) {
	        rs1 = entry_finish(ep + i) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (caldays_resultfins) */


int caldays_lookcite(op,curp,qvp)
CALDAYS	*op ;
CALDAYS_CUR	*curp ;
CALDAYS_QUERY	*qvp ;
{
	CALDAYS_CAL	*calp ;

	vecobj	res ;

	int	rs ;
	int	opts ;
	int	size ;
	int	c = 0 ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != CALDAYS_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;
	if (qvp == NULL) return SR_FAULT ;

	if (curp->magic != CALDAYS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("caldays_lookcite: q=(%d,%d,%d)\n",
	    qvp->y,qvp->m,qvp->d) ;
#endif

	if (curp->results != NULL) {
	    caldays_resultfins(op,curp) ;
	    uc_free(curp->results) ;
	    curp->results = NULL ;
	}

	opts = 0 ;
	opts |= VECOBJ_OORDERED ;
	opts |= VECOBJ_OSTATIONARY ;
	size = sizeof(CALDAYS_ENT) ;
	if ((rs = vecobj_start(&res,size,0,opts)) >= 0) {
	    int	i ;
	    for (i = 0 ; vechand_get(&op->cals,i,&calp) >= 0 ; i += 1) {
	        if (calp == NULL) continue ;
	        rs = caldays_calcite(op,&res,calp,qvp) ;
	        c += rs ;
#if	CF_DEBUGS
	        debugprintf("caldays_lookcite: i=%u "
	            "caldays_calcite() rs=%d\n",
	            i,rs) ;
#endif
	        if (rs < 0) break ;
	    } /* end for */
	    if (rs >= 0) {
	        rs = caldays_mkresults(op,&res,curp) ;
#if	CF_DEBUGS
	        debugprintf("caldays_lookcite: caldays_mkresults() rs=%d\n",
	            rs) ;
#endif
	    }
	    if ((rs < 0) || (c > 0)) {
	        CALDAYS_ENT	*ep ;
	        for (i = 0 ; vecobj_get(&res,i,&ep) >= 0 ; i += 1) {
	            if (ep == NULL) continue ;
	            entry_finish(ep) ;
	        }
	    }
	    vecobj_finish(&res) ;
	} /* end if (res) */

#if	CF_DEBUGS
	debugprintf("caldays_lookcite: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (caldays_lookcite) */


int caldays_read(op,curp,qvp,rbuf,rlen)
CALDAYS	*op ;
CALDAYS_CUR	*curp ;
CALDAYS_CITE	*qvp ;
char		*rbuf ;
int		rlen ;
{
	int	rs = SR_OK ;
	int	len = 0 ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != CALDAYS_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;
	if (qvp == NULL) return SR_FAULT ;

	if (curp->magic != CALDAYS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("caldays_read: ent\n") ;
#endif

	if (curp->results != NULL) {
	    const int	i = curp->i ;
#if	CF_DEBUGS
	    debugprintf("caldays_read: c_i=%d\n",i) ;
#endif
	    if ((i >= 0) && (i < curp->nresults)) {
	        CALDAYS_ENT	*ep = (CALDAYS_ENT *) curp->results ;
	        qvp->m = ep->m ;
	        qvp->d = ep->d ;
	        if (rbuf != NULL) {
#if	CF_DEBUGS
	            debugprintf("caldays_read: caldays_loadbuf()\n") ;
#endif
	            rs = caldays_loadbuf(op,(ep + i),rbuf,rlen) ;
	            len = rs ;
#if	CF_DEBUGS
	            debugprintf("caldays_read: caldays_loadbuf() rs=%d\n",
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
	debugprintf("caldays_read: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (caldays_read) */


/* ARGSUSED */
int caldays_check(op,daytime)
CALDAYS	*op ;
time_t		daytime ;
{
	int	rs = SR_OK ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != CALDAYS_MAGIC)
	    return SR_NOTOPEN ;
#endif

#ifdef	COMMENT
	rs = caldays_checkupdate(op,daytime) ;
#endif

	return rs ;
}
/* end subroutine (caldays_check) */


/* private subroutines */


static int caldays_dirnamescreate(op,dirnames)
CALDAYS		*op ;
const char	**dirnames ;
{
	int	rs = SR_OK ;

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
/* end subroutine (caldays_dirnamescreate) */


static int caldays_dirnamesdestroy(op)
CALDAYS	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;

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
/* end subroutine (caldays_dirnamesdestroy) */


static int caldays_calcite(op,rlp,calp,qvp)
CALDAYS		*op ;
vecobj		*rlp ;
CALDAYS_CAL	*calp ;
CALDAYS_QUERY	*qvp ;
{
	CYI		*cip = &calp->vind ;
	CYI_CUR		ccur ;
	CYI_QUERY	cq ;
	CYI_ENT	ce ;

	int	rs ;
	int	rs1 ;
	int	c = 0 ;

	if ((rs = cal_idxget(calp)) >= 0) {
	    int	cidx = rs ;
#if	CF_DEBUGS
	    debugprintf("caldays_calcite: cidx=%d\n",cidx) ;
#endif
	    memset(&cq,0,sizeof(CYI_QUERY)) ;
	    cq.y = qvp->y ;
	    cq.m = qvp->m ;
	    cq.d = qvp->d ;
	    if ((rs = cyi_curbegin(cip,&ccur)) >= 0) {
	        if ((rs = cyi_lookcite(cip,&ccur,&cq)) >= 0) {
	            CALDAYS_ENT	e ;
	            uint	loff ;
	            const int	celen = CEBUFLEN ;
	            int		llen ;
	            int		f_ent = FALSE ;
	            int		f_already = FALSE ;
	            char	cebuf[CEBUFLEN + 1] ;

	            while (rs >= 0) {

#if	CF_DEBUGS && CF_DEBUGCUR
	                debugprintf("caldays_calcite: cyi_read() c=%u\n",c) ;
	                caldays_debugcur(op,rlp,"before cyi_read") ;
#endif

	                rs1 = cyi_read(cip,&ccur,&ce,cebuf,celen) ;

#if	CF_DEBUGS && CF_DEBUGCUR
	                debugprintf("caldays_calcite: cyi_read() rs1=%d\n",
	                    rs1) ;
	                caldays_debugcur(op,rlp,"after cyi_read") ;
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
	                        debugprintf("caldays_calcite: "
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
	                        rs = caldays_already(op,rlp,&e) ;
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

	        } else if (rs == SR_NOTFOUND)
	            rs = SR_OK ;
	        cyi_curend(cip,&ccur) ;
	    } /* end if (cursor) */
	} /* end if (cal_idxget) */

#if	CF_DEBUGS
	debugprintf("caldays_calcite: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (caldays_calcite) */


static int caldays_already(op,rlp,ep)
CALDAYS		*op ;
vecobj		*rlp ;
CALDAYS_ENT	*ep ;
{
	CALDAYS_ENT	*oep ;

	int	rs = SR_OK ;
	int	i ;
	int	f = FALSE ;

	for (i = 0 ; vecobj_get(rlp,i,&oep) >= 0 ; i += 1) {
	    if (oep == NULL) continue ;

	    rs = entry_samehash(ep,op,oep) ;
	    if (rs == 0) continue ; /* not the same */

	    if (rs >= 0)
	        rs = entry_same(ep,op,oep) ;

	    f = (rs > 0) ; /* same? */
	    if (f) break ;

	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (caldays_already) */


static int caldays_mkresults(op,rlp,curp)
CALDAYS		*op ;
vecobj		*rlp ;
CALDAYS_CUR	*curp ;
{
	int	rs = SR_OK ;
	int	n ;
	int	c = 0 ;

#if	CF_DEBUGS
	debugprintf("caldays_mkresults: ent\n") ;
#endif

	vecobj_sort(rlp,vrcmp) ; /* sort results in ascending order */

	if ((n = vecobj_count(rlp)) > 0) {
	    CALDAYS_ENT	*rp ;
	    CALDAYS_ENT	*ep ;
	    const int	size = n * sizeof(CALDAYS_ENT) ;
	    if ((rs = uc_malloc(size,&rp)) >= 0) {
	        int	i ;
		for (i = 0 ; vecobj_get(rlp,i,&ep) >= 0 ; i += 1) {
	    	    if (ep == NULL) continue ;
#if	CF_DEBUGS
	    {
	        struct caldays_eline	*lines = ep->lines ;
	        int	j ;
	        if (lines != NULL) {
	            for (j = 0 ; j < ep->i ; j += 1) {
	                debugprintf("caldays_mkresults: "
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
	debugprintf("caldays_mkresults: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (caldays_mkresults) */


static int caldays_tmpdels(op)
CALDAYS	*op ;
{
	int	i ;

	const char	*sp ;

	for (i = 0 ; vecstr_get(&op->tmpfiles,i,&sp) >= 0 ; i += 1) {
	    if (sp == NULL) continue ;
	    if (sp[0] != '\0')
	        u_unlink(sp) ;
	} /* end for */

	return SR_OK ;
}
/* end subroutine (caldays_tmpdels) */


static int caldays_calscreate(op,sip,calnames)
CALDAYS	*op ;
struct subinfo	*sip ;
const char	*calnames[] ;
{
	int	rs = SR_OK ;
	int	c = 0 ;

	const char	**dirnames = sip->dirnames ;

	if (dirnames != NULL) {
	    int	i ;
	    for (i = 0 ; dirnames[i] != NULL ; i += 1) {
	        if (dirnames[i] == '\0') continue ;
	        rs = caldays_calscreater(op,sip,dirnames[i],calnames) ;
	        c += rs ;
	        if (rs < 0) break ;
	    } /* end for (dirnames) */
	} /* end if (dirnames) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (caldays_calscreate) */


static int caldays_calscreater(op,sip,dirname,calnames)
CALDAYS		*op ;
struct subinfo	*sip ;
const char	*dirname ;
const char	*calnames[] ;
{
	vecstr	cals ;

	int	rs = SR_OK ;
	int	j ;
	int	n = 0 ;
	int	c = 0 ;
	int	f_search = FALSE ;

	const char	**npp ;
	const char	**names = NULL ;


	if (dirname == NULL)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("caldays_calscreater: dirname=%s\n",dirname) ;
#endif

	if (calnames == NULL) {
	    rs = vecstr_start(&cals,1,0) ;
	    f_search = (rs >= 0) ;
	    if (rs >= 0) {
	        rs = subinfo_loadnames(sip,&cals,dirname) ;
	        n = rs ;
	        if ((rs >= 0) && (n > 0)) {
	            rs = vecstr_getvec(&cals,&npp) ;
	            if (rs >= 0)
	                names = npp ;
	        }
	    }
	} else
	    names = calnames ;

#if	CF_DEBUGS
	debugprintf("caldays_calscreater: mid rs=%d n=%d\n",rs,n) ;
#endif

	if (rs >= 0)
	    rs = subinfo_ids(sip) ;

	if ((rs >= 0) && (names != NULL)) {

	    for (j = 0 ; names[j] != NULL ; j += 1) {
	        if (names[j][0] == '\0') continue ;

#if	CF_DEBUGS
	        debugprintf("caldays_calscreater: caldays_calcreate()\n") ;
	        debugprintf("caldays_calscreater: calname=>%s<\n",names[j]) ;
#endif

	        rs = caldays_calcreate(op,sip,dirname,names[j]) ;
	        c += rs ;

	        if (rs < 0) break ;
	    } /* end for (calnames) */

	} /* end if */

ret1:
	if (f_search)
	    vecstr_finish(&cals) ;

ret0:

#if	CF_DEBUGS
	debugprintf("caldays_calscreater: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (caldays_calscreater) */


static int caldays_calsdestroy(op)
CALDAYS	*op ;
{
	CALDAYS_CAL	*calp ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;

	for (i = 0 ; vechand_get(&op->cals,i,&calp) >= 0 ; i += 1) {
	    if (calp == NULL) continue ;
	    rs1 = caldays_caldestroy(op,calp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end for */

	return rs ;
}
/* end subroutine (caldays_calsdestroy) */


static int caldays_calcreate(op,sip,dirname,calname)
CALDAYS	*op ;
struct subinfo	*sip ;
const char	dirname[] ;
const char	calname[] ;
{
	struct ustat	sb ;

	CALDAYS_CAL	*calp ;

	const int	size = sizeof(CALDAYS_CAL) ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	cidx ;
	int	f = FALSE ;

	const char	*suf = CALDAYS_DBSUF ;

	char	cname[MAXNAMELEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;


	rs = snsds(cname,MAXNAMELEN,calname,suf) ;

	if (rs >= 0)
	    rs = mkpath2(tmpfname,dirname,cname) ;

	if (rs < 0)
	    goto bad0 ;

	rs1 = u_stat(tmpfname,&sb) ;

#if	CF_DEBUGS
	debugprintf("caldays_calcreate: u_stat() rs=%d\n",rs1) ;
#endif

	if (rs1 >= 0)
	    rs1 = sperm(&sip->id,&sb,R_OK) ;

#if	CF_DEBUGS
	debugprintf("caldays_calcreate: fn=%s rs=%d\n",tmpfname,rs1) ;
#endif

	if (rs1 < 0)
	    goto bad0 ;

	f = f || S_ISREG(sb.st_mode) ;
	if (! f)
	    rs = SR_LIBEXEC ; /* or should it be SR_OPNOTSUPP */

	if (rs < 0)
	    goto bad0 ;

	rs = uc_malloc(size,&calp) ;
	if (rs < 0)
	    goto bad0 ;

	rs = vechand_add(&op->cals,calp) ;
	cidx = rs ;
	if (rs < 0)
	    goto bad1 ;

	rs = cal_open(calp,sip,cidx,dirname,calname) ;
	if (rs < 0)
	    goto bad2 ;

	f = TRUE ;

ret0:
	return (rs >= 0) ? f : rs ;

/* bad stuff */
bad2:
	vechand_del(&op->cals,cidx) ;

bad1:
	uc_free(calp) ;

bad0:
	goto ret0 ;
}
/* end subroutine (caldays_calcreate) */


static int caldays_caldestroy(op,calp)
CALDAYS	*op ;
CALDAYS_CAL	*calp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


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
/* end subroutine (caldays_caldestroy) */


#ifdef	COMMENT
static int caldays_checkupdate(op,daytime)
CALDAYS	*op ;
time_t		daytime ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	to = TO_CHECK ;
	int	f = FALSE ;


	if (op->ncursors > 0)
	    goto ret0 ;

	if (daytime <= 0)
	    daytime = time(NULL) ;

	if ((daytime - op->ti_lastcheck) >= to) {
	    struct ustat	sb ;
	    op->ti_lastcheck = daytime ;
	    if ((rs1 = u_stat(op->dbfname,&sb)) >= 0) {
	        if ((sb.st_mtime > op->ti_db) || (sb.st_mtime > op->ti_map)) {
	            struct subinfo	si ;

	            f = TRUE ;
	            caldays_dbloadend(op) ;

	            if ((rs = subinfo_start(&si,op,0)) >= 0) {

	                rs = caldays_dbloadbegin(op,&si) ;

	                subinfo_finish(&si) ;
	            } /* end if */

	        } /* end if (update) */
	    } /* end if (stat) */
	} /* end if (time-out) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (caldays_checkupdate) */
#endif /* COMMENT */


static int caldays_loadbuf(op,ep,rbuf,rlen)
CALDAYS	*op ;
CALDAYS_ENT	*ep ;
char		rbuf[] ;
int		rlen ;
{
	CALDAYS_CAL	*calp ;

	int	rs ;
	int	cidx = ep->cidx ;

#if	CF_DEBUGS
	debugprintf("caldays_loadbuf: cidx=%d\n",cidx) ;
#endif

	if ((rs = vechand_get(&op->cals,cidx,&calp)) >= 0) {
	    rs = cal_loadbuf(calp,ep,rbuf,rlen) ;
	}

#if	CF_DEBUGS
	debugprintf("caldays_loadbuf: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (caldays_loadbuf) */


static int cal_open(calp,sip,cidx,dirname,calname)
CALDAYS_CAL	*calp ;
struct subinfo	*sip ;
int		cidx ;
const char	dirname[] ;
const char	calname[] ;
{
	int	rs ;
	const char	*cp ;

	memset(calp,0,sizeof(CALDAYS_CAL)) ;
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
CALDAYS_CAL	*calp ;
{
	int	rs = SR_OK ;
	int	rs1 ;

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


static int cal_dbloadbegin(calp,sip)
CALDAYS_CAL	*calp ;
struct subinfo	*sip ;
{
	int	rs ;

	if ((rs = cal_dbmapcreate(calp,sip->daytime)) >= 0) {
	    rs = cal_indopen(calp,sip) ;
	    if (rs < 0)
	        cal_dbmapdestroy(calp) ;
	}

	return rs ;
}
/* end subroutine (cal_dbloadbegin) */


static int cal_dbloadend(calp)
CALDAYS_CAL	*calp ;
{
	int	rs = SR_OK ;
	int	rs1 ;

	rs1 = cal_indclose(calp) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = cal_dbmapdestroy(calp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (cal_dbloadend) */


static int cal_dbmapcreate(calp,daytime)
CALDAYS_CAL	*calp ;
time_t		daytime ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	fd ;

	char	cname[MAXNAMELEN + 1] ;
	char	dbfname[MAXPATHLEN + 1] ;


	rs = snsds(cname,MAXNAMELEN,calp->calname,CALDAYS_DBSUF) ;
	if (rs < 0)
	    goto ret0 ;

	rs = mkpath2(dbfname,calp->dirname,cname) ;
	if (rs < 0)
	    goto ret0 ;

/* open it */

	rs = u_open(dbfname,O_RDONLY,0666) ;
	fd = rs ;
	if (rs < 0)
	    goto ret0 ;

	rs = u_fstat(fd,&sb) ;
	if (rs < 0)
	    goto ret1 ;

	if (! S_ISREG(sb.st_mode)) {
	    rs = SR_NOTSUP ;
	    goto ret1 ;
	}

	if ((sb.st_size > INT_MAX) || (sb.st_size < 0)) {
	    rs = SR_TOOBIG ;
	    goto ret1 ;
	}

	calp->filesize = sb.st_size ;
	calp->ti_db = sb.st_mtime ;

/* map it */

	{
	    size_t	msize = (size_t) calp->filesize ;
	    int		mprot = PROT_READ ;
	    int		mflags = MAP_SHARED ;
	    void	*mdata ;
	    if ((rs = u_mmap(NULL,msize,mprot,mflags,fd,0L,&mdata)) >= 0) {
	        calp->mapdata = mdata ;
	        calp->mapsize = calp->filesize ;
	        calp->ti_map = daytime ;
	        calp->ti_lastcheck = daytime ;
	    } /* end if (u_mmap) */
	} /* end block */

/* close it */
ret1:
	u_close(fd) ;

ret0:
	return rs ;
}
/* end subroutine (cal_dbmapcreate) */


static int cal_dbmapdestroy(calp)
CALDAYS_CAL	*calp ;
{
	int	rs = SR_OK ;

	if (calp->mapdata != NULL) {
	    rs = u_munmap(calp->mapdata,calp->mapsize) ;
	    calp->mapdata = NULL ;
	    calp->mapsize = 0 ;
	}

	return rs ;
}
/* end subroutine (cal_dbmapdestroy) */


static int cal_indopen(calp,sip)
CALDAYS_CAL	*calp ;
struct subinfo	*sip ;
{
	int	rs ;


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
CALDAYS_CAL	*calp ;
struct subinfo	*sip ;
{
	int	rs ;
	int	year = sip->year ;
	int	f_search = FALSE ;
	int	f_mkind = FALSE ;

	const char	*idxdname = IDXDNAME ;

	char	idname[MAXNAMELEN + 1] ;


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
	    rs = cal_mkdirs(calp,idname,CALDAYS_DMODE) ;

#if	CF_DEBUGS
	    debugprintf("cal_indopenperm: cal_mkdirs() rs=%d\n",rs) ;
#endif

	}

	f_mkind = f_mkind || (rs == SR_NOENT) ;
	f_mkind = f_mkind || (rs == SR_STALE) ;
	f_mkind = f_mkind || (rs == SR_NOCSI) ; /* zero sized file */
	if (f_mkind) {
	    rs = cal_indmk(calp,sip,idname,f_search,sip->daytime) ;

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
CALDAYS_CAL	*calp ;
struct subinfo	*sip ;
{
	CALDAYS	*op = sip->op ;

	int	rs ;
	int	year = sip->year ;
	int	f_search = TRUE ;
	int	f_mkind = FALSE ;

	const char	*idxdname = IDXDNAME ;

	char	idname[MAXPATHLEN + 1] ;


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
	    rs = cal_mkdirs(calp,idname,CALDAYS_DMODE) ;

#if	CF_DEBUGS
	    debugprintf("cal_indopentmp: cal_mkdirs() rs=%d\n",rs) ;
#endif

	}

	f_mkind = f_mkind || (rs == SR_NOENT) ;
	f_mkind = f_mkind || (rs == SR_STALE) ;
	f_mkind = f_mkind || (rs == SR_NOCSI) ; /* zero sized file */
	if (f_mkind) {
	    rs = cal_indmk(calp,sip,idname,f_search,sip->daytime) ;

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
CALDAYS_CAL	*calp ;
const char	dname[] ;
mode_t		dm ;
{
	struct ustat	sb ;

	int	rs ;


	dm &= S_IAMB ;
	if ((rs = mkdirs(dname,dm)) >= 0) {
	    rs = u_stat(dname,&sb) ;
	    if ((rs >= 0) && ((sb.st_mode & dm) != dm)) {
	        rs = u_chmod(dname,dm) ;
	    }
	} /* end if (mkdirs) */

	return rs ;
}
/* end subroutine (cal_mkdirs) */


static int cal_indopencheck(calp,dname,year,f_search)
CALDAYS_CAL	*calp ;
const char	dname[] ;
int		year ;
int		f_search ;
{
	CYI_INFO	binfo ;

	int	rs = SR_OK ;


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

	rs = cyi_open(&calp->vind,dname,calp->calname,f_search) ;

#if	CF_DEBUGS
	debugprintf("cal_indopencheck: cyi_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
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
	} /* end if */

	calp->f.vind = (rs >= 0) ;

#if	CF_DEBUGS
	debugprintf("cal_indopencheck: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (caldays_indopencheck) */


#ifdef	COMMENT

static int caldays_indopens(op,sip,oflags)
CALDAYS	*op ;
struct subinfo	*sip ;
int		oflags ;
{
	int	rs = SR_NOENT ;
	int	i ;


	for (i = 0 ; indopens[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("caldays_indopens: i=%u\n",i) ;
#endif

	    rs = (*indopens[i])(op,sip,oflags) ;

	    if ((rs == SR_BADFMT) || (rs == SR_NOMEM))
	        break ;

	    if (rs >= 0)
	        break ;

	} /* end for */

	return rs ;
}
/* end subroutine (caldays_indopens) */


static int caldays_indopenpr(op,sip,oflags)
CALDAYS	*op ;
struct subinfo	*sip ;
int		oflags ;
{
	int	rs = SR_OK ;

	char	idname[MAXPATHLEN + 1] ;


	rs = mkpath3(idname,op->pr,VCNAME,IDXDNAME) ;
	if (rs < 0)
	    goto ret0 ;

	rs = caldays_indopendname(op,sip,idname,oflags) ;

ret0:
	return rs ;
}
/* end subroutine (caldays_indopenpr) */


static int caldays_indopentmp(op,sip,oflags)
CALDAYS	*op ;
struct subinfo	*sip ;
int		oflags ;
{
	int	rs = SR_OK ;

	const char	*idxdname = IDXDNAME ;

	char	idname[MAXPATHLEN + 1] ;


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
	    rs = caldays_indopendname(op,sip,idname,oflags) ;

ret0:
	return rs ;
}
/* end subroutine (caldays_indopentmp) */


static int caldays_indopendname(op,sip,dname,oflags)
CALDAYS	*op ;
struct subinfo	*sip ;
const char	dname[] ;
int		oflags ;
{
	int	rs ;
	int	f_ok = FALSE ;
	int	f_mk = FALSE ;

	char	indname[MAXPATHLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("caldays_indopendname: dname=%s of=%05x\n",
	    dname,oflags) ;
#endif

	rs = mkpath2(indname,dname,op->dbname) ;
	if (rs < 0)
	    goto ret0 ;

	if (oflags & O_CREAT) {

	    rs = caldays_indcheck(op,indname,sip->daytime) ;
	    f_ok = (rs > 0) ;

#if	CF_DEBUGS
	    debugprintf("caldays_indopendname: "
	        "caldays_indcheck() rs=%d f_ok=%u\n",
	        rs,f_ok) ;
#endif

	    if (rs < 0)
	        goto ret0 ;

#ifdef	COMMENT
	    if ((rs < 0) || (! f_ok)) {
	        rs = caldays_mksysvarsi(op,dname) ;
	        if (rs >= 0) {
	            f_mk = TRUE ;
	            rs = caldays_indcheck(op,indname,sip->daytime) ;
	            f_ok = (rs > 0) ;
	        }
	    }
#endif /* COMMENT */

	    if ((rs < 0) || (! f_ok)) {
	        f_mk = TRUE ;
	        rs = caldays_indmk(op,sip,dname,sip->daytime) ;

#if	CF_DEBUGS
	        debugprintf("caldays_indopendname: "
	            "caldays_indmk() rs=%d\n",
	            rs) ;
#endif

	    } /* end if */

	    if (rs >= 0) {
	        rs = cyi_open(&op->vind,indname) ;
	        op->f.vind = (rs >= 0) ;

#if	CF_DEBUGS
	        debugprintf("caldays_indopendname: "
	            "1 cyi_open() rs=%d\n",rs) ;
#endif

	    } /* end if */

	    if ((rs < 0) && (rs != SR_BADFMT) && (! f_mk)) {
	        rs = caldays_indmk(op,sip,dname,sip->daytime) ;
	        if (rs >= 0) {
	            rs = cyi_open(&op->vind,indname) ;
	            op->f.vind = (rs >= 0) ;

#if	CF_DEBUGS
	            debugprintf("caldays_indopendname: "
	                "2 cyi_open() rs=%d\n",rs) ;
#endif

	        }
	    }

	} else {

	    rs = cyi_open(&op->vind,indname) ;
	    op->f.vind = (rs >= 0) ;

#if	CF_DEBUGS
	    debugprintf("caldays_indopendname: "
	        "0 cyi_open() rs=%d\n",rs) ;
#endif

	} /* end if (open-only or open-create) */

ret0:

#if	CF_DEBUGS
	debugprintf("caldays_indopendname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (caldays_indopendname) */


static int caldays_indcheck(op,indname,daytime)
CALDAYS	*op ;
const char	indname[] ;
time_t		daytime ;
{
	int	rs ;
	int	rs1 ;
	int	f = FALSE ;

	char	indfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("caldays_indcheck: indname=%s\n",indname) ;
#endif

	if ((rs = mkfnamesuf2(indfname,indname,IDXSUF,ENDIANSTR)) >= 0) {
	    struct ustat	sb ;
	    time-t		ti_ind ;
	    rs1 = u_stat(indfname,&sb) ;
	    ti_ind = sb.st_mtime ;
	    if ((rs1 >= 0) && (op->ti_db > ti_ind))
	        rs1 = SR_TIMEDOUT ;
	    if ((rs1 >= 0) && ((daytime - ti_ind) >= TO_FILEMOD))
	        rs1 = SR_TIMEDOUT ;
	    f = (rs1 >= 0) ;
	} /* end if (mkfnamesuf) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (caldays_indcheck) */

#endif /* COMMENT */


#if	CF_DEBUGS && CF_DEBUGCUR

static int caldays_debugcur(op,rlp,s)
CALDAYS	*op ;
vecobj		*rlp ;
const char	s[] ;
{
	struct caldays_eline	*lines ;

	CALDAYS_ENT		*ep ;

	int	rs = SR_OK ;
	int	n ;
	int	i, j ;


	n = vecobj_count(rlp) ;
	debugprintf("caldays_debugcur: %s n=%u\n",s,n) ;
	for (i = 0 ; vecobj_get(rlp,i,&ep) >= 0 ; i += 1) {
	    lines = ep->lines ;
	    for (j = 0 ; j < ep->i ; j += 1) {
	        debugprintf("caldays_debugcur: i=%u loff[%u]=%u\n",
	            i,j,lines[j].loff) ;
	        debugprintf("caldays_debugcur: i=%u llen[%u]=%u\n",
	            i,j,lines[j].llen) ;
	    }
	} /* end for */

ret0:
	return rs ;
}
/* end subroutine (caldays_debugcur) */

#endif /* CF_DEBUGS */


static int cal_indmk(calp,sip,dname,f_tmp,daytime)
CALDAYS_CAL	*calp ;
struct subinfo	*sip ;
const char	dname[] ;
int		f_tmp ;
time_t		daytime ;
{
	const mode_t	operms = 0664 ;
	int	rs ;
	int	c = 0 ;


/* check the given directory for writability */

	rs = subinfo_checkdname(sip,dname) ;

	if (rs == SR_NOENT)
	    rs = mkdirs(dname,0777) ; /* will fail if parent is not writable */

	if (rs >= 0) {
	    if ((rs = cal_indmkdata(calp,sip,dname,operms,f_tmp)) >= 0) {
	        c += rs ;
	        calp->ti_vind = daytime ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("caldays_indmk: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (cal_indmk) */


static int cal_indmkdata(calp,sip,dname,operms,f_tmp)
CALDAYS_CAL	*calp ;
struct subinfo	*sip ;
const char	dname[] ;
mode_t		operms ;
int		f_tmp ;
{
	CALDAYS_ENT	e ;
	CALDAYS_QUERY	q ;

	CYIMK		cyind ;
	CYIMK_ENT	bve ;

	uint	fileoff = 0 ;

	int	rs = SR_NOANODE ;
	int	rs1 ;
	int	oflags ;
	int	ml, ll ;
	int	si ;
	int	len ;
	int	cidx ;
	int	year ;
	int	to ;
	int	c = 0 ;
	int	f_ent = FALSE ;
	int	f ;

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
	debugprintf("caldays_indmkdata: cidx=%d\n",cidx) ;
#endif

	cn = calp->calname ;
	oflags = 0 ;
	year = sip->year ;
	rs = cyimk_open(&cyind,dname,cn,oflags,operms,year,f_tmp) ;

#if	CF_DEBUGS
	debugprintf("caldays_indmkdata: cyimk_open() rs=%d\n",rs) ;
#endif

	if (rs == SR_INPROGRESS)
	    goto retinprogress ;

	if (rs < 0)
	    goto ret0 ;

mkgo:
	mp = calp->mapdata ;
	ml = calp->mapsize ;

#if	CF_DEBUGS
	debugprintf("caldays_indmkdata: mp=%p ml=%d\n",mp,ml) ;
#endif

	while ((tp = strnchr(mp,ml,'\n')) != NULL) {

	    len = ((tp + 1) - mp) ;
	    lp = mp ;
	    ll = (len - 1) ;

	    if (! isempty(lp,ll)) {

#if	CF_DEBUGS
	        debugprintf("caldays_indmkdata: line=>%t<\n",
	            lp,strnlen(lp,MIN(ll,40))) ;
	        if (ll > 40)
	            debugprintf("caldays_indmkdata: cont=>%t<\n",
	                (lp+40),strnlen((lp+40),MIN((ll-40),40))) ;
#endif

	        rs1 = subinfo_havestart(sip,&q,lp,ll) ;
	        si = rs1 ;

#if	CF_DEBUGS
	        debugprintf("caldays_indmkdata: subinfo_havestart() rs1=%d\n",
	            rs1) ;
	        debugprintf("caldays_indmkdata: q=(%u:%u)\n",q.m,q.d) ;
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
	debugprintf("caldays_indmkdata: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;

retinprogress:
	rs1 = SR_EXIST ;
	cn = calp->calname ;
	oflags = (O_CREAT | O_EXCL) ;
	for (to = 0 ; to < TO_MKWAIT ; to += 1) {
	    sleep(1) ;
	    rs1 = cyimk_open(&cyind,dname,cn,oflags,operms,year,f_tmp) ;
	    if ((rs1 >= 0) || (rs1 == SR_EXIST))
	        break ;
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
CALDAYS_CAL	*calp ;
{
	int	rs = SR_OK ;

	if (calp->f.vind) {
	    calp->f.vind = FALSE ;
	    rs = cyi_close(&calp->vind) ;
	}

	return rs ;
}
/* end subroutine (cal_indclose) */


#ifdef	COMMENT
static int cal_idxset(calp,cidx)
CALDAYS_CAL	*calp ;
int		cidx ;
{

	calp->cidx = cidx ;
	return SR_OK ;
}
/* end subroutine (cal_idxset) */
#endif /* COMMENT */


static int cal_idxget(calp)
CALDAYS_CAL	*calp ;
{
	int	cidx = calp->cidx ;


	return cidx ;
}
/* end subroutine (cal_idxget) */


static int cal_audit(calp)
CALDAYS_CAL	*calp ;
{
	int	rs = SR_OK ;


	rs = cyi_audit(&calp->vind) ;

	return rs ;
}
/* end subroutine (cal_audit) */


static int cal_loadbuf(calp,ep,rbuf,rlen)
CALDAYS_CAL	*calp ;
CALDAYS_ENT	*ep ;
char		rbuf[] ;
int		rlen ;
{
	int	rs ;

	const char	*mp ;

	if ((rs = cal_mapdata(calp,&mp)) >= 0)
	    rs = entry_loadbuf(ep,mp,rbuf,rlen) ;

#if	CF_DEBUGS
	debugprintf("caldays/cal_loadbuf: entry_loadbuf() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cal_loadbuf) */


static int cal_mapdata(calp,mpp)
CALDAYS_CAL	*calp ;
const char	**mpp ;
{
	int	rs ;


	if (mpp != NULL)
	    *mpp = calp->mapdata ;

	rs = calp->mapsize ;
	return rs ;
}
/* end subroutine (cal_mapdata) */


static int subinfo_start(sip,op,daytime)
struct subinfo	*sip ;
CALDAYS		*op ;
time_t		daytime ;
{
	int	rs = SR_OK ;


	if (daytime == 0)
	    daytime = time(NULL) ;

	memset(sip,0,sizeof(struct subinfo)) ;
	sip->op = op ;
	sip->daytime = daytime ;

	sip->tmpdname = getenv(VARTMPDNAME) ;
	if (sip->tmpdname == NULL)
	    sip->tmpdname = TMPDNAME ;

	if ((op->dirnames == NULL) || (op->dirnames[0] == NULL)) {
	    rs = subinfo_mkdirnames(sip) ;
	} else
	    sip->dirnames = (const char **) op->dirnames ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(sip)
struct subinfo	*sip ;
{
	int	rs = SR_OK ;
	int	rs1 ;

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
struct subinfo	*sip ;
{
	CALDAYS	*op = sip->op ;

	int	rs = SR_OK ;
	int	tl ;
	int	c = 0 ;

	const char	*sharedname = CALDAYS_DBDIR ;
	char	tmpdname[MAXPATHLEN + 1] ;

	if ((rs = subinfo_username(sip)) >= 0) {
	    if ((rs = vecstr_start(&sip->defdirs,0,0)) >= 0) {
	        sip->f.defdirs = (rs >= 0) ;

/* user-home area */

	        if (rs >= 0) {
	            const char	*un = sip->username ;
	            if ((rs = mkpath2(tmpdname,un,sharedname)) >= 0) {
	                tl = rs ;
	                if ((rs = subinfo_havedir(sip,tmpdname)) > 0) {
	                    c += 1 ;
	                    rs = vecstr_add(&sip->defdirs,tmpdname,tl) ;
	                }
	            }
	        } /* end if */

/* system area */

	        if (rs >= 0) {
	            if ((rs = mkpath2(tmpdname,op->pr,sharedname)) >= 0) {
	                tl = rs ;
	                if ((rs = subinfo_havedir(sip,tmpdname)) > 0) {
	                    c += 1 ;
	                    rs = vecstr_add(&sip->defdirs,tmpdname,tl) ;
	                }
	            }
	        } /* end if */

/* finish */

	        if (rs >= 0) {
	            const char	**dap ;
	            rs = vecstr_getvec(&sip->defdirs,&dap) ;
	            if (rs >= 0)
	                sip->dirnames = (const char **) dap ;
	        }

	        if (rs < 0) {
	            if (sip->f.defdirs) {
	                sip->f.defdirs = FALSE ;
	                vecstr_finish(&sip->defdirs) ;
	            }
	        }
	    } /* end if (vecstr) */
	} /* end if (username) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_mkdirnames) */


static int subinfo_havedir(sip,tmpdname)
struct subinfo	*sip ;
char		tmpdname[] ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	rs1 ; /* lint-ok */
	int	f = FALSE ;

	if ((rs1 = u_stat(tmpdname,&sb)) >= 0) {
	    f = S_ISDIR(sb.st_mode) ? 1 : 0 ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_havedir) */


static int subinfo_ids(sip)
struct subinfo	*sip ;
{
	int	rs = SR_OK ;

	if (! sip->f.id) {
	    sip->f.id = TRUE ;
	    rs = ids_load(&sip->id) ;
	}

	return rs ;
}
/* end subroutine (subinfo_ids) */


static int subinfo_username(sip)
struct subinfo	*sip ;
{
	int	rs = SR_OK ;

	if (sip->username[0] == '\0') {
	    struct passwd	pw ;
	    const int	pwlen = PWBUFLEN ;
	    char	pwbuf[PWBUFLEN + 1] ;
	    if ((rs = getpwusername(&pw,pwbuf,pwlen,-1)) >= 0) {
	        const char	*cp ;
	        strwcpy(sip->username,pw.pw_name,USERNAMELEN) ;
	        rs = uc_mallocstrw(pw.pw_dir,-1,&cp) ;
	        if (rs >= 0) sip->userhome = cp ;
	    }
	}

	return rs ;
}
/* end subroutine (subinfo_username) */


#ifdef	COMMENT
static int subinfo_tmpuserdir(sip)
struct subinfo	*sip ;
{
	const mode_t	dmode = 0775 ;

	int	rs ;

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


static int subinfo_loadnames(sip,nlp,dirname)
struct subinfo	*sip ;
vecstr		*nlp ;
const char	dirname[] ;
{
	struct ustat	sb ;

	FSDIR		dir ;
	FSDIR_ENT	ds ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	nl ;
	int	c = 0 ;

	const char	*calsuf = CALDAYS_DBSUF ;
	const char	*tp ;
	const char	*np ;

	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("caldays_loadnames: dirname=%s\n",dirname) ;
#endif

	if ((rs = fsdir_open(&dir,dirname)) >= 0) {

	    while (fsdir_read(&dir,&ds) > 0) {
	        if (ds.name[0] == '.') continue ;

#if	CF_DEBUGS
	        debugprintf("caldays_loadnames: name=%s\n",ds.name) ;
#endif

	        if ((tp = strrchr(ds.name,'.')) != NULL) {

	            rs1 = mkpath2(tmpfname,dirname,ds.name) ;

	            if (rs1 >= 0)
	                rs1 = u_stat(tmpfname,&sb) ;

#if	CF_DEBUGS
	            debugprintf("caldays_loadnames: u_stat() rs=%d\n",rs1) ;
#endif

	            if ((rs1 >= 0) && S_ISREG(sb.st_mode)) {

	                if (strcmp((tp+1),calsuf) == 0) {

	                    np = ds.name ;
	                    nl = (tp - ds.name) ;

#if	CF_DEBUGS
	                    debugprintf("caldays_loadnames: calname=%t\n",
	                        np,nl) ;
#endif

	                    c += 1 ;
	                    rs = vecstr_add(nlp,np,nl) ;

	                } /* end if (correct file extension) */

	            } /* end if (regular file) */

	        } /* end if */

	        if (rs < 0) break ;
	    } /* end while */

	    fsdir_close(&dir) ;
	} /* end if (fsdir) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_loadnames) */


static int subinfo_havestart(sip,qp,lp,ll)
struct subinfo	*sip ;
CALDAYS_CITE	*qp ;
const char	*lp ;
int		ll ;
{
	int	rs1 = SR_OK ;
	int	cl ;
	int	si = 0 ;
	int	f ;

	const char	*tp, *cp ;


#if	CF_DEBUGS
	debugprintf("caldays/subinfo_havestart: >%t<\n",
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
	    debugprintf("caldays/subinfo_havestart: digitlatin\n") ;
#endif

	    if ((tp = strnchr(lp,ll,'/')) != NULL) {

	        rs1 = mkmonth(lp,(tp - lp)) ;
	        qp->m = (rs1 & UCHAR_MAX) ;

#if	CF_DEBUGS
	        debugprintf("caldays/subinfo_havestart: mkmonth() rs1=%d\n",
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
	            debugprintf("caldays/subinfo_havestart: mkday() rs1=%d\n",
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
	    debugprintf("caldays/subinfo_havestart: !digitlatin\n") ;
	    debugprintf("caldays/subinfo_havestart: name=>%t<\n",lp,si) ;
#endif

#if	CF_TRANSHOL
	    rs1 = subinfo_transhol(sip,qp,lp,si) ;
#else
	    rs1 = SR_NOTSUP ;
#endif

	} /* end if */

#if	CF_DEBUGS
	debugprintf("caldays/subinfo_havestart: mid rs=%d si=%u\n",rs1,si) ;
#endif

	if (rs1 >= 0)
	    si += siskipwhite((lp+si),(ll-si)) ;

ret1:
ret0:

#if	CF_DEBUGS
	debugprintf("caldays/subinfo_havestart: ret rs=%d si=%u\n",rs1,si) ;
#endif

	return (rs1 >= 0) ? si : rs1 ;
}
/* end subroutine (subinfo_havestart) */


static int subinfo_year(sip)
struct subinfo	*sip ;
{
	int	rs = SR_OK ;

	if (sip->year == 0) {
	    TMTIME	tm ;
	    rs = tmtime_localtime(&tm,sip->daytime) ;
	    sip->year = (tm.year + TM_YEAR_BASE) ;
	    sip->isdst = tm.isdst ;
	    sip->gmtoff = tm.gmtoff ;
	}

	return rs ;
}
/* end subroutine (subinfo_year) */


static int subinfo_mkday(sip,m,cp,cl)
struct subinfo	*sip ;
int		m ;
const char	*cp ;
int		cl ;
{
	DAYOFMONTH	*dmp = &sip->dom ;

	int	rs = SR_OK ;

/* open the DAYOFMONTH database (manager?) if it is not already open */

	if (! sip->f.dom) {
	    rs = dayofmonth_start(dmp,sip->year) ;
	    sip->f.dom = (rs >= 0) ;
	}

	if (rs >= 0)
	    rs = dayofmonth_mkday(dmp,m,cp,cl) ;

#if	CF_DEBUGS
	debugprintf("caldays/subinfo_mkday: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_mkday) */


static int subinfo_transhol(sip,qp,sp,sl)
struct subinfo	*sip ;
CALDAYS_CITE	*qp ;
const char	*sp ;
int		sl ;
{
	HOLIDAYS_CUR	hcur ;
	HOLIDAYS	*holp ;

	CALDAYS		*op ;

	TMTIME		tm ;

	int	rs = SR_OK ;
	int	nl ;
	int	f_negative = FALSE ;
	int	f_inityear = FALSE ;
	int	f_found = FALSE ;
	int	f ;

	const char	*tp ;
	const char	*np ;


#if	CF_DEBUGS
	debugprintf("caldays/subinfo_transhol: >%t<\n",sp,sl) ;
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
	    debugprintf("caldays/subinfo_transhol: n=>%t<\n",np,nl) ;
	    debugprintf("caldays/subinfo_transhol: f_neg=%u\n",f_negative) ;
	} else
	    debugprintf("caldays/subinfo_transhol: *no_number*\n") ;
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
	        debugprintf("caldays/subinfo_transhol: "
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
	    debugprintf("caldays/subinfo_transhol: fq=>%t<\n",sp,sl) ;
#endif

	    if ((rs = holidays_curbegin(holp,&hcur)) >= 0) {

	        rs = holidays_fetchname(holp,sp,sl,&hcur,&hc,hbuf,hlen) ;
	        if (rs >= 0) {
	            f_found = TRUE ;
	            qp->m = hc.m ;
	            qp->d = hc.d ;
	        }

	        holidays_curend(holp,&hcur) ;
	    } /* end if (cursor) */

#if	CF_DEBUGS
	    debugprintf("caldays/subinfo_transhol: "
	        "holidays_fetchname() rs=%d\n",rs) ;
	    debugprintf("caldays/subinfo_transhol: un q=(%u:%u)\n",
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
	            debugprintf("caldays/subinfo_transhol: "
	                "adjusted q=(%u:%u)\n",qp->m,qp->d) ;
#endif

	        } /* end if (odays) */
	    } /* end if (got year) */

	} /* end if (day offset required) */

ret0:

#if	CF_DEBUGS
	debugprintf("caldays/subinfo_transhol: ret rs=%d f_found=%u\n",
	    rs,f_found) ;
#endif

	return (rs >= 0) ? f_found : rs ;
}
/* end subroutine (subinfo_transhol) */


static int subinfo_checkdname(sip,dname)
struct subinfo	*sip ;
const char	dname[] ;
{
	int	rs = SR_OK ;

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


static int entry_start(ep,qp,loff,llen)
CALDAYS_ENT	*ep ;
CALDAYS_CITE	*qp ;
int		loff, llen ;
{
	struct caldays_eline	*elp ;

	const int	ne = CALDAYS_NLE ;
	int	rs ;
	int	size ;


	if (ep == NULL)
	    return SR_FAULT ;

	memset(ep,0,sizeof(CALDAYS_ENT)) ;
	ep->cidx = -1 ;
	ep->m = qp->m ;
	ep->d = qp->d ;
	ep->voff = loff ;
	ep->vlen = llen ;

	size = ne * sizeof(struct caldays_eline) ;
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


static int entry_setidx(ep,cidx)
CALDAYS_ENT	*ep ;
int		cidx ;
{

	if (ep == NULL) return SR_FAULT ;

	ep->cidx = cidx ;
	return SR_OK ;
}

/* end subroutine (entry_setidx) */


static int entry_add(ep,loff,llen)
CALDAYS_ENT	*ep ;
uint		loff, llen ;
{
	struct caldays_eline	*elp ;

	int	rs = SR_OK ;
	int	ne ;
	int	size ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->e <= 0) return SR_NOTOPEN ;

	if ((ep->i < 0) || (ep->i > ep->e)) return SR_BADFMT ;

	if (ep->i == ep->e) {
	    ne = (ep->e * 2) + CALDAYS_NLE ;
	    size = ne * sizeof(struct caldays_eline) ;
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


static int entry_finish(ep)
CALDAYS_ENT	*ep ;
{
	int	rs = SR_OK ;
	int	rs1 ;

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


#if	CF_SAMECITE
static int entry_samecite(ep,oep)
CALDAYS_ENT	*ep ;
CALDAYS_ENT	*oep ;
{
	int	rs1 = SR_OK ;

	if ((ep->m == oep->m) && (ep->d == oep->d))
	    rs1 = 1 ;

	return rs1 ;
}
/* end subroutine (entry_samecite) */
#endif /* CF_SAMECITE */


static int entry_samehash(ep,op,oep)
CALDAYS_ENT	*ep ;
CALDAYS	*op ;
CALDAYS_ENT	*oep ;
{
	int	rs = SR_OK ;

/* the following checks (code) are not needed in the present implementation! */

	if ((rs >= 0) && (! ep->f.hash))
	    rs = entry_mkhash(ep,op) ;

	if ((rs >= 0) && (! oep->f.hash))
	    rs = entry_mkhash(oep,op) ;

/* we continue with the real (needed) work here */

	if (rs >= 0)
	    rs = (ep->hash == oep->hash) ? 1 : 0 ;

	return rs ;
}
/* end subroutine (entry_samehash) */


static int entry_mkhash(ep,op)
CALDAYS_ENT	*ep ;
CALDAYS	*op ;
{
	CALDAYS_CAL	*calp ;

	int	rs ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->e <= 0) return SR_NOTOPEN ;

	if (ep->lines == NULL) return SR_NOTOPEN ;

	if ((rs = vechand_get(&op->cals,ep->cidx,&calp)) >= 0) {
	    const char	*mp ;
	    if ((rs = cal_mapdata(calp,&mp)) >= 0) {
	        struct caldays_eline	*elp = ep->lines ;
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


static int entry_sethash(ep,hash)
CALDAYS_ENT	*ep ;
uint		hash ;
{

	ep->hash = hash ;
	ep->f.hash = TRUE ;
	return SR_OK ;
}
/* end subroutine (entry_sethash) */


static int entry_same(ep,op,oep)
CALDAYS_ENT	*ep ;
CALDAYS		*op ;
CALDAYS_ENT	*oep ;
{
	WORDER	w1, w2 ;

	int	rs = SR_OK ;
	int	c1l, c2l ;
	int	f = FALSE ;

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


static int entry_loadbuf(ep,mp,rbuf,rlen)
CALDAYS_ENT	*ep ;
const char	*mp ;
char		rbuf[] ;
int		rlen ;
{
	SBUF	b ;

	int	rs ;
	int	len = 0 ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    struct caldays_eline	*lines = ep->lines ;
	    int		nlines = ep->i ; /* number of line elements */
	    int		i ;
	    int		ll ;
	    const char	*lp ;

	    for (i = 0 ; i < nlines ; i += 1) {

	        if (i > 0) sbuf_char(&b,' ') ;

	        lp = (mp + lines[i].loff) ;
	        ll = lines[i].llen ;

#if	CF_DEBUGS
	        debugprintf("caldays/entry_loadbuf: i=%u loff=%u llen=%u\n",
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


static int mkbve_start(bvep,sip,ep)
CYIMK_ENT	*bvep ;
struct subinfo	*sip ;
CALDAYS_ENT	*ep ;
{
	int	rs ;
	int	nlines = 0 ;

	if (ep == NULL)
	    return SR_FAULT ;

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
	} /* end if ((entry_mkhash) */

	return (rs >= 0) ? nlines : rs ;
}
/* end subroutine (mkbve_start) */


static int mkbve_finish(bvep)
CYIMK_ENT	*bvep ;
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (bvep == NULL)
	    return SR_FAULT ;

	if (bvep->lines != NULL) {
	    rs1 = uc_free(bvep->lines) ;
	    if (rs >= 0) rs = rs1 ;
	    bvep->lines = NULL ;
	}

	return rs ;
}
/* end subroutine (mkbve_finish) */


int worder_start(wp,op,ep)
WORDER		*wp ;
CALDAYS		*op ;
CALDAYS_ENT	*ep ;
{
	CALDAYS_CAL	*calp ;

	int	rs ;

	if ((rs = vechand_get(&op->cals,ep->cidx,&calp)) >= 0) {
	    const char	*mp ;
	    if ((rs = cal_mapdata(calp,&mp)) >= 0) {
	        struct caldays_eline	*lines = ep->lines ;
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
int worder_finish(wp)
WORDER		*wp ;
{
	if (wp == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (worder_finish) */


int worder_get(wp,rpp)
WORDER		*wp ;
const char	**rpp ;
{
	int	cl = 0 ;

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


static int dayofmonth_mkday(dmp,m,cp,cl)
DAYOFMONTH	*dmp ;
uint		m ;
const char	*cp ;
int		cl ;
{
	uint	ch ;

	int	rs = SR_NOTFOUND ;
	int	wday ;
	int	oday ;
	int	mday = 0 ;


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
	debugprintf("caldays/dayofmonth_mkday: m=%u >%t<\n",m,cp,cl) ;
#endif

	ch = (cp[0] & 0xff) ;

	if (isdigitlatin(ch)) {
	    rs = cfdeci(cp,cl,&mday) ;
#if	CF_DEBUGS
	    debugprintf("caldays/dayofmonth_mkday: digit_day rs=%d\n",
	        rs) ;
#endif
	} else if (cl >= 3) {
	    if ((wday = matcasestr(days,cp,3)) >= 0) {
	        cp += 3 ;
	        cl -= 3 ;
	        oday = matocasestr(daytypes,2,cp,cl) ;
#if	CF_DEBUGS
	        debugprintf("caldays/dayofmonth_mkday: "
	            "matpcasestr() oday=%d\n",oday) ;
#endif
	        if (oday >= 0) {
	            rs = dayofmonth_lookup(dmp,m,wday,oday) ;
	            mday = rs ;
	        }
#if	CF_DEBUGS
	        debugprintf("caldays/dayofmonth_mkday: "
	            "dayofmonth_lookup() rs=%d\n", rs) ;
#endif
	    } else
	        rs = SR_ILSEQ ;
	} else
	    rs = SR_ILSEQ ;

ret0:

#if	CF_DEBUGS
	debugprintf("caldays/dayofmonth_mkday: ret rs=%d mday=%u\n",rs,mday) ;
#endif

	return (rs >= 0) ? mday : rs ;
}
/* end subroutine (dayofmonth_mkday) */


static int isempty(lp,ll)
const char	*lp ;
int		ll ;
{
	int	f = FALSE ;

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


static int mkmonth(cp,cl)
const char	*cp ;
int		cl ;
{
	int	rs ;
	int	v ;

	rs = cfdeci(cp,cl,&v) ;
	v -= 1 ;

	return (rs >= 0) ? v : rs ;
}
/* end subroutine (mkmonth) */


/* for use with 'vecobj_sort(3dam)' or similar */
static int vrcmp(v1p,v2p)
const void	*v1p, *v2p ;
{
	CALDAYS_ENT	*e1p, **e1pp = (CALDAYS_ENT **) v1p ;
	CALDAYS_ENT	*e2p, **e2pp = (CALDAYS_ENT **) v2p ;

	int	rc ;


	if (*e1pp == NULL) {
	    rc = 1 ;
	    goto ret0 ;
	}

	if (*e2pp == NULL) {
	    rc = -1 ;
	    goto ret0 ;
	}

	e1p = *e1pp ;
	e2p = *e2pp ;

	rc = (e1p->m - e2p->m) ;
	if (rc == 0)
	    rc = (e1p->d - e2p->d) ;

ret0:
	return rc ;
}
/* end subroutine (vrcmp) */


