/* last modified %G% version %I% */
/* eas (Extract Advice Subcircuit) */

/* flatten and possibly extract an ADVICE subcircuit */


#define	DEBUGS		0
#define	DEBUG		1


/*
	= 1994, Dave Morano

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/****************************************************************************

	* said above *

	synopsis :

	$ eas [sub1 [sub2 [...]]] [-s] 
		[-o outputfile] [-i inputfile] [-a] [-VD?]

	where :

	-s	extract subcircuits into separate files by subcircuit name
	-t	specify circuit types to extract
	-c	printout only table of circuits that would have been extracted


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/utsname.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<errno.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<signal.h>
#include	<string.h>

#include	<bfile.h>
#include	<baops.h>
#include	<char.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define		MAXARGINDEX	100
#define		MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external functions */

extern int	mktmpfile(char *,mode_t,const char *) ;

extern char	*putheap() ;
extern char	*strbasename() ;


/* forward references */

struct circuit		*mkcircuit() ;

offset_t		gotcircuit() ;

int			addblock() ;
int			writecir() ;
int			loadtypes(), loadtype() ;
int			cirtypematch() ;

#ifdef	COMMENT
void			deleteblock() ;
#endif


/* local data structures */


/* local globals */

struct global		g ;


/* local statics */

static const char	*keywords[] = {
	NULL,
	".main",
	".subckt",
	".finis",
	".end",
	NULL,
} ;


/* define command option words */

static const char *argopts[] = {
	    "version",		/* 0 */
	    NULL,
} ;

#define	ARGOPT_VERSION	0


/* circuit types */

const char	*cirtypes[] = {
	"envelope",
	"main",
	"subckt",
	NULL,
} ;


#define	CIR_ENVELOPE	0
#define	CIR_MAIN	1
#define	CIR_SUB		2


/* exported subroutines */


int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile	infile, *ifp = &infile ;
	bfile	tmpfile, *tfp = &tmpfile ;
	bfile	outfile ;
	bfile	errfile, *efp = &errfile ;

	struct global	*gdp = &g ;

	struct circuit	*e_cirp, *cirp ;

	struct type	*tp, *headp = NULL ;

	offset_t	blockstart, offset ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	npa, i, ai ;
	int	len, rs ;
	int	maxai ;
	int	l, pn ;
	int	blen ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_extra = FALSE ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_bol, f_eol ;
	int	f_cktmain, f_cktsub ;
	int	f_contents = FALSE ;
	int	type ;
	int	line ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	cirtype[4] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	subfname[MAXPATHLEN + 1] ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp, *subname ;


	g.progname = strbasename(argv[0]) ;

	if (bopen(efp,BERR,"wca",0664) < 0) return BAD ;

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	bcontrol(efp,BC_LINEBUF,0) ;

/* some very initial stuff */

	gdp->efp = efp ;
	gdp->ofp = &outfile ;
	gdp->buf = buf ;
	gdp->debuglevel = 0 ;
	gdp->suffix = NULL ;
	cirtype[0] = 0 ;

	gdp->f.debug = FALSE ;
	gdp->f.separate = FALSE ;
	gdp->f.verbose = FALSE ;

/* process program arguments */

#if	DEBUGS
	eprintf("main: before loop ct=%02X\n",
	    (cirtype[0] & 255)) ;
#endif

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while (argr > 0) {

#if	DEBUGS
	    eprintf("main: top of loop ct=%02X\n",
	        (cirtype[0] & 255)) ;
#endif

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

#if	DEBUGS
	        eprintf("main: inside loop ct=%02X\n",
	            (cirtype[0] & 255)) ;
#endif

	        if (argl > 1) {

#if	DEBUGS
	            eprintf("main: got an option\n") ;
#endif

#if	DEBUGS
	            eprintf("main: more inside ct=%02X\n",
	                (cirtype[0] & 255)) ;
#endif

	            aop = argp + 1 ;
	            aol = argl - 1 ;
			akp = aop ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

#if	DEBUGS
	                eprintf("main: got an option key w/ a value\n") ;
#endif

			akl = avp - aop ;
	                aol = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                f_optequal = TRUE ;

	            } else {

			akl = aol ;
	                avl = 0 ;

			}

/* do we have a keyword match or should we assume only key letters ? */

#if	DEBUGS
	            eprintf("main: about to check for a key word match\n") ;
#endif

	            if ((kwi = opt_nmatch(argopts,aop,aol)) >= 0) {

#if	DEBUGS
	                eprintf("main: got an option keyword, kwi=%d\n",
	                    kwi) ;
#endif

	                switch (kwi) {

	                case ARGOPT_VERSION:
	                    f_version = TRUE ;
	                    if (f_optequal) goto badargextra ;

	                    break ;

/* default action and user specified help */
	                default:
	                    f_usage = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

#if	DEBUGS
	                eprintf("main: got an option key letter\n") ;
#endif

	                while (aol--) {

#if	DEBUGS
	                    eprintf("main: option key letters\n") ;
#endif

	                    switch (*aop) {

	                    case 'D':
	                        g.f.debug = TRUE ;
	                        gdp->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (cfdec(avp,avl,&gdp->debuglevel) != OK)
					goto badargvalue ;

	                        }

	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* input file */
	                    case 'i':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					ifname = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
					ifname = argp ;

	                        }

	                        break ;

/* output file when all subcircuit are combined */
	                    case 'o':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					ofname = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
	                        	ofname = argp ;

	                        }

	                        break ;

/* separate subcircuit files for extracted subcircuits */
	                    case 's':
	                        g.f.separate = TRUE ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
					gdp->suffix = avp ;

	                            else
					gdp->suffix = "" ;

	                        }

	                        break ;

/* define circuit types to extract */
	                    case 't':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) loadtypes(&headp,avp) ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) loadtypes(&headp,argp) ;

	                        }

	                        break ;

/* table of contents only */
				case 'c':
				f_contents = TRUE ;
				break ;

	                    default:
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            g.progname,*aop) ;

	                    case '?':
	                        f_usage = TRUE ;

	                    } /* end switch */

	                    aop += 1 ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (something in addition to just the sign) */

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

	if (gdp->debuglevel > 1)
	    bprintf(gdp->efp,
	        "%s: finished parsing arguments\n",
	        gdp->progname) ;


/* done with argument procurement */

	if (f_version)
	    bprintf(efp,"%s: version %s\n",
	        gdp->progname,VERSION) ;

	if (f_usage) goto usage ;

	if (f_version) goto done ;

	if (gdp->debuglevel > 0) bprintf(gdp->efp,
		"%s: debug level %d\n",
		gdp->progname,gdp->debuglevel) ;


/* check arguments */

/* circuit types */

	if ((gdp->debuglevel > 0) && (headp != NULL)) {

	    bprintf(gdp->efp,
	        "%s: circuit types to be extracted\n\t",
	        gdp->progname) ;

	    tp = headp ;
	    for (i = 0 ; tp != NULL ; i += 1) {

	        if (i > 0) bprintf(gdp->efp,", ") ;

	        bprintf(gdp->efp,"%s",tp->type) ;

	        tp = tp->next ;
	        i += 1 ;
	    }

	    bprintf(gdp->efp,"\n") ;

	} /* end if (printing out circuit types for debugging) */


/* process some arguments */

/* suffix */

	if (gdp->suffix == NULL) gdp->suffix = "adv" ;

/* circuit types */

#if	DEBUG
	if (gdp->debuglevel > 0)
	    bprintf(gdp->efp,"%s: circuit types A %02X\n",
	        gdp->progname,(int) cirtype[0]) ;
#endif

	for (tp = headp ; tp != NULL ; tp = tp->next) {

#if	DEBUG
	    if (gdp->debuglevel > 0) eprintf(gdp->efp,
	        "testing one \"%s\"\n",
	        tp->type) ;
#endif

	    if ((i = opt_nmatch(cirtypes,tp->type,-1)) >= 0) {

#if	DEBUG
	        if (gdp->debuglevel > 1) eprintf(
	            "main: got a circuit type \"%s\"\n",
	            tp->type) ;
#endif

	        BASET(cirtype,i) ;

#if	DEBUG
	        if (gdp->debuglevel > 0)
	            bprintf(gdp->efp,"%s: circuit types B (%d) %02X\n",
	                gdp->progname,i,(int) cirtype[0]) ;
#endif

	    } else
	        bprintf(gdp->efp,
	            "%s: unrecognized circuit type \"%s\"\n",
	            gdp->progname,tp->type) ;

	} /* end for */

#if	DEBUG
	if (gdp->debuglevel > 0)
	    bprintf(gdp->efp,"%s: circuit types B (final) %02X\n",
	        gdp->progname,(int) cirtype[0]) ;
#endif

/* initialize some basic stuff */

#if	DEBUG
	if (gdp->debuglevel > 0) eprintf(
	    "main: about to decide input\n") ;
#endif

	if (ifname == NULL) ifname = BFILE_STDIN ;

	if ((rs = bopen(ifp,ifname,"r",0666)) < 0) goto badopenin ;

#if	DEBUG
	if (gdp->debuglevel > 0) eprintf(
	    "main: about to check for copy of input\n") ;
#endif

/* copy the input file to a temporary file if necessary (if not seekable) */

	if ((rs = bseek(ifp,0L,SEEK_CUR)) < 0) {

#if	DEBUG
	    if (gdp->debuglevel > 0) 
		eprintf("main: about to 'mktmpfile'\n") ;
#endif

	    rs = mktmpfile( tmpfname, 0644, "/tmp/easXXXXXXXXXXX") ;
	    if (rs < 0)
	        goto badtmpmake ;

#if	DEBUG
	    if (gdp->debuglevel > 0) eprintf(
	        "main: about to 'bopen'\n") ;
#endif

	    if ((rs = bopen(tfp,tmpfname,"wct",0644)) < 0)
	        goto badtmpopen ;

#if	DEBUG
	    if (gdp->debuglevel > 0) bprintf(gdp->efp,
	        "%s: made a temporay file \"%s\"\n",
	        gdp->progname,tmpfname) ;
#endif

	    unlink(tmpfname) ;

/* copy input file to our temporary file */

#if	DEBUG
	    if (gdp->debuglevel > 0) eprintf(
	        "main: about to do the copy \n") ;
#endif

	    while ((l = bread(ifp,buf,BUFLEN)) > 0) {

	        if ((rs = bwrite(tfp,buf,l)) < l) goto badwrite ;

	    }

	    bclose(ifp) ;

	    bseek(tfp,0L,SEEK_SET) ;

	    ifp = tfp ;

	} /* end if (check for input file seekable) */

	gdp->ifp = ifp ;

#if	DEBUG
	if (gdp->debuglevel > 0) eprintf(
	    "main: got input, about to separate\n") ;
#endif

/* are we in "separate" mode or not ? */

	if (! gdp->f.separate) {

	    if (ofname == NULL) ofname = BFILE_STDOUT ;

	    if ((rs = bopen(gdp->ofp,ofname,"wct",0666)) < 0)
	        goto badopenout ;

	}

#if	DEBUGS
	bprintf(gdp->ofp,"* got stuff in output\n\n") ;

	bflush(gdp->ofp) ;
#endif

/* start processing the (possibly null) envelope circuit */

	if ((e_cirp = mkcircuit("envelope",-1,CIR_ENVELOPE,0)) == NULL)
	    goto badalloc ;

	gdp->top = e_cirp ;
	gdp->bottom = e_cirp ;

#if	DEBUG
	if (gdp->debuglevel > 1)
	    eprintf("main: made envelope circuit %08lX\n",e_cirp) ;
#endif

/* do it -- go through all lines looking for SUBCIRCUITs (or MAIN) */

	line = 0 ;
	f_bol = TRUE ;
	offset = 0 ;
	blen = 0 ;
	blockstart = 0 ;
	while ((len = bgetline(gdp->ifp,gdp->buf,BUFLEN)) > 0) {

	    l = len ;
	    f_eol = FALSE ;
	    if (gdp->buf[l - 1] == '\n')
	        ((l -= 1), (f_eol = TRUE)) ;

	    gdp->buf[l] = '\0' ;

	    if (f_bol) {

#if	DEBUG
	        if (gdp->debuglevel > 1) {

	            eprintf("main: beginning of line\n") ;

	            eprintf("main: >%s<\n", gdp->buf) ;

	        }
#endif

/* skip over white space */

	        cp = gdp->buf ;
	        while (ISWHITE(*cp)) cp += 1 ;

/* scan for an ADVICE circuit (main or sub) keyword */

	        f_cktsub = FALSE ;
	        f_cktmain = (strncasecmp(cp,".main",5) == 0) ;

	        if (! f_cktmain)
	            f_cktsub = (strncasecmp(cp,".subckt",7) == 0) ;

	        if (f_cktmain || f_cktsub) {

	            if (gdp->debuglevel > 0) bprintf(gdp->efp,
	                "%s: got a new subcircuit\n",
	                gdp->progname) ;

	            type = CIR_MAIN ;
	            if (f_cktsub) type = CIR_SUB ;

/* we got a new subcircuit, finish off the block of this circuit */

	            if (addblock(e_cirp,blockstart,blen) < 0)
	                goto badalloc ;

#if	DEBUG
	            if (gdp->debuglevel > 1)
	                eprintf("main: closed off block s=%ld bl=%d\n",
	                    blockstart,blen) ;
#endif

/* start the new circuit */

	            l = cp - gdp->buf ;
	            if ((offset = gotcircuit(gdp,offset,
	                gdp->buf,len,l,type,1)) < 0) goto badgotcir ;

/* think about it, it is pretty neat that we do NOT have to do this seek */

#ifdef	COMMENT
	            bseek(gdp->ifp,offset,SEEK_SET) ;
#endif

	            blockstart = offset ;
	            blen = 0 ;
	            len = 0 ;

	        } /* end if (a new subcircuit) */

#if	DEBUG
	        if (gdp->debuglevel > 1)
	            eprintf("main: bottom of BOL o=%ld\n",offset) ;
#endif

	    } /* end if (BOL) */

#if	DEBUG
	    if (gdp->debuglevel > 1)
	        eprintf("main: bottom of while loop o=%ld\n",
	            offset) ;
#endif

	    offset += len ;
	    blen += len ;
	    if ((len > 0) && f_eol) line += 1 ;

	    f_bol = f_eol ;

	} /* end while (scanning circuits) */

/* finish the last block */

	if (addblock(e_cirp,blockstart,blen) < 0)
	    goto badalloc ;

	e_cirp->lines = line ;

/* end of looping through (cataloging) the stuff */

#if	DEBUG
	if (gdp->debuglevel > 0) bprintf(gdp->efp,
	    "%s: finished scanning circuits, input file len=%ld\n",
	    gdp->progname,offset) ;
#endif

/* write the scanned circuits out ? */

	if (npa > 0) {

/* write out only those subcircuits specified */

	    if (gdp->debuglevel > 0) bprintf(gdp->efp,
	        "%s: extracting specified circuit names\n",
	        gdp->progname) ;

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if ((! BATST(argpresent,i)) ||
	            (argv[i][0] == '-') || (argv[i][0] == '+'))
	            continue ;

	        if (gdp->debuglevel > 0) bprintf(gdp->efp,
	            "%s: looking for circuit \"%s\"\n",
	            gdp->progname,argv[i]) ;

/* search for the named subcircutis */

	        for (cirp = e_cirp ; cirp != NULL ; cirp = cirp->next) {

	            if (gdp->debuglevel > 1) bprintf(gdp->efp,
	                "%s: checking circuit \"%s\"\n",
	                gdp->progname,cirp->name) ;

	            if (strcasecmp(argv[i],cirp->name) == 0) {

	                if (gdp->debuglevel > 1) bprintf(gdp->efp,
	                    "%s: found circuit \"%s\"\n",
	                    gdp->progname,argv[i]) ;

	                if (cirtypematch(cirtype,cirp)) {

			if (f_contents)
				bprintf(gdp->ofp,
				"%W\n",cirp->name,cirp->nlen) ;

			else
	                    if ((rs = writecir(gdp,cirp)) < 0)
	                        goto badwritecir ;

	                }
	                break ;
	            }

	        } /* end for (searching for circuit) */

	    } /* end for (looping through requested circuits) */

	} else {

/* write out all subcircuits found */

	    if (gdp->debuglevel > 0) bprintf(gdp->efp,
	        "%s: extracting all circuit names\n",
	        gdp->progname) ;

	    for (cirp = e_cirp ; cirp != NULL ; cirp = cirp->next) {

		if (gdp->debuglevel > 0) bprintf(gdp->efp,
			"%s: checking circuit \"%W\"\n",
			gdp->progname,cirp->name,cirp->nlen) ;

	        if (cirtypematch(cirtype,cirp)) {

	    if (gdp->debuglevel > 0) bprintf(gdp->efp,
	        "%s: circuit type match\n",
	        gdp->progname) ;

			if (f_contents)
				bprintf(gdp->ofp,
				"%W\n",cirp->name,cirp->nlen) ;

			else
	            if ((rs = writecir(gdp,cirp)) < 0)
	                goto badwritecir ;

	        } else
	    if (gdp->debuglevel > 0) bprintf(gdp->efp,
	        "%s: circuit type mismatch\n",
	        gdp->progname) ;

	    } /* end for (looping through all circuits) */

	} /* end if */

	if (gdp->debuglevel > 0) bprintf(gdp->efp,
	    "%s: program exiting\n",
	    gdp->progname) ;

	bclose(ifp) ;

	if (! gdp->f.separate)
	    bclose(gdp->ofp) ;

/* done */
done:
	bclose(ifp) ;

	if (! gdp->f.separate)
	    bclose(gdp->ofp) ;

	bclose(efp) ;

	return OK ;

badret:
	bclose(efp) ;

	return BAD ;

usage:
	bprintf(efp,
	    "%s: USAGE> %s [sub1 [sub2[ ...]]] ",
	    g.progname,g.progname) ;

	bprintf(efp,
	    "[-i infile] [-o outfile] [-csVD?] [-t type]\n") ;

	bprintf(efp,
		"\t-c\tprint out subcircuit names only\n") ;

	bprintf(efp,
		"\t-s\tseparate subcircuits into individual files\n") ;

	bprintf(efp,
		"\t-t types\tspecify circuit types to extract\n") ;

	goto badret ;

badarg:
	bprintf(efp,"%s: bad argument given\n",g.progname) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",g.progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value specified\n",
	    g.progname) ;

	goto badret ;

badargextra:
	bprintf(efp,"%s: cannot have extra argument values for option\n",
	    g.progname) ;

	goto badret ;

badopenin:
	bprintf(efp,"%s: could not open input file (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badopenout:
	bprintf(efp,"%s: could not open output file (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badtmpmake:
	bprintf(efp,"%s: could not create a temporary file (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badtmpopen:
	bprintf(efp,"%s: could not open a temporary file (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badwrite:
	bprintf(efp,"%s: bad write (rs %d) - possible full filesystem\n",
	    g.progname,rs) ;

	goto badret ;

badalloc:
	bprintf(efp,"%s: could not allocate a circuit block\n",
	    g.progname) ;

	goto badret ;

badgotcir:
	bprintf(efp,"%s: bad return from 'gotcircuit' (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badwritecir:
	bprintf(efp,"%s: bad return from 'writecir' (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

}
/* end subroutine (main) */


/* process a subcircuit */

offset_t gotcircuit(gdp,blockstart,linebuf,linelen,index,type,sl)
struct global	*gdp ;
char		linebuf[] ;
int	linelen ;
int	index ;
offset_t	blockstart ;
int	type ;
int	sl ;
{
	bfile		outfile, *ofp = &outfile ;

	struct circuit	*cirp ;

	offset_t		offset = blockstart ;

	int	l, len, blen = linelen ;
	int	f_bol, f_eol ;
	int	f_finis = FALSE ;
	int	f_exit ;
	int	f_cktmain, f_cktsub ;
	int	line = 1 ;

	char	subname[MAXPATHLEN + 1] ;
	char	subfname[MAXPATHLEN + 1] ;
	char	*cp ;


#if	DEBUG
	if (gdp->debuglevel > 1)
	    eprintf("gotcircuit: entered sl=%d o=%ld linelen=%d\n",
	        sl,offset,linelen) ;
#endif

	linebuf[linelen] = '\0' ;

#if	DEBUG
	if (gdp->debuglevel > 1)
	    eprintf("gotcircuit: line end zapped\n") ;
#endif

/* try to get a subcricuit "name" */

	cp = linebuf + index ;

#if	DEBUG
	if (gdp->debuglevel > 1)
	    eprintf("gotcircuit: starting point calculated\n") ;
#endif

/* skip over white space, if any (there should not be any) */

	while (ISWHITE(*cp)) cp += 1 ;

#if	DEBUG
	if (gdp->debuglevel > 1)
	    eprintf("gotcircuit: white space\n") ;
#endif

/* skip over the keyword */

	while (*cp && (! ISWHITE(*cp))) cp += 1 ;

#if	DEBUG
	if (gdp->debuglevel > 1)
	    eprintf("gotcircuit: non-white space\n") ;
#endif

/* skip over possibly more white space */

	while (ISWHITE(*cp)) cp += 1 ;

/* copy the line buffer to the subname buffer */

	strcpy(subname,cp) ;

#if	DEBUG
	if (gdp->debuglevel > 1)
	    eprintf("gotcircuit: copied circuit name\n") ;
#endif

	if ((cp = strpbrk(subname,". \t(")) != NULL)
	    *cp = '\0' ;

#if	DEBUG
	if (gdp->debuglevel > 1)
	    eprintf("gotcircuit: subname=%s\n",subname) ;
#endif

	if (gdp->debuglevel > 0) bprintf(gdp->efp,
		"%s: subcircuit name \"%s\"\n",
		gdp->progname,subname) ;

/* make a new circuit descriptor, 'blockstart' is set */

	if ((cirp = mkcircuit(subname,-1,type,sl)) == NULL)
	    goto bad ;

#if	DEBUG
	if (gdp->debuglevel > 1)
	    eprintf("gotcircuit: made a circuit sl=%d %08lX %08lX\n",
	        sl,cirp,gdp->bottom) ;
#endif

/* link this circuit description block into the list of them */

	(gdp->bottom)->next = cirp ;
	gdp->bottom = cirp ;

/* increment the offset in the file */

	offset += linelen ;

/* scan lines until we see another subcircuit or the end of this one ! */

	f_bol = TRUE ;
	f_eol = FALSE ;
	f_exit = FALSE ;
	while ((! (f_exit && f_eol)) &&
	    ((len = bgetline(gdp->ifp,gdp->buf,BUFLEN)) > 0)) {

	    l = len ;
	    f_eol = FALSE ;
	    if (gdp->buf[l - 1] == '\n')
	        ((l -= 1), (f_eol = TRUE)) ;

	    gdp->buf[l] = '\0' ;

#if	DEBUG
	    if (gdp->debuglevel > 1) {

	        eprintf("gotcircuit: looping\n") ;

	        eprintf("gotcircuit: >%s<\n",gdp->buf) ;

	    }
#endif

	    if (f_bol) {

#if	DEBUG
	        if (gdp->debuglevel > 1)
	            eprintf("gotcircuit: top of BOL sl=%d\n",sl) ;
#endif

	        cp = gdp->buf ;
	        while (ISWHITE(*cp)) cp += 1 ;

	        f_cktsub = FALSE ;
	        f_cktmain = (strncasecmp(cp,".main",5) == 0) ;

	        if (! f_cktmain)
	            f_cktsub = (strncasecmp(cp,".subckt",7) == 0) ;

#if	DEBUG
	        if (gdp->debuglevel > 1)
	            eprintf("gotcircuit: about to decide new circuit sl=%d\n",
	                sl) ;
#endif

/* do we have a subcircuit ? */

	        if (f_cktmain || f_cktsub) {

#if	DEBUG
	            if (gdp->debuglevel > 1) {

	                eprintf("gotcircuit: new circuit again\n") ;

	                eprintf("gotcircuit: >%s<\n",gdp->buf) ;

	            }
#endif

	            type = CIR_MAIN ;
	            if (f_cktsub) type = CIR_SUB ;

/* we got a new subcircuit, finish off the block of this circuit */

	            if (addblock(cirp,blockstart,blen) < 0)
	                goto bad ;

#if	DEBUG
	            if (gdp->debuglevel > 1) eprintf(
	                "gotcircuit: closed off this block o=%ld bl=%d\n",
	                blockstart,blen) ;
#endif

/* start the new circuit */

	            l = cp - gdp->buf ;
	            if ((offset = gotcircuit(gdp,cirp,offset,gdp->buf,len,
	                l,type,sl + 1)) < 0)
	                return BAD ;

#if	DEBUG
	            if (gdp->debuglevel > 1) eprintf(
	                "gotcircuit: returned from 'gotcircuit' o=%ld\n",
	                offset) ;
#endif

/* think about it, it is pretty neat that we do NOT have to do this seek */

#ifdef	COMMENT
	            bseek(gdp->ifp,offset,SEEK_SET) ;
#endif

	            blockstart = offset ;
	            blen = 0 ;
	            len = 0 ;

	        } else if ((strncasecmp(cp,".finis",6) == 0) ||
	            (strncasecmp(cp,".end",4) == 0)) {

#if	DEBUG
	            if (gdp->debuglevel > 1)
	                eprintf("gotcircuit: end of circuit sl=%d\n",
	                    sl) ;
#endif

	            f_finis = TRUE ;
	            f_exit = TRUE ;

	        } /* end if (what type of action) */

#if	DEBUG
	        if (gdp->debuglevel > 1)
	            eprintf("gotcircuit: end of BOL sl=%d\n",
	                sl) ;
#endif

	    } /* end if (BOL) */

#if	DEBUG
	    if (gdp->debuglevel > 1)
	        eprintf("gotcircuit: bottom of while sl=%d\n",sl) ;
#endif

	    offset += len ;
	    blen += len ;
	    if ((len > 0) && f_eol) line += 1 ;

	    f_bol = f_eol ;

	} /* end while */

/* store the last block */

	if (addblock(cirp,blockstart,blen) < 0)
	    goto bad ;

	cirp->lines = line ;
	return offset ;

bad:
	if (gdp->debuglevel > 2) 
		eprintf( "gotcircuit: got a bad circuit discovered\n") ;

	return BAD ;

badwrite:
	return BAD ;
}
/* end subroutine (gotcircuit) */


/* get circuit descriptor */

struct circuit *mkcircuit(name,nlen,type,sl)
char	*name ;
int	nlen, type ;
int	sl ;
{
	struct circuit	*cirp ;

	char		*np ;


	if (nlen > 0)
	    name[nlen] = '\0' ;

	else
	    nlen = (int) strlen(name) ;

	if ((np = putheap(name)) == NULL)
	    return NULL ;

	if ((cirp = (struct circuit *) malloc(sizeof(struct circuit))) == NULL)
	    return NULL ;

	cirp->next = NULL ;
	cirp->bp = NULL ;
	cirp->name = np ;
	cirp->nlen = nlen ;
	cirp->type = type ;
	cirp->sl = sl ;
	return cirp ;
}
/* end subroutine (mkcircuit) */


/* add a block to the circuit */

int addblock(cirp,offset,len)
struct circuit	*cirp ;
offset_t	offset ;
long	len ;
{
	struct block	*hbp, *bp ;

	char		*np ;


	if (len <= 0) return OK ;

	if ((bp = (struct block *) malloc(sizeof(struct block))) == NULL)
	    return BAD ;

	bp->next = NULL ;
	bp->start = offset ;
	bp->len = len ;
	if (cirp->bp == NULL) {

	    cirp->bp = bp ;

	} else {

	    hbp = cirp->bp ;
	    while (hbp->next != NULL) hbp = hbp->next ;

	    hbp->next = bp ;
	}

	return OK ;
}
/* end subroutine (addblock) */


#ifdef	COMMENT

/* delete a block sub-list from a point in a block list */

void deleteblock(be)
struct block	*be ;
{


	if (be == NULL) return ;

	if (be->next != NULL) deleteblolk(be->next) ;

	free(be) ;

}
/* end subroutine (deleteblock) */

#endif


/* write out a circuit to the output file */

int writecir(gdp,cirp)
struct global	*gdp ;
struct circuit	*cirp ;
{
	bfile		subfile, *sfp ;

	struct block	*bp ;

	int		i, len, sl = cirp->sl, rs ;

	char		subfname[MAXPATHLEN + 1] ;
	char		namebuf[MAXPATHLEN + 1] ;


#if	DEBUG
	if (gdp->debuglevel > 1)
	    eprintf("writecir: entered cir=%s\n",cirp->name) ;
#endif

/* if we are in "separate" mode, then create the new file */

	if (gdp->f.separate) {

#if	DEBUG
	    if (gdp->debuglevel > 1)
	        eprintf("writecir: separated\n") ;
#endif

/* convert the name to something a little nicer */

	    for (i = 0 ; cirp->name[i] != '\0' ; i += 1) {

	        if (cirp->name[i] == '/')
	            subfname[i] = '_' ;

	        else
	            subfname[i] = tolower(cirp->name[i]) ;

	    } /* end for */

	    subfname[i] = '\0' ;
	    if ((gdp->suffix != NULL) && (gdp->suffix[0] != '\0')) {

	        subfname[i++] = '.' ;
	        strcpy(subfname + i,gdp->suffix) ;

	    }

	    sfp = &subfile ;
	    if ((rs = bopen(sfp,subfname,"wct",0666)) < 0)
	        return rs ;

	} else {

#if	DEBUG
	    if (gdp->debuglevel > 1)
	        eprintf("writecir: not separated\n") ;
#endif

	    sfp = gdp->ofp ;

	} /* end if (determining output mode) */

/* write out the header commnets */

	if (strcasecmp(cirp->name,"envelope") == 0)
	    bprintf(sfp,
	        "* start circuit \"envelope\" (lines=%d)\n\n",
	        cirp->lines + 6) ;

	else
	    bprintf(sfp,
	        "* start circuit \"%s\" (level=%d, lines=%d)\n\n",
	        cirp->name,sl,cirp->lines + 6) ;

/* loop through the blocks of this circuit, writing them out */

	bp = cirp->bp ;
	while (bp != NULL) {

#if	DEBUG
	    if (gdp->debuglevel > 1)
	        eprintf("writecir: got a block start=%ld len=%d\n",
	            bp->start,bp->len) ;
#endif

#if	DEBUG
	    if (gdp->debuglevel > 2) {

	        eprintf(
	            "writecir: writing circuit \"%s\" block s=%ld l=%d\n",
	            cirp->name,bp->start,bp->len) ;

	        if ((rs = bseek(gdp->ifp,bp->start,SEEK_SET)) < 0)
	            eprintf("writecir: error from seek (rs %d)\n",
	                rs) ;

	        while ((len = bgetline(gdp->ifp,namebuf,MAXPATHLEN)) > 0) {

	            if (namebuf[len - 1] == '\n') len -= 1 ;

	            namebuf[len] = '\0' ;
	            eprintf("writecir: >%s<\n",namebuf) ;

	        }
	    }
#endif

	    bseek(gdp->ifp,bp->start,SEEK_SET) ;

	    if ((rs = bcopyblock(gdp->ifp,sfp,(int) bp->len)) < 0)
	        goto badwrite ;

	    bp = bp->next ;

	} /* end while */

/* write out the trailer commnets */

	if (strcasecmp(cirp->name,"envelope") == 0)
	    bprintf(sfp,
	        "\n* end circuit \"envelope\"\n\n\n") ;

	else
	    bprintf(sfp,
	        "\n* end circuit \"%s\" (level=%d)\n\n\n",
	        cirp->name,sl) ;

	rs = OK ;

#if	DEBUG
	if (gdp->debuglevel > 1)
	    eprintf("writecir: exiting normally\n") ;
#endif

done:
	if (gdp->f.separate)
	    bclose(sfp) ;

	else
	    bflush(sfp) ;

	return rs ;

badwrite:
	rs = BAD ;
	goto done ;
}
/* end subroutine (writecir) */


/* option match up, something ! */

int opt_nmatch(os,s,l)
char	*os[] ;
char	*s ;
int	l ;
{
	int	i = 0 ;


	if (l < 0) l = strlen(s) ;

	if (l <= 0) return -1 ;

#if	DEBUGS
	eprintf("opt_nmatch: searching on \"%W\"\n",
	    s,l) ;
#endif

	while ((os[i] != NULL) && (os[i][0] != '\0')) {

#if	DEBUGS
	    eprintf("opt_nmatch: checking \"%s\"\n",
	        os[i]) ;
#endif

	    if (strnlead(os[i],s,l)) return i ;

	    i += 1 ;
	}

	return -1 ;
}
/* end subroutine (opt_nmatch) */


/* load circuit types into the "type" linked list */

int loadtypes(headp,s)
struct type	**headp ;
char		*s ;
{
	struct type	*tp ;

	char		*cp ;


	while ((cp = strpbrk(s," \t,")) != NULL) {

	    *cp = '\0' ;
	    if (loadtype(headp,s) < 0) return BAD ;

	    s = cp + 1 ;

	}  /* end while */

	if (((int) strlen(s)) > 0)
	    if (loadtype(headp,s) < 0) return BAD ;

	return OK ;
}
/* end subroutine (loadtypes) */


/* load a single type into the "type" linked list */

int loadtype(headp,s)
struct type	**headp ;
char		*s ;
{
	struct type	*tp2, *tp ;

	char		*cp ;


	if (((int) strlen(s)) <= 0) return OK ;

	if ((tp = (struct type *) malloc(sizeof(struct type))) == NULL)
	    return BAD ;

	tp->next = NULL ;
	if (*headp == NULL) {

	    *headp = tp ;

	} else {

	    tp2 = *headp ;
	    while (tp2->next != NULL) tp2 = tp2->next ;

	    tp2->next = tp ;

	}

	if ((tp->type = putheap(s)) == NULL) return BAD ;

	return OK ;
}
/* end subroutine (loadtype) */


int cirtypematch(cirtype,cirp)
char		cirtype[] ;
struct circuit	*cirp ;
{


	if (cirtype[0] == 0) return TRUE ;

	if (BATST(cirtype,cirp->type)) return TRUE ;

	return FALSE ;
}
/* end subroutine (cirtypematch) */


