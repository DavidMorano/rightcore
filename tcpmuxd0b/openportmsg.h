/* openportmsg */

/* subroutine to open a priviledged network port */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	OPENPORTMSG_INCLUDE
#define	OPENPORTMSG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>

#include	<sockaddress.h>
#include	<localmisc.h>


/* this is the message that is passed from the caller */
struct openportmsg_request {
	SOCKADDRESS	sa ;
	int		pf ;		/* suitable for 'socket(3xnet)' */
	int		ptype ;		/* suitable for 'socket(3xnet)' */
	int		proto ;		/* suitable for 'socket(3xnet)' */
	uint		msglen ;
	uchar		msgtype ;
	char		username[USERNAMELEN+1] ;
} ;

struct openportmsg_response {
	int		rs ;
	uint		msglen ;
	uchar		msgtype ;
} ;


/* message types */

enum openportmsgtypes {
	openportmsgtype_request,
	openportmsgtype_response,
	openportmsgtype_overlast
} ;


#if	(! defined(OPENPORTMSG_MASTER)) || (OPENPORTMSG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int openportmsg_request(struct openportmsg_request *,int,char *,int) ;
extern int openportmsg_response(struct openportmsg_response *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* OPENPORTMSG_MASTER */

#endif /* OPENPORTMSG_INCLUDE */


