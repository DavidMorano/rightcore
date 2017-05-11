/* mailmsgfrom */

/* mail-message-from processing */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Process a time value and return whether it is a "hit."


*******************************************************************************/


#define	MAILMSGFROM_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"mailmsgfrom.h"


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mailmsgfrom_start(MAILMSGFROM *op,char *fbuf,int flen)
{
	if (op == NULL) return SR_FAULT ;
	memset(op,0,sizeof(MAILMSGFROM)) ;
	op->fbuf = fbuf ;
	op->flen = flen ;
	return SR_OK ;
}
/* end subroutine (mailmsgfrom_start) */


int mailmsgfrom_finish(MAILMSGFROM *op)
{
	if (op == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (mailmsgfrom_finish) */


int mailmsgfrom_test(MAILMSGFROM *op,time_t t)
{
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (op == NULL) return SR_FAULT ;
	if (t > op->ti_msg) {
	    f = TRUE ;
	    op->ti_msg = t ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mailmsgfrom_test) */


int mailmsgfrom_loadfrom(MAILMSGFROM *op,cchar *sp,int sl)
{
	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	op->fl = strdcpy1w(op->fbuf,op->flen,sp,sl) - op->fbuf ;
	return op->fl ;
}
/* end subroutine (mailmsgfrom_loadfrom) */


