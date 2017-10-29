/* configfile */

/* parse a configuration file */


#define	F_DEBUGS	0
#define	F_DEBUGSFIELD	0


/* revision history :

	= 91/06/01, David A­D­ Morano

	This subroutine was originally written.


	= 00/01/21, David A­D­ Morano

	This subroutine was enhanced for use by LevoSim.


*/


/******************************************************************************

	This is the old configuration file reader object.  It is cheap,
	it is ill-conceived, it is a mess, it works well enough to be
	used for cheap code.  I didn't want to use this junk for the
	Levo machine simulator but time pressure decided for us !

	Although this whole configuration scheme is messy, it gives
	us enough of what we need to get some configuration information
	into the Levo machine simulator and to get a parameter file
	name.  This is good enough for now.


******************************************************************************/


#define	CONFIGFILE_MASTER	1


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<buffer.h>
#include	<mallocstuff.h>

#include	"misc.h"
#include	"configfile.h"

#if	F_DEBUGS
#include	"config.h"
#include	"defs.h"
#endif



/* local defines */

#define	CONFIGFILE_MAGIC	0x04311633

#undef	LINELEN
#define	LINELEN		128
#undef	BUFLEN
#define	BUFLEN		(LINELEN * 2)



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecmfi(const char *,int,int *) ;

extern char	*strncpylow(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static void	checkfree() ;


/* local static data */

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

static char	*const configkeys[] = {
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


#define	CONFIGKEY_DEFINE	0
#define	CONFIGKEY_EXPORT	1
#define	CONFIGKEY_TMPDIR	2
#define	CONFIGKEY_ROOT		3
#define	CONFIGKEY_PIDFILE	4
#define	CONFIGKEY_LOCKFILE	5
#define	CONFIGKEY_LOG		6
#define	CONFIGKEY_LOGLEN	7
#define	CONFIGKEY_WORKDIR	8
#define	CONFIGKEY_PORT		9
#define	CONFIGKEY_USER		10
#define	CONFIGKEY_GROUP		11
#define	CONFIGKEY_USERPASS	12
#define	CONFIGKEY_MACHPASS	13
#define	CONFIGKEY_SRVTAB	14
#define	CONFIGKEY_SENDMAIL	15
#define	CONFIGKEY_ENVFILE	16
#define	CONFIGKEY_PATHFILE	17
#define	CONFIGKEY_DEVICEFILE	18
#define	CONFIGKEY_SEEDFILE	19
#define	CONFIGKEY_LOGSIZE	20
#define	CONFIGKEY_ORGANIZATION	21
#define	CONFIGKEY_UNSET		22
#define	CONFIGKEY_TIMEOUT	23
#define	CONFIGKEY_REMOVEMUL	24
#define	CONFIGKEY_ACCTAB	25
#define	CONFIGKEY_PARAMFILE	26
#define	CONFIGKEY_NRECIPS	27
#define	CONFIGKEY_HELPFILE	28
#define	CONFIGKEY_PARAMTAB	29
#define	CONFIGKEY_PINGTAB	30
#define	CONFIGKEY_PINGSTAT	31
#define	CONFIGKEY_OPTION	32
#define	CONFIGKEY_MINTEXEC	33
#define	CONFIGKEY_INTERVAL	34
#define	CONFIGKEY_STAMPDIR	35
#define	CONFIGKEY_MAXJOBS	36
#define	CONFIGKEY_DIRECTORY	37
#define	CONFIGKEY_INTERRUPT	38
#define	CONFIGKEY_POLLTIME	39
#define	CONFIGKEY_FILETIME	40
#define	CONFIGKEY_PASSFILE	41






int configfile_init(csp,configfname)
CONFIGFILE	*csp ;
char		configfname[] ;
{
	bfile	configfile, *cfp = &configfile ;

	BUFFER	options ;

	FIELD	fsb ;

	vecstr	*vsp ;

	int	srs, rs = SR_OK ;
	int	i ;
	int	c, len1, len, line = 0 ;
	int	noptions = 0 ;

	char	linebuf[LINELEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	buf2[BUFLEN + 1] ;
	char	*bp, *cp ;


#if	F_DEBUGS
	eprintf("configfile_init: entered filename=%s\n",configfname) ;
#endif

	if (csp == NULL)
	    return SR_FAULT ;

	csp->magic = 0 ;
	if ((configfname == NULL) || (configfname[0] == '\0'))
	    return SR_NOEXIST ;

	(void) memset(csp,0,sizeof(CONFIGFILE)) ;

/* open configuration file */

#if	F_DEBUGS
	eprintf("configfile_init: opened file\n") ;
#endif

	csp->srs = 0 ;
	csp->badline = -1 ;
	if ((rs = bopen(cfp,configfname,"r",0664)) < 0)
	    goto badopen ;

/* initialize */

#if	F_DEBUGS
	eprintf("configfile_init: initializing\n") ;
#endif

	csp->root = NULL ;
	csp->tmpdir = NULL ;
	csp->pidfname = NULL ;
	csp->lockfname = NULL ;
	csp->logfname = NULL ;
	csp->loglen = -1 ;
	csp->workdir = NULL ;
	csp->port = NULL ;
	csp->user = NULL ;
	csp->group = NULL ;
	csp->userpass = NULL ;
	csp->machpass = NULL ;
	csp->srvtab = NULL ;
	csp->sendmail = NULL ;
	csp->envfname = NULL ;
	csp->pathfname = NULL ;
	csp->devicefname = NULL ;
	csp->seedfname = NULL ;
	csp->logsize = NULL ;
	csp->organization = NULL ;
	csp->timeout = NULL ;
	csp->removemul = NULL ;
	csp->acctab = NULL ;
	csp->paramfname = NULL ;
	csp->nrecips = NULL ;
	csp->helpfname = NULL ;
	csp->statfname = NULL ;
	csp->options = NULL ;
	csp->interval = NULL ;
	csp->stampdir = NULL ;
	csp->maxjobs = NULL ;
	csp->directory = NULL ;
	csp->interrupt = NULL ;
	csp->polltime = NULL ;
	csp->filetime = NULL ;
	csp->passfname = NULL ;

	buffer_init(&options,-1) ;

	if ((rs = vecstr_start(&csp->defines,10,0)) < 0)
	    goto baddefines ;

	if ((rs = vecstr_start(&csp->unsets,10,0)) < 0)
	    goto badunsets ;

	if ((rs = vecstr_start(&csp->exports,10,0)) < 0)
	    goto badexports ;


/* start processing the configuration file */

#if	F_DEBUGS
	eprintf("configfile_init: reading lines\n") ;
#endif

	while ((len = bgetline(cfp,linebuf,LINELEN)) > 0) {

	    line += 1 ;
	    if (len == 1) continue ;	/* blank line */

	    if (linebuf[len - 1] != '\n') {

#ifdef	COMMENT
	        f_trunc = TRUE ;
#endif
	        while ((c = bgetc(cfp)) >= 0)
	            if (c == '\n') break ;

	        continue ;
	    }

	    fsb.lp = linebuf ;
	    fsb.rlen = len - 1 ;

#if	F_DEBUGS
	    eprintf("configfile_init: line> %W\n",fsb.lp,fsb.rlen) ;
#endif

	    field_get(&fsb,fterms) ;

#if	F_DEBUGS
	    {
	        if (fsb.flen >= 0) {
	            eprintf("configfile_init: field> %W\n",
	                fsb.fp,fsb.flen) ;
	        } else
	            eprintf("configfile_init: field> *none*\n") ;
	    }
#endif /* F_DEBUGSFIELD */

/* empty or comment only line */

	    if (fsb.flen <= 0) continue ;

/* convert key to lower case */

	    strncpylow(buf,fsb.fp,fsb.flen) ;

	    buf[fsb.flen] = '\0' ;

/* check if key is a valid one, ignore invalid keys */

	    i = optmatch(configkeys,buf,fsb.flen) ;

	    if (i >= 0) {

#if	F_DEBUGS
	        eprintf("configfile_init: i=%d keyword=%s\n",
	            i,configkeys[i]) ;
#endif

	        switch (i) {

	        case CONFIGKEY_ROOT:
	        case CONFIGKEY_TMPDIR:
	        case CONFIGKEY_LOG:
	        case CONFIGKEY_WORKDIR:
	        case CONFIGKEY_PIDFILE:
	        case CONFIGKEY_LOCKFILE:
	        case CONFIGKEY_USER:
	        case CONFIGKEY_GROUP:
	        case CONFIGKEY_PORT:
	        case CONFIGKEY_USERPASS:
	        case CONFIGKEY_MACHPASS:
	        case CONFIGKEY_SRVTAB:
	        case CONFIGKEY_SENDMAIL:
	        case CONFIGKEY_MINTEXEC:
	        case CONFIGKEY_ENVFILE:
	        case CONFIGKEY_PATHFILE:
	        case CONFIGKEY_LOGSIZE:
	        case CONFIGKEY_ORGANIZATION:
	        case CONFIGKEY_TIMEOUT:
	        case CONFIGKEY_REMOVEMUL:
	        case CONFIGKEY_ACCTAB:
	        case CONFIGKEY_PARAMFILE:
	        case CONFIGKEY_PARAMTAB:
	        case CONFIGKEY_NRECIPS:
	        case CONFIGKEY_HELPFILE:
	        case CONFIGKEY_PINGTAB:
	        case CONFIGKEY_PINGSTAT:
	        case CONFIGKEY_INTERVAL:
	        case CONFIGKEY_STAMPDIR:
	        case CONFIGKEY_MAXJOBS:
	        case CONFIGKEY_DIRECTORY:
	        case CONFIGKEY_INTERRUPT:
	        case CONFIGKEY_POLLTIME:
	        case CONFIGKEY_FILETIME:
	        case CONFIGKEY_PASSFILE:
	            field_get(&fsb,fterms) ;

	            if (fsb.flen > 0)
	                bp = mallocstrn(fsb.fp,fsb.flen) ;

	            else 
	                bp = mallocstrn(buf,0) ;

#if	F_DEBUGSFIELD
	            eprintf("configfile_init: bp=%s\n",bp) ;
#endif

	            switch (i) {

	            case CONFIGKEY_ROOT:
	                if (csp->root != NULL)
	                    free(csp->root) ;

	                csp->root = bp ;
	                break ;

	            case CONFIGKEY_LOG:
	                if (csp->logfname != NULL)
	                    free(csp->logfname) ;

	                csp->logfname = bp ;
	                break ;

	            case CONFIGKEY_TMPDIR:
	                if (csp->tmpdir != NULL)
	                    free(csp->tmpdir) ;

	                csp->tmpdir = bp ;
	                break ;

	            case CONFIGKEY_WORKDIR:
	                if (csp->workdir != NULL)
	                    free(csp->workdir) ;

	                csp->workdir = bp ;
	                break ;

	            case CONFIGKEY_USER:
	                if (csp->user != NULL)
	                    free(csp->user) ;

	                csp->user = bp ;
	                break ;

	            case CONFIGKEY_GROUP:
	                if (csp->group != NULL)
	                    free(csp->group) ;

	                csp->group = bp ;
	                break ;

	            case CONFIGKEY_PIDFILE:
	                if (csp->pidfname != NULL)
	                    free(csp->pidfname) ;

	                csp->pidfname = bp ;
#if	F_DEBUGS
	                eprintf("configfile_init: pidfname=%s\n",bp) ;
#endif

	                break ;

	            case CONFIGKEY_LOCKFILE:
	                if (csp->lockfname != NULL)
	                    free(csp->lockfname) ;

	                csp->lockfname = bp ;
	                break ;

	            case CONFIGKEY_PORT:
	                if (csp->port != NULL)
	                    free(csp->port) ;

	                csp->port = bp ;
	                break ;

	            case CONFIGKEY_USERPASS:
	                if (csp->userpass != NULL)
	                    free(csp->userpass) ;

	                csp->userpass = bp ;
	                break ;

	            case CONFIGKEY_MACHPASS:
	                if (csp->machpass != NULL)
	                    free(csp->machpass) ;

	                csp->machpass = bp ;
	                break ;

	            case CONFIGKEY_SRVTAB:
	                if (csp->srvtab != NULL)
	                    free(csp->srvtab) ;

	                csp->srvtab = bp ;
	                break ;

	            case CONFIGKEY_SENDMAIL:
	            case CONFIGKEY_MINTEXEC:
	                if (csp->sendmail != NULL)
	                    free(csp->sendmail) ;

	                csp->sendmail = bp ;
	                break ;

	            case CONFIGKEY_ENVFILE:
	                if (csp->envfname != NULL)
	                    free(csp->envfname) ;

	                csp->envfname = bp ;
	                break ;

	            case CONFIGKEY_PATHFILE:
	                if (csp->pathfname != NULL)
	                    free(csp->pathfname) ;

	                csp->pathfname = bp ;
	                break ;

	            case CONFIGKEY_DEVICEFILE:
	                if (csp->devicefname != NULL)
	                    free(csp->devicefname) ;

	                csp->devicefname = bp ;
	                break ;

	            case CONFIGKEY_SEEDFILE:
	                if (csp->seedfname != NULL)
	                    free(csp->seedfname) ;

	                csp->seedfname = bp ;
	                break ;

	            case CONFIGKEY_LOGSIZE:
	                if (csp->logsize != NULL)
	                    free(csp->logsize) ;

	                csp->logsize = bp ;
	                break ;

	            case CONFIGKEY_ORGANIZATION:
	                if (csp->organization != NULL)
	                    free(csp->organization) ;

	                csp->organization = bp ;
	                break ;

	            case CONFIGKEY_TIMEOUT:
	                if (csp->timeout != NULL)
	                    free(csp->timeout) ;

	                csp->timeout = bp ;
	                break ;

	            case CONFIGKEY_INTERVAL:
	                if (csp->interval != NULL)
	                    free(csp->interval) ;

	                csp->interval = bp ;
	                break ;

	            case CONFIGKEY_REMOVEMUL:
	                if (csp->removemul != NULL)
	                    free(csp->removemul) ;

	                csp->removemul = bp ;
	                break ;

	            case CONFIGKEY_ACCTAB:
	                if (csp->acctab != NULL)
	                    free(csp->acctab) ;

	                csp->acctab = bp ;
	                break ;

	            case CONFIGKEY_PARAMFILE:
	            case CONFIGKEY_PARAMTAB:
	            case CONFIGKEY_PINGTAB:
	                if (csp->paramfname != NULL)
	                    free(csp->paramfname) ;

	                csp->paramfname = bp ;
	                break ;

	            case CONFIGKEY_NRECIPS:
	                if (csp->nrecips != NULL)
	                    free(csp->nrecips) ;

	                csp->nrecips = bp ;
	                break ;

	            case CONFIGKEY_HELPFILE:
	                if (csp->helpfname != NULL)
	                    free(csp->helpfname) ;

	                csp->helpfname = bp ;
	                break ;

	            case CONFIGKEY_PINGSTAT:
	                if (csp->statfname != NULL)
	                    free(csp->statfname) ;

	                csp->statfname = bp ;
	                break ;

	            case CONFIGKEY_STAMPDIR:
	                if (csp->stampdir != NULL)
	                    free(csp->stampdir) ;

	                csp->stampdir = bp ;
	                break ;

	            case CONFIGKEY_MAXJOBS:
	                if (csp->maxjobs != NULL)
	                    free(csp->maxjobs) ;

	                csp->maxjobs = bp ;
	                break ;

	            case CONFIGKEY_DIRECTORY:
	                if (csp->directory != NULL)
	                    free(csp->directory) ;

	                csp->directory = bp ;
	                break ;

	            case CONFIGKEY_INTERRUPT:
	                if (csp->interrupt != NULL)
	                    free(csp->interrupt) ;

	                csp->interrupt = bp ;
	                break ;

	            case CONFIGKEY_POLLTIME:
	                if (csp->polltime != NULL)
	                    free(csp->polltime) ;

	                csp->polltime = bp ;
	                break ;

	            case CONFIGKEY_FILETIME:
	                if (csp->filetime != NULL)
	                    free(csp->filetime) ;

	                csp->filetime = bp ;
	                break ;

	            case CONFIGKEY_PASSFILE:
	                if (csp->passfname != NULL)
	                    free(csp->passfname) ;

	                csp->passfname = bp ;
	                break ;

	            } /* end switch (inner) */

	            break ;

/* options */
	        case CONFIGKEY_OPTION:

#if	F_DEBUGS
	            eprintf("configfile_init: option=>%W<\n",
	                fsb.fp,fsb.flen) ;
#endif

	            while ((fsb.term != '#') &&
	                (field_get(&fsb,oterms) >= 0)) {

	                if (fsb.flen > 0) {

	                    if (noptions > 0)
	                        buffer_addchar(&options,',') ;

	                    buffer_addstrn(&options,fsb.fp,fsb.flen) ;

	                    noptions += 1 ;
	                }

	            } /* end while */

	            break ;

/* unsets */
	        case CONFIGKEY_UNSET:
	            field_get(&fsb,fterms) ;

	            if (fsb.flen > 0) {

	                if ((rs = vecstr_add(&csp->unsets,fsb.fp,fsb.flen)) < 0)
	                    goto badalloc ;

	            }

	            break ;

/* export environment */
	        case CONFIGKEY_DEFINE:
	        case CONFIGKEY_EXPORT:
	            bp = buf2 ;

/* get first part */

#if	F_DEBUGS
	            eprintf("configfile_init: D/E first part\n") ;
#endif

	            field_get(&fsb,fterms) ;

#if	F_DEBUGS
	            eprintf("configfile_init: D/E first part flen=%d\n",
	                fsb.flen) ;
#endif

	            if (fsb.flen <= 0) {

	                rs = SR_INVALID ;
	                csp->badline = line ;
	                goto badconfig  ;
	            }

	            len1 = fsb.flen ;
	            strncpy(bp,fsb.fp,fsb.flen) ;

	            bp += fsb.flen ;

/* get second part */

	            field_get(&fsb,fterms) ;

	            if (fsb.flen >= 0) {

#if	F_DEBUGS
	                eprintf("configfile_init: D/E field >%W<\n",
	                    fsb.fp,fsb.flen) ;
#endif

	                *bp++ = '=' ;
	                strncpy(bp,fsb.fp,fsb.flen) ;

	                bp += fsb.flen ;

	            }

	            *bp++ = '\0' ;

/* store it away */

	            if (i == CONFIGKEY_EXPORT)
	                vsp = &csp->exports ;

	            else
	                vsp = &csp->defines ;

#if	F_DEBUGS
	            eprintf("configfile_init: about to add >%s<\n",buf2) ;
#endif

	            if ((rs = vecstr_add(vsp,buf2,-1)) < 0)
	                goto badalloc ;

#if	F_DEBUGS
	            eprintf("configfile_init: added, rs=%d\n",
	                rs) ;
#endif

/* if this is an export variable, we do extra stuff */

	            if (i == CONFIGKEY_EXPORT) {

	                int	index ;


	                index = rs ;

#if	F_DEBUGS
	                eprintf("configfile_init: export=>%s< i=%d\n", 
	                    buf2,index) ;
#endif

/* check for our favorite environment variables */

	                if (strncmp(buf2,"TMPDIR",len1) == 0) {

#if	F_DEBUGS
	                    eprintf("configfile_init: TMPDIR=>%s< i=%d\n", 
	                        buf2,index) ;
#endif

	                    if (csp->tmpdir != NULL)
	                        free(csp->tmpdir) ;

	                    csp->tmpdir = mallocstr((csp->exports).va[index]) ;

	                } /* end if (handling TMPDIR specially) */

	            } /* end if (got an export) */

	            break ;

	        case CONFIGKEY_LOGLEN:
	            field_get(&fsb,fterms) ;

	            if ((fsb.flen <= 0) ||
	                (cfdecmfi(fsb.fp,fsb.flen,&csp->loglen) < 0)) {

	                csp->badline = line ;
	                rs = SR_INVALID ;
	                goto badconfig ;
	            }

	            break ;

	        default:
	            rs = SR_NOTSUP ;
	            goto badprogram ;

	        } /* end switch */

	    } /* end if (valid key) */

	} /* end while (reading lines) */

#if	F_DEBUGS
	eprintf("configfile_init: done reading lines\n") ;
#endif

	bclose(cfp) ;

	srs = len ;
	if (len < 0) {

	    rs = len ;
	    goto badread ;
	}


/* load up the options if we got any */

	if (noptions > 0) {

	    len = buffer_get(&options,&cp) ;

#if	F_DEBUGS
	    eprintf("configfile_init: final options=>%W<\n",cp,len) ;
#endif

	    if (len > 0)
	        csp->options = mallocstrn(cp,len) ;

	} /* end if (options) */


/* done with configuration file processing */

	buffer_free(&options) ;

#if	F_DEBUGS
	eprintf("configfile_init: exiting OK\n") ;
#endif

	csp->magic = CONFIGFILE_MAGIC ;
	return SR_OK ;

/* handle bad things */
badalloc:
badprogram:
badread:
badconfig:
	csp->badline = line ;
	vecstr_finish(&csp->exports) ;

badexports:
	vecstr_finish(&csp->unsets) ;

badunsets:
	vecstr_finish(&csp->defines) ;

baddefines:
	bclose(cfp) ;

	buffer_free(&options) ;

badopen:

#if	F_DEBUGS
	eprintf("configfile_init: exiting bad rs=%d\n",rs) ;
#endif

	return rs ;

badkey:
	rs = SR_INVALID ;
	goto badprogram ;
}
/* end subroutine (configfile_init) */


/* free up this object */
int configfile_free(csp)
CONFIGFILE	*csp ;
{


	if (csp == NULL)
	    return SR_FAULT ;

	if (csp->magic != CONFIGFILE_MAGIC)
	    return SR_NOTOPEN ;

	csp->magic = 0 ;

/* free up the complex data types */

	vecstr_finish(&csp->defines) ;

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

	return SR_OK ;
}
/* end subroutine (configfile_free) */



/* LOCAL SUBROUTINES */



/* free up the resources occupied by a CONFIG_STRUCTURE */
static void checkfree(vp)
char	**vp ;
{


	if (*vp != NULL) {

	    free(*vp) ;

	    *vp = NULL ;
	}
}
/* end subroutine (checkfree) */



