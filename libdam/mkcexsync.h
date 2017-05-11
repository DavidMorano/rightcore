/* mkcexsync */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MKCEXSYNC_INCLUDE
#define	MKCEXSYNC_INCLUDE	1


#define	MKCEXSYNC_MKLEN		10	/* length to create (in bytes) */
#define	MKCEXSYNC_REQLEN	6	/* required length */
#define	MKCEXSYNC_FINLEN	2	/* number of finishing bytes */


#ifdef	__cplusplus
extern "C" {
#endif

extern int mkcexsync(char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* MKCEXSYNC_INCLUDE */


