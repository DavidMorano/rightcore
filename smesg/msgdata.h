/* msgdata */


/* revision history:

	= 2001-11-01, David A­D­ Morano
	Written to have a place for the various KSH initialization subroutines.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

#ifndef	MSGDATA_INCLUDE
#define	MSGDATA_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sockaddress.h>
#include	<localmisc.h>


#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#ifndef	MSGHDR
#define	MSGHDR		srtuct msghdr
#endif

#ifndef	CMSGHDR
#define	CMSGHDR		srtuct cmsghdr
#endif

#define	MSGDATA		struct msgdata


struct msgdata {
	struct msghdr	msg ;
	SOCKADDRESS	from ;
	struct iovec	vecs[1] ;
	CMSGHDR		*cmsgp ;	/* Control-Message-Header */
	const void	*a ;		/* allocation */
	char		*mbuf ;		/* buffer */
	int		clen ;		/* Control-Message length */
	int		mlen ;		/* fixed at buffer size */
	int		ml ;		/* variable as used */
	int		ns ;
} ;


#if	(! defined(MSGDATA_MASTER)) || (MSGDATA_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	msgdata_init(MSGDATA *,int) ;
extern int	msgdata_setaddr(MSGDATA *,const void *,int) ;
extern int	msgdata_getbufsize(MSGDATA *) ;
extern int	msgdata_getbuf(MSGDATA *,char **) ;
extern int	msgdata_getdatalen(MSGDATA *) ;
extern int	msgdata_getdata(MSGDATA *,char **) ;
extern int	msgdata_setdatalen(MSGDATA *,int) ;
extern int	msgdata_send(MSGDATA *,int,int,int) ;
extern int	msgdata_recv(MSGDATA *,int) ;
extern int	msgdata_recvto(MSGDATA *,int,int) ;
extern int	msgdata_conpass(MSGDATA *,int) ;
extern int	msgdata_getpassfd(MSGDATA *) ;
extern int	msgdata_rmeol(MSGDATA *) ;
extern int	msgdata_fini(MSGDATA *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(MSGDATA_MASTER)) || (MSGDATA_MASTER == 0) */

#endif /* MSGDATA_INCLUDE */


