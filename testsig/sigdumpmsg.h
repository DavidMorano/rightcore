/* sigdumpmsg */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	SIGDUMPMSG_INCLUDE
#define	SIGDUMPMSG_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<netdb.h>

#include	<localmisc.h>


/* client request message */
struct sigdumpmsg_request {
	uint	tag ;
	uint	pid ;
	char	fname[MAXNAMELEN + 1] ;
	uchar	type ;
} ;


/* request types */
enum sigdumpmsgtypes {
	sigdumpmsgtype_request,
	sigdumpmsgtype_overlast
} ;


/* response codes */
enum sigdumpmsgrcs {
	sigdumpmsgrc_ok,
	sigdumpmsgrc_invalid,
	sigdumpmsgrc_notavail,
	sigdumpmsgrc_done,
	sigdumpmsgrc_goingdown,
	sigdumpmsgrc_overlast
} ;


/* message sizes */

#define	SIGDUMPMSG_SREQUEST	((7 * sizeof(uint)) + (3 * sizeof(ushort)) + 1)


/* options */

#define	SIGDUMPMSG_MBLANK	0x00
#define	SIGDUMPMSG_MTCP		0x01		/* also the default */
#define	SIGDUMPMSG_MUDP		0x02
#define	SIGDUMPMSG_MLOADAVE	0x04		/* load average */
#define	SIGDUMPMSG_MEXTRA	0x08		/* all information */


#if	(! defined(SIGDUMPMSG_MASTER)) || (SIGDUMPMSG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	sigdumpmsg_request(struct sigdumpmsg_request *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* SIGDUMPMSG_MASTER */

#endif /* SIGDUMPMSG_INCLUDE */


