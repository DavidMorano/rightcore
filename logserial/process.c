/* process */

/* process (monitor) the serial communications line */


#define	CF_DEBUG 	0
#define	F_REMOVE 	1


/* revision history:

	= 1996-03-01, David A­D­ Morano

	This subroutine was originally written.

*/


/***********************************************************************

	Description:


	Synopsis:


	Arguments:

	- pip		pointer to program environment
	- basedir	directory at top of tree
	- termname	name of line device to monitor
	- ofd		file descriptor to log to
	- logsize	size of output buffer information

	Returns:


***********************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<dirent.h>
#include	<limits.h>
#include	<fcntl.h>
#include	<termios.h>

#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 10),2048)
#endif


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;


/* external variables */


/* exported subroutines */


int process(pip,basedir,termname,ofd,logsize)
struct proginfo	*pip ;
char		basedir[] ;
char		termname[] ;
int		ofd, logsize ;
{
	struct ustat	sb ;

	struct termios	settings ;

	int	tfd ;
	int	rs, len ;

	char	dfname[MAXPATHLEN + 1] ;
	char	*buf ;
	char	*bdp ;


	if (termname == NULL)
	    return BAD ;

	bdp = termname ;
	if (((rs = perm(termname,-1,-1,NULL,R_OK | W_OK)) < 0) &&
	    (termname[0] != '/')) {

	    if ((basedir == NULL) || (basedir[0] == '\0'))
	        basedir = "/dev" ;

	    bdp = dfname ;
	    mkpath2(dfname,basedir,termname) ;

	    if ((rs = perm(bdp,-1,-1,NULL,R_OK | W_OK)) < 0) {

	        bprintf(pip->efp,
	            "%s: could not access \"%s\" rs=%d\n",
	            pip->progname,bdp,rs) ;

	        return rs ;
	    }

	} /* end if */

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,
	        "%s: device=%s\n",pip->progname,bdp) ;

	if (pip->f.print)
	    return OK ;


	rs = u_open(bdp,(O_RDONLY | O_NONBLOCK | O_NOCTTY),0666) ;
	    tfd = rs ;
	if (rs >= 0) {

	    rs = u_mmap(NULL,logsize,
	        PROT_WRITE,MAP_SHARED,ofd,0L,&buf) ;

	    if (rs >= 0) {

/* extend the output file if necessary */

	        u_fstat(ofd,&sb) ;

	        if (sb.st_size < logsize) {

	            int	len ;
	            int	flen = (logsize - sb.st_size) ;

	            char	*blanks = "         " ;


	            while (flen > 0) {

	                if ((len = u_write(ofd,blanks,flen)) < 0)
	                    break ;

	                flen -= len ;

	            }

	            u_write(ofd,"\n",1) ;

	        } /* end if (extending file) */

/* continue with the logging process */


	        if ((rs = uc_tcgetattr(tfd,&settings)) >= 0) {

	            cfsetospeed(&settings,B0) ;

/* hang it up, baby ! */

	            if (! pip->f.fakeit)
	                rs = uc_tcsetattr(tfd,TCSANOW,&settings) ;

	        }

	        u_munmap(buf,logsize) ;

	    } else {

	        bprintf(pip->efp,
	            "%s: could map outfile (%d)\n",
	            pip->progname,rs) ;

	    }

	} else {

	    bprintf(pip->efp,
	        "%s: could not open file (%d)\n",
	        pip->progname,rs) ;

	}

	return rs ;
}
/* end subroutine (process) */



