/* txtindexmks */

/* make a TXTINDEX database */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGEIGEN	0		/* debug the EIGEN stuff */
#define	CF_DEBUGCECK	0		/* debug "checkXXX" */
#define	CF_LATE		0		/* make new index file late */
#define	CF_SETINT	1		/* use |osetint(3dam)| */


/* revision history:

	= 2008-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates a TXTINDEX database (currently consisting of
	two files).

	Synopsis:

	int txtindexmks_open(op,pp,dbname,of,om)
	TXTINDEXMKS	*op ;
	TXTINDEXMKS_PA	*pp ;
	const char	dbname[] ;
	int		of ;
	mode_t		om ;

	Arguments:

	- op		object pointer
	- pp		pointer to parameters
	- dbname	name of (path-to) DB
	- of		open-flags
	- om		open-permissions

	Returns:

	>=0		OK
	<0		error code


	Notes:

	= possible returns to an open attempt

	- OK (creating)
	- already exists
	- doesn't exist but is in progress
	- exists and is in progress

	= open-flags

			if DB exits	if NDB exists	returns
	___________________________________________________________________

	-		no		no		SR_OK (created)
	-		no		yes		SR_INPROGRESS
	-		yes		no		SR_OK
	-		yes		yes		SR_INPROGRESS

	O_CREAT|O_EXCL	no		no		SR_OK (created)
	O_CREAT|O_EXCL	no		yes		SR_INPROGRESS
	O_CREAT|O_EXCL	yes		no		SR_INPROGRESS
	O_CREAT|O_EXCL	yes		yes		SR_INPROGRESS

	O_CREAT		x		x		SR_OK (created)


*******************************************************************************/


#define	TXTINDEXMKS_MASTER	0


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endian.h>
#include	<endianstr.h>
#include	<estrings.h>
#include	<vecint.h>
#include	<osetint.h>
#include	<strtab.h>
#include	<bfile.h>
#include	<filebuf.h>
#include	<char.h>
#include	<localmisc.h>

#include	"txtindexmks.h"
#include	"txtindexhdr.h"
#include	"naturalwords.h"


/* local defines */

#define	TXTINDEXMKS_TABLEN	(16 * 1024)
#define	TXTINDEXMKS_EIGENSIZE	(2 * 1024)	/* usually sufficient! */
#define	TXTINDEXMKS_LISTLEN	6		/* default */
#define	TXTINDEXMKS_NSKIP	7		/* reasonable value */

#ifndef	KEYBUFLEN
#ifdef	NATURALWORDLEN
#define	KEYBUFLEN	NATURALWORDLEN
#else
#define	KEYBUFLEN	80
#endif
#endif

#define	HDRBUFLEN	(sizeof(TXTINDEXHDR) + 128)
#define	BUFLEN		(sizeof(TXTINDEXHDR) + 128)
#define	SUFBUFLEN	32
#define	TMPFNAMELEN	14

#define	FSUF_IND	"hash"
#define	FSUF_TAG	"tag"

#define	TO_OLDFILE	(5 * 60)

#define	MODP2(v,n)	((v) & ((n) - 1))

#if	CF_SETINT
#define	LISTOBJ				osetint
#define	LISTOBJ_OORDERED 		0
#define	LISTOBJ_CUR			osetint_cur
#define	LISTOBJ_START(op,n,opts)	osetint_start((op))
#define	LISTOBJ_FINISH(op)		osetint_finish((op))
#define	LISTOBJ_ADD(op,v)		osetint_addval((op),(v))
#define	LISTOBJ_COUNT(op)		osetint_count((op))
#define	LISTOBJ_MKVEC(op,va)		osetint_mkvec((op),(va))
#define	LISTOBJ_CURBEGIN(op,cp)		osetint_curbegin((op),(cp))
#define	LISTOBJ_ENUM(op,cp,rp)		osetint_enum((op),(cp),(rp))
#define	LISTOBJ_CUREND(op,cp)		osetint_curend((op),(cp))
#else /* CF_SETINT */
#define	LISTOBJ				vecint
#define	LISTOBJ_OORDERED		VECINT_OORDERED
#define	LISTOBJ_CUR			vecint_cur
#define	LISTOBJ_START(op,n,opts)	vecint_start((op),(n),(opts))
#define	LISTOBJ_FINISH(op)		vecint_finish((op))
#define	LISTOBJ_ADD(op,v)		vecint_adduniq((op),(v))
#define	LISTOBJ_COUNT(op)		vecint_count((op))
#define	LISTOBJ_MKVEC(op,va)		vecint_mkvec((op),(va))
#define	LISTOBJ_CURBEGIN(op,cp)		vecint_curbegin((op),(cp))
#define	LISTOBJ_ENUM(op,cp,rp)		vecint_enum((op),(cp),(rp))
#define	LISTOBJ_CUREND(op,cp)		vecint_curend((op),(cp))
#endif /* CF_SETINT */


/* external subroutines */

extern uint	hashelf(const char *,int) ;
extern uint	hashagain(uint,int,int) ;

extern int	mkfnamesuf1(char *,cchar *,cchar *) ;
extern int	mkfnamesuf2(char *,cchar *,cchar *,cchar *) ;
extern int	mkfnamesuf3(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getpwd(char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	mktmpfile(char *,mode_t,cchar *) ;
extern int	msleep(int) ;
extern int	filebuf_writefill(FILEBUF *,const char *,int) ;
extern int	hasuc(const char *,int) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strwset(char *,int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

#if	CF_DEBUGS && CF_DEBUGEIGEN
extern int	strtabfind(const char *,int (*)[3],int,int,const char *,int) ;
#endif


/* external variables */


/* exported variables */

TXTINDEXMKS_OBJ	txtindexmks = {
	"txtindexmks",
	sizeof(TXTINDEXMKS)
} ;


/* local structures */


/* forward references */

static int	txtindexmks_checkparams(TXTINDEXMKS *) ;
static int	txtindexmks_idxdirbegin(TXTINDEXMKS *) ;
static int	txtindexmks_idxdirend(TXTINDEXMKS *) ;
static int	txtindexmks_filesbegin(TXTINDEXMKS *) ;
static int	txtindexmks_filesbeginc(TXTINDEXMKS *) ;
static int	txtindexmks_filesbeginwait(TXTINDEXMKS *) ;
static int	txtindexmks_filesbeginopen(TXTINDEXMKS *,cchar *) ;
static int	txtindexmks_filesend(TXTINDEXMKS *) ;
static int	txtindexmks_listbegin(TXTINDEXMKS *) ;
static int	txtindexmks_listend(TXTINDEXMKS *) ;
static int	txtindexmks_addtag(TXTINDEXMKS *,TXTINDEXMKS_TAG *) ;
static int	txtindexmks_mkhash(TXTINDEXMKS *) ;
static int	txtindexmks_mkhashwrmain(TXTINDEXMKS *,TXTINDEXHDR *) ;
static int	txtindexmks_mkhashwrhdr(TXTINDEXMKS *,TXTINDEXHDR *,
			FILEBUF *,int) ;
static int	txtindexmks_mkhashwrtab(TXTINDEXMKS *,TXTINDEXHDR *,
			FILEBUF *,int) ;
static int	txtindexmks_mkhashwreigen(TXTINDEXMKS *,TXTINDEXHDR *,
			FILEBUF *,int) ;
static int	txtindexmks_mkhashwrtabone(TXTINDEXMKS *,TXTINDEXHDR *,
			FILEBUF *,int,int *,int) ;
static int	txtindexmks_nhashopen(TXTINDEXMKS *) ;
static int	txtindexmks_nhashclose(TXTINDEXMKS *) ;
static int	txtindexmks_ntagclose(TXTINDEXMKS *) ;
static int	txtindexmks_renamefiles(TXTINDEXMKS *) ;

#if	CF_DEBUGS && CF_DEBUGEIGEN
struct printeigen {
	int		*ertab ;
	char		*estab ;
	int		ersize ;
	int		erlen ;
} ;
static int	txtindexmks_printeigen(TXTINDEXMKS *) ;
static int	txtindexmks_printeigener(TXTINDEXMKS *,struct printeigen *) ;
#endif

static int	mknewfname(char *,int,cchar *,cchar *) ;
static int	unlinkstale(cchar *,int) ;

#if	CF_DEBUGS && CF_DEBUGCHECK
static int	checksize(const char *,int,int) ;
static int	checkalign(const char *,uint) ;
#endif


/* local variables */

static const char	zerobuf[4] = {
	0, 0, 0, 0 
} ;


/* exported subroutines */


int txtindexmks_open(TXTINDEXMKS *op,TXTINDEXMKS_PA *pp,cchar db[],
		int of,mode_t om)
{
	int		rs ;
	int		f = FALSE ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	{
		char	obuf[100+1] ;
	debugprintf("txtindexmks_open: ent db=%s\n",db) ;
		snopenflags(obuf,-1,of) ;
	debugprintf("txtindexmks_open: of=%s\n",obuf) ;
	}
#endif /* CF_DEBUGS */

	if (pp == NULL) return SR_FAULT ;
	if (db == NULL) return SR_FAULT ;

	if (db[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(TXTINDEXMKS)) ;
	op->om = (om|0600) ;
	op->nfd = -1 ;
	op->pi = *pp ;			/* copy the given parameters */

	op->f.ofcreat = MKBOOL(of & O_CREAT) ;
	op->f.ofexcl = MKBOOL(of & O_EXCL) ;

	if ((rs = uc_mallocstrw(db,-1,&cp)) >= 0) {
	    op->dbname = cp ;
	    if ((rs = txtindexmks_checkparams(op)) >= 0) {
	        if ((rs = txtindexmks_idxdirbegin(op)) >= 0) {
	            if ((rs = txtindexmks_filesbegin(op)) >= 0) {
	                f = rs ;
	                if ((rs = txtindexmks_listbegin(op)) >= 0) {
	                    op->ti.maxtags = 0 ;
	                    op->magic = TXTINDEXMKS_MAGIC ;
	                }
	                if (rs < 0)
	                    txtindexmks_filesend(op) ;
	            } /* end if (files-begin) */
	            if (rs < 0)
	                txtindexmks_idxdirend(op) ;
	        } /* end if (txtindexmks_idxdirbegin) */
	    } /* end if (check-params) */
	    if (rs < 0) {
	        if (op->dbname != NULL) {
	            uc_free(op->dbname) ;
	            op->dbname = NULL ;
	        }
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("txtindexmks_open: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (txtindexmks_open) */


int txtindexmks_close(TXTINDEXMKS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		clists = 0 ;
	int		f_go  ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TXTINDEXMKS_MAGIC) return SR_NOTOPEN ;

	f_go = (! op->f.abort) ;
	rs1 = txtindexmks_ntagclose(op) ;
	if (rs >= 0) rs = rs1 ;
	if (f_go) f_go = (rs1 >= 0) ;

	if ((op->ti.nkeys > 0) && f_go) {
	    rs1 = txtindexmks_mkhash(op) ;
	    if (rs >= 0) rs = rs1 ;
	    f_go = (rs1 >= 0) ;
	    clists = op->clists ;
	}

	rs1 = txtindexmks_listend(op) ;
	if (rs >= 0) rs = rs1 ;
	if (f_go) f_go = (rs1 >= 0) ;

	if ((rs >= 0) && (op->ti.nkeys > 0) && f_go) {
	    rs1 = txtindexmks_renamefiles(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = txtindexmks_filesend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = txtindexmks_idxdirend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->dbname != NULL) {
	    rs1 = uc_free(op->dbname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbname = NULL ;
	}

	op->magic = 0 ;
	return (rs >= 0) ? clists : rs ;
}
/* end subroutine (txtindexmks_close) */


int txtindexmks_noop(TXTINDEXMKS *op)
{
	if (op == NULL) return SR_FAULT ;
	if (op->magic != TXTINDEXMKS_MAGIC) return SR_NOTOPEN ;
	return SR_OK ;
}
/* end subroutine (txtindexmks_noop) */


int txtindexmks_abort(TXTINDEXMKS *op)
{
	if (op == NULL) return SR_FAULT ;
	if (op->magic != TXTINDEXMKS_MAGIC) return SR_NOTOPEN ;
	op->f.abort = TRUE ;
	return SR_OK ;
}
/* end subroutine (txtindexmks_abort) */


int txtindexmks_addeigens(TXTINDEXMKS *op,TXTINDEXMKS_KEY eigens[],int n)
{
	int		rs = SR_OK ;
	int		i ;
	int		kbl = KEYBUFLEN ;
	int		kl ;
	const char	*kp ;
	char		keybuf[KEYBUFLEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (eigens == NULL) return SR_FAULT ;

	if (op->magic != TXTINDEXMKS_MAGIC) return SR_NOTOPEN ;

	if (n < 0) return SR_INVALID ;

	for (i = 0 ; i < n ; i += 1) {

	    kp = (const char *) eigens[i].kp ;
	    kl = eigens[i].kl ;
	    if (kl < 0) kl = strlen(kp) ;

	    if (hasuc(kp,kl)) {
	        if (kl > kbl) kl = kbl ;
	        strwcpylc(keybuf,kp,kl) ;
	        kp = keybuf ;
	    }

	    rs = strtab_add(&op->eigens,kp,kl) ;

	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("txtindexmks_addeigens: ret rs=%d neigens=%u\n",rs,n) ;
#endif

	return rs ;
}
/* end subroutine (txtindexmks_addeigens) */


int txtindexmks_addtags(TXTINDEXMKS *op,TXTINDEXMKS_TAG tags[],int ntags)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (tags == NULL) return SR_FAULT ;

	if (op->magic != TXTINDEXMKS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("txtindexmks_addtags: tags(%p) ntags=%u\n",tags,ntags) ;
#endif

	for (i = 0 ; i < ntags ; i += 1) {

	    rs = txtindexmks_addtag(op,(tags + i)) ;
	    c += rs ;

#if	CF_DEBUGS
	    debugprintf("txtindexmks_addtags: _addtag() rs=%d\n",rs) ;
#endif

	    if (rs < 0) break ;
	} /* end for */

	if (rs >= 0)
	    op->ti.ntags += ntags ;

#if	CF_DEBUGS
	debugprintf("txtindexmks_addtags: ret rs=%d c=%u total_ntags=%u\n",
	    rs,c,op->ti.ntags) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (txtindexmks_addtags) */


/* private subroutines */


static int txtindexmks_checkparams(TXTINDEXMKS *op)
{
	TXTINDEXMKS_PA	*pp = &op->pi ;
	int		rs = SR_OK ;

	if (pp->tablen == 0)
	    pp->tablen = TXTINDEXMKS_TABLEN ;

	if (pp->minwlen == 0)
	    pp->minwlen = TXTINDEXMKS_MINWLEN ;

	if (pp->maxwlen == 0)
	    pp->maxwlen = TXTINDEXMKS_MAXWLEN ;

	if (pp->maxwlen > KEYBUFLEN)
	    pp->maxwlen = KEYBUFLEN ;

	return rs ;
}
/* end subroutine (txtindexmks_checkparams) */


static int txtindexmks_filesbegin(TXTINDEXMKS *op)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (op->f.ofcreat) {
	    rs = txtindexmks_filesbeginc(op) ;
	} else {
	    rs = txtindexmks_filesbeginwait(op) ;
	    c = rs ;
	}
#if	CF_DEBUGS
	debugprintf("txtindexmks_filesbegin: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (txtindexmks_filesbegin) */


static int txtindexmks_filesbeginc(TXTINDEXMKS *op)
{
	const int	type = (op->f.ofcreat && (! op->f.ofexcl)) ;
	int		rs ;
	cchar		*dbn = op->dbname ;
	cchar		*suf = FSUF_TAG	 ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mknewfname(tbuf,type,dbn,suf)) >= 0) {
	    const mode_t	om = op->om ;
	    cchar		*tfn = tbuf ;
	    char		rbuf[MAXPATHLEN+1] ;
	    if (type) {
	        if ((rs = mktmpfile(rbuf,om,tbuf)) >= 0) {
	    	    op->f.created = TRUE ;
	            tfn = rbuf ;
		}
	    }
	    if (rs >= 0) {
	        bfile	*tfp = &op->tagfile ;
	        char	ostr[8] = "wc" ;
	        if (op->f.ofexcl) strcat(ostr,"e") ;
	        if ((rs = bopen(tfp,tfn,ostr,om)) >= 0) {
	            cchar	*cp ;
	    	    op->f.created = TRUE ;
	            if ((rs = uc_mallocstrw(tfn,-1,&cp)) >= 0) {
	                op->ntagfname = (char *) cp ;
	            }
	            if (rs < 0)
	                bclose(tfp) ;
	        } /* end if (bopen) */
		if ((rs < 0) && type) {
		    uc_unlink(rbuf) ;
		}
	    } /* end if (ok) */
	} /* end if (mknewfname) */
	return rs ;
}
/* end subroutine (txtindexmks_filesbeginc) */


static int txtindexmks_filesbeginwait(TXTINDEXMKS *op)
{
	int		rs ;
	int		c = 0 ;
	cchar		*dbn = op->dbname ;
	cchar		*suf = FSUF_TAG	 ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mknewfname(tbuf,FALSE,dbn,suf)) >= 0) {
		const int	to_stale = TXTINDEXMKS_INTSTALE ;
		const int	nrs = SR_EXISTS ;
		int		to = TXTINDEXMKS_INTOPEN ;
		while ((rs = txtindexmks_filesbeginopen(op,tbuf)) == nrs) {
		    c = 1 ;
		    sleep(1) ;
		    unlinkstale(tbuf,to_stale) ;
		    if (to-- == 0) break ;
		} /* end while (db exists) */
		if (rs == nrs) {
		    op->f.ofcreat = FALSE ;
		    c = 0 ;
		    rs = txtindexmks_filesbeginc(op) ;
		}
	} /* end if (mknewfname) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (txtindexmks_filesbeginwait) */


static int txtindexmks_filesbeginopen(TXTINDEXMKS *op,cchar *tfn)
{
	bfile		*tfp = &op->tagfile ;
	const mode_t	om = op->om ;
	int		rs ;
	char		ostr[8] = "wce" ;
	if ((rs = bopen(tfp,tfn,ostr,om)) >= 0) {
	    cchar	*cp ;
	    op->f.created = TRUE ;
	    if ((rs = uc_mallocstrw(tfn,-1,&cp)) >= 0) {
		op->ntagfname = (char *) cp ;
	    }
	    if (rs < 0)
		bclose(tfp) ;
	} /* end if (bopen) */
	return rs ;
}
/* end subroutine (txtindexmks_filesbeginopen) */


static int txtindexmks_filesend(TXTINDEXMKS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f.tagopen) {
	    op->f.tagopen = FALSE ;
	    rs1 = bclose(&op->tagfile) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->ntagfname != NULL) {
	    if (op->f.created && (op->ntagfname[0] != '\0')) {
	        u_unlink(op->ntagfname) ;
	    }
	    rs1 = uc_free(op->ntagfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->ntagfname = NULL ;
	}

	if (op->nidxfname != NULL) {
	    if (op->f.created && (op->nidxfname[0] != '\0')) {
	        u_unlink(op->nidxfname) ;
	    }
	    rs1 = uc_free(op->nidxfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nidxfname = NULL ;
	}

	return rs ;
}
/* end subroutine (txtindexmks_filesend) */


static int txtindexmks_idxdirbegin(TXTINDEXMKS *op)
{
	int		rs = SR_INVALID ;
	int		dnl ;
	const char	*dnp ;
#if	CF_DEBUGS
	debugprintf("txtindexmks_idxdirbegin: ent db=%s\n",op->dbname) ;
#endif
	if ((dnl = sfdirname(op->dbname,-1,&dnp)) >= 0) {
	    char	tbuf[MAXPATHLEN + 1] ;
#if	CF_DEBUGS
	    debugprintf("txtindexmks_idxdirbegin: dnl=%d\n",dnl) ;
#endif
	    if (dnl == 0) {
	        rs = getpwd(tbuf,MAXPATHLEN) ;
	        dnl = rs ;
	    } else {
	        rs = mkpath1w(tbuf,dnp,dnl) ;
	    }
	    if (rs >= 0) {
	        int	am = (X_OK | W_OK) ;
	        if ((rs = perm(tbuf,-1,-1,NULL,am)) >= 0) {
	            const char	*cp ;
	            if ((rs = uc_mallocstrw(tbuf,dnl,&cp)) >= 0) {
	                op->idname = cp ;
	            }
	        }
#if	CF_DEBUGS
		debugprintf("txtindexmks_idxdirbegin: perm() rs=%d\n",rs) ;
#endif
	    }
	}
#if	CF_DEBUGS
	debugprintf("txtindexmks_idxdirbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (txtindexmks_idxdirbegin) */


static int txtindexmks_idxdirend(TXTINDEXMKS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->idname != NULL) {
	    rs1 = uc_free(op->idname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->idname = NULL ;
	}
	return rs ;
}
/* end subroutine (txtindexmks_idxdirend) */


static int txtindexmks_listbegin(TXTINDEXMKS *op)
{
	int		rs ;
	int		size = (op->pi.tablen * sizeof(LISTOBJ)) ;
	void		*p ;

	if ((rs = uc_malloc(size,&p)) >= 0) {
	    LISTOBJ	*lop = (LISTOBJ *) p ;
	    const int	lo = LISTOBJ_OORDERED ;
	    int		n = 0 ;
	    int		i ;
	    op->lists = p ;

#if	CF_DEBUGS 
	    debugprintf("txtindexmks_listbegin: tablen=%d\n", op->pi.tablen) ;
#endif

	    if ((lo >= 0) && (n >= 0)) {
	        for (i = 0 ; (rs >= 0) && (i < op->pi.tablen) ; i += 1) {
	            rs = LISTOBJ_START((lop+i),n,lo) ;
	        }
	    } else
	        rs = SR_BUGCHECK ;

	    if (rs >= 0) {
	        size = TXTINDEXMKS_EIGENSIZE ;
	        rs = strtab_start(&op->eigens,size) ;
	    }

	    if (rs < 0) {
	        int	j ;
	        for (j = 0 ; j < i ; j += 1) {
	            LISTOBJ_FINISH(lop+j) ;
	        }
	    }

	    if (rs < 0) {
	        uc_free(op->lists) ;
	        op->lists = NULL ;
	    }
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (txtindexmks_listbegin) */


static int txtindexmks_listend(TXTINDEXMKS *op)
{
	LISTOBJ		*lop ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	rs1 = strtab_finish(&op->eigens) ;
	if (rs >= 0) rs = rs1 ;

	lop = (LISTOBJ *) op->lists ;
	for (i = 0 ; i < op->pi.tablen ; i += 1) {
	    rs1 = LISTOBJ_FINISH(lop+i) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->lists != NULL) {
	    rs1 = uc_free(op->lists) ;
	    if (rs >= 0) rs = rs1 ;
	    op->lists = NULL ;
	}

	return rs ;
}
/* end subroutine (txtindexmks_listend) */


static int txtindexmks_addtag(TXTINDEXMKS *op,TXTINDEXMKS_TAG *tagp)
{
	LISTOBJ		*lop = op->lists ;
	bfile		*tfp = &op->tagfile ;
	uint		tagoff ;
	int		rs = SR_OK ;
	int		roff = tagp->recoff ;
	int		rlen = tagp->reclen ;
	int		c = 0 ;
	const char	*fmt = "%s:%u,%u\n" ;
	const char	*fp ;

#if	CF_DEBUGS 
	debugprintf("txtindexmks_addtag: tag=%u,%u\n",
	    tagp->recoff,tagp->reclen) ;
	debugprintf("txtindexmks_addtag: fname=%s\n",tagp->fname) ;
#endif

/* write out the tag to the tag-file */

	tagoff = op->tagoff ;
	fp = (tagp->fname != NULL) ? tagp->fname : zerobuf ;
	if ((rs = bprintf(tfp,fmt,fp,roff,rlen)) >= 0) {
	    uint	hv ;
	    const int	maxkl = MIN(op->pi.maxwlen,KEYBUFLEN) ;
	    int		i ;
	    int		kl ;
	    int		hi ;
	    const char	*kp ;
	    char	keybuf[KEYBUFLEN + 1] ;
	    op->tagoff += rs ;

/* process the keys associated with this tag */

	    for (i = 0 ; i < tagp->nkeys ; i += 1) {

	        kp = tagp->keys[i].kp ;
	        kl = tagp->keys[i].kl ;

#if	CF_DEBUGS 
	        debugprintf("txtindexmks_addtag: key=>%t<\n",kp,kl) ;
#endif

/* overflow and maximum word length management */

	        if (kl >= 0) {
	            if (kl > maxkl)
	                kl = maxkl ;
	        } else
	            kl = strnlen(kp,maxkl) ;

	        if (kl < op->pi.minwlen) continue ;

/* convert to lower-case as necessary */

	        if (hasuc(kp,kl)) {
	            strwcpylc(keybuf,kp,kl) ; /* can't overflow (see 'maxkl') */
	            kp = keybuf ;
	        }

/* hash it */

	        hv = hashelf(kp,kl) ;

	        hi = (hv % op->pi.tablen) ;

#if	CF_DEBUGS 
	        debugprintf("txtindexmks_addtag: tablen=%d hi=%d\n",
	            op->pi.tablen,hi) ;
#endif

	        if ((rs = LISTOBJ_ADD((lop+hi),tagoff)) >= 0) {
	            if (rs != INT_MAX) {
	                c += 1 ;
	                op->ti.nkeys += 1 ;
	            }
	        }

	        if (rs < 0) break ;
	    } /* end for (processing keys) */

#if	CF_DEBUGS 
	    debugprintf("txtindexmks_addtag: for-out rs=%d\n",rs) ;
#endif

	} /* end if (bprintf) */

#if	CF_DEBUGS 
	debugprintf("txtindexmks_addtag: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (txtindexmks_addtag) */


static int txtindexmks_mkhash(TXTINDEXMKS *op)
{
	int		rs ;
	int		rs1 ;
	int		clists = 0 ;

	if ((rs = txtindexmks_nhashopen(op)) >= 0) {
	    TXTINDEXHDR	hdr ;

	    memset(&hdr,0,sizeof(TXTINDEXHDR)) ;
	    hdr.vetu[0] = TXTINDEXHDR_VERSION ;
	    hdr.vetu[1] = ENDIAN ;
	    hdr.vetu[2] = 0 ;
	    hdr.vetu[3] = 0 ;
	    hdr.wtime = (uint) time(NULL) ;
	    hdr.tfsize = op->tagsize ;
	    hdr.taglen = op->ti.ntags ;
	    hdr.minwlen = op->pi.minwlen ;
	    hdr.maxwlen = op->pi.maxwlen ;
	    hdr.tablen = op->pi.tablen ;

	    if ((rs = txtindexmks_mkhashwrmain(op,&hdr)) >= 0) {
	        const int	hlen = HDRBUFLEN ;
	        char		hbuf[HDRBUFLEN+1] ;
	        hdr.hfsize = rs ;

#if	CF_DEBUGS
	        debugprintf("txtindexmks_mkhash: hfsize=%u\n",hdr.hfsize) ;
	        debugprintf("txtindexmks_mkhash: tfsize=%u\n",hdr.tfsize) ;
	        debugprintf("txtindexmks_mkhash: wtime=%u\n",hdr.wtime) ;
	        debugprintf("txtindexmks_mkhash: sdnoff=%u\n",hdr.sdnoff) ;
	        debugprintf("txtindexmks_mkhash: sfnoff=%u\n",hdr.sfnoff) ;
	        debugprintf("txtindexmks_mkhash: listoff=%u\n",hdr.listoff) ;
	        debugprintf("txtindexmks_mkhash: taboff=%u\n",hdr.taboff) ;
	        debugprintf("txtindexmks_mkhash: tablen=%u\n",hdr.tablen) ;
	        debugprintf("txtindexmks_mkhash: taglen=%u\n",hdr.taglen) ;
	        debugprintf("txtindexmks_mkhash: maxtags=%u\n",hdr.maxtags) ;
	        debugprintf("txtindexmks_mkhash: minwlen=%u\n",hdr.minwlen) ;
	        debugprintf("txtindexmks_mkhash: maxwlen=%u\n",hdr.maxwlen) ;
#endif /* CF_DEBUGS */

	        if ((rs = txtindexhdr(&hdr,0,hbuf,hlen)) >= 0) {
	            const int	bl = rs ;
	            if ((rs = u_pwrite(op->nfd,hbuf,bl,0L)) >= 0) {
	                const mode_t	om = op->om ;
	                rs = uc_fminmod(op->nfd,om) ;
	            }
	        }

	    } /* end if (txtindexmks_mkhashwrmain) */

	    rs1 = txtindexmks_nhashclose(op) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (txtindexmks_nhash) */

#if	CF_DEBUGS
	debugprintf("txtindexmks_mkhash: ret rs=%d clists=%u\n",
	    rs,clists) ;
#endif

	return (rs >= 0) ? clists : rs ;
}
/* end subroutine (txtindexmks_mkhash) */


static int txtindexmks_mkhashwrmain(TXTINDEXMKS *op,TXTINDEXHDR *hdrp)
{
	FILEBUF		hf, *hfp = &hf ;
	const int	nfd = op->nfd ;
	const int	ps = getpagesize() ;
	int		bsize ;
	int		rs ;
	int		rs1 ;
	int		off = 0 ;
	bsize = (ps * 4) ;
	if ((rs = filebuf_start(hfp,nfd,0,bsize,0)) >= 0) {
	    if ((rs = txtindexmks_mkhashwrhdr(op,hdrp,hfp,off)) >= 0) {
	        off += rs ;
/* write SDN string */
	        if ((rs >= 0) && (op->pi.sdn != NULL)) {
	            hdrp->sdnoff = off ;
	            rs = filebuf_writefill(hfp,op->pi.sdn,-1) ;
	            off += rs ;
	        }
/* write SFN string */
	        if ((rs >= 0) && (op->pi.sfn != NULL)) {
	            hdrp->sfnoff = off ;
	            rs = filebuf_writefill(hfp,op->pi.sfn,-1) ;
	            off += rs ;
	        }
/* write out the lists while creating the offset table */
	        if (rs >= 0) {
	            rs = txtindexmks_mkhashwrtab(op,hdrp,hfp,off) ;
	            off += rs ;
	        }
/* make and write out the eigen: string, record, and index tables */
	        if (rs >= 0) {
	            rs = txtindexmks_mkhashwreigen(op,hdrp,hfp,off) ;
	            off += rs ;
	        }
	    } /* end if (txtindexmks_mkhashwrhdr) */
	    rs1 = filebuf_finish(hfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */
	return (rs >= 0) ? off : rs ;
}
/* end subroutine (txtindexmks_mkhashwrmain) */


/* ARGSUSED */
static int txtindexmks_mkhashwrhdr(TXTINDEXMKS *op,TXTINDEXHDR *hdrp,
		FILEBUF *hfp,int off)
{
	const int	hlen = HDRBUFLEN ;
	int		rs ;
	int		wlen = 0 ;
	char		hbuf[HDRBUFLEN+1] ;
	if (op == NULL) return SR_FAULT ; /* LINT */
	if ((rs = txtindexhdr(hdrp,0,hbuf,hlen)) >= 0) {
	    const int	bl = rs ;
	    rs = filebuf_writefill(hfp,hbuf,bl) ;
	    wlen += rs ;
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (txtindexmks_mkhashwrhdr) */


static int txtindexmks_mkhashwrtab(TXTINDEXMKS *op,TXTINDEXHDR *hdrp,
		FILEBUF *hfp,int off)
{
	int		*table = NULL ;
	const int	tsize = op->pi.tablen * sizeof(uint) ;
	int		rs ;
	int		wlen = 0 ;
	if ((rs = uc_malloc(tsize,&table)) >= 0) {
	    int		i ;
	    hdrp->listoff = off ;
	    for (i = 0 ; (rs >= 0) && (i < op->pi.tablen) ; i += 1) {
	        rs = txtindexmks_mkhashwrtabone(op,hdrp,hfp,off,table,i) ;
	        off += rs ;
	        wlen += rs ;
	    } /* end for (table lists) */
	    hdrp->maxtags = op->ti.maxtags ;
	    hdrp->taboff = off ;
	    if (rs >= 0) {
	        const int	tsize = op->pi.tablen * sizeof(uint) ;
	        rs = filebuf_write(hfp,table,tsize) ;
	        off += rs ;
	        wlen += rs ;
	    }
	    uc_free(table) ;
	} /* end if (m-a) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (txtindexmks_mkhashwrtab) */


/* ARGSUSED */
static int txtindexmks_mkhashwrtabone(TXTINDEXMKS *op,TXTINDEXHDR *hdrp,
		FILEBUF *hfp,int off,int *tab,int i)
{
	LISTOBJ		*lop = (LISTOBJ *) op->lists ;
	int		rs ;
	int		wlen = 0 ;
	lop += i ;
	if ((rs = LISTOBJ_COUNT(lop)) > 0) {
	    const int	c = rs ;
	    const int	asize = ((c+1)*sizeof(int)) ;
	    int		*va ;
	    if ((rs = uc_malloc(asize,&va)) >= 0) {
	        if ((rs = LISTOBJ_MKVEC(lop,va)) >= 0) {
	            if (c > op->ti.maxtags) op->ti.maxtags = c ;
	            if (c > 0) {
	                const int	csize = sizeof(int) ;
	                op->clists += 1 ;
	                tab[i] = off ;
	                if ((rs = filebuf_write(hfp,&c,csize)) >= 0) {
	                    off += rs ;
	                    wlen += rs ;
	                    if (c > 0) {
	                        const int	vsize = (c*sizeof(uint)) ;
	                        rs = filebuf_write(hfp,va,vsize) ;
	                        off += rs ;
	                        wlen += rs ;
	                    }
	                } /* end if (filebuf_write) */
	            } /* end if (positive) */
	        } /* end if (LISTINT_MKVEC) */
	        uc_free(va) ;
	    } /* end if (m-a) */
	} else if (rs == 0) {
	    tab[i] = 0 ;
	} /* end if (LISTOBJ_COUNT) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (txtindexmks_mkhashwrtabone) */


static int txtindexmks_mkhashwreigen(TXTINDEXMKS *op,TXTINDEXHDR *hdrp,
		FILEBUF *hfp,int off)
{
	int		rs ;
	int		wlen = 0 ;

	if ((rs = strtab_strsize(&op->eigens)) >= 0) {
	    const int	nskip = TXTINDEXMKS_NSKIP ;
	    int		essize = rs ;
	    char	*estab = NULL ;

	    hdrp->eiskip = nskip ;
	    if (essize > 0) {

#if	CF_DEBUGS
	        debugprintf("txtindexmks_wreigen: "
	            "strtab_strsize() rs=%d\n",rs) ;
#endif

	        if ((rs = uc_malloc(essize,&estab)) >= 0) {

	            hdrp->esoff = off ;
	            hdrp->essize = essize ;
	            rs = strtab_strmk(&op->eigens,estab,essize) ;

#if	CF_DEBUGS
	            debugprintf("txtindexmks_wreigen: "
	                "strtab_strmk() rs=%d\n",rs) ;
#endif

	            if (rs >= 0) {
	                rs = filebuf_write(hfp,estab,essize) ;
	                off += rs ;
	                wlen += rs ;
	            }

	            uc_free(estab) ;
	        } /* end if (eigen-string table) */

#if	CF_DEBUGS
	        debugprintf("txtindexmks_wreigen: st rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {
	            int		ersize = 0 ;
	            int		n, erlen ;
	            int		*ertab = NULL ;

	            n = strtab_count(&op->eigens) ;
	            erlen = (n+1) ;

#if	CF_DEBUGS
	            debugprintf("txtindexmks_wreigen: n=%u erlen=%u\n",
	                n,erlen) ;
#endif

	            if ((rs = strtab_recsize(&op->eigens)) >= 0) {
	                void	*p ;
	                ersize = rs ;
	                if ((rs = uc_malloc(ersize,&p)) >= 0) {
	                    ertab = p ;

	                    hdrp->ersize = ersize ;
	                    hdrp->eroff = off ;
	                    hdrp->erlen = erlen ;
	                    rs = strtab_recmk(&op->eigens,ertab,ersize) ;

#if	CF_DEBUGS
	                    debugprintf("txtindexmks_wreigen: "
	                        "strtab_recmk() rs=%d\n",rs) ;
#endif

	                    if (rs >= 0) {
	                        rs = filebuf_write(hfp,ertab,ersize) ;
	                        off += rs ;
	                        wlen += rs ;
	                    }

	                    uc_free(ertab) ;
	                } /* end if (memory-allocation) */
	            } /* end if (recsize) */

	        } /* end if (eigen-record table) */

#if	CF_DEBUGS
	        debugprintf("txtindexmks_wreigen: rt rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {
	            int		eisize = 0 ;
	            int		eilen ;
	            int		(*eitab)[3] = NULL ;

	            eilen = strtab_indlen(&op->eigens) ;

	            if ((rs = strtab_indsize(&op->eigens)) >= 0) {
	                void	*p ;
	                eisize = rs ;
	                if ((rs = uc_malloc(eisize,&p)) >= 0) {
	                    eitab = p ;

	                    hdrp->eisize = eisize ;
	                    hdrp->eioff = off ;
	                    hdrp->eilen = eilen ;
	                    rs = strtab_indmk(&op->eigens,eitab,eisize,nskip) ;

#if	CF_DEBUGS
	                    debugprintf("txtindexmks_wreigen: "
	                        "strtab_indmk() rs=%d\n",rs) ;
#endif

	                    if (rs >= 0) {
	                        rs = filebuf_write(hfp,eitab,eisize) ;
	                        off += rs ;
	                        wlen += rs ;
	                    }

	                    uc_free(eitab) ;
	                } /* end if (memory-allocation) */
	            } /* end if (indsize) */

	        } /* end if (eigen-index table) */

	    } /* end if (positive) */
	} /* end if (strtab_strsize) */

#if	CF_DEBUGS && CF_DEBUGEIGEN
	if (rs >= 0)
	    rs = txtindexmks_printeigen(op) ;
#endif

#if	CF_DEBUGS
	debugprintf("txtindexmks_wreigen: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (txtindexmks_mkhashwreigen) */


static int txtindexmks_nhashopen(TXTINDEXMKS *op)
{
	const int	type = (op->f.ofcreat && (! op->f.ofexcl)) ;
	int		rs ;
	cchar		*dbn = op->dbname ;
	cchar		*suf = FSUF_IND ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mknewfname(tbuf,type,dbn,suf)) >= 0) {
	    const mode_t	om = op->om ;
	    int			of = (O_CREAT|O_WRONLY) ;
	    cchar		*tfn = tbuf ;
	    char		rbuf[MAXPATHLEN+1] ;
	    if (type) {
	        rs = opentmpfile(tbuf,of,om,rbuf) ;
	        op->nfd = rs ;
	        tfn = rbuf ;
	    } else {
	        if (op->f.ofexcl) of |= O_EXCL ;
	        rs = uc_open(tbuf,of,om) ;
	        op->nfd = rs ;
	    }
	    if (rs >= 0) {
	        cchar	*cp ;
	        if ((rs = uc_mallocstrw(tfn,-1,&cp)) >= 0) {
	            op->nidxfname = (char *) cp ;
	        }
	    } /* end if (ok) */
	} /* end if (mknewfname) */
	return rs ;
}
/* end subroutine (txtindexmks_nhashopen) */


static int txtindexmks_nhashclose(TXTINDEXMKS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->nfd >= 0) {
	    rs1 = u_close(op->nfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfd = -1 ;
	}
	return rs ;
}
/* end subroutine (txtindexmks_nhashclose) */


static int txtindexmks_renamefiles(TXTINDEXMKS *op)
{
	int		rs ;
	const char	*end = ENDIANSTR ;
	char		hashfname[MAXPATHLEN + 1] ;

	if ((rs = mkfnamesuf2(hashfname,op->dbname,FSUF_IND,end)) >= 0) {
	    char	tagfname[MAXPATHLEN + 1] ;
	    if ((rs = mkfnamesuf1(tagfname,op->dbname,FSUF_TAG)) >= 0) {

	        if ((rs = u_rename(op->ntagfname,tagfname)) >= 0) {
	            op->ntagfname[0] = '\0' ;
	            if ((rs = u_rename(op->nidxfname,hashfname)) >= 0) {
	                op->nidxfname[0] = '\0' ;
		    }
		}
		if (rs < 0) {
	            if (op->ntagfname[0] != '\0') {
	                u_unlink(op->ntagfname) ;
	                op->ntagfname[0] = '\0' ;
	            }
	            if (op->nidxfname[0] != '\0') {
	                u_unlink(op->nidxfname) ;
	                op->nidxfname[0] = '\0' ;
	            }
		}

	    } /* end if (ok) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (txtindexmks_renamefiles) */


int txtindexmks_ntagclose(TXTINDEXMKS *op)
{
	int		rs ;
	int		rs1 ;
	int		tagsize = 0 ;
	if ((rs = bsize(&op->tagfile)) >= 0) {
	    mode_t	om = op->om ;
	    op->tagsize = rs ;
	    tagsize = rs ;
	    rs = bcontrol(&op->tagfile,BC_MINMOD,om) ;
	}
	rs1 = bclose(&op->tagfile) ;
	if (rs >= 0) rs = rs1 ;
	op->f.tagopen = FALSE ;
	return (rs >= 0) ? tagsize : rs ;
}
/* end if (txtindexmks_ntagclose) */


#if	CF_DEBUGS && CF_DEBUGEIGEN

static int txtindexmks_printeigen(TXTINDEXMKS *op)
{
	STRTAB		*edp = &op->eigens ;
	int		rs ;

	if ((rs = strtab_count(edp)) >= 0) {
	    int		erlen = (rs+1) ;
	    if ((rs = strtab_strsize(edp)) >= 0) {
		int	essize = rs ;
		char	*estab ;
		if ((rs = uc_malloc(essize,&estab)) >= 0) {
		    if ((rs = strtab_strmk(edp,estab,essize)) >= 0) {
			if ((rs = strtab_recsize(edp)) >= 0) {
			    int	ersize = rs ;
			    int	*ertab ;
			    if ((rs = uc_malloc(ersize,&ertab)) >= 0) {
				struct printeigen	a ;
				a.ertab = ertab ;
				a.estab = estab ;
				a.ersize = ersize ;
				a.erlen = erlen ;
				rs = txtindexmks_printeigener(op,&a) ;
				uc_free(ertab) ;
			    } /* end if (m-a) */
			}
		    }
		    uc_free(estab) ;
		} /* end if (m-a) */
	    }
	}

	return rs ;
}
/* end subroutine (txtindexmks_printeigen) */


static int txtindexmks_printeigener(TXTINDEXMKS *op,struct printeigen *ap)
{
	STRTAB		*edp = &op->eigens ;
	int		rs ;
	int		rs1 ;
	int		*ertab = ap->ertab ;
	char		*estab = ap->estab ;
	int		ersize = ap->ersize ;
	int		erlen = ap->erlen ;
	if ((rs = strtab_recmk(edp,ertab,ersize)) >= 0) {
	    if ((rs = strtab_indsize(edp)) >= 0) {
		int	(*eitab)[3] ;
		int	eisize = rs ;
		int	eilen = strtab_indlen(edp) ;
		if ((rs = uc_malloc(eisize,&eitab)) >= 0) {
		    int	ns = TXTINDEXMKS_NSKIP ;
		    if ((rs = strtab_indmk(edp,eitab,eisize,ns)) >= 0) {
			int	i ;
			int	si ;
			int	sl ;
			cchar	*sp ;
			cchar	*fmt ;
			fmt = "txtindexmks_printeigen: i=%u si=%u s=%s\n" ;
			for (i = 1 ; i < erlen ; i += 1) {
	    		    si = ertab[i] ;
	    		    if (si > 0) {
	        		debugprintf(fmt,i,si,(estab + si)) ;
			    }
	    		    sp = (estab + si) ;
	    		    sl = strlen(sp) ;
	    		    rs1 = strtabfind(estab,eitab,eilen,ns,sp,sl) ;
	    		    fmt = "txtindexmks_printeigen: stabfind() rs=%d\n";
	    		    debugprintf(fmt,rs1) ;
			} /* end for */
		    }
		    uc_free(eitab) ;
		} /* end if (m-a) */
	    }
	}
	return rs ;
}
/* end subroutine (txtindexmks_printeigener) */

#endif /* CF_DEBUGS */


static int mknewfname(char *tbuf,int type,cchar *dbn,cchar *suf)
{
	cchar		*end = ENDIANSTR ;
	cchar		*fin = (type) ? "xXXXX" : "n" ;
	return mkfnamesuf3(tbuf,dbn,suf,end,fin) ;
}
/* end subroutine (mknewfname) */


static int unlinkstale(cchar *fn,int to)
{
	struct ustat	sb ;
	const time_t	dt = time(NULL) ;
	int		rs ;
	if ((rs = uc_stat(fn,&sb)) >= 0) {
	    if ((dt-sb.st_mtime) >= to) {
		uc_unlink(fn) ;
		rs = 1 ;
	    } else {
	        rs = 0 ;
	    }
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}
	return rs ;
}
/* end subroutine (unlinkstale) */


#if	CF_DEBUGS && CF_DEBUGCECK

static int checksize(cchar *s,int rs,int size)
{
	if (rs != size) {
	    debugprintf("txtindexmks/checksize: %s mismatch rs=%d size=%d\n",
	        s,rs,size) ;
	}
	return 0 ;
}
/* end subroutine (checksize) */

static int checkalign(cchar *s,uint off)
{
	if ((off & 3) != 0) {
	    debugprintf("txtindexmks/checkalign: %s misalign off=%08lx\n",
	        s,off) ;
	}
	return 0 ;
}
/* end subroutine (checkalign) */

#endif /* CF_DEBUGS */


