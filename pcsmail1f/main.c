/* main */


#define	CF_DEBUGS	0
#define	CF_DEBUG	0
#define	CF_NOSEND	0
#define	CF_MAILER	0


/*******************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
		David A­D­ Morano
 *		J.Mukerji						
 *									
 * 									
 * Record of changes:							
 *									
 * 11/30: Fix for subject line pickup after a +edit invocation was put  
 *	  in today. The lines added are marked (11/30)	JM		
 *									
 * 2/7/85 Further changes to make the 11/30 fix work right JM           
 *		MR# gw85-00801						
 *									
 * 2/7/85 Fix to deal with the MR on undoing default options when new   
 *	  options are specified in the send options prompt		
 *		MR# gx84-33465

	= Jishnu Mukerji 11/13/89
	Yanked out the network routing related stuff and 
	got it to work purely as a User Agent for use in Sun 
	networks on top of the 'sendmail(8)' daemon program.

	= David A.D. Morano, 94/01/06
	What a piece of absolute )^&$#$(%^#% junk !!
*
*
*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<netinet/in.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<signal.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<logfile.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"prompt.h"
#include	"header.h"


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkgecosname(char *,int,const char *) ;
extern int	mkmailname(char *,int,const char *,int) ;
extern int	getnodedomain(char *,char *) ;

extern char	*strbasename(char *) ;
extern char	*mallocstr(), *malloc_sbuf() ;
extern char	*timestr_edate(time_t,char *) ;


/* external data */

extern struct global	g ;


/* global data */


/* forward references */

extern void	i_rmtemp() ;


/* exported subroutines */


int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile	errfile, *efp = &errfile ;
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	msgfile, *mfp = &msgfile ;
	bfile	tmpfile, *tfp = &tmpfile ;
	bfile	*fpa[3] ;

	struct ustat	sb ;

	struct tm	*timep ;

	struct passwd	*pp ;

	struct group	*gp ;

	struct address	*as_to = NULL ;
	struct address	*as_from = NULL ;
	struct address	*as_sender = NULL ;
	struct address	*as_replyto = NULL ;
	struct address	*as_cc = NULL ;
	struct address	*as_bcc = NULL ;

	offset_t		offset ;

	int		len, rs, status ;
	int		wstatus ;
	int		i, l ;
	int		f_send ;
	int		pid_child ;

	char		*option, *tmp ;
	char		nodename[NODENAMELEN + 1], domainname[MAXPATHLEN + 1] ;
	char		buf[BUFSIZE + 1], *bp ;
	char		linebuf[LINELEN + 1] ;
	char		*namep, *cp, *cp2 ;
	char		**ptr ;
	char		*realname ;


#if	CF_DEBUGS || CF_DEBUG
	if (((cp = getenv("ERROR_FD")) != NULL) &&
	      (cfdeci(cp,-1,&err_fd) >= 0))
		debugsetfd(err_fd) ;

	else
		debugsetfd(-1) ;
#endif


	g.progname = "PCSMAIL" ;
	if ((argc >= 1) && ((l = strlen(argv[0])) > 0)) {

	    while (argv[0][l - 1] == '/') l -= 1 ;

	    argv[0][l] = '\0' ;
	    g.progname = strbasename(argv[0]) ;

	    strcpy(buf,g.progname) ;

	    bp = buf ;
	    if (*bp == 'n') bp += 1 ;

	}

/* open the error output */

	g.efp = efp ;
	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;

	g.ifp = ifp ;
	if ((rs = bopen(g.ifp,BFILE_STDIN,"dr",0666)) < 0) 
		goto badinopen ;

	g.ofp = ofp ;
	if ((rs = bopen(ofp,BFILE_STDOUT,"dwct",0666)) < 0) 
		goto badoutopen ;


/* continue with less critical things */

	g.debuglevel = 0 ;
	g.pid = getpid() ;


/* default option is standard */

#if	CF_DEBUG
	g.debuglevel = 1 ;
	g.f.debug = TRUE ;
#else
	g.debuglevel = 0 ;
	g.f.debug = FALSE ;
#endif

	g.f.version = FALSE ;
	g.f.exit = FALSE ;

#if	CF_NOSEND
	g.f.nosend = TRUE ;
#else
	g.f.nosend = FALSE ;
#endif

#ifdef	SYSV
	g.f.sysv_ct = TRUE ;
#else
	g.f.sysv_ct = FALSE ;
#endif

	g.f.sysv_rt = FALSE ;
	if (access("/usr/sbin",R_OK) >= 0) g.f.sysv_rt = TRUE ;

	standard = 1 ;
	verbose = 0 ;

	f_name = TRUE ;
	f_fullname = FALSE ;
	g.f.interactive = FALSE ;
	if (isatty(0)) g.f.interactive = TRUE ;

/* get UNIX system name */

	nodename[0] = '\0' ;
	domainname[0] = '\0' ;
	getnodedomain(nodename,domainname) ;

	g.nodename = nodename ;
	g.localdomain = domainname ;
	if (g.localdomain[0] == '\0') g.localdomain = LOCALDOMAIN ;

/* try to set our company domain name */

	if ((cp = strrchr(g.localdomain,'.')) != NULL) {

#if	CF_DEBUG
	    debugprintf("main: got some company butt off local domain\n") ;
#endif

	    if ((cp2 = strchr(g.localdomain,'.')) != NULL) {

	        if (cp != cp2)
	            g.companydomain = cp2 + 1 ;

	        else
	            g.companydomain = g.localdomain ;

	    } else
	        g.companydomain = g.localdomain ;

	} else
	    g.companydomain = COMPANYDOMAIN ;

/* mail host */

	if ((g.mailhost = getenv("MAILHOST")) == NULL)
	    g.mailhost = MAILHOST ;

	if ((cp = strchr(g.mailhost,'.')) != NULL) {

	    strcpy(linebuf,g.mailhost) ;

	    linebuf[cp - g.mailhost] = '\0' ;
	    g.mailnode = mallocstr(linebuf) ;

	} else
	    g.mailnode = g.mailhost ;

/* mail domain */

	if ((g.maildomain = getenv("MAILDOMAIN")) == NULL)
	    g.maildomain = MAILDOMAIN ;


#if	CF_DEBUG
	debugprintf("main: ld=%s cd=%s md=%s\n",
	    g.localdomain,g.companydomain,g.maildomain) ;

	debugprintf("main: mh=%s mn=%s\n",
	    g.mailhost,g.mailnode) ;
#endif


/* initialize strings to ""*/

	*mess_id = *from = *sentby = *fromaddr = *reference = '\0' ;
	*keys = *subject = *moptions = *copyto = *bcopyto = *appfile = '\0' ;
	*received = *forwfile = *eforwfile = *retpath = *message = '\0' ;


/* save command name by which we have been invoked */

#ifdef	COMMENT
	strcpy(buf,argv[0]) ;

	if ((realname = strrchr(buf,'/')) == NULL)
	    realname = buf ;

	if (*realname == '/') realname += 1 ;

	strcpy(comm_name, realname) ;
#else
	strcpy(comm_name,bp) ;
#endif


/* determine mode of execution */

	if (strncmp(bp,"rpcsmail",6) == 0)
	    ex_mode = EM_REMOTE ;

	else if (strncmp(bp,"pcsinfo",7) == 0)
	    ex_mode = EM_PCSINFO ;

	else if (strcmp(bp,"info") == 0) {

	    if (buf[0] != 'n') g.progname = "PCSINFO" ;

	    else g.progname = "NPCSINFO" ;

	    argv[0] = g.progname ;
	    ex_mode = EM_PCSINFO ;

	} else if (strcmp(bp,"pc") == 0)
	    ex_mode = EM_PC ;

	else if (strcmp(bp,"pcsmail") == 0)
	    ex_mode = EM_PCSMAIL ;

	else {

	    if (buf[0] == 'n') g.progname = "NPCSMAIL" ;

	    else g.progname = "PCSMAIL" ;

	    ex_mode = EM_PCSMAIL ;
	    argv[0] = g.progname ;
	}

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: we are %s\n",g.progname) ;
#endif

/* initialize some other common stuff only needed for mail operations */

	g.username = NULL ;
	g.uid = getuid() ;

/* try to get a passwd entry */

	if ((cp = getenv("LOGNAME")) != NULL) {

	    if ((pp = getpwnam(cp)) != NULL)
	        if (g.uid == pp->pw_uid) g.username = cp ;

	}

	if ((g.username == NULL) && ((cp = getlogin()) != NULL)) {

	    if ((pp = getpwnam(cp)) != NULL)
	        if (g.uid == pp->pw_uid) g.username = mallocstr(cp) ;

	}

	if (g.username == NULL) {

	    if ((pp = getpwuid(g.uid)) != NULL)
	        g.username = mallocstr(pp->pw_name) ;

	    else
	        g.username = "GUEST_UID" ;

	}

/* get the GECOS name field (leave in variable 'cp') if we can also */

	if (pp != NULL) {

	    g.gid = pp->pw_gid ;
	    g.homedir = mallocstr(pp->pw_dir) ;

	    cp = pp->pw_gecos ;

	} else {

	    g.gid = 1 ;
	    if ((gp = getgrnam("other")) != NULL)
	        g.gid = gp->gr_gid ;

	    g.homedir = getenv("HOME") ;

	    cp = getenv("NAME") ;

	}

/* process the GECOS name if supplied */

	g.gecosname = NULL ;
	if (cp != NULL) {

	    if (ns_fixgecos(cp,buf,BUFSIZE) >= 0)
	        g.gecosname = mallocstr(buf) ;

	}

/* can we make some sort of perferred 'mail' name ? */

	g.mailname = g.gecosname ;
	if (g.gecosname != NULL) {

	    if (mkmailname(buf,BUFSIZE,g.gecosname,-1) >= 0)
	        g.mailname = mallocstr(buf) ;

	}


/* get the 'mail' group ID */

	g.gid_mail = MAILGROUP ;
	if ((gp = getgrnam("mail")) != NULL)
	    g.gid_mail = gp->gr_gid ;


/* get the time and the broken out time structure */

	time(&g.daytime) ;

	timep = localtime(&g.daytime) ;


/* get the package directory path */

	if ((g.pcs = getenv("PCS")) == NULL)
		g.pcs = PCS ;


/* The following section is used only for debugging purposes */
/* if LOGFILE is defined then open it */

/* create a log ID */

	snsd(buf,BUFLEN,g.nodename,g.pid) ;

	g.logid = mallocstr(buf) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: logid=%s\n",g.logid) ;
#endif

	mkpath2(buf,g.pcs,LOGFILE) ;

/* open the log file */

#if	CF_DEBUG
	debugprintf("main: logfile_open logfile=\"%s\"\n",buf) ;
#endif

	rs = logfile_open(&g.lh,buf,0,0666,g.logid) ;

#if	CF_DEBUG
	debugprintf("main: logfile_open return (rs %d)\n",rs) ;
#endif

/* make a log entry */

	buf[0] = '\0' ;
	if (g.mailname != NULL) 
		sprintf(buf,"(%s)",g.mailname) ;

	logfile_printf(&g.lh,"%02d%02d%02d %02d%02d:%02d %-14s %s/%s\n",
	    timep->tm_year,
	    timep->tm_mon + 1,
	    timep->tm_mday,
	    timep->tm_hour,
	    timep->tm_min,
	    timep->tm_sec,
	    g.progname,
	    VERSION,(g.f.sysv_ct ? "SYSV" : "BSD")) ;

	logfile_printf(&g.lh,"os=%s %s!%s %s\n",
	    (g.f.sysv_rt ? "SYSV" : "BSD"),g.nodename,g.username,buf) ;

	logfile_printf(&g.lh,"ld=%s cd=%s md=%s\n",
	    g.localdomain,g.companydomain,g.maildomain) ;

	logfile_printf(&g.lh,"mh=%s mn=%s\n",
	    g.mailhost,g.mailnode) ;


/* create an ERROR log file */

	mkpath2(buf,g.pcs,ERRFILE) ;

	logfile_open(&g.eh,buf,0,0666,g.logid) ;


/* write the user file */

	mkpath2(buf,g.pcs,USERFILE) ;

	if (bopen_wall(tfp,buf) >= 0) {

	    bprintf(tfp,"%s!%s\n",g.nodename,g.username) ;

	    bclose(tfp) ;

	} /* end if */

/* make a "N"ew user file entry if we are running a "N"ew program version */

#ifdef	NUSERFILE
	mkpath2(buf,g.pcs,NUSERFILE) ;

	if (bopen_wall(tfp,buf) >= 0) {

	    bprintf(tfp,"%s!%s\n",g.nodename,g.username) ;

	    bclose(tfp) ;

	} /* end if */
#endif /* NUSERFILE */


/* get some programs */

	g.prog_editor = NULL ;
	g.prog_sendmail = PROG_SENDMAIL ;

	if ((g.prog_pager = getenv("PAGER")) == NULL)
		g.prog_pager = PROG_PAGER ;

	g.prog_pcscl = PROG_PCSCL ;
	cp = PROG_PCSCL ;
	if (cp[0] != '/') {

	    if (getfiledirs(NULL,cp,"x",NULL) <= 0) {

	        mkpath2(buf,g.pcs,cp) ;

	        g.prog_pcscl = mallocstr(buf) ;

	    }

	}

	g.prog_pcscleanup = PROG_PCSCLEANUP ;
	cp = PROG_PCSCLEANUP ;
	if (cp[0] != '/') {

	    if (getfiledirs(NULL,PROG_PCSCLEANUP,"x",NULL) <= 0) {

	        mkpath2(buf,g.pcs,PROG_PCSCLEANUP) ;

	        g.prog_pcscleanup = mallocstr(buf) ;

	    }

	}


/* cleanup work (failing parent and successful child execute inside) */

	if ((pid_child = uc_fork()) <= 0) {

	    if (pid_child == 0) {
	        for (i = 0 ; i < 4 ; i += 1) close(i) ;
	        setsid() ;
	    }

	    fpa[0] = NULL ;
	    fpa[1] = NULL ;
	    fpa[2] = NULL ;
	    if ((rs = bopencmd(fpa,g.prog_pcscleanup)) < 0) {

	        logfile_printf(&g.lh,
	            "could not run the cleanup program (rs %d)\n",rs) ;

	    }

/* the successful child (if any) exits here */

	    if (pid_child == 0) exit(0) ;

	} /* end if (cleanup code) */


/* get environment variables */

	getvars() ;

/* handle some cases right now ! */

	if (ex_mode == EM_PCSINFO) {

	    doinfo(argc, argv) ;

	    goto goodret ;

	} /* end if (done with program) */


/* we continue with other initialization that is needed for mail operations */

/* get the standard UNIX envelope date string */

	(void) timestr_edate(g.daytime,buf) ;

	g.envdate = mallocstr(buf) ;

/* get the standard RFC 822 "DATE" header date string */

	msgdate(g.daytime,buf) ;

	g.msgdate = mallocstr(buf) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: env=\"%s\" header=\"%s\"\n",
	        g.envdate,
	        g.msgdate) ;
#endif


/* create a message ID */

	if (g.localdomain != NULL) {

	    sprintf(buf, "<%d.%02d%03d%02d%02d%02d@%s.%s>",
	        g.pid,
	        timep->tm_year,timep->tm_yday,
	        timep->tm_hour,timep->tm_min,timep->tm_sec,
	        g.nodename,g.localdomain) ;

	} else {

	    sprintf(buf, "<%d.%02d%03d%02d%02d%02d@%s>",
	        g.pid,
	        timep->tm_year,timep->tm_yday,
	        timep->tm_hour,timep->tm_min,timep->tm_sec,
	        g.nodename) ;

	}

	g.messageid = mallocstr(buf) ;

	strcpy(mess_id,g.messageid) ;

/* try to determine the user's favorite editor program */

	if (((cp = getenv("ED")) != NULL) && (*cp != '\0'))
	    g.prog_editor = cp ;

	else if (((cp = getenv("EDITOR")) != NULL) && (*cp != '\0'))
	    g.prog_editor = cp ;

	else {

	    if (g.f.sysv_rt)
	        g.prog_editor = "/usr/bin/ed" ;

	    else
	        g.prog_editor = "ed" ;

	}

	logfile_printf(&g.lh,"editor=\"%s\"\n",g.prog_editor) ;


/* continue with the rest of the program */

	iswait = FALSE ;
	    isfile = FALSE ;		/* CAUTION: MUST be initialized here */
	isedit = TRUE ;
	isforward = FALSE ;
	iseforward = FALSE ;
	isappend = FALSE ;
	if (ex_mode == EM_PCSMAIL) {

	    if (namelist[0] == '\0') {

	        bprintf(g.efp,
	            "%s: warning - translation table not defined\n",
	            g.progname) ;

	        logfile_printf(&g.lh,"translation table not defined\n") ;

	    } /* end if */

/* get options from environment */

#if	CF_DEBUG
	    if (g.f.debug) {

	        for (i = 0 ; i < argc ; i += 1)
	            debugprintf("main: arg%d> %s\n",i,argv[i]) ;

	    }
#endif

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: about to call first 'getoptions'\n") ;
#endif

	    if (getoptions( mailopts,NULL) < 0) goto badret ;

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: called first getoptions\n") ;
#endif

#if	CF_MAILER
		g.prog_sendmail = "mailer" ;
#endif

	} /* end if (we are in regular mode) */

#if	CF_DEBUG
	if (g.f.debug) {

	    for (i = 0 ; i < argc ; i += 1)
	        debugprintf("main: arg%d> %s\n",i,argv[i]) ;

	}
#endif

	if (ex_mode == EM_PC) {

	    getpcheader( argv ) ;

	} else {

/* get options from the command line */

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: about to call second 'getoptions'\n") ;
#endif

	    if (getoptions(NULL, argv ) < 0) goto badret ;

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: called second 'getoptions'\n") ;
#endif

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: after 2nd 'getoptions' isfile=%d\n",
	            isfile) ;
#endif

#if	CF_DEBUG
	    if (g.f.debug) {

	        for (i = 0 ; i < argc ; i += 1)
	            debugprintf("main: arg%d> %s\n",i,argv[i]) ;

	    }
#endif

	}

/* still not beyond the point of no return */

	if (g.f.version) goto version ;

	if (g.f.exit) goto goodret ;

/* first order of business */

	if (g.f.nosend) {

	    logfile_printf(&g.lh,"running in NOSEND mode\n") ;

	    bprintf(g.efp,
	        "%s: special note -- running in NOSEND mode\n",
	        g.progname) ;

	}

/* if invalid options were found, punt this execution!		*/

	if (! g.f.interactive) isedit = 0 ;

	if (runcmd == NO) {

	    rmtemp(TRUE,"main 1") ;

	    goto badret ;
	}

/* clean up recipient list */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: recipient so far:\n%s\n",recipient) ;
#endif

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: isfile=%d\n",isfile) ;
#endif

	tonames = getnames(recipient) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: tonames %d \n", tonames) ;
#endif

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: isfile=%d\n",isfile) ;
#endif

	ccnames = getnames(copyto) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: isfile=%d\n",isfile) ;
#endif

	bccnames = getnames(bcopyto) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: isfile=%d\n",isfile) ;
#endif

/* generate name for temporary message file */

	strcpy(tempfile,"/tmp/smailXXXXXX") ;   /* unique gensym name */

	mktemp(tempfile) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to create temporary message file\n") ;
#endif

/* create file for temporarily putting the message in */

	if ((rs = bopen(mfp,tempfile,"wct",0600)) < 0) {

	    bprintf(g.efp,
	        "%s: can't open necessary temporary file - system problem\n",
	        g.progname) ;

	    logfile_printf(&g.lh,"couldn't open TMP file \"%s\"\n",tempfile) ;

	    rmtemp(TRUE,"main 2") ;

	    goto badret ;
	}

#ifdef	COMMENT

/* set group ownership */
/* to mailgroup, so that mail program can read it		*/
/* set protection to rw-r----- so that	*/
/* the owner can read/write and mail can only read		*/

	chown(tempfile, g.uid,g.gid_mail) ;

	chmod(tempfile,0640) ;

#endif

/* remove tempfile on interrupt */

	if (ex_mode != EM_PC)	/* PAS-JM 2/14/85 */
	    setsig(SIGINT,i_rmtemp) ;

/* miscellaneous variables */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: misc variables isfile=%d\n",isfile) ;
#endif


/* does the user have a NAME defined ? */

	g.name = g.gecosname ;
	if ((cp = getenv("NAME")) != NULL) {

	    if (mkmailname(buf,BUFSIZE,cp,-1) >= 0)
	        g.name = mallocstr(buf) ;

	}


/* does the user have a FULLNAME defined ? */

	if (f_fullname)
	    g.fullname = getenv("FULLNAME") ;

	else 
	    g.fullname = NULL ;


#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to go and get names ? isfile=%d\n",
	        isfile) ;
#endif

/* go */

	if (isfile) {

#if	CF_DEBUG
	    if (g.f.debug) {

	        debugprintf("main: 'isfile' is ON \"%s\"\n",filename) ;

	        fileinfo(filename,"main got a ISFILE") ;

	    }
#endif

	    if (access(filename,R_OK) < 0) {

	        logfile_printf(&g.eh,"main: could not access FILENAME \"%s\"\n",
	            filename) ;

	    }

	    getheader(filename,g.mailname) ;

	    tonames = getnames(recipient) ;

	    bccnames = getnames(bcopyto) ;

	    ccnames = getnames(copyto) ;

/* prompt 5/2/85 (JM)		*/

/* if we have a return path, then we do not include the rest of the file */

#ifdef	COMMENT
	    if (isret) {

	        isfile = 0 ;	/* added to force reply message */
	        logfile_printf(&g.eh,"main: turned off 'isfile'\n") ;

	    }
#endif

	} /* end if (we have a file) */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to get the message contents\n") ;
#endif

/* the message itself */

/* if we already have recipients then the following will not ask ? */

	if (ex_mode == EM_PCSMAIL) {

	    prompt(ALL) ;

	}

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to put the header\n") ;
#endif

/* write the headers that we have so far to the message file */

	writeheader(mfp,TRUE) ;

	bclose(mfp) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to execute non-PC code\n") ;
#endif

/* now we fork so that we can process the inputting of the message */

	fp = NULL ;
	pid_child = 0 ;
	if (ex_mode != EM_PC) {

/*
 This process collects the message from the terminal and	 
 stuffs it into tempfile while the main branch continues on 
 and reads in the humongously large translation table, which 
 takes anywhere from 4 to 8 seconds to read in! Then it waits
 for this process to terminate, and takes appropriate action 
 based on the termination status of this process. If this    
 process terminates normally (exit status 0) then processing 
 the message continues as usual. If terminal status is non-zero 
 then the main branch exits. If wait returns error then the  
 main branch saves the message in $HOME/dead.letter and exits 
*/

	    bflush(g.efp) ;

	    bflush(g.ofp) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to execute the fork code\n") ;
#endif

	pid_child = -1 ;
	    if ((! g.f.interactive) || iswait || 
		((pid_child = uc_fork()) <= 0)) {

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("main: forked pid=%d\n",pid_child) ;
#endif

/* if we forked, the child is executing the following code */

	        if ((rs = bopen(mfp,tempfile,"wca",0600)) < 0) {

	            bprintf(g.efp,"%s: could not open TMP file (rs %d)\n",
	                g.progname,rs) ;

	            logfile_printf(&g.lh,"could not open TMP file (rs %d)\n",
	                g.progname,rs) ;

	            rmtemp(TRUE,"main 3") ;

	            goto badret ;
	        }

	        bseek(mfp,0L,SEEK_END) ;

#if	CF_DEBUG
	            if (g.f.debug) {

			bflush(mfp) ;

	                fileinfo(tempfile,"after seek end") ;

			}
#endif

	        if (ex_mode == EM_PCSMAIL) {

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("main: interactive program mode \n") ;
#endif

/* body of the message */

	            if (*message != '\0') {

#if	CF_DEBUG
	                if (g.f.debug)
	                    debugprintf("main: we have a message ? \"%s\"\n",
	                        message) ;
#endif

/* we were given the message on the command invocation line */

	                bprintf(mfp,"\n%s\n",message) ;

	            } else if (isfile) {

#if	CF_DEBUG
	                if (g.f.debug) {

	                    debugprintf("main: we have a file already \"%s\"\n",
	                        filename) ;

	                    fileinfo(filename,"main filename") ;

	                    fileinfo(tempfile,"before add on") ;

	                }
#endif

/* we were not given the message body on the command invocation line */

	                if ((rs = bopen(tfp,filename,"r",0600)) < 0) {

#if	CF_DEBUG
	                if (g.f.debug)
	                    	debugprintf("main: cannot open \"%s\" rs %d\n",
				filename,rs) ;
#endif

	                    bprintf(g.efp,
	                        "%s: ERROR cannot open \"%s\"\n",
				filename) ;

	                    logfile_printf(&g.eh,
	                        "cannot read file \"%s\" rs=%d\n",
	                        filename,rs) ;

	                    rmtemp(TRUE,"main 4") ;

	                    goto badret ;
	                }

/* copy the file to the mail file */

	                rs = 0 ;

/* skip any headers that may be in the file */

#ifdef	COMMENT
	                len = 0 ;
	                while ((l = breadline(tfp,linebuf,LINELEN)) > 0) {

				len += l ;
	                    if (linebuf[0] == '\n') break ;

			}
#else
	                if ((l = skipheaders(tfp,linebuf,LINELEN)) > 0)
				bwrite(mfp,linebuf,l) ;
#endif

#if	CF_DEBUG
	                if (g.f.debug) {

				offset = bseek(tfp,0L,SEEK_CUR) ;

	                    debugprintf("main: skipped headers, off=%ld\n",
				offset) ;

	                    fileinfo(tempfile,"after skip headers") ;

			}
#endif

/* copy the body of the message that was given us in the input file */

#if	CF_DEBUG
	if (g.f.debug) {

		bflush(mfp) ;

	                    fileinfo(tempfile,"before copyblock") ;

			bflush(tfp) ;

	                    fileinfo(filename,"before copyblock") ;

	}
#endif

	                while ((rs = bcopyblock(tfp,mfp,BUFLEN)) > 0)
	                    len += rs ;

#if	CF_DEBUG
	if (g.f.debug) {

			bflush(tfp) ;

			bflush(mfp) ;

	                    fileinfo(tempfile,"after copyblock") ;

	                    fileinfo(filename,"after copyblock") ;

	}
#endif

	                bclose(tfp) ;

#if	CF_DEBUG
	                if (g.f.debug)
	                    debugprintf("main: %d MSG body bytes copied\n",len) ;
#endif

	                if (rs < 0) {

	                    logfile_printf(&g.lh,
	                        "possible full filesystem (rs %d)\n",
	                        rs) ;

	                    bprintf(g.efp,
	                        "%s: possible full filesystem (rs %d)\n",
	                        g.progname,rs) ;

	                }

	            } /* end if (have a file given) */

	        } /* end if (regular mode) */

	        bflush(mfp) ;

#if	CF_DEBUG
	            if (g.f.debug)
	                fileinfo(tempfile,"before isedit") ;
#endif

	        if (isedit) {

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("main: we are working on an edit\n") ;
#endif

	            bclose(mfp) ;

	            if (iseforward) {

#if	CF_DEBUG
	                if (g.f.debug) {

	                    debugprintf("main: we are working on an eforward\n") ;

	                    fileinfo(eforwfile,"eforward") ;

	                }
#endif

	                fappend(eforwfile,tempfile) ;

	                unlink(eforwfile) ;

	                iseforward = 0 ;

	            } /* end if (we have a forward file) */

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("main: about to edit it\n") ;
#endif

#if	CF_DEBUG
	            if (g.f.debug)
	                fileinfo(tempfile,"main before editit") ;
#endif

	            editit(tempfile) ;

#if	DEBUG && 0
	            sprintf(buf,"cp %s /home/dam/rje/edit2.out",tempfile) ;
	            system(buf) ;
#endif

#if	CF_DEBUG
	            if (g.f.debug)
	                fileinfo(tempfile,"main after editit") ;
#endif

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("main: edited it\n") ;
#endif

	        } else if ((! isfile) && (*message == '\0')) {

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("main: not file and not messages\n") ;
#endif

	            if (g.f.interactive) bprintf(g.ofp,
	                "enter message - terminate by period or EOF:\n") ;

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf(
	                    "main: about to read the message that we got ??\n") ;
#endif

	            while (((len = breadline(g.ifp,linebuf,LINELEN)) > 0) && 
	                (strncmp(linebuf,".\n",2) != 0)) {

	                linebuf[len] = '\0' ;
	                if (g.f.interactive && (strcmp(linebuf,"?\n") == 0)) {

	                    help(4) ;

	                    continue ;
	                }

	                bwrite(mfp,linebuf,len) ;

	            } /* end while */

	            bclose(mfp) ;

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("main: read the message that we collected\n") ;
#endif

	        } else {

	            bclose(mfp) ;

	        } /* end if (gathering or editing the file) */

	        fp = NULL ;

/* exit only if fork did happen, otherwise just continue */

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: child about to possibly exit\n") ;
#endif

	        if (g.f.interactive && (! iswait) && (pid_child != -1))
	            exit(0) ;

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: child DID NOT exit\n") ;
#endif

	    } /* end if (of forked off code) */

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: end of forked off code \n") ;
#endif

/* while the other process is collecting input from the terminal */
/* the main process ignores any interrupts and continues with its*/
/* housekeeping chores, i.e. read translation table from the disk*/

	    if (pid_child != 0) {

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: about to do this 'setsig' thing\n") ;
#endif

	        setsig(SIGINT,SIG_IGN) ;

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: did the 'setsig' thing\n") ;
#endif

/*	f_isedit = 0 ; removed to fix the dit bug 7/5/85 (JM)	*/

	    } /* end if (parent) */

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf(
	            "main: about to exit the 'not PC' if\n") ;
#endif

	} /* end if (not running as PCS PostCard) */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to read in translation table\n") ;
#endif

/* get last->myname conversion table */

	gettable(g.nodename) ;

	if ((tablelen == 0) && (namelist[0] != '\0'))
	    bprintf(g.efp,
	        "%s: warning - no entries in the translation table\n",
	        g.progname) ;


#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: we are about to attempt joining\n") ;
#endif

/* This is where the join happens 				*/
/* the join needs to be done only if the forking actually	*/
/* happened. pid_child has a positive non-zero value only if the	*/
/* forking actually happened					*/

	if (pid_child > 0) {

	    while ((wstatus = u_waitpid(pid_child,&status,0)) >= 0) {

	        if (wstatus == pid_child) break ;

	    }

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf(
	            "main: we're in the join code w/ wstatus=%02X status=%d\n",
	            wstatus,status) ;
#endif

	    if (wstatus == -1) {

	        rmtemp(TRUE,"main 5") ;

	        goto badret ;
	    }

	    if (status != 0) goto goodret ;

	    setsig(SIGINT,i_rmtemp) ;

	} /* end if */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf( "main: we joined \n") ;
#endif

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf( "main: about to get names again\n") ;
#endif

	if (pid_child > 0) {

/* scan message for changes */

	    if (isedit != 0) {

	        *subject = *recipient = '\0';			/* (11/30) */
	        *copyto  = *bcopyto = '\0' ;

	        isedit = 0 ;	/* MR# gw85-00801 2/7/85 (JM)*/

	    }

	    getheader(tempfile,syscom) ;	/* (11/30) */

	    if (isedit != 0) {

	        getfield(tempfile,HS_CC,copyto) ;

	        getfield(tempfile,HS_BCC,bcopyto) ;

	    }

	    tonames = getnames(recipient) ;

	    ccnames = getnames(copyto) ;

	    bccnames = getnames(bcopyto) ;

	    isedit = 0 ;

	} /* end if (getting name ?) */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf(
	        "main: end of getting names again\n") ;
#endif

/* convert user name (in 'username') to real name and put in variable 'from' */

#ifdef	COMMENT
	myname = getname(g.username,buf) ;

	if (myname == 0) g.name = mallocstr(buf) ;
#endif


#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to create the 'from' address\n") ;
#endif

/* create the 'fromaddr' header field */

	strcpy(fromaddr,g.mailnode) ;

	strcat(fromaddr,"!") ;

	strcat(fromaddr,g.username) ;


#ifndef	COMMENT

	namep = NULL ;
	if (f_name) {

	    if (g.mailname != NULL)
	        namep = g.mailname ;

	    else if (g.gecosname != NULL)
	        namep = g.gecosname ;

	    else if (g.name != NULL)
	        namep = g.name ;

	    else if (f_fullname && (g.fullname != NULL))
	        namep = g.fullname ;

	}

	if (namep != NULL) {

	    if (strchr(namep,' ') != NULL) {

	        if (f_internet)  {
		    sprintf(from, "%s@%s (%s)", 
	            g.username,MAILNODE,namep) ;

	        } else 
		     sprintf(from, "%s!%s (%s)", 
	            g.mailnode,g.username,namep) ;

	    } else {

	        if (f_internet) {
		    sprintf(from, "%s <%s@%s>", 
	            namep, g.username, MAILNODE) ;

	        } else 
		    sprintf(from, "%s <%s!%s>", 
	            namep, g.mailnode,g.username) ;

	    }

	} else {

	    if (f_internet) {
		sprintf(from, "%s@%s", 
	        g.username, MAILNODE) ;

	    } else 
		sprintf(from, "%s!%s", 
	        g.mailnode,g.username) ;

	}
#else
	if (*from == '\0') {

	    if (myname < 0) 
		sprintf(from,"%s!%s",
	        g.nodename,g.username) ;

/* setup local or internet from field */

	    if ((tonames != 0) && 
	        (strpbrk(recipient,"@_") != NULL) || f_internet)

/* this is going internet */

	        if (myname >= 0) {

	            strcpy(syscom,from) ;

	            sprintf(from,"%s@%s.%s (%s)",
	                g.username,g.mailnode,g.companydomain,syscom) ;

	        }
	        else {

	            sprintf(from,"%s@%s.%s",
	                g.username,g.mailnode,g.companydomain) ;

	        }
	}
#endif

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to do some real work again\n") ;
#endif

	if (ex_mode != EM_PCSMAIL) {

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: execution mode not PCSMAIL\n") ;
#endif

	    if (ex_mode == EM_PC) redo_message() ;

	    getheader(tempfile,from) ;

	    ccnames = bccnames = 0 ;

	} else {

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: execution mode is PCSMAIL\n") ;
#endif

	    if (iseforward) {

	        fappend(eforwfile,tempfile) ;

	        unlink(eforwfile) ;

	    }

/* check TO:, CC:, BCC: list */

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: about to get header fields\n") ;
#endif

	    getfield(tempfile,HS_CC,copyto) ;

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: and the BCC\n") ;
#endif

	    getfield(tempfile,HS_BCC,bcopyto) ;

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: about to 'getnames(copyto)'\n") ;
#endif

	    if (*copyto != '\0') ccnames = getnames(copyto) ;

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: about to 'getnames(bcopyto)'\n") ;
#endif

	    if (*bcopyto != '\0') bccnames = getnames(bcopyto) ;

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: about to 'checkreclist()'\n") ;
#endif

	    checkreclist(0) ;

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: about to 'redo_message()'\n") ;
#endif

#if	CF_DEBUG
	    if (g.f.debug)
	        fileinfo(tempfile,
	            "main: about to 'redo_message()'") ;
#endif

	    redo_message() ;

#if	CF_DEBUG
	    if (g.f.debug)
	        fileinfo(tempfile,
	            "main: after 'redo_message()'") ;
#endif

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: about to ask for sending command\n") ;
#endif

/* ask for sending command */
opts:
	    if (ambiguous) redo_message() ;

	    if (! g.f.interactive) {

	        strcpy(command,"send") ;

	        isedit = 0 ;

	    } else {

	        if (*recipient == '\0') {

	            bprintf(g.ofp,"\nno recipient(s) specified\n") ;

	            tonames = 0 ;
	            prompt(TO) ;

	            strcpy(realto, recipient) ;

	            checkreclist(0) ;

	            redo_message() ;

	        }

	        if (error != 0) {

	            bprintf(g.ofp,"%s%s","\nthe recipient lists contains",
	                " unknown names\n") ;

	            bprintf(g.ofp,"%s%s","do you really want to send",
	                " the message ? [no] ") ;

	            if (fgetline(stdin,s,BUFSIZE) <= 0) {

	                rmtemp(TRUE,"main 6") ;

	                goto badret ;
	            }

	            if (*s != 'y') {

	                bprintf(g.ofp,"%s%s","OK, please edit",
	                    " the message or quit\n") ;

	            }
	        }

	        if (isedit != 0) strcpy(command,"edit") ;

	        else prompt(SENDCOM) ;

	    } /* end if */

	    cp = command ;
	    while (ISWHITE(*cp)) cp += 1 ;

	    option = strtok(cp,",: \t") ;

/* top of parsing the command */
opts1:
	    if ((option == NULL) || (! g.f.interactive))
	        isdef = 1 ;

	    else {

/* send with current list of names */

	        f_send = FALSE ;
	        switch ((int) *option) {

	        case 's':
	            standard = 1 ;
	            copy = 0 ;
	            verify = 0 ;
	            f_send = TRUE ;
	            break ;

/* review the message */
	        case 'r':
	            reviewit(option) ;

#if	CF_DEBUG
	            if (g.f.debug)
	                fileinfo(tempfile,"main reviewit") ;
#endif

	            break ;

/* check recipient list */
	        case 'c':

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("main: about to call 'checkit()'\n") ;
#endif

	            checkit() ;

	            break ;

/* edit the message */
	        case 'e':
	            editit(tempfile) ;

#if	CF_DEBUG
	            if (g.f.debug)
	                fileinfo(tempfile,"main CMD editit") ;
#endif

	            checkreclist(0) ;

	            redo_message() ;

	            ambiguous = 0 ;
	            break ;

/* terminate the mail */
	        case 'q':

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("main: quitting program\n") ;
#endif

	            if (fork() == 0) {

	                close(0) ;

	                close(1) ;

	                close(2) ;

	                if (fork() == 0) {

	                    sleep(100) ;

	                    unlink(tempfile) ;

	                }

	            }

	            goto goodret ;

/* help user */
	        case '?':
	            help(0) ;

	            break ;

/* unknown. try again. */
	        default:
	            bprintf(g.ofp,"unknown command -- please try again") ;

	        } /* end switch */

	        if (! f_send) goto opts ;

	    } /* end if */

/* check for send options */
opts2:

/* check for default options */
/* MR# gx84-33465 JM 2/7/85 *//*begin*/

	    if (isdef != 1) {

	        standard = 1 ;
	        copy = verify = 0 ;
	    }

/* MR# gx84-33465 JM 2/7/85 *//*end*/

	    if ((! g.f.interactive) || isdef) option = NULL ;

	    else if ((option = strtok(NULL,", \t")) == NULL) {

	        prompt(SENDOPT) ;

	        option = strtok(sendopt,"+, \t") ;

	    }

	    while (option != NULL) {

/* set flags for options */
	        switch (*option) {

/* acknowledge delivery of message */
	        case 'd':
	            break ;

/* add sender to list of recipients */
	        case 'f':
	            copy = TRUE ;
	            break ;

/* notify recipient of incomming mail */
	        case 'n':
	            break ;

/* verify that the message is sent */
	        case 'v':
	            verify = TRUE ;
	            break ;

/* standard mail */
	        case 's':
	            standard = TRUE ;
	            verify = FALSE ;
	            break ;

/* if check, edit, review, go back */
	        case 'c':
	        case 'e':
	        case 'r':
	            goto opts1; /* break; */

/* allow quit here too */
	        case 'q':
	            rmtemp(FALSE,"main 7") ;

	            goto goodret ;

/* fall through to next case */
	        case '?':
	            help(1) ;

	            strcpy(sendopt,"send") ;

	            goto opts2; /* break; */

/* unknown ; try again */
	        default:
	            bprintf(g.efp,"%s%s","unknown send option",
	                " -- please try again.\n") ;

	            strcpy(sendopt,"send") ;

	            goto opts2 ;	/* break; */
	        }

	        option = strtok(0,":+, 	") ;

	    } /* end while */

	    if (! g.f.interactive) {

	        getheader(tempfile,from) ;

	        if (*recipient != '\0') tonames = getnames(recipient) ;

	        if (*copyto != '\0') ccnames = getnames(copyto) ;

	        if (*bcopyto != '\0') bccnames = getnames(bcopyto) ;

	    }

	} /* end if (execution mode PCSMAIL or not) */

/* ignore signals */

	setsig(SIGINT,SIG_IGN) ;

/* 
	Fork off code to issue unix 'mail' commands and 
	  then do own notification (since Unix doesn't always),
	  once separately for each recipient (to avoid UNIX mail 'to' line).
	  Do the actual forking only if the user has not specified the '+wait'
	  option.  If 'ex_mode' is EM_PC, forking doesn't happen because
	  'iswait' is set to 1 in these modes.
*/

#ifdef COMMENT
	fclose(errlog) ;
#endif

/* fork off to UNIX top-level */

	if ((tonames > 0) && 
	    (iswait || ((pid_child = fork()) == 0) || (pid_child == -1))) {

#if	CF_DEBUG
	    if (g.f.debug)
	        fileinfo(tempfile,"main 1") ;
#endif

	    deliverem() ;

/* if we forked, the child exits here */

	    rmtemp(FALSE,"main 8") ;

	    goto goodret ;

	} /* end of forked off code */

/* no one to send to Sighhh...! */

	if (tonames == 0) {

	    if (access(tempfile,R_OK) < 0) {

	        debugprintf("main: no tempfile here\n") ;

	        sprintf(syscom,"cp /dev/null %s",tempfile) ;

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: SYSTEM> %s\n",syscom) ;
#endif

	        system(syscom) ;

	    }

	    sprintf(syscom, "cat %s >> %s/%s\n", tempfile,
	        g.homedir, DEADFILE) ;

	    system(syscom) ;

	    unlink(tempfile) ;

	    if (isforward > 0) unlink(forwfile) ;

	    if (g.f.interactive) {

	        bprintf(g.ofp,"\nno recipients were specified\n") ;

	        bprintf(g.ofp,"mail saved in file '%s'\n",DEADFILE) ;

	    }

	    rmtemp(TRUE,"main 9") ;

	    goto badret ;
	}

	if (g.f.interactive) {

	    bprintf(g.ofp,"sending mail ") ;

	    bputc(g.ofp,C_LPAREN) ;

	    if (standard) bprintf(g.ofp,"standard") ;

	    if (verify) bprintf(g.ofp," verify") ;

	    if (copy) bprintf(g.ofp," filecopy") ;

	    if (f_notify) bprintf(g.ofp," notify") ;

	    bprintf(g.ofp,"%c\n",C_RPAREN) ;

	}

done:
goodret:
#ifdef	COMMENT
	if (tempfile[0] != '\0') unlink(tempfile) ;
#endif

	bclose(g.ofp) ;

	bclose(g.efp) ;

	return OK ;

badret:
#ifdef	COMMENT
	if (tempfile[0] != '\0') unlink(tempfile) ;
#endif

	bclose(g.ofp) ;

	bclose(g.efp) ;

	return BAD ;

badinopen:
	bprintf(g.efp,"%s: could not open the input (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badoutopen:
	bprintf(g.efp,"%s: could not open the output (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

version:
	bprintf(g.efp,"%s: version %s/%s\n",
	    g.progname,
	    VERSION,(g.f.sysv_ct ? "SYSV" : "BSD")) ;

	goto goodret ;
}
/* end subroutine (main) */


void i_rmtemp(in)
int	in ;
{


	rmtemp(TRUE,"interrupt") ;

}



