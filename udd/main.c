/* main */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	CF_DEBUGS	0
#define	CF_DEBUG	0	


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<string.h>
#include	<utime.h>
#include	<time.h>
#include	<pwd.h>

#include	<bfile.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<field.h>
#include	<logfile.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"commands.h"
#include	"returns.h"


/* local defines */

#define		MAXARGINDEX	100
#define		MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	cfdecl(const char *,int,long) ;

extern char	*strbasename(char *) ;
extern char	*timestr_log() ;


/* external variables */


/* global variables */

struct global		g ;


/* local structures */


/* forward references */

static int	getfilepath() ;


/* local variables */

static const char *argopts[] = {
	    "version",
	        NULL,
} ;


#define	ARGOPT_VERSION	0


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{

	struct sigaction	ss ;
	sigset_t	signalmask ;
	struct ustat	sb ;
	struct dirstat	*dsp ;
	struct tm	ts, *timep ;
	struct utimbuf	ft ;
	struct passwd	ps, *pp ;
	struct field	fsb, *fbp = &fsb ;
	SERINFO		u ;
	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		infile, *ifp = &infile ;
	time_t		newtime ;
	mode_t		newmode ;

	int	argr, argl, aol, avl, npa, pan, maxai, kwi ;
	int	i ;
	int	len, l, fd, rs ;
	int	dirbufl = 0 ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_listopen  ;
	int	cmdl, cmdr ;
	int	f_bol, f_eol ;
	int	fl ;
	int	f_dir ;

	char	*argp, *aop, *akp, *avp ;
	char	*ufname = NULL ;
	char	linebuf[LINELEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	dirbuf[BUFLEN + 1] ;
	char	*cp ;
	char	argpresent[MAXARGGROUPS] ;
	char	ubuf[USERINFO_LEN+1] ;
	char	fterms[32] ;
	char	*cmdp ;
	char	*fnp ;


	g.progname = strbasename(argv[0]) ;

	if (bopen(efp,(char *) 2,"wca",0644) < 0) return BAD ;

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

#if	CF_DEBUGS
	debugsetfd(2) ;

	debugprintf("main: efp=%08X\n",efp) ;
#endif

	g.efp = efp ;

/* early things to initialize */

	g.ofp = ofp ;
	g.ifp = ifp ;

	g.f.debug = FALSE ;
	g.f.verbose = FALSE ;

#if	defined(SYSV)
	g.f.sysv_ct = TRUE ;
#else
	g.f.sysv_ct = FALSE ;
#endif

	g.f.sysv_rt = FALSE ;
	if (access("/usr/sbin",R_OK) >= 0) g.f.sysv_rt = TRUE ;

	if (g.progname[0] == 'n') g.f.newprogram = TRUE ;

	dirbuf[0] = '\0' ;

/* get user profile information */

	if ((rs = userinfo(&u,ubuf,USERINFO_LEN,NULL)) < 0) goto baduser ;

#if	CF_DEBUGS
	debugprintf("main: user=%s homedir=%s\n",
	    u.username,u.homedname) ;
#endif


/* get the current time-of-day */

	g.daytime = time(NULL) ;

#ifdef	SYSV
	timep = (struct tm *) localtime_r((time_t *) &g.daytime,&ts) ;
#else
	timep = (struct tm *) localtime((time_t *) &g.daytime) ;
#endif

/* initialize some stuff before command line argument processing */

	g.logfname = NULL ;
	g.debuglevel = 0 ;

/* process program arguments */

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

#if	CF_DEBUGS
	            debugprintf("main: got an option\n") ;
#endif

	            aop = argp + 1 ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                debugprintf("main: got an option key w/ a value\n") ;
#endif

	                aol = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                f_optequal = TRUE ;

	            } else
	                avl = 0 ;

/* do we have a keyword match or should we assume only key letters ? */

#if	CF_DEBUGS
	            debugprintf("main: about to check for a key word match\n") ;
#endif

	            if ((kwi = matstr(argopts,aop,aol)) >= 0) {

#if	CF_DEBUGS
	                debugprintf("main: got an option keyword, kwi=%d\n",
	                    kwi) ;
#endif

	                switch (kwi) {

	                case ARGOPT_VERSION:
#if	CF_DEBUGS
	                    debugprintf("main: version key-word\n") ;
#endif
	                    f_version = TRUE ;
	                    if (f_optequal) goto badargextra ;

	                    break ;

/* default action and user specified help */
	                default:
	                    f_usage = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

#if	CF_DEBUGS
	                debugprintf("main: got an option key letter\n") ;
#endif

	                while (akl--) {

#if	CF_DEBUGS
	                    debugprintf("main: option key letters\n") ;
#endif

	                    switch ((int) *aop) {

	                    case 'V':

#if	CF_DEBUGS
	                        debugprintf("main: version key-letter\n") ;
#endif
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        g.f.debug = TRUE ;
	                        g.debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (cfdec(avp,avl, &g.debuglevel) != OK)
/* code folded from here */
	                                goto badargvalue ;

/* unfolding */
	                        }

	                        break ;

/* log file name */
	                    case 'l':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            g.logfname = NULL ;
	                            if (avl) g.logfname = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.logfname = argp ;

	                        }

	                        break ;

	                    default:
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            g.progname,*aop) ;

	                    case '?':
	                        f_usage = TRUE ;

	                    } /* end switch */

	                    akp += 1 ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } else {

/* an argument consisting of only a plus or minus sign character */

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



#if	CF_DEBUGS
	debugprintf("main: finished parsing command line arguments\n") ;
#endif

	if (g.f.debug) {

	    bprintf(g.efp,
	        "%s: debbugging turned on to level %d\n",
	        g.progname,g.debuglevel) ;

	    bcontrol(g.efp,BC_LINEBUF,0) ;

	    bflush(g.efp) ;

	}


/* check out if the log file is OK */

/* make some log entries */

	if ((g.logfname != NULL) && (g.logfname[0] != '\0')) {

#if	CF_DEBUGS
	    debugprintf("main: about to do 'logfile_open'\n") ;
#endif

	    if ((rs = logfile_open(&g.lh,g.logfname,0,0666,u.logid)) < 0)
	        bprintf(g.efp,"%s: could not open the log file\n",
	            g.progname) ;

	    buf[0] = '\0' ;
	    if (u.mailname != NULL) sprintf(buf,"(%s)",u.mailname) ;

#if	CF_DEBUGS
	    debugprintf("main: about to do 'logfile_printf'\n") ;
#endif

	    logfile_printf("%02d%02d%02d %02d%02d:%02d %-14s %s/%s\n",
	        timep->tm_year,
	        timep->tm_mon + 1,
	        timep->tm_mday,
	        timep->tm_hour,
	        timep->tm_min,
	        timep->tm_sec,
	        g.progname,(g.f.sysv_ct) ? "SYSV" : "BSD",VERSION) ;

	    logfile_printf("os=%s %s!%s %s\n",
	        g.f.sysv_rt ? "SYSV" : "BSD",u.nodename,u.username,buf) ;

	} /* end if */

/* handle some simple stuff early */

	if (f_version) {

	    bprintf(efp,"%s: version %s [%s]\n",
	        g.progname,VERSION,(g.f.sysv_ct) ? "SYSV" : "BSD") ;

	    bprintf(efp,"%s: m=%s os=%s u=%s\n",
	        g.progname,u.nodename,(g.f.sysv_rt) ? "SYSV" : "BSD",
	        u.username) ;

	}

	if (f_usage) goto usage ;

	if (f_version) goto earlyret ;


/* load up the positional arguments */

#if	CF_DEBUG
	if (g.f.debug) debugprintf(
	    "main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i)) {

#if	CF_DEBUG
	        if (g.f.debug) debugprintf(
	            "main: got a positional argument i=%d pan=%d\n",
	            i,pan) ;
#endif

	        switch (pan) {

	        case 0:
	            strcpy(dirbuf,argv[i]) ;

	            break ;

	        default:
	            bprintf(g.efp,
	                "%s: extra positional arguments ignored\n",
	                g.progname) ;

	        } /* end switch */

	        pan += 1 ;

	    } /* end if (got a positional argument) */

	} /* end for (loading positional arguments) */

	if (g.debuglevel > 1) bprintf(g.efp,
	    "%s: %d positional arguments\n",g.progname,npa) ;

/* check arguments */

	if (dirbuf[0] == '-') dirbuf[0] = '\0' ;

	dirbufl = strlen(dirbuf) ;
	f_dir = FALSE ;
	if ((dirbufl > 0) && (chdir(dirbuf) >= 0))
	    f_dir = TRUE ;

/* continue with initialization */

	if (g.f.debug)
	    bprintf(g.efp,"%s: current time %s\n",
	        g.progname,timestr_log(g.daytime,buf)) ;


/* initialize some stuff */

	if ((rs = bopen(ifp,(char *) 0,"wct",0644)) < 0)
	    goto badinopen ;

	if ((rs = bopen(ofp,(char *) 1,"wct",0644)) < 0)
	    goto badoutopen ;

	bcontrol(ofp,BC_LINEBUF,0) ;


/* field terminators */

	fieldterms(fterms,0,"# \t\r\n") ;

/* go through the loops */

	bprintf(ofp,"#### Update Directory Daemon server says hello !\n") ;


	f_bol = TRUE ;
	while ((len = breadline(ifp,linebuf,LINELEN)) > 0) {

	    f_eol = FALSE ;
	    if (linebuf[len - 1] == '\n') f_eol = TRUE ;

/* handle network disassembling parties */

	    if (! f_eol) {

	        while ((! f_eol) && (len < LINELEN) &&
	            ((l = breadline(ifp,linebuf + len,LINELEN - len)) > 0)) {

	            len += l ;
	            if (linebuf[len - 1] == '\n')
	                f_eol = TRUE ;

	        } /* end while */

	        if ((l < 0) || ((! f_eol) && (l == 0)))
	            goto badread ;

	    } /* end if (insuring EOL) */

/* we have a line (finally !), process it */

	    linebuf[--len] = '\0' ;

/* empty (and comment only) lines are ignored */

	    if (field_start(fbp,linebuf,len) <= 0)
	        continue ;

	    if (fbp->lp[0] == '#')
	        continue ;

/* parse out the service request */

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: ll=%d\n",
	            fbp->rlen) ;
#endif

	    if ((l = field(fbp,fterms)) <= 0)
	        continue ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: l=%d %02X cmd>%W<\n",
	            l,fbp->fp[0],fbp->fp,fbp->flen) ;
#endif

	    if ((i = matstr(commands,fbp->fp,l)) < 0) {

	        bprintf(ofp,"%W %d\n",fbp->fp,l,UDR_ILLEGAL) ;

	        continue ;
	    }

	    cmdp = fbp->fp ;
	    cmdl = fbp->flen ;

	    logfile_printf("CMD> %W\n",linebuf,len) ;

/* switch on the command */

	    cmdr = UDR_NOTIMPLEMENTED ;
	    switch (i) {

/* return the length of a file */
	    case CMD_GETLEN:
	        if ((cmdr = getfilepath(dirbuf,dirbufl,f_dir,
	            fbp,fterms,&fnp)) < 0)
	            break ;

	        cmdr = UDR_NOEXIST ;
	        if (stat(fnp,&sb) < 0) break ;

	        logfile_printf("RSP> %W %d %ld\n",
	            cmdp,cmdl,UDR_OK,sb.st_size) ;

	        bprintf(ofp,"%W %d %ld\n",
	            cmdp,cmdl,UDR_OK,sb.st_size) ;

	        cmdr = UDR_OK ;
	        break ;

	    case CMD_GETTIME:
	        if ((cmdr = getfilepath(dirbuf,dirbufl,f_dir,
	            fbp,fterms,&fnp)) < 0)
	            break ;

	        cmdr = UDR_NOEXIST ;
	        if (stat(fnp,&sb) < 0) break ;

	        logfile_printf("RSP> %W %d %ld\n",
	            cmdp,cmdl,UDR_OK,sb.st_mtime) ;

	        bprintf(ofp,"%W %d %ld\n",
	            cmdp,cmdl,UDR_OK,sb.st_mtime) ;

	        cmdr = UDR_OK ;
	        break ;

	    case CMD_SETTIME:
	        if ((cmdr = getfilepath(dirbuf,dirbufl,f_dir,
	            fbp,fterms,&fnp)) < 0)
	            break ;

/* get the time paramter */

	        cmdr = UDR_NOTENOUGH ;
	        if ((fl = field(fbp,fterms)) <= 0) break ;

	        cmdr = UDR_BADVAL ;
	        if (cfdecl(fbp->fp,fl,&newtime) < 0) break ;

	        cmdr = UDR_NOEXIST ;
	        if (stat(fnp,&sb) < 0) break ;

	        cmdr = UDR_READONLY ;
	        if (access(fnp,W_OK) < 0) break ;

/* do it, if we can */

	        ft.actime = sb.st_atime ;
	        ft.modtime = newtime ;
	        cmdr = UDR_BAD ;
	        if (utime(fnp,&ft) < 0) break ;

/* send back acknowledgement */

	        logfile_printf("RSP> %W %d %ld\n",
	            cmdp,cmdl,UDR_OK,newtime) ;

	        bprintf(ofp,"%W %d %ld\n",
	            cmdp,cmdl,UDR_OK,newtime) ;

	        cmdr = UDR_OK ;
	        break ;

	    case CMD_GETINFO:
	        if ((cmdr = getfilepath(dirbuf,dirbufl,f_dir,
	            fbp,fterms,&fnp)) < 0)
	            break ;

	        if (g.debuglevel > 1) 
			bprintf(g.efp,
	            "%s: GETINFO fnp=%s\n",g.progname,fnp) ;

	        cmdr = UDR_NOEXIST ;
	        if (stat(fnp,&sb) < 0) 
			break ;

/* get the username for the UID of this file */

	        pp = &ps ;
	        if ((rs = getpw_uid(&ps,buf,BUFLEN,sb.st_uid)) >= 0) {
	            cp = buf ;
	            sprintf(buf,"%d",sb.st_uid) ;
	        } else
	            cp = pp->pw_name ;

	        cmdr = UDR_OK ;
	        logfile_printf("RSP> %W %d %ld %ld %5o %s\n",
	            cmdp,cmdl,cmdr,
	            sb.st_size,sb.st_mtime,sb.st_mode,cp) ;

	        bprintf(ofp,"%W %d %ld %ld %5o %s\n",
	            cmdp,cmdl,cmdr,
	            sb.st_size,sb.st_mtime,sb.st_mode,cp) ;

	        break ;

/* change the mode of a file */
	    case CMD_CHMOD:
	        if ((cmdr = getfilepath(dirbuf,dirbufl,f_dir,
	            fbp,fterms,&fnp)) < 0)
	            break ;

/* get the mode paramter */

	        cmdr = UDR_NOTENOUGH ;
	        if ((fl = field(fbp,fterms)) <= 0) break ;

	        cmdr = UDR_BADVAL ;
	        if (cfoct(fbp->fp,fl,&newmode) < 0) break ;

	        cmdr = UDR_NOEXIST ;
	        if (stat(fnp,&sb) < 0) break ;

	        cmdr = UDR_READONLY ;
	        if (access(fnp,W_OK) < 0) break ;

	        newmode &= 07777 ;
	        newmode |= (sb.st_mode & (~ 07777)) ;

/* do it, if we can */

	        cmdr = UDR_BAD ;
	        if (chmod(fnp,newmode) < 0) break ;

/* send back acknowledgement */

	        cmdr = UDR_OK ;
	        logfile_printf("RSP> %W %d %5o\n",
	            cmdp,cmdl,UDR_OK,newmode) ;

	        bprintf(ofp,"%W %d %5o\n",
	            cmdp,cmdl,UDR_OK,newmode) ;

	        cmdr = UDR_OK ;
	        break ;

/* set the working directory for the server */
	    case CMD_SETDIR:
	        cmdr = UDR_NOTENOUGH ;
	        if ((fl = field(fbp,fterms)) <= 0) break ;

	        strwcpy(buf,fbp->fp,fl) ;

	        cmdr = UDR_NOEXIST ;
	        if (stat(buf,&sb) < 0) break ;

	        dirbufl = fl ;
	        strwcpy(dirbuf,fbp->fp,fl) ;

/* do it, if we can */

	        cmdr = UDR_OK ;
	        f_dir = TRUE ;
	        if (chdir(dirbuf) < 0) {

	            f_dir = FALSE ;
	            cmdr = UDR_PERM ;
	        }

/* send back acknowledgement */

	        cmdr = UDR_OK ;
	        logfile_printf("RSP> %W %d %W\n",
	            cmdp,cmdl,cmdr,dirbuf,dirbufl) ;

	        bprintf(ofp,"%W %d %W\n",
	            cmdp,cmdl,cmdr,dirbuf,dirbufl) ;

	        break ;

/* exit the server */
	    case CMD_EXIT:
	        cmdr = UDR_OK ;
	        logfile_printf("RSP> %W %d\n",cmdp,cmdl,UDR_OK) ;

	        bprintf(ofp,"%W %d\n",cmdp,cmdl,UDR_OK) ;

	        goto goodret ;

/* miscellaneous */
	    case CMD_GETDIR:
	    case CMD_NOOP:

	        if (g.debuglevel > 1) bprintf(g.efp,
	            "%s: GETINFO dir=%d\n",g.progname,f_dir) ;

	        cmdr = UDR_OK ;
	        logfile_printf("RSP> %W %d %W\n",cmdp,cmdl,cmdr,dirbuf,dirbufl) ;

	        bprintf(ofp,"%W %d %W\n",cmdp,cmdl,cmdr,dirbuf,dirbufl) ;

	        break ;

	    } /* end switch */

/* return acknowledgement */

	    if (cmdr < 0) {

	        logfile_printf("RSP> %W %d\n",
	            cmdp,cmdl,cmdr) ;

	        bprintf(g.ofp,"%W %d\n",
	            cmdp,cmdl,cmdr) ;

	    }

/* standard bottom of loop processing */

	    f_bol = f_eol ;

	} /* end while (command read loop) */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: exiting\n") ;
#endif

/* good return from program */
goodret:
	bclose(g.ifp) ;

	bclose(g.ofp) ;

earlyret:
	bclose(g.efp) ;

	return OK ;

/* come here for a bad return from the program */
badret:

#if	CF_DEBUGS
	debugprintf("main: exiting program BAD\n") ;
#endif

	bclose(g.ifp) ;

	bclose(g.ofp) ;

	bclose(g.efp) ;

	return BAD ;

/* program usage */
usage:
	bprintf(g.efp,
	    "%s: USAGE> %s [-DV] [-l logfile]",
	    g.progname,g.progname) ;

	bprintf(g.efp,
	    "[-newsgroups | -subject | -count]\n\t") ;

	goto badret ;

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
	bprintf(g.efp,"%s: could not open output (rs=%d)\n",
	    g.progname,rs) ;

	goto badret ;

badinopen:
	bprintf(g.efp,"%s: could not open standard input (rs=%d)\n",
	    g.progname,rs) ;

	goto badret ;

badopt:
	bprintf(g.efp,"%s: bad option someplace -- program error\n",
	    g.progname) ;

	goto badret ;

baduser:
	bprintf(g.efp,"%s: could not get user information (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badread:
	bprintf(g.efp,"%s: bad line read on input\n",
	    g.progname) ;

	bprintf(ofp,"ERROR %d\n",UDR_BAD) ;

	goto badret ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int getfilepath(buf,used,f_dir,fbp,fterms,rpp)
char		buf[] ;
struct field	*fbp ;
char		fterms[] ;
int		f_dir, used ;
char		**rpp ;
{
	int	fl ;


	if ((fl = field(fbp,fterms)) <= 0)
	    return UDR_NOTENOUGH ;

	memcpy(buf + used,fbp->fp,fl) ;

	buf[used + fl] = '\0' ;
	if (f_dir || (fbp->fp[0] == '/'))
	    *rpp = buf + used ;

	    else
	    *rpp = buf ;

	return OK ;
}
/* end subroutine (getfilepath) */



