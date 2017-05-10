/* procfile */

/* print out the status of a file */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1987-09-01, David A­D­ Morano

	This subroutine was originally written.


*/


/*******************************************************************

	This program will return the various time statusi of a file.


*********************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<tzfile.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<bfile.h>
#include	<paramopt.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */



/* external subroutines */


/* external variables */







int procfile(pip,pp,ofp,otype,fname)
struct proginfo	*pip ;
int		otype ;
bfile		*ofp ;
char		fname[] ;
PARAMOPT	*pp ;
{
	struct ustat	sb ;

	struct tm	*ftsp ;

	int	rs, i ;

	char	dbuf[MAXPATHLEN + 1], *bp ;
	char	*cbuf ;
	char	*cp ;


	if (fname == NULL)
	    return SR_FAULT ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: file=%s\n",pip->progname,fname) ;

	if (pip->verboselevel >= 2)
	    bprintf(ofp,"%s\n",fname) ;

/* do it */

	if ((fname[0] != '\0') && (strcmp(fname,"-") != 0))
	    rs = u_stat(fname,&sb) ;

	else
	    rs = u_fstat(FD_STDIN,&sb) ;

	if (rs < 0)
	    goto ret0 ;

	if (pip->verboselevel >= 3)
	    bprintf(ofp,"mode=\\o%08o\n",sb.st_mode) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procfile: about to get time\n") ;
#endif

	ftsp = localtime(&sb.st_mtime) ;

	if (ftsp != NULL) {

	    switch (otype) {

	    case OTYPE_DEC:
	    case OTYPE_DECIMAL:
	    case OTYPE_INT:
	        rs = bprintf(ofp,"%lu\n",sb.st_mtime) ;

	        break ;

	    case OTYPE_TOUCH:
	        rs = bprintf(ofp,"%02u%02u%02u%02u%02u\n",
	            (ftsp->tm_mon + 1),
	            ftsp->tm_mday,
	            ftsp->tm_hour,
	            ftsp->tm_min,
	            (ftsp->tm_year % 100)) ;

	        break ;

	    case OTYPE_TOUCHT:
	    case OTYPE_TTOUCH:
	        rs = bprintf(ofp,"%04u%02u%02u%02u%02u.%02u\n",
	            (ftsp->tm_year + TM_YEAR_BASE),
	            (ftsp->tm_mon + 1),
	            ftsp->tm_mday,
	            ftsp->tm_hour,
	            ftsp->tm_min,
	            ftsp->tm_sec) ;

	        break ;

	    case OTYPE_INFO:
	        bp = dbuf ;

	        cbuf = ctime(&sb.st_atime) ;
	        cbuf[24] = '\0' ;

	        bp += sprintf(bp," accessed %s\n",
	            cbuf) ;


	        cbuf = ctime(&sb.st_mtime) ;

	        cbuf[24] = '\0' ;

	        bp += sprintf(bp," modified %s\n",
	            cbuf) ;

	        cbuf = ctime(&sb.st_ctime) ;
	        cbuf[24] = '\0' ;

	        bp += sprintf(bp," changed  %s\n",
	            cbuf) ;

	        bp += sprintf(bp," inode=%08x\n",sb.st_ino) ;


	        bp += sprintf(bp," dev=%08x\n",sb.st_dev) ;


	        bp += sprintf(bp," rdev=%08x\n",sb.st_rdev) ;


	        rs = bwrite(ofp,dbuf,(bp - dbuf)) ;

	        break ;

	    case OTYPE_ACCESS:
	        cbuf = ctime(&sb.st_atime) ;
	        cbuf[24] = '\0' ;

	        rs = bprintf(ofp,"%s\n",cbuf,24) ;

	        break ;

	    default:
	        rs = SR_NOTSUP ;

	    } /* end switch */

	} else
	    rs = SR_BADFMT ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procfile) */



