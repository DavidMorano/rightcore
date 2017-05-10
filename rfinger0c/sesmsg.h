/* sesmsg */


/* revision history:

	= 2002-07-21, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

#ifndef	SESMSG_INCLUDE
#define	SESMSG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<localmisc.h>


/* service name */
#ifndef	SVCNAMELEN
#define	SVCNAMELEN		32
#endif

#define	SESMSG_EXIT		struct sesmsg_exit
#define	SESMSG_NOOP		struct sesmsg_noop
#define	SESMSG_MBUF		struct sesmsg_mbuf
#define	SESMSG_GEN		struct sesmsg_gen
#define	SESMSG_BIFF		struct sesmsg_biff
#define	SESMSG_ECHO		struct sesmsg_echo
#define	SESMSG_RESPONSE		struct sesmsg_response
#define	SESMSG_PASSFD		struct sesmsg_passfd

#define	SESMSG_USERLEN		USERNAMELEN
#define	SESMSG_NBUFLEN		MAXNAMELEN
#define	SESMSG_REASONLEN	100
#define	SESMSG_SVCLEN		MAX(SVCNAMELEN,32)
#define	SESMSG_CMDLEN		MAX(SVCNAMELEN,32)
#define	SESMSG_LNAMELEN		MAXNAMELEN
#define	SESMSG_LADDRLEN		(MAXPATHLEN + 20)


struct sesmsg_exit {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;
	char	reason[SESMSG_REASONLEN + 1] ;
} ;

struct sesmsg_noop {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;
} ;

struct sesmsg_mbuf {
	time_t	stime ;
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;
	char	user[SESMSG_USERLEN+1] ;
	char	nbuf[SESMSG_NBUFLEN+ 1] ;
	uchar	rc ;
} ;

struct sesmsg_gen {
	time_t	stime ;
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;
	char	user[SESMSG_USERLEN+1] ;
	char	nbuf[SESMSG_NBUFLEN+ 1] ;
	uchar	rc ;
} ;

struct sesmsg_biff {
	time_t	stime ;
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;
	char	user[SESMSG_USERLEN+1] ;
	char	nbuf[SESMSG_NBUFLEN+ 1] ;
	uchar	rc ;
} ;

struct sesmsg_echo {
	time_t	stime ;
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;
	char	user[SESMSG_USERLEN+1] ;
	char	nbuf[SESMSG_NBUFLEN+ 1] ;
	uchar	rc ;
} ;

struct sesmsg_response {
	uint	msglen ;
	uint	tag ;
	uint	pid ;
	uchar	msgtype ;
	uchar	rc ;
} ;

struct sesmsg_passfd {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;
	char	svc[SESMSG_SVCLEN + 1] ;
	char	user[SESMSG_USERLEN+1] ;
} ;

/* request to parent server */
struct sesmsg_getsysmisc {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;
} ;

/* response to sub-server */
struct sesmsg_sysmisc {
	uint	msglen ;
	uint	tag ;
	uint	la_1min ;
	uint	la_5min ;
	uint	la_15min ;
	uint	boottime ;
	uint	nproc ;
	uchar	rc ;
	uchar	msgtype ;
} ;

struct sesmsg_getloadave {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
} ;

struct sesmsg_loadave {
	uint	msglen ;
	uint	tag ;
	uint	la_1min ;
	uint	la_5min ;
	uint	la_15min ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
} ;

struct sesmsg_reploadave {
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
struct sesmsg_getlistener {
	uint	msglen ;
	uint	tag ;
	uint	idx ;			/* listener index */
	uchar	msgtype ;		/* message type */
} ;

/* listener information (response) */
struct sesmsg_listener {
	uint	msglen ;
	uint	tag ;
	uint	idx ;			/* listener index */
	uint	pid ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
	uchar	ls ;			/* listener status */
	char	name[SESMSG_LNAMELEN + 1] ;
	char	addr[SESMSG_LADDRLEN + 1] ;
} ;

struct sesmsg_mark {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
} ;

struct sesmsg_unknown {
	uint	msglen ;
	uchar	msgtype ;
} ;

/* request help information */
struct sesmsg_gethelp {
	uint	msglen ;
	uint	tag ;
	uint	idx ;			/* help index */
	uchar	msgtype ;		/* message type */
} ;

/* help information (response) */
struct sesmsg_help {
	uint	msglen ;
	uint	tag ;
	uint	idx ;			/* help index */
	uint	pid ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
	char	name[SESMSG_LNAMELEN + 1] ;
} ;

struct sesmsg_cmd {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	char	cmd[SESMSG_CMDLEN + 1] ;
} ;

/* message types */
enum sesmsgtypes {
	sesmsgtype_exit,		/* by convention should be first */
	sesmsgtype_noop,
	sesmsgtype_gen,
	sesmsgtype_biff,
	sesmsgtype_echo,
	sesmsgtype_response,
	sesmsgtype_passfd,
	sesmsgtype_getsysmisc,
	sesmsgtype_sysmisc,
	sesmsgtype_getloadave,
	sesmsgtype_loadave,
	sesmsgtype_reploadave,
	sesmsgtype_getlistener,
	sesmsgtype_listener,
	sesmsgtype_mark,
	sesmsgtype_unknown,
	sesmsgtype_gethelp,
	sesmsgtype_help,
	sesmsgtype_cmd,	
	sesmsgtype_invalid,
	sesmsgtype_overlast
} ;

/* response codes */
enum sesmsgrcs {
	sesmsgrc_ok,
	sesmsgrc_invalid,
	sesmsgrc_nofd,
	sesmsgrc_notavail,
	sesmsgrc_error,
	sesmsgrc_overflow,
	sesmsgrc_overlast
} ;


#if	(! defined(SESMSG_MASTER)) || (SESMSG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sesmsg_exit(struct sesmsg_exit *,int,char *,int) ;
extern int sesmsg_noop(struct sesmsg_noop *,int,char *,int) ;
extern int sesmsg_gen(struct sesmsg_gen *,int,char *,int) ;
extern int sesmsg_biff(struct sesmsg_biff *,int,char *,int) ;
extern int sesmsg_echo(struct sesmsg_echo *,int,char *,int) ;
extern int sesmsg_response(struct sesmsg_response *,int,char *,int) ;
extern int sesmsg_passfd(struct sesmsg_passfd *,int,char *,int) ;
extern int sesmsg_getsysmisc(struct sesmsg_getsysmisc *,int,char *,int) ;
extern int sesmsg_sysmisc(struct sesmsg_sysmisc *,int,char *,int) ;
extern int sesmsg_getloadave(struct sesmsg_getloadave *,int,char *,int) ;
extern int sesmsg_sysmisc(struct sesmsg_sysmisc *,int,char *,int) ;
extern int sesmsg_reploadave(struct sesmsg_reploadave *,int,char *,int) ;
extern int sesmsg_loadave(struct sesmsg_loadave *,int,char *,int) ;
extern int sesmsg_getlistener(struct sesmsg_getlistener *,int,char *,int) ;
extern int sesmsg_listener(struct sesmsg_listener *,int,char *,int) ;
extern int sesmsg_mark(struct sesmsg_mark *,int,char *,int) ;
extern int sesmsg_unknown(struct sesmsg_unknown *,int,char *,int) ;
extern int sesmsg_gethelp(struct sesmsg_gethelp *,int,char *,int) ;
extern int sesmsg_help(struct sesmsg_help *,int,char *,int) ;
extern int sesmsg_cmd(struct sesmsg_cmd *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* SESMSG_MASTER */

#endif /* SESMSG_INCLUDE */


