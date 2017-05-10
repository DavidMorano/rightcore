/* progdecode */

/* decode a file (encoded in BASE64) */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The subroutine was newly written.


*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This module does the work of decoding the BASE64 input.

	Synopsis:

	int progdecode(pip,ofp,name)
	PROGINFO	*pip ;
	bfile		*ofp ;
	char		name[] ;

	Arguments:

	pip		program information pointer
	name		filename
	ofp		(BIO) output file pointer

	Returns:

	>=		OK
	<0		error


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"b64decoder.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	STATE	struct state


/* external subroutines */

extern int	base64_d(const char *,int,char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */

struct state {
	char		*obuf ;
	int		olen ;
	int		cr ;
} ;


/* forward references */

static int	procline(PROGINFO *,bfile *,B64DECODER *,STATE *,cchar *,int) ;

static int	bwritetext(bfile *,int *,cchar *,int) ;

static int	haseol(cchar *,int) ;


/* local variables */


/* exported subroutines */


int progdecode(PROGINFO *pip,bfile *ofp,cchar name[])
{
	const int	llen = LINEBUFLEN ;
	const int	olen = LINEBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		size = 0 ;
	int		wlen = 0 ;
	char		*abuf ;

	if (name == NULL) return SR_FAULT ;
	if (name[0] == '\0') return SR_INVALID ;

	if (name[0] == '-') name = BFILE_STDIN ;

	size += (llen+1) ;
	size += (olen+1) ;
	if ((rs = uc_malloc(size,&abuf)) >= 0) {
	    B64DECODER	d ;
	    char	*lbuf = (abuf + 0) ;
	    char	*obuf = (abuf + (llen+1)) ;
	    if ((rs = b64decoder_start(&d)) >= 0) {
	        bfile	ifile, *ifp = &ifile ;
	        if ((rs = bopen(ifp,name,"r",0666)) >= 0) {
	    	    STATE	cb ;
	    	    cb.obuf = obuf ;
	    	    cb.olen = olen ;
	    	    cb.cr = 0 ;
	            while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	                int	len = rs ;
	                int	el ;

	                if ((el = haseol(lbuf,len)) > 0) len -= el ;

	                if (len > 0) {
			    rs = procline(pip,ofp,&d,&cb,lbuf,len) ;
			    wlen += rs ;
			} /* end if (positive) */

	                if (rs < 0) break ;
	            } /* end while (reading) */
	            rs1 = bclose(ifp) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (ifile) */
	        rs1 = b64decoder_finish(&d) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (b64decoder) */
	    rs1 = uc_free(abuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (m-a) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progdecode: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progdecode) */


/* local subroutines */


static int procline(pip,ofp,dp,statep,lp,ll)
PROGINFO	*pip ;
bfile		*ofp ;
B64DECODER	*dp ;
STATE		*statep ;
cchar		*lp ;
int		ll ;
{
	int		rs ;
	int		wlen = 0 ;
	if ((rs = b64decoder_load(dp,lp,ll)) > 0) {
	    const int	olen = statep->olen ;
	    int		*crp = &statep->cr ;
	    char	*obuf = statep->obuf ;
	    while ((rs = b64decoder_read(dp,obuf,olen)) > 0) {
		const int	ol = rs ;
		if (pip->f.text) {
		    rs = bwritetext(ofp,crp,obuf,ol) ;
	    	    wlen += rs ;
		} else {
		    rs = bwrite(ofp,obuf,ol) ;
		    wlen += rs ;
		}
		if (rs < 0) break ;
	    } /* end while (reading result) */
	} /* end if (b64decoder_load) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procline) */


/* write out adjusted text */
static int bwritetext(ofp,crp,sp,sl)
bfile		*ofp ;
int		*crp ;
const char	*sp ;
int		sl ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*cp ;

	while ((cp = strnpbrk(sp,sl,"\r\n")) != NULL) {

	    if (*crp && (*sp != '\n')) {
	        *crp = FALSE ;
	        rs = bputc(ofp,'\r') ;
	        wlen += rs ;
	    } /* end if */

	    if ((rs >= 0) && ((cp - sp) > 0)) {
	        rs = bwrite(ofp,sp,(cp - sp)) ;
	        wlen += rs ;
	    } /* end if */

	    if ((rs >= 0) && (*cp == '\n')) {
	        *crp = FALSE ;
	        rs = bputc(ofp,'\n') ;
	        wlen += rs ;
	    } else if (*cp == '\r') {
	        *crp = TRUE ;
	    }

	    sl -= ((cp + 1) - sp) ;
	    sp = (cp + 1) ;

	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (sl > 0)) {

	    if (*crp) {
	        *crp = FALSE ;
	        rs = bputc(ofp,'\r') ;
	        wlen += rs ;
	    } /* end if */

	    if (rs >= 0) {
	        rs = bwrite(ofp,sp,sl) ;
	        wlen += rs ;
	    }

	} /* end if (end processing) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bwritetext) */


static int haseol(cchar *lp,int ll)
{
	int		el = 0 ;
	if ((ll > 0) && (lp[-1] == CH_NL)) {
	    ll -= 1 ;
	    el += 1 ;
	}
	if ((ll > 0) && (lp[-1] == CH_CR)) {
	    ll -= 1 ;
	    el += 1 ;
	}
	return el ;
}
/* end subroutine (haseol) */


