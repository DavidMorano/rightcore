/* textlook */

/* bible-query database manager */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* special debugging */
#define	CF_EXTRASTRONG	0		/* don't use Strong's eigen-words */
#define	CF_EXTRAEIGEN	0		/* perform extra EIGEN-DB check */
#define	CF_SINGLEWORD	1		/* treat extra words as single */
#define	CF_LOOKPARALLEL	1		/* perform lookup in parallel */
#define	CF_MULTIPROCESS	1		/* multi-process */
#define	CF_TESTERROR	0		/* test thread error-exit */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little object provides access to the TEXTLOOK database and index
	(if any).

	Note on Strong's eigen-words: There is a compile-time switch
	('CF_EXTRASTRONG') that chooses between using an internal list of
	Strong's 1980 set of eigen-words; or, alternatively, to use an
	eigen-database on the current system.  Using the Strong's list (an
	internally stored list) has the advantage of giving query consistent
	results with what would be returned if one was to actually use Strong's
	concordance.  The disadvantage of using the internal list (Strong's
	list) is that it is small and may make queries a little bit more time
	consuming than would be the case when using a typical system eigen-word
	list (although this should be a very small effect at best).

	Note that any eigen-word list can be used because the list is stored in
	the index of the DB so that the same list is always used on queries as
	was used in the original creation of the index itself.  The DB proper
	only stores the real data, no eigen-words; so eigen-word lists can be
	changed on every recreation of the index.

	Notes:

	= Number of ourstanding lookups: Only one query can be outstanding at a
	time.  But any number of queries can be done successively.

	= On memory usage and the work queue: We limit the number of items that
	the producer thread puts onto the work queue (to some reasonable number
	like about (15 * n-cpus) because the work queue forms a real FIFO.  The
	items that are being FIFOed are text-reference tags.  These tags each
	can be something over MAXPATHLEN in length.  And the main producer
	thread can easily produce thousands of such tags since they represent
	files or pieces of files to be verified for queue-key content.  So if
	we multiply up what is possible, we can easily get something like (2048
	* MAXPATHLEN) amount of data just in the work queue FIFO!  This will
	blow the memory limit on most systems with only a 4-GB memory address
	space to start with.  So my limiting the number of items in the work
	queue to some reasonable maximum we avoid this problem while still
	maintaining maximum concurrency.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<char.h>
#include	<vechand.h>
#include	<vecstr.h>
#include	<expcook.h>
#include	<storebuf.h>
#include	<eigendb.h>
#include	<ids.h>
#include	<ascii.h>
#include	<field.h>
#include	<sbuf.h>
#include	<bfile.h>
#include	<psem.h>
#include	<ptm.h>
#include	<ciq.h>
#include	<endianstr.h>
#include	<localmisc.h>

#include	"naturalwords.h"
#include	"txtindexmk.h"
#include	"txtindex.h"
#include	"xwords.h"
#include	"textlook.h"
#include	"searchkeys.h"
#include	"rtags.h"
#include	"upt.h"


/* local defines */

#define	TEXTLOOK_NVERSES	33000
#define	TEXTLOOK_MINWLEN	2		/* minimum word-length */
#define	TEXTLOOK_MAXWLEN	6		/* less collisions? */
#define	TEXTLOOK_NEIGEN		2000		/* number of keys in chunk */
#define	TEXTLOOK_DIRPERM	0775		/* parent directory */
#define	TEXTLOOK_INDPERM	0664		/* the index files */
#define	TEXTLOOK_DBDNAME	"share/bibledbs"
#define	TEXTLOOK_DBNAME		"av"
#define	TEXTLOOK_QLEN		16
#define	TEXTLOOK_MAXRECLEN	(10*1024*1024)

#ifndef	VARSYSNAME
#define	VARSYSNAME	"SYSNAME"
#endif

#ifndef	VARRELEASE
#define	VARRELEASE	"RELEASE"
#endif

#ifndef	VARVERSION
#define	VARVERSION	"VERSION"
#endif

#ifndef	VARMACHINE
#define	VARMACHINE	"MACHINE"
#endif

#ifndef	VARARCHITECTURE
#define	VARARCHITECTURE	"ARCHITECTURE"
#endif

#ifndef	VARHZ
#define	VARHZ		"HZ"
#endif

#ifndef	VARDOMAIN
#define	VARDOMAIN	"DOMAIN"
#endif

#ifndef	VARNODE
#define	VARNODE		"NODE"
#endif

#ifndef	VARHOMEDNAME
#define	VARHOMEDNAME	"HOME"
#endif

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#ifndef	VARLOGNAME
#define	VARLOGNAME	"LOGNAME"
#endif

#ifndef	VARTZ
#define	VARTZ		"TZ"
#endif

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"
#endif

#undef	VARDBNAME
#define	VARDBNAME	"MKTEXTLOOKI_DBNAME"

#undef	VARPRTEXTLOOK
#define	VARPRTEXTLOOK	"MKTEXTLOOKI_PROGRAMROOT"

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	TMPVARDNAME
#define	TMPVARDNAME	"/var/tmp"
#endif

#ifndef	NCPUMAX
#define	NCPUMAX		256
#endif

#define	INDDNAME	"textlook"

#define	DBSUF		"txt"
#define	INDSUF		"hash"
#define	TAGSUF		"tag"

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	NATURALWORDLEN
#endif

#define	TO_FILEMOD	(60 * 24 * 3600)
#define	TO_MKWAIT	(5 * 50)

#define	PROG_MKTEXTLOOKI	"mktextlooki"

#ifndef	TAGBUFLEN
#define	TAGBUFLEN	MAX((MAXPATHLEN + 40),(2 * 1024))
#endif

#ifndef	LOWBUFLEN
#define	LOWBUFLEN	NATURALWORDLEN
#endif

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#define	WARGS		struct disp_wargs
#define	DISP		struct disp_head
#define	DISP_THREAD	struct disp_thread
#define	TAGQ		struct tagq 
#define	TAGQ_THING	struct tagq_thing

#define	NDEBFNAME	"textlook.deb"


/* external subroutines */

extern uint	ufloor(uint,int) ;
extern uint	uceil(uint,int) ;
extern uint	hashelf(void *,int) ;

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy4(char *,int, const char *,const char *,
			const char *,const char *) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	snwcpylc(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sfword(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	strpcmp(const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	field_word(FIELD *,const uchar *,const char **) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	getnprocessors(const char **,int) ;
extern int	msleep(int) ;
extern int	ifloor(int,int) ;
extern int	iceil(int,int) ;
extern int	haslc(const char *,int) ;
extern int	hasuc(const char *,int) ;
extern int	isalnumlatin(int) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	nprintf(const char *,const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* exported variables */

TEXTLOOK_OBJ	textlook = {
	"textlook",
	sizeof(TEXTLOOK),
	sizeof(TEXTLOOK_CUR)
} ;


/* local structures */

struct subinfo_flags {
	uint		id:1 ;
} ;

struct subinfo {
	IDS		id ;
	SUBINFO_FL	f ;
	time_t		daytime ;
} ;

struct tagq {
	PSEM		wsem ;
	CIQ		q ;
} ;

struct tagq_thing {
	uint		recoff ;
	uint		reclen ;
	char		fname[1] ;
} ;

struct disp_wargs {
	DISP		*dop ;
	TEXTLOOK	*op ;
	RTAGS		*rtp ;
	SEARCHKEYS	*skp ;
	const uchar	*terms ;
	int		qo ;		/* query options */
} ;

struct disp_thread {
	WARGS		*wap ;
	pthread_t	tid ;
	volatile int	f_busy ;
	volatile int	f_exited ;
	volatile int	rs ;
} ;

struct disp_head {
	WARGS		wa ;		/* worker arguments */
	TAGQ		wq ;		/* work-queue */
	PSEM		sem_wq ;	/* work-queue semaphore */
	PSEM		sem_done ;	/* done-semaphore */
	PTM		mutex_threads ;	/* nbusy-mutex */
	vechand		threads ;	/* thread structures (w/ t-IDs) */
	volatile int	f_exit ;	/* assumed atomic */
	volatile int	f_done ;	/* assumed atomic */
	int		npar ;		/* n-parallelism */
	int		qlen ;		/* max work-queue length */
} ;


/* forward references */

static int	textlook_infoloadbegin(TEXTLOOK *,const char *,const char *) ;
static int	textlook_infoloadend(TEXTLOOK *) ;
static int	textlook_indopen(TEXTLOOK *,SUBINFO *) ;

static int	textlook_snbegin(TEXTLOOK *) ;
static int	textlook_snend(TEXTLOOK *) ;

#if	CF_LOOKPARALLEL
static int	textlook_dispstart(TEXTLOOK *) ;
static int	textlook_dispfinish(TEXTLOOK *) ;
#endif

static int	textlook_indclose(TEXTLOOK *) ;
static int	textlook_havekeys(TEXTLOOK *,TXTINDEX_TAG *,int,SEARCHKEYS *) ;
static int	textlook_havekeysline(TEXTLOOK *,
			SEARCHKEYS *,SEARCHKEYS_POP *,const char *,int) ;
static int	textlook_matchkeys(TEXTLOOK *,
			SEARCHKEYS *,SEARCHKEYS_POP *,const char *,int) ;
static int	textlook_mkhkeys(TEXTLOOK *,vecstr *,SEARCHKEYS *) ;

#if	CF_LOOKPARALLEL
static int	textlook_lookparallel(TEXTLOOK *,TEXTLOOK_CUR *,
			int,SEARCHKEYS *,const char **) ;
static int	textlook_checkdisp(TEXTLOOK *) ;
#else
static int	textlook_lookserial(TEXTLOOK *,TEXTLOOK_CUR *,
			int,SEARCHKEYS *,const char **) ;
#endif /* CF_LOOKPARALLEL */

#if	CF_EXTRASTRONG
static int	textlook_eigenopen(TEXTLOOK *) ;
static int	textlook_eigenclose(TEXTLOOK *) ;
#endif

static int	subinfo_start(SUBINFO *) ;
static int	subinfo_finish(SUBINFO *) ;

#ifdef	COMMENT
static int	subinfo_ids(SUBINFO *) ;
#endif

#if	CF_EXTRASTRONG
static int	eigenfind(EIGENDB *,const char *,const char *,int) ;
#endif

#if	CF_LOOKPARALLEL
static int	disp_start(DISP *,TEXTLOOK *) ;
static int	disp_finish(DISP *,int) ;
static int	disp_setparams(DISP *,int,SEARCHKEYS *,RTAGS *) ;
static int	disp_addwork(DISP *,TXTINDEX_TAG *) ;
static int	disp_setstate(DISP *,DISP_THREAD *,int) ;
static int	disp_nbusy(DISP *) ;
static int	disp_nexited(DISP *) ;
static int	disp_waitdone(DISP *) ;
static int	disp_threadsort(DISP *) ;
static int	worker(void *) ;
#endif /* CF_LOOKPARALLEL */

#if	CF_LOOKPARALLEL
static int	tagq_start(TAGQ *,int) ;
static int	tagq_finish(TAGQ *) ;
static int	tagq_count(TAGQ *) ;
static int	tagq_ins(TAGQ *,TXTINDEX_TAG *) ;
static int	tagq_rem(TAGQ *,TXTINDEX_TAG *) ;
#endif /* CF_LOOKPARALLEL */

static int	mkfieldterms(uchar *) ;

#if	CF_LOOKPARALLEL
static int	vcmpthreads(DISP_THREAD **,DISP_THREAD **) ;
#endif /* CF_LOOKPARALLEL */


/* local variables */


/* exported subroutines */


int textlook_open(TEXTLOOK *op,cchar *pr,cchar *dbname,cchar *basedname)
{
	SUBINFO		si ;
	int		rs ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;
	if (dbname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("textlook_open: dbname=%s\n",dbname) ;
#endif

	memset(op,0,sizeof(TEXTLOOK)) ;
	op->pr = pr ;
	op->pagesize = getpagesize() ;

	if ((rs = textlook_infoloadbegin(op,dbname,basedname)) >= 0) {
	    if ((rs = subinfo_start(&si)) >= 0) {
	        {
		    rs = textlook_indopen(op,&si) ;
		}
	        rs1 = subinfo_finish(&si) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if */
	    if (rs < 0) {
#if	CF_EXTRASTRONG
	        textlook_eigenclose(op) ;
#endif
	        textlook_infoloadend(op) ;
	    }
	} /* end if (textlook_infoloadbegin) */

	if (rs >= 0) {
	    mkfieldterms(op->wterms) ;
	    op->magic = TEXTLOOK_MAGIC ;
	}

#if	CF_DEBUGS
	debugprintf("textlook_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (textlook_open) */


int textlook_close(TEXTLOOK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TEXTLOOK_MAGIC) return SR_NOTOPEN ;

#if	CF_LOOKPARALLEL
	rs1 = textlook_dispfinish(op) ;
	if (rs >= 0) rs = rs1 ;
#endif /* CF_LOOKPARALLEL */

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"textlook_close: _dispfinish() rs=%d\n",rs) ;
#endif

	rs1 = textlook_snend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = textlook_indclose(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_EXTRASTRONG
	rs1 = textlook_eigenclose(op) ;
	if (rs >= 0) rs = rs1 ;
#endif

	rs1 = textlook_infoloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"textlook_close: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (textlook_close) */


int textlook_audit(TEXTLOOK *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TEXTLOOK_MAGIC) return SR_NOTOPEN ;

	rs = txtindex_audit(&op->ind) ;

#if	CF_DEBUGS
	debugprintf("textlook_audit: txtindex_audit() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (textlook_audit) */


int textlook_info(TEXTLOOK *op,TEXTLOOK_INFO *tlip)
{
	TXTINDEX_INFO	ti ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (tlip == NULL) return SR_FAULT ;

	if (op->magic != TEXTLOOK_MAGIC) return SR_NOTOPEN ;

	memset(tlip,0,sizeof(TEXTLOOK_INFO)) ;	/* do we want this? */

	if ((rs = txtindex_info(&op->ind,&ti)) >= 0) {
	    tlip->ctime = ti.ctime ;		/* index creation-time */
	    tlip->mtime = ti.mtime ;		/* index modification-time */
	    tlip->count = ti.count ;		/* number of tags */
	    tlip->neigen = ti.neigen ;
	    tlip->minwlen = ti.minwlen ;	/* minimum word length */
	    tlip->maxwlen = ti.maxwlen ;	/* maximum word length */
	    strwcpy(tlip->sdn,ti.sdn,MAXPATHLEN) ;
	    strwcpy(tlip->sfn,ti.sfn,MAXPATHLEN) ;
	} /* end if (textindex_info) */

#if	CF_DEBUGS
	debugprintf("textlook_info: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (textlook_info) */


int textlook_curbegin(TEXTLOOK *op,TEXTLOOK_CUR *curp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != TEXTLOOK_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(TEXTLOOK_CUR)) ;
	if ((rs = rtags_start(&curp->tags,0)) >= 0) {
	    if ((rs = rtags_curbegin(&curp->tags,&curp->tcur)) >= 0) {
	        op->ncursors += 1 ;
	        curp->magic = TEXTLOOK_MAGIC ;
	    }
	    if (rs < 0)
	        rtags_finish(&curp->tags) ;
	} /* end if (rtags_start) */

	return rs ;
}
/* end subroutine (textlook_curbegin) */


int textlook_curend(TEXTLOOK *op,TEXTLOOK_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != TEXTLOOK_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != TEXTLOOK_MAGIC) return SR_NOTOPEN ;

	rs1 = rtags_curend(&curp->tags,&curp->tcur) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = rtags_finish(&curp->tags) ;
	if (rs >= 0) rs = rs1 ;

	curp->ntags = 0 ;
	if (op->ncursors > 0)
	    op->ncursors -= 1 ;

	return rs ;
}
/* end subroutine (textlook_curend) */


int textlook_lookup(TEXTLOOK *op,TEXTLOOK_CUR *curp,int qo,cchar **qsp)
{
	SEARCHKEYS	sk ;
	vecstr		hkeys ;			/* hash-keys */
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	const char	**hkeya ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (qsp == NULL) return SR_FAULT ;

	if (op->magic != TEXTLOOK_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != TEXTLOOK_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("textlook_lookup: qs¬\n") ;
	    for (i = 0 ; qsp[i] != NULL ; i += 1) {
	        debugprintf("textlook_lookup: q=>%s<\n",qsp[i]) ;
	    }
	}
#endif /* CF_DEBUGS */

/* as a curtesy, dump any prior results (sort of a compatibility thing) */

	if (curp->ntags > 0) {
	    curp->ntags = 0 ;
	    rs = rtags_curdump(&curp->tags,&curp->tcur) ;
	}

/* go with this lookup */

	if (rs >= 0) {
	    if ((rs = searchkeys_start(&sk,qsp)) >= 0) {
	        const int	vo = (VECSTR_OCOMPACT) ;
	        if ((rs = vecstr_start(&hkeys,10,vo)) >= 0) {
	            if ((rs = textlook_mkhkeys(op,&hkeys,&sk)) >= 0) {
	                if ((rs = vecstr_getvec(&hkeys,&hkeya)) >= 0) {

#if	CF_DEBUGS
		{
	    	    int	i ;
	    	    debugprintf("textlook_lookup: n=%u hkeys¬\n",rs) ;
	    	    for (i = 0 ; hkeya[i] != NULL ; i += 1) {
	        	    debugprintf("textlook_lookup: hkey=>%s<\n",
				hkeya[i]) ;
	    	    }
		}
#endif /* CF_DEBUGS */

#if	CF_LOOKPARALLEL
	rs = textlook_lookparallel(op,curp,qo,&sk,hkeya) ;
	c = rs ;
#else /* CF_LOOKPARALLEL */
	rs = textlook_lookserial(op,curp,qo,&sk,hkeya) ;
	c = rs ;
#endif /* CF_LOOKPARALLEL */

/* sort the secondary tags */

	if (rs >= 0) {
	    curp->ntags = c ;
	    if (c > 1)
	        rtags_sort(&curp->tags,NULL) ;
	}

	                } /* end if (vecstr_getvec) */
	            } /* end if (textlook_mkhkeys) */
	            rs1 = vecstr_finish(&hkeys) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (hkeys) */
	        rs1 = searchkeys_finish(&sk) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (searchkeys) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("textlook_lookup: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (textlook_lookup) */


int textlook_read(TEXTLOOK *op,TEXTLOOK_CUR *curp,TEXTLOOK_TAG *tagp)
{
	RTAGS_TAG	rt ;
	int		rs = SR_OK ;
	int		fl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (tagp == NULL) return SR_FAULT ;

	if (op->magic != TEXTLOOK_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != TEXTLOOK_MAGIC) return SR_NOTOPEN ;

	if (curp->ntags > 0) {
	    if ((rs = rtags_enum(&curp->tags,&curp->tcur,&rt)) >= 0) {
	        tagp->recoff = rt.recoff ;
	        tagp->reclen = rt.reclen ;
	        tagp->fname[0] = '\0' ;
	        if (rt.fname[0] != '\0') {
	            const int	plen = MAXPATHLEN ;
	            fl = strwcpy(tagp->fname,rt.fname,plen) - tagp->fname ;
	        }
	    }
	} else
	    rs = SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("textlook_read: ret rs=%d fl=%u\n",rs,fl) ;
#endif

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (textlook_read) */


int textlook_count(TEXTLOOK *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TEXTLOOK_MAGIC) return SR_NOTOPEN ;

	rs = txtindex_count(&op->ind) ;

	return rs ;
}
/* end subroutine (textlook_count) */


/* private subroutines */


static int textlook_infoloadbegin(TEXTLOOK *op,cchar dbname[],cchar *basedname)
{
	int		rs ;
	const char	*cp ;

	if ((rs = uc_mallocstrw(dbname,-1,&cp)) >= 0) {
	    op->dbname = cp ;
	    if (basedname != NULL) {
	        rs = uc_mallocstrw(basedname,-1,&cp) ;
	        op->basedname = cp ;
	    }
	    if (rs < 0)
	        uc_free(op->dbname) ;
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (textlook_infoloadbegin) */


static int textlook_infoloadend(TEXTLOOK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->basedname != NULL) {
	    rs1 = uc_free(op->basedname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->basedname = NULL ;
	}

	if (op->dbname != NULL) {
	    rs1 = uc_free(op->dbname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbname = NULL ;
	}

	return rs ;
}
/* end subroutine (textlook_infoloadend) */


static int textlook_indopen(TEXTLOOK *op,SUBINFO *sip)
{
	int		rs ;

	if (sip == NULL) return SR_FAULT ;

	if ((rs = txtindex_open(&op->ind,op->pr,op->dbname)) >= 0) {
	    op->f.ind = TRUE ;
	    rs = textlook_snbegin(op) ;
	    if (rs < 0)
	        txtindex_close(&op->ind) ;
	} /* end if (txtindex_open) */

#if	CF_DEBUGS
	debugprintf("textlook_indopen: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (textlook_indopen) */


static int textlook_snbegin(TEXTLOOK *op)
{
	TXTINDEX_INFO	tinfo ;
	int		rs = SR_OK ;
	int		rs1 ;
	const char	*cp ;

	rs1 = textlook_snend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = txtindex_info(&op->ind,&tinfo) ;
	if (rs >= 0) rs = rs1 ;

	if ((rs >= 0) && (tinfo.sdn[0] != '\0')) {
	    rs = uc_mallocstrw(tinfo.sdn,-1,&cp) ;
	    op->sdn = cp ;
	} /* end if (SDN) */

	if ((rs >= 0) && (tinfo.sfn[0] != '\0')) {
	    if ((rs = uc_mallocstrw(tinfo.sfn,-1,&cp)) >= 0) {
	        op->sfn = cp ;
	    }
	    if (rs < 0) {
	        if (op->sdn != NULL) {
	            uc_free(op->sdn) ;
	            op->sdn = NULL ;
	        }
	    }
	} /* end if (SFN) */

	return rs ;
}
/* end subroutines (textlook_snbegin) */


static int textlook_snend(TEXTLOOK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->sfn != NULL) {
	    rs1 = uc_free(op->sfn) ;
	    if (rs >= 0) rs = rs1 ;
	    op->sfn = NULL ;
	}

	if (op->sdn != NULL) {
	    rs1 = uc_free(op->sdn) ;
	    if (rs >= 0) rs = rs1 ;
	    op->sdn = NULL ;
	}

	return rs ;
}
/* end subroutines (textlook_snend) */


static int textlook_indclose(TEXTLOOK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = textlook_snend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->f.ind) {
	    op->f.ind = FALSE ;
	    rs1 = txtindex_close(&op->ind) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (textlook_inclose) */


/* does this primary tag have the query keys? */
static int textlook_havekeys(TEXTLOOK *op,TXTINDEX_TAG *tagp,int qo,
		SEARCHKEYS *skp)
{
	struct ustat	sb ;
	SEARCHKEYS_POP	pkeys ;
	offset_t	moff ;
	offset_t	recoff ;
	offset_t	recext ;
	pthread_t	tid ;
	size_t		ms ;
	size_t		fsize ;
	uint		reclen ;
	const int	maxreclen = TEXTLOOK_MAXRECLEN ;
	int		rs ;
	int		rs1 ;
	int		c ;
	int		len ;
	int		ll ;
	int		fd ;
	int		mprot ;
	int		mflags ;
	int		ml ;
	int		f_prefix ;
	int		f = FALSE ;
	const char	*lp ;
	const char	*dn, *fn ;
	const char	*mp, *tp ;
	char		fname[MAXPATHLEN + 1] ;
	char		*md = NULL ;

	uptself(&tid) ;

#if	CF_DEBUGS
	debugprintf("textlook_havekeys: tid=%u tag recoff=%u reclen=%u \n",
	    tid,tagp->recoff,tagp->reclen) ;
#endif

	if (tagp->reclen == 0) goto ret0 ;

	f_prefix = (qo & TEXTLOOK_OPREFIX) ;
	rs = searchkeys_popbegin(skp,&pkeys,f_prefix) ;
	c = rs ;
	if (rs < 0)
	    goto ret0 ;

	if (c == 0) {
	    f = TRUE ;
	    goto ret1 ;
	}

/* create the filename */

	fn = tagp->fname ;
	if ((fn[0] == '\0') && (op->sfn != NULL)) fn = op->sfn ;
	if (fn[0] == '\0')
	    goto ret1 ;

	if (fn[0] != '/') {
	    dn = op->sdn ;
	    if ((dn == NULL) || (dn[0] == '\0')) dn = op->basedname ;

	    if ((dn != NULL) && (dn[0] != '\0')) {
	        rs = mkpath2(fname,dn,fn) ;
	        fn = fname ;
	    }
	}

	if (rs < 0)
	    goto ret1 ;

#if	CF_DEBUGS
	memset(&sb,0,sizeof(struct ustat)) ;
	debugprintf("textlook_havekeys: tid=%u fname=%s\n",tid,fn) ;
#endif

	rs1 = u_open(fn,O_RDONLY,0666) ;
	fd = rs1 ;
	if ((rs1 == SR_NOENT) || (rs1 == SR_ACCESS))
	    goto ret1 ;

	rs = rs1 ;
	if (rs < 0)
	    goto ret1 ;

	rs = u_fstat(fd,&sb) ;
	if (rs < 0)
	    goto ret2 ;

#if	CF_DEBUGS
	debugprintf("textlook_havekeys: tid=%u st_size=%llu\n",
	    tid,sb.st_size) ;
#endif

	fsize = (sb.st_size & INT_MAX) ;
	if ((fsize == 0) || (! S_ISREG(sb.st_mode)))
	    goto ret2 ;

#if	CF_DEBUGS
	debugprintf("textlook_havekeys: tid=%u tag recoff=%u reclen=%u \n",
	    tid,tagp->recoff,tagp->reclen) ;
	debugprintf("textlook_havekeys: tid=%u fsize=%lu\n",tid,fsize) ;
#endif

	if (tagp->recoff >= fsize)
	    goto ret2 ;

	recoff = tagp->recoff ;
	reclen = tagp->reclen ;

#if	CF_DEBUGS
	debugprintf("textlook_havekeys: tid=%u recoff=%llu reclen=%u \n",
	    tid,recoff,reclen) ;
#endif

	if (reclen > maxreclen) reclen = maxreclen ;

	mprot = PROT_READ ;
	mflags = MAP_SHARED ;
	moff = ufloor(recoff,op->pagesize) ;

	recext = (recoff + reclen) ;
	ms = uceil(recext,op->pagesize) - moff ;

#if	CF_DEBUGS
	debugprintf("textlook_havekeys: tid=%u recext=%llu \n",tid,recext) ;
	debugprintf("textlook_havekeys: tid=%u moff=%llu ms=%lu\n",
	    tid,moff,ms) ;
#endif

	if ((rs = u_mmap(NULL,ms,mprot,mflags,fd,moff,&md)) >= 0) {

/* process this tag */

	    mp = (md + (tagp->recoff - moff)) ;
	    ml = reclen ;
	    while ((tp = strnchr(mp,ml,'\n')) != NULL) {

	        len = ((tp + 1) - mp) ;
	        lp = mp ;
	        ll = (len - 1) ;

	        if (ll > 0) {
	            rs = textlook_havekeysline(op,skp,&pkeys,lp,ll) ;
	            f = (rs > 0) ;
	            if (f) break ;
	        } /* end if */

	        ml -= len ;
	        mp += len ;
	        if (rs < 0) break ;
	    } /* end while (readling lines) */

	    u_munmap(md,ms) ;
	} /* end if (memory-map) */

ret2:
	if (fd >= 0) u_close(fd) ;

ret1:
	searchkeys_popend(skp,&pkeys) ;

ret0:

#if	CF_DEBUGS
	debugprintf("textlook_havekeys: tid=%u ret rs=%d f=%u\n",tid,rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (textlook_havekeys) */


static int textlook_havekeysline(op,skp,pkp,lp,ll)
TEXTLOOK	*op ;
SEARCHKEYS	*skp ;
SEARCHKEYS_POP	*pkp ;
const char	*lp ;
int		ll ;
{
	FIELD		fsb ;
	int		rs ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("textlook_havekeysline: line¬\n") ;
	debugprintf(">%t<\n",lp,ll) ;
#endif

	if ((rs = field_start(&fsb,lp,ll)) >= 0) {
	    int		fl, sl, kl ;
	    const char	*fp, *sp, *kp ;
	    char	keybuf[KEYBUFLEN + 1] ;

	    while ((fl = field_word(&fsb,op->wterms,&fp)) >= 0) {

	        if (fl && (fp[0] == CH_SQUOTE)) {
	            fp += 1 ;
	            fl -= 1 ;
	        }

	        if (fl == 0) continue ;

#if	CF_DEBUGS
	        debugprintf("textlook_havekeysline: fl=%u fp=>%t<\n",
	            fl,fp,fl) ;
#endif

	        sl = sfword(fp,fl,&sp) ;

#if	CF_DEBUGS
	        debugprintf("textlook_havekeysline: sl=%u sp=>%t<\n",
	            sl,sp,sl) ;
#endif

	        if ((sl <= 0) || (sp == NULL)) continue ;

	        kp = sp ;
	        kl = sl ;
	        if (kl > KEYBUFLEN)			/* prevents overflow */
	            kl = KEYBUFLEN ;

	        if (hasuc(kp,kl)) {
	            strwcpylc(keybuf,kp,kl) ;	/* can't overflow */
	            kp = keybuf ;
	        }

#if	CF_DEBUGS
	        debugprintf("textlook_havekeysline: match? w=>%t<\n",
	            kp,kl) ;
#endif

	        rs = textlook_matchkeys(op,skp,pkp,kp,kl) ;
	        f = (rs > 0) ;

	        if (f) break ;
	        if (rs < 0) break ;
	    } /* end while (fielding words) */

	    field_finish(&fsb) ;
	} /* end if (field) */

#if	CF_DEBUGS
	debugprintf("textlook_havekeysline: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (textlook_havekeysline) */


/* do the keys match? */
static int textlook_matchkeys(op,skp,pkp,sp,sl)
TEXTLOOK	*op ;
SEARCHKEYS	*skp ;
SEARCHKEYS_POP	*pkp ;
const char	*sp ;
int		sl ;
{
	XWORDS		xw ;

#if	CF_DEBUGS
	uint		id ;
#endif

	int		rs ;
	int		f = FALSE ;

#if	CF_DEBUGS
	{
	    pthread_t	tid ;
	    uptself(&tid) ;
	    id = (uint) tid ;
	}
#endif

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS 
	debugprintf("textlook_matchkeys: id=%u s=>%t<\n",id,sp,sl) ;
#endif

/* deal with extra (ex: possessive) words */

#if	CF_SINGLEWORD
	if ((rs = xwords_start(&xw,sp,sl)) >= 0) {

	    rs = searchkeys_processxw(skp,pkp,&xw) ;
	    f = (rs > 0) ;

	    xwords_finish(&xw) ;
	} /* end if */
#else /* CF_SINGLEWORD */
	if ((rs = xwords_start(&xw,sp,sl)) >= 0) {
	    int		wi ;
	    int		cl ;
	    const char	*cp ;

	    f = FALSE ;
	    wi = 0 ;
	    while ((cl = xwords_get(&xw,wi,&cp)) > 0) {

#if	CF_DEBUGS 
	        debugprintf("textlook_matchkeys: xwords_get() rs=%d\n",cl) ;
	        if (cl >= 0)
	            debugprintf("textlook_matchkeys: c=>%t<\n",cp,cl) ;
#endif

	        rs = searchkeys_process(skp,pkp,cp,cl) ;
	        f = (rs > 0) ;
	        if (rs < 0) break ;

#if	CF_DEBUGS 
	        debugprintf("textlook_matchkeys: searchkeys_process() f=%u\n",
	            f) ;
#endif

	        if (f)
	            break ;

	        wi += 1 ;
	    } /* end while (matching words) */

	    xwords_finish(&xw) ;
	} /* end if */
#endif /* CF_SINGLEWORD */

#if	CF_DEBUGS && 0
	debugprintf("textlook_matchkeys: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (textlook_matchkeys) */


#if	CF_EXTRASTRONG

static int textlook_eigenopen(TEXTLOOK *op)
{
	int		rs1 ;
	int		f = FALSE ;

	if (! op->f.edbinit) {
	op->f.edbinit = TRUE ;
	rs1 = eigenfind(&op->edb,op->pr,op->dbname,op->minwlen) ;
	op->f.edb = (rs1 > 0) ;
	f = op->f.edb ;
	}

#if	CF_DEBUGS
	debugprintf("textlook_eigenopen: eigenfind() rs=%d\n",rs1) ;
#endif

	return f ;
}
/* end subroutine (textlook_eigenopen) */


static int textlook_eigenclose(TEXTLOOK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f.edb) {
	    op->f.edb = FALSE ;
	    rs1 = eigendb_close(&op->edb) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (textlook_eigenopen) */

#endif /* CF_EXTRASTRONG */


static int textlook_mkhkeys(TEXTLOOK *op,vecstr *hkp,SEARCHKEYS *skp)
{
	SEARCHKEYS_CUR	cur ;
	int		rs ;
	int		rs1 ;
	int		nkeys = 0 ;

#if	CF_DEBUGS
	debugprintf("textlook_mkhkeys: ent\n") ;
#endif

	if ((rs = searchkeys_curbegin(skp,&cur)) >= 0) {
	    int		kl ;
	    const char	*kp ;

	    while (rs >= 0) {

	        kl = searchkeys_enum(skp,&cur,&kp) ;
	        if (kl < 0) break ;

	        if (kp == NULL) continue ;

	        if (kl < op->minwlen) continue ;

	        rs1 = SR_NOTFOUND ;

#if	CF_EXTRAEIGEN && CF_EXTRASTRONG
	        if (op->f.edb)
	            rs1 = eigendb_exists(&op->edb,kp,kl) ;
#endif

	        if (rs1 == SR_NOTFOUND) {
	            rs = vecstr_adduniq(hkp,kp,kl) ;
	            if (rs < INT_MAX) nkeys += 1 ;
	        }

	        if (rs < 0) break ;
	    } /* end while (enumerating search-keys) */

	    searchkeys_curend(skp,&cur) ;
	} /* end if (cursor) */

#if	CF_DEBUGS
	debugprintf("textlook_mkhkeys: ret rs=%d nkeys=%u\n",rs,nkeys) ;
#endif

	return (rs >= 0) ? nkeys : rs ;
}
/* end subroutine (textlook_mkhkeys) */


#if	CF_LOOKPARALLEL

static int textlook_lookparallel(op,curp,qo,skp,hkeya)
TEXTLOOK	*op ;
TEXTLOOK_CUR	*curp ;
int		qo ;
SEARCHKEYS	*skp ;
const char	**hkeya ;
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("textlook_lookup: ent\n") ;
	{
	    int	i ;
	    debugprintf("textlook_lookup: keys¬\n") ;
	    for (i = 0 ; hkeya[i] != NULL ; i += 1) {
	        debugprintf("textlook_lookup: k=>%s<\n",hkeya[i]) ;
	    }
	}
#endif

	if ((rs = textlook_checkdisp(op)) >= 0) {
	    DISP	*dop = op->disp ;
	    RTAGS	*rtp = &curp->tags ;

	    if ((rs = disp_setparams(dop,qo,skp,rtp)) >= 0) {
	        TXTINDEX_CUR	tcur ;
	        TXTINDEX_TAG	ttag ;

	        if ((rs = txtindex_curbegin(&op->ind,&tcur)) >= 0) {
	            int	ntags ;

	            rs = txtindex_lookup(&op->ind,&tcur,hkeya) ;
	            ntags = rs ;

#if	CF_DEBUGS
	            debugprintf("textlook_lookup: ntags=%d\n",ntags) ;
#endif

	            while ((rs >= 0) && (ntags-- > 0)) {

	                rs1 = txtindex_read(&op->ind,&tcur,&ttag) ;
	                if (rs1 == SR_NOTFOUND) break ;
	                rs = rs1 ;

	                if ((rs >= 0) && (ttag.reclen > 0)) {
	                    rs = disp_addwork(dop,&ttag) ;
			}

	            } /* end while */

	            rs1 = txtindex_curend(&op->ind,&tcur) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (txtindex-cur) */

	        rs1 = disp_waitdone(dop) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (set-params) */

	    if (rs >= 0) {
	        rs = rtags_count(rtp) ;
	        c = rs ;
	    }

	} /* end if (disp-checkstart) */

#if	CF_DEBUGS
	debugprintf("textlook_lookparallel: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (textlook_lookparallel) */

static int textlook_checkdisp(TEXTLOOK *op)
{
	int		rs = SR_OK ;

	if (op->disp == NULL) {
	    rs = textlook_dispstart(op) ;
	}

	return rs ;
}
/* end subroutine (textlook_checkdisp) */

#else /* CF_LOOKPARALLEL */

static int textlook_lookserial(op,curp,qo,skp,hkeya)
TEXTLOOK	*op ;
TEXTLOOK_CUR	*curp ;
int		qo ;
SEARCHKEYS	*skp ;
const char	**hkeya ;
{
	TXTINDEX_CUR	tcur ;
	TXTINDEX_TAG	ttag ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = txtindex_curbegin(&op->ind,&tcur)) >= 0) {
	    RTAGS_TAG	rt ;
	    int		ntags ;

	    rs = txtindex_lookup(&op->ind,&tcur,hkeya) ;
	    ntags = rs ;

#if	CF_DEBUGS
	    debugprintf("textlook_lookup: ntags=%d\n",ntags) ;
#endif

	    while ((rs >= 0) && (ntags-- > 0)) {
	        rs1 = txtindex_read(&op->ind,&tcur,&ttag) ;
	        if (rs1 == SR_NOTFOUND) break ;
	        rs = rs1 ;

	        if ((rs >= 0) && (ttag.reclen > 0)) {
	            rs = textlook_havekeys(op,&ttag,qo,skp) ;
	            if (rs > 0) {
	                c += 1 ;
	                rt.hash = 0 ;
	                rt.recoff = ttag.recoff ;
	                rt.reclen = ttag.reclen ;
	                rt.fname[0] = '\0' ;
	                if (ttag.fname[0] != '\0') {
	                    strwcpy(rt.fname,ttag.fname,MAXPATHLEN) ;
			}
	                rs = rtags_add(&curp->tags,&rt) ;
	            }
	        }

	    } /* end while */

	    rs1 = txtindex_curend(&op->ind,&tcur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cursor) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (textlook_lookserial) */

#endif /* CF_LOOKPARALLEL */


#if	CF_LOOKPARALLEL
static int textlook_dispstart(TEXTLOOK *op)
{
	int		rs = SR_OK ;

	if (op->disp == NULL) {
	    const int	size = sizeof(DISP) ;
	    void	*p ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        DISP	*dop = p ;
	        if ((rs = disp_start(dop,op)) >= 0) {
	            op->disp = dop ;
	        }
	        if (rs < 0) uc_free(p) ;
	    } /* end if (memory-allocation) */
	} /* end if (needed start-up) */

#if	CF_DEBUGS
	debugprintf("textlook_dispstart: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (textlook_dispstart) */
#endif /* CF_LOOKPARALLEL */


#if	CF_LOOKPARALLEL
static int textlook_dispfinish(TEXTLOOK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_abort = TRUE ;

	if (op->disp != NULL) {
	    DISP	*dop = op->disp ;
	    rs1 = disp_finish(dop,f_abort) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(dop) ;
	    if (rs >= 0) rs = rs1 ;
	    op->disp = NULL ;
	}

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"textlook_dispfinish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (textlook_dispfinish) */
#endif /* CF_LOOKPARALLEL */


static int subinfo_start(SUBINFO *sip)
{
	int		rs = SR_OK ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->daytime = time(NULL) ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->f.id) {
	    sip->f.id = FALSE ;
	    rs1 = ids_release(&sip->id) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (subinfo_finish) */


#if	CF_EXTRASTRONG
static int eigenfind(EIGENDB *edbp,cchar *pr,cchar *dbname,int minwlen)
{
	struct ustat	sb ;
	IDS		id ;
	EXPCOOK		cooks ;
	int		rs ;
	int		i ;
	int		efl ;
	const char	*efp = NULL ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		efname[MAXPATHLEN + 1] ;

	rs = ids_load(&id) ;
	if (rs < 0)
	    goto ret0 ;

	rs = expcook_start(&cooks) ;
	if (rs < 0)
	    goto ret1 ;

	rs = expcook_add(&cooks,"n",dbname,-1) ;
	if (rs >= 0)
	    rs = expcook_add(&cooks,"f","eign",-1) ;
	if (rs < 0)
	    goto ret2 ;

	rs = SR_NOTOPEN ;
	efname[0] = '\0' ;
	for (i = 0 ; eigenfnames[i] != NULL ; i += 1) {

	    rs = SR_OK ;
	    efp = eigenfnames[i] ;
	    efl = -1 ;
	    if (efp[0] != '/') {
	        rs = mkpath2(tmpfname,pr,efp) ;
	        efl = rs ;
	        if (rs <= 0) {
	            rs = SR_NOENT ;
	            efp = NULL ;
	        } else {
	            efp = tmpfname ;
		}
	    }

	    if (rs >= 0)
	        rs = expcook_exp(&cooks,0,efname,MAXPATHLEN,efp,efl) ;

	    if (rs >= 0)
	        rs = u_stat(efname,&sb) ;

	    if ((rs >= 0) && S_ISDIR(sb.st_mode))
	        rs = SR_ISDIR ;

	    if (rs >= 0)
	        rs = sperm(&id,&sb,R_OK) ;

#if	CF_DEBUGS
	    debugprintf("textlook/eigenfind: fname=%s rs=%d\n",
	        efname,rs) ;
#endif

	    if (rs >= 0)
	        break ;

	} /* end for */

	if (rs >= 0)
	    rs = eigendb_open(edbp,INT_MAX,minwlen,efname) ;

#if	CF_DEBUGS
	debugprintf("textlook/eigenfind: eigendb_open() rs=%d\n",rs) ;
#endif

ret2:
	expcook_finish(&cooks) ;

ret1:
	ids_release(&id) ;

ret0:
	return rs ;
}
/* end subroutine (eigenfind) */
#endif /* CF_EXTRASTRONG */


#if	CF_LOOKPARALLEL

static int disp_start(DISP *dop,TEXTLOOK *op)
{
	WARGS		*wap ;
	const int	qlen = TEXTLOOK_QLEN ;
	int		rs = SR_OK ;
	int		size ;
	int		opts ;
	int		n, i ;
	int		f_shared = FALSE ;
	void		*p ;

	if (dop == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("textlook/disp_start: dop{%p}\n",dop) ;
#endif

	memset(dop,0,sizeof(DISP)) ;

	{
	    wap = &dop->wa ;
	    wap->dop = dop ;
	    wap->op = op ;
	    wap->terms = op->wterms ;
	}

#if	CF_MULTIPROCESS
	n = uc_nprocessors(0) ;
	if (n < 1) n = 1 ;
	n += 1 ;
	if (n > NCPUMAX) n = NCPUMAX ;
#else /* CF_MULTIPROCESS */
	n = 1 ;
#endif /* CF_MULTIPROCESS */
	dop->npar = n ;
	dop->qlen = (n + qlen) ;

#if	CF_DEBUGS
	debugprintf("textlook/disp_start: npar=%u\n",dop->npar) ;
#endif

	rs = ptm_create(&dop->mutex_threads,NULL) ;
	if (rs < 0) goto bad0 ;

	rs = psem_create(&dop->sem_wq,f_shared,0) ;
	if (rs < 0) goto bad2 ;

	rs = psem_create(&dop->sem_done,f_shared,0) ;
	if (rs < 0) goto bad3 ;

	opts = (VECHAND_OREUSE | VECHAND_OSTATIONARY) ;
	rs = vechand_start(&dop->threads,dop->npar,opts) ;
	if (rs < 0) goto bad4 ;

	rs = tagq_start(&dop->wq,dop->qlen) ;
	if (rs < 0) goto bad5 ;

/* set the concurrency for systems that do something w/ it */

	n = uptgetconcurrency() ;
	if (n < dop->npar) {
	    rs = uptsetconcurrency(dop->npar) ;
	}

/* create the threads */

	for (i = 0 ; (rs >= 0) && (i < dop->npar) ; i += 1) {
	    pthread_t	tid ;
	    DISP_THREAD	*tip ;
	    size = sizeof(DISP_THREAD) ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        int	ti = -1 ;
	        memset(p,0,size) ;
	        tip = p ;
	        if ((rs = vechand_add(&dop->threads,tip)) >= 0) {
	            ti = rs ;
	            tip->wap = wap ;
	            if ((rs = uptcreate(&tid,NULL,worker,tip)) >= 0) {
	                tip->tid = tid ;
		    }
	            if (rs < 0)
	                vechand_del(&dop->threads,ti) ;
	        } /* end if (thread added to list) */
	        if (rs < 0)
	            uc_free(p) ;
	    } /* end if (memory-alloccation) */
	} /* end for */

/* sort the created thread-ids (why are we doing this?) */

	if (rs >= 0) {
	    rs = disp_threadsort(dop) ;
	}

/* handle early errors */

	if (rs < 0) {
	    DISP_THREAD	*tip ;
	    dop->f_exit = TRUE ;
	    for (i = 0 ; i < dop->npar ; i += 1) {
	        psem_post(&dop->sem_wq) ;
	    }
	    for (i = 0 ; vechand_get(&dop->threads,i,&tip) >= 0 ; i += 1) {
	        if (tip != NULL) {
	            uptjoin(tip->tid,NULL) ;
	            uc_free(tip) ;
		}
	    } /* end for */
	} /* end if (failure) */

	if (rs < 0) goto bad6 ;

ret0:
	return rs ;

/* bad stuff */
bad6:
	tagq_finish(&dop->wq) ;

bad5:
	vechand_finish(&dop->threads) ;

bad4:
	psem_destroy(&dop->sem_done) ;

bad3:
	psem_destroy(&dop->sem_wq) ;

bad2:
	ptm_destroy(&dop->mutex_threads) ;

bad0:
	goto ret0 ;
}
/* end subroutine (disp_start) */


static int disp_finish(DISP *dop,int f_abort)
{
	DISP_THREAD	*tip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (dop == NULL)
	    return SR_FAULT ;

	dop->f_done = TRUE ;		/* assumed to be aromic! */
	if (f_abort)
	    dop->f_exit = TRUE ;	/* assumed to be atomic! */

	for (i = 0 ; i < dop->npar ; i += 1) {
	    rs1 = psem_post(&dop->sem_wq) ;
	    if (rs >= 0) rs = rs1 ;
	}

	for (i = 0 ; vechand_get(&dop->threads,i,&tip) >= 0 ; i += 1) {
	    if (tip != NULL) {
	        rs1 = uptjoin(tip->tid,NULL) ;
	        if (rs >= 0) rs = rs1 ;
	        if (rs >= 0) rs = tip->rs ;
	        rs1 = uc_free(tip) ;
		if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	rs1 = tagq_finish(&dop->wq) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&dop->threads) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = psem_destroy(&dop->sem_done) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = psem_destroy(&dop->sem_wq) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ptm_destroy(&dop->mutex_threads) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("textlook/disp_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (disp_finish) */


static int disp_threadsort(DISP *dop)
{
	int	rs ;

	rs = vechand_sort(&dop->threads,vcmpthreads) ;

	return rs ;
}
/* end subroutine (disp_threadsort) */


static int disp_setparams(dop,qo,skp,rtp)
DISP		*dop ;
int		qo ;
SEARCHKEYS	*skp ;
RTAGS		*rtp ;
{
	WARGS		*wap = &dop->wa ;
	int		rs = SR_OK ;

	wap->qo = qo ;
	wap->skp = skp ;
	wap->rtp = rtp ;
	return rs ;
}
/* end subroutine (disp_setparams) */


static int disp_addwork(DISP *dop,TXTINDEX_TAG *tagp)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("disp_addwork: ent\n") ;
#endif

	if ((rs = tagq_ins(&dop->wq,tagp)) >= 0) {
	    rs = psem_post(&dop->sem_wq) ; /* post always (more parallelism) */
	}

#if	CF_DEBUGS
	debugprintf("disp_addwork: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (disp_addwork) */


static int disp_setstate(DISP *dop,DISP_THREAD *tip,int f)
{
	int		rs = SR_OK ;
	int		f_prev = FALSE ;

#ifdef	COMMENT
	if ((rs = ptm_lock(&dop->mutex_threads)) >= 0) {
	    f_prev = tip->f_busy ;
	    tip->f_busy = f ;
	    ptm_unlock(&dop->mutex_threads) ;
	} /* end if */
#else /* COMMENT */
	f_prev = tip->f_busy ;
	tip->f_busy = f ;
#endif /* COMMENT */

	if ((rs >= 0) && (! f) && f_prev) {
	    rs = psem_post(&dop->sem_done) ;
	}

	return (rs >= 0) ? f_prev : rs ;
}
/* end subroutine (disp_setstate) */


static int disp_nbusy(DISP *dop)
{
	DISP_THREAD	*tip ;
	int		rs = SR_OK ;
	int		i ;
	int		n = 0 ;

#ifdef	COMMENT
	if ((rs = ptm_lock(&dop->mutex_threads)) >= 0) {
	    for (i = 0 ; vechand_get(&dop->threads,i,&tip) >= 0 ; i += 1) {
	        if (tip != NULL) {
	            if (tip->f_busy) n += 1 ;
		}
	    }
	    ptm_unlock(&dop->mutex_threads) ;
	} /* end if */
#else /* COMMENT */
	for (i = 0 ; vechand_get(&dop->threads,i,&tip) >= 0 ; i += 1) {
	    if (tip != NULL) {
	        if (tip->f_busy) n += 1 ;
	    }
	}
#endif /* COMMENT */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (disp_nbusy) */


static int disp_nexited(DISP *dop)
{
	DISP_THREAD	*tip ;
	int		rs = SR_OK ;
	int		i ;
	int		n = 0 ;

#ifdef	COMMENT
	if ((rs = ptm_lock(&dop->mutex_threads)) >= 0) {
	    for (i = 0 ; vechand_get(&dop->threads,i,&tip) >= 0 ; i += 1) {
	        if (tip != NULL) {
	            if (tip->f_exited) n += 1 ;
		}
	    }
	    ptm_unlock(&dop->mutex_threads) ;
	} /* end if */
#else /* COMMENT */
	for (i = 0 ; vechand_get(&dop->threads,i,&tip) >= 0 ; i += 1) {
	    if (tip != NULL) {
	        if (tip->f_exited) n += 1 ;
	    }
	}
#endif /* COMMENT */

#if	CF_DEBUGS
	debugprintf("disp_nexited: ret rs=%d n=%d\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (disp_nexited) */


static int disp_waitdone(DISP *dop)
{
	int		rs = SR_OK ;
	int		i ;
	int		nbusy = -1 ;
	int		nexited = -1 ;
	int		nwq = -1 ;
	int		f_cond = FALSE ;
	int		f_notbusy = FALSE ;
	int		f_empty = FALSE ;
	int		f_allexited = FALSE ;

#if	CF_DEBUGS
	debugprintf("textlook/disp_waitdone: ent\n") ;
#endif

	for (i = 0 ; (rs >= 0) && (! f_cond) ; i += 1) {

	    if ((rs >= 0) && (i > 0)) {
	        rs = psem_wait(&dop->sem_done) ;
	        if (rs == SR_INTR) rs = SR_OK ;
	    }

#if	CF_DEBUGS
	    debugprintf("textlook/disp_waitdone: wakeup\n") ;
#endif

	    if ((rs >= 0) && (! f_empty)) {
	        rs = tagq_count(&dop->wq) ;
	        nwq = rs ;
	        f_empty = (nwq == 0) ;
	    }

	    if ((rs >= 0) && f_empty && (! f_notbusy)) {
	        rs = disp_nbusy(dop) ;
	        nbusy = rs ;
	        f_notbusy = (nbusy == 0) ;
	    }

	    f_cond = (f_empty && f_notbusy) ;

	    if ((rs >= 0) && (! f_cond)) {
	        rs = disp_nexited(dop) ;
	        nexited = rs ;
	        f_allexited = (nexited == dop->npar) ;
#if	CF_DEBUGS
	        debugprintf("textlook/disp_waitdone: f_allexited=%u\n",
	            f_allexited) ;
#endif
	        if (f_allexited) f_cond = TRUE ;
	    }

#if	CF_DEBUGS
	    debugprintf("textlook/disp_waitdone: nwq=%d f_empty=%u\n",
	        nwq,f_empty) ;
	    debugprintf("textlook/disp_waitdone: nbusy=%d f_notbusy=%u\n",
	        nbusy,f_notbusy) ;
	    debugprintf("textlook/disp_waitdone: nexited=%d f_allexited=%u\n",
	        nexited,f_allexited) ;
#endif

	} /* end for (waiting for done-ness) */

#if	CF_DEBUGS
	debugprintf("textlook/disp_waitdone: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (disp_waitdone) */


static int worker(void *ptvp)
{
	WARGS		*wap ;
	DISP		*dop ;
	DISP_THREAD	*tip = (DISP_THREAD *) ptvp ;
	TEXTLOOK	*op ;
	SEARCHKEYS	*skp ;
	RTAGS		*rtp ;
	RTAGS_TAG	rt ;
	TXTINDEX_TAG	ttag ;
	pthread_t	tid ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		qo ;
	int		c = 0 ;

	wap = tip->wap ;		/* get basic arguments */

/* these arguments never change (but the rest do -- dynamically) */

	dop = wap->dop ;
	op = wap->op ;

	uptself(&tid) ;

#if	CF_DEBUGS
	debugprintf("procquery/worker: tid=%u starting\n",tid) ;
	debugprintf("procquery/worker: dop{%p}\n",dop) ;
#endif

	while ((rs >= 0) && (! dop->f_exit)) {

	    while ((rs = psem_wait(&dop->sem_wq)) < 0) {
	        if (rs != SR_INTR) break ;
	    } /* end while */

	    if (dop->f_exit) break ;

	    if ((rs >= 0) && ((rs = disp_setstate(dop,tip,TRUE)) >= 0)) {

	        while ((rs >= 0) && (! dop->f_exit)) {

	            rs1 = tagq_rem(&dop->wq,&ttag) ;
	            if (rs1 == SR_NOTFOUND) break ;
	            rs = rs1 ;
	            if (rs < 0) break ;

	            c += 1 ;
	            qo = wap->qo ;	/* new for each work cycle */
	            skp = wap->skp ;	/* new for each work cycle */
	            rtp = wap->rtp ;	/* new for each work cycle */

#if	CF_DEBUGS
	            debugprintf("procquery/worker: tid=%u fname=%s\n",
	                tid,ttag.fname) ;
#endif

#if	CF_TESTERROR && CF_DEBUGS
	            if (strstr(ttag.fname,"belgium") != NULL)
	                rs = SR_BADFMT ;
#endif

	            if ((rs = textlook_havekeys(op,&ttag,qo,skp)) > 0) {
	                rt.hash = 0 ;
	                rt.recoff = ttag.recoff ;
	                rt.reclen = ttag.reclen ;
	                rt.fname[0] = '\0' ;
	                if (ttag.fname[0] != '\0') {
	                    strwcpy(rt.fname,ttag.fname,MAXPATHLEN) ;
			}
	                rs = rtags_add(rtp,&rt) ;
	            } /* end if (found a key) */

	        } /* end while (work) */

	        if (rs >= 0) {
		    rs = disp_setstate(dop,tip,FALSE) ;
		}
	    } /* end if (worker was busy) */

	    if (dop->f_done) break ;
	} /* end while (waiting) */

	if (rs >= 0) {
	    disp_setstate(dop,tip,FALSE) ;
	}

#if	CF_DEBUGS
	debugprintf("procquery/worker: tid=%u ret rs=%d c=%u\n",
	    tid,rs,c) ;
#endif

	tip->rs = rs ;
	tip->f_exited = TRUE ;
#if	CF_DEBUGN
	nprintf(NDEBFNAME,"textlook/worker: ret tid=%u rs=%d c=%u\n",tid,rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (worker) */


/* this object ('TAGQ') forms the Q of work going into the worker threads */
static int tagq_start(TAGQ *tqp,int n)
{
	const int	f_shared = FALSE ;
	int		rs ;

	if (n < 1) n = 1 ;

	if ((rs = psem_create(&tqp->wsem,f_shared,n)) >= 0) {
	    rs = ciq_start(&tqp->q) ;
	    if (rs < 0)
	        psem_destroy(&tqp->wsem) ;
	}

	return rs ;
}
/* end subroutine (tagq_start) */


static int tagq_finish(TAGQ *tqp)
{
	CIQ		*cqp = &tqp->q ;
	int		rs = SR_OK ;
	int		rs1 ;
	char		*p ;

	while (ciq_rem(cqp,&p) >= 0) {
	    rs1 = uc_free(p) ; /* these things are opaque */
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = ciq_finish(cqp) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = psem_destroy(&tqp->wsem) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (tagq_finish) */


static int tagq_count(TAGQ *tqp)
{

	return ciq_count(&tqp->q) ;
}
/* end subroutine (tagq_finish) */


static int tagq_ins(TAGQ *tqp,TXTINDEX_TAG *tagp)
{
	TAGQ_THING	*ttp ;
	int		rs ;
	int		size ;
	int		rc = 0 ;
	void		*p ;

	size = sizeof(TAGQ_THING) + strlen(tagp->fname) + 1 ;
	if ((rs = uc_malloc(size,&p)) >= 0) {

	    ttp = (TAGQ_THING *) p ;
	    ttp->recoff = tagp->recoff ;
	    ttp->reclen = tagp->reclen ;
	    strwcpy(ttp->fname,tagp->fname,MAXPATHLEN) ;

	    rs = psem_wait(&tqp->wsem) ;
	    if (rs == SR_INTR) rs = SR_OK ;

	    if (rs >= 0) {
	        rs = ciq_ins(&tqp->q,ttp) ;
	        rc = rs ;
	    }

	    if (rs < 0) uc_free(p) ;
	} /* end if */

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (tagq_ins) */


static int tagq_rem(TAGQ *tqp,TXTINDEX_TAG *tagp)
{
	TAGQ_THING	*ttp ;
	int		rs ;
	int		rc = 0 ;

#if	CF_DEBUGS && 0
	debugprintf("tagq_rem: ent\n") ;
#endif

	if ((rs = ciq_rem(&tqp->q,&ttp)) >= 0) {
	    rc = rs ;

	    tagp->recoff = ttp->recoff ;
	    tagp->reclen = ttp->reclen ;
	    strwcpy(tagp->fname,ttp->fname,MAXPATHLEN) ;

	    uc_free(ttp) ;

	    rs = psem_post(&tqp->wsem) ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("tagq_rem: ret rs=%d rc=%d\n",rs,rc) ;
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (tagq_rem) */

#endif /* CF_LOOKPARALLEL */


static int mkfieldterms(uchar *wterms)
{
	int		i ;

	for (i = 0 ; i < 32 ; i += 1) {
	    wterms[i] = 0xFF ;
	}
	for (i = 0 ; i < 256 ; i += 1) {
	    if (isalnumlatin(i)) {
	        BACLR(wterms,i) ;
	    }
	} /* end for */
	BACLR(wterms,CH_SQUOTE) ;		/* allow apostrophe */
	BACLR(wterms,'_') ;			/* allow under-score */
	BACLR(wterms,'-') ;			/* allow minus-sign */

	return i ;
}
/* end subroutine (mkfieldterms) */


#if	CF_LOOKPARALLEL
static int vcmpthreads(DISP_THREAD **e1pp,DISP_THREAD **e2pp)
{
	int		rc = 0 ;

	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            DISP_THREAD	*e1p = *e1pp ;
	            DISP_THREAD	*e2p = *e2pp ;
	            if ((rc = uptequal(e1p->tid,e2p->tid)) == 0) {
	                int t1 = (int) e1p->tid ;
	                int t2 = (int) e2p->tid ;
	                rc = (int) (t1-t2) ;
	            }
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}

	return rc ;
}
/* end subroutine (vcmpthread) */
#endif /* CF_LOOKPARALLEL */


