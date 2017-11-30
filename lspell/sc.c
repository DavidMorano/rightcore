/* spellchecks */

/* SPELLCHECKS object implementation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGCUR	0		/* debug cursor operation */
#define	CF_SAFE		1		/* normal safety */
#define	CF_SAFE2	1		/* extra safety */
#define	CF_EMPTYTERM	1		/* terminate entry on empty line */
#define	CF_TMPPRNAME	1		/* put under a PRNAME in /tmp */
#define	CF_SAMECITE	0		/* same entry citation? */
#define	CF_ALREADY	1		/* do not allow duplicate results */
#define	CF_TRANSHOL	1		/* translate holidays */
#define	CF_GETPWUSER	1		/* use |getpwusername(3dam)| */


/* revision history:

	- 2008-10-01, David A­D­ Morano

	This object module was originally written.


*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module manages access and maintenance for the spell-check facility.
        This facility may contain several spelling lists that may need to have
        their indices rebuilt as needed.


*******************************************************************************/


#define	SPELLCHECKS_MASTER	1


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
#include	<endianstr.h>
#include	<getbufsize.h>
#include	<estrings.h>
#include	<vecobj.h>
#include	<sbuf.h>
#include	<char.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<vechand.h>
#include	<tmtime.h>
#include	<holidays.h>
#include	<getxusername.h>
#include	<fsdir.h>
#include	<paramfile.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"spellchecks.h"
#include	"strlist.h"
#include	"strlistmk.h"


/* local defines */

#define	SPELLCHECKS_DBSUF	"calendar"
#define	SPELLCHECKS_NLE		1	/* default number line entries */
#define	SPELLCHECKS_DMODE	0777
#define	SPELLCHECKS_DBDIR	"share/calendar"
#define	SPELLCHECKS_LE		struct spellchecks_list
#define	SPELLCHECKS_LEF		struct spellchecks_lflags
#define	SPELLCHECKS_CD		struct spellchecks_cachedir
#define	SPELLCHECKS_CDF		struct spellchecks_cdflags

#define	STRDESC			struct strdesc
#define	SUBINFO			struct subinfo
#define	SUBINFO_FL		struct subinfo_flags
#define	CONFIG			struct config
#define	CACHEDIR		struct spellchecks_cachedir
#define	CACHEDIR_NFIELDS	2
#define	DB			struct spellchecks_list
#define	DB_NFIELDS		3

#undef	WORDER
#define	WORDER			struct worder

#define	IDXDNAME	".spellchecks"
#define	IDXSUF		"cyi"

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	KBUFLEN
#define	KBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	PBUFLEN
#define	PBUFLEN		(KBUFLEN + VBUFLEN + MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(4 * MAXPATHLEN)
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
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sichr(const char *,int,int) ;
extern int	sibreak(const char *,int,const char *) ;
extern int	siskipwhite(const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matocasestr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	matpcasestr(const char **,int,const char *,int) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	mktmpuserdir(char *,const char *,const char *,mode_t) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	prsetfname(cchar *,char *,cchar *,int,cchar *,cchar *,cchar *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_log(time_t,char *) ;


/* local structures */

struct strdesc {
	const char	*sp ;
	int		sl ;
} ;

struct config {
	SUBINFO		*sip ;
	PARAMFILE	p ;
	EXPCOOK		c ;
	uint		f_p:1 ;
	uint		f_c:1 ;
} ;

struct subinfo_flags {
	uint		stores:1 ;
	uint		dbs:1 ;
	uint		cachedirs:1 ;
	uint		logsize:1 ;
	uint		lfname:1 ;
	uint		id:1 ;
	uint		dom:1 ;
	uint		hols:1 ;
} ;

struct subinfo {
	vecstr		stores ;
	vechand		dbs ;
	vechand		cachedirs ;
	IDS		id ;
	SUBINFO_FL	init, f ;
	SUBINFO_FL	open ;
	SPELLCHECKS	*op ;
	CONFIG		*cfp ;
	const char	*pr ;
	const char	*sn ;
	const char	*dbfname ;
	const char	*tmpdname ;
	const char	*tudname ;
	const char	*userhome ;
	const char	*lfname ;
	time_t		daytime ;
	int		logsize ;
	char		username[USERNAMELEN + 1] ;
} ;

struct spellchecks_lflags {
	uint		open:1 ;
} ;

struct spellchecks_list {
	const char	*a ;		/* allocation */
	const char	*idxname ;	/* index file name */
	const char	*dbfname ;	/* DB (actual list) file-name */
	const char	*cdname ;	/* cache directory name */
	SPELLCHECKS_LEF	f ;
	STRLIST		idx ;		/* the STRLIST object */
} ;

struct spellchecks_cdflags {
	uint		dummy:1 ;
} ;

struct spellchecks_cachedir {
	const char	*sname ;	/* symbolic-name */
	const char	*dname ;	/* directory-name */
	const char	*a ;		/* memory-allocation */
} ;

struct spellchecks_calflags {
	uint		vind:1 ;
} ;

struct spellchecks_cal {
	const char	*dirname ;
	const char 	*calname ;		/* DB file-name */
	const char	*mapdata ;		/* DB memory-map address */
	struct spellchecks_calflags	f ;
	time_t		ti_db ;			/* DB file modification */
	time_t		ti_map ;		/* DB map */
	time_t		ti_lastcheck ;		/* DB last check */
	time_t		ti_vind ;		/* verse-index */
	size_t		filesize ;		/* DB file size */
	size_t		mapsize ;		/* DB map length */
	int		nentries ;		/* DB entries */
	int		cidx ;			/* ordinal */
} ;

struct spellchecks_eline {
	uint		loff ;
	uint		llen ;
} ;

struct spellchecks_eflags {
	uint		hash:1 ;
} ;

struct spellchecks_e {
	struct spellchecks_eline	*lines ;
	struct spellchecks_eflags	f ;
	uint		voff ;
	uint		vlen ;
	uint		hash ;
	int		e, i ;
	int		cidx ;
	uchar		m, d ;
} ;

struct spellchecks_citer {
	int		i, e ;
} ;

struct worder {
	struct spellchecks_eline	*lines ;
	const char	*mp ;
	const char	*sp ;
	int		sl ;
	int		i ;
	int		nlines ;
} ;


/* forward references */

static int	spellchecks_dirnamescreate(SPELLCHECKS *,const char **) ;
static int	spellchecks_dirnamesdestroy(SPELLCHECKS *) ;

static int	spellchecks_checkupdate(SPELLCHECKS *,time_t) ;
static int	spellchecks_loadbuf(SPELLCHECKS *,SPELLCHECKS_ENT *,
			char *,int) ;
static int	spellchecks_tmpfrees(SPELLCHECKS *) ;
static int	spellchecks_calscreate(SPELLCHECKS *,struct subinfo *,
			const char **) ;
static int	spellchecks_calscreater(SPELLCHECKS *,struct subinfo *,
			const char *,const char **) ;
static int	spellchecks_calsdestroy(SPELLCHECKS *) ;
static int	spellchecks_listcreate(SPELLCHECKS *,struct subinfo *,
			const char *,const char *) ;
static int	spellchecks_listdestroy(SPELLCHECKS *,SPELLCHECKS_CAL *) ;

#ifdef	COMMENT
static int	spellchecks_mksysvarsi(SPELLCHECKS *,const char *) ;
#endif

static int	spellchecks_resultfins(SPELLCHECKS *,SPELLCHECKS_CUR *) ;
static int	spellchecks_calcite(SPELLCHECKS *,vecobj *,SPELLCHECKS_CAL *,
			SPELLCHECKS_QUERY *) ;
static int	spellchecks_mkresults(SPELLCHECKS *,vecobj *,SPELLCHECKS_CUR *) ;
static int	spellchecks_already(SPELLCHECKS *,vecobj *,
			SPELLCHECKS_ENT *) ;

#if	CF_DEBUGS && CF_DEBUGCUR
static int	spellchecks_debugcur(SPELLCHECKS *,vecobj *,const char *) ;
#endif

static int	cal_open(SPELLCHECKS_CAL *,struct subinfo *,int,
			const char *,const char *) ;
static int	cal_close(SPELLCHECKS_CAL *) ;
static int	cal_dbloadbegin(SPELLCHECKS_CAL *,struct subinfo *) ;
static int	cal_dbloadend(SPELLCHECKS_CAL *) ;
static int	cal_indopen(SPELLCHECKS_CAL *,struct subinfo *) ;
static int	cal_dbmapcreate(SPELLCHECKS_CAL *,time_t) ;
static int	cal_dbmapdestroy(SPELLCHECKS_CAL *) ;
static int	cal_indopenperm(SPELLCHECKS_CAL *,struct subinfo *) ;
static int	cal_indopentmp(SPELLCHECKS_CAL *,struct subinfo *) ;

#ifdef	COMMENT
static int	cal_idxset(SPELLCHECKS_CAL *,int) ;
#endif

static int	cal_idxget(SPELLCHECKS_CAL *) ;
static int	cal_indopencheck(SPELLCHECKS_CAL *,const char *,int,int) ;
static int	cal_mkdirs(SPELLCHECKS_CAL *,const char *,int) ;
static int	cal_audit(SPELLCHECKS_CAL *) ;

static int	cal_indmk(SPELLCHECKS_CAL *,struct subinfo *,const char *,
			int,time_t) ;
static int	cal_indmkdata(SPELLCHECKS_CAL *,struct subinfo *,const char *,
			mode_t,int) ;
static int	cal_indclose(SPELLCHECKS_CAL *) ;

static int	cal_loadbuf(SPELLCHECKS_CAL *,SPELLCHECKS_ENT *,char *,int) ;
static int	cal_mapdata(SPELLCHECKS_CAL *,const char **) ;

static int	subinfo_start(struct subinfo *,SPELLCHECKS *,
			const char *,const char *) ;
static int	subinfo_finish(struct subinfo *) ;
static int	subinfo_setentry(SUBINFO *,const char **,const char *,int) ;
static int	subinfo_userbegin(SUBINFO *) ;
static int	subinfo_userend(SUBINFO *) ;
static int	subinfo_confbegin(SUBINFO *,CONFIG *) ;
static int	subinfo_confread(SUBINFO *) ;
static int	subinfo_confend(SUBINFO *) ;
static int	subinfo_cachedir(SUBINFO *,CACHEDIR *) ;
static int	subinfo_cachedirfins(SUBINFO *) ;
static int	subinfo_db(SUBINFO *,DB *) ;
static int	subinfo_dbfins(SUBINFO *) ;
static int	subinfo_ids(struct subinfo *) ;
static int	subinfo_cachedir(SUBINFO *,const char *,int) ;
static int	subinfo_tmpuserdir(struct subinfo *) ;
static int	subinfo_mkdirnames(struct subinfo *) ;
static int	subinfo_havedir(struct subinfo *,char *) ;
static int	subinfo_loadnames(struct subinfo *,vecstr *,const char *) ;
static int	subinfo_havestart(struct subinfo *,
			SPELLCHECKS_QUERY *,const char *,int) ;
static int	subinfo_year(struct subinfo *) ;
static int	subinfo_mkday(struct subinfo *,int,const char *,int) ;
static int	subinfo_transhol(struct subinfo *,SPELLCHECKS_CITE *,
			const char *,int) ;
static int	subinfo_checkdname(struct subinfo *,const char *) ;

#ifdef	COMMENT
static int	subinfo_username(struct subinfo *) ;
#endif

static int	cachedir_start(CACHEDIR *,STRDESC *,int) ;
static int	cachedir_finish(CACHEDIR *) ;

static int	db_start(DB *,STRDESC *,int) ;
static int	db_finish(DB *) ;

static int	entry_start(SPELLCHECKS_ENT *,SPELLCHECKS_CITE *,int,int) ;
static int	entry_setidx(SPELLCHECKS_ENT *,int) ;
static int	entry_add(SPELLCHECKS_ENT *,uint,uint) ;
static int	entry_finish(SPELLCHECKS_ENT *) ;
static int	entry_mkhash(SPELLCHECKS_ENT *,SPELLCHECKS *) ;
static int	entry_sethash(SPELLCHECKS_ENT *,uint) ;
static int	entry_samehash(SPELLCHECKS_ENT *,SPELLCHECKS *,
			SPELLCHECKS_ENT *) ;
static int	entry_same(SPELLCHECKS_ENT *,SPELLCHECKS *,SPELLCHECKS_ENT *) ;
static int	entry_loadbuf(SPELLCHECKS_ENT *,const char *,char *,int) ;

#if	CF_SAMECITE
static int	entry_samecite(SPELLCHECKS_ENT *,SPELLCHECKS *,
			SPELLCHECKS_ENT *) ;
#endif

static int	mkbve_start(CYIMK_ENT *,struct subinfo *,SPELLCHECKS_ENT *) ;
static int	mkbve_finish(CYIMK_ENT *) ;

static int	worder_start(WORDER *,SPELLCHECKS *,SPELLCHECKS_ENT *) ;
static int	worder_finish(WORDER *) ;
static int	worder_get(WORDER *,const char **) ;

static int	config_start(CONFIG *,SUBINFO *,const char *) ;
static int	config_read(CONFIG *) ;
static int	config_finish(CONFIG *) ;
static int	config_cookbegin(CONFIG *) ;
static int	config_cookend(CONFIG *) ;
static int	config_setlfname(CONFIG *,const char *,int) ;
static int	config_cachedir(CONFIG *,const char *,int) ;
static int	config_cachedirone(CONFIG *,STRDESC *,int,const char *,int) ;
static int	config_db(CONFIG *,const char *,int) ;
static int	config_dbone(CONFIG *,STRDESC *,int,const char *,int) ;

static int	isempty(const char *,int) ;

static int	vrcmp(const void *,const void *) ;


/* exported variables */

SPELLCHECKS_OBJ	spellchecks = {
	"spellchecks",
	sizeof(SPELLCHECKS),
	sizeof(SPELLCHECKS_CUR)
} ;


/* local variables */

static const char	*sched1[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;

static const char	*cparams[] = {
	"cachedir",
	"db",
	"logfile",
	"logsize",
	NULL
} ;

enum cparams {
	cparam_cachedir,
	cparam_db,
	cparam_logfile,
	cparam_logsize,
	cparam_overlast
} ;

static const char	*cooks[] = {
	"pr",
	"pn",
	"bn",
	NULL
} ;

enum cooks {
	cook_pr,
	cook_pn,
	cook_bn,
	cook_overlast
} ;


/* exported subroutines */


int spellchecks_start(op,pr,dbfname)
SPELLCHECKS	*op ;
const char	pr[] ;
const char	*dbfname[] ;
{
	SUBINFO	si, *sip = &si ;
	int	rs ;
	int	c = 0 ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;
#endif

#if	CF_DEBUGS
	debugprintf("spellchecks_start: pr=%s\n",pr) ;
#endif

	if (pr == NULL)
	    return SR_FAULT ;

	if (pr[0] == '\0')
	    return SR_INVALID ;

	memset(op,0,sizeof(SPELLCHECKS)) ;
	op->pr = pr ;
	op->tmpdname = TMPDNAME ;

	if ((rs = subinfo_start(sip,op,pr,dbfname)) >= 0) {
	    c = rs ;
	    op->magic = SPELLCHECKS_MAGIC ;
	    subinfo_finish(sip) ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("spellchecks_start: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (spellchecks_start) */


int spellchecks_finish(op)
SPELLCHECKS	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SPELLCHECKS_MAGIC)
	    return SR_NOTOPEN ;
#endif

	rs1 = spellchecks_listfins(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&op->lists) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("spellchecks_finish: ret rs=%d\n",rs) ;
#endif

	op->nlists = 0 ;
	op->magic = 0 ;
	return rs ;
}
/* end subroutine (spellchecks_finish) */


int spellchecks_count(op)
SPELLCHECKS	*op ;
{
	int	rs ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SPELLCHECKS_MAGIC)
	    return SR_NOTOPEN ;
#endif

	rs = op->nlists ;

	return rs ;
}
/* end subroutine (spellchecks_count) */


int spellchecks_audit(op)
SPELLCHECKS	*op ;
{
	SPELLCHECKS_LE	*lep ;

	int	rs = SR_OK ;
	int	i ;
	int	c = 0 ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SPELLCHECKS_MAGIC)
	    return SR_NOTOPEN ;
#endif

	for (i = 0 ; vechand_get(&op->lists,i,&lep) >= 0 ; i += 1) {
	    STRLIST	*slp ;
	    if (lep == NULL) continue ;
	    slp = &lep->sl ;
	    c += 1 ;
	    rs = strlist_audit(slp) ;
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (spellchecks_audit) */


int spellchecks_curbegin(op,curp)
SPELLCHECKS	*op ;
SPELLCHECKS_CUR	*curp ;
{


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SPELLCHECKS_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (curp == NULL)
	    return SR_FAULT ;

	memset(curp,0,sizeof(SPELLCHECKS_CUR)) ;

	op->ncursors += 1 ;

	curp->i = -1 ;
	curp->magic = SPELLCHECKS_MAGIC ;
	return SR_OK ;
}
/* end subroutine (spellchecks_curbegin) */


int spellchecks_curend(op,curp)
SPELLCHECKS	*op ;
SPELLCHECKS_CUR	*curp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SPELLCHECKS_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (curp == NULL)
	    return SR_FAULT ;

	if (curp->magic != SPELLCHECKS_MAGIC)
	    return SR_NOTOPEN ;

	if (curp->results != NULL) {
	    rs1 = spellchecks_resultfins(op,curp) ;
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
/* end subroutine (spellchecks_curend) */


static int spellchecks_resultfins(op,curp)
SPELLCHECKS	*op ;
SPELLCHECKS_CUR	*curp ;
{
	SPELLCHECKS_ENT	*ep = (SPELLCHECKS_ENT *) curp->results ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep != NULL) {
	    int	i ;
	    for (i = 0 ; i < curp->nresults ; i += 1) {
		rs1 = entry_finish(ep + i) ;
		if (rs >= 0) rs = rs1 ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (spellchecks_resultfins) */


int spellchecks_lookcite(op,curp,qvp)
SPELLCHECKS	*op ;
SPELLCHECKS_CUR	*curp ;
SPELLCHECKS_QUERY	*qvp ;
{
	SPELLCHECKS_CAL	*calp ;
	vecobj		res ;
	int		rs = SR_OK ;
	int		i ;
	int		opts ;
	int		size ;
	int		c = 0 ;

#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SPELLCHECKS_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;
	if (qvp == NULL) return SR_FAULT ;

	if (curp->magic != SPELLCHECKS_MAGIC) return SR_NOTOPEN ;

	if (curp->results != NULL) {
	    spellchecks_resultfins(op,curp) ;
	    uc_free(curp->results) ;
	    curp->results = NULL ;
	}

	opts = 0 ;
	opts |= VECOBJ_OORDERED ;
	opts |= VECOBJ_OSTATIONARY ;
	size = sizeof(SPELLCHECKS_ENT) ;
	rs = vecobj_start(&res,size,0,opts) ;
	if (rs < 0)
	    goto ret0 ;

	for (i = 0 ; vechand_get(&op->cals,i,&calp) >= 0 ; i += 1) {
	    if (calp == NULL) continue ;

	    rs = spellchecks_calcite(op,&res,calp,qvp) ;
	    c += rs ;

#if	CF_DEBUGS
	debugprintf("spellchecks_lookcite: i=%u spellchecks_calcite() rs=%d\n",
		i,rs) ;
#endif

	    if (rs < 0)
	        break ;

	} /* end for */

	if (rs >= 0) {

	    rs = spellchecks_mkresults(op,&res,curp) ;

#if	CF_DEBUGS
	debugprintf("spellchecks_lookcite: spellchecks_mkresults() rs=%d\n",rs) ;
#endif

	}

	if ((rs < 0) || (c > 0)) {
	    SPELLCHECKS_ENT	*ep ;
	    for (i = 0 ; vecobj_get(&res,i,&ep) >= 0 ; i += 1) {
		if (ep == NULL) continue ;
		entry_finish(ep) ;
	    }
	}

ret1:
	vecobj_finish(&res) ;

ret0:

#if	CF_DEBUGS
	debugprintf("spellchecks_lookcite: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (spellchecks_lookcite) */


int spellchecks_read(op,curp,qvp,rbuf,rlen)
SPELLCHECKS	*op ;
SPELLCHECKS_CUR	*curp ;
SPELLCHECKS_CITE	*qvp ;
char		*rbuf ;
int		rlen ;
{
	SPELLCHECKS_ENT	*ep ;

	int	rs = SR_OK ;
	int	i ;
	int	len = 0 ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SPELLCHECKS_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;
	if (qvp == NULL) return SR_FAULT ;

	if (curp->magic != SPELLCHECKS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("spellchecks_read: entered\n") ;
#endif

	if (curp->results == NULL) {
	    rs = SR_NOTFOUND ;
	    goto ret0 ;
	}

	i = curp->i ;

#if	CF_DEBUGS
	debugprintf("spellchecks_read: c_i=%d\n",i) ;
#endif

	if (i < 0) {
	    rs = SR_NOTFOUND ;
	    goto ret0 ;
	}

	if (i >= curp->nresults) {
	    rs = SR_NOTFOUND ;
	    goto ret0 ;
	}

	ep = (SPELLCHECKS_ENT *) curp->results ;
	qvp->m = ep->m ;
	qvp->d = ep->d ;
	if (rbuf != NULL) {

#if	CF_DEBUGS
	debugprintf("spellchecks_read: spellchecks_loadbuf()\n") ;
#endif

	    rs = spellchecks_loadbuf(op,(ep + i),rbuf,rlen) ;
	    len = rs ;

#if	CF_DEBUGS
	debugprintf("spellchecks_read: spellchecks_loadbuf() rs=%d\n",rs) ;
#endif

	} /* end if */

	if (rs >= 0)
	    curp->i = (i + 1) ;

ret0:

#if	CF_DEBUGS
	debugprintf("spellchecks_read: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (spellchecks_read) */


/* ARGSUSED */
int spellchecks_check(op,daytime)
SPELLCHECKS	*op ;
time_t		daytime ;
{
	int	rs = SR_OK ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SPELLCHECKS_MAGIC)
	    return SR_NOTOPEN ;
#endif

#ifdef	COMMENT
	rs = spellchecks_checkupdate(op,daytime) ;
#endif

	return rs ;
}
/* end subroutine (spellchecks_check) */


/* private subroutines */


static int spellchecks_dirnamescreate(op,dirnames)
SPELLCHECKS	*op ;
const char	**dirnames ;
{
	int	rs = SR_OK ;
	int	strsize ;
	int	size ;
	int	i ;

	char	*p ;
	char	*sp ;


	if (dirnames == NULL)
	    goto ret0 ;

	strsize = 1 ;
	for (i = 0 ; dirnames[i] != NULL ; i += 1) {
	    strsize += (strlen(dirnames[i]) + 1) ;
	} /* end if */

	size = (i + 1) * sizeof(char *) ;
	rs = uc_malloc(size,&p) ;
	if (rs < 0)
	    goto bad0 ;

	op->dirnames = (char **) p ;
	rs = uc_malloc(strsize,&p) ;
	if (rs < 0)
	    goto bad1 ;

	op->dirstrtab = p ;
	sp = p ;
	*sp++ = '\0' ;
	for (i = 0 ; dirnames[i] != NULL ; i += 1) {
	    op->dirnames[i] = sp ;
	    sp = strwcpy(sp,dirnames[i],-1) + 1 ;
	} /* end for */
	op->dirnames[i] = NULL ;

ret0:
	return rs ;

/* bad stuff */
bad1:
	uc_free(op->dirnames) ;
	op->dirnames = NULL ;

bad0:
	goto ret0 ;
}
/* end subroutine (spellchecks_dirnamescreate) */


static int spellchecks_dirnamesdestroy(op)
SPELLCHECKS	*op ;
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
/* end subroutine (spellchecks_dirnamesdestroy) */


static int spellchecks_calcite(op,rlp,calp,qvp)
SPELLCHECKS	*op ;
vecobj		*rlp ;
SPELLCHECKS_CAL	*calp ;
SPELLCHECKS_QUERY	*qvp ;
{
	SPELLCHECKS_ENT	e ;

	CYI		*cip ;
	CYI_CUR		ccur ;
	CYI_QUERY	cq ;
	CYI_ENT	ce ;

	uint	loff, llen ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	celen = CEBUFLEN ;
	int	cidx ;
	int	n, i ;
	int	c = 0 ;
	int	f_ent = FALSE ;
	int	f_already = FALSE ;

	char	cebuf[CEBUFLEN + 1] ;


	cip = &calp->vind ;
	rs = cal_idxget(calp) ;
	cidx = rs ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("spellchecks_calcite: cidx=%d\n",cidx) ;
#endif

	memset(&cq,0,sizeof(CYI_QUERY)) ;
	cq.m = qvp->m ;
	cq.d = qvp->d ;

	if ((rs = cyi_curbegin(cip,&ccur)) >= 0) {

	    rs = cyi_lookcite(cip,&ccur,&cq) ;

#if	CF_DEBUGS
	debugprintf("spellchecks_calcite: cyi_lookcite() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

	        while (rs >= 0) {

#if	CF_DEBUGS && CF_DEBUGCUR
		debugprintf("spellchecks_calcite: cyi_read() c=%u\n",c) ;
		spellchecks_debugcur(op,rlp,"before cyi_read") ;
#endif

	            rs1 = cyi_read(cip,&ccur,&ce,cebuf,celen) ;

#if	CF_DEBUGS && CF_DEBUGCUR
		debugprintf("spellchecks_calcite: cyi_read() rs1=%d\n",rs1) ;
		spellchecks_debugcur(op,rlp,"after cyi_read") ;
#endif

	            if ((rs1 == SR_NOTFOUND) || (rs1 == 0))
		        break ;

		    rs = rs1 ;
		    if (rs < 0)
			break ;

		    if (rs1 > 0) {

			n = 0 ;
			for (i = 0 ; i < ce.nlines ; i += 1) {
			    loff = ce.lines[i].loff ;
			    llen = ce.lines[i].llen ;

#if	CF_DEBUGS
			debugprintf("spellchecks_calcite: i=%u loff=%u llen=%u\n",
				i,loff,llen) ;
#endif

			    n += 1 ;
			    if (! f_ent) {
	        		rs = entry_start(&e,qvp,loff,llen) ;
				if (rs >= 0) {
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
			    rs = spellchecks_already(op,rlp,&e) ;
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
	} /* end if */

ret0:

#if	CF_DEBUGS
	debugprintf("spellchecks_calcite: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (spellchecks_calcite) */


static int spellchecks_already(op,rlp,ep)
SPELLCHECKS	*op ;
vecobj		*rlp ;
SPELLCHECKS_ENT	*ep ;
{
	SPELLCHECKS_ENT	*oep ;

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
	    if (f) 
		break ;

	    if (rs < 0)
		break ;

	} /* end for */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (spellchecks_already) */


static int spellchecks_mkresults(op,rlp,curp)
SPELLCHECKS	*op ;
vecobj		*rlp ;
SPELLCHECKS_CUR	*curp ;
{
	SPELLCHECKS_ENT	*rp ;
	SPELLCHECKS_ENT	*ep ;

	int	rs = SR_OK ;
	int	n ;
	int	i ;
	int	size ;
	int	c = 0 ;


#if	CF_DEBUGS
	debugprintf("spellchecks_mkresults: entered\n") ;
#endif

	vecobj_sort(rlp,vrcmp) ; /* sort results in ascending order */

	n = vecobj_count(rlp) ;
	if (n <= 0)
	    goto ret0 ;

	size = n * sizeof(SPELLCHECKS_ENT) ;
	rs = uc_malloc(size,&rp) ;
	if (rs < 0)
	    goto ret0 ;

	for (i = 0 ; vecobj_get(rlp,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;

#if	CF_DEBUGS
	{
	    struct spellchecks_eline	*lines = ep->lines ;
		int	j ;
	    if (lines != NULL) {
		for (j = 0 ; j < ep->i ; j += 1) {
		debugprintf("spellchecks_mkresults: i=%u j=%u loff=%u llen=%u\n",
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

ret0:

#if	CF_DEBUGS
	debugprintf("spellchecks_mkresults: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (spellchecks_mkresults) */


static int spellchecks_tmpfrees(op)
SPELLCHECKS	*op ;
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
/* end subroutine (spellchecks_tmpfrees) */


static int spellchecks_calscreate(op,sip,calnames)
SPELLCHECKS	*op ;
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
	        rs = spellchecks_calscreater(op,sip,dirnames[i],calnames) ;
	        c += rs ;
	        if (rs < 0) break ;
	    } /* end for (dirnames) */
	} /* end if (dirnames) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (spellchecks_calscreate) */


static int spellchecks_calscreater(op,sip,dirname,calnames)
SPELLCHECKS	*op ;
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
	debugprintf("spellchecks_calscreater: dirname=%s\n",dirname) ;
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
	debugprintf("spellchecks_calscreater: mid rs=%d n=%d\n",rs,n) ;
#endif

	if (rs >= 0)
	    rs = subinfo_ids(sip) ;

	if ((rs >= 0) && (names != NULL)) {

	    for (j = 0 ; names[j] != NULL ; j += 1) {
	        if (names[j][0] == '\0') continue ;

#if	CF_DEBUGS
		debugprintf("spellchecks_calscreater: spellchecks_listcreate()\n") ;
		debugprintf("spellchecks_calscreater: calname=>%s<\n",names[j]) ;
#endif

		rs = spellchecks_listcreate(op,sip,dirname,names[j]) ;
		c += rs ;
		if (rs < 0)
		    break ;

	    } /* end for (calnames) */

	} /* end if */

ret1:
	if (f_search)
	    vecstr_finish(&cals) ;

ret0:

#if	CF_DEBUGS
	debugprintf("spellchecks_calscreater: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (spellchecks_calscreater) */


static int spellchecks_listfins(op)
SPELLCHECKS	*op ;
{
	SPELLCHECKS_LE	*lep ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;


	for (i = 0 ; vechand_get(&op->lists,i,&lep) >= 0 ; i += 1) {
	    if (lep == NULL) continue ;
	    rs1 = spellchecks_listdestroy(op,lep) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end for */

	return rs ;
}
/* end subroutine (spellchecks_listfins) */


static int spellchecks_listcreate(op,sip,dirname,calname)
SPELLCHECKS	*op ;
struct subinfo	*sip ;
const char	dirname[] ;
const char	calname[] ;
{
	struct ustat	sb ;

	SPELLCHECKS_CAL	*calp ;

	const int	size = sizeof(SPELLCHECKS_CAL) ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	cidx ;
	int	f = FALSE ;

	const char	*suf = SPELLCHECKS_DBSUF ;

	char	cname[MAXNAMELEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;


	rs = snsds(cname,MAXNAMELEN,calname,suf) ;

	if (rs >= 0)
	    rs = mkpath2(tmpfname,dirname,cname) ;

	if (rs < 0)
	    goto bad0 ;

	rs1 = u_stat(tmpfname,&sb) ;

#if	CF_DEBUGS
	debugprintf("spellchecks_listcreate: u_stat() rs=%d\n",rs1) ;
#endif

	if (rs1 >= 0)
	    rs1 = sperm(&sip->id,&sb,R_OK) ;

#if	CF_DEBUGS
	debugprintf("spellchecks_listcreate: fn=%s rs=%d\n",tmpfname,rs1) ;
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
/* end subroutine (spellchecks_listcreate) */


static int spellchecks_listdestroy(op,lep)
SPELLCHECKS	*op ;
SPELLCHECKS_LE	*lep ;
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (lep->f.open) {
	    rs1 = strlist_close(&lep->sl) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lep->a != NULL) {
	    rs1 = uc_free(lep->a) ;
	    if (rs >= 0) rs = rs1 ;
	    lep->a = NULL ;
	}

	if ((rs1 = vechand_ent(&op->lists,lep)) >= 0) {
	    rs1 = vechand_del(&op->lists,rs1) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = uc_free(lep) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (spellchecks_listdestroy) */


static int spellchecks_loadbuf(op,ep,rbuf,rlen)
SPELLCHECKS	*op ;
SPELLCHECKS_ENT	*ep ;
char		rbuf[] ;
int		rlen ;
{
	SPELLCHECKS_CAL	*calp ;

	int	rs ;
	int	cidx ;


	cidx = ep->cidx ;

#if	CF_DEBUGS
	debugprintf("spellchecks_loadbuf: cidx=%d\n",cidx) ;
#endif

	rs = vechand_get(&op->cals,cidx,&calp) ;
	if (rs < 0)
	    goto ret0 ;

	rs = cal_loadbuf(calp,ep,rbuf,rlen) ;

ret0:

#if	CF_DEBUGS
	debugprintf("spellchecks_loadbuf: ret rs=%d\n",rs) ;
#endif

	return rs ;
} 
/* end subroutine (spellchecks_loadbuf) */


static int cal_open(calp,sip,cidx,dirname,calname)
SPELLCHECKS_CAL	*calp ;
struct subinfo	*sip ;
int		cidx ;
const char	dirname[] ;
const char	calname[] ;
{
	int	rs ;


	memset(calp,0,sizeof(SPELLCHECKS_CAL)) ;

	calp->cidx = cidx ;
	rs = uc_mallocstrw(dirname,-1,&calp->dirname) ;
	if (rs < 0)
	    goto bad0 ;

	rs = uc_mallocstrw(calname,-1,&calp->calname) ;
	if (rs < 0)
	    goto bad1 ;

	rs = cal_dbloadbegin(calp,sip) ;
	if (rs < 0)
	    goto bad2 ;

ret0:

#if	CF_DEBUGS
	debugprintf("cal_open: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad2:
	uc_free(calp->calname) ;

bad1:
	uc_free(calp->dirname) ;

bad0:
	goto ret0 ;
}
/* end subroutine (cal_open) */


static int cal_close(calp)
SPELLCHECKS_CAL	*calp ;
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
SPELLCHECKS_CAL	*calp ;
struct subinfo	*sip ;
{
	int	rs ;


	rs = cal_dbmapcreate(calp,sip->daytime) ;
	if (rs < 0)
	    goto bad0 ;

	rs = cal_indopen(calp,sip) ;
	if (rs < 0)
	    goto bad1 ;

ret0:
	return rs ;

/* bad stuff */
bad1:
	cal_dbmapdestroy(calp) ;

bad0:
	goto ret0 ;
}
/* end subroutine (cal_dbloadbegin) */


static int cal_dbloadend(calp)
SPELLCHECKS_CAL	*calp ;
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
SPELLCHECKS_CAL	*calp ;
time_t		daytime ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	fd ;

	char	cname[MAXNAMELEN + 1] ;
	char	dbfname[MAXPATHLEN + 1] ;


	rs = snsds(cname,MAXNAMELEN,calp->calname,SPELLCHECKS_DBSUF) ;
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
SPELLCHECKS_CAL	*calp ;
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
SPELLCHECKS_CAL	*calp ;
struct subinfo	*sip ;
{
	int	rs ;


#if	CF_DEBUGS
	debugprintf("cal_indopen: entered\n") ;
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
SPELLCHECKS_CAL	*calp ;
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
	    rs = cal_mkdirs(calp,idname,SPELLCHECKS_DMODE) ;

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
SPELLCHECKS_CAL	*calp ;
struct subinfo	*sip ;
{
	SPELLCHECKS	*op = sip->op ;

	int	rs ;
	int	year = sip->year ;
	int	f_search = TRUE ;
	int	f_mkind = FALSE ;

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

	if (rs < 0)
	    goto ret0 ;

try:
	rs = cal_indopencheck(calp,idname,year,f_search) ;

#if	CF_DEBUGS
	debugprintf("cal_indopentmp: 1 cal_indopencheck() rs=%d\n",rs) ;
#endif

	if (rs == SR_NOTDIR) {
	    f_mkind = TRUE ;
	    rs = cal_mkdirs(calp,idname,SPELLCHECKS_DMODE) ;

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


static int cal_mkdirs(calp,dname,mode)
SPELLCHECKS_CAL	*calp ;
const char	dname[] ;
int		mode ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;


	mode &= S_IAMB ;
	rs = mkdirs(dname,mode) ;

	if (rs >= 0) {
	    rs = u_stat(dname,&sb) ;
	    if ((rs >= 0) && ((sb.st_mode & mode) != mode))
		rs = u_chmod(dname,mode) ;
	}

	return rs ;
}
/* end subroutine (cal_mkdirs) */


static int cal_indopencheck(calp,dname,year,f_search)
SPELLCHECKS_CAL	*calp ;
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
/* end subroutine (spellchecks_indopencheck) */


#ifdef	COMMENT

static int spellchecks_indopens(op,sip,oflags)
SPELLCHECKS	*op ;
struct subinfo	*sip ;
int		oflags ;
{
	int	rs = SR_NOENT ;
	int	i ;


	for (i = 0 ; indopens[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	debugprintf("spellchecks_indopens: i=%u\n",i) ;
#endif

	    rs = (*indopens[i])(op,sip,oflags) ;

	    if ((rs == SR_BADFMT) || (rs == SR_NOMEM))
		break ;

	    if (rs >= 0)
	        break ;

	} /* end for */

	return rs ;
}
/* end subroutine (spellchecks_indopens) */


static int spellchecks_indopenpr(op,sip,oflags)
SPELLCHECKS	*op ;
struct subinfo	*sip ;
int		oflags ;
{
	int	rs = SR_OK ;

	char	idname[MAXPATHLEN + 1] ;


	rs = mkpath3(idname,op->pr,VCNAME,IDXDNAME) ;
	if (rs < 0)
	    goto ret0 ;

	rs = spellchecks_indopendname(op,sip,idname,oflags) ;

ret0:
	return rs ;
}
/* end subroutine (spellchecks_indopenpr) */


static int spellchecks_indopentmp(op,sip,oflags)
SPELLCHECKS	*op ;
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
	    rs = spellchecks_indopendname(op,sip,idname,oflags) ;

ret0:
	return rs ;
}
/* end subroutine (spellchecks_indopentmp) */


static int spellchecks_indopendname(op,sip,dname,oflags)
SPELLCHECKS	*op ;
struct subinfo	*sip ;
const char	dname[] ;
int		oflags ;
{
	int	rs ;
	int	f_ok = FALSE ;
	int	f_mk = FALSE ;

	char	indname[MAXPATHLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("spellchecks_indopendname: dname=%s of=%05x\n",
		dname,oflags) ;
#endif

	rs = mkpath2(indname,dname,op->dbname) ;
	if (rs < 0)
	    goto ret0 ;

	if (oflags & O_CREAT) {

	    rs = spellchecks_indcheck(op,indname,sip->daytime) ;
	    f_ok = (rs > 0) ;

#if	CF_DEBUGS
	debugprintf("spellchecks_indopendname: "
		"spellchecks_indcheck() rs=%d f_ok=%u\n",
		rs,f_ok) ;
#endif

	    if (rs < 0)
	        goto ret0 ;

#ifdef	COMMENT
	    if ((rs < 0) || (! f_ok)) {
	        rs = spellchecks_mksysvarsi(op,dname) ;
	        if (rs >= 0) {
		    f_mk = TRUE ;
	            rs = spellchecks_indcheck(op,indname,sip->daytime) ;
	            f_ok = (rs > 0) ;
	        }
	    }
#endif /* COMMENT */

	    if ((rs < 0) || (! f_ok)) {
	        f_mk = TRUE ;
	        rs = spellchecks_indmk(op,sip,dname,sip->daytime) ;

#if	CF_DEBUGS
	    debugprintf("spellchecks_indopendname: "
			"spellchecks_indmk() rs=%d\n",
		rs) ;
#endif

	    } /* end if */

	    if (rs >= 0) {
	        rs = cyi_open(&op->vind,indname) ;
	        op->f.vind = (rs >= 0) ;

#if	CF_DEBUGS
	    debugprintf("spellchecks_indopendname: "
			"1 cyi_open() rs=%d\n",rs) ;
#endif

	    } /* end if */

	    if ((rs < 0) && (rs != SR_BADFMT) && (! f_mk)) {
	        rs = spellchecks_indmk(op,sip,dname,sip->daytime) ;
	        if (rs >= 0) {
		    rs = cyi_open(&op->vind,indname) ;
	            op->f.vind = (rs >= 0) ;

#if	CF_DEBUGS
	    debugprintf("spellchecks_indopendname: "
			"2 cyi_open() rs=%d\n",rs) ;
#endif

	        }
	    }

	} else {

	    rs = cyi_open(&op->vind,indname) ;
	    op->f.vind = (rs >= 0) ;

#if	CF_DEBUGS
	    debugprintf("spellchecks_indopendname: "
			"0 cyi_open() rs=%d\n",rs) ;
#endif

	} /* end if (open-only or open-create) */

ret0:

#if	CF_DEBUGS
	debugprintf("spellchecks_indopendname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (spellchecks_indopendname) */


static int spellchecks_indcheck(op,indname,daytime)
SPELLCHECKS	*op ;
const char	indname[] ;
time_t		daytime ;
{
	struct ustat	sb ;

	time_t	ti_ind ;

	int	rs ;
	int	rs1 ;
	int	f = FALSE ;

	char	indfname[MAXPATHLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("spellchecks_indcheck: indname=%s\n",indname) ;
#endif

	rs = mkfnamesuf2(indfname,indname,IDXSUF,ENDIANSTR) ;
	if (rs < 0)
	    goto ret0 ;

	rs1 = u_stat(indfname,&sb) ;

	ti_ind = sb.st_mtime ;
	if ((rs1 >= 0) && (op->ti_db > ti_ind))
	    rs1 = SR_TIMEDOUT ;

	if ((rs1 >= 0) && ((daytime - ti_ind) >= TO_FILEMOD))
	    rs1 = SR_TIMEDOUT ;

	f = (rs1 >= 0) ;

ret0:
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (spellchecks_indcheck) */

#endif /* COMMENT */


#if	CF_DEBUGS && CF_DEBUGCUR

static int spellchecks_debugcur(op,rlp,s)
SPELLCHECKS	*op ;
vecobj		*rlp ;
const char	s[] ;
{
	struct spellchecks_eline	*lines ;

	SPELLCHECKS_ENT		*ep ;

	int	rs = SR_OK ;
	int	n ;
	int	i, j ;


	n = vecobj_count(rlp) ;
	debugprintf("spellchecks_debugcur: %s n=%u\n",s,n) ;
	for (i = 0 ; vecobj_get(rlp,i,&ep) >= 0 ; i += 1) {
		lines = ep->lines ;
	   for (j = 0 ; j < ep->i ; j += 1) {
		debugprintf("spellchecks_debugcur: i=%u loff[%u]=%u\n",
			i,j,lines[j].loff) ;
		debugprintf("spellchecks_debugcur: i=%u llen[%u]=%u\n",
			i,j,lines[j].llen) ;
	    }
	} /* end for */

ret0:
	return rs ;
}
/* end subroutine (spellchecks_debugcur) */

#endif /* CF_DEBUGS */


static int cal_indmk(calp,sip,dname,f_tmp,daytime)
SPELLCHECKS_CAL	*calp ;
struct subinfo	*sip ;
const char	dname[] ;
int		f_tmp ;
time_t		daytime ;
{
	const mode_t	operms = 0664 ;
	int	rs = SR_OK ;
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
	debugprintf("spellchecks_indmk: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (cal_indmk) */


static int cal_indmkdata(calp,sip,dname,operms,f_tmp)
SPELLCHECKS_CAL	*calp ;
struct subinfo	*sip ;
const char	dname[] ;
mode_t		operms ;
int		f_tmp ;
{
	SPELLCHECKS_ENT	e ;
	SPELLCHECKS_QUERY	q ;

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
	debugprintf("spellchecks_indmkdata: cidx=%d\n",cidx) ;
#endif

	cn = calp->calname ;
	oflags = 0 ;
	year = sip->year ;
	rs = cyimk_open(&cyind,dname,cn,oflags,operms,year,f_tmp) ;

#if	CF_DEBUGS
	debugprintf("spellchecks_indmkdata: cyimk_open() rs=%d\n",rs) ;
#endif

	if (rs == SR_INPROGRESS)
	    goto retinprogress ;

	if (rs < 0)
	    goto ret0 ;

mkgo:
	mp = calp->mapdata ;
	ml = calp->mapsize ;

#if	CF_DEBUGS
	debugprintf("spellchecks_indmkdata: mp=%p ml=%d\n",mp,ml) ;
#endif

	while ((tp = strnchr(mp,ml,'\n')) != NULL) {

	    len = ((tp + 1) - mp) ;
	    lp = mp ;
	    ll = (len - 1) ;

	    if (! isempty(lp,ll)) {

#if	CF_DEBUGS
		debugprintf("spellchecks_indmkdata: line=>%t<\n",
			lp,strnlen(lp,MIN(ll,40))) ;
		if (ll > 40)
		debugprintf("spellchecks_indmkdata: cont=>%t<\n",
			(lp+40),strnlen((lp+40),MIN((ll-40),40))) ;
#endif

	        rs1 = subinfo_havestart(sip,&q,lp,ll) ;
		si = rs1 ;

#if	CF_DEBUGS
		debugprintf("spellchecks_indmkdata: subinfo_havestart() rs1=%d\n",
			rs1) ;
		debugprintf("spellchecks_indmkdata: q=(%u:%u)\n",q.m,q.d) ;
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
	debugprintf("spellchecks_indmkdata: ret rs=%d c=%u\n",rs,c) ;
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
SPELLCHECKS_CAL	*calp ;
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
SPELLCHECKS_CAL	*calp ;
int		cidx ;
{


	calp->cidx = cidx ;
	return SR_OK ;
}
/* end subroutine (cal_idxset) */

#endif /* COMMENT */


static int cal_idxget(calp)
SPELLCHECKS_CAL	*calp ;
{
	int	cidx = calp->cidx ;


	return cidx ;
}
/* end subroutine (cal_idxget) */


static int cal_audit(calp)
SPELLCHECKS_CAL	*calp ;
{
	int	rs = SR_OK ;


	rs = cyi_audit(&calp->vind) ;

	return rs ;
}
/* end subroutine (cal_audit) */


static int cal_loadbuf(calp,ep,rbuf,rlen)
SPELLCHECKS_CAL	*calp ;
SPELLCHECKS_ENT	*ep ;
char		rbuf[] ;
int		rlen ;
{
	int	rs ;

	const char	*mp ;

	if ((rs = cal_mapdata(calp,&mp)) >= 0)
	    rs = entry_loadbuf(ep,mp,rbuf,rlen) ;

#if	CF_DEBUGS
	debugprintf("spellchecks/cal_loadbuf: entry_loadbuf() rs=%d\n",rs) ;
#endif

	return rs ;
} 
/* end subroutine (cal_loadbuf) */


static int cal_mapdata(calp,mpp)
SPELLCHECKS_CAL	*calp ;
const char	**mpp ;
{
	int	rs ;


	if (mpp != NULL)
	    *mpp = calp->mapdata ;

	rs = calp->mapsize ;
	return rs ;
}
/* end subroutine (cal_mapdata) */


static int subinfo_start(sip,op,pr,dbfname)
struct subinfo	*sip ;
SPELLCHECKS	*op ;
const char	*pr ;
const char	*dbfname ;
{
	CONFIG	cf ;
	int	rs = SR_OK ;

	memset(sip,0,sizeof(struct subinfo)) ;
	sip->op = op ;
	sip->pr = pr ;
	sip->sn = SPELLCHECKS_SEARCHNAME ;
	sip->dbfname = dbfname ;

	if ((rs = ids_load(&sip->id)) >= 0) {
	    sip->open.id = TRUE ;
	    if ((rs = subinfo_confbegin(sip,&cf)) >= 0) {

	        rs = subinfo_confread(sip) ;

	        subinfo_confend(sip) ;
	    } /* end if (subinfo_conf) */
	    if (rs < 0) {
	        sip->open.id = FALSE ;
	        ids_release(&sip->id) ;
	    }
	} /* end if (ids) */

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(sip)
struct subinfo	*sip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = subinfo_dbfins(sip) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = subinfo_cachedirfins(sip) ;
	if (rs >= 0) rs = rs1 ;

	if (sip->userhome != NULL) {
	    rs1 = uc_free(sip->userhome) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->userhome = NULL ;
	}

	if (sip->open.dbs) {
	    sip->open.dbs = FALSE ;
	    rs1 = vechand_finish(&sip->dbs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sip->open.cachedirs) {
	    sip->open.cachedirs = FALSE ;
	    rs1 = vechand_finish(&sip->cachedirs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sip->open.stores) {
	    sip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&sip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sip->open.id) {
	    sip->open.id = FALSE ;
	    rs1 = ids_release(&sip->id) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_setentry(sip,epp,vp,vl)
struct subinfo	*sip ;
const char	**epp ;
const char	vp[] ;
int		vl ;
{
	int	rs = SR_OK ;
	int	vnlen = 0 ;


	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (! sip->open.stores) {
	    rs = vecstr_start(&sip->stores,4,0) ;
	    sip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;

	    if (*epp != NULL) oi = vecstr_findaddr(&sip->stores,*epp) ;

	    if (vp != NULL) {
		vnlen = strnlen(vp,vl) ;
	        if ((rs = vecstr_add(&sip->stores,vp,vnlen)) >= 0) {
	            rs = vecstr_get(&sip->stores,rs,epp) ;
	        } /* end if (added new entry) */
	    } /* end if (had a new entry) */

	    if ((rs >= 0) && (oi >= 0))
	        vecstr_del(&sip->stores,oi) ;

	} /* end if */

	return (rs >= 0) ? vnlen : rs ;
}
/* end subroutine (subinfo_setentry) */


#if	CF_GETPWUSER
static int subinfo_userbegin(SUBINFO *sip)
{
	int	rs = SR_OK ;

	if (sip->username[0] == '\0') {
	    struct passwd	pw ;
	    const int		pwlen = getbufsize(getbufsize_pw) ;
	    char		*pwbuf ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	        if ((rs = getpwusername(&pw,pwbuf,pwlen,-1)) >= 0) {
	            const int	ulen = USERNAMELEN ;
		    cchar	**vpp = &sip->userhome ;
		    strwcpy(sip->username,pw.pw_name,ulen) ;
		    rs = subinfo_setentry(sip,vpp,pw.pw_dir,-1) ;
		}
		uc_free(pwbuf) ;
	    } /* end if (memory-allocation) */
	} /* end if (needed) */

	return rs ;
}
/* end subroutine (subinfo_userbegin) */
#else /* CF_GETPWUSER */
static int subinfo_userbegin(SUBINFO *sip)
{
	const int	ulen = USERNAMELEN ;
	int		rs ;
	char		*ubuf = sip->username ;

	if ((rs = getusername(ubuf,ulen,-1)) >= 0) {
	    const int	hlen = MAXPATHLEN ;
	    char	hbuf[MAXPATHLEN+1] ;
	    if ((rs == getuserhome(hbuf,hlen,ubuf)) >= 0) {
		cchar	**vpp = &sip->userhome ;
		rs = subinfo_setentry(sip,vpp,hbuf,rs)) >= 0) {
	    }
	}

	return rs ;
}
/* end subroutine (subinfo_userbegin) */
#endif /* CF_GETPWUSER */


/* no need to really do anything here */
static int subinfo_userend(SUBINFO *sip)
{
	if (sip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (subinfo_userend) */


static int subinfo_confbegin(SUBINFO *sip,CONFIG *cfp)
{
	int	rs ;
	const char	*cfname = SPELLCHECKS_CONFNAME ;

	if ((rs = subinfo_userbegin(sip)) >= 0) {
	    if ((rs = config_start(cfp,sip,cfname)) >= 0) {
	        sip->cfp = cfp ;
	    } /* end if (config_start) */
	    if (rs < 0)
		subinfo_userend(sip) ;
	} /* end if (user) */

	return rs ;
}
/* end subroutine (subinfo_confbegin) */


static int subinfo_confend(SUBINFO *sip)
{
	CONFIG		*cfp = sip->cfp ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (cfp != NULL) {
	    rs1 = config_finish(cfp) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->cfp = NULL ;
	}

	rs1 = subinfo_userend(sip) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (subinfo_confend) */


static int subinfo_confread(SUBINF *sip)
{
	CONFIG	*cfp = sip->cfp ;
	int	rs = SR_OK ;

	if (cfp != NULL) {
	    rs = config_read(cfp) ;
	}

	return rs ;
}
/* end subroutine (subinfo_confread) */


static int subinfo_cachedir(SUBINFO *sip,CACHEDIR *cdp)
{
	int	rs = SR_OK ;

	if (! sip->open.cachedirs) {
	    if ((rs = vechand_start(&op->cachedirs,2,0)) >= 0) {
	        sip->open.cachedirs = TRUE ;
	    }
	}

	if (rs >= 0)
	    rs = vechand_add(&sip->cachedirs,cdp) ;

	return rs ;
}
/* end subroutine (subinfo_cachedir) */


static int subinfo_cachedirfins(SUBINFO *sip)
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (sip->open.cachedirs) {
	    CACHEDIR	*cdp ;
	    int	i ;
	    for (i = 0 ; vechand_get(&op->cachedirs,i,&cdp) >= 0 ; i += 1) {
		if (cdp == NULL) continue ;
		rs1 = cachedir_finish(cdp) ;
		if (rs >= 0) rs = rs1 ;
		rs1 = uc_free(cdp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end for */
	} /* end if (cachedirs) */

	return rs ;
}
/* end subroutine (subinfo_cachedirfins) */


static int subinfo_db(SUBINFO *sip,DB *dbp)
{
	int	rs = SR_OK ;

	if (! sip->open.dbs) {
	    if ((rs = vechand_start(&op->dbs,2,0)) >= 0) {
	        sip->open.dbs = TRUE ;
	    }
	}

	if (rs >= 0)
	    rs = vechand_add(&sip->dbs,dbp) ;

	return rs ;
}
/* end subroutine (subinfo_db) */


static int subinfo_dbfins(SUBINFO *sip)
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (sip->open.dbs) {
	    DB		*dbp ;
	    int		i ;
	    for (i = 0 ; vechand_get(&op->dbs,i,&dbp) >= 0 ; i += 1) {
		if (dbp == NULL) continue ;
		rs1 = db_finish(dbp) ;
		if (rs >= 0) rs = rs1 ;
		rs1 = uc_free(dbp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end for */
	} /* end if (dbs) */

	return rs ;
}
/* end subroutine (subinfo_dbfins) */


static int subinfo_mkdirnames(sip)
struct subinfo	*sip ;
{
	SPELLCHECKS	*op = sip->op ;

	int	rs = SR_OK ;
	int	tl ;
	int	c = 0 ;

	const char	*sharedname = SPELLCHECKS_DBDIR ;
	char	tmpdname[MAXPATHLEN + 1] ;


	rs = subinfo_username(sip) ;
	if (rs < 0)
	    goto ret0 ;

	rs = vecstr_start(&sip->defdirs,0,0) ;
	sip->f.defdirs = (rs >= 0) ;
	if (rs < 0)
	    goto ret0 ;

/* user-home area */

	if (rs >= 0) {
	    if ((rs = mkpath2(tmpdname,sip->userhome,sharedname)) >= 0) {
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

ret1:
	if (rs < 0) {
	    if (sip->f.defdirs) {
		sip->f.defdirs = FALSE ;
		vecstr_finish(&sip->defdirs) ;
	    }
	}

ret0:
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


	if ((rs1 = u_stat(tmpdname,&sb)) >= 0)
	    f = S_ISDIR(sb.st_mode) ? 1 : 0 ;

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


static int subinfo_tmpuserdir(sip)
struct subinfo	*sip ;
{
	const mode_t	dmode = 0775 ;
	int		rs ;
	int		dl ;
	char		tmpdname[MAXPATHLEN + 1] ;

	rs = subinfo_username(sip) ;

	if ((rs >= 0) && (sip->tudname == NULL)) {

	    rs = mktmpuserdir(tmpdname,sip->username,IDXDNAME,dmode) ;
	    dl = rs ;
	    if (rs >= 0) {
		const char	*dp ;
	        rs = uc_mallocstrw(tmpdname,dl,&dp) ;
		if (rs >= 0) sip->tudname = dp ;
	    }

	} /* end if */

	return rs ;
}
/* end subroutine (subinfo_tmpuserdir) */


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

	const char	*calsuf = SPELLCHECKS_DBSUF ;
	const char	*tp ;
	const char	*np ;

	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("spellchecks_loadnames: dirname=%s\n",dirname) ;
#endif

	rs = fsdir_open(&dir,dirname) ;
	if (rs < 0)
	    goto ret0 ;

	while (fsdir_read(&dir,&ds) > 0) {
	    if (ds.name[0] == '.') continue ;

#if	CF_DEBUGS
	debugprintf("spellchecks_loadnames: name=%s\n",ds.name) ;
#endif

	    if ((tp = strrchr(ds.name,'.')) != NULL) {

		rs1 = mkpath2(tmpfname,dirname,ds.name) ;

	   	if (rs1 >= 0)
		    rs1 = u_stat(tmpfname,&sb) ;

#if	CF_DEBUGS
	debugprintf("spellchecks_loadnames: u_stat() rs=%d\n",rs1) ;
#endif

		if ((rs1 >= 0) && S_ISREG(sb.st_mode)) {

		if (strcmp((tp+1),calsuf) == 0) {

		    np = ds.name ;
		    nl = (tp - ds.name) ;

#if	CF_DEBUGS
	debugprintf("spellchecks_loadnames: calname=%t\n",np,nl) ;
#endif

		    c += 1 ;
		    rs = vecstr_add(nlp,np,nl) ;
	            if (rs < 0)
		        break ;

	        } /* end if (correct file extension) */

		} /* end if (regular file) */

	    } /* end if */

	} /* end while */

ret1:
	fsdir_close(&dir) ;

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_loadnames) */


static int subinfo_havestart(sip,qp,lp,ll)
struct subinfo	*sip ;
SPELLCHECKS_CITE	*qp ;
const char	*lp ;
int		ll ;
{
	int		rs1 = SR_OK ;
	int		ch ;
	int		cl ;
	int		si = 0 ;
	int		f ;
	const char	*tp, *cp ;

#if	CF_DEBUGS
	debugprintf("spellchecks/subinfo_havestart: >%t<\n",
		lp,strlinelen(lp,ll,40)) ;
#endif

	if (CHAR_ISWHITE(lp[0])) /* continuation */
	    goto ret0 ;

	si = sibreak(lp,ll," \t") ;
	if (si < 3) {
	    rs1 = SR_ILSEQ ;
	    goto ret1 ;
	}

	ch = MKCHAR(lp[0]) ;
	if (isdigitlatin(ch)) {

#if	CF_DEBUGS
	debugprintf("spellchecks/subinfo_havestart: digitlatin\n") ;
#endif

	    tp = strnchr(lp,ll,'/') ;
	    if (tp != NULL) {

	        rs1 = mkmonth(lp,(tp - lp)) ;
	        qp->m = (rs1 & UCHAR_MAX) ;

#if	CF_DEBUGS
		debugprintf("spellchecks/subinfo_havestart: mkmonth() rs1=%d\n",
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
	debugprintf("spellchecks/subinfo_havestart: mkday() rs1=%d\n",rs1) ;
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
	debugprintf("spellchecks/subinfo_havestart: !digitlatin\n") ;
	debugprintf("spellchecks/subinfo_havestart: name=>%t<\n",lp,si) ;
#endif

#if	CF_TRANSHOL
		rs1 = subinfo_transhol(sip,qp,lp,si) ;
#else
		rs1 = SR_NOTSUP ;
#endif

	} /* end if */

#if	CF_DEBUGS
	debugprintf("spellchecks/subinfo_havestart: mid rs=%d si=%u\n",rs1,si) ;
#endif

	if (rs1 >= 0)
	    si += siskipwhite((lp+si),(ll-si)) ;

ret1:
ret0:

#if	CF_DEBUGS
	debugprintf("spellchecks/subinfo_havestart: ret rs=%d si=%u\n",rs1,si) ;
#endif

	return (rs1 >= 0) ? si : rs1 ;
}
/* end subroutine (subinfo_havestart) */


static int subinfo_year(sip)
struct subinfo	*sip ;
{
	TMTIME	tm ;

	int	rs = SR_OK ;


	if (sip->year == 0) {
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
	DAYOFMONTH	*dmp ;

	int	rs = SR_OK ;


	dmp = &sip->dom ;

/* open the DAYOFMONTH database (manager?) if it is not already open */

	if (! sip->f.dom) {
	    rs = dayofmonth_start(dmp,sip->year) ;
	    sip->f.dom = (rs >= 0) ;
	}

	if (rs >= 0)
	    rs = dayofmonth_mkday(dmp,m,cp,cl) ;

#if	CF_DEBUGS
	debugprintf("spellchecks/subinfo_mkday: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_mkday) */


static int subinfo_transhol(sip,qp,sp,sl)
struct subinfo	*sip ;
SPELLCHECKS_CITE	*qp ;
const char	*sp ;
int		sl ;
{
	HOLIDAYS_CUR	hcur ;
	HOLIDAYS	*holp ;

	SPELLCHECKS	*op ;

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
	debugprintf("spellchecks/subinfo_transhol: >%t<\n",sp,sl) ;
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
	debugprintf("spellchecks/subinfo_transhol: n=>%t<\n",np,nl) ;
	debugprintf("spellchecks/subinfo_transhol: f_neg=%u\n",f_negative) ;
	} else
	debugprintf("spellchecks/subinfo_transhol: *no_number*\n") ;
#endif

	holp = &sip->hols ;

/* open the HOLIDAYS database if it is not already open */

	if (! sip->init.hols) {
	    sip->init.hols = TRUE ;
	    f_inityear = TRUE ;
	    rs = subinfo_year(sip) ;
	    if (rs >= 0) {
	        rs = holidays_open(holp,op->pr,sip->year,NULL) ;
	        sip->f.hols = (rs >= 0) ;

#if	CF_DEBUGS
	debugprintf("spellchecks/subinfo_transhol: holidays_open() rs=%d\n",
		rs) ;
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
	    const int	hollen = HOLBUFLEN ;
	    char	holbuf[HOLBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("spellchecks/subinfo_transhol: fq=>%t<\n",sp,sl) ;
#endif

	    if ((holidays_curbegin(holp,&hcur)) >= 0) {
	
	        rs = holidays_fetchname(holp,sp,sl,&hcur,&hc,holbuf,hollen) ;
	        if (rs >= 0) {
		    f_found = TRUE ;
		    qp->m = hc.m ;
		    qp->d = hc.d ;
	        }

	        holidays_curend(holp,&hcur) ;
	    } /* end if (cursor) */

#if	CF_DEBUGS
	debugprintf("spellchecks/subinfo_transhol: "
		"holidays_fetchname() rs=%d\n",rs) ;
	debugprintf("spellchecks/subinfo_transhol: un q=(%u:%u)\n",
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
		    rs = tmtime_adjtime(&tm,&t) ;

		    if (rs >= 0) {
			qp->m = tm.mon ;
			qp->d = tm.mday ;
		    }

#if	CF_DEBUGS
	debugprintf("spellchecks/subinfo_transhol: adjusted q=(%u:%u)\n",
		qp->m,qp->d) ;
#endif

		} /* end if (odays) */

	    } /* end if (got year) */

	} /* end if (day offset required) */

ret0:

#if	CF_DEBUGS
	debugprintf("spellchecks/subinfo_transhol: ret rs=%d f_found=%u\n",
		rs,f_found) ;
#endif

	return (rs >= 0) ? f_found : rs ;
}
/* end subroutine (subinfo_transhol) */


static int subinfo_checkdname(sip,dname)
struct subinfo	*sip ;
const char	dname[] ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;


	if (dname[0] != '/') {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

	rs = u_stat(dname,&sb) ;

	if ((rs >= 0) && (! S_ISDIR(sb.st_mode)))
	    rs = SR_NOTDIR ;

	if (rs >= 0) {
	    rs = subinfo_ids(sip) ;
	    if (rs >= 0)
	        rs = sperm(&sip->id,&sb,W_OK) ;
	}

ret0:
	return rs ;
}
/* end subroutine (subinfo_checkdname) */


static int cachedir_start(CACHEDIR *cdp,STRDESC *dp,int nf)
{
	int	rs = SR_OK ;
	int	i ;
	int	size = 0 ;
	char	*bp ;

	{
	    int		cl ;
	    const char	*cp ;
	    for (i = 0 ; i < nf ; i += 1) {
		cp = dp[i].sp ;
		cl = dp[i].sl ;
		if (cl < 0) cl = strlen(cp) ;
	        size += (cl + 1) ;
	    } /* end for */
	}

	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    cdp->a = bp ;
	    for (i = 0 ; i < nf ; i += 1) {
		switch (i) {
		case 0:
		    cdp->sname = bp ;
		    break ;
		case 1:
		    cdp->dname = bp ;
		    break ;
		} /* end switch */
		if (dp[i].sp != NULL) {
		    bp = (strwcpy(bp,dp[i].sp,dp[i].sl) + 1) ;
		}
	    } /* end for */
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (cachedir_start) */


static int cachedir_finish(CACHEDIR *cdp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (cdp->a != NULL) {
	    rs1 = uc_free(cdp->a) ;
	    if (rs >= 0) rs = rs1 ;
	    cdp->a = NULL ;
	}

	return rs ;
}
/* end subroutine (cachedir_finish) */


static int db_start(DB *dbp,STRDESC *dp,int nf)
{
	int	rs = SR_OK ;
	int	i ;
	int	size = 0 ;
	char	*bp ;

	memset(dbp,0,sizeof(DB)) ;

	{
	    int		cl ;
	    const char	*cp ;
	    for (i = 0 ; i < nf ; i += 1) {
		cp = dp[i].sp ;
		cl = dp[i].sl ;
		if (cl < 0) cl = strlen(cp) ;
	        size += (cl + 1) ;
	    } /* end for */
	}

	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    cdp->a = bp ;
	    for (i = 0 ; i < nf ; i += 1) {
		switch (i) {
		case 0:
		    dbp->idxname = bp ; /* index file name */
		    break ;
		case 1:
		    dbp->dbfname = bp ; /* DB (actual list) file-name */
		    break ;
		case 2:
		    dbp->cdname = bp ; /* cache directory name */
		    break ;
		} /* end switch */
		if (dp[i].sp != NULL) {
		    bp = (strwcpy(bp,dp[i].sp,dp[i].sl) + 1) ;
		}
	    } /* end for */
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (db_start) */


static int db_finish(DB *dbp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (dbp->a != NULL) {
	    rs1 = uc_free(dbp->a) ;
	    if (rs >= 0) rs = rs1 ;
	    dbp->a = NULL ;
	}

	return rs ;
}
/* end subroutine (db_finish) */


static int entry_start(ep,qp,loff,llen)
SPELLCHECKS_ENT		*ep ;
SPELLCHECKS_CITE	*qp ;
int		loff, llen ;
{
	struct spellchecks_eline	*elp ;

	int		rs = SR_OK ;
	int		ne = SPELLCHECKS_NLE ;
	int		size ;

	if (ep == NULL)
	    return SR_FAULT ;

	memset(ep,0,sizeof(SPELLCHECKS_ENT)) ;
	ep->cidx = -1 ;
	ep->m = qp->m ;
	ep->d = qp->d ;
	ep->voff = loff ;
	ep->vlen = llen ;

	size = ne * sizeof(struct spellchecks_eline) ;
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
SPELLCHECKS_ENT	*ep ;
int		cidx ;
{


	if (ep == NULL)
	    return SR_FAULT ;

	ep->cidx = cidx ;
	return SR_OK ;
}

/* end subroutine (entry_setidx) */


static int entry_add(ep,loff,llen)
SPELLCHECKS_ENT	*ep ;
uint		loff, llen ;
{
	struct spellchecks_eline	*elp ;

	int	rs = SR_OK ;
	int	ne ;
	int	size ;


	if (ep == NULL)
	    return SR_FAULT ;

	if (ep->e <= 0)
	    return SR_NOTOPEN ;

	if ((ep->i < 0) || (ep->i > ep->e))
	    return SR_BADFMT ;

	if (ep->i == ep->e) {
	    ne = (ep->e * 2) + SPELLCHECKS_NLE ;
	    size = ne * sizeof(struct spellchecks_eline) ;
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
SPELLCHECKS_ENT	*ep ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL)
	    return SR_FAULT ;

	if (ep->e <= 0)
	    return SR_NOTOPEN ;

	if ((ep->i < 0) || (ep->i > ep->e))
	    return SR_BADFMT ;

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
SPELLCHECKS_ENT	*ep ;
SPELLCHECKS_ENT	*oep ;
{
	int	rs1 = SR_OK ;


	if ((ep->m == oep->m) && (ep->d == oep->d))
	    rs1 = 1 ;

	return rs1 ;
}
/* end subroutine (entry_samecite) */

#endif /* CF_SAMECITE */


static int entry_samehash(ep,op,oep)
SPELLCHECKS_ENT	*ep ;
SPELLCHECKS	*op ;
SPELLCHECKS_ENT	*oep ;
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
SPELLCHECKS_ENT	*ep ;
SPELLCHECKS	*op ;
{
	SPELLCHECKS_CAL	*calp ;

	int	rs = SR_OK ;
	int	sl, cl ;

	const char	*sp ;
	const char	*cp ;


	if (ep == NULL)
	    return SR_FAULT ;

	if (ep->e <= 0)
	    return SR_NOTOPEN ;

	if (ep->lines == NULL)
	    return SR_NOTOPEN ;

	if ((rs = vechand_get(&op->cals,ep->cidx,&calp)) >= 0) {
	    const char	*mp ;
	    if ((rs = cal_mapdata(calp,&mp)) >= 0) {
		struct spellchecks_eline	*elp = ep->lines ;
	        uint	hash = 0 ;
		int	i ;
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
SPELLCHECKS_ENT	*ep ;
uint		hash ;
{


	ep->hash = hash ;
	ep->f.hash = TRUE ;
	return SR_OK ;
}
/* end subroutine (entry_sethash) */


static int entry_same(ep,op,oep)
SPELLCHECKS_ENT	*ep ;
SPELLCHECKS	*op ;
SPELLCHECKS_ENT	*oep ;
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
SPELLCHECKS_ENT	*ep ;
const char	*mp ;
char		rbuf[] ;
int		rlen ;
{
	struct spellchecks_eline	*lines ;

	SBUF	b ;

	int	rs ;
	int	i ;
	int	nlines ;
	int	len = 0 ;

	lines = ep->lines ;
	nlines = ep->i ; /* number of line elements */

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    int		ll ;
	    const char	*lp ;

	    for (i = 0 ; i < nlines ; i += 1) {

	        if (i > 0) sbuf_char(&b,' ') ;

	        lp = (mp + lines[i].loff) ;
	        ll = lines[i].llen ;

#if	CF_DEBUGS
	        debugprintf("spellchecks/entry_loadbuf: i=%u loff=%u llen=%u\n",
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
SPELLCHECKS_ENT	*ep ;
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
	int		rs = SR_OK ;
	int		rs1 ;

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
SPELLCHECKS	*op ;
SPELLCHECKS_ENT	*ep ;
{
	SPELLCHECKS_CAL	*calp ;
	int		rs ;

	if ((rs = vechand_get(&op->cals,ep->cidx,&calp)) >= 0) {
	    const char	*mp ;
	    if ((rs = cal_mapdata(calp,&mp)) >= 0) {
		struct spellchecks_eline	*lines = ep->lines ;
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
	int		cl = 0 ;
	const char	*cp ;

	while (wp->i < wp->nlines) {

	    cl = nextfield(wp->sp,wp->sl,&cp) ;
	    if (cl > 0) {
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


/* configuration management */
static int config_start(csp,sip,cfname)
CONFIG		*csp ;
SUBINFO		*sip ;
const char	*cfname ;
{
	int	rs = SR_OK ;

	char	tmpfname[MAXPATHLEN+1] = { 0 } ;


	if (csp == NULL) return SR_FAULT ;

	if (cfname == NULL) cfname = SPELLCHECKS_CONFNAME ;

	memset(csp,0,sizeof(struct config)) ;
	csp->sip = sip ;

	if (strchr(cfname,'/') == NULL) {
	    rs = config_findfile(csp,tmpfname,cfname) ;
	    if (rs > 0) cfname = tmpfname ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: mid rs=%d cfname=%s\n",rs,cfname) ;
#endif

	if ((rs >= 0) && (sip->debuglevel > 0))
	    shio_printf(sip->efp,"%s: conf=%s\n",
	        sip->progname,cfname) ;

	if (rs >= 0) {
	    if ((rs = paramfile_open(&csp->p,sip->envv,cfname)) >= 0) {
	        if ((rs = config_cookbegin(csp)) >= 0) {
	            csp->f_p = (rs >= 0) ;
	        }
	        if (rs < 0)
	            paramfile_close(&csp->p) ;
	    } else if (isNotPresent(rs))
	        rs = SR_OK ;
	} else if (isNotPresent(rs))
	    rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_start) */


static int config_finish(CONFIG *csp)
{
	SUBINFO		*sip = csp->sip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (csp == NULL) return SR_FAULT ;

	if (csp->f_p) {

	    if (csp->f_c) {
	        rs1 = config_cookend(csp) ;
	        if (rs >= 0) rs = rs1 ;
	    }

	    rs1 = paramfile_close(&csp->p) ;
	    if (rs >= 0) rs = rs1 ;

	    csp->f_p = FALSE ;
	} /* end if (active) */

	return rs ;
}
/* end subroutine (config_finish) */


static int config_cookbegin(CONFIG *csp)
{
	SUBINFO		*sip = csp->sip ;
	EXPCOOK		*ecp = &csp->c ;
	int		rs ;
	int		c = 0 ;

	if ((rs = expcook_start(ecp)) >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    int		i ;
	    int		kch ;
	    int		vl ;
	    const char	*ks = "PSNDHRUsrpuh" ;
	    const char	*vp ;
	    char	hbuf[MAXHOSTNAMELEN+1] ;
	    char	kbuf[2] ;

	    kbuf[1] = '\0' ;
	    for (i = 0 ; (rs >= 0) && (ks[i] != '\0') ; i += 1) {
	        kch = MKCHAR(ks[i]) ;
	        vp = NULL ;
	        vl = -1 ;
	        switch (kch) {
	        case 'P':
	        case 'p':
	        case 'r':
	            vp = sip->progname ;
	            break ;
	        case 'S':
	        case 's':
	            vp = sip->searchname ;
	            break ;
	        case 'N':
	            vp = sip->nodename ;
	            break ;
	        case 'D':
	            vp = sip->domainname ;
	            break ;
	        case 'H':
	            {
	                const char	*nn = sip->nodename ;
	                const char	*dn = sip->domainname ;
	                rs = snsds(hbuf,hlen,nn,dn) ;
	                vl = rs ;
	                if (rs >= 0) vp = hbuf ;
	            }
	            break ;
	        case 'R':
	            vp = sip->pr ;
	            break ;
	        case 'U':
	        case 'u':
	            vp = sip->username ;
	            break ;
		case 'h':
	            vp = sip->userhome ;
	            break ;
	        } /* end switch */
	        if ((rs >= 0) && (vp != NULL)) {
		    c += 1 ;
	            kbuf[0] = kch ;
	            rs = expcook_add(ecp,kbuf,vp,vl) ;
	        }
	    } /* end for */

	    if (rs >= 0) {
		const char	*kp ;
	        for (i = 0 ; (rs >= 0) && (i < cook_overlast ; i += 1) {
		    kp = cooks[i] ;
		    vp = NULL ;
		    vl = -1 ;
		    switch (i) {
		    case cook_pr:
			vp = sip->pr ;
			break ;
		    case cook_pn:
			{
			    int		cl ;
			    const char	*cp ;
			    if ((cl = sfbasename(sip->pr,-1,&cp)) > 0) {
				vp = cp ;
				vl = cl ;
			    }
			}
			break ;
		    case cook_bn:
			if (sip->dbfname != NULL) {
			    int		cl ;
			    const char	*cp ;
			    if ((cl = sfbasename(sip->dbfname,-1,&cp)) > 0) {
				vp = cp ;
				vl = cl ;
			    }
			} else
			    vp = "null" ;
			break ;
		    } /* end switch */
	            if ((rs >= 0) && (vp != NULL)) {
		        c += 1 ;
	                rs = expcook_add(ecp,kp,vp,vl) ;
	            }
		} /* end for */
	    } /* end if */

	    if (rs >= 0) {
	        op->f_c = TRUE ;
	    } else
	        expcook_finish(ecp) ;
	} /* end if (expcook_start) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (config_cookbegin) */


static int config_cookend(CONFIG *csp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (csp->f_c) {
	    csp->f_c = FALSE ;
	    rs1 = expcook_finish(&csp->c) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (config_cookend) */


static int config_read(op)
struct config	*op ;
{
	LOCINFO		*lip = op->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_read: f_p=%u\n",op->f_p) ;
#endif

	lip = sip->lip ;
	if (lip == NULL) return SR_FAULT ;

	if (op->f_p) {
	    const int	plen = PBUFLEN ;
	    int		size ;
	    char	*pbuf ;
	    size = (plen+1) ;
	    if (rs = uc_malloc(size,&pbuf)) >= 0) {
	        rs = config_reader(op,pbuf,plen) ;
		uc_free(pbuf) ;
	    } /* end if (memory-allocation) */
	}

	return rs ;
}
/* end subroutine (config_read) */


static int config_reader(CONFIG *op,char *pbuf,int plen)
{
	PARAMFILE	*pfp = &op->p ;
	PARAMFILE_CUR	cur ;
	PARAMFILE_ENT	pe ;
	int		rs ;

	if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {
	    EXPCOOK	*ecp = &op->c ;
	    const int	elen = EBUFLEN ;
	    int		vl, el ;
	    int		ml ;
	    int		pi ;
	    int		v ;
	    const char	*kp ;
	    const char	*vp ;
	    char	ebuf[EBUFLEN + 1] ;

	    while (rs >= 0) {
	        kl = paramfile_enum(pfp,&cur,&pe,pbuf,plen) ;
	        if (kl == SR_NOTFOUND) break ;
	        rs = kl ;
	        if (rs < 0) break ;

	            kp = pe.key ;
	            vp = pe.value ;
	            vl = pe.vlen ;

	            pi = matpstr(params,2,kp,kl) ;
	            if (pi < 0) continue ;

	            ebuf[0] = '\0' ;
	            el = 0 ;
	            if (vl > 0) {
	                el = expcook_exp(ecp,0,ebuf,elen,vp,vl) ;
	                if (el >= 0) ebuf[el] = '\0' ;
	            } /* end if */

	            if (el < 0) continue ;

		switch (pi) {

		case cparam_logsize:
				if (el > 0) {
	                            if (cfdecmfi(ebuf,el,&v) >= 0) {
	                                if (v >= 0) {
	                                    switch (i) {
	                                    case cparam_logsize:
	                                        sip->logsize = v ;
	                                        break ;
	                                    } /* end switch */
	                                }
	                            } /* end if (valid number) */
				}
	                        break ;

		case cparam_logfile:
				if (el > 0) {
	                            if (! sip->final.lfname) {
	                                sip->final.lfname = TRUE ;
	                                sip->have.lfname = TRUE ;
				        rs = config_setlfname(op,ebuf,el) ;
	                            }
				}
	                        break ;

		case cparam_cachedir:
		    if (el > 0) {
			rs = config_cachedir(op,ebuf,el) ;
		    }
		    break ;

		case cparam_db:
		    if (el > 0) {
			rs = config_db(op,ebuf,el) ;
		    }
		    break ;

		} /* end switch */

	    } /* end while (emumerating) */

	    paramfile_curend(pfp) ;
	} /* end if (cursor) */

	return rs ;
}
/* end subroutine (config_reader) */


static int config_setlfname(cfp,vp,vl)
CONFIG		*cfp ;
const char	*vp ;
int		vl ;
{
	SUBINFO		*sip = cfp->sip ;
	const char	*lfn ;
	const char	*pr ;
	const char	*sn ;
	char		tbuf[MAXPATHLEN_1] ;
	int	rs = SR_OK ;
	int	tl ;

	pr = sip->pr ;
	sn = sip->sn ;
	lfn = sip->lfname ;
	tl = prsetfname(pr,tbuf,vp,vl,TRUE,LOGCNAME,sn,"") ;

	if ((lfn == NULL) || (strcmp(lfn,tbuf) != 0)) {
	    const char	**vpp = &sip->lfname ;
	    sip->changed.lfname = TRUE ;
	    rs = subinfo_setentry(sip,vpp,tbuf,-1) ;
	}

	return rs ;
}
/* end subroutine (config_setlfname) */


static int config_cachedir(CONFIG *csp,const char *pp,int pl)
{
	STRDESC		d[CACHEDIR_NFIELDS] ;
	const int	nf = CACHEDIR_NFIELDS ;
	int		rs = SR_OK ;
	int		fi = 0 ;
	int		si ;
	int		f = FALSE ;

	while ((fi < nf) && ((si = sichr(pp,pl,CH_FS)) >= 0)) {
	    rs = config_cachedirone(csp,d,fi++,pp,si) ;
	    pl -= (si+1) ;
	    pp += (si+1) ;
	    if (rs < 0) break ;
	} /* end while */
	if ((rs >= 0) && (fi < nf) && (pl > 0)) {
	    rs = config_cachedirone(csp,d,fi++,pp,pl) ;
	}

	if ((rs >= 0) && (fi >= nf)) {
	    const int	size = sizeof(CACHEDIR) ;
	    void	*p ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
		CACHEDIR	*cdp = p ;
	        if ((rs = cachedir_start(cdp,d)) >= 0) {
		    SUBINFO	*sip = csp->sip ;
		    f = TRUE ;
		    rs = subinfo_cachedir(sip,cdp) ;
		    if (rs < 0)
			cachedir_finish(cdp) ;
		} /* end if (start) */
		if (rs < 0)
		    uc_free(p) ;
	    } /* end if (memory-allocation) */
	} /* end if (had data for new entry) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (config_cachedir) */


/* ARGSUSED */
static int config_cachedirone(csp,dp,fi,pp,pl)
CONFIG		*csp ;
STRDESC		*dp ;
int		fi ;
const char	*pp ;
int		pl ;
{
	int	rs = SR_OK ;

		switch (fi) {
		case 0:
		case 1:
		    dp[fi].sp = pp ;
		    dp[fi].sl = pl ;
		    break ;
		} /* end switch */

	return rs ;
}
/* end subroutine (config_cachedirone) */


static int config_db(CONFIG *csp,const char *ebuf,int el)
{
	STRDESC		d[DB_NFIELDS] ;
	const int	nf = DB_NFIELDS ;
	int		rs = SR_OK ;
	int		fi = 0 ;
	int		si ;
	int		f = FALSE ;

	while ((fi < nf) && ((si = sichr(pp,pl,CH_FS)) >= 0)) {
	    rs = config_dbone(csp,d,fi++,pp,si) ;
	    pl -= (si+1) ;
	    pp += (si+1) ;
	    if (rs < 0) break ;
	} /* end while */
	if ((rs >= 0) && (fi < nf) && (pl > 0)) {
	    rs = config_dbone(csp,d,fi++,pp,pl) ;
	}

	if ((rs >= 0) && (fi >= nf)) {
	    SUBINFO	*sip = csp->sip ;
	    int		sch = (d[1].sp[0] & 0xff) ;
	    char	dbuf[MAXPATHLEN+1] ;
	    if ((sch == '-') && (sip->dbfname != NULL)) {
		d[1].sp = sip->dbfname ;
		d[1].sl = -1 ;
	    }
	    if ((sch != '/') && (sch != '-')) {
		rs = mkpath2(dbuf,sip->pr,d[1]) ;
		d[1].sl = rs ;
		d[1].sp = dbuf ;
	    }
	    if (rs >= 0) {
		struct ustat	sb ;
		int	f_skip = (sch == '-') ;
		if (f_skip || (u_stat(d[1],&sb) >= 0)) {
		    if (f_skip || ((rs = sperm(&sip->id,&sb,R_OK)) >= 0)) {
	                const int	size = sizeof(DB) ;
	                void		*p ;
	                if ((rs = uc_malloc(size,&p)) >= 0) {
		            DB	*dbp = p ;
	                    if ((rs = db_start(dbp,d,nf)) >= 0) {
		                SUBINFO	*sip = csp->sip ;
		                f = TRUE ;
		                rs = subinfo_db(sip,dbp) ;
		                if (rs < 0)
			            db_finish(dbp) ;
		            } /* end if (start) */
		            if (rs < 0)
		                uc_free(p) ;
	                } /* end if (memory-allocation) */
		    } else (rs == SR_ACCESS)
			rs = SR_OK ;
		} /* end if (stat) */
	    } /* end if (ok) */
	} /* end if (had data for new entry) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (config_db) */


/* ARGSUSED */
static int config_dbone(csp,dp,fi,pp,pl)
CONFIG		*csp ;
STRDESC		*dp ;
int		fi ;
const char	*pp ;
int		pl ;
{
	int	rs = SR_OK ;

		switch (fi) {
		case 0:
		case 1:
		case 2:
		    dp[fi].sp = pp ;
		    dp[fi].sl = pl ;
		    break ;
		} /* end switch */

	return rs ;
}
/* end subroutine (config_dbone) */


#if	CF_CONFIGCHECK
static int config_check(CONFIG *csp)
{
	SUBINFO	*sip = csp->sip ;

	int	rs = SR_OK ;

	if (csp->f_p) {
	    rs = paramfile_check(&csp->p,sip->daytime) ;
	    if (rs > 0)
	        rs = config_read(op) ;
	}

	return rs ;
}
/* end subroutine (config_check) */
#endif /* CF_CONFIGCHECK */


#ifdef	COMMENT
static int config_findfile(csp,tbuf,cfname)
struct config	*csp ;
char		tbuf[] ;
const char	cfname[] ;
{
	SUBINFO	*sip = csp->sip ;

	VECSTR	sv ;

	int	rs ;
	int	tl = 0 ;

	tbuf[0] = '\0' ;
	if ((cfname != NULL) && (cfname[0] != '\0')) {
	    if ((rs = vecstr_start(&sv,6,0)) >= 0) {
	        const int	tlen = MAXPATHLEN ;
	        int		i ;
	        int		vl ;
	        int		kch ;
	        const char	*ks = "pen" ;
	        const char	*vp ;
	        char	kbuf[2] ;
	        kbuf[1] = '\0' ;
	        for (i = 0 ; (rs >= 0) && (ks[i] != '\0') ; i += 1) {
		    kch = (ks[i] & 0xff) ;
		    kbuf[0] = kch ;
		    vp = NULL ;
		    vl = -1 ;
		    switch (kch) {
		    case 'p':
		        vp = sip->pr ;
		        break ;
		    case 'e':
		        vp = "etc" ;
		        break ;
		    case 'n':
		        vp = sip->searchname ;
		        break ;
		    } /* end switch */
		    if (rs >= 0) {
	    	        rs = vecstr_envset(&sv,"p",sip->pr,-1) ;
		    }
	        } /* end for */
	        if (rs >= 0) {
	            rs = permsched(sched1,&sv,tbuf,tlen,cfname,R_OK) ;
	            tl = rs ;
	        }
	        vecstr_finish(&sv) ;
	    } /* end if (finding file) */
	} /* end if (non-null) */

	return (rs >= 0) ? tl : rs ;
}
/* end subroutine (config_findfile) */
#endif /* COMMENT */


static int isempty(lp,ll)
const char	*lp ;
int		ll ;
{
	int	cl ;
	int	f = FALSE ;

	const char	*cp ;


	f = f || (ll == 0) ;
	f = f || (lp[0] == '#') ;
	if ((! f) && CHAR_ISWHITE(*lp)) {
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
	SPELLCHECKS_ENT	*e1p, **e1pp = (SPELLCHECKS_ENT **) v1p ;
	SPELLCHECKS_ENT	*e2p, **e2pp = (SPELLCHECKS_ENT **) v2p ;

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



