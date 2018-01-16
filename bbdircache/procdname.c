/* progdname */

/* process a directory */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
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
extern int	vecpstr_loaddirs(VECPSTR *,const char *) ;
extern int	isOneOf(const int *,int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

extern int	proglog_printf(PROGINFO *,cchar *,...) ;

extern int	progpcsuid(PROGINFO *) ;
extern int	progpcsgid(PROGINFO *) ;


/* external variables */


/* local structures */


/* forward references */

static int procdiffer(PROGINFO *,VECPSTR *,const char *) ;
static int procdircache(PROGINFO *,bfile *,VECPSTR *,const char *) ;
static int procdircacher(PROGINFO *,vecpstr *,cchar *) ;
static int procrem(PROGINFO *,cchar *,cchar *) ;


/* local variables */


/* exported subroutines */


int progdname(PROGINFO *pip,bfile *ofp,cchar *np,int nl)
{
	NULSTR		nd ;
	VECPSTR		dirs ;
	int		rs ;
	int		rs1 ;
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
	                int	i ;
	                cchar	*dp ;
	                for (i = 0 ; vecpstr_get(&dirs,i,&dp) >= 0 ; i += 1) {
	                    if (dp != NULL) {
	                        rs = bprintln(ofp,dp,-1) ;
	                    }
	                    if (rs < 0) break ;
	                } /* end for */
	            } /* end if (verbosity) */
	        } /* end if (vecpstr_loaddirs) */
	        rs1 = vecpstr_finish(&dirs) ;
	        if (rs >= 0) rs = rs1 ;
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

	    rs1 = nulstr_finish(&nd) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (nulstr-newsdname) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (progdname) */


/* local subroutines */


static int procdiffer(PROGINFO *pip,VECPSTR *dlp,cchar *newsdname)
{
	const int	ml = strlen(DIRCACHE_MAGIC) ;
	int		rs ;
	int		rs1 ;
	int		f = TRUE ;

	if ((rs = vecpstr_getsize(dlp)) >= 0) {
	    const int	dsize = rs ;
	    const char	*dc = DIRCACHEFNAME ;
	    char	dcfname[MAXPATHLEN+1] ;
	    if ((rs = mkpath2(dcfname,newsdname,dc)) >= 0) {
	        struct ustat	sb ;
	        if ((rs = uc_stat(dcfname,&sb)) >= 0) {
	            bfile	cfile, *cfp = &cfile ;
	            const int	fsize = (sb.st_size & INT_MAX) ;
	            if (dsize == (fsize-ml-1)) {
	                if ((rs = bopen(cfp,dcfname,"r",0666)) >= 0) {
	                    const int	dlen = MAXPATHLEN ;
	                    int		line = 0 ;
	                    int		f_mis = FALSE ;
	                    cchar	*dp ;
	                    char	dbuf[MAXPATHLEN+1] ;
	                    while ((rs = breadline(cfp,dbuf,dlen)) > 0) {
	                        int	dl = rs ;
	                        if (line > 0) {
	                            if (dbuf[dl-1] == '\n') dl -= 1 ;
	                            if (vecpstr_get(dlp,(line-1),&dp) >= 0) {
	                                f_mis = (strwcmp(dp,dbuf,dl) != 0) ;
	                            } else {
	                                f_mis = TRUE ;
	                            }
	                        }
	                        line += 1 ;
	                        if (f_mis) break ;
	                    } /* end while */
	                    if (! f_mis) f = FALSE ;
	                    rs1 = bclose(cfp) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (file-open) */
	            } /* end if (sizes were the same) */
	        } else if (isNotAccess(rs)) {
	            rs = SR_OK ;
	        }
	    } /* end if */
	} /* end if (vecpstr-getsize) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procdiffer) */


static int procdircache(PROGINFO *pip,bfile *ofp,VECPSTR *dlp,cchar *newsdname)
{
	int		rs ;
	int		c = 0 ;
	cchar		*cfn = DIRCACHEFNAME ;
	char		dcfname[MAXPATHLEN + 1] ;
	if ((rs = mkpath2(dcfname,newsdname,cfn)) >= 0) {
	    char	template[MAXPATHLEN + 1] ;
	    cfn = ".dircacheXXXXXX" ;
	    if ((rs = mkpath2(template,newsdname,cfn)) >= 0) {
	        char	tbuf[MAXPATHLEN+1] ;
	        if ((rs = mktmpfile(tbuf,0664,template)) >= 0) {
	            if ((rs = procdircacher(pip,dlp,tbuf)) >= 0) {
	                c = rs ;
	                rs = procrem(pip,tbuf,dcfname) ;
	            }
	        }
	    }
	}
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdircache) */


static int procdircacher(PROGINFO *pip,vecpstr *dlp,cchar *fn)
{
	bfile		dcfile, *dcfp = &dcfile ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	if ((rs = bopen(dcfp,fn,"wct",0664)) >= 0) {
	    if ((rs = bprintln(dcfp,DIRCACHE_MAGIC,-1)) >= 0) {
	        int	i ;
	        int	dl ;
	        cchar	*dp ;
	        for (i = 0 ; vecpstr_get(dlp,i,&dp) >= 0 ; i += 1) {
	            if (dp != NULL) {
	                dl = strlen(dp) ;
	                c += 1 ;
	                rs = bprintln(dcfp,dp,dl) ;
	            }
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if */
	    rs1 = bclose(dcfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (opened replacement file) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdircacher) */


static int procrem(PROGINFO *pip,cchar *tbuf,cchar *dcfname)
{
	int		rs ;
	if ((rs = progpcsuid(pip)) >= 0) {
	    const uid_t	uid_pcs = rs ;
	    if ((rs = progpcsgid(pip)) >= 0) {
	        const gid_t	gid_pcs = rs ;
	        int		f_chown ;
	        f_chown = ((gid_pcs >= 0) && (gid_pcs != pip->egid)) ;
	        if (f_chown) u_chown(tbuf,uid_pcs,gid_pcs) ;
	        rs = u_rename(tbuf,dcfname) ;
	    }
	} /* end if (progpcsuid) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progrem: ret rs=%d c=%u\n",rs,c) ;
#endif

	return rs ;
}
/* end subroutine (procrem) */


