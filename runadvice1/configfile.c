/* configfile */


#define	CF_DEBUG	0
#define	CF_DEBUGFIELD	0


/* revision history:

	- 01/06/01, David A­D­ Morano

	This subroutine was originally written (and largely forgotten).


	- 96/02/01, David A­D­ Morano

	This subroutine was adopted for use in the RUNADVICE program.


*/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<logfile.h>

#include	"localmisc.h"
#include	"configfile.h"



/* local defines */

#define	LINELEN		128
#define	BUFLEN		((MAXPATHLEN * 2) + LINELEN)



/* external subroutines */

extern char	*strncpylow(char *,const char *,int) ;


/* local static data */

static unsigned char 	fterms[32] = {
	0x7F, 0xFE, 0xC0, 0xFE,
	0x8B, 0x00, 0x00, 0x24, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
} ;

static char	*configkeys[] = {
	"export",
	"default",
	"advice",
	"control",
	"main",
	"params",
	"machine",
	NULL,
} ;


#define	CONFIGKEY_EXPORT	0
#define	CONFIGKEY_DEFAULT	1
#define	CONFIGKEY_ADVICE	2
#define	CONFIGKEY_CONTROL	3
#define	CONFIGKEY_MAIN		4
#define	CONFIGKEY_PARAMS	5
#define	CONFIGKEY_MACHINE	6







int configread(csp,configfname)
struct confighead	*csp ;
char			configfname[] ;
{
	bfile			configfile, *cfp = &configfile ;

	struct field		fsb ;

	vecstr		*vsp ;

	int	srs, rs = CFR_OK ;
	int	i ;
	int	c, len1, len, line = 0 ;

	char	linebuf[LINELEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	buf2[BUFLEN + 1] ;
	char	*bp, *cp ;


/* open configuration file */

	csp->srs = 0 ;
	csp->badline = -1 ;
	if ((srs = bopen(cfp,configfname,"r",0664)) < 0) goto badopen ;

/* initialize */

	csp->advice = NULL ;
	csp->runadvice_advice = NULL ;
	csp->tmpdir = NULL ;
	csp->control = NULL ;
	csp->main = NULL ;
	csp->params = NULL ;

	if ((srs = vecstr_start(&csp->exports,10,0)) < 0) goto badexports ;

	if ((srs = vecstr_start(&csp->defaults,10,0)) < 0) goto baddefaults ;

	if ((srs = vecstr_start(&csp->machines,10,0)) < 0) goto badmachines ;

/* start processing the configuration file */

	while ((len = breadline(cfp,linebuf,LINELEN)) > 0) {

	    line += 1 ;
	    if (len == 1) continue ;	/* blank line */

	    if (linebuf[len - 1] != '\n') {

#ifdef	COMMENT
	        f_trunc = TRUE ;
#endif
	        while ((c = bgetc(cfp)) >= 0) if (c == '\n') break ;

	        continue ;
	    }

	    fsb.rlen = len - 1 ;
	    fsb.lp = linebuf ;
#if	CF_DEBUGFIELD
	    debugprintf("configread: line> %W\n",fsb.lp,fsb.rlen) ;
#endif
	    field_get(&fsb,fterms) ;

#if	CF_DEBUGFIELD
	    debugprintf("configread: field> %W\n",fsb.fp,fsb.flen) ;
#endif


/* empty or comment only line */

	    if (fsb.flen <= 0) 
		continue ;

/* convert key to lower case */

	    strncpylow(buf,fsb.fp,fsb.flen) ;

	    buf[fsb.flen] = '\0' ;

/* check if key is a valid one, ignore invalid keys */

	    if ((i = matstr(configkeys,buf,fsb.flen)) >= 0) {

#if	CF_DEBUG
	debugprintf("configread: got a %s keyword\n",configkeys[i]) ;
#endif

	        switch (i) {

	        case CONFIGKEY_CONTROL:
	        case CONFIGKEY_MAIN:
	        case CONFIGKEY_PARAMS:
	        case CONFIGKEY_ADVICE:
	            field_get(&fsb,fterms) ;

	            if (fsb.flen == 0) 
			goto badconfig ;

	            bp = (char *) malloc(fsb.flen + 1) ;

	            strncpy(bp,fsb.fp,fsb.flen) ;

	            bp[fsb.flen] = '\0' ;

	            switch (i) {

	            case CONFIGKEY_ADVICE:
	                if (csp->advice != NULL) free(csp->advice) ;

	                csp->advice = bp ;
	                break ;

	            case CONFIGKEY_CONTROL:
	                if (csp->control != NULL) free(csp->control) ;

	                csp->control = bp ;
	                break ;

	            case CONFIGKEY_PARAMS:
	                if (csp->params != NULL) free(csp->params) ;

	                csp->params = bp ;
	                break ;

	            case CONFIGKEY_MAIN:
	                if (csp->main != NULL) free(csp->main) ;

	                csp->main = bp ;
	                break ;

	            } /* end switch (inner) */

	            break ;

/* export environment */
	        case CONFIGKEY_EXPORT:
	        case CONFIGKEY_DEFAULT:
	        case CONFIGKEY_MACHINE:
	            bp = buf2 ;

/* get first part */

	            field_get(&fsb,fterms) ;

	            if ((fsb.flen == 0) || (fsb.term == '#')) 
			goto badconfig  ;

	            len1 = fsb.flen ;
	            strncpy(bp,fsb.fp,fsb.flen) ;

	            bp += fsb.flen ;
	            *bp++ = '=' ;

/* get second part */

	            field_get(&fsb,fterms) ;

	            if (fsb.flen == 0) goto badconfig ;

	            strncpy(bp,fsb.fp,fsb.flen) ;

	            bp += fsb.flen ;
	            *bp++ = '\0' ;

/* store it away */

	            if (i == CONFIGKEY_EXPORT)
	                vsp = &csp->exports ;

	            else if (i == CONFIGKEY_DEFAULT)
	                vsp = &csp->defaults ;

	            else 
	                vsp = &csp->machines ;

#if	CF_DEBUG
	debugprintf("configread: about to add \n") ;
#endif

	            if ((srs = vecstr_add(vsp,buf2,-1)) < 0)
	                goto badalloc ;

#if	CF_DEBUG
	debugprintf("configread: added, srs=%d\n",
		srs) ;
#endif

/* if this is an export variable, we do extra stuff */

	            if (i == CONFIGKEY_EXPORT) {

#if	CF_DEBUG
	debugprintf("configread: got an export \"%s\", srs=%d\n",
		buf2,srs) ;
#endif

/* check for our favorite environment variables */

	                if (strncmp(buf2,"TMPDIR",len1) == 0) {

#if	CF_DEBUG
	debugprintf("configread: got a TMPDIR \"%s\", srs=%d\n",
		buf2,srs) ;
#endif

	                    csp->tmpdir = (csp->exports).va[srs] ;
#ifdef	COMMENT
				vecstr_get(vsp,srs,&csp->tmpdir) ;
#endif

			} else if (strncmp(buf2,"RUNADVICE_ADVICE",len1) == 0) {

#if	CF_DEBUG
	debugprintf("configread: got a RUNADVICE_ADVICE \"%s\", srs=%d\n",
		buf2,srs) ;
#endif

	                    csp->runadvice_advice = (csp->exports).va[srs] ;


			}

	            } /* end if (got an export) */

	            break ;

#ifdef	COMMENT
	        case KEY_WAIT:
	            field_get(&fsb,fterms) ;

	            if (fsb.flen == 0) goto badconfig ;

	            if (cfdec(fsb.fp,fsb.flen,&wtime) < 0) 
			goto badconfig ;

	            break ;
#endif

	        default:
	            goto badprogram ;

	        } /* end switch */

	    } /* end if (we got a valid keyword match) */

	} /* end while (reading lines) */

	bclose(cfp) ;

	srs = len ;
	if (len < 0) goto badread ;

/* done with configuration file processing */

	return OK ;

/* handle bad things */
badalloc:
	if (csp->srs == 0) {

		csp->srs = srs ;
		rs = CFR_BADALLOC ;
	}

badprogram:
	if (csp->srs == 0) {

		csp->srs = srs ;
		rs = CFR_BADPROGRAM ;
	}

badread:
	if (csp->srs == 0) {

		csp->srs = srs ;
		rs = CFR_BADREAD ;
	}

badconfig:
	if (csp->srs == 0) {

		csp->srs = srs ;
		rs = CFR_BADCONFIG ;
		csp->badline = line ;
	}

	vecstr_finish(&csp->machines) ;

badmachines:
	if (csp->srs == 0) {

		csp->srs = srs ;
		rs = CFR_BADMACHINES ;
	}

	vecstr_finish(&csp->defaults) ;

baddefaults:
	if (csp->srs == 0) {

		csp->srs = srs ;
		rs = CFR_BADDEFAULTS ;
	}

	vecstr_finish(&csp->exports) ;

badexports:
	if (csp->srs == 0) {

		csp->srs = srs ;
		rs = CFR_BADEXPORTS ;
	}

	bclose(cfp) ;

badopen:
	if (csp->srs == 0) {

		csp->srs = srs ;
		rs = CFR_BADOPEN ;
	}

	return rs ;
}
/* end subroutine (configread) */


/* free up the resources occupied by a CONFIG_STRUCTURE */
int configfree(csp)
struct confighead	*csp ;
{


/* free up the complex data types */

	vecstr_finish(&csp->exports) ;

	vecstr_finish(&csp->defaults) ;

	vecstr_finish(&csp->machines) ;

/* free up the simple ones */

	if (csp->advice != NULL) {

	    free(csp->advice) ;

	    csp->advice = NULL ;
	}

	if (csp->main != NULL) {

	    free(csp->main) ;

	    csp->main = NULL ;
	}

	if (csp->params != NULL) {

	    free(csp->params) ;

	    csp->params = NULL ;
	}

	if (csp->control != NULL) {

	    free(csp->control) ;

	    csp->control = NULL ;
	}

	return OK ;
}
/* end subroutine (configfree) */



