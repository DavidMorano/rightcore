/* sysprojects */

/* system project-entry enumeration */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-24, David A­D­ Morano
        This object module was morphed from some previous one. I do not remember
        what the previous one was.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We enumerate (reentrantly and thread safely) project names from the
	system PROJECT database.


*******************************************************************************/


#define	SYSPROJECTS_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getax.h>
#include	<localmisc.h>

#include	"sysprojects.h"


/* local defines */


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


int sysprojects_open(SYSPROJECTS *op,const char *spfname)
{
	const size_t	max = INT_MAX ;
	int		rs ;
	const char	*pfname = SYSPROJECTS_FNAME ;

	if (op == NULL) return SR_FAULT ;

	if (spfname == NULL) spfname = pfname ; /* default */

	memset(op,0,sizeof(SYSPROJECTS)) ;

	    if ((rs = filemap_open(&op->b,spfname,O_RDONLY,max)) >= 0) {
		op->magic = SYSPROJECTS_MAGIC ;
	    }

	return rs ;
}
/* end if (sysprojects_start) */


int sysprojects_close(SYSPROJECTS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != SYSPROJECTS_MAGIC) return SR_NOTOPEN ;

	rs1 = filemap_close(&op->b) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
} 
/* end subroutine (sysprojects_close) */


int sysprojects_readent(op,pjp,pjbuf,pjlen)
SYSPROJECTS	*op ;
struct project	*pjp ;
char		pjbuf[] ;
int		pjlen ;
{
	const int	plen = PROJNAMELEN ;
	int		rs ;
	int		ll ;
	const char	*lp ;
	char		pbuf[PROJNAMELEN+1] = { 0 } ;
	if (op == NULL) return SR_FAULT ;
	if (pjp == NULL) return SR_FAULT ;
	if (pjbuf == NULL) return SR_FAULT ;
	while ((rs = filemap_getline(&op->b,&lp)) > 0) {
	    ll = rs ;
	    if (lp[ll-1] == '\n') ll -= 1 ;
	    if ((rs = (strdcpy1w(pbuf,plen,lp,ll)-pbuf)) > 0) {
	        rs = getpj_name(pjp,pjbuf,pjlen,pbuf) ;
	        if (rs == SR_NOTFOUND) rs = SR_OK ;
	    }
	    if (rs > 0) break ;
	    if (rs < 0) break ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("sysprojects_readent: ret rs=%d\n",rs) ;
	debugprintf("sysprojects_readent: pn=%s\n",pbuf) ;
#endif

	return rs ;
}
/* end subroutine (sysprojects_readent) */


int sysprojects_reset(op)
SYSPROJECTS	*op ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	rs = filemap_rewind(&op->b) ;

	return rs ;
}
/* end subroutine (sysprojects_reset) */


