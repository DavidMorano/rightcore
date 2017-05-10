/* sysusershells */

/* system user-shell enumeration */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-24, David A­D­ Morano
        This object module was morphed from some previous one. I do not remember
        what the previous one was.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We enumerate (reentrantly and thread safely) user-shell entries from the
        system user-shell database.


*******************************************************************************/


#define	SYSUSERSHELLS_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getax.h>
#include	<localmisc.h>

#include	"sysusershells.h"


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int sysusershells_open(SYSUSERSHELLS *op,const char *sufname)
{
	const size_t	max = INT_MAX ;
	int		rs ;
	const char	*defufname = SYSUSERSHELLS_FNAME ;

	if (op == NULL) return SR_FAULT ;

	if (sufname == NULL) sufname = defufname ; /* default */

	memset(op,0,sizeof(SYSUSERSHELLS)) ;

	if ((rs = filemap_open(&op->b,sufname,O_RDONLY,max)) >= 0) {
	    op->magic = SYSUSERSHELLS_MAGIC ;
	}

	return rs ;
}
/* end if (sysusershells_start) */


int sysusershells_close(SYSUSERSHELLS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != SYSUSERSHELLS_MAGIC) return SR_NOTOPEN ;

	rs1 = filemap_close(&op->b) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
} 
/* end subroutine (sysusershells_close) */


int sysusershells_readent(SYSUSERSHELLS *op,char *ubuf,int ulen)
{
	int		rs ;
	int		ll ;
	const char	*lp ;

	if (op == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;
	if (op->magic != SYSUSERSHELLS_MAGIC) return SR_NOTOPEN ;

	ubuf[0] = '\0' ;
	while ((rs = filemap_getline(&op->b,&lp)) > 0) {
	    ll = rs ;
	    if (lp[ll-1] == '\n') ll -= 1 ;
	    rs = snwcpy(ubuf,ulen,lp,ll) ;
	    if (rs > 0) break ;
	    if (rs < 0) break ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("sysusershells_readent: ret rs=%d\n",rs) ;
	debugprintf("sysusershells_readent: un=%s\n",ubuf) ;
#endif

	return rs ;
}
/* end subroutine (sysusershells_readent) */


int sysusershells_reset(SYSUSERSHELLS *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != SYSUSERSHELLS_MAGIC) return SR_NOTOPEN ;

	rs = filemap_rewind(&op->b) ;

	return rs ;
}
/* end subroutine (sysusershells_reset) */


