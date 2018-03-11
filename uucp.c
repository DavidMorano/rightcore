/* uucp */

/* get a FD to a file from another host using UUCP */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_BDUMP	1		/* dump extraneous output */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine opens a file on a remote machine using UUCP. If then
        return an FD to the open file to caller.

	Synopsis:

	int uucp(rhost,filename,fd2p)
	const char	rhost[] ;
	const char	filename[] ;
	int		*fd2p ;

	Arguments:

	rhost		nodename of UUCP machine to contact
	filename	filename on remote machine to write
	fd2p		pointer to an integer to receive an FD to
			standard error from queuing process

	Returns:

	>=0		FD to remote file
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/wait.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<strings.h>		/* |strncasecmp(3c)| */
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#define	PROG_UUNAME	"/usr/bin/uuname"
#define	PROG_UUCP	"/usr/bin/uucp"

#define	CACHEDIR	"/var/tmp/cache"
#define	DUMPLEN		50
#define	DSTLEN		(2 * MAXPATHLEN)


/* external subroutines */

#if	defined(BSD) && (! defined(EXTERN_STRNCASECMP))
extern int	strncasecmp(const char *,const char *,int) ;
#endif

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strshrink(char *) ;


/* external variables */


/* forward reference */

static int	testuucp(const char *) ;

#if	CF_BDUMP
static void	bdump(bfile *,bfile *) ;
#endif


/* local variables */


/* exported subroutines */


int uucp(cchar *rhost,cchar *filename,int *fd2p)
{
	const mode_t	omode = (0600 | S_IFIFO) ;
	int		rs = SR_OK ;
	int		i ;
	int		pipes[3][2] ;
	int		pfd ;
	const char	*cmd_uucp = PROG_UUCP ;
	char		pfname[MAXPATHLEN + 1] ;
	char		dst[DSTLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("uucp: ent\n") ;
#endif

/* check for bad input */

	if ((rhost == NULL) || (rhost[0] == '\0'))
	    return SR_INVAL ;

	if (u_access(cmd_uucp,X_OK) < 0) 
	    return SR_PROTO ;

#if	CF_DEBUGS
	debugprintf("uucp: got in\n") ;
#endif

/* test the remote host for accessibility */

	rs = testuucp(rhost) ;
	if (rs < 0)
	    return SR_HOSTUNREACH ;

#if	CF_DEBUGS
	debugprintf("uucp: continuing\n") ;
#endif

	pfname[0] = '\0' ;

/* open up the necessary pipes */

	if ((fd2p != NULL) && ((rs = u_pipe(pipes[2])) < 0))
	    goto badpipes ;

#if	CF_DEBUGS
	debugprintf("uucp: about to make pipe file\n") ;
#endif

	rs = mktmpfile(pfname,omode,"/tmp/uufileXXXXXXXX") ;
	if (rs < 0) {
	    rs = SR_PROTO ;
	    goto badpipemk ;
	}


#if	CF_DEBUGS
	debugprintf("uucp: got pipes \n") ;
#endif

/* form the necessary UUCP command */

	rs = sncpy3(dst,DSTLEN,rhost,"!",filename) ;
	if (rs < 0) goto baddst ;

/* we fork the command */

#if	CF_DEBUGS
	debugprintf("uucp: about to fork\n") ;
#endif

	if ((rs = uc_fork()) == 0) { /* child */
	    int		fd ;

#if	CF_DEBUGS
	    debugprintf("uucp: inside fork\n") ;
#endif

	    for (i = 0 ; i < 3 ; i += 1)
	        u_close(i) ;

	    fd = u_open("/dev/null",O_RDWR,0600) ;

	    u_dup(fd) ;

	    if (fd2p != NULL) {

	        u_close(pipes[2][0]) ;

	        u_dup(pipes[2][1]) ;

	        u_close(pipes[2][1]) ;

	    } else {

	        u_dup(fd) ;

	    }

/* do the exec */

	    execl(cmd_uucp,"uucp","-C",pfname,dst,NULL) ;

	    uc_exit(EX_NOEXEC) ;

	} else if (rs < 0)
		goto badfork ;

#if	CF_DEBUGS
	debugprintf("uucp: main line continue\n") ;
#endif

/* close some pipe ends */

	if (fd2p != NULL) {
	    u_close(pipes[2][1]) ;
	    pipes[2][1] = -1 ;
	}

/* open the pipe file */

#if	CF_DEBUGS
	debugprintf("uucp: about to open the pipe\n") ;
#endif

	rs = u_open(pfname,O_WRONLY,0600) ;
	pfd = rs ;
	if (rs < 0)
	    goto badopen ;

#if	CF_DEBUGS
	debugprintf("uucp: opened pipe, about to read answer\n") ;
	sleep(10) ;
	system("ps -f") ;
#endif

/* read out the answer */

#ifdef	COMMENT
	{
	int f_bad = FALSE ;
	while ((rs = reade(pipes[2][0],pfname,1,0,10)) > 0) {

#if	CF_DEBUGS
	    debugprintf("uucp: 'reade' goto one\n") ;
#endif

	    f_bad = TRUE ;
	}

#if	CF_DEBUGS
	debugprintf("uucp: read answer, f_bad=%d\n",f_bad) ;
#endif
	}
#endif /* COMMENT */

/* we are out of here */

#ifdef	COMMENT
	u_close(pipes[2][0]) ;
#endif

	u_unlink(pfname) ;

ret0:

#if	CF_DEBUGS
	debugprintf("uucp: ret rs=%d fd=%d\n", rs,pfd) ;
#endif

	return (rs >= 0) ? pfd : rs ;

/* handle the bad cases */
badopen:

badfork:
baddst:
	if (pfname[0] != '\0')
		u_unlink(pfname) ;

badpipemk:
	if (fd2p != NULL) {
	    u_close(pipes[2][0]) ;
	    u_close(pipes[2][1]) ;
	}

badpipes:
	goto ret0 ;
}
/* end subroutine (uucp) */


/* local subroutines */


/* check for UUCP availability */
static int testuucp(queue_machine)
const char	queue_machine[] ;
{
	bfile		file0, file2 ;
	bfile		procfile, *pfp = &procfile ;
	bfile		*fpa[3] ;
	pid_t		pid ;
	int		rs = SR_OK ;
	int		child_stat ;
	int		i, j, l ;
	const char	*cmd_uuname = PROG_UUNAME ;
	char		buf[NODENAMELEN + 1] ;
	char		*cp ;

#if	CF_DEBUGS
	debugprintf("testuucp: ent\n") ;
#endif

	if ((queue_machine == NULL) || (queue_machine[0] == '\0'))
	    return BAD ;

#if	CF_DEBUGS
	debugprintf("testuucp: got in\n") ;
#endif

	if (u_access(cmd_uuname,X_OK) < 0) 
		return BAD ;

	fpa[0] = &file0 ;
	fpa[1] = pfp ;		/* capture the standard output ! */
	fpa[2] = &file2 ;

#if	CF_DEBUGS
	debugprintf("testuucp: about to open command - FPA[1]=%08X\n",fpa[1]) ;
	debugprintf("testuucp: FPA[0]=%08X\n",fpa[0]) ;
#endif

	if ((rs = bopencmd(fpa,cmd_uuname)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("testuucp: opened command OK\n") ;
#endif

	    pid = rs ;

	    bclose(fpa[0]) ;

/* find the part of the machine name that we like */

	    i = strlen(queue_machine) ;

	    if ((cp = strchr(queue_machine,'.')) != NULL)
	        i = (cp - queue_machine) ;

	    rs = BAD ;
	    while ((l = breadline(pfp,buf,NODENAMELEN)) > 0) {

#if	CF_DEBUGS && 0
	        debugprintf("testuucp: got a line\n") ;
#endif

	        buf[l] = '\0' ;
	        cp = strshrink(buf) ;

	        j = strlen(cp) ;

	        if ((i == j) && (strncasecmp(cp,queue_machine,i) == 0)) {

#if	CF_DEBUGS
	            debugprintf("testuucp: got a machine match\n") ;
#endif

	            rs = OK ;
	            break ;
	        }
	    }

#if	CF_BDUMP
	    bdump(fpa[0],fpa[2]) ;
#endif

	    bclose(pfp) ;

	    bclose(fpa[2]) ;

	    u_waitpid(pid,&child_stat,0) ;

	} /* end if (program spawned) */

#if	CF_DEBUGS
	debugprintf("testuucp: passed the command OK\n") ;
#endif

	return rs ;
}
/* end subroutine (testuucp) */


#if	CF_BDUMP

static void bdump(f1p,f2p)
bfile	*f1p ;
bfile	*f2p ;
{
	int	f_done1 = FALSE ;
	int	f_done2 = FALSE ;

	char	buf[DUMPLEN + 1] ;


	while ((! f_done1) || (! f_done2)) {

	    if (! f_done1) {

	        if (bread(f1p,buf,DUMPLEN) <= 0)
	            f_done1 = TRUE ;

	    }

	    if (! f_done2) {

	        if (bread(f2p,buf,DUMPLEN) <= 0)
	            f_done2 = TRUE ;

	    }

	} /* end while */

}
/* end subroutine (bdump) */

#endif /* CF_BDUMP */


