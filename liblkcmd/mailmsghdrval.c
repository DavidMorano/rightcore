/* mailmsghdrval */

/* message header-value handling */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2002-01-10, David A­D­ Morano
	This was written to support the MAILBOX object.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a *simple* message header-value object.  


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"mailmsghdrval.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN		MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN		2048
#endif
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	mailmsgsmathead(const char *,int,int *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* local (forward) subroutines */

static int mailmsghdrval_loadadd(MAILMSGHDRVAL *,const char *,int) ;


/* local variables */


/* exported subroutines */


int mailmsghdrval_start(MAILMSGHDRVAL *mbp,int i,cchar *hp,int hl)
{
	int		rs ;

	if (mbp == NULL) return SR_FAULT ;

	mbp->v = NULL ;
	mbp->vlen = 0 ;
	mbp->vbuf[0] = '\0' ;
	mbp->i = i ;
	rs = mailmsghdrval_loadadd(mbp,hp,hl) ;

	return rs ;
}
/* end subroutine (mailmsghdrval_start) */


int mailmsghdrval_finish(MAILMSGHDRVAL *mbp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mbp == NULL) return SR_FAULT ;

	mbp->vbuf[0] = '\0' ;
	if (mbp->v != NULL) {
	    rs1 = uc_free(mbp->v) ;
	    if (rs >= 0) rs = rs1 ;
	    mbp->v = NULL ;
	}

	return rs ;
}
/* end subroutine (mailmsghdrval_finish) */


int mailmsghdrval_add(MAILMSGHDRVAL *mbp,cchar *hp,int hl)
{
	int		rs ;

	if (mbp == NULL) return SR_FAULT ;

	rs = mailmsghdrval_loadadd(mbp,hp,hl) ;

	return rs ;
}
/* end subroutine (mailmsghdrval_add) */


int mailmsghdrval_get(MAILMSGHDRVAL *mbp,cchar **vpp,int *vlp)
{
	const char	*vp = NULL ;

	if (mbp == NULL) return SR_FAULT ;

	if (mbp->v != NULL) {
	    vp = mbp->v ;
	} else if (mbp->vbuf[0] != '\0') {
	    vp = mbp->vbuf ;
	}

	if (vpp != NULL) *vpp = vp ;

	if (vlp != NULL) *vlp = mbp->vlen ;

	return mbp->i ;
}
/* end subroutine (mailmsghdrval_get) */


/* private subroutines */


static int mailmsghdrval_loadadd(MAILMSGHDRVAL *mbp,cchar *hp,int hl)
{
	int		rs = SR_OK ;
	int		sl ;
	const char	*sp ;

	if ((sl = sfshrink(hp,hl,&sp)) > 0) {
	    int		size ;
	    int		al ;
	    char	*nvp ;
	    char	*vp ;

	    al = (mbp->vlen > 0) ? (mbp->vlen + (sl + 1)) : sl ;
	    if (mbp->v == NULL) {
	        int	lenr = (MAILMSGHDRVAL_BUFLEN - mbp->vlen) ;

	        if (al <= lenr) {

	            vp = (mbp->vbuf + mbp->vlen) ;
	            if (mbp->vlen > 0)
	                *vp++ = ' ' ;

	            strwcpy(vp,sp,sl) ;
	            mbp->vlen += al ;

	        } else {

	            size = mbp->vlen + al + 1 ;
	            if ((rs = uc_malloc(size,&nvp)) >= 0) {

	                vp = nvp ;
	                if (mbp->vlen > 0) {
	                    vp = strwcpy(vp,mbp->vbuf,mbp->vlen) ;
	                    *vp++ = ' ' ;
	                }

	                strwcpy(vp,sp,sl) ;

	                mbp->vbuf[0] = '\0' ;
	                mbp->v = nvp ;
	                mbp->vlen += al ;

	            } /* end if */

	        } /* end if */

	    } else {

	        size = mbp->vlen + al + 1 ;
	        if ((rs = uc_realloc(mbp->v,size,&nvp)) >= 0) {

	            vp = (nvp + mbp->vlen) ;
	            *vp++ = ' ' ;
	            strwcpy(vp,sp,sl) ;

	            mbp->v = nvp ;
	            mbp->vlen += al ;

	        } /* end if */

	    } /* end if (dynamic allocation) */

	} /* end if (sfshrink) */

	return (rs >= 0) ? mbp->vlen : rs ;
}
/* end subroutine (mailmsghdrval_loadadd) */


