/* debug */

/* debug utilities */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEBUG_INCLUDE
#define	DEBUG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<netdb.h>
#include	<localmisc.h>


#if	(! defined(DEBUG_MASTER)) || (DEBUG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int debugopen(cchar *) ;
extern int debugclose(void) ;
extern int debugprint(cchar *,int) ;
extern int debugprintf(cchar *,...) ;
extern int debugprinthexblock(cchar *,int,const void *,int) ;
extern int nprintf(cchar *,cchar *,...) ;
extern int nprinthexblock(cchar *,cchar *,int,const void *,int) ;
extern int strlinelen(cchar *,int,int) ;
extern int strnnlen(cchar *,int,int) ;

extern int d_openfiles() ;
extern int d_ispath(cchar *) ;
extern int mkhexstr(char *,int,const void *,int) ;
extern int mkhexnstr(char *,int,int,cchar *,int) ;
extern int debugprinthex(cchar *,int,cchar *,int) ;
extern int debugprinthexblock(cchar *,int,const void *,int) ;
extern int hexblock(cchar *,cchar *,int) ;
extern int heaudit(struct hostent *,cchar *,int) ;
extern char *stroflags(char *,int) ;
extern char *d_reventstr(int,char *,int) ;
extern void d_whoopen(int *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEBUG_MASTER */

#endif /* DEBUG_INCLUDE */


