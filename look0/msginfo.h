/* msginfo */


/* revision history:

	= 2001-11-01, David A­D­ Morano

	Written to have a place for the various KSH initialization
	subroutines.


*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

#ifndef	MSGINFO_INCLUDE
#define	MSGINFO_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sockaddress.h>
#include	<localmisc.h>


#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#ifndef	CMSGBUFLEN
#define	CMSGBUFLEN	256
#endif

#ifndef	MSGHDR
#define	MSGHDR		srtuct msghdr
#endif

#ifndef	CMSGHDR
#define	CMSGHDR		srtuct cmsghdr
#endif

#define	MSGINFO		struct msginfo
#define	MSGINFO_CON	union msginfo_con


union msginfo_con {
	struct cmsghdr	cm ;
	char		cmbuf[CMSGBUFLEN + 1] ;
} ;

struct msginfo {
	struct msghdr	msg ;
	MSGINFO_CON	cbuf ;
	SOCKADDRESS	from ;
	struct iovec	vecs[1] ;
	int		ns ;
	int		mlen ;
	char		mbuf[MSGBUFLEN + 1] ;
} ;


#if	(! defined(MSGINFO_MASTER)) || (MSGINFO_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	msginfo_init(MSGINFO *) ;
extern int	msginfo_sendmsg(MSGINFO *,int,int) ;
extern int	msginfo_recvmsg(MSGINFO *,int) ;
extern int	msginfo_recvmsgto(MSGINFO *,int,int) ;
extern int	msginfo_conpass(MSGINFO *,int) ;
extern int	msginfo_setaddr(MSGINFO *,const void *,int) ;
extern int	msginfo_fini(MSGINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(MSGINFO_MASTER)) || (MSGINFO_MASTER == 0) */

#endif /* MSGINFO_INCLUDE */


