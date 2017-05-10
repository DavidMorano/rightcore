/* checklockfile */

/* check if we still own the lock file */
/* version %I% last modified %G% */


#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 1998-09-10, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	We want to check that we still own the lock file and also
	update it by writting a current time into it.


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;

extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */







int checklockfile(pip,fp,filename,banner,daytime,pid)
struct proginfo	*pip ;
bfile	*fp ;
char	banner[], filename[] ;
time_t	daytime ;
pid_t	pid ;
{
	struct ustat	sb1, sb2 ;

	pid_t	oldpid ;

	int	rs ;
	int	iw, len ;

	char	buf[BUFLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("checklockfile: entered, filename=%s\n",filename) ;
#endif

	if (fp == NULL) 
		return -1001 ;

/* rewind */

	if ((rs = bseek(fp,0L,SEEK_SET)) < 0)
		return rs ;

/* read the first line */

	if ((rs = breadline(fp,buf,BUFLEN)) <= 0) 
		return -1002 ;

	len = rs ;
	if (cfdeci(buf,len,&iw) < 0) 
		return -1003 ;

	oldpid = (pid_t) iw ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("checklockfile: 1003 \n") ;
#endif

	if (oldpid != pip->pid) 
		return ((int) pid) ;

/* is the file the same file ? */

	if (u_stat(filename,&sb1) >= 0) {

	    if (bcontrol(fp,BC_STAT,&sb2) < 0)
	        return -1004 ;

	    if ((sb1.st_dev != sb2.st_dev) ||
	        (sb1.st_ino != sb2.st_ino)) 
		return -1005 ;

	} /* end if */

/* read the second line (the 'node!user' thing) */

	breadline(fp,buf,BUFLEN) ;

/* write an updated time at this offset in the file */

	bprintf(fp,"%s", timestr_logz(daytime,timebuf)) ;

	bflush(fp) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("checklockfile: exiting\n") ;
#endif

	return OK ;
}
/* end subroutine (checklockfile) */



