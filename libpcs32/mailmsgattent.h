/* mailmsgattent */

/* mail-message attachment entry object */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILMSGATTENT_INCLUDE
#define	MAILMSGATTENT_INCLUDE	1


#include	<envstandards.h>
#include	<contypevals.h>		/* content-type values */
#include	<mimetypes.h>
#include	<localmisc.h>


#define	MAILMSGATTENT_MAGIC	0x49827261
#define	MAILMSGATTENT		struct mailmsgattent


struct mailmsgattent {
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

#if	(! defined(MAILMSGATTENT_MASTER)) || (MAILMSGATTENT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailmsgattent_start(MAILMSGATTENT *,cchar *,cchar *,cchar *,int) ;
extern int mailmsgattent_type(MAILMSGATTENT *,MIMETYPES *) ;
extern int mailmsgattent_typeset(MAILMSGATTENT *,cchar *,cchar *) ;
extern int mailmsgattent_isplaintext(MAILMSGATTENT *) ;
extern int mailmsgattent_finish(MAILMSGATTENT *) ;
extern int mailmsgattent_code(MAILMSGATTENT *,cchar *) ;
extern int mailmsgattent_setcode(MAILMSGATTENT *,int) ;
extern int mailmsgattent_analyze(MAILMSGATTENT *,cchar *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILMSGATTENT_MASTER */

#endif /* MAILMSGATTENT_INCLUDE */


