/* termdevice */

/* find the name of the device for the given file descriptor */


#define	CF_DEBUGS	0		/* debug print-outs */


/* revision history:

	= 1998-06-15, David A­D­ Morano
        This subroutine was originally written. This was also inspired by the
        fact that the Sun Solaris 2.5.1 POSIX version of 'ttyname_r' does not
        appear to work. I got the idea for this subroutine from the GNU standard
        C library implementation. It seems like Slowlaris 5.x certainly had a
        lot of buggy problems (sockets, I/O, virtual memory, more)!

	= 2011-10-12, David A­D­ Morano
        I am changing the order of attempts to put 'ttyname_r(3c)' before
        forking a process. Even though we are still on Slowlaris we hope that
        'ttyname_r(3c)' is now working properly!

*/

/* Copyright © 1998,2011 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Store at most BUFLEN character of the pathname, if the terminal FD is
        open, in the caller specified buffer.

	Synopsis:

	int termdevice(dbuf,dben,fd)
	int		fd ;
	char		dbuf[] ;
	int		dlen ;

	Arguments:

	dbuf		buffer to store name
	dlen		length of buffer to store name
	fd		file descriptor

	Return:

	>=0	length of device name
	0	on success
	<0	otherwise


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/wait.h>
#if	defined(OSNAME_SunOS)
#include	<sys/mkdev.h>
#endif
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stddef.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#ifndef	TERMBUFLEN
#define	TERMBUFLEN	(MAXPATHLEN + 20)
#endif

#undef	MINBUFLEN
#define	MINBUFLEN	32

#ifndef	DEVDNAME
#define	DEVDNAME	"/dev"
#endif

#ifndef	TTYFNAME
#define	TTYFNAME	"/dev/tty"
#endif

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	PROG_TTY
#define	PROG_TTY	"/usr/bin/tty"
#endif

#ifndef	VARTERMDEV
#define	VARTERMDEV	"TERMDEV"
#endif

#ifndef	VARAUDIODEV
#define	VARAUDIODEV	"AUDIODEV"
#endif

#define	TO_READ		20		/* timeout waiting for sub-process */

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,char *,int) ;


/* local structures */

struct subinfo_flags {
	uint		init:1 ;
} ;

struct subinfo {
	char		*dbuf ;
	SUBINFO_FL	f ;
	int		dlen ;
	int		fd ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,char *,int,int) ;
static int	subinfo_finish(SUBINFO *) ;

static int	getname_var(SUBINFO *) ;
static int	getname_ttyname(SUBINFO *) ;
static int	getname_fork(SUBINFO *) ;


/* local variables */

static int	(*getnames[])(SUBINFO *) = {
	getname_var,
	getname_ttyname,
	getname_fork,
	NULL
} ;


/* exported subroutines */


int termdevice(char *dbuf,int dlen,int fd)
{
	int		rs = SR_NOENT ;

	if (fd < 0) return SR_BADF ;

	if (dbuf == NULL) return SR_FAULT ;

	if (dlen < MINBUFLEN) return SR_OVERFLOW ;

	if (isatty(fd)) {
	    SUBINFO	si ;
	    dbuf[0] = '\0' ;
	    if ((rs = subinfo_start(&si,dbuf,dlen,fd)) >= 0) {
	        int	i ;
	        for (i = 0 ; getnames[i] != NULL ; i += 1) {
	            rs = (getnames[i])(&si) ;
	            if (rs != 0) break ;
	        } /* end for */
	        subinfo_finish(&si) ;
	    } /* end if (subinfo) */
	} else {
	    rs = SR_NOTTY ;
	}

	return rs ;
}
/* end subroutine (termdevice) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,char *dbuf,int dlen,int fd)
{
	int		rs = SR_OK ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->fd = fd ;
	sip->dbuf = dbuf ;
	sip->dlen = dlen ;

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_finish(SUBINFO *sip)
{
	if (sip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (subinfo_finish) */


static int getname_var(SUBINFO *sip)
{
	struct ustat	st1, st2 ;
	int		rs ;
	int		len = 0 ;

	if ((rs = u_fstat(sip->fd,&st1)) >= 0) {
	    cchar	*cp ;
	    if ((cp = getenv(VARTERMDEV)) != NULL) {
	        if ((rs = u_stat(cp,&st2)) >= 0) {
	            rs = SR_NOENT ;
	            if (st1.st_rdev == st2.st_rdev) {
	                rs = sncpy1(sip->dbuf,sip->dlen,cp) ;
	                len = rs ;
	            }
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        }
	    } /* end if (environment variable worked out !) */
	} /* end if (stat) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getname_var) */


static int getname_ttyname(SUBINFO *sip)
{
	int		rs ;
	int		len = 0 ;

	if ((rs = uc_ttyname(sip->fd,sip->dbuf,sip->dlen)) >= 0) {
	    len = strlen(sip->dbuf) ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getname_ttyname) */


static int getname_fork(SUBINFO *sip)
{
	int		rs ;
	int		len = 0 ;
	int		pfds[2] ;

	if ((rs = u_pipe(pfds)) >= 0) {
	    const int	dlen = MAXPATHLEN ;
	    int		i = 0 ;
	    int		cs ;
	    int		fd = sip->fd ;
	    char	dbuf[MAXPATHLEN + 2] = { '\0' } ;

	    if ((rs = uc_fork()) == 0) {
	        const char	*av[3] ;

#if	CF_DEBUGS
	        debugprintf("termdevice: child process\n") ;
#endif

	        u_close(pfds[0]) ; /* not used */

	        for (i = 0 ; i < 3 ; i += 1) u_close(i) ;

	        u_dup(fd) ;
	        u_close(fd) ;		/* done using it */

	        u_dup(pfds[1]) ;		/* standard output */
	        u_close(pfds[1]) ;		/* done using it */

	        u_open(NULLFNAME,O_WRONLY,0666) ;

	        av[0] = "tty" ;
	        av[1] = NULL ;

	        u_execv(PROG_TTY,av) ;

	        uc_exit(EX_NOEXEC) ;

	    } else if (rs > 0) {
	        const pid_t	pid_child = rs ;
	        const int	to = TO_READ ;
	        const int	opts = FM_TIMED ;

	        u_close(pfds[1]) ;	/* not used */
	        pfds[1] = -1 ;

	        if ((rs = uc_reade(pfds[0],dbuf,dlen,to,opts)) > 0) {
	            len = rs ;
	            for (i = 0 ; i < len ; i += 1) {
	                if (dbuf[i] == '\n') break ;
	            }
	            if (i == len) rs = SR_RANGE ;
	            dbuf[i] = '\0' ;
	        } /* end if */

#if	CF_DEBUGS
	        debugprintf("termdevice: termdevice=%s\n",dbuf) ;
#endif

	        u_waitpid(pid_child,&cs,0) ;

	    } else {
	        rs = SR_BADE ;
	    }

	    u_close(pfds[0]) ;
	    if (pfds[1] >= 0) {
	        u_close(pfds[1]) ;
	    }

	    if (rs >= 0) {
	        rs = snwcpy(sip->dbuf,sip->dlen,dbuf,i) ;
	        len = rs ;
	    }

	} /* end if (pipe) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getname_fork) */


