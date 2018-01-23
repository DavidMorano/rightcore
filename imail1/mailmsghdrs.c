/* mailmsghdrs */

/* put all of the header values of a message into an array */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2002-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine depends on a MAILMSG object to have already been
	instantiated (and initialized).    This present object should then be
	initialized with a pointer to the MAILMSG object.  This object will
	then put all of the message header values of the MAILMSG object into an
	array for quick (indexed) access.


*******************************************************************************/


#define	MAILMSGHDRS_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<mailmsg.h>
#include	<localmisc.h>

#include	"mailmsghdrs.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	SPACETAB(c)	(((c) == ' ') || ((c) == '\t'))


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* global variables */

const char *mailmsghdrs_names[] = {
	"From",			/* 0 */
	"To",
	"Date",
	"Subject",
	"title",
	"message-id",		/* 5 */
	"article-id",
	"content-length",
	"newsgroups",
	"in-reply-to",
	"board",		/* 10 */
	"lines",
	"reply-to",
	"references",
	"content-type",
	"expires",		/* 15 */
	"keywords",
	"control",
	"x-lines",
	"path",
	"errors-to",		/* 20 */
	"return-path",
	"received",
	"x-queuespec",		/* 23 */
	"x-service",
	"x-jobid",
	"x-orighost",
	"x-origuser",
	"x-username",
	"sender",
	"cc",
	"bcc",
	"status",
	"content-lines",	/* 33 */
	"content-transfer-encoding",	/* 34 */
	"organization",		/* 35 */
	"delivered-to",		/* 36 */
	"x-original-to",	/* 37 */
	"x-priority",		/* 38 */
	"priority",		/* 39 */
	"x-face",		/* 40 */
	"x-bbnews",		/* 41 */
	"x-universally-unique-identifier",
	"x-uniform-type-identifier",
	"x-mail-created-date",
	"x-mailer",		/* 45 */
	"x-forwarded-to",	/* 46 */
	"subj",			/* 47 */
	NULL			/* 48 */
} ;


/* local variables */


/* exported subroutines */


int mailmsghdrs_start(MAILMSGHDRS *mhp,MAILMSG *msgp)
{
	const int	n = (HI_NULL + 1) ;
	int		rs ;
	int		i ;
	int		size ;
	int		hl ;
	int		c = 0 ;
	const char	**mhnames = mailmsghdrs_names ;
	const char	*hp ;
	void		*p ;

	if ((mhp == NULL) || (msgp == NULL)) return SR_FAULT ;

	size = (n+1) * sizeof(char **) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    mhp->v = p ;

	    for (i = 0 ; (i < n) && (mhnames[i] != NULL) ; i += 1) {

	        mhp->v[i] = NULL ;
	        hl = mailmsg_hdrval(msgp,mhnames[i],&hp) ;
	        if (hl >= 0) {

	            mhp->v[i] = hp ;
	            c += 1 ;

	        } /* end if (message header search) */

	    } /* end for (looping over header names) */

	    mhp->v[i] = NULL ;
	    mhp->magic = MAILMSGHDRS_MAGIC ;
	} /* end if (memory-allocation) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mailmsghdrs_start) */


int mailmsghdrs_finish(MAILMSGHDRS *mhp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mhp == NULL) return SR_FAULT ;

	if (mhp->magic != MAILMSGHDRS_MAGIC) return SR_NOTOPEN ;

	if (mhp->v != NULL) {
	    rs1 = uc_free(mhp->v) ;
	    if (rs >= 0) rs = rs1 ;
	    mhp->v = NULL ;
	}

	mhp->magic = 0 ;
	return rs ;
}
/* end subroutine (mailmsghdrs_finish) */


