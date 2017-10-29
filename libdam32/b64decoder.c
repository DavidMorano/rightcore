/* b64decoder */
/* lang=C++98 */

/* Base-64 (B64) decoding */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2016-06-29, David A­D­ Morano
	This was really made from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We facilitate Base-64 (B64) (coded input) decoding.

		b64decoder_start
		b64decoder_load
		b64decoder_read
		b64decoder_finish


*******************************************************************************/


#define	B64DECODER_MASTER	0	/* necessary for proper symbol names */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<string.h>

#include	<vector>
#include	<new>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"b64decoder.h"
#include	"base64.h"
#include	"obuf.hh"


/* local defines */


/* namespaces */

using namespace		std ;		/* yes, we want punishment! */


/* external subroutines */

extern "C" int	nextfield(cchar *,int,cchar **) ;
extern "C" int	ifloor(int,int) ;

#if	CF_DEBUGS
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif

extern "C" char	*strwcpy(char *,cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	b64decoder_cvt(B64DECODER *,cchar *,int) ;


/* local variables */


/* exported subroutines */


int b64decoder_start(B64DECODER *op)
{
	int		rs = SR_OK ;
	obuf		*obp ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(B64DECODER)) ;

	if ((obp = new(nothrow) obuf) != NULL) {
	    op->outbuf = (void *) obp ;
	    op->magic = B64DECODER_MAGIC ;
	} else {
	    rs = SR_NOMEM ;
	}

	return rs ;
}
/* end subroutine (b64decoder_start) */


int b64decoder_finish(B64DECODER *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != B64DECODER_MAGIC) return SR_NOTOPEN ;

	if (op->outbuf != NULL) {
	    obuf *obp = (obuf *) op->outbuf ;
	    delete obp ;
	    op->outbuf = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("b64decoder_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (b64decoder_finish) */


int b64decoder_load(B64DECODER *op,cchar *sp,int sl)
{
	obuf		*obp ;
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("b64decoder_load: ent sl=%d\n",sl) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != B64DECODER_MAGIC) return SR_NOTOPEN ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("b64decoder_load: s=>%t<\n",
	    sp,strlinelen(sp,sl,50)) ;
#endif

	if ((obp = ((obuf *) op->outbuf)) != NULL) {
	    int		cl ;
	    cchar	*cp ;
	    while ((cl = nextfield(sp,sl,&cp)) > 0) {
#if	CF_DEBUGS
		debugprintf("b64decoder_load: chunk=>%t<\n",cp,cl) ;
#endif
	        sl -= ((cp+cl)-sp) ;
	        sp = (cp+cl) ;
	        if (op->rl > 0) {
	            int		ml = MIN(cl,(4-op->rl)) ;
	            const int	rl = op->rl ;
	            char	*rb = op->rb ;
#if	CF_DEBUGS
		    debugprintf("b64decoder_load: existing ml=%d r=>%t<\n",
			ml,rb,rl) ;
#endif
	            strwcpy((rb+rl),cp,ml) ;
	            op->rl += ml ;
	            cp += ml ;
	            cl -= ml ;
	            if (op->rl == 4) {
#if	CF_DEBUGS
			debugprintf("b64decoder_load: cvt one rc=>%t<\n",
			op->rb,4) ;
#endif
	                rs = b64decoder_cvt(op,op->rb,4) ;
	                c += rs ;
	                op->rl = 0 ;
	            }
	        } /* end if (positive residue) */
	        while ((rs >= 0) && (cl >= 4)) {
		    const int	ml = ifloor(cl,4) ;
#if	CF_DEBUGS
		    debugprintf("b64decoder_load: cvt ml=%d c=>%t<\n",
			ml,sp,ml) ;
#endif
	            rs = b64decoder_cvt(op,cp,ml) ;
	            c += rs ;
	            cp += ml ;
	            cl -= ml ;
	            if (rs < 0) break ;
	        } /* end while */
	        if ((rs >= 0) && (cl > 0)) {
#if	CF_DEBUGS
	debugprintf("b64decoder_load: storing c=>%t<\n",cp,cl) ;
#endif
	            strwcpy(op->rb,cp,cl) ;
	            op->rl = cl ;
	        } /* end if (remaining source) */
	        if (rs < 0) break ;
	    } /* end while (nextfield) */
	} else {
	    rs = SR_BUGCHECK ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (b64decoder_load) */


int b64decoder_read(B64DECODER *op,char *rbuf,int rlen)
{
	obuf		*obp ;
	int		rs = SR_OK ;
	int		i = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != B64DECODER_MAGIC) return SR_NOTOPEN ;

	if (rlen < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("b64decoder_read: ent rlen=%d\n",rlen) ;
#endif

	rbuf[0] = '\0' ;
	if (rlen > 0) {
	    if ((obp = ((obuf *) op->outbuf)) != NULL) {
	        const int	len = obp->len() ;
	        int		ml ;
	        ml = MIN(len,rlen) ;
	        for (i = 0 ; i < ml ; i += 1) {
		    rbuf[i] = obp->at(i) ;
	        }
	        rbuf[i] = '\0' ;
	        rs = obp->adv(i) ;
	    } else {
	        rs = SR_BUGCHECK ;
	    }
	} /* end if (positive) */

#if	CF_DEBUGS
	debugprintf("b64decoder_read: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (b64decoder_read) */


/* private subroutines */


static int b64decoder_cvt(B64DECODER *op,cchar *cp,int cl)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (op->outbuf != NULL) {
	    obuf 	*obp = (obuf *) op->outbuf ;
	    char	*rbuf ;
	    if ((rbuf = new(nothrow) char [cl+1]) != NULL) {
	        if ((c = base64_d(cp,cl,rbuf)) > 0) {
	            rbuf[c] = '\0' ;
	            obp->add(rbuf,c) ;
	        } else {
	            rs = SR_ILSEQ ;
		}
	        delete [] rbuf ;
	    } else { /* memory allocation */
	        rs = SR_NOMEM ;
	    }
	} else {
	    rs = SR_BUGCHECK ;
	}
#if	CF_DEBUGS
	debugprintf("b64decoder_cvt: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (b64decoder_cvt) */


