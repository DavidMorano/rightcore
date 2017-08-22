/* handle_login */

/* handle the login-based serivce */
/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_ALLDEF	0		/* always use default access */
#define	CF_ACCESSCHECK	0


/* revision history:

	= 1986-07-01, David A­D­ Morano
	This program was originally written.

	= 1998-07-01, David A­D­ Morano
	This subroutine has been enhanced with some modifications to
	the user and group handling of the spawned server program.

*/

/* Copyright © 1996,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Handle a request for which we have a matching login entry.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<varsub.h>
#include	<vecstr.h>
#include	<storebuf.h>
#include	<acctab.h>
#include	<localmisc.h>

#include	"connection.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	LOGBUFLEN	(LOGNAMELEN + 20)
#define	O_FLAGS		(O_CREAT | O_RDWR | O_TRUNC)

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	findfilepath(char *,char *,int,char *) ;
extern int	vstrkeycmp(char *const *,char *const *) ;
extern int	mkquoted(char *,int,const char *,int) ;

extern char	*strbasename(char *) ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int handle_login(pip,cip,cnp,sap,ofd,pep)
struct proginfo		*pip ;
struct clientinfo	*cip ;
CONNECTION	*cnp ;
vecstr		*sap ;
int		ofd ;
struct passwd	*pep ;
{
	time_t	daytime ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i, j ;
	int	sl, si ;
	int	fd, count ;
	int	tlen = 0 ;

	char	svcargbuf[BUFLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	*username ;
	char	*cp, *cp2 ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("handle_login: login=%s\n",pep->pw_name) ;
#endif

#ifdef	COMMENT

/* quote any service arguments given w/ the request into a single buffer */

	si = 0 ;
	for (i = 0 ; vecstr_get(sap,i,&ap) >= 0 ; i += 1) {

	    int		qlen ;

	    char	qbuf[BUFLEN + 1] ;


	    if (ap == NULL) continue ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("handle_login: svcarg >%s<\n",ap) ;
#endif

	    if (si > 0)
	        si += storebuf_char(svcargbuf,BUFLEN,si,' ') ;

	    qlen = mkquoted(qbuf,BUFLEN,ap,-1) ;
	    if (qlen > 0)
	        si += storebuf_buf(svcargbuf,BUFLEN,si,qbuf,qlen) ;

	} /* end for */

	svcargbuf[si] = '\0' ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("handle_login: i=%d svcargbuf >%W<\n",
	        i,svcargbuf,si) ;
#endif

#endif /* COMMENT */


#if	CF_ACCESSCHECK

/**** PROGRAM NOTE:
		
	At this point, we have a processed (variable substituted, et
	cetera) server entry in variable 'ese'.  From here on, we want
	to check the server access list to see if this current
	connection is allowed.  Finally, we dump the user information.

****/


/* check if this connection is allowed based on the service access */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("handle_login: ese.access=>%s< defacc=>%s<\n",
	        ese.access,pip->defacc) ;
	    debugprintf("handle_login: f_acctab=%d f_inet=%d\n",
	        pip->f.acctab,cnp->f.inet) ;
	}
#endif

	rs = SR_OK ;
	if (cnp->f.inet && ((ese.access != NULL) || (pip->defacc != NULL))) {
	    FIELD	af ;
	    vecstr	netgroups, names ;
	    int	fl ;


	    vecstr_start(&netgroups,4,0) ;

	    vecstr_start(&names,4,0) ;

	    cp = (ese.access != NULL) ? ese.access : pip->defacc ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("handle_login: netgroups >%s<\n",cp) ;
#endif

/* process the server access list */

	    field_start(&af,cp,-1) ;

	    while ((fl = field_get(&af,NULL)) >= 0) {

	        if (fl > 0) {
	            int		bl = fl ;
	            char	*bp = af.fp ;

	            while ((sl = nextfield(bp,bl,&cp)) > 0) {

	                vecstr_add(&netgroups,cp,sl) ;

	                bl -= ((cp + sl) - bp) ;
	                bp = (cp + sl) ;

	            } /* end while */

	        } /* end if */

	    } /* end while */

#if	CF_ALLDEF
	    if (vecstr_find(&netgroups,"DEFAULT") < 0)
	        vecstr_add(&netgroups,"DEFAULT",-1) ;
#else
	    if (vecstr_count(&netgroups) <= 0)
	        vecstr_add(&netgroups,"DEFAULT",-1) ;
#endif

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("handle_login: netgroups:\n") ;
	        for (i = 0 ; vecstr_get(&netgroups,i,&cp) >= 0 ; i+= 1)
	            debugprintf("handle_login: ng=%s\n",cp) ;
	    }
#endif

	    rs = connection_mknames(cnp,&names) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("handle_login: connection_mknames rs=%d\n",rs) ;
	        for (i = 0 ; vecstr_get(&names,i,&cp) >= 0 ; i += 1)
	            debugprintf("handle_login: mname=%s\n",cp) ;
	    }
#endif

/* try our own netgroups */

	    if ((rs >= 0) && pip->f.acctab)
	        rs = acctab_anyallowed(&pip->atab,&netgroups,&names,NULL,NULL) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("handle_login: acctab anyallowed rs=%d\n",rs) ;
#endif

/* try the system netgroups (UNIX does not have one simple call as above !) */

	    if ((! pip->f.acctab) || (rs < 0)) {

	        char	*ngp, *mnp ;


	        for (i = 0 ; (rs = vecstr_get(&netgroups,i,&ngp)) >= 0 ; 
	            i += 1) {

	            if (ngp == NULL) continue ;

	            for (j = 0 ; (rs = vecstr_get(&names,j,&mnp)) >= 0 ; 
	                j += 1) {

	                if (mnp == NULL) continue ;

	                if (isdigit(mnp[0])) continue ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("handle_login: sysnetgr n=%s m=%s\n",
	                        ngp,mnp) ;
#endif

	                if (innetgr(ngp,mnp,NULL,pip->domainname))
	                    break ;

	            } /* end for (machine names) */

	            if (rs >= 0) break ;
	        } /* end for (netgroups) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("handle_login: system anyallowed rs=%d\n",rs) ;
#endif

	    } /* end if */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("handle_login: anyallowed rs=%d\n",rs) ;
#endif

	    field_finish(&af) ;

	    vecstr_finish(&names) ;

	    vecstr_finish(&netgroups) ;

	} /* end if (checking client for access to this server) */

	if (rs < 0) {

	    logfile_printf(&pip->lh,
	        "access denied to requesting machine\n") ;

	    goto badaccess ;
	}

#endif /* CF_ACCESSCHECK */

	count = 0 ;

/* project file */

	mkpath2(tmpfname,pep->pw_dir,PROJECTFNAME) ;

	rs1 = u_open(tmpfname,O_RDONLY | O_NDELAY,0666) ;
	fd = rs1 ;
	if (rs1 >= 0) {

	    rs = uc_copy(fd,ofd,-1) ;
	    if (rs > 0)
	        tlen += rs ;

	    u_close(fd) ;

	    count += 1 ;
	    if ((rs >= 0) && pip->f.log)
	        logfile_printf(&pip->lh,"project=%u",rs) ;

	} /* end if (project file) */

/* plan file */

	if (rs >= 0) {

	    mkpath2(tmpfname,pep->pw_dir,PLANFNAME) ;

	    rs1 = u_open(tmpfname,O_RDONLY | O_NDELAY,0666) ;
	    fd = rs1 ;
	    if (rs1 >= 0) {

	        rs = uc_copy(fd,ofd,-1) ;
	        if (rs > 0)
	            tlen += rs ;

	        u_close(fd) ;

	        count += 1 ;
	        if ((rs >= 0) && pip->f.log)
	            logfile_printf(&pip->lh,"plan=%u",rs) ;

	    } /* end if (project file) */

	}

/* we are out of here */
badnoserver:
badnoexist:
done:

badaccess:
baddone:
	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (handle_login) */



