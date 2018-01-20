/* progpass */

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

#include	<vsystem.h>
#include	<sockaddress.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"config.h"
#include	"muximsg.h"


/* local defines */

#ifndef	IPCDIRMODE
#define	IPCDIRMODE	0777
#endif

#define	W_OPTIONS	(WNOHANG)

#define	IPCBUFLEN	MSGBUFLEN
#define	CMSGBUFLEN	256

#define	NIOVECS		1

#define	O_SRVFLAGS	(O_RDWR | O_CREAT)

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	snddd(char *,int,uint,uint) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	dupup(int,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmpusd(const char *,int,mode_t,char *) ;

extern int	progjobdir(PROGINFO *,char *) ;
extern int	progreqfile(PROGINFO *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

union conmsg {
	struct cmsghdr	cm ;
	char		cmbuf[CMSGBUFLEN + 1] ;
} ;


/* forward references */

static int	cmsg_passfd(struct cmsghdr *,int) ;


/* local variables */


/* exported subroutines */


int progpass(PROGINFO *pip,ARGINFO *aip)
{
	struct muximsg_response	m0 ;
	struct muximsg_passfd	m2 ;
	struct msghdr	ipcmsg ;
	union conmsg	conbuf ;	/* aligned for bad architectures */
	struct cmsghdr	*cmp ;
	struct iovec	vecs[NIOVECS + 1] ;
	SOCKADDRESS	sa ;
	mode_t		operms ;

	int		rs = SR_OK ;
	int		rs1 ;
	int		len ;
	int		oflags ;
	int		fdlen ;
	int		size ;
	int		cmsize ;
	int		conbufsize = 0 ;
	int		salen = 0 ;

	char		template[MAXPATHLEN + 1] ;
	char		ourdname[MAXPATHLEN + 1] ;
	char		fname[MAXPATHLEN + 1] ;
	char		ipcbuf[IPCBUFLEN + 1] ;

/* some initialization (there is a good bit to setup for this operation) */

	fdlen = sizeof(int) ;

	size = NIOVECS * sizeof(struct iovec) ;
	memset(&vecs,0,size) ;

	vecs[0].iov_base = ipcbuf ;
	vecs[0].iov_len = IPCBUFLEN ;

	memset(&ipcmsg,0,sizeof(struct msghdr)) ;

	ipcmsg.msg_name = NULL ;
	ipcmsg.msg_namelen = 0 ;
	ipcmsg.msg_iov = vecs ;
	ipcmsg.msg_iovlen = NIOVECS ;
	ipcmsg.msg_control = &conbuf ;
	ipcmsg.msg_controllen = CMSG_SPACE(fdlen) ;

/* create (or divine) the "request" filename as needed */

	if ((rs = progreqfile(pip)) >= 0) {
	    if ((rs = progjobdir(pip,ourdname)) >= 0) {
		rs = mkpath2(template,ourdname,"callXXXXXXXXXX") ;
	    }
	}

	if (rs < 0) goto ret0 ;

/* create our socket */

	oflags = O_SRVFLAGS ;
	operms = (S_IFSOCK | 0600) ;
	if ((rs = opentmpusd(template,oflags,operms,fname)) >= 0) {
	    const int	af = AF_UNIX ;
	    const char	*rf = pip->reqfname ;
	    int	fd = rs ;

/* create the socket-address to the server request socket */

	if ((rs = sockaddress_start(&sa,af,rf,0,0)) >= 0) {

	salen = sockaddress_getlen(&sa) ;

	ipcmsg.msg_name = &sa ;
	ipcmsg.msg_namelen = salen ;

/* formulate the content of our message */

	memset(&m2,0,sizeof(struct muximsg_passfd)) ;

	if ((pip->svcpass != NULL) && (pip->svcpass[0] != '\0'))
	    strwcpy(m2.svc,pip->svcpass,MUXIMSG_SVCLEN) ;

	if ((rs = muximsg_passfd(&m2,0,ipcbuf,IPCBUFLEN)) >= 0) {
	    int	blen = rs ;

	vecs[0].iov_len = blen ;

/* build the control message */

	cmp = CMSG_FIRSTHDR(&ipcmsg) ;

	cmsize = cmsg_passfd(cmp,pip->fd_pass) ;

	conbufsize += cmsize ;

	ipcmsg.msg_controllen = conbufsize ;

/* send it */

	if ((rs = u_sendmsg(fd,&ipcmsg,0)) >= 0) {

	    vecs[0].iov_len = IPCBUFLEN ;

	    ipcmsg.msg_control = NULL ;
	    ipcmsg.msg_controllen = 0 ;

	    rs = uc_recvmsge(fd,&ipcmsg,0,TO_RECVMSG,0) ;
	    len = rs ;
	    if ((rs >= 0) && (len > 0)) {

	        rs1 = muximsg_response(&m0,1,ipcbuf,len) ;

	        rs = ((rs1 >= 0) && (m0.rc == 0)) ? SR_OK : SR_PROTO ;

	    } else
		rs = SR_PROTO ;

	} /* end if (sendmsg) */

	} /* end if (muximsg_passfd) */

	sockaddress_finish(&sa) ;
	} /* end if (sockaddress) */

	    u_close(fd) ;
	} /* end if (opentmpusd) */

	if (fname[0] != '\0') {
	    u_unlink(fname) ;
	    fname[0] = '\0' ;
	}

ret0:
	return rs ;
}
/* end subroutine (progpass) */


/* local subroutines */


static int cmsg_passfd(cmp,fd)
struct cmsghdr	*cmp ;
int		fd ;
{
	int	cmsize ;
	int	fdlen = sizeof(int) ;

	uchar	*up ;


	cmsize = CMSG_SPACE(fdlen) ;

	memset(cmp,0,sizeof(struct cmsghdr)) ;
	cmp->cmsg_level = SOL_SOCKET ;
	cmp->cmsg_type = SCM_RIGHTS ;
	cmp->cmsg_len = CMSG_LEN(fdlen) ;

	up = CMSG_DATA(cmp) ;
	memcpy(up,&fd,fdlen) ;

	return cmsize ;
}
/* end subroutine (cmsg_passfd) */


