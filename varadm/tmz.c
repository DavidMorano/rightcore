/* tmz */

/* time and timezone parsing */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */	
#define	CF_SAFE		0		/* safe mode */
#define	CF_MULTIZONE	1		/* allow fragmented time zone names */


/* revision history:

	= 1999-05-01, David A­D­ Morano
        This was created along with the DATE object. This code was based on
        older code I wrote to handle the "STRDIG" form for the BBNEWS facility.
        That goes back in time to possibly as far back as 1992!

	= 2014-07-15, David A­D­ Morano
        I laugh sometimes as how long some of these objects (or subroutines)
        last without change. Yes, most of this code looks pretty bad by today's
        standards. But to the point, I am enhancing this object (specially the
        'tmz_std()' subroutine) so that it recognizes a date-string without
        either a time-zone abbreviation or a year but which does have a
        time-zone offset. In the past, the case of an offset without a year was
        supposed to be an error in conversion. But now-a-days, in an effort to
        make almost all date-strings work similarly, we enhance this subroutine
        to make this above case allowable. This case never has occurred (because
        having zone-offsets is so relatively new in itself) and really never can
        happen (because we always now have fully qualified date-strings with the
        only exception possibly having the zone-offset excluded). But we will
        make this enhancement and just to round out the symmetry of the various
        date-strings.

*/

/* Copyright © 1999,2014 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object is used to parse date strings that are in ASCII and are
        human-readable. There are tons of places where ASCII strings that
        represent a date are used. I will leave it as an exercise for the reader
        to think up some of those!


	Note for STD type dates:

	Note that our 'tmz_std()' will parse both the old style standard date
	strings (a la 'ctime(3c)') as well as the new style date strings
	('date(1)').  An old style standard date string looked like:

		[Mon] Nov 26 21:48[:25] 1999

	The new style standard date string looks like:

		[Mon] Nov 26 21:48[:25] [EST] 1999

	Both are handled.  The newer style, used in newer email envelopes, is
	prefered since it includes the timezone name.

	Updated note:
	It is reported that some envelopes of the form:

		[Mon] Nov 26 21:48[:25] [EST] 1999 [-0500]

	are being used in the world (and particularly on Sun Solaris boxes).
	We also handle this variant.


*******************************************************************************/


#define	TMZ_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<estrings.h>
#include	<char.h>
#include	<field.h>
#include	<tmstrs.h>
#include	<localmisc.h>

#include	"tmz.h"


/* local defines */

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY	100
#endif

#ifndef	CENTURY_BASE
#define	CENTURY_BASE	19
#endif

#ifndef	MKCHAR
#define	MKCHAR(c)	(c & 0xff)
#endif


/* external subroutines */

extern int	nextfield(cchar *,int,cchar **) ;
extern int	siskipwhite(cchar *,int) ;
extern int	sialpha(cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	hasalldig(cchar *,int) ;
extern int	isalphalatin(int) ;
extern int	isalnumlatin(int) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strnwcpy(char *,int,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strdcpy1w(char *,int,cchar *,int) ;


/* local structures */


/* forward references */

static int	tmz_timeparts(TMZ *,cchar *,int) ;
static int	tmz_stdtrailing(TMZ *,cchar *,int) ;
static int	tmz_procday(TMZ *,cchar *,int) ;
static int	tmz_procmonth(TMZ *,cchar *,int) ;
static int	tmz_procyear(TMZ *,cchar *,int) ;
static int	tmz_proczoff(TMZ *,cchar *,int) ;
static int	tmz_proczname(TMZ *,cchar *,int) ;
static int	tmz_yearadj(TMZ *,int) ;

static int	getzoff(int *,cchar *,int) ;
static int	val(cchar *) ;
static int	silogend(cchar *,int) ;

static int	isplusminus(int) ;

#if	defined(CF_MULTIZONE) && (CF_MULTIZONE == 0)
static int	isgoodname(cchar *,int) ;
#endif

static cchar	*strnzone(cchar *,int) ;


/* local variables */

static const uchar	tpterms[] = {
	0x00, 0x1F, 0x00, 0x00,
	0x01, 0x00, 0x00, 0x04,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int tmz_init(TMZ *op)
{
	if (op == NULL) return SR_FAULT ;
	memset(op,0,sizeof(TMZ)) ;
	op->zoff = SHORT_MIN ;
	return SR_OK ;
}
/* end subroutine (tmz_init) */


/* format> [Wed] Nov 14 19:24[:04] [EST] [[19]99] [±0400] */
int tmz_std(TMZ *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (sp == NULL) return SR_FAULT ;
#endif

	if (sl < 0)
	    sl = strlen(sp) ;

	memset(op,0,sizeof(TMZ)) ;
	op->zoff = SHORT_MIN ;
	op->st.tm_year = -1 ;
	op->st.tm_wday = -1 ;
	op->st.tm_isdst = -1 ;

	if ((rs >= 0) && ((rs = tmz_procmonth(op,sp,sl)) > 0)) {
	    sp += rs ;
	    sl -= rs ;
	}

	if ((rs >= 0) && ((rs = tmz_procday(op,sp,sl)) > 0)) {
	    sp += rs ;
	    sl -= rs ;
	}

	if ((rs >= 0) && (op->st.tm_mday > 31)) {
	    rs = SR_INVALID ;
	}

	if ((rs >= 0) && ((rs = tmz_timeparts(op,sp,sl)) > 0)) {
	    sp += rs ;
	    sl -= rs ;
	}

	for (i = 0 ; (rs >= 0) && (i < 3) ; i += 1) {
	    rs = tmz_stdtrailing(op,sp,sl) ;
	    if (rs == 0) break ;
	    sp += rs ;
	    sl -= rs ;
	} /* end for */

	if (rs >= 0) {
	    rs = strnlen(op->zname,TMZ_ZNAMESIZE) ;
	}

#if	CF_DEBUGS
	debugprintf("tmz_std: ret rs=%d zname=%t\n",
	    rs,op->zname,strnlen(op->zname,TMZ_ZNAMESIZE)) ;
#endif

	return rs ;
}
/* end subroutine (tmz_std) */


/* format> [Weekday,] DD MMM [CC]YY hh:mm[:ss] [±hhmm] [zname] */
int tmz_msg(TMZ *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		cl ;
	int		zl = 0 ;
	cchar		*tp ;
	cchar		*cp ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (sp == NULL) return SR_FAULT ;
#endif

	if (sl < 0)
	    sl = strlen(sp) ;

	memset(op,0,sizeof(TMZ)) ;
	op->zoff = SHORT_MIN ;
	op->st.tm_year = -1 ;
	op->st.tm_wday = -1 ;
	op->st.tm_isdst = -1 ;

#if	CF_DEBUGS
	debugprintf("tmz_msg: sl=%d\n",sl) ;
	debugprintf("tmz_msg: >%t<\n",sp,strlinelen(sp,sl,76)) ;
#endif

	if ((tp = strnchr(sp,sl,',')) != NULL) {
	    if ((cl = nextfield(sp,(tp-sp),&cp)) > 0) {
	        rs = tmstrsday(cp,cl) ;
	        op->st.tm_wday = rs ;
	    }
	    sl -= ((tp+1)-sp) ;
	    sp = (tp+1) ;
	}

	if ((rs >= 0) && ((rs = tmz_procday(op,sp,sl)) > 0)) {
	    sp += rs ;
	    sl -= rs ;
	}

	if ((rs >= 0) && ((rs = tmz_procmonth(op,sp,sl)) > 0)) {
	    sp += rs ;
	    sl -= rs ;
	}

	if ((rs >= 0) && (op->st.tm_mday > 31)) {
	    rs = SR_INVALID ;
	}

	if ((rs >= 0) && ((rs = tmz_procyear(op,sp,sl)) > 0)) {
	    sp += rs ;
	    sl -= rs ;
	}

	if ((rs >= 0) && ((rs = tmz_timeparts(op,sp,sl)) > 0)) {
	    sp += rs ;
	    sl -= rs ;
	}

	if ((rs >= 0) && ((rs = tmz_proczoff(op,sp,sl)) > 0)) {
	    sp += rs ;
	    sl -= rs ;
	}

	if ((rs >= 0) && ((rs = tmz_proczname(op,sp,sl)) > 0)) {
	    sp += rs ;
	    sl -= rs ;
	}

	if (rs >= 0) {
	    rs = strnlen(op->zname,TMZ_ZNAMESIZE) ;
	    zl = rs ; /* return value for subroutine */
	}

#if	CF_DEBUGS
	debugprintf("tmz_msg: ret f_zoff=%d zoff=%dm\n",
	    op->f.zoff,op->zoff) ;
	debugprintf("tmz_msg: ret zname=%t\n",
	    op->zname,strnlen(op->zname,TMZ_ZNAMESIZE)) ;
#endif

	return (rs >= 0) ? zl : rs ;
}
/* end subroutine (tmz_msg) */


/* convert from a TOUCH (original) format> MMDDhhmm[YY] */
int tmz_touch(TMZ *op,cchar *sp,int sl)
{
	struct tm	*stp ;
	const int	n = 5 ;
	int		rs = SR_OK ;
	int		i = 0 ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (sp == NULL) return SR_FAULT ;
#endif

	if (sl < 0)
	    sl = strlen(sp) ;

	stp = &op->st ;
	memset(op,0,sizeof(TMZ)) ;
	op->zoff = SHORT_MIN ;
	stp->tm_year = -1 ;
	stp->tm_wday = -1 ;
	stp->tm_isdst = -1 ;

/* skip leading white space */

	while (sl && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}
	while (sl && CHAR_ISWHITE(sp[sl-1])) sl -= 1 ;

#if	CF_DEBUGS
	debugprintf("tmz_touch: stripped string=%t\n",sp,sl) ;
#endif

	if (hasalldig(sp,sl)) {
	    while ((sl >= 2) && (i < n)) {
	        switch (i++) {
	        case 0:
	            stp->tm_mon = (val(sp) - 1) ;
	            break ;
	        case 1:
	            stp->tm_mday = val(sp) ;
	            break ;
	        case 2:
	            stp->tm_hour = val(sp) ;
	            break ;
	        case 3:
	            stp->tm_min = val(sp) ;
	            break ;
	        case 4:
	            stp->tm_year = val(sp) ;
	            break ;
	        } /* end switch */
	        sp += 2 ;
	        sl -= 2 ;
	    } /* end while */
	} else {
	    rs = SR_INVALID ;
	}

	if (rs >= 0) {
	    if (i >= n) {
	        op->f.year = TRUE ;
	        if ((stp->tm_year >= 0) && (stp->tm_year <= 38)) {
	            stp->tm_year += NYEARS_CENTURY ;
	        }
	    } else if (i < (n-1)) {
	        rs = SR_INVALID ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (tmz_touch) */


/* convert from a TOUCH-t (new '-t') format> [[CC]YY]MMDDhhmm[.SS] */
int tmz_toucht(TMZ *op,cchar *sp,int sl)
{
	struct tm	*stp ;
	const int	n = 4 ;
	int		rs = SR_OK ;
	int		i = 0 ;
	int		cc = -1 ;
	cchar		*tp ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (sp == NULL) return SR_FAULT ;
#endif

	if (sl < 0)
	    sl = strlen(sp) ;

	stp = &op->st ;
	memset(op,0,sizeof(TMZ)) ;
	op->zoff = SHORT_MIN ;
	stp->tm_year = -1 ;
	stp->tm_wday = -1 ;
	stp->tm_isdst = -1 ;

/* skip leading white space */

	while (sl && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}
	while (sl && CHAR_ISWHITE(sp[sl-1])) sl -= 1 ;

#if	CF_DEBUGS
	debugprintf("tmz_toucht: s=%t\n",sp,sl) ;
#endif

	if ((tp = strnchr(sp,sl,'.')) != NULL) {
	    const int	cl = (sl-((tp+1)-sp)) ;
	    cchar	*cp = (tp+1) ;
	    if (cl >= 2) {
	        const int	tch = MKCHAR(*cp) ;
	        if (isdigitlatin(tch)) {
	            stp->tm_sec = val(cp) ;
	        } else {
	            rs = SR_INVALID ;
	        }
	    }
	    sl = (tp-sp) ;
	} /* end if (tried for seconds) */

#if	CF_DEBUGS
	debugprintf("tmz_toucht: mid1 rs=%d\n",rs) ;
	debugprintf("tmz_toucht: s=%t\n",sp,sl) ;
#endif

	if (rs >= 0) {
	    if (hasalldig(sp,sl)) {

	        if ((rs >= 0) && (sl >= 12)) {
	            cc = val(sp) ;
	            sp += 2 ;
	            sl -= 2 ;
	        } /* end if (CC) */

#if	CF_DEBUGS
	        debugprintf("tmz_toucht: mid2 rs=%d\n",rs) ;
	        debugprintf("tmz_toucht: s=%t\n",sp,sl) ;
#endif

	        if (sl >= 10) {
	            op->f.year = TRUE ;
	            stp->tm_year = val(sp) ;
	            sp += 2 ;
	            sl -= 2 ;
	        } /* end if (YY) */

#if	CF_DEBUGS
	        debugprintf("tmz_toucht: mid3 rs=%d\n",rs) ;
	        debugprintf("tmz_toucht: s=%t\n",sp,sl) ;
#endif

	        while ((sl >= 2) && (i < n)) {
	            switch (i++) {
	            case 0:
	                stp->tm_mon = (val(sp) - 1) ;
	                break ;
	            case 1:
	                stp->tm_mday = val(sp) ;
	                break ;
	            case 2:
	                stp->tm_hour = val(sp) ;
	                break ;
	            case 3:
	                stp->tm_min = val(sp) ;
	                break ;
	            } /* end switch */
	            sp += 2 ;
	            sl -= 2 ;
	        } /* end while */

	    } else {
	        rs = SR_INVALID ;
	    }
	} /* end if (ok) */

	if (rs >= 0) {
	    if (i >= n) {
	        tmz_yearadj(op,cc) ;
	    } else {
	        rs = SR_INVALID ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("tmz_toucht: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (date_toucht) */


/* format> [[CC]]YYMMDDhhmm[ss][±hhmm][zname] */
int tmz_strdig(TMZ *op,cchar *sp,int sl)
{
	struct tm	*stp ;
	const int	n = 6 ;
	int		rs = SR_OK ;
	int		i = 0 ;
	int		cc = -1 ;
	int		zl = 0 ;
	cchar		*tp ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (sp == NULL) return SR_FAULT ;
#endif

	if (sl < 0)
	    sl = strlen(sp) ;

	stp = &op->st ;
	memset(op,0,sizeof(TMZ)) ;
	op->zoff = SHORT_MIN ;
	stp->tm_year = -1 ;
	stp->tm_wday = -1 ;
	stp->tm_isdst = -1 ;

/* skip leading and trailing white space */

	while (sl && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}
	while (sl && CHAR_ISWHITE(sp[sl-1])) sl -= 1 ;

#if	CF_DEBUGS
	debugprintf("tmz_strdig: s=%t\n",sp,sl) ;
#endif

	if ((tp = strnzone(sp,sl)) != NULL) {
	    cchar	*cp = tp ;
	    int		cl = (sl-(tp-sp)) ;
	    if ((cl >= 1) && isplusminus(*cp)) {
	        int	zol = cl ;
	        int	zo ;
	        int	si ;
	        cchar	*zop = cp ;
	        if ((si = sialpha(cp,cl)) > 0) {
	            zol = si ;
	        }
	        if ((rs = getzoff(&zo,zop,zol)) >= 0) {
	            op->zoff = zo ;
	            op->f.zoff = TRUE ;
	            cp += rs ;
	            cl -= rs ;
	        }
	    }
	    if ((rs >= 0) && (cl > 0)) {
	        int	ch = MKCHAR(*cp) ;
	        if (isalphalatin(ch)) {
		    const int	zlen = TMZ_ZNAMESIZE ;
	            rs = strnwcpy(op->zname,zlen,cp,cl) - op->zname ;
	            zl = rs ;
	        } else {
	            rs = SR_INVALID ;
	        }
	    }
	    sl = (tp-sp) ;
	} /* end if (tried for ZOFF and ZNAME) */

#if	CF_DEBUGS
	debugprintf("tmz_strdig: mid1 rs=%d\n",rs) ;
	debugprintf("tmz_strdig: s=%t\n",sp,sl) ;
#endif

	if (rs >= 0) {
	    i = 0 ;
	    if (hasalldig(sp,sl)) {

	        if ((rs >= 0) && (sl >= 14)) {
	            cc = val(sp) ;
	            sp += 2 ;
	            sl -= 2 ;
	        } /* end if (CC) */

#if	CF_DEBUGS
	        debugprintf("tmz_strdig: mid2 rs=%d\n",rs) ;
	        debugprintf("tmz_strdig: s=%t\n",sp,sl) ;
#endif

	        while ((sl >= 2) && (i < n)) {
	            switch (i++) {
	            case 0:
	                stp->tm_year = val(sp) ;
	                break ;
	            case 1:
	                stp->tm_mon = (val(sp) - 1) ;
	                break ;
	            case 2:
	                stp->tm_mday = val(sp) ;
	                break ;
	            case 3:
	                stp->tm_hour = val(sp) ;
	                break ;
	            case 4:
	                stp->tm_min = val(sp) ;
	                break ;
	            case 5:
	                stp->tm_sec = val(sp) ;
	                break ;
	            } /* end switch */
	            sp += 2 ;
	            sl -= 2 ;
	        } /* end while */

	        if (i >= (n-1)) {
	            tmz_yearadj(op,cc) ;
	        } else {
	            rs = SR_INVALID ;
	        }

	    } else {
	        rs = SR_INVALID ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("tmz_strdig: ret rs=%d zl=%u\n",rs,zl) ;
	debugprintf("tmz_strdig: f_zoff=%u zoff=%d\n",op->f.zoff,op->zoff) ;
	debugprintf("tmz_strdig: zname=%t\n",op->zname,8) ;
#endif

	return (rs >= 0) ? zl : rs ;
}
/* end subroutine (tmz_strdig) */


/* format> [CC]YYMMDD_hhmm[:ss][_][zname] */
int tmz_logz(TMZ *op,cchar *sp,int sl)
{
	struct tm	*stp ;
	int		rs = SR_OK ;
	int		zl = 0 ;
	cchar		*tp ;

#if	CF_DEBUGS
	debugprintf("tmz_logz: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (sp == NULL) return SR_FAULT ;
#endif

	if (sl < 0)
	    sl = strlen(sp) ;

	stp = &op->st ;
	memset(op,0,sizeof(TMZ)) ;
	op->zoff = SHORT_MIN ;
	stp->tm_year = -1 ;
	stp->tm_wday = -1 ;
	stp->tm_isdst = -1 ;

/* skip leading and trailing white space */

	while (sl && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}
	while (sl && CHAR_ISWHITE(sp[sl-1])) sl -= 1 ;

#if	CF_DEBUGS
	debugprintf("tmz_logz: 1 s=%t\n",sp,sl) ;
#endif

	if ((tp = strnchr(sp,sl,'_')) != NULL) {
	    int		si = (tp-sp) ;
#if	CF_DEBUGS
	    debugprintf("tmz_logz: 2 si=%u\n",si) ;
	    debugprintf("tmz_logz: 2 s=%t\n",sp,si) ;
#endif
	    if (hasalldig(sp,si)) {
	        int	cc = -1 ;
	        int	i = 0 ;
	        if (si >= 8) {
	            cc = val(sp) ;
	            sp += 2 ;
	            sl -= 2 ;
	        } /* end if (CC) */
#if	CF_DEBUGS
	        debugprintf("tmz_logz: 2 cc=%d\n",cc) ;
#endif
	        while ((sl >= 2) && (i < 3)) {
#if	CF_DEBUGS
	            debugprintf("tmz_logz: 2a s=>%t<\n",sp,sl) ;
#endif
	            switch (i++) {
	            case 0:
	                stp->tm_year = val(sp) ;
	                break ;
	            case 1:
	                stp->tm_mon = (val(sp) - 1) ;
	                break ;
	            case 2:
	                stp->tm_mday = val(sp) ;
	                break ;
	            } /* end switch */
	            sp += 2 ;
	            sl -= 2 ;
	        } /* end while */
	        sp += 1 ;
	        sl -= 1 ;
#if	CF_DEBUGS
	        debugprintf("tmz_logz: 3 s=>%t<\n",sp,sl) ;
#endif
	        if ((si = silogend(sp,sl)) >= 4) {
#if	CF_DEBUGS
	            debugprintf("tmz_logz: 3 si=%d\n",si) ;
	            debugprintf("tmz_logz: 3 s=>%t<\n",sp,si) ;
#endif
		    if (hasalldig(sp,4)) {
			int	ch ;
	                while ((sl >= 2) && (i < 6)) {
#if	CF_DEBUGS
	                    debugprintf("tmz_logz: i=%u\n",i) ;
#endif
	                    switch (i++) {
	                    case 3:
	                        stp->tm_hour = val(sp) ;
	                        break ;
	                    case 4:
	                        stp->tm_min = val(sp) ;
	                        break ;
	                    case 5:
	                        if ((sl >= 3) && (sp[0] == ':')) {
	                            sp += 1 ;
	                            sl -= 1 ;
	                        }
	                        stp->tm_sec = val(sp) ;
	                        break ;
	                    } /* end switch */
	                    sp += 2 ;
	                    sl -= 2 ;
	                } /* end while */
	                tmz_yearadj(op,cc) ;
	                if (sl && (*sp == '_')) {
	                    sp += 1 ;
	                    sl -= 1 ;
	                }
#if	CF_DEBUGS
	                debugprintf("tmz_logz: zname s=>%t<\n",sp,sl) ;
#endif
	                if (sl && ((ch = MKCHAR(*sp)),isalphalatin(ch))) {
	                    rs = tmz_proczname(op,sp,sl) ;
	                    zl = strlen(op->zname) ;
	                }
		    } else {
			rs = SR_INVALID ;
		    } /* end if (hasalldig) */
	        } /* end if (silogend) */
	    } else {
	        rs = SR_INVALID ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("tmz_logz: ret rs=%d zl=%u\n",rs,zl) ;
	debugprintf("tmz_logz: f_zoff=%u zoff=%d\n",op->f.zoff,op->zoff) ;
	debugprintf("tmz_logz: zname=%t\n",op->zname,8) ;
#endif

	return (rs >= 0) ? zl : rs ;
}
/* end subroutine (tmz_logz) */


/* format> [CC]YYMMDD */
int tmz_day(TMZ *op,cchar *sp,int sl)
{
	struct tm	*stp ;
	int		rs = SR_OK ;
	int		cc = -1 ;

	if (op == NULL) return SR_FAULT ;

#if	CF_SAFE
	if (sp == NULL) return SR_FAULT ;
#endif

	if (sl < 0)
	    sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("tmz_day: ent string=%t\n",sp,sl) ;
#endif

	stp = &op->st ;
	memset(op,0,sizeof(TMZ)) ;
	op->zoff = SHORT_MIN ;
	stp->tm_year = -1 ;
	stp->tm_wday = -1 ;
	stp->tm_isdst = -1 ;

	while (sl && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}
	while (sl && CHAR_ISWHITE(sp[sl-1])) sl -= 1 ;

#if	CF_DEBUGS
	debugprintf("tmz_day: stripped string=%t\n",sp,sl) ;
#endif

	if (hasalldig(sp,sl)) {
	    const int	n = 3 ;
	    int		i = 0 ;

	    if (sl >= 8) {
	        cc = val(sp) ;
	        sp += 2 ;
	        sl -= 2 ;
	    } /* end if (CC) */

	    while ((sl >= 2) && (i < n)) {
	        switch (i++) {
	        case 0:
	            stp->tm_year = val(sp) ;
	            break ;
	        case 1:
	            stp->tm_mon = (val(sp) - 1) ;
	            break ;
	        case 2:
	            stp->tm_mday = val(sp) ;
	            break ;
	        } /* end switch */
	        sp += 2 ;
	        sl -= 2 ;
	    } /* end while */

	    if (i >= n) {
	        tmz_yearadj(op,cc) ;
	    } else {
	        rs = SR_INVALID ;
	    }

	} else {
	    rs = SR_INVALID ;
	}

#if	CF_DEBUGS
	debugprintf("tmz_day: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (tmz_day) */


int tmz_isset(TMZ *op)
{
	if (op == NULL) return SR_FAULT ;
	return (op->st.tm_mday) ;
}
/* end subroutine (tmz_isset) */


int tmz_hasyear(TMZ *op)
{
	if (op == NULL) return SR_FAULT ;
	return (op->f.year) ;
}
/* end subroutine (tmz_hasyear) */


int tmz_haszoff(TMZ *op)
{
	if (op == NULL) return SR_FAULT ;
	return op->f.zoff ;
}
/* end subroutine (tmz_haszoff) */


int tmz_haszone(TMZ *op)
{
	if (op == NULL) return SR_FAULT ;
	return (op->zname[0] != '\0') ;
}
/* end subroutine (tmz_haszone) */


int tmz_setday(TMZ *op,int y,int m,int d)
{
	struct tm	*stp ;
	const int	cc = -1 ;
	if (op == NULL) return SR_FAULT ;
	stp = &op->st ;
	if (y >= TM_YEAR_BASE) {
	    y -= TM_YEAR_BASE ;
	}
	stp->tm_year = y ;
	stp->tm_mon = m ;
	stp->tm_mday = d ;
	tmz_yearadj(op,cc) ;
	return SR_OK ;
}
/* end subroutine (tmz_setday) */


int tmz_setyear(TMZ *op,int year)
{
	if (op == NULL) return SR_FAULT ;
	op->st.tm_year = year ;
	op->f.year = TRUE ;
	return SR_OK ;
}
/* end subroutine (tmz_setyear) */


int tmz_setzone(TMZ *op,cchar *zp,int zl)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	rs = strnwcpy(op->zname,TMZ_ZNAMESIZE,zp,zl) - op->zname ;

	return rs ;
}
/* end subroutine (tmz_setzone) */


int tmz_gettm(TMZ *op,struct tm *tmp)
{
	*tmp = op->st ;
	return SR_OK ;
}
/* end subroutine (tmz_gettm) */


int tmz_getdst(TMZ *op)
{
	return op->st.tm_isdst ;
}
/* end subroutine (tmz_getdst) */


int tmz_getzoff(TMZ *op)
{
	return op->zoff ;
}
/* end subroutine (tmz_getzoff) */


/* private subroutines */


/* format> hh:mm[:ss] */
static int tmz_timeparts(TMZ *op,cchar *sp,int sl)
{
	FIELD		fsb ;
	int		rs ;
	int		si = 0 ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    int		v ;
	    int		fl ;
	    cchar	*lp = sp ;
	    cchar	*fp ;

/* get hours */

	    if ((fl = field_get(&fsb,tpterms,&fp)) > 0) {
	        lp = (fp + fl) ;
	        rs = cfdeci(fp,fl,&v) ;
	        op->st.tm_hour = v ;
	    }

	    if ((rs >= 0) && (fsb.term == ':')) {

	        if ((fl = field_get(&fsb,tpterms,&fp)) > 0) {
	            lp = (fp + fl) ;
	            rs = cfdeci(fp,fl,&v) ;
	            op->st.tm_min = v ;
	        }

	    } /* end if */

	    if ((rs >= 0) && (fsb.term == ':')) {

	        if ((fl = field_get(&fsb,tpterms,&fp)) > 0) {
	            lp = (fp + fl) ;
	            rs = cfdeci(fp,fl,&v) ;
	            op->st.tm_sec = v ;
	        }

	    } /* end if */

	    si = (lp - sp) ;

	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? si : rs ;
}
/* end subroutine (tmz_timeparts) */


static int tmz_stdtrailing(TMZ *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		si = 0 ;
	int		wi ;

	if ((wi = siskipwhite(sp,sl)) >= 0) {
	    si += wi ;
	    sp += wi ;
	    sl -= wi ;
#if	CF_DEBUGS
	    debugprintf("tmz_stdtrailing: wi=%u s=>%t<\n",wi,sp,sl) ;
#endif
	    if (sl > 0) {
	        int	ch = MKCHAR(*sp) ;
	        if (isalphalatin(ch)) {
	            rs = tmz_proczname(op,sp,sl) ;
	        } else if (isdigitlatin(ch) && (! op->f.year)) {
	            rs = tmz_procyear(op,sp,sl) ;
	        } else if (isplusminus(ch) || isdigitlatin(ch)) {
	            rs = tmz_proczoff(op,sp,sl) ;
	        } else {
	            rs = SR_INVALID ;
	        }
	        si += rs ;
	        sp += si ;
	        sl -= si ;
	    } /* end if (non-zero string) */
	} /* end if (siskipwhite) */

#if	CF_DEBUGS
	debugprintf("tmz_stdtrailing: ret rs=%d si=%u\n",rs,si) ;
#endif

	return (rs >= 0) ? si : rs ;
}
/* end subroutine (tmz_stdtrailing) */


/* parse out> dd */
static int tmz_procday(TMZ *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		si = 0 ;
	int		cl ;
	cchar		*cp ;

	if ((cl = nextfield(sp,sl,&cp)) > 0) {
	    const int	tch = MKCHAR(*cp) ;
	    if (isdigitlatin(tch)) {
	        int	v ;
	        rs = cfdeci(cp,cl,&v) ;
	        op->st.tm_mday = v ;
	        si += ((cp+cl)-sp) ;
	        sp += si ;
	        sl -= si ;
	    } else {
	        rs = SR_INVALID ;
	    }
	} else {
	    rs = SR_INVALID ;
	}

	return (rs >= 0) ? si : rs ;
}
/* end subroutine (tmz_procday) */


/* parse out> [DDD] MMM */
static int tmz_procmonth(TMZ *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		si = 0 ;
	int		cl ;
	cchar		*cp ;

	if ((cl = nextfield(sp,sl,&cp)) > 0) {
	    int	ch = MKCHAR(*cp) ;
	    if (isalphalatin(ch)) {
	        int	ml = cl ;
	        cchar	*mp = cp ;
	        si += ((cp+cl)-sp) ;
	        sp += si ;
	        sl -= si ;
	        if ((cl = nextfield(sp,sl,&cp)) > 0) {
	            ch = MKCHAR(*cp) ;
	            if (isalphalatin(ch)) {
	                rs = tmstrsday(mp,ml) ;
	                op->st.tm_wday = rs ;
	                mp = cp ;
	                ml = cl ;
	                si += ((cp+cl)-sp) ;
	                sp += si ;
	                sl -= si ;
	            }
	        }
	        if (rs >= 0) {
	            rs = tmstrsmonth(mp,ml) ;
	            op->st.tm_mon = rs ;
	        }
	    } else {
	        rs = SR_INVALID ;
	    }
	} else {
	    rs = SR_INVALID ;
	}

	return (rs >= 0) ? si : rs ;
}
/* end subroutine (tmz_procmonth) */


static int tmz_procyear(TMZ *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		si = 0 ;
	int		cl ;
	cchar		*cp ;

	if ((cl = nextfield(sp,sl,&cp)) > 0) {
	    int	f = FALSE ;
	    f = f || isdigitlatin(MKCHAR(*cp)) ;
	    if (f) {
	        rs = tmstrsyear(cp,cl) ;
	        op->st.tm_year = rs ;
	        op->f.year = TRUE ;
	        si += ((cp+cl)-sp) ;
	        sp += si ;
	        sl -= si ;
	    }
	} /* end if (nextfield) */

#if	CF_DEBUGS
	debugprintf("tmz_procyear: ret rs=%d si=%u\n",rs,si) ;
	debugprintf("tmz_procyear: year=%u\n",op->st.tm_year) ;
#endif

	return (rs >= 0) ? si : rs ;
}
/* end subroutine (tmz_procyear) */


static int tmz_proczoff(TMZ *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		si = 0 ;
	int		cl ;
	cchar		*cp ;

	if ((cl = nextfield(sp,sl,&cp)) > 0) {
	    int	ch = MKCHAR(*cp) ;
	    int	f = FALSE ;
	    f = f || isplusminus(ch) ;
	    f = f || isdigitlatin(ch) ;
	    if (f) {
	        int	v ;
	        rs = getzoff(&v,cp,cl) ;
	        op->zoff = v ;
	        op->f.zoff = TRUE ;
	        si += ((cp+cl)-sp) ;
	        sp += si ;
	        sl -= si ;
	    }
	} /* end if (nextfield) */

#if	CF_DEBUGS
	debugprintf("tmz_proczoff: ret rs=%d si=%u\n",rs,si) ;
	debugprintf("tmz_proczoff: zoff=%u\n",op->zoff) ;
#endif

	return (rs >= 0) ? si : rs ;
}
/* end subroutine (tmz_proczoff) */


static int tmz_proczname(TMZ *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		si = 0 ;
	int		cl ;
	cchar		*cp ;

#if	CF_DEBUGS
	debugprintf("tmz_proczname: ent s=>%t<\n",sp,sl) ;
#endif

	if ((cl = nextfield(sp,sl,&cp)) > 0) {
	    int		ch = MKCHAR(*cp) ;
	    int		f = FALSE ;
	    f = f || isalphalatin(ch) ;
	    if (f) {
	        const int	znl = TMZ_ZNAMESIZE ;
	        rs = strnwcpy(op->zname,znl,cp,cl)  - op->zname ;
	        si += ((cp+cl)-sp) ;
	        sp += si ;
	        sl -= si ;
	    }
	} /* end if (nextfield) */

#if	CF_DEBUGS
	debugprintf("tmz_proczname: ret zn=%t\n",op->zname,TMZ_ZNAMESIZE) ;
	debugprintf("tmz_proczname: ret rs=%d si=%u\n",rs,si) ;
#endif

	return (rs >= 0) ? si : rs ;
}
/* end subroutine (tmz_proczname) */


static int tmz_yearadj(TMZ *op,int cc)
{
	struct tm	*stp = &op->st ;
#if	CF_DEBUGS
	debugprintf("tmz_yearadj: ent year=%d\n",stp->tm_year) ;
	debugprintf("tmz_yearadj: cc=%d\n",cc) ;
#endif
	if (stp->tm_year >= 0) {
	    op->f.year = TRUE ;
	    if (cc >= 0) {
	        const int	yy = ((cc*NYEARS_CENTURY)-TM_YEAR_BASE) ;
	        stp->tm_year += yy ;
	    } else {
	        if ((stp->tm_year >= 0) && (stp->tm_year <= 38)) {
	            stp->tm_year += NYEARS_CENTURY ;
	        } else if (stp->tm_year >= TM_YEAR_BASE) {
	            stp->tm_year -= TM_YEAR_BASE ;
		}
	    }
	} /* end if (had a year) */
#if	CF_DEBUGS
	debugprintf("tmz_yearadj: ret year=%d\n",stp->tm_year) ;
#endif
	return SR_OK ;
}
/* end subroutine (tmz_yearadj) */


#if	defined(CF_MULTIZONE) && (CF_MULTIZONE == 0)

/* do we have a valid time-zone name */
static int isgoodname(cchar *sp,int sl)
{
	int		ch ;
	int		f = FALSE ;

	while ((sl != 0) && (sp[0] != '\0')) {
	    ch = (*sp & 0xff) ;
	    f = isalnumlatin(ch) ;
	    if (! f) break ;
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */

	return f ;
}
/* end subroutine (isgoodname) */

#endif /* defined(CF_MULTIZONE) && (CF_MULTIZONE == 0) */


/* parse minutes west of GMT */
static int getzoff(int *zop,cchar *sp,int sl)
{
	int		rs = SR_INVALID ;
	int		cl ;
	int		zoff ;
	int		f = FALSE ;
	cchar		*cp ;

	f = f || isplusminus(MKCHAR(*sp)) ;
	f = f || isdigitlatin(MKCHAR(*sp)) ;
	if ((sl >= 2) && f) {
	    int		i, sign ;
	    int		hours, mins ;

	    rs = SR_OK ;
	    sign = ((*sp == '+') || isdigitlatin(MKCHAR(*sp))) ? -1 : 1 ;

	    cp = sp ;
	    cl = sl ;
	    if ((*sp == '-') || (*sp == '+')) {
	        cp += 1 ;
	        cl -= 1 ;
	    }

	    for (i = 0 ; 
	        (i < cl) && cp[i] && 
	        (! CHAR_ISWHITE(cp[i])) && (cp[i] != ',') ; 
	        i += 1) {

	        if (! isdigitlatin(MKCHAR(cp[i]))) {
	            rs = SR_INVALID ;
	            break ;
	        }

	    } /* end for (extra sanity check) */

/* skip over extra leading digits (usually '0' but whatever) */

	    if (i > 4) {
	        cp += (i - 4) ;
	        cl -= (i - 4) ;
	    }

/* extract hours and minutes from remaining 3 or 4 digits */

	    hours = (*cp++ - '0') ;
	    if (cl > 3) {
	        hours *= 10 ;
	        hours += (*cp++ - '0') ;
	    }

	    mins = (*cp++ - '0') * 10 ;
	    mins += (*cp++ - '0') ;

	    zoff = ((hours * 60) + mins) ;

/* reportedly, there are time zones at up to 14 hours off of GMT! */

#ifdef	COMMENT
	    if (zoff > (14 * 60))
	        rs = SR_INVALID ;
#endif

	    zoff *= sign ;
	    if (zop != NULL)
	        *zop = zoff ;

	    if (rs >= 0) {
	        rs = (cp - sp) ;
	    }

	} /* end if (getting timezone offset) */

#if	CF_DEBUGS
	debugprintf("getzoff: ret rs=%d zo=%d\n",rs,zoff) ;
#endif

	return rs ;
}
/* end subroutine (getzoff) */


static int val(cchar *sp)
{
	int		v = 0 ;
	v += (10 * (sp[0] - '0')) ;
	v += ( 1 * (sp[1] - '0')) ;
	return v ;
}
/* end subroutine (val) */


static int silogend(cchar *sp,int sl)
{
	int		i = 0 ;
	int		f = FALSE ;
#if	CF_DEBUGS
	debugprintf("tmz/silogend: ent s=>%t<\n",sp,sl) ;
#endif
	for (i = 0 ; sl-- && *sp ; i += 1) {
	    int	ch = MKCHAR(sp[i]) ;
	    f = f || (ch == '_') ;
	    f = f || isalphalatin(ch) ;
	    if (f) break ;
	} /* end while */
#if	CF_DEBUGS
	debugprintf("tmz/silogend: ret i=%d\n",i) ;
#endif
	return i ;
}
/* end subroutine (silogend) */


static int isplusminus(int ch)
{
	return ((ch == '+') || (ch == '-')) ;
}
/* end subroutine (isplusminus) */


static cchar *strnzone(cchar *sp,int sl)
{
	int		f = FALSE ;
	while (sl && sp[0]) {
	    cchar	ch = MKCHAR(*sp) ;
	    f = f || (ch == '+') ;
	    f = f || (ch == '-') ;
	    f = f || isalphalatin(ch) ;
	    if (f) break ;
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */
	return sp ;
}
/* end subroutine (strnzone) */


