/* termdevice */

/* find the name of the device for the given file descriptor */


#define	CF_DEBUGS	0		/* debug print-outs */


/* revision history:

	= 1998-06-15, David A­D­ Morano

	This subroutine was originally written.  This was also inspired by
	the fact that the Sun Solaris 2.5.1 POSIX version of 'ttyname_r'
	does not appear to work.  I got the idea for this subroutine
	from the GNU standard C library implementation.  It seems like
	Slowlaris 5.x certainly had a lot of buggy problems (sockets,
	I/O, virtual memory, more)!

	= 2011-10-12, David A­D­ Morano

	I am changing the order of attempts to put 'ttyname_r(3c)' before
	forking a process.  Even though we are still on Slowlaris we
	hope that 'ttyname_r(3c)' is now working properly!

*/


/* **********************************************************************

	Store at most BUFLEN character of the pathname, if the terminal
	FD is open, in the caller specified buffer.

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


*************************************************************************/


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

static int	subinfo_start(struct subinfo *,char *,int,int) ;
static int	subinfo_finish(struct subinfo *) ;

static int	getname_var(struct subinfo *) ;
static int	getname_ttyname(struct subinfo *) ;
static int	getname_fork(struct subinfo *) ;

static int	istermrs(int) ;


/* local variables */

static int	(*getnames[])(struct subinfo *) = {
	getname_var,
	getname_ttyname,
	getname_fork,
	NULL
} ;

static const int	termrs[] = {
	SR_FAULT,
	SR_INVALID,
	SR_NOMEM,
	SR_NOANODE,
	SR_BADFMT,
	SR_NOSPC,
	SR_NOSR,
	SR_NOBUFS,
	SR_BADF,
	SR_OVERFLOW,
	SR_RANGE,
	0
} ;


/* exported subroutines */


int termdevice(char *dbuf,int dlen,int fd)
{
	struct subinfo	si ;

	int	rs = SR_NOENT ;

	if (fd < 0) return SR_BADF ;

	if (! isatty(fd)) return SR_NOTTY ;

	if (dbuf == NULL) return SR_FAULT ;

	if (dlen < MINBUFLEN) return SR_OVERFLOW ;

	dbuf[0] = '\0' ;
	if ((rs = subinfo_start(&si,dbuf,dlen,fd)) >= 0) {
	    int	i ;
	    for (i = 0 ; getnames[i] != NULL ; i += 1) {
	        rs = (getnames[i])(&si) ;
	        if ((rs >= 0) || istermrs(rs))
		    break ;
	    } /* end for */
	    subinfo_finish(&si) ;
	} /* end if (subinfo) */

	return rs ;
}
/* end subroutine (termdevice) */


/* local subroutines */


static int subinfo_start(sip,dbuf,dlen,fd)
struct subinfo	*sip ;
char		*dbuf ;
int		dlen ;
int		fd ;
{
	int	rs = SR_OK ;

	memset(sip,0,sizeof(struct subinfo)) ;
	sip->fd = fd ;
	sip->dbuf = dbuf ;
	sip->dlen = dlen ;

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_finish(sip)
struct subinfo	*sip ;
{

	return SR_OK ;
}
/* end subroutine (subinfo_finish) */


static int getname_var(sip)
struct subinfo	*sip ;
{
	struct ustat	st1, st2 ;

	int	rs ;

	if ((rs = u_fstat(sip->fd,&st1)) >= 0) {
	    const char	*cp ;
	    if ((cp = getenv(VARTERMDEV)) != NULL) {
	        if ((rs = u_stat(cp,&st2)) >= 0) {
		    rs = SR_NOENT ;
	            if (st1.st_rdev == st2.st_rdev) {
		        rs = sncpy1(sip->dbuf,sip->dlen,cp) ;
		    }
	        }
	    } /* end if (environment variable worked out !) */
	} /* end if (stat) */

	return rs ;
}
/* end subroutine (getname_var) */


static int getname_ttyname(sip)
struct subinfo	*sip ;
{
	int	rs ;

	rs = uc_ttyname(sip->fd,sip->dbuf,sip->dlen) ;

	return rs ;
}
/* end subroutine (getname_ttyname) */


static int getname_fork(sip)
struct subinfo	*sip ;
{
	pid_t	pid_child ;

	int	rs ;
	int	cs ;
	int	len ;
	int	pfds[2] ;
	int	fd = sip->fd ;
	int	i = 0 ;

	char	devicename[MAXPATHLEN + 2] ;


	devicename[0] = '\0' ;
	rs = u_pipe(pfds) ;
	if (rs < 0)
	    goto ret0 ;

	rs = uc_fork() ;
	pid_child = rs ;
	if ((rs >= 0) && (pid_child == 0)) {
	    char	*av[3] ;

#if	CF_DEBUGS
	    debugprintf("termdevice: child process\n") ;
#endif

	    u_close(pfds[0]) ; /* not used */

	    for (i = 0 ; i < 3 ; i += 1) u_close(i) ;

	    u_dup(fd) ;
	    u_close(fd) ;		/* done using it */

	    u_dup(pfds[1]) ;		/* the pipe is now standard output */
	    u_close(pfds[1]) ;		/* done using it */

	    u_open(NULLFNAME,O_WRONLY,0666) ;

	    av[0] = "tty" ;
	    av[1] = NULL ;

	    u_execv(PROG_TTY,av) ;

	    uc_exit(EX_NOEXEC) ;

	} /* end if (chld_process) */

	u_close(pfds[1]) ;	/* not used */

	if (rs > 0) {

	    int	to = TO_READ ;
	    int	opts = FM_TIMED ;

	    rs = uc_reade(pfds[0],devicename,(MAXPATHLEN+1),to,opts) ;
	    len = rs ;
	    if (rs > 0) {

	        for (i = 0 ; i < len ; i += 1) {
	            if (devicename[i] == '\n')
	                break ;
	        }

	        if (i == len)
	            rs = SR_RANGE ;

	        devicename[i] = '\0' ;

	    } /* end if */

#if	CF_DEBUGS
	    debugprintf("termdevice: termdevice=%s\n",devicename) ;
#endif

	    u_waitpid(pid_child,&cs,0) ;

	} else
	    rs = SR_BADE ;

	u_close(pfds[0]) ;

	if (rs >= 0)
	    rs = snwcpy(sip->dbuf,sip->dlen,devicename,i) ;

ret0:
	return rs ;
}
/* end subroutine (getname_fork) */


static int istermrs(rs)
int	rs ;
{
	int	i ;
	int	f = FALSE ;

	for (i = 0 ; termrs[i] != 0 ; i += 1) {
	    f = (rs == termrs[i]) ;
	    if (f) break ;
	} /* end if */

	return f ;
}
/* end subroutine (istermrs) */


