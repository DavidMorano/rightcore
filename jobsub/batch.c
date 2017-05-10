/* batch */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-out */
#define	CF_SYSLOG	0


/******************************************************************************

	This subroutine is the main code for submitting a job.

	Synopsis:

 	jobsub [-v] [-j jobname] [-m mailuserid] 
         [-q queue] [file ...] 


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include <pwd.h>
#include <stdio.h>
#include <errno.h>

#include	<vsystem.h>
#include	<exitcodes.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"stat32.h"
#include	"config.h"
#include	"defs.h"
#include "bat_system.h"		/* sets BSD4_2 and JOBCTL flags */
#include "bat_common.h"


/* local defines */


/* external subroutines */

extern int	sfsub(const char *,int,const char *,char **) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	findfilepath(const char *,char *,const char *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	getpwd(char *,int) ;


/* external variables */


/* forward references */

static int	concat() ;

static void 	putstring() ;
static void	putoutenv() ;


/* local variables */

static char grade = 'q';	/* Priority of this job */
static enum { 
	BOURNE, CSH 
} shelltype ;
static char *deleteerr ;

/* Make sure all this matches stuff in batchd too.  */

static char *spooldir = SPOOLDIR ;
static char errstr[STRSIZ + 1] ; /* Buffer for SPRINTF error messages */
static int debug = 0 ;		/* turn on for debugging output */
static long unique() ;







int batch(pip,ofp,flp)
struct proginfo	*pip ;
void		*ofp ;
vecstr		*flp ;
{
	struct ustat32	sbuf ;
	struct ustat	*sbp = (struct ustat *) &sbuf ;

	FILE *tf ;

	long spoolnumber ;

	int	rs = SR_OK, rs1 ;
	int i ;
	int currumask ;
	int nocshrc = 0;		/* turn on to enable "#!/bin/csh -f" */
	int uid ;
	int status ;

#ifdef MODIFYPRIORITY
	int njobs ;
#endif

	char fname[MAXPATHLEN + 1] ;
	char tfname[MAXPATHLEN + 1] ;
	char cfname[MAXPATHLEN + 1] ;
	char cwdirectory[MAXPATHLEN + 1] ;
	char *jobname = NULL ;
	char *userid = NULL ;
	char *mailuserid = NULL ;
	char *queue ;
	char *shell ;
	char *command = NULL ;
	char	*progname ;
	char	*fnp ;
	char	**epp ;


	progname = pip->progname ;

	uid = pip->uid ;
	currumask = umask(0077);	/* Privacy for created files */

	spooldir = pip->spooldname ;

	queue = pip->queuename ;
	jobname = pip->jobname ;
	userid= pip->username ;
	mailuserid = pip->mailaddr ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("batch: spooldname=%s\n",pip->spooldname) ;
	    debugprintf("batch: mailaddr=%s\n",pip->mailaddr) ;
	    debugprintf("batch: jobname=%s\n",pip->jobname) ;
	    debugprintf("batch: queuename=%s\n",pip->queuename) ;
	}
#endif

/*
		 * Print "no such queue" if the queue is not there or if
		 * it is not a directory.
		 */
	mkpath2(fname, spooldir, queue) ;

	rs = u_stat(fname,sbp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("batch: spoolqueue=%s rs=%d\n",
	        fname,rs) ;
#endif

	if ((rs >= 0) && (! S_ISDIR(sbp->st_mode)))
	    rs = SR_NOTDIR ;

	if (rs < 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("batch: spoolqueue=%s rs=%d\n",
	        fname,rs) ;
#endif

	    bprintf(pip->efp,"%s: not such queue=%s\n",
	        pip->progname,pip->queuename) ;

	    goto bad0 ;
	}

/*
		 * If the underlying Q_CFDIR is missing, create it.
		 */
	mkpath3(fname, spooldir, queue, Q_CFDIR) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("batch: cfdir=%s rs=%d\n", fname,rs) ;
#endif

	for (i = 0 ; i < 5 ; i += 1) {

	    rs = u_stat(fname,sbp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("batch: cfdir u_stat() rs=%d\n", rs) ;
#endif

	    if (rs >= 0)
		break ;

	    mkdirs(fname, 0775) ;

	    sleep(1) ;

	} /* end for */

#ifdef MODIFYPRIORITY
/* Around here people tend to submit a lot jobs at one time which can */
/* cause the queue to become clogged.  So we count the number of */
/* currently queued, uncanceled jobs and adjust the priority based */
/* on this information.  Note that really low priority jobs */
/* can be starved by the batchd so you may not want to do this. GRS */
	njobs = CountJobs(fname) ;
	grade += njobs ;
	if (grade > 'z')
	    grade = 'z' ;
#endif /* MODIFYPRIORITY */

/*
* Spool temp file name starts with tf, followed by grade, followed by
* a unique sequential number.  Real file name starts with cf.
* The files are sorted by this name for priority of execution.
*/

#ifdef BSD4_2
#define FMT "%s/%s/%s/%cf%c%ld"
#else
#define FMT "%s/%s/%s/%cf%c%1d"
#endif

	spoolnumber = unique() ;

	snprintf(tfname,MAXPATHLEN, FMT, spooldir, queue,
	    Q_CFDIR, 't', grade, spoolnumber) ;

	snprintf(cfname,MAXPATHLEN, FMT, spooldir, queue,
	    Q_CFDIR, 'c', grade, spoolnumber) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("batch: tfname=%s\n",tfname) ;
	    debugprintf("batch: cfname=%s\n",cfname) ;
	}
#endif

	tf = fopen(tfname, "w") ;

	if (tf == NULL) {
	    rs = SR_ACCESS ;
	    goto bad0 ;
	}

/* "shelltype" tells which type of environment setting to use.  */

	if ((shell = getenv("SHELL")) == NULL)
	    shell = BAT_SH ;

	if (strcmp(shell, BAT_SH) == 0)
	    shelltype = BOURNE ;

	else if (strncmp(shell+strlen(shell)-3, BAT_CSHELL, 3) == 0)
	    shelltype = CSH ;

	else {

/* Assume it's a bourne-like shell. Don't you just
				   love hard-coded shells? */
	    shelltype = BOURNE ;

	}

/* Write out the shell line and the header information that
* the Batch Daemon will need to start running this job.
* Since it will be directly executable by the system put out a
* system magic word followed by the current shell.
*
* Things to associate with the queued batch file:
*
* <no name>: (protected)
*   - local userid on this machine for executing/accounting
*      -> this is the uid of the queued batch file
* jobname: (user-settable)
*   - job name/identifier
* userid: (protected)
*   - full userid@host responsible for job
*      - same as local userid, unless file is uid ROOT
*      - for uid ROOT file, believe the "userid" header, if any
* mailuserid: (user-settable)
*   - userid@host for mail from job
*      - use "userid" or local uid if no "mailuserid" found
* directory: (semi-protected)
*   - current directory
* umask: (user-settable)
*   - current umask
* ???: (user-settable)
*   - shell to run it with
* ???: (protected)
*   - priority of file
* ???: (semi-protected)
*   - system resource limits
* ???: (user-settable)
*   - environment
*     -> currently copied as "setenv" commands to the batch file
*/

	rs = SR_OK ;

	{
	    char	progbuf[MAXPATHLEN + 1] ;
	    char	*pbp ;


	    pbp = shell ;
	    if (shell[0] != '/') {

	        pbp = shell ;
	        rs1 = findfilepath(DEFPATH,progbuf,shell,X_OK) ;

		if (rs1 > 0)
		pbp = progbuf ;

	    }

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("batch: fprintf starting\n") ;
#endif

	    fprintf(tf, "#!%s %s\n", 
	        pbp,((shelltype==CSH && nocshrc)?"-f":"")) ;

	} /* end block */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("batch: fprintf jobname=%s\n",jobname) ;
#endif

	if ( jobname )
	    fprintf(tf, "# jobname: %s\n", jobname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("batch: fprintf userid=%s\n",userid) ;
#endif

	if ( userid )
	    fprintf(tf, "# userid: %s\n", userid ) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("batch: fprintf mailuserid=%s\n",mailuserid) ;
#endif

	if ( mailuserid )
	    fprintf(tf, "# mailuserid: %s\n", mailuserid ) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("batch: fprintf currumask=%06o\n",currumask) ;
#endif

	fprintf(tf, "# umask: 0%03o\n", currumask ) ;

	if (getpwd(cwdirectory,MAXPATHLEN) < 0) {
	    goto bad1 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("batch: fprintf cwdirectory=%s\n",cwdirectory) ;
#endif

/*
* Peel off the leading /tmp/mnt_ from the cwd,
* so things work correctly when using Sun's automounter.
*	- Dr. J. Holzfuss
*/

	{
		int	f ;


	    f = (strncmp(cwdirectory, "/tmp_mnt/", 9) == 0) ;
	    fprintf(tf, "# directory: %s\n",
		((f) ? (cwdirectory+8) : cwdirectory)) ;

	}

	fprintf(tf, "\n");	/* null line ends header */

/* The big problem with using these commands is that C Shell users
		 * can redefine them with aliases in their .cshrc files.
		 * That can screw up system programs, such as typeset, that
		 * try to create batch jobs for users.  So I added the -f
		 * "don't use .cshrc" option.  -IAN!
		 */

	if ( shelltype == CSH )
	    fprintf(tf,"set histchars\n") ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("batch: fprintf regular environment\n") ;
#endif


/*
		 * Set everything up so that it will execute in the same env. as
		 * we have right now.  Dump the current environment variables. 
		 * Ignore them if they don't have = signs, throw away TERMCAP and TERM.
		 */
	for (epp = pip->envv ; *epp ; epp += 1) {

	    char *p ;


	    if ((p = strchr(*epp, '=')) == NULL)
	        continue ;

	    p += 1 ;
	    if (strkeycmp(*epp, "TERMCAP") == 0)
	        continue ;

	    if (strkeycmp(*epp, "TERM") == 0)
	        continue ;

	    putoutenv(tf, *epp, p) ;

	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("batch: fprintf regular environment done\n") ;
#endif

/*
* If using csh, set $home so the ~ character expands correctly.
* This should be something CSH does automatically when you
* set $HOME.
*/

	if (shelltype == CSH)
	    fprintf(tf, "if ( $?HOME ) then\n\tset home=$HOME\nendif\n") ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("batch: fprintf control string\n") ;
#endif

/*
*  Dump the files specified; or stdin otherwise.
*  First, put out a CONTROL_STR comment line to indicate we
*  are starting the real part of the command file.
*/

	fprintf(tf,CONTROL_STR) ;

	for (i = 0 ; vecstr_get(flp,i,&fnp) >= 0 ; i += 1) {

	    FILE *input ;

	    char *name ;


	    if (fnp == NULL) continue ;

	    name = fnp ;
	    if ((strcmp(fnp,"-") == 0) || (strcmp(fnp,STDINFNAME) == 0)) {

	        input = stdin ;
	        name  = "(stdin)" ;

	    } else {

/* Verify read permission using access() for real uid */

	        if (access(name, R_OK) != 0 ||
	            (input = fopen(name, "r")) == NULL) {
	            rs = SR_ACCESS ;
	            break ;
	        }
	    }

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("batch: fprintf comcat\n") ;
#endif

	    rs = concat(tf, input, name) ;

	    if ( input != stdin )
	        (void)fclose(input) ;

	    else
	        clearerr(input) ;

	    if (rs < 0)
	        break ;

	} /* end for (looping through files) */

	if (rs < 0)
	    goto bad1 ;

/*
* Append a wait command in case the user put the job 
* in the background. GRS
*/

	fprintf(tf, "wait\n") ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("batch: fprintf done\n") ;
#endif

/*
*  Set everything up for direct execution; and then link it to
*  its real filename.
*/

	rs = u_chmod(tfname, 0700) ;

	if (rs < 0)
	    goto bad1 ;

	fclose(tf) ;

	rs = u_rename(tfname, cfname) ;

	if (rs < 0)
	    goto bad0 ;

	if (pip->verboselevel > 0)
	    bprintf(ofp,"%s\n", cfname) ;

done:
	umask(currumask) ;

bad0:
	return rs ;

/* bad stuff */
bad1:
	if (tfname[0] != '\0')
	    u_unlink(tfname) ;

	goto done ;
}
/* end subroutine (batch) */



/* LOCAL SUBROUTINES */



static int concat(to, from, file)
register FILE *to, *from ;
char *file ;
{
	int	rs = SR_OK ;
	int c ;
	int ttyin ;


	ttyin = isatty(fileno(from)) ;

	if ( ttyin ) {

	    fprintf( stderr, "submit> ") ;

	    fflush(stderr) ;

	}

	while ((c = getc(from)) != EOF) {

	    putc(c, to) ;

	    if ( c == '\n' && ttyin ) {

	        rs = fprintf( stderr, "submit> ") ;

	        fflush(stderr) ;
	    }

	} /* end while */

	return rs ;
}
/* end subroutine (concat) */


/* Dump out an environment variable in the format for this shell */
static void putoutenv(tf, ep, p)
FILE	*tf ;
char	*ep, *p ;
{
	char	*tp ;


#if	CF_DEBUGS
	    debugprintf("batch: putoutenv() in\n") ;
#endif

	tp = strchr(ep,'=') ;

	if (tp != NULL) {

	switch (shelltype) {

	case BOURNE:
	    fbwrite(tf,ep,(tp - ep)) ;

	    fbwrite(tf,"=",1) ;

	    putstring(tf, p ) ;

	    fbwrite(tf,"; export ",9) ;

	    fbwrite(tf,ep,(tp - ep)) ;

	    fbwrite(tf,"\n",1) ;

	    break ;

	case CSH:
	    fbwrite(tf,"setenv ",7) ;

	    fbwrite(tf,ep,(tp - ep)) ;

	    fbwrite(tf," ",1) ;

	    putstring(tf, p ) ;

	    fprintf(tf, "\n") ;

	    break ;

	} /* end switch */

	} /* end if */

#if	CF_DEBUGS
	    debugprintf("batch: putoutenv() out\n") ;
#endif

}
/* end subroutine (putoutenv) */


/*
 *  God help us; put out a string with whatever escapes are needed to make
 *  damn sure that the shell doesn't interpret it.  BOURNE shell doesn't
 *  need escapes on newlines; CSH does.
 */
static void putstring(tf, s)
register FILE *tf ;
register char *s ;
{


#if	CF_DEBUGS
	    debugprintf("batch: putstring() in\n") ;
#endif

	putc('\'', tf) ;

	while (*s) {

	    if (*s == '\n' && shelltype == CSH )
	        putc('\\', tf) ;

	    if (*s == '\'')
	        s++, fprintf(tf,"'\"'\"'"); /* 'You can'"'"'t come' */

	    else
	        putc(*s++, tf) ;
	}

	putc('\'', tf) ;

#if	CF_DEBUGS
	    debugprintf("batch: putstring() out\n") ;
#endif

}
/* end subroutine (putstring) */


/*
 * Generate a unique number.
 * We use the time of day for the high bits
 * (so sorting the files by ASCII string compares
 * sorts them by creation time - an important property)
 * concatenated with some lower bits of the process ID.
 *
 * Possible screwups:
 *    -	If this UNIX ever creates more than
 *	2^PIDBITS processes in one second, and two of them are
 *	"batch", then they might come up with the same filename.
 *	Make PIDBITS large enough to make this arbitrarily unlikely.
 *    - Every 24855/(2^PIDBITS) days, the unique number overflows.
 *	This results in newer entries being processed before ones
 *	that predate the rollover.  This is a problem only if
 *	the queue never drains completely, leaving the old ones stuck
 *	for a LONG time, or if absolute order of processing is important.
 *
 * Setting PIDBITS to 4 handles process creation rates of up to 16/sec,
 * and gives a unique number overflow every 4 years or so.  This should
 * be good enough for most applications.
 */

#define	PIDBITS	4

static long unique()
{

	return ( (time((long *)0)<<PIDBITS) | getpid()&((1<<PIDBITS)-1) )
	    & 0x7fffffff ;
}


#ifdef MODIFYPRIORITY
/*
 * CountJobs:  This function counts the number of currently queued,
 * non-cancelled jobs submitted by the user in this queue.
 *
 * Written by G. Strachan June 1992 
 */

static int CountJobs(fname)
char *fname ;
{
	DIR *dir ;

	struct ustat stb ;
	struct_dir *dp ;

	int njobs = 0 ;
	int uid = getuid() ;

	char job[MAXPATHLEN + 1] ;


	dir = opendir(fname) ;

	if (dir == NULL)
	    goto done ;

	while ((dp = readdir(dir)) != NULL) {

	    if (strncmp(dp->d_name,"cf",2) != 0)
	        continue ;

	    mkpath2(job,fname,dp->d_name) ;

/* check if we own it and it isn't world readable */
	    if (stat(job,&stb) == 0) {

	        if ((stb.st_uid == uid) &&
	            ((stb.st_mode & S_IRGRP) == 0) &&
	            ((stb.st_mode & S_IROTH) == 0))
	            njobs++ ;

	    }
	}

done:
	return njobs ;
}
/* end subroutine */

#endif /* MODIFYPRIORITY */



