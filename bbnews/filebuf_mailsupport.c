/* filebuf_mailsupport */

/* mail supprt from the FILEBUF object */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Write a specified number of blanks to a FILEBUF object.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<filebuf.h>
#include	<mailmsghdrfold.h>
#include	<localmisc.h>


/* local defines */

#ifndef	MAILMSGCOLS
#define	MAILMSGCOLS	76
#endif


/* external subroutines */

extern char	*strnchr(cchar *,int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local structures */


/* forward references */

static int filebuf_writehdrval(FILEBUF *,cchar *,int) ;


/* local variables */


/* exported subroutines */


int filebuf_writehdr(FILEBUF *fbp,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		kl ;
	int		vl = -1 ;
	int		wlen = 0 ;
	const char	*tp ;
	const char	*vp = NULL ;

#if	CF_SAFE
	if (sp == NULL)
	    return SR_FAULT ;
#endif

	if (sl < 0)
	    sl = strlen(sp) ;

	kl = sl ;
	if ((tp = strnchr(sp,sl,'=')) != NULL) {
	    kl = (tp - sp) ;
	    vp = (tp+1) ;
	    vl = ((sp+sl)-vp) ;
	}

	if (kl > 0) {
	    if ((rs = filebuf_write(fbp,sp,kl)) >= 0) {
	        wlen += rs ;
	        if ((rs = filebuf_write(fbp,": ",2)) >= 0) {
	            wlen += rs ;
	            if (vl > 0) {
	                rs = filebuf_writehdrval(fbp,vp,vl) ;
	                wlen += rs ;
	            } else {
	                rs = filebuf_print(fbp,sp,0) ;
	                wlen += rs ;
	            }
	        }
	    } /* end if (filebuf_write) */
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_writehdr) */


int filebuf_writehdrkey(FILEBUF *fbp,cchar *kn)
{
	int		rs ;
	int		wlen = 0 ;

	if (fbp == NULL) return SR_FAULT ;
	if (kn == NULL) return SR_FAULT ;

	if (kn[0] == '\0') return SR_INVALID ;

	if ((rs = filebuf_write(fbp,kn,-1)) >= 0) {
	    wlen += rs ;
	    rs = filebuf_write(fbp,": ",2) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_writehdrkey) */


int filebuf_printcont(FILEBUF *fbp,int leader,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (fbp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	if (sl > 0) {
	    char	buf[2] ;

	    if ((rs >= 0) && (leader > 0)) {
	        buf[0] = leader ;
	        buf[1] = '\0' ;
	        rs = filebuf_write(fbp,buf,1) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = filebuf_write(fbp,sp,sl) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        buf[0] = '\n' ;
	        buf[1] = '\0' ;
	        rs = filebuf_write(fbp,buf,1) ;
	        wlen += rs ;
	    }

	} /* end if (non-empty value) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_printcont) */


/* private subroutines */


static int filebuf_writehdrval(FILEBUF *fbp,cchar *vp,int vl)
{
	const int	ln = 0 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

	if (vp != NULL) {
	    MAILMSGHDRFOLD	folder, *fp = &folder ;
	    const int		mcols = MAILMSGCOLS ;
	    int			i = 0 ;
	    int			ll ;
	    const char		*lp ;

	    if (vl < 0) vl = strlen(vp) ;

	    while (vl && CHAR_ISWHITE(*vp)) {
	        vp += 1 ;
	        vl -= 1 ;
	    }

	    if ((rs = mailmsghdrfold_start(fp,mcols,ln,vp,vl)) >= 0) {
	        int	indent = wlen ;

	        while (rs >= 0) {
	            rs = mailmsghdrfold_get(fp,indent,&lp) ;
	            if (rs <= 0) break ;
	            ll = rs ;

	            if ((rs >= 0) && (i > 0)) {
	                rs = filebuf_write(fbp," ",1) ;
	                wlen += rs ;
	            }

	            if (rs >= 0) {
	                rs = filebuf_print(fbp,lp,ll) ;
	                wlen += rs ;
	            }

	            indent = 1 ;
	            i += 1 ;

	        } /* end while */

	        rs1 = mailmsghdrfold_finish(fp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (mailmsghdrfold) */

	} /* end if (non-null) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mailbox_writehdrval) */


