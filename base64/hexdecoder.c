/* hexdecoder */
/* lang=C++98 */

/* HEX decoding */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2016-06-29, David A­D­ Morano
	This was really made from scratch.

*/

/* Copyright © 2016 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We facilitate HEX (coded input) decoding.

		hexdecoder_start
		hexdecoder_load
		hexdecoder_read
		hexdecoder_finish


*******************************************************************************/


#define	HEXDECODER_MASTER	0	/* necessary for proper symbol names */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<string.h>

#include	<vector>
#include	<new>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"hexdecoder.h"
#include	"obuf.hh"


/* local defines */


/* namespaces */

using namespace		std ;		/* yes, we want punishment! */


/* external subroutines */

extern "C" int	hexval(int) ;
extern "C" int	isprintlatin(int) ;
extern "C" int	ishexlatin(int) ;

#if	CF_DEBUGS
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */

static int	hexdecoder_cvt(HEXDECODER *) ;


/* local variables */


/* exported subroutines */


int hexdecoder_start(HEXDECODER *op)
{
	int		rs = SR_OK ;
	obuf		*obp ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(HEXDECODER)) ;

	if ((obp = new(nothrow) obuf) != NULL) {
	    op->outbuf = (void *) obp ;
	    op->magic = HEXDECODER_MAGIC ;
	} else {
	    rs = SR_NOMEM ;
	}

	return rs ;
}
/* end subroutine (hexdecoder_start) */


int hexdecoder_finish(HEXDECODER *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != HEXDECODER_MAGIC) return SR_NOTOPEN ;

	if (op->outbuf != NULL) {
	    obuf *obp = (obuf *) op->outbuf ;
	    delete obp ;
	    op->outbuf = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (hexdecoder_finish) */


int hexdecoder_load(HEXDECODER *op,cchar *sp,int sl)
{
	obuf		*obp ;
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("hexdecoder_load: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != HEXDECODER_MAGIC) return SR_NOTOPEN ;

	if (sl < 0) sl = strlen(sp) ;

	if ((obp = ((obuf *) op->outbuf)) != NULL) {
	    int	ch ;
	    while ((rs >= 0) && (sl > 0) && *sp) {
		ch = MKCHAR(*sp) ;
		if (ishexlatin(ch)) {
		    if (op->rl == 0) {
			op->rb[0] = ch ;
			op->rl = 1 ;
		    } else {
			op->rb[1] = ch ;
			rs = hexdecoder_cvt(op) ;
			c += rs ;
			op->rl = 0 ;
		    }
		} /* end if (ishexlatin) */
		sp += 1 ;
		sl -= 1 ;
	    } /* end while */
	} else {
	    rs = SR_BUGCHECK ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (hexdecoder_load) */


int hexdecoder_read(HEXDECODER *op,char *rbuf,int rlen)
{
	obuf		*obp ;
	int		rs = SR_OK ;
	int		i = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != HEXDECODER_MAGIC) return SR_NOTOPEN ;

	rbuf[0] = '\0' ;
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

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (hexdecoder_read) */


/* private subroutines */


static int hexdecoder_cvt(HEXDECODER *op)
{
	int		rs = SR_OK ;
	cchar		*rb = op->rb ;
	if (op->outbuf != NULL) {
	    obuf *obp = (obuf *) op->outbuf ;
	    int	ch0 = MKCHAR(rb[0]) ;
	    int	ch1 = MKCHAR(rb[1]) ;
	    int	v = 0 ;
	    v |= (hexval(ch0)<<4) ;
	    v |= (hexval(ch1)<<0) ;
	    obp->add(v) ;
	} else {
	    rs = SR_BUGCHECK ;
	}
	return (rs >= 0) ? 1 : rs ;
}
/* end subroutine (hexdecoder_cvt) */


