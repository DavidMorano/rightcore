/* rmermsg */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	RMERMSG_INCLUDE
#define	RMERMSG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* general acknowledgement response */
struct rmermsg_fname {
	uint	msglen ;
	uint	tag ;
	uint	delay ;
	uid_t	uid ;
	uchar	msgtype ;		/* message type */
	char	fname[MAXPATHLEN + 1] ;
} ;

/* unknown */
struct rmermsg_unknown {
	uint	msglen ;
	uchar	msgtype ;		/* message type */
} ;


/* message types */
enum rmermsgtypes {
	rmermsgtype_fname,
	rmermsgtype_unknown,
	rmermsgtype_overlast
} ;


#if	(! defined(RMERMSG_MASTER)) || (RMERMSG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	rmermsg_fname(struct rmermsg_fname *,int,char *,int) ;
extern int	rmermsg_unknown(struct rmermsg_unknown *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* RMERMSG_MASTER */

#endif /* RMERMSG_INCLUDE */


