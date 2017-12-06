/* pcsdialuucp */

/* send a FD over to another host using UUCP */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1996-07-10, David A­D­ Morano
	This subroutine was originally written.

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1996,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int pcsdialuucp(pr,nodename,filepath)
	char	pr[] ;
	char	nodename[]
	char	filepath[] ;

	Arguments:

	pr		programroot (if available)
	nodename	nodename of target machine
	filepath	filepath on the target machine

	Returns:

	>=0		file descriptor to write to target machine
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/wait.h>
#include	<netinet/in.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<sbuf.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#ifndef	PCS
#define	PCS		"/usr/add-on/pcs"
#endif

#ifndef	NULLDEV
#define	NULLDEV		"/dev/null"
#endif

#ifndef	VARPRPCS
#define	VARPRPCS	"PCS"
#endif

#define	PROG_UUCP	"uucp"
#define	PROG_UUNAME	"uuname"

#define	DSTLEN		(2 * MAXPATHLEN)
#define	BUFLEN		(20 * MAXPATHLEN)
#define	TO_WAITPID	5


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strshrink(char *) ;
extern char	*strbasename(char *) ;


/* external variables */

extern char	**environ ;


/* local structures */


/* forward reference */

static int	findprogname(const char *,char *,const char *) ;
static int	testuucp(const char *,const char *) ;
static int	waitpidtimed(pid_t,int *) ;


/* local variablies */

static const char	*goodenvs[] = {
	"MAIL",
	"NAME",
	"FULLNAME",
	"ORGANIZATION",
	"HOME",
	"LOGNAME",
	"HZ",
	"TZ",
	"LANG",
	"PATH",
	"LOCAL",
	"NCMP",
	"PCS",
	NULL
} ;


/* exported subroutines */


int pcsdialuucp(pr,uuhost,filename)
const char	pr[] ;
const char	uuhost[] ;
const char	filename[] ;
{
	pid_t		pid ;
	mode_t		mkmode ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		child_stat = 0 ;
	int		pfd = -1 ;
	const char	*varpcs = VARPRPCS ;
	const char	*proguucp = PROG_UUCP ;
	char		progfname[MAXPATHLEN + 2] ;
	char		pfname[MAXPATHLEN + 2] ;
	char		dst[DSTLEN + 2] ;

#if	CF_DEBUGS
	debugprintf("uucp: ent\n") ;
#endif

/* check for bad input */

	if ((uuhost == NULL) || (filename == NULL))
	    return SR_FAULT ;

	pfname[0] = '\0' ;
	if ((uuhost[0] == '\0') || (filename[0] == '\0')) {

	    rs = SR_INVAL ;
	    goto bad0 ;
	}

/* try to get a program root if we do not already have one */

	if ((pr == NULL) || (pr[0] == '\0'))
	    pr = getenv(varpcs) ;

	if ((pr == NULL) || (pr[0] == '\0'))
	    pr = PCS ;

	if (perm(pr,-1,-1,NULL,X_OK | R_OK) < 0) {

	    rs = SR_PROTO ;
	    goto bad0 ;
	}

/* make the program filepath */

	rs = findprogname(pr,progfname,proguucp) ;

	if (rs < 0) {
	        rs = SR_PROTO ;
	        goto bad0 ;
	    }

#if	CF_DEBUGS
	debugprintf("uucp: got in\n") ;
#endif

/* test the remote host for accessibility */

	rs = testuucp(pr,uuhost) ;
	if (rs < 0)
	    goto bad0 ;

#if	CF_DEBUGS
	debugprintf("uucp: continuing\n") ;
#endif

/* form the necessary UUCP destination string */

	rs = sncpy3(dst,DSTLEN,uuhost,"!",filename) ;
	if (rs < 0) {
	    rs = SR_TOOBIG ;
	    goto bad0 ;
	}

#if	CF_DEBUGS
	debugprintf("uucp: about to make pipe file\n") ;
#endif

	mkmode = (0600 | S_IFIFO) ;
	rs = mktmpfile(pfname,mkmode,"/tmp/uufileXXXXXXXX") ;

	if (rs < 0) {
	    rs = SR_PROTO ;
	    goto bad0 ;
	}

#if	CF_DEBUGS
	debugprintf("uucp: got pipe\n") ;
#endif

/* we spawn the program */

#if	CF_DEBUGS
	debugprintf("uucp: about to fork\n") ;
#endif

	rs = uc_fork() ;
	pid = rs ;
	if (rs < 0)
	    goto badfork ;

	if (rs == 0) {

	    vecstr	args, envs ;

	    int		fd ;

	    char	*arg0 ;


#if	CF_DEBUGS
	    debugprintf("uucp: inside fork\n") ;
#endif

	    vecstr_start(&args,10,0) ;

	    vecstr_start(&envs,10,0) ;

	    for (i = 0 ; i < 3 ; i += 1)
	        u_close(i) ;

	    u_open(NULLDEV,O_RDONLY,0600) ;

	    fd = u_open(NULLDEV,O_WRONLY,0600) ;

	    u_dup(fd) ;

/* pop us one ... more ... time! */

	    rs = uc_fork() ;

	    if (rs > 0)
	        uc_exit(EX_OK) ;

/* new child program name */

	    arg0 = strbasename(progfname) ;

/* arguments */

	    vecstr_add(&args,arg0,-1) ;

	    vecstr_add(&args,"-C",2) ;

	    vecstr_add(&args,pfname,-1) ;

	    vecstr_add(&args,dst,-1) ;

/* environment */

#if	CF_DEBUGS
	    debugprintf("pcsdialuucp: environment\n") ;
#endif

	    if (environ != NULL) {

#if	CF_DEBUGS
	        debugprintf("pcsdialuucp: got some\n") ;
#endif

	        for (i = 0 ; environ[i] != NULL ; i += 1) {

#if	CF_DEBUGS && 0
	            debugprintf("pcsdialuucp: E> %s\n",environ[i]) ;
#endif

	            if (matkeystr(goodenvs,environ[i],-1) >= 0)
	                vecstr_add(&envs,environ[i],-1) ;

	        } /* end for */

	        if (vecstr_finder(&envs,varpcs,vstrkeycmp,NULL) < 0)
	            vecstr_envadd(&envs,varpcs,pr,-1) ;

	    } else
	        vecstr_envadd(&envs,varpcs,pr,-1) ;

/* do the exec */

	    {
		const char	**eav = (const char **) args.va ;
		const char	**eev = (const char **) envs.va ;
	        u_execve(progfname,eav,eev) ;
	    }

	    uc_exit(EX_NOEXEC) ;

	} /* end if (fork) */

#if	CF_DEBUGS
	debugprintf("uucp: main line continue\n") ;
#endif

/* open the pipe file */

#if	CF_DEBUGS
	debugprintf("uucp: about to open the pipe\n") ;
#endif

	rs = u_open(pfname,O_WRONLY,0600) ;
	pfd = rs ;
	if (rs < 0)
	    goto badopen ;

/* we are out of here */

	if (pfname[0] != '\0') {
	    u_unlink(pfname) ;
	    pfname[0] = '\0' ;
	}

	rs1 = waitpidtimed(pid,&child_stat) ;
	if (rs >= 0) rs = rs1 ;

	if ((rs < 0) && (pfd >= 0)) u_close(pfd) ;

ret0:

#if	CF_DEBUGS
	debugprintf("uucp: ret rs=%d fd=%d\n",
	    rs,pfd) ;
#endif

	return (rs >= 0) ? pfd : rs ;

/* handle the bad cases */
badopen:
	waitpidtimed(pid,&child_stat) ;

badfork:
	if (pfname[0] != '\0') {
	    u_unlink(pfname) ;
	    pfname[0] = '\0' ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (pcsdialuucp) */


/* local subroutines */


/* check for UUCP availability */
static int testuucp(pr,uunode)
const char	pr[] ;
const char	uunode[] ;
{
	bfile	file0, file2 ;
	bfile	procfile, *pfp = &procfile ;
	bfile	*fpa[3] ;

	pid_t	pid ;

	int	rs ;
	int	i ;
	int	len, sl ;
	int	child_stat ;

	const char	*progname = PROG_UUNAME ;

	char	progfname[MAXPATHLEN + 2] ;
	char	buf[NODENAMELEN + 1] ;
	char	*cp ;


#if	CF_DEBUGS
	debugprintf("testuucp: ent\n") ;
#endif

	if ((uunode == NULL) || (uunode[0] == '\0'))
	    return SR_HOSTUNREACH ;

#if	CF_DEBUGS
	debugprintf("testuucp: got in\n") ;
#endif

	rs = findprogname(pr,progfname,progname) ;
	if (rs < 0) goto ret0 ;

	fpa[0] = &file0 ;
	fpa[1] = pfp ;		/* capture the standard output! */
	fpa[2] = &file2 ;

#if	CF_DEBUGS
	debugprintf("testuucp: about to open command - FPA[1]=%08X\n",fpa[1]) ;
	debugprintf("testuucp: FPA[0]=%08X\n",fpa[0]) ;
#endif

	if ((rs = bopencmd(fpa,progfname)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("testuucp: opened command OK\n") ;
#endif

	    pid = rs ;

	    bclose(fpa[0]) ;

/* find the part of the machine name that we like */

	    i = strlen(uunode) ;

	    if ((cp = strchr(uunode,'.')) != NULL)
	        i = cp - uunode ;

	    rs = SR_HOSTUNREACH ;
	    while ((len = breadline(pfp,buf,NODENAMELEN)) > 0) {

#if	CF_DEBUGS && 0
	        debugprintf("testuucp: got a line\n") ;
#endif

	        buf[len] = '\0' ;
	        cp = strshrink(buf) ;

	        sl = strlen(cp) ;

	        if ((i == sl) && (strncasecmp(cp,uunode,i) == 0)) {

#if	CF_DEBUGS
	            debugprintf("testuucp: got a machine match\n") ;
#endif

	            rs = SR_OK ;
	            break ;
	        }

	    } /* end while */

	    bclose(pfp) ;

	    bclose(fpa[2]) ;

	    u_waitpid(pid,&child_stat,WUNTRACED) ;

	} /* end if (program spawned) */

ret0:

#if	CF_DEBUGS
	debugprintf("testuucp: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (testuucp) */


static int findprogname(pr,progfname,pn)
const char	pr[] ;
char		progfname[] ;
const char	pn[] ;
{
	int	rs ;
	int	len = 0 ;

	char	cname[MAXNAMELEN+1] ;


	rs = sncpy2(cname,MAXNAMELEN,"pcs",pn) ;
	if (rs >= 0) {
	    rs = mkpath3(progfname,pr,"bin",cname) ;
	    len = rs ;
	}

	if (rs >= 0) 
	    rs = perm(progfname,-1,-1,NULL,X_OK) ;

	if ((rs == SR_NOENT) || (rs == SR_ACCESS)) {

	    rs = mkpath3(progfname,pr,"bin",pn) ;
	    len = rs ;
	    if (rs >= 0)
	        rs = perm(progfname,-1,-1,NULL,X_OK) ;

	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (findprogname) */


/* waitpidtimed */
static int waitpidtimed(pid,cp)
pid_t	pid ;
int	*cp ;
{
	const int	wopt = (WUNTRACED | WNOHANG) ;

	int	rs = SR_OK ;
	int	to = TO_WAITPID ;
	int	i ;


	for (i = 0 ; to-- > 0 ; i += 1) {

	    if (i > 0) sleep(1) ;

	    rs = u_waitpid(pid,cp,wopt) ;

	    if (rs != 0)
	        break ;

	} /* end while */

	return rs ;
}
/* end subroutine (waitpidtimed) */



