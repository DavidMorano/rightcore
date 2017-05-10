/* ulmsg */

/* User-Location-Manager */

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */


#ifndef	ULMSG_INCLUDE
#define	ULMSG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<netdb.h>

#include	<localmisc.h>


#define	ULMSG_LLINE	32
#define	ULMSG_LTERMTYPE	32
#define	ULMSG_LUSERNAME	LOGNAMELEN
#define	ULMSG_LNODE	NODENAMELEN
#define	ULMSG_LCLUSTER	NODENAMELEN
#define	ULMSG_LDOMAIN	MAXHOSTNAMELEN
#define	ULMSG_LHOST	MAXHOSTNAMELEN
#define	ULMSG_LID	4		/* UTMPX ID */


struct ulmsg_update {
	uint	msglen ;
	uint	ts ;			/* time stamp */
	uint	ta, tm ;		/* time access, time modification */
	uint	tu_sec, tu_usec ;	/* time UTMPX */
	int	pid ;			/* process ID */
	int	sid ;			/* session ID */
	ushort	termination, exit ;
	ushort	utype ;			/* UTMPX type */
	uchar	msgtype ;		/* message type */
	uchar	stype ;			/* sub-type */
	uchar	status ;		/* termtype status */
	char	username[LOGNAMELEN + 1] ;
	char	line[ULMSG_LLINE + 1] ;
	char	node[NODENAMELEN + 1] ;
	char	cluster[ULMSG_LCLUSTER + 1] ;
	char	domain[MAXHOSTNAMELEN + 1] ;
	char	host[MAXHOSTNAMELEN + 1] ;
	char	termtype[ULMSG_LTERMTYPE + 1] ;
	char	id[5] ;			/* UTMPX ID */
} ;

struct ulmsg_request {
	uint	msglen ;
	uint	tag ;
	uint	ts ;			/* time stamp */
	uint	expire ;		/* expire time */
	uchar	msgtype ;		/* message type */
	uchar	stype ;			/* sub-type */
	char	username[LOGNAMELEN + 1] ;
	char	line[ULMSG_LLINE + 1] ;
	char	node[NODENAMELEN + 1] ;
	char	cluster[ULMSG_LCLUSTER + 1] ;
	char	domain[MAXHOSTNAMELEN + 1] ;
	char	host[MAXHOSTNAMELEN + 1] ;
	char	termtype[ULMSG_LTERMTYPE + 1] ;
	char	id[5] ;			/* UTMPX ID */
} ;

struct ulmsg_response {
	uint	msglen ;
	uint	tag ;
	uint	ts ;			/* time stamp */
	uint	ta, tm ;		/* time access, time modification */
	uint	tu_sec, tu_usec ;	/* time UTMPX */
	int	pid ;			/* process ID */
	int	sid ;			/* session ID */
	ushort	termination, exit ;
	ushort	utype ;			/* UTMPX type */
	uchar	msgtype ;		/* message type */
	uchar	stype ;			/* sub-type */
	uchar	status ;		/* termtype status */
	uchar	rc ;
	char	username[LOGNAMELEN + 1] ;
	char	line[ULMSG_LLINE + 1] ;
	char	node[NODENAMELEN + 1] ;
	char	cluster[ULMSG_LCLUSTER + 1] ;
	char	domain[MAXHOSTNAMELEN + 1] ;
	char	host[MAXHOSTNAMELEN + 1] ;
	char	termtype[ULMSG_LTERMTYPE + 1] ;
	char	id[5] ;			/* UTMPX ID */
} ;


/* request types */
enum ulmsgtypes {
	ulmsgtype_update,
	ulmsgtype_request,
	ulmsgtype_response,
	ulmsgtype_overlast
} ;

/* response codes */
enum ulmsgrcs {
	ulmsgrc_ok,
	ulmsgrc_notfound,
	ulmsgrc_goingdown,
	ulmsgrc_overlast
} ;


#if	(! defined(ULMSG_MASTER)) || (ULMSG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ulmsg_update(struct ulmsg_update *,int, char *,int) ;
extern int	ulmsg_request(struct ulmsg_request *,int, char *,int) ;
extern int	ulmsg_response(struct ulmsg_response *,int,char *,int) ;

#ifdef	__cplusplus
extern "C" {
#endif

#endif /* ULMSG_MASTER */


#endif /* ULMSG_INCLUDE */



