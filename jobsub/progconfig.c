/* progconfig */

/* program configuration */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_GETEXECNAME	1		/* get the 'exec(2)' name */
#define	CF_CPUSPEED	1		/* calculate CPU speed */


/* revision history:

	= 89/03/01, David A­D­ Morano

	This subroutine was originally written.  


	= 98/06/01, David A­D­ Morano

	I enhanced the program a little to print out some other
	information.


	= 99/03/01, David A­D­ Morano

	I enhanced the program a little to to do something (I forget
	what).


	= 04/01/10, David A­D­ Morano

	The KSH program switched to using a fakey "large file" (64-bit
	fake-out mode) compilation mode on Solaris.  This required
	some checking to see if any references to 'u_stat()' had to be
	updated to work with the new KSH.  Although we call 'u_stat()'
	here, its structure is not passed to other subroutines expecting
	the regular 32-bit structure.


	= 05/04/20, David A­D­ Morano

	I changed the program so that the configuration file is consulted
	even if the program is not run in daemon-mode.	Previously, the
	configuration file was only consulted when run in daemon-mode.
	The thinking was that running the program in regular (non-daemon)
	mode should be quick.  The problem is that the MS file had to
	be guessed without the aid of consulting the configuration file.
	Although not a problem in most practice, it was not aesthetically
	appealing.  It meant that if the administrator changed the MS file
	in the configuration file, it also had to be changed by specifying
	it explicitly at invocation in non-daemon-mode of the program.
	This is the source of some confusion (which the world really
	doesn't need).	So now the configuration is always consulted.
	The single one-time invocation is still fast enough for the
	non-smoker aged under 40 ! :-) :-)


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This is a built-in command to the KSH shell.  It should also
	be able to be made into a stand-alone program without much
	(if almost any) difficulty, but I have not done that yet (we
	already have a MSU program out there).

	Note that special care needed to be taken with the child processes
	because we cannot let them ever return normally !  They cannot
	return since they would be returning to a KSH program that thinks
	it is alive (!) and that geneally causes some sort of problem or
	another.  That is just some weird thing asking for trouble.  So we
	have to take care to force child processes to exit explicitly.
	Child processes are only created when run in "daemon" mode.

	Execute as :

	$ msu [-speed[=<name>]] [-zerospeed] [-msfile <file>]


	Implementation note:

	It is difficult to close files when run as a SHELL builtin !
	We want to close files when we run in the background, but when
	running as a SHELL builtin, we cannot close file descriptors
	untill after we fork (else we corrupt the enclosing SHELL).
	However, we want to keep the files associated with persistent
	objects open across the fork.  This problem is under review.
	Currently, there is not an adequate self-contained solution.


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<netdb.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<vecstr.h>
#include	<paramfile.h>
#include	<expcook.h>
#include	<logfile.h>
#include	<msfile.h>
#include	<kinfo.h>
#include	<lfm.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"msflag.h"
#include	"shio.h"


/* local defines */


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,const char *,
			const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	findfilepath(const char *,char *,const char *,int) ;
extern int	setfname(struct proginfo *,char *,const char *,int,int,
			const char *,const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

int progconfig_read(struct proginfo *) ;


/* local variables */

static const char	*sched1[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;

static const char	*params[] = {
	"cmd",
	"loglen",
	"server",
	"mailer",
	"msfile",
	"pidfile",
	"logfile",
	"runint",
	"pollint",
	"markint",
	"lockint",
	"speedint",
	NULL
} ;

enum params {
	param_cmd,
	param_loglen,
	param_server,
	param_mailer,
	param_msfile,
	param_pidfile,
	param_logfile,
	param_runint,
	param_pollint,
	param_markint,
	param_lockint,
	param_speedint,
	param_overlast
} ;


/* exported subroutines */


int progconfig_init(pip,configfname)
struct proginfo	*pip ;
const char	*configfname ;
{
	struct proginfo_config	*op = &pip->config ;

	VECSTR	sv ;

	int	rs = SR_OK ;

	char	tmpfname[MAXPATHLEN + 1] ;


	memset(op,0,sizeof(struct proginfo_config)) ;

	tmpfname[0] = '\0' ;
	if (strchr(configfname,'/') == NULL) {

	    rs = vecstr_envset(&sv,6,0) ;

	    if (rs >= 0) {

	        vecstr_envset(&sv,"p",pip->pr,-1) ;

	        vecstr_envset(&sv,"e","etc",-1) ;

	        vecstr_envset(&sv,"n",pip->searchname,-1) ;

	        rs = permsched(sched1,&sv,
			tmpfname,MAXPATHLEN,configfname,R_OK) ;

	        if (rs == 0)
	            mkpath1(tmpfname,configfname) ;

	        vecstr_finish(&sv) ;
	    } /* end if (finding file) */

	} else
	    mkpath1(tmpfname,configfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_init: rs=%d configfname=%s\n",rs,tmpfname) ;
#endif

	if (rs >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("config_init: configfname=%s\n",tmpfname) ;
#endif

	    rs = paramfile_open(&op->p,(const char **) pip->envv,tmpfname) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("config_init: paramfile_open() rs=%d\n",rs) ;
#endif

	}

	if (rs >= 0) {

	    rs = expcook_start(&op->cooks) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("config_init: expcook_start() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto bad1 ;

	    expcook_add(&op->cooks,"N",pip->nodename,-1) ;

	    expcook_add(&op->cooks,"D",pip->domainname,-1) ;

	    snsds(tmpfname,MAXPATHLEN,pip->nodename,pip->domainname) ;

	    expcook_add(&op->cooks,"H",tmpfname,-1) ;

	    expcook_add(&op->cooks,"R",pip->pr,-1) ;

	    expcook_add(&op->cooks,"S",pip->searchname,-1) ;

	    expcook_add(&op->cooks,"U",pip->username,-1) ;

	    op->f_p = TRUE ;
	    rs = progconfig_read(pip) ;

	    op->f_p = (rs >= 0) ;
	    if (rs < 0)
	        goto bad2 ;

	} /* end if */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_init: ret rs=%d \n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad2:
	expcook_finish(&op->cooks) ;

bad1:
	paramfile_close(&op->p) ;

bad0:
	goto ret0 ;
}
/* end subroutine (progconfig_init) */


int progconfig_check(pip)
struct proginfo	*pip ;
{
	struct proginfo_config	*op = &pip->config ;

	int	rs = SR_NOTOPEN ;


	if (op->f_p) {

	    rs = paramfile_check(&op->p,pip->daytime) ;

	    if (rs > 0)
	        rs = progconfig_read(pip) ;

	}

	return rs ;
}
/* end subroutine (progconfig_check) */


int progconfig_free(pip)
struct proginfo	*pip ;
{
	struct proginfo_config	*op = &pip->config ;

	int	rs = SR_NOTOPEN ;


	if (op->f_p) {

	    expcook_finish(&op->cooks) ;

	    rs = paramfile_close(&op->p) ;

	}

	return rs ;
}
/* end subroutine (progconfig_free) */


int progconfig_read(pip)
struct proginfo	*pip ;
{
	struct proginfo_config	*op = &pip->config ;

	PARAMFILE_CUR	cur ;

	int	rs = SR_NOTOPEN, rs1, i ;
	int	ml, vl, el ;
	int	v ;

	char	vbuf[VBUFLEN + 1] ;
	char	ebuf[EBUFLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("config_read: f_p=%u\n",op->f_p) ;
#endif

	if (! op->f_p)
	    return SR_NOTOPEN ;

	rs = SR_OK ;
	for (i = 0 ; params[i] != NULL ; i += 1) {

	    rs1 = 0 ;
	    switch (i) {

	    case param_msfile:
	        if (pip->final.msfile)
	            rs1 = TRUE ;

	        break ;

	    case param_runint:
	        if (pip->final.runint)
	            rs1 = TRUE ;

	        break ;

	    } /* end switch */

	    if (rs1)
	        continue ;

	    if ((rs = paramfile_curbegin(&op->p,&cur)) >= 0) {

	    while (rs >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("config_read: checking for param=%s\n",
	                params[i]) ;
#endif

	        vl = paramfile_fetch(&op->p,params[i],&cur,
	            vbuf,VBUFLEN) ;

	        if (vl < 0)
	            break ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("config_read: vbuf=>%t<\n",vbuf,vl) ;
#endif

	        ebuf[0] = '\0' ;
	        el = 0 ;
	        if (vl > 0) {

	            el = expcook_exp(&op->cooks,0,ebuf,EBUFLEN,
	                vbuf,vl) ;

	            if (el >= 0)
	                ebuf[el] = '\0' ;

	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("config_read: ebuf=>%t<\n",ebuf,el) ;
#endif

	        if (el > 0) {

	            char	tmpfname[MAXPATHLEN + 1] ;


	            switch (i) {

	            case param_loglen:
	            case param_runint:
	            case param_pollint:
	            case param_markint:
	            case param_lockint:
	            case param_speedint:
	                rs1 = cfdecti(ebuf,el,&v) ;

	                if ((rs1 >= 0) && (v >= 0)) {

	                    switch (i) {

	                    case param_loglen:
	                        pip->loglen = v ;
	                        break ;

	                    case param_speedint:
	                        pip->speedint = v ;
	                        break ;

	                    case param_pollint:
	                        if (! pip->final.pollint)
	                            pip->pollint = v ;

	                        break ;

	                    case param_lockint:
	                        if (! pip->final.lockint)
	                            pip->lockint = v ;

	                        break ;

	                    case param_markint:
	                        if (! pip->final.markint)
	                            pip->markint = v ;

	                        break ;

	                    case param_runint:
	                        if (! pip->final.runint)
	                            pip->runint = v ;

	                        break ;

	                    } /* end switch */

	                } /* end if (valid number) */

	                break ;

	            case param_msfile:
	                if (! pip->final.msfile) {

	                    setfname(pip,tmpfname,ebuf,el,TRUE,
	                        MSDNAME,MSFNAME,"") ;

	                    if (strcmp(pip->msfname,tmpfname) != 0) {
	                        pip->changed.msfile = TRUE ;
	                        mkpath1(pip->msfname,tmpfname) ;
	                    }

	                }

	                break ;

	            case param_pidfile:
	                if (! pip->final.pidfile) {

	                    setfname(pip,tmpfname,ebuf,el,TRUE,
	                        RUNDNAME,pip->nodename,PIDFNAME) ;

	                    if (strcmp(pip->pidfname,tmpfname) != 0) {
	                        pip->changed.pidfile = TRUE ;
	                        mkpath1(pip->pidfname,tmpfname) ;
	                    }

	                }

	                break ;

	            case param_logfile:
	                if (! pip->final.logfile) {

	                    setfname(pip,tmpfname,ebuf,el,TRUE,
	                        LOGDNAME,pip->searchname,"") ;

	                    if (strcmp(pip->logfname,tmpfname) != 0) {
	                        pip->changed.logfile = TRUE ;
	                        mkpath1(pip->logfname,tmpfname) ;
	                    }

	                }

	                break ;

	            case param_server:
	                if (! pip->final.server) {

	                    ml = setfname(pip,tmpfname,ebuf,el,TRUE,
	                        "bin",PROGSERVER,"") ;

	                    if (xfile(pip,tmpfname) < 0)
	                        ml = setfname(pip,tmpfname,ebuf,el,TRUE,
	                            "sbin",PROGSERVER,"") ;

	                    if ((ml > 0) &&
	                        (xfile(pip,tmpfname) >= 0)) {

	                        if ((pip->prog_server == NULL) ||
	                            (strcmp(pip->prog_server,tmpfname) != 0)) {

	                            pip->changed.server = TRUE ;
	                            proginfo_setentry(pip,&pip->prog_server,
	                                tmpfname,ml) ;

	                        }
	                    }

	                }

	                break ;

	            case param_mailer:
	                if (! pip->final.mailer) {

	                    ml = setfname(pip,tmpfname,ebuf,el,TRUE,
	                        "/usr/bin",PROGMAILER,"") ;

#if	CF_DEBUG
	debugprintf("progconfig_read: mailer 1 ml=%d tf=%s\n",
	ml,tmpfname) ;
#endif

	                    if (xfile(pip,tmpfname) < 0)
	                        ml = setfname(pip,tmpfname,ebuf,el,TRUE,
	                            "bin",PROGMAILER,"") ;

#if	CF_DEBUG
	debugprintf("progconfig_read: mailer 2 ml=%d tf=%s\n",
	ml,tmpfname) ;
#endif

	                    if ((ml > 0) && (xfile(pip,tmpfname) < 0)) {
				const char	*pn = PROGMAILER ;

	                        ml = findfilepath(NULL,tmpfname,pn,X_OK) ;
	                        if (ml == 0)
	                            ml = mkpath1(tmpfname,pn) ;

#if	CF_DEBUG
	debugprintf("progconfig_read: mailer 3 ml=%d tf=%s\n",
	ml,tmpfname) ;
#endif

	                    }

#if	CF_DEBUG
	debugprintf("progconfig_read: mailer pm=%s\n",
	pip->prog_mailer) ;
#endif

	                    if ((ml >= 0) &&
	                        ((pip->prog_mailer == NULL) ||
	                        (strcmp(pip->prog_mailer,tmpfname) != 0))) {
	                        pip->changed.mailer = TRUE ;
	                        proginfo_setentry(pip,&pip->prog_mailer,
	                            tmpfname,ml) ;
	                    }

	                }

	                break ;

	            case param_cmd:
	                ml = MIN(LOGIDLEN,el) ;
	                if (ml && (pip->cmd[0] == '\0'))
	                    strwcpy(pip->cmd,ebuf,ml) ;

	                break ;

	            } /* end switch */

	        } /* end if (got one) */

	    } /* end while (fetching) */

	    paramfile_curend(&op->p,&cur) ;
	} /* end if (parameters) */

	    if (rs < 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (progconfig_read) */


