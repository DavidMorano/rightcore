/* main */

/* include graphical images into a DWB document */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written (from previous like it).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a filter for DWB type documents that invoke the '.BG' (Begin
        Graphic) macro. It is used as standard filters are used, just give it
        input, it will give you output with the macro removed !

	Synopsis:

	$ incima [infile|-] 


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<utime.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<char.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	CMDBUFLEN	((MAXPATHLEN * 2) + 50)

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	NPARG		1


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;
static int	mkimadir(const char *) ;

static char	*ps_getfile(char *,char *,char *) ;


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"HELP",
	"ROOT",
	"of",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_root,
	argopt_of,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
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

static const char	*psdir[] = {
	"/proj/starbase/tools/lib/incima/ps",
	"/mt/mtgzfs8/hw/tools/lib/incima/ps",
	"/usr/add-on/local/lib/incima/ps",
	NULL
} ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct ustat	sa, sb ;
	struct utimbuf	ft ;
	PROGINFO	pi, *pip = &pi ;

	bfile	errfile ;
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	*fpa[3] ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, rs1 ;
	int	len, i, j, k ;
	int	ifd, line ;
	int	macrolen ;
	int	macrolen1 = strlen(MACRONAME1) ;
#ifdef	MACRONAME2
	int	macrolen2 = strlen(MACRONAME2) ;
#endif
#ifdef	MACRONAME3
	int	macrolen3 = strlen(MACRONAME3) ;
#endif
	int	bo ;
	int	rlen ;
	int	ii ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_dash = FALSE ;
	int	f_verbose = FALSE ;
	int	f_help = FALSE ;
	int	f_bol = TRUE ;
	int	f_eol ;
	int	f_dirpresent = FALSE ;
	int	f_noexist = FALSE ;
	int	f_noread = FALSE ;
	int	f_unknown = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*pr = NULL ;
	const char	*rline ;
	const char	*homedir = getenv(VARHOMEDNAME) ;
	const char	*imatype ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*imafname = NULL ;
	const char	*cp, *cp2 ;
	char	argpresent[MAXARGGROUPS] ;
	char	linebuf[LINELEN + 1] ;
	char	buf[CMDBUFLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	rootfname[MAXPATHLEN + 1] ;
	char	psfname[MAXPATHLEN + 1] ;
	char	ps_noexist[MAXPATHLEN + 1] ;
	char	ps_noread[MAXPATHLEN + 1] ;
	char	ps_unknown[MAXPATHLEN + 1] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;
	
	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(pip->efp,BC_LINEBUF) ;
	}

/* initial stuff */

	for (i = 0 ; i < 3 ; i += 1) fpa[i] = NULL ;

	pip->verboselevel = 1 ;

/* go to the arguments */

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
	                        rs = SR_INVALID ;
	                         printf(pip->efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

	                	case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    rs = cfdeci(avp,avl,
	                                        &pip->debuglevel) ;

	                            }

	                            break ;

	                case 'V':
	                    f_version = TRUE ;
	                    break ;

	                case 'v':
	                    f_verbose = TRUE ;
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
	if (DEBUGLEVEL(1))
	    debugprintf("main: debuelevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    usage(pip) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

/* program search name */

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,NULL) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    bprintf(pip->efp,"%s: sn=%s\n",
	        pip->progname,pip->searchname) ;

	}

/* help file */

	if (f_usage)
	    usage(pip) ;

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check arguments */

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
			switch (pan) {

			case 0:
				ifname = cp ;
				break ;

			} /* end switch */

			pan += 1 ;

	    } /* end for */

/* open files */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: ifname=%s\n",ifname) ;
#endif

	rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;
	if (rs < 0) {

	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: output unavailable (%d)\n",
	    pip->progname,rs) ;

	    goto badoutopen ;
	}

/* other initializations */

	if ((ifname != NULL) && (ifname[0] != '\0'))
	rs = bopen(ifp,ifname,"r",0666) ;

	else
	rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	if (rs < 0)
	    goto badinfile ;

/* go through the loops */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: process\n") ;
#endif

	line = 1 ;
	f_bol = TRUE ;
	while ((rs = breadline(ifp,linebuf,LINELEN)) > 0) {

	    len = rs ;
	    f_eol = (linebuf[len - 1] == '\n') ;
	    linebuf[len] = '\0' ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	        debugprintf("main: processing a line begin=%d end=%d\n%t",
	            f_bol,f_eol,linebuf,len) ;
#endif

	    macrolen = 0 ;
	    if (f_bol) {

	        if ((len >= (macrolen1 + 1)) &&
	            (strncmp(linebuf,MACRONAME1,macrolen1) == 0))
	            macrolen = macrolen1 ;

#ifdef	MACRONAME2
	        if ((! macrolen) && (len >= (macrolen2 + 1)) &&
	            (strncmp(linebuf,MACRONAME2,macrolen2) == 0))
	            macrolen = macrolen2 ;
#endif

#ifdef	MACRONAME3
	        if ((! macrolen) && (len >= (macrolen3 + 1)) &&
	            (strncmp(linebuf,MACRONAME3,macrolen3) == 0))
	            macrolen = macrolen3 ;
#endif

	    }

/* do we have a macro invocation? */

	    if (macrolen > 0) {

	        if (len > (macrolen + 1)) {

	if (DEBUGLEVEL(5))
			bprintf(pip->efp,
	                "%s: got an image include request - line=%d\n",
	                pip->progname,line) ;

/* get the image type */

	            cp = linebuf + (macrolen + 1) ;
	            while (CHAR_ISWHITE(*cp)) 
			cp += 1 ;

	            imatype = cp ;
	            cp2 = cp ;
	            while (*cp2 && (! CHAR_ISWHITE(*cp2))) 
				cp2 += 1 ;

	            if (*cp2 != '\0') 
			*cp2++ = '\0' ;

/* get the file name */

	            while (CHAR_ISWHITE(*cp2)) 
			cp2 += 1 ;

	            imafname = cp2 ;
	            while (*cp2 && (! CHAR_ISWHITE(*cp2))) 
			cp2 += 1 ;

	            if (*cp2 != '\0') 
			*cp2++ = '\0' ;

/* strip the newline from the end if there is one */

	            if (linebuf[len - 1] == '\n')
	                linebuf[--len] = '\0' ;

	            rline = cp2 ;
	            rlen = strlen(rline) ;

/* does the file exist? */

	            if ((u_stat(imafname,&sa) >= 0) && 
	                (! S_ISDIR(sa.st_mode)) && 
	                (u_access(imafname,R_OK) >= 0)) {

/* check if the PostScript directory exists and is writable */

	                if (! f_dirpresent) {

	                    f_dirpresent = TRUE ;
	                    mkimadir(IMADIR) ;

	                }

/* get file root name */

	                cp = strbasename(imafname) ;

	                strcpy(rootfname,cp) ;

	                if ((cp = strrchr(rootfname,'.')) != NULL)
	                    *cp++ = '\0' ;

	                if (*imatype == '-') {

	                    if (*cp != '\0') {
	                        imatype = cp ;
	                    } else
	                        imatype = "unk" ;

	                }

/* make the PS file name for this image */

			mkpath2(tmpfname,IMADIR,rootfname) ;

			mkfnamesuf1(psfname,tmpfname,"ps") ;

	                rs = u_stat(psfname,&sb) ;

/* perform the time checks */

	                if ((rs < 0) || (sa.st_mtime > sb.st_mtime)) {

	                    if ((u_access(psfname,F_OK) >= 0) &&
	                        (u_access(psfname,W_OK) < 0)) {

	                        snprintf(buf,CMDBUFLEN,
	                            "/bin/rm -fr %s 2> /dev/null",
	                            psfname) ;

	                        system(buf) ;

	                    }

	                    snprintf(buf,CMDBUFLEN,
				"incima-cvt %s %s > %s",
	                        imatype,imafname,psfname) ;

		if (DEBUGLEVEL(5))
				bprintf(pip->efp,
	                        "%s: executing $ %s\n",pip->progname,buf) ;

	                    system(buf) ;

/* check resulting file for zero length */

	                    if ((u_stat(psfname,&sb) >= 0) &&
	                        (sb.st_size == 0)) {

#ifdef	COMMENT
	                        snprintf(buf,CMDBUFLEN,
	                            "/bin/touch 0101002370 %s 2> /dev/null",
	                            psfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	                            debugprintf("main: cmd=\"%s\"\n",buf) ;
#endif

#ifdef	BAD_FOR_SOME_REASON
	                        system(buf) ;
#else
	                        bopencmd(fpa,buf) ;
#endif

#else
	                        ft.actime = sb.st_atime ;
	                        ft.modtime = 10000 ;
	                        utime(psfname,&ft) ;
#endif
	                        if (! f_unknown) {

	                            f_unknown = TRUE ;
	                            ps_getfile(cp,PS_UNKNOWN,ps_unknown) ;

	                        }

	                        if ((ps_unknown != NULL) && 
	                            (ps_unknown[0] != '\0'))
	                            strwcpy(psfname,ps_unknown,MAXPATHLEN) ;

	                    } /* end if (zero length size) */

	                } /* end if (PostScript file update) */

/* write out the PostScript file reference */

	                bprintf(ofp,".BP %s %t\n",psfname,rline,rlen) ;

	            } else {

/* handle error conditions */

	                if ((! S_ISDIR(sa.st_mode)) &&
	                    (u_access(imafname,F_OK) >= 0)) {

	                    if (! f_noread) {

	                        f_noread = TRUE ;
	                        ps_getfile(cp,PS_NOREAD,ps_noread) ;

	                    }

	                    if ((ps_noread != NULL) && (ps_noread[0] != '\0'))
	                        bprintf(ofp,".BP %s %t\n",
	                            ps_noread,rline,rlen) ;

	                    else
	                        bprintf(ofp,".\\\"_ .BG %s %s %t\n",
	                            imatype, imafname,rline,rlen) ;

	                    bprintf(pip->efp,
	                        "%s: file \"%s\" is unreadable\n",
	                        pip->progname,imafname) ;

	                } else {

	                    if (! f_noexist) {

	                        f_noexist = TRUE ;
	                        ps_getfile(cp,PS_NOEXIST,ps_noexist) ;

	                    }

	                    if ((ps_noexist != NULL) && (ps_noexist[0] != '\0'))
	                        bprintf(ofp,".BP %s %t\n",
	                            ps_noexist,rline,rlen) ;

	                    else
	                        bprintf(ofp,".\\\"_ .BG %s %s %t\n",
	                            imatype, imafname,rline,rlen) ;

	                    bprintf(pip->efp,
	                        "%s: file \"%s\" does not exist\n",
	                        pip->progname,imafname) ;

	                }
	            }

/* end of hassle */

	        } else {

	            bprintf(pip->efp,
	                "%s: incomplete macro invocation - line=%d\n",
	                pip->progname,line) ;

	            bprintf(ofp,
	                ".\\\"_ .BG ** incomplete invocation **\n") ;

	        }

/* whew, done with this reference */

	if (DEBUGLEVEL(5))
			bprintf(pip->efp,
	            "%s: done with this image include macro - line=%d\n",
	            pip->progname,line) ;

	    } else if ((rs = bwrite(ofp,linebuf,len)) < 0)
	        goto badwrite ;

	    if (f_eol) 
		line += 1 ;

/* bottom of loop */

	    f_bol = f_eol ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	        debugprintf("main: done with this line\n") ;
#endif

	} /* end while */

	bclose(ifp) ;

badinopen:
	bclose(ofp) ;

done:
badoutopen:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: existing ex=%u (%d)\n",
		pip->progname,ex,rs) ;

retearly:
ret2:
	bclose(pip->efp) ;

ret1:
	proginfo_finish(pip) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad arguments */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
		pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

/* other bad things */
badinfile:
	ex = EX_NOINPUT ;
	bprintf(pip->efp,"%s: cannot open the input file (%d)\n",
	    pip->progname,rs) ;

	goto retearly ;

badwrite:
	ex = EX_DATAERR ;
	bclose(ifp) ;

	bclose(ofp) ;

	bprintf(pip->efp,
	    "%s: write failed (%d)\n",
	    pip->progname,rs) ;

	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [<infile>] [-?VD]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s: \tthe macro usage is '.BG format file [args ...]'\n",
		pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* get an error type PostScript file */
static char *ps_getfile(homedir,filename,buf)
char	*homedir, *filename ;
char	*buf ;
{
	int	rs ;
	int	i, j ;
	char	*rname ;
	char	*fname ;

#if	CF_DEBUG
	debugprintf("ps_getfile: entered \n") ;
#endif

	buf[0] = '\0' ;
	rname = NULL ;
	fname = filename ;
	for (i = 0 ; (rname == NULL) && (i < 2) ; i += 1) {

#if	CF_DEBUG
	    debugprintf("ps_getfile: top of loop %d\n",i) ;
#endif

	    if (i == 1) fname = PS_DEFAULT ;

/* check current directory */

#if	CF_DEBUG
	    debugprintf("ps_getfile: check current \n") ;
#endif

	    if (rname == NULL) {

	        if ((rs = u_access(fname,R_OK)) >= 0)
	            rname = fname ;

	    }

#if	CF_DEBUG
	    debugprintf("ps_getfile: more current \n") ;
#endif

	    if (rname == NULL) {

	        mkpath2(buf,"ps",fname) ;

	        if ((rs = u_access(buf,R_OK)) >= 0)
	            rname = buf ;

	    }

/* check HOME directory */

#if	CF_DEBUG
	    debugprintf("ps_getfile: home \n") ;
#endif

	    if ((rname == NULL) && (homedir != NULL)) {

	        snprintf(buf,CMDBUFLEN,
			"%s/lib/incima/ps/%s",homedir,fname) ;

	        if ((rs = u_access(buf,R_OK)) >= 0)
	            rname = buf ;

	    }

#if	CF_DEBUG
	    debugprintf("ps_getfile: more home \n") ;
#endif

	    if ((rname == NULL) && (homedir != NULL)) {

	        snprintf(buf,CMDBUFLEN,
			"%s/lib/incima/%s",homedir,fname) ;

	        if ((rs = u_access(buf,R_OK)) >= 0)
	            rname = buf ;

	    }

/* check other places */

#if	CF_DEBUG
	    debugprintf("ps_getfile: other places \n") ;
#endif

	    for (j = 0 ; ((rname == NULL) && (psdir[j] != NULL)) ; j += 1) {

#if	CF_DEBUG
	        debugprintf("ps_getfile: other loop %d\n",j) ;

	        debugprintf("trying directory > %s\n",psdir[j]) ;
#endif

	        mkpath2(buf,psdir[j],fname) ;

#if	CF_DEBUG
	        debugprintf("trying file > %s\n",buf) ;
#endif

	        if ((rs = u_access(buf,R_OK)) >= 0)
	            rname = buf ;

	    } /* end for */

#if	CF_DEBUG
	    debugprintf("ps_getfile: bottom of outer loop\n") ;
#endif

	} /* end while */

/* return with what we have */

	if (rname == NULL) 
		buf[0] = '\0' ;

	else if (rname != buf) 
		strwcpy(buf,rname,MAXPATHLEN) ;

#if	CF_DEBUG
	debugprintf("ps_getfile: ret w/ > %s\n",buf) ;
#endif

	return rname ;
}
/* end subroutine (ps_getfile) */


/* make the image directory writable ! */
static int mkimadir(dir)
const char	dir[] ;
{
	int	f_make = FALSE ;

	char	buf[CMDBUFLEN + 1] ;


	if (u_access(dir,F_OK) < 0) 
		f_make = TRUE ;

	if (! f_make) {

	    if (u_access(dir,W_OK) < 0) {

	        snprintf(buf,CMDBUFLEN,
			"/bin/chmod 777 %s 2> /dev/null",dir) ;

	        system(buf) ;

	    }

	    if (u_access(dir,W_OK) < 0) {

	        snprintf(buf,CMDBUFLEN,
			"/bin/rm -fr %s 2> /dev/null",dir) ;

	        system(buf) ;

	        f_make = TRUE ;
	    }

	}  /* end if */

	if (f_make) {

	    snprintf(buf,CMDBUFLEN,
	        "{ /bin/mkdir %s ; /bin/chmod ugo+rwx %s ; } 2> /dev/null",
	        dir,dir) ;

	    system(buf) ;

	}

	return 0 ;
}
/* end subroutine (mkimadir) */



