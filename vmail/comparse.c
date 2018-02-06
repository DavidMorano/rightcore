/* comparse */

/* comment-separate (parse) a mail header field value */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2004-05-29, David A­D­ Morano
	This code was adapted from the EMA (E-Mail Address) code (which has
	code that does a similar function).

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine separates comment characters from the value characters
	in a mail message header field value.  This is not meant for use with
	mail addresses (although they certainly need to be comment-separated)
	but rather with shorter fixed-length header field values.  This
	subroutine was especially created to separate out the comments from the
	DATE header field value!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<buffer.h>
#include	<char.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"comparse.h"


/* local defines */

#define	COMPARSE_DEFSIZE	80
#define	COMPARSE_SVALUE		0
#define	COMPARSE_SCOMMENT	1
#define	COMPARSE_SOVERLAST	2


/* external subroutines */


/* forward references */

int	comparse_bake(COMPARSE *,const char *,int) ;


/* local variables */

#if	CF_DEBUGS
static const char	*statename[] = {
	"V",
	"C",
	NULL
} ;
#endif /* CF_DEBUGS */


/* exported subroutines */


int comparse_start(COMPARSE *cpp,cchar *sp,int sl)
{
	int		rs ;
	int		vl = 0 ;

	if (cpp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	memset(cpp,0,sizeof(COMPARSE)) ;

#if	CF_DEBUGS
	debugprintf("comparse_start: ent s=>%t<\n",sp,sl) ;
#endif /* CF_DEBUGS */

/* skip over any leading white space */

	while ((sl > 0) && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}

/* initialize for this entry */

	if ((rs = comparse_bake(cpp,sp,sl)) >= 0) {
	    vl = rs ;
	    cpp->magic = COMPARSE_MAGIC ;
	}

#if	CF_DEBUGS
	debugprintf("comparse_start: val=>%s<\n",cpp->val) ;
	debugprintf("comparse_start: com=>%s<\n",cpp->com) ;
	debugprintf("comparse_start: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (comparse_start) */


int comparse_finish(COMPARSE *cpp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (cpp == NULL) return SR_FAULT ;
	if (cpp->magic != COMPARSE_MAGIC) return SR_NOTOPEN ;

	if (cpp->val.sp != NULL) {
	    rs1 = uc_free(cpp->val.sp) ;
	    if (rs >= 0) rs = rs1 ;
	    cpp->val.sp = NULL ;
	}

	if (cpp->com.sp != NULL) {
	    rs1 = uc_free(cpp->com.sp) ;
	    if (rs >= 0) rs = rs1 ;
	    cpp->com.sp = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("comparse_finish: ret\n") ;
#endif

	cpp->magic = 0 ;
	return rs ;
}
/* end subroutine (comparse_finish) */


int comparse_getval(COMPARSE *cpp,cchar **rpp)
{
	int		rs = 0 ;

	if (cpp == NULL) return SR_FAULT ;
	if (cpp->magic != COMPARSE_MAGIC) return SR_NOTOPEN ;

	if (rpp != NULL)
	    *rpp = cpp->val.sp ;

	if (cpp->val.sp != NULL) rs = cpp->val.sl ;

#if	CF_DEBUGS
	debugprintf("comparse_getval: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (comparse_getval) */


int comparse_getcom(COMPARSE *cpp,cchar **rpp)
{
	int		rs = 0 ;

	if (cpp == NULL) return SR_FAULT ;
	if (cpp->magic != COMPARSE_MAGIC) return SR_NOTOPEN ;

	if (rpp != NULL)
	    *rpp = cpp->com.sp ;

	if (cpp->com.sp != NULL) rs = cpp->com.sl ;

	return rs ;
}
/* end subroutine (comparse_getcom) */


/* private subroutines */


int comparse_bake(COMPARSE *cpp,cchar *sp,int sl)
{
	BUFFER		as[COMPARSE_SOVERLAST] ;
	const int	defsize = COMPARSE_DEFSIZE ;
	int		rs ;
	int		rs1 ;
	int		cl ;
	int		pc ;
	int		ch ;
	int		vl = 0 ;
	int		f_quote = FALSE ;

#if	CF_DEBUGS
	debugprintf("comparse_bake: ent s=>%t<\n",sp,sl) ;
#endif /* CF_DEBUGS */

/* skip over any leading white space */

	while ((sl > 0) && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}

/* initialize for this entry */

	if ((rs = buffer_start(&as[0],defsize)) >= 0) {

	    if ((rs = buffer_start(&as[1],defsize)) >= 0) {
	        int	pstate = COMPARSE_SVALUE ;
	        int	state ;
	        int	c_comment = 0 ;

/* start scanning */

	        state = COMPARSE_SVALUE ;
	        while ((sl != 0) && (*sp != '\0')) {

#if	CF_DEBUGS
	            debugprintf("comparse_start: C='%c' S=%s P=%s cl=%d q=%d\n",
	                sp[0],statename[state],
	                statename[pstate],c_comment,f_quote) ;
#endif

	            ch = (*sp & 0xff) ;
	            switch (ch) {

	            case '\\':
	                if (f_quote) {
	                    sp += 1 ;
	                    sl -= 1 ;
	                    if ((sl > 0) && (sp[0] != '\0')) {
	                        buffer_char((as + state),*sp++) ;
	                        sl -= 1 ;
	                    }
	                } else {
	                    buffer_char((as + state),*sp++) ;
	                    sl -= 1 ;
	                }
	                break ;

	            case CH_DQUOTE:
	                f_quote = (! f_quote) ;
	                sp += 1 ;
	                sl -= 1 ;
	                break ;

	            case CH_LPAREN:
	                if (! f_quote) {
	                    if (c_comment == 0) {
	                        pc = buffer_getprev(as + state) ;
	                        if ((pc >= 0) && (! CHAR_ISWHITE(pc)))
	                            buffer_char((as + state),' ') ;
	                        pstate = state ;
	                        state = COMPARSE_SCOMMENT ;
	                        pc = buffer_getprev(as + state) ;
	                        if ((pc >= 0) && (! CHAR_ISWHITE(pc)))
	                            buffer_char((as + state),' ') ;
	                        sp += 1 ;
	                        sl -= 1 ;
	                    } else {
	                        buffer_char((as + state),*sp++) ;
	                        sl -= 1 ;
	                    }
	                    c_comment += 1 ;
	                } else {
	                    buffer_char((as + state),*sp++) ;
	                    sl -= 1 ;
	                }
	                break ;

	            case CH_RPAREN:
	                if ((! f_quote) && (c_comment > 0)) {
	                    c_comment -= 1 ;
	                    if (c_comment == 0) {
	                        state = pstate ;
	                        sp += 1 ;
	                        sl -= 1 ;
	                    } else {
	                        buffer_char((as + state),*sp++) ;
	                        sl -= 1 ;
	                    }
	                } else {
	                    buffer_char((as + state),*sp++) ;
	                    sl -= 1 ;
	                }
	                break ;

	            case ' ':
	            case '\t':
	                if (! f_quote) {
	                    cl = buffer_get((as+state),NULL) ;
	                    pc = buffer_getprev(as+state) ;
	                    if ((cl == 0) || ((pc >= 0) && CHAR_ISWHITE(pc))) {
	                        sp += 1 ;
	                        sl -= 1 ;
	                        break ;
	                    }
	                } /* end if (not in a quote) */

/* FALLTHROUGH */
	            default:
	                if (c_comment > 0) {
	                    buffer_char((as + COMPARSE_SCOMMENT),*sp++) ;
	                } else {
	                    buffer_char((as + state),*sp++) ;
			}
	                sl -= 1 ;
	                break ;

	            } /* end switch */

	        } /* end while (scanning characters) */

#if	CF_DEBUGS
	        debugprintf("comparse_start: finishing\n") ;
#endif

	        if (rs >= 0) {
	            const char	*cp ;
	            const char	*bp ;
	            int		bl ;
	            int		w = COMPARSE_SCOMMENT ;
	            if ((rs = buffer_get((as+w),&bp)) >= 0) {
	                bl = rs ;
	                while (bl && CHAR_ISWHITE(bp[bl-1])) bl -= 1 ;
	                if ((rs = uc_mallocstrw(bp,bl,&cp)) >= 0) {
	                    cpp->com.sp = cp ;
	                    cpp->com.sl = bl ;
	                    w = COMPARSE_SVALUE ;
	                    if ((rs = buffer_get((as+w),&bp)) >= 0) {
	                        bl = rs ;
	                        while (bl && CHAR_ISWHITE(bp[bl-1])) bl -= 1 ;
	                        if ((rs = uc_mallocstrw(bp,bl,&cp)) >= 0) {
	                            cpp->val.sp = cp ;
	                            cpp->val.sl = bl ;
	                        } /* end if (memory-allocation) */
	                    }
	                    if (rs < 0) {
	                        uc_free(cpp->com.sp) ;
	                        cpp->com.sp = NULL ;
	                    }
	                } /* end if (memory-allocation) */
	            } /* end if (buffer_get) */
	        } /* end if (finishing-up) */

	        rs1 = buffer_finish(&as[1]) ;	/* comment */
		if (rs >= 0) rs = rs1 ;
	    } /* end if (buffer) */

	    vl = buffer_finish(&as[0]) ;	/* value */
	    if (rs >= 0) rs = vl ;
	} /* end if (buffer) */

#if	CF_DEBUGS
	debugprintf("comparse_start: value=>%s<\n",cpp->val) ;
	debugprintf("comparse_start: comment=>%s<\n",cpp->com) ;
	debugprintf("comparse_start: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (comparse_bake) */


