/* syspasswd */

/* system user-entry enumeration */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 1998-03-24, David A­D­ Morano
        This object module was morphed from some previous one. I do not remember
        what the previous one was.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We enumerate (reentrantly and thread safely) user names from the system
	PASSWD database.


*******************************************************************************/


#define	SYSPASSWD_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<passwdent.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<localmisc.h>

#include	"syspasswd.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */
#undef	COMMENT


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int syspasswd_open(SYSPASSWD *op,cchar *sufname)
{
	const size_t	max = INT_MAX ;
	int		rs ;
	const char	*defufname = SYSPASSWD_FNAME ;

	if (op == NULL) return SR_FAULT ;

	if (sufname == NULL) sufname = defufname ; /* default */

	memset(op,0,sizeof(SYSPASSWD)) ;

	if ((rs = filemap_open(&op->b,sufname,O_RDONLY,max)) >= 0) {
	        op->magic = SYSPASSWD_MAGIC ;
	}

	return rs ;
}
/* end if (syspasswd_start) */


int syspasswd_close(SYSPASSWD *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != SYSPASSWD_MAGIC) return SR_NOTOPEN ;

	rs1 = filemap_close(&op->b) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
} 
/* end subroutine (syspasswd_close) */


int syspasswd_readent(SYSPASSWD *op,struct passwd *pwp,char *pwbuf,int pwlen)
{
	int		rs ;
	int		pwl = 0 ;
	const char	*lp ;

	if (op == NULL) return SR_FAULT ;
	if (pwp == NULL) return SR_FAULT ;
	if (pwbuf == NULL) return SR_FAULT ;
	if (op->magic != SYSPASSWD_MAGIC) return SR_NOTOPEN ;

	while ((rs = filemap_getline(&op->b,&lp)) > 0) {
	    rs = passwdent_parse(pwp,pwbuf,pwlen,lp,rs) ;
	    pwl = rs ;
	    if (pwl > 0) break ;
	    if (rs < 0) break ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("syspasswd_readent: ret rs=%d pwl=%u\n",rs,pwl) ;
#endif

	return (rs >= 0) ? pwl : rs ;
}
/* end subroutine (syspasswd_readent) */


int syspasswd_reset(SYSPASSWD *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != SYSPASSWD_MAGIC) return SR_NOTOPEN ;

	rs = filemap_rewind(&op->b) ;

	return rs ;
}
/* end subroutine (syspasswd_reset) */


