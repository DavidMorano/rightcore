/* varmks */

/* make a VAR database */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_FIRSTHASH	0		/* arrange for first-attempt hashing */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates a VAR database file.

	Synopsis:

	int varmks_open(op,dbname,of,om,n)
	VARMKS		*op ;
	const char	dbname[] ;
	int		of ;
	mode_t		om ;
	int		n ;

	Arguments:

	op		object pointer
	dbname		name of (path-to) DB
	of		open-flags
	om		open-mode
	n		starting estimate of numbers of variables

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
	O_CREAT|O_EXCL	yes		no		SR_EXIST
	O_CREAT|O_EXCL	yes		yes		SR_INPROGRESS

	O_CREAT		x		x		SR_OK (created)


*******************************************************************************/


#define	VARMKS_MASTER	0


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endian.h>
#include	<estrings.h>
#include	<vecobj.h>
#include	<strtab.h>
#include	<filebuf.h>
#include	<endianstr.h>
#include	<localmisc.h>

#include	"varmks.h"
#include	"varhdr.h"


/* local defines */

#define	VARMKS_SIZEMULT	4
#define	VARMKS_NSKIP	5
#define	VARMKS_INDPERMS	0664

#undef	RECTAB
#define	RECTAB		struct varmks_rectab

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	120
#endif

#define	HDRBUFLEN	(sizeof(VARHDR) + 128)
#define	BUFLEN		(sizeof(VARHDR) + 128)

#define	FSUF_IDX	VARHDR_FSUF

#define	TO_OLDFILE	(5 * 60)

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	NDF		"varmks.deb"


/* external subroutines */

extern uint	hashelf(const char *,int) ;
extern uint	hashagain(uint,int,int) ;
extern uint	nextpowtwo(uint) ;

extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	strnnlen(const char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	getpwd(char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	mktmpfile(char *,mode_t,cchar *) ;
extern int	vstrkeycmp(const char *,const char *) ;
extern int	filebuf_writefill(FILEBUF *,cchar *,int) ;
extern int	hasuc(const char *,int) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* external variables */


/* exported variables */

VARMKS_OBJ	varmks = {
	"varmks",
	sizeof(VARMKS)
} ;


/* local structures */

struct varentry {
	uint	khash ;
	uint	ri ;
	uint	ki ;
	uint	hi ;
} ;


/* forward references */

static int	varmks_filesbegin(VARMKS *) ;
static int	varmks_filesbeginc(VARMKS *) ;
static int	varmks_filesbeginwait(VARMKS *) ;
static int	varmks_filesbegincreate(VARMKS *,cchar *,int,mode_t) ;
static int	varmks_filesend(VARMKS *) ;

static int	varmks_listbegin(VARMKS *,int) ;
static int	varmks_listend(VARMKS *) ;

static int	varmks_mkvarfile(VARMKS *) ;
static int	varmks_mkvarfiler(VARMKS *) ;
static int	varmks_mkidxwrmain(VARMKS *,VARHDR *) ;
static int	varmks_mkidxwrhdr(VARMKS *,VARHDR *,FILEBUF *) ;
static int	varmks_mkrectab(VARMKS *,VARHDR *,FILEBUF *,int) ;
static int	varmks_mkind(VARMKS *,const char *,uint (*)[3],int) ;
static int	varmks_mkstrtab(VARMKS *,VARHDR *,FILEBUF *,int) ;
static int	varmks_nidxopen(VARMKS *) ;
static int	varmks_nidxclose(VARMKS *) ;
static int	varmks_renamefiles(VARMKS *) ;

static int	rectab_start(RECTAB *,int) ;
static int	rectab_add(RECTAB *,uint,uint) ;
static int	rectab_done(RECTAB *) ;
static int	rectab_getvec(RECTAB *,uint (**)[2]) ;
static int	rectab_extend(RECTAB *) ;
static int	rectab_finish(RECTAB *) ;

#ifdef	COMMENT
static int	rectab_count(RECTAB *) ;
#endif

static int	mknewfname(char *,int,cchar *,cchar *) ;
static int	unlinkstale(cchar *,int) ;

static int	indinsert(uint (*rt)[2],uint (*it)[3],int,struct varentry *) ;
static int	hashindex(uint,int) ;


/* local variables */


/* exported subroutines */


int varmks_open(VARMKS *op,cchar dbname[],int of,mode_t om,int n)
{
	int		rs ;
	int		c = 0 ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if (dbname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("varmks_open: ent dbname=%s\n",dbname) ;
#endif /* CF_DEBUGS */


	if (n < VARMKS_NENTRIES)
	    n = VARMKS_NENTRIES ;

	memset(op,0,sizeof(VARMKS)) ;
	op->om = om ;
	op->nfd = -1 ;
	op->gid = -1 ;

	op->f.ofcreat = MKBOOL(of & O_CREAT) ;
	op->f.ofexcl = MKBOOL(of & O_EXCL) ;

	if ((rs = uc_mallocstrw(dbname,-1,&cp)) >= 0) {
	    op->dbname = cp ;
	    if ((rs = varmks_filesbegin(op)) >= 0) {
		c = rs ;
	        if ((rs = varmks_listbegin(op,n)) >= 0) {
	            op->magic = VARMKS_MAGIC ;
	        }
	        if (rs < 0)
	            varmks_filesend(op) ;
	    } /* end if */
	    if (rs < 0) {
	        uc_free(op->dbname) ;
	        op->dbname = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("varmks_open: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (varmks_open) */


int varmks_close(VARMKS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		nvars = 0 ;
	int		f_go = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != VARMKS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("varmks_close: nvars=%u\n",op->nvars) ;
#endif

	f_go = (! op->f.abort) ;
	nvars = op->nvars ;
	if (! op->f.abort) {
	    rs1 = varmks_mkvarfile(op) ;
	    if (rs >= 0) rs = rs1 ;
	    f_go = f_go && (rs1 >= 0) ;
	}

#if	CF_DEBUGS
	debugprintf("varmks_close: varmks_mkvarfile() rs=%d\n",rs) ;
#endif

	if (op->nfd >= 0) {
	    rs1 = u_close(op->nfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfd = -1 ;
	}

	rs1 = varmks_listend(op) ;
	if (rs >= 0) rs = rs1 ;
	f_go = f_go && (rs1 >= 0) ;

	if ((rs >= 0) && (nvars > 0) && f_go) {
	    rs1 = varmks_renamefiles(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("varmks_close: varmks_renamefiles() rs=%d\n",rs) ;
#endif

	rs1 = varmks_filesend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->dbname != NULL) {
	    rs1 = uc_free(op->dbname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("varmks_close: ret=%d\n",rs) ;
#endif /* CF_DEBUGS */

	op->magic = 0 ;
	return (rs >= 0) ? nvars : rs ;
}
/* end subroutine (varmks_close) */


int varmks_addvar(VARMKS *op,cchar k[],cchar vp[],int vl)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (k == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	if (op->magic != VARMKS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("varmks_addvar: k=%s v=>%t<\n",k,vp,vl) ;
#endif

	if ((rs = strtab_add(&op->keys,k,-1)) >= 0) {
	    uint	ki = rs ;
	    if ((rs = strtab_add(&op->vals,vp,vl)) >= 0) {
	        uint	vi = rs ;
	        if ((rs = rectab_add(&op->rectab,ki,vi)) >= 0) {
	            op->nvars += 1 ;
	        }
	    }
	}

#if	CF_DEBUGS
	debugprintf("varmks_addvar: ret=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (varmks_addvar) */


int varmks_abort(VARMKS *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != VARMKS_MAGIC) return SR_NOTOPEN ;

	op->f.abort = TRUE ;
	return SR_OK ;
}
/* end subroutine (varmks_abort) */


int varmks_chgrp(VARMKS *op,gid_t gid)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != VARMKS_MAGIC) return SR_NOTOPEN ;

	op->gid = gid ;
	return SR_OK ;
}
/* end subroutine (varmks_chgrp) */


/* private subroutines */


static int varmks_filesbegin(VARMKS *op)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (op->f.ofcreat) {
	    rs = varmks_filesbeginc(op) ;
	} else {
	    rs = varmks_filesbeginwait(op) ;
	    c = rs ;
	}
#if	CF_DEBUGS
	debugprintf("varmks_filesbegin: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (varmks_filesbegin) */


static int varmks_filesbeginc(VARMKS *op)
{
	const int	type = (op->f.ofcreat && (! op->f.ofexcl)) ;
	int		rs ;
	cchar		*dbn = op->dbname ;
	cchar		*suf = FSUF_IDX	 ;
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
	        mode_t	om = op->om ;
	        int	of = O_CREAT ;
	        if (op->f.ofexcl) of |= O_EXCL ;
	        rs = varmks_filesbegincreate(op,tfn,of,om) ;
		if ((rs < 0) && type) {
		    uc_unlink(rbuf) ;
		}
	    } /* end if (ok) */
	} /* end if (mknewfname) */
	return rs ;
}
/* end subroutine (varmks_filesbeginc) */


static int varmks_filesbeginwait(VARMKS *op)
{
	int		rs ;
	int		c = 0 ;
	cchar		*dbn = op->dbname ;
	cchar		*suf = FSUF_IDX	 ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mknewfname(tbuf,FALSE,dbn,suf)) >= 0) {
	    const mode_t	om = op->om ;
	    const int		to_stale = VARMKS_INTSTALE ;
	    const int		nrs = SR_EXISTS ;
	    const int		of = (O_CREAT|O_WRONLY|O_EXCL) ;
	    int			to = VARMKS_INTOPEN ;
	    while ((rs = varmks_filesbegincreate(op,tbuf,of,om)) == nrs) {
#if	CF_DEBUGS
	        debugprintf("varmks_filesbeginwait: loop ret rs=%d\n",rs) ;
#endif
	        c = 1 ;
	        sleep(1) ;
	        unlinkstale(tbuf,to_stale) ;
	        if (to-- == 0) break ;
	    } /* end while (db exists) */
	    if (rs == nrs) {
	        op->f.ofcreat = FALSE ;
	        c = 0 ;
	        rs = varmks_filesbeginc(op) ;
	    }
	} /* end if (mknewfname) */
#if	CF_DEBUGS
	debugprintf("varmks_filesbeginwait: ret ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (varmks_filesbeginwait) */


static int varmks_filesbegincreate(VARMKS *op,cchar *tfn,int of,mode_t om)
{
	int		rs ;
#if	CF_DEBUGS
	{
	    char	obuf[100+1] ;
	    snopenflags(obuf,100,of) ;
	    debugprintf("varmks_filesbegincreate: ent of=%s\n",obuf) ;
	    debugprintf("varmks_filesbegincreate: om=%05o\n",om) ;
	}
#endif
	if ((rs = uc_open(tfn,of,om)) >= 0) {
	    const int	fd = rs ;
	    cchar	*cp ;
	    op->f.created = TRUE ;
	    if ((rs = uc_mallocstrw(tfn,-1,&cp)) >= 0) {
	        op->nidxfname = (char *) cp ;
	    }
	    u_close(fd) ;
	} /* end if (create) */

#if	CF_DEBUGS
	debugprintf("varmks_filesbegincreate: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (varmks_filesbegincreate) */


static int varmks_filesend(VARMKS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->nidxfname != NULL) {
	    if (op->f.created && (op->nidxfname[0] != '\0')) {
	        u_unlink(op->nidxfname) ;
	    }
	    rs1 = uc_free(op->nidxfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nidxfname = NULL ;
	}

	if (op->idname != NULL) {
	    rs1 = uc_free(op->idname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->idname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("varmks_filesend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (varmks_filesend) */


static int varmks_listbegin(VARMKS *op,int n)
{
	const int	size = (n * VARMKS_SIZEMULT) ;
	int		rs ;

	if ((rs = strtab_start(&op->keys,size)) >= 0) {
	    if ((rs = strtab_start(&op->vals,size)) >= 0) {
	        rs = rectab_start(&op->rectab,n) ;
	        if (rs < 0)
	            strtab_finish(&op->vals) ;
	    } /* end if (strtab-vals) */
	    if (rs < 0)
	        strtab_finish(&op->keys) ;
	} /* end if (strtab-keys) */

	return rs ;
}
/* end subroutine (varmks_listbegin) */


static int varmks_listend(VARMKS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = rectab_finish(&op->rectab) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = strtab_finish(&op->vals) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = strtab_finish(&op->keys) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (varmks_listend) */


static int varmks_mkvarfile(VARMKS *op)
{
	int		rs = SR_OK ;
	int		rtl ;

	if ((rtl = rectab_done(&op->rectab)) >= 0) {
	    if (rtl == (op->nvars + 1)) {
	        rs = varmks_mkvarfiler(op) ;
	    } else
	        rs = SR_BUGCHECK ;
	}

	return (rs >= 0) ? op->nvars : rs ;
}
/* end subroutine (varmks_mkvarfile) */


static int varmks_mkvarfiler(VARMKS *op)
{
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUGS
	debugprintf("varmks_mkidx: ent\n") ;
#endif

	if ((rs = varmks_nidxopen(op)) >= 0) {
	    VARHDR	hdr ;

	    memset(&hdr,0,sizeof(VARHDR)) ;
	    hdr.vetu[0] = VARHDR_VERSION ;
	    hdr.vetu[1] = ENDIAN ;
	    hdr.vetu[2] = 0 ;
	    hdr.vetu[3] = 0 ;
	    hdr.wtime = (uint) time(NULL) ;
	    hdr.nvars = op->nvars ;
	    hdr.nskip = VARMKS_NSKIP ;

	    if ((rs = varmks_mkidxwrmain(op,&hdr)) >= 0) {
	        const int	hlen = HDRBUFLEN ;
	        char		hbuf[HDRBUFLEN+1] ;
	        hdr.fsize = rs ;
	        wlen = rs ;

	        if ((rs = varhdr(&hdr,0,hbuf,hlen)) >= 0) {
	            const int	bl = rs ;
	            if ((rs = u_pwrite(op->nfd,hbuf,bl,0L)) >= 0) {
	                const mode_t	om = op->om ;
	                rs = uc_fminmod(op->nfd,om) ;
	            }
	        }

	    } /* end if (varmks_mkidxwrmain) */

	    rs1 = varmks_nidxclose(op) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (varmks_nidx) */

#if	CF_DEBUGS
	debugprintf("varmks_mkidx: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (varmks_mkvarfiler) */


static int varmks_mkidxwrmain(VARMKS *op,VARHDR *hdrp)
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
	    if ((rs = varmks_mkidxwrhdr(op,hdrp,hfp)) >= 0) {
	        off += rs ;
	        if (rs >= 0) {
	            rs = varmks_mkrectab(op,hdrp,hfp,off) ;
	            off += rs ;
	        }
	        if (rs >= 0) {
	            rs = varmks_mkstrtab(op,hdrp,hfp,off) ;
	            off += rs ;
	        }
	    } /* end if (varmks_mkidxwrhdr) */
	    rs1 = filebuf_finish(hfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */
	return (rs >= 0) ? off : rs ;
}
/* end subroutine (varmks_mkidxwrmain) */


/* ARGSUSED */
static int varmks_mkidxwrhdr(VARMKS *op,VARHDR *hdrp,FILEBUF *hfp)
{
	const int	hlen = HDRBUFLEN ;
	int		rs ;
	int		wlen = 0 ;
	char		hbuf[HDRBUFLEN+1] ;
	if (op == NULL) return SR_FAULT ; /* LINT */
	if ((rs = varhdr(hdrp,0,hbuf,hlen)) >= 0) {
	    rs = filebuf_writefill(hfp,hbuf,rs) ;
	    wlen += rs ;
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (varmks_mkidxwrhdr) */


static int varmks_mkrectab(VARMKS *op,VARHDR *hdrp,FILEBUF *hfp,int off)
{
	int		rs ;
	int		rtl ;
	int		size ;
	int		wlen = 0 ;
	uint		(*rt)[2] ;
	rtl = rectab_getvec(&op->rectab,&rt) ;
	hdrp->rtoff = off ;
	hdrp->rtlen = rtl ;
	size = ((rtl + 1) * 2 * sizeof(uint)) ;
	if ((rs = filebuf_write(hfp,rt,size)) >= 0) {
	    STRTAB	*ksp = &op->keys ;
	    char	*kstab = NULL ;
	    off += rs ;
	    wlen += rs ;
	    size = strtab_strsize(ksp) ;
	    hdrp->ksoff = off ;
	    hdrp->kslen = size ;
	    if ((rs = uc_malloc(size,&kstab)) >= 0) {
	        if ((rs = strtab_strmk(ksp,kstab,size)) >= 0) {
	            rs = filebuf_write(hfp,kstab,size) ;
	            off += rs ;
	    	    wlen += rs ;
	        }
	        if (rs >= 0) {
	            uint	(*indtab)[3] = NULL ;
		    int		itl = nextpowtwo(rtl) ;
	            hdrp->itoff = off ;
	            hdrp->itlen = itl ;
	            size = (itl + 1) * 3 * sizeof(int) ;
	            if ((rs = uc_malloc(size,&indtab)) >= 0) {
	                memset(indtab,0,size) ;
	                if ((rs = varmks_mkind(op,kstab,indtab,itl)) >= 0) {
	                    rs = filebuf_write(hfp,indtab,size) ;
	                    off += rs ;
	    		    wlen += rs ;
	                }
	                uc_free(indtab) ;
	            } /* end if (memory allocation) */
	        } /* end if (record-index table) */
	        uc_free(kstab) ;
	    } /* end if (memory allocation) */
	} /* end if (key-string table) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (varmks_mkrectab) */


/* make an index table of the record table */
int varmks_mkind(op,kst,it,il)
VARMKS		*op ;
const char	kst[] ;
uint		(*it)[3] ;
int		il ;
{
	struct varentry	ve ;
	uint		ri, ki, hi ;
	uint		khash ;
	uint		(*rt)[2] ;
	int		rs = SR_OK ;
	int		rtl ;
	int		sc = 0 ;
	const char	*kp ;

	rtl = rectab_getvec(&op->rectab,&rt) ;

#if	CF_DEBUGS
	debugprintf("varmks_mkind: rtl=%u\n",rtl) ;
#endif

#if	CF_FIRSTHASH
	{
	    struct varentry	*vep ;
	    VECOBJ		ves ;
	    int			size, opts ;

	    size = sizeof(struct varentry) ;
	    opts = VECOBJ_OCOMPACT ;
	    if ((rs = vecobj_start(&ves,size,rtl,opts)) >= 0) {

	        for (ri = 1 ; ri < rtl ; ri += 1) {

	            ki = rt[ri][0] ;
	            kp = kst + ki ;
	            khash = hashelf(kp,-1) ;

	            hi = hashindex(khash,il) ;

	            if (it[hi][0] == 0) {

	                it[hi][0] = ri ;
	                it[hi][1] = (khash & INT_MAX) ;
	                it[hi][2] = 0 ;
	                sc += 1 ;

	            } else {

	                ve.ri = ri ;
	                ve.ki = ki ;
	                ve.khash = chash ;
	                ve.hi = hi ;
	                rs = vecobj_add(&ves,&ve) ;

	            }

	            if (rs < 0) break ;
	        } /* end for */

	        if (rs >= 0) {
	            int	i ;
	            for (i = 0 ; vecobj_get(&ves,i,&vep) >= 0 ; i += 1) {
	                sc += indinsert(rt,it,il,vep) ;
	            } /* end for */
	        }

	        vecobj_finish(&ves) ;
	    } /* end if (ves) */

	} /* end block */
#else /* CF_FIRSTHASH */
	{
	    for (ri = 1 ; ri < rtl ; ri += 1) {

	        ki = rt[ri][0] ;
	        kp = kst + ki ;

#if	CF_DEBUGS
	        debugprintf("varmks_mkind: ri=%u k=%s\n",ri,
	            kp,strnlen(kp,20)) ;
#endif

	        khash = hashelf(kp,-1) ;

	        hi = hashindex(khash,il) ;

	        ve.ri = ri ;
	        ve.ki = ki ;
	        ve.khash = khash ;
	        ve.hi = hi ;
	        sc += indinsert(rt,it,il,&ve) ;

	    } /* end for */
	}
#endif /* CF_FIRSTHASH */

	it[il][0] = UINT_MAX ;
	it[il][1] = 0 ;
	it[il][2] = 0 ;

	if (sc < 0)
	    sc = 0 ;

#if	CF_DEBUGS
	debugprintf("varmks_mkind: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? sc : rs ;
}
/* end subroutine (varmks_mkind) */


static int varmks_mkstrtab(VARMKS *op,VARHDR *hdrp,FILEBUF *hfp,int off)
{
	STRTAB		*vsp = &op->vals ;
	int		rs ;
	int		size ;
	int		wlen = 0 ;
	char		*vstab ;
	    size = strtab_strsize(vsp) ;
	    hdrp->vsoff = off ;
	    hdrp->vslen = size ;
	    if ((rs = uc_malloc(size,&vstab)) >= 0) {
	        if ((rs = strtab_strmk(vsp,vstab,size)) >= 0) {
	            rs = filebuf_write(hfp,vstab,size) ;
	            off += rs ;
		    wlen += rs ;
	        }
	        uc_free(vstab) ;
	    } /* end if (memory allocation) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (varmks_mkstrtab) */


static int varmks_nidxopen(VARMKS *op)
{
	const mode_t	om = op->om ;
	int		of = (O_CREAT|O_WRONLY) ;
	int		rs ;
	int		fd = -1 ;
#if	CF_DEBUGS
	debugprintf("varmks_nidxopen: ent nidxfname=%s\n",op->nidxfname) ;
#endif
	if (op->nidxfname == NULL) {
	    const int	type = (op->f.ofcreat && (! op->f.ofexcl)) ;
	    cchar	*dbn = op->dbname ;
	    cchar	*suf = FSUF_IDX ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mknewfname(tbuf,type,dbn,suf)) >= 0) {
	        cchar	*tfn = tbuf ;
	        char	rbuf[MAXPATHLEN+1] ;
	        if (type) {
	            rs = opentmpfile(tbuf,of,om,rbuf) ;
	            op->nfd = rs ;
		    fd = rs ;
	            tfn = rbuf ;
	        } else {
	            if (op->f.ofexcl) of |= O_EXCL ;
	            rs = uc_open(tbuf,of,om) ;
	            op->nfd = rs ;
		    fd = rs ;
	        }
	        if (rs >= 0) {
	            cchar	*cp ;
	            if ((rs = uc_mallocstrw(tfn,-1,&cp)) >= 0) {
	                op->nidxfname = (char *) cp ;
	            }
	        } /* end if (ok) */
	    } /* end if (mknewfname) */
	} else {
	    if (op->f.ofexcl) of |= O_EXCL ;
	    rs = uc_open(op->nidxfname,of,om) ;
	    op->nfd = rs ;
	    fd = rs ;
	}
#if	CF_DEBUGS
	debugprintf("varmks_nidxopen: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (varmks_nidxopen) */


static int varmks_nidxclose(VARMKS *op)
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
/* end subroutine (varmks_nidxclose) */


static int varmks_renamefiles(VARMKS *op)
{
	int		rs ;
	const char	*suf = FSUF_IDX ;
	const char	*end = ENDIANSTR ;
	char		idxfname[MAXPATHLEN + 1] ;

	if ((rs = mkfnamesuf2(idxfname,op->dbname,suf,end)) >= 0) {
	    if ((rs = u_rename(op->nidxfname,idxfname)) >= 0) {
	        op->nidxfname[0] = '\0' ;
	    } else {
	        u_unlink(op->nidxfname) ;
	        op->nidxfname[0] = '\0' ;
	    }
	} /* end if (mkfnamesuf) */

	return rs ;
}
/* end subroutine (varmks_renamefiles) */


static int rectab_start(RECTAB *rtp,int n)
{
	int		rs = SR_OK ;
	int		size ;
	void		*p ;

	if (n < 10)
	    n = 10 ;

	rtp->i = 0 ;
	rtp->n = n ;
	size = (n + 1) * 2 * sizeof(int) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    rtp->rectab = p ;
	    rtp->rectab[0][0] = 0 ;
	    rtp->rectab[0][1] = 0 ;
	    rtp->i = 1 ;
	}

	return rs ;
}
/* end subroutine (rectab_start) */


static int rectab_finish(RECTAB *rtp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (rtp->rectab != NULL) {
	    rs1 = uc_free(rtp->rectab) ;
	    if (rs >= 0) rs = rs1 ;
	    rtp->rectab = NULL ;
	}

	return rs ;
}
/* end subroutine (rectab_finish) */


static int rectab_add(RECTAB *rtp,uint ki,uint vi)
{
	int		rs = SR_OK ;
	int		i ;

	i = rtp->i ;
	if ((i + 1) > rtp->n)
	    rs = rectab_extend(rtp) ;

	if (rs >= 0) {
	    rtp->rectab[i][0] = ki ;
	    rtp->rectab[i][1] = vi ;
	    rtp->i += 1 ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (rectab_add) */


static int rectab_extend(RECTAB *rtp)
{
	int		rs = SR_OK ;

	if ((rtp->i + 1) > rtp->n) {
	    uint	(*va)[2] ;
	    int		nn, size ;

	    nn = (rtp->n + 1) * 2 ;
	    size = (nn + 1) * 2 * sizeof(int) ;
	    if ((rs = uc_realloc(rtp->rectab,size,&va)) >= 0) {
	        rtp->rectab = va ;
	        rtp->n = nn ;
	    }

	} /* end if */

	return rs ;
}
/* end subroutine (rectab_extend) */


static int rectab_done(RECTAB *rtp)
{
	int		i = rtp->i ;
	rtp->rectab[i][0] = UINT_MAX ;
	rtp->rectab[i][1] = 0 ;
	return i ;
}
/* end subroutine (rectab_done) */


static int rectab_getvec(RECTAB *rtp,uint (**rpp)[2])
{

	*rpp = rtp->rectab ;
	return rtp->i ;
}
/* end subroutine (rectab_getvec) */


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


static int indinsert(uint (*rt)[2],uint (*it)[3],int il,struct varentry *vep)
{
	uint		nhash, chash ;
	uint		ri, ki ;
	uint		lhi, nhi, hi ;
	int		c = 0 ;

	hi = vep->hi ;
	nhash = vep->khash ;
	chash = (nhash & INT_MAX) ;

#if	CF_DEBUGS
	debugprintf("indinsert: ve ri=%u ki=%u khash=%08X hi=%u\n",
	    vep->ri,vep->ki,vep->khash,vep->hi) ;
	debugprintf("indinsert: il=%u loop 1\n",il) ;
#endif

/* CONSTCOND */
	while (TRUE) {

#if	CF_DEBUGS
	    debugprintf("indinsert: it%u ri=%u nhi=%u\n",
	        hi,it[hi][0],it[hi][2]) ;
#endif

	    if (it[hi][0] == 0)
	        break ;

	    ri = it[hi][0] ;
	    ki = rt[ri][0] ;
	    if (ki == vep->ki)
	        break ;

	    it[hi][1] |= (~ INT_MAX) ;
	    nhash = hashagain(nhash,c++,VARMKS_NSKIP) ;

	    hi = hashindex(nhash,il) ;

#if	CF_DEBUGS
	    debugprintf("indinsert: nhash=%08X nhi=%u\n",nhash,hi) ;
#endif

	} /* end while */

	if (it[hi][0] > 0) {

#if	CF_DEBUGS
	    debugprintf("indinsert: loop 2\n") ;
#endif

	    lhi = hi ;
	    while ((nhi = it[lhi][2]) > 0)
	        lhi = nhi ;

	    hi = hashindex((lhi + 1),il) ;

#if	CF_DEBUGS
	    debugprintf("indinsert: loop 3 lhi=%u\n",lhi) ;
#endif

	    while (it[hi][0] > 0)
	        hi = hashindex((hi + 1),il) ;

	    it[lhi][2] = hi ;

#if	CF_DEBUGS
	    debugprintf("indinsert: loop 3 it%u ki=%u nhi=%u\n",lhi,
	        it[lhi][0],hi) ;
#endif

	} /* end if (same-key continuation) */

	it[hi][0] = vep->ri ;
	it[hi][1] = chash ;
	it[hi][2] = 0 ;

#if	CF_DEBUGS
	debugprintf("indinsert: ret hi=%u c=%u\n",hi,c) ;
#endif

	return c ;
}
/* end subroutine (indinsert) */


static int hashindex(uint i,int n)
{
	int	hi = MODP2(i,n) ;
	if (hi == 0) hi = 1 ;
	return hi ;
}
/* end subroutine (hashindex) */


