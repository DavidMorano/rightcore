/* mailmsgenv */

/* manage an UNIX® message envelope */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_GETTIME	0		/* allow getting time? */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This is a small collection of subroutines that form a simple object for
        parsing and storing the elements of an UNIX® message envelope.

        There are many (too many) subroutines out there that detect, parse, or
        otherwise process UNIX® message envelope information. I really wish that
        another set of subroutines was not needed. But, no, we need to do just
        that simple little thing that nobody else currently does! Isn't this the
        way?!


******************************************************************************/


#define	MAILMSGENV_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/timeb.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<char.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"mailmsgenv.h"


/* local defines */

#define	SPACETAB(c)	(((c) == ' ') || ((c) == '\t'))

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

#if	CF_GETTIME
extern time_t	getabsdate(const char *,struct timeb *) ;
#endif

extern int	sisub(const char *,int,const char *) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	isalphalatin(int) ;
extern int	isalnumlatin(int) ;
extern int	isdigitlatin(int) ;

extern char	*timestr_edate(time_t, char *) ;


/* external variables */


/* local structures */


/* forward references */

#if	CF_GETTIME
static int	mailmsgenv_proctime(MAILMSGENV *) ;
#endif /* CF_GETTIME */

static int	freeit(const char **) ;


/* local variables */

static const char	*months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	NULL
} ;

static const char	*days[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", NULL
} ;


/* exported subroutines */


int mailmsgenv_start(MAILMSGENV *mep,cchar *sbuf,int slen)
{
	int		rs = SR_OK ;
	int		i, ll ;
	int		ch ;
	int		datelen = 0 ; /* ¥ GCC false complaint */
	int		alen = 0 ;
	int		f_len, f_esc ;
	const char	*cp ;
	const char	*p_address = NULL ;
	const char	*p_date = NULL ;
	const char	*p_remote = NULL ;
	const char	*pd_day, *pd_month ;
	const char	*cp2 ;
	const char	*ccp ;

	if (mep == NULL) return SR_FAULT ;
	if (sbuf == NULL) return SR_FAULT ;

	memset(mep,0,sizeof(MAILMSGENV)) ;

	mep->address = NULL ;
	mep->origdate = NULL ;
	mep->remote = NULL ;
	mep->tzname = NULL ;
	mep->daytime = 0 ;
	mep->alen = -1 ;

#if	CF_DEBUGS
	if (slen >= 0) {
	    debugprintf("mailmsgenv_start: ent> %t\n",sbuf,slen) ;
	} else
	    debugprintf("mailmsgenv_start: ent> %s\n",sbuf) ;
#endif

	if ((! (f_esc = (strncmp(sbuf,">From ",6) == 0))) &&
	    (strncmp(sbuf,"From ",5) != 0)) {
	    rs = SR_INVALID ;
	    goto bad0 ;
	}

	mep->f.start = (! f_esc) ;

#if	CF_DEBUGS
	debugprintf("mailmsgenv_start: match so far\n") ;
#endif

	f_len = (slen >= 0) ;
	cp = (sbuf + 5) ;
	slen -= 5 ;
	if (f_esc) {
	    cp += 1 ;
	    slen -= 1 ;
	}

/* skip the white space after the first token */

	while (((! f_len) || (slen > 0)) && CHAR_ISWHITE(*cp)) {
	    cp += 1 ;
	    slen -= 1 ;
	} /* end while */

	if (cp[0] == ':') {
	    rs = SR_INVALID ;
	    goto bad0 ;
	}

	p_address = cp ;

/* skip the envelope address */

	while ((((! f_len) && (*cp != '\0')) || (slen > 0)) && 
	    (! CHAR_ISWHITE(*cp))) {
	    cp += 1 ;
	    slen -= 1 ;
	} /* end while */

	alen = cp - p_address ;

#if	CF_DEBUGS
	debugprintf("mailmsgenv_start: alen=%d\n",alen) ;
#endif

/* handle the case of the old fashioned "short" envelopes! */

/*
	Short envelopes only had the leading "From " portion
	followed by the envelope address, which was always
	just a simple user name (no non-alphanumerics).
*/

	if ((f_len && (slen <= 0)) || (*cp == '\0')) {

/* check if the address looks OK */

	    ch = (p_address[0] & 0xff) ;
	    if (! isalphalatin(ch))
	        rs = SR_INVALID ;

	    for (i = 1 ; (rs >= 0) && (i < (cp - p_address)) ; i += 1) {

	        ch = (p_address[i] & 0xff) ;
	        if (! isalnumlatin(ch))
	            rs = SR_INVALID ;

	    } /* end for */

	    if (rs < 0)
	        goto bad0 ;

	    goto load ;

	} /* end if (possible early return) */

#if	CF_DEBUGS
	debugprintf("mailmsgenv_start: long haul\n") ;
#endif

/* get the field that is supposed to have the day of the week */

	datelen = slen ;		/* length remaining at this point */
	if ((ll = nextfield(cp,slen,&pd_day)) != 3) {
	    rs = SR_INVALID ;
	    goto bad0 ;
	}

	p_date = pd_day ;
	slen -= (pd_day + ll - cp) ;

	cp = pd_day + ll ;
	if (matcasestr(days,pd_day,3) < 0) {
	    rs = SR_INVALID ;
	    goto bad0 ;
	}

/* get the field that is supposed to have the month in it */

	if ((ll = nextfield(cp,slen,&pd_month)) != 3) {
	    rs = SR_INVALID ;
	    goto bad0 ;
	}

	slen -= (pd_month + ll - cp) ;
	cp = pd_month + ll ;
	if (matcasestr(months,pd_month,3) < 0) {
	    rs = SR_INVALID ;
	    goto bad0 ;
	}

/* does the next field contain only digits? */

	if ((ll = nextfield(cp,slen,&cp2)) > 2)
	    rs = SR_INVALID ;

	for (i = 0 ; (rs >= 0) && (i < ll) ; i += 1) {
	    ch = (cp2[i] & 0xff) ;
	    if (! isdigitlatin(ch))
	        rs = SR_INVALID ;
	}

	if (rs < 0)
	    goto bad0 ;

/* the next field should have one or more colons in it (the rest digits) */

	slen -= (cp2 + ll - cp) ;
	cp = cp2 + ll ;
	if ((ll = nextfield(cp,slen,&cp2)) < 4) {
	    rs = SR_INVALID ;
	    goto bad0 ;
	}

	ch = (cp2[0] & 0xff) ;
	if ((! isdigitlatin(ch)) || (strchr(cp2,':') == NULL)) {
	    rs = SR_INVALID ;
	    goto bad0 ;
	}

/* if we have gotten this far OK, then we assume this IS an envelope! */
load:

#if	CF_DEBUGS
	debugprintf("mailmsgenv_start: loading\n") ;
#endif

	rs = uc_mallocstrw(p_address,alen,&ccp) ;
	if (rs < 0) goto bad0 ;

	mep->address = ccp ;
	if (p_date != NULL) {

	    if ((i = sisub(p_date,datelen,"remote from")) >= 0) {
	        int	sl ;

#if	CF_DEBUGS
	        debugprintf("mailmsgenv_start: have remote machine\n") ;
#endif

	        sl = sfshrink((p_date + i + 11),(datelen - i - 11),
	            &p_remote) ;

	        datelen = i ;
	        rs = uc_mallocstrw(p_remote,sl,&ccp) ;
	        if (rs < 0) goto bad1 ;

	        mep->remote = ccp ;

	    } /* end if (remote) */

/* clean up the end of the date string so far (the front is in good shape) */

	    while ((datelen > 0) && CHAR_ISWHITE(p_date[datelen - 1]))
	        datelen -= 1 ;

#if	CF_DEBUGS
	    debugprintf("mailmsgenv_start: datelen=%d date=>%t<\n",
	        datelen,p_date,datelen) ;
#endif

	    rs = uc_mallocstrw(p_date,datelen,&ccp) ;
	    if (rs < 0) goto bad2 ;

	    mep->origdate = ccp ;

	} /* end if (it had a date) */

	mep->alen = alen ;

ret0:
	return (rs >= 0) ? alen : rs ;

/* handle bad things */
bad2:
	if (mep->remote != NULL) {
	    uc_free(mep->remote) ;
	    mep->remote = NULL ;
	}

bad1:
	if (mep->address != NULL) {
	    uc_free(mep->address) ;
	    mep->address = NULL ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (mailmsgenv_start) */


/* test for the start of a mesage by the existence of a UNIX® envelope */
int mailmsgenv_isstart(MAILMSGENV *mep)
{

	if (mep == NULL) return SR_FAULT ;

	if (mep->address == NULL) return SR_NOTOPEN ;

	return (mep->f.start) ;
}
/* end subroutine (mailmsgenv_isstart) */


int mailmsgenv_getaddress(MAILMSGENV *mep,cchar **app)
{

	if (mep == NULL) return SR_FAULT ;

	if (mep->address == NULL) return SR_NOTOPEN ;

	if (app != NULL) {
	    *app = mep->address ;
	}

	return mep->alen ;
}
/* end subroutine (mailmsgenv_getaddress) */


int mailmsgenv_getremote(MAILMSGENV *mep,cchar **app)
{
	int		rs = SR_OK ;

	if (mep == NULL) return SR_FAULT ;

	if (mep->address == NULL) return SR_NOTOPEN ;

	if (app != NULL) {
	    *app = mep->remote ;
	}

	if (mep->remote != NULL) {
	    rs = strlen(mep->remote) ;
	}

	return rs ;
}
/* end subroutine (mailmsgenv_getremote) */


/* get the timezone string if there was one */
int mailmsgenv_gettzname(MAILMSGENV *mep,cchar **app)
{

	if (mep == NULL) return SR_FAULT ;

	if (mep->address == NULL) return SR_NOTOPEN ;

	if (app != NULL) {
	    *app = mep->tzname ;
	}

	return mep->alen ;
}
/* end subroutine (mailmsgenv_gettzname) */


#if	CF_GETTIME

int mailmsgenv_gettime(MAILMSGENV *mep,time_t *timep)
{
	time_t		d ;

	if (mep == NULL) return SR_FAULT ;

	if (mep->address == NULL) return SR_NOTOPEN ;

	if (! mep->f.init_time) {
	    mailmsgenv_proctime(mep) ;
	}

	if (timep != NULL) {
	    *timep = mep->daytime ;
	}

	return (mep->f.hastime) ;
}
/* end subroutine (mailmsgenv_gettime) */

#endif /* CF_GETTIME */


#if	CF_GETTIME

/* make a date string (suitable for use in an evelope) */
int mailmsgenv_mkdatestr(mep,name,buf,buflen)
MAILMSGENV	*mep ;
const char	name[] ;
char		buf[] ;
int		buflen ;
{

	if (mep == NULL) return SR_FAULT ;
	if (buf == NULL) return SR_FAULT ;

	if (mep->address == NULL) return SR_NOTOPEN ;

	if ((buflen >= 0) && (buflen < 29)) return SR_TOOBIG ;

	if (! mep->f.init_time) {
	    mailmsgenv_proctime(mep) ;
	}

	if (mep->f.hastime) {
	    timestr_edate(mep->daytime,buf) ;
	} else
	    buf[0] = '\0' ;

	return (mep->f.hastime) ;
}
/* end subroutine (mailmsgenv_datestr) */

#endif /* CF_GETTIME */


/* make an entire evelope out of this object */
int mailmsgenv_mkenv(MAILMSGENV *mep,char *rbuf,int rlen)
{
	SBUF		ub ;
	int		rs ;
	int		rs1 ;

	if (mep == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (mep->address == NULL) return SR_NOTOPEN ;

	if ((rs = sbuf_start(&ub,rbuf,rlen)) >= 0) {

/* the address */

	    sbuf_strw(&ub,"From ",5) ;
	    sbuf_strw(&ub,mep->address,-1) ;

/* the date string */

	    sbuf_char(&ub,' ') ;

	    {
	        char	timebuf[TIMEBUFLEN + 1] ;
	        time_t	d = (mep->f.hastime) ? mep->daytime : time(NULL) ;
	        timestr_edate(d,timebuf) ;
	        sbuf_strw(&ub,timebuf,-1) ;
	    }

/* the optional remote machine */

	    if ((mep->remote != NULL) && (mep->remote[0] != '\0')) {
	        sbuf_strw(&ub," remote from ",-1) ;
	        sbuf_strw(&ub,mep->remote,-1) ;
	    } /* end if */

	    sbuf_char(&ub,'\n') ;

	    rs1 = sbuf_finish(&ub) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return rs ;
}
/* end subroutine (mailmsgenv_mkenv) */


int mailmsgenv_finish(MAILMSGENV *mep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mep == NULL) return SR_FAULT ;

	if (mep->address == NULL) return SR_NOTOPEN ;

	mep->alen = -1 ;
	rs1 = freeit(&mep->address) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = freeit(&mep->origdate) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = freeit(&mep->remote) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = freeit(&mep->tzname) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (mailmsgenv_finish) */


/* private subroutines */


#if	CF_GETTIME

static int mailmsgenv_proctime(MAILMSGENV *mep)
{

	if (! mep->f.init_time) {
	    time_t	d ;

	    mep->f.init_time = TRUE ;
	    if (mep->origdate != NULL) {
	        if ((d = getabsdate(mep->origdate,NULL)) >= 0) {
	            mep->f.hastime = TRUE ;
	            mep->daytime = d ;
	        }
	    } /* end if */

	} /* end if (filled in the time) */

	return (mep->f.hastime) ;
}
/* end subroutine (mailmsgenv_proctime) */

#endif /* CF_GETTIME */


static int freeit(cchar **pp)
{
	int		rs = SR_OK ;
	if (*pp != NULL) {
	    rs = uc_free(*pp) ;
	    *pp = NULL ;
	}
	return rs ;
}
/* end subroutine (freeit) */


