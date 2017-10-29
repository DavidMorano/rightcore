/* defs */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1



#include	<sys/types.h>

#include	<vecstr.h>
#include	<logfile.h>
#include	<lfm.h>




#define	LINELEN		256		/* size of input line */
#define	BUFLEN		(5 * 1024)
#define	NLENV		40
#define	ENVLEN		2000

#ifndef	LOGIDLEN
#define	LOGIDLEN	LOGFILE_LOGIDLEN
#endif

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	2048
#endif

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
#endif

#define	CMDBUFLEN	(2 * MAXPATHLEN)

#define	MAXSLEEPTIME	270	/* must not be longer than 2min 30sec */
#define	LOCKTIMEOUT	300	/* lock file timeout (5min) */



/* program exit codes */

#define	PRS_USAGE	1
#define	PRS_OK		0
#define	PRS_BAD		-1
#define	PRS_BADFORK	-2
#define	PRS_BADARGS	-3
#define	PRS_BADLOG	-4
#define	PRS_BADOPT	-5
#define	PRS_BADARG	-6
#define	PRS_BADNUM	-7
#define	PRS_BADEXTRA	-8
#define	PRS_BADVALUE	-9
#define	PRS_BADWORKING	-10
#define	PRS_BADQUEUE	-11
#define	PRS_BADUSER	-12
#define	PRS_BADINT	-13
#define	PRS_BADSRV	-14
#define	PRS_NOSRV	-15
#define	PRS_BADDIR	-16
#define	PRS_BADCONFIG	-17
#define	PRS_BADLOCK	-18
#define	PRS_BADPIDOPEN	-19
#define	PRS_BADPIDLOCK	-20
#define	PRS_BADLOCK2	-21
#define	PRS_BADPID2	-22



/* global data */

struct proginfo_flags {
	uint	error : 1 ;		/* we have STDERR */
	uint	log : 1 ;
	uint	srvtab : 1 ;		/* we have a service table file */
	uint	interrupt : 1 ;		/* we-re supposed to have INT file */
	uint	quiet : 1 ;
	uint	poll : 1 ;		/* poll only mode */
} ;

struct proginfo {
	struct proginfo_flags	f ;
	logfile	lh ;		/* program activity log */
	logfile	eh ;		/* error log */
	bfile	*efp ;
	vecstr	exports ;
	vecstr	paths ;
	LFM	pider ;
	bfile	*lfp ;		/* system log "Basic" file */
	bfile	*lockfp ;	/* lock file BIO FP */
	char	*progname ;		/* program name */
	char	*version ;		/* program version string */
	char	*programroot ;		/* program root directory */
	char	*searchname ;		/* program search name */
	char	*nodename ;		/* machine node name */
	char	*domainname ;		/* INET domain name */
	char	*logid ;
	char	*command ;
	char	*srvtab ;		/* service file name */
	char	*workdir ;
	char	*tmpdir ;		/* temporary directory */
	char	*username ;
	char	*groupname ;
	char	*directory ;		/* directory to watch */
	char	*interrupt ;
	char	*lockfname ;		/* lock file */
	char	*pidfname ;		/* mutex lock file */
	pid_t	pid ;
	uid_t	uid ;			/* real UID */
	uid_t	euid ;			/* effective UID */
	VOLATILE int	f_exit ;
	int	debuglevel ;		/* debugging level */
	int	verboselevel ;		/* verbosity level */
	int	pollmodetime ;
	int	polltime ;
} ;


/* job states */

#define	STATE_WATCH	0		/* just entered system */
#define	STATE_WAIT	1		/* resource wait */
#define	STATE_STARTED	2		/* execution has been started */
#define	STATE_STALE	3		/* no service entry for job */


struct jobentry {
	offset_t	size ;
	time_t	daytime ;		/* daytime at start of job */
	time_t	stime ;			/* initial creation time */
	time_t	mtime ;
	pid_t	pid ;
	int	state ;
	char	filename[MAXNAMELEN + 1] ;
	char	ofname[MAXNAMELEN + 1] ;
	char	efname[MAXNAMELEN + 1] ;
	char	logid[LOGFILE_LOGIDLEN + 1] ;
} ;



/*************************************************************************

#	The following substitutions are made on service daemon invocations :
#
#		%f	file name
#		%j	job name
#		%s	service name
#		%d	directory path
#		%h	remote hostname
#		%a	service arguments
#		%n	current node name
#		%u	username
#

**************************************************************************/

struct expand {
	char	*a ;
	char	*s ;
	char	*f ;
	char	*m ;
	char	*d ;
	char	*l ;
	char	*c ;
	char	*w ;
	char	*r ;
} ;


#endif /* DEFS_INCLUDE */



