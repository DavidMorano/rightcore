/* main (filterelf) */

/* generic front-end */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-06-01, David A­D­ Morano

	This program was written for handling (filtering) backup
	file lists.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ filterelf


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 1),2048)
#endif


/* external subroutines */

extern int	sfshrink(const char *,int,char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isdigitlatin(int) ;

extern char	*strbasename(char *) ;
extern char	*strshrink(char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* forward references */

static int	isfileok(const char *) ;


/* local structures */


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"af",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_af,
	argopt_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	struct ustat	sb ;

	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		errfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, i, j, k ;
	int	len, sl, cl ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_self = FALSE ;
	int	f_entok = FALSE ;
	int	f ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	linebuf[NODENAMELEN + 1] ;
	char	*pr = NULL ;
	char	*afname = NULL ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	memset(pip,0,sizeof(struct proginfo)) ;

	pip->version = VERSION ;
	pip->progname = strbasename(argv[0]) ;

	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* initialize */

	pip->verboselevel = 1 ;

/* start parsing the arguments */

	rs = SR_OK ;
	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	ai_max = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {
		    const int	ach = MKCHAR(argp[1]) ;

	            if (isdigitlatin(ach)) {

	                if (((argl - 1) > 0) && 
	                    (cfdeci(argp + 1,argl - 1,&argvalue) < 0))
	                    goto badarg ;

	            } else {

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

	                }

/* do we have a keyword match or should we assume only key letters? */

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                    switch (kwi) {

/* version */
	                    case argopt_version:
	                        f_version = TRUE ;
	                        if (f_optequal)
	                            goto badargextra ;

	                        break ;

/* verbose mode */
	                    case argopt_verbose:
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }
	                        }

	                        break ;

/* handle all keyword defaults */
	                    default:
	                        ex = EX_USAGE ;
	                        f_usage = TRUE ;
	                        bprintf(pip->efp,
					"%s: option (%s) not supported\n",
	                            pip->progname,akp) ;

	                        goto badarg ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {
			        const int	kc = MKCHAR(*akp) ;

	                        switch (kc) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,
	                                        &pip->debuglevel) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                }
	                            }

	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* quiet mode */
	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* verbose mode */
	                        case 'v':
	                            pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,
	                                        &pip->verboselevel) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                }
	                            }

	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
	                            break ;

	                        default:
	                            ex = EX_USAGE ;
	                            f_usage = TRUE ;
	                            bprintf(pip->efp,
					"%s: unknown option - %c\n",
	                                pip->progname,*aop) ;

	                        } /* end switch */

	                        akp += 1 ;
				if (rs < 0)
				    break ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minux sign character alone on the command line */

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                ai_max = i ;

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,i) ;
	            ai_max = i ;

	        } else {

	            if (! f_extra) {

			ex = EX_USAGE ;
	                f_extra = TRUE ;
	                bprintf(pip->efp,"%s: extra arguments specified\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: finished parsing arguments\n") ;
#endif

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel %d\n",
	        pip->progname,pip->debuglevel) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 2)
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

/* open output */

	rs = bopen(&outfile,BFILE_STDOUT,"dwct",0666) ;
	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: output unaccessible (%d)\n",
	        pip->progname,rs) ;
	    goto badopenout ;
	}

/* open input */

	rs = bopen(&infile,BFILE_STDIN,"dr",0666) ;
	if (rs < 0) {
	    ex = EX_NOINPUT ;
	    bprintf(pip->efp,"%s: input unaccessible (%d)\n",
	        pip->progname,rs) ;
	    goto badopenin ;
	}

/* read an input line */

	while ((rs = breadline(&infile,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    cl = sfshrink(linebuf,len,&cp) ;

	    if (cl > 0) {

	        cp[cl] = '\0' ;
	        if (isfileok(cp)) {

	            cp[cl++] = '\n' ;
	            rs = bwrite(&outfile,cp,cl) ;
		    if (rs < 0)
			break ;

	        } /* end if (file is OK) */

	    } /* end if (non-empty line) */

	} /* end while (reading lines) */

	bclose(&infile) ;

badopenin:
	bclose(&outfile) ;

/* we are done */
badopenout:
done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

/* early return thing */
retearly:
ret2:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

ret1:
	bclose(pip->efp) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* error types of returns */
badret:
	ex = EX_DATAERR ;
	goto retearly ;

/* the information type thing */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [-V]\n",
	    pip->progname,pip->progname) ;

	goto retearly ;

/* the bad things */
badargnum:
	bprintf(pip->efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badargextra:
	bprintf(pip->efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(pip->efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: bad argument(s) given\n",
	    pip->progname) ;

	goto badret ;

}
/* end subroutine (main) */


/* local subroutines */


static int isfileok(fname)
const char	fname[] ;
{
	struct ustat	sb ;

	int	rs ;
	int	fd ;

	char	buf[10] ;


	rs = u_open(fname,O_RDONLY,0666) ;
	fd = rs ;
	if (rs < 0)
	    goto ret0 ;

	rs = u_fstat(fd,&sb) ;

	if ((rs >= 0) && (! S_ISREG(sb.st_mode)))
	    rs = SR_NOTSUP ;

	if (rs >= 0) {

	    rs = u_read(fd,buf,4) ;

	    if (rs >= 4)
	        rs = (memcmp(buf,"\177ELF",4) != 0) ? SR_OK : SR_NOTSUP ;

	} /* end if (opened) */

	u_close(fd) ;

ret0:
	return (rs >= 0) ? TRUE : FALSE ;
}
/* end subroutine (isfileok) */



