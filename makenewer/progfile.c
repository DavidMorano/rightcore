/* progfile */

/* process a name */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The subroutine was written from scratch.


*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	We process each file individually.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<utime.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<storebuf.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	procfiler(struct proginfo *,int,struct ustat *,
			const char *,const char *) ;

static int	filesuf(struct proginfo *,vecstr *,char *,
			const char *,int) ;
static int	mknewfname(char *,const char *,const char *,const char *) ;


/* local variables */


/* exported subroutines */


int progfile(pip,dstdname,name)
struct proginfo	*pip ;
const char	dstdname[] ;
const char	name[] ;
{
	struct ustat	ssb ;

	int	rs = SR_OK ;
	int	of ;
	int	f_updated = FALSE ;

	const char	*fname ;

	char	mapfname[MAXPATHLEN + 1] ;


	if ((dstdname == NULL) || (name == NULL))
	    return SR_FAULT ;

	if ((name[0] == '\0') || (name[0] == '-'))
	    return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progfile: name=%s\n",name) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: name=%s\n",pip->progname,name) ;

/* form the actual file name to be processed after suffix mapping */

	fname = name ;
	rs = filesuf(pip,&pip->sufmaps,mapfname,name,-1) ;
	if (rs > 0)
	    fname = mapfname ;

	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progfile: fname=%s\n",fname) ;
#endif

/* continue with the processing operation */

	of = O_RDONLY ;
	if ((rs = uc_open(fname,of,0666)) >= 0) {
	    int	sfd = rs ;

	    rs = u_fstat(sfd,&ssb) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progfile: u_fstat() rs=%d fname=%s mode=%0o\n",
	            rs, fname,ssb.st_mode) ;
#endif

	    if ((rs >= 0) && S_ISREG(ssb.st_mode)) {
	        rs = procfiler(pip,sfd,&ssb,dstdname,fname) ;
		f_updated = (rs > 0) ;
	    }

	    u_close(sfd) ;
	} /* end if (source file opened) */

	if ((rs < 0) && pip->f.zero) {
	    if (isNotPresent(rs)) rs = SR_OK ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progfile: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_updated : rs ;
}
/* end subroutine (progfile) */


/* local subroutines */


int procfiler(pip,sfd,ssbp,dstdname,fname)
struct proginfo	*pip ;
int		sfd ;
struct ustat	*ssbp ;
const char	dstdname[] ;
const char	fname[] ;
{
	struct utimbuf	ut ;

	struct ustat	dsb ;

	offset_t	uoff ;

	int	rs = SR_OK ;
	int	bfnamelen ;
	int	of ;
	int	f_create = FALSE ;
	int	f_update = FALSE ;
	int	f_updated = FALSE ;
	int	f ;

	const char	*bfname ;

	char	dstfname[MAXPATHLEN + 2] ;
	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("procfiler: dstdname=%s\n",dstdname) ;
	    debugprintf("procfiler: fname=%s\n",fname) ;
	}
#endif

	ut.actime = ssbp->st_atime ;
	ut.modtime = ssbp->st_mtime ;

/* form the filename of the one in the directory */

	bfnamelen = sfbasename(fname,-1,&bfname) ;

	if (bfnamelen > 0) {

	    if (pip->f.rmsuf) {
	        const char	*tp = strnrchr(bfname,bfnamelen,'.') ;
	        int	bfl ;

	        bfl = (tp != NULL) ? (tp-bfname) : -1 ;
	        rs = mkpath2w(dstfname,dstdname,bfname,bfl) ;

	    } else {

	        rs = filesuf(pip,&pip->sufsubs,tmpfname,bfname,bfnamelen) ;
	        if (rs > 0) {
	            rs = mkpath2(dstfname,dstdname,tmpfname) ;
	        } else if (rs == 0)
	            rs = mkpath2w(dstfname,dstdname,bfname,bfnamelen) ;

	    } /* end if */

	} else
	    rs = SR_NOENT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("procfiler: mid rs=%d\n",rs) ;
	    debugprintf("procfiler: dstfname=%s\n",dstfname) ;
	}
#endif

	if (rs < 0) goto ret0 ;

/* continue */

	if ((rs = u_stat(dstfname,&dsb)) >= 0) {
	    f = S_ISREG(dsb.st_mode) ;

	    f_update = f && (ssbp->st_mtime > dsb.st_mtime) ;

	    if (f && pip->f.rmfile) {

	        f_create = TRUE ;
	        rs = u_unlink(dstfname) ;

	    } /* end if (deleted the file first) */

	} else if (rs == SR_NOENT) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("procfiler: no entry for source \n") ;
#endif

	    rs = SR_OK ;
	    f_create = TRUE ;
	    f_update = TRUE ;
	    dsb.st_mtime = 0 ;
	    dsb.st_size = 0 ;

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procfiler: source rs=%d f_update=%u f_create=%u\n",
	        rs,f_update,f_create) ;
#endif

	if (rs < 0) goto ret0 ;
	if (! (f_update || f_create)) goto ret0 ;

/* update (or create) the target file */

	of = O_WRONLY ;
	if (f_create) of |= O_CREAT ;

	if ((rs = uc_open(dstfname,of,0666)) >= 0) {
	    int	dfd = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procfiler: uc_open() rs=%d\n",rs) ;
#endif

	    if (f_create) {
	        dsb.st_mtime = 0 ;
	        dsb.st_size = 0 ;
	        f_update = TRUE ;
	        rs = u_fchmod(dfd,ssbp->st_mode) ;
	    } /* end if (needed to be created) */

/* do we need to UPDATE the destination? */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("procfiler: need update? rs=%d f_update=%d\n",
	            rs,f_update) ;
#endif

	    if ((rs >= 0) && f_update) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("procfiler: updating fname=%s\n",
	                fname) ;
#endif

	        if (pip->f.print || (pip->verboselevel > 1))
	            bprintf(pip->ofp,"%s\n",fname) ;

	        if ((! pip->f.nochange) && (! pip->f.print)) {
		    int	len ;

	            rs = uc_copy(sfd,dfd,-1) ;
	            len = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("procfiler: uc_copy() rs=%d\n",rs) ;
#endif

	            if ((rs >= 0) && (len < dsb.st_size)) {
	                uoff = len ;
	                rs = uc_ftruncate(dfd,uoff) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("procfiler: uc_ftruncate() rs=%d\n",rs) ;
#endif

	            }

	            if (rs >= 0) {
	                f_updated = TRUE ;
	                u_utime(dstfname,&ut) ;
	            }

	        } /* end if (allowable actual update) */

	    } /* end if (update was needed) */

	    u_close(dfd) ;
	} /* end if (destination file opened) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procfiler: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_updated : rs ;
}
/* end subroutine (procfiler) */


static int filesuf(pip,slp,newfname,fname,fnamelen)
struct proginfo	*pip ;
vecstr		*slp ;
char		newfname[] ;
const char	fname[] ;
int		fnamelen ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	sl ;
	int	bnl ;
	int	fl = 0 ;

	const char	*tp, *sp ;
	const char	*bnp ;


#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progfile/filesuf: fname=%t\n",
	        fname,fnamelen) ;
#endif

	newfname[0] = '\0' ;
	bnl = sfbasename(fname,fnamelen,&bnp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progfile/filesuf: bname=%t\n", bnp,bnl) ;
#endif

	if (bnl > 0) {

	    if ((tp = strnrchr(bnp,bnl,'.')) != NULL) {

	        sl = ((bnp + bnl) - (tp + 1)) ;
	        sp = (tp + 1) ;

	        {
	            char	sufbuf[SUFLEN + 1] ; /* could be variable */
	            const char	*cp ;

	            rs1 = snwcpy(sufbuf,SUFLEN,sp,sl) ;

	            if (rs1 >= 0)
	                rs1 = vecstr_search(slp,sufbuf,vstrkeycmp,&cp) ;

	            if (rs1 >= 0) {
	                rs = mknewfname(newfname,fname,tp,cp) ;
	                fl = rs ;
	            }

	        } /* end block */

	    } /* end if */

	} /* end if */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    if (rs >= 0)
	        debugprintf("progfile/filesuf: newfname=%s\n",newfname) ;
	    debugprintf("progfile/filesuf: ret rs=%d fl=%u\n",rs,fl) ;
	}
#endif

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (filesuf) */


static int mknewfname(newfname,fname,sp,cp)
char		newfname[] ;
const char	fname[] ;
const char	*sp ;
const char	*cp ;
{
	int	rs ;
	int	buflen = MAXPATHLEN ;
	int	i = 0 ;

	const char	*tp ;


	rs = storebuf_strw(newfname,buflen,i,fname,(sp - fname)) ;
	i += rs ;

	if ((rs >= 0) && ((tp = strchr(cp,'=')) != NULL)) {

	    if ((tp+1)[0] != '\0') {
	        rs = storebuf_char(newfname,buflen,i,'.') ;
	        i += rs ;
	        if (rs >= 0) {
	            rs = storebuf_strw(newfname,buflen,i,(tp + 1),-1) ;
	            i += rs ;
	        }
	    }

	} /* end if (adding new suffix) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mknewfname) */



