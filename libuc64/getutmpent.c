/* getutmpent */

/* get user information from UTMP database */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_TMPX		0		/* try TMPX */
#define	CF_UTMPX	1		/* try UTMPX (otherwise UTMP) */
#define	CF_FETCHPID	1		/* use 'fetchpid()' where possible */
#define	CF_UTMPACC	1		/* use |ugetutmpacc()| */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Get the user login name from the UTMP database.

	Synopsis:

	int getutmpname(rbuf,rlen,sid)
	char	rbuf[] ;
	int	rlen ;
	pid_t	sid ;

	int getutmphost(rbuf,rlen,sid)
	char	rbuf[] ;
	int	rlen ;
	pid_t	sid ;

	int getutmpline(rbuf,rlen,sid)
	char	rbuf[] ;
	int	rlen ;
	pid_t	sid ;

	Arguments:

	rbuf		buffer to hold result
	rlen		length of user supplied buffer
	sid		session ID to lookup

	Returns:

	>=0		length of user buf
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<utmp.h>

#if	CF_UTMPX && defined(SYSHAS_UTMPX) && (SYSHAS_UTMPX > 0)
#include	<utmpx.h>
#endif

#include	<vsystem.h>

#if	CF_TMPX || CF_UTMPX
#include	<tmpx.h>
#endif

#include	<utmpacc.h>
#include	<localmisc.h>

#include	"getutmpent.h"


/* local defines */

#define	AEBUFLEN	sizeof(struct utmpx)


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int getutmpent_utmpacc(GETUTMPENT *,pid_t) ;
static int getutmpent_tmpx(GETUTMPENT *,pid_t) ;
static int getutmpent_utmpx(GETUTMPENT *,pid_t) ;
static int getutmpent_utmp(GETUTMPENT *,pid_t) ;


/* exported subroutines */


int getutmpent(GETUTMPENT *ep,pid_t sid)
{
	int		rs = SR_NOTFOUND ;

	if (ep == NULL) return SR_FAULT ;

	ep->id[0] = '\0' ;
	ep->line[0] = '\0' ;
	ep->user[0] = '\0' ;
	ep->host[0] = '\0' ;
	ep->session = 0 ;
	ep->date = 0 ;

	if (sid <= 0) sid = getsid(0) ;

	ep->sid = sid ;

/* do the DB search */

#if	CF_UTMPACC
	if (rs == SR_NOTFOUND) {
	    rs = getutmpent_utmpacc(ep,sid) ;
	}
#else /* CF_UTMPACC */
#if	CF_TMPX
	if (rs == SR_NOTFOUND) {
	    rs = getutmpent_tmpx(ep,sid) ;
	}
#else /* CF_TMPX */
#if	CF_UTMPX && defined(SYSHAS_UTMPX) && (SYSHAS_UTMPX > 0)
	if (rs == SR_NOTFOUND) {
	    rs = getutmpent_utmpx(ep,sid) ;
	}
#else /* CF_UTMPX */
	if (rs == SR_NOTFOUND) {
	    rs = getutmpent_utmp(ep,sid) ;
	}
#endif /* CF_UTMPX */
#endif /* CF_TMPX */
#endif /* CF_UTMPACC */

	return rs ;
}
/* end subroutine (getutmpent) */


int getutmpname(char *rbuf,int rlen,pid_t sid)
{
	GETUTMPENT	e ;
	int		rs ;

	if (rbuf == NULL) return SR_FAULT ;

	if (rlen < 0) rlen = GETUTMPENT_LUSER ;

	rbuf[0] = '\0' ;
	if ((rs = getutmpent(&e,sid)) >= 0)
	    rs = sncpy1(rbuf,rlen,e.user) ;

	return rs ;
}
/* end subroutine (getutmpname) */


int getutmphost(char *rbuf,int rlen,pid_t sid)
{
	GETUTMPENT	e ;
	int		rs ;

	if (rbuf == NULL) return SR_FAULT ;

	if (rlen < 0) rlen = GETUTMPENT_LHOST ;

	rbuf[0] = '\0' ;
	if ((rs = getutmpent(&e,sid)) >= 0)
	    rs = sncpy1(rbuf,rlen,e.host) ;

	return rs ;
}
/* end subroutine (getutmphost) */


int getutmpline(char *rbuf,int rlen,pid_t sid)
{
	GETUTMPENT	e ;
	int		rs ;

	if (rbuf == NULL) return SR_FAULT ;

	if (rlen < 0) rlen = GETUTMPENT_LLINE ;

	rbuf[0] = '\0' ;
	if ((rs = getutmpent(&e,sid)) >= 0)
	    rs = sncpy1(rbuf,rlen,e.line) ;

	return rs ;
}
/* end subroutine (getutmpline) */


/* local subroutines */


#if	CF_UTMPACC
static int getutmpent_utmpacc(GETUTMPENT *ep,pid_t sid)
{
	UTMPACC_ENT	ae ;
	const int	aelen = AEBUFLEN ;
	int		rs ;
	char		aebuf[AEBUFLEN+1] ;

	if ((rs = utmpacc_entsid(&ae,aebuf,aelen,sid)) >= 0) {
	    strwcpy(ep->id,ae.id,MIN(GETUTMPENT_LID,TMPX_LID)) ;
	    strwcpy(ep->user,ae.user,MIN(GETUTMPENT_LUSER,TMPX_LUSER)) ;
	    strwcpy(ep->line,ae.line,MIN(GETUTMPENT_LLINE,TMPX_LLINE)) ;
	    strwcpy(ep->host,ae.host,MIN(GETUTMPENT_LHOST,TMPX_LHOST)) ;
	    ep->session = ae.session ;
	    ep->date = ae.ctime ;
	    ep->sid = ae.sid ;
	} /* end if (utmpacc_entsid) */

	return rs ;
}
/* end subroutine (getutmpent_utmpacc) */
#else /* CF_UTMPACC */
#if	CF_TMPX

static int getutmpent_tmpx(GETUTMPENT *ep,pid_t sid)
{
	TMPX		db ;
	TMPX_ENT	e ;
	int		rs ;
	int		c = 0 ;

	if ((rs = tmpx_open(&db,NULL,O_RDONLY)) >= 0) {

#if	CF_FETCHPID
	    {
	    rs = tmpx_fetchpid(&db,&e,sid) ;
	    c = rs ;
	    }
#else /* CF_FETCHPID */
	    {
	        TMPX_CUR	cur ;

	        if ((rs = tmpx_curbegin(&db,&cur)) >= 0) {

	            while ((rs = tmpx_enum(&db,&cur,&e)) >= 0) {

	                if ((e.ut_type == TMPX_TUSERPROC) &&
	                    (e.ut_pid == sid))
	                    break ;

	                c += 1 ;

	            } /* end while */

	            tmpx_curend(&db,&cur) ;
	        } /* end if (cursor) */

	    } /* end block */
#endif /* CF_FETCHPID */

	    if (rs >= 0) {

	        strwcpy(ep->id,e.ut_id,MIN(GETUTMPENT_LID,TMPX_LID)) ;

	        strwcpy(ep->line,e.ut_line,MIN(GETUTMPENT_LLINE,TMPX_LLINE)) ;

	        strwcpy(ep->user,e.ut_user,MIN(GETUTMPENT_LUSER,TMPX_LUSER)) ;

	        strwcpy(ep->host,e.ut_host,MIN(GETUTMPENT_LHOST,TMPX_LHOST)) ;

	        ep->session = e.ut_session ;
	        ep->date = e.ut_tv.tv_sec ;

	    } /* end if (opened the TMPX DB) */

	    tmpx_close(&db) ;
	} /* end if (opened TMPX DB) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (getutmpent_tmpx) */

#endif /* CF_TMPX */


#if	CF_UTMPX && defined(SYSHAS_UTMPX) && (SYSHAS_UTMPX > 0)

static int getutmpent_utmpx(GETUTMPENT *ep,pid_t sid)
{
	struct utmpx	*up ;
	int		rs = SR_NOTFOUND ;
	int		c = 0 ;

	setutxent() ;

	while ((up = getutxent()) != NULL) {

	    if ((up->ut_type == UTMPX_TUSERPROC) &&
	        (up->ut_pid == sid)) {
	        rs = SR_OK ;
	        break ;
	    }

	    c += 1 ;

	} /* end while */

	if (rs >= 0) {

	    strwcpy(ep->id,up->ut_id,MIN(GETUTMPENT_LID,UTMPX_LID)) ;

	    strwcpy(ep->line,up->ut_line,MIN(GETUTMPENT_LLINE,UTMPX_LLINE)) ;

	    strwcpy(ep->user,up->ut_user,MIN(GETUTMPENT_LUSER,UTMPX_LUSER)) ;

	    strwcpy(ep->host,up->ut_host,MIN(GETUTMPENT_LHOST,UTMPX_LHOST)) ;

	    ep->session = up->ut_session ;
	    ep->date = up->ut_tv.tv_sec ;

#if	CF_DEBUGS
	    debugprintf("getutmpent/_utmpx: session=%d\n",ep->session) ;
#endif

	} /* end if */

	endutxent() ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (getutmpent_utmpx) */

#else /* CF_UTMPX */

static int getutmpent_utmp(GETUTMPENT *ep,pid_t sid)
{
	struct utmp	*up ;
	int		rs = SR_NOTFOUND ;
	int		c = 0 ;

	setutent() ;

	while ((up = getutent()) != NULL) {

	    if ((up->ut_type == UTMP_TUSERPROC) &&
	        (up->ut_pid == sid)) {
	        rs = SR_OK ;
	        break ;
	    }

	    c += 1 ;

	} /* end while */

	if (rs >= 0) {

	    strwcpy(ep->id,up->ut_id,MIN(GETUTMPENT_LID,UTMP_LID)) ;

	    strwcpy(ep->line,up->ut_line,MIN(GETUTMPENT_LLINE,UTMP_LLINE)) ;

	    strwcpy(ep->user,up->ut_user,MIN(GETUTMPENT_LUSER,UTMP_LUSER)) ;

#ifdef	COMMENT /* does not exist! */
	    strwcpy(ep->host,up->ut_host,MIN(GETUTMPENT_LHOST,UTMPX_LHOST)) ;
#endif

	    ep->date = e.ut_time ;

	} /* end if */

	endutent() ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (getutmpent_utmp) */

#endif /* CF_UTMPX */
#endif /* CF_UTMPACC */


