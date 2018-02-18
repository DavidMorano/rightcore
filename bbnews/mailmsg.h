/* mailmsg */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILMSG_INCLUDE
#define	MAILMSG_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>

#include	<vecobj.h>
#include	<strpack.h>
#include	<localmisc.h>


/* object defines */

#define	MAILMSG_MAGIC		0x97634587
#define	MAILMSG			struct mailmsg_head
#define	MAILMSG_ENV		struct mailmsg_env
#define	MAILMSG_HDR		struct mailmsg_hdr
#define	MAILMSG_MAXLINELEN	MAX((5 * LINEBUFLEN),MAXPATHLEN)


struct mailmsg_env {
	VECOBJ		insts ;
} ;

struct mailmsg_hdr {
	VECOBJ		names ;
	int		lastname ;	/* index of last HDR-name */
} ;

struct mailmsg_head {
	uint		magic ;
	STRPACK		stores ;
	MAILMSG_ENV	envs ;
	MAILMSG_HDR	hdrs ;
	int		msgstate ;
} ;


#if	(! defined(MAILMSG_MASTER)) || (MAILMSG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailmsg_start(MAILMSG *) ;
extern int mailmsg_loadline(MAILMSG *,const char *,int) ;
extern int mailmsg_envcount(MAILMSG *) ;
extern int mailmsg_envaddress(MAILMSG *,int,const char **) ;
extern int mailmsg_envdate(MAILMSG *,int,const char **) ;
extern int mailmsg_envremote(MAILMSG *,int,const char **) ;
extern int mailmsg_hdrcount(MAILMSG *,const char *) ;
extern int mailmsg_hdrikey(MAILMSG *,int,const char **) ;
extern int mailmsg_hdriline(MAILMSG *,const char *,int,int,const char **) ;
extern int mailmsg_hdrival(MAILMSG *,const char *,int,const char **) ;
extern int mailmsg_hdrval(MAILMSG *,const char *,const char **) ;
extern int mailmsg_finish(MAILMSG *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILMSG_MASTER */

#endif /* MAILMSG_INCLUDE */


