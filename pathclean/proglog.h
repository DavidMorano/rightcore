/* proglog */

/* program-logging */


/* revision history:

	= 2008-09-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Manage program logging operations.


*******************************************************************************/


#ifndef	PROGLOG_INCLUDE
#define	PROGLOG_INCLUDE	1


#include	<envstandards.h>	/* must be before others */
#include	<sys/types.h>
#include	<stdarg.h>
#include	<userinfo.h>
#include	<localmisc.h>

#include	"defs.h"


#if	(! defined(PROGLOG_MASTER)) || (PROGLOG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int proglog_begin(PROGINFO *,USERINFO *) ;
extern int proglog_end(PROGINFO *) ;
extern int proglog_intro(PROGINFO *,USERINFO *) ;
extern int proglog_checksize(PROGINFO *) ;
extern int proglog_check(PROGINFO *) ;
extern int proglog_print(PROGINFO *,cchar *,int) ;
extern int proglog_printf(PROGINFO *,cchar *,...) ;
extern int proglog_vprintf(PROGINFO *,cchar *,va_list) ;
extern int proglog_printfold(PROGINFO *,cchar *,cchar *,int) ;
extern int proglog_getid(PROGINFO *,char *,int) ;
extern int proglog_setid(PROGINFO *,cchar *,int) ;
extern int proglog_ssprint(PROGINFO *,cchar *,cchar *,int) ;
extern int proglog_ssprintf(PROGINFO *,cchar *,cchar *,...) ;
extern int proglog_ssvprintf(PROGINFO *,cchar *,cchar *,va_list) ;
extern int proglog_flush(PROGINFO *) ;
 
#ifdef	__cplusplus
}
#endif

#endif /* PROGLOG_MASTER */

#endif /* PROGLOG_INCLUDE */


