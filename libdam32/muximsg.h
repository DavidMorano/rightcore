/* muximsg */


/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

#ifndef	MUXIMSG_INCLUDE
#define	MUXIMSG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<localmisc.h>


/* service name */
#ifndef	SVCNAMELEN
#define	SVCNAMELEN	32
#endif

#define	MUXIMSG_REASONLEN	100
#define	MUXIMSG_SVCLEN		MAX(SVCNAMELEN,32)
#define	MUXIMSG_CMDLEN		MAX(SVCNAMELEN,32)
#define	MUXIMSG_LNAMELEN	MAXNAMELEN
#define	MUXIMSG_LADDRLEN	(MAXPATHLEN + 20)


/* general acknowledgement response */
struct muximsg_response {
	uint	msglen ;
	uint	tag ;
	uint	pid ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
} ;

struct muximsg_noop {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
} ;

struct muximsg_passfd {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	char	svc[MUXIMSG_SVCLEN + 1] ;
} ;

struct muximsg_exit {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	char	reason[MUXIMSG_REASONLEN + 1] ;
} ;

/* request to parent server */
struct muximsg_getsysmisc {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
} ;

/* response to sub-server */
struct muximsg_sysmisc {
	uint	msglen ;
	uint	tag ;
	uint	la_1min ;
	uint	la_5min ;
	uint	la_15min ;
	uint	boottime ;
	uint	nproc ;
	uchar	rc ;
	uchar	msgtype ;		/* message type */
} ;

struct muximsg_getloadave {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
} ;

struct muximsg_loadave {
	uint	msglen ;
	uint	tag ;
	uint	la_1min ;
	uint	la_5min ;
	uint	la_15min ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
} ;

struct muximsg_reploadave {
	uint	msglen ;
	uint	tag ;			/* our tag (unused) */
	uint	utag ;			/* user's tag */
	uint	duration ;		/* turns on repetition (secs) */
	uint	interval ;		/* repetition interval (secs) */
	ushort	addrfamily ;		/* used for UDP response */
	ushort	addrport ;		/* used for UDP response */
	uint	addrhost[4] ;		/* used for UDP response */
	uchar	msgtype ;		/* message type */
} ;

/* request listener information */
struct muximsg_getlistener {
	uint	msglen ;
	uint	tag ;
	uint	idx ;			/* listener index */
	uchar	msgtype ;		/* message type */
} ;

/* listener information (response) */
struct muximsg_listener {
	uint	msglen ;
	uint	tag ;
	uint	idx ;			/* listener index */
	uint	pid ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
	uchar	ls ;			/* listener status */
	char	name[MUXIMSG_LNAMELEN + 1] ;
	char	addr[MUXIMSG_LADDRLEN + 1] ;
} ;

struct muximsg_mark {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
} ;

struct muximsg_unknown {
	uint	msglen ;
	uchar	msgtype ;
} ;

/* request help information */
struct muximsg_gethelp {
	uint	msglen ;
	uint	tag ;
	uint	idx ;			/* help index */
	uchar	msgtype ;		/* message type */
} ;

/* help information (response) */
struct muximsg_help {
	uint	msglen ;
	uint	tag ;
	uint	idx ;			/* help index */
	uint	pid ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
	char	name[MUXIMSG_LNAMELEN + 1] ;
} ;

struct muximsg_cmd {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	char	cmd[MUXIMSG_CMDLEN + 1] ;
} ;

/* message types */
enum muximsgtypes {
	muximsgtype_response,		/* 00 */
	muximsgtype_noop,
	muximsgtype_passfd,
	muximsgtype_exit,
	muximsgtype_getsysmisc,
	muximsgtype_sysmisc,
	muximsgtype_getloadave,
	muximsgtype_loadave,
	muximsgtype_reploadave,
	muximsgtype_getlistener,
	muximsgtype_listener,
	muximsgtype_mark,
	muximsgtype_unknown,		/* 12 */
	muximsgtype_gethelp,
	muximsgtype_help,
	muximsgtype_cmd,		/* 15 */
	muximsgtype_overlast
} ;

/* response codes */
enum muximsgrcs {
	muximsgrc_ok,
	muximsgrc_invalid,
	muximsgrc_nofd,
	muximsgrc_notavail,
	muximsgrc_error,
	muximsgrc_overflow,
	muximsgrc_overlast
} ;


#if	(! defined(MUXIMSG_MASTER)) || (MUXIMSG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int muximsg_response(struct muximsg_response *,int,char *,int) ;
extern int muximsg_noop(struct muximsg_noop *,int,char *,int) ;
extern int muximsg_passfd(struct muximsg_passfd *,int,char *,int) ;
extern int muximsg_exit(struct muximsg_exit *,int,char *,int) ;
extern int muximsg_getsysmisc(struct muximsg_getsysmisc *,int,char *,int) ;
extern int muximsg_sysmisc(struct muximsg_sysmisc *,int,char *,int) ;
extern int muximsg_getloadave(struct muximsg_getloadave *,int,char *,int) ;
extern int muximsg_sysmisc(struct muximsg_sysmisc *,int,char *,int) ;
extern int muximsg_reploadave(struct muximsg_reploadave *,int,char *,int) ;
extern int muximsg_loadave(struct muximsg_loadave *,int,char *,int) ;
extern int muximsg_getlistener(struct muximsg_getlistener *,int,char *,int) ;
extern int muximsg_listener(struct muximsg_listener *,int,char *,int) ;
extern int muximsg_mark(struct muximsg_mark *,int,char *,int) ;
extern int muximsg_unknown(struct muximsg_unknown *,int,char *,int) ;
extern int muximsg_gethelp(struct muximsg_gethelp *,int,char *,int) ;
extern int muximsg_help(struct muximsg_help *,int,char *,int) ;
extern int muximsg_cmd(struct muximsg_cmd *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* MUXIMSG_MASTER */

#endif /* MUXIMSG_INCLUDE */


