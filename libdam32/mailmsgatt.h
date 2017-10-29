/* mailmsgatt */

/* message attachment object */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILMSGATT_INCLUDE
#define	MAILMSGATT_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<vecitem.h>
#include	<mimetypes.h>
#include	<contentencodings.h>
#include	<mailmsgattent.h>


#define	MAILMSGATT		VECITEM


#if	(! defined(MAILMSGATT_MASTER)) || (MAILMSGATT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailmsgatt_start(MAILMSGATT *) ;
extern int mailmsgatt_finish(MAILMSGATT *) ;
extern int mailmsgatt_add(MAILMSGATT *,cchar *,cchar *,cchar *,int) ;
extern int mailmsgatt_count(MAILMSGATT *) ;
extern int mailmsgatt_enum(MAILMSGATT *,int,MAILMSGATTENT **) ;
extern int mailmsgatt_typeatts(MAILMSGATT *,MIMETYPES *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILMSGATT_MASTER */

#endif /* MAILMSGATT_INCLUDE */


