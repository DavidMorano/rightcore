/* defs */

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
		David A.D. Morano
 *									

 ************************************************************************/


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>


#ifndef	INC_BIO
#include	<bfile.h>
#endif

#ifndef	INC_LOGFILE
#include	<logfile.h>
#endif



/* standard stuff */

#ifndef	BAD
#define	TRUE	1
#define	FALSE	0
#define	OK	0
#define	BAD	-1
#endif

#ifndef	NULL
#define	NULL	((char *) 0)
#endif


/* some constant definitions, DO NOT CHANGE ! */

#define	SAME	0
#ifndef	YES
#define YES 	1
#define NO 	0
#endif
#define SPECIAL -1



/* struct to hold a master name */

struct name {
	char	*first ;
	char	*middle1 ;
	char	*middle2 ;
	char	*last ;
	char	*trail ;
} ;


/* translation table structure */

#define	TT_ALL		0
#define	TT_USER		1
#define	TT_SYSTEM	2

struct table {
	char		realname[LENNAME] ;	/* real name of user */
	char		mailaddress[SYSLEN] ;	/* system and login of user */
	char		mail ;
	char		type ;
	struct table	*syslink;	/* pointer links table by sysname */
} ;


/* table of system names with pointer into translation table */

struct sys_tbl {
	char sysname[SYSNAMELEN];	/* name of system (i.e., hocse ) */
	struct table *trans_link ;	/* pointer into translation table */
} ;



struct gflags {
	unsigned int	debug : 1 ;
	unsigned int	version : 1 ;
	unsigned int	exit : 1 ;
	unsigned int	nosend : 1 ;
	unsigned int	sysv_ct : 1 ;
	unsigned int	sysv_rt : 1 ;
	unsigned int	interactive : 1 ;
} ;

struct global {
	struct gflags	f ;
	logfile	lh ;
	logfile	eh ;
	bfile	*efp ;
	bfile	*ifp ;
	bfile	*ofp ;
	time_t	daytime ;
	uid_t	uid ;
	gid_t	gid ;
	gid_t	gid_mail ;
	pid_t	pid ;
	int	debuglevel ;
	char	*progname ;
	char	*pcs ;
	char	*logid ;
	char	*nodename ;
	char	*username ;
	char	*mailnode ;
	char	*mailhost ;
	char	*companydomain ;
	char	*localdomain ;
	char	*maildomain ;
	char	*mailname ;
	char	*name ;
	char	*fullname ;
	char	*gecosname ;
	char	*homedir ;
	char	*date_envelope ;	/* UNIX mail message envelope date */
	char	*date_header ;		/* header date string */
	char	*messageid ;
	char	*prog_editor ;
	char	*prog_pcscleanup ;	/* PCS "cleanup" program */
	char	*prog_sendmail ;	/* Berkley SENDMAIL daemon */
	char	*prog_pcscl ;		/* PCS "content-lenght" adder */
	char	*prog_pager ;
} ;



#define	CHECK	0
#define	SETUP	1
#define	SEND	2


/* sendmail execution (invocation) modes */

#define EM_PCSMAIL	0
#define EM_REMOTE	1
#define EM_PC		2
#define EM_PCSINFO	3

#define ORIG		0
#define REMOTE		1
#define PC		2


/* structure to hold parsed address entries */

struct address {
	char	*gaddress ;		/* given address */
	char	*address ;		/* address */
	char	*raddress ;		/* route address */
	char	*comment ;
	char	*xaddress ;		/* translated address */
	char	*realname ;
	struct address	*next ;
	struct address	*matches ;
	struct address	*list ;
} ;


/* global variable declarations */

#ifdef MASTER
#define VAR
#else
#define VAR extern
#endif

/* global variables */

VAR int myname;			/* pointer into namedb to users name	*/
VAR int ex_mode;		/* local invocation ORIG, remote REMOTE */
VAR int tot_sys;		/* tottal number of mailbags used	*/

VAR char comm_name[MAXNAMELEN + 1] ;	/* name by which program invoked*/
VAR char selectname[LENNAME + 1] ;	/* used to pass realnames around */
VAR char curaddr[BUFSIZE + 1] ;	/* pass address by parsename */
VAR char  thisys[SYSNAMELEN + 1] ;	/* holds system name */
VAR char stddatetime[64];	/* holds date and time in header std form*/

VAR char tempfile[MAXPATHLEN + 1] ;	/* temporary file */
VAR char tmpf[MAXPATHLEN + 1] ;		/* temporary filename */
VAR char tmpf1[MAXPATHLEN + 1] ;	/* temporary filename */

/* message header fields */

VAR char mess_id[SYSLEN + 1] ;	/* message id				*/
VAR char received[BUFSIZE + 1];	/* all received[-by] fields		*/
VAR char from[BUFSIZE + 1]  ;	/* the from field			*/
VAR char realto[BUFSIZE + 1];	/* address of the real addressee	*/
VAR char date[100] ;		/* date					*/
VAR char sentby[BUFSIZE + 1];	/* identity of the sender		*/
VAR char fromaddr[BUFSIZE + 1];	/* holds the from address in UUCP form  */
VAR char reference[BUFSIZE + 1] ;	/* reference field */
VAR char keys[BUFSIZE + 1] ;	/* keywords */
VAR char subject[BUFSIZE + 1] ;	/* holds the subject line of the message*/
VAR char moptions[SYSLEN + 1];	/* the options line			*/

VAR FILE  *fp, *fp1;		/* file pointers to open files		*/
VAR FILE *fwho;			/* pointer to utmp (who info) 		*/
 
VAR char ambiguous;		/* =1 if there is an ambiguous recipient*/
				/* causes a redo_message to happen	*/


/* information collected from the command line and SMAILOPTS */

VAR char recipient[BUFSIZE + 1] ;	/* name(s) of primary recipient */
VAR char copyto[BUFSIZE + 1] ;	/* name(s) of copy recipients		*/
VAR char bcopyto[BUFSIZE + 1];	/* name(s) of blindcopy recipients	*/
VAR char appfile[SYSLEN + 1] ;	/* name of file to be appended to mail	*/
VAR char forwfile[SYSLEN + 1] ;	/* filename of the file to be forwarded */
 				/* for forwarding mail. jm	        */
VAR char eforwfile[MAXPATHLEN + 1] ;	/* for edit&forwarding mail */
VAR char filename[MAXPATHLEN + 1] ;	/* file containing message attached*/
VAR char origfile[MAXPATHLEN + 1] ;	/* file containing the message */
				/* a reply is being set up		*/
VAR char retpath[(2*BUFSIZE) + 1] ;	/* return path from mail header */
				/* message that is being replied to	*/
VAR char message[BUFSIZE + 1] ;	/* message collected from the "m" option*/


/* scratchpad buffer */
VAR char  syscom[(2*BUFSIZE) + 1];
VAR char  s[(2*BUFSIZE) + 1];
VAR char  s1[(2*BUFSIZE) + 1];

 
#ifdef DEBUG
VAR FILE *errlog;		/* file structure for error log 	*/
#endif
 
VAR int	table_is_in ;		/* set = 1 when translation tables are  */
				/* read in				*/
VAR int	verbose ;		/* flag for verbose			*/
VAR int	verify ;		/* flag for verify 			*/
VAR int	f_confirm ;		/* flag for confirmation of delivery  	*/
VAR int	f_notify ;		/* flag for notification on delivery  	*/
VAR int	isappend ;		/* append flag, >0 => *appfile != NULL	*/
VAR int	isforward ;		/* forward flag,>0 => *forwfile != NULL */
VAR int	iseforward ;		/* edit&forward,>0 => *eforwfile != NULL*/
VAR int	f_internet ;		/* on if internet option is specified	*/
VAR int	standard ;		/* flag for standard 			*/
VAR int	keyword ;		/* flag for keyword 			*/
VAR int	isreferenced ;		/* flag for reference 			*/
VAR int	iswait ;		/* flag for wait 			*/
VAR int	f_ispipe ;		/* on if invoked as a pipe		*/

VAR int	isdef ;			/*					*/
VAR int	isfile ;		/* on if file= is used			*/
VAR int	isret ;			/* flag for return address 1227		*/
VAR int	isoriginal;		/* on if original message for reply is	*/
				/* available				*/
VAR int	runcmd ;		/* 					*/
VAR int	copy ;			/* flag for copy mail 			*/
VAR int	cc ;			/* flag for cc (copy to) list 		*/
VAR int	bcc;			/* flag for bcc list			*/
VAR int	error ;			/* error flag				*/
VAR int	flag ;			/* flag for miscellaneous local use	*/
VAR int	orig_edit ;		/* on to pass original message to the   */
				/* editor for a reply -  /*PAS-JM 2/8/85*/
VAR int	f_name ;		/* use a name in the "FROM" line */
VAR int	f_fullname ;		/* use FULLNAME env variable	*/
VAR int	f_noprompt ;		/* do not perform prompting */
VAR int	f_version ;		/* version wanted */


/* counting variables */

VAR int	isedit ;		/* on if +edit is used			*/
VAR int goodmail ;		/* count of number of messages sent	*/
VAR int tonames ;		/* how many names there actually are 	*/
VAR int ccnames ;		/* how many names there actually are 	*/
VAR int bccnames;		/* how many names there actually are	*/

/* translation table */
VAR int tablelen ;		/* number entries in master table */
VAR struct table *name[NAMEDB];	/* the table itself		  */

/* environment variables */
#ifdef MASTER
char namelist[BUFSIZE + 1] = DSMAILNAMES;
char mailopts[BUFSIZE + 1] = DSMAILOPTS;
char maillist[BUFSIZE + 1] = DSMAILLISTS;
#else
extern char namelist[];
extern char mailopts[];
extern char maillist[];
#endif

 
#endif /* DEFS_INCLUDE */


