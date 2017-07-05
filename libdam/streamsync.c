/* streamsync */

/* stream synchronization operations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_MEMCMP	0		/* use 'memcmp(3c)' */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This obeject is used to acquire synchronization on a data stream.


*******************************************************************************/


#define	STREAMSYNC_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"streamsync.h"


/* local defines */


/* external subroutines */


/* forward references */


/* exported subroutines */


int streamsync_start(STREAMSYNC *ssp,cchar *st,int stlen)
{
	int		rs ;
	int		size ;
	char		*p ;

	if (ssp == NULL) return SR_FAULT ;

	if (stlen < 0)
	    stlen = strlen(st) ;

	if (stlen < 1)
	    return SR_INVALID ;

	memset(ssp,0,sizeof(STREAMSYNC)) ;
	ssp->i = 0 ;
	ssp->stlen = stlen ;

	size = (2 * stlen * sizeof(char)) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {

	ssp->st = p + 0 ;
	ssp->data = p + stlen ;

	memcpy(ssp->st,st,stlen) ;

#if	CF_DEBUGS
	{
		int	i ;
		for (i = 0 ; i < stlen ; i += 1)
		debugprintf("streamsync_start: st=%02x\n",ssp->st[i]) ;
	}
#endif /* CF_DEBUGS */

	memset(ssp->data,0,stlen) ;

	ssp->magic = STREAMSYNC_MAGIC ;

	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (streamsync_start) */


/* free up the entire vector string data structure object */
int streamsync_finish(STREAMSYNC *ssp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ssp == NULL) return SR_FAULT ;

	if (ssp->st != NULL) {
	    rs1 = uc_free(ssp->st) ;
	    if (rs >= 0) rs = rs1 ;
	    ssp->st = NULL ;
	}

	ssp->magic = 0 ;
	return rs ;
}
/* end subroutine (streamsync_finish) */


int streamsync_test(STREAMSYNC *ssp,int ch)
{
	int		i, j ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("streamsync_test: ent ch=%02x\n",ch) ;
#endif

	if (ssp == NULL) return SR_FAULT ;

/* shift the new byte in */

	for (i = 0 ; i < (ssp->stlen - 1) ; i += 1) {
	    ssp->data[i] = ssp->data[i + 1] ;
	}
	ssp->data[i] = ch ;

/* now do the test */

	if (ssp->data[i] == ssp->st[i]) {

#if	CF_DEBUGS
	debugprintf("streamsync_test: one match i=%d\n",i) ;
#endif

#if	CF_MEMCMP
	    f = (memcmp(ssp->st,ssp->data,i) == 0) ;
#else
	    for (j = 0 ; j < i ; j += 1) {

#if	CF_DEBUGS
	debugprintf("streamsync_test: st=%02x data=%02x\n",
		ssp->st[j],ssp->data[j]) ;
#endif

	        if (ssp->st[j] != ssp->data[j])
	            break ;

	    } /* end for */

#if	CF_DEBUGS
	debugprintf("streamsync_test: i=%d j=%d\n",i,j) ;
#endif

	    f = (j >= i) ;

#endif /* CF_MEMCMP */

	} /* end if (test good so far) */

	return f ;
}
/* end subroutine (streamsync_test) */


