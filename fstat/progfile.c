/* progfile */

/* print out the status of a file */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************

	This program will return the various time statusi of a file.


*********************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mkdev.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>
#include	<tzfile.h>

#include	<vsystem.h>
#include	<paramopt.h>
#include	<bfile.h>
#include	<tmtime.h>
#include	<sntmtime.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	DBUFLEN		MAXPATHLEN


/* external subroutines */


/* external variables */


/* exported subroutines */


int progfile(pip,pp,ofp,otype,fname)
struct proginfo	*pip ;
int		otype ;
bfile		*ofp ;
char		fname[] ;
PARAMOPT	*pp ;
{
	struct ustat	sb ;

	TMTIME		ts ;

	const int	dlen = DBUFLEN ;

	int	rs ;
	int	i ;

	const char	*cp ;

	char	dbuf[DBUFLEN+ 1], *bp ;
	char	*cbuf ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("progfile: sizeof(ino)=%u\n",sizeof(sb.st_ino)) ;
	    debugprintf("progfile: sizeof(dev)=%u\n",sizeof(sb.st_dev)) ;
	}
#endif

	if (fname == NULL)
	    return SR_FAULT ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: file=%s\n",pip->progname,fname) ;

	if (pip->verboselevel >= 2)
	    bprintf(ofp,"%s\n",fname) ;

/* do it */

	if ((fname[0] != '\0') && (strcmp(fname,"-") != 0)) {
	    rs = uc_stat(fname,&sb) ;
	} else
	    rs = u_fstat(FD_STDIN,&sb) ;

	if (rs < 0)
	    goto ret0 ;

	if (pip->verboselevel >= 3)
	    bprintf(ofp,"mode=\\o%08o\n",sb.st_mode) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progfile: about to get time\n") ;
#endif

	if ((rs = tmtime_localtime(&ts,sb.st_mtime)) >= 0) {
	    switch (otype) {

	    case OTYPE_DEC:
	    case OTYPE_DECIMAL:
	    case OTYPE_INT:
	        rs = bprintf(ofp,"%lu\n",sb.st_mtime) ;
	        break ;

	    case OTYPE_TOUCH:
	        rs = bprintf(ofp,"%02u%02u%02u%02u%02u\n",
	            (ts.mon + 1),
	            ts.mday,
	            ts.hour,
	            ts.min,
	            (ts.year % 100)) ;
	        break ;

	    case OTYPE_TOUCHT:
	    case OTYPE_TTOUCH:
	        rs = bprintf(ofp,"%04u%02u%02u%02u%02u.%02u\n",
	            (ts.year + TM_YEAR_BASE),
	            (ts.mon + 1),
	            ts.mday,
	            ts.hour,
	            ts.min,
	            ts.sec) ;
	        break ;

	    case OTYPE_INFO:
	        bp = dbuf ;

	        cbuf = ctime(&sb.st_atime) ;
	        cbuf[24] = '\0' ;

	        bp += sprintf(bp," accessed %s\n", cbuf) ;

	        cbuf = ctime(&sb.st_mtime) ;

	        cbuf[24] = '\0' ;

	        bp += sprintf(bp," modified %s\n",
	            cbuf) ;

	        cbuf = ctime(&sb.st_ctime) ;
	        cbuf[24] = '\0' ;

	        bp += sprintf(bp," changed  %s\n", cbuf) ;

	        bp += sprintf(bp," inode=%llu\n",sb.st_ino) ;

	        bp += sprintf(bp," dev=%08x\n",sb.st_dev) ;

	        bp += sprintf(bp," rdev=%08x\n",sb.st_rdev) ;

		{
		    major_t	mj = major(sb.st_rdev) ;
		    major_t	mn = minor(sb.st_rdev) ;
	            bp += sprintf(bp," maj=%u\n",mj) ;
	            bp += sprintf(bp," min=%u\n",mn) ;
		}

	        rs = bwrite(ofp,dbuf,(bp - dbuf)) ;

	        break ;

	    case OTYPE_ACCESS:
	        cbuf = ctime(&sb.st_atime) ;
	        cbuf[24] = '\0' ;
	        rs = bprintf(ofp,"%s\n",cbuf,24) ;
	        break ;

		case OTYPE_LOG:
	    	    rs = sntmtime(dbuf,dlen,&ts,"%y%m%d_%H%M:%S") ;
		    if (rs >= 0)
	        	rs = bprintln(ofp,dbuf,rs) ;
		    break ;

		case OTYPE_LOGZ:
	    	    rs = sntmtime(dbuf,dlen,&ts,"%y%m%d_%H%M:%S_%Z") ;
		    if (rs >= 0)
	        	rs = bprintln(ofp,dbuf,rs) ;
		    break ;

		case OTYPE_STD:
	    	    rs = sntmtime(dbuf,dlen,&ts,"%a %b %d %T %Z %Y %O") ;
		    if (rs >= 0)
	        	rs = bprintln(ofp,dbuf,rs) ;
		    break ;

	    default:
	        rs = SR_NOTSUP ;
		break ;

	    } /* end switch */
	} /* end if (localtime) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progfile) */



