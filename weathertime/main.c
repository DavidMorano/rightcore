/* main */

/* program to set the modification time on a decoded NWS observation file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_SETTMZO	1


/* revision history:

	= 1999-08-17, David A­D­ Morano
	Some of the source for this program was grabbed from elsewhere.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ weathertime <filename>


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<utime.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<field.h>
#include	<dater.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LINELEN
#define	LINELEN		100
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;

extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

#define	A	(__STDC__ != 0) 
#define	B	defined(_POSIX_C_SOURCE) 
#define	C	defined(_XOPEN_SOURCE)

#if	(A != 0) || (B != 0) || (C != 0)
extern long	altzone ;
#endif


/* local structures */

struct obdate {
	int	mday ;
	int	hour ;
	int	min ;
} ;


/* forward references */

static int	out_open(PROGINFO *) ;
static int	out_close(PROGINFO *) ;
static int	getobdate(PROGINFO *,cchar *,int,struct obdate *) ;

static int	ismonthok(int,int) ;


/* locally defined global variables */


/* local variables */

static const char	*argopts[] = {
	    "VERSION",
	    "VERBOSE",
	    NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_overlast
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		infile ;

	int	argr, argl, aol, akl, avl ;
	int	argvalue = -1 ;
	int	maxai, pan, npa, kwi, i, j, k ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	rs, iw ;
	int	sl ;
	int	ex = EX_INFO ;
	int	fd_debug ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*obfname = NULL ;
	const char	*sp ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;
	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;
#endif /* CF_DEBUGS */


#ifdef	COMMENT
	(void) memset(pip,0,sizeof(PROGINFO)) ;
#endif

	pip->progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;


/* continue with the rest of the program */

	ex = EX_INFO ;

/* initialize */

	pip->programroot = NULL ;
	pip->efp = efp ;
	pip->ofp = ofp ;
	pip->verboselevel = 1 ;
	pip->f.no = FALSE ;
	pip->f.quiet = FALSE ;
	pip->f.outopen = FALSE ;

/* get the current time-of-day */

	{
	    struct tm	st, *stp ;

	    char	*tznp ;

	    int	zo ;


	    uc_ftime(&pip->now) ;

	    stp = &st ;
	    uc_localtime(&pip->now.time,&st) ;

	    zo = (stp->tm_isdst <= 0) ? timezone : altzone ;
	    pip->now.timezone = zo / 60 ;
	    pip->now.dstflag = daylight ;

	    tznp = (stp->tm_isdst <= 0) ? tzname[0] : tzname[1] ;
	    strncpy(pip->zname,tznp,DATE_ZNAMESIZE) ;

	    dater_start(&pip->tmpdate,&pip->now,pip->zname,-1) ;

	} /* end block (getting some current time stuff) */

/* start parsing the arguments */

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while (argr > 0) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                if (((argl - 1) > 0) && 
	                    (cfdeci(argp + 1,argl - 1,&argvalue) < 0))
	                    goto badargval ;

	            } else {

#if	CF_DEBUGS
	                debugprintf("main: got an option\n") ;
#endif

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key w/ a value\n") ;
#endif

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

#if	CF_DEBUGS
	                    debugprintf("main: aol=%d avp=\"%s\" avl=%d\n",
	                        aol,avp,avl) ;

	                    debugprintf("main: akl=%d akp=\"%s\"\n",
	                        akl,akp) ;
#endif

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

	                }

/* do we have a keyword match or should we assume only key letters ?
					    */

#if	CF_DEBUGS
	                debugprintf("main: about to check for a key word match\n") ;
#endif

	                if ((kwi = matstr(argopts,akp,akl)) >= 0) {

#if	CF_DEBUGS
	                    debugprintf("main: option keyword=%W kwi=%d\n",
	                        akp,akl,kwi) ;
#endif

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

	                                rs = cfdeci(avp,avl,&iw) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                                pip->verboselevel = iw ;
	                            }
	                        }

	                        break ;

/* handle all keyword defaults */
	                    default:
	                        f_usage = TRUE ;
	                        ex = EX_USAGE ;
	                        bprintf(efp,"%s: option (%s) not supported\n",
	                            pip->progname,akp) ;

	                        goto badarg ;

	                    } /* end switch */

	                } else {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key letter\n") ;
#endif

	                    while (akl--) {

#if	CF_DEBUGS
	                        debugprintf("main: option key letters\n") ;
#endif

	                        switch ((int) *akp) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,&iw) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                    pip->debuglevel = iw ;
	                                }
	                            }

	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

	                        case 'n':
	                            pip->f.no = TRUE ;
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

	                                    rs = cfdeci(avp,avl,&iw) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                    pip->verboselevel = iw ;
	                                }
	                            }

	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
	                            break ;

	                        default:
	                            f_usage = TRUE ;
	                            ex = EX_USAGE ;
	                            bprintf(efp,"%s: unknown option - %c\n",
	                                pip->progname,*aop) ;

	                        } /* end switch */

	                        akp += 1 ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minux sign character alone on the command line */

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                ex = EX_USAGE ;
	                bprintf(efp,"%s: extra arguments ignored\n",
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
	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage || f_extra)
	    goto usage ;

	if (f_version)
	    goto earlyret ;

	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: special debugging turned on\n") ;
#endif


/* do some stuff early ? */



/* get the first positional argument as the filename to read */

	pan = 0 ;
	for (i = 0 ; i <= maxai ; i += 1) {

	    if (! BATST(argpresent,i))
	        continue ;

	    switch (pan) {

	    case 0:
	        obfname = argv[i] ;
	        pan += 1 ;
	        break ;

	    } /* end switch */

	} /* end for */



/* check up on the arguments */

	if ((obfname == NULL) || (obfname[0] == '\0'))
	    goto badnofile ;


	ex = EX_OK ;

	rs = bopen(&infile,obfname,"r",0777) ;

	if (rs >= 0) {

	    int	len ;

	    char	linebuf[LINELEN + 1] ;


	    while ((len = breadline(&infile,linebuf,LINELEN)) > 0) {

	        if (strncmp(linebuf,"ob:",3) == 0)
	            break ;

	    } /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main: len=%d linebuf=>%t<\n",
		len,linebuf,strnlen(linebuf,30)) ;
#endif

	    rs = SR_NOTFOUND ;
	    if (len > 0) {

	        struct obdate	hm ;

	        FIELD		of ;


	        field_start(&of,linebuf + 3,len - 3) ;

	        field_get(&of,NULL) ;

	        sl = field_get(&of,NULL) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main: field=%t\n",
		of.fp,of.flen) ;
#endif

	        if ((sl > 0) && 
	            (getobdate(pip,of.fp,of.flen,&hm) >= 0)) {
	            struct utimbuf	ut ;
	            struct tm	ts ;
			DATER		moddate ;
	            time_t		modtime ;

			dater_start(&moddate,&pip->now,pip->zname,-1) ;

	            uc_gmtime(&pip->now.time,&ts) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main: GMT isdst=%d\n",ts.tm_isdst) ;
#endif

			ts.tm_mday = hm.mday ;
	            ts.tm_hour = hm.hour ;
	            ts.tm_min = hm.min ;
			ts.tm_sec = 0 ;
#if	CF_SETTMZO
			dater_settmzo(&moddate,&ts,0) ;
#else
			dater_settmzn(&moddate,&ts,"gmt",3) ;
#endif

			dater_gettime(&moddate,&modtime) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	debugprintf("main: LOCAL modtime=%s\n",
		timestr_log(modtime,timebuf)) ;
	debugprintf("main: GMT modtime=%s\n",
		timestr_gmlog(modtime,timebuf)) ;
	}
#endif

	            ut.actime = pip->now.time ;
	            ut.modtime = modtime ;
	            rs = utime(obfname,&ut) ;

			dater_finish(&moddate) ;

	        } /* end if */

	        field_finish(&of) ;

	    } /* end if */

	    bclose(&infile) ;

	} /* end if (opened the file) */


	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;


/* we are done */

done1:

done0:

	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: program exiting ex=%d\n",
	        pip->progname,ex) ;


/* early return thing */
earlyret:
badret:
	bclose(efp) ;

	return ex ;

/* the information type thing */
usage:
	bprintf(efp,
	    "%s: USAGE> %s file ",
	    pip->progname,pip->progname) ;

	bprintf(efp," [-v] [-DV]\n") ;

	goto earlyret ;

/* the ARGuments */
badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto earlyret ;

/* so-so things */
retnonetname:
	out_open(pip) ;

	if (pip->verboselevel > 0)
	    bprintf(pip->ofp,"you have no netname on this system !\n") ;

	goto retregular ;

retalready:
	out_open(pip) ;

	if (pip->verboselevel > 0)
	    bprintf(pip->ofp,"you are already key-logged in !\n") ;


retregular:
	out_close(pip) ;

	goto earlyret ;

/* bad ERRORs */
badnofile:
	ex = EX_NOINPUT ;
	if (! pip->f.quiet)
	    bprintf(efp,"%s: no filename was specified\n",
	        pip->progname) ;

	goto done0 ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int out_open(pip)
PROGINFO	*pip ;
{
	int	rs = SR_OK ;


	if (! pip->f.outopen) {

	    rs = bopen(pip->ofp,BFILE_STDOUT,"drwc",0666) ;

	    if (rs < 0)
	        return rs ;

	    pip->f.outopen = TRUE ;

	}

	return rs ;
}
/* end subroutine (out_open) */


static int out_close(pip)
PROGINFO	*pip ;
{
	int	rs = SR_OK ;


	if (pip->f.outopen) {

	    (void) bclose(pip->ofp) ;

	    pip->f.outopen = FALSE ;

	}

	return rs ;
}
/* end subroutine (out_close) */


/* get the hour and minutes from the METAR time field */
static int getobdate(PROGINFO *pip,cchar *sp,int sl,struct obdate *hmp)
{
	struct tm	ts ;
	int		rs ;
	int		iw ;

	if (sl < 7)
	    return SR_INVALID ;

	uc_gmtime(&pip->now.time,&ts) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("getobdate: mday=%d hour=%d min=%d\n",
		ts.tm_mday,ts.tm_hour,ts.tm_min) ;
#endif

	rs = cfdeci(sp,2,&iw) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("getobdate: cfdeci() rs=%d mday=%d\n",rs,iw) ;
#endif

	if (rs < 0)
	    return rs ;

	hmp->mday = iw ;

#ifdef	COMMENT
	if (! ismonthok(hmp->mday,ts.tm_mday))
	    return SR_INVALID ;
#endif /* COMMENT */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("getobdate: month is OK\n") ;
#endif

	rs = cfdeci(sp + 2,2,&iw) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("getobdate: cfdeci() rs=%d hour=%d\n",rs,iw) ;
#endif

	if (rs < 0)
	    return rs ;

	hmp->hour = iw ;
	rs = cfdeci(sp + 4,2,&iw) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("getobdate: cfdeci() rs=%d min=%d\n",rs,iw) ;
#endif

	hmp->min = iw ;
	return rs ;
}
/* end subroutine (getobdate) */


static int ismonthok(int today,int otherday)
{

	if (today == otherday)
	    return TRUE ;

	if ((otherday > 0) && (today == (otherday - 1)))
	    return TRUE ;

	return FALSE ;
}
/* end subroutine (ismonthok) */


