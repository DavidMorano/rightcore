/* logfile_userinfo */

/* log user information */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2005-01-20, David A­D­ Morano
        This was originally written to collect some common PCS code into one
        subroutines.

*/

/* Copyright © 2005 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutines is generally used to make the first log-file entry at
        the start of a program involcation. The subroutine requires that both a
        LOGFILE object have already been opened and that the caller has
        retrieved the user information using the USERINFO facility (and passed a
        pointer to that down to this subroutine).

	Synopsis:

	int logfile_userinfo(op,uip,dt,pn,vn)
	LOGFILE		*op ;
	USERINFO	*uip ;
	time_t		dt ;
	const char	pn[] ;
	const char	vn[] ;

	Arguments:

	op		pointer to LOGFILE object
	uip		pointer to USERINFO object
	dt		current UNIX® time of day
	pn		program name
	vn		program-version name

	Returns:

	>=0	number of bytes written to log
	<0	error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<logfile.h>
#include	<userinfo.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARARCHITECTURE
#define VARARCHITECTURE	"ARCHITECTURE"
#endif

#ifndef	BANGBUFLEN
#define	BANGBUFLEN	(NODENAMELEN+1+USERNAMELEN)
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkuibang(char *,int,USERINFO *) ;
extern int	mkuiname(char *,int,USERINFO *) ;

extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int logfile_userinfo(LOGFILE *op,USERINFO *uip,time_t dt,cchar *pn,cchar *vn)
{
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	const char	*a = getenv(VARARCHITECTURE) ;
	const char	*fmt ;
	char		timebuf[TIMEBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("logfile_userinfo: op=%p\n",op) ;
	debugprintf("logfile_userinfo: uip=%p\n",uip) ;
	debugprintf("logfile_userinfo: pn=%s\n",pn) ;
	debugprintf("logfile_userinfo: vn=%s\n",vn) ;
	debugprintf("logfile_userinfo: a=%s\n",a) ;
#endif /* CF_DEBUGS */

	if ((op == NULL) || (uip == NULL))
	    return SR_FAULT ;

	if (pn == NULL)
	    pn = "" ;

	if (vn == NULL)
	    vn = "" ;

	if (dt <= 0)
	    dt = time(NULL) ;

/* first line (w/ possible additional information) */

	fmt = "%s starting" ;
	if ((pn[0] != '\0') || (vn[0] != '\0'))
	    fmt = "%s %s %s/%s" ;

	{
	    const char	*ts = timestr_logz(dt,timebuf) ;
	    const char	*st = (uip->f.sysv_ct) ? "SYSV" : "BSD" ;
	    rs = logfile_printf(op,fmt,ts,pn,vn,st) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    const char	*sn = uip->sysname ;
	    const char	*rn = uip->release ;
	    const char	*dn = uip->domainname ;
	    if (a != NULL) {
	        rs = logfile_printf(op,"a=%s os=%s(%s) d=%s",a,sn,rn,dn) ;
		wlen += rs ;
	    } else {
	        rs = logfile_printf(op,"os=%s(%s) d=%s",sn,rn,dn) ;
		wlen += rs ;
	    }
	}

	if (rs >= 0) {
	    char	namebuf[BANGBUFLEN + 1] ;
	    if ((rs1 = mkuibang(namebuf,BANGBUFLEN,uip)) >= 0) {
	        rs = logfile_print(op,namebuf,rs1) ;
	        wlen += rs ;
	    }
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (logfile_userinfo) */


