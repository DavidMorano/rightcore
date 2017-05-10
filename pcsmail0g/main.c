/* main */


#define	DEBUG_MAIN	1
#define	DEBUG_NOSEND	1


/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
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



	Yanked out the network routing related stuff and 
	got it to work purely as a User Agent for use in Sun 
	networks on top of the 'sendmail(8)' daemon program.
	- Jishnu Mukerji 11/13/89
*
*
************************************************************************/



#include	<sys/utsname.h>
#include	<sys/types.h>
#include	<grp.h>
#include	<string.h>
#include	<signal.h>
#include	<time.h>
#include	<pwd.h>
#include	<stdio.h>

#include	<baops.h>
#include	<bfile.h>

#include	"config.h"
#include	"smail.h"
#include	"prompt.h"
#include	"header.h"
#include	"localmisc.h"



/* external subroutines */

extern void	rmtemp() ;			/* PAS-JM 2/14/85 */

extern int	mkgecosname(char *,int,const char *) ;
extern int	mkmailname(char *,int,const char *,int) ;

extern char	*getenv() ;
extern char	*getlogin() ;
extern char	*strbasename() ;
extern char	*putheap() ;


/* external data */

extern struct global	g ;



/* global data */

struct utsname	uts ;

time_t		daytime ;



int main(argc,argv)
int argc ;
char *argv[] ;
{
	FILE	*fw ;	/* pointer to dev/tty** */

	bfile	tfile ;
	bfile	*fpa[3] ;

	struct tm	*timep ;

	struct passwd	*pp ;

	struct group	*gp ;

	struct address	*as_to = NULL ;
	struct address	*as_from = NULL ;
	struct address	*as_sender = NULL ;
	struct address	*as_replyto = NULL ;
	struct address	*as_cc = NULL ;
	struct address	*as_bcc = NULL ;

	int		rs, status ;
	int		wstatus ;
	int		i, l ;
	int		f_send ;
	int		cpid ;

	char		*option, *tmp ;
	char		buf[BUFSIZE + 1], *bp ;
	char		*namep, *cp ;
	char		**ptr ;
	char		*realname ;


	g.progname = "PCSMAIL" ;
	if ((argc >= 1) && ((l = strlen(argv[0])) > 0)) {

	    while (argv[0][l - 1] == '/') l -= 1 ;

	    argv[0][l] = '\0' ;
	    g.progname = strbasename(argv[0]) ;

	    strcpy(buf,g.progname) ;

	    bp = buf ;
	    if (*bp == 'n') bp += 1 ;

	}

	g.pid = getpid() ;


/* get today's date and current time jm*/

	getdate(datetime) ;		/*jm*/

	g.datetime = datetime ;


/* default option is standard */

	for (i = 0 ; i < NUO ; i += 1) g.uo[i] = g.uom[i] = 0 ;

#if	DEBUG_MAIN
	BASET(g.uo,UOV_DEBUG) ;
#endif

#if	DEBUG_NOSEND
	BASET(g.uo,UOV_NOSEND) ;
#endif

	standard = 1 ;
	verbose = 0 ;

	f_name = TRUE ;
	f_fullname = FALSE ;
	f_internet = f_version = FALSE ;

/* set some defaults */

	g.domain = DOMAIN ;
	g.mailhost = MAILHOST ;


/* initialize strings to ""*/

	*mess_id = *from = *sentby = *fromaddr = *reference = '\0' ;
	*keys = *subject = *moptions = *copyto = *bcopyto = *appfile = '\0' ;
	*received = *forwfile = *eforwfile = *retpath = *message = '\0' ;

/* get home directory */

	strcpy(homedir,getenv("HOME")) ;


/* get UNIX system name */

	uname(&uts) ;

	g.nodename = uts.nodename ;
	if ((cp = strchr(uts.nodename,'.')) != NULL) {

	    *cp++ = '\0' ;
	    g.domain = cp ;
	}


/* save command name by which we have been invoked */

	strcpy(syscom,argv[0]) ;

	if ((realname = strrchr(syscom,'/')) == NULL)
	    realname = syscom ;

	if (*realname == '/') realname++ ;

	strcpy(comm_name, realname) ;


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

#if	DEBUG_MAIN
	errprintf("we are %s\n",g.progname) ;
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
	        if (g.uid == pp->pw_uid) g.username = putheap(cp) ;

	}

	if (g.username == NULL) {

	    if ((pp = getpwuid(g.uid)) != NULL)
	        g.username = putheap(pp->pw_name) ;

	    else
	        g.username = "GUEST_UID" ;

	}

/* get the GECOS name field (leave in variable 'cp') if we can also */

	if (pp != NULL) {

	    g.gid = pp->pw_gid ;
	    g.homedir = putheap(pp->pw_dir) ;

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

	    if (fixgecos(cp,buf,BUFSIZE) >= 0)
	        g.gecosname = putheap(buf) ;

	}

/* can we make some sort of perferred 'mail' name ? */

	g.mailname = g.gecosname ;
	if (g.gecosname != NULL) {

	    if (mkmailname(buf,BUFSIZE,g.gecosname,-1) >= 0)
	        g.mailname = putheap(buf) ;

	}


/* get the 'mail' group ID */

	g.gid_mail = 1 ;
	if ((gp = getgrnam("mail")) != NULL)
	    g.gid_mail = gp->gr_gid ;


/* get the time and the broken out time structure */

	time(&daytime) ;

	timep = localtime(&daytime) ;


/* The following section is used only for debugging purposes */
/* if LOGFILE is defined then open it */

	sprintf(buf,"%s%d",g.nodename,g.pid) ;

	g.logid = putheap(buf) ;

#ifdef LOGFILE
	logfile_open(&g.lh,LOGFILE,0,0666,g.logid) ;

	buf[0] = '\0' ;
	if (g.mailname != NULL) sprintf(buf,"(%s)",g.mailname) ;

	logfile_printf(&g.lh,
		"%02d%02d%02d %02d%02d:%02d %-14s %s %s!%s%s\n",
	    timep->tm_year,
	    timep->tm_mon + 1,
	    timep->tm_mday,
	    timep->tm_hour,
	    timep->tm_min,
	    timep->tm_sec,
	    g.progname,VERSION,g.nodename,g.username,buf) ;

#endif

	erropen(ERRFILE,g.logid) ;

/* OK, do some cleanup work */

	logfile_flush(&g.lh) ;

	if ((cpid = fork()) <= 0) {

	    fpa[0] = NULL ;
	    fpa[1] = NULL ;
	    fpa[2] = NULL ;
	    if ((rs = bopencmd(fpa,
	        "/usr/add-on/pcs/bin/pcscleanup > /dev/null 2>&1")) < 0) {

	        logfile_printf(&g.lh,
			"could not run the cleanup code (rs %d)\n",rs) ;

	    }

	    if (cpid == 0) {
		logfile_close(&g.lh) ;
		exit(0) ;
	    }

	} /* end if (cleanup code) */

	logfile_close(&g.lh) ;

/* get environment variables */

	getvars() ;

/* handle some cases right now! */

	if (ex_mode == EM_PCSINFO) {

	    doinfo(argc, argv) ;

	    return OK ;
	}


/* create a message ID */

	strcpy(stddatetime, asctime(timep)) ;

	*strchr(stddatetime, '\n') = '\0' ;

#ifdef	MESS_ID_FIRST 
	if (g.domain != NULL) {
	    sprintf(buf,"%s.%s",g.nodename,g.domain) ;

	} else
	    strcpy(buf,g.nodename) ;

	sprintf( mess_id, "<%d%d%d@%s>", 
	    timep->tm_hour,timep->tm_yday,g.pid, buf) ;
#endif

#ifdef	MESS_ID_LAST
	if (g.domain != NULL) {
	    sprintf(buf,"%s.%s",g.nodename,g.domain) ;

	} else
	    strcpy(buf,g.nodename) ;

	sprintf( mess_id, "<%d%d%d@%s>", 
	    timep->tm_hour,timep->tm_yday,g.pid, buf) ;
#endif

	if (ex_mode == EM_PCSMAIL) {

	    if (namelist[0] == '\0')
	        fprintf(stdout,
	            "%s: warning - translation table not defined\n",
	            g.progname) ;

/* get options from environment */

	    getoptions( mailopts, 0 ) ;

	}

	if (ex_mode == EM_PC) {

	    getpcheader( argv ) ;

	} else {

/* get options from the command line */

	    getoptions( 0 , argv ) ;

	}

/* if invalid options were found, punt this execution!		*/

	if (runcmd == NO) rmtemp(1) ;

/* clean up recipient list */

#if	DEBUG_MAIN
	errprintf("recipient so far :\n%s\n",recipient) ;
#endif

	tonames = getnames(recipient) ;

#if	DEBUG_MAIN
	if (BATST(g.uo,UOV_DEBUG))
	    errprintf("tonames %d \n", tonames) ;
#endif

	ccnames = getnames(copyto) ;

	bccnames = getnames(bcopyto) ;

/* generate name for temporary message file */

	strcpy(tempfile,"/tmp/smailXXXXXX") ;   /* unique gensym name */

	mktemp(tempfile) ;

#if	DEBUG_MAIN
	if (BATST(g.uo,UOV_DEBUG))
	    errprintf("main: about to create temporary message file\n") ;
#endif

/* create file for temporarily putting the message in */

	if ((fp = fopen (tempfile, "w")) == NULL) {

	    fprintf(stderr,
	        "%s: can't open necessary temporary file - system problem\n",
	        g.progname) ;

	    rmtemp(1) ;
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
	    setsig(rmtemp) ;

/* miscellaneous variables */

	strcpy(s_sendmail,SENDMAIL) ;


/* does the user have a NAME defined ? */

	g.name = g.gecosname ;
	if ((cp = getenv("NAME")) != NULL) {

	    if (mkmailname(buf,BUFSIZE,cp,-1) >= 0)
	        g.name = putheap(buf) ;

	}


/* does the user have a FULLNAME defined ? */

	if (f_fullname) g.fullname = getenv("FULLNAME") ;

	else g.fullname = NULL ;


#if	DEBUG_MAIN
	if (BATST(g.uo,UOV_DEBUG))
	    errprintf("main: about to go and get names ??\n") ;
#endif

/* go */

	if (isfile) {

	    getheader(filename,syscom) ;

	    tonames = getnames(recipient) ;

	    bccnames = getnames(bcopyto) ;

	    ccnames = getnames(copyto) ;

/* prompt 5/2/85 (JM)		*/

	    if (isret) isfile = 0 ;	/* added to force reply message */

	}

#if	DEBUG_MAIN
	if (BATST(g.uo,UOV_DEBUG))
	    errprintf("main: about to get the message contents\n") ;
#endif

/* the message itself */

	if (ex_mode == EM_PCSMAIL) {

	    if (f_version)
	        fprintf(stderr,"%s: version %s\n",
	            g.progname,VERSION) ;

	    prompt(ALL) ;

	}

#if	DEBUG_MAIN
	if (BATST(g.uo,UOV_DEBUG))
	    errprintf("main: about to put the header\n") ;
#endif

	putheader(fp, 1) ;

	fclose(fp) ;        /* finish off message file */

#if	DEBUG_MAIN
	if (BATST(g.uo,UOV_DEBUG))
	    errprintf("main: about to execute the fork code\n") ;
#endif

	fp = 0 ;
	cpid = 0 ;
	if (ex_mode != EM_PC) {

/* This process collects the message from the terminal and	 */
/* stuffs it into tempfile while the main branch continues on  */
/* and reads in the humongously large translation table, which */
/* takes anywhere from 4 to 8 seconds to read in! Then it waits*/
/* for this process to terminate, and takes appropriate action */
/* based on the termination status of this process. If this    */
/* process terminates normally (exit status 0) then processing */
/* the message continues as usual. If terminal status is non-0 */
/* then the main branch exits. If wait returns error then the  */
/* main branch saves the message in $HOME/dead.letter and exits */

	    fflush(stdout) ;

	    fflush(stderr) ;

	    if ((! f_interactive) || iswait || 
	        ((cpid = fork()) == 0) || (cpid == -1)) {

/* if we forked, the child is executing the following code */

	        fp = fopen(tempfile,"a") ;

	        if (ex_mode == EM_PCSMAIL) {

/* body of the message */

	            if (*message != '\0')

	                fprintf(fp,"\n%s\n",message) ;

	            else if (isfile) {

	                if ((fw = fopen(filename,"r")) == NULL) {

	                    printf("error: cannot read %s\n",filename) ;

	                    rmtemp(1) ;

	                }

	                copymessage(fp,fw) ;

	                fflush(fp) ;

	                fclose(fw) ;

	            }
	        }

	        if (isedit != 0) {

	            if (iseforward) {

	                fappend(eforwfile,tempfile) ;

	                unlink(eforwfile) ;

	                iseforward = 0 ;
	            }

#if	DEBUG_MAIN
	            if (BATST(g.uo,UOV_DEBUG))
	                errprintf("main: about to edit it\n") ;
#endif

	            editit() ;

#if	DEBUG_MAIN
	            if (BATST(g.uo,UOV_DEBUG))
	                errprintf("main: edited it\n") ;
#endif

	        } else if ((! isfile) && (*message == '\0')) {

	            if (f_interactive) printf(
	                "enter message - terminate by period or EOF :\n") ;

#if	DEBUG_MAIN
	            if (BATST(g.uo,UOV_DEBUG)) errprintf(
	                "main: about to read the message that we got ??\n") ;
#endif

	            while ((fgetline(stdin,s, BUFSIZE) > 0) && 
	                (strcmp(s,".\n") != 0)) {

	                if (f_interactive && strcmp("?\n",s) == 0) {

	                    help(4) ;

	                    continue ;
	                }

	                fputs(s,fp) ;

	            } /* end while */

#if	DEBUG_MAIN
	            if (BATST(g.uo,UOV_DEBUG)) errprintf(
	                "main: read the message that we collected\n") ;
#endif

	        }
	        fclose (fp);        /* finish off message file */

	        fp = 0 ;

/* exit only if fork did happen, otherwise just continue */

#if	DEBUG_MAIN
	        if (BATST(g.uo,UOV_DEBUG))
	            errprintf("main: child about to possibly exit\n") ;
#endif

	        if (f_interactive && (! iswait) && (cpid != -1))
	            exit(0) ;

#if	DEBUG_MAIN
	        if (BATST(g.uo,UOV_DEBUG))
	            errprintf("main: child DID NOT exit\n") ;
#endif

	    } /* end if (of forked off code) */

#if	DEBUG_MAIN
	    if (BATST(g.uo,UOV_DEBUG))
	        errprintf("main: end of forked off code \n") ;
#endif

/* while the other process is collecting input from the terminal */
/* the main process ignores any interrupts and continues with its*/
/* housekeeping chores, i.e. read translation table from the disk*/

	    if (cpid != 0) {

#if	DEBUG_MAIN
	        if (BATST(g.uo,UOV_DEBUG))
	            errprintf("main: about to do this 'setsig' thing\n") ;
#endif

	        setsig(SIG_IGN) ;

#if	DEBUG_MAIN
	        if (BATST(g.uo,UOV_DEBUG))
	            errprintf("main: did the 'setsig' thing\n") ;
#endif

/*	f_isedit = 0 ; removed to fix the dit bug 7/5/85 (JM)	*/

	    } /* end if */

#if	DEBUG_MAIN
	    if (BATST(g.uo,UOV_DEBUG)) errprintf(
	        "main: about to exit the 'not PC' if\n") ;
#endif

	} /* end if (not running as PCS PostCard) */

#if	DEBUG_MAIN
	if (BATST(g.uo,UOV_DEBUG))
	    errprintf("main: about to read in translation table\n") ;

	sleep(2) ;
#endif

/* get last->myname conversion table */

	gettable(g.nodename) ;

	if (tablelen == 0 && namelist[0] != '\0') fprintf(stderr,
	    "%s: warning - no entries in the translation table\n",
	    g.progname) ;


#if	DEBUG_MAIN
	if (BATST(g.uo,UOV_DEBUG))
	    errprintf("main: we are about to attempt joining\n") ;
#endif

/* This is where the join happens 				*/
/* the join needs to be done only if the forking actually	*/
/* happened. cpid has a positive non-zero value only if the	*/
/* forking actually happened					*/

	if (cpid > 0) {

	    while ((wstatus = wait(&status)) != -1 && wstatus != cpid ) ;

#if	DEBUG_MAIN
	    if (BATST(g.uo,UOV_DEBUG)) errprintf(
	        "main: we're in the join code w/ wstatus=%02X status=%d\n",
	        wstatus,status) ;
#endif

	    if (wstatus == -1) rmtemp(1) ;

	    if (status != 0) exit(0) ;

	    setsig(rmtemp) ;

	} /* end if */

#if	DEBUG_MAIN
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	    "main: we joined \n") ;
#endif

#if	DEBUG_MAIN
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	    "main: about to get names again\n") ;
#endif

	if (cpid > 0) {

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

#if	DEBUG_MAIN
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	    "main: end of getting names again\n") ;
#endif

/* convert user name (in 'username') to real name and put in variable 'from' */

#ifdef	COMMENT
	myname = getname(g.username,buf) ;

	if (myname == 0) g.name = putheap(buf) ;
#endif


#if	DEBUG_MAIN
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	    "main: about to create the 'from' address\n") ;
#endif

/* create the 'fromaddr' header field */

	strcpy(fromaddr,g.mailhost) ;

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

	        if (f_internet) sprintf(from, "%s@%s (%s)", 
	            g.username,MAILNAME,namep) ;

	        else sprintf(from, "%s!%s (%s)", 
	            g.mailhost,g.username,namep) ;

	    } else {

	        if (f_internet) sprintf(from, "%s <%s@%s>", 
	            namep, g.username, MAILNAME) ;

	        else sprintf(from, "%s <%s!%s>", 
	            namep, g.mailhost,g.username) ;

	    }

	} else {

	    if (f_internet) sprintf(from, "%s@%s", 
	        g.username, MAILNAME) ;

	    else sprintf(from, "%s!%s", 
	        g.mailhost,g.username) ;

	}
#else
	if (*from == '\0') {

	    if (myname < 0) sprintf(from,"%s!%s",
	        uts.nodename,g.username) ;

/* setup local or internet from field */

	    if ((tonames != 0) && 
	        (strpbrk(recipient,"@_") != NULL) || f_internet)

/* this is going internet */

	        if (myname >= 0) {

	            strcpy(syscom,from) ;

	            sprintf(from,"%s@%s.%s (%s)",
	                g.username,g.mailhost,g.domain,syscom) ;

	        } 
	        else {

	            sprintf(from,"%s@%s.%s",
	                g.username,g.mailhost,g.domain) ;

	        }
	}
#endif

#if	DEBUG_MAIN
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	    "main: about to do some real work again\n") ;
#endif

	if (ex_mode != EM_PCSMAIL) {

#if	DEBUG_MAIN
	    if (BATST(g.uo,UOV_DEBUG)) errprintf(
	        "main: execution mode not PCSMAIL\n") ;
#endif

	    if (ex_mode == EM_PC) redo_message() ;

	    getheader(tempfile,from) ;

	    ccnames = bccnames = 0 ;

	} else {

#if	DEBUG_MAIN
	    if (BATST(g.uo,UOV_DEBUG)) errprintf(
	        "main: execution mode is PCSMAIL\n") ;
#endif

	    if (iseforward) {

	        fappend(eforwfile,tempfile) ;

	        unlink(eforwfile) ;

	    }

/* check TO:, CC:, BCC: list */

#if	DEBUG_MAIN
	    if (BATST(g.uo,UOV_DEBUG)) errprintf(
	        "main: about to get header fields\n") ;
#endif

	    getfield(tempfile,HS_CC,copyto) ;

#if	DEBUG_MAIN
	    if (BATST(g.uo,UOV_DEBUG)) errprintf(
	        "main: and the BCC\n") ;
#endif

	    getfield(tempfile,HS_BCC,bcopyto) ;

#if	DEBUG_MAIN
	    if (BATST(g.uo,UOV_DEBUG)) errprintf(
	        "main: about to 'getnames(copyto)'\n") ;
#endif

	    if (*copyto != '\0') ccnames = getnames(copyto) ;

#if	DEBUG_MAIN
	    if (BATST(g.uo,UOV_DEBUG)) errprintf(
	        "main: about to 'getnames(bcopyto)'\n") ;
#endif

	    if (*bcopyto != '\0') bccnames = getnames(bcopyto) ;

#if	DEBUG_MAIN
	    if (BATST(g.uo,UOV_DEBUG)) errprintf(
	        "main: about to 'checkreclist()'\n") ;
#endif

	    checkreclist(0) ;

#if	DEBUG_MAIN
	    if (BATST(g.uo,UOV_DEBUG)) errprintf(
	        "main: about to 'redo_message(copyto)'\n") ;
#endif

	    redo_message() ;

#if	DEBUG_MAIN
	    if (BATST(g.uo,UOV_DEBUG)) errprintf(
	        "main: about to ask for sending command\n") ;
#endif

/* ask for sending command */
opts:
	    if (ambiguous) redo_message() ;

	    if (! f_interactive) {

	        strcpy(command,"send") ;

	        isedit = 0 ;

	    } else {

	        if (*recipient == '\0') {

	            printf("\nno recipient(s) specified\n") ;

	            tonames = 0 ;
	            prompt(TO) ;

	            strcpy(realto, recipient) ;

	            checkreclist(0) ;

	            redo_message() ;

	        }

	        if (error != 0) {

	            printf("%s%s","\nthe recipient lists contains",
	                " unknown names\n") ;

	            printf("%s%s","do you really want to send",
	                " the message ? [no] ") ;

	            if (fgetline(stdin,s,BUFSIZE) <= 0) rmtemp(1) ;

	            if (*s != 'y') {

	                printf("%s%s","OK, please edit",
	                    " the message or quit\n") ;
	            }
	        }

	        if (isedit != 0) strcpy(command,"edit") ;

	        else prompt(SENDCOM) ;

	    }

	    cp = command ;
	    while (ISWHITE(*cp)) cp += 1 ;

	    option = strtok(cp,",: \t") ;

/* top of parsing the command */
opts1:
	    if ((option == NULL) || (! f_interactive)) isdef = 1 ;

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

	            break ;

/* check recipient list */
	        case 'c':

#if	DEBUG_MAIN
	            if (BATST(g.uo,UOV_DEBUG))
	                errprintf("main: about to call 'checkit()'\n") ;
#endif

	            checkit() ;

	            break ;

/* edit the message */
	        case 'e':
	            editit() ;

	            checkreclist(0) ;

	            redo_message() ;

	            ambiguous = 0 ;
	            break ;

/* terminate the mail */
	        case 'q':

#if	DEBUG_MAIN
	            if (BATST(g.uo,UOV_DEBUG))
	                errprintf("main: quitting program'n") ;
#endif

	            rmtemp(0) ; /* rmtemp removes tempfiles and exits */

	            fprintf(stderr,
	                "%s: real bad program bug - please report this\n",
	                g.progname) ;

	            goto done ;

/* help user */
	        case '?':
	            help(0) ;

	            break ;

/* unknown. try again. */
	        default:
	            printf("unknown command -- please try again") ;

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

	    if ((! f_interactive) || isdef) option = NULL ;

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
	            rmtemp(0) ; /* quit */

	            fprintf(stderr,
	                "%s: real bad program bug - please report this\n",
	                g.progname) ;

	            goto done ;

/* fall through to next case */
	        case '?':
	            help(1) ;

	            strcpy(sendopt,"send") ;

	            goto opts2; /* break; */

/* unknown ; try again */
	        default:
	            printf("%s%s","unknown send option",
	                " -- please try again.\n") ;

	            strcpy(sendopt,"send") ;

	            goto opts2;	/* break; */
	        }

	        option = strtok(0,":+, 	") ;

	    } /* end while */

	    if (! f_interactive) {

	        getheader(tempfile,from) ;

	        if (*recipient != '\0') tonames = getnames(recipient) ;

	        if (*copyto != '\0') ccnames = getnames(copyto) ;

	        if (*bcopyto != '\0') bccnames = getnames(bcopyto) ;

	    }

	} /* end if (execution mode PCSMAIL or not) */

/* ignore signals */
	setsig(SIG_IGN) ;

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

	fflush(stdout) ;

	fflush(stderr) ;

	if ((tonames > 0) && 
	    (iswait || ((cpid = fork()) == 0) || (cpid == -1))) {

	    deliverem() ;

/* if we forked, the child exits here */

	    rmtemp(0) ;

	} /* end of forked off code */

/* no one to send to Sighhh...! */

	if (tonames == 0) {

	    sprintf(syscom, "cat %s >> %s/%s\n", tempfile,
	        homedir, DEAD) ;

	    system(syscom) ;

	    unlink(tempfile) ;

	    if (isforward > 0) unlink(forwfile) ;

	    if (f_interactive) {

	        printf("\nno recipients were specified\n") ;

	        printf("mail saved in file '%s'\n",DEAD) ;
	    }

	    rmtemp(1) ;

	}

	if (f_interactive) {

	    printf("sending mail ") ;

	    if (standard) printf("(standard") ;

	    if (verify) printf(" verify") ;

	    if (copy) printf(" filecopy") ;

	    if (f_notify) printf(" notify") ;

	    printf(")\n") ;

	}

done:
	fclose(stdout) ;

	fclose(stderr) ;

	return OK ;
}
/* end subroutine (main) */


