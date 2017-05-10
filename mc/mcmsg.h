/* mcmsg */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MCMSG_INCLUDE
#define	MCMSG_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	NODENAMELEN
#define	NODENAMELEN	257		/* System V */
#endif

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#define	MCMSG_LMAILUSER		LOGNAMELEN
#define	MCMSG_LCLUSTER		NODENAMELEN
#define	MCMSG_LMSGID		MAILADDRLEN
#define	MCMSG_LFROM		MAILADDRLEN


/* client request message */
struct mcmsg_request {
	uint	msglen ;
	uint	tag ;
	uint	timestamp ;
	char	cluster[MCMSG_LCLUSTER + 1] ;
	char	mailuser[MCMSG_LMAILUSER + 1] ;
	uchar	msgtype ;
	uchar	seq ;
} ;

/* server report */
struct mcmsg_report {
	uint	msglen ;
	uint	tag ;
	uint	timestamp ;
	uint	mlen ;
	char	cluster[MCMSG_LCLUSTER + 1] ;
	char	mailuser[MCMSG_LMAILUSER + 1] ;
	char	msgid[MCMSG_LMSGID + 1] ;
	char	from[MCMSG_LFROM + 1] ;
	uchar	flags ;
	uchar	msgtype ;
	uchar	seq ;
	uchar	rc ;
} ;

/* general acknowledgement */
struct mcmsg_ack {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;
	uchar	seq ;
	uchar	rc ;
} ;


/* request types */
enum mcmsgtypes {
	mcmsgtype_request,
	mcmsgtype_report,
	mcmsgtype_ack,
	mcmsgtype_overlast
} ;


/* response codes */
enum mcmsgrcs {
	mcmsgrc_ok,
	mcmsgrc_invalid,
	mcmsgrc_notavail,
	mcmsgrc_goingdown,
	mcmsgrc_overlast
} ;


#if	(! defined(MCMSG_MASTER)) || (MCMSG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	mcmsg_request(struct mcmsg_request *,int,char *,int) ;
extern int	mcmsg_report(struct mcmsg_report *,int,char *,int) ;
extern int	mcmsg_ack(struct mcmsg_ack *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* MCMSG_MASTER */

#endif /* MCMSG_INCLUDE */


