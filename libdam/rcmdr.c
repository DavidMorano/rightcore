/* rcmdr */

/* get connection to remote command (unpriviledged) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_TESTRCMD	0		/* run the access-test code? */
#define	CF_PIPES	0		/* use System V pipes */
#define	CF_CMDPATH	1		/* try PATH for finding cmd_rsh */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a dialer to use the underlying RSH program to make a "SHELL"
        remote login connection to another machine.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/wait.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#if	defined(IRIX) && (! defined(TYPEDEF_INADDRT))
#define	TYPEDEF_INADDRT	1
typedef unsigned int	in_addr_t ;
#endif

#ifndef	NOADDR
#define	NOADDR		((in_addr_t) (~ 0))
#endif

#ifndef	NOFILE
#define	NOFILE		20
#endif

#define	CMDLEN		(64 * 1024)
#define	NTRIES		3
#define	NWAITINT	1
#define	TO_READ		5

#define	TESTRCMDU_LOOKLEN	64

#define	DUMPLEN			50


/* external subroutines */

extern int	getehostname(const char *,char *) ;
extern int	findfilepath(const char *,char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* forward references */

#if	CF_TESTRCMD
static int	testrcmdu(const char *,const char *,const char *,int) ;

static void	dump() ;
#endif /* CF_TESTRCMD */

#if	CF_DEBUGS
static int showdev(int) ;
#endif


/* local variables */

static const char	*rshs[] = {
	"/usr/bin/rsh"
	"/usr/bin/remsh",
	"/usr/bin/ssh",
	"/usr/bin/ssh2",
	"/usr/bin/ssh1",
	"/usr/ucb/rsh",
	NULL
} ;


/* exported subroutines */


int rcmdr(rhost,ruser,cmd,fd2p)
const char	rhost[] ;
const char	ruser[] ;
const char	cmd[] ;
int		*fd2p ;
{
	int	rs = SR_OK ;
	int	i, j ;
	int	pipes[3][2] ;

	const char	*cmd_rsh = NULL ;

	char	cmdfname[MAXPATHLEN + 1] ;
	char	ehostname[MAXHOSTNAMELEN + 1] ;


#if	CF_DEBUGS
	debugprintf("rcmdr: rhost=%s\n",rhost) ;
	debugprintf("rcmdr: ruser=%s\n",ruser) ;
	debugprintf("rcmdr: cmd=>%t<\n",
	    cmd,strlinelen(cmd,-1,50)) ;
#endif

/* check for bad input */

	if ((rhost == NULL) || (rhost[0] == '\0'))
	    return SR_INVALID ;

	if ((ruser != NULL) && (strlen(ruser) > USERNAMELEN))
	    return SR_TOOBIG ;

	if ((cmd == NULL) || (cmd[0] == '\0'))
	    return SR_INVALID ;

	if ((strlen(cmd) + 6) > CMDLEN)
	    return SR_2BIG ;

#if	CF_DEBUGS
	debugprintf("rcmdr: got in\n") ;
#endif

/* find the RSH program */

#if	CF_CMDPATH
	cmd_rsh = cmdfname ;
	rs = findfilepath(NULL,cmdfname,"rsh",X_OK) ;
#else
	rs = SR_NOENT ;
#endif

	if (rs == SR_NOENT) {

	    for (i = 0 ; rshs[i] != NULL ; i += 1) {
	        cmd_rsh = rshs[i] ;
	        rs = u_access(cmd_rsh,X_OK) ;
	        if (rs >= 0) break ;
	    } /* end for */

	    if (rshs[i] == NULL) rs = SR_NOPKG ;

	} /* end if (trying alternatives) */

#if	CF_DEBUGS
	debugprintf("rcmdr: rs=%d cmd_rsh=%s\n",rs,cmd_rsh) ;
#endif

/* test the host name for addressability */

	if ((rs >= 0) && (inet_addr(rhost) == NOADDR)) {

	    if (getehostname(rhost,ehostname) < 0)
	        rs = SR_HOSTUNREACH ;

	    if ((rs >= 0) && (strcmp(rhost,ehostname) != 0))
	        rhost = ehostname ;

	} /* end if (needed name service) */

	if (rs < 0)
	    goto ret0 ;

#if	CF_TESTRCMD

/* test the remote host for accessibility */

	rs = SR_AGAIN ;
	for (i = 0 ; i < NTRIES ; i += 1) {

	    if (i > 0)
	        sleep(NWAITINT) ;

	    rs = testrcmdu(cmd_rsh,rhost,ruser) ;

	    if (rs != SR_AGAIN)
	        break ;

	} /* end for */

	if (rs < 0)
	    goto ret0 ;

#endif /* CF_TESTRCMD */

/* open up the necessary pipes */

#if	CF_PIPES
	rs = u_pipe(pipes[1]) ;
#else
	rs = u_socketpair(PF_UNIX,SOCK_STREAM,0,pipes[1]) ;
#endif

	if (rs < 0)
	    goto badpipe ;

	if (fd2p != NULL) {

#if	CF_PIPES
	    rs = u_pipe(pipes[2]) ;
#else
	    rs = u_socketpair(PF_UNIX,SOCK_STREAM,0,pipes[2]) ;
#endif

	    if (rs < 0) {

	        for (j = 0 ; j < 2 ; j += 1)
	            u_close(pipes[1][j]) ;

	        goto badpipe ;
	    }

	} /* end if */

#if	CF_DEBUGS
	debugprintf("rcmdr: got pipes \n") ;
#endif

/* we fork RSH */

#if	CF_DEBUGS
	debugprintf("rcmdr: about to fork\n") ;
#endif

	rs = uc_fork() ;
	if (rs < 0)
	    goto badfork ;

	if (rs == 0) {

#if	CF_DEBUGS
	    debugprintf("rcmdr: inside fork\n") ;
	    showdev(pipes[1][1]) ;
	    if (fd2p != NULL)
	        showdev(pipes[2][1]) ;
#endif

	    for (i = 0 ; i < 3 ; i += 1)
	        u_close(i) ;

	    u_dup(pipes[1][1]) ;

	    u_dup(pipes[1][1]) ;

	    u_close(pipes[1][0]) ;

	    if (fd2p != NULL) {
	        u_dup(pipes[2][1]) ;
	        for (j = 0 ; j < 2 ; j += 1) {
	            u_close(pipes[2][j]) ;
		}
	    } else {
	        u_dup(pipes[1][1]) ;
	    }

	    u_close(pipes[1][1]) ;

#if	CF_DEBUGS
	    showdev(0) ;
	    showdev(1) ;
	    if (fd2p != NULL)
	        showdev(2) ;
#endif

/* just check some other stuff a little bit */

#if	(! CF_DEBUGS)
	    for (i = 3 ; i < NOFILE ; i += 1)
	        u_close(i) ;
#endif

/* do the exec */

#if	CF_DEBUGS
	    debugprintf("rcmdr: RSH rhost=%s cmd=>%s<\n",rhost,cmd) ;
#endif

	    if (ruser != NULL) {
	        execlp(cmd_rsh,"rsh","-l",ruser,rhost,cmd,NULL) ;
	    } else {
	        execlp(cmd_rsh,"rsh",rhost,cmd,NULL) ;
	    }

#if	CF_DEBUGS
	    debugprintf("rcmdr: exec() failed\n") ;
#endif

	    uc_exit(EX_NOEXEC) ;
	} /* end if (child) */

#if	CF_DEBUGS
	debugprintf("rcmdr: parent continues here\n") ;
#endif

/* close some pipe ends */

	u_close(pipes[1][1]) ;

	if (fd2p != NULL) {
	    *fd2p = pipes[2][0] ;
	    u_close(pipes[2][1]) ;
	}

	rs = pipes[1][0] ;

ret0:

#if	CF_DEBUGS
	debugprintf("rcmdr: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
badfork:
	u_close(pipes[1][0]) ;

	u_close(pipes[1][1]) ;

	if (fd2p != NULL) {
	    for (j = 0 ; j < 2 ; j += 1) {
	        u_close(pipes[2][j]) ;
	    }
	}

badpipe:
	goto ret0 ;
}
/* end subroutine (rmcdr) */


/* local subroutines */


#if	CF_TESTRCMD

static int testrcmdu(cmd_rsh,rhost,ruser)
const char	cmd_rsh[] ;
const char	rhost[] ;
const char	ruser[] ;
{
	pid_t	pid_child ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	pipes[3][2] ;
	int	child_stat ;
	int	len1, len2 ;
	int	i ;
	int	f_done1 ;
	int	f_done2 ;

	char	buf1[TESTRCMDU_LOOKLEN + 1] ;
	char	buf2[TESTRCMDU_LOOKLEN + 1] ;


	rs = u_socketpair(PF_UNIX,SOCK_STREAM,0,pipes[1]) ;
	if (rs < 0)
	    goto badpipe ;

	rs = u_socketpair(PF_UNIX,SOCK_STREAM,0,pipes[2]) ;
	if (rs < 0) {

	    u_close(pipes[1][0]) ;

	    u_close(pipes[1][1]) ;

	    goto badpipe ;
	}

#if	CF_DEBUGS
	debugprintf("rcmdr: got pipes \n") ;
#endif

/* we fork RSH */

#if	CF_DEBUGS
	debugprintf("rcmdr: fork()\n") ;
#endif

	rs = uc_fork() ;
	pid_child = rs ;
	if (rs < 0)
	    goto ret0 ;

	if (rs == 0) {

#if	CF_DEBUGS
	    debugprintf("rcmdr: child\n") ;
#endif

	    for (i = 0 ; i < 3 ; i += 1)
	        u_close(i) ;

	    u_dup(pipes[1][1]) ;

	    u_dup(pipes[1][1]) ;

	    u_close(pipes[1][0]) ;

	    u_close(pipes[1][1]) ;

	    u_dup(pipes[2][1]) ;

	    u_close(pipes[2][0]) ;

	    u_close(pipes[2][1]) ;

/* do the exec */

	    if (ruser != NULL) {
	        execlp(cmd_rsh,"rsh","-n","-l",ruser,rhost,
	            "/bin/echo","YES",NULL) ;
	    } else {
	        execlp(cmd_rsh,"rsh","-n",rhost,
	            "/bin/echo","YES",NULL) ;
	    }

	    uc_exit(EX_NOEXEC) ;
	} /* end if */

/* parent continues here */

#if	CF_DEBUGS
	debugprintf("rcmdr: parent continues\n") ;
#endif

/* close some pipe ends */

	u_close(pipes[1][1]) ;

	u_close(pipes[2][1]) ;

/* the command is not reading the other end of this pipe anyway */

#if	(! CF_PIPES)
	u_shutdown(pipes[1][0],SHUT_WR) ;
#endif

/* read both the standard output and the error output for possible data */

	buf1[0] = buf2[0] = '\0' ;
	f_done1 = f_done2 = FALSE ;
	len1 = len2 = 0 ;
	while ((! f_done1) && (! f_done2)) {

	    if (! f_done1) {

	        rs = u_read(pipes[1][0],(buf1 + len1),
	            (TESTRCMDU_LOOKLEN - len1)) ;

	        if (rs <= 0) {
	            f_done1 = TRUE ;
	        } else {
	            len1 += rs ;
		}

	        if (len1 >= TESTRCMDU_LOOKLEN) {
	            f_done1 = TRUE ;
		}

	    }

	    if (! f_done2) {

	        rs = u_read(pipes[2][0],buf2 + len2,
	            TESTRCMDU_LOOKLEN - len2) ;

	        if (rs <= 0) {
	            f_done2 = TRUE ;
		} else {
	            len2 += rs ;
		}

	        if (len2 >= TESTRCMDU_LOOKLEN) {
	            f_done2 = TRUE ;
		}

	    }

	} /* end while */

/* compare who has what */

	if ((len1 == 0) && (len2 == 0))
	    rs = SR_HOSTDOWN ;

	if (strncmp(buf1,"YES",3) == 0) {
	    rs = SR_OK ;

	} else if (strncmp(buf2,"permi",5) == 0) {
	    rs = SR_ACCES ;

	} else if (strncmp(buf2,"Login",5) == 0) {
	    rs = SR_ACCES ;

	} else if (strncmp(buf2,"comma",5) == 0) {
	    rs = SR_2BIG ;

	} else if (strncmp(buf2,"Hostn",5) == 0) {
	    rs = SR_NOTSUP ;

	} else if (strncmp(buf2,"Try a",5) == 0) {
	    rs = SR_AGAIN ;

	} else if (strncmp(buf2,"Can't",5) == 0) {
	    rs = SR_AGAIN ;

	} else {

#if	CF_DEBUGS
	    debugprintf("rcmdr: far_side> %s\n",buf2) ;
#endif

	    rs = SR_PROTO ;

	}

/* we are out of here! */

	dump(pipes[1][0],pipes[2][0]) ;

	u_close(pipes[1][0]) ;

	u_close(pipes[2][0]) ;

	u_waitpid(pid_child,&child_stat,0) ;

ret0:
	return rs ;

/* bad returns here */
badfork:
	u_close(pipes[1][0]) ;

	u_close(pipes[1][1]) ;

	u_close(pipes[2][0]) ;

	u_close(pipes[2][1]) ;

badpipe:
	goto ret0 ;
}
/* end subroutine (testrmcdu) */


static void dump(int fd1,int fd2)
{
	int		f_done1 = FALSE ;
	int		f_done2 = FALSE ;
	char		buf[DUMPLEN + 1] ;

	while ((! f_done1) || (! f_done2)) {

	    if (! f_done1) {
	        if (u_read(fd1,buf,DUMPLEN) <= 0)
	            f_done1 = TRUE ;
	    }

	    if (! f_done2) {
	        if (u_read(fd2,buf,DUMPLEN) <= 0)
	            f_done2 = TRUE ;
	    }

	} /* end while */

}
/* end subroutine (dump) */

#endif /* CF_TESTRCMD */


#if	CF_DEBUGS
static int showdev(fd)
int	fd ;
{
	struct ustat	sb ;
	int		rs ;
	rs = u_fstat(fd,&sb) ;
	debugprintf("rcmdr: fd=%d rs=%d ino=%u dev=%08x\n",
	    fd,rs,sb.st_ino,sb.st_dev) ;
	return rs ;
}
/* end subroutine (showdev) */
#endif /* CF_DEBUGS */


