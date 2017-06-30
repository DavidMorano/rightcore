/* pingstatmsg */


/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */


#ifndef	PINGSTATMSG_INCLUDE
#define	PINGSTATMSG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<netdb.h>
#include	<localmisc.h>


struct pingstatmsg_update {
	uint	msglen ;
	uint	timestamp ;
	short	hostnamelen ;
	uchar	msgtype ;		/* message type */
	char	hostname[MAXHOSTNAMELEN + 1] ;
} ;

struct pingstatmsg_uptime {
	uint	msglen ;
	uint	timestamp ;
	uint	timechange ;
	uint	count ;
	short	hostnamelen ;
	uchar	msgtype ;		/* message type */
	char	hostname[MAXHOSTNAMELEN + 1] ;
} ;

/* unknown */
struct pingstatmsg_unknown {
	uint	msglen ;
	uchar	msgtype ;		/* message type */
} ;


/* request types */
enum pingstatmsgtypes {
	pingstatmsgtype_update,
	pingstatmsgtype_uptime,
	pingstatmsgtype_unknown,
	pingstatmsgtype_overlast
} ;


/* response codes */
enum pingstatmsgrcs {
	pingstatmsgrc_ok,
	pingstatmsgrc_invalid,
	pingstatmsgrc_notavail,
	pingstatmsgrc_done,
	pingstatmsgrc_goingdown,
	pingstatmsgrc_overlast
} ;


/* message sizes */

#define	PINGSTATMSG_SUPDATE		sizeof(struct pingstatmsg_update) ;
#define	PINGSTATMSG_SUPTIME		sizeof(struct pingstatmsg_uptime) ;


#if	(! defined(PINGSTATMSG_MASTER)) || (PINGSTATMSG_MASTER == 0)

extern int pingstatmsg_update(struct pingstatmsg_update *,int,char *,int) ;
extern int pingstatmsg_uptime(struct pingstatmsg_uptime *,int,char *,int) ;
extern int pingstatmsg_unknown(struct pingstatmsg_unknown *,int,char *,int) ;

#ifdef	COMMENT
extern int pingstatmsg_msglen(int) ;
#endif

#endif /* PINGSTATMSG_MASTER */

#endif /* PINGSTATMSG_INCLUDE */


