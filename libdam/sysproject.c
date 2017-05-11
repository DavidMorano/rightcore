/* sysproject */

/* system project-entry enumeration */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-24, David A­D­ Morano
        This object module was morphed from some previous one. I do not remember
        what the previous one was.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We enumberate (reentrantly and thread safely) project entries from the
	system PROJECT database.


*******************************************************************************/


#define	SYSPROJECT_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<projectent.h>
#include	<localmisc.h>

#include	"sysproject.h"


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


int sysproject_open(SYSPROJECT *op,const char *spfname)
{
	const size_t	max = INT_MAX ;
	int		rs ;
	const char	*pfname = SYSPROJECT_FNAME ;

	if (op == NULL) return SR_FAULT ;

	if (spfname == NULL) spfname = pfname ; /* default */

	memset(op,0,sizeof(SYSPROJECT)) ;

	if ((rs = filemap_open(&op->b,spfname,O_RDONLY,max)) >= 0) {
	    op->magic = SYSPROJECT_MAGIC ;
	}

	return rs ;
}
/* end if (sysproject_start) */


int sysproject_close(SYSPROJECT *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != SYSPROJECT_MAGIC) return SR_NOTOPEN ;

	rs1 = filemap_close(&op->b) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
} 
/* end subroutine (sysproject_close) */


int sysproject_readent(SYSPROJECT *op,struct project *pjp,char *pjbuf,int pjlen)
{
	int		rs ;
	int		ll ;
	const char	*lp ;
	if (op == NULL) return SR_FAULT ;
	if (pjp == NULL) return SR_FAULT ;
	if (pjbuf == NULL) return SR_FAULT ;
	while ((rs = filemap_getline(&op->b,&lp)) > 0) {
	    ll = rs ;
	    if (lp[ll-1] == '\n') ll -= 1 ;
#if	CF_DEBUGS
	    debugprintf("sysproject_readent: pl=>%t<\n",lp,ll) ;
#endif
	    rs = projectent_parse(pjp,pjbuf,pjlen,lp,ll) ;
#if	CF_DEBUGS
	    debugprintf("sysproject_readent: projectent_parse() rs=%d\n",rs) ;
#endif
	    if (rs > 0) break ;
	    if (rs < 0) break ;
	} /* end while */
#if	CF_DEBUGS
	debugprintf("sysproject_readent: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (sysproject_readent) */


int sysproject_reset(SYSPROJECT *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	rs = filemap_rewind(&op->b) ;

	return rs ;
}
/* end subroutine (sysproject_reset) */


