/* dialcprogmsg */

/* dial a program within the current machine cluster */


/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

#ifndef	DIALCPROGMSG_INCLUDE
#define	DIALCPROGMSG_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/socket.h>

#include	<sockaddress.h>
#include	<localmisc.h>


/* message types */
enum dialcprogmsgtypes {
	dialcprogmsgtype_end,
	dialcprogmsgtype_pwd,
	dialcprogmsgtype_fname,
	dialcprogmsgtype_arg,
	dialcprogmsgtype_env,
	dialcprogmsgtype_light,
	dialcprogmsgtype_nodename,
	dialcprogmsgtype_overlast
} ;


struct dialcprogmsg_end {
	uchar		type ;
	ushort		len ;
	ushort		flags ;
	int		opts ;
} ;

struct dialcprogmsg_light {
	uchar		type ;
	ushort		len ;
	ushort		salen1 ;
	ushort		salen2 ;
	SOCKADDRESS	saout ;
	SOCKADDRESS	saerr ;
} ;


#define	DIALCPROGMSG_END	(1 + (2 * 2) + sizeof(int))
#define	DIALCPROGMSG_LIGHT	(1 + (3 * 2) + (2 * sizeof(SOCKADDRESS)))

/* flags */
#define	DIALCPROGMSG_FERRCHAN	0x01


#if	(! defined(DIALCPROGMSG_MASTER)) || (DIALCPROGMSG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	dialcprogmsg_end(char *,int,int,struct dialcprogmsg_end *) ;
extern int	dialcprogmsg_light(char *,int,int,struct dialcprogmsg_light *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DIALCPROGMSG_MASTER */

#endif /* DIALCPROGMSG_INCLUDE */


