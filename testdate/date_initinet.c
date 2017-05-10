/* date_initinet */

/* parse the common Internet date case (rfc 822 & 1123) *fast* */


#define	F_DEBUGS	1
#define	F_TMZ		1


/* revision history :

	= 97/06/03, David A­D­ Morano

	I modified the 'prsindate' subroutine to make it "Year 2000"
	safe.


*/


/******************************************************************************


 * If the date is in the form
 *	[Weekday,] dd Mmm [19]yy hh:mm[:ss] Timezone
 * as most dates in news articles are, then we can parse it much quicker than
 * getdate and quite a bit faster than getabsdate.
 *
 * parse and convert Internet date in timestr (the normal interface)
 

******************************************************************************/



#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include	<vsystem.h>
#include	<ascii.h>

#include	"misc.h"
#include	"date.h"
#include	"tmz.h"
#include "datetok.h"



/* local defines */

/* STREQ is an optimised strcmp(a,b)==0 */
#define STREQ(a, b) ((a)[0] == (b)[0] && strcmp(a, b) == 0)

#define	PACK_TWO_CHARS(c1, c2)	(((c1)<<8)|(c2))
#define ISSPACE(c) ((c) == ' ' || (c) == '\n' || (c) == '\t')
#define SKIPTOSPC(s) \
	while ((ch = *(s)++), (!ISSPACE(ch) && ch != '\0')) \
	    ; \
	(s)--			/* N.B.: no semi-colon */
#define SKIPSPC(s) \
	while ((ch = *(s)++), ISSPACE(ch)) \
	    ; \
	(s)--			/* N.B.: no semi-colon */
#define SKIPOVER(s) \
	SKIPTOSPC(s); \
	SKIPSPC(s)		/* N.B.: no semi-colon */

/* this is fast but dirty.  note the return's in the middle. */
#define GOBBLE_NUM(cp, c, x, ip) \
	(c) = *(cp)++; \
	if ((c) < '0' || (c) > '9') \
	    return -1;		/* missing digit */ \
	(x) = (c) - '0'; \
	(c) = *(cp)++; \
	if ((c) >= '0' && (c) <= '9') { \
	    (x) = 10*(x) + (c) - '0'; \
	    (c) = *(cp)++; \
	} \
	if ((c) != ':' && (c) != '\0' && !ISSPACE(c)) \
	    return -1;		/* missing colon */ \
	*(ip) = (x)			/* N.B.: no semi-colon here */



/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	prsindate(char *,struct tm *,char **) ;
static int	parsetime(char *,struct tm *) ;






int date_initinet(dp,s,slen)
DATE	*dp ;
char	s[] ;
int	slen ;
{
	TMZ		stz ;

	struct tm	st ;

	int	rs ;
	int	tzlen ;

	char	buf[100] ;
	char	*tznp ;


#if	F_DEBUGS
	eprintf("date_initinet: entered dp=%p\n",dp) ;
#endif

	if (dp == NULL)
	    return SR_FAULT ;

	if (slen < 0)
	    slen = strlen(s) ;

	if (slen > 99)
	    return SR_TOOBIG ;

#if	F_DEBUGS
	eprintf("date_initinet: 1 dp=%p\n",dp) ;
#endif

#if	F_TMZ
	rs = tmz_inet(&stz,s,slen) ;

#if	F_DEBUGS
	eprintf("date_initinet: 2 dp=%p\n",dp) ;

	    eprintf("date_initinet: tmz_inet() rs=%d\n",rs) ;
	eprintf(
	"date_initinet: year=%d mon=%d mday=%d hour=%d min=%d sec=%d\n",
		stz.st.tm_year,
		stz.st.tm_mon,
		stz.st.tm_mday,
		stz.st.tm_hour,
		stz.st.tm_min,
		stz.st.tm_sec) ;
	eprintf(
	"date_initinet: timezone=%d zonename=>%W<\n",
		stz.timezone,
		stz.zonename,strnlen(stz.zonename,MIN(rs,TMZ_NAMELEN))) ;

	eprintf("date_initinet: 3 entered dp=%p\n",dp) ;

#endif /* F_DEBUGS */

	if (rs < 0)
	    return SR_INVALID ;

	tzlen = rs ;

/* can we supply some missing pieces ? */

	if ((! stz.f_timezone) && (stz.zonename[0] == '\0')) {

		time_t	daytime ;


		u_time(&daytime) ;

		localtime_r(&daytime,&st) ;

#if	F_DEBUGS
	eprintf("date_std: localtime() year=%d f_dst=%d\n",
		st.tm_year,st.tm_isdst) ;
#endif

	if (stz.st.tm_year == 0)
		stz.st.tm_year = st.tm_year ;

	if (stz.zonename[0] == '\0') {

	        tznp = (st.tm_isdst <= 0) ? tzname[0] : tzname[1] ;

		tzlen = -1 ;
		strncpy(stz.zonename,tznp,TMZ_NAMELEN) ;

	}

#if	F_DEBUGS
	eprintf("date_std: adjusted year=%d timezone=%d zonename=>%W<\n",
		stz.st.tm_year,
		stz.timezone,
		stz.zonename,strnlen(stz.zonename,TMZ_NAMELEN)) ;
#endif /* F_DEBUGS */

	} /* end if (getting missing stuff) */

/* do it */

	if (stz.zonename[0] != '\0')
	rs = date_initsplitname(dp,&stz.st,stz.zonename,tzlen) ;

	else
	rs = date_inittmz(dp,&stz.st,stz.timezone) ;

#if	F_DEBUGS
	eprintf("date_initinet: date_initsplitname() rs=%d dp=%p\n",rs,dp) ;
#endif

#else
	strwcpy(buf,s,slen) ;

	rs = prsindate(buf, &st, &tznp) ;

#if	F_DEBUGS
	if (rs < 0)
	    eprintf("date_initinet: prsindate() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    return SR_INVALID ;

	tzlen = rs ;
	tznp[tzlen] = '\0' ;

#if	F_DEBUGS
	eprintf("date_initinet: tzname=>%W<\n",tznp,tzlen) ;
#endif

	rs = date_initsplitname(dp,&st,tznp,-1) ;

#endif /* F_TMZ */

#if	F_DEBUGS
	if (rs < 0)
	    eprintf("date_initinet: date_initsplitname() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (date_initinet) */



/* PRIVATE SUBROUTINES */



/* just parse the Internet date in timestr and get back a broken-out date */
static int prsindate(line, tm, tznpp)
register char *line;			/* can be modified */
register struct tm *tm ;
char		**tznpp ;
{
	int	rs ;
	int	zoneoff, *zoneoffp = &zoneoff ;
	int	c ;


	register char ch;		/* used by SKIPTOSPC */
	register char c2 ;
	register char *cp ;


	tm->tm_isdst = 0 ;
	SKIPSPC(line) ;
	if ((ch = *line) < '0' || ch > '9') {

	    cp = line ;
	    while ((ch = *cp++), (!ISSPACE(ch) && ch != ',' && ch != '\0'))
	         ;

	    cp-- ;
	    if (ch == ',') {

	        line = cp ;
	        SKIPOVER(line);		/* skip weekday */

	    } else {

#if	F_DEBUGS
	        eprintf("date_initinet/prsindate: missing comma\n") ;
#endif

	        return -1;	/* missing comma after weekday */
	    }

	}

	GOBBLE_NUM(line, ch, c, &tm->tm_mday) ;

/*
* we have to map to canonical case because RFC 822 requires
* case independence, so we pay a performance penalty for the sake
* of 0.1% of dates actually seen in Date: headers in news.
* Way to go, IETF.
*/

	ch = *line++ ;
	if (ch == '\0') {

#if	F_DEBUGS
	    eprintf("date_initinet/prsindate: no month\n") ;
#endif

	    return -1;		/* no month */
	}

#ifdef	COMMENT
	if (isascii(ch) && islower(ch))
	    ch = toupper(ch) ;
#else
	ch = toupper(ch) ;
#endif

	c2 = *line++ ;
	if (c2 == '\0') {

#if	F_DEBUGS
	    eprintf("date_initinet/prsindate: month too short\n") ;
#endif

	    return -1;		/* month too short */
	}

#ifdef	COMMENT
	if (isascii(c2) && isupper(c2))
	    c2 = tolower(c2) ;
#else
	c2 = tolower(c2) ;
#endif

	switch (PACK_TWO_CHARS(ch, c2)) {

	case PACK_TWO_CHARS('J', 'a'):
	    tm->tm_mon = 1 ;
	    break ;

	case PACK_TWO_CHARS('F', 'e'):
	    tm->tm_mon = 2 ;
	    break ;

	case PACK_TWO_CHARS('M', 'a'):	/* March, May */
	    tm->tm_mon = ((ch = *line) == 'r' || ch == 'R'? 3: 5) ;
	    break ;

	case PACK_TWO_CHARS('A', 'p'):
	    tm->tm_mon = 4 ;
	    break ;

	case PACK_TWO_CHARS('J', 'u'):
	    tm->tm_mon = 6 ;
	    if ((ch = *line) == 'l' || ch == 'L')
	        tm->tm_mon++;		/* July */

	    break ;

	case PACK_TWO_CHARS('A', 'u'):
	    tm->tm_mon = 8 ;
	    break ;

	case PACK_TWO_CHARS('S', 'e'):
	    tm->tm_mon = 9 ;
	    break ;

	case PACK_TWO_CHARS('O', 'c'):
	    tm->tm_mon = 10 ;
	    break ;

	case PACK_TWO_CHARS('N', 'o'):
	    tm->tm_mon = 11 ;
	    break ;

	case PACK_TWO_CHARS('D', 'e'):
	    tm->tm_mon = 12 ;
	    break ;

	default:

#if	F_DEBUGS
	    eprintf("date_initinet/prsindate: bad month\n") ;
#endif

	    return -1;		/* bad month name */

	} /* end switch */

	tm->tm_mon--;			/* convert month to zero-origin */
	SKIPOVER(line);			/* skip month */

	tm->tm_year = atoi(line) ;

	if (tm->tm_year < 0) {

#if	F_DEBUGS
	    eprintf("date_initinet/prsindate: year is bad\n") ;
#endif

	    return -1;		/* year is non-positive or missing */
	}

	if (tm->tm_year >= 1900)	/* convert year to 1900 origin, */
	    tm->tm_year -= 1900;	/* but 2-digit years need no work */

	if (tm->tm_year < 70)
	    tm->tm_year += 100 ;

	SKIPOVER(line);			/* skip year */

	if (parsetime(line, tm) < 0) {

#if	F_DEBUGS
	    eprintf("date_initinet/prsindate: parsetime() bad\n") ;
#endif

	    return -1 ;
	}

	SKIPOVER(line);			/* skip time */

	cp = line ;
	if (*cp++ == 'G' && *cp++ == 'M' && *cp++ == 'T' &&
	    (*cp == '\n' || *cp == '\0')) {

	    *zoneoffp = 0 ;
	    *tznpp = (cp - 3) ;
	    rs = 3 ;

	} else {				/* weirdo time zone */

	    register datetkn *tp ;


	    *tznpp = line ;

	    cp = line ;		/* time zone start */
	    SKIPTOSPC(line) ;
	    c = *line;		/* save old delimiter */
	    *line = '\0';		/* terminate time zone */

	    rs = (line - cp) ;

	    tp = datetoktype(cp, (int *)NULL) ;

	    switch (tp->type) {

	    case DTZ:
#if 0
	        tm->tm_isdst++ ;
#endif
/* FALLTHROUGH */

	    case TZ:
	        *zoneoffp = FROMVAL(tp) ;
/* FALLTHROUGH */

	    case IGNORE:
	        break ;

	    default:

#if	F_DEBUGS
	        eprintf("date_initinet/prsindate: bad token type\n") ;
#endif

	        return -1;	/* bad token type */

	    } /* end switch */

	    *line = c;		/* restore old delimiter */
	    SKIPSPC(line) ;
	    if (*line != '\0') {	/* garbage after the date? */

	        if (*line != CH_LPAREN) { /* not even an 822 comment? */

#if	F_DEBUGS
	            eprintf("date_initinet/prsindate: not RFC822 comment\n") ;
#endif

	            return -1 ;
	        }

/*
* a full 822 parse of the comment would
* be ridiculously complicated, so nested
* comments and quotes are not honoured.
* just look for a closing paren; it's only
* a time zone name.
*/

	        while ((c = *++line) != CH_RPAREN && c != '\0')
	             ;

	        if (c == CH_RPAREN)
	            ++line ;

	        else {

#if	F_DEBUGS
	            eprintf("date_initinet/prsindate: not terminated\n") ;
#endif

	            return -1;	/* comment not terminated */
	        }

	        SKIPSPC(line) ;
	        if (*line != '\0') {	/* trash left? */

#if	F_DEBUGS
	            eprintf("date_initinet/prsindate: trash left\n") ;
#endif

	            return -1 ;
	        }

	    }

	} /* end if (GMT or other) */

	return rs ;
}
/* end subroutine (prsindate) */


/* return -1 on failure */
static int parsetime(time, tm)
register char *time ;
register struct tm *tm ;
{
	register char c ;
	register int x ;


	tm->tm_sec = 0 ;
	GOBBLE_NUM(time, c, x, &tm->tm_hour) ;
	if (c != ':')
	    return -1;		/* only hour; too short */

	GOBBLE_NUM(time, c, x, &tm->tm_min) ;
	if (c != ':')
	    return 0;		/* no seconds; okay */

	GOBBLE_NUM(time, c, x, &tm->tm_sec) ;

/* this may be considered too strict.  garbage at end of time? */

	return (((c == '\0') || ISSPACE(c)) ? 0 : -1) ;
}
/* end subroutine (parsetime) */



