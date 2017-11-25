/* pcspoll */

/* initiate possible polling of PCS activities */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DAMAGED	0		/* protect against damage */
#define	CF_ISAEXEC	1		/* use 'isaexec(3c)' */
#define	CF_SNCPY	1		/* use 'sncpyx(3dam)' */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is called by PCS programs. This may initiate an
        invocation of the PCSPOLL program. Some quickie checks are made here
        first before calling that program in order to reduce the number of times
        that program needs to be invoked unnecessarily.

	Synopsis:

	int pcspoll(pr,searchname,csp,sp)
	const char	pr[] ;
	const char	searchname[] ;
	PCSCONF		*csp ;
	VECSTR		*sp ;

	Arguments:

	pr		PCS system program root (if available)
	searchname	name to use as program search-name
	csp		pointer to a PCSCONF block (if available)
	sp		pointer to VECSTR object of the 'set's (if available)

	Returns:

	>=0		OK
	<0		some error


*******************************************************************************/


#define	PCSCONF_MASTER		0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<sbuf.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"pcsconf.h"


/* local defines */

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	NOFILE
#define	NOFILE		20
#endif

#define	BUFLEN		(MAXPATHLEN + MAXHOSTNAMELEN + 100)

/* these are assumed (cannot be changed) */
#define	PCSPOLL_PROGRAM		"pcspoll"
#define	PCSPOLL_SEARCHNAME	"pcspoll"

/* these are defaults (can be changed) */
#define	PCSPOLL_TIMESTAMP	"spool/timestamps/pcspoll"
#define	PCSPOLL_MINCHECK	(3 * 60)

#ifndef	EX_NOEXEC
#define	EX_NOEXEC	126
#endif

#ifndef	VARPRPCS
#define	VARPRPCS	"PCS"
#endif

#ifndef	VARPROGPR
#define	VARPROGPR	"PCSPOLL_PROGRAMROOT"
#endif

#ifndef	VARPROGNAME
#define	VARPROGNAME	"PCSPOLL_NAME"
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	matkeystr(const char **,char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	findfilepath(const char *,char *,const char *,int) ;
extern int	bopenroot(bfile *,const char *,const char *,char *,
			const char *,int) ;
extern int	prgetprogpath(const char *,char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;

#if	CF_DEBUGS
extern char	*timestr_log(time_t,char *) ;
#endif


/* external variables */

extern const char	**environ ;


/* local structures */


/* forward references */

static int	matme(const char *,const char *,const char **,const char **) ;
static int	checkstamp(const char *,const char *,int) ;


/* local variables */

static const char *configkeys[] = {
	"timestamp",
	"mincheck",
	"pidmutex",
	NULL
} ;

enum configukeys {
	configkey_timestamp,
	configkey_mincheck,
	configkey_pidmutex,
	configkey_overlast
} ;

static const char *envok[] = {
	"HZ",
	"TZ",
	"USERNAME",
	"LOGNAME",
	"HOME",
	"PATH",
	"LANG",
	"NODE",
	"DOMAIN",
	"LOCALDOMAIN",
	"NAME",
	"FULLNAME",
	"MAILNAME",
	"ORGANIZATION",
	"PCS",
	NULL
} ;


/* exported subroutines */


int pcspoll(pr,searchname,csp,setp)
const char	pr[] ;
const char	searchname[] ;
PCSCONF		*csp ;
vecstr		*setp ;
{
	PCSCONF	pc ;

	VECSTR	sets ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;
	int	sl, cl ;
	int	mincheck = 0 ;
	int	f_localsets ;
	int	f_searchname ;
	int	f_pr ;
	int	f_polled = FALSE ;

	const char	*sp, *cp ;

	char	pcsconfbuf[PCSCONF_LEN + 2] ;
	char	stampfname[MAXPATHLEN + 2] ;
	char	pidfname[MAXPATHLEN + 2] ;
	char	progfname[MAXPATHLEN + 2] ;


#if	CF_DEBUGS
	debugprintf("pcspoll: searchname=%s\n",searchname) ;
#endif

/* our program (PCS) root */

	f_pr = TRUE ;
	if (pr == NULL) {

	    f_pr = FALSE ;
	    if (csp != NULL)
	        pr = csp->pr ;

	    if (pr == NULL)
	        pr = getenv(VARPRPCS) ;

	    if (pr == NULL)
	        pr = PCSCONF_PCS ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("pcspoll: PCS pr=%s\n",pr) ;
#endif

#if	CF_DAMAGED
	if (searchname != NULL) {
	    uint	uch = searchname[0] ;
	    if (uch >= 128)
		searchname = NULL ;
	}
#endif /* CF_DAMAGED */

	f_searchname = TRUE ;
	if ((searchname == NULL) || (searchname[0] == '\0')) {
		f_searchname = FALSE ;
		searchname = PCSPOLL_SEARCHNAME ;
	}

/* call any loadable modules before we continue */


/* before we do anything else, verify that the PCSPOLL program exists! */

	rs1 = prgetprogpath(pr,progfname,searchname,-1) ;

#if	CF_DEBUGS
	debugprintf("pcspoll: pcsgetprog() rs=%d progfname=%s\n",
		rs1,progfname) ;
#endif

	if (rs1 == 0)
	    rs1 = mkpath1(progfname,searchname) ;

	if (rs1 < 0)
	    goto ret0 ;

/* we really need that list of 'set' variables in PCSCONF! */

	f_localsets = FALSE ;
	if (setp == NULL) {

	    rs = vecstr_start(&sets,10,0) ;
	    if (rs < 0)
	        goto ret0 ;

	    f_localsets = TRUE ;
	    rs = pcsconf(pr,NULL,&pc,&sets,NULL,pcsconfbuf,PCSCONF_LEN) ;
	    if (rs < 0)
	        goto ret1 ;

	    setp = &sets ;

	} /* end if (given NULL sets) */

/* get the values we want from the PCSCONF sets */

	pidfname[0] = '\0' ;
	stampfname[0] = '\0' ;
	if (setp != NULL) {

	    int		kl ;
	    int		val ;

	    const char	*kp, *vp ;


	    for (i = 0 ; vecstr_get(setp,i,&sp) >= 0 ; i += 1) {
	        if (sp == NULL) continue ;

#if	CF_DEBUGS
	        debugprintf("pcspoll: PCSCONF set=>%s<\n",sp) ;
#endif

	        if ((kl = matme(searchname,sp,&kp,&vp)) < 0)
	            continue ;

#if	CF_DEBUGS
	        debugprintf("pcspoll: PCSCONF my key=%s\n",kp) ;
#endif

	        i = matpstr(configkeys,2,kp,kl) ;

	        if (i >= 0) {

	            switch (i) {

	            case configkey_pidmutex:
	                cl = sfshrink(vp,-1,&cp) ;

	                if ((cl > 0) && (cl < MAXPATHLEN))
	                    strwcpy(pidfname,cp,cl) ;

	                break ;

	            case configkey_timestamp:
	                cl = sfshrink(vp,-1,&cp) ;

	                if ((cl > 0) && (cl < MAXPATHLEN))
	                    strwcpy(stampfname,cp,cl) ;

	                break ;

	            case configkey_mincheck:
	                cl = sfshrink(vp,-1,&cp) ;

	                if (cl > 0) {

	                    rs1 = cfdecti(cp,cl,&val) ;

			if (rs1 >= 0)
				mincheck = val ;

#if	CF_DEBUGS
	                    debugprintf("pcspoll: 0 mincheck=%d\n",mincheck) ;
#endif

	                }

	                break ;

	            } /* end switch */

	        } /* end if */

	    } /* end for */

	} /* end if (getting values) */

/* check on what we have so far */

#if	CF_DEBUGS
	debugprintf("pcspoll: 1 mincheck=%d\n",mincheck) ;
#endif

	if (mincheck < 0)
	    goto ret1 ;

	if (mincheck == 0)
	    mincheck = PCSPOLL_MINCHECK ;

#if	CF_DEBUGS
	debugprintf("pcspoll: 2 mincheck=%d\n",mincheck) ;
#endif

	if (stampfname[0] == '\0')
	    strwcpy(stampfname,PCSPOLL_TIMESTAMP,MAXPATHLEN) ;

	if (stampfname[0] != '/') {
	    char	tmpfname[MAXPATHLEN + 2] ;

	    sl = mkpath2(tmpfname,pr,stampfname) ;

	    if (sl >= 0)
	        strwcpy(stampfname,tmpfname,MIN(MAXPATHLEN,sl)) ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("pcspoll: stampfname=%s\n",stampfname) ;
#endif

/* OK, now check the stamp file */

	rs = SR_OK ;
	if (checkstamp(pr,stampfname,mincheck) > 0) {

#if	CF_DEBUGS
	    debugprintf("pcspoll: needs processing\n") ;
#endif

/* it needs processing, pop the program */

	    f_polled = TRUE ;
	    if ((rs = uc_fork()) == 0) {

	        vecstr	args ;
	        vecstr	envs ;

	        char	tmpfname[MAXPATHLEN + 1] ;
	        char	*execfname ;
		char	*argz ;
		char	*nullfname = NULLFNAME ;


#if	CF_DEBUGS
	        debugprintf("pcspoll: child forked\n") ;
#endif

		vecstr_start(&args,2,0) ;

		vecstr_start(&envs,10,0) ;

#if	CF_DEBUGS
	        for (i = 0 ; i < 3 ; i += 1)
	            u_close(i) ;
#else
	        for (i = 0 ; i < NOFILE ; i += 1)
	            u_close(i) ;
#endif /* CF_DEBUGS */

		u_setsid() ;

	        for (i = 0 ; i < 3 ; i += 1) {

	            if (i != 0) {

#if	CF_DEBUGS
	                if (i == 2) {
	                    u_open("/tmp/pcspoll.err",O_WRONLY,0666) ;
	                } else
	                    u_open(nullfname,O_WRONLY,0666) ;
#else
	                u_open(nullfname,O_WRONLY,0666) ;
#endif

	            } else
	                u_open(nullfname,O_RDONLY,0666) ;

	        } /* end for */

/* if 'progfname' is empty, then the program was in the PATH!, get it */

	        execfname = progfname ;
	        if (progfname[0] == '\0') {

	            execfname = tmpfname ;
	            rs = findfilepath(NULL,tmpfname,PCSPOLL_PROGRAM,X_OK) ;

	        } /* end if */

#if	CF_DEBUGS
	        debugprintf("pcspoll: execfname=%s\n",execfname) ;
#endif

		if (f_searchname) {
	            argz = (char *) searchname ;
		} else {
	            argz = strbasename(execfname) ;
	        }

#if	CF_DEBUGS
	        debugprintf("pcspoll: argz=%s\n",argz) ;
#endif

/* arguments */

	        vecstr_add(&args,argz,-1) ;

#if	CF_DEBUGS
	        vecstr_add(&args,"-D",2) ;
	        vecstr_add(&args,"-f",2) ;
#endif

/* environment */

#if	CF_DEBUGS
	        debugprintf("pcspoll: environment\n") ;
#endif

		cp = VARPRPCS ;
	        if (environ != NULL) {

	            for (i = 0 ; environ[i] != NULL ; i += 1) {

	                if (matkeystr(envok,environ[i],-1) >= 0)
	                    rs = vecstr_add(&envs,environ[i],-1) ;

			if (rs < 0)
			    break ;

	            } /* end for */

	            if (vecstr_finder(&envs,cp,vstrkeycmp,NULL) < 0)
	                vecstr_envadd(&envs,cp,pr,-1) ;

	        } else
	            vecstr_envadd(&envs,cp,pr,-1) ;

		if (f_pr)
	        vecstr_envadd(&envs,VARPROGPR,pr,-1) ;

		if (f_searchname)
	        vecstr_envadd(&envs,VARPROGNAME,searchname,-1) ;

	        vecstr_envadd(&envs,"_",execfname,-1) ;

	        vecstr_envadd(&envs,"_EF",execfname,-1) ;

#if	CF_DEBUGS
	        debugprintf("pcspoll: u_execve()\n") ;
	        debugprintf("pcspoll: execfname=%s\n",execfname) ;
		for (i = 0 ; envs.va[i] != NULL ; i += 1)
	        	debugprintf("pcspoll: env[%02u] %s\n",i,envs.va[i]) ;
#endif /* CF_DEBUGS */

		if (rs >= 0) {
		    const char	**av = (const char **) args.va ;
		    const char	**av = (const char **) envs.va ;

#if	CF_ISAEXEC && defined(SOLARIS) && (SOLARIS >= 8)
	        rs = uc_isaexecve(execfname,av,ev) ;
#else
	        rs = uc_execve(execfname,av,ev) ;
#endif

#if	CF_DEBUGS
	        debugprintf("pcspoll: uc_execve() rs=%d\n",rs) ;
#endif

		}

	        uc_exit(EX_NOEXEC) ;
	    } /* end if (child) */

#if	CF_DEBUGS
	    debugprintf("pcspoll: uc_fork() rs=%d\n",rs) ;
#endif

	} /* end if (checked stamp file) */

/* we're out of here! */
ret1:
	if (f_localsets)
	    vecstr_finish(&sets) ;

ret0:

#if	CF_DEBUGS
	debugprintf("pcspoll: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_polled : rs ;
}
/* end subroutine (pcspoll) */


/* local subroutines */


/* does a key match my search name? */
static int matme(key,ts,kpp,vpp)
const char	key[] ;
const char	ts[] ;
const char	**kpp, **vpp ;
{
	char	*cp2, *cp3 ;


	if ((cp2 = strchr(ts,'=')) == NULL)
	    return -1 ;

	if (vpp != NULL)
	    *vpp = cp2 + 1 ;

	if ((cp3 = strchr(ts,':')) == NULL)
	    return -1 ;

	if (cp3 > cp2)
	    return -1 ;

	if (kpp != NULL)
	    *kpp = cp3 + 1 ;

	if (strncmp(ts,key,(cp3 - ts)) != 0)
	    return -1 ;

	return (cp2 - (cp3 + 1)) ;
}
/* end subroutine (matme) */


/* check the program time stamp file for need of processing or not */
static int checkstamp(pr,stampfname,mintime)
const char	pr[] ;
const char	stampfname[] ;
int		mintime ;
{
	struct ustat	sb ;

	bfile	tsfile ;

	time_t	daytime ;

	int	rs ;
	int	f_process = TRUE ;

	char	outfname[MAXPATHLEN + 2] ;


#if	CF_DEBUGS
	debugprintf("checkstamp: ent mintime=%d\n",mintime) ;
#endif

	if ((rs = bopenroot(&tsfile,pr,stampfname,outfname, "r",0666)) >= 0) {
	    f_process = FALSE ;
	    daytime = time(NULL) ;

	    bcontrol(&tsfile,BC_STAT,&sb) ;

#if	CF_DEBUGS
	{
		char	timebuf[TIMEBUFLEN + 1] ;
	    debugprintf("checkstamp: daytime=%s\n",
	        timestr_log(daytime,timebuf)) ;
	    debugprintf("checkstamp: mtime=%s\n",
	        timestr_log(sb.st_mtime,timebuf)) ;
	}
#endif /* CF_DEBUGS */

	    if (daytime > (sb.st_mtime + mintime))
	        f_process = TRUE ;

#ifdef	COMMENT
	    bprintf(&tsfile,"%s\n",
	        timestr_log(daytime,timebuf)) ;
#endif

	    bclose(&tsfile) ;
	} /* end if (opened) */

#if	CF_DEBUGS
	debugprintf("checkstamp: exiting f_process=%d\n",f_process) ;
#endif

	return f_process ;
}
/* end subroutine (checkstamp) */



