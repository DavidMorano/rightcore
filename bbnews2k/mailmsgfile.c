/* mailmsgfile */

/* maintain translations for MSGID to filenames */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2009-01-20, David A­D­ Morano
	This is new code.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object implements a translation mapping from message-ids (MSGIDs)
        to unique temporary filenames. Although not our business, these
        filenames point to files that hold the content (body) of mail messages.

	Implementation notes:

	+ Why the child process on exit?

        Because deleting files is just way too slow -- for whatever reason. So
        we have a child do it do that we can finish up *fast* and get out. This
        is a user response-time issue. We want the user to have the program exit
        quickly so that they are not annoyed (as they were when we previously
        deleted all of the files inline).


*******************************************************************************/


#define	MAILMSGFILE_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<hdb.h>
#include	<filebuf.h>
#include	<linefold.h>
#include	<char.h>
#include	<fsdir.h>
#include	<vecstr.h>
#include	<vecpstr.h>
#include	<upt.h>
#include	<localmisc.h>
#include	<exitcodes.h>

#include	"mailmsgfile.h"


/* local defines */

#define	MAILMSGFILE_MINCOLS	38
#define	MAILMSGFILE_DEFIND	4
#define	MAILMSGFILE_FILEINT	(2 * 24 * 3600)

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	TABCOLS
#define	TABCOLS		8
#endif

#ifndef	VARCOLUMNS
#define	VARCOLUMNS	"COLUMNS"
#endif

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#define	INBUFLEN	LINEBUFLEN
#define	OUTBUFLEN	(INBUFLEN * 2)


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	pathadd(char *,int,const char *) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	mkdisplayable(char *,int,const char *,int) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	msleep(int) ;
extern int	nlinecols(int,int,const char *,int) ;
extern int	filebuf_writeblanks(FILEBUF *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpyblanks(char *,int) ;
extern char	*strncpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnrpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */

struct dargs {
	HDB		*mp ;
	const char	*tmpdname ;
} ;


/* forward references */

static int mailmsgfile_mk(MAILMSGFILE *,const char *,const char *,int,
			offset_t,int) ;
static int mailmsgfile_mkdis(MAILMSGFILE *,MAILMSGFILE_MI *,
			int,int,offset_t) ;
static int mailmsgfile_proclines(MAILMSGFILE *,MAILMSGFILE_MI *,
			FILEBUF *,const char *,int) ;

static int mailmsgfile_procout(MAILMSGFILE *,FILEBUF *,int,const char *,int,
			int) ;
static int mailmsgfile_store(MAILMSGFILE *,MAILMSGFILE_MI *) ;
static int mailmsgfile_filefins(MAILMSGFILE *) ;

static int mailmsgfile_checkbegin(MAILMSGFILE *) ;
static int mailmsgfile_checkend(MAILMSGFILE *) ;
static int mailmsgfile_checkout(MAILMSGFILE *) ;
static int mailmsgfile_checker(MAILMSGFILE *) ;
static int mailmsgfile_checkerx(MAILMSGFILE *) ;

static int mi_start(MAILMSGFILE_MI *,const char *,const char *,int) ;
static int mi_finish(MAILMSGFILE_MI *) ;
static int mi_newlines(MAILMSGFILE_MI *,int) ;


/* local variables */


/* exported subroutines */


int mailmsgfile_start(op,tmpdname,cols,ind)
MAILMSGFILE	*op ;
const char	tmpdname[] ;
int		cols ;
int		ind ;
{
	int		rs = SR_OK ;
	int		n = 30 ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;

	if (tmpdname == NULL) return SR_INVALID ;

	if (cols < MAILMSGFILE_MINCOLS) {
	    if ((cp = getenv(VARCOLUMNS)) != NULL) {
	        uint	v ;
	        int	rs1 = cfdecui(cp,-1,&v) ;
	        cols = (rs1 >= 0) ? v : 0 ;
	    }
	    if (cols < 2)
	        cols = COLUMNS ;
	} /* end if */

	if (ind < 0) ind = MAILMSGFILE_DEFIND ;

	memset(op,0,sizeof(MAILMSGFILE)) ;
	op->cols = cols ;
	op->ind = ind ;
	op->pagesize = getpagesize() ;

	if ((rs = uc_mallocstrw(tmpdname,-1,&cp)) >= 0) {
	    op->tmpdname = cp ;
	    if ((rs = hdb_start(&op->files,n,TRUE,NULL,NULL)) >= 0) {
	        op->f.files = TRUE ;
	        if ((rs = mailmsgfile_checkbegin(op)) >= 0) {
	            op->magic = MAILMSGFILE_MAGIC ;
	        }
	    }
	    if (rs < 0) {
	        uc_free(op->tmpdname) ;
	        op->tmpdname = NULL ;
	    }
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (mailmsgfile_start) */


int mailmsgfile_finish(op)
MAILMSGFILE	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILMSGFILE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("mailmsgfile_finish: ent\n") ;
#endif

	rs1 = mailmsgfile_checkend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = mailmsgfile_filefins(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->f.files) {
	    op->f.files = FALSE ;
	    rs1 = hdb_finish(&op->files) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->tmpdname != NULL) {
	    rs1 = uc_free(op->tmpdname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->tmpdname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("mailmsgfile_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (mailmsgfile_finish) */


int mailmsgfile_new(op,type,msgid,mfd,boff,blen)
MAILMSGFILE	*op ;
int		type ;
const char	msgid[] ;
int		mfd ;
offset_t	boff ;
int		blen ;
{
	HDB_DATUM	key, val ;
	int		rs ;
	int		vlines = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (msgid == NULL) return SR_FAULT ;

	if (op->magic != MAILMSGFILE_MAGIC) return SR_NOTOPEN ;

	if (type < 0) return SR_INVALID ;
	if (msgid[0] == NULL) return SR_INVALID ;
	if (boff < 0) return SR_INVALID ;
	if (blen < 0) return SR_INVALID ;

	if (mfd < 0) return SR_BADF ;

#if	CF_DEBUGS
	debugprintf("mailmsgfile_new: blen=%d\n",blen) ;
	debugprintf("mailmsgfile_new: mid=%s\n",msgid) ;
#endif

	key.buf = msgid ;
	key.len = strlen(msgid) ;
	if ((rs = hdb_fetch(&op->files,key,NULL,&val)) >= 0) {
	    MAILMSGFILE_MI	*mip = (MAILMSGFILE_MI *) val.buf ;
	    vlines = mip->vlines ;
	} else if (rs == SR_NOTFOUND) {
	    const char	*template = "msgXXXXXXXXXXX" ;
	    char	infname[MAXPATHLEN + 1] ;
	    if ((rs = mkpath2(infname,op->tmpdname,template)) >= 0) {
	        rs = mailmsgfile_mk(op,msgid,infname,mfd,boff,blen) ;
	        vlines = rs ;
	    }
	} /* end if (found or not) */

#if	CF_DEBUGS
	debugprintf("mailmsgfile_new: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? vlines : rs ;
}
/* end subroutine (mailmsgfile_new) */


int mailmsgfile_msginfo(op,mipp,msgid)
MAILMSGFILE	*op ;
MAILMSGFILE_MI	**mipp ;
const char	msgid[] ;
{
	int		rs ;
	int		vlines = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (msgid == NULL) return SR_FAULT ;
	if (mipp == NULL) return SR_FAULT ;

	if (op->magic != MAILMSGFILE_MAGIC) return SR_NOTOPEN ;

	if (msgid[0] == '\0') return SR_INVALID ;

	if ((rs = mailmsgfile_checkout(op)) >= 0) {
	    HDB_DATUM	key, val ;
	    key.buf = msgid ;
	    key.len = strlen(msgid) ;
	    if ((rs = hdb_fetch(&op->files,key,NULL,&val)) >= 0) {
	        *mipp = (MAILMSGFILE_MI *) val.buf ;
	        vlines = (*mipp)->vlines ;
	    }
	} /* end if (checkout) */

	return (rs >= 0) ? vlines : rs ;
}
/* end subroutine (mailmsgfile_msginfo) */


int mailmsgfile_get(op,msgid,rpp)
MAILMSGFILE	*op ;
const char	msgid[] ;
const char	**rpp ;
{
	MAILMSGFILE_MI	*mip ;
	int		rs ;
	int		vlines = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (msgid == NULL) return SR_FAULT ;

	if (op->magic != MAILMSGFILE_MAGIC) return SR_NOTOPEN ;

	if (msgid[0] == NULL) return SR_INVALID ;

	if ((rs = mailmsgfile_msginfo(op,&mip,msgid)) >= 0) {
	    vlines = mip->vlines ;
	    if (rpp != NULL)
	        *rpp = (const char *) mip->mfname ;
	}

	return (rs >= 0) ? vlines : rs ;
}
/* end subroutine (mailmsgfile_get) */


/* private subroutines */


static int mailmsgfile_mk(op,msgid,infname,mfd,boff,blen)
MAILMSGFILE	*op ;
const char	msgid[] ;
const char	infname[] ;
int		mfd ;
offset_t	boff ;
int		blen ;
{
	MAILMSGFILE_MI	mi ;
	const mode_t	omode = 0666 ;
	const int	oflags = (O_WRONLY | O_CREAT) ;
	int		rs ;
	int		vlines = 0 ;
	char		mfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("mailmsgfile_mk: mid=%s \n",msgid) ;
	debugprintf("mailmsgfile_mk: infname=%s\n",infname) ;
	debugprintf("mailmsgfile_mk: mfd=%d\n",mfd) ;
#endif

	mfname[0] = '\0' ;
	if ((rs = opentmpfile(infname,oflags,omode,mfname)) >= 0) {
	    int	tfd = rs ;
	    if ((rs = u_seek(mfd,boff,SEEK_SET)) >= 0) {
	        if ((rs = mi_start(&mi,msgid,mfname,blen)) >= 0) {
	            if ((rs = mailmsgfile_mkdis(op,&mi,tfd,mfd,boff)) >= 0) {
	                vlines = rs ;
	                rs = mailmsgfile_store(op,&mi) ;
	            }
	            if (rs < 0)
	                mi_finish(&mi) ;
	        } /* end if (mi) */
	    } /* end if (seek) */
	    u_close(tfd) ;
	    if (rs < 0)
	        u_unlink(mfname) ;
	} /* end if (tmpfile) */

#if	CF_DEBUGS
	debugprintf("mailmsgfile_mk: ret rs=%d lines=%u\n",rs,vlines) ;
#endif

	return (rs >= 0) ? vlines : rs ;
}
/* end subroutine (mailmsgfile_mk) */


static int mailmsgfile_filefins(op)
MAILMSGFILE	*op ;
{
	HDB		*mp = &op->files ;
	HDB_DATUM	key, val ;
	HDB_CUR		cur ;
	int		rs = SR_OK ;
	int		rs1 ;

	if ((rs1 = hdb_curbegin(mp,&cur)) >= 0) {
	    MAILMSGFILE_MI	*mip ;
	    while (hdb_enum(mp,&cur,&key,&val) >= 0) {
	        mip = (MAILMSGFILE_MI *) val.buf ;
	        if (mip != NULL) {
	            rs1 = mi_finish(mip) ;
	            if (rs >= 0) rs = rs1 ;
	            rs1 = uc_free(mip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if */
	    } /* end while */
	    hdb_curend(mp,&cur) ;
	} /* end if (cursor) */
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (mailmsgfile_filefins) */


static int mailmsgfile_mkdis(op,mip,tfd,mfd,boff)
MAILMSGFILE	*op ;
MAILMSGFILE_MI	*mip ;
offset_t	boff ;
int		tfd ;
int		mfd ;
{
	FILEBUF		in ;
	const int	ntab = TABCOLS ;
	const int	blen = mip->nsize ;
	int		rs ;
	int		inlen ;
	int		ibsize ;
	int		vlines = 0 ;
	int		wlen = 0 ;

	ibsize = MIN(blen,(op->pagesize*8)) ;
	if ((rs = filebuf_start(&in,mfd,boff,ibsize,0)) >= 0) {
	    FILEBUF	out ;
	    const int	obsize = rs ;
	    int		rlen = blen ;

	    if ((rs = filebuf_start(&out,tfd,0L,obsize,0)) >= 0) {
	        const int	llen = LINEBUFLEN ;
		int		ncols ;
	        int		ll ;
	        char		lbuf[INBUFLEN + 1] ;

	        while ((rs >= 0) && (rlen > 0)) {
	            rs = filebuf_readline(&in,lbuf,llen,0) ;
	            inlen = rs ;
	            if (rs <= 0) break ;

#if	CF_DEBUGS
	            debugprintf("mailmsgfile_mkdis: line=>%t<\n",
	                lbuf,strlinelen(lbuf,inlen,45)) ;
#endif

	            ll = inlen ;
	            if (lbuf[ll-1] == '\n') {
	                mip->nlines += 1 ;
	                ll -= 1 ;
	            }

	            while ((ll > 0) && CHAR_ISWHITE(lbuf[ll-1]))
	                ll -= 1 ;

/* calculate the number of columns this line would take up */

	            ncols = nlinecols(ntab,0,lbuf,ll) ;

/* take action based on whether the line fits in the available columns */

	            if (ncols > op->cols) {
	                rs = mailmsgfile_proclines(op,mip,&out,lbuf,ll) ;
	                wlen += rs ;
	            } else {
	                if ((rs = mi_newlines(mip,1)) >= 0) {
	                    rs = mailmsgfile_procout(op,&out,0,lbuf,ll,FALSE) ;
	                    wlen += rs ;
	                }
	            } /* end if (what kind of printing) */

	            rlen -= inlen ;

	        } /* end while (reading lines) */

	        if (rs >= 0) {
	            mip->vsize = wlen ;
	        }

	        filebuf_finish(&out) ;
	    } /* end if (filebuf) */

	    filebuf_finish(&in) ;
	} /* end if (filebuf) */

	return (rs >= 0) ? vlines : rs ;
}
/* end subroutine (mailmsgfile_mkdis) */


static int mailmsgfile_proclines(op,mip,fbp,lbuf,llen)
MAILMSGFILE	*op ;
MAILMSGFILE_MI	*mip ;
FILEBUF		*fbp ;
const char	lbuf[] ;
int		llen ;
{
	LINEFOLD	ff ;
	const int	c = op->cols ;
	const int	i = op->ind ;
	int		rs ;
	int		wlen = 0 ;

	if ((rs = linefold_start(&ff,c,i,lbuf,llen)) >= 0) {
	    int		li = 0 ;
	    int		plen ;
	    int		sl ;
	    int		f ;
	    const char	*sp ;

	    while ((rs = linefold_getline(&ff,li,&sp)) > 0) {
		sl = rs ;

#if	CF_DEBUGS
	        debugprintf("mailmsgfile_mkdis: "
	            "frag=>%t<\n",
	            sp,strlinelen(sp,sl,45)) ;
#endif

	        while ((rs >= 0) && (sl > 0)) {

	            f = FALSE ;
	            plen = sl ;
	            if (plen > op->cols) {
	                f = TRUE ;
	                plen = (op->cols - 1) ;
	            }

	            if ((rs = mi_newlines(mip,1)) >= 0) {
	                rs = mailmsgfile_procout(op,fbp,li,sp,plen,f) ;
	                wlen += rs ;
	            }

	            sp += plen ;
	            sl -= plen ;

	        } /* end while */

	        li += 1 ;
	        if (rs < 0) break ;
	    } /* end while (reading folded lines) */

	    linefold_finish(&ff) ;
	} /* end if (linefold) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mailmsgfile_proclines) */


static int mailmsgfile_procout(op,fbp,li,lp,ll,f_cont)
MAILMSGFILE	*op ;
FILEBUF		*fbp ;
int		li ;
const char	*lp ;
int		ll ;
int		f_cont ;
{
	const int	ind = op->ind ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	int		f_eol = FALSE ;

	if ((li > 0) && (ind > 0)) {
	    rs = filebuf_writeblanks(fbp,ind) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    const int	outlen = OUTBUFLEN ;
	    char	outbuf[OUTBUFLEN + 1] ;

	    f_eol = ((ll > 0) && (lp[ll-1] == '\n')) ;
	    if ((rs = mkdisplayable(outbuf,outlen,lp,ll)) >= 0) {
	        int	olen = rs ;

	        if (f_cont) {

	            if ((olen > 0) && (outbuf[olen-1] == '\n'))
	                olen -= 1 ;

	            rs = filebuf_write(fbp,outbuf,olen) ;
	            wlen += rs ;
	            if (rs >= 0) {
	                f_eol = TRUE ;
	                outbuf[0] = '¬' ;
	                outbuf[1] = '\n' ;
	                rs = filebuf_write(fbp,outbuf,2) ;
	                wlen += rs ;
	            }

	        } else {
	            f_eol = TRUE ;
	            rs = filebuf_print(fbp,outbuf,olen) ;
	            wlen += rs ;
	        }

	    } /* end if (mkdisplayable) */

	    if ((rs >= 0) && (! f_eol)) {
	        outbuf[0] = '\n' ;
	        rs = filebuf_write(fbp,outbuf,1) ;
	        wlen += rs ;
	    }

	} /* end if (ok) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mailmsgfile_procout) */


static int mailmsgfile_store(op,mip)
MAILMSGFILE	*op ;
MAILMSGFILE_MI	*mip ;
{
	MAILMSGFILE_MI	*ep ;
	int		rs ;
	int		size ;

	if (mip == NULL) return SR_NOANODE ;

	size = sizeof(MAILMSGFILE_MI) ;
	if ((rs = uc_malloc(size,&ep)) >= 0) {
	    HDB_DATUM	key, val ;
	    *ep = *mip ; /* copy */
	    key.buf = ep->mid ;
	    key.len = strlen(ep->mid) ;
	    val.buf = ep ;
	    val.len = size ;
	    rs = hdb_store(&op->files,key,val) ;
	    if (rs < 0)
	        uc_free(ep) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (mailmsgfile_store) */


static int mailmsgfile_checkbegin(MAILMSGFILE *op)
{
	pthread_t	tid ;
	int		rs ;
	int	(*thrsub)(void *) = (int (*)(void *)) mailmsgfile_checker ;

	if ((rs = uptcreate(&tid,NULL,thrsub,op)) >= 0) {
	    op->tid = tid ;
	    op->f.checkout = TRUE ;
	}

	return rs ;
}
/* end subroutine (mailmsgfile_checkbegin) */


static int mailmsgfile_checkend(MAILMSGFILE *op)
{
	int		rs = SR_OK ;

	if (op->f.checkout) {
	    int	trs = SR_OK ;
	    if ((rs = uptjoin(op->tid,&trs)) >= 0) {
	        op->f.checkout = FALSE ;
	        rs = trs ;
	    }
	}

	return rs ;
}
/* end subroutine (mailmsgfile_checkend) */


static int mailmsgfile_checkout(MAILMSGFILE *op)
{
	int		rs = SR_OK ;

	if (op->f.checkout && op->f_checkdone) {
	    int		trs = SR_OK ;
	    if ((rs = uptjoin(op->tid,&trs)) >= 0) {
	        op->f.checkout = FALSE ;
	        rs = trs ;
	    }
	}

	return rs ;
}
/* end subroutine (mailmsgfile_checkout) */


static int mailmsgfile_checker(MAILMSGFILE *op)
{
	int		rs = SR_OK ;

	if (op->tmpdname != NULL) {
	    if (op->tmpdname[0] != '\0') {
		msleep(10) ; /* some time for real work to happen elsewhere */
		rs = mailmsgfile_checkerx(op) ;
	    } else {
	        rs = SR_INVALID ;
	    }
	} else {
	    rs = SR_FAULT ;
	}

	op->f_checkdone = TRUE ;
	return rs ;
}
/* end subroutine (mailmsgfile_checker) */


static int mailmsgfile_checkerx(MAILMSGFILE *op)
{
	const time_t	dt = time(NULL) ;
	const int	to = MAILMSGFILE_FILEINT ;
	int		rs ;
	int		rs1 ;
	char		pbuf[MAXPATHLEN+1] ;
	if ((rs = mkpath1(pbuf,op->tmpdname)) >= 0) {
	    vecpstr	files ;
	    int		plen = rs ;
	    if ((rs = vecpstr_start(&files,10,100,0)) >= 0) {
	        FSDIR		dir ;
	        FSDIR_ENT	de ;

	        if ((rs = fsdir_open(&dir,op->tmpdname)) >= 0) {
	            struct ustat	sb ;
	            int			pl ;

	            while ((rs = fsdir_read(&dir,&de)) > 0) {
	                if (de.name[0] != '.') {

	                if ((rs = pathadd(pbuf,plen,de.name)) >= 0) {
	                    pl = rs ;
	                    if (u_stat(pbuf,&sb) >= 0) {
	                        if ((dt - sb.st_mtime) >= to) {
	                            rs = vecpstr_add(&files,pbuf,pl) ;
	                        }
	                    } /* end if (stat) */
	                } /* end if (pathadd) */

		        }
	                if (rs < 0) break ;
	            } /* end while (reading entries) */

	            rs1 = fsdir_close(&dir) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (fsdir) */

	        if (rs >= 0) {
	            int		i ;
	            const char	*fp ;
	            for (i = 0 ; vecpstr_get(&files,i,&fp) >= 0 ; i += 1) {
	                if (fp != NULL) {
	                    if (fp[0] != '\0') {
	                        u_unlink(fp) ;
	                    }
			}
	            } /* end for */
	        } /* end if */

	        rs1 = vecpstr_finish(&files) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (vecpstr) */
	} /* end if (mkpath) */
	return rs ;
}
/* end subroutine (mailmsgfile_checkerx) */


static int mi_start(mip,msgid,mfname,blen)
MAILMSGFILE_MI	*mip ;
const char	msgid[] ;
const char	mfname[] ;
int		blen ;
{
	int		rs ;
	int		milen, mflen ;
	int		size = 0 ;
	char		*bp = NULL ;

	if (mip == NULL) return SR_FAULT ;

	memset(mip,0,sizeof(MAILMSGFILE_MI)) ;

	milen = strlen(msgid) ;
	mflen = strlen(mfname) ;
	size = (milen + 1 + mflen + 1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    mip->a = bp ;
	    mip->nsize = blen ;
	    mip->mid = bp ;
	    bp = (strwcpy(bp,msgid,milen) + 1) ;
	    mip->mfname = bp ;
	    bp = (strwcpy(bp,mfname,mflen) + 1) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (mi_start) */


static int mi_finish(mip)
MAILMSGFILE_MI	*mip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mip == NULL) return SR_FAULT ;

	if ((mip->mfname != NULL) && (mip->mfname[0] != '\0')) {
	    rs1 = u_unlink(mip->mfname) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (mip->a != NULL) {
	    rs1 = uc_free(mip->a) ;
	    if (rs >= 0) rs = rs1 ;
	    mip->a = NULL ;
	}

	mip->mid = NULL ;
	mip->mfname = NULL ;
	mip->nsize = 0 ;
	mip->vsize = 0 ;
	mip->vlines = 0 ;
	return rs ;
}
/* end subroutine (mi_finish) */


static int mi_newlines(MAILMSGFILE_MI *mip,int n)
{
	mip->vlines += n ;
	return SR_OK ;
}
/* end subroutine (mi_newlines) */


