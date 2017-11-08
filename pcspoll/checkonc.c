/* checkonc */

/* check for the ONC private key stuff */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1999-08-17, David A­D­ Morano
        This subroutine was taken from the LOGDIR/LOGNAME program (fist written
        for the SunOS 4.xx environment in 1989). This is severly hacked to work
        in the strange environment of PCSPOLL!

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine checks that we have a decrypted ONC private key and that
        KEYSERV has it.

	Synopsis:

	int checkonc(pr,unetname,uname,afname)
	const char	pr[] ;
	const char	unetname[] ;
	const char	uname[] ;
	const char	afname[] ;

	Arguments:

	pr		program root (for PCS)
	unetname	the user's ONC "netname"
	uname		username if known
	afname		authorization file name if known

	Returns:

	>=0		OK
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<rpc/rpc.h>
#include	<rpc/key_prot.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<netdb.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<netfile.h>
#include	<vecstr.h>

#include	"localmisc.h"



/* local defines */

#ifndef	PWENTRY_BUFLEN
#define	PWENTRY_BUFLEN	256
#endif

#ifndef	VARPCS
#define	VARPCS		"PCS"
#endif

#ifndef	PCS
#define	PCS		"/usr/add-on/pcs"
#endif



/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	onckeyalready(const char *) ;
extern int	onckeygetset(const char *,const char *) ;
extern int	vecstr_adduniq(VECSTR *,const char *,int) ;

extern int	havenis() ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* locally defined variables */

static const char *authfname[] = {
	".auth",
	"etc/auth",
	NULL
} ;

static const char *netrcfname[] = {
	".netrc",
	"etc/netrc",
	NULL
} ;







int checkonc(pr,unetname,uname,afname)
const char	pr[] ;
const char	unetname[] ;
const char	uname[] ;
const char	afname[] ;
{
	NETFILE	nfile ;

	VECSTR	unames ;

	int	rs ;
	int	i, j, k ;
	int	nunames ;
	int	f_keyed ;
	int	f_uname ;

	const char	*cp ;

	char	tmpfname[MAXPATHLEN + 2] ;
	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;
	char	hostname[MAXHOSTNAMELEN + 2] ;
	char	netname[MAXNETNAMELEN + 1] ;
	char	a_username[LOGNAMELEN + 1] ;
	char	a_password[PASSWORDLEN + 1] ;


	rs = havenis() ;
	if (rs < 0)
	    goto ret0 ;

/* do we even have an ONC 'netname'? */

#if	CF_DEBUGS
	debugprintf("main: getting NIS+ netname\n") ;
#endif

	if ((unetname == NULL) || (unetname[0] == '\0')) {

	    netname[0] = '\0' ;
	    rs = getnetname(netname) ;

/* get out if we do not even have a 'netname'! (if the call "fails") */

	    if (rs == 0)
	        goto retnonetname ;

	} else
	    strwcpy(netname,unetname,MAXNETNAMELEN) ;

#if	CF_DEBUGS
	debugprintf("main: we have a netname=%s\n",netname) ;
#endif

/* get out now if we have already given KEYSERV a key */

#if	CF_DEBUGS
	    debugprintf("main: onckeyalready()\n") ;
#endif

	rs = onckeyalready(netname) ;

#if	CF_DEBUGS
	    debugprintf("main: onckeyalready() rs=%d\n",rs) ;
#endif

	if (rs == SR_NOTSUP)
	    goto retnosup ;

	if (rs >= 0) {

#if	CF_DEBUGS
	    debugprintf("main: already key-logged in\n") ;
#endif

	    goto retalready ;

	} /* end if (already key-logged-in) */

#if	CF_DEBUGS
	debugprintf("main: not already keylogged in, netname=%s\n",netname) ;
#endif


	f_uname = ((uname != NULL) && (uname[0] != '\0')) ;

	if (! f_uname)
	    rs = vecstr_start(&unames,10,0) ;

	if (rs < 0)
	    goto ret0 ;

/* if we have an authorization file, use that first! */

#if	CF_DEBUGS
	debugprintf("main: specified AUTH file\n") ;
#endif

	rs = SR_INVALID ;
	a_username[0] = '\0' ;
	if ((afname != NULL) && (afname[0] != '\0')) {

	    if (authfile(afname,a_username,a_password) >= 0) {

	        if (a_password[0] != '\0') {

#if	CF_DEBUGS
	    debugprintf("main: 1 onckeygetset()\n") ;
#endif

	            rs = onckeygetset(netname,a_password) ;

#if	CF_DEBUGS
	    debugprintf("main: onckeygetset() rs=%d\n",rs) ;
#endif

		}

	    }

	} /* end if (explicit authorization file) */

	if (rs >= 0)
	    goto done0 ;

/* accumulate possible usernames if we were not given one! */

	if ((a_username[0] != '\0') && (! f_uname))
	    vecstr_adduniq(&unames,a_username,-1) ;

/* try to get a program root if we don't have one already! */

	if (pr == NULL)
	    pr = getenv(VARPCS) ;

	if (pr == NULL)
	    pr = PCS ;

/* see if there is an authorization file at the program root */

	rs = SR_ACCESS ;
	for (i = 0 ; authfname[i] != NULL ; i += 1) {

	    mkpath2(tmpfname,pr,authfname[i]) ;

	    if (authfile(tmpfname,a_username,a_password) >= 0) {

	        if (a_password[0] != '\0') {

#if	CF_DEBUGS
	    debugprintf("main: 2 onckeygetset()\n") ;
#endif

	            rs = onckeygetset(netname,a_password) ;

#if	CF_DEBUGS
	    debugprintf("main: onckeygetset() rs=%d\n",rs) ;
#endif

	            if (rs >= 0)
	                break ;

	        } /* end if (got an entry) */

	        if (! f_uname)
		    vecstr_adduniq(&unames,a_username,-1) ;

	    } /* end if */

	} /* end for */

	if (rs >= 0)
	    goto done1 ;

/* create the two most common names that this machine has (no NIS+ needed) */

	getnodedomain(nodename,domainname) ;

	hostname[0] = '\0' ;
	if (domainname[0] != '\0')
	    snsds(hostname,MAXHOSTNAMELEN,
	        nodename,domainname) ;

/* OK, now we have to do more work, look for a local NETRC file */

#if	CF_DEBUGS
	debugprintf("main: local NETRC file\n") ;
#endif

	nunames = (! f_uname) ? vecstr_count(&unames) : 0 ;

	f_keyed = FALSE ;
	for (i = 0 ; netrcfname[i] != NULL ; i += 1) {

	    mkpath2(tmpfname, pr,netrcfname[i]) ;

	    if ((rs = netfile_open(&nfile,tmpfname)) >= 0) {

	        NETFILE_ENT	*nep ;


	        for (j = 0 ; netfile_get(&nfile,i,&nep) >= 0 ; j += 1) {

	            if (nep == NULL)
	                continue ;

	            if ((strcmp(nep->machine,nodename) != 0) &&
	                (strcmp(nep->machine,hostname) != 0))
	                continue ;

	            if (f_uname) {

	                if (strcmp(nep->login,uname) != 0)
	                    continue ;

	            } else if (nunames > 0) {

	                for (k = 0 ; (rs = vecstr_get(&unames,k,&cp)) >= 0 ; 
	                    k += 1) {

	                    if (cp == NULL)
	                        continue ;

	                    if (strcmp(nep->login,cp) == 0)
	                        break ;

	                } /* end for */

	                if (rs < 0)
	                    continue ;

	            } /* end if */

	            if ((nep->password == NULL) ||
	                (nep->password[0] == '\0'))
	                continue ;

#if	CF_DEBUGS
	    debugprintf("main: 3 onckeygetset()\n") ;
#endif

	            rs = onckeygetset(netname,nep->password) ;

#if	CF_DEBUGS
	    debugprintf("main: onckeygetset() rs=%d\n",rs) ;
#endif

	            if (rs >= 0) {

	                f_keyed = TRUE ;
	                break ;
	            }

	        } /* end for */

	        netfile_close(&nfile) ;

	    } /* end if */

	    if (f_keyed)
	        break ;

	} /* end for */

	if ((! f_keyed) && (rs >= 0))
	    rs = SR_ACCESS ;

/* we are done (for good or for bad) */
done1:
ret1:
	if (! f_uname)
	    vecstr_finish(&unames) ;

retalready:
retnosup:
retnonetname:
done0:
ret0:
	return rs ;
}
/* end subroutine (checkonc) */



/* LOCAL SUBROUTINES */



