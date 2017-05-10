/* mailmsgatt */

/* message attachment object */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILMSGATT_INCLUDE
#define	MAILMSGATT_INCLUDE	1


#include	<envstandards.h>

#include	<vecitem.h>
#include	<mimetypes.h>

#include	<contentencodings.h>
#include	<mimetypes.h>


/* object defines */

#define	MAILMSGATTENT_MAGIC	0x49827261
#define	MAILMSGATT		VECITEM
#define	MAILMSGATT_ENT		struct mailmsgatt_ent

#ifndef	MAILMSGATTENT
#define	MAILMSGATTENT		MAILMSGATT_ENT
#endif

struct mailmsgatt_ent {
	uint		magic ;
	const char	*type ;		/* content-type */
	const char	*subtype ;
	const char	*attfname ;	/* attachment-filename */
	const char	*auxfname ;	/* auxiliary-filename */
	const char	*ext ;
	const char	*encoding ;	/* content-encoding */
	const char	*description ;
	int		clen ;		/* content-length */
	int		clines ;	/* content-lines */
	int		cte ;		/* content-transfer-encoding */
	int		f_plaintext ;
} ;


#if	(! defined(MAILMSGATT_MASTER)) || (MAILMSGATT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailmsgatt_start(MAILMSGATT *) ;
extern int mailmsgatt_finish(MAILMSGATT *) ;
extern int mailmsgatt_add(MAILMSGATT *,cchar *,cchar *,cchar *,int) ;
extern int mailmsgatt_count(MAILMSGATT *) ;
extern int mailmsgatt_enum(MAILMSGATT *,int,MAILMSGATT_ENT **) ;
extern int mailmsgatt_typeatts(MAILMSGATT *,MIMETYPES *) ;

extern int mailmsgattent_start(MAILMSGATTENT *,cchar *,cchar *,cchar *,int) ;
extern int mailmsgattent_type(MAILMSGATTENT *,MIMETYPES *) ;
extern int mailmsgattent_typeset(MAILMSGATTENT *,const char *,const char *) ;
extern int mailmsgattent_isplaintext(MAILMSGATTENT *) ;
extern int mailmsgattent_finish(MAILMSGATTENT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILMSGATT_MASTER */

#endif /* MAILMSGATT_INCLUDE */


