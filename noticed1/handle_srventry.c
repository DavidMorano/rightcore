/* handle_srventry */

/* handle a connect request for a service when we have a server entry */
/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	F_ALWAYSDEFAULT	0		/* always use default access */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Handle a request for which we have a matching server entry.


***************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<varsub.h>
#include	<vecstr.h>
#include	<storebuf.h>
#include	<getax.h>
#include	<srvtab.h>
#include	<acctab.h>
#include	<localmisc.h>

#include	"srventry.h"
#include	"connection.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	LOGBUFLEN	(LOGNAMELEN + 20)
#define	O_FLAGS		(O_CREAT | O_RDWR | O_TRUNC)


/* external subroutines */

extern int	getgid_name(cchar *) ;
extern int	quoteshellarg() ;
extern int	findfilepath(const char *,char *,const char *,int) ;
extern int	field_svcargs(FIELD *,vecstr *) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	processargs(PROGINFO *,char *,vecstr *) ;
extern int	execute(PROGINFO *,int,char *,char *,vecstr *,vecstr *) ;
extern int	badback(int,int,char *) ;

extern char	*strbasename(char *) ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int handle_srventry(pip,sip,cip,cnp,atp,fsbp,sep,ifd,ofd,ssp,elp)
PROGINFO		*pip ;
struct serverinfo	*sip ;
struct clientinfo	*cip ;
CONNECTION	*cnp ;
ACCTAB		*atp ;
FIELD		*fsbp ;
SRVTAB_ENT	*sep ;
int		ifd, ofd ;
varsub		*ssp ;
vecstr		*elp ;
{
	SRVENTRY	ese ;

	SRVENTRY_ARGS	esa ;

	vecstr	svcargs, alist ;

	time_t		daytime ;

	gid_t	gid ;

	int	rs, i, j ;
	int	sl, si ;
	int	cs ;			/* connection status */
	int	f_switched ;

	char	svcargbuf[BUFLEN + 1] ;
	char	lognamebuf[LOGBUFLEN + 1] ;
	char	programpath[MAXPATHLEN + 1] ;
	char	*username ;
	char	*program, *arg0 ;
	char	*ap ;
	char	*cp, *cp2 ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle_srventry: entered, s=%d\n",ifd) ;
#endif

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("handle_srventry: srv program=%s\n",sep->program) ;
	    debugprintf("handle_srventry: srv args=>%s<\n",sep->args) ;
	}
#endif

	vecstr_start(&svcargs,6,0) ;

/* parse out the individual service arguments given with the request */

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("handle_srventry: calling field_svcargs\n") ;
	    debugprintf("handle_srventry: fbuf=>%W<\n",fsbp->lp,fsbp->rlen) ;
	}
#endif

	rs = field_svcargs(fsbp,&svcargs) ;

/* quote any service arguments given w/ the request into a single buffer */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle_srventry: field_svcargs rs=%d\n",rs) ;
#endif

	si = 0 ;
	for (i = 0 ; vecstr_get(&svcargs,i,&ap) >= 0 ; i += 1) {

	    int		qlen ;

	    char	qbuf[BUFLEN + 1] ;


	    if (ap == NULL) continue ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle_srventry: svcarg >%s<\n",ap) ;
#endif

	    if (si > 0)
	        si += storebuf_char(svcargbuf,BUFLEN,si,' ') ;

	    if ((qlen = quoteshellarg(ap,-1,qbuf,BUFLEN,NULL)) >= 0)
	        si += storebuf_buf(svcargbuf,BUFLEN,si,qbuf,qlen) ;

	} /* end for */

	svcargbuf[si] = '\0' ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle_srventry: i=%d svcargbuf >%W<\n",
	        i,svcargbuf,si) ;
#endif


/* process expand this service entry */

	srventry_init(&ese) ;

/* set the arguments to the next call */

	(void) memset(&esa,0,sizeof(SRVENTRY_ARGS)) ;

	esa.version = pip->version ;
	esa.searchname = pip->searchname ;
	esa.programroot = pip->programroot ;
	esa.nodename = pip->nodename ;
	esa.domainname = pip->domainname ;
	esa.hostname = pip->hostname ;

	esa.peername = cnp->peername ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("handle_srventry: peername=%s\n",cnp->peername) ;
	}
#endif

	esa.service = sep->service ;
	esa.svcargs = svcargbuf ;
	esa.nethost = NULL ;
	esa.netuser = NULL ;
	esa.netpass = NULL ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    varsub_dumpfd(ssp,-1) ;
	    debugprintf("handle_srventry: calling srventry_process\n") ;
	}
#endif

	srventry_process(&ese,ssp,pip->envv,sep,&esa) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle_srventry: returned srventry_process\n") ;
#endif

/** PROGRAM NOTE :
		
	At this point, we have a processed (variable substituted, et
	cetera) server entry in variable 'ese'.  From here on, we want
	to check the server access list to see if this current
	connection is allowed and then we want to extract the server
	program file path and prepare the arguments to the server
	program itself.

**/


/* check if this connection is allowed based on the service access */

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("handle_srventry: ese.access=>%s< defacc=>%s<\n",
	        ese.access,pip->defacc) ;
	    debugprintf("handle_srventry: f_acctab=%d f_inet=%d\n",
	        pip->f.acctab,cnp->f.inet) ;
	}
#endif

	rs = SR_OK ;
	if (cnp->f.inet && ((ese.access != NULL) || (pip->defacc != NULL))) {
	    FIELD	af ;
	    vecstr	netgroups, names ;
	    int		fl ;


	    vecstr_start(&netgroups,4,0) ;

	    vecstr_start(&names,4,0) ;

	    cp = (ese.access != NULL) ? ese.access : pip->defacc ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle_srventry: netgroups >%s<\n",cp) ;
#endif

/* process the server access list */

	    field_start(&af,cp,-1) ;

	    while ((fl = field_get(&af,NULL)) >= 0) {

	        if (fl > 0) {
			int	bl = fl ;
			char	*bp = af.fp ;

		    while ((sl = nextfield(bp,bl,&cp)) > 0) {

	            	vecstr_add(&netgroups,cp,sl) ;

			bl -= (cp + sl - bp) ;
			bp = cp + sl ;

		    } /* end while */

		} /* end if */

	    } /* end while */

#if	F_ALWAYSDEFAULT
	    if (vecstr_find(&netgroups,"DEFAULT") < 0)
	        vecstr_add(&netgroups,"DEFAULT",-1) ;
#else
	    if (vecstr_count(&netgroups) <= 0)
	        vecstr_add(&netgroups,"DEFAULT",-1) ;
#endif

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        debugprintf("handle_srventry: netgroups:\n") ;
	        for (i = 0 ; vecstr_get(&netgroups,i,&cp) >= 0 ; i+= 1)
	            debugprintf("handle_srventry: ng=%s\n",cp) ;
	    }
#endif

	    rs = connection_mknames(cnp,&names) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        debugprintf("handle_srventry: connection_mknames rs=%d\n",rs) ;
	        for (i = 0 ; vecstr_get(&names,i,&cp) >= 0 ; i += 1)
	            debugprintf("handle_srventry: mname=%s\n",cp) ;
	    }
#endif


/* try our own netgroups */

	    if (pip->f.acctab)
	        rs = acctab_anyallowed(atp,&netgroups,&names,NULL,NULL) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle_srventry: acctab anyallowed rs=%d\n",rs) ;
#endif

/* try the system netgroups (UNIX does not have one simple call as above !) */

	    if ((! pip->f.acctab) || (rs < 0)) {

	        char	*ngp, *mnp ;


	        for (i = 0 ; (rs = vecstr_get(&netgroups,i,&ngp)) >= 0 ; 
	            i += 1) {

	            if (ngp == NULL) continue ;

	            for (j = 0 ; (rs = vecstr_get(&names,j,&mnp)) >= 0 ; 
	                j += 1) {
	                if (mnp != NULL) {
			    int	ch = MKCHAR(mnp[0]) ;
			    if (! isdigitlatin(ch)) {
	                        if (innetgr(ngp,mnp,NULL,pip->domainname)) {
	                    	    break ;
				}
			    }
			}
	            } /* end for (machine names) */

	            if (rs >= 0) break ;
	        } /* end for (netgroups) */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle_srventry: system anyallowed rs=%d\n",rs) ;
#endif

	    } /* end if */

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        debugprintf("handle_srventry: anyallowed rs=%d\n",rs) ;
	    }
#endif

	    field_finish(&af) ;

	    vecstr_finish(&names) ;

	    vecstr_finish(&netgroups) ;

	} /* end if (checking client for access to this server) */

	if (rs < 0)
	    goto badaccess ;


/* OK, find the server program file path and create its program arguments */

	program = ese.program ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("handle_srventry: initial program=%s\n",ese.program) ;
	    debugprintf("handle_srventry: initial srvargs=%s\n",ese.srvargs) ;
	}
#endif


/* break the server arguments up so that we can find the first one */

	vecstr_start(&alist,10,0) ;

	arg0 = NULL ;
	if (ese.srvargs != NULL) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle_srventry: processing srvargs\n") ;
#endif

	    if ((rs = processargs(pip,ese.srvargs,&alist)) > 0)
	        vecstr_get(&alist,0,&arg0) ;

	}

/* get a program if we do not have one already */

	if (program == NULL) {

	    rs = SR_NOEXIST ;
	    if (ese.srvargs == NULL)
	        goto badnoserver ;

	    program = arg0 ;

	} /* end if (getting a program) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle_srventry: intermediate program=%s\n",program) ;
#endif


/* can we execute this service daemon ? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle_srventry: starting X check\n") ;
#endif

	if ((rs = findfilepath(NULL,programpath,program,X_OK)) < 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle_srventry: could not execute\n") ;
#endif

	    logfile_printf(&pip->lh,"cannot execute service daemon\n") ;

	    logfile_printf(&pip->lh,"program=%s\n",program) ;

	    rs = SR_NOEXIST ;
	    goto badnoexec ;

	} /* end if (could not find program) */

	    if (programpath[0] != '/')
	        bufprintf(programpath,MAXPATHLEN,"%s/%s",
	            pip->pwd,program) ;

	    program = programpath ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle_srventry: past X check, program=%s\n",program) ;
#endif


/* get a basename for ARG0 */

	if (arg0 == NULL) {
	    cp = program ;
	} else
	    cp = arg0 ;

	if ((cp2 = strbasename(cp)) != arg0)
	    arg0 = cp2 ;

	if (vecstr_count(&alist) < 1)
	    vecstr_add(&alist,arg0,-1) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle_srventry: final arg0=%s\n",arg0) ;
#endif


/* do we want to change user or group ? (more involved than you think !) */

	gid = -1 ;
	if (sep->gid >= 0) {
	    gid = sep->gid ;
	    setgid(sep->gid) ;
	} else if (pip->defgroup != NULL) {
	    if ((rs = getgid_name(pip->defgroup)) >= 0) {
	        gid = rs ;
	        setgid(gid) ;
	    } else if (isNotPresent(rs)) {
	        gid = getgid() ;
	        setgid(gid) ;
		rs = SR_OK ;
	    }
	}


#ifdef	COMMENT
	username = pip->username ;
#else
	username = NULL ;
#endif /* COMMENT */

	if (sep->uid >= 0) {
	    username = sep->username ;
	} else if (pip->defuser != NULL) {
	    username = pip->defuser ;
	}

	if (username != NULL) {
	    struct passwd	pe ;
	    char		pwbuf[PWBUFLEN + 1] ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle_srventry: username=%s\n",username) ;
#endif

	    if (getpw_name(&pe,pwbuf,PWBUFLEN,username) >= 0) {

	        if (sep->uid < 0)
	            sep->uid = pe.pw_uid ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("handle_srventry: gid=%d pw_gid=%d\n",
	                gid,pe.pw_gid) ;
#endif

	        if (gid != pe.pw_gid) {

	            if (gid >= 0) {

#if	CF_DEBUG
	                if (pip->debuglevel > 1) {

	                    debugprintf("handle_srventry: ngroups=%d\n",
	                        sep->ngroups) ;

	                    for (i = 0 ; i < sep->ngroups ; i += 1)
	                        debugprintf("handle_srventry: 1 group=%d\n",
	                            sep->groups[i]) ;

	                }
#endif /* end if */

	                if (sep->ngroups < 0) {

	                    sep->groups[0] = pe.pw_gid ;
	                    sep->ngroups = 1 ;

	                } else {

	                    for (i = 0 ; (i < sep->ngroups) ; i += 1)
	                        if (pe.pw_gid == sep->groups[i]) break ;

#if	CF_DEBUG
	                    if (pip->debuglevel > 1)
	                        debugprintf("handle_srventry: i=%d\n", i) ;
#endif

	                    if ((i >= sep->ngroups) && (i < NGROUPS_MAX)) {

	                        sep->groups[i++] = pe.pw_gid ;
	                        sep->ngroups = i ;

	                    }

	                } /* end if */

	            } else
	                gid = pe.pw_gid ;

	        } /* end if (running GID is different than user's default) */

	    } /* end if (username was in our 'passwd' DB) */

	} /* end if (username was not NULL) */


	if (sep->ngroups >= 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {

	        debugprintf("handle_srventry: explicit groups\n") ;

	        for (i = 0 ; i < sep->ngroups ; i += 1)
	            debugprintf("handle_srventry: 2 group=%d\n",sep->groups[i]) ;

	    }
#endif /* CF_DEBUG */

	    u_setgroups(sep->ngroups,sep->groups) ;

	} else {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle_srventry: default groups, u=%s gid=%d\n",
	            username,gid) ;
#endif

	    if ((username != NULL) && (gid >= 0)) {

	        rs = uc_initgroups(username,gid) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("handle_srventry: initgroups rs=%d\n",rs) ;
#endif

	    }

	} /* end if (setting supplementary groups) */


	f_switched = FALSE ;
	if (sep->uid >= 0) {

	    if (sep->uid != pip->uid) {

	        if ((rs = u_setuid(sep->uid)) >= 0)
	            f_switched = TRUE ;

	    } else {

	        f_switched = TRUE ;
	        if (sep->uid != pip->euid)
	            u_seteuid(sep->uid) ;

	    }
	}


/* put the LOGNAME into the exported environment */

	sl = bufprintf(lognamebuf,LOGBUFLEN,"LOGNAME=%s",
	    ((f_switched && (username != NULL)) ? username : pip->username)) ;

	vecstr_add(elp,lognamebuf,sl) ;


/* we are good ! */

	if ((rs = uc_writen(ofd,"+\n",2)) >= 2) {


/* do it ! */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle_srventry: executing\n") ;
#endif

	    (void) time(&daytime) ;

	    logfile_printf(&pip->lh,"server=%s\n",
	        arg0) ;

	if (sip->fd_ipc >= 0)
		u_close(sip->fd_ipc) ;

	    rs = execute(pip,ifd,program,arg0,&alist,elp) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle_srventry: execute rs=%d\n",rs) ;
#endif


	} /* end if (the socket is still good) */


/* we are out of here */
done:
	vecstr_finish(&alist) ;

baddone:
	srventry_free(&ese) ;

	vecstr_finish(&svcargs) ;

	return rs ;

/* bad stuff */
badnoserver:
	cs = TCPMUXD_CFNOSRV ;
	logfile_printf(&pip->lh,"no service configured for service\n") ;

	goto badsendback ;

badnoexec:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle_srventry: bad happened, rs=%d\n",rs) ;
#endif

	cs = TCPMUXD_CFNOSRV ;
	logfile_printf(&pip->lh,"could not find server program\n") ;

badsendback:
	badback(ofd,cs,NULL) ;

	goto done ;

badaccess:
	cs = TCPMUXD_CFACCESS ;
	logfile_printf(&pip->lh,"access denied to client machine\n") ;

	badback(ofd,cs,NULL) ;

	goto baddone ;
}
/* end subroutine (handle_srventry) */



