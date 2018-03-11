/* cfnum */

/* convert from a number to an integer */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-17, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines convert numeric strings in a variety of formats or
	bases into the integer-binary form.  These subroutines handle some
	cases that the 'atox(3c)' family do not.  Also, we handle Latin-1
	characters correctly: specifically with our use of:

		CHAR_TOLC(3dam)
		isdigitlatin(3dam)
		isalphalatin(3dam)

	instead of:

		tolower(3c)
		isdigit(3c)
		islapha(3c)

	The standard subroutines often go crazy when confronted with Latin-1
	characters with values >= 128.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<cfbin.h>
#include	<cfoct.h>
#include	<cfdec.h>
#include	<cfhex.h>
#include	<localmisc.h>


/* local defines */

#define	TOLC(ch)	CHAR_TOLC(ch)


/* external subroutines */

extern int	isdigitlatin(int) ;
extern int	isalphalatin(int) ;


/* forward references */

int		cfnumi(cchar *,int,int *) ;


/* local variables */


/* exported subroutines */


int cfnum(cchar *bp,int bl,int *rp)
{

	return cfnumi(bp,bl,rp) ;
}
/* end subroutine (cfnum) */


int cfnumi(cchar *bp,int bl,int *rp)
{
	int		rs = SR_INVALID ;
	int		ch ;
	int		f_negative = FALSE ;

	bl = strnlen(bp,bl) ;

	while ((bl > 0) && CHAR_ISWHITE(*bp)) {
	    bp += 1 ;
	    bl -= 1 ;
	}

	if ((bl > 0) && ((*bp == '+') || (*bp == '-'))) {
	    f_negative = (*bp == '-') ;
	    bp += 1 ;
	    bl -= 1 ;
	}

	if (bl > 0) {

#if	CF_DEBUGS
	    debugprintf("cfnumi: buf=>%t<\n",bp,bl) ;
#endif

	    if (*bp == '\\') {
	        bp += 1 ;
	        bl -= 1 ;
	        if (bl > 1) {
	            ch = TOLC(bp[0]) ;
#if	CF_DEBUGS
	            debugprintf("cfnumi: escape '%c' numstr=>%t<\n",
	                bp[0],bp,bl) ;
#endif
	            bp += 1 ;
	            bl -= 1 ;
	            switch (ch) {
	            case 'd':
	                rs = cfdeci(bp,bl,rp) ;
	                break ;
	            case 'x':
	                rs = cfhexi(bp,bl,rp) ;
	                break ;
	            case 'o':
	                rs = cfocti(bp,bl,rp) ;
	                break ;
	            case 'b':
	                rs = cfbini(bp,bl,rp) ;
	                break ;
	            default:
	                rs = SR_INVALID ;
	                break ;
	            } /* end switch */
	        } else {
	            rs = SR_INVALID ;
		}
	    } else if (isdigitlatin(MKCHAR(*bp))) {
	        if (bl > 1) {
	            ch = TOLC(bp[1]) ;
	            if (isalphalatin(ch)) {
	                bp += 2 ;
	                bl -= 2 ;
	                switch (ch) {
	                case 'd':
	                    rs = cfdeci(bp,bl,rp) ;
	                    break ;
	                case 'x':
	                    rs = cfhexi(bp,bl,rp) ;
	                    break ;
	                case 'o':
	                    rs = cfocti(bp,bl,rp) ;
	                    break ;
	                case 'b':
	                    rs = cfbini(bp,bl,rp) ;
	                    break ;
	                default:
	                    rs = SR_INVALID ;
	                    break ;
	                } /* end switch */
	            } else if (bp[0] == '0') {
	                rs = cfocti((bp+1),(bl-1),rp) ;
	            } else {
	                rs = cfdeci(bp,bl,rp) ;
		    }
	        } else {
	            rs = cfdeci(bp,bl,rp) ;
		}
	    } /* end if */

	    if (f_negative)
	        *rp = (- *rp) ;

	} else
	    rs = SR_DOM ;

#if	CF_DEBUGS
	debugprintf("cfnumi: val=\\x%08x\n",*rp) ;
#endif

	return rs ;
}
/* end subroutine (cfnumi) */


/* handle unsigned */
int cfnumui(cchar *bp,int bl,uint *rp)
{
	int		rs = SR_INVALID ;
	int		ch ;
	int		f_negative = FALSE ;

	bl = strnlen(bp,bl) ;

	while ((bl > 0) && CHAR_ISWHITE(*bp)) {
	    bp += 1 ;
	    bl -= 1 ;
	}

#if	CF_DEBUGS
	debugprintf("cfnumui: 1 buf=>%t<\n",bp,bl) ;
#endif

	if ((bl > 0) && ((*bp == '+') || (*bp == '-'))) {
	    f_negative = (*bp == '-') ;
	    bp += 1 ;
	    bl -= 1 ;
	}

	if (bl > 0) {

#if	CF_DEBUGS
	    debugprintf("cfnumui: 2 buf=>%t<\n",bp,bl) ;
#endif

	    if (*bp == '\\') {
#if	CF_DEBUGS
	        debugprintf("cfnumui: explicit radix supplied\n") ;
#endif
	        bp += 1 ;
	        bl -= 1 ;
	        if (bl > 0) {
	            ch = TOLC(bp[0]) ;
#if	CF_DEBUGS
	            debugprintf("cfnumi: escape '%c' numstr=>%t<\n",
	                bp[0],bp,bl) ;
#endif
	            bp += 1 ;
	            bl -= 1 ;
	            switch (ch) {
	            case 'd':
	                rs = cfdecui(bp,bl,rp) ;
	                break ;
	            case 'x':
	                rs = cfhexui(bp,bl,rp) ;
	                break ;
	            case 'o':
	                rs = cfoctui(bp,bl,rp) ;
	                break ;
	            case 'b':
	                rs = cfbinui(bp,bl,rp) ;
	                break ;
	            default:
	                rs = SR_INVALID ;
	                break ;
	            } /* end switch */
	        } else {
	            rs = SR_INVALID ;
		}
	    } else if (isdigitlatin(MKCHAR(*bp))) {
	        if (bl > 1) {
	            ch = TOLC(bp[1]) ;

	            if (isalphalatin(ch)) {

	                bp += 2 ;
	                bl -= 2 ;
	                switch (ch) {
	                case 'd':
	                    rs = cfdecui(bp,bl,rp) ;
	                    break ;
	                case 'x':
	                    rs = cfhexui(bp,bl,rp) ;
	                    break ;
	                case 'o':
	                    rs = cfoctui(bp,bl,rp) ;
	                    break ;
	                case 'b':
	                    rs = cfbinui(bp,bl,rp) ;
	                    break ;
	                default:
	                    rs = SR_INVALID ;
	                    break ;
	                } /* end switch */

	            } else if (bp[0] == '0') {
	                rs = cfoctui((bp+1),(bl-1),rp) ;
	            } else {
	                rs = cfdecui(bp,bl,rp) ;
		    }
	        } else {
	            rs = cfdecui(bp,bl,rp) ;
		}
	    } /* end if */

	    if (f_negative)
	        *rp = (- *rp) ;

	} else
	    rs = SR_DOM ;

#if	CF_DEBUGS
	debugprintf("cfnumui: ret rs=%d val=\\x%08x\n",rs,*rp) ;
#endif

	return rs ;
}
/* end subroutine (cfnumui) */


/* convert from a string form to a long-integer (32-its) */
int cfnuml(cchar *bp,int bl,long *rp)
{
	int		rs = SR_INVALID ;
	int		ch ;
	int		f_negative = FALSE ;

	bl = strnlen(bp,bl) ;

	while ((bl > 0) && CHAR_ISWHITE(*bp)) {
	    bp += 1 ;
	    bl -= 1 ;
	}

	if ((bl > 0) && ((*bp == '+') || (*bp == '-'))) {
	    f_negative = (*bp == '-') ;
	    bp += 1 ;
	    bl -= 1 ;
	}

	if (bl > 0) {

	    if (*bp == '\\') {
	        bp += 1 ;
	        bl -= 1 ;
	        if (bl > 1) {
	            ch = TOLC(bp[0]) ;
	            bp += 1 ;
	            bl -= 1 ;
	            switch (ch) {
	            case 'd':
	                rs = cfdecl(bp,bl,rp) ;
	                break ;
	            case 'x':
	                rs = cfhexl(bp,bl,rp) ;
	                break ;

	            case 'o':
	                rs = cfoctl(bp,bl,rp) ;
	                break ;

	            case 'b':
	                rs = cfbinl(bp,bl,rp) ;
	                break ;
	            default:
	                rs = SR_INVALID ;
	                break ;
	            } /* end switch */
	        } else {
	            rs = SR_INVALID ;
		}
	    } else if (isdigitlatin(MKCHAR(*bp))) {
	        if (bl > 1) {
	            ch = TOLC(bp[1]) ;
	            if (isalphalatin(ch)) {
	                bp += 2 ;
	                bl -= 2 ;
	                switch (ch) {
	                case 'd':
	                    rs = cfdecl(bp,bl,rp) ;
	                    break ;
	                case 'x':
	                    rs = cfhexl(bp,bl,rp) ;
	                    break ;
	                case 'o':
	                    rs = cfoctl(bp,bl,rp) ;
	                    break ;
	                case 'b':
	                    rs = cfbinl(bp,bl,rp) ;
	                    break ;
	                default:
	                    rs = SR_INVALID ;
	                    break ;
	                } /* end switch */
	            } else if (bp[0] == '0') {
	                rs = cfoctl((bp+1),(bl-1),rp) ;
	            } else {
	                rs = cfdecl(bp,bl,rp) ;
		    }
	        } else {
	            rs = cfdecl(bp,bl,rp) ;
		}
	    } /* end if */

	    if (f_negative)
	        *rp = (- *rp) ;

	} else
	    rs = SR_DOM ;

	return rs ;
}
/* end subroutine (cfnuml) */


/* handle unsigned long integer (32-bits) */
int cfnumul(cchar *bp,int bl,ulong *rp)
{
	int		rs = SR_INVALID ;
	int		ch ;
	int		f_negative = FALSE ;

	bl = strnlen(bp,bl) ;

	while ((bl > 0) && CHAR_ISWHITE(*bp)) {
	    bp += 1 ;
	    bl -= 1 ;
	}

	if ((bl > 0) && ((*bp == '+') || (*bp == '-'))) {
	    f_negative = (*bp == '-') ;
	    bp += 1 ;
	    bl -= 1 ;
	}

	if (bl > 0) {

	    if (*bp == '\\') {
	        bp += 1 ;
	        bl -= 1 ;
	        if (bl > 0) {
	            ch = TOLC(bp[0]) ;
	            bp += 1 ;
	            bl -= 1 ;
	            switch (ch) {
	            case 'd':
	                rs = cfdecul(bp,bl,rp) ;
	                break ;
	            case 'x':
	                rs = cfhexul(bp,bl,rp) ;
	                break ;
	            case 'o':
	                rs = cfoctul(bp,bl,rp) ;
	                break ;
	            case 'b':
	                rs = cfbinul(bp,bl,rp) ;
	                break ;
	            default:
	                rs = SR_INVALID ;
	                break ;
	            } /* end switch */
	        } else {
	            rs = SR_INVALID ;
		}
	    } else if (isdigitlatin(MKCHAR(*bp))) {
	        if (bl > 1) {
	            ch = TOLC(bp[1]) ;
	            if (isalphalatin(ch)) {
	                bp += 2 ;
	                bl -= 2 ;
	                switch (ch) {
	                case 'd':
	                    rs = cfdecul(bp,bl,rp) ;
	                    break ;
	                case 'x':
	                    rs = cfhexul(bp,bl,rp) ;
	                    break ;
	                case 'o':
	                    rs = cfoctul(bp,bl,rp) ;
	                    break ;
	                case 'b':
	                    rs = cfbinul(bp,bl,rp) ;
	                    break ;
	                default:
	                    rs = SR_INVALID ;
	                    break ;
	                } /* end switch */
	            } else if (bp[0] == '0') {
	                rs = cfoctul((bp+1),(bl-1),rp) ;
	            } else {
	                rs = cfdecul(bp,bl,rp) ;
		    }
	        } else {
	            rs = cfdecul(bp,bl,rp) ;
		}
	    } /* end if */

	    if (f_negative)
	        *rp = (- *rp) ;

	} else
	    rs = SR_DOM ;

	return rs ;
}
/* end subroutine (cfnumul) */


/* convert from a string form to a LONG integer (64 bits) */
int cfnumll(cchar *bp,int bl,longlong *rp)
{
	int		rs = SR_INVALID ;
	int		ch ;
	int		f_negative = FALSE ;

	bl = strnlen(bp,bl) ;

	while ((bl > 0) && CHAR_ISWHITE(*bp)) {
	    bp += 1 ;
	    bl -= 1 ;
	}

	if ((bl > 0) && ((*bp == '+') || (*bp == '-'))) {
	    f_negative = (*bp == '-') ;
	    bp += 1 ;
	    bl -= 1 ;
	}

	if (bl > 0) {

#if	CF_DEBUGS
	    debugprintf("cfnumll: buf=>%t<\n",bp,bl) ;
#endif

	    if (*bp == '\\') {
	        bp += 1 ;
	        bl -= 1 ;
	        if (bl > 1) {
	            ch = TOLC(bp[0]) ;
	            bp += 1 ;
	            bl -= 1 ;
	            switch (ch) {
	            case 'd':
	                rs = cfdecll(bp,bl,rp) ;
	                break ;
	            case 'x':
	                rs = cfhexll(bp,bl,rp) ;
	                break ;
	            case 'o':
	                rs = cfoctll(bp,bl,rp) ;
	                break ;
	            case 'b':
	                rs = cfbinll(bp,bl,rp) ;
	                break ;
	            default:
	                rs = SR_INVALID ;
	                break ;
	            } /* end switch */
	        } else {
	            rs = SR_INVALID ;
		}
	    } else if (isdigitlatin(MKCHAR(*bp))) {
	        if (bl > 1) {
	            ch = TOLC(bp[1]) ;
	            if (isalphalatin(ch)) {
	                bp += 2 ;
	                bl -= 2 ;
	                switch (ch) {
	                case 'd':
	                    rs = cfdecll(bp,bl,rp) ;
	                    break ;
	                case 'x':
	                    rs = cfhexll(bp,bl,rp) ;
	                    break ;
	                case 'o':
	                    rs = cfoctll(bp,bl,rp) ;
	                    break ;
	                case 'b':
	                    rs = cfbinll(bp,bl,rp) ;
	                    break ;
	                default:
	                    rs = SR_INVALID ;
	                    break ;
	                } /* end switch */
	            } else if (bp[0] == '0') {
	                rs = cfoctll((bp+1),(bl-1),rp) ;
	            } else {
	                rs = cfdecll(bp,bl,rp) ;
		    }
	        } else {
	            rs = cfdecll(bp,bl,rp) ;
		}
	    } /* end if */

	    if (f_negative)
	        *rp = (- *rp) ;

	} else
	    rs = SR_DOM ;

#if	CF_DEBUGS
	debugprintf("cfnumll: ret rs=%d val=\\x%08llx\n",,rs,*rp) ;
#endif

	return rs ;
}
/* end subroutine (cfnumll) */


/* handle unsigned LONG (64 bits) */
int cfnumull(cchar *bp,int bl,ulonglong *rp)
{
	int		rs = SR_INVALID ;
	int		ch ;
	int		f_negative = FALSE ;

	bl = strnlen(bp,bl) ;

	while ((bl > 0) && CHAR_ISWHITE(*bp)) {
	    bp += 1 ;
	    bl -= 1 ;
	}

	if ((bl > 0) && ((*bp == '+') || (*bp == '-'))) {
	    f_negative = (*bp == '-') ;
	    bp += 1 ;
	    bl -= 1 ;
	}

	if (bl > 0) {

#if	CF_DEBUGS
	    debugprintf("cfnumull: buf=>%t<\n",bp,bl) ;
#endif

	    if (*bp == '\\') {
	        bp += 1 ;
	        bl -= 1 ;
	        if (bl > 0) {
	            ch = TOLC(bp[0]) ;
	            bp += 1 ;
	            bl -= 1 ;
	            switch (ch) {
	            case 'd':
	                rs = cfdecull(bp,bl,rp) ;
	                break ;
	            case 'x':
	                rs = cfhexull(bp,bl,rp) ;
	                break ;
	            case 'o':
	                rs = cfoctull(bp,bl,rp) ;
	                break ;
	            case 'b':
	                rs = cfbinull(bp,bl,rp) ;
	                break ;
	            default:
	                rs = SR_INVALID ;
	                break ;
	            } /* end switch */
	        } else {
	            rs = SR_INVALID ;
		}
	    } else if (isdigitlatin(MKCHAR(*bp))) {
	        if (bl > 1) {
	            ch = TOLC(bp[1]) ;
	            if (isalphalatin(ch)) {
	                bp += 2 ;
	                bl -= 2 ;
	                switch (ch) {
	                case 'd':
	                    rs = cfdecull(bp,bl,rp) ;
	                    break ;
	                case 'x':
	                    rs = cfhexull(bp,bl,rp) ;
	                    break ;
	                case 'o':
	                    rs = cfoctull(bp,bl,rp) ;
	                    break ;
	                case 'b':
	                    rs = cfbinull(bp,bl,rp) ;
	                    break ;
	                default:
	                    rs = SR_INVALID ;
	                    break ;
	                } /* end switch */
	            } else if (bp[0] == '0') {
	                rs = cfoctull((bp+1),(bl-1),rp) ;
	            } else {
	                rs = cfdecull(bp,bl,rp) ;
		    }
	        } else {
	            rs = cfdecull(bp,bl,rp) ;
		}
	    } /* end if */

	    if (f_negative)
	        *rp = (- *rp) ;

	} else
	    rs = SR_DOM ;

#if	CF_DEBUGS
	debugprintf("cfnumull: ret rs=%d val=\\x%08llx\n",rs,*rp) ;
#endif

	return rs ;
}
/* end subroutine (cfnumull) */


