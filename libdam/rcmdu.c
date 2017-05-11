/* rcmdu */

/* get connection to remote command (unpriviledged) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_TESTRCMD	0		/* first test the connection */
#define	CF_PIPES	0		/* use System V pipes */
#define	CF_CMDPATH	1		/* try PATH for finding cmd_rsh */


/* revision history:

	= 1998-07-10, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a dialer to use the underlying RSH program to make
	a "SHELL" remote connection to another machine.


******************************************************************************/


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

#ifndef	PROG_ECHO
#define	PROG_ECHO	"/bin/echo"
#endif

#define	RSH_NAME	"rsh"

#define	CMDLEN		(64 * 1024)

#define	NTRIES		1
#define	NWAITINT	1

#define	TESTRCMD_LOOKLEN	64

#define	DUMPLEN			50


/* external subroutines */

extern int	getehostname(const char *,char *) ;
extern int	findfilepath(const char *,char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */

extern char 	**environ ;


/* forward reference */

#if	CF_TESTRCMD
static int	testrcmdu(const char *,const char *,const char *,int) ;

static void	dump() ;
#endif /* CF_TESTRCMD */


/* local variables */

static const char	*rshs[] = {
	"/usr/bin/rsh",
	"/usr/bin/remsh",
	"/bin/rsh",
	"/bin/remsh",
	"/usr/ucb/rsh",
	NULL
} ;


/* exported subroutines */


int rcmdu(rhost,ruser,cmd,fd2p)
const char	rhost[] ;
const char	ruser[] ;
const char	cmd[] ;
int		*fd2p ;
{
	int	rs = SR_OK ;
	int	i, j ;
	int	pipes[3][2] ;

	const char	*cmd_rsh = NULL ;
	const char	*rsh_name = RSH_NAME ;

	char	cmdfname[MAXPATHLEN + 1] ;
	char	ehostname[MAXHOSTNAMELEN + 1] ;


#if	CF_DEBUGS
	debugprintf("rcmdu: rhost=%s\n",rhost) ;
	debugprintf("rcmdu: ruser=%s\n",ruser) ;
	debugprintf("rcmdu: cmd=>%t<\n",
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
	debugprintf("rcmdu: got in\n") ;
#endif

/* where is the RSH program */

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
	} /* end if */

#if	CF_DEBUGS
	debugprintf("rcmdu: rs=%d cmd_rsh=%s\n",rs,cmd_rsh) ;
#endif

/* test the host name for addressability */

	if ((rs >= 0) && (inet_addr(rhost) == NOADDR)) {

	    rs = getehostname(rhost,ehostname) ;

	    if ((rs >= 0) && (strcmp(rhost,ehostname) != 0))
	        rhost = ehostname ;

	} /* end if */

	if (rs < 0)
	    goto ret0 ;

/* test the remote host for accessibility */

#if	CF_TESTRCMD
	rs = SR_AGAIN ;
	for (i = 0 ; i < NTRIES ; i += 1) {

	    if (i > 0)
	        sleep(NWAITINT) ;

	    rs = testrcmdu(rhost,ruser,cmd_rsh,to) ;
	    if (rs != SR_AGAIN)
	        break ;

	} /* end for */
#endif /* CF_TESTRCMD */

	if (rs < 0) 
	    goto ret0 ;

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
	debugprintf("rcmdu: got pipes \n") ;
#endif

/* we fork RSH */

#if	CF_DEBUGS
	debugprintf("rcmdu: about to fork\n") ;
#endif

	rs = uc_fork() ;
	if (rs < 0)
	    goto badfork ;

	if (rs == 0) {
	   int		rs1 ;
	   const char	*av[6] ;


#if	CF_DEBUGS
	    debugprintf("rcmdu: child cmd_rsh=%s\n",cmd_rsh) ;
#endif

	    for (i = 0 ; i < 3 ; i += 1)
	        u_close(i) ;

	    u_dup(pipes[1][1]) ;

	    u_dup(pipes[1][1]) ;

	    u_close(pipes[1][0]) ;

	    if (fd2p != NULL) {

	        u_dup(pipes[2][1]) ;

		for (j = 0 ; j < 2 ; j += 1)
	            u_close(pipes[2][j]) ;

	    } else {

	        u_dup(pipes[1][1]) ;

	    }

	    u_close(pipes[1][1]) ;

/* just check some other stuff a little bit */

#if	(! CF_DEBUGS)
	    for (i = 3 ; i < NOFILE ; i += 1)
		u_close(i) ;
#endif

/* do the exec */

	    i = 0 ;
	    av[i++] = rsh_name ;
	    if (ruser != NULL) {
	        av[i++] = "-l" ;
	        av[i++] = ruser ;
	    }
	    av[i++] = rhost ;
	    av[i++] = cmd ;
	    av[i] = NULL ;
	    {
		const char **eav = (const char **) av ;
		const char **eev = (const char **) environ ;
	        rs1 = u_execve(cmd_rsh,eav,eev) ;
	    }

#if	CF_DEBUGS
	debugprintf("rcmdu: exec() rs=%d \n",rs1) ;
#endif

	    uc_exit(EX_NOEXEC) ;
	} /* end if (child) */

#if	CF_DEBUGS
	debugprintf("rcmdu: mainline continue\n") ;
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
	debugprintf("rcmdu: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
badfork:
	u_close(pipes[1][0]) ;

	u_close(pipes[1][1]) ;

	if (fd2p != NULL) {
	    for (j = 0 ; j < 2 ; j += 1)
	        u_close(pipes[2][j]) ;
	}

badpipe:
	goto ret0 ;
}
/* end subroutine (rmcdu) */


/* local subroutines */


#if	CF_TESTRCMD

static int testrcmdu(rhost,ruser,cmd_rsh,to)
const char	rhost[] ;
const char	ruser[] ;
const char	cmd_rsh[] ;
int		to ;
{
	pid_t	pid_child ;

	int	rs = SR_OK ;
	int	pipes[3][2] ;
	int	child_stat ;
	int	f_done1, f_done2 ;
	int	len1, len2 ;
	int	bl ;
	int	i ;

	const char	*prog_echo = PROG_ECHO ;
	const char	*ss = "YES" ;

	char	buf1[TESTRCMD_LOOKLEN + 1] ;
	char	buf2[TESTRCMD_LOOKLEN + 1] ;
	char	*bp ;


#if	CF_DEBUGS
	debugprintf("rcmdu/testrcmdu: rhost=%s\n",rhost) ;
	debugprintf("rcmdu/testrcmdu: ruser=%s\n",ruser) ;
	debugprintf("rcmdu/testrcmdu: cmd=%s\n",cmd_rsh) ;
#endif

	rs = socketpair(PF_UNIX,SOCK_STREAM,0,pipes[1]) ;
	if (rs < 0)
	    goto badpipe ;

	rs = socketpair(PF_UNIX,SOCK_STREAM,0,pipes[2])  ;
	if (rs < 0) {
	    u_close(pipes[1][0]) ;
	    u_close(pipes[1][1]) ;
	    goto badpipe ;
	}

#if	CF_DEBUGS
	debugprintf("rcmdu/testrcmdu: got pipes \n") ;
#endif

/* we fork RSH */

#if	CF_DEBUGS
	debugprintf("rcmdu/testrcmdu: about to fork\n") ;
#endif

	rs = uc_fork() ;
	pid_child = rs ;
	if (rs < 0)
	    goto badfork ;

	if (rs == 0) {

#if	CF_DEBUGS
	    debugprintf("rcmdu/testrcmdu: inside fork\n") ;
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
	            prog_echo,ss,NULL) ;

	    } else
	        execlp(cmd_rsh,"rsh","-n",rhost,
	            prog_echo,ss,NULL) ;

#if	CF_DEBUGS
	debugprintf("rcmdu/testrcmdu: exec() failed\n") ;
#endif

	    uc_exit(EX_NOEXEC) ;

	} /* end if (child) */

#if	CF_DEBUGS
	debugprintf("rcmdu/testrcmdu: mainline continue\n") ;
#endif

/* close some pipe ends */

	u_close(pipes[1][1]) ;

	u_close(pipes[2][1]) ;

/* the command is not reading the other end of this pipe anyway */

#if	(! CF_PIPES)
	u_shutdown(pipes[1][0],SHUT_WR) ;
#endif

/* read both the regular-output and the error-output for possible data */

	buf1[0] = buf2[0] = '\0' ;
	f_done1 = f_done2 = FALSE ;
	len1 = len2 = 0 ;
	while ((! f_done1) && (! f_done2)) {

	    if (! f_done1) {

		bp = (buf1 + len1) ;
		bl = (TESTRCMD_LOOKLEN - len1) ;
	        rs = u_read(pipes[1][0],bp,bl) ;
		len1 += rs ;

#if	CF_DEBUGS
	debugprintf("rcmdu/testrcmdu: 1 u_read() rs=%d\n",rs) ;
#endif

	        if (rs <= 0)
	            f_done1 = TRUE ;

	        if (len1 >= TESTRCMD_LOOKLEN) 
			f_done1 = TRUE ;

	    } /* end if */

	    if (! f_done2) {

		bp = (buf2 + len2) ;
		bl = (TESTRCMD_LOOKLEN - len2) ;
	        rs = u_read(pipes[2][0],bp,bl) ;
		len2 += rs ;

#if	CF_DEBUGS
	debugprintf("rcmdu/testrcmdu: 2 u_read() rs=%d\n",rs) ;
#endif

	        if (rs <= 0)
	            f_done2 = TRUE ;

	        if (len2 >= TESTRCMD_LOOKLEN) 
			f_done2 = TRUE ;

	    } /* end if */

	} /* end while */

/* compare who has what */

	if (rs >= 0) {

	if ((len1 == 0) && (len2 == 0))  {
	    rs = SR_HOSTDOWN ;

	} else if (strncmp(buf1,"YES",3) == 0) {
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
	    debugprintf("rcmdu/testrcmdu: far_side> %s\n",buf2) ;
#endif

	    rs = SR_PROTO ;

	}

	} /* end if */

/* we are out of here! */

	dump(pipes[1][0],pipes[2][0]) ;

	u_close(pipes[1][0]) ;

	u_close(pipes[2][0]) ;

	u_waitpid(pid_child,&child_stat,WUNTRACED) ;

ret0:

#if	CF_DEBUGS
	    debugprintf("rcmdu/testrcmdu: ret rs=%d\n",rs) ;
#endif

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


static void dump(fd1,fd2)
int	fd1, fd2 ;
{
	int	f_done1 = FALSE ;
	int	f_done2 = FALSE ;

	char	buf[DUMPLEN + 1] ;


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



