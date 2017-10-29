/* mkdatefile */

/* make a temporary date-type job file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Make a file which can serve as a job submital file.  The file will have
	a name which has the current date in it.

	Description:

	int mkdatefile(tmpdname,fs,fm,outname)
	const char	tmpdname[] ;
	char		fs[] ;
	mode_t		fm ;
	char		outname[] ;

	Arguments:

	- tmpdname	buffer holding the name of a directory w/ file
	- fs		file suffix for created file name
	- fm		mode of created file
	- outname	resuling buffer

	Retruns:

	>=0		completed successfully
	<0		bad


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#define	O_MODE		(O_CREAT | O_EXCL | O_RDWR)
#define	MAXNLOOP	100

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	bufprintf(char *,int,const char *,...) ;


/* external variables */


/* exported subroutines */


int mkdatefile(tmpdname,fs,fm,outname)
const char	tmpdname[] ;
char		fs[] ;
mode_t		fm ;
char		outname[] ;
{
	struct tm	*timep ;
	time_t		daytime = time(NULL) ;
	int		rs = SR_OK ;
	int		fd ;
	int		loop ;
	int		len, nbr ;
	int		f_exit = FALSE ;
	char		namebuf[MAXNAMELEN + 1], *nbp ;
	char		*fsp = NULL ;

#if	CF_DEBUGS
	debugprintf("mkmtgfile: ent\n") ;
#endif

	if (tmpdname == NULL) return SR_FAULT ;
	if (outname == NULL) return SR_FAULT ;

/* handle the complicated process of what ever! */

	if (fs != NULL) 
	    fsp = fs ;

	if (tmpdname[0] == '\0') {

		tmpdname = getenv(VARTMPDNAME) ;

		if ((tmpdname == NULL) || (tmpdname[0] == '\0'))
			tmpdname = TMPDNAME ;

	} /* end if (empty TMPDIR given) */

/* go through the loops! */

	for (loop = 0 ; (loop < MAXNLOOP) && (! f_exit) ; loop += 1) {

#if	CF_DEBUGS
	    debugprintf("mkmtgfile: top of while @ loop %d\n",loop) ;
#endif

	    f_exit = TRUE ;
	    timep = localtime(&daytime) ;

	    nbp = namebuf ;
	    nbr = MAXNAMELEN ;

	    rs = bufprintf(namebuf,nbr,"%02d%02d%02d",
	        timep->tm_year,
	        (timep->tm_mon + 1),
	        timep->tm_mday) ;

	    if (rs < 0)
		break ;

	    nbp += rs ;
	    nbr -= rs ;

	    if ((loop > 0) && (loop <= 26)) {

	        *nbp++ = 'a' + loop - 1 ;
		nbr -= 1 ;

	    } else if (loop > 26) {

	        rs = bufprintf(nbp,nbr,"%02d%02d%02d%02d",
	            timep->tm_hour,
	            timep->tm_min,
	            timep->tm_sec,
	            loop) ;

	        if (rs < 0)
		    break ;

		nbp += rs ;
		nbr -= rs ;

	    } /* end if */

	    if (fsp != NULL)  {

		rs = bufprintf(nbp,nbr,fsp) ;
	        if (rs < 0)
		    break ;

		nbp += rs ;
		nbr -= rs ;
	    }

	    *nbp = '\0' ;

/* put it together with the path prefix */

	    rs = mkpath2(outname,tmpdname,namebuf) ;
	    len = rs ;
	    if (rs < 0)
		break ;

#if	CF_DEBUGS
	    debugprintf("mkmtgfile: filename=%s\n",outname) ;
#endif

/* OK, try and make the thing */

	    if (S_ISDIR(fm)) {

	        rs = u_mkdir(outname,fm) ;

		if (rs == SR_EXIST) {
			rs = SR_OK ;
			f_exit = FALSE ;
		}

		if (rs < 0)
			break ;

	    } else if (S_ISREG(fm) || ((fm & (~ 0777)) == 0)) {

	        rs = u_open(outname,O_MODE,(fm & 0777)) ;
		fd = rs ;
		if (rs == SR_EXIST) {
			rs = SR_OK ;
			f_exit = FALSE ;
		}

		if (rs < 0)
		    break ;

		if (fd >= 0) u_close(fd) ;

#if	CF_DEBUGS
	        debugprintf( "mkmtgfile: regular file rs=%d\n", rs) ;
#endif

	    } else if (S_ISFIFO(fm)) {

#if	CF_DEBUGS
	        debugprintf("mkmtgfile: got a FIFO\n") ;
#endif

	        rs = u_mknod(outname,fm,0) ;

		if (rs == SR_EXIST) {
		    rs = SR_OK ;
		    f_exit = FALSE ;
		}

		if (rs < 0)
		    break ;

	    } else if (S_ISSOCK(fm)) {

	        rs = SR_NOSYS ;
		break ;

	    } else {

	        rs = SR_INVALID ;
		break ;
	    }

	} /* end for (looping) */

	if ((rs >= 0) && (loop >= MAXNLOOP))
		rs = SR_TIMEDOUT ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkdatefile) */


