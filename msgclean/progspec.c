/* progspec */

/* process a file */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* time-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that did similar
	types of functions.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine just provides optional expansion of directories.
	The real work is done by the |progfile()| subroutine.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<paramopt.h>
#include	<fsdir.h>
#include	<fsdirtree.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	SUFBUFLEN
#define	SUFBUFLEN	MAXNAMELEN
#endif


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	isDotDir(const char *) ;

extern int	progabort(PROGINFO *) ;
extern int	progfile(PROGINFO *,PARAMOPT *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int progspec(pip,pop,fname)
PROGINFO	*pip ;
PARAMOPT	*pop ;
const char	fname[] ;
{
	struct ustat	sb ;
	int		rs ;
	int		c = 0 ;
	int		f_dir ;

	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progspec: fname=%s\n",fname) ;
#endif /* CF_DEBUG */

	rs = u_stat(fname,&sb) ;

	f_dir = S_ISDIR(sb.st_mode) ;
	if ((rs >= 0) && f_dir && (! pip->f.follow)) {
	    rs = u_lstat(fname,&sb) ;
	    f_dir = S_ISDIR(sb.st_mode) ;
	}

	if (rs >= 0) {

	    if (f_dir) {
	        int	del ;
	        char	tmpfname[MAXPATHLEN + 1] ;

	        if (pip->f.recurse) {
	            FSDIRTREE		dt ;
	            FSDIRTREE_STAT	fsb ;
	            int		dtopts = 0 ;
	            char	dename[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progspec: recursing\n") ;
#endif

	            dtopts |= ((pip->f.follow) ? FSDIRTREE_MFOLLOW : 0) ;
	            if ((rs = fsdirtree_open(&dt,fname,dtopts)) >= 0) {
	                const int	mpl = MAXPATHLEN ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progspec: reading\n") ;
#endif

	                while (rs >= 0) {
	                    del = fsdirtree_read(&dt,&fsb,dename,mpl) ;
	                    if (del == 0) break ;

	                    rs = del ;
	                    if (rs < 0) break ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("progspec: de name=%s\n",
	                            dename) ;
#endif

	                    if (isDotDir(fname)) {
	                        rs = mkpath1(tmpfname,dename) ;
	                    } else {
	                        rs = mkpath2(tmpfname,fname,dename) ;
			    }

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("progspec: tmpfname=%s\n",
	                            tmpfname) ;
#endif

	                    if (rs > 0) {

	                        rs = progfile(pip,pop,tmpfname) ;

#if	CF_DEBUG
	                        if (DEBUGLEVEL(2))
	                            debugprintf("progspec: progfile() "
	                                "rs=%d\n",rs) ;
#endif

	                        if (rs > 0)
	                            c += 1 ;

	                    } /* end if */

	                    if (progabort(pip) > 0) break ;
	                    if ((rs < 0) && (! pip->f.nostop)) break ;
	                } /* end while (looping through entries) */

	                fsdirtree_close(&dt) ;
	            } /* end if (opened directory tree) */

	        } else {
	            FSDIR	d ;
	            FSDIR_ENT	ds ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progspec: not recursing\n") ;
#endif

	            if ((rs = fsdir_open(&d,fname)) >= 0) {

	                while ((rs = fsdir_read(&d,&ds)) > 0) {
	                    del = rs ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("progspec: de name=%s\n",
	                            ds.name) ;
#endif

	                    if (ds.name[0] == '.') continue ;

	                    if ((rs = mkpath2(tmpfname,fname,ds.name)) > 0) {

#if	CF_DEBUG
	                        if (DEBUGLEVEL(4))
	                            debugprintf("progspec: tmpfname=%s\n",
	                                tmpfname) ;
#endif

	                        rs = progfile(pip,pop,tmpfname) ;

#if	CF_DEBUG
	                        if (DEBUGLEVEL(2))
	                            debugprintf("progspec: progfile() "
	                                "rs=%d\n",rs) ;
#endif

	                        if (rs > 0)
	                            c += 1 ;

	                    } /* end if */

	                    if (progabort(pip) > 0) break ;
	                    if ((rs < 0) && (! pip->f.nostop)) break ;
	                } /* end while (looping through entries) */

	                fsdir_close(&d) ;
	            } /* end if (opened directory tree) */

	        } /* end if (directory) */

	    } else if (S_ISREG(sb.st_mode)) {

	        rs = progfile(pip,pop,fname) ;

	        if (rs > 0)
	            c += 1 ;

	    } /* end if */

	    if (rs == SR_ACCESS) {
	        if (! pip->f.quiet) {
	            printf(pip->efp,
	                "%s: could not open directory (%d)\n",
	                pip->progname,rs) ;
	            printf(pip->efp,
	                "%s: directory=%s\n",
	                pip->progname,fname) ;
	        }
	    } /* end if (could not open directory) */

	} /* end if (ok) */

ret0:
bad0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progspec: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (progspec) */


