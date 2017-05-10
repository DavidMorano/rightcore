/* uux */

/* SYSDIALER "uux" module */


/* revision history:

	- 1996-02-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	UUX_INCLUDE
#define	UUX_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<logfile.h>
#include	<localmisc.h>

#include	"sysdialer.h"


#define	UUX_MAGIC	0x31455926
#define	UUX		struct uux_head
#define	UUX_FL		struct uux_flags


struct uux_flags {
	uint		log:1 ;
} ;

struct uux_head {
	uint		magic ;
	LOGFILE		lh ;
	UUX_FL		f ;
	int		fd ;
	int		tlen ;
} ;


#if	(! defined(UUX_MASTER)) || (UUX_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int uux_open(UUX *,SYSDIALER_ARGS *,
		const char *,const char *,const char **) ;
extern int uux_reade(UUX *,char *,int,int,int) ;
extern int uux_recve(UUX *,char *,int,int,int,int) ;
extern int uux_recvfrome(UUX *,char *,int,int,void *,int *,int,int) ;
extern int uux_recvmsge(UUX *,struct msghdr *,int,int,int) ;
extern int uux_write(UUX *,const char *,int) ;
extern int uux_send(UUX *,const char *,int,int) ;
extern int uux_sendto(UUX *,const char *,int,int,void *,int) ;
extern int uux_sendmsg(UUX *,struct msghdr *,int) ;
extern int uux_shutdown(UUX *,int) ;
extern int uux_close(UUX *) ;

#ifdef	__cplusplus
}
#endif

#endif /* UUX_MASTER */

#endif /* UUX_INCLUDE */


