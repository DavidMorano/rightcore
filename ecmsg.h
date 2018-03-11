/* ecmsg */


/* revision history:

	= 1999-06-13, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */


#ifndef	ECMSG_INCLUDE
#define	ECMSG_INCLUDE	1


/* object defines */

#define	ECMSG			struct ecmsg
#define	ECMSG_MAXBUFLEN		(8 * 1024)


struct ecmsg {
	char	*ebuf ;
	int	elen ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int ecmsg_start(ECMSG *) ;
extern int ecmsg_loadbuf(ECMSG *,cchar *,int) ;
extern int ecmsg_already(ECMSG *) ;
extern int ecmsg_finish(ECMSG *) ;

#ifdef	__cplusplus
}
#endif

#endif /* ECMSG_INCLUDE */


