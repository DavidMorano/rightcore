/* b64decoder */

/* object to decode data (encoded in BASE64) */


#define	CF_DEBUGS	1		/* non-switchable debug print-outs */
#define	CF_OPTIONAL	0		/* ?? */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that did similar types
	of functions.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This obejct module does the work of decoding BASE64 data.

	The main subroutine to call for processing is 'b64decoder_process()'.
	Once the input buffer is setup for that call, the user should loop
	processing the output provided on each call.  The output buffer is
	supplied by the user at object initialization.  Once there is no
	further output (zero is returned), a new call with new input is
	possible.


*******************************************************************************/


#define	B64DECODER_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>

#include	"b64decoder.h"


/* local defines */


/* external subroutines */

extern int	base64_d(const char *,int,char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */

static int	nextgroup(const char *,int,const char **) ;
static int	nextchar(B64DECODER_BUF *) ;


/* local variables */


/* exported subroutines */


int b64decoder_start(B64DECODER *op,char *rbuf,int rlen)
{

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (rlen < 4) return SR_INVALID ;

	memset(op,0,sizeof(B64DECODER)) ;
	op->obuf = rbuf ;
	op->olen = rlen ;

#if	CF_DEBUGS
	debugprintf("b64decoder_process: ret \n") ;
#endif

	return SR_OK ;
}
/* end subroutine (b64decoder_start) */


int b64decoder_process(B64DECODER *op,B64DECODER_BUF *ibp,cchar **rpp)
{
	int		rs = SR_OK ;
	int		i ;
	int		c ;
	int		sl ;
	const char	*sp ;

#if	CF_DEBUGS
	debugprintf("b64decoder_process: ent i_bp{%p} i_bl=%d\n",
	    ibp->bp,ibp->bl) ;
#endif

#if	CF_OPTIONAL

	if ((op->olen - op->bl) < 3) {

#if	CF_DEBUGS
	    debugprintf("b64decoder_process: output full\n") ;
#endif

	    if (rpp != NULL) {
	        *rpp = (op->obuf + op->bl) ;
	    }
	    rs = op->bl ;
	    op->bl = 0 ;
	    goto ret0 ;
	}

	if (op->sl > 0) {

#if	CF_DEBUGS
	    debugprintf("b64decoder_process: 1 loading stage=>%t< sl=%d\n",
	        op->stage,op->sl,op->sl) ;
#endif

	    while (op->sl < 4) {
	        c = nextchar(ibp) ;
	        if (c <= 0) break ;
	        op->stage[op->sl++] = c ;
	    } /* end while */

#if	CF_DEBUGS
	    debugprintf("b64decoder_process: 1 stage=>%t<\n",op->stage,op->sl) ;
#endif

	    if (op->sl == 4) {

#if	CF_DEBUGS
	        debugprintf("b64decoder_process: 0 sl == 4\n") ;
	        debugprintf("b64decoder_process: stage=>%t<\n",op->stage,4) ;
#endif

	        rs = base64_d(op->stage,4,op->obuf + op->bl) ;

#if	CF_DEBUGS
	        debugprintf("b64decoder_process: 0 base64_d() rs=%d\n",rs) ;
	        debugprintf("b64decoder_process: out=>%t<\n",
	            (op->obuf + op->bl),rs) ;
#endif

	        if (rs < 0) goto ret0 ;

	        op->bl += rs ;
	        op->sl = 0 ;

	    }

	} /* end if (had some staged already) */

	if ((op->olen - op->bl) < 3) {

#if	CF_DEBUGS
	    debugprintf("b64decoder_process: output full 2\n") ;
#endif

	    if (rpp != NULL) {
	        *rpp = (op->obuf + op->bl) ;
	    }
	    rs = op->bl ;
	    op->bl = 0 ;
	    goto ret0 ;
	}

#endif /* CF_OPTIONAL */

	while ((rs >= 0) && ((op->olen - op->bl) >= 3) && (ibp->bl > 0)) {
	    int		ml ;
	    int		ndg ;
	    const char	*mp ;

#if	CF_DEBUGS
	    debugprintf("b64decoder_process: W-T i_bp=%08x i_bl=%d\n",
		ibp->bp,ibp->bl) ;
#endif

	    if (op->sl > 0) {

#if	CF_DEBUGS
	        debugprintf("b64decoder_process: 2 loading stage=>%t< sl=%d\n",
	            op->stage,op->sl,op->sl) ;
#endif

	        while (op->sl < 4) {
	            c = nextchar(ibp) ;
	            if (c <= 0) break ;
	            op->stage[op->sl++] = c ;
	        } /* end while */

#if	CF_DEBUGS
	        debugprintf("b64decoder_process: 2 stage=>%t<\n",
		op->stage,op->sl) ;
#endif

	        if (op->sl == 4) {

#if	CF_DEBUGS
	            debugprintf("b64decoder_process: 1 sl == 4\n") ;
	            debugprintf("b64decoder_process: stage=>%t<\n",
			op->stage,4) ;
#endif

	            rs = base64_d(op->stage,4,op->obuf + op->bl) ;

#if	CF_DEBUGS
	            debugprintf("b64decoder_process: 1 base64_d() rs=%d\n",rs) ;
	            debugprintf("b64decoder_process: out=>%t<\n",
	                (op->obuf + op->bl),rs) ;
#endif

	            if (rs < 0) break ;

	            op->bl += rs ;
	            op->sl = 0 ;

	        } /* end if */

	    } /* end if (had some staged already) */

#if	CF_DEBUGS
	    debugprintf("b64decoder_process: i_bp=%08x i_bl=%d\n",
	        ibp->bp,ibp->bl) ;
	    debugprintf("b64decoder_process: nextgroup in=>%t<\n",
	        ibp->bp,ibp->bl) ;
#endif

	    sl = nextgroup(ibp->bp,ibp->bl,&sp) ;

	    mp = (sl > 0) ? sp : (ibp->bp + ibp->bl) ;
	    ml = 0 ;

#if	CF_DEBUGS
	    debugprintf("b64decoder_process: nextgroup() sl=%d\n",sl) ;
	    debugprintf("b64decoder_process: sp=>%t<\n",sp,sl) ;
#endif

	    if (sl >= 4) {

	        ndg = MIN((sl / 4),((op->olen - op->bl) / 3)) ;

#if	CF_DEBUGS
	        debugprintf("b64decoder_process: ndg=%d\n",ndg) ;
	        debugprintf("b64decoder_process: 2 inlen=%d\n",(ndg * 4)) ;
	        debugprintf("b64decoder_process: in=>%t<\n",sp,(ndg * 4)) ;
#endif

	        rs = base64_d(sp,(ndg * 4),op->obuf + op->bl) ;

#if	CF_DEBUGS
	        debugprintf("b64decoder_process: 2 base64_d() rs=%d\n",rs) ;
	        debugprintf("b64decoder_process: out=>%t<\n",
	            (op->obuf + op->bl),rs) ;
#endif

	        if (rs < 0) break ;

	        op->bl += rs ;
	        ml += (ndg * 4) ;

	        sp += (ndg * 4) ;
	        sl -= (ndg * 4) ;

	    } /* end if (greater or equal to four input characters) */

	    if (sl > 0) {

#if	CF_DEBUGS
	        debugprintf("b64decoder_process: 3 loading stage=>%t< sl=%d\n",
	            op->stage,op->sl,op->sl) ;
#endif

	        for (i = 0 ; i < sl ; i += 1) {
	            op->stage[op->sl++] = *sp++ ;
		}
	        ml += sl ;

#if	CF_DEBUGS
	        debugprintf("b64decoder_process: 3 stage=>%t<\n",
		op->stage,op->sl) ;
#endif

	    } /* end if (some extra remaining) */

	    ibp->bl -= (mp + ml - ibp->bp) ;
	    ibp->bp = mp + ml ;

#if	CF_DEBUGS
	    debugprintf("b64decoder_process: while-bot i_bp=%08x i_bl=%d\n",
	        ibp->bp,ibp->bl) ;
	    debugprintf("b64decoder_process: W-B\n") ;
#endif

	} /* end while (looping over contiguous areas) */

#if	CF_DEBUGS
	debugprintf("b64decoder_process: while-out, o_bl=%d i_bl=%d\n",
	    op->bl,ibp->bl) ;
	debugprintf("b64decoder_process: o_bl=%d decoded=>%t<\n",
	    op->bl,op->obuf,op->bl) ;
#endif

/* if we have no output, load any remnant input into the stage buffer */

	if ((rs >= 0) && (op->bl == 0) && (ibp->bl > 0)) {

#if	CF_DEBUGS
	    debugprintf("b64decoder_process: 4 loading stage=>%t< sl=%d\n",
	        op->stage,op->sl,op->sl) ;
#endif

	    while (op->sl < 4) {
	        c = nextchar(ibp) ;
	        if (c <= 0) break ;
	        op->stage[op->sl++] = c ;
	    } /* end while */

#if	CF_DEBUGS
	    debugprintf("b64decoder_process: 4 stage=>%t<\n",op->stage,op->sl) ;
#endif

	} /* end if (extra input needed to be saved) */

#if	CF_DEBUGS
	debugprintf("b64decoder_process: returning o_bl=%d decoded=>%t<\n",
	    op->bl,op->obuf,op->bl) ;
#endif

	if (rs >= 0) {
		if (rpp != NULL) *rpp = op->obuf ;
		rs = op->bl ;
		op->bl = 0 ;
	}

	return rs ;
}
/* end subroutine (b64decoder_process) */


int b64decoder_finish(B64DECODER *op)
{
	if (op == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (b64decoder_finish) */


/* private subroutines */


/* get the next non-white-space character from the input buffer */
static int nextchar(B64DECODER_BUF *ibp)
{
	int		c = 0 ;

	while (ibp->bl && (ibp->bp[0] != '\0') && CHAR_ISWHITE(*ibp->bp)) {
	    ibp->bp += 1 ;
	    ibp->bl -= 1 ;
	} /* end while */

	if (ibp->bl > 0) {
	    c = *ibp->bp++ ;
	    ibp->bl -= 1 ;
	}

#if	CF_DEBUGS
	debugprintf("nextchar: c=%c\n",((c == 0) ? ' ' : c)) ;
#endif

	return c ;
}
/* end subroutine (nextchar) */


int nextgroup(cchar *sp,int sl,cchar **spp)
{
	int		f_len = (sl >= 0) ;

/* skip leading white space */

	while (((! f_len) || (sl > 0)) && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	*spp = sp ;

/* skip the non-white space */

	while ((((! f_len) && *sp) || (sl > 0)) && (! CHAR_ISWHITE(*sp))) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	return (sp - (*spp)) ;
}
/* end subroutine (nextgroup) */


