/* main */

/* MRFINGER client */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 2003-10-07, David A­D­ Morano
	I hacked this up to clean up stupid garbage from stupid M$ servers.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a quick and dirty hack to get some working FINGER client
	on the MacIntosh platform.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	"vsystem.h"
#include	"baops.h"
#include	"exitcodes.h"
#include	"localmisc.h"

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	NICHOSTNAME	"whois.internic.net"
#define	QUERYLEN	(MAXHOSTNAMELEN + 20)

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	SVC_WHOIS
#define	SVC_WHOIS	"whois"
#endif

#ifndef	VARCOLUMNS
#define	VARCOLUMNS	"COLUMNS"
#endif


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	bprintlines(bfile *,int,const char *,int) ;
extern int	isdigitlatin(int) ;

extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct dialspec {
	const char	*host ;
	const char	*portspec ;
	int		af ;
	int		timeout ;
	int		opts ;
} ;


/* forward references */

static int	usage(struct proginfo *) ;
static int	procquery(struct proginfo *,void *,struct dialspec *,
			const char *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"of",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_of,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
} ;

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ 0, 0 }
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo		pi, *pip = &pi ;

	struct dialspec		ds ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs = SR_OK ;
	int	rs1 ;
	int	af = AF_INET ;
	int	timeout = TO_CONNECT ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*pr = NULL ;
	const char	*searchname = NULL ;
	const char	*ofname = NULL ;
	const char	*hostname = NICHOSTNAME ;
	const char	*portspec = SVC_WHOIS ;
	const char	*query = NULL ;
	const char	*cp ;
	char	argpresent[MAXARGGROUPS] ;
	char	tmpfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;

	if (rs >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* initialization */

	pip->verboselevel = 1 ;
	pip->linelen = -1 ;

/* start parsing the arguments */

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1) 
		argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
		const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

		    rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

		} else {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {
	                f_optequal = TRUE ;
	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	            } else {
			avp = NULL ;
	                avl = 0 ;
	                akl = aol ;
	            }

/* keyword match or only key letters? */

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                    switch (kwi) {

/* version */
	                    case argopt_version:
	                        f_version = TRUE ;
	                        if (f_optequal)
	                            rs = SR_INVALID ;

	                        break ;

/* verbose mode */
	                    case argopt_verbose:
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                        }

	                        break ;

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                pr = avp ;

	                        } else {

	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pr = argp ;

	                        }

	                        break ;

	                    case argopt_help:
	                        f_help = TRUE ;
	                        break ;

/* output file name */
	                    case argopt_of:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                ofname = avp ;

	                        } else {

	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                ofname = argp ;

	                        }

	                        break ;

/* handle all keyword defaults */
	                    default:
				f_usage = TRUE ;
	                        rs = SR_INVALID ;
	                        bprintf(pip->efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    rs = cfdeci(avp,avl,
	                                        &pip->debuglevel) ;

	                            }

	                            break ;

/* program-root */
	                        case 'R':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pr = argp ;

	                            break ;

/* queit mode */
	                        case 'Q':
	                            pip->have.quiet = TRUE ;
	                            pip->f.quiet = TRUE ;
	                            pip->final.quiet = TRUE ;
	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* WHOIS server host */
	                        case 'h':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                hostname = argp ;

	                            break ;

/* line-length */
	                        case 'l':
				case 'w':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                    rs = cfdeci(argp,argl,
	                                        &pip->linelen) ;

	                            break ;

/* portspec */
	                        case 'p':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                portspec = argp ;

	                            break ;

				case 'q':
				    pip->verboselevel = 0 ;
	                            break ;

	                        case 't':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                rs = cfdecti(argp,argl,&timeout) ;

	                            break ;

/* verbose mode */
	                        case 'v':
	                            pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    rs = cfdeci(avp,avl,
	                                        &pip->verboselevel) ;

	                            }

	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
	                            break ;

	                        default:
	                            rs = SR_INVALID ;
				    f_usage = TRUE ;
	                            bprintf(pip->efp,
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                        } /* end switch */

	                        akp += 1 ;
				if (rs < 0)
					break ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,
	        "%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	    bflush(pip->efp) ;

	}

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

/* program search name */

	if (rs >= 0) 
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,searchname) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pr=%s\n",pip->pr) ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    bprintf(pip->efp,"%s: sn=%s\n",
	        pip->progname,pip->searchname) ;

	} /* end if */

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_usage)
	    usage(pip) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* argument defaults */

	if (pip->linelen <= 0) {

	    if ((cp = getenv(VARCOLUMNS)) != NULL) {

		rs1 = ctdeci(cp,-1,&pip->linelen) ;

		if (rs1 < 0)
		    pip->linelen = -1 ;

	    } /* end if */

	    if (pip->linelen <= 0)
		pip->linelen = LINELEN ;

	} /* end if */

/* do we have something to query? */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

		if (argv[ai][0] != '\0') {
			BACLR(argpresent,ai) ;
			query = argv[ai] ;
			break ;
		}

	} /* end for */

	if ((query == NULL) || (query[0] == '\0')) {
	    ex = EX_USAGE ;
	    bprintf(pip->efp, "%s: no query was specified\n",
	        pip->progname) ;

	    goto badnoquery ;
	}

/* open output file if we have to */

	if ((ofname != NULL) && (ofname[0] != '\0'))
	    rs = bopen(ofp,ofname,"wct",0644) ;

	else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0644) ;

	if (rs < 0) {
		ex = EX_CANTCREAT ;
		bprintf(pip->efp,"%s: could not open output (%d)\n",
	    		pip->progname,rs) ;

		goto badoutopen ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: host=%s\n",hostname) ;
#endif

	memset(&ds,0,sizeof(struct dialspec)) ;

	ds.host = hostname ;
	ds.portspec = portspec ;
	ds.af = af ;
	ds.timeout = timeout ;
	ds.opts = 0 ;
	rs = procquery(pip,ofp,&ds,query) ;

done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",
		ex,rs) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
		pip->progname,ex,rs) ;

	bclose(ofp) ;

/* we're done */
badoutopen:
badnoquery:
retearly:
ret2:
	if (pip->efp != NULL)
	    bclose(pip->efp) ;

ret1:
	proginfo_finish(pip) ;

badprogstart:
ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [-h <host>] <query> [-t <timeout>]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procquery(pip,ofp,dsp,query)
struct proginfo	*pip ;
void		*ofp ;
struct dialspec	*dsp ;
const char	query[] ;
{
	const int	to = dsp->timeout ;
	const int	opts = dsp->opts ;

	int	rs ;
	int	s ;
	int	len ;
	int	fbo = FILEBUF_ONET ;
	int	wlen = 0 ;

	char	querybuf[QUERYLEN + 1], *bp = querybuf ;
	char	linebuf[LINEBUFLEN + 1] ;


	rs = dialtcp(dsp->host,dsp->portspec,dsp->af,to,opts) ;
	s = rs ;
	if (rs < 0)
	    goto badconnect ;

/* send the query */

	bp = strwcpy(querybuf,query,(QUERYLEN - 2)) ;

	*bp++ = '\r' ;
	*bp++ = '\n' ;
	*bp = '\0' ;

	rs = u_write(s,querybuf,(bp - querybuf)) ;
	if (rs >= 0) {
	    FILEBUF	rd ;

	    u_shutdown(s,SHUT_WR) ;

	    rs = filebuf_start(&rd,s,0L,BUFLEN,fbo) ;
	    if (rs >= 0) {

	        while (rs >= 0) {

	            rs = filebuf_readline(&rd,linebuf,LINEBUFLEN,to) ;
	            len = rs ;
	            if (rs <= 0)
	                break ;

	            while ((len > 0) && isspace(linebuf[len - 1]))
	                len -= 1 ;

	            linebuf[len] = '\0' ;
	            rs = bprintlines(ofp,pip->linelen,linebuf,len) ;
		    wlen += rs ;
		    if (rs < 0)
			break ;

	        } /* end while */

	        filebuf_finish(&rd) ;
	    } /* end if (initialized file buffer) */

	} /* end if (successful write) */

	u_close(s) ;

badconnect:
ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procquery) */



