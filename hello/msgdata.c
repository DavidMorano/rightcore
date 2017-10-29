/* msgdata */

/* library initialization for KSH built-in command libraries */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUGN	0		/* extra-special debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Message support.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"msgdata.h"


/* local defines */

#ifndef	CMSGBUFLEN
#define	CMSGBUFLEN	(2*256)
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy2w(char *,int,const char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	ctdecui(char *,int,uint) ;
extern int	cmsghdr_passed(CMSGHDR *) ;
extern int	iseol(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUGN
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	msgdata_setrecv(MSGDATA *) ;


/* local variables */


/* exported subroutines */


int msgdata_init(MSGDATA *mip,int mlen)
{
	const int	clen = MAX(CMSGBUFLEN,sizeof(CMSGHDR)) ;
	int		rs ;
	int		size = 0 ;
	int		ml = 0 ;
	char		*bp ;

	memset(mip,0,sizeof(MSGDATA)) ;
	mip->ns = -1 ;

	if (mlen < MSGBUFLEN) mlen = MSGBUFLEN ;

	size += (clen+1) ;
	size += (mlen+1) ;
	if ((rs = uc_libmalloc(size,&bp)) >= 0) {
	    MSGHDR	*mp = &mip->msg ;
	    mip->a = bp ;
	    ml = mlen ;

	    mip->mbuf = bp ;
	    mip->mlen = mlen ;
	    bp += (mlen+1) ;
	    mip->cmsgp = (CMSGHDR *) bp ;
	    mip->clen = clen ;
	    bp += (clen+1) ;

	    mip->mbuf[0] = '\0' ;
	    mip->vecs[0].iov_base = mip->mbuf ;
	    mip->vecs[0].iov_len = mip->mlen ;

	    memset(mip->cmsgp,0,clen) ; /* clear control-message */

	    memset(mp,0,sizeof(MSGHDR)) ;
	    mp->msg_name = &mip->from ;
	    mp->msg_namelen = sizeof(SOCKADDRESS) ;
	    mp->msg_control = mip->cmsgp ;
	    mp->msg_controllen = 0 ;
	    mp->msg_iov = mip->vecs ;
	    mp->msg_iovlen = 1 ;

	} /* end if (m-a) */

	return (rs >= 0) ? ml : rs ;
}
/* end subroutine (msgdata_init) */


int msgdata_fini(MSGDATA *mip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (mip == NULL) return SR_FAULT ;
	if (mip->a != NULL) {
	    rs1 = uc_libfree(mip->a) ;
	    if (rs >= 0) rs = rs1 ;
	    mip->a = NULL ;
	    mip->mbuf = NULL ;
	    mip->mlen = 0 ;
	    mip->ml = 0 ;
	    mip->cmsgp = NULL ;
	    mip->clen = 0 ;
	}
	return rs ;
}
/* end subroutine (msgdata_fini) */


int msgdata_getbufsize(MSGDATA *mip)
{
	return mip->mlen ;
}
/* end subroutine (msgdata_bufsize) */


int msgdata_getbuf(MSGDATA *mip,char **rpp)
{
	if (rpp != NULL) {
	    *rpp = mip->mbuf ;
	}
	return mip->mlen ;
}
/* end subroutine (msgdata_get) */


int msgdata_getdatalen(MSGDATA *mip)
{
	return mip->ml ;
}
/* end subroutine (msgdata_getdatalen) */


int msgdata_setdatalen(MSGDATA *mip,int dlen)
{
	mip->ml = dlen ;
	return dlen ;
}
/* end subroutine (msgdata_setdatalen) */


int msgdata_getdata(MSGDATA *mip,char **rpp)
{
	if (rpp != NULL) {
	    *rpp = mip->mbuf ;
	}
	return mip->ml ;
}
/* end subroutine (msgdata_getdata) */


int msgdata_recvto(MSGDATA *mip,int fd,int to)
{
	struct msghdr	*mp = &mip->msg ;
	int		rs ;
	msgdata_setrecv(mip) ;
	if ((rs = uc_recvmsge(fd,mp,0,to,0)) >= 0) {
	    mip->ml = rs ;
	}
	return rs ;
}
/* end subroutine (msgdata_recvto) */


int msgdata_recv(MSGDATA *mip,int fd)
{
	struct msghdr	*mp = &mip->msg ;
	int		rs ;
	msgdata_setrecv(mip) ;
	if ((rs = u_recvmsg(fd,mp,0)) >= 0) {
	    mip->ml = rs ;
	}
	return rs ;
}
/* end subroutine (msgdata_recv) */


int msgdata_send(MSGDATA *mip,int fd,int dl,int cl)
{
	int		rs ;
	if (cl <= mip->clen) {
	    MSGHDR	*mp = &mip->msg ;
	    mip->vecs[0].iov_len = (dl >= 0) ? (mip->ml = dl) : mip->ml ;
	    mp->msg_controllen = cl ;
	    rs = u_sendmsg(fd,mp,0) ;
	} else {
	    rs = SR_TOOBIG ;
	}
	return rs ;
}
/* end subroutine (msgdata_send) */


/* receive or reject a passed FD (f=1 -> receive, f=0 -> reject) */
int msgdata_conpass(MSGDATA *mip,int f_passfd)
{
	struct msghdr	*mp = &mip->msg ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;
	mip->ns = -1 ;
	if (mp->msg_controllen > 0) {
	    CMSGHDR	*cmp = CMSG_FIRSTHDR(mp) ;
	    int		fd ;
	    while (cmp != NULL) {
		if ((fd = cmsghdr_passed(cmp)) >= 0) {
	            if ((mip->ns < 0) && f_passfd) {
	                mip->ns = fd ;
			f = TRUE ;
	            } else {
	                rs1 = u_close(fd) ;
			if (rs >= 0) rs = rs1 ;
		    }
	        } /* end if (cmsghdr_pass) */
	        cmp = CMSG_NXTHDR(mp,cmp) ;
	    } /* end while */
	} /* end if (had a control-part) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (msgdata_conpass) */


int msgdata_getpassfd(MSGDATA *mip)
{
	int		rs = mip->ns ;
	if (rs < 0) rs = SR_NOTOPEN ;
	return rs ;
}
/* end subroutine (msgdata_getpassfd) */


int msgdata_setaddr(MSGDATA *mip,const void *sap,int sal)
{
	const int	flen = sizeof(SOCKADDRESS) ;
	int		rs ;
	if (sal <= flen) {
	    memcpy(&mip->from,sap,sal) ;
	    mip->msg.msg_namelen = sal ;
	} else {
	    rs = SR_TOOBIG ;
	}
	return rs ;
}
/* end subroutine (msgdata_setaddr) */


int msgdata_rmeol(MSGDATA *mip)
{
	int		ch ;
	while (mip->ml > 0) {
	    ch = MKCHAR(mip->mbuf[mip->ml-1]) ;
 	    if (! iseol(ch)) break ;
	    mip->ml -= 1 ;
	}
	return mip->ml ;
}
/* end subroutine (msgdata_rmeol) */


/* private subroutines */


static int msgdata_setrecv(MSGDATA *mip)
{
	MSGHDR		*mp = &mip->msg ;
	mip->mbuf[0] = '\0' ;
	mip->vecs[0].iov_base = mip->mbuf ;
	mip->vecs[0].iov_len = mip->mlen ;
	mip->ml = 0 ;
	mp->msg_controllen = mip->clen ;
	return SR_OK ;
}
/* end subroutine (msgdata_setrecv) */



