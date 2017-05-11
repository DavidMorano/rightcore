/* strlist */

/* read or audit a STRLIST database */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine opens and allows for reading or auditing of a STRLIST
	database (which currently consists of two files).

	Synopsis:

	int strlist_open(op,dbname)
	STRLIST		*op ;
	const char	dbname[] ;

	Arguments:

	- op		object pointer
	- dbname	name of (path-to) DB

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endianstr.h>
#include	<localmisc.h>

#include	"strlist.h"
#include	"strlisthdr.h"


/* local defines */

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	NATURALWORDLEN
#endif

#define	FE_VI		STRLISTHDR_FSUF

#define	SHIFTINT	(6 * 60)	/* possible time-shift */

#define	MODP2(v,n)	((v) & ((n) - 1))

#ifndef	MAXMAPSIZE
#define	MAXMAPSIZE	(512*1024*1024)
#endif


/* external subroutines */

extern uint	hashelf(const char *,int) ;
extern uint	hashagain(uint,int,int) ;

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	getpwd(char *,int) ;
extern int	hasuc(const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	timestr_log(time_t,char *) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* exported variables */

STRLIST_OBJ	strlist = {
	"strlist",
	sizeof(STRLIST),
	sizeof(STRLIST_CUR)
} ;


/* local structures */

enum itentries {
	itentry_ri,
	itentry_info,
	itentry_nhi,
	itentry_overlast
} ;


/* forward references */

static int	strlist_dbloadbegin(STRLIST *,time_t) ;
static int	strlist_dbloadend(STRLIST *) ;
static int	strlist_dbmapcreate(STRLIST *,time_t) ;
static int	strlist_dbmapdestroy(STRLIST *) ;
static int	strlist_filemapcreate(STRLIST *,STRLIST_FM *,cchar *,time_t) ;
static int	strlist_filemapdestroy(STRLIST *,STRLIST_FM *) ;
static int	strlist_dbproc(STRLIST *,time_t) ;
static int	strlist_viverify(STRLIST *,time_t) ;
static int	strlist_ouraudit(STRLIST *) ;

static int	hashindex(uint,int) ;
static int	ismatkey(const char *,const char *,int) ;


/* local variables */


/* exported subroutines */


int strlist_open(STRLIST *op,cchar dbname[])
{
	const time_t	daytime = time(NULL) ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("strlist_open: dbname=%s\n",dbname) ;
#endif

	if (dbname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(STRLIST)) ;

	{
	    int		pl = -1 ;
	    char	adb[MAXPATHLEN+1] ;
	    if (dbname[0] != '/') {
	        char	pwd[MAXPATHLEN+1] ;
	        if ((rs = getpwd(pwd,MAXPATHLEN)) >= 0) {
	            rs = mkpath2(adb,pwd,dbname) ;
	            pl = rs ;
	            dbname = adb ;
	        }
	    }
	    if (rs >= 0) {
	    	const char	*cp ;
	        if ((rs = uc_mallocstrw(dbname,pl,&cp)) >= 0) {
	            op->dbname = cp ;
		    if ((rs = strlist_dbloadbegin(op,daytime)) >= 0) {
			op->ti_lastcheck = daytime ;
			op->magic = STRLIST_MAGIC ;
		    }
		    if (rs < 0) {
	    		uc_free(op->dbname) ;
	    		op->dbname = NULL ;
		    }
		} /* end if (memory-allocation) */
	    } /* end if */
	} /* end block */

#if	CF_DEBUGS
	debugprintf("strlist_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (strlist_open) */


int strlist_close(STRLIST *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STRLIST_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("strlist_close: ent\n") ;
#endif

	rs1 = strlist_dbloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->dbname != NULL) {
	    rs1 = uc_free(op->dbname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbname = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (strlist_close) */


int strlist_info(STRLIST *op,STRLIST_INFO *vip)
{
	STRLIST_FM	*fip ;
	STRLISTHDR	*hip ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (vip == NULL) return SR_FAULT ;

	if (op->magic != STRLIST_MAGIC) return SR_NOTOPEN ;

	memset(vip,0,sizeof(STRLIST_INFO)) ;

	fip = &op->vf ;
	hip = &op->hdr ;

	vip->mtime = fip->ti_mod ;
	vip->wtime = (time_t) hip->wtime ;

	vip->nstrlist = hip->nstrs ;
	vip->nskip = hip->nskip ;

	return rs ;
}
/* end subroutine (strlist_info) */


int strlist_audit(STRLIST *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STRLIST_MAGIC) return SR_NOTOPEN ;

/* verify that all list pointers and list entries are valid */

	rs = strlist_ouraudit(op) ;

#if	CF_DEBUGS
	debugprintf("strlist_audit: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (strlist_audit) */


int strlist_count(STRLIST *op)
{
	STRLISTHDR	*hip ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STRLIST_MAGIC) return SR_NOTOPEN ;

	hip = &op->hdr ;
	return (rs >= 0) ? hip->nstrs : rs ;
}
/* end subroutine (strlist_count) */


int strlist_curbegin(STRLIST *op,STRLIST_CUR *curp)
{

#if	CF_DEBUGS
	debugprintf("strlist_curbegin: thinking about it\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != STRLIST_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("strlist_curbegin: ent\n") ;
#endif

	curp->i = 0 ;
	curp->chash = 0 ;
	op->ncursors += 1 ;

	return SR_OK ;
}
/* end subroutine (strlist_curbegin) */


int strlist_curend(STRLIST *op,STRLIST_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != STRLIST_MAGIC) return SR_NOTOPEN ;

	curp->i = 0 ;
	if (op->ncursors > 0)
	    op->ncursors -= 1 ;

	return SR_OK ;
}
/* end subroutine (strlist_curend) */


int strlist_look(STRLIST *op,STRLIST_CUR *curp,cchar *kp,int kl)
{
	STRLIST_CUR	dcur ;
	STRLIST_MI	*mip ;
	STRLISTHDR	*hip ;
	uint		khash, nhash, chash ;
	uint		hi ;
	uint		ki ;
	int		rs = SR_OK ;
	int		ri, c ;
	int		(*rt)[1] ;
	int		(*it)[3] ;
	int		vl = 0 ;
	int		f_mat = FALSE ;
	const char	*kst ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (op->magic != STRLIST_MAGIC) return SR_NOTOPEN ;

	if (curp == NULL) {
	    curp = &dcur ;
	    curp->i = 0 ;
	}

	if (kl < 0)
	    kl = strlen(kp) ;

	mip = &op->mi ;
	hip = &op->hdr ;

	kst = mip->kst ;
	rt = mip->rt ;
	it = mip->it ;

	if (curp->i <= 0) {

/* unhappy or not, the index-table uses same-hash-linking! */

	    khash = hashelf(kp,kl) ;

	    nhash = khash ;
	    chash = (khash & INT_MAX) ;
	    curp->chash = chash ;	/* store "check" hash */

	    hi = hashindex(khash,hip->itlen) ;

	    c = 0 ;
	    while ((ri = it[hi][itentry_ri]) > 0) {

	        f_mat = ((it[hi][itentry_info] & INT_MAX) == chash) ;
	        if (f_mat) {
	            ki = rt[ri][0] ;
	            cp = (kst + ki) ;
	            f_mat = (cp[0] == kp[0]) && ismatkey(cp,kp,kl) ;
	        }

	        if (f_mat)
	            break ;

	        if ((it[hi][itentry_info] & (~ INT_MAX)) == 0)
	            break ;

	        if (c >= (hip->itlen + hip->nskip))
	            break ;

	        nhash = hashagain(nhash,c++,hip->nskip) ;

	        hi = hashindex(nhash,hip->itlen) ;

	    } /* end while */

	    if ((rs >= 0) && (! f_mat))
	        rs = SR_NOTFOUND ;

	} else {

	    chash = curp->chash ;
	    hi = curp->i ;

	    if (hi < hip->itlen) {

	        ri = it[hi][itentry_ri] ;

	        if (ri > 0) {

	            hi = it[hi][itentry_nhi] ;

	            if (hi != 0) {

	                ri = it[hi][itentry_ri] ;
	                f_mat = ((it[hi][itentry_info] & INT_MAX) == chash) ;
	                if ((ri > 0) && f_mat) {
	                    ki = rt[ri][0] ;
	                    f_mat = ismatkey((kst + ki),kp,kl) ;
	                }

	                if (! f_mat)
	                    rs = SR_NOTFOUND ;

	            } else
	                rs = SR_NOTFOUND ;

	        } else
	            rs = SR_NOTFOUND ;

	    } else
	        rs = SR_NOTFOUND ;

	} /* end if (preparation) */

/* if successful, retrieve value */

	if (rs >= 0) {
	        curp->i = hi ;
	} /* end if (got one) */

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (strlist_look) */


int strlist_enum(STRLIST *op,STRLIST_CUR *curp,char kbuf[],int klen)
{
	STRLIST_MI	*mip ;
	STRLISTHDR	*hip ;
	uint		ri, ki ;
	int		rs = SR_OK ;
	const char	*kp ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (kbuf == NULL) return SR_FAULT ;

	if (op->magic != STRLIST_MAGIC) return SR_NOTOPEN ;

	if (op->ncursors == 0) return SR_INVALID ;

	kbuf[0] = '\0' ;

	mip = &op->mi ;
	hip = &op->hdr ;

	ri = (curp->i < 1) ? 1 : (curp->i + 1) ;

/* ok, we're good to go */

	if (ri < hip->rtlen) {
	    ki = mip->rt[ri][0] ;
	    if (ki < hip->stlen) {
	        kp = mip->kst + ki ;
	        if ((rs = sncpy1(kbuf,klen,kp)) >= 0)
	            curp->i = ri ;
	    } else
	        rs = SR_BADFMT ;
	} else
	    rs = SR_NOTFOUND ;

	return rs ;
}
/* end subroutine (strlist_enum) */


/* private subroutines */


static int strlist_dbloadbegin(STRLIST *op,time_t daytime)
{
	int		rs ;

	if ((rs = strlist_dbmapcreate(op,daytime)) >= 0) {
	    rs = strlist_dbproc(op,daytime) ;
	    if (rs < 0)
		strlist_dbmapdestroy(op) ;
	}

	return rs ;
}
/* end subroutine (strlist_dbloadbegin) */


static int strlist_dbloadend(STRLIST *op)
{
	int		rs ;

	if ((rs = strlist_dbmapdestroy(op)) >= 0) {
	    STRLIST_MI	*mip = &op->mi ;
	    mip->rt = NULL ;
	    mip->it = NULL ;
	    mip->kst = NULL ;
	}

	return rs ;
}
/* end subroutine (strlist_dbloadend) */


static int strlist_dbmapcreate(STRLIST *op,time_t daytime)
{
	int		rs ;
	const char	*end = ENDIANSTR ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if ((rs = mkfnamesuf2(tmpfname,op->dbname,FE_VI,end)) >= 0) {
	    rs = strlist_filemapcreate(op,&op->vf,tmpfname,daytime) ;
	}

	return rs ;
}
/* end subroutine (strlist_dbmapcreate) */


static int strlist_dbmapdestroy(STRLIST *op)
{
	int		rs ;

	rs = strlist_filemapdestroy(op,&op->vf) ;

	return rs ;
}
/* end subroutine (strlist_dbmapdestroy) */


static int strlist_filemapcreate(STRLIST *op,STRLIST_FM *fip,cchar fname[],
		time_t daytime)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = u_open(fname,O_RDONLY,0666)) >= 0) {
	    struct ustat	sb ;
	    int			fd = rs ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
	  	if (sb.st_size <= MAXMAPSIZE) {
	            size_t	ms = (size_t) sb.st_size ;
	            int		mp = PROT_READ ;
	            int		mf = MAP_SHARED ;
	            void	*md ;
	            if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	                fip->mdata = md ;
	                fip->msize = ms ;
	                fip->ti_mod = sb.st_mtime ;
	                fip->ti_map = daytime ;
	            }
		} else
		    rs = SR_TOOBIG ;
	    } /* end if (stat) */
	    u_close(fd) ;
	} /* end if (mapped file) */

	return rs ;
}
/* end subroutine (strlist_filemapcreate) */


static int strlist_filemapdestroy(STRLIST *op,STRLIST_FM *fip)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (fip->mdata != NULL) {
	    rs = u_munmap(fip->mdata,fip->msize) ;
	    fip->mdata = NULL ;
	    fip->msize = 0 ;
	    fip->ti_map = 0 ;
	}

	return rs ;
}
/* end subroutine (strlist_filemapdestroy) */


static int strlist_dbproc(STRLIST *op,time_t daytime)
{
	STRLIST_FM	*fip = &op->vf ;
	STRLIST_MI	*mip = &op->mi ;
	STRLISTHDR	*hip = &op->hdr ;
	int		rs ;

	if ((rs = strlisthdr(hip,1,fip->mdata,fip->msize)) >= 0) {
	    if ((rs = strlist_viverify(op,daytime)) >= 0) {
	        mip->rt = (int (*)[1]) (fip->mdata + hip->rtoff) ;
	        mip->it = (int (*)[3]) (fip->mdata + hip->itoff) ;
	        mip->kst = (char *) (fip->mdata + hip->stoff) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("strlist_dbproc: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (strlist_dbproc) */


static int strlist_viverify(STRLIST *op,time_t daytime)
{
	STRLIST_FM	*fip = &op->vf ;
	STRLISTHDR	*hip = &op->hdr ;
	uint		utime = (uint) daytime ;
	int		rs = SR_OK ;
	int		size ;
	int		f = TRUE ;

	f = f && (hip->fsize == fip->msize) ;

#if	CF_DEBUGS
	debugprintf("strlist_viverify: fsize=%u f=%u\n",
	    hip->fsize,f) ;
#endif

	f = f && (hip->wtime > 0) && (hip->wtime <= (utime + SHIFTINT)) ;

#if	CF_DEBUGS
	{
	    char	tbuf[TIMEBUFLEN + 1] ;
	    time_t	t = (time_t) hip->wtime ;
	    timestr_log(t,tbuf) ;
	    debugprintf("strlist_viverify: wtime=%s f=%u\n",tbuf,f) ;
	}
#endif

	f = f && (hip->stoff <= fip->msize) ;
	f = f && ((hip->stoff + hip->stlen) <= fip->msize) ;

	f = f && (hip->rtoff <= fip->msize) ;
	size = (hip->rtlen + 1) * 2 * sizeof(int) ;
	f = f && ((hip->rtoff + size) <= fip->msize) ;

	f = f && (hip->itoff <= fip->msize) ;
	size = (hip->itlen + 1) * 3 * sizeof(int) ;
	f = f && ((hip->itoff + size) <= fip->msize) ;

/* an extra (redundant) value */

	f = f && (hip->nstrs == (hip->rtlen - 1)) ;

#if	CF_DEBUGS
	debugprintf("strlist_viverify: nstrlist=%u rtlen=%u\n",
	    hip->nstrs,hip->rtlen) ;
#endif

/* get out */

	if (! f)
	    rs = SR_BADFMT ;

#if	CF_DEBUGS
	debugprintf("strlist_viverify: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (strlist_viverify) */


static int strlist_ouraudit(STRLIST *op)
{
	STRLIST_MI	*mip = &op->mi ;
	STRLISTHDR	*hip = &op->hdr ;
	uint		ri, ki, hi ;
	uint		khash, chash ;
	int		rs = SR_OK ;
	int		i ;
	int		cl ;
	int		(*rt)[1] ;
	int		(*it)[3] ;
	const char	*kst ;
	const char	*cp ;

	rt = mip->rt ;
	it = mip->it ;
	kst = mip->kst ;

/* record table */

	if ((rt[0][0] != 0) || (rt[0][1] != 0))
	    rs = SR_BADFMT ;

	for (ri = 1 ; (rs >= 0) && (ri < hip->rtlen) ; ri += 1) {

	    ki = rt[ri][0] ;
	    if (ki >= hip->stlen)
	        rs = SR_BADFMT ;

	    if (rs >= 0) {
	        cp = (kst + ki) ;
	        cl = strlen(cp) ;

	        if (cp[-1] != '\0')
	            rs = SR_BADFMT ;
	    }

	    if (rs >= 0) {
	        rs = strlist_look(op,NULL,cp,cl) ;
	    }

	    if (rs < 0) break ;
	} /* end for (record table entries) */

#if	CF_DEBUGS
	debugprintf("strlist_ouraudit: RT rs=%d\n",rs) ;
#endif

/* index table */

	if (rs >= 0) {
	    if ((it[0][0] != 0) || (it[0][1] != 0) || (it[0][2] != 0)) {
	        rs = SR_BADFMT ;
	    }
	}

	for (i = 1 ; (rs >= 0) && (i < hip->itlen) ; i += 1) {

	    if (it[i][0] != 0) {

	        ri = it[i][0] ;
	        hi = it[i][2] ;
	        if (ri >= hip->rtlen)
	            rs = SR_BADFMT ;

	        if ((rs >= 0) && (hi >= hip->itlen)) {
	            rs = SR_BADFMT ;
		}

	        if (rs >= 0) {

	            ki = rt[ri][0] ;
	            khash = hashelf((kst + ki),-1) ;

	            chash = (khash & INT_MAX) ;
	            if (chash != (it[i][1] & INT_MAX)) {
	                rs = SR_BADFMT ;
		    }

	        } /* end if */

	    } else {

	        if ((it[i][1] != 0) || (it[i][2] != 0)) {
	            rs = SR_BADFMT ;
		}

	    } /* end if */

	} /* end for (index table entries) */

#if	CF_DEBUGS
	debugprintf("strlist_ouraudit: IT rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (strlist_ouraudit) */


/* calculate the next hash from a given one */
static int hashindex(uint i,int n)
{
	int		hi = MODP2(i,n) ;
	if (hi == 0) hi = 1 ;
	return hi ;
}
/* end subroutine (hashindex) */


static int ismatkey(cchar *key,cchar *kp,int kl)
{
	int		m ;
	int		f = (key[0] == kp[0]) ;
	if (f) {
	    m = nleadstr(key,kp,kl) ;
	    f = (m == kl) && (key[m] == '\0') ;
	}
	return f ;
}
/* end subroutine (ismatkey) */


