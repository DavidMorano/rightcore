/* sysgroup */

/* system group-entry enumeration */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-24, David A­D­ Morano
        This object module was morphed from some previous one. I do not remember
        what the previous one was.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We enumerate (reentrantly and thread safely) group entries from the
	system GROUP database.


*******************************************************************************/


#define	SYSGROUP_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<groupent.h>
#include	<localmisc.h>

#include	"sysgroup.h"


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


int sysgroup_open(SYSGROUP *op,const char *sgfname)
{
	const size_t	max = INT_MAX ;
	int		rs ;
	const char	*defgfname = SYSGROUP_FNAME ;

	if (op == NULL) return SR_FAULT ;

	if (sgfname == NULL) sgfname = defgfname ; /* default */

	memset(op,0,sizeof(SYSGROUP)) ;

	if ((rs = filemap_open(&op->b,sgfname,O_RDONLY,max)) >= 0) {
	    op->magic = SYSGROUP_MAGIC ;
	}

	return rs ;
}
/* end if (sysgroup_open) */


int sysgroup_close(SYSGROUP *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != SYSGROUP_MAGIC) return SR_NOTOPEN ;

	rs1 = filemap_close(&op->b) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
} 
/* end subroutine (sysgroup_close) */


int sysgroup_readent(SYSGROUP *op,struct group *grp,char grbuf[],int grlen)
{
	int		rs ;
	int		ll ;
	const char	*lp ;

	if (op == NULL) return SR_FAULT ;
	if (grp == NULL) return SR_FAULT ;
	if (grbuf == NULL) return SR_FAULT ;
	if (op->magic != SYSGROUP_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("sysgroup_readent: ent\n") ;
#endif

	while ((rs = filemap_getline(&op->b,&lp)) > 0) {
	    ll = rs ;
	    if (lp[ll-1] == '\n') ll -= 1 ;
	    rs = groupent_parse(grp,grbuf,grlen,lp,ll) ;
#if	CF_DEBUGS
	    debugprintf("sysgroup_readent: groupent_parse() rs=%d\n",rs) ;
	    debugprintf("sysgroup_readent: gn=%s\n",grp->gr_name) ;
#endif
	    if (rs != 0) break ;
	} /* end while */

#if	CF_DEBUGS
	{
	    int		i ;
	    cchar	**groups = (const char **) grp->gr_mem ;
	    if (groups != NULL) {
		for (i = 0 ; groups[i] != NULL ; i += 1) {
		    debugprintf("sysgroup_readent: gm=%s\n",groups[i]) ;
		}
	    }
	    debugprintf("sysgroup_readent: ret rs=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (sysgroup_readent) */


int sysgroup_reset(SYSGROUP *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != SYSGROUP_MAGIC) return SR_NOTOPEN ;

	rs = filemap_rewind(&op->b) ;

	return rs ;
}
/* end subroutine (sysgroup_reset) */


