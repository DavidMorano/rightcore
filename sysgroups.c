/* sysgroups */

/* system group-entry enumeration */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-24, David A­D­ Morano
        This object module was morphed from some previous one. I do not remember
        what the previous one was.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We enumerate (reentrantly and thread safely) group names from the
	system GROUP database.


*******************************************************************************/


#define	SYSGROUPS_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getax.h>
#include	<localmisc.h>

#include	"sysgroups.h"


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


int sysgroups_open(SYSGROUPS *op,const char *sgfname)
{
	const size_t	max = INT_MAX ;
	int		rs ;
	const char	*defgfname = SYSGROUPS_FNAME ;

	if (op == NULL) return SR_FAULT ;

	if (sgfname == NULL) sgfname = defgfname ; /* default */

	memset(op,0,sizeof(SYSGROUPS)) ;

	if ((rs = filemap_open(&op->b,sgfname,O_RDONLY,max)) >= 0) {
	    op->magic = SYSGROUPS_MAGIC ;
	}

	return rs ;
}
/* end if (sysgroups_open) */


int sysgroups_close(SYSGROUPS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != SYSGROUPS_MAGIC) return SR_NOTOPEN ;

	rs1 = filemap_close(&op->b) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
} 
/* end subroutine (sysgroups_close) */


int sysgroups_readent(SYSGROUPS *op,struct group *grp,char *grbuf,int grlen)
{
	const int	glen = GROUPNAMELEN ;
	int		rs ;
	int		ll ;
	const char	*lp ;
	char		gbuf[GROUPNAMELEN+1] = { 0 } ;

	if (op == NULL) return SR_FAULT ;
	if (grp == NULL) return SR_FAULT ;
	if (grbuf == NULL) return SR_FAULT ;
	if (op->magic != SYSGROUPS_MAGIC) return SR_NOTOPEN ;

	while ((rs = filemap_getline(&op->b,&lp)) > 0) {
	    ll = rs ;
	    if (lp[ll-1] == '\n') ll -= 1 ;
	    if ((rs = (strdcpy1w(gbuf,glen,lp,ll)-gbuf)) > 0) {
	        rs = getgr_name(grp,grbuf,grlen,gbuf) ;
	        if (rs == SR_NOTFOUND) rs = SR_OK ;
	    }
	    if (rs > 0) break ;
	    if (rs < 0) break ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("sysgroups_readent: ret rs=%d\n",rs) ;
	debugprintf("sysgroups_readent: gn=%s\n",gbuf) ;
#endif

	return rs ;
}
/* end subroutine (sysgroups_readent) */


int sysgroups_reset(SYSGROUPS *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != SYSGROUPS_MAGIC) return SR_NOTOPEN ;

	rs = filemap_rewind(&op->b) ;

	return rs ;
}
/* end subroutine (sysgroups_reset) */


