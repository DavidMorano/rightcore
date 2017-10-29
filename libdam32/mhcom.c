/* mhcom */

/* comment-separate (parse) a mail header field value */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_STRIPESC	1		/* strip escape characters */


/* revision history:

	= 2002-05-29, David A­D­ Morano
        This code was adapted from the EMA (E-Mail Address) code (which does a
        similar function).


*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object separates comment characters from the value characters in a
	mail message header field value.  Comment characters are introduced
	with parentheses.  See RFC-822 for more information on how comments
	operate within header field values.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<char.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"mhcom.h"


/* local defines */

#define	MHCOM_SVALUE	0
#define	MHCOM_SCOMMENT	1
#define	MHCOM_SOVERLAST	2


/* external subroutines */


/* local structures */


/* forward references */

static int mhcom_bake(MHCOM *,int,const char *,int) ;


/* local variables */

#if	CF_DEBUGS
static const char	*statename[] = {
	    "value",
	    "comment",
	    NULL
} ;
#endif /* CF_DEBUGS */


/* exported subroutines */


int mhcom_start(MHCOM *op,cchar sp[],int sl)
{
	int		rs ;
	int		size ;
	int		buflen ;
	void		*p ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("mhcom_start: ent sl=%d s=>%t<\n",sl,sp,sl) ;
#endif /* CF_DEBUGS */

/* skip over any leading white space */

	while ((sl > 0) && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}

/* initialize stuff */

	buflen = (sl + 2) ;

	memset(op,0,sizeof(MHCOM)) ;

	size = (2*buflen) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    op->a = p ;
	    op->value = (op->a + (0*buflen)) ;
	    op->comment = (op->a + (1*buflen)) ;
	    if ((rs = mhcom_bake(op,buflen,sp,sl)) >= 0) {
	        op->magic = MHCOM_MAGIC ;
	    }
	    if (rs < 0) {
	        uc_free(op->a) ;
	        op->a = NULL ;
	    }
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("mhcom_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mhcom_start) */


int mhcom_finish(MHCOM *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MHCOM_MAGIC) return SR_NOTOPEN ;

	if (op->a != NULL) {
	    rs1 = uc_free(op->a) ;
	    if (rs >= 0) rs = rs1 ;
	    op->a = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("mhcom_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (mhcom_finish) */


int mhcom_getval(MHCOM *op,cchar **rpp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MHCOM_MAGIC) return SR_NOTOPEN ;

	if (rpp != NULL)
	    *rpp = op->value ;

	rs = (op->value != NULL) ? op->vlen : SR_NOENT ;

#if	CF_DEBUGS
	debugprintf("mhcom_getval: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mhcom_getval) */


int mhcom_getcom(MHCOM *op,cchar **rpp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MHCOM_MAGIC) return SR_NOTOPEN ;

	if (rpp != NULL)
	    *rpp = op->comment ;

	rs = (op->comment != NULL) ? op->clen : SR_NOENT ;
	return rs ;
}
/* end subroutine (mhcom_getcom) */


/* private subroutines */


static int mhcom_bake(MHCOM *op,int bl,cchar *sp,int sl)
{
	SBUF		as[MHCOM_SOVERLAST] ;
	int		rs ;
	int		rs1 ;
	int		cl, vl ;
	int		pstate = MHCOM_SVALUE ;
	int		state ;
	int		ch ;
	int		c_comment = 0 ;
	int		pc ;
	int		f_quote = FALSE ;

	if ((rs = sbuf_start((as+MHCOM_SVALUE),op->value,bl)) >= 0) {
	    if ((rs = sbuf_start((as+MHCOM_SCOMMENT),op->comment,bl)) >= 0) {

/* start scanning */

	        state = MHCOM_SVALUE ;
	        while ((sl != 0) && (*sp != '\0')) {
#if	CF_DEBUGS
	            vl = sbuf_getlen((as + MHCOM_SVALUE)) ;
	            debugprintf("mhcom_start: value len=%d\n",vl) ;
	            vl = sbuf_getlen((as + MHCOM_SCOMMENT)) ;
	            debugprintf("mhcom_start: comment len=%d\n",vl) ;
#endif
#if	CF_DEBUGS
	            debugprintf("mhcom_start: C='%c' S=%s P=%s cl=%d q=%d\n",
	                *s,statename[state],
	                statename[pstate],c_comment,f_quote) ;
#endif
	            ch = (*sp & 0xff) ;
	            switch (ch) {
	            case '\\':
#if	CF_STRIPESC
	                sp += 1 ;
	                if ((f_quote || c_comment) &&
	                    (sl > 1) && (sp[0] != '\0')) {
	                    sbuf_char((as + state),*sp++) ;
	                    sl -= 1 ;
	                }
#else /* CF_STRIPESC */
	                if ((f_quote || c_comment) &&
	                    (sl > 1) && (sp[1] != '\0')) {
	                    sbuf_char((as + state),*sp++) ;
	                    slen -= 1 ;
	                }
	                sbuf_char((as + state),*sp++) ;
#endif /* CF_STRIPESC */
	                break ;
	            case CH_DQUOTE:
	                f_quote = (! f_quote) ;
	                sbuf_char((as + state),*sp++) ;
	                break ;
	            case CH_LPAREN:
	                if (! f_quote) {
	                    if (c_comment == 0) {
	                        pc = sbuf_getprev(as + state) ;
	                        if ((pc >= 0) && (pc != ' ')) {
	                            sbuf_char((as + state),' ') ;
				}
	                        pstate = state ;
	                        state = MHCOM_SCOMMENT ;
	                        pc = sbuf_getprev(as + state) ;
	                        if ((pc >= 0) && (pc != ' ')) {
	                            sbuf_char((as + state),' ') ;
				}
	                        sp += 1 ;
	                    } else {
	                        sbuf_char((as + state),*sp++) ;
			    }
	                    c_comment += 1 ;
	                } else {
	                    sbuf_char((as + state),*sp++) ;
			}
	                break ;
	            case CH_RPAREN:
	                if ((! f_quote) && (c_comment > 0)) {
	                    c_comment -= 1 ;
	                    if (c_comment == 0) {
	                        state = pstate ;
	                        sp += 1 ;
	                    } else {
	                        sbuf_char((as + state),*sp++) ;
			    }
	                } else {
	                    sbuf_char((as + state),*sp++) ;
			}
	                break ;
/* I think these cases are just optimizations (not required) */
	            case '\t':
	            case ' ':
	                if (! f_quote) {
	                    if (c_comment == 0) {
	                        cl = sbuf_getlen(as + state) ;
	                        pc = sbuf_getprev(as + state) ;
	                        if ((cl == 0) || (pc == ' ')) {
	                            sp += 1 ;
	                            break ;
	                        }
	                    } else {
	                        cl = sbuf_getlen(as + state) ;
	                        pc = sbuf_getprev(as + state) ;
	                        if ((cl == 0) || (pc == ' ')) {
	                            sp += 1 ;
	                            break ;
	                        }
	                    } /* end if */
	                } /* end if (not in a quote) */
/* we fall through to the next case below */
/* FALLTHROUGH */
	            default:
	                if (c_comment) {
	                    sbuf_char((as + MHCOM_SCOMMENT),*sp++) ;
	                } else {
	                    sbuf_char((as + state),*sp++) ;
			}
	                break ;
	            } /* end switch */

	            if (sl > 0) sl -= 1 ;
	        } /* end while (scanning characters) */
#if	CF_DEBUGS
	        debugprintf("mhcom_start: finishing\n") ;
#endif

	        rs1 = sbuf_finish(&as[MHCOM_SCOMMENT]) ;
	        if (rs >= 0) rs = rs1 ;
	        op->clen = rs1 ;
	    } /* end if (sbuf) */

	    vl = sbuf_finish(&as[MHCOM_SVALUE]) ;
	    if (rs >= 0) rs = vl ;
	    op->vlen = vl ;
	} /* end if (sbuf) */

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (mhcom_bake) */


