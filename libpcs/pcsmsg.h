/* pcsmsg */


/* revision history:

	= 1999-05-24, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */


#ifndef	PCSMSG_INCLUDE
#define	PCSMSG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<realname.h>
#include	<localmisc.h>


#ifndef	PCSMSG_KEYLEN
#define	PCSMSG_KEYLEN	MAX(60,USERNAMELEN)
#endif


struct pcsmsg_getstatus {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
} ;

struct pcsmsg_status {
	uint	msglen ;
	uint	tag ;
	uint	pid ;
	uint	queries ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
} ;

struct pcsmsg_getval {
	uint	msglen ;
	uint	tag ;
	uchar	w ;			/* request code */
	uchar	msgtype ;		/* message type */
	char	key[PCSMSG_KEYLEN+1] ;
} ;

struct pcsmsg_val {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	uchar	w ;			/* request code */
	uchar	rc ;
	uchar	vl ;
	char	val[REALNAMELEN+1] ;
} ;

struct pcsmsg_gethelp {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	uchar	idx ;
} ;

struct pcsmsg_help {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	uchar	idx ;
	uchar	rc ;
	uchar	vl ;
	char	val[REALNAMELEN+1] ;
} ;

struct pcsmsg_getname {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	char	un[USERNAMELEN+1] ;
} ;

struct pcsmsg_name {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
	char	rn[REALNAMELEN+1] ;
} ;

struct pcsmsg_getuser {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	char	spec[REALNAMELEN+1] ;
} ;

struct pcsmsg_user {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
	char	un[USERNAMELEN+1] ;
} ;

struct pcsmsg_exit {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	char	reason[REALNAMELEN+1] ;
} ;

struct pcsmsg_mark {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
} ;

struct pcsmsg_ack {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
} ;


/* request types */
enum pcsmsgtypes {
	pcsmsgtype_status,	/* 0 */
	pcsmsgtype_getstatus,	/* 1 */
	pcsmsgtype_getval,	/* 2 */
	pcsmsgtype_gethelp,
	pcsmsgtype_getname,
	pcsmsgtype_getuser,
	pcsmsgtype_exit,
	pcsmsgtype_mark,
	pcsmsgtype_val,
	pcsmsgtype_help,
	pcsmsgtype_name,
	pcsmsgtype_user,
	pcsmsgtype_ack,
	pcsmsgtype_overlast
} ;


/* response codes */
enum pcsmsgrcs {
	pcsmsgrc_ok,
	pcsmsgrc_invalid,
	pcsmsgrc_badfmt,
	pcsmsgrc_notavail,
	pcsmsgrc_notfound,
	pcsmsgrc_overlast
} ;


#if	(! defined(PCSMSG_MASTER)) || (PCSMSG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pcsmsg_getstatus(struct pcsmsg_getstatus *,int,char *,int) ;
extern int pcsmsg_gethelp(struct pcsmsg_gethelp *,int,char *,int) ;
extern int pcsmsg_getval(struct pcsmsg_getval *,int,char *,int) ;
extern int pcsmsg_getname(struct pcsmsg_getname *,int,char *,int) ;
extern int pcsmsg_getuser(struct pcsmsg_getuser *,int,char *,int) ;
extern int pcsmsg_mark(struct pcsmsg_mark *,int,char *,int) ;
extern int pcsmsg_exit(struct pcsmsg_exit *,int,char *,int) ;
extern int pcsmsg_status(struct pcsmsg_status *,int,char *,int) ;
extern int pcsmsg_val(struct pcsmsg_val *,int,char *,int) ;
extern int pcsmsg_help(struct pcsmsg_help *,int,char *,int) ;
extern int pcsmsg_name(struct pcsmsg_name *,int,char *,int) ;
extern int pcsmsg_user(struct pcsmsg_user *,int,char *,int) ;
extern int pcsmsg_ack(struct pcsmsg_ack *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* PCSMSG_MASTER */

#endif /* PCSMSG_INCLUDE */


