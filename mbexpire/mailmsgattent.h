/* mailmsgattent */

/* mail-message attachment entry object */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILMSGATTENT_INCLUDE
#define	MAILMSGATTENT_INCLUDE	1


#include	<envstandards.h>

#include	<mailmsgatt.h>


/* object defines */

#define	MAILMSGATTENT		MAILMSGATT_ENT


#if	(! defined(MAILMSGATTENT_MASTER)) || (MAILMSGATTENT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailmsgattent_code(MAILMSGATTENT *,const char *) ;
extern int mailmsgattent_setcode(MAILMSGATTENT *,int) ;
extern int mailmsgattent_analyze(MAILMSGATTENT *,const char *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILMSGATTENT_MASTER */

#endif /* MAILMSGATTENT_INCLUDE */


