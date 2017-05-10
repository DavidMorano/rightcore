/* recipcopyparts */

/* copies parts of a file to another file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug prints */


/* revision history:

	= 1990-07-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine copies part of one file (specified by a container type)
	to another file.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<signal.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"recip.h"
#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int recipcopyparts(RECIP *rp,int tfd,int sfd)
{
	int		rs = SR_OK ;
	int		i ;
	int		off = 0 ;
	int		moff ;
	int		mlen ;
	int		clen = 0 ;
	int		tlen = 0 ;
	int		f_disjoint = FALSE ;

#if	CF_DEBUGS
	debugprintf("recipcopyparts: ent\n") ;
#endif

/* first, figure out if we can do a contiguous copy or not */

	for (i = 0 ; (mlen = recip_getmo(rp,i,&moff)) >= 0 ; i += 1) {

#if	CF_DEBUGS
	debugprintf("recipcopyparts: moff=%u mlen=%u\n",moff,mlen) ;
#endif

	    f_disjoint = (off != moff) ;
	    if (f_disjoint) break ;

	    off += mlen ;
	    clen += mlen ;

	} /* end for */

#if	CF_DEBUGS
	debugprintf("recipcopyparts: f_disjoint=%u\n",f_disjoint) ;
#endif

/* OK, do the copying according to whether contiguous or not */

	if (f_disjoint) {
	    int		len = 0 ;
	    off = 0 ;
	    for (i = 0 ; (mlen = recip_getmo(rp,i,&moff)) >= 0 ; i += 1) {

#if	CF_DEBUGS
		debugprintf("recipcopyparts: moff=%u mlen=%u\n",
			moff,mlen) ;
#endif

	        if (mlen > 0) {

	            if (moff != off) {
			offset_t	uoff = moff ;
	                rs = u_seek(tfd,uoff,SEEK_SET) ;

#if	CF_DEBUGS
	                debugprintf("recipcopyparts: u_seek() rs=%d\n",rs) ;
#endif

	            }

	            if (rs >= 0) {
	                if ((rs = uc_writedesc(sfd,tfd,mlen)) > 0) {
			    len = rs ;
	                    off += len ;
	                    tlen += len ;
			}
		    }

	        } /* end if */

	        if (rs < 0) break ;
	    } /* end for */

	} else {

#if	CF_DEBUGS
	    debugprintf("recipcopyparts: copy i=%d clen=%d\n",i,clen) ;
#endif

	    if (i > 0) {
	        rs = uc_writedesc(sfd,tfd,clen) ;
	        tlen = rs ;
	    }

	} /* end if */

#if	CF_DEBUGS
	debugprintf("recipcopyparts: ret rs=%d tlen=%d\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (recipcopyparts) */


