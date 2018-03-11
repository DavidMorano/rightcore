/* votdc */

/* VOTDs system cache management */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_GETACOUNT	0		/* |votdc_getacount()| */
#define	CF_UPDATE	0		/* |votdc_update()| */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This is really a new thing but with pieces borrowed from things from
	the way past.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This modules provides management for the system Verse-of-the-Day (VOTD)
	cache.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<strings.h>		/* for |strcasecmp(3c)| */
#include	<stdarg.h>

#include	<vsystem.h>
#include	<endian.h>
#include	<getbufsize.h>
#include	<intceil.h>
#include	<estrings.h>
#include	<filebuf.h>
#include	<storebuf.h>
#include	<ptma.h>
#include	<ptm.h>
#include	<sigblock.h>
#include	<getax.h>
#include	<localmisc.h>

#include	"votdc.h"
#include	"votdchdr.h"
#include	"cvtdater.h"
#include	"shmalloc.h"


/* local defines */

#define	VOTDC_OBJNAME		"votdc"
#define	VOTDC_SHMPOSTFIX	"votdc"
#define	VOTDC_PERMS		0666
#define	VOTDC_MUSIZE		sizeof(PTM)
#define	VOTDC_BOOKSIZE		(VOTDC_NBOOKS*sizeof(VOTDC_BOOK))
#define	VOTDC_VERSESIZE		(VOTDC_NVERSES*sizeof(VOTDC_VERSE))
#define	VOTDC_MINSIZE		\
				VOTDC_BOOKIZE+ \
				VOTDC_VERSESIZE+ \
				VOTDC_BSTRSIZE+ \
				VOTDC_VSTRSIZE+ \
				(2*sizeof(SHMALLOC))
#define	VOTDC_BOOKLEN		80 /* should match 'BIBLEBOOK_LEN' */

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

#define	HDRBUFLEN	(sizeof(VOTDCHDR) + MAXNAMELEN)

#ifndef	TO_WAITSHM
#define	TO_WAITSHM	30		/* seconds */
#endif


/* external subroutines */

extern int	sfbasename(const char *,int,const char **) ;
extern int	sfrootname(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matocasestr(const char **,int,const char *,int) ;
extern int	strnnlen(const char *,int,int) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	msleep(uint) ;
extern int	isOneOf(const int *,int) ;
extern int	filebuf_writefill(FILEBUF *,const char *,int) ;
extern int	filebuf_writezero(FILEBUF *,int) ;
extern int	filebuf_writealign(FILEBUF *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpyopaque(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;

#if	CF_DEBUGS
extern char	*timestr_log(time_t,char *) ;
#endif


/* exported variables */

VOTDC_OBJ	votdc = {
	VOTDC_OBJNAME,
	sizeof(VOTDC)
} ;


/* local structures */


/* forward references */

static int	votdc_shmhandbegin(VOTDC *,const char *) ;
static int	votdc_shmhandend(VOTDC *) ;

static int	votdc_strbegin(VOTDC *,cchar *,cchar *) ;
static int	votdc_strend(VOTDC *) ;

static int	votdc_shmbegin(VOTDC *,int,mode_t) ;
static int	votdc_shmbeginer(VOTDC *,time_t,int,mode_t,int) ;
static int	votdc_shmend(VOTDC *) ;

static int	votdc_mapbegin(VOTDC *,time_t,int) ;
static int	votdc_mapend(VOTDC *) ;

static int	votdc_shmhdrin(VOTDC *,VOTDCHDR *) ;
static int	votdc_shmprep(VOTDC *,time_t,int,mode_t,VOTDCHDR *) ;
static int	votdc_shmpreper(VOTDC *,time_t,int,mode_t,VOTDCHDR *) ;
static int	votdc_shmwriter(VOTDC *,time_t,int fd,VOTDCHDR *,cchar *,int) ;
static int	votdc_allocinit(VOTDC *,VOTDCHDR *) ;
static int	votdc_mutexinit(VOTDC *) ;
static int	votdc_shmchown(VOTDC *) ;
static int	votdc_verify(VOTDC *) ;

static int	votdc_bookslotfind(VOTDC *,const char *) ;
static int	votdc_bookslotload(VOTDC *,time_t,int,cchar *,cchar **) ;
static int	votdc_bookslotdump(VOTDC *,int) ;
static int	votdc_verselangdump(VOTDC *,int) ;

static int	votdc_booklanghave(VOTDC *,const char *) ;
static int	votdc_versehave(VOTDC *,int,int) ;
static int	votdc_verseslotnext(VOTDC *,int) ;
static int	votdc_verseslotfind(VOTDC *) ;
static int	votdc_verseslotfinder(VOTDC *) ;

static int	votdc_mktitles(VOTDC *,const char *,int) ;
static int	votdc_titlematcher(VOTDC *,int,int,const char *,int) ;
static int	votdc_titlevalid(VOTDC *,int) ;
static int	votdc_titletouse(VOTDC *) ;
static int	votdc_access(VOTDC *) ;
static int	votdc_getwcount(VOTDC *) ;
static int	votdc_titlefins(VOTDC *) ;

#if	CF_GETACOUNT
static int	votdc_getacount(VOTDC *) ;
#endif /* CF_GETACOUNT */

#if	CF_UPDATE
static int	votdc_update(VOTDC *) ;
#endif /* CF_UPDATE */

static int	verse_dump(VOTDC_VERSE *,SHMALLOC *,int) ;
static int	verse_match(VOTDC_VERSE *,int,int) ;
static int	verse_read(VOTDC_VERSE *,char *,
			VOTDC_CITE *,char *,int) ;
static int	verse_load(VOTDC_VERSE *,time_t,int,SHMALLOC *,char *,
			VOTDC_CITE *,int,cchar *,int) ;
static int	verse_isempty(VOTDC_VERSE *) ;
static int	verse_isused(VOTDC_VERSE *) ;
static int	verse_isleast(VOTDC_VERSE *,time_t *) ;
static int	verse_accessed(VOTDC_VERSE *,time_t) ;

static int	book_load(VOTDC_BOOK *,SHMALLOC *,char *,time_t,int,
			cchar *,cchar **) ;
static int	book_dump(VOTDC_BOOK *,SHMALLOC *) ;
static int	book_getwmark(VOTDC_BOOK *) ;
static int	book_getwmarklang(VOTDC_BOOK *,const char **) ;
static int	book_read(VOTDC_BOOK *,char *,int,char *,int,int) ;

static int	titlecache_load(VOTDC_TC *,int,cchar *,char *,int *) ;
static int	titlecache_release(VOTDC_TC *) ;

static int	mkshmname(char *,int,cchar *,int,cchar *) ;


/* local variables */


/* exported subroutines */


int votdc_open(VOTDC *op,cchar *pr,cchar *lang,int of)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("votdc_open: ent\n") ;
	debugprintf("votdc_open: bstrsize=%d\n",VOTDC_BSTRSIZE) ;
	debugprintf("votdc_open: vstrsize=%d\n",VOTDC_VSTRSIZE) ;
	{
	    char	obuf[100+1] ;
	    snopenflags(obuf,100,of) ;
	    debugprintf("votdc_open: of=%s\n",obuf) ;
	}
#endif /* CF_DEBUGS */

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if ((lang == NULL) || (lang[0] == '\0'))
 	    lang = VOTDC_DEFLANG ;

#if	CF_DEBUGS
	debugprintf("votdc_open: pr=%s lang=%s\n",pr,lang) ;
#endif

	of &= (~ O_ACCMODE) ;
	of |= O_RDWR ;

	memset(op,0,sizeof(VOTDC)) ;
	op->pagesize = getpagesize() ;
	op->fd = -1 ;

	if ((rs = votdc_shmhandbegin(op,pr)) >= 0) {
	    if ((rs = votdc_strbegin(op,pr,lang)) >= 0) {
		const mode_t	om = VOTDC_PERMS ;
	        if ((rs = votdc_shmbegin(op,of,om)) >= 0) {
	   	    op->magic = VOTDC_MAGIC ;
	        } /* end if (votdc_shmbegin) */
	        if (rs < 0) {
		    votdc_strend(op) ;
		}
	    } /* end if (memory-allocation) */
	    if (rs < 0)
		votdc_shmhandend(op) ;
	} /* end if (votdc-shmname) */

#if	CF_DEBUGS
	debugprintf("votdc_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (votdc_open) */


int votdc_close(VOTDC *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("votdc_close: ent\n") ;
#endif
	if (op == NULL) return SR_FAULT ;

	if (op->magic != VOTDC_MAGIC) return SR_NOTOPEN ;

	rs1 = votdc_titlefins(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = votdc_shmend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = votdc_strend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = votdc_shmhandend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
#if	CF_DEBUGS
	debugprintf("votdc_close: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (votdc_close) */


/* loads all book-titles with one call */
int votdc_titleloads(VOTDC *op,cchar *lang,cchar **tv)
{
	SIGBLOCK	s ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("votdc_titleloads: ent lang=%s\n",lang) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (lang == NULL) return SR_FAULT ;
	if (tv == NULL) return SR_FAULT ;

	if (op->magic != VOTDC_MAGIC) return SR_NOTOPEN ;

	if (lang[0] == '\0') return SR_INVALID ;

	if ((rs = sigblock_start(&s,NULL)) >= 0) {
	    if ((rs = ptm_lock(op->mp)) >= 0) {
#if	CF_DEBUGS
		debugprintf("votdc_titleloads: locked\n") ;
#endif
		if ((rs = votdc_access(op)) >= 0) {
		    const time_t	dt = time(NULL) ;
	            if ((rs = votdc_bookslotfind(op,lang)) >= 0) {
	    	        rs = votdc_bookslotload(op,dt,rs,lang,tv) ;
	            } /* end if (votdc_findslot) */
		} /* end if (votdc_access) */
	        rs1 = ptm_unlock(op->mp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	    rs1 = sigblock_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */

#if	CF_DEBUGS
	debugprintf("votdc_titleloads: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (votdc_titleloads) */


/* do we have book-titles in a language? */
int votdc_titlelang(VOTDC *op,cchar *lang)
{
	SIGBLOCK	s ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("votdc_titlelang: ent\n") ;
#endif
	if (op == NULL) return SR_FAULT ;
	if (lang == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("votdc_titlelang: lang=%s\n",lang) ;
#endif
	if (op->magic != VOTDC_MAGIC) return SR_NOTOPEN ;

	if (lang[0] == '\0') return SR_INVALID ;

	if ((rs = sigblock_start(&s,NULL)) >= 0) {
	    if ((rs = ptm_lock(op->mp)) >= 0) {
	            int		bi ;
	            const char	*blang ;
#if	CF_DEBUGS
		    debugprintf("votdc_titlelang: inside\n") ;
#endif
	            for (bi = 0 ; bi < VOTDC_NBOOKS ; bi += 1) {
		        blang = op->books[bi].lang ;
		        f = (strcasecmp(blang,lang) == 0) ;
		        if (f) break ;
	            } /* end for */
	        rs1 = ptm_unlock(op->mp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	    rs1 = sigblock_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */

#if	CF_DEBUGS
	debugprintf("votdc_titlelang: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (votdc_titlelang) */


int votdc_titleget(VOTDC *op,char *rbuf,int rlen,int li,int ti)
{
	SIGBLOCK	s ;
	int		rs ;
	int		rs1 ;
	int		rl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != VOTDC_MAGIC) return SR_NOTOPEN ;

	if ((li < 0) || (li >= VOTDC_NBOOKS)) return SR_INVALID ;
	if ((ti < 0) || (ti >= VOTDC_NTITLES)) return SR_INVALID ;

	rbuf[0] = '\0' ;
	if ((rs = sigblock_start(&s,NULL)) >= 0) {
	    if ((rs = ptm_lock(op->mp)) >= 0) {
		if ((rs = votdc_access(op)) >= 0) {
		    VOTDC_BOOK	*bep = (op->books+li) ;
		    const int	ac = rs ;
		    char	*bstr = op->bstr ;
		    rs = book_read(bep,bstr,ac,rbuf,rlen,ti) ;
		    rl = rs ;
		} /* end if (votdc_access) */
	        rs1 = ptm_unlock(op->mp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	    rs1 = sigblock_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (votdc_titleget) */


int votdc_titlefetch(VOTDC *op,char *rbuf,int rlen,cchar *lang,int ti)
{
	SIGBLOCK	s ;
	int		rs ;
	int		rs1 ;
	int		rl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (lang == NULL) return SR_FAULT ;

	if (op->magic != VOTDC_MAGIC) return SR_NOTOPEN ;

	if (lang[0] == '\0') return SR_INVALID ;
	if ((ti < 0) || (ti >= VOTDC_NTITLES)) return SR_INVALID ;

	rbuf[0] = '\0' ;
	if ((rs = sigblock_start(&s,NULL)) >= 0) {
	    if ((rs = ptm_lock(op->mp)) >= 0) {
		if ((rs = votdc_access(op)) >= 0) {
		    const int	ac = rs ;
	            int		bi ;
	            int		f = FALSE ;
	            const char	*blang ;
	            for (bi = 0 ; bi < VOTDC_NBOOKS ; bi += 1) {
		        blang = op->books[bi].lang ;
		        f = (strcasecmp(blang,lang) == 0) ;
		        if (f) break ;
	            } /* end for */
	            if (f) {
			VOTDC_BOOK	*bep = (op->books+bi) ;
			char		*bstr = op->bstr ;
			rs = book_read(bep,bstr,ac,rbuf,rlen,ti) ;
		        rl = rs ;
		    } else {
		        rs = SR_NOTFOUND ;
		    }
		} /* end if (votdc_access) */
	        rs1 = ptm_unlock(op->mp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	    rs1 = sigblock_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (votdc_titlefetch) */


int votdc_titlematch(VOTDC *op,cchar *lang,cchar *sp,int sl)
{
	SIGBLOCK	s ;
	int		rs ;
	int		rs1 ;
	int		bi = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (lang == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != VOTDC_MAGIC) return SR_NOTOPEN ;

	if (lang[0] == '\0') return SR_INVALID ;
	if (sp[0] == '\0') return SR_INVALID ;

	if ((rs = sigblock_start(&s,NULL)) >= 0) {
	    if ((rs = ptm_lock(op->mp)) >= 0) {
		if ((rs = votdc_access(op)) >= 0) {
		    const int	ac = rs ;
	            int		f = FALSE ;
	            const char	*blang ;
	            for (bi = 0 ; bi < op->hdr.booklen ; bi += 1) {
		        blang = op->books[bi].lang ;
		        f = (strcasecmp(blang,lang) == 0) ;
		        if (f) break ;
	            } /* end for */
	            if (f) {
		        const int	clen = VOTDC_BOOKLEN ;
		        int		cl ;
		        char		cbuf[VOTDC_BOOKLEN+1] ;
		        cl = strwcpyopaque(cbuf,sp,MIN(clen,sl)) - cbuf ;
		        if ((rs = votdc_mktitles(op,lang,bi)) >= 0) {
			    rs = votdc_titlematcher(op,ac,rs,cbuf,cl) ;
			    bi = rs ;
		        }
	            } else {
		        rs = SR_NOTFOUND ;
		    }
		} /* end if (votdc_access) */
	        rs1 = ptm_unlock(op->mp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	    rs1 = sigblock_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */

	return (rs >= 0) ? bi : rs ;
}
/* end subroutine (votdc_titlematch) */


int votdc_versefetch(op,citep,rbuf,rlen,lang,mjd)
VOTDC		*op ;
VOTDC_CITE	*citep ;
char		*rbuf ;
int		rlen ;
const char	*lang ;
int		mjd ;
{
	SIGBLOCK	s ;
	int		rs ;
	int		rs1 ;
	int		vl = 0 ;
#if	CF_DEBUGS
	debugprintf("votdc_versefetch: ent mjd=%u\n",mjd) ;
#endif
	if (op == NULL) return SR_FAULT ;
	if (citep == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (lang == NULL) return SR_FAULT ;
	if (lang[0] == '\0') return SR_INVALID ;
	if (mjd < 0) return SR_INVALID ;
	if ((rs = sigblock_start(&s,NULL)) >= 0) {
	    if ((rs = ptm_lock(op->mp)) >= 0) {
	        if ((rs = votdc_booklanghave(op,lang)) >= 0) {
	            int	li = rs ;
		    if ((rs = votdc_versehave(op,li,mjd)) >= 0) {
		        VOTDC_VERSE	*vep = (op->verses+rs) ;
			char		*vstr = op->vstr ;
		        if ((rs = verse_read(vep,vstr,citep,rbuf,rlen)) >= 0) {
			    const time_t	dt = time(NULL) ;
			    vl = rs ;
			    verse_accessed(vep,dt) ;
			}
		    } /* end if (votdc_versehave) */
	        } /* end if (votdc_booklanghave) */
	        rs1 = ptm_unlock(op->mp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	    rs1 = sigblock_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */
#if	CF_DEBUGS
	if (rs >= 0) {
	    debugprintf("votdc_versefetch: found mjd=%u b=%u\n",mjd,citep->b) ;
	}
	debugprintf("votdc_versefetch: ret rs=%d vl=%u\n",rs,vl) ;
#endif
	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (votdc_versefetch) */


int votdc_verseload(op,lang,citep,mjd,vp,vl)
VOTDC		*op ;
const char	*lang ;
VOTDC_CITE	*citep ;
int		mjd ;
const char	*vp ;
int		vl ;
{
	SIGBLOCK	s ;
	int		rs ;
	int		rs1 ;
#if	CF_DEBUGS
	debugprintf("votdc_verseload: ent mjd=%u\n",mjd) ;
	debugprintf("votdc_verseload: lang=%s\n",lang) ;
	debugprintf("votdc_verseload: cite=%u:%u:%u\n",
		citep->b,citep->c,citep->v) ;
#endif
	if (op == NULL) return SR_FAULT ;
	if (lang == NULL) return SR_FAULT ;
	if (citep == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;
	if (vp[0] == '\0') return SR_INVALID ;
	if (mjd < 0) return SR_INVALID ;
	if ((rs = sigblock_start(&s,NULL)) >= 0) {
	    if ((rs = ptm_lock(op->mp)) >= 0) {
		if ((rs = votdc_access(op)) >= 0) {
		    const int	ac = rs ;
	            if ((rs = votdc_booklanghave(op,lang)) >= 0) {
		        SHMALLOC	*vap = op->vall ;
		        const time_t	dt = time(NULL) ;
			const int	rsn = SR_NOTFOUND ;
	                int		li = rs ;
		        int		vi = -1 ;
		        char		*vstr = op->vstr ;
			citep->l = li ;
		        if ((rs = votdc_versehave(op,li,mjd)) >= 0) {
			    VOTDC_VERSE	*vep = (op->verses+rs) ;
			    vi = rs ;
			    rs = verse_dump(vep,vap,li) ;
		        } else if (rs == rsn) {
			    rs = SR_OK ;
			}
		        if (rs >= 0) {
#if	CF_DEBUGS
			    debugprintf("votdc_verseload: vi=%d\n",vi) ;
#endif
			    if (vi < 0) {
			        if ((rs = votdc_verseslotfind(op)) == rsn) {
				    if ((rs = votdc_verseslotfinder(op)) >= 0) {
			    		VOTDC_VERSE	*vep = (op->verses+rs) ;
			            	vi = rs ;
			    		rs = verse_dump(vep,vap,li) ;
				    }
			        } else {
			            vi = rs ;
				}
#if	CF_DEBUGS
				debugprintf("votdc_verseload: "
					"_verseslotfind() rs=%d\n",rs) ;
#endif
			    } /* end if (need slot) */
#if	CF_DEBUGS
				debugprintf("votdc_verseload: "
				"rs=%d load vi=%d\n",rs,vi) ;
#endif
			    if (rs >= 0) {
			        VOTDC_VERSE	*vep = (op->verses+vi) ;
				VOTDC_CITE	*cp = citep ;
		                rs = verse_load(vep,dt,ac,vap,vstr,
					cp,mjd,vp,vl) ;
				vl = rs ;
#if	CF_DEBUGS
	if (rs >= 0) {
	debugprintf("votdc_verseload: "
		"verse_load() rs=%d mjd=%u b=%u\n",
		rs,vep->mjd, vep->book) ;
	}
#endif
			    }
		        } /* end if (ok) */
	            } /* end if (votdc_booklanghave) */
		} /* end if (votdc_access) */
	        rs1 = ptm_unlock(op->mp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	    rs1 = sigblock_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */
#if	CF_DEBUGS
	debugprintf("votdc_verseload: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (votdc_verseload) */


int votdc_info(VOTDC *op,VOTDC_INFO *bip)
{
	SIGBLOCK	s ;
	int		rs ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (bip == NULL) return SR_FAULT ;

	if (op->magic != VOTDC_MAGIC) return SR_NOTOPEN ;

	if ((rs = sigblock_start(&s,NULL)) >= 0) {
	    if ((rs = ptm_lock(op->mp)) >= 0) {
	        VOTDCHDR	*hdrp = &op->hdr ;
	        int		n, i ;
	        bip->wtime = (hdrp->wtime & UINT_MAX) ;
	        bip->atime = (hdrp->atime & UINT_MAX) ;
	        {
	            VOTDC_BOOK	*vbp = (VOTDC_BOOK *) op->books ;
		    n = 0 ;
	            for (i = 0 ; i < op->hdr.booklen ; i += 1) {
		        if (vbp[i].b[0] != '\0') n += 1 ;
	            } /* end for */
	            bip->nbooks = n ;
	        }
	        {
	            VOTDC_VERSE	*vvp = (VOTDC_VERSE *) op->verses ;
		    n = 0 ;
	            for (i = 0 ; i < op->hdr.reclen ; i += 1) {
		        if (vvp[i].voff != '\0') n += 1 ;
	            } /* end for */
	            bip->nverses = n ;
	        }
	        rs1 = ptm_unlock(op->mp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	    rs1 = sigblock_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */

	return rs ;
}
/* end subroutine (votdc_info) */


int votdc_vcurbegin(VOTDC *op,VOTDC_VCUR *curp)
{
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (votdc_vcurbegin) */


int votdc_vcurend(VOTDC *op,VOTDC_VCUR *curp)
{
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (votdc_vcurend) */


int votdc_vcurenum(VOTDC *op,VOTDC_VCUR *curp,VOTDC_CITE *citep,
	char *rbuf,int rlen)
{
	SIGBLOCK	s ;
	int		rs ;
	int		rs1 ;
	int		vl = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (citep == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
#if	CF_DEBUGS
	debugprintf("votdc_vcurenum: ent\n") ;
#endif
	if ((rs = sigblock_start(&s,NULL)) >= 0) {
	    if ((rs = ptm_lock(op->mp)) >= 0) {
		const int	vi = (curp->i >= 0) ? (curp->i+1) : 0 ;
		if ((rs = votdc_verseslotnext(op,vi)) >= 0) {
		    VOTDC_VERSE	*vep = (op->verses+rs) ;
		    const int	ci = rs ;
		    char	*vstr = op->vstr ;
		    if ((rs = verse_read(vep,vstr,citep,rbuf,rlen)) >= 0) {
			vl = rs ;
			curp->i = ci ;
		    }
		} /* end if (votdc_verseslotnext) */
	        rs1 = ptm_unlock(op->mp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	    rs1 = sigblock_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sigblock) */
#if	CF_DEBUGS
	debugprintf("votdc_vcurenum: ret rs=%d vl=%u\n",rs,vl) ;
#endif
	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (votdc_vcurenum) */


/* private subroutines */


static int votdc_strbegin(VOTDC *op,cchar *pr,cchar *lang)
{
	int		rs ;
	int		size = 0 ;
	char		*bp ;
	size += (strlen(pr)+1) ;
	size += (strlen(lang)+1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    op->a = bp ;
	    op->pr = bp ;
	    bp = (strwcpy(bp,pr,-1)+1) ;
	    op->lang = bp ;
	    bp = (strwcpy(bp,lang,-1)+1) ;
	}
	return rs ;
}
/* end subroutine (votdc_strbegin) */


static int votdc_strend(VOTDC *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->a != NULL) {
	    rs1 = uc_free(op->a) ;
	    if (rs >= 0) rs = rs1 ;
	    op->a = NULL ;
	    op->pr = NULL ;
	    op->lang = NULL ;
	}

	return rs ;
}
/* end subroutine (votdc_strend) */


static int votdc_shmbegin(VOTDC *op,int of,mode_t om)
{
	VOTDCHDR	*hdrp = &op->hdr ;
	const time_t	dt = time(NULL) ;
	const int	rsn = SR_NOENT ;
	int		rs = SR_OK ;
	int		fd = -1 ;
	int		f_needinit = FALSE ;
	const char	*shmname = op->shmname ;

#if	CF_DEBUGS
	debugprintf("votdc_shmbegin: ent\n") ;
#endif

#ifdef	COMMENT
	of &= (~ O_ACCMODE) ;
	of |= O_RDWR ;
#endif /* COMMENT */

	memset(hdrp,0,sizeof(VOTDCHDR)) ;

	if (rs >= 0) {
	    mode_t	mof = (of & (~ O_CREAT)) ;
#if	CF_DEBUGS
	debugprintf("votdc_shmbegin: shmname=%s\n",shmname) ;
#endif
	    if ((rs = uc_openshm(shmname,mof,om)) == rsn) {
	        if (of & O_CREAT) {
	            of |= O_EXCL ;
	            if ((rs = uc_openshm(shmname,of,(om & 0444))) >= 0) {
	                fd = rs ;
	                if ((rs = votdc_shmprep(op,dt,fd,om,hdrp)) >= 0) {
	                    f_needinit = TRUE ;
		        }
	            } /* end if (uc_openshm) */
	        } /* end if (create mode) */
	    } else {
	        fd = rs ;
	    }
	} /* end if (uc_openshm) */

#if	CF_DEBUGS
	debugprintf("votdc_shmbegin: 1 try rs=%d\n",rs) ;
#endif

	if ((rs == SR_ACCESS) || (rs == SR_EXIST)) {
	    const int	to = TO_WAITSHM ;
	    op->shmsize = 0 ;
	    rs = uc_openshmto(shmname,of,om,to) ;
	    fd = rs ;
	    if ((rs == SR_TIMEDOUT) && (of & O_CREAT)) {
#if	CF_DEBUGS
	debugprintf("votdc_shmbegin: timout rs=%d\n",rs) ;
#endif
		if ((rs = uc_unlinkshm(shmname)) >= 0) {
	            if ((rs = uc_openshm(shmname,of,(om & 0444))) >= 0) {
	                fd = rs ;
	                if ((rs = votdc_shmprep(op,dt,fd,om,hdrp)) >= 0) {
	                    f_needinit = TRUE ;
		        }
		    }
		} /* end if (uc_unlinkshm) */
	    } /* end if (timed-out) */
	} /* end if (waiting for file to be ready) */

#if	CF_DEBUGS
	debugprintf("votdc_shmbegin: 2 try rs=%d\n",rs) ;
#endif

	if (rs >= 0) { /* opened */
	    if ((rs = uc_fsize(fd)) > 0) {
#if	CF_DEBUGS
	debugprintf("votdc_shmbegin: fsize=%u\n",rs) ;
#endif
	        op->shmsize = rs ;
		rs = votdc_shmbeginer(op,dt,fd,om,f_needinit) ;
	    } else if ((rs == 0) && (of & O_CREAT)) {
#if	CF_DEBUGS
	debugprintf("votdc_shmbegin: fsize=%u\n",rs) ;
#endif
		if ((rs = votdc_shmprep(op,dt,fd,om,hdrp)) >= 0) {
	    	    if ((rs = uc_fsize(fd)) > 0) {
	                op->shmsize = rs ;
			rs = votdc_shmbeginer(op,dt,fd,om,TRUE) ;
		    } else {
			rs = SR_LIBACC ;
		    }
		}
	    } /* end if (uc_fsize) */
	    if (rs < 0) {
		u_close(fd) ;
		op->fd = -1 ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("votdc_shmbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (votdc_shmbegin) */


static int votdc_shmbeginer(op,dt,fd,om,f_needinit)
VOTDC		*op ;
time_t		dt ;
int		fd ;
mode_t		om ;
int		f_needinit ;
{
	int		rs ;
#if	CF_DEBUGS
	debugprintf("votdc_shmbeginer: ent f_needinit=%u\n",f_needinit) ;
#endif
	if ((rs = votdc_mapbegin(op,dt,fd)) >= 0) {
	    VOTDCHDR	*hdrp = &op->hdr ;
	    if ((rs = votdc_shmhdrin(op,hdrp)) >= 0) {
		op->fd = fd ;
		if (f_needinit) {
		    if ((rs = votdc_allocinit(op,hdrp)) >= 0) {
			if ((rs = votdc_mutexinit(op)) >= 0) {
			    if ((rs = u_fchmod(fd,om)) >= 0) {
				rs = votdc_shmchown(op) ;
			    }
			}
		    }
		} /* end if (needed it) */
	    } /* end if (votdc_shmhdrin) */
	    if (rs < 0)
	        votdc_mapend(op) ;
	} /* end if (votdc_mapbegin) */
#if	CF_DEBUGS
	debugprintf("votdc_shmbeginer: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (votdc_shmbeginer) */


static int votdc_shmend(VOTDC *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->mapdata != NULL) {
	    rs1 = votdc_mapend(op) ;
	    if (rs >= 0) rs = rs1 ;
	}
	if (op->fd >= 0) {
	    rs1 = u_close(op->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fd = -1 ;
	}
	return rs ;
}
/* end subroutine (votdc_shmend) */


static int votdc_mapbegin(VOTDC *op,time_t dt,int fd)
{
	size_t		ms ;
	int		rs ;
	int		mp ;
	int		mf ;
	void		*md ;

	if (fd < 0) return SR_INVALID ;

	if (dt == 0) dt = time(NULL) ;

	ms = op->shmsize ;
	mp = (PROT_READ | PROT_WRITE) ;
	mf = MAP_SHARED ;
	if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	    op->mapdata = md ;
	    op->mapsize = ms ;
	    op->ti_map = dt ;
	} /* end if (map) */

	return rs ;
}
/* end subroutine (votdc_mapbegin) */


static int votdc_mapend(VOTDC *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->mapdata != NULL) {
	    caddr_t	md = op->mapdata ;
	    size_t	ms = op->mapsize ;
	    rs1 = u_munmap(md,ms) ;
	    if (rs >= 0) rs = rs1 ;
	    op->mapdata = NULL ;
	    op->mapsize = 0 ;
	    op->mp = NULL ;
	    op->ti_map = 0 ;
	} /* end if (non-null) */

	return rs ;
}
/* end subroutine (votdc_mapend) */


static int votdc_shmprep(op,dt,fd,om,hdrp)
VOTDC		*op ;
time_t		dt ;
int		fd ;
mode_t		om ;
VOTDCHDR	*hdrp ;
{
	int		rs ;
	int		foff = 0 ;

	op->shmsize = 0 ;
	if (dt == 0) dt = time(NULL) ;

	hdrp->vetu[0] = VOTDCHDR_VERSION ;
	hdrp->vetu[1] = ENDIAN ;
	hdrp->vetu[2] = 0 ;
	hdrp->vetu[3] = 0 ;
	hdrp->wtime = (utime_t) dt ;

	rs = votdc_shmpreper(op,dt,fd,om,hdrp) ;
	foff = rs ;

	return (rs >= 0) ? foff : rs ;
}
/* end subroutine (votdc_shmprep) */


static int votdc_shmpreper(op,dt,fd,om,hdrp)
VOTDC		*op ;
time_t		dt ;
int		fd ;
mode_t		om ;
VOTDCHDR	*hdrp ;
{
	const int	hlen = HDRBUFLEN ;
	int		rs ;
	int		foff = 0 ;
	char		hbuf[HDRBUFLEN + 1] ;

	op->shmsize = 0 ;
	if (dt == 0) dt = time(NULL) ;

	if ((rs = votdchdr(hdrp,0,hbuf,hlen)) >= 0) {
	    int	hl = rs ;
	    if ((rs = votdc_shmwriter(op,dt,fd,hdrp,hbuf,hl)) >= 0) {
	        hdrp->shmsize = foff ;
	        if ((rs = u_rewind(fd)) >= 0) {
	            if ((rs = votdchdr(hdrp,0,hbuf,hlen)) >= 0) {
	                if ((rs = u_write(fd,hbuf,rs)) >= 0) {
	    		    op->shmsize = foff ;
	    		    rs = u_fchmod(fd,om) ;
		        }
		    } /* end if (votdchdr) */
		} /* end if (u_rewind) */
	    } /* end if (votdc_shmwriter) */
	} /* end if (votdchdr) */

	return (rs >= 0) ? foff : rs ;
}
/* end subroutine (votdc_shmpreper) */


static int votdc_shmwriter(op,dt,fd,hdrp,hbuf,hlen)
VOTDC		*op ;
time_t		dt ;
int		fd ;
VOTDCHDR	*hdrp ;
const char	hbuf[] ;
int		hlen ;
{
	FILEBUF		sfile, *sfp = &sfile ;
	const int	bsize = 2048 ;
	int		rs ;
	int		rs1 ;
	int		size ;
	int		foff = 0 ;

	op->shmsize = 0 ;
	if (dt == 0) dt = time(NULL) ;

	if ((rs = filebuf_start(sfp,fd,0,bsize,0)) >= 0) {
	    const int	asize = SHMALLOC_ALIGNSIZE ;

	    if (rs >= 0) {
	        rs = filebuf_write(sfp,hbuf,hlen) ;
	        foff += rs ;
	    }

	    if (rs >= 0) {
	        if ((rs = filebuf_writealign(sfp,asize)) >= 0) {
	            foff += rs ;
		    size = VOTDC_MUSIZE ;
		    hdrp->muoff = foff ;
		    hdrp->musize = size ;
		    rs = filebuf_writezero(sfp,size) ;
	            foff += rs ;
	        }
	    }

	    if (rs >= 0) {
	        if ((rs = filebuf_writealign(sfp,asize)) >= 0) {
	            foff += rs ;
		    size = VOTDC_BOOKSIZE ;
		    hdrp->bookoff = foff ;
		    hdrp->booklen = VOTDC_NBOOKS ;
		    rs = filebuf_writezero(sfp,size) ;
	            foff += rs ;
	        }
	    }

	    if (rs >= 0) {
	        if ((rs = filebuf_writealign(sfp,asize)) >= 0) {
	            foff += rs ;
		    size = VOTDC_VERSESIZE ;
		    hdrp->recoff = foff ;
		    hdrp->reclen = VOTDC_NVERSES ;
		    rs = filebuf_writezero(sfp,size) ;
	            foff += rs ;
	        }
	    }

	    if (rs >= 0) {
	        if ((rs = filebuf_writealign(sfp,asize)) >= 0) {
	            foff += rs ;
		    size = sizeof(SHMALLOC) ;
		    hdrp->balloff = foff ;
		    hdrp->ballsize = size ;
		    rs = filebuf_writezero(sfp,size) ;
	            foff += rs ;
	        }
	    }

	    if (rs >= 0) {
	        if ((rs = filebuf_writealign(sfp,asize)) >= 0) {
	            foff += rs ;
		    size = sizeof(SHMALLOC) ;
		    hdrp->valloff = foff ;
		    hdrp->vallsize = size ;
		    rs = filebuf_writezero(sfp,size) ;
	            foff += rs ;
	        }
	    }

	    if (rs >= 0) {
	        if ((rs = filebuf_writealign(sfp,asize)) >= 0) {
	            foff += rs ;
		    size = VOTDC_BSTRSIZE ;
		    hdrp->bstroff = foff ;
		    hdrp->bstrlen = size ;
		    rs = filebuf_writezero(sfp,size) ;
	            foff += rs ;
	        }
	    }

	    if (rs >= 0) {
	        if ((rs = filebuf_writealign(sfp,asize)) >= 0) {
	            foff += rs ;
		    size = VOTDC_VSTRSIZE ;
		    hdrp->vstroff = foff ;
		    hdrp->vstrlen = size ;
		    rs = filebuf_writezero(sfp,size) ;
	            foff += rs ;
	        }
	    }

	    if (rs >= 0) {
		size = iceil(foff,op->pagesize) - foff ;
		rs = filebuf_writezero(sfp,size) ;
	        foff += rs ;
	    }

	    rs1 = filebuf_finish(sfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */

	hdrp->shmsize = foff ;
	return (rs >= 0) ? foff : rs ;
}
/* end subroutine (votdc_shmwriter) */


static int votdc_shmhdrin(VOTDC *op,VOTDCHDR *hdrp)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("votdc_shmhdrin: ent\n") ;
#endif

	if ((rs = votdchdr(hdrp,1,op->mapdata,op->mapsize)) >= 0) {
	    if ((rs = votdc_verify(op)) >= 0) {
	        op->mp = (PTM *) (op->mapdata + hdrp->muoff) ;
	        op->books = (VOTDC_BOOK *) (op->mapdata + hdrp->bookoff) ;
	        op->verses = (VOTDC_VERSE *) (op->mapdata + hdrp->recoff) ;
	        op->ball = (SHMALLOC *) (op->mapdata + hdrp->balloff) ;
	        op->vall = (SHMALLOC *) (op->mapdata + hdrp->valloff) ;
	        op->bstr = (char *) (op->mapdata + hdrp->bstroff) ;
	        op->vstr = (char *) (op->mapdata + hdrp->vstroff) ;
		op->nents = (int) hdrp->reclen ;
	    }
	} /* end if (votdchdr) */

#if	CF_DEBUGS
	debugprintf("votdc_shmhdrin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (votdc_shmhdrin) */


static int votdc_allocinit(VOTDC *op,VOTDCHDR *hdrp)
{
	int		rs ;

	if ((rs = shmalloc_init(op->ball,op->bstr,hdrp->bstrlen)) >= 0) {
	    rs = shmalloc_init(op->vall,op->vstr,hdrp->vstrlen) ;
	    if (rs < 0)
	        shmalloc_fini(op->ball) ;
	} /* end if (SHMALLOC initialization) */

	return rs ;
}
/* end subroutine (votdc_allocinit) */


static int votdc_mutexinit(VOTDC *op)
{
	PTMA		ma ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("votdc_mutexinit: ent\n") ;
#endif

	if ((rs = ptma_create(&ma)) >= 0) {
	    const int	cmd = PTHREAD_PROCESS_SHARED ;
	    if ((rs = ptma_setpshared(&ma,cmd)) >= 0) {
	        PTM	*mp = (PTM *) op->mp ;

#ifdef	OPTIONAL
	        memset(mp,0,sizeof(PTM)) ;
#endif

	        rs = ptm_create(mp,&ma) ; /* we leave the MUTEX initialized */

	    } /* end if (ptma_setpshared) */
	    ptma_destroy(&ma) ;
	} /* end if (mutex-lock attribute) */

	return rs ;
}
/* end subroutine (votdc_mutexinit) */


static int votdc_shmchown(VOTDC *op)
{
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	char		*pwbuf ;
	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    int		nl ;
	    cchar	*np ;
	    if ((nl = sfrootname(op->pr,-1,&np)) > 0) {
	        const int	ulen = USERNAMELEN ;
	        char		ubuf[USERNAMELEN+1] ;
		strdcpy1w(ubuf,ulen,np,nl) ;
	        if ((rs = getpw_name(&pw,pwbuf,pwlen,ubuf)) >= 0) {
		    const uid_t	uid = pw.pw_uid ;
		    u_fchown(op->fd,uid,-1) ;
	        } else if (rs == SR_NOENT) {
		    rs = SR_OK ;
		}
	    } /* end if (sfrootname) */
	    uc_free(pwbuf) ;
	} /* end if (m-a-f) */
	return rs ;
}
/* end subroutine (votdc_shmchown) */


static int votdc_verify(VOTDC *op)
{
	VOTDCHDR	*hdrp = &op->hdr ;
	int		rs = SR_OK ;
	int		size ;
	int		f = TRUE ;

#if	CF_DEBUGS
	debugprintf("votdc_verify: ent shmsize=%u\n",op->shmsize) ;
#endif

	f = f && (hdrp->muoff > 0) ;
	f = f && (hdrp->muoff < op->shmsize) ;
	f = f && (hdrp->bookoff > 0) ;
	f = f && (hdrp->bookoff < op->shmsize) ;
	f = f && (hdrp->recoff > 0) ;
	f = f && (hdrp->recoff < op->shmsize) ;
	f = f && (hdrp->balloff > 0) ;
	f = f && (hdrp->balloff < op->shmsize) ;
	f = f && (hdrp->valloff > 0) ;
	f = f && (hdrp->valloff < op->shmsize) ;
	f = f && (hdrp->bstroff > 0) ;
	f = f && (hdrp->bstroff < op->shmsize) ;
	f = f && (hdrp->vstroff > 0) ;
	f = f && (hdrp->vstroff < op->shmsize) ;
	f = f && ((hdrp->muoff + hdrp->musize) < op->shmsize) ;
	size = sizeof(VOTDC_BOOK) ;
	f = f && ((hdrp->bookoff + (hdrp->booklen*size)) < op->shmsize) ;
	size = sizeof(VOTDC_VERSE) ;
	f = f && ((hdrp->recoff + (hdrp->reclen*size)) < op->shmsize) ;
	f = f && ((hdrp->balloff + hdrp->ballsize) < op->shmsize) ;
	f = f && ((hdrp->valloff + hdrp->vallsize) < op->shmsize) ;
	f = f && ((hdrp->bstroff + hdrp->bstrlen) < op->shmsize) ;
	f = f && ((hdrp->vstroff + hdrp->vstrlen) < op->shmsize) ;

	if (! f)
	    rs = SR_BADFMT ;

#if	CF_DEBUGS
	debugprintf("votdc_verify: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (votdc_verify) */


static int votdc_shmhandbegin(VOTDC *op,const char *pr)
{
	int		rs = SR_OK ;
	int		rl ;
	const char	*rn ;
	if ((rl = sfrootname(pr,-1,&rn)) > 0) {
	    const int	slen = MAXNAMELEN ;
	    const char	*suf = VOTDC_SHMPOSTFIX ;
	    char	sbuf[MAXNAMELEN+1] ;
	    if ((rs = mkshmname(sbuf,slen,rn,rl,suf)) >= 0) {
	        const char	*np ;
	        if ((rs = uc_mallocstrw(sbuf,rs,&np)) >= 0) {
	            op->shmname = np ;
	        }
	    } /* end if (mkshmname) */
	} else {
	    rs = SR_INVALID ;
	}
	return rs ;
}
/* end subroutine (votdc_shmhandbegin) */


static int votdc_shmhandend(VOTDC *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->shmname != NULL) {
	    rs1 = uc_free(op->shmname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->shmname = NULL ;
	}
	return rs ;
}
/* end subroutine (votdc_shmhandend) */


static int votdc_bookslotload(op,dt,si,lang,tv)
VOTDC		*op ;
time_t		dt ;
int		si ;
const char	*lang ;
const char	**tv ;
{
	int		rs ;

#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("votdc_bookslotload: ent\n") ;
	    debugprintf("votdc_bookslotload: titles¬\n") ;
	    for (i = 0 ; (i < VOTDC_NTITLES) && (tv[i] != NULL) ; i += 1) {
	        debugprintf("votdc_bookslotload: title[%02u]=>%s<\n",i,tv[i]) ;
	    }
	}
#endif /* CF_DEBUGS */
	if ((rs = votdc_getwcount(op)) >= 0) {
	    VOTDC_BOOK	*blp = (op->books + si) ;
	    SHMALLOC	*bap = op->ball ;
	    const int	wc = rs ;
#if	CF_DEBUGS
	    debugprintf("votdc_bookslotload: votdc_getwcount() wc=%u\n",wc) ;
#endif
	    rs = book_load(blp,bap,op->bstr,dt,wc,lang,tv) ;
	}

#if	CF_DEBUGS
	debugprintf("votdc_bookslotload: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (votdc_bookslotload) */


static int votdc_bookslotfind(VOTDC *op,const char *lang)
{
	VOTDC_BOOK	*blp = op->books ;
	int		rs = SR_OK ;
	int		i ;
	int		i_empty = -1 ;
	int		f_same = FALSE ;

#if	CF_DEBUGS
	debugprintf("votdc_bookslotfind: ent lang=%s\n",lang) ;
#endif
	for (i = 0 ; i < op->hdr.booklen ; i += 1) {
	    const char	*elang = blp[i].lang ;
	    if (elang[0] != '\0') {
		f_same = (strcmp(elang,lang) == 0) ;
	    } else {
		i_empty = i ;
	    }
	    if (f_same) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("votdc_bookslotfind: mid f_same=%u\n",f_same) ;
#endif
	if (! f_same) {
	    if (i_empty < 0) {
	        time_t	oldest = INT_MAX ;
	        int	oi = 0 ;
	        for (i = 0 ; i < op->hdr.booklen ; i += 1) {
	            if (blp[i].atime < oldest) {
		        oldest = op->books[i].atime ;
		        oi = i ;
		    }
	        } /* end for */
	        rs = votdc_bookslotdump(op,oi) ;
		i = oi ;
	    } else {
		i = i_empty ;
	    }
	} else {
	    rs = votdc_bookslotdump(op,i) ;
	}

#if	CF_DEBUGS
	debugprintf("votdc_bookslotfind: ret rs=%d i=%u\n",rs,i) ;
#endif
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (votdc_bookslotfind) */


static int votdc_bookslotdump(VOTDC *op,int ei)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("votdc_bookslotdump: ent ei=%u\n",ei) ;
#endif
	if ((rs = votdc_verselangdump(op,ei)) >= 0) {
	    VOTDC_BOOK	*blp = (op->books + ei) ;
	    SHMALLOC	*bap = op->ball ;
	    rs = book_dump(blp,bap) ;
	} /* end if (votdc_verselangdump) */
#if	CF_DEBUGS
	debugprintf("votdc_bookslotdump: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (votdc_bookslotdump) */


static int votdc_verselangdump(VOTDC *op,int li)
{
	VOTDC_VERSE	*vep = op->verses ;
	SHMALLOC	*vap = op->vall ;
	int		rs = SR_OK ;
	int		i ;
	for (i = 0 ; i < op->hdr.reclen ; i += 1) {
	    rs = verse_dump((vep+i),vap,li) ;
	    if (rs < 0) break ;
	} /* end for */
	return rs ;
}
/* end subroutine (votdc_verselangdump) */


static int votdc_booklanghave(VOTDC *op,const char *lang)
{
	VOTDC_BOOK	*bap = op->books ;
	int		rs = SR_NOTFOUND ;
	int		i ;
	for (i = 0 ; i < op->hdr.booklen ; i += 1) {
	    if (strcmp(bap[i].lang,lang) == 0) {
		rs = SR_OK ;
		break ;
	    }
	} /* end for */
#if	CF_DEBUGS
	debugprintf("votdc_booklanghave: ret rs=%d i=%u\n",rs,i) ;
#endif
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (votdc_booklanghave) */


static int votdc_versehave(VOTDC *op,int li,int mjd)
{
	VOTDC_VERSE	*vep = op->verses ;
	int		rs = SR_NOTFOUND ;
	int		i ;
#if	CF_DEBUGS
	debugprintf("votdc_versehave: ent li=%u mjd=%u\n",li,mjd) ;
#endif
	for (i = 0 ; i < op->hdr.reclen ; i += 1) {
	    rs = verse_match((vep+i),li,mjd) ;
#if	CF_DEBUGS
	    debugprintf("votdc_versehave: i=%u verse_match() rs=%d\n",i,rs) ;
#endif
	    if (rs != SR_NOTFOUND) break ;
	} /* end for */
#if	CF_DEBUGS
	if (rs >= 0) {
	debugprintf("votdc_versehave: found mjd=%u b=%u\n",
		(vep+i)->mjd, (vep+i)->book) ;
	}
	debugprintf("votdc_versehave: ret rs=%d ii=%u\n",rs,li) ;
#endif
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (votdc_versehave) */


static int votdc_verseslotnext(VOTDC *op,int vi)
{
	VOTDC_VERSE	*vep = op->verses ;
	int		rs = SR_NOTFOUND ;
	int		i ;
#if	CF_DEBUGS
	debugprintf("votdc_verseslotnext: ent vi=%u\n",vi) ;
#endif
	for (i = vi ; i < op->hdr.reclen ; i += 1) {
	    rs = verse_isused(vep+i) ;
	    if (rs != SR_NOTFOUND) break ;
	} /* end for */
#if	CF_DEBUGS
	debugprintf("votdc_verseslotnext: ret rs=%d i=%u\n",i) ;
#endif
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (votdc_verseslotnext) */


static int votdc_verseslotfind(VOTDC *op)
{
	VOTDC_VERSE	*vep = op->verses ;
	int		rs = SR_NOTFOUND ;
	int		i ;
	for (i = 0 ; i < op->hdr.reclen ; i += 1) {
	    rs = verse_isempty((vep+i)) ;
	    if (rs != SR_NOTFOUND) break ;
	} /* end for */
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (votdc_verseslotfind) */


static int votdc_verseslotfinder(VOTDC *op)
{
	VOTDC_VERSE	*vep = op->verses ;
	time_t		mt = INT_MAX ;
	int		rs = SR_OK ;
	int		mi = 0 ;
	int		i ;
	for (i = 0 ; i < op->hdr.reclen ; i += 1) {
	    if ((rs = verse_isleast((vep+i),&mt)) > 0) {
		mi = i ;
	    } /* end if (verse_isleast) */
	    if (rs < 0) break ;
	} /* end for */
#if	CF_DEBUGS
	debugprintf("votdc_verseslotfinder: ret rs=%d mi=%u\n",rs,mi) ;
#endif
	return (rs >= 0) ? mi : rs ;
}
/* end subroutine (votdc_verseslotfinder) */


static int votdc_getwcount(VOTDC *op)
{
	int		rs ;
	char		*mdp = (char *) op->mapdata ;
	int		*htab ;
	int		*wcp ;
	htab = (int *) (mdp+VOTDCHDR_IDLEN) ;
	wcp = (int *) (htab + votdchdrh_wcount) ;
	rs = *wcp ;
	return rs ;
}
/* end subroutine (votdc_getwcount) */


static int votdc_access(VOTDC *op)
{
	uint		*htab ;
	int		rs ;
	char		*mdp = (char *) op->mapdata ;
#if	CF_DEBUGS
	debugprintf("votdc_access: ent\n") ;
#endif
	htab = (uint *) (mdp+VOTDCHDR_IDLEN) ;
	{
	    int	*acp = (int *) (htab + votdchdrh_acount) ;
	    *acp += 1 ;
	    rs = *acp ;
	}
#if	CF_DEBUGS
	debugprintf("votdc_access: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (votdc_access) */


#if	CF_UPDATE
static int votdc_update(VOTDC *op)
{
	uint		*htab ;
	int		rs ;
	char		*mdp = (char *) op->mapdata ;
#if	CF_DEBUGS
	debugprintf("votdc_update: ent\n") ;
#endif
	htab = (uint *) (mdp + VOTDCHDR_IDLEN) ;
	{
	    int	*wcp = (int *) (htab + votdchdrh_wcount) ;
	    rs = *wcp ;
	    *wcp += 1 ;
	}
#if	CF_DEBUGS
	debugprintf("votdc_update: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (votdc_update) */
#endif /* CF_UPDATE */


#if	CF_GETACOUNT
static int votdc_getacount(VOTDC *op)
{
	uint		*htab ;
	int		rs ;
	int		*acp ;
	char		*mdp = (char *) op->mapdata ;
	htab = (uint *) (mdp+VOTDCHDR_IDLEN) ;
	acp = (int *) (htab + votdchdrh_acount) ;
	rs = *acp ;
	return rs ;
}
/* end subroutine (votdc_getacount) */
#endif /* CF_GETACOUNT */


static int votdc_mktitles(VOTDC *op,const char *lang,int bi)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;
	int		ci = 0 ;

	if ((rs = votdc_titlevalid(op,bi)) == rsn) {
	    if ((rs = votdc_titletouse(op)) >= 0) {
	        VOTDC_TC	*tcp = (op->tcs+rs) ;
		VOTDC_BOOK	*bep = (op->books+bi) ;
		ci = rs ;
		if ((rs = book_getwmark(bep)) >= 0) {
		    const int	wm = rs ;
		    char	*bstr = op->bstr ;
		    rs = titlecache_load(tcp,wm,lang,bstr,bep->b) ;
		} /* end if (book_getwmark) */
	    } /* end if (votdc_titletouse) */
	} else {
	    ci = rs ;
	}
	return (rs >= 0) ? ci : rs ;
}
/* end subroutine (votdc_mktitles) */


static int votdc_titlematcher(VOTDC *op,int ac,int ci,const char *cbuf,int cl)
{
	VOTDC_TC	*tcp = (op->tcs+ci) ;
	int		rs = SR_NOTFOUND ;
	if (tcp->titles != NULL) {
	    tcp->amark = ac ;
	    if ((rs = matostr(tcp->titles,2,cbuf,cl)) < 0) {
		rs = SR_NOTFOUND ;
	    }
	} /* end if (non-null) */
	return rs ;
}
/* end subroutine (votdc_titlematcher) */


static int votdc_titletouse(VOTDC *op)
{
	VOTDC_TC	*tcp = op->tcs ;
	int		rs = SR_OK ;
	int		ci = -1 ;
	int		i ;
	for (i = 0 ; i < VOTDC_NTITLES ; i += 1) {
	    if (tcp[i].lang[0] == '\0') {
		ci = i ;
		break ;
	    }
	} /* end for */
	if (ci < 0) {
	    int		minamark = INT_MAX ;
	    for (i = 0 ; i < VOTDC_NTITLES ; i += 1) {
	        if ((tcp[i].lang[0] != '\0') && (tcp[i].amark < minamark)) {
		    ci = i ;
		    minamark = tcp[i].amark ;
	        }
	    } /* end for */
	} /* end if (find oldest) */
	return (rs >= 0) ? ci : rs ;
}
/* end subroutine (votdc_titletouse) */


static int votdc_titlevalid(VOTDC *op,int bi)
{
	VOTDC_BOOK	*bep = (op->books+bi) ;
	int		rs ;
	int		i = 0 ;
	const char	*blang ;
	if ((rs = book_getwmarklang(bep,&blang)) >= 0) {
	    VOTDC_TC	*tcp = op->tcs ;
	    const int	wm = rs ;
	    const int	n = VOTDC_NBOOKS ;
	    int		f = FALSE ;
	    for (i = 0 ; i < n ; i += 1) {
		if ((tcp[i].lang[0] != '\0') && (tcp[i].wmark == wm)) {
		    f = (strcmp(tcp[i].lang,blang) == 0) ;
		    if (f) break ;
		}
	    } /* end for */
	    if (!f) rs = SR_NOTFOUND ;
	} /* end if (votdc_getwcount) */
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (votdc_titlevalid) */


static int votdc_titlefins(VOTDC *op)
{
	const int	n = VOTDC_NBOOKS ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	for (i = 0 ; i < n ; i += 1) {
	    rs1 = titlecache_release((op->tcs+i)) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end for */
	return rs ;
}
/* end subroutine (votdc_titlefins) */


static int verse_dump(VOTDC_VERSE *vep,SHMALLOC *vap,int li)
{
	int		rs = SR_OK ;
	li &= UCHAR_MAX ;
	if ((vep->lang == li) && (vep->vlen > 0)) {
	    vep->mjd = 0 ;
	    if ((rs = shmalloc_free(vap,vep->voff)) >= 0) {
	        vep->voff = 0 ;
	        vep->vlen = 0 ;
	        vep->ctime = 0 ;
	        vep->atime = 0 ;
	        rs = 1 ;
	    }
	}
	return rs ;
}
/* end subroutine (verse_dump) */


static int verse_match(VOTDC_VERSE *vep,int li,int mjd)
{
	int	rs = SR_NOTFOUND ;
#if	CF_DEBUGS
	debugprintf("verse_match: ent li=%u mjd=%u\n",li,mjd) ;
#endif
#if	CF_DEBUGS
	debugprintf("verse_match: slot li=%u mjd=%u\n",vep->lang,vep->mjd) ;
#endif
	if ((vep->lang == li) && (vep->mjd == mjd)) {
	    rs = SR_OK ;
	}
#if	CF_DEBUGS
	debugprintf("verse_match: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (verse_match) */


static int verse_read(vep,vstr,citep,rbuf,rlen)
VOTDC_VERSE	*vep ;
char		*vstr ;
VOTDC_CITE	*citep ;
char		*rbuf ;
int		rlen ;
{
	int		rs ;
	citep->b = vep->book ;
	citep->c = vep->chapter ;
	citep->v = vep->verse ;
	citep->l = vep->lang ;
	rs = sncpy1(rbuf,rlen,(vstr+vep->voff)) ;
	return rs ;
}
/* end subroutine (verse_read) */


static int verse_accessed(VOTDC_VERSE *vep,time_t dt)
{
	int		rs = SR_OK ;
	vep->atime = dt ;
	return rs ;
}
/* end subroutine (verse_accessed) */


static int verse_load(vep,dt,wm,vap,vstr,citep,mjd,vp,vl)
VOTDC_VERSE	*vep ;
SHMALLOC	*vap ;
time_t		dt ;
int		wm ;
char		*vstr ;
VOTDC_CITE	*citep ;
int		mjd ;
const char	*vp ;
int		vl ;
{
	int		rs ;
	int		voff = 0 ;
	if (vl < 0) vl = strlen(vp) ;
	if ((rs = shmalloc_alloc(vap,(vl+1))) >= 0) {
	    char	*bp = (vstr+rs) ;
	    voff = rs ;
 	    vep->ctime = dt ;
	    vep->atime = dt ;
	    vep->voff = voff ;
	    vep->vlen = vl ;
	    vep->wmark = wm ;
	    vep->amark = 0 ;
	    vep->book  = citep->b ;
	    vep->chapter = citep->c ;
	    vep->verse = citep->v ;
	    vep->lang = citep->l ;
	    vep->mjd = mjd ;
	    strwcpy(bp,vp,vl) ;
	} /* end if (m-a) */
	return (rs >= 0) ? voff : rs ;
}
/* end subroutine (verse_load) */


static int verse_isempty(VOTDC_VERSE *vep)
{
	int		rs = SR_OK ;
	if (vep->mjd > 0) rs = SR_NOTFOUND ;
	return rs ;
}
/* end subroutine (verse_isempty) */


static int verse_isused(VOTDC_VERSE *vep)
{
	int		rs = SR_OK ;
	if (vep->mjd == 0) rs = SR_NOTFOUND ;
	return rs ;
}
/* end subroutine (verse_isused) */


static int verse_isleast(VOTDC_VERSE *vep,time_t *tp)
{
	int		rs = 0 ;
	if ((vep->mjd == 0) || (vep->atime < *tp)) {
	    rs = 1 ;
	    *tp = vep->atime ;
	}
	return rs ;
}
/* end subroutine (verse_isleast) */


static int book_load(blp,bap,bsp,dt,wm,lang,tv)
VOTDC_BOOK	*blp ;
SHMALLOC	*bap ;
char		*bsp ;
time_t		dt ;
int		wm ;
const char	*lang ;
const char	**tv ;
{
	int		rs = SR_OK ;
	int		i ;
	char		*bp ;

#if	CF_DEBUGS
	debugprintf("votdc/book_load: ent\n") ;
#endif
	strwcpy(blp->lang,lang,VOTDC_LANGLEN) ;
	blp->ctime = dt ;
	blp->atime = dt ;
	blp->wmark = wm ;
	for (i = 0 ; (i < VOTDC_NTITLES) && (tv[i] != NULL) ; i += 1) {
	    int	bl = strlen(tv[i]) ;
#if	CF_DEBUGS
	    debugprintf("votdc/book_load: title=>%s<\n",tv[i]) ;
#endif
	    if ((rs = shmalloc_alloc(bap,(bl+1))) >= 0) {
#if	CF_DEBUGS
	    debugprintf("votdc/book_load: shmalloc_alloc() rs=%d\n",rs) ;
#endif
		blp->b[i] = rs ;
		bp = (bsp+rs) ;
		strwcpy(bp,tv[i],bl) ;
	    } /* end if (shmalloc_alloc) */
#if	CF_DEBUGS
	    debugprintf("votdc/book_load: for-bot rs=%d\n",rs) ;
#endif
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("votdc/book_load: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (book_load) */


static int book_dump(VOTDC_BOOK *blp,SHMALLOC *bap)
{
	int		rs = SR_OK ;
	int		i ;

	blp->ctime = 0 ;
	blp->atime = 0 ;
	blp->lang[0] = '\0' ;
	for (i = 0 ; i < VOTDC_NTITLES ; i += 1) {
	    int	toff = blp->b[i] ;
	    if (toff > 0) {
	        rs = shmalloc_free(bap,toff) ;
	    }
	    if (rs < 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (book_dump) */


static int book_getwmark(VOTDC_BOOK *bep)
{
	int		rs = SR_NOTFOUND ;
	if (bep->lang[0] != '\0') {
	    rs = bep->wmark ;
	}
	return rs ;
}
/* end subroutine (book_getwmark) */


static int book_getwmarklang(VOTDC_BOOK *bep,const char **lpp)
{
	int		rs = SR_NOTFOUND ;
	if (bep->lang[0] != '\0') {
	    if (lpp != NULL) *lpp = bep->lang ;
	    rs = bep->wmark ;
	}
	return rs ;
}
/* end subroutine (book_getwmarklang) */


static int book_read(bep,bstr,ac,rbuf,rlen,ti)
VOTDC_BOOK	*bep ;
char		*bstr ;
int		ac ;
char		rbuf[] ;
int		rlen ;
int		ti ;
{
	const int	boff = bep->b[ti] ;
	int		rs ;
	bep->amark = ac ;
	rs = sncpy1(rbuf,rlen,(bstr+boff)) ;
	return rs ;
}
/* end subroutine (book_read) */


static int titlecache_load(tcp,wm,lang,bstr,boffs) 
VOTDC_TC	*tcp ;
int		wm ;
const char	*lang ;
char		*bstr ;
int		*boffs ;
{
	const int	n = VOTDC_NTITLES ;
	int		rs ;
	int		size = sizeof(const char *) ;
	int		i ;
	const char	*np ;
	char		*bp ;
	memset(tcp,0,sizeof(VOTDC_TC)) ;
	tcp->wmark = wm ;
	tcp->amark = 0 ;
	strwcpy(tcp->lang,lang,VOTDC_LANGLEN) ;
	for (i = 0 ; i < n ; i += 1) {
	    np = (bstr + boffs[i]) ;
	    size += sizeof(const char *) ;
	    size += (strlen(np)+1) ;
	} /* end for */
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    tcp->a = (const char *) bp ;
	    tcp->titles = (const char **) bp ;
	    bp += ((n+1)*sizeof(const char *)) ;
	    for (i = 0 ; i < n ; i += 1) {
		np = (bstr + boffs[i]) ;
		tcp->titles[i] = np ;
	    } /* end for */
	    tcp->titles[i] = NULL ;
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (titlecache_load) */


static int titlecache_release(VOTDC_TC *tcp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (tcp->a != NULL) {
	    rs1 = uc_free(tcp->a) ;
	    if (rs >= 0) rs = rs1 ;
	    tcp->a = NULL ;
	    tcp->titles = NULL ;
	}
	tcp->lang[0] = '\0' ;
	tcp->wmark = 0 ;
	tcp->amark = 0 ;
	return rs ;
}
/* end subroutine (titlecache_release) */


static int mkshmname(char *rbuf,int rlen,const char *rn,int rl,const char *suf)
{
	int		rs = SR_OK ;
	int		i = 0 ;
	if ((rl != 0) && (rn[0] != '/')) {
	    rs = storebuf_char(rbuf,rlen,i,'/') ;
	    i += 1 ;
	}
	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,rn,rl) ;
	    i += rs ;
	}
	if (rs >= 0) {
	    rs = storebuf_char(rbuf,rlen,i,'$') ;
	    i += rs ;
	}
	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,suf,-1) ;
	    i += rs ;
	}
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkshmname) */


