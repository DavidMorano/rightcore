/* sntmtime */

/* make string of time-component values */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Ths subroutine is similar to 'sncpy1(3dam)' but it takes a broken-out
	time-specification (a la object TMTIME) and creates the corresponding
	string in the destination buffer.  A format specifiction is supplied to
	determine what the resulting string looks like.  A format string is a
	string of characters but with format-codes embedded.  The format-codes
	select for elements of the TMTIME object.  See below for a list of the
	format-codes.

	Synopsis:

	int sntmtime(dbuf,dlen,tmp,fmt)
	char		*dbuf ;
	int		dlen ;
	TMTIME		*tmp ;
	const char	*fmt ;

	Arguments:

	dbuf		destination string buffer
	dlen		destination string buffer length
	tmp		pointer to TMTIME object
	fmt		format string

	Returns:

	>=0		number of bytes in result
	<0		error

	Format codes:

	- A		full weekday name (not yet coded)
	- a		abbreviated weekday name
	- B		full month name (not yet coded)
	- b		abbreviated month name
	- C		century 00-99
	- d		day of month 01-31
	- D		short for '%m/%d/%y'
	- e		day of month 1-31 (leading space as appropriate)
	- h		abbreviated month name
	- H		24-hour 00-23
	- I		12-hour 01-24
	- j		day of year 001-366
	- m		month of year 01-12
	- M		minute 00-61 (for leap-seconds)
	- n		insert a new-line (NL) character
	- p		'am' or 'pm'
	- R		same as '%H:%M'
	- r		a 12-hour time specification w/ am-pm following
	- T		same as %H:%M:%S
	- t		insert a TAB character
	- S		seconds 00-61
	- u		day of week 1-7
	- w		day of week 0-6
	- y		year within century 00-99
	- Y		year 0000-9999
	- Z		time zone abbreviation
	- O		zone-offset ±HHMM
	- Ð		yyyy-mm-dd
	- x		dd mmm yyyy
	- X		HH:MM:SS
	- :		blinking ':' character

	Usage note:

	Provision is made in this subroutine to handle years less than 1900,
	but still greater than or equal to 0000.  In order to get those years,
	one has to supply the 'year' member of the TMTIME-object with a
	negative number (not less than -1900).

	Years greater than 9999 seem to be allowed by standards, so we
	(begrudgingly) allow them also.

	All other TMTIME-object values must be within (and assumed to be within)
	proper range.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>		/* for |abs(3c)| */
#include	<string.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<calstrs.h>
#include	<sbuf.h>
#include	<tmtime.h>
#include	<zoffparts.h>
#include	<localmisc.h>


/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(c)	((c) & 0xff)
#endif

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY	100
#endif


/* external subroutines */

extern int	ctdeci(char *,int,int) ;
extern int	ctdecpi(char *,int,int,int) ; /* provides leading zeros */


/* external variables */


/* local structures */


/* forward references */

static int	sbuf_fmtstrs(SBUF *,TMTIME *,const char *) ;
static int	sbuf_twodig(SBUF *,int) ;
static int	sbuf_digs(SBUF *,int,int,int) ;
static int	sbuf_coder(SBUF *,TMTIME *,int) ;
static int	sbuf_year(SBUF *,TMTIME *) ;
static int	sbuf_zoff(SBUF *,TMTIME *) ;
static int	sbuf_dated(SBUF *,TMTIME *) ;
static int	sbuf_dater(SBUF *,TMTIME *) ;
static int	sbuf_datex(SBUF *,TMTIME *) ;


/* local variables */

static const char	blinker[] = "\033[5m:\033[0m" ;


/* exported subroutines */


int sntmtime(char *dbuf,int dlen,TMTIME *tmp,cchar *fmt)
{
	SBUF		ss, *ssp = &ss ;
	int		rs ;
	int		rs1 ;

	if (dbuf == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;
	if (tmp == NULL) return SR_FAULT ;

	if ((rs = sbuf_start(ssp,dbuf,dlen)) >= 0) {

	    rs = sbuf_fmtstrs(&ss,tmp,fmt) ;

	    rs1 = sbuf_finish(&ss) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

	return rs ;
}
/* end subroutine (sntmtime) */


/* local subroutines */


static int sbuf_fmtstrs(SBUF *ssp,TMTIME *tmp,const char *fmt)
{
	int		rs = SR_OK ;
	cchar		**m = calstrs_months ;
	cchar		**d = calstrs_days ;

	while ((rs >= 0) && *fmt) {
	    int		ch = *fmt++ ;
	    if (ch == '%') {
	        ch = MKCHAR(*fmt++) ;
	        switch (ch) {
	        case '%':
	            rs = sbuf_char(ssp,ch) ;
	            break ;
	        case 'a':
	            rs = sbuf_strw(ssp,d[tmp->wday],3) ;
	            break ;
	        case 'b':
	        case 'h':
	            rs = sbuf_strw(ssp,m[tmp->mon],3) ;
	            break ;
	        case 'C':
	            {
			const int	nyears = NYEARS_CENTURY ;
	                int		y ;
	                y = ((tmp->year+TM_YEAR_BASE)/nyears) ;
	                rs = sbuf_twodig(ssp,y) ;
	            }
	            break ;
	        case 'd':
	            rs = sbuf_twodig(ssp,tmp->mday) ;
	            break ;
	        case 'e':
	            rs = sbuf_digs(ssp,tmp->mday,2,1) ;
	            break ;
	        case 'H':
	            rs = sbuf_twodig(ssp,tmp->hour) ;
	            break ;
	        case 'I':
	            {
	                int	h = (tmp->hour%12) ;
	                if (h == 0) h = 12 ;
	                rs = sbuf_twodig(ssp,h) ;
	            }
	            break ;
	        case 'j':
	            rs = sbuf_digs(ssp,(tmp->yday+1),3,0) ;
	            break ;
	        case 'k':
	            rs = sbuf_digs(ssp,tmp->hour,2,1) ;
	            break ;
	        case 'l':
	            {
	                int	h = (tmp->hour%12) ;
	                if (h == 0) h = 12 ;
	                rs = sbuf_digs(ssp,h,2,1) ;
	            }
	            break ;
	        case 'm':
	            rs = sbuf_twodig(ssp,(tmp->mon+1)) ;
	            break ;
	        case 'M':
	            rs = sbuf_twodig(ssp,tmp->min) ;
	            break ;
	        case 'n':
	            rs = sbuf_char(ssp,'\n') ;
	            break ;
	        case 't':
	            rs = sbuf_char(ssp,'\t') ;
	            break ;
	        case 'r': /* this is what Free-BSD does */
	            rs = sbuf_fmtstrs(ssp,tmp,"%I:%M:%S ") ;
/* FALLTHROUGH */
	        case 'p':
	            if (rs >= 0) {
	                const char	*cp = (tmp->hour < 12) ? "am" : "pm" ;
	                rs = sbuf_strw(ssp,cp,2) ;
	            }
	            break ;
	        case 'S':
	            rs = sbuf_twodig(ssp,tmp->sec) ;
	            break ;
	        case 'U':
	            {
	                int	w = ((tmp->yday + 7 - tmp->wday) / 7) ;
	                rs = sbuf_twodig(ssp,w) ;
	            }
	            break ;
	        case 'u':
	            {
	                int	d = tmp->wday ;
	                if (d == 0) d = 7 ;
	                rs = sbuf_digs(ssp,d,1,0) ;
	            }
	            break ;
	        case 'V':
	            {
	                int	w ;
	                w = (tmp->yday+10-(tmp->wday?(tmp->wday-1):6))/7 ;
	                if (w == 0) w = 53 ;
	                rs = sbuf_twodig(ssp,w) ;
	            }
	            break ;
	        case 'W':
	            {
	                int	w ;
	                w = (tmp->yday+7-(tmp->wday?(tmp->wday-1):6))/7 ;
	                rs = sbuf_twodig(ssp,w) ;
	            }
	            break ;
	        case 'w':
	            rs = sbuf_digs(ssp,tmp->wday,1,0) ;
	            break ;
	        case 'y':
	            {
	                const int	mod = NYEARS_CENTURY ;
	                int		y ;
	                y = ((tmp->year+TM_YEAR_BASE)%mod) ;
	                rs = sbuf_twodig(ssp,y) ;
	            }
	            break ;
	        case 'Y':
	            rs = sbuf_year(ssp,tmp) ;
	            break ;
	        case 'Z':
	            rs = sbuf_strw(ssp,tmp->zname,8) ;
	            break ;
	        case 'R':
	            rs = sbuf_coder(ssp,tmp,0) ;
	            break ;
	        case 'X':
	        case 'T':
	            rs = sbuf_coder(ssp,tmp,1) ;
	            break ;
	        case 'O':
	            rs = sbuf_zoff(ssp,tmp) ;
	            break ;
	        case 'D':
	            rs = sbuf_dated(ssp,tmp) ;
	            break ;
	        case MKCHAR('Ð'):
	            rs = sbuf_dater(ssp,tmp) ;
	            break ;
	        case 'x':
	            rs = sbuf_datex(ssp,tmp) ;
	            break ;
	        case ':':
	            rs = sbuf_strw(ssp,blinker,-1) ;
	            break ;
	        default:
	            rs = SR_ILSEQ ;
	            break ;
	        } /* end switch */
	    } else {
	        rs = sbuf_char(ssp,ch) ;
	    }
	} /* end while */

	return rs ;
}
/* end subroutine (sbuf_fmtstrs) */


static int sbuf_twodig(SBUF *ssp,int v)
{
	int		rs ;
	char		dbuf[2+1] ;

	dbuf[0] = (v/10) + '0' ;
	dbuf[1] = (v%10) + '0' ;
	rs = sbuf_strw(ssp,dbuf,2) ;

	return rs ;
}
/* end subroutine (sbuf_twodig) */


static int sbuf_digs(SBUF *ssp,int v,int n,int f_space)
{
	int		rs = SR_OK ;
	int		ch ;
	char		dbuf[3+1] ;

	switch (n) {
	case 1:
	    dbuf[0] = (v%10) + '0' ;
	    break ;
	case 2:
	    ch = (v/10) + '0' ;
	    dbuf[0] = ch ;
	    if ((ch == '0') && f_space) dbuf[0] = ' ' ;
	    dbuf[1] = (v%10) + '0' ;
	    break ;
	case 3:
	    rs = ctdecpi(dbuf,n,n,v) ;
	    if (f_space) {
	        int	i ;
	        for (i = 0 ; i < (n-1) ; i += 1) {
	            if (dbuf[i] == '0') {
			dbuf[i] = ' ' ;
		    } else
			break ;
	        } /* end for */
	    } /* end if (space) */
	    break ;
	} /* end switch */

	if (rs >= 0) {
	    rs = sbuf_strw(ssp,dbuf,n) ;
	}

	return rs ;
}
/* end subroutine (sbuf_digs) */


static int sbuf_year(SBUF *ssp,TMTIME *tmp)
{
	const int	y = ((tmp->year + TM_YEAR_BASE)%10000) ;
	int		rs ;
	char		dbuf[4+1] ;

	if ((rs = ctdecpi(dbuf,4,4,y)) >= 0) { /* leading zeros to 4 digits */
	    rs = sbuf_strw(ssp,dbuf,4) ;
	}

	return rs ;
}
/* end subroutine (sbuf_year) */


static int sbuf_coder(SBUF *ssp,TMTIME *tmp,int f_sec)
{
	int		rs ;

	if ((rs = sbuf_twodig(ssp,tmp->hour)) >= 0) {
	    if ((rs = sbuf_char(ssp,':')) >= 0) {
	        if ((rs = sbuf_twodig(ssp,tmp->min)) >= 0) {
		    if (f_sec) {
			if ((rs = sbuf_char(ssp,':')) >= 0) {
	        	    rs = sbuf_twodig(ssp,tmp->sec) ;
			}
		    }
		}
	    }
	}

	return rs ;
}
/* end subroutine (sbuf_coder) */


static int sbuf_zoff(SBUF *ssp,TMTIME *tmp)
{
	const int	zo = (tmp->gmtoff / 60) ; /* minutes west of GMT */
	int		rs ;
	{
	    const int	ch = ((zo >= 0) ? '-' : '+') ;
	    if ((rs = sbuf_char(ssp,ch)) >= 0) {
	        const int	zh = abs(zo / 60) % 100 ;
	        const int	zm = abs(zo % 60) ;
	        if ((rs = sbuf_twodig(ssp,zh)) >= 0) {
	    	    rs = sbuf_twodig(ssp,zm) ;
		}
	    }
	}
	return rs ;
}
/* end subroutine (sbuf_zoff) */


static int sbuf_dated(SBUF *ssp,TMTIME *tmp)
{
	int		rs ;

	if ((rs = sbuf_twodig(ssp,(tmp->mon+1))) >= 0) {
	    if ((rs = sbuf_char(ssp,'/')) >= 0) {
	        if ((rs = sbuf_twodig(ssp,tmp->mday)) >= 0) {
	    	    if ((rs = sbuf_char(ssp,'/')) >= 0) {
			const int	mod = NYEARS_CENTURY ;
	        	int		y ;
	        	y = ((tmp->year+TM_YEAR_BASE)%mod) ;
	        	rs = sbuf_twodig(ssp,y) ;
		    }
		}
	    }
	}

	return rs ;
}
/* end subroutine (sbuf_dated) */


static int sbuf_dater(SBUF *ssp,TMTIME *tmp)
{
	int		rs ;

	if ((rs = sbuf_year(ssp,tmp)) >= 0) {
	    if ((rs = sbuf_char(ssp,'-')) >= 0) {
	        if ((rs = sbuf_twodig(ssp,(tmp->mon+1))) >= 0) {
	    	    if ((rs = sbuf_char(ssp,'-')) >= 0) {
			rs = sbuf_twodig(ssp,tmp->mday) ;
		    }
		}
	    }
	}

	return rs ;
}
/* end subroutine (sbuf_dater) */


static int sbuf_datex(SBUF *ssp,TMTIME *tmp)
{
	int		rs ;

	if ((rs = sbuf_twodig(ssp,tmp->mday)) >= 0) {
	    if ((rs = sbuf_char(ssp,' ')) >= 0) {
		cchar	**m = calstrs_months ;
	        if ((rs = sbuf_strw(ssp,m[tmp->mon],3)) >= 0) {
	    	    if ((rs = sbuf_char(ssp,' ')) >= 0) {
	        	rs = sbuf_year(ssp,tmp) ;
		    }
		}
	    }
	}

	return rs ;
}
/* end subroutine (sbuf_datex) */


