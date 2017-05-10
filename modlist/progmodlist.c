/* progmodlist */

/* get the module list for whatever */


#define	CF_DEBUG 	0		/* run-time debugging */


/* revision history:

	= 1996-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/***********************************************************************

	Synopsis:

	int progmodlist(pip,ofp,basedir,name)
	PROGINFO	*pip ;
	bfile		*ofp ;
	const char	basedir[] ;
	const char	name[] ;

	Arguments:

	pip		program information pointer
	ofp		output file pointer
	basedir		directory at top of tree
	name		name of file

	Returns:

	<0		error
	>=0		OK


***********************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/conf.h>
#include	<stropts.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<limits.h>
#include	<fcntl.h>
#include	<termios.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	DEVDNAME
#define	DEVDNAME	"/dev"
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 10),2048)
#endif

#define	O_FLAGS		(O_RDONLY | O_NONBLOCK | O_NOCTTY)


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;


/* external variables */


/* local structures */

struct stdfile {
	const char	*name ;
	int		fd ;
} ;


/* forward references */

static int	lister(PROGINFO *,bfile *,int) ;


/* local variables */

static struct stdfile	stds[] = {
	{ STDINFNAME, FD_STDIN },
	{ STDOUTFNAME, FD_STDOUT },
	{ STDERRFNAME, FD_STDERR },
	{ NULL, 0 }
} ;


/* exported subroutines */


int progmodlist(pip,ofp,basedir,name)
PROGINFO	*pip ;
bfile		*ofp ;
const char	basedir[] ;
const char	name[] ;
{
	int		rs ; /* initialized below */
	int		i ;
	int		sfd = -1 ;
	const char	*bdp ;
	char		dfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: name=%s\n",name) ;
#endif

	if (name == NULL)
	    return SR_FAULT ;

	sfd = -1 ;
	for (i = 0 ; stds[i].name != NULL ; i += 1) {
	    if (strcmp(stds[i].name,name) == 0) {
	        sfd = stds[i].fd ;
	        break ;
	    }
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: sfd=%d\n",sfd) ;
#endif

	if (sfd < 0) {

	    bdp = name ;
	    if (((rs = u_access(name,0)) < 0) &&
	        (name[0] != '/')) {

	        if ((basedir == NULL) || (basedir[0] == '\0'))
	            basedir = DEVDNAME ;

	        bdp = dfname ;
	        mkpath2(dfname,basedir,name) ;

	        rs = u_access(bdp,R_OK) ;

	    } /* end if */

	    if (pip->verboselevel > 0)
	        bprintf(ofp,"device=%s\n",bdp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("process: about to open\n") ;
#endif

	    if (rs >= 0) {
	        if ((rs = u_open(bdp,O_FLAGS,0666)) >= 0) {
	            sfd = rs ;

	            rs = lister(pip,ofp,sfd) ;

	            u_close(sfd) ;
	        } /* end if (open) */
	    } else {
	        bprintf(pip->efp,"%s: file=%s\n",bdp) ;
	        bprintf(pip->efp,"%s: inaccessible (%d)\n",
	            pip->progname,rs) ;
	    }

	} else {

	    rs = lister(pip,ofp,sfd) ;

	} /* end if */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progmodlist) */


/* local subroutines */


/* list the modules */
static int lister(pip,ofp,sfd)
PROGINFO	*pip ;
bfile		*ofp ;
int		sfd ;
{
	int		rs ;
	int		n = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("lister: ent\n") ;
#endif

	if (sfd < 0)
	    return SR_BADF ;

	if ((rs = u_ioctl(sfd,I_LIST,NULL)) > 0) {
	    void	*p ;
	    int		i ;
	    int		size ;
	    n = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("lister: non-zero IOCTL, rs=%d\n",rs) ;
#endif

	    size = (n + 1) * sizeof(struct str_mlist) ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        struct str_list		sl ;
	        memset(&sl,0,sizeof(struct str_mlist)) ;
	        sl.sl_nmods = n ;
	        sl.sl_modlist = p ;

	        if ((rs = u_ioctl(sfd,I_LIST,&sl)) >= 0) {

	            for (i = 0 ; i < sl.sl_nmods ; i += 1) {
	                bprintf(ofp,"%s\n",sl.sl_modlist[i].l_name) ;
	            } /* end for */

	        } else {
	            bprintf(pip->efp,
	                "%s: could not get the module list (%d)\n",
	                pip->progname,rs) ;
		}

	        uc_free(p) ;
	    } /* end if (memory-allocation) */

	} /* end if (ioctl) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("lister: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (lister) */



