/* mfsmsg */

/* last modified %G% version %I% */


/* revision history:

	= 2004-05-24, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code MFSERVE.

*/

/* Copyright © 2004,2017 David A­D­ Morano.  All rights reserved. */


#ifndef	MFSMSG_INCLUDE
#define	MFSMSG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<realname.h>
#include	<localmisc.h>


#ifndef	MFSMSG_KEYLEN
#define	MFSMSG_KEYLEN	MAX(60,USERNAMELEN)
#endif

#define	MFSMSG_REASONLEN	100
#define	MFSMSG_SVCLEN		MAX(SVCNAMELEN,32)
#define	MFSMSG_CMDLEN		MAX(SVCNAMELEN,32)
#define	MFSMSG_LNAMELEN		MAXNAMELEN
#define	MFSMSG_LADDRLEN		(MAXPATHLEN + 20)
#define	MFSMSG_GETLISTENER	struct mfsmsg_getlistener
#define	MFSMSG_LISTENER		struct mfsmsg_listener


struct mfsmsg_getstatus {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
} ;

struct mfsmsg_status {
	uint	msglen ;
	uint	tag ;
	uint	pid ;
	uint	queries ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
} ;

struct mfsmsg_getval {
	uint	msglen ;
	uint	tag ;
	uchar	w ;			/* request code */
	uchar	msgtype ;		/* message type */
	char	key[MFSMSG_KEYLEN+1] ;
} ;

struct mfsmsg_val {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	uchar	w ;			/* request code */
	uchar	rc ;
	uchar	vl ;
	char	val[REALNAMELEN+1] ;
} ;

struct mfsmsg_gethelp {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	uchar	idx ;
} ;

struct mfsmsg_help {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	uchar	idx ;
	uchar	rc ;
	uchar	vl ;
	char	val[REALNAMELEN+1] ;
} ;

struct mfsmsg_getname {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	char	un[USERNAMELEN+1] ;
} ;

struct mfsmsg_name {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
	char	rn[REALNAMELEN+1] ;
} ;

struct mfsmsg_getuser {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	char	spec[REALNAMELEN+1] ;
} ;

struct mfsmsg_user {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
	char	un[USERNAMELEN+1] ;
} ;

struct mfsmsg_exit {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	char	reason[REALNAMELEN+1] ;
} ;

struct mfsmsg_mark {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
} ;

struct mfsmsg_ack {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
} ;

/* request listener information */
struct mfsmsg_getlistener {
	uint	msglen ;
	uint	tag ;
	uint	idx ;			/* listener index */
	uchar	msgtype ;		/* message type */
} ;

/* listener information (response) */
struct mfsmsg_listener {
	uint	msglen ;
	uint	tag ;
	uint	idx ;			/* listener index */
	uint	pid ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
	uchar	ls ;			/* listener status */
	char	name[MFSMSG_LNAMELEN + 1] ;
	char	addr[MFSMSG_LADDRLEN + 1] ;
} ;


/* request types */
enum mfsmsgtypes {
	mfsmsgtype_status,	/* 0 */
	mfsmsgtype_getstatus,	/* 1 */
	mfsmsgtype_getval,	/* 2 */
	mfsmsgtype_gethelp,
	mfsmsgtype_getname,
	mfsmsgtype_getuser,
        mfsmsgtype_getlistener,
	mfsmsgtype_exit,
	mfsmsgtype_mark,
	mfsmsgtype_val,
	mfsmsgtype_help,
	mfsmsgtype_name,
	mfsmsgtype_user,
	mfsmsgtype_ack,
	mfsmsgtype_listener,
	mfsmsgtype_overlast
} ;


/* response codes */
enum mfsmsgrcs {
	mfsmsgrc_ok,
	mfsmsgrc_invalid,
	mfsmsgrc_badfmt,
	mfsmsgrc_notavail,
	mfsmsgrc_notfound,
	mfsmsgrc_overlast
} ;


#if	(! defined(MFSMSG_MASTER)) || (MFSMSG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mfsmsg_getstatus(struct mfsmsg_getstatus *,int,char *,int) ;
extern int mfsmsg_gethelp(struct mfsmsg_gethelp *,int,char *,int) ;
extern int mfsmsg_getval(struct mfsmsg_getval *,int,char *,int) ;
extern int mfsmsg_getname(struct mfsmsg_getname *,int,char *,int) ;
extern int mfsmsg_getuser(struct mfsmsg_getuser *,int,char *,int) ;
extern int mfsmsg_getlistener(struct mfsmsg_getlistener *,int,char *,int) ;
extern int mfsmsg_mark(struct mfsmsg_mark *,int,char *,int) ;
extern int mfsmsg_exit(struct mfsmsg_exit *,int,char *,int) ;
extern int mfsmsg_status(struct mfsmsg_status *,int,char *,int) ;
extern int mfsmsg_val(struct mfsmsg_val *,int,char *,int) ;
extern int mfsmsg_help(struct mfsmsg_help *,int,char *,int) ;
extern int mfsmsg_name(struct mfsmsg_name *,int,char *,int) ;
extern int mfsmsg_user(struct mfsmsg_user *,int,char *,int) ;
extern int mfsmsg_listener(struct mfsmsg_listener *,int,char *,int) ;
extern int mfsmsg_ack(struct mfsmsg_ack *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* MFSMSG_MASTER */

#endif /* MFSMSG_INCLUDE */


