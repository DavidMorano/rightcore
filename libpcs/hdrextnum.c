/* hdrextnum */

/* comment separate (parse) a mail header field value */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2004-05-29, David A­D­ Morano
        This code was adapted from the EMA (E-Mail Address) code (which has code
        that does a similar function).

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine extracts a numeric value from the characters in a mail
        message header field value. This is not meant for use with mail
        addresses (although they certainly need to be comment-separated) but
        rather with shorter fixed-length header field values. This subroutine
        was especially created to extract the numeric value element from the
        CONTENT-LENGTH header field value!


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


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	sibreak(const char *,int,const char *) ;


/* local structures */


/* forward references */

extern int	hdrextnum_ext(char *,const char *,int) ;


/* local variables */


/* exported subroutines */


int hdrextnum(cchar *sp,int sl)
{
	int		rs ;
	int		v = 0 ;
	char		digbuf[DIGBUFLEN + 1] ;

	if (sp == NULL) return SR_FAULT ;

	if ((rs = hdrextnum_ext(digbuf,sp,sl)) > 0) {
	    int		len = rs ;
	    int		si ;
	    if ((si = sibreak(digbuf,len," \t")) >= 0) {
		len = si ;
	    }
	    rs = cfdeci(digbuf,len,&v) ;
	}

	return (rs >= 0) ? v : rs ;
}
/* end subroutine (hdrextnum) */


/* local subroutines */


int hdrextnum_ext(char *digbuf,cchar *sp,int sl)
{
	SBUF		b ;
	int		rs ;
	int		vl = 0 ;

	if (sl < 0)
	    sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("hdrextnum: ent s=>%t<\n",sp,sl) ;
#endif /* CF_DEBUGS */

/* skip over any leading white space */

	while ((sl > 0) && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}

/* initialize for this entry */

	if ((rs = sbuf_start(&b,digbuf,DIGBUFLEN)) >= 0) {
	    int		ch ;
	    int		c_comment = 0 ;
	    int		f_quote = FALSE ;
	    int		f_wslast = FALSE ;
	    int		f_wsnew = FALSE ;
	    int		f_exit = FALSE ;

	    while ((sl >= 0) && (*sp != '\0')) {

	        f_wsnew = FALSE ;
	        ch = (*sp & 0xff) ;
	        switch (ch) {
	        case '\\':
	            if (f_quote) {
	                sp += 1 ;
	                sl -= 1 ;
	                if ((sl > 0) && (sp[0] != '\0')) {
	                    if (c_comment == 0) {
	                        sbuf_char(&b,*sp++) ;
	                        sl -= 1 ;
	                    } else {
	                        sp += 1 ;
	                        sl -= 1 ;
	                    }
	                }
	            } else {
	                if (c_comment == 0) {
	                    sbuf_char(&b,*sp++) ;
	                    sl -= 1 ;
	                } else {
	                    sp += 1 ;
	                    sl -= 1 ;
	                }
	            }
	            break ;
	        case CH_DQUOTE:
	            f_quote = (! f_quote) ;
	            sp += 1 ;
	            sl -= 1 ;
	            break ;
	        case CH_LPAREN:
	            if (! f_quote) {
	                sp += 1 ;
	                sl -= 1 ;
	                c_comment += 1 ;
	            } else {
	                if (c_comment == 0) {
	                    sbuf_char(&b,*sp++) ;
	                    sl -= 1 ;
	                } else {
	                    sp += 1 ;
	                    sl -= 1 ;
	                }
	            }
	            break ;
	        case CH_RPAREN:
	            if (! f_quote) {
	                sp += 1 ;
	                sl -= 1 ;
	                if (c_comment > 0)
	                    c_comment -= 1 ;
	            } else {
	                if (c_comment == 0) {
	                    sbuf_char(&b,*sp++) ;
	                    sl -= 1 ;
	                } else {
	                    sp += 1 ;
	                    sl -= 1 ;
	                }
	            }
	            break ;
	        case CH_COMMA:
	            if (c_comment) {
	                sp += 1 ;
	                sl -= 1 ;
	            } else
	                f_exit = TRUE ;
	            break ;
	        case ' ':
	        case '\t':
	            if ((c_comment == 0) && (! f_wslast)) {
	                sbuf_char(&b,*sp++) ;
	                sl -= 1 ;
	            } else {
	                sp += 1 ;
	                sl -= 1 ;
	            }
	            f_wsnew = TRUE ;
	            break ;
	        default:
	            if (c_comment == 0) {
	                sbuf_char(&b,*sp++) ;
	                sl -= 1 ;
	            } else {
	                sp += 1 ;
	                sl -= 1 ;
	            }
	            break ;
	        } /* end switch */

	        f_wslast = f_wsnew ;
	        if (f_exit) break ;
	    } /* end while (scanning characters) */

	    vl = sbuf_finish(&b) ;
	    if (rs >= 0) rs = vl ;
	} /* end if (sbuf) */

#if	CF_DEBUGS
	debugprintf("hdrextnum_load: ret rs=%d vl=%u\n",rs,vl) ;
	if ((rs >= 0) && (vl >= 0))
	    debugprintf("hdrextnum_load: ret v=>%t<\n",digbuf,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (hdrextnum_ext) */


