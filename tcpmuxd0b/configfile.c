/* configfile */

/* parse a configuration file */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGSFIELD	0
#define	CF_EXPORTEQUAL	0		/* add equal for empty-value exports */


/* revision history:

	= 2000-01-21, David A­D­ Morano
	This subroutine was enhanced for use by LevoSim.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is the old configuration file reader object. It is cheap, it is
        ill-conceived, it is a mess, it works well enough to be used for cheap
        code. I didn't want to use this junk for the Levo machine simulator but
        time pressure decided for us!

        Although this whole configuration scheme is messy, it gives us enough of
        what we need to get some configuration information into the Levo machine
        simulator and to get a parameter file name. This is good enough for now.


*******************************************************************************/


#define	CONFIGFILE_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<field.h>
#include	<buffer.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"configfile.h"


/* local defines */

#define	CONFIGFILE_MAGIC	0x04311633

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#undef	BUFLEN
#define	BUFLEN		(LINEBUFLEN * 2)


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecmfi(const char *,int,int *) ;

extern char	*strncpylc(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static void	checkfree() ;

#if	CF_DEBUGS
static int vardump(const char *,int) ;
#endif


/* local variables */

/* these are the terminators for most everything */
static const unsigned char 	fterms[32] = {
	0x7F, 0xFE, 0xC0, 0xFE,
	0x8B, 0x00, 0x00, 0x24, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
} ;

/* these are the terminators for options */
static const unsigned char 	oterms[32] = {
	0x00, 0x0B, 0x00, 0x00,
	0x09, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const char	*configkeys[] = {
	"define",
	"export",
	"tmpdir",
	"root",
	"pidfile",
	"lockfile",
	"log",
	"loglen",
	"workdir",
	"port",
	"username",
	"groupname",
	"userpassword",
	"machpassword",
	"srvtab",
	"sendmail",
	"envfile",
	"pathfile",
	"devicefile",
	"seedfile",
	"logsize",
	"organization",
	"unset",
	"timeout",
	"removemul",
	"acctab",
	"paramfile",
	"nrecips",
	"helpfile",
	"paramtab",
	"pingtab",
	"pingstat",
	"option",
	"mintexec",
	"interval",
	"stampdir",
	"maxjobs",
	"directory",
	"interrupt",
	"polltime",
	"filetime",
	"passfile",
	NULL
} ;

enum configkeys {
	configkey_define,
	configkey_export,
	configkey_tmpdir,
	configkey_root,
	configkey_pidfile,
	configkey_lockfile,
	configkey_log,
	configkey_loglen,
	configkey_workdir,
	configkey_port,
	configkey_user,
	configkey_group,
	configkey_userpass,
	configkey_machpass,
	configkey_srvtab,
	configkey_sendmail,
	configkey_envfile,
	configkey_pathfile,
	configkey_devicefile,
	configkey_seedfile,
	configkey_logsize,
	configkey_organization,
	configkey_unset,
	configkey_timeout,
	configkey_removemul,
	configkey_acctab,
	configkey_paramfile,
	configkey_nrecips,
	configkey_helpfile,
	configkey_paramtab,
	configkey_pingtab,
	configkey_pingstat,
	configkey_option,
	configkey_mintexec,
	configkey_interval,
	configkey_stampdir,
	configkey_maxjobs,
	configkey_directory,
	configkey_interrupt,
	configkey_polltime,
	configkey_filetime,
	configkey_passfile,
	configkey_eigenfile,
	configkey_minwordlen,
	configkey_maxwordlen,
	configkey_keys,
	configkey_overlast
} ;


/* exported subroutines */


int configfile_start(csp,configfname)
CONFIGFILE	*csp ;
const char	configfname[] ;
{
	BUFFER	options ;

	FIELD	fsb ;

	bfile	cfile, *cfp = &cfile ;

	vecstr	*vsp ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;
	int	c, len1, len ;
	int	bl, cl ;
	int	fl ;
	int	line = 0 ;
	int	noptions = 0 ;

	const char	*fp ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	buf2[BUFLEN + 1] ;
	char	*bp, *cp ;


#if	CF_DEBUGS
	debugprintf("configfile_start: entered filename=%s\n",configfname) ;
#endif

	if (csp == NULL)
	    return SR_FAULT ;

	memset(csp,0,sizeof(CONFIGFILE)) ;

	if ((configfname == NULL) || (configfname[0] == '\0'))
	    return SR_NOEXIST ;

/* initialize */

#if	CF_DEBUGS
	debugprintf("configfile_start: initializing\n") ;
#endif

	csp->srs = 0 ;
	csp->badline = -1 ;
	csp->loglen = -1 ;
	csp->minwordlen = -1 ;
	csp->maxwordlen = -1 ;
	csp->keys = -1 ;

	rs = vecstr_start(&csp->defines,10,0) ;
	if (rs < 0)
	    goto bad0 ;

	rs = vecstr_start(&csp->unsets,10,0) ;
	if (rs < 0)
	    goto bad1 ;

	rs = vecstr_start(&csp->exports,10,0) ;
	if (rs < 0)
	    goto bad2 ;

/* buffer initialization */

	rs = buffer_start(&options,-1) ;
	if (rs < 0)
		goto bad3 ;

/* open configuration file */

#if	CF_DEBUGS
	debugprintf("configfile_start: opened file\n") ;
#endif

	rs = bopen(cfp,configfname,"r",0664) ;
	if (rs < 0)
	    goto ret1 ;

/* start processing the configuration file */

#if	CF_DEBUGS
	debugprintf("configfile_start: reading lines\n") ;
#endif

	while ((rs = breadline(cfp,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    line += 1 ;
	    if (len == 1) continue ;	/* blank line */

	    if (linebuf[--len] != '\n') {

#ifdef	COMMENT
	        f_trunc = TRUE ;
#endif
	        while ((c = bgetc(cfp)) >= 0)
	            if (c == '\n') break ;

	        continue ;
	    }

	    if ((len == 0) || (linebuf[0] == '#'))
	        continue ;

	    if ((rs = field_start(&fsb,linebuf,len)) >= 0) {

	    	fl = field_get(&fsb,fterms,&fp) ;

/* convert key to lower case */

	    bl = MIN(fl,BUFLEN) ;
	    strncpylc(buf,fp,bl) ;

	    i = matpstr(configkeys,1,buf,bl) ;

	    if (i >= 0) {

#if	CF_DEBUGS
	        debugprintf("configfile_start: i=%d keyword=%s\n",
	            i,configkeys[i]) ;
#endif

	        switch (i) {

	        case configkey_root:
	        case configkey_tmpdir:
	        case configkey_log:
	        case configkey_workdir:
	        case configkey_pidfile:
	        case configkey_lockfile:
	        case configkey_user:
	        case configkey_group:
	        case configkey_port:
	        case configkey_userpass:
	        case configkey_machpass:
	        case configkey_srvtab:
	        case configkey_sendmail:
	        case configkey_mintexec:
	        case configkey_envfile:
	        case configkey_pathfile:
	        case configkey_logsize:
	        case configkey_organization:
	        case configkey_timeout:
	        case configkey_removemul:
	        case configkey_acctab:
	        case configkey_paramfile:
	        case configkey_paramtab:
	        case configkey_nrecips:
	        case configkey_helpfile:
	        case configkey_pingtab:
	        case configkey_pingstat:
	        case configkey_interval:
	        case configkey_stampdir:
	        case configkey_maxjobs:
	        case configkey_directory:
	        case configkey_interrupt:
	        case configkey_polltime:
	        case configkey_filetime:
	        case configkey_passfile:
	        case configkey_eigenfile:
	            fl = field_get(&fsb,fterms,&fp) ;

	            if (fl > 0)
	                bp = mallocstrw(fp,fl) ;

	            else 
	                bp = mallocstrw(buf,0) ;

#if	CF_DEBUGS && CF_DEBUGSFIELD
	            debugprintf("configfile_start: bp=%s\n",bp) ;
#endif

	            switch (i) {

	            case configkey_root:
	                if (csp->root != NULL)
	                    uc_free(csp->root) ;

	                csp->root = bp ;
	                break ;

	            case configkey_log:
	                if (csp->logfname != NULL)
	                    uc_free(csp->logfname) ;

	                csp->logfname = bp ;
	                break ;

	            case configkey_tmpdir:
	                if (csp->tmpdir != NULL)
	                    uc_free(csp->tmpdir) ;

	                csp->tmpdir = bp ;
	                break ;

	            case configkey_workdir:
	                if (csp->workdir != NULL)
	                    uc_free(csp->workdir) ;

	                csp->workdir = bp ;
	                break ;

	            case configkey_user:
	                if (csp->user != NULL)
	                    uc_free(csp->user) ;

	                csp->user = bp ;
	                break ;

	            case configkey_group:
	                if (csp->group != NULL)
	                    uc_free(csp->group) ;

	                csp->group = bp ;
	                break ;

	            case configkey_pidfile:
	                if (csp->pidfname != NULL)
	                    uc_free(csp->pidfname) ;

	                csp->pidfname = bp ;
#if	CF_DEBUGS
	                debugprintf("configfile_start: pidfname=%s\n",bp) ;
#endif

	                break ;

	            case configkey_lockfile:
	                if (csp->lockfname != NULL)
	                    uc_free(csp->lockfname) ;

	                csp->lockfname = bp ;
	                break ;

	            case configkey_port:
	                if (csp->port != NULL)
	                    uc_free(csp->port) ;

	                csp->port = bp ;
	                break ;

	            case configkey_userpass:
	                if (csp->userpass != NULL)
	                    uc_free(csp->userpass) ;

	                csp->userpass = bp ;
	                break ;

	            case configkey_machpass:
	                if (csp->machpass != NULL)
	                    uc_free(csp->machpass) ;

	                csp->machpass = bp ;
	                break ;

	            case configkey_srvtab:
	                if (csp->srvtab != NULL)
	                    uc_free(csp->srvtab) ;

	                csp->srvtab = bp ;
	                break ;

	            case configkey_sendmail:
	            case configkey_mintexec:
	                if (csp->sendmail != NULL)
	                    uc_free(csp->sendmail) ;

	                csp->sendmail = bp ;
	                break ;

	            case configkey_envfile:
	                if (csp->envfname != NULL)
	                    uc_free(csp->envfname) ;

	                csp->envfname = bp ;
	                break ;

	            case configkey_pathfile:
	                if (csp->pathfname != NULL)
	                    uc_free(csp->pathfname) ;

	                csp->pathfname = bp ;
	                break ;

	            case configkey_devicefile:
	                if (csp->devicefname != NULL)
	                    uc_free(csp->devicefname) ;

	                csp->devicefname = bp ;
	                break ;

	            case configkey_seedfile:
	                if (csp->seedfname != NULL)
	                    uc_free(csp->seedfname) ;

	                csp->seedfname = bp ;
	                break ;

	            case configkey_logsize:
	                if (csp->logsize != NULL)
	                    uc_free(csp->logsize) ;

	                csp->logsize = bp ;
	                break ;

	            case configkey_organization:
	                if (csp->organization != NULL)
	                    uc_free(csp->organization) ;

	                csp->organization = bp ;
	                break ;

	            case configkey_timeout:
	                if (csp->timeout != NULL)
	                    uc_free(csp->timeout) ;

	                csp->timeout = bp ;
	                break ;

	            case configkey_interval:
	                if (csp->interval != NULL)
	                    uc_free(csp->interval) ;

	                csp->interval = bp ;
	                break ;

	            case configkey_removemul:
	                if (csp->removemul != NULL)
	                    uc_free(csp->removemul) ;

	                csp->removemul = bp ;
	                break ;

	            case configkey_acctab:
	                if (csp->acctab != NULL)
	                    uc_free(csp->acctab) ;

	                csp->acctab = bp ;
	                break ;

	            case configkey_paramfile:
	            case configkey_paramtab:
	            case configkey_pingtab:
	                if (csp->paramfname != NULL)
	                    uc_free(csp->paramfname) ;

	                csp->paramfname = bp ;
	                break ;

	            case configkey_nrecips:
	                if (csp->nrecips != NULL)
	                    uc_free(csp->nrecips) ;

	                csp->nrecips = bp ;
	                break ;

	            case configkey_helpfile:
	                if (csp->helpfname != NULL)
	                    uc_free(csp->helpfname) ;

	                csp->helpfname = bp ;
	                break ;

	            case configkey_pingstat:
	                if (csp->statfname != NULL)
	                    uc_free(csp->statfname) ;

	                csp->statfname = bp ;
	                break ;

	            case configkey_stampdir:
	                if (csp->stampdir != NULL)
	                    uc_free(csp->stampdir) ;

	                csp->stampdir = bp ;
	                break ;

	            case configkey_maxjobs:
	                if (csp->maxjobs != NULL)
	                    uc_free(csp->maxjobs) ;

	                csp->maxjobs = bp ;
	                break ;

	            case configkey_directory:
	                if (csp->directory != NULL)
	                    uc_free(csp->directory) ;

	                csp->directory = bp ;
	                break ;

	            case configkey_interrupt:
	                if (csp->interrupt != NULL)
	                    uc_free(csp->interrupt) ;

	                csp->interrupt = bp ;
	                break ;

	            case configkey_polltime:
	                if (csp->polltime != NULL)
	                    uc_free(csp->polltime) ;

	                csp->polltime = bp ;
	                break ;

	            case configkey_filetime:
	                if (csp->filetime != NULL)
	                    uc_free(csp->filetime) ;

	                csp->filetime = bp ;
	                break ;

	            case configkey_passfile:
	                if (csp->passfname != NULL)
	                    uc_free(csp->passfname) ;

	                csp->passfname = bp ;
	                break ;

	            case configkey_eigenfile:
	                if (csp->eigenfname != NULL)
	                    uc_free(csp->eigenfname) ;

	                csp->eigenfname = bp ;
	                break ;

	            } /* end switch (inner) */

	            break ;

/* options */
	        case configkey_option:
	            while ((fsb.term != '#') &&
	                ((fl = field_get(&fsb,oterms,&fp)) >= 0)) {

	                if (fl > 0) {

	                    if (noptions > 0)
	                        rs = buffer_char(&options,',') ;

			    if (rs >= 0)
	                        buffer_strw(&options,fp,fl) ;

	                    noptions += 1 ;
	                }

	            } /* end while */

	            break ;

/* unsets */
	        case configkey_unset:
	            fl = field_get(&fsb,fterms,&fp) ;

	            if (fl > 0)
	                rs = vecstr_add(&csp->unsets,fp,fl) ;

	            break ;

/* export environment */
	        case configkey_define:
	        case configkey_export:
	            {
	                int	index, f1l, f2l ;
			int	f_equal, f ;

	                char	*f1p, *f2p ;


/* get first part */

	                fl = field_get(&fsb,fterms,&fp) ;

	                if (fl <= 0) {
	                    rs = SR_INVALID ;
	                    csp->badline = line ;
	                    break ;
	                }

	                if (fsb.term == '#')
	                    break ;

			f_equal = (fsb.term == '=') ;
	                len1 = fl ;
	                f1p = (char *) fp ;
	                f1l = fl ;

/* get second part */

	                fl = field_get(&fsb,fterms,&fp) ;

	                f2l = 0 ;
	                if (fl >= 0) {
	                    f2p = (char *) fp ;
	                    f2l = fl ;
	                } /* end if */

#if	CF_EXPORTEQUAL
	                f1p[f1l] = '\0' ;
	                if (f2l > 0) {

	                    f2p[f2l] = '\0' ;
	                    rs1 = sncpy3(buf2,BUFLEN,f1p,"=",f2p) ;

	                } else
	                    rs1 = sncpy2(buf2,BUFLEN,f1p,"=") ;

#else /* CF_EXPORTEQUAL */

	                f1p[f1l] = '\0' ;
	                if (f2l > 0) {

	                    f2p[f2l] = '\0' ;
	                    rs1 = sncpy3(buf2,BUFLEN,f1p,"=",f2p) ;

			} else if (f_equal) {

	                    rs1 = sncpy2(buf2,BUFLEN,f1p,"=") ;

	                } else
	                    rs1 = sncpy1(buf2,BUFLEN,f1p) ;

#endif /* CF_EXPORTEQUAL */

/* store it away */

	                if (i == configkey_export)
	                    vsp = &csp->exports ;

	                else
	                    vsp = &csp->defines ;

#if	CF_DEBUGS
	                debugprintf("configfile_start: about to add >%s<\n",buf2) ;
#endif

			f = (rs1 > 0) ;

#if	CF_EXPORTEQUAL
			f = f && (strchr(buf2,'=') != NULL) ;
#endif

			if (f) {

	                    rs = vecstr_add(vsp,buf2,rs1) ;

	                    index = rs ;
	                    if (rs < 0)
	                        break ;

#if	CF_DEBUGS
	                    debugprintf("configfile_start: added rs=%d\n", rs) ;
#endif

/* if this is an export variable, we do extra stuff */

	                    if (f_equal && (i == configkey_export)) {

#if	CF_DEBUGS
	                        debugprintf("configfile_start: export=>%s< i=%d\n", 
	                            buf2,index) ;
#endif

/* check for our favorite environment variables */

	                        if (strncmp(buf2,"TMPDIR",len1) == 0) {

#if	CF_DEBUGS
	                            debugprintf("configfile_start: TMPDIR=>%s< "
					"i=%d\n",
	                                buf2,index) ;
#endif

	                            if (csp->tmpdir != NULL)
	                                uc_free(csp->tmpdir) ;

	                            csp->tmpdir = 
	                                mallocstr((csp->exports).va[index]) ;

	                        } /* end if (handling TMPDIR specially) */

	                    } /* end if (got an export) */

	                } /* end if */

	            } /* end block */

	            break ;

	        case configkey_loglen:
	            fl = field_get(&fsb,fterms,&fp) ;

	            if ((fl <= 0) ||
	                (cfdecmfi(fp,fl,&csp->loglen) < 0)) {

	                csp->badline = line ;
	                rs = SR_INVALID ;
	                break ;
	            }

	            break ;

	        case configkey_minwordlen:
	            fl = field_get(&fsb,fterms,&fp) ;

	            if ((fl <= 0) ||
	                (cfdecmfi(fp,fl,&csp->minwordlen) < 0)) {

	                csp->badline = line ;
	                rs = SR_INVALID ;
	                break ;
	            }

	            break ;

	        case configkey_maxwordlen:
	            fl = field_get(&fsb,fterms,&fp) ;

	            if ((fl <= 0) ||
	                (cfdecmfi(fp,fl,&csp->maxwordlen) < 0)) {

	                csp->badline = line ;
	                rs = SR_INVALID ;
	                break ;
	            }

	            break ;

	        case configkey_keys:
	            fl = field_get(&fsb,fterms,&fp) ;

	            if ((fl <= 0) ||
	                (cfdecmfi(fp,fl,&csp->keys) < 0)) {

	                csp->badline = line ;
	                rs = SR_INVALID ;
	                break ;
	            }

	            break ;

	        default:
	            rs = SR_NOTSUP ;
	            break ;

	        } /* end switch */

	    } /* end if (valid key) */

	    field_finish(&fsb) ;

	    } /* end if */

	    if (rs < 0)
	        break ;

	} /* end while (reading lines) */

	bclose(cfp) ;

/* load up the options if we got any */

	if ((rs >= 0) && (noptions > 0)) {

	    cl = buffer_get(&options,&cp) ;

#if	CF_DEBUGS
	debugprintf("procfilepaths: final options\n") ;
	vardump(cp,cl) ;
#endif /* CF_DEBUGS */

	    if (cl > 0)
	        csp->options = mallocstrw(cp,cl) ;

	} /* end if (options) */

/* done with configuration file processing */

	if (rs >= 0)
	    csp->magic = CONFIGFILE_MAGIC ;

ret1:
	buffer_finish(&options) ;

	if (rs < 0)
		goto bad3 ;

ret0:

#if	CF_DEBUGS
	debugprintf("configfile_start: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* handle bad things */
bad3:
	vecstr_finish(&csp->exports) ;

bad2:
	vecstr_finish(&csp->unsets) ;

bad1:
	vecstr_finish(&csp->defines) ;

bad0:
	goto ret0 ;
}
/* end subroutine (configfile_start) */


/* free up this object */
int configfile_finish(csp)
CONFIGFILE	*csp ;
{


	if (csp == NULL)
	    return SR_FAULT ;

	if (csp->magic != CONFIGFILE_MAGIC)
	    return SR_NOTOPEN ;

/* free up the complex data types */

	vecstr_finish(&csp->defines) ;

	vecstr_finish(&csp->unsets) ;

	vecstr_finish(&csp->exports) ;

/* free up the simple ones */

	checkfree(&csp->root) ;

	checkfree(&csp->tmpdir) ;

	checkfree(&csp->pidfname) ;

	checkfree(&csp->lockfname) ;

	checkfree(&csp->logfname) ;

	checkfree(&csp->workdir) ;

	checkfree(&csp->port) ;

	checkfree(&csp->user) ;

	checkfree(&csp->group) ;

	checkfree(&csp->userpass) ;

	checkfree(&csp->machpass) ;

	checkfree(&csp->srvtab) ;

	checkfree(&csp->sendmail) ;

	checkfree(&csp->envfname) ;

	checkfree(&csp->pathfname) ;

	checkfree(&csp->devicefname) ;

	checkfree(&csp->seedfname) ;

	checkfree(&csp->logsize) ;

	checkfree(&csp->organization) ;

	checkfree(&csp->timeout) ;

	checkfree(&csp->removemul) ;

	checkfree(&csp->acctab) ;

	checkfree(&csp->paramfname) ;

	checkfree(&csp->nrecips) ;

	checkfree(&csp->helpfname) ;

	checkfree(&csp->statfname) ;

	checkfree(&csp->options) ;

	checkfree(&csp->interval) ;

	checkfree(&csp->stampdir) ;

	checkfree(&csp->maxjobs) ;

	checkfree(&csp->directory) ;

	checkfree(&csp->interrupt) ;

	checkfree(&csp->polltime) ;

	checkfree(&csp->filetime) ;

	checkfree(&csp->passfname) ;

	checkfree(&csp->eigenfname) ;

	csp->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (configfile_finish) */



/* LOCAL SUBROUTINES */



/* free up the resources occupied by a CONFIG_STRUCTURE */
static void checkfree(vp)
char	**vp ;
{


	if (*vp != NULL) {

	    uc_free(*vp) ;

	    *vp = NULL ;
	}
}
/* end subroutine (checkfree) */


#if	CF_DEBUGS

static int vardump(pathbuf,pbi)
const char	pathbuf[] ;
int		pbi ;
	{
	int	rs = SR_OK ;
		int	mlen, rlen = pbi ;
	int	wlen = 0 ;

		const char	*pp = pathbuf ;


		while (rlen > 0) {
			mlen = MIN(rlen,40) ;
			rs = debugprintf("configfile: var| %t\n",
				pp,mlen) ;

			wlen += rs ;
			if (rs < 0)
				break ;

			pp += mlen ;
			rlen -= mlen ;
		}

	return (rs >= 0) ? wlen : rs ;
	}
/* end subroutine (vardump) */

#endif /* CF_DEBUGS */



