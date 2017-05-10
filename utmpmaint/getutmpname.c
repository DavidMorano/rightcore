/* getutmpname */

/* get user information from UTMP database */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_TMPX		1		/* try TMPX */
#define	CF_UTMPX	1		/* try UTMPX (otherwise UTMP) */
#define	CF_FETCHPID	1		/* use 'fetchpid()' where possible */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Get the user login name from the UTMP database.

	Synopsis:

	int getutmpname(rbuf,rlen,sid)
	char		rbuf[] ;
	int		rlen ;
	pid_t		sid ;

	Arguments:

	sid		session ID to lookup
	rbuf		buffer to hold resulting buf
	rlen		length of user supplied buffer

	Returns:

	>=0		length of user buf
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
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

#include	<localmisc.h>


/* local defines */

#ifndef	LOGNAMELEN
#ifdef	LOGNAME_MAX
#define	LOGNAMELEN	LOGNAME_MAX
#else
#define	LOGNAMELEN	32
#endif
#endif

#ifndef	TMPX_TEMPTY
#define	TMPX_TEMPTY		0	/* entry is unused */
#define	TMPX_TRUNLEVEL		1
#define	TMPX_TBOOTTIME		2
#define	TMPX_TOLDTIME		3
#define	TMPX_TNEWTIME		4
#define	TMPX_TINITPROC		5	/* process spawned by "init" */
#define	TMPX_TLOGINPROC		6	/* a "getty" waiting for login */
#define	TMPX_TUSERPROC		7	/* a regular user process */
#define	TMPX_TDEADPROC		8	/* used in WTMP only? */
#define	TMPX_TACCOUNT		9	/* used in WTMPX only? */
#define	TMPX_TSIGNATURE		10	/* used in WTMPX only? */
#endif

#ifndef	UTMPX_LUSER
#define	UTMPX_LUSER		32
#endif

#ifndef	UTMP_LUSER
#define	UTMP_LUSER		8
#endif


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

enum getutmp {
	getutmp_id,
	getutmp_line,
	getutmp_user,
	getutmp_host,
} ;

#ifndef	GETUTMP_ID
#define	GETUTMP_ID	0
#define	GETUTMP_LINE	1
#define	GETUTMP_USER	2
#define	GETUTMP_HOST	3
#endif


/* forward references */

static int get_tmpx(char *,int,pid_t,int) ;
static int get_utmpx(char *,int,pid_t,int) ;
static int get_utmp(char *,int,pid_t,int) ;


/* exported subroutines */


int getutmpname(char *rbuf,int rlen,pid_t sid)
{
	const int	type = GETUTMP_USER ;
	int		rs = SR_OK ;

	if (rbuf == NULL) return SR_FAULT ;

	rbuf[0] = '\0' ;
	if (sid <= 0) {
	    rs = u_getsid(0) ;
	    sid = rs ;
	}

	if (rs < 0)
	    goto ret0 ;

	if (rlen < 0)
	    rlen = LOGNAMELEN ;

	rs = SR_NOENT ;

/* do the DB search */

#if	CF_TMPX
	if (rs < 0) {
	    rs = get_tmpx(rbuf,rlen,sid,type) ;
	}
#endif /* CF_TMPX */

#if	CF_UTMPX && defined(SYSHAS_UTMPX) && (SYSHAS_UTMPX > 0)
	if (rs < 0) {
	    rs = get_utmpx(rbuf,rlen,sid,type) ;
	}
#else /* CF_UTMPX */
	if (rs < 0) {
	    rs = get_utmp(rbuf,rlen,sid,type) ;
	}
#endif /* CF_UTMPX */

/* get out */
ret0:

#if	CF_DEBUGS
	debugprintf("getutmpname: ret rs=%d rbuf=%s\n",rs,rbuf) ;
#endif

	return rs ;
}
/* end subroutine (getutmpname) */


int getutmphost(char *rbuf,int rlen,pid_t sid)
{
	const int	type = GETUTMP_HOST ;
	int		rs = SR_OK ;

	if (rbuf == NULL) return SR_FAULT ;

	rbuf[0] = '\0' ;
	if (sid <= 0) {
	    rs = u_getsid(0) ;
	    sid = rs ;
	}

	if (rs < 0)
	    goto ret0 ;

	if (rlen < 0) rlen = LOGNAMELEN ;

	rs = SR_NOENT ;

/* do the DB search */

#if	CF_TMPX
	if (rs < 0) {
	    rs = get_tmpx(rbuf,rlen,sid,type) ;
	}
#endif /* CF_TMPX */

#if	CF_UTMPX && defined(SYSHAS_UTMPX) && (SYSHAS_UTMPX > 0)
	if (rs < 0) {
	    rs = get_utmpx(rbuf,rlen,sid,type) ;
	}
#else /* CF_UTMPX */
	if (rs < 0) {
	    rs = get_utmp(rbuf,rlen,sid,type) ;
	}
#endif /* CF_UTMPX */

/* get out */
ret0:

#if	CF_DEBUGS
	debugprintf("getutmphost: ret rs=%d rbuf=%s\n",rs,rbuf) ;
#endif

	return rs ;
}
/* end subroutine (getutmphost) */


/* local subroutines */


#if	CF_TMPX

static int get_tmpx(char *rbuf,int rlen,pid_t sid,int type)
{
	TMPX		db ;
	TMPX_CUR	cur ;
	TMPX_ENT	e ;
	int		rs ;
	int		rs1 ;
	int		cl = 0 ;

	rbuf[0] = '\0' ;
	if ((rs = tmpx_open(&db,NULL,O_RDONLY)) >= 0) {

#if	CF_FETCHPID
	    {
	    rs = tmpx_fetchpid(&db,&e,sid) ;
	    }
#else /* CF_FETCHPID
	    if ((rs = tmpx_curbegin(&db,&cur)) >= 0) {
	        while ((rs = tmpx_enum(&db,&cur,&e)) >= 0) {
	            if ((e.ut_type == TMPX_TUSERPROC) &&
	                (e.ut_pid == sid))
	                break ;
	        } /* end while */
	        tmpx_curend(&db,&cur) ;
	    }
#endif /* CF_FETCHPID */

	    if (rs >= 0) {
		switch (type) {
		case GETUTMP_ID:
	            cl = MIN(rlen,UTMPX_LID) ;
	            cl = strwcpy(rbuf,e.ut_id,cl) - buf ;
		    break ;
		case GETUTMP_LINE:
	            cl = MIN(rlen,UTMPX_LLINE) ;
	            cl = strwcpy(rbuf,e.ut_line,cl) - buf ;
		    break ;
		case GETUTMP_USER:
	            cl = MIN(rlen,UTMPX_LUSER) ;
	            cl = strwcpy(rbuf,e.ut_user,cl) - buf ;
		    break ;
		case GETUTMP_HOST:
	            cl = MIN(rlen,UTMPX_LHOST) ;
	            cl = strwcpy(rbuf,e.ut_host,cl) - buf ;
		    break ;
		} /* end switch */
	    } /* end if (opened the TMPX DB) */

	    tmpx_close(&db) ;
	} /* end if (opened TMPX DB) */

	return (rs >= 0) ? cl : rs ;
}
/* end subroutine (get_tmpx) */

#endif /* CF_TMPX */


#if	CF_UTMPX && defined(SYSHAS_UTMPX) && (SYSHAS_UTMPX > 0)

static int get_utmpx(char *rbuf,int rlen,pid_t sid,int type)
{
	struct utmpx	*up ;
	int		rs = SR_NOTFOUND ;
	int		cl = 0 ;

	rbuf[0] = '\0' ;
	setutxent() ;

	while ((up = getutxent()) != NULL) {
	    if ((up->ut_type == UTMPX_TUSERPROC) &&
	        (up->ut_pid == sid)) {
		rs = SR_OK ;
	        break ;
	    } /* end if */
	} /* end while */

	if (rs >= 0) {
		switch (type) {
		case GETUTMP_ID:
	            cl = MIN(rlen,UTMPX_LID) ;
	            cl = strwcpy(rbuf,up->ut_id,cl) - rbuf ;
		    break ;
		case GETUTMP_LINE:
	            cl = MIN(rlen,UTMPX_LLINE) ;
	            cl = strwcpy(rbuf,up->ut_line,cl) - rbuf ;
		    break ;
		case GETUTMP_USER:
	            cl = MIN(rlen,UTMPX_LUSER) ;
	            cl = strwcpy(rbuf,up->ut_user,cl) - rbuf ;
		    break ;
		case GETUTMP_HOST:
	            cl = MIN(rlen,UTMPX_LHOST) ;
	            cl = strwcpy(rbuf,up->ut_host,cl) - rbuf ;
		    break ;
		} /* end switch */
	} /* end if (ok) */

	endutxent() ;

	return (rs >= 0) ? cl : rs ;
}
/* end subroutine (get_utmpx) */

#endif /* CF_UTMPX */


static int get_utmp(char *rbuf,int rlen,pid_t sid,int type)
{
	struct utmp	*up ;
	int		rs = SR_NOTFOUND ;
	int		cl = 0 ;

	rbuf[0] = '\0' ;
	setutent() ;

	while ((up = getutent()) != NULL) {
	    if ((up->ut_type == UTMP_TUSERPROC) &&
	        (up->ut_pid == sid)) {
		rs = SR_OK ;
		break ;
	    }
	} /* end while */

	if (rs >= 0) {
		switch (type) {
		case GETUTMP_ID:
	            cl = MIN(rlen,UTMP_LID) ;
	            cl = strwcpy(rbuf,up->ut_id,cl) - rbuf ;
		    break ;
		case GETUTMP_LINE:
	            cl = MIN(rlen,UTMP_LLINE) ;
	            cl = strwcpy(rbuf,up->ut_line,cl) - rbuf ;
		    break ;
		case GETUTMP_USER:
	            cl = MIN(rlen,UTMP_LUSER) ;
	            cl = strwcpy(rbuf,up->ut_user,cl) - rbuf ;
		    break ;
		case GETUTMP_HOST:
	            cl = MIN(rlen,UTMP_LHOST) ;
	            cl = strwcpy(rbuf,up->ut_host,cl) - rbuf ;
		    break ;
		} /* end switch */
	} /* end if (ok) */

	endutent() ;

	return (rs >= 0) ? cl : rs ;
}
/* end subroutine (get_utmp) */


