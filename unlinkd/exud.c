/* main (unlinkd) */

/* the "unlink" daemon */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGFD	0		/* use DEBUGFD */
#define	CF_LOG		1		/* do logging? */
#define	CF_SETRUID	1		/* set real UID to EUID */


/* revision history:

	= 1997-05-09, David A­D­ Morano

	This subroutine and program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a program that delays the deletion of files.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<vecitem.h>
#include	<char.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#if	CF_LOG
#include	<userinfo.h>
#include	<logfile.h>
#endif
#include	<localmisc.h>

#include	"unlinkd_config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 50),2048)
#endif

#define	DEBFNAME	"/tmp/unlinkd.deb"


/* local structures */

struct file {
	const char	*filename ;
	int		delay ;
} ;


/* extern subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;

extern char	*strbasename(char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* forward references */

static int	sortfunc() ;
static int	nulltermed(char *,int) ;


/* exported subroutines */


int b_unlinkd(argc,argv,contextp)
int	argc ;
char	*argv[] ;
void	*contextp ;
{
	struct file	*table, *fp, f ;

	struct ustat	sb ;

	VECITEM		flist ;

	bfile		infile, *ifp = &infile ;

#if	CF_LOG
	logfile	lh ;
#endif

	pid_t	pid ;

	int	rs = OK ;
	int	i ;
	int	delay ;
	int	len ;
	int	uid, euid ;
	int	timedelay ;
	int	sl ;
	int	ex = EX_OK ;

	const char	*progname = NULL ;
	const char	*pr = NULL ;
	const char	*sp ;
	const char	*cp ;
	char	lbuf[MAXPATHLEN + 50] ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	progname = SEARCHNAME ;

/* early initialization */

	uid = getuid() ;

	euid = geteuid() ;

/* continue */

	if (argv[0] != NULL) {

#if	CF_DEBUGS
	    debugprintf("main: non-NULL first argument\n") ;
	    debugprintf("main: argv[0]=%s\n",argv[0]) ;
#endif

	    progname = strbasename(argv[0]) ;

	}

#if	CF_DEBUGS
	debugprintf("main: continuing 2\n") ;
	debugprintf("main: progname=%s\n",progname) ;
#endif

/* initialize the file list data structure */

	vecitem_start(&flist,4,VECITEM_PSWAP) ;

	if ((argc > 1) && (argv[1] != NULL) && (argv[1][0] != '\0')) {

#if	CF_DEBUGS
	    debugprintf("main: we have an arguement\n") ;
#endif

	    if (u_stat(argv[1],&sb) >= 0) {

	        delay = DEFDELAY ;
	        if ((argc >= 3) && (argv[2] != NULL)) {

	            if (cfdeci(argv[2],-1,&delay) < 0)
	                delay = DEFDELAY ;

	        }

	        f.delay = delay ;
	        f.filename = mallocstr(argv[1]) ;

	        vecitem_add(&flist,&f,sizeof(struct file)) ;

	    } /* end if (good stat) */

	} else if ((rs = bopen(ifp,BFILE_STDIN,"dr",0666)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("main: look on STDIN for work\n") ;
#endif

	    while ((rs = breadline(ifp,lbuf,LINEBUFLEN)) > 0) {

		len = rs ;
	        if (lbuf[len - 1] == '\n')
	            len -= 1 ;

	        lbuf[len] = '\0' ;

#if	CF_DEBUGS
	        debugprintf("main: line=>%t<\n",lbuf,len) ;
#endif

	        cp = lbuf ;
	        while (CHAR_ISWHITE(*cp))
	            cp += 1 ;

	        if ((cp[0] == '#') || (cp[0] == '\0'))
	            continue ;

	        sp = cp ;
	        while (cp[0] && (! CHAR_ISWHITE(*cp)))
	            cp += 1 ;

	        sl = cp - sp ;
	        if (*cp != '\0')
	            *cp++ = '\0' ;

	        if (u_stat(sp,&sb) < 0)
	            continue ;

	        f.filename = mallocstrw(sp,sl) ;

	        while (CHAR_ISWHITE(*cp))
	            cp += 1 ;

	        sp = cp ;
	        while (cp[0] && (! CHAR_ISWHITE(*cp)))
	            cp += 1 ;

	        sl = cp - sp ;
	        if (*cp != '\0')
	            *cp++ = '\0' ;

	        f.delay = DEFDELAY ;
	        if (*sp) {

#if	CF_DEBUGS
	            debugprintf("main: delay string=%t\n",sp,sl) ;
#endif

	            if (cfdeci(sp,sl,&f.delay) < 0)
	                f.delay = DEFDELAY ;

	        } /* end if (we had a delay specified) */

#if	CF_DEBUGS
	        debugprintf("main: f=%s d=%d\n",f.filename,f.delay) ;
#endif

/* file the entry away */

	        if (f.filename != NULL)
	            vecitem_add(&flist,&f,sizeof(struct file)) ;

#if	CF_DEBUGS
	        debugprintf("main: bottom of while reading input\n") ;
#endif

	    } /* end while (reading lines) */

	    bclose(ifp) ;

	} /* end if (reading standard input) */

#if	CF_DEBUGS
	debugprintf("main: done getting work\n") ;
#endif

/* go on to the second phase of the program */

	ex = EX_OK ;
	if ((rs = vecitem_count(&flist)) > 0) {

#if	CF_DEBUGS
	    debugprintf("main: we have some stuff in the process list\n") ;
#endif

/* we go into daemon mode */

#if	CF_DEBUGS
	    debugprintf("main: closing FDs\n") ;
#endif

	    for (i = 0 ; i < NOFILE ; i += 1) {

#if	CF_DEBUGS
	        if (i == fd_debug) continue ;
#endif

	        u_close(i) ;

	    } /* end for */

	    rs = uc_fork() ;
	    pid = rs ;

#if	CF_DEBUGS
	    debugprintf("main: uc_fork() rs=%d\n",rs) ;
#endif

	    if (rs == 0) {
	        int	rs2 ;

	        rs2 = u_setsid() ;

#if	CF_SETRUID
	        if (uid != euid)
	            u_setreuid(euid,-1) ;
#endif

#if	CF_DEBUGS
	        debugprintf("main: setsid, rs=%d\n",rs2) ;
#endif

/* OK, let's sort the table that we have */

	        vecitem_sort(&flist,sortfunc) ;


/* do we want to log these out? */

#if	CF_LOG
	        {
	            struct userinfo	u ;

	            time_t	daytime ;

	            int	ii ;

	            char	userbuf[USERINFO_LEN + 1] ;
	            char	logfname[MAXPATHLEN + 1] ;
	            char	tmpbuf[100] ;
	            char	*programroot ;
	            char	*cp ;


	            programroot = getenv(VARPROGRAMROOT1) ;

	            if (programroot == NULL)
	                programroot = getenv(VARPROGRAMROOT2) ;

	            if (programroot == NULL)
	                programroot = getenv(VARPROGRAMROOT3) ;

	            if (programroot == NULL)
	                programroot = PROGRAMROOT ;

#if	CF_DEBUGS
	            debugprintf("main: programroot=%s\n",programroot) ;
#endif


	            cp = LOGFNAME ;
	            if (cp[0] != '/') {

	                cp = logfname ;
	                mkpath2(logfname, programroot,LOGFNAME) ;

	            }

#if	CF_DEBUGS
	            debugprintf("main: logfname=%s\n",cp) ;
#endif

	            if (userinfo(&u,userbuf,USERINFO_LEN,NULL) >= 0) {

#if	CF_DEBUGS
	                debugprintf("main: username=%s\n",u.username) ;
#endif

	                if ((u_access(cp,W_OK) >= 0) &&
	                    (logfile_open(&lh,cp,0,0666,u.logid) >= 0)) {

#if	CF_DEBUGS
	                    debugprintf("main: logfile_open\n") ;
#endif

	                    daytime = time(NULL) ;

	                    logfile_printf(&lh,"%s %-14s %s/%s\n",
	                        timestr_logz(daytime,tmpbuf),
	                        progname,
	                        VERSION,
	                        ((u.f.sysv) ? "SYSV" : "BSD")) ;

	                    logfile_printf(&lh,"ostype=%s\n",
	                        u.f.sysv_rt ? "SYSV" : "BSD") ;

	                    tmpbuf[0] = '\0' ;
	                    if ((u.name != NULL) && (u.name[0] != '\0'))
	                        sprintf(tmpbuf,"(%s)",u.name) ;

	                    else if ((u.gecosname != NULL) &&
	                        (u.gecosname[0] != '\0'))
	                        sprintf(tmpbuf,"(%s)",u.gecosname) ;

	                    else if ((u.fullname != NULL) &&
	                        (u.fullname[0] != '\0'))
	                        sprintf(tmpbuf,"(%s)",u.fullname) ;

	                    logfile_printf(&lh,"%s!%s %s\n",
	                        u.nodename,u.username,tmpbuf) ;


	                    for (ii = 0 ; vecitem_get(&flist,ii,&fp) >= 0 ; 
	                        ii += 1) {

	                        if (fp == NULL) continue ;

#if	CF_DEBUGS
	                        debugprintf("main: filename=%s\n",
	                            fp->filename) ;
#endif

	                        logfile_printf(&lh,"%6d %s\n",
	                            fp->delay,fp->filename) ;

	                    } /* end for */

#if	CF_DEBUGS
	                    debugprintf("main: about to logfile_close\n") ;
#endif

	                    logfile_close(&lh) ;
	                } /* end if (logfile_open) */

	            } /* end if (userinfo) */

#if	CF_DEBUGS
	            debugprintf("main: bottom of log block\n") ;
#endif

	        } /* end block (loggin) */
#endif /* CF_LOG */

/* go to work! */

	        timedelay = 0 ;
	        for (i = 0 ; vecitem_get(&flist,i,&fp) >= 0 ; i += 1) {

	            if (fp == NULL)
	                continue ;

	            if (fp->filename == NULL)
	                continue ;

	            if ((delay = (fp->delay - timedelay)) > 0) {

	                while (delay > 0) {

	                    int	waittime ;


	                    waittime = MIN(delay,DEFWAITTIME) ;

	                    sleep(waittime) ;

	                    timedelay += waittime ;
	                    delay -= waittime ;

	                    if (u_stat(fp->filename,&sb) < 0)
	                        break ;

	                } /* end while */

	            } /* end if (we have some time to wait) */

	            u_unlink(fp->filename) ;

#ifdef	OPTIONAL
	            free(fp->filename) ;

	            fp->filename = NULL ;
#endif /* OPTIONAL */

	        } /* end for */

/* child exit */

	        uc_exit(EX_OK) ;

	    } else if (pid < 0)
	        ex = EX_OSERR ;

	} /* end if (we had some files to delete) */

#if	CF_DEBUGS
	debugprintf("main: done w/ something rs=%d\n",rs) ;
#endif


#ifdef	OPTIONAL
	for (i = 0 ; vecitem_get(&flist,i,&fp) >= 0 ; i += 1) {
	    if (fp == NULL) continue ;
	    if (fp->filename != NULL)
	        free(fp->filename) ;
	} /* end for */
	vecitem_finish(&flist) ;
#endif /* OPTIONAL */

ret0:

#if	CF_DEBUGS
	debugprintf("main: exiting w/ ex=%d\n",ex) ;
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

help:

#ifdef	COMMENT

#if	CF_SFIO
	printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif

#endif /* COMMENT */

	goto ret0 ;

}
/* end subroutine (b_unlinkd) */



/* LOCAL SUBROUTINES */



static int sortfunc(f1pp,f2pp)
struct file	**f1pp, **f2pp ;
{


	if ((f1pp == NULL) && (f2pp == NULL))
	    return 0 ;

	if (f1pp == NULL)
	    return 1 ;

	if (f2pp == NULL)
	    return -1 ;

	return ((*f1pp)->delay - (*f2pp)->delay) ;
}
/* end subroutine (sortfunc) */


/* is this buffer NULL terminated? */
static int nulltermed(buf,len)
const char	buf[] ;
int		buflen ;
{
	int	i ;


	for (i = 0 ; i < buflen ; i += 1) {

	    if (buf[i] == '\0')
	        break ;

	} /* end for */

	return (i < buflen) ;
}
/* end subroutine (nulltermed) */



