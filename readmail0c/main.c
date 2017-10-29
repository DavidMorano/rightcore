/* main */


#define	CF_DEBUG	0


/************************************************************************
 *                                                                      
 * The information contained herein is for use of   AT&T Information    
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   
 *                                                                      
 *     (c) 1984 AT&T Information Systems                                
 *                                                                      
 * Authors of the contents of this file:                                
 *                                                                      
 *                      Bruce Schatz, Jishnu Mukerji                    
			David A.D. Morano

 *									
 ***********************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<time.h>

#include	<userinfo.h>
#include	<logfile.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"config.h"


#define	BUFLEN		4096


/* external variables */

extern char	*strbasename() ;
extern char	*putheap() ;
extern char	*strtimelog() ;
extern char	*getenv() ;


/* local variables */

struct global		g ;

struct userinfo		u ;




int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	time_t	daytime ;

	int	rs ;
	int	failed ;
	int	f_userinfo ;
	int	f_sysvct ;
	int	f_sysvrt ;

	char	startup[LINELEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;


	g.progname = strbasename(argv[0]) ;

/* get some machine information */

#ifdef	BSD
	f_sysvct = FALSE ;
#else
	f_sysvct = TRUE ;
#endif

	f_sysvrt = FALSE ;
	if (access("/usr/sbin",R_OK) >= 0) f_sysvrt = TRUE ;

/* find our program root directory */

	if ((g.progroot = getenv("PCS")) == NULL)
		g.progroot = PCS ;

/* get some user information */

#if	CF_DEBUG
	debugprintf("main: before userinfo %s\n",
		strtimelog(time(NULL),timebuf)) ;
#endif

	f_userinfo = TRUE ;
	if ((rs = userinfo(&u,userbuf,USERINFO_LEN,NULL)) < 0) {

	    f_userinfo = FALSE ;
	    fprintf(stderr,
	        "%s: could not get user information (rs %d)\n",
	        g.progname,rs) ;

	    sprintf(buf,"unknown%d",getpid()) ;

	    u.logid = putheap(buf) ;

	}

#if	CF_DEBUG
	debugprintf("main: after userinfo, before logfile_open %s\n",
		strtimelog(time(NULL),timebuf)) ;
#endif

/* log us and that is about it for now */

	sprintf(buf,"%s/%s",g.progroot,LOGFILE) ;

	if ((rs = logfile_open(&g.lh,buf,0,0666,u.logid)) >= 0) {

	    buf[0] = '\0' ;
	    if (u.mailname != NULL) sprintf(buf,"(%s)",u.mailname) ;

	    daytime = time(NULL) ;

	    logfile_printf(&g.lh,"%s %-14s %s/%s\n",
	        strtimelog(daytime,timebuf),
	        g.progname,
		VERSION, (f_sysvct) ? "SYSV" : "BSD") ;

	    logfile_printf(&g.lh,
	        "os=%s d=%s %s!%s %s\n",
	        (f_sysvrt) ? "SYSV" : "BSD",
	        u.domainname,
	        u.nodename,u.username,buf) ;

	} /* end if (opened the log file) */

#if	CF_DEBUG
	debugprintf("main: after logfile_open %s\n",
		strtimelog(time(NULL),timebuf)) ;
#endif

/* are we new or old, et cetera */

	g.prog_mailer = PROG_MAILER ;
	if ((g.progname[0] == 'n') || (g.progname[0] == 'o')) {
	
		sprintf(buf,"%c%s",g.progname[0],PROG_MAILER) ;

		g.prog_mailer = putheap(buf) ;

	}


/* clear umask so creat permissions are used without modification */

	umask(0000) ;

	get_screensize() ;

/* initialize the user profile options */

	profinit() ;

	if (argc == 1) {	/* no args. standard startup in new box */

	    failed =  setup_mailbox("new") ;

	} else {

/* first argument is startup mail box */
	    full_boxname (startup,argv[1]) ;

	    if ((access(startup, A_READ)) == 0) {

	        failed =  setup_mailbox(argv[1]) ;

	    } else	{

	        printf(" no access to mailbox \"%s\",  ",argv[1]) ;
	        printf("starting up in the \"new\" box.\n") ;
	        failed =  setup_mailbox("new") ;
	    }
	}

/* interactively read and execute the commands */
	if (failed)
	    printf("\n *** program aborting *** \n") ;

	else
	    inter() ;

	fflush(stdout) ;

	fflush(stderr) ;

	return 0 ;
}
/* end subroutine (main) */



