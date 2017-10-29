/* msginfo */

/* library initialization for KSH built-in command libraries */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_DEBUGENV	0		/* debug environment */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Message support.


*******************************************************************************/


#define	KSHLIB_MASTER	1	/* claim excemption from own forwards */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<poll.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"msginfo.h"


/* local defines */

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000		/* poll-time multiplier */
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	45		/* can hold int128_t in decimal */
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
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	msleep(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUGN
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(const char *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* local variables */


/* exported subroutines */


int msginfo_recvmsgto(MSGINFO *mip,int fd,int to)
{
	struct msghdr	*mp = &mip->msg ;
	int		rs ;
	msginfo_init(mip) ;
	if ((rs = uc_recvmsge(fd,mp,0,to,0)) >= 0) {
	    mip->mlen = rs ;
	}
	return rs ;
}
/* end subroutine (msginfo_recvmsg) */


int msginfo_recvmsg(MSGINFO *mip,int fd)
{
	struct msghdr	*mp = &mip->msg ;
	int		rs ;
	msginfo_init(mip) ;
	if ((rs = u_recvmsg(fd,mp,0)) >= 0) {
	    mip->mlen = rs ;
	}
	return rs ;
}
/* end subroutine (msginfo_recvmsg) */


int msginfo_sendmsg(MSGINFO *mip,int fd,int clen)
{
	struct msghdr	*mp = &mip->msg ;
	mip->vecs[0].iov_len = mip->mlen ;
	mp->msg_controllen = clen ;
	return u_sendmsg(fd,mp,0) ;
}
/* end subroutine (msginfo_sendmsg) */


/* receive or reject a passed FD (f=1 -> receive, f=0 -> reject) */
int msginfo_conpass(MSGINFO *mip,int f_passfd)
{
	struct msghdr	*mp = &mip->msg ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;
	mip->ns = -1 ;
	if (mp->msg_controllen > 0) {
	    struct cmsghdr	*cmp = CMSG_FIRSTHDR(mp) ;
	    const int		fdlen = sizeof(int) ;
	    int			*ip ;
	    while (cmp != NULL) {

	        ip = (int *) CMSG_DATA(cmp) ;
	        if ((cmp->cmsg_level == SOL_SOCKET) && 
	            (cmp->cmsg_len == CMSG_LEN(fdlen)) &&
	            (cmp->cmsg_type == SCM_RIGHTS) && (ip != NULL)) {

	            if ((mip->ns < 0) && f_passfd) {
	                mip->ns = *ip ;
			f = TRUE ;
	            } else {
	                rs1 = u_close(*ip) ;
			if (rs >= 0) rs = rs1 ;
		    }

	        } /* end if */
	        cmp = CMSG_NXTHDR(mp,cmp) ;

	    } /* end while */
	} /* end if (had a control-part) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (msginfo_conpass) */


int msginfo_setaddr(MSGINFO *mip,const void *sap,int sal)
{
	memcpy(&mip->from,sap,sal) ;
	mip->msg.msg_namelen = sal ;
	return SR_OK ;
}
/* end subroutine (msginfo_setaddr) */


int msginfo_init(MSGINFO *mip)
{
	struct msghdr	*mp = &mip->msg ;

	mip->ns = -1 ;
	mip->mbuf[0] = '\0' ;
	mip->mlen = 0 ;

	mip->vecs[0].iov_base = mip->mbuf ;
	mip->vecs[0].iov_len = MSGBUFLEN ;

	memset(&mip->cbuf,0,sizeof(MSGINFO_CON)) ;

	memset(mp,0,sizeof(struct msghdr)) ;
	mp->msg_name = &mip->from ;
	mp->msg_namelen = sizeof(SOCKADDRESS) ;
	mp->msg_control = &mip->cbuf ;
	mp->msg_controllen = CMSGBUFLEN ;
	mp->msg_iov = mip->vecs ;
	mp->msg_iovlen = 1 ;

	return SR_OK ;
}
/* end subroutine (msginfo_init) */


int msginfo_fini(MSGINFO *mip)
{
	if (mip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (msginfo_fini) */


