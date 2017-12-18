/* mailmsg_loadfile */

/* load the header-lines from a MAILMSG file into the object */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine load lines from a file containing a mail-message into
	the MAILMSG object (an dobject for abstracting mail-message
	information).

	Synopsis:

	int mailmsg_loadfile(msgp,fp)
	MAILMSG		*msgp ;
	bfile		*fp ;

	Arguments:

	msgp		pointer to MAILMSG object
	fp		pointer to 'bfile' file handle

	Returns:

	>=0		OK
	<0		some error


	Note: At first we skip empty lines until we find a non-empty line;
	afterwards we do not ignore empty lines.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"mailmsg.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	MAILMSGLINEBUFLEN
#define	MAILMSGLINEBUFLEN	(LINEBUFLEN * 5)
#endif

#define	ISEND(c)	(((c) == '\n') || ((c) == '\r'))


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mknpath1(char *,int,const char *) ;
extern int	mknpath2(char *,int,const char *,const char *) ;
extern int	matkeystr(const char **,char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mailmsg_loadfile(MAILMSG *msgp,bfile *fp)
{
	const int	llen = MAILMSGLINEBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		tlen = 0 ;
	char		*lbuf ;

	if (msgp == NULL) return SR_FAULT ;
	if (fp == NULL) return SR_FAULT ;

	if ((rs = uc_malloc((llen+1),&lbuf)) >= 0) {
	    int		ll ;
	    int		line = 0 ;
	    cchar	*lp = lbuf ;

	    while ((rs = breadline(fp,lbuf,llen)) > 0) {
	        ll = rs ;

	        tlen += ll ;
	        while ((ll > 0) && ISEND(lp[0]))
	            ll -= 1 ;

	        if ((ll > 0) || (line > 0)) {
	            line += 1 ;
	            rs = mailmsg_loadline(msgp,lp,ll) ;
	        }

	        if (rs <= 0) break ;
	    } /* end while */

	    rs1 = uc_free(lbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (memory-allocation) */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (mailmsg_loadfile) */


