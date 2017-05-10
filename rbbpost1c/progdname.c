/* progdname */

/* process a directory */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1996-03-01, David A­D­ Morano
        The subroutine was adapted from others programs that did similar types
        of functions.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module traverses the given directory and creates the Directory
        Cache (DIRCACHE) file from the traversal.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<nulstr.h>
#include	<vecpstr.h>
#include	<bfile.h>
#include	<fsdirtree.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	DIRCACHE_MAGIC	"DIRCACHE"


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	strwcmp(const char *,const char *,int) ;
extern int	isOneOf(const int *,int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

extern int	proglog_printf(PROGINFO *,cchar *,...) ;

extern int	progpcsuid(struct proginfo *) ;
extern int	progpcsgid(struct proginfo *) ;


/* external variables */


/* local structures */


/* forward references */

static int procdiffer(struct proginfo *,VECPSTR *,const char *) ;
static int procdircache(struct proginfo *,bfile *,VECPSTR *,const char *) ;
static int vecpstr_loaddirs(VECPSTR *,const char *) ;


/* local variables */


/* exported subroutines */


int progdname(PROGINFO *pip,bfile *ofp,cchar *np,int nl)
{
	NULSTR		nd ;
	VECPSTR		dirs ;
	int		rs ;
	int		c = 0 ;
	int		f_updated = FALSE ;
	const char	*newsdname ;

	if (np == NULL) return SR_FAULT ;
	if (np[0] == '\0') return SR_INVALID ;

	if ((rs = nulstr_start(&nd,np,nl,&newsdname)) >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progdname: dname=%s\n",newsdname) ;
#endif

	if ((rs = vecpstr_start(&dirs,25,0,0)) >= 0) {
	    if ((rs = vecpstr_loaddirs(&dirs,newsdname)) >= 0) {
	        c = rs ;
	        if ((rs = procdiffer(pip,&dirs,newsdname)) > 0) {
	            f_updated = TRUE ;
	            rs = procdircache(pip,ofp,&dirs,newsdname) ;
	        }
	        if ((rs >= 0) && (pip->verboselevel >= 2)) {
		    int		i ;
		    const char	*dp ;
		    for (i = 0 ; vecpstr_get(&dirs,i,&dp) >= 0 ; i += 1) {
			if (dp == NULL) continue ;
	                rs = bprintline(ofp,dp,-1) ;
			if (rs < 0) break ;
		    } /* end for */
		} /* end if (verbosity) */
	    }
	    vecpstr_finish(&dirs) ;
	} /* end if (vecpstr-dirs) */

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    const char	*pn = pip->progname ;
	    const char	*fmt = "%s: dir=%s updated=%u\n" ;
	    bprintf(pip->efp,fmt,pn,newsdname,f_updated) ;
	} /* end if */

	if ((rs >= 0) && (pip->open.logprog> 0)) {
	    const char	*fmt = "dir=%s updated=%u" ;
	    proglog_printf(pip,fmt,newsdname,f_updated) ;
	} /* end if */

	    nulstr_finish(&nd) ;
	} /* end if (nulstr-newsdname) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (progdname) */


/* local subroutines */


static int procdiffer(pip,dlp,newsdname)
struct proginfo	*pip ;
VECPSTR		*dlp ;
const char	newsdname[] ;
{
	const int	ml = strlen(DIRCACHE_MAGIC) ;
	int		rs ;
	int		f = TRUE ;
	const char	*dc = DIRCACHEFNAME ;
	char		dcfname[MAXPATHLEN+1] ;

	if ((rs = vecpstr_getsize(dlp)) >= 0) {
	    int	dsize = rs ;
	    if ((rs = mkpath2(dcfname,newsdname,dc)) >= 0) {
	        struct ustat	sb ;
	        if ((rs = uc_stat(dcfname,&sb)) >= 0) {
	            bfile	cfile, *cfp = &cfile ;
	            int	fsize = (sb.st_size & INT_MAX) ;
	            if (dsize == (fsize-ml-1)) {
	                if ((rs = bopen(cfp,dcfname,"r",0666)) >= 0) {
	                    const int	dlen = MAXPATHLEN ;
	                    int		line = 0 ;
	                    int		f_mis = FALSE ;
	                    const char	*dp ;
	                    char	dbuf[MAXPATHLEN+1] ;
	                    while ((rs = breadline(cfp,dbuf,dlen)) > 0) {
	                        int	dl = rs ;
	                        if (line > 0) {
	                            if (dbuf[dl-1] == '\n') dl -= 1 ;
	                            if (vecpstr_get(dlp,(line-1),&dp) >= 0) {
	                                f_mis = (strwcmp(dp,dbuf,dl) != 0) ;
	                            } else
	                                f_mis = TRUE ;
	                        }
	                        line += 1 ;
	                        if (f_mis) break ;
	                    } /* end while */
	                    if (! f_mis) f = FALSE ;
	                    bclose(cfp) ;
	                } /* end if (file-open) */
	            } /* end if (sizes were the same) */
	        } else if (isNotAccess(rs))
	            rs = SR_OK ;
	    } /* end if */
	} /* end if (vecpstr-getsize) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procdiffer) */


static int procdircache(pip,ofp,dlp,newsdname)
struct proginfo	*pip ;
bfile		*ofp ;
VECPSTR		*dlp ;
const char	newsdname[] ;
{
	bfile		dcfile, *dcfp = &dcfile ;
	uid_t		uid_pcs ;
	gid_t		gid_pcs ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	int		f_chown = FALSE ;
	char		dcfname[MAXPATHLEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		template[MAXPATHLEN + 1] ;

	if ((rs = progpcsuid(pip)) >= 0) {
	    uid_pcs = rs ;
	    if ((rs = progpcsgid(pip)) >= 0) {
		gid_pcs = rs ;
	    }
	}
	if (rs < 0) goto ret0 ;

	f_chown = ((gid_pcs >= 0) && (gid_pcs != pip->egid)) ;

	rs = mkpath2(dcfname,newsdname,DIRCACHEFNAME) ;
	if (rs < 0) goto ret0 ;

/* create the replacement file for the directory cache */

	rs = mkpath2(template,newsdname,".dircacheXXXXXX") ;
	if (rs < 0) goto ret0 ;

	rs = mktmpfile(tmpfname,0664,template) ;
	if (rs < 0) goto ret0 ;

	if ((rs = bopen(dcfp,tmpfname,"wct",0664)) >= 0) {
	    int		i ;
	    int		dl ;
	    const char	*dp ;

	    if (f_chown) u_chown(tmpfname,uid_pcs,gid_pcs) ;

	    bprintline(dcfp,DIRCACHE_MAGIC,-1) ;

	    for (i = 0 ; vecpstr_get(dlp,i,&dp) >= 0 ; i += 1) {
	        if (dp == NULL) continue ;
	        dl = strlen(dp) ;
		c += 1 ;
	        rs = bprintline(dcfp,dp,dl) ;
	        if (rs < 0) break ;
	    } /* end for */

/* only gets directories, but follow links to find them */

	    bclose(dcfp) ;
	} /* end if (opened replacement file) */

	if (rs >= 0) {
	    rs = u_rename(tmpfname,dcfname) ;
	    if ((rs == SR_ACCESS) && f_chown) {
	        rs1 = u_seteuid(pip->uid) ;
	        if (rs1 >= 0) {
	            rs = u_rename(tmpfname,dcfname) ;
	            u_seteuid(pip->euid) ;
	        }
	    }
	} /* end if (renaming attempt) */

	if ((rs < 0) && (tmpfname[0] != '\0')) {

	    rs1 = u_unlink(tmpfname) ;

	    if ((rs1 < 0) && f_chown) {
	        rs1 = u_seteuid(pip->uid) ;
	        if (rs1 >= 0) {
	            u_unlink(tmpfname) ;
	            u_seteuid(pip->euid) ;
	        }
	    }

	} /* end if (unlink attempt) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progdname: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdircache) */


static int vecpstr_loaddirs(VECPSTR *op,const char *newsdname)
{
	FSDIRTREE	dir ;
	int		rs ;
	int		opts ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (newsdname == NULL) return SR_FAULT ;

	opts = (FSDIRTREE_MFOLLOW | FSDIRTREE_MDIR) ;
	if ((rs = fsdirtree_open(&dir,newsdname,opts)) >= 0) {
	    struct ustat	sb ;
	    const int	flen = MAXPATHLEN ;
	    int		fl ;
	    char	fbuf[MAXPATHLEN+1] ;

	    while ((rs = fsdirtree_read(&dir,&sb,fbuf,flen)) > 0) {
	        fl = rs ;

	        if (fbuf[0] != '.') {
	            c += 1 ;
	            rs = vecpstr_add(op,fbuf,fl) ;
	        }

	        if (rs < 0) break ;
	    } /* end while */

	    fsdirtree_close(&dir) ;
	} /* end if (fsdirtree) */

	if (rs >= 0)
	    vecpstr_sort(op,NULL) ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecpstr_loaddirs) */


