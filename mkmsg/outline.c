/* outline */

/* manage printing lines */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

	= 1999-02-01, David A­D­ Morano
	I added a little code to "post" articles that do not have a valid
	newsgroup to a special "dead article" directory in the BB spool area.

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object deals with printing lines.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"outline.h"
#include	"contentencodings.h"


/* local defines */

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	LOGLINELEN	(80 - 16)

#define	DATEBUFLEN	80
#define	STACKADDRBUFLEN	(2 * 1024)

#undef	BUFLEN
#define	BUFLEN		(2 * 1024)

#ifndef	NTABCOLS
#define	NTABCOLS	8
#endif

#define	BASE64LINELEN	72
#define	BASE64BUFLEN	((BASE64LINELEN / 4) * 3)

#ifndef	FROM_ESCAPE
#define	FROM_ESCAPE	'\b'
#endif

#undef	CHAR_TOKSEP
#define	CHAR_TOKSEP(c)	(CHAR_ISWHITE(c) || (! isprintlatin(c)))


/* external subroutines */

extern int	nextfield(const char *,int,const char **) ;
extern int	isprintlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int outline_start(OUTLINE *op,bfile *ofp,int maxlen)
{

	if (op == NULL) return SR_FAULT ;
	if (ofp == NULL) return SR_FAULT ;

	memset(op,0,sizeof(OUTLINE)) ;
	op->maxlen = maxlen ;
	op->rlen = (maxlen -1) ;
	op->ofp = ofp ;

	return SR_OK ;
}
/* end subroutine (outline_start) */


int outline_finish(OUTLINE *ldp)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (ldp == NULL) return SR_FAULT ;
	if (ldp->ofp == NULL) return SR_FAULT ;

	if (ldp->llen > 0) {
	    rs = bputc(ldp->ofp,'\n') ;
	    wlen += rs ;
	    ldp->wlen += rs ;
	    ldp->rlen = (ldp->maxlen - 1) ;
	    ldp->llen = 0 ;
	}

	ldp->ofp = NULL ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outline_finish) */


int outline_item(OUTLINE *ldp,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (vp == NULL) return SR_FAULT ;

	if (vl < 0)
	    vl = strlen(vp) ;

	if (vl > 0) {
	    ldp->f.comma = TRUE ;
	    rs = outline_value(ldp,vp,vl) ;
	    wlen += rs ;
	    ldp->c_items += 1 ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outline_item) */


/* output a single value for a header (folding lines as needed) */
int outline_value(OUTLINE *ldp,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		nlen ;
	int		cl, cl2 ;
	int		wlen = 0 ;
	int		f_comma = FALSE ;
	const char	*fmt ;
	const char	*tp, *cp ;

	if (ldp == NULL) return SR_INVALID ;
	if (vp == NULL) return SR_FAULT ;

	if (vp[0] == '\0') return SR_OK ;

	if (vl < 0)
	    vl = strlen(vp) ;

	ldp->c_values = 0 ;
	while ((rs >= 0) && (vl > 0)) {

	    if ((cl = nextfield(vp,vl,&cp)) > 0) {

	        f_comma = (ldp->f.comma && (ldp->c_items > 0)) ;
	        nlen = outline_needlength(ldp,cl) ;

	        if (nlen > ldp->rlen) {

	            if (ldp->llen > 0) {
	                fmt = "\n" ;
	                if (f_comma) {
	                    f_comma = FALSE ;
	                    ldp->f.comma = FALSE ;
	                    fmt = ",\n" ;
	                }
	                rs = bwrite(ldp->ofp,fmt,-1) ;
	                wlen += rs ;
	            }

	            ldp->rlen = (ldp->maxlen - 1) ;
	            ldp->llen = 0 ;
	            ldp->c_values = 0 ;

	        } /* end if (overflow) */

	        if (rs >= 0) {
	            fmt = " %t" ;
	            if (f_comma) {
	                f_comma = FALSE ;
	                ldp->f.comma = FALSE ;
	                fmt = ", %t" ;
	            }
	            rs = bprintf(ldp->ofp,fmt,cp,cl) ;
	            wlen += rs ;
	            ldp->llen += rs ;
	            ldp->rlen -= rs ;
	        }

	        ldp->c_values += 1 ;
	        cl2 = ((cp+cl) - vp) ;
	        vp += cl2 ;
	        vl -= cl2 ;

	    } else if ((tp = strnchr(vp,vl,'\n')) != NULL) {
	        vl -= ((tp + 1) - vp) ;
	        vp = (tp + 1) ;
	    } else
	        vl = 0 ;

	} /* end while */

	ldp->wlen += wlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outline_value) */


int outline_write(OUTLINE *ldp,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (vl < 0) vl = strlen(vp) ;

	if (vl > 0) {
	    if ((rs = bwrite(ldp->ofp,vp,vl)) >= 0) {
	        wlen += rs ;
	        ldp->wlen += rs ;
	        ldp->llen += rs ;
	        ldp->rlen -= rs ;
	    }
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outline_write) */


int outline_printf(OUTLINE *ldp,cchar *fmt,...)
{
	int		rs ;
	int		wlen = 0 ;

	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = bvprintf(ldp->ofp,fmt,ap) ;
	    va_end(ap) ;
	}

	if (rs >= 0) {
	    wlen += rs ;
	    ldp->wlen += rs ;
	    ldp->llen += rs ;
	    ldp->rlen -= rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outline_printf) */


int outline_needlength(OUTLINE *ldp,int cl)
{
	int		nlen = (cl + 1) ;

	if (ldp->llen == 0)
	    nlen += 1 ;

	if (ldp->f.comma && (ldp->c_items > 0))
	    nlen += 1 ;

	return nlen ;
}
/* end subroutine (outline_needlength) */


