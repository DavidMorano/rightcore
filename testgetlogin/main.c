/* main */

/* get login information */

#define	CF_DEBUGS	1
#define	CF_NPRINTF	0
#define	CF_GETUTMPNAME	1
#define	CF_GETLOGNAME	1


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<pwd.h>
#include	<utmpx.h>
#include	<errno.h>
#include	<stdio.h>

#include	<tmpx.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif


/* external subroutines */

extern int	getutmpname(char *,int,pid_t) ;
extern int	getlogname(char *,int) ;


/* forward references */

static int	isproctype(int) ;


/* local variables */

static const int	proctypes[] = {
	    TMPX_TINITPROC,
	    TMPX_TLOGINPROC,
	    TMPX_TUSERPROC,
	    TMPX_TDEADPROC,
	    -1
} ;


/* exported subroutines */


int main()
{
	struct passwd	pe ;
	int		rs ;
	int		i ;
	int		fd_debug = -1 ;
	int		sid, pgid ;
	char		logname[LOGNAMELEN + 1] ;
	char		pebuf[1000] ;
	char		ubuf[100] ;
	char		*cp ;

	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


#if	CF_DEBUGS
	debugprintf("main: starting\n") ;

#if	CF_NPRINTF
	nprintf("here","main: here\n") ;
#endif
#endif

	sid = getsid((pid_t) 0) ;

	pgid = getpgrp() ;

	printf("sid=%d pgid=%d\n",sid,pgid) ;


	rs = getlogin_r(logname,LOGNAMELEN) ;

	printf("getlogin() rs=%d logname=%s errno=%d\n",
	    rs,logname,errno) ;


	rs = getpwlogname(&pe,pebuf,1000) ;

	printf("getpwlogname() rs=%d logname=%s\n",
	    rs,pe.pw_name) ;


	for (i = 9 ; i > 1 ; i -= 1) {

	    rs = uc_getlogin(logname,i) ;

	    printf("uc_getlogin() buflen=%d rs=%d logname=%s\n",
	        i,rs,logname) ;

	} /* end for */

/* full UTMP analysis */

	{
	    struct utmpx	*up ;


	    setutxent() ;

	    for (i = 0 ; (up = getutxent()) != NULL ; i += 1) {

	        if (isproctype(up->ut_type)) {

	            strwcpy(ubuf,up->ut_user,32) ;

#if	CF_DEBUGS
	            debugprintf("main: UTMP i=%u ut_user=%s\n",
	                i,ubuf) ;
#endif

	            printf("UTMP i=%2u ut_user=%s\n",
	                i,ubuf) ;

	        }

	    } /* end for */

	    endutxent() ;

	} /* end block */

/* our analysis */

	{
	    TMPX	ut ;
	    TMPX_ENT	e ;
	    TMPX_CUR	cur ;
	    const char	*un = "dam" ;


	    rs = tmpx_open(&ut,NULL,O_RDONLY) ;

	    if (rs >= 0) {

	        tmpx_curbegin(&ut,&cur) ;

	        while ((rs = tmpx_enum(&ut,&cur,&e)) >= 0) {

	            if (isproctype(e.ut_type)) {

	                strwcpy(ubuf,e.ut_user,32) ;

#if	CF_DEBUGS
	                debugprintf("main: TMPX enum rs=%d ut_user=%s\n",
	                    rs,ubuf) ;
#endif

	                printf("TMPX enum rs=%d ut_user=%s\n",
	                    rs,ubuf) ;

	            }

	        } /* end while */

	        tmpx_curend(&ut,&cur) ;

/* get a non-existance name from UTMP to test TMPX */

	        rs = tmpx_fetchpid(&ut,&e,100) ;

#if	CF_DEBUGS
	        debugprintf("main: tmpx_fetchpid() rs=%d\n",rs) ;
#endif

/* OK, now fetch all users by a given name */

	        tmpx_curbegin(&ut,&cur) ;

	        while ((rs = tmpx_fetchuser(&ut,&cur,&e,un)) >= 0) {

	            strwcpy(ubuf,e.ut_user,32) ;

#if	CF_DEBUGS
	            debugprintf("main: TMPX fetchuser rs=%d ut_user=%s\n",
	                rs,ubuf) ;
#endif

	            printf("TMPX fetchuser rs=%d ut_user=%s\n",
	                rs,ubuf) ;

	        } /* end while */

	        tmpx_curend(&ut,&cur) ;

/* close up and get out */

	        tmpx_close(&ut) ;

	    }

	} /* end block */


/* GETUTMPNAME */

	i = ttyslot() ;

#if	CF_DEBUGS
	debugprintf("main: ttyslot() i=%u\n",i) ;
#endif

	rs = getutmpname(logname,LOGNAMELEN,sid) ;

	printf("getutmpname() i=%u rs=%d logname=%s\n",
	    i,rs,logname) ;

/* GETLOGNAME */

	rs = getlogname(logname,LOGNAMELEN) ;

	printf("getlogname() rs=%d logname=%s\n",
	    rs,logname) ;


/* out of here */


	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int isproctype(type)
int	type ;
{
	int	i ;


	for (i = 0 ; proctypes[i] >= 0 ; i += 1) {
	    if (type == proctypes[i])
	        break ;
	}

	return (proctypes[i] >= 0) ? TRUE : FALSE ;
}
/* end subroutine (isproctype) */



