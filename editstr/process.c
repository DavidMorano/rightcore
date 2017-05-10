/* process */

/* process a file */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* time-time debug print-outs */


/* revision history:

	= 96/03/01, David A­D­ Morano

	The subroutine was adapted from others programs that
	did similar types of functions.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine just provides optional expansion of directories.
	The real work is done by the PROCFILE subroutine.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

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

extern int	procfile(struct proginfo *,PARAMOPT *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */

extern int	if_int ;


/* local structures */


/* forward references */

static int isdotdir(const char *) ;


/* local variables */


/* exported subroutines */


int process(pip,pop,fname)
struct proginfo	*pip ;
PARAMOPT	*pop ;
const char	fname[] ;
{
	struct ustat	sb ;

	int	rs ;
	int	c ;
	int	f_dir ;


	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: fname=%s\n",fname) ;
#endif /* CF_DEBUG */

	c = 0 ;
	rs = u_stat(fname,&sb) ;

	f_dir = S_ISDIR(sb.st_mode) ;
	if ((rs >= 0) && f_dir && (! pip->f.follow)) {

	    rs = u_lstat(fname,&sb) ;

	    f_dir = S_ISDIR(sb.st_mode) ;
	}

	if (rs < 0)
	    goto ret0 ;

	if (f_dir) {

	    int		del ;

	    char	tmpfname[MAXPATHLEN + 1] ;


	    if (pip->f.recurse) {

	        FSDIRTREE	dt ;

	        int	dtopts = 0 ;

	        char	dename[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: recursing\n") ;
#endif

	        dtopts |= ((pip->f.follow) ? FSDIRTREE_MFOLLOW : 0) ;
	        rs = fsdirtree_open(&dt,fname,dtopts) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: fsdirtree_open() rs=%d\n",rs) ;
#endif

	        if (rs < 0) {

	            if (! pip->f.quiet) {

	                printf(pip->efp,
	                    "%s: could not open directory (%d)\n",
	                    pip->progname,rs) ;

	                printf(pip->efp,
	                    "%s: directory=%s\n",
	                    pip->progname,fname) ;

	            }

	            if (! pip->f.nostop)
	                goto bad0 ;

	        } /* end if (could not open directory) */

	        if (rs >= 0) {
		    const int	mpl = MAXPATHLEN ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("process: reading\n") ;
#endif

	            while (! if_int) {

	                del = fsdirtree_read(&dt,&sb,dename,mpl) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("process: fsdirtree_read() rs=%d\n",
	                        del) ;
#endif

	                if (del <= 0)
	                    break ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("process: de name=%s\n",dename) ;
#endif

	                if (isdotdir(fname)) {
	                    rs = mkpath1(tmpfname,dename) ;
	                } else
	                    rs = mkpath2(tmpfname,fname,dename) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("process: tmpfname=%s\n",tmpfname) ;
#endif

	                if (rs > 0) {

	                    rs = procfile(pip,pop,tmpfname) ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("process: procfile() rs=%d\n",rs) ;
#endif

	                    if (rs > 0)
	                        c += 1 ;

	                }

	                if ((rs < 0) && (! pip->f.nostop))
	                    break ;

	            } /* end while (looping through entries) */

	            fsdirtree_close(&dt) ;

	        } /* end if (opened directory tree) */

	    } else {

	        FSDIR		d ;

	        FSDIR_ENT	ds ;


#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: not recursing\n") ;
#endif

	        rs = fsdir_open(&d,fname) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: fsdir_open() rs=%d\n",rs) ;
#endif

	        if (rs < 0) {

	            if (! pip->f.quiet) {

	                printf(pip->efp,
	                    "%s: could not open directory (%d)\n",
	                    pip->progname,rs) ;

	                printf(pip->efp,
	                    "%s: directory=%s\n",
	                    pip->progname,fname) ;

	            }

	            if (! pip->f.nostop)
	                goto bad0 ;

	        } /* end if (could not open directory) */

	        if (rs >= 0) {

	            while ((! if_int) &&
	                ((del = fsdir_read(&d,&ds)) > 0)) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("process: de name=%s\n",ds.name) ;
#endif

	                if (ds.name[0] == '.') {
	                    if ((ds.name[1] == '\0') ||
	                        ((ds.name[1] == '.') && (ds.name[2] == '\0')))
	                        continue ;
	                }

	                if (isdotdir(fname))
	                    rs = mkpath1(tmpfname,ds.name) ;

	                else
	                    rs = mkpath2(tmpfname,fname,ds.name) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("process: tmpfname=%s\n",
	                        tmpfname) ;
#endif

	                if (rs > 0) {

	                    rs = procfile(pip,pop,tmpfname) ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(2))
	                        debugprintf("process: procfile() rs=%d\n",rs) ;
#endif

	                    if (rs > 0)
	                        c += 1 ;

	                }

	                if ((rs < 0) && (! pip->f.nostop))
	                    break ;

	            } /* end while (looping through entries) */

	            fsdir_close(&d) ;

	        } /* end if (opened directory tree) */

	    } /* end if (directory) */

	} else if (S_ISREG(sb.st_mode)) {

	    rs = procfile(pip,pop,fname) ;

	    if (rs > 0)
	        c += 1 ;

	} /* end if */

ret0:
bad0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (process) */



/* LOCAL SUBROUTINES */



static int isdotdir(dname)
const char	dname[] ;
{
	int	f = FALSE ;


	if (dname[0] == '.') {

	    f = (dname[1] == '\0') ;

	    if (! f)
	        f = ((dname[1] == '/') && (dname[2] == '\0')) ;

	}

	return f ;
}
/* end subroutine (isdotdir) */



