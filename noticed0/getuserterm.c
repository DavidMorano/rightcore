/* getuserterm */

/* get the name of the controlling terminal for the current session */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1999-01-10, David A­D­ Morano
        This subroutine was originally written. It was prompted by the failure
        of other terminal message programs from finding the proper controlling
        terminal.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will find and returns an open FD for the controlling
        terminal for the given username, if that user is logged in and even has
        a controlling terminal.

	Synopsis:

	int getuserterm(username,buf,buflen,fdp)
	char	username[] ;
	char	buf[] ;
	int	buflen ;
	int	*fdp ;

	Arguments:

	- username	session ID to find controlling terminal for
	- buf		user buffer to receive name of controlling terminal
	- len		length of user supplied buffer
	fdp		pointer to open file-descriptor

	Returns:

	>=	length of name of controlling terminal
	<0	error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<tmpx.h>
#include	<localmisc.h>


/* local defines */

#define	DEVDNAME	"/dev/"


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

static int	getatime(char *,time_t,time_t *) ;


/* exported subroutines */



int getuserterm(username,buf,buflen,fdp)
const char	username[] ;
char		buf[] ;
int		buflen ;
int		*fdp ;
{
	TMPX		utmp ;

	TMPX_ENT	ue ;

	time_t	ti_tmp ;
	time_t	ti_access = 0 ;

	int	rs ;
	int	tlen ;
	int	len = 0 ;
	int	fd_termdev = -1 ;

	char	tmptermdev[MAXPATHLEN + 1] ;
	char	termdev[MAXPATHLEN + 1] ;
	char	devdname = DEVDNAME ;


	if (buf == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("getuserterm: entered, sid=%d\n",sid) ;
#endif

	buf[0] = '\0' ;
	if ((username == NULL) || (username[0] == '\0'))
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("getuserterm: username=%s\n",username) ;
#endif

	termdev[0] = '\0' ;
	fd_termdev = -1 ;
	ti_access = 0 ;
	if ((rs = tmpx_open(&utmp,NULL,O_RDONLY)) >= 0) {
	    TMPX_CUR	cur ;

#if	CF_DEBUGS
	    debugprintf("getuserterm: tmpx_open() rs=%d\n",rs) ;
#endif

	    tmpx_curbegin(&utmp,&cur) ;

	    while (tmpx_fetchuser(&utmp,&cur,&ue,username) >= 0) {

#if	CF_DEBUGS
	        debugprintf("getuserterm: tmpx_fetchuser() rs=%d\n",rs) ;
#endif

	        if ((ue.ut_type != TMPX_TUSERPROC) || (ue.ut_line[0] == '\0'))
	            continue ;

	        strcpy(tmptermdev,DEVDNAME) ;

	        tlen = strwcpy((tmptermdev + 5),ue.ut_line,32) - tmptermdev ;

/* check the access time of this terminal and if it is enable for notices */

	        rs = getatime(tmptermdev,ti_access,&ti_tmp) ;

	        if (rs >= 0) {

	            if (fd_termdev >= 0)
	                u_close(fd_termdev) ;

	            fd_termdev = rs ;
	            ti_access = ti_tmp ;
	            len = tlen ;
	            strcpy(termdev,tmptermdev) ;

	        } /* end if (we had a better one) */

	    } /* end while (looping through entries) */

	    tmpx_curend(&utmp,&cur) ;

	    tmpx_close(&utmp) ;

	} /* end if (UTMPX open) */

/* store the result */

	rs = SR_NOTFOUND ;
	if ((termdev[0] != '\0') && (fd_termdev >= 0)) {

	    rs = SR_TOOBIG ;
	    if ((buflen < 0) || (len <= buflen)) {

	        rs = SR_OK ;
	        strcpy(buf,termdev) ;

	    } else
	        u_close(fd_termdev) ;

	} /* end if (got one) */

/* finish up */

	if (fdp != NULL)
	    *fdp = (rs >= 0) ? fd_termdev : -1 ;

	else if (fd_termdev >= 0)
		u_close(fd_termdev) ;

#if	CF_DEBUGS
	debugprintf("getuserterm: ret rs=%d len=%d\n",rs,len) ;
	if (rs >= 0)
	    debugprintf("getuserterm: term=>%t<\n",buf,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getuserterm) */



/* LOCAL SUBROUTINES */



static int getatime(termdev,current,tp)
char	termdev[] ;
time_t	current ;
time_t	*tp ;
{
	struct ustat	sb ;

	int	rs, fd ;


	if ((rs = u_open(termdev,O_WRONLY,0666)) < 0)
	    return rs ;

	fd = rs ;
	if ((rs = u_fstat(fd,&sb)) >= 0) {

		*tp = sb.st_atime ;
	    if (((sb.st_mode & S_IWGRP) != S_IWGRP) ||
	        (sb.st_atime <= current))
	        rs = SR_INVALID ;

	} /* end if */

	if (rs < 0)
	    u_close(fd) ;

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (getatime) */



