/* watch */

/* watch (listen on) the specified socket */
/* version %I% last modified %G% */


#define	CF_DEBUG	1		/* switchable debug print-outs */
#define	CF_WHOOPEN	0		/* who is open */
#define	CF_SIGCHILD	0		/* catch SIGCHLD ? */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	This subroutine was adopted from the DWD program.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is responsible for listening on the given socket and
	spawning off a program to handle any incoming connection.

	Synopsis:

	int watch(pip,fd_listen,char *,char *)
	struct proginfo	*pip ;
	int		fd_listen ;
	char		progfname[] ;
	char		username[] ;

	Arguments:

	pip	program information pointer

	Returns:

	OK	doesn't really matter in the current implementation


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/time.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<sys/msg.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stropts.h>
#include	<poll.h>
#include	<signal.h>
#include	<time.h>
#include	<netdb.h>
#include	<dirent.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<varsub.h>
#include	<vecstr.h>
#include	<sockaddress.h>
#include	<exitcodes.h>
#include	<getax.h>
#include	<localmisc.h>

#include	"connection.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	IPCDIRMODE	0777
#define	W_OPTIONS	(WNOHANG)
#define	TI_POLL		10		/* u_poll(2) interval seconds */
#define	TI_MAINT	(3 * 60)	/* miscellaneous maintenance */

#define	IPCBUFLEN	MAXPATHLEN
#define	PWDBUFLEN	500
#define	MAXOUTLEN	62

#define	NIOVECS		1
#define	O_SRVFLAGS	(O_RDWR | O_CREAT)


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;

#if	CF_DEBUG
extern char	*d_reventstr() ;
#endif


/* externals variables */


/* local structures */

struct clientinfo {
	cchar		*username ;
	cchar		*progfname ;
	SOCKADDRESS	*fromp ;
	int		fromlen ;
	int		fd ;
} ;


/* forward references */

static int	spawnserver(struct proginfo *,int,struct clientinfo *) ;
static int	mktmp(struct proginfo *,char *) ;
static int	writeout() ;

static void	int_exit(int) ;
static void	int_child(int) ;


/* local global variables */

static int	f_exit, f_child ;


/* local variables */


/* exported subroutines */


int watch(pip,fd_listen,progfname,username)
struct proginfo	*pip ;
int		fd_listen ;
char		progfname[] ;
char		username[] ;
{
	struct sigaction	sigs ;
	struct clientinfo	ci ;
	struct ustat	sb ;
	struct pollfd	fds[4] ;
	SOCKADDRESS	from ;
	CONNECTION	conn ;
	sigset_t	signalmask ;
	int	rs, rs1, i ;
	int	fd, ifd = FD_STDIN ;
	int	nfd, nfds ;
	int	to_poll ;
	int	re, sl ;
	int	flen ;
	int	pid ;
	char	tmpbuf[100] ;
	char	*sp ;


	f_exit = FALSE ;
	f_child = FALSE ;


	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = int_exit ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGTERM,&sigs,NULL) ;


	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = int_exit ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGHUP,&sigs,NULL) ;


	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = int_exit ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGINT,&sigs,NULL) ;


#if	CF_SIGCHILD

	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = int_child ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = SA_NOCLDSTOP ;
	sigaction(SIGCHLD,&sigs,NULL) ;

#endif /* CF_SIGCHILD */


	nfds = 0 ;
	fds[nfds].fd = fd_listen ;
	fds[nfds].events = POLLIN | POLLPRI ;
	fds[nfds].revents = 0 ;
	nfds += 1 ;

	fds[nfds].fd = -1 ;

	while (! f_exit) {


	    to_poll = 10 * 1000 ;
	    rs = u_poll(fds,nfds,to_poll) ;

#if	CF_DEBUG
	    if (pip->debuglevel >= 4)
	        debugprintf("watch: back from poll w/ rs=%d f_child=%d\n",
	            rs,f_child) ;
#endif

	    if (rs > 0) {

	        for (nfd = 0 ; nfd < nfds ; nfd += 1) {

#if	CF_DEBUG
	            if (pip->debuglevel >= 4) {

	                re = fds[nfd].revents ;
	                debugprintf("watch: nfd=%d FD=%2d re=>%s<\n",
	                    nfd,fds[nfd].fd,
	                    d_reventstr(re,tmpbuf,BUFLEN)) ;

	                rs1 = u_fstat(fds[nfd].fd,&sb) ;

	                debugprintf("watch: nfd=%d FD=%2d open=%d\n",
	                    nfd,fds[nfd].fd,
	                    (rs1 >= 0)) ;

	            }
#endif /* CF_DEBUG */

/* handle any activity on our listen socket or FIFO */

	            if ((fds[nfd].fd == fd_listen) &&
	                ((re = fds[nfd].revents) != 0)) {

#if	CF_DEBUG
	                if (pip->debuglevel >= 4)
	                    debugprintf("watch: poll FD=%d re=>%s<\n",
	                        fds[nfd].fd,
	                        d_reventstr(re,tmpbuf,BUFLEN)) ;
#endif

	                if ((re & POLLIN) || (re & POLLPRI)) {

#if	CF_DEBUG
	                    if (pip->debuglevel >= 4)
	                        debugprintf("watch: poll n=%d p=%d\n",
	                            (re & POLLIN) ? 1 : 0,
	                            (re & POLLPRI) ? 1 : 0) ;
#endif

	                    flen = sizeof(SOCKADDRESS) ;
	                    rs = u_accept(fd_listen,&from,&flen) ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("watch: u_accept() rs=%d\n",rs) ;
#endif

	                    fd = rs ;
	                    if (rs >= 0) {
	                        ci.progfname = progfname ;
	                        ci.username = username ;
	                        ci.fromp = &from ;
	                        ci.fromlen = flen ;
				ci.fd = fd ;
	                        rs = spawnserver(pip,fd_listen,&ci) ;
	                    }

	                } /* end if (POLLIN) */

	            } /* end if (got a request) */

/* handle other FDs here */

	        } /* end for (looping through FDs) */

	    } /* end if (got something back from polling) */

/* do some other periodic checking things here */

	} /* end while */

	logfile_printf(&pip->lh,"listener exiting\n") ;

	return rs ;
}
/* end subroutine (watch) */


/* local subroutines */


static int spawnserver(pip,fd_listen,cip)
struct proginfo		*pip ;
int			fd_listen ;
struct clientinfo	*cip ;
{
	int	rs ;
	int	i ;
	int	ifd = FD_STDIN ;
	int	pid ;

	char	peername[MAXHOSTNAMELEN + 1] ;
	char	*username ;


	bflush(pip->efp) ;

	rs = uc_fork() ;

	if (rs == 0) {
	    CONNECTION	conn ;

	    connection_start(&conn,pip->domainname) ;

/* can we get the peername of the other end o f this socket, if a socket ? */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: connection_peername()\n") ;
#endif

	    if (cip->fromlen > 0) {
	        int		af ;
	        in_addr_t	addr1, addr2 ;

	        rs = sockaddress_getaf(cip->fromp) ;
		af = rs ;

	        if ((rs >= 0) && (af == ACF_INET)) {

	            rs = sockaddress_getaddr(cip->fromp,
	                (char *) &addr1,sizeof(in_addr_t)) ;

	            addr2 = htonl(0x40c05865) ;

	            if ((rs >= 0) && 
	                (memcmp(&addr1,&addr2,sizeof(in_addr_t)) == 0)) {
	                rs = strwcpy(peername,
	                    "levo.levosim.org",-1) - peername ;

	            } else
	                rs = -1 ;

	        } /* end if (address family INET4) */

	        if (rs < 0)
	            rs = connection_peername(&conn,
	                cip->fromp,cip->fromlen,peername) ;

	    } else
	        rs = connection_sockpeername(&conn,peername,ifd) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: connection_peername rs=%d\n",
	            rs) ;
#endif

	    if (rs > 0) {
	        logfile_printf(&pip->lh,"from host=%s\n",
	            peername) ;
	    }

/* spawn the real server */

	    bflush(pip->efp) ;

	    pid = uc_fork() ;

	    if (pid == 0) {
	        char	*av[4] ;

	        bclose(pip->efp) ;

	        for (i = 0 ; i < 3 ; i += 1)
	            u_close(i) ;

	        u_close(fd_listen) ;

	        u_dup(cip->fd) ;

	        u_dup(cip->fd) ;

	        u_open("/dev/null",O_WRONLY,0666) ;

	        u_close(cip->fd) ;

	        {
	            struct passwd	pe ;
	            char	pwdbuf[PWDBUFLEN + 1] ;

	            username = cip->username ;
	            if (username != NULL) {

	                rs = getpw_name(&pe,pwdbuf,PWDBUFLEN,username) ;

	                if (rs < 0)
	                    logfile_printf(&
	                        pip->lh,
	                        "notfound username=%s\n",
	                        username) ;

	            }

	            if (rs < 0) {

	                username = DEFUSERNAME ;
	                rs = getpw_name(&pe,pwdbuf,PWDBUFLEN,username) ;

	                if (rs < 0)
	                    logfile_printf(&
	                        pip->lh,
	                        "notfound username=%s\n",
	                        username) ;

	            }

	            if (rs >= 0) {
	                uc_initgroups(username, pe.pw_gid) ;
	                u_setgid(pe.pw_gid) ;
	                u_setuid(pe.pw_uid) ;
	            } else {
	                u_setgid(DEFUID) ;
	                u_setuid(DEFGID) ;
	            }

	        } /* end clock (setting new IDs) */

	        av[0] = cip->progfname ;
	        av[1] = NULL ;
	        av[2] = NULL ;
	        u_execv(cip->progfname,av) ;

	        uc_exit(EX_NOEXEC) ;
	    } /* end if (child) */

	    if (pip > 0) {
	        int	childstat ;

	        logfile_printf(&pip->lh,"request pid=%d\n",
	            pid) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: waiting for child pid=%d\n",
	                pid) ;
#endif

	        u_waitpid(pid,&childstat,0) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: child exited pid=%d\n",
	                pid) ;
#endif

	        logfile_printf(&pip->lh,"server exited ex=%d\n",
	            (childstat & 0xff)) ;

	    } else
	        logfile_printf(&pip->lh,
	            "failed to start server (%d)\n",pid) ;

	    connection_finish(&conn) ;

	    uc_exit(EX_OK) ;
	} /* end if (child) */

	if (rs < 0)
	    sleep(1) ;

	return rs ;
}
/* end subroutine (spawnserver) */


static void int_exit(sn)
int	sn ;
{

	f_exit = TRUE ;
}
/* end subroutine (int_exit) */


static void int_child(sn)
int	sn ;
{

	f_child = TRUE ;
}


#ifdef	COMMENT

/* write out the output files from the executed program */
static int writeout(pip,fd,s)
struct proginfo	*pip ;
int	fd ;
char	s[] ;
{
	struct ustat	sb ;
	bfile		file, *fp = &file ;
	int		tlen, len ;
	char		lbuf[LINELEN + 1] ;

	tlen = 0 ;
	if ((u_fstat(fd,&sb) >= 0) && (sb.st_size > 0)) {

	    u_rewind(fd) ;

	    logfile_printf(&pip->lh,s) ;

	    if (bopen(fp,(char *) fd,"dr",0666) >= 0) {

	        while ((len = breadline(fp,lbuf,MAXOUTLEN)) > 0) {

	            tlen += len ;
	            if (lbuf[len - 1] == '\n') lbuf[--len] = '\0' ;

	            logfile_printf(&pip->lh,"| %W\n",
	                lbuf,MIN(len,MAXOUTLEN)) ;

	        } /* end while (reading lines) */

	        bclose(fp) ;
	    } /* end if (opening file) */

	} /* end if (non-zero file size) */

	return tlen ;
}
/* end subroutine (writeout) */


/* make the private TMP area */
static int mktmp(pip,jobdname)
struct proginfo	*pip ;
char		jobdname[] ;
{
	int	rs ;
	int	bnlen, len ;
	int	sumask ;
	char	buf[MAXPATHLEN + 1] ;
	char	*bn ;

/* make some directories if need be */

	bnlen = sfbasename(pip->programroot,-1,&bn) ;

	if (bn[bnlen] != '\0') {

	    strwcpy(buf,bn,bnlen) ;

	    bn = buf ;

	}

	len = mkpath3(jobdname,pip->tmpdname,bn,pip->searchname) ;

#ifdef	OPTIONAL
	sumask = umask(0000) ;
#endif

	rs = mkdirs(jobdname,IPCDIRMODE) ;

#ifdef	OPTIONAL
	umask(sumask) ;
#endif

	if (rs < 0)
	    return rs ;

	u_chmod(jobdname,IPCDIRMODE | S_ISVTX) ;

	return len ;
}
/* end subroutine (mktmp) */

#endif /* COMMENT */


