/* sysusers */

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


#define	SYSUSERS_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<localmisc.h>

#include	"sysusers.h"


/* local defines */

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


int sysusers_open(SYSUSERS *op,cchar *sufname)
{
	const size_t	max = INT_MAX ;
	int		rs ;
	const char	*defufname = SYSUSERS_FNAME ;

	if (op == NULL) return SR_FAULT ;

	if (sufname == NULL) sufname = defufname ; /* default */

	memset(op,0,sizeof(SYSUSERS)) ;

	if ((rs = filemap_open(&op->b,sufname,O_RDONLY,max)) >= 0) {
	    op->magic = SYSUSERS_MAGIC ;
	}

	return rs ;
}
/* end if (sysusers_start) */


int sysusers_close(SYSUSERS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != SYSUSERS_MAGIC) return SR_NOTOPEN ;

	rs1 = filemap_close(&op->b) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
} 
/* end subroutine (sysusers_close) */


int sysusers_readent(SYSUSERS *op,struct passwd *pwp,char *pwbuf,int pwlen)
{
	const int	ulen = (USERNAMELEN+1) ;
	int		rs ;
	int		ll ;
	const char	*lp ;
	char		ubuf[(USERNAMELEN+1)+1] = { 0 } ;

	if (op == NULL) return SR_FAULT ;
	if (pwp == NULL) return SR_FAULT ;
	if (pwbuf == NULL) return SR_FAULT ;
	if (op->magic != SYSUSERS_MAGIC) return SR_NOTOPEN ;

	while ((rs = filemap_getline(&op->b,&lp)) > 0) {
	    ll = rs ;
	    if (lp[ll-1] == '\n') ll -= 1 ;
	    if ((rs = (strdcpy1w(ubuf,ulen,lp,ll)-ubuf)) > 0) {
	        rs = GETPW_NAME(pwp,pwbuf,pwlen,ubuf) ;
	        if (rs == SR_NOTFOUND) rs = SR_OK ;
	    }
	    if (rs > 0) break ;
	    if (rs < 0) break ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("sysusers_readent: ret rs=%d\n",rs) ;
	debugprintf("sysusers_readent: un=%s\n",ubuf) ;
#endif

	return rs ;
}
/* end subroutine (sysusers_readent) */


int sysusers_reset(SYSUSERS *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != SYSUSERS_MAGIC) return SR_NOTOPEN ;

	rs = filemap_rewind(&op->b) ;

	return rs ;
}
/* end subroutine (sysusers_reset) */


