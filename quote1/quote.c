
* quote */

/* quote database operations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGCUR	0		/* debug cursor operation */
#define	CF_SAFE		1		/* normal safety */
#define	CF_SAFE2	1		/* extra safety */
#define	CF_EMPTYTERM	1		/* terminate entry on empty line */
#define	CF_TMPPRNAME	1		/* put under a PRNAME in /tmp */
#define	CF_SAMECITE	0		/* same entry citation? */
#define	CF_ALREADY	1		/* do not allow duplicate results */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module implements an interface (a trivial one) that allows access
        to the QUOTE datbase.

	Implementation notes:

	= parsing a calendar file

	There are several valid forms for the date (month-day) part of
	a calendar entry.  These are:
		mm/dd		most common
		name[±ii]	name plus-or-minus increment in days
	The subroutine 'subinfo_havestart()' parses this out.


*******************************************************************************/


#define	QUOTE_MASTER	1


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
#include	<vecobj.h>
#include	<sbuf.h>
#include	<char.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<tmtime.h>
#include	<fsdir.h>
#include	<getxusername.h>
#include	<txtindexmk.h>
#include	<localmisc.h>

#include	"quote.h"
#include	"textlook.h"


/* local defines */

#undef	COMMENT

#define	QUOTE_MAGIC		0x99447245
#define	QUOTE_DBSUF		"calendar"
#define	QUOTE_CAL		struct quote_qdir

#define	QDIR			struct quote_dir
#define	QDIR_DBDIR		".quotes"
#define	QDIR_DBDIRMODE		0777
#define	QDIR_QUOTES		"share/quotes" ;

#define	QUOTE_NLE		1	/* default number line entries */
#define	QUOTE_DMODE		0777

#undef	WORDER
#define	WORDER			struct worder

#define	IDXDNAME	"quote"
#define	IDXNAME		"quote"
#define	IDXSUF		"cyi"

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

#undef	NLINES
#define	NLINES		20

#define	CEBUFLEN	(NLINES * 3 * sizeof(int))

#define	TO_FILEMOD	(60 * 24 * 3600)
#define	TO_MKWAIT	(5 * 50)
#define	TO_CHECK	4



/* external subroutines */

extern uint	hashelf(const char *,int) ;

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	mkpath3w(char *,const char *,const char *,const char *,int) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	siskipwhite(const char *,int) ;
extern int	sibreak(const char *,int,const char *) ;
extern int	nextfield(const char *,int,const char **) ;
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


/* local structures */

struct subinfo_flags {
	uint		id : 1 ;
	uint		defdirs : 1 ;
	uint		dom : 1 ;
	uint		hols : 1 ;
} ;

struct subinfo {
	IDS		id ;
	vecstr		defdirs ;
	struct subinfo_flags	init, f ;
	QUOTE		*op ;
	const char	*tmpdname ;
	char		*tudname ;
	char		*userhome ;
	const char	**dirnames ;
	time_t		daytime ;
	int		year ;
	int		isdst ;
	int		gmtoff ;
	char		username[USERNAMELEN + 1] ;
} ;

struct quote_dirflags {
	uint		tind : 1 ;
} ;

struct quote_dir {
	struct quote_dirflags	f ;
	char		*dirname ;
	TEXTLOOK	looker ;
	time_t		ti_mtime ;		/* latest for all entries */
	int		nentries ;		/* DB entries */
	int		cidx ;			/* ordinal */
} ;

struct quote_qdirflags {
	uint		writedbdir : 1 ;
} ;

struct quote_qdir {
	char		*dirname ;
	char 		*calname ;		/* DB file-name */
	char		*mapdata ;		/* DB memory-map address */
	struct quote_qdirflags	f ;
	TXTLOOK	vind ;			/* verse-index */
	time_t		ti_db ;			/* DB file modification */
	time_t		ti_map ;		/* DB map */
	time_t		ti_lastcheck ;		/* DB last check */
	time_t		ti_vind ;		/* verse-index */
	uint		filesize ;		/* DB file size */
	uint		mapsize ;		/* DB map length */
	int		nentries ;		/* DB entries */
	int		cidx ;			/* ordinal */
} ;

struct quote_eline {
	uint		loff ;
	uint		llen ;
} ;

struct quote_eflags {
	uint		hash : 1 ;
} ;

struct quote_e {
	struct quote_eline	*lines ;
	struct quote_eflags	f ;
	uint		voff ;
	uint		vlen ;
	uint		hash ;
	int		e, i ;
	int		cidx ;
	uchar		m, d ;
} ;

struct quote_citer {
	int		i, e ;
} ;

struct worder {
	struct quote_eline	*lines ;
	const char	*mp ;
	const char	*sp ;
	int		sl ;
	int		i ;
	int		nlines ;
} ;


/* forward references */

static int	quote_dirnamescreate(QUOTE *,const char **) ;
static int	quote_dirnamesdestroy(QUOTE *) ;

static int	quote_checkupdate(QUOTE *,time_t) ;
static int	quote_loadbuf(QUOTE *,QUOTE_ENT *,
			char *,int) ;
static int	quote_tmpfrees(QUOTE *) ;

static int	quote_dirsopen(QUOTE *,struct subinfo *,
			const char **,const char **) ;
static int	quote_dirsclose(QUOTE *) ;
static int	quote_diropen(QUOTE *,struct subinfo *,
			const char *,const char **) ;
static int	quote_dirclose(QUOTE *,QDIR *) ;

static int	quote_qdirsdestroy(QUOTE *) ;
static int	quote_qdircreate(QUOTE *,struct subinfo *,
			const char *,const char *) ;
static int	quote_qdirdestroy(QUOTE *,QUOTE_CAL *) ;

#ifdef	COMMENT
static int	quote_mksysvarsi(QUOTE *,const char *) ;
#endif

static int	quote_freeresults(QUOTE *,QUOTE_CUR *) ;
static int	quote_qdircite(QUOTE *,vecobj *,QUOTE_CAL *,
			QUOTE_QUERY *) ;
static int	quote_mkresults(QUOTE *,vecobj *,QUOTE_CUR *) ;
static int	quote_already(QUOTE *,vecobj *,
			QUOTE_ENT *) ;

#if	CF_DEBUGS && CF_DEBUGCUR
static int	quote_debugcur(QUOTE *,vecobj *,const char *) ;
#endif

static int	cal_open(QUOTE_CAL *,struct subinfo *,int,
			const char *,const char *) ;
static int	cal_close(QUOTE_CAL *) ;
static int	cal_dbloadinit(QUOTE_CAL *,struct subinfo *) ;
static int	cal_dbloadfree(QUOTE_CAL *) ;
static int	cal_indopen(QUOTE_CAL *,struct subinfo *) ;
static int	cal_dbmapcreate(QUOTE_CAL *,time_t) ;
static int	cal_dbmapdestroy(QUOTE_CAL *) ;
static int	cal_indopenperm(QUOTE_CAL *,struct subinfo *) ;
static int	cal_indopentmp(QUOTE_CAL *,struct subinfo *) ;

#ifdef	COMMENT
static int	cal_idxset(QUOTE_CAL *,int) ;
#endif

static int	cal_idxget(QUOTE_CAL *) ;
static int	cal_indopencheck(QUOTE_CAL *,const char *,int,int) ;
static int	cal_mkdirs(QUOTE_CAL *,const char *,mode_t) ;
static int	cal_audit(QUOTE_CAL *) ;

static int	cal_indmk(QUOTE_CAL *,struct subinfo *,const char *,
			int,time_t) ;
static int	cal_indmkdata(QUOTE_CAL *,struct subinfo *,const char *,int,
			int) ;
static int	cal_indclose(QUOTE_CAL *) ;

static int	cal_loadbuf(QUOTE_CAL *,QUOTE_ENT *,char *,int) ;
static int	cal_mapdata(QUOTE_CAL *,const char **) ;

static int	subinfo_start(struct subinfo *,QUOTE *,time_t,const char **) ;
static int	subinfo_ids(struct subinfo *) ;
static int	subinfo_finish(struct subinfo *) ;
static int	subinfo_username(struct subinfo *) ;
static int	subinfo_tmpuserdir(struct subinfo *) ;
static int	subinfo_mkdirnames(struct subinfo *) ;
static int	subinfo_havedir(struct subinfo *,char *) ;
static int	subinfo_loadnames(struct subinfo *,vecstr *,const char *) ;
static int	subinfo_havestart(struct subinfo *,
			QUOTE_QUERY *,const char *,int) ;
static int	subinfo_year(struct subinfo *) ;
static int	subinfo_checkdname(struct subinfo *,const char *) ;

#ifdef	COMMENT
static int	subinfo_mkday(struct subinfo *,int,const char *,int) ;
static int	subinfo_transhol(struct subinfo *,QUOTE_CITE *,
			const char *,int) ;
#endif

static int	entry_start(QUOTE_ENT *,QUOTE_CITE *,int,int) ;
static int	entry_setidx(QUOTE_ENT *,int) ;
static int	entry_add(QUOTE_ENT *,uint,uint) ;
static int	entry_finish(QUOTE_ENT *) ;

#ifdef	COMMENT
static int	entry_release(QUOTE_ENT *) ;
#endif

static int	entry_mkhash(QUOTE_ENT *,QUOTE *) ;
static int	entry_sethash(QUOTE_ENT *,uint) ;
static int	entry_samehash(QUOTE_ENT *,QUOTE *,QUOTE_ENT *) ;
static int	entry_same(QUOTE_ENT *,QUOTE *,QUOTE_ENT *) ;
static int	entry_loadbuf(QUOTE_ENT *,const char *,char *,int) ;

#if	CF_SAMECITE
static int	entry_samecite(QUOTE_ENT *,QUOTE *,QUOTE_ENT *) ;
#endif

static int	mkbve_start(CYIMK_ENT *,struct subinfo *,QUOTE_ENT *) ;
static int	mkbve_finish(CYIMK_ENT *) ;

static int	worder_start(WORDER *,QUOTE *,QUOTE_ENT *) ;
static int	worder_finish(WORDER *) ;
static int	worder_get(WORDER *,const char **) ;

static int	isempty(const char *,int) ;

static int	mkmonth(const char *,int) ;
static int	dayofmonth_mkday(DAYOFMONTH *,uint,const char *,int) ;

static int	vrcmp(const void *,const void *) ;


/* exported variables */

QUOTE_OBJ	quote = {
	"quote",
	sizeof(QUOTE),
	sizeof(QUOTE_CUR)
} ;


/* local variables */

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


/* exported subroutines */


int quote_open(op,pr,dirnames,quotenames)
QUOTE		*op ;
const char	pr[] ;
const char	*dirnames[] ;
const char	*quotenames[] ;
{
	struct subinfo	si, *sip = &si ;

	int	rs ;
	int	opts ;
	int	c = 0 ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;
#endif

	if (pr == NULL)
	    return SR_FAULT ;

	if (pr[0] == '\0')
	    return SR_INVALID ;

	memset(op,0,sizeof(QUOTE)) ;

	op->pr = pr ;
	op->tmpdname = getenv(VARTMPDNAME) ;
	if (op->tmpdname == NULL)
	    op->tmpdname = TMPDNAME ;

	rs = quote_dirnamescreate(op,dirnames) ;
	if (rs < 0)
	    goto bad0 ;

	rs = vecstr_start(&op->tmpfiles,2,0) ;
	if (rs < 0)
	    goto bad1 ;

	opts = VECHAND_OSTATIONARY ;
	rs = vechand_start(&op->dirs,2,opts) ;
	if (rs < 0)
	    goto bad2 ;

	rs = subinfo_start(sip,op,0L,dirnames) ;
	if (rs >= 0) {

	    rs = quote_dirsopen(op,sip,sip->dirnames,quotenames) ;
	    c = rs ;

	    subinfo_finish(sip) ;

	} /* end if */

	if (rs < 0)
	    goto bad3 ;

/* done */

	op->nentries = c ;
	op->magic = QUOTE_MAGIC ;

ret0:

#if	CF_DEBUGS
	debugprintf("quote_open: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;

/* bad stuff */
bad3:
	quote_qdirsdestroy(op) ;
	vechand_finish(&op->dirs) ;

bad2:
	quote_tmpfrees(op) ;
	vecstr_finish(&op->tmpfiles) ;

bad1:
	quote_dirnamesdestroy(op) ;

bad0:
	goto ret0 ;
}
/* end subroutine (quote_open) */


/* free up the entire vector string data structure object */
int quote_close(op)
QUOTE		*op ;
{
	int	rs ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != QUOTE_MAGIC)
	    return SR_NOTOPEN ;
#endif

	rs = quote_qdirsdestroy(op) ;

	vechand_finish(&op->cals) ;

	quote_tmpfrees(op) ;
	vecstr_finish(&op->tmpfiles) ;

	quote_dirnamesdestroy(op) ;

#if	CF_DEBUGS
	debugprintf("quote_close: ret rs=%d\n",rs) ;
#endif

	op->nentries = 0 ;
	op->magic = 0 ;
	return rs ;
}
/* end subroutine (quote_close) */


int quote_count(op)
QUOTE		*op ;
{
	int	rs ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != QUOTE_MAGIC)
	    return SR_NOTOPEN ;
#endif

	rs = op->nentries ;

	return rs ;
}
/* end subroutine (quote_count) */


int quote_audit(op)
QUOTE		*op ;
{
	QUOTE_CAL	*calp ;

	int	rs = SR_OK ;
	int	i ;
	int	c = 0 ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != QUOTE_MAGIC)
	    return SR_NOTOPEN ;
#endif

	for (i = 0 ; vechand_get(&op->cals,i,&calp) >= 0 ; i += 1) {
	    if (calp == NULL) continue ;
	    c += 1 ;
	    rs = cal_audit(calp) ;
	    if (rs < 0)
		break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (quote_audit) */


int quote_curbegin(op,curp)
QUOTE		*op ;
QUOTE_CUR	*curp ;
{


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != QUOTE_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (curp == NULL)
	    return SR_FAULT ;

	memset(curp,0,sizeof(QUOTE_CUR)) ;

	op->ncursors += 1 ;

	curp->i = -1 ;
	curp->magic = QUOTE_MAGIC ;
	return SR_OK ;
}
/* end subroutine (quote_curbegin) */


int quote_curend(op,curp)
QUOTE		*op ;
QUOTE_CUR	*curp ;
{


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != QUOTE_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (curp == NULL)
	    return SR_FAULT ;

	if (curp->magic != QUOTE_MAGIC)
	    return SR_NOTOPEN ;

	if (curp->results != NULL) {
	    quote_freeresults(op,curp) ;
	    uc_free(curp->results) ;
	    curp->results = NULL ;
	}

	if (op->ncursors > 0)
	    op->ncursors -= 1 ;

	curp->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (quote_curend) */


static int quote_freeresults(op,curp)
QUOTE		*op ;
QUOTE_CUR	*curp ;
{
	QUOTE_ENT	*ep ;
	int		i ;

	ep = (QUOTE_ENT *) curp->results ;
	if (ep != NULL) {
	    for (i = 0 ; i < curp->nresults ; i += 1) {
		entry_finish(ep + i) ;
	    }
	} /* end if */

	return SR_OK ;
}
/* end subroutine (quote_freeresults) */


int quote_lookup(op,curp,qopts,qvp)
QUOTE		*op ;
QUOTE_CUR	*curp ;
int		qopts ;
QUOTE_QUERY	*qvp ;
{
	QUOTE_CAL	*calp ;

	vecobj	res ;

	int	rs = SR_OK ;
	int	i ;
	int	opts ;
	int	size ;
	int	c = 0 ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != QUOTE_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (curp == NULL)
	    return SR_FAULT ;

	if (curp->magic != QUOTE_MAGIC)
	    return SR_NOTOPEN ;

	if (qvp == NULL)
	    return SR_FAULT ;

	if (curp->results != NULL) {
	    quote_freeresults(op,curp) ;
	    uc_free(curp->results) ;
	    curp->results = NULL ;
	}

	opts = 0 ;
	opts |= VECOBJ_OORDERED ;
	opts |= VECOBJ_OSTATIONARY ;
	size = sizeof(QUOTE_ENT) ;
	rs = vecobj_start(&res,size,0,opts) ;
	if (rs < 0)
	    goto ret0 ;

	for (i = 0 ; vechand_get(&op->cals,i,&calp) >= 0 ; i += 1) {
	    if (calp == NULL) continue ;

	    rs = quote_qdircite(op,&res,calp,qvp) ;
	    c += rs ;

#if	CF_DEBUGS
	debugprintf("quote_lookcite: i=%u quote_qdircite() rs=%d\n",
		i,rs) ;
#endif

	    if (rs < 0)
	        break ;

	} /* end for */

	if (rs >= 0) {

	    rs = quote_mkresults(op,&res,curp) ;

#if	CF_DEBUGS
	debugprintf("quote_lookcite: quote_mkresults() rs=%d\n",rs) ;
#endif

	}

	if ((rs < 0) || (c > 0)) {
	    QUOTE_ENT	*ep ;
	    for (i = 0 ; vecobj_get(&res,i,&ep) >= 0 ; i += 1) {
		if (ep == NULL) continue ;
		entry_finish(ep) ;
	    } /* end for */
	}

ret1:
	vecobj_finish(&res) ;

ret0:

#if	CF_DEBUGS
	debugprintf("quote_lookcite: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (quote_lookcite) */


int quote_read(op,curp,qvp,rbuf,rlen)
QUOTE		*op ;
QUOTE_CUR	*curp ;
QUOTE_CITE	*qvp ;
char		*rbuf ;
int		rlen ;
{
	QUOTE_ENT	*ep ;

	int	rs = SR_OK ;
	int	i ;
	int	len = 0 ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != QUOTE_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (curp == NULL)
	    return SR_FAULT ;

	if (curp->magic != QUOTE_MAGIC)
	    return SR_NOTOPEN ;

	if (qvp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("quote_read: entered\n") ;
#endif

	if (curp->results == NULL) {
	    rs = SR_NOTFOUND ;
	    goto ret0 ;
	}

	i = curp->i ;

#if	CF_DEBUGS
	debugprintf("quote_read: c_i=%d\n",i) ;
#endif

	if (i < 0) {
	    rs = SR_NOTFOUND ;
	    goto ret0 ;
	}

	if (i >= curp->nresults) {
	    rs = SR_NOTFOUND ;
	    goto ret0 ;
	}

	ep = (QUOTE_ENT *) curp->results ;
	qvp->m = ep->m ;
	qvp->d = ep->d ;
	if (rbuf != NULL) {

#if	CF_DEBUGS
	debugprintf("quote_read: quote_loadbuf()\n") ;
#endif

	    rs = quote_loadbuf(op,(ep + i),rbuf,rlen) ;
	    len = rs ;

#if	CF_DEBUGS
	debugprintf("quote_read: quote_loadbuf() rs=%d\n",rs) ;
#endif

	} /* end if */

	if (rs >= 0)
	    curp->i = (i + 1) ;

ret0:

#if	CF_DEBUGS
	debugprintf("quote_read: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (quote_read) */


int quote_check(op,daytime)
QUOTE		*op ;
time_t		daytime ;
{
	int	rs = SR_OK ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != QUOTE_MAGIC)
	    return SR_NOTOPEN ;
#endif

#ifdef	COMMENT
	rs = quote_checkupdate(op,daytime) ;
#endif

	return rs ;
}
/* end subroutine (quote_check) */


/* private subroutines */


static int quote_dirnamescreate(op,dirnames)
QUOTE		*op ;
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

bad1:
	uc_free(op->dirnames) ;
	op->dirnames = NULL ;

bad0:
	goto ret0 ;
}
/* end subroutine (quote_dirnamescreate) */


static int quote_dirnamesdestroy(op)
QUOTE		*op ;
{
	int	rs = SR_OK ;


	if (op->dirnames != NULL) {
	    uc_free(op->dirnames) ;
	    op->dirnames = NULL ;
	}

	if (op->dirstrtab != NULL) {
	    uc_free(op->dirstrtab) ;
	    op->dirstrtab = NULL ;
	}

	return rs ;
}
/* end subroutine (quote_dirnamesdestroy) */


static int quote_qdircite(op,rlp,calp,qvp)
QUOTE		*op ;
vecobj		*rlp ;
QUOTE_CAL	*calp ;
QUOTE_QUERY	*qvp ;
{
	QUOTE_ENT	e ;

	TXTLOOK		*cip ;
	TEXTLOOK_CUR	ccur ;
	TEXTLOOK_QUERY	cq ;
	TEXTLOOK_ENT	ce ;

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
	debugprintf("quote_qdircite: cidx=%d\n",cidx) ;
#endif

	memset(&cq,0,sizeof(TEXTLOOK_QUERY)) ;
	cq.m = qvp->m ;
	cq.d = qvp->d ;

	if ((rs = cyi_curbegin(cip,&ccur)) >= 0) {

	    rs = cyi_lookcite(cip,&ccur,&cq) ;

#if	CF_DEBUGS
	debugprintf("quote_qdircite: cyi_lookcite() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

	        while (rs >= 0) {

#if	CF_DEBUGS && CF_DEBUGCUR
		debugprintf("quote_qdircite: cyi_read() c=%u\n",c) ;
		quote_debugcur(op,rlp,"before cyi_read") ;
#endif

	            rs1 = cyi_read(cip,&ccur,&ce,cebuf,celen) ;

#if	CF_DEBUGS && CF_DEBUGCUR
		debugprintf("quote_qdircite: cyi_read() rs1=%d\n",rs1) ;
		quote_debugcur(op,rlp,"after cyi_read") ;
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
			debugprintf("quote_qdircite: i=%u loff=%u llen=%u\n",
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
			    rs = quote_already(op,rlp,&e) ;
			    f_already = (rs > 0) ;
#endif

			    f_ent = FALSE ;
			    if ((rs >= 0) && (! f_already))
		                rs = vecobj_add(rlp,&e) ;
		            if ((rs < 0) || f_already)
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
	debugprintf("quote_qdircite: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (quote_qdircite) */


static int quote_already(op,rlp,ep)
QUOTE		*op ;
vecobj		*rlp ;
QUOTE_ENT	*ep ;
{
	QUOTE_ENT	*oep ;

	int	rs = SR_OK ;
	int	i ;
	int	f = FALSE ;


	for (i = 0 ; vecobj_get(rlp,i,&oep) >= 0 ; i += 1) {
	    if (oep == NULL) continue ;

	    rs = entry_samehash(ep,op,oep) ;
	    if (rs == 0) continue ;

	    if (rs >= 0)
	        rs = entry_same(ep,op,oep) ;

	    f = (rs > 0) ;
	    if (f) break ;

	    if (rs < 0)
		break ;

	} /* end for */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (quote_already) */


static int quote_mkresults(op,rlp,curp)
QUOTE		*op ;
vecobj		*rlp ;
QUOTE_CUR	*curp ;
{
	QUOTE_ENT	*rp ;
	QUOTE_ENT	*ep ;

	int	rs = SR_OK ;
	int	n ;
	int	i ;
	int	size ;
	int	c = 0 ;


#if	CF_DEBUGS
	debugprintf("quote_mkresults: entered\n") ;
#endif

	vecobj_sort(rlp,vrcmp) ; /* sort results in ascending order */

	n = vecobj_count(rlp) ;
	if (n <= 0)
	    goto ret0 ;

	size = n * sizeof(QUOTE_ENT) ;
	rs = uc_malloc(size,&rp) ;
	if (rs < 0)
	    goto ret0 ;

	for (i = 0 ; vecobj_get(rlp,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;

#if	CF_DEBUGS
	{
	struct quote_eline	*lines = ep->lines ;
		int	j ;
	if (lines != NULL) {
		for (j = 0 ; j < ep->i ; j += 1) {
		debugprintf("quote_mkresults: i=%u j=%u loff=%u llen=%u\n",
		i,j,lines[j].loff,lines[j].llen) ;
		}
	}
	}
#endif

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
	debugprintf("quote_mkresults: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (quote_mkresults) */


static int quote_tmpfrees(op)
QUOTE		*op ;
{
	int	i ;

	char	*sp ;


	for (i = 0 ; vecstr_get(&op->tmpfiles,i,&sp) >= 0 ; i += 1) {
	    if (sp == NULL) continue ;
	    if (sp[0] != '\0')
		u_unlink(sp) ;
	} /* end for */

	return SR_OK ;
}
/* end subroutine (quote_tmpfrees) */


static int quote_dirsopen(op,sip,dirnames,quotenames)
QUOTE		*op ;
struct subinfo	*sip ;
const char	*dirnames[] ;
const char	*quotenames[] ;
{
	int	rs = SR_OK ;
	int	i ;
	int	c = 0 ;


	if (dirnames == NULL)
	    goto ret0 ;

	for (i = 0 ; dirnames[i] != NULL ; i += 1) {
	    if (dirnames[i] == '\0') continue ;

	    rs = quote_diropen(op,sip,dirnames[i],quotenames) ;
	    c += rs ;
	    if (rs < 0)
		break ;

	} /* end for (dirnames) */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (quote_dirsopen) */


static int quote_dirsclose(op)
QUOTE		*op ;
{
	QDIR	*qdirp ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;


	for (i = 0 ; vechand_get(&op->cals,i,&qdirp) >= 0 ; i += 1) {
	    if (qdirp == NULL) continue ;
	    rs1 = quote_dirclose(op,qdirp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end for */

	return rs ;
}
/* end subroutine (quote_dirsclose) */


static int quote_diropen(op,sip,dirname,quotenames)
QUOTE		*op ;
struct subinfo	*sip ;
const char	*dirname ;
const char	*quotenames[] ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	size ;
	int	c = 0 ;

	QDIR	*qdirp = NULL ;

	char	*p ;


	if (dirname == NULL)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("quote_diropen: dirname=%s\n",dirname) ;
#endif

	size = sizeof(QDIR) ;
	rs = uc_malloc(size,&p) ;
	if (rs < 0)
	    goto bad0 ;

	qdirp = (QDIR *) p ;
	rs1 = qdir_open(qdirp,sip,dirname) ;
	if (rs1 == SR_NOENT)
	    goto bad1 ;

	rs = rs1 ;
	c += rs1 ;
	if (rs < 0)
	    goto bad1 ;

	rs = vechand_add(&op->dirs,qdirp) ;
	if (rs < 0)
	    goto bad1 ;

ret0:

#if	CF_DEBUGS
	debugprintf("quote_diropen: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;

bad1:
	uc_free(qdirp) ;

bad0:
	goto ret0 ;
}
/* end subroutine (quote_diropen) */


static int quote_dirclose(op,qdirp)
QUOTE		*op ;
QDIR		*qdirp ;
{
	int	rs ;
	int	rs1 ;


	rs = cal_close(qdirp) ;

	rs1 = vechand_ent(&op->cals,qdirp) ;
	if (rs1 >= 0)
	    vechand_del(&op->dirs,rs1) ;

	uc_free(qdirp) ;

	return rs ;
}
/* end subroutine (quote_dirclose) */


static int quote_qdircreate(op,sip,dirname,calname)
QUOTE		*op ;
struct subinfo	*sip ;
const char	dirname[] ;
const char	calname[] ;
{
	struct ustat	sb ;

	QUOTE_CAL	*calp ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	cidx ;
	int	size = sizeof(QUOTE_CAL) ;
	int	f = FALSE ;

	const char	*suf = QUOTE_DBSUF ;

	char	cname[MAXNAMELEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;


	rs = snsds(cname,MAXNAMELEN,calname,suf) ;

	if (rs >= 0)
	    rs = mkpath2(tmpfname,dirname,cname) ;

	if (rs < 0)
	    goto bad0 ;

	rs1 = u_stat(tmpfname,&sb) ;
	if (rs1 >= 0)
	    rs1 = sperm(&sip->id,&sb,R_OK) ;

#if	CF_DEBUGS
	debugprintf("quote_qdircreate: fn=%s (%d)\n",tmpfname,rs1) ;
#endif

	if (rs1 < 0)
	    goto bad0 ;

	f = f || S_ISREG(sb.st_mode) ;
	f = f || S_ISCHR(sb.st_mode) ;
	f = f || S_ISSOCK(sb.st_mode) ;
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
/* end subroutine (quote_qdircreate) */


static int quote_qdirdestroy(op,calp)
QUOTE		*op ;
QUOTE_CAL	*calp ;
{
	int	rs ;
	int	rs1 ;


	rs = cal_close(calp) ;

	rs1 = vechand_ent(&op->cals,calp) ;
	if (rs1 >= 0)
	    vechand_del(&op->cals,rs1) ;

	uc_free(calp) ;

	return rs ;
}
/* end subroutine (quote_qdirdestroy) */


#ifdef	COMMENT

static int quote_checkupdate(op,daytime)
QUOTE		*op ;
time_t		daytime ;
{
	struct subinfo	si ;

	struct ustat	sb ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	f = FALSE ;


	if (op->ncursors > 0)
	    goto ret0 ;

	if (daytime <= 0)
	    daytime = time(NULL) ;

	if ((daytime - op->ti_lastcheck) < TO_CHECK)
	    goto ret0 ;

	op->ti_lastcheck = daytime ;
	rs1 = u_stat(op->dbfname,&sb) ;
	if (rs1 < 0)
	    goto ret0 ;

	if ((sb.st_mtime > op->ti_db) || (sb.st_mtime > op->ti_map)) {

	    f = TRUE ;
	    quote_dbloadfree(op) ;

	    rs = subinfo_start(&si,op,op->daytime,NULL) ;
	    if (rs >= 0) {

	        rs = quote_dbloadinit(op,&si) ;

		subinfo_finish(&si) ;

	    } /* end if */

	} /* end if (update) */

ret0:
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (quote_checkupdate) */

#endif /* COMMENT */


static int quote_loadbuf(op,ep,rbuf,rlen)
QUOTE		*op ;
QUOTE_ENT	*ep ;
char		rbuf[] ;
int		rlen ;
{
	QUOTE_CAL	*calp ;

	int	rs ;
	int	cidx ;


	cidx = ep->cidx ;

#if	CF_DEBUGS
	debugprintf("quote_loadbuf: cidx=%d\n",cidx) ;
#endif

	rs = vechand_get(&op->cals,cidx,&calp) ;
	if (rs < 0)
	    goto ret0 ;

	rs = cal_loadbuf(calp,ep,rbuf,rlen) ;

ret0:

#if	CF_DEBUGS
	debugprintf("quote_loadbuf: ret rs=%d\n",rs) ;
#endif

	return rs ;
} 
/* end subroutine (quote_loadbuf) */


static int qdir_open(qdirp,sip,dirname)
QDIR		*qdirp ;
struct subinfo	*sip ;
const char	dirname[] ;
{
	QUOTE	*op = sip->op ;

	TEXTLOOK_INFO	tinfo ;

	time_t	mtime ;

	int	rs = SR_NOENT ;
	int	dbl ;
	int	f_remake = FALSE ;
	int	f_textlook = FALSE ;

	const char	*basedname ;
	const char	*dbdir = QDIR_DBDIR ;

	char	dbdname[MAXPATHLEN + 1] ;
	char	dbname[MAXPATHLEN + 1] ;
	char	*dbp ;


	dbl = sfbasename(dirname,-1,&dbp) ;
	if (dbl <= 0)
	    goto ret0 ;

	memset(qdirp,0,sizeof(QDIR)) ;

	rs = mkpath2(dbdname,dirname,dbdir) ;
	if (rs < 0)
	    goto ret0 ;

	rs = mkpath2w(dbname,dbdname,dbp,dbl) ;
	if (rs < 0)
	    goto ret0 ;

	rs = qdir_dbdir(qdirp,sip,dbdname) ;
	if ((rs1 == SR_NOENT) || (rs1 == SR_ACCESS) || (rs1 == SR_NOTDIR))
	    goto ret0 ;

	rs = rs1 ;
	if (rs < 0)
	    goto ret0 ;

	rs = uc_mallocstrw(dirname,-1,&qdirp->dirname) ;
	if (rs < 0)
	    goto bad0 ;

	basedname = dirname ;
	rs1 = textlook_open(&qdirp->looker,op->pr,dbname,basedname) ;
	f_textlook = (rs1 >= 0) ;
	if (rs1 == SR_NOENT) {
	    f_remake = TRUE ;
	} else
	    rs = rs1 ;

	if (rs < 0)
	    goto bad1 ;

	if (! f_remake) {

	    rs = textlook_info(&qdirp->looker,&binfo) ;

	    if (rs >= 0) {
		mtime = binfo.mtime ;
	        rs = qdir_newer(qdirp,dirname,mtime) ;
		f_remake = (rs > 0) ;
	    }

	} /* end if */

	if (rs < 0)
	    goto bad2 ;

	if (f_remake) {
	    if (f_textlook) {
		f_textlook = FALSE ;
	        textlook_close(&qdirp->looker) ;
	    }
	    rs = qdir_remake(qdirp,sip,dbname,basedname) ;
	    if (rs >= 0) {
	        rs = textlook_open(&qdirp->looker,op->pr,dbname,basedname) ;
		f_textlook = (rs >= 0) ;
	    }
	}

	if (rs < 0)
	    goto bad2 ;

ret0:

#if	CF_DEBUGS
	debugprintf("cal_open: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad2:
	if (f_textlook) {
		f_textlook = FALSE ;
	        textlook_close(&qdirp->looker) ;
	}

bad1:
	uc_free(qdirp->dirname) ;

bad0:
	goto ret0 ;
}
/* end subroutine (qdir_open) */


static int qdir_close(qdirp)
QDIR		*qdirp ;
{
	int	rs ;


	rs = textlook_close(&qdirp->looker) ;

	if (qdirp->dirname != NULL) {
	    uc_free(qdirp->dirname) ;
	    qdirp->dirname = NULL ;
	}

	return rs ;
}
/* end subroutine (qdir_close) */


static int qdir_dbdir(qdirp,sip,dbdname)
QDIR		*qdirp ;
struct subinfo	*sip ;
const char	dbdname[] ;
{
	struct ustat	sb ;

	const int	dmode = QDIR_DBDIRMODE ;

	int	rs = SR_OK ;
	int	rs1 ;


	rs = u_stat(dbdname,&sb) ;

	if ((rs >= 0) && (! S_ISDIR(sb.st_mode)))
	    rs = SR_NOTDIR ;

	if (rs == SR_NOENT) {
	    dmode |= 0755 ;
	    rs = u_mkdir(dbdname,dmode) ;
	    if (rs >= 0) {
		qdirp->f.writedbdir = TRUE :
		rs = uc_minmod(dbdname,dmode) ;
	    }
	}

	if ((rs >= 0) && (! qdirp->f.writedbdir)) {
	    rs = subinfo_ids(sip) ;
	    if (rs >= 0) {
	        rs1 = sperm(&sip->id,&sb,W_OK) ;
		qdirp->f.writedbdir = (rs1 >= 0) ;
	    }
	}

ret0:
	return rs ;
}
/* end subroutine (qdir_dbdir) */


static int qdir_newer(qdirp,dirname,mtime)
QDIR		*qdirp ;
const char	dirname[] ;
time_t		mtime ;
{
	struct ustat	sb ;

	FSDIR		d ;

	FSDIR_ENT	de ;

	time_t		mtime = 0 ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	f_dbdir = FALSE ;
	int	f_newer = FALSE ;

	const char	*dbdir = QDIR_DBDIR ;
	const char	*denp ;

	char	fname[MAXPATHLEN + 1] ;


	if (dirname[0] == '\0')
	    return SR_NOENT ;

	if ((rs = fsdir_open(&d,dirname)) >= 0) {

	    while (fsdir_read(&d,&de) > 0) {

	        denp = de.name ;
	        if (! f_dbdir) {
	            if (strcmp(denp,dbdir) == 0) 
		        f_dbdir = TRUE ;
	        }

	        if (denp[0] == '.') continue ;

	        rs1 = mkpath2(fname,dirname,denp) ;
	        if (rs1 >= 0) {
	            rs1 = u_stat(fname,&sb) ;
		    if ((rs1 >= 0) && (! S_ISDIR(sb.st_mode))) {
		         if (sb.st_mtime > mtime) {
			    f_newer = TRUE ;
			    break ;
		         }
		    }
	        }

	    } /* end while (reading directory entries) */

	    fsdir_close(&d) ;

	} /* end if (directory traversal) */

ret0:
	return (rs >= 0) ? f_newer : rs ;
}
/* end subroutine (qdir_newer) */


static int qdir_remake(qdirp,sip,dbname,basedname) ;
QDIR		*qdirp ;
struct subinfo	*sip ;
const char	dbname[] ;
const char	basedname[] ;
{
	QUOTE	*op = sip->op ;

	int	rs = SR_OK ;



	return rs ;
}
/* end subroutine (qdir_remake) */


#ifdef	COMMENT

static int cal_open(calp,sip,cidx,dirname,calname)
QUOTE_CAL	*calp ;
struct subinfo	*sip ;
int		cidx ;
const char	dirname[] ;
const char	calname[] ;
{
	int	rs ;


	memset(calp,0,sizeof(QUOTE_CAL)) ;

	calp->cidx = cidx ;
	rs = uc_mallocstrw(dirname,-1,&calp->dirname) ;
	if (rs < 0)
	    goto bad0 ;

	rs = uc_mallocstrw(calname,-1,&calp->calname) ;
	if (rs < 0)
	    goto bad1 ;

	rs = cal_dbloadinit(calp,sip) ;
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
QUOTE_CAL	*calp ;
{
	int	rs ;


	rs = cal_dbloadfree(calp) ;

	if (calp->calname != NULL) {
	    uc_free(calp->calname) ;
	    calp->calname = NULL ;
	}

	if (calp->dirname != NULL) {
	    uc_free(calp->dirname) ;
	    calp->dirname = NULL ;
	}

	return rs ;
}
/* end subroutine (cal_close) */


static int cal_dbloadinit(calp,sip)
QUOTE_CAL	*calp ;
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
/* end subroutine (cal_dbloadinit) */


static int cal_dbloadfree(calp)
QUOTE_CAL	*calp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	rs1 = cal_indclose(calp) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = cal_dbmapdestroy(calp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (cal_dbloadfree) */


static int cal_dbmapcreate(calp,daytime)
QUOTE_CAL	*calp ;
time_t		daytime ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	fd ;
	int	mprot, mflags ;

	char	cname[MAXNAMELEN + 1] ;
	char	dbfname[MAXPATHLEN + 1] ;
	char	*mp ;


	rs = snsds(cname,MAXNAMELEN,calp->calname,QUOTE_DBSUF) ;
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

	mprot = PROT_READ ;
	mflags = MAP_SHARED ;
	rs = u_mmap(NULL,(size_t) calp->filesize,mprot,mflags,fd,0L,&mp) ;

	if (rs >= 0) {
	    calp->mapdata = mp ;
	    calp->mapsize = calp->filesize ;
	    calp->ti_map = daytime ;
	    calp->ti_lastcheck = daytime ;
	}

/* close it */
ret1:
	u_close(fd) ;

ret0:
	return rs ;
}
/* end subroutine (cal_dbmapcreate) */


static int cal_dbmapdestroy(calp)
QUOTE_CAL	*calp ;
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
QUOTE_CAL	*calp ;
struct subinfo	*sip ;
{
	int	rs = SR_NOENT ;


	rs = subinfo_year(sip) ; /* the current year is needed later */
	if (rs < 0)
	    goto ret0 ;

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
QUOTE_CAL	*calp ;
struct subinfo	*sip ;
{
	int	rs ;
	int	year = sip->year ;
	int	f_search = FALSE ;
	int	f ;

	const char	*idxdname = IDXDNAME ;

	char	idname[MAXNAMELEN + 1] ;


	rs = mkpath2(idname,calp->dirname,idxdname) ;
	if (rs < 0)
	    goto ret0 ;

try:
	rs = cal_indopencheck(calp,idname,year,f_search) ;

	if (rs == SR_NOTDIR)
	    rs = cal_mkdirs(calp,idname,QUOTE_DMODE) ;

	f = FALSE ;
	f = f || (rs == SR_NOENT) ;
	f = f || (rs == SR_STALE) ;
	f = f || (rs == SR_NOCSI) ; /* zero sized file */
	if (f) {
	    rs = cal_indmk(calp,sip,idname,f_search,sip->daytime) ;
	    if (rs >= 0)
		rs = cal_indopencheck(calp,idname,year,f_search) ;
	}

ret0:
	return rs ;
}
/* end subroutine (cal_indopenperm) */


static int cal_indopentmp(calp,sip)
QUOTE_CAL	*calp ;
struct subinfo	*sip ;
{
	QUOTE	*op = sip->op ;

	int	rs ;
	int	year = sip->year ;
	int	f_search = TRUE ;
	int	f ;

	const char	*idxdname = IDXDNAME ;

	char	idname[MAXPATHLEN + 1] ;


#if	CF_TMPPRNAME
	{

	    char	*prname ;
	

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

	    rs = cal_mkdirs(calp,idname,QUOTE_DMODE) ;

#if	CF_DEBUGS
	debugprintf("cal_indopentmp: cal_mkdirs() rs=%d\n",rs) ;
#endif

	}

	f = FALSE ;
	f = f || (rs == SR_NOENT) ;
	f = f || (rs == SR_STALE) ;
	f = f || (rs == SR_NOCSI) ; /* zero sized file */
	if (f) {
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

	return rs ;
}
/* end subroutine (cal_indopentmp) */


static int cal_mkdirs(calp,dname,mode)
QUOTE_CAL	*calp ;
const char	dname[] ;
mode_t		mode ;
{
	struct ustat	sb ;

	int	rs ;


	mode &= S_IAMB ;
	if ((rs = mkdirs(dname,mode)) >= 0) {
	    rs = u_stat(dname,&sb) ;
	    if ((rs >= 0) && ((sb.st_mode & mode) != mode))
		rs = u_chmod(dname,mode) ;
	}

	return rs ;
}
/* end subroutine (cal_mkdirs) */


static int cal_indopencheck(calp,dname,year,f_search)
QUOTE_CAL	*calp ;
const char	dname[] ;
int		year ;
int		f_search ;
{
	TEXTLOOK_INFO	binfo ;

	int	rs = SR_OK ;
	int	f ;


	rs = cyi_open(&calp->vind,dname,calp->calname,f_search) ;

	if (rs >= 0) {
	    rs = cyi_info(&calp->vind,&binfo) ;
	    if (rs >= 0) {
		f = FALSE ;
		f = f || (binfo.ctime < calp->ti_db) ;
		f = f || (binfo.year < year) ;
		if (f) {
		    rs = SR_STALE ;
	            cyi_close(&calp->vind) ;
		}
	    } /* end if (cyi_open) */
	} /* end if */

	calp->f.vind = (rs >= 0) ;
	return rs ;
}
/* end subroutine (quote_indopencheck) */

#endif /* COMMENT */


#if	CF_DEBUGS && CF_DEBUGCUR

static int quote_debugcur(op,rlp,s)
QUOTE		*op ;
vecobj		*rlp ;
const char	s[] ;
{
	struct quote_eline	*lines ;

	QUOTE_ENT		*ep ;

	int	rs = SR_OK ;
	int	n ;
	int	i, j ;


	n = vecobj_count(rlp) ;
	debugprintf("quote_debugcur: %s n=%u\n",s,n) ;
	for (i = 0 ; vecobj_get(rlp,i,&ep) >= 0 ; i += 1) {
		lines = ep->lines ;
	   for (j = 0 ; j < ep->i ; j += 1) {
		debugprintf("quote_debugcur: i=%u loff[%u]=%u\n",
			i,j,lines[j].loff) ;
		debugprintf("quote_debugcur: i=%u llen[%u]=%u\n",
			i,j,lines[j].llen) ;
	    }
	} /* end for */

ret0:
	return rs ;
}
/* end subroutine (quote_debugcur) */

#endif /* CF_DEBUGS */


#ifdef	COMMENT

static int cal_indmk(calp,sip,dname,f_tmp,daytime)
QUOTE_CAL	*calp ;
struct subinfo	*sip ;
const char	dname[] ;
int		f_tmp ;
time_t		daytime ;
{
	int	rs = SR_OK ;
	int	operms = 0664 ;
	int	c = 0 ;


/* check the given directory for writability */

	rs = subinfo_checkdname(sip,dname) ;

	if (rs == SR_NOENT)
	    rs = mkdirs(dname,0777) ; /* will fail if parent is not writable */

	if (rs < 0)
	    goto ret0 ;

	rs = cal_indmkdata(calp,sip,dname,operms,f_tmp) ;
	c += rs ;

	if (rs >= 0)
	    calp->ti_vind = daytime ;

ret0:

#if	CF_DEBUGS
	debugprintf("quote_indmk: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (cal_indmk) */


static int cal_indmkdata(calp,sip,dname,operms,f_tmp)
QUOTE_CAL	*calp ;
struct subinfo	*sip ;
const char	dname[] ;
int		operms ;
int		f_tmp ;
{
	QUOTE_ENT	e ;

	QUOTE_QUERY	q ;

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

	char	*tp, *mp, *lp ;


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
	debugprintf("quote_indmkdata: cidx=%d\n",cidx) ;
#endif

	cn = calp->calname ;
	oflags = 0 ;
	year = sip->year ;
	rs = cyimk_open(&cyind,dname,cn,oflags,operms,year,f_tmp) ;
	if (rs == SR_INPROGRESS)
	    goto retinprogress ;

	if (rs < 0)
	    goto ret0 ;

mkgo:
	mp = calp->mapdata ;
	ml = calp->mapsize ;

#if	CF_DEBUGS
	debugprintf("quote_indmkdata: mp=%p ml=%d\n",mp,ml) ;
#endif

	while ((tp = strnchr(mp,ml,'\n')) != NULL) {

	    len = ((tp + 1) - mp) ;
	    lp = mp ;
	    ll = (len - 1) ;

	    if (! isempty(lp,ll)) {

#if	CF_DEBUGS
		debugprintf("quote_indmkdata: line=>%t<\n",
			lp,strnlen(lp,MIN(ll,40))) ;
		if (ll > 40)
		debugprintf("quote_indmkdata: cont=>%t<\n",
			(lp+40),strnlen((lp+40),MIN((ll-40),40))) ;
#endif

	        rs1 = subinfo_havestart(sip,&q,lp,ll) ;
		si = rs1 ;

#if	CF_DEBUGS
		debugprintf("quote_indmkdata: subinfo_havestart() rs1=%d\n",
			rs1) ;
		debugprintf("quote_indmkdata: q=(%u:%u)\n",q.m,q.d) ;
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

	    if (rs < 0)
	        break ;

	    fileoff += len ;
	    ml -= len ;
	    mp += len ;

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
	debugprintf("quote_indmkdata: ret rs=%d c=%u\n",rs,c) ;
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
	    if (rs1 >= 0)
		goto mkgo ;
	    rs = rs1 ;
	} else
	    rs = SR_TIMEDOUT ;

	goto ret0 ;
}
/* end subroutine (cal_indmkdata) */


static int cal_indclose(calp)
QUOTE_CAL	*calp ;
{
	int	rs = SR_OK ;


	if (calp->f.vind) {
	    calp->f.vind = FALSE ;
	    rs = cyi_close(&calp->vind) ;
	}

	return rs ;
}
/* end subroutine (cal_indclose) */

#endif /* COMMENT */


#ifdef	COMMENT

static int cal_idxset(calp,cidx)
QUOTE_CAL	*calp ;
int		cidx ;
{


	calp->cidx = cidx ;
	return SR_OK ;
}
/* end subroutine (cal_idxset) */

#endif /* COMMENT */


#ifdef	COMMENT

static int cal_idxget(calp)
QUOTE_CAL	*calp ;
{
	int	cidx = calp->cidx ;


	return cidx ;
}
/* end subroutine (cal_idxget) */


static int cal_audit(calp)
QUOTE_CAL	*calp ;
{
	int	rs = SR_OK ;


	rs = cyi_audit(&calp->vind) ;

	return rs ;
}
/* end subroutine (cal_audit) */


static int cal_loadbuf(calp,ep,rbuf,rlen)
QUOTE_CAL	*calp ;
QUOTE_ENT	*ep ;
char		rbuf[] ;
int		rlen ;
{
	int	rs ;

	const char	*mp ;


	rs = cal_mapdata(calp,&mp) ;
	if (rs < 0)
	    goto ret0 ;

	rs = entry_loadbuf(ep,mp,rbuf,rlen) ;

#if	CF_DEBUGS
	debugprintf("quote/cal_loadbuf: entry_loadbuf() rs=%d\n",rs) ;
#endif

ret0:
	return rs ;
} 
/* end subroutine (cal_loadbuf) */


static int cal_mapdata(calp,mpp)
QUOTE_CAL	*calp ;
const char	**mpp ;
{
	int	rs ;


	if (mpp != NULL)
	    *mpp = calp->mapdata ;

	rs = calp->mapsize ;
	return rs ;
}
/* end subroutine (cal_mapdata) */

#endif /* COMMENT */


static int subinfo_start(sip,op,daytime,dirnames)
struct subinfo	*sip ;
QUOTE		*op ;
time_t		daytime ;
const char	*dirnames[] ;
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

	if ((dirnames == NULL) || (dirnames[0] == NULL)) {
	    rs = subinfo_mkdirnames(sip) ;
	} else
	    sip->dirnames = op->dirnames ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_mkdirnames(sip)
struct subinfo	*sip ;
{
	QUOTE	*op = sip->op ;

	int	rs = SR_OK ;
	int	tl ;
	int	c = 0 ;

	const char	*sharedname = QDIR_QUOTES ;

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

	    rs = mkpath2(tmpdname,sip->userhome,sharedname) ;
	    tl = rs ;
	    if (rs >= 0) {
	        rs = subinfo_havedir(sip,tmpdname) ;
	        if (rs > 0) {
		    c += 1 ;
	            rs = vecstr_add(&sip->defdirs,tmpdname,tl) ;
	        }
	    }

	} /* end if */

/* system area */

	if (rs >= 0) {

	    rs = mkpath2(tmpdname,op->pr,sharedname) ;
	    tl = rs ;
	    if (rs >= 0) {
	        rs = subinfo_havedir(sip,tmpdname) ;
	        if (rs > 0) {
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
	int	rs1 ;
	int	f = FALSE ;


	rs1 = u_stat(tmpdname,&sb) ;
	if (rs1 >= 0)
	    f = S_ISDIR(sb.st_mode) ? 1 : 0 ;

ret0:
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


static int subinfo_finish(sip)
struct subinfo	*sip ;
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


static int subinfo_username(sip)
struct subinfo	*sip ;
{
	int		rs = SR_OK ;

	if (sip->username[0] == '\0') {
	    struct passwd	pw ;
	    const int		pwlen = getbufsize(getbufsize_pw) ;
	    char		*pwbuf ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	        if ((rs = getpwusername(&pw,pwbuf,pwlen,-1)) >= 0) {
	            char	*ap ;
		    strwcpy(sip->username,pw.pw_name,USERNAMELEN) ;
		    if ((rs = uc_mallocstrw(pw.pw_dir,-1,&ap)) >= 0) {
		        sip->userhome = ap ;
		    }
		}
		uc_free(pwbuf) ;
	    } /* end if (memory-allocation) */
	} /* end if (needed) */

	return rs ;
}
/* end subroutine (subinfo_username) */


static int subinfo_tmpuserdir(sip)
struct subinfo	*sip ;
{
	const int	dmode = 0775 ;

	int	rs ;
	int	dl ;

	char	tmpdname[MAXPATHLEN + 1] ;
	char	*dp ;


	rs = subinfo_username(sip) ;

	if ((rs >= 0) && (sip->tudname == NULL)) {

	    rs = mktmpuserdir(tmpdname,sip->username,IDXDNAME,dmode) ;
	    dl = rs ;
	    if (rs >= 0) {
	        rs = uc_mallocstrw(tmpdname,dl,&dp) ;
		if (rs >= 0)
		    sip->tudname = dp ;
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

	const char	*calsuf = QUOTE_DBSUF ;
	const char	*tp ;
	const char	*np ;

	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("quote_loadnames: dirname=%s\n",dirname) ;
#endif

	rs = fsdir_open(&dir,dirname) ;
	if (rs < 0)
	    goto ret0 ;

	while (fsdir_read(&dir,&ds) > 0) {
	    if (ds.name[0] == '.') continue ;

	    if (strchr(ds.name,'.') != NULL) continue ; /* just plain names */

	    rs1 = mkpath2(tmpfname,dirname,ds.name) ;

	    if (rs1 >= 0)
	        rs1 = u_stat(ds.name,&db) ;

	    if ((rs1 >= 0) && S_ISREG(sb.st_mode)) {

		    c += 1 ;
		    rs = vecstr_add(nlp,ds.name,-1) ;
	            if (rs < 0)
		        break ;

	    } /* end if (regular file) */

	} /* end while */

ret1:
	fsdir_close(&dir) ;

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_loadnames) */


static int subinfo_havestart(sip,qp,lp,ll)
struct subinfo	*sip ;
QUOTE_CITE	*qp ;
const char	*lp ;
int		ll ;
{
	int		rs1 = SR_OK ;
	int		ch ;
	int		cl ;
	int		si = 0 ;
	int		f ;
	cchar		*tp, *cp ;

#if	CF_DEBUGS
	debugprintf("quote/subinfo_havestart: >%t<\n",
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
	debugprintf("quote/subinfo_havestart: digitlatin\n") ;
#endif

	    tp = strnchr(lp,ll,'/') ;
	    if (tp != NULL) {

	        rs1 = mkmonth(lp,(tp - lp)) ;
	        qp->m = (rs1 & UCHAR_MAX) ;

#if	CF_DEBUGS
		debugprintf("quote/subinfo_havestart: mkmonth() rs1=%d\n",
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
	debugprintf("quote/subinfo_havestart: mkday() rs1=%d\n",rs1) ;
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
	debugprintf("quote/subinfo_havestart: !digitlatin\n") ;
	debugprintf("quote/subinfo_havestart: name=>%t<\n",lp,si) ;
#endif

		rs1 = SR_NOTSUP ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("quote/subinfo_havestart: mid rs=%d si=%u\n",rs1,si) ;
#endif

	if (rs1 >= 0)
	    si += siskipwhite((lp+si),(ll-si)) ;

ret1:
ret0:

#if	CF_DEBUGS
	debugprintf("quote/subinfo_havestart: ret rs=%d si=%u\n",rs1,si) ;
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
	    sip->isdst = tm.tm_isdst ;
	    sip->gmtoff = tm.gmtoff ;
	}

	return rs ;
}
/* end subroutine (subinfo_year) */


#ifdef	COMMENT

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

	return rs ;
}
/* end subroutine (subinfo_mkday) */


static int subinfo_transhol(sip,qp,sp,sl)
struct subinfo	*sip ;
QUOTE_CITE	*qp ;
const char	*sp ;
int		sl ;
{
	HOLIDAYS_CUR	hcur ;

	HOLIDAYS	*holp ;

	QUOTE		*op ;

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
	debugprintf("quote/subinfo_transhol: >%t<\n",sp,sl) ;
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
	debugprintf("quote/subinfo_transhol: n=>%t<\n",np,nl) ;
	debugprintf("quote/subinfo_transhol: f_neg=%u\n",f_negative) ;
	} else
	debugprintf("quote/subinfo_transhol: *no_number*\n") ;
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
	debugprintf("quote/subinfo_transhol: holidays_open() rs=%d\n",
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

	    char	holbuf[HOLBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("quote/subinfo_transhol: fq=>%t<\n",sp,sl) ;
#endif

	    holidays_curbegin(holp,&hcur) ;
	
	    rs = holidays_fetchname(holp,sp,sl,&hcur,&hc,holbuf,HOLBUFLEN) ;
	    if (rs >= 0) {
		f_found = TRUE ;
		qp->m = hc.m ;
		qp->d = hc.d ;
	    }

	    holidays_curend(holp,&hcur) ;

#if	CF_DEBUGS
	debugprintf("quote/subinfo_transhol: "
		"holidays_fetchname() rs=%d\n",rs) ;
	debugprintf("quote/subinfo_transhol: un q=(%u:%u)\n",
		qp->m,qp->d) ;
#endif

	} /* end if */

	if ((rs >= 0) && f_found && (nl > 0)) {

	    if (! f_inityear)
		rs = subinfo_year(sip) ;

	    if (rs >= 0) {
		int	odays ;

		rs = cfdeci(np,nl,&odays) ;

		if (rs >= 0) {
		    time_t	t ;

		    if (f_negative) odays = (- odays) ;

		    memset(&tm,0,sizeof(TMTIME)) ;
		    tm.gmtoff = sip->gmtoff ;
		    tm.isdst = sip->isdst ;
		    tm.year = (sip->year - TM_YEAR_BASE) ;
		    tm.mon = qp->m ;
		    tm.mday = (qp->d + odays) ;
		    rs = tmtime_adjtime(&tm,&t) ;

		    if (rs >= 0) {
			qp->m = tm.mon ;
			qp->d = tm.mday ;
		    }

#if	CF_DEBUGS
	debugprintf("quote/subinfo_transhol: adjusted q=(%u:%u)\n",
		qp->m,qp->d) ;
#endif

		} /* end if (odays) */

	    } /* end if (got year) */

	} /* end if (day offset required) */

ret0:

#if	CF_DEBUGS
	debugprintf("quote/subinfo_transhol: ret rs=%d f_found=%u\n",
		rs,f_found) ;
#endif

	return (rs >= 0) ? f_found : rs ;
}
/* end subroutine (subinfo_transhol) */

#endif /* COMMENT */


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


static int entry_start(ep,qp,loff,llen)
QUOTE_ENT	*ep ;
QUOTE_CITE	*qp ;
int		loff, llen ;
{
	struct quote_eline	*elp ;

	int	rs = SR_OK ;
	int	ne = QUOTE_NLE ;
	int	size ;


	if (ep == NULL)
	    return SR_FAULT ;

	memset(ep,0,sizeof(QUOTE_ENT)) ;

	ep->cidx = -1 ;
	ep->m = qp->m ;
	ep->d = qp->d ;
	ep->voff = loff ;
	ep->vlen = llen ;
	size = ne * sizeof(struct quote_eline) ;
	rs = uc_malloc(size,&elp) ;

	if (rs >= 0) {
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
QUOTE_ENT	*ep ;
int		cidx ;
{


	if (ep == NULL)
	    return SR_FAULT ;

	ep->cidx = cidx ;
	return SR_OK ;
}

/* end subroutine (entry_setidx) */


static int entry_add(ep,loff,llen)
QUOTE_ENT	*ep ;
uint		loff, llen ;
{
	struct quote_eline	*elp ;

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
	    ne = (ep->e * 2) + QUOTE_NLE ;
	    size = ne * sizeof(struct quote_eline) ;
	    rs = uc_realloc(ep->lines,size,&elp) ;
	    if (rs >= 0) {
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


#ifdef	COMMENT

static int entry_release(ep)
QUOTE_ENT	*ep ;
{


	if (ep == NULL)
	    return SR_FAULT ;

	if (ep->e <= 0)
	    return SR_NOTOPEN ;

	if ((ep->i < 0) || (ep->i > ep->e))
	    return SR_BADFMT ;

	ep->i = 0 ;
	ep->lines = NULL ;
	return SR_OK ;
}
/* end subroutine (entry_release) */

#endif /* COMMENT */


static int entry_finish(ep)
QUOTE_ENT	*ep ;
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
QUOTE_ENT	*ep ;
QUOTE_ENT	*oep ;
{
	int	rs1 = SR_OK ;


	if ((ep->m == oep->m) && (ep->d == oep->d))
	    rs1 = 1 ;

	return rs1 ;
}
/* end subroutine (entry_samecite) */

#endif /* CF_SAMECITE */


static int entry_samehash(ep,op,oep)
QUOTE_ENT	*ep ;
QUOTE	*op ;
QUOTE_ENT	*oep ;
{
	int	rs = SR_OK ;


/* the following checks (code) is not needed in the present implementation! */

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
QUOTE_ENT	*ep ;
QUOTE	*op ;
{
	struct quote_eline	*elp ;

	QUOTE_CAL	*calp ;

	uint	hash ;

	int	rs = SR_OK ;
	int	i ;
	int	sl, cl ;

	const char	*sp ;
	const char	*mp ;

	char	*cp ;


	if (ep == NULL)
	    return SR_FAULT ;

	if (ep->e <= 0)
	    return SR_NOTOPEN ;

	if (ep->lines == NULL)
	    return SR_NOTOPEN ;

	rs = vechand_get(&op->cals,ep->cidx,&calp) ;
	if (rs >= 0)
	    rs = cal_mapdata(calp,&mp) ;

	if (rs < 0)
	    goto ret0 ;

	elp = ep->lines ;
	hash = 0 ;
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

ret0:
	return rs ;
}
/* end subroutine (entry_mkhash) */


static int entry_sethash(ep,hash)
QUOTE_ENT	*ep ;
uint		hash ;
{


	ep->hash = hash ;
	ep->f.hash = TRUE ;
	return SR_OK ;
}
/* end subroutine (entry_sethash) */


static int entry_same(ep,op,oep)
QUOTE_ENT	*ep ;
QUOTE		*op ;
QUOTE_ENT	*oep ;
{
	WORDER	w1, w2 ;

	int	rs = SR_OK ;
	int	c1l, c2l ;
	int	f = FALSE ;

	const char	*c1p, *c2p ;


	rs = worder_start(&w1,op,ep) ;
	if (rs < 0)
	    goto bad0 ;

	rs = worder_start(&w2,op,oep) ;
	if (rs < 0)
	    goto bad1 ;

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

	worder_finish(&w1) ;

ret0:
	return (rs >= 0) ? f : rs ;

/* bad stuff (of course!) :-) */
bad1:
	worder_finish(&w1) ;

bad0:
	goto ret0 ;
}
/* end subroutine (entry_same) */


static int entry_loadbuf(ep,mp,rbuf,rlen)
QUOTE_ENT	*ep ;
const char	*mp ;
char		rbuf[] ;
int		rlen ;
{
	struct quote_eline	*lines ;
	SBUF		b ;
	int		rs ;
	int		rs1 ;
	int		i ;
	int		nlines ;
	int		ll ;
	int		len = 0 ;
	const char	*lp ;

	lines = ep->lines ;
	nlines = ep->i ; /* number of line elements */

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {

	for (i = 0 ; i < nlines ; i += 1) {

	    if (i > 0)
		sbuf_char(&b,' ') ;

	    lp = (mp + lines[i].loff) ;
	    ll = lines[i].llen ;

#if	CF_DEBUGS
	    debugprintf("cal_loadbuf: i=%u loff=%u llen=%u\n",
	        i,lines[i].loff,lines[i].llen) ;
#endif

	    rs = sbuf_strw(&b,lp,ll) ;
	    if (rs < 0)
	        break ;

	} /* end for */

	len = sbuf_finish(&b) ;
	if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

	return (rs >= 0) ? len : rs ;
} 
/* end subroutine (entry_loadbuf) */


#ifdef	COMMENT

static int mkbve_start(bvep,sip,ep)
CYIMK_ENT	*bvep ;
struct subinfo	*sip ;
QUOTE_ENT	*ep ;
{
	CYIMK_LINE	*lines ;

	uint	nlines = 0 ;

	int	rs = SR_OK ;
	int	size ;
	int	i ;


	if (ep == NULL)
	    return SR_FAULT ;

	rs = entry_mkhash(ep,sip->op) ;
	if (rs < 0)
	    goto ret0 ;

	bvep->m = ep->m ;
	bvep->d = ep->d ;
	bvep->voff = ep->voff ;
	bvep->vlen = ep->vlen ;
	bvep->hash = ep->hash ;
	bvep->lines = NULL ;

	nlines = ep->i ;
	if (nlines <= UCHAR_MAX) {

	    bvep->nlines = nlines ;
	    size = (nlines + 1) * sizeof(CYIMK_LINE) ;
	    rs = uc_malloc(size,&lines) ;
	    if (rs >= 0) {

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

ret0:
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

#endif /* COMMENT */


int worder_start(wp,op,ep)
WORDER		*wp ;
QUOTE	*op ;
QUOTE_ENT	*ep ;
{
	struct quote_eline	*lines ;

	QUOTE_CAL	*calp ;

	int	rs ;

	const char	*mp ;


	rs = vechand_get(&op->cals,ep->cidx,&calp) ;
	if (rs >= 0)
	    rs = cal_mapdata(calp,&mp) ;

	if (rs < 0)
	    goto ret0 ;

	wp->i = 0 ;
	lines = ep->lines ;
	wp->nlines = ep->i ;
	wp->lines = ep->lines ;
	wp->mp = mp ;
	if (lines != NULL) {
	    wp->sp = (mp + lines[0].loff) ;
	    wp->sl = (lines[0].llen) ;
	}

ret0:
	return rs ;
}
/* end subroutine (worder_start) */


int worder_finish(wp)
WORDER		*wp ;
{

	return SR_OK ;
}
/* end subroutine (worder_finish) */


int worder_get(wp,rpp)
WORDER		*wp ;
const char	**rpp ;
{
	int	cl = 0 ;

	char	*cp ;


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

ret0:
	return cl ;
}
/* end subroutine (worder_get) */


static int isempty(lp,ll)
const char	*lp ;
int		ll ;
{
	int	cl ;
	int	f = FALSE ;

	char	*cp ;


	f = f || (ll == 0) ;
	f = f || (lp[0] == '#') ;
	if ((! f) && CHAR_ISWHITE(*lp)) {
	    cl = sfskipwhite(lp,ll,&cp) ;
	    f = ((cl == 0) || (cp[0] == '#')) ;
	}

	return f ;
}
/* end subroutine (isempty) */


/* for use with 'vecobj_sort(3dam)' or similar */
static int vrcmp(v1p,v2p)
const void	*v1p, *v2p ;
{
	QUOTE_ENT	*e1p, **e1pp = (QUOTE_ENT **) v1p ;
	QUOTE_ENT	*e2p, **e2pp = (QUOTE_ENT **) v2p ;

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


