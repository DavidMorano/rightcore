/* logfile_printf */


/***********************************************************************

	Module to operate on the logfile.


************************************************************************/



#define	CF_DEBUG	0



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<time.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<varargs.h>

#include	<bfile.h>

#include	"localmisc.h"
#include	"defs.h"




#define	OFLAGS	(O_WRONLY | O_APPEND | O_CREAT)
#define	BUFLEN	100
#define	TIMEOUT	20



/* external subroutines */

extern int	chmod() ;
extern int	bcontrol() ;
extern int	format(), sprintf() ;


/* external variables */

extern struct global	g ;

extern int		errno ;






int logfile_printf(va_alist)
va_dcl
{
	va_list		ap ;

	struct flock	fl ;

	struct tm	ts, *timep ;

	int		pl, lfd = -1 ;
	int		rs, i, l, l2 ;

#if	CF_DEBUG
	char		buf2[100] ;
#endif
	char	buf[BUFLEN + 1] ;
	char	*fmt ;


	if (! g.f.logging) return OK ;

	rs = bcontrol(g.lfp,BC_GETFD,&lfd) ;

	if ((rs < 0) || (lfd < 0)) return BAD ;

/* get the current time-of-day */

	g.daytime = (utime_t) time(NULL) ;

#ifdef	SYSV
	timep = (struct tm *) localtime_r((time_t *) &g.daytime,&ts) ;
#else
	timep = (struct tm *) localtime((time_t *) &g.daytime) ;
#endif

	pl = sprintf(buf,"%02d%02d%02d %02d%02d:%02d ",
	    timep->tm_year,
	    timep->tm_mon + 1,
	    timep->tm_mday,
	    timep->tm_hour,
	    timep->tm_min,
	    timep->tm_sec) ;

	va_begin(ap) ;

	fmt = (char *) va_arg(ap,char *) ;

	l = format(buf + pl,(BUFLEN - pl),NULL,NULL,fmt,ap) ;

#if	CF_DEBUG
	l2 = sprintf(buf2,"opid=%d cpid=%d length=%d buflen-pl=%d\n",
	    g.pid,getpid(),l,BUFLEN - pl) ;

	write(2,buf2,l2) ;
#endif

	if (l >= (BUFLEN - pl - 1)) buf[pl + l++] = '\n' ;

/* lock it */

	fl.l_type = F_UNLCK ;
	fl.l_whence = SEEK_SET ;
	fl.l_start = 0L ;
	fl.l_len = 0L ;
	for (i = 0 ; i < TIMEOUT ; i += 1) {

	    if (fcntl(lfd,F_SETLK,&fl) != -1) {

	        rs = OK ;
	        break ;
	    }

	    rs = BAD ;
	    if (errno != EAGAIN) break ;

	} /* end for */

/* write it, lock or no lock */

	l = write(lfd,buf,pl + l) ;

/* do we have a lock ? ; unlock if we do */

	if (rs >= 0) {

	    fl.l_type = F_UNLCK ;
	    fcntl(lfd,F_SETLK,&fl) ;

	}

/* exit subroutine */

	va_end(ap) ;

	return l ;
}
/* end subroutine (logfile_printf) */



