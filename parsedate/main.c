/* main (parsedate) */

/* the "parsedate" daemon */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */
#define	CF_GETDATE2	0


/* revision history:

	= 1997-05-09, David A­D­ Morano
	This is is the original writing of this subroutine and program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We parse date-strings here.


*******************************************************************************/


#include	<envatandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<time.h>
#include	<string.h>
#include	<ctype.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<char.h>
#include	<baops.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"incfile_datemsk.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* local structures */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"TMPDIR",
	NULL
} ;

#define	ARGOPT_ROOT	0
#define	ARGOPT_VERSION	1
#define	ARGOPT_TMPDIR	2


/* extern subroutines */

extern time_t	getabsdate(), getindate() ;

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;

extern char	*malloc_str(), *malloc_sbuf() ;
extern char	*strbasename() ;
extern char	*timestr_log(), *timestr_gmlog() ;


/* external variables */


/* forward references */

static struct tm	*getdate2() ;

static void		openerrfile() ;
static void		printout() ;


/* local global variables */

struct global	g ;


/* exported subroutines */


int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		infile, *ifp = NULL ;
	bfile		outfile, *ofp = &outfile ;
	bfile		errfile ;

	struct tm	ts, *timep ;

	struct ustat	sb ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	ngi, i, j ;
	int	rs ;
	int	len ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_done = FALSE ;
	int	f_version = FALSE ;
	int	efd ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	mskfname_buf[MAXPATHLEN + 1] ;
	char	*mskfname = NULL ;
	char	*datestring = NULL ;
	char	buf[BUFSIZE + 1] ;
	char	linebuf[LINELEN + 1] ;
	char	*cp, *cp1, *cp2 ;


#if	CF_DEBUGS || CF_DEBUG
	if (fstat(3,&sb) >= 0) {
	    debugsetfd(3) ;
	} else {
	    while ((efd = dup(2)) < 3) ;
	    debugsetfd(efd) ;
	}
#endif

#if	CF_DEBUGS
	debugprintf("main: entered\n") ;
#endif

	g.progname = strbasename(argv[0]) ;

	efd = dup(2) ;

	g.efp = NULL ;
	if (bopen(&errfile,(char *) efd,"dwca",0644) >= 0)
	    g.efp = &errfile ;


/* get some program information */

	if ((g.programroot = getenv("PROGRAMROOT")) == NULL)
	    g.programroot = PROGRAMROOT ;


/* initialize some stuff before command line argument processing */

	if ((g.tmpdir = getenv("TMPDIR")) == NULL)
	    g.tmpdir = TMPDIR ;

	g.debuglevel = 0 ;

	g.f.verbose = FALSE ;
	g.f.quiet = FALSE ;
	g.f.daemon = FALSE ;
	g.f.tmpfile = FALSE ;
	g.f.alternate = FALSE ;


/* process program arguments */

	rs = SR_OK ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (! f_done) && (argr > 0)) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	                f_optequal = TRUE ;

	            } else {

	                avl = 0 ;
	                akl = aol ;

	            }

/* do we have a keyword match or should we assume only key letters ? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

	                case ARGOPT_ROOT:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) g.programroot = avp ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) g.programroot = argp ;

	                    }

	                    break ;

/* a temporary directory */
	                case ARGOPT_TMPDIR:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) g.tmpdir = avp ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) g.tmpdir = argp ;

	                    }

	                    break ;

/* version */
	                case ARGOPT_VERSION:
	                    f_version = TRUE ;
	                    if (f_optequal) goto badargextra ;

	                    break ;

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;
	                        bprintf(g.efp,
	                            "%s: unknown argument keyword \"%s\"\n",
	                            g.progname,aop) ;

	                    f_usage = TRUE ;
	                    f_done = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *akp) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        g.debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (cfdec(avp,avl, &g.debuglevel) < 0)
	                                goto badargvalue ;

	                        }

	                        break ;

	                    case 'v':
	                        g.f.verbose = TRUE ;
	                        break ;

	                    case 'a':
	                        g.f.alternate = TRUE ;
	                        break ;

	                    case 'd':
	                        g.f.daemon = TRUE ;
	                        break ;

/* quiet mode */
	                    case 'q':
	                        g.f.quiet = TRUE ;
	                        break ;

/* get a DATEMSK file */
	                    case 'm':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) mskfname = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) mskfname = argp ;

	                        }

	                        break ;

/* date string */
	                    case 's':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) datestring = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) datestring = argp ;

	                        }

	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
				break ;

	                    default:
				rs = SR_INVALID ;
	                            bprintf(g.efp.
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                    } /* end switch */

	                    akp += 1 ;
			    if (rs < 0)
				break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } else {

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGGROUPS) {

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                if (g.efp != NULL)
	                    bprintf(g.efp,"%s: extra arguments ignored\n",
	                        g.progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

#if	CF_DEBUGS
	    debugprintf("main: bottom of loop\n") ;
#endif

	} /* end while (all command line argument processing) */

#if	CF_DEBUGS
	debugprintf("main: finished parsing arguments\n") ;
#endif

	if (f_version) {

	    if (g.efp != NULL)
	        bprintf(g.efp,"%s: version %s\n",
	            g.progname,VERSION) ;

	}

	if (g.debuglevel > 0) {

	    if (g.efp != NULL)
	        bprintf(g.efp,"%s: debuglevel %d\n",
	            g.progname,g.debuglevel) ;

	}

	if (f_usage) goto usage ;

	if (f_version) goto earlyret ;


/* process the arguments */

	if ((stat(g.tmpdir,&sb) < 0) && (! S_ISDIR(sb.st_mode)))
	    g.tmpdir = TMPDIR ;


	mskfname_buf[0] = '\0' ;
	if (mskfname == NULL) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("main: using internal DATEMSK specifications\n") ;
#endif

	    mkapth2(tmpfname,g.tmpdir,"pdXXXXXXXXXXXX") ;

	    rs = mktmpfile(tmpfname, 0664, mskfname_buf) ;
	    if (rs < 0)
	        goto badtmpmk ;

	    g.f.tmpfile  = TRUE ;
	    mskfname = mskfname_buf ;
	    if ((rs = u_open(mskfname,O_WRONLY,0664)) < 0)
	        goto badmskopen ;

	    write(rs,incfile_datemsk,INCFILELEN_datemsk) ;

	    close(rs) ;

	} /* end if (no DATEMSK file was given) */

	if (access(mskfname,R_OK) != 0) {

	    rs = SR_NOENT ;
	    goto badmskfile ;
	}

	bufprintf(buf,BUFSIZE,"DATEMSK=%s",mskfname) ;

	putenv(buf) ;

	g.mskfname = mskfname ;


/* open the files that we need for normal operation */

	if ((rs = bopen(ofp,BFILE_STDOUT,"dwct",0666)) < 0)
	    goto badoutopen ;


	if ((datestring != NULL) && (datestring[0] != '\0')) {

	    printout(datestring,ofp,g.efp) ;

	} else {

	    ifp = &infile ;
	    if ((rs = bopen(ifp,BFILE_STDIN,"dr",0666)) < 0)
	        goto badinopen ;

/* let's boogie */

	    while ((len = breadline(ifp,linebuf,LINELEN)) > 0) {

	        if (linebuf[len - 1] == '\n') len -= 1 ;

	        linebuf[len] = '\0' ;
	        cp = linebuf ;
	        while (CHAR_ISWHITE(*cp)) cp += 1 ;

	        if (cp[0] == '\n') continue ;

/* do it, baby ! */

	        printout(cp,ofp,g.efp) ;

	    } /* end while */

	    rs = len ;

	    bclose(ifp) ;

	} /* end if (opened input file) */

	bclose(ofp) ;

	if (g.f.tmpfile && (mskfname_buf[0] != '\0'))
	    unlink(mskfname_buf) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	debugprintf("main: exiting w/ rs=%d\n",rs) ;
#endif

earlyret:
	if (g.efp != NULL)
	    bclose(g.efp) ;

	return rs ;

/* handle the bad things */
badret:
	if (ifp != NULL)
	    bclose(ifp) ;

	bclose(ofp) ;

	if (g.f.tmpfile && (mskfname_buf[0] != '\0'))
	    unlink(mskfname_buf) ;

	if (g.efp != NULL)
	    bclose(g.efp) ;

	return BAD ;

/* program usage */
usage:
	if (g.efp == NULL) goto badret ;

	bprintf(g.efp,
	    "%s: USAGE> %s [-s date_string] [-m datemskfile] [-d]\n", 
	    g.progname,g.progname) ;

	bprintf(g.efp,
	    " [-VD?]\n") ;

	goto badret ;

badargextra:
	if (g.efp == NULL) goto badret ;

	bprintf(g.efp,"%s: no value associated with this option key\n",
	    g.progname) ;

	goto badret ;

badargvalue:
	if (g.efp == NULL) goto badret ;

	bprintf(g.efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto badret ;

badargnum:
	if (g.efp == NULL) goto badret ;

	bprintf(g.efp,"%s: not enough arguments specified\n",
	    g.progname) ;

	goto badret ;

badinopen:
	if (g.f.quiet || g.f.daemon) goto badret ;

	if (g.efp == NULL) goto badret ;

	bprintf(g.efp,"%s: could not open input file, rs=%d\n",
	    g.progname,rs) ;

	goto badret ;

badoutopen:
	if (g.f.quiet || g.f.daemon) goto badret ;

	if (g.efp == NULL) goto badret ;

	bprintf(g.efp,"%s: could not open output file, rs=%d\n",
	    g.progname,rs) ;

	goto badret ;

badtmpmk:
	if (g.f.quiet || g.f.daemon) goto badret ;

	if (g.efp == NULL) goto badret ;

	bprintf(g.efp,"%s: could not make a temporary file, rs=%d\n",
	    g.progname,rs) ;

	goto badret ;

badmskopen:
badmskfile:
	if (g.f.quiet || g.f.daemon) goto badret ;

	if (g.efp == NULL) goto badret ;

	bprintf(g.efp,"%s: could not open the mask file, rs=%d\n",
	    g.progname,rs) ;

	goto badret ;

}
/* end subroutine (main) */



static void printout(datestring,ofp,efp)
char	datestring[] ;
bfile	*ofp, *efp ;
{
	struct tm	*timep ;

	time_t		time ;

	int		len, f_good = FALSE  ;

	char		*tp ;
	char		timebuf[TIMEBUFLEN + 1], timebuf2[TIMEBUFLEN + 1] ;


#ifdef	COMMENT

#if	defined(SYSV) && CF_GETDATE2
	if (g.f.alternate)
	timep = getdate2(datestring) ;

	else
#endif
	timep = getdate(datestring) ;

/* try our selves ! */

	if (timep == NULL) {

	    len = strlen(datestring) ;

	    tp = datestring + len ;
	    while ((tp > datestring) && CHAR_ISWHITE(tp[-1])) tp -= 1 ;

	    while ((tp > datestring) && isdigit(tp[-1])) tp -= 1 ;

	    if ((tp > datestring) && (tp[-1] == '-')) {

	        tp -= 1 ;
	        while ((tp > datestring) && CHAR_ISWHITE(tp[-1])) tp -= 1 ;

	        *tp = '\0' ;

#if	defined(SYSV) && CF_GETDATE2
		if (g.f.alternate)
	        timep = getdate2(datestring) ;

		else
#endif
	        timep = getdate(datestring) ;

	    time = mktime(timep) ;

	    }

	} else
	    time = mktime(timep) ;

	if (timep != NULL) f_good = TRUE ;

#else

	time = getindate(datestring,NULL) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("printout: 1st time=%s\n",
		timestr_log(time,timebuf)) ;
#endif

	if (time == ((time_t) -1)) {

		time = getabsdate(datestring,NULL) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("printout: 2nd time=%s\n",
		timestr_gmlog(time,timebuf)) ;
#endif

		if (time != ((time_t) -1)) f_good = TRUE ;

	} else
		f_good = TRUE ;

#endif

	if (! f_good) {

	    if (g.f.daemon)
	        bprintf(ofp,"BAD\n") ;

	    else
	        bprintf(ofp,"*\n") ;

	    return ;

	} else {

	    if (g.f.daemon)
	        bprintf(ofp,"OK %ld\n",time) ;

	    else
	        bprintf(ofp,"%s LOC, %s GMT\n",
			timestr_log(time,timebuf),
			timestr_gmlog(time,timebuf2)) ;

	}

}
/* end subroutine (printout) */


#if	defined(SYSV) && CF_GETDATE2

static struct tm	tm_getdate ;

static struct tm *getdate2(s)
char	s[] ;
{
	bfile	mskfile, *mfp = &mskfile ;

	struct tm	ts, *timep ;

	int	len, i ;

	char	linebuf[LINELEN + 1] ;
	char	*cp ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("getdate2: entered\n") ;
#endif

	if ((s == NULL) || (s[0] == '\0')) return NULL ;

	if (bopen(mfp,g.mskfname,"r",0666) < 0) return NULL ;

	i = 0 ;
	timep = NULL ;
	while ((len = breadline(mfp,linebuf,LINELEN)) > 0) {

	    if (linebuf[len - 1] == '\n') len -= 1 ;

	    linebuf[len] = '\0' ;

	    cp = strptime(s,linebuf,&ts) ;

	    if (cp != NULL) break ;

	    i += 1 ;
	}

	bclose(mfp) ;

	if (cp == NULL) return NULL ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("getdate: found match at line %d\n",i) ;
#endif

	memcpy(&tm_getdate,&ts,sizeof(struct tm)) ;

	return &tm_getdate ;
}

#endif /* defined(SYSV) && CF_GETDATE2 */



