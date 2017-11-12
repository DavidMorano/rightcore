/* svcentsub */

/* build up a server entry piece-meal as it were */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little object takes a regular server entry and substitutes
	values for keys possibly embedded in the string values inside the
	server entry.


*******************************************************************************/


#define	SVCENTSUB_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<varsub.h>
#include	<expcook.h>
#include	<localmisc.h>

#include	"svcentsub.h"
#include	"mfslocinfo.h"
#include	"svcfile.h"


/* local defines */

#ifndef	VBUFLEN
#define	VBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(4 * MAXPATHLEN)
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matostr(cchar **,int,cchar *,int) ;


/* forward references */


/* local subroutines */


/* local variables */

static cchar	*svckeys[] = {
	"so",
	"program",
	"passfile",
	"args",
	"username",
	"groupname",
	"interval",
	"access",
	"opts",
	"failcont",
	NULL
} ;


/* external variables */


/* exported subroutines */


int svcentsub_start(SVCENTSUB *op,LOCINFO *lip,SVCFILE_ENT *sep)
{
	STRPACK		*spp = &op->strs ;
	const int	csize = SVCENTSUB_CSIZE ;
	int		rs ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (lip == NULL) return SR_FAULT ;
	if (sep == NULL) return SR_FAULT ;

	memset(op,0,sizeof(SVCENTSUB)) ;
	if ((rs = strpack_start(spp,csize)) >= 0) {
	    const int	elen = EBUFLEN ;
	    char	*ebuf ;
	    if ((rs = uc_malloc((elen+1),&ebuf)) >= 0) {
	        int	i ;
	        for (i = 0 ; sep->keyvals[i][0] != NULL ; i += 1) {
	            int		ki ;
	            cchar	*kp = sep->keyvals[i][0] ;
	            cchar	*vp = sep->keyvals[i][1] ;
	            if ((ki = matostr(svckeys,1,kp,-1)) >= 0) {
#if	CF_DEBUGS
	                debugprintf("svckey_load: ki=%d\n",ki) ;
	                debugprintf("svckey_load: kp=%s vp=>%s<\n",kp,vp) ;
#endif
	                if ((rs = locinfo_varsub(lip,ebuf,elen,vp,-1)) >= 0) {
	                    cchar	*ep ;
	                    if ((rs = strpack_store(spp,ebuf,rs,&ep)) >= 0) {
	                        op->var[ki] = ep ;
	                    }
	                }
	            } /* end if (matostr) */
	            if (rs < 0) break ;
	        } /* end for */
	        rs1 = uc_free(ebuf) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (m-a-f) */
	    if (rs < 0)
	        strpack_finish(&op->strs) ;
	} /* end if (strpack_start) */

	return rs ;
}
/* end subroutine (svcentsub_start) */


int svcentsub_finish(SVCENTSUB *op)
{
	const int	vsize = (sizeof(cchar *) * svckey_overlast) ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	rs1 = strpack_finish(&op->strs) ;
	if (rs >= 0) rs = rs1 ;

	memset(op->var,0,vsize) ;

	return rs ;
}
/* end subroutine (svcentsub_finish) */


