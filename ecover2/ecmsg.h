/* ecmsg */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	ECMSG_INCLUDE
#define	ECMSG_INCLUDE	1


/* object defines */

#define	ECMSG			struct ecmsg
#define	ECMSG_MAXBUFLEN		(8 * 1024)


struct ecmsg {
	char	*buf ;
	int	buflen ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int ecmsg_start(ECMSG *) ;
extern int ecmsg_loadbuf(ECMSG *,const char *,int) ;
extern int ecmsg_already(ECMSG *) ;
extern int ecmsg_finish(ECMSG *) ;

#ifdef	__cplusplus
}
#endif

#endif /* ECMSG_INCLUDE */


