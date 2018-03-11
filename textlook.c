/* textlook */

/* text look-up manager (we use the index and verify speculative resutls) */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* special debugging */
#define	CF_SINGLEWORD	1		/* treat extra words as single */
#define	CF_TESTERROR	0		/* test thread error-exit */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little object provides access to the TEXTLOOK database and index
	(if any).

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
#include	<ptc.h>
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

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	NATURALWORDLEN
#endif

#define	TO_FILEMOD	(60 * 24 * 3600)
#define	TO_MKWAIT	(5 * 50)

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#define	DISP		struct disp_head
#define	DISP_ARGS	struct disp_args
#define	DISP_THR	struct disp_thr

#define	TAGQ		struct tagq 
#define	TAGQ_THING	struct tagq_thing

#define	NDF		"textlook.deb"


/* external subroutines */

extern uint	ufloor(uint,int) ;
extern uint	uceil(uint,int) ;
extern uint	hashelf(void *,int) ;

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy4(char *,int, const char *,const char *,cchar *,cchar *) ;
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
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;
extern int	isFailOpen(int) ;

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

struct disp_args {
	TEXTLOOK	*op ;
	RTAGS		*rtp ;
	SEARCHKEYS	*skp ;
	const uchar	*wterms ;
	int		qo ;		/* query options */
	int		npar ;		/* n-parallelism */
} ;

struct disp_thr {
	pthread_t	tid ;
	volatile int	f_busy ;
	volatile int	f_exiting ;
	volatile int	f_active ;
	volatile int	rs ;
} ;

struct disp_head {
	DISP_ARGS	a ;		/* arguments */
	TAGQ		wq ;		/* work-queue */
	PSEM		sem_wq ;	/* work-queue semaphore */
	PSEM		sem_done ;	/* done-semaphore */
	PTM		m ;		/* nbusy-mutex */
	PTC		cond ;		/* condition variable */
	DISP_THR	*threads ;	/* thread-privte data */
	volatile int	f_exit ;	/* assumed atomic */
	volatile int	f_done ;	/* assumed atomic */
	volatile int	f_ready ;	/* ready for workers to access */
	int		qlen ;		/* max work-queue length */
	int		nthr ;		/* bumber of threads configured */
} ;


/* forward references */

static int	textlook_infoloadbegin(TEXTLOOK *,const char *,const char *) ;
static int	textlook_infoloadend(TEXTLOOK *) ;
static int	textlook_indopen(TEXTLOOK *,SUBINFO *) ;

static int	textlook_snbegin(TEXTLOOK *) ;
static int	textlook_snend(TEXTLOOK *) ;

static int	textlook_dispstart(TEXTLOOK *,int,SEARCHKEYS *,RTAGS *) ;
static int	textlook_dispfinish(TEXTLOOK *) ;

static int	textlook_indclose(TEXTLOOK *) ;
static int	textlook_havekeys(TEXTLOOK *,TXTINDEX_TAG *,int,SEARCHKEYS *) ;
static int	textlook_havekeyer(TEXTLOOK *,TXTINDEX_TAG *,int,
			SEARCHKEYS *,SEARCHKEYS_POP *,cchar *) ;
static int	textlook_havekeyers(TEXTLOOK *,TXTINDEX_TAG *,int,
			SEARCHKEYS *,int,SEARCHKEYS_POP *) ;
static int	textlook_havekeysline(TEXTLOOK *,
			SEARCHKEYS *,SEARCHKEYS_POP *,cchar *,int) ;
static int	textlook_matchkeys(TEXTLOOK *,
			SEARCHKEYS *,SEARCHKEYS_POP *,cchar *,int) ;
static int	textlook_mkhkeys(TEXTLOOK *,vecstr *,SEARCHKEYS *) ;

static int	textlook_lookuper(TEXTLOOK *,TEXTLOOK_CUR *,int,
			SEARCHKEYS *,cchar **) ;
static int	textlook_checkdisp(TEXTLOOK *,int,SEARCHKEYS *,RTAGS *) ;

static int	subinfo_start(SUBINFO *) ;
static int	subinfo_finish(SUBINFO *) ;

static int	disp_start(DISP *,DISP_ARGS *) ;
static int	disp_starter(DISP *) ;
static int	disp_finish(DISP *,int) ;
static int	disp_addwork(DISP *,TXTINDEX_TAG *) ;
static int	disp_setstate(DISP *,DISP_THR *,int) ;
static int	disp_nbusy(DISP *) ;
static int	disp_nexiting(DISP *) ;
static int	disp_waitdone(DISP *) ;
static int	disp_worker(DISP *) ;
static int	disp_getourthr(DISP *,DISP_THR **) ;
static int	disp_readyset(DISP *) ;
static int	disp_readywait(DISP *) ;

static int	tagq_start(TAGQ *,int) ;
static int	tagq_finish(TAGQ *) ;
static int	tagq_count(TAGQ *) ;
static int	tagq_ins(TAGQ *,TXTINDEX_TAG *) ;
static int	tagq_rem(TAGQ *,TXTINDEX_TAG *) ;

static int	mkfieldterms(uchar *) ;

#ifdef	COMMENT
static int	vcmpthreads(DISP_THR **,DISP_THR **) ;
#endif /* COMMENT */


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

	rs1 = textlook_dispfinish(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGN
	nprintf(NDF,"textlook_close: _dispfinish() rs=%d\n",rs) ;
#endif

	rs1 = textlook_snend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = textlook_indclose(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = textlook_infoloadend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGN
	nprintf(NDF,"textlook_close: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
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
	                        debugprintf("textlook_lookup: hkeys=%u¬\n",rs) ;
	                        for (i = 0 ; hkeya[i] != NULL ; i += 1) {
	                            debugprintf("textlook_lookup: hkey=>%s<\n",
	                                hkeya[i]) ;
	                        }
	                    }
#endif /* CF_DEBUGS */

	                    rs = textlook_lookuper(op,curp,qo,&sk,hkeya) ;
	                    c = rs ;

/* sort the secondary tags */

	                    if (rs >= 0) {
	                        curp->ntags = c ;
	                        if (c > 1) {
	                            rtags_sort(&curp->tags,NULL) ;
	                        }
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
	} else {
	    rs = SR_NOTFOUND ;
	}

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


static int textlook_infoloadbegin(TEXTLOOK *op,cchar *dbname,cchar *basedname)
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
	int		rs ;
	const char	*cp ;

	if ((rs = textlook_snend(op)) >= 0) {
	    TXTINDEX_INFO	tinfo ;
	    if ((rs = txtindex_info(&op->ind,&tinfo)) >= 0) {
	        if (tinfo.sdn[0] != '\0') {
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
	        }
	    }
	} /* end if (textlook_snend) */

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
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

#if	CF_DEBUGS
	{
	    pthread_t	tid = pthread_self() ;
	    debugprintf("textlook_havekeys: tid=%u tag recoff=%u reclen=%u\n",
	    tid,tagp->recoff,tagp->reclen) ;
	}
#endif

	if (tagp->reclen > 0) {
	    SEARCHKEYS_POP	pkeys ;
	    const int		f_prefix = (qo & TEXTLOOK_OPREFIX) ;
	    if ((rs = searchkeys_popbegin(skp,&pkeys,f_prefix)) >= 0) {
	        if (rs > 0) {
	            cchar	*fn = tagp->fname ;
	            cchar	*dn ;
	            char	fname[MAXPATHLEN + 1] ;

	            if ((fn[0] == '\0') && (op->sfn != NULL)) {
	                fn = op->sfn ;
	            }

	            if (fn[0] != '\0') {

	                if (fn[0] != '/') {
	                    dn = op->sdn ;
	                    if ((dn == NULL) || (dn[0] == '\0')) {
	                        dn = op->basedname ;
	                    }
	                    if ((dn != NULL) && (dn[0] != '\0')) {
	                        rs = mkpath2(fname,dn,fn) ;
	                        fn = fname ;
	                    }
	                }

	                if (rs >= 0) {
	                    rs = textlook_havekeyer(op,tagp,qo,skp,&pkeys,fn) ;
	                    f = rs ;
	                }

	            } /* end if */

	        } /* end if (positive) */
	        rs1 = searchkeys_popend(skp,&pkeys) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (searchkeys-pop) */
	} /* end if (positive) */

#if	CF_DEBUGS
	{
	    pthread_t	tid = pthread_self() ;
	    debugprintf("textlook_havekeys: tid=%u ret rs=%d f=%u\n",tid,rs,f) ;
	}
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (textlook_havekeys) */


static int textlook_havekeyer(TEXTLOOK *op,TXTINDEX_TAG *tagp,int qo,
		SEARCHKEYS *skp,SEARCHKEYS_POP *pkp,cchar *fn)
{
	const int	of = O_RDONLY ;
	int		rs ;
	int		f = FALSE ;
	if ((rs = u_open(fn,of,0666)) >= 0) {
	    USTAT	sb ;
	    const int	fd = rs ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
	        if (S_ISREG(sb.st_mode)) {
	            const size_t	fsize = (sb.st_size & INT_MAX) ;
	            if (tagp->recoff < fsize) {
	                rs = textlook_havekeyers(op,tagp,qo,skp,fd,pkp) ;
	                f = (rs > 0) ;
	            }
	        }
	    }
	    u_close(fd) ;
	} else if (isNotAccess(rs)) {
	    rs = SR_OK ;
	} /* end if (file) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (textlook_havekeyer) */


/* ARGSUSED */
static int textlook_havekeyers(TEXTLOOK *op,TXTINDEX_TAG *tagp,int qo,
		SEARCHKEYS *skp,int fd,SEARCHKEYS_POP *pkp)
{
	offset_t	recoff = tagp->recoff ;
	offset_t	recext ;
	offset_t	mo ;
	size_t		ms ;
	uint		reclen = tagp->reclen ;
	const int	maxreclen = TEXTLOOK_MAXRECLEN ;
	const int	mt = PROT_READ ;
	const int	mf = MAP_SHARED ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;
	char		*md ;
	if (reclen > maxreclen) reclen = maxreclen ;
	mo = ufloor(recoff,op->pagesize) ;
	recext = (recoff + reclen) ;
	ms = uceil(recext,op->pagesize) - mo ;
	if ((rs = u_mmap(NULL,ms,mt,mf,fd,mo,&md)) >= 0) {
	    int		ml = reclen ;
	    int		ll ;
	    int		len ;
	    cchar	*lp ;
	    cchar	*tp ;
	    cchar	*mp = (md + (tagp->recoff - mo)) ;
	    while ((tp = strnchr(mp,ml,'\n')) != NULL) {

	        len = ((tp + 1) - mp) ;
	        lp = mp ;
	        ll = (len - 1) ;

	        if (ll > 0) {
	            rs = textlook_havekeysline(op,skp,pkp,lp,ll) ;
	            f = (rs > 0) ;
	            if (f) break ;
	        } /* end if */

	        ml -= len ;
	        mp += len ;
	        if (rs < 0) break ;
	    } /* end while (readling lines) */

	    rs1 = u_munmap(md,ms) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (memory-map) */
#if	CF_DEBUGS
	debugprintf("textlook_havekeyers: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (textlook_havekeyers) */


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
	    cchar	*fp, *sp, *kp ;
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
	int		rs1 ;
	int		f = FALSE ;

#if	CF_DEBUGS
	{
	    pthread_t	tid = pthread_self() ;
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

	    rs1 = xwords_finish(&xw) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */
#else /* CF_SINGLEWORD */
	if ((rs = xwords_start(&xw,sp,sl)) >= 0) {
	    int		wi = 0 ;
	    int		cl ;
	    cchar	*cp ;

	    while ((cl = xwords_get(&xw,wi,&cp)) > 0) {

#if	CF_DEBUGS 
	        debugprintf("textlook_matchkeys: xwords_get() rs=%d\n",cl) ;
	        if (cl >= 0)
	            debugprintf("textlook_matchkeys: c=>%t<\n",cp,cl) ;
#endif

	        rs = searchkeys_process(skp,pkp,cp,cl) ;
	        f = (rs > 0) ;

#if	CF_DEBUGS 
	        debugprintf("textlook_matchkeys: searchkeys_process() f=%u\n",
	            f) ;
#endif

	        if (rs < 0) break ;
	        if (f) break ;
	        wi += 1 ;
	    } /* end while (matching words) */

	    rs1 = xwords_finish(&xw) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */
#endif /* CF_SINGLEWORD */

#if	CF_DEBUGS && 0
	debugprintf("textlook_matchkeys: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (textlook_matchkeys) */


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
	    cchar	*kp ;

	    while (rs >= 0) {

	        kl = searchkeys_enum(skp,&cur,&kp) ;
	        if (kl < 0) break ;

	        if (kp == NULL) continue ;

	        if (kl < op->minwlen) continue ;

	        rs1 = SR_NOTFOUND ;

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


static int textlook_lookuper(op,curp,qo,skp,hkeya)
TEXTLOOK	*op ;
TEXTLOOK_CUR	*curp ;
int		qo ;
SEARCHKEYS	*skp ;
const char	**hkeya ;
{
	RTAGS		*rtp = &curp->tags ;
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
#endif /* CF_DEBUGS */

	if ((rs = textlook_checkdisp(op,qo,skp,rtp)) >= 0) {
	    TXTINDEX_CUR	tcur ;
	    TXTINDEX_TAG	ttag ;
	    DISP		*dop = op->disp ;

	    if ((rs = txtindex_curbegin(&op->ind,&tcur)) >= 0) {
	        if ((rs = txtindex_lookup(&op->ind,&tcur,hkeya)) >= 0) {
	            int	ntags = rs ;
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
	        } /* end if (txtindex_lookup) */
	        rs1 = txtindex_curend(&op->ind,&tcur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (txtindex-cur) */

	    rs1 = disp_waitdone(dop) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (disp-checkstart) */

	if (rs >= 0) {
	    rs = rtags_count(rtp) ;
	    c = rs ;
	}

#if	CF_DEBUGS
	debugprintf("textlook_lookuper: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (textlook_lookuper) */


static int textlook_checkdisp(TEXTLOOK *op,int qo,SEARCHKEYS *skp,RTAGS *rtp)
{
	int		rs = SR_OK ;

	if (op->disp == NULL) {
	    rs = textlook_dispstart(op,qo,skp,rtp) ;
	}

	return rs ;
}
/* end subroutine (textlook_checkdisp) */


static int textlook_dispstart(TEXTLOOK *op,int qo,SEARCHKEYS *skp,RTAGS *rtp)
{
	int		rs = SR_OK ;

	if (op->disp == NULL) {
	    if ((rs = uptgetconcurrency()) >= 0) {
	        const int	npar = (rs+1) ;
	        const int	size = sizeof(DISP) ;
	        void		*p ;
	        if ((rs = uc_malloc(size,&p)) >= 0) {
	            DISP	*dop = p ;
	            {
	                DISP_ARGS	a ;
	                memset(&a,0,sizeof(DISP_ARGS)) ;
	                a.op = op ;
	                a.qo = qo ;
	                a.skp = skp ;
	                a.rtp = rtp ;
	                a.npar = npar ;
	                a.wterms = op->wterms ;
	                if ((rs = disp_start(dop,&a)) >= 0) {
	                    op->disp = dop ;
	                }
	            } /* end block */
	            if (rs < 0) uc_free(p) ;
	        } /* end if (memory-allocation) */
	    } /* end if (uptgetconcurrency) */
	} /* end if (needed start-up) */

#if	CF_DEBUGS
	debugprintf("textlook_dispstart: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (textlook_dispstart) */


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
	nprintf(NDF,"textlook_dispfinish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (textlook_dispfinish) */


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


static int disp_start(DISP *dop,DISP_ARGS *dap)
{
	const int	qlen = TEXTLOOK_QLEN ;
	int		rs ;

	if (dop == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("textlook/disp_start: ent\n") ;
#endif

	memset(dop,0,sizeof(DISP)) ;
	dop->a = *dap ;
	dop->nthr = dap->npar ;

	if ((rs = ptm_create(&dop->m,NULL)) >= 0) {
	    if ((rs = ptc_create(&dop->cond,NULL)) >= 0) {
	        const int	f_sh = FALSE ;
	        if ((rs = psem_create(&dop->sem_wq,f_sh,0)) >= 0) {
	            PSEM	*ws = &dop->sem_done ;
	            if ((rs = psem_create(ws,f_sh,0)) >= 0) {
	                dop->qlen = (dop->a.npar + qlen) ;
	                if ((rs = tagq_start(&dop->wq,dop->qlen)) >= 0) {
	                    {
	                        rs = disp_starter(dop) ;
	                    }
	                    if (rs < 0)
	                        tagq_finish(&dop->wq) ;
	                } /* end if (tagq_start) */
	                if (rs < 0)
	                    psem_destroy(&dop->sem_done) ;
	            }
	            if (rs < 0)
	                psem_destroy(&dop->sem_wq) ;
	        }
	        if (rs < 0)
	            ptc_destroy(&dop->cond) ;
	    } /* end if (ptc-create) */
	    if (rs < 0)
	        ptm_destroy(&dop->m) ;
	} /* end if (ptm-create) */

	return rs ;
}
/* end subroutine (disp_start) */


/* start-up helper */
static int disp_starter(DISP *dop)
{
	const int	size = (dop->nthr*sizeof(DISP_THR)) ;
	int		rs ;
	int		i ;
	void		*p ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    DISP_THR	*dtp ;
	    pthread_t	tid ;
	    uptsub_t	fn = (uptsub_t) disp_worker ;
	    dop->threads = p ;
	    memset(p,0,size) ;
	    for (i = 0 ; i < dop->nthr ; i += 1) {
	        dtp = (dop->threads+i) ;
	        if ((rs = uptcreate(&tid,NULL,fn,dop)) >= 0) {
	            dtp->tid = tid ;
	            dtp->f_active = TRUE ;
	        }
	        if (rs < 0) break ;
	    } /* end for */
	    if (rs >= 0) {
	        rs = disp_readyset(dop) ;
	    }
	    if (rs < 0) {
	        const int	n = i ;
	        int		i ;
	        dop->f_exit = TRUE ;
	        for (i = 0 ; i < n ; i += 1) {
	            psem_post(&dop->sem_wq) ;
	        }
	        for (i = 0 ; i < n ; i += 1) {
	            dtp = (dop->threads+i) ;
	            uptjoin(dtp->tid,NULL) ;
	        } /* end for */
	        uc_free(p) ;
	        dop->threads = NULL ;
	    } /* end if (error) */
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (disp_starter) */


static int disp_finish(DISP *dop,int f_abort)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (dop == NULL) return SR_FAULT ;

	dop->f_done = TRUE ;		/* assumed to be atomic! */
	if (f_abort)
	    dop->f_exit = TRUE ;	/* assumed to be atomic! */

	for (i = 0 ; i < dop->nthr ; i += 1) {
	    rs1 = psem_post(&dop->sem_wq) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (dop->threads != NULL) {
	    DISP_THR	*dtp ;
	    pthread_t	tid ;
	    for (i = 0 ; i < dop->nthr ; i += 1) {
	        dtp = (dop->threads+i) ;
	        if (dtp->f_active) {
	            dtp->f_active = FALSE ;
	            tid = dtp->tid ;
	            rs1 = uptjoin(tid,NULL) ;
	            if (rs >= 0) rs = rs1 ;
	            if (rs >= 0) rs = dtp->rs ;
	        } /* end if (active) */
	    } /* end for */
	    rs1 = uc_free(dop->threads) ;
	    if (rs >= 0) rs = rs1 ;
	    dop->threads = NULL ;
	} /* end if (threads) */

	rs1 = tagq_finish(&dop->wq) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = psem_destroy(&dop->sem_done) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = psem_destroy(&dop->sem_wq) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ptm_destroy(&dop->m) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("textlook/disp_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (disp_finish) */


static int disp_addwork(DISP *dop,TXTINDEX_TAG *tagp)
{
	int		rs ;

	if ((rs = tagq_ins(&dop->wq,tagp)) >= 0) {
	    rs = psem_post(&dop->sem_wq) ; /* post always (more parallelism) */
	}

	return rs ;
}
/* end subroutine (disp_addwork) */


/* worker threads call this to set their "busy" status */
static int disp_setstate(DISP *dop,DISP_THR *tip,int f)
{
	PTM		*mp = &dop->m ;
	int		rs ;
	int		rs1 ;
	int		f_prev = FALSE ;
	if ((rs = ptm_lock(mp)) >= 0) {
	    f_prev = tip->f_busy ;
	    tip->f_busy = f ;
	    if ((! f) && f_prev) {
	        rs = psem_post(&dop->sem_done) ;
	    }
	    rs1 = ptm_unlock(mp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

	return (rs >= 0) ? f_prev : rs ;
}
/* end subroutine (disp_setstate) */


/* main or manager thread calls this to find out how many workers are busy */
static int disp_nbusy(DISP *dop)
{
	int		rs = SR_OK ;
	int		n = 0 ;
	if (dop->threads != NULL) {
	    DISP_THR	*dtp ;
	    int		i ;
	    for (i = 0 ; i < dop->nthr ; i += 1) {
	        dtp = (dop->threads+i) ;
	        if (dtp->f_busy) n += 1 ;
	    }
	}
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (disp_nbusy) */


/* manager thread calls this get get number of exited workers */
static int disp_nexiting(DISP *dop)
{
	DISP_THR	*dtp = dop->threads ;
	int		rs = SR_OK ;
	int		i ;
	int		n = 0 ;
	for (i = 0 ; i < dop->nthr ; i += 1) {
	    if (dtp[i].f_exiting) n += 1 ;
	}
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (disp_nexiting) */


/* main-thread calls this to wait for a parallel query to complete */
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
	    }

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
	        rs = disp_nexiting(dop) ;
	        nexited = rs ;
	        f_allexited = (nexited == dop->nthr) ;
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


static int disp_worker(DISP *dop)
{
	DISP_THR	*dtp ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
#if	CF_DEBUGS
	{
	    pthread_t	tid = pthread_self() ;
	    debugprintf("procquery/worker: ent tid=%u\n",tid) ;
	}
#endif
	if ((rs = disp_getourthr(dop,&dtp)) >= 0) {
	    TEXTLOOK		*op = dop->a.op ;
	    SEARCHKEYS		*skp ;
	    RTAGS		*rtp ;
	    int			qo ;

	    while ((rs >= 0) && (! dop->f_exit)) {
	        if ((rs = psem_wait(&dop->sem_wq)) >= 0) {
	            if (dop->f_exit) break ;

	            if ((rs = disp_setstate(dop,dtp,TRUE)) >= 0) {
	    		TXTINDEX_TAG	qv ;

	                while ((rs = tagq_rem(&dop->wq,&qv)) > 0) {

	                    qo = dop->a.qo ; /* renew */
	                    skp = dop->a.skp ; /* renew */
	                    rtp = dop->a.rtp ; /* renew */

#if	CF_DEBUGS
			    {
			    pthread_t	tid = pthread_self() ;
	                    debugprintf("procquery/worker: "
	                        "tid=%u fname=%s\n", tid,qv.fname) ;
			    }
#endif

	                    if ((rs = textlook_havekeys(op,&qv,qo,skp)) > 0) {
	    			RTAGS_TAG	rt ;
	                        c += 1 ;
	                        rt.hash = 0 ;
	                        rt.recoff = qv.recoff ;
	                        rt.reclen = qv.reclen ;
	                        rt.fname[0] = '\0' ;
	                        if (qv.fname[0] != '\0') {
	                            mkpath1(rt.fname,qv.fname) ;
	                        }
	                        rs = rtags_add(rtp,&rt) ;
	                    } /* end if (found a key) */

	            	    if (dop->f_exit) break ;
	                } /* end while (work) */

	                rs1 = disp_setstate(dop,dtp,FALSE) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (worker was busy) */

	        } /* end if (psem_wait) */
	        if (dop->f_done) break ;
	    } /* end while (waiting) */

#if	CF_DEBUGS
	    {
	        pthread_t	tid = pthread_self() ;
	        debugprintf("procquery/worker: tid=%u ret rs=%d c=%u\n",
	            tid,rs,c) ;
	    }
#endif

	    dtp->rs = rs ;
	    dtp->f_exiting = TRUE ;
	} /* end if (disp_getourthr) */
#if	CF_DEBUGN
	{
	    pthread_t	tid = pthread_self() ;
	    nprintf(NDF,"textlook/worker: ret tid=%u rs=%d c=%u\n",
		tid,rs,c) ;
	}
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (disp_worker) */


/* child thread calls this to get its own local-data pointer */
static int disp_getourthr(DISP *dop,DISP_THR **rpp)
{
	int		rs ;
	int		i = 0 ;
	int		f = FALSE ;
	if ((rs = disp_readywait(dop)) >= 0) {
	    DISP_THR	*dtp ;
	    pthread_t	tid = pthread_self() ;
	    for (i = 0 ; i < dop->nthr ; i += 1) {
	        dtp = (dop->threads+i) ;
	        f = uptequal(dtp->tid,tid) ;
	        if (f) break ;
	    } /* end for */
	    if (f) {
	        if (rpp != NULL) *rpp = dtp ;
	    } else {
	        rs = SR_BUGCHECK ;
	    }
	} /* end if (disp_readywait) */
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (disp_getourthr) */


/* main-thread calls this to indicate sub-threads can read completed object */
static int disp_readyset(DISP *dop)
{
	PTM		*mp = &dop->m ;
	int		rs ;
	int		rs1 ;
	if ((rs = ptm_lock(mp)) >= 0) {
	    {
	        dop->f_ready = TRUE ;
	        rs = ptc_broadcast(&dop->cond) ; /* 0-bit semaphore */
	    }
	    rs1 = ptm_unlock(mp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */
	return rs ;
}
/* end subroutine (disp_readyset) */


/* sub-threads call this to wait until object is ready */
static int disp_readywait(DISP *dop)
{
	PTM		*mp = &dop->m ;
	int		rs ;
	int		rs1 ;
	if ((rs = ptm_lock(mp)) >= 0) {
	    while ((! dop->f_ready) && (! dop->f_exit)) {
	        rs = ptc_wait(&dop->cond,mp) ;
	        if (rs < 0) break ;
	    } /* end while */
	    rs1 = ptm_unlock(mp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */
	return rs ;
}
/* end subroutine (disp_readywait) */


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
	int		rs2 ;
	char		*p ;

	while ((rs2 = ciq_rem(cqp,&p)) >= 0) {
	    rs1 = uc_free(p) ; /* these things are opaque */
	    if (rs >= 0) rs = rs1 ;
	}
	if ((rs >= 0) && (rs2 != SR_NOTFOUND)) rs = rs2 ;

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

	    if ((rs = psem_wait(&tqp->wsem)) >= 0) {
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
	    tagp->recoff = ttp->recoff ;
	    tagp->reclen = ttp->reclen ;
	    rc = strwcpy(tagp->fname,ttp->fname,MAXPATHLEN) - tagp->fname ;
	    uc_free(ttp) ;
	    rs = psem_post(&tqp->wsem) ;
	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	} /* end if (ciq_rem) */

#if	CF_DEBUGS
	debugprintf("tagq_rem: ret rs=%d rc=%d\n",rs,rc) ;
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (tagq_rem) */


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


#ifdef	COMMENT
static int vcmpthreads(DISP_THR **e1pp,DISP_THR **e2pp)
{
	int		rc = 0 ;

	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            DISP_THR	*e1p = *e1pp ;
	            DISP_THR	*e2p = *e2pp ;
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
#endif /* COMMENT */


