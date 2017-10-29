/* checklockfile */

/* check if we still own the lock file */
/* version %I% last modified %G% */


#define	F_DEBUG		0		/* switchable debug print-outs */


/* revision history :

	= 91/09/10, David A­D­ Morano

	This program was originally written.


*/



/*****************************************************************************

	We want to check that we still own the lock file and also
	update it by writting a current time into it.



*****************************************************************************/



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

#include	"misc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;

extern char	*timestr_log(time_t,char *) ;


/* externals variables */


/* forward references */


/* local global variables */


/* local structures */


/* local data */






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
	char	timebuf[50] ;


#if	F_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("checklockfile: entered, filename=%s\n",filename) ;
#endif

	if (fp == NULL) 
		return -1001 ;

	if ((rs = bseek(fp,0L,SEEK_SET)) < 0)
		return rs ;

	if ((rs = bgetline(fp,buf,BUFLEN)) <= 0) 
		return -1002 ;

	len = rs ;
	if (cfdeci(buf,len,&iw) < 0) 
		return -1003 ;

	oldpid = (pid_t) iw ;

#if	F_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("checklockfile: 1003 \n") ;
#endif

	if (oldpid != pip->pid) 
		return ((int) pid) ;

	if (u_stat(filename,&sb1) >= 0) {

	    if (bcontrol(fp,BC_STAT,&sb2) < 0)
	        return -1004 ;

	    if ((sb1.st_dev != sb2.st_dev) ||
	        (sb1.st_ino != sb2.st_ino)) 
		return -1005 ;

	} /* end if */

	bgetline(fp,buf,BUFLEN) ;

	bprintf(fp,"%s", timestr_log(daytime,timebuf)) ;

	bflush(fp) ;

#if	F_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("checklockfile: exiting\n") ;
#endif

	return OK ;
}
/* end subroutine (checklockfile) */



