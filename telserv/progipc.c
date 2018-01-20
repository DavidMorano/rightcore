/* progipc */

/* handle IPC-related things */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 2008-09-01, David A­D­ Morano
        This subroutine was adopted from the DWD program. I may not have changed
        all of the comments correctly though!

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines handle some IPC related functions.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<sockaddress.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"config.h"


/* local defines */

#ifndef	IPCDMODE
#define	IPCDMODE	0777
#endif

#define	W_OPTIONS	(WNOHANG)

#define	IPCBUFLEN	MSGBUFLEN

#define	MAXOUTLEN	62
#define	NIOVECS		1
#define	O_SRVFLAGS	(O_RDWR | O_CREAT)

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snddd(char *,int,uint,uint) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	dupup(int,int) ;
extern int	opentmpfile(cchar *,int,mode_t,char *) ;
extern int	opentmpusd(cchar *,int,mode_t,char *) ;

extern int	progjobdir(PROGINFO *,char *) ;
extern int	progcheckdir(PROGINFO *,cchar *,int,int) ;
extern int	progreqfile(PROGINFO *) ;

extern int	proglog_printf(PROGINFO *,cchar *,...) ;
extern int	proglog_flush(PROGINFO *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	progipcbeginshared(PROGINFO *,mode_t,char *) ;
static int	progipcbeginprivate(PROGINFO *,mode_t,char *) ;


/* local variables */


/* exported subroutines */


int progipcbegin(PROGINFO *pip)
{
	PROGINFO_IPC	*ipp = &pip->ipc ;
	const mode_t	om = 0666 ;
	int		rs = SR_OK ;
	int		fl = 0 ;
	int		f ;
	char		fname[MAXPATHLEN + 1] = { 0 } ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progipc/open: ent reqfname=%s\n",pip->reqfname) ;
#endif

	ipp = &pip->ipc ;
	memset(ipp,0,sizeof(PROGINFO_IPC)) ;
	ipp->fd_req = -1 ;

	f = (pip->reqfname != NULL) && (pip->reqfname[0] == '-') ;
	if (pip->f.daemon && (! f)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progipc/open: progipcbeginshared() \n") ;
#endif

	    rs = progipcbeginshared(pip,om,fname) ;
	    fl = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progipc/open: progipcbeginshared() rs=d\n",
		rs) ;
#endif

	} else {

	    rs = progipcbeginprivate(pip,om,fname) ;
	    fl = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progipc/open: progipcbeginprivate() rs=%d\n",
		rs) ;
#endif

	} /* end if */

	if (rs >= 0) {
	    const char	*cp ;

#if	CF_DEBUG && 0
	if (DEBUGLEVEL(4))
	    debugprintf("progipc/open: fname=%s\n",fname) ;
#endif

	    if ((rs = uc_mallocstrw(fname,fl,&cp)) >= 0) {
	        ipp->fname = cp ;
		if (om != 0777) u_chmod(fname,om) ;
	        uc_closeonexec(ipp->fd_req,TRUE) ;
		if (pip->debuglevel > 0)
	            bprintf(pip->efp,"%s: req=%s\n",pip->progname,ipp->fname) ;
	        if (pip->f.daemon && pip->open.logprog)
		    proglog_printf(pip,"req=%s",ipp->fname) ;
	    }
	    if (rs < 0) {
	        if (ipp->fd_req >= 0) {
	            u_close(ipp->fd_req) ;
	            ipp->fd_req = -1 ;
	        }
	        if (fname[0] != '\0') {
	            u_unlink(fname) ;
	            fname[0] = '\0' ;
	        }
		ipp->fname = NULL ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progipc/open: ret rs=%d\n",rs) ;
	    if (ipp->fname != NULL)
	    debugprintf("progipc/open: ipp->fname=%s\n",ipp->fname) ;
	}
#endif

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (progipcbegin) */


int progipcend(PROGINFO *pip)
{
	PROGINFO_IPC	*ipp = &pip->ipc ;
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = sockaddress_finish(&ipp->sa) ;
	if (rs >= 0) rs = rs1 ;

	if (ipp->fd_req >= 0) {
	    rs1 = u_close(ipp->fd_req) ;
	    if (rs >= 0) rs = rs1 ;
	    ipp->fd_req = -1 ;
	}

	    if (ipp->fname != NULL) {
	        if (ipp->fname[0] != '\0') {
	            rs1 = u_unlink(ipp->fname) ;
		    if (rs >= 0) rs = rs1 ;
		}
	        rs1 = uc_free(ipp->fname) ;
	        if (rs >= 0) rs = rs1 ;
	        ipp->fname = NULL ;
	    }

	return rs ;
}
/* end subroutine (progipcend) */


/* local subroutines */


static int progipcbeginshared(PROGINFO *pip,mode_t om,char *fname)
{
	PROGINFO_IPC	*ipp = &pip->ipc ;
	SOCKADDR	*sap ;
	int		rs = SR_OK ;
	int		cl ;
	int		salen ;
	int		fl = 0 ;
	const char	*cp ;

	fname[0] = '\0' ;
	ipp = &pip->ipc ;

	rs = progreqfile(pip) ;
	fl = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progipc/shared: progreqfile() rs=%d fl=%u\n",
		rs,fl) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	mkpath1(fname,pip->reqfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progipc/shared: REQ fname=%s\n",fname) ;
#endif

	if ((fl > 0) && ((cl = sfdirname(fname,fl,&cp)) > 0)) {
	    const mode_t	dm = (IPCDMODE | 0555) ;
	    rs = progcheckdir(pip,cp,cl,dm) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progipc/shared: progcheckdir() rs=%d \n",rs) ;
#endif

/* continue with socket creation */

	if (rs >= 0) {
	    rs = u_socket(PF_UNIX,SOCK_DGRAM,0) ;
	    ipp->fd_req = rs ;
	}
	if (rs < 0)
	    goto ret0 ;

	if (rs >= 0) {
	    if (om != 0777) u_chmod(fname,om) ;
	    uc_closeonexec(ipp->fd_req,TRUE) ;
	    u_unlink(fname) ;
	    rs = sockaddress_start(&ipp->sa,AF_UNIX,fname,0,0) ;
	}

	if (rs >= 0) {

	    salen = sockaddress_getlen(&ipp->sa) ;
	    ipp->salen = salen ;

	    sap = (struct sockaddr *) &ipp->sa ;
	    rs = u_bind(ipp->fd_req,sap,salen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progipc/shared: u_bind() rs=%d\n",rs) ;
#endif

	    if (rs < 0) {
	        sockaddress_finish(&ipp->sa) ;
	        if (fname[0] != '\0') {
	            u_unlink(fname) ;
		    fname[0] = '\0' ;
		}
	    } /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    struct ustat	sb ;
	    int	rs1 ;
	    int	fd = ipp->fd_req ;
	    rs1 = u_fstat(fd,&sb) ;
	    debugprintf("progipc/shared: u_fstat() rs=%d perm=%06o\n",
		rs1,sb.st_mode) ;
	}
#endif /* CF_DEBUG */

	} /* end if */

	if ((rs < 0) && (ipp->fd_req >= 0)) {
	    u_close(ipp->fd_req) ;
	    ipp->fd_req = -1 ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progipc/shared: ret rs=%d fl=%u\n",rs,fl) ;
#endif

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (progipcbeginshared) */


static int progipcbeginprivate(PROGINFO *pip,mode_t om,char *fname)
{
	PROGINFO_IPC	*ipp = &pip->ipc ;
	int		rs = SR_OK ;
	int		oflags ;
	int		salen ;
	int		fl = 0 ;
	char		ourdname[MAXPATHLEN + 1] ;
	char		template[MAXPATHLEN + 1] ;

	fname[0] = '\0' ;
	ipp = &pip->ipc ;

	if ((rs = progjobdir(pip,ourdname)) >= 0) {
	    pip->f.reqfnametmp = TRUE ;		/* mark as temporary */
	    rs = mkpath2(template,ourdname,"reqXXXXXXXXXXX") ;
	}
	if (rs < 0) goto ret0 ;

/* create our socket there */

	oflags = O_SRVFLAGS ;
	om |= S_IFSOCK ;

/* create the UNIX-domain socket (in the file-system) */

	rs = opentmpusd(template,oflags,om,fname) ;
	ipp->fd_req = rs ;
	if (rs < 0) goto ret0 ;

/* create its socket-address (SA) for use later in connecting to it */

	rs = sockaddress_start(&ipp->sa,AF_UNIX,fname,0,0) ;
	if (rs >= 0) {
	    salen = sockaddress_getlen(&ipp->sa) ;
	    ipp->salen = salen ;
	}

/* file-name length */

	fl = strlen(fname) ;

/* handle any necessary cleanup */

	if (rs < 0) {

	    sockaddress_finish(&ipp->sa) ;

	    if (fname[0] != '\0') {
	        u_unlink(fname) ;
		fname[0] = '\0' ;
	    }

	    u_close(ipp->fd_req) ;
	    ipp->fd_req = -1 ;

	} /* end if (error) */

ret0:
	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (progipcbeginprivate) */


