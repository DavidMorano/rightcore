/* main */

/* part of the MPACK program */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The program was written from scratch to do what
	the previous program by the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ mpack [-c content_type] [-e content_encoding] [-o output] 
			[-s subject] [-a attachment_spec] [address [...]]


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<mimetypes.h>
#include	<ema.h>
#include	<localmisc.h>

#include	"msgattach.h"
#include	"config.h"
#include	"defs.h"
#include	"headaddr.h"


/* local defines */

#define	MAXARGINDEX	600
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern int	optmatch() ;
extern int	buildmsg(struct global *,EMA *,MIMETYPES *,ATTACH_ENT *,
			ATTACH *,bfile *) ;

extern char	*strbasename(char *), *strshrink(char *) ;


/* external variables */


/* local structures */


/* forward references */


/* global variables */

struct global		g ;


/* local variables */

static char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"ROOT",
	"CONFIG",
	"attach",
	"af",
	NULL,
} ;

#define	ARGOPT_VERSION		0
#define	ARGOPT_VERBOSE		1
#define	ARGOPT_TMPDIR		2
#define	ARGOPT_ROOT		3
#define	ARGOPT_CONFIG		4
#define	ARGOPT_ATTACH		5
#define	ARGOPT_ATTFNAME		6


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	infile, *ifp = &infile ;

	ATTACH		atts ;		/* attachment specifications */

	ATTACH_ENT	ie ;

	EMA		adds[NADDS] ;

	MIMETYPES	types ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	rs = SR_OK ;
	int	i ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_input = FALSE ;
	int	recips = 0 ;
	int	err_fd ;
	int	es ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	const char	*configfname = NULL ;
	const char	*ofname = NULL ;
	const char	*recipfname = NULL ;
	const char	*attfname = NULL ;
	const char	*content_type = NULL ;
	const char	*content_encoding = NULL ;
	const char	*cp ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;

	g.progname = strbasename(argv[0]) ;

	g.efp = efp ;
	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

/* early things to initialize */

	g.programroot = NULL ;
	g.ofp = ofp ;
	g.debuglevel = 0 ;
	g.tmpdir = NULL ;
	g.version = VERSION ;
	g.header_subject = NULL ;
	g.header_mailer = NULL ;

	g.f.verbose = FALSE ;
	g.f.quiet = FALSE ;
	g.f.noinput = FALSE ;


#if	CF_DEBUGS
	            debugprintf("main: msgattach_init\n") ;
#endif

	(void) msgattach_init(&atts) ;

	for (i = 0 ; i < NADDS ; i += 1)
		ema_start(&adds[i]) ;

/* process program arguments */

	rs = SR_OK ;
	for (i = 0 ; i < MAXARGGROUPS ; i += 1)
		argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
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

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* version */
	                case ARGOPT_VERSION:
	                    f_version = TRUE ;
	                    if (f_optequal) goto badargextra ;

	                    break ;

/* verbose */
	                case ARGOPT_VERBOSE:
	                    g.f.verbose = TRUE ;
	                    if (f_optequal) goto badargextra ;

	                    break ;

/* temporary directory */
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

/* get a program root */
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

/* configuration file */
	                    case ARGOPT_CONFIG:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) configfname = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) configfname = argp ;

	                        }

	                        break ;

/* specifiy an attachment */
	                    case ARGOPT_ATTACH:
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
					msgattach_add(&atts,
						content_type,content_encoding,
						argp,argl) ;

	                        break ;

/* attachments list file */
	                    case ARGOPT_ATTFNAME:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) attfname = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) attfname = argp ;

	                        }

	                        break ;

/* default action and user specified help */
	                default:
	                    f_usage = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

			    int	kc = (*akp & 0xff) ;
	                    switch (kc) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        g.debuglevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
				    if (avl)
	                            rs = cfdeci(avp,avl, &g.debuglevel) ;
	                        }

	                        break ;

/* attachment specification */
	                    case 'a':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

				if (argl) {
					rs = msgattach_add(&atts,
						content_type,content_encoding,
					argp,argl) ;
				}

				break ;

/* blind-carbon-copy */
	                    case 'b':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

				if (argl)
					rs = ema_parse(&adds[A_BCC],argp,argl) ;

				break ;

/* carbon-copy */
	                    case 'c':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

				if (argl)
					rs = ema_parse(&adds[A_CC],argp,argl) ;

				break ;

/* input file for the main text portion of the message */
	                    case 'i':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) {
					f_input = TRUE ;
					rs = attachentry_init(&ie,
						content_type,content_encoding,
						argp,-1) ;
				}

				break ;

/* content_encoding */
	                    case 'e':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
					content_encoding = argp ;

				break ;

	                    case 'f':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
					rs = ema_parse(&adds[A_FROM],
						argp,argl) ;

				break ;

	                    case 'M':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
					g.header_mailer = argp ;

				break ;

	                    case 'n':
	                        g.f.noinput = TRUE ;
	                        break ;

	                    case 'o':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
					ofname = argp ;

				break ;

	                    case 'q':
	                        g.f.quiet = TRUE ;
	                        break ;

/* recipients list file */
	                    case 'r':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
					recipfname = argp ;

				break ;

	                    case 's':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
					g.header_subject = argp ;

				break ;

/* content_type */
	                    case 't':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
					content_type = argp ;

				break ;

/* verbose output */
	                    case 'v':
	                        g.f.verbose = TRUE ;
	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
				break ;

	                    default:
				rs = SR_INVALID ;
	                        bprintf(efp,
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

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                bprintf(efp,"%s: extra arguments ignored\n",
	                    g.progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (g.debuglevel > 0) {

	    bprintf(g.efp,
	        "%s: debuglevel %d\n",
	        g.progname,g.debuglevel) ;

	    bcontrol(g.efp,BC_LINEBUF,0) ;

	    bflush(g.efp) ;

	}

	if (f_version)
	    bprintf(efp,"%s: version %s\n",
	        g.progname,VERSION) ;

	if (f_usage) goto usage ;

	if (f_version) goto done ;


/* check arguments */

	if (g.programroot == NULL) {

	    g.programroot = getenv(VARPROGRAMROOT1) ;

	    if (g.programroot == NULL)
	        g.programroot = getenv(VARPROGRAMROOT2) ;

	    if (g.programroot == NULL)
	        g.programroot = getenv(VARPROGRAMROOT3) ;

	    if (g.programroot == NULL)
	        g.programroot = PROGRAMROOT ;

	}

	if (g.header_mailer == NULL)
		g.header_mailer = MAILER ;

/* what about the intput */

	if (! g.f.noinput) {
		if (! f_input) {
			f_input = TRUE ;
			attachentry_init(&ie,"text/plain",NULL,"",-1) ;
		}
	}

/* check a few more things */

	if (g.tmpdir == NULL)
	    g.tmpdir = getenv("TMPDIR") ;

	if (g.tmpdir == NULL)
	    g.tmpdir = TMPDIR ;

/* open the output file */

	if ((ofname == NULL) || (ofname[0] == '\0')) {
		rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;
	} else
		rs = bopen(ofp,ofname,"wct",0666) ;

	if (rs < 0) 
		goto badoutopen ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
		debugprintf("main: opened output file\n") ;
#endif

/* OK, get any recipients specified as arguments, if we have them */

	if (npa > 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if ((! BATST(argpresent,i)) &&
	            (argv[i][0] != '\0')) continue ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: aa=\"%s\"\n",argv[i]) ;
#endif

		rs = ema_parse(&adds[A_TO],argv[i],-1) ;

		if (rs > 0)
			recips += rs ;

	    } /* end for (looping through requested circuits) */

	} /* end if (address arguments) */

/* process any recipients in the recipient list file */

	if (recipfname != NULL) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: we have an argument file list >%s<\n",
			recipfname) ;
#endif

	    if ((strcmp(recipfname,"-") == 0) || (recipfname[0] == '\0')) {
	        rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	    } else
	        rs = bopen(ifp,recipfname,"r",0666) ;

	    if (rs >= 0) {

	        int	len ;

	        char	buf[BUFLEN + 1] ;


#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: processing the argument file list\n") ;
#endif

	        while ((len = breadline(ifp,buf,BUFLEN)) > 0) {

	            if (buf[len - 1] == '\n') len -= 1 ;

	            buf[len] = '\0' ;
	            cp = strshrink(buf) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

			rs = ema_parse(&adds[A_TO],cp,-1) ;

		if (rs > 0)
			recips += rs ;

	        } /* end while (reading lines) */

	        bclose(ifp) ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: done processing file list\n") ;
#endif

	    } else if (! g.f.quiet) {

	            bprintf(efp,
	                "%s: could not open the argument list file\n",
	                g.progname) ;

	            bprintf(efp,"\trs=%d argfile=%s\n",rs,recipfname) ;

	    }

	} /* end if (processing file argument file list */

#if	CF_DEBUG
	if (g.debuglevel > 3) {
		ATTACH_ENT	*aep ;
		EMA_ENT	*ep ;
			debugprintf("main: addresses\n") ;
		for (i = 0 ; ema_get(&adds[A_TO],i,&ep) >= 0 ; i += 1) {
			debugprintf("main: a=%s\n",ep->address) ;
		} /* end for */
			debugprintf("main: attachments\n") ;
		for (i = 0 ; msgattach_enum(&atts,i,&aep) >= 0 ; i += 1) {
			debugprintf("main: att file=%s type=%s subtype=%s\n",
				aep->filename,aep->type,aep->subtype) ;
		}
	}
#endif /* CF_DEBUG */

/* find some MIME types to look at! */

	mimetypes_start(&types) ;

#if	CF_DEBUG
	if (g.debuglevel > 5) {
		rs = mimetypes_get(&types,"gif",tmpfname) ;
		debugprintf("main: mimetypes rs=%d typespec=%s\n",rs,tmpfname) ;
	}
#endif /* CF_DEBUG */

	bufprintf(tmpfname,MAXPATHLEN,"%s/%s",
		g.programroot,TYPESFILE) ;

	mimetypes_file(&types,tmpfname) ;

#if	CF_DEBUG
	if (g.debuglevel > 4) {
		MIMETYPES_CUR	cur ;
		char	ext[MIMETYPES_TYPELEN + 1] ;
		char	typespec[MIMETYPES_TYPELEN + 1] ;
		mimetypes_curbegin(&types,&cur) ;
		while (mimetypes_enum(&types,&cur,ext,typespec) >= 0) {
			debugprintf("main:\t   ext=%s\ttypespec=%s\n",
				ext,typespec) ;
		} /* end while */
		mimetypes_curend(&types,&cur) ;
	}
#endif /* CF_DEBUG */

/* build the message! */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: building message\n") ;
#endif

	rs = buildmsg(&g,adds,&types,&ie,&atts,ofp) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: exiting\n") ;
#endif

	mimetypes_finish(&types) ;

	bclose(ofp) ;

	if (f_input)
		attachentry_free(&ie) ;

	es = 1 ;
	if (rs >= 0)
		es = 0 ;

	if (g.debuglevel > 0)
	    bprintf(g.efp,"%s: exiting ex=%u (%d)\n",
	        g.progname,es,rs) ;

/* we are out of here */
done:
	for (i = 0 ; i < NADDS ; i += 1)
		ema_finish(&adds[i]) ;

	msgattach_free(&atts) ;

	bclose(g.efp) ;

	return es ;

/* come here for a bad return from the program */
badret:
	es = 1 ;

#if	CF_DEBUGS
	debugprintf("main: exiting program BAD\n") ;
#endif

	goto done ;

/* program usage */
usage:
	bprintf(g.efp,
	    "%s: USAGE> %s [<address(s)> ...] [-o <outfile>] ",
	    g.progname,g.progname) ;

	bprintf(g.efp,"[-a <attachment_spec>]\n") ;

	es = 0 ;
	goto done ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    g.progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    g.progname) ;

	goto badret ;

badoutopen:
	bprintf(g.efp,"%s: inaccesssible output (%d)\n",
	    g.progname,rs) ;

	goto badret ;

}
/* end subroutine (main) */


