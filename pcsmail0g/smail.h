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



#ifndef	INC_BIO
#include	<bfile.h>
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
	char sysname[SYSNAME];		/* name of system (i.e., hocse ) */
	struct table *trans_link ;	/* pointer into translation table */
} ;


#define	NUO	8

#define	UOV_DEBUG	0
#define	UOV_NOSEND	1


struct global {
	bfile	*efp ;
	bfile	*ofp ;
	int	uo[NUO] ;
	int	uom[NUO] ;
	int	uid ;
	int	gid ;
	int	gid_mail ;
	int	pid ;
	char	*logid ;
	char	*progname ;
	char	*nodename ;
	char	*username ;
	char	*mailhost ;
	char	*domain ;
	char	*mailname ;
	char	*name ;
	char	*fullname ;
	char	*gecosname ;
	char	*homedir ;
	char	*datetime ;
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

VAR char comm_name[128];	/* name by which program has been invoked*/
VAR char selectname[LENNAME];	/* used to pass realnames around	 */
VAR char curaddr[SYSLEN];	/* used to pass address by parsename	 */
VAR char  thisys[64];		/* holds system name			 */
VAR char  datetime[64];		/* holds today's date and current time 	 */
VAR char stddatetime[64];	/* holds date and time in header std form*/

VAR char tempfile[MAXPATHLEN] ;	/* temporary file for holding message 	 */
VAR char tmpf[MAXPATHLEN] ;		/* temporary filename */
VAR char tmpf1[MAXPATHLEN] ;		/* temporary filename */
VAR char uuname[64] ;		/* file to hold the output of uuname	 */
VAR char homedir[MAXPATHLEN] ;	 	/* Holds users home directory name 	 */

/* message header fields */

VAR char mess_id[SYSLEN] ;	/* message id				*/
VAR char received[BUFSIZE];	/* all received[-by] fields		*/
VAR char from[BUFSIZE]  ;	/* the from field			*/
VAR char realto[BUFSIZE];	/* address of the real addressee	*/
VAR char date[100] ;		/* date					*/
VAR char sentby[BUFSIZE];	/* identity of the sender		*/
VAR char fromaddr[BUFSIZE];	/* holds the from address in UUCP form  */
VAR char reference[BUFSIZE] ;	/* reference field			*/
VAR char keys[BUFSIZE] ;	/* keywords				*/
VAR char subject[BUFSIZE] ;	/* holds the subject line of the message*/
VAR char moptions[SYSLEN];	/* the options line			*/

VAR FILE  *fp, *fp1;		/* file pointers to open files		*/
VAR FILE *fwho;			/* pointer to utmp (who info) 		*/
 
VAR char ambiguous;		/* =1 if there is an ambiguous recipient*/
				/* causes a redo_message to happen	*/


/* information collected from the command line and SMAILOPTS */

VAR char recipient[BUFSIZE] ;	/* name(s) of primary recipients	*/
VAR char copyto[BUFSIZE] ;	/* name(s) of copy recipients		*/
VAR char bcopyto[BUFSIZE];	/* name(s) of blindcopy recipients	*/
VAR char appfile[SYSLEN] ;	/* name of file to be appended to mail	*/
VAR char forwfile[SYSLEN] ;	/* filename of the file to be forwarded */
 				/* for forwarding mail. jm	        */
VAR char eforwfile[MAXPATHLEN] ;	/* for edit&forwarding mail */
VAR char filename[MAXPATHLEN] ;	/* file containing message to be attached*/
VAR char origfile[MAXPATHLEN] ;	/* file containing the message to which */
				/* a reply is being set up		*/
VAR char retpath[2*BUFSIZE] ;	/* return path from mail header of the 	*/
				/* message that is being replied to	*/
VAR char message[BUFSIZE] ;	/* message collected from the "m" option*/

VAR char	s_sendmail[BUFSIZE] ;	/* mail delivery daemon	*/


/* scratchpad buffer */
VAR char  syscom[2*BUFSIZE];
VAR char  s[2*BUFSIZE];
VAR char  s1[2*BUFSIZE];

 
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
VAR int	f_interactive ;		/* interactive ?			*/
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
char namelist[BUFSIZE] = DSMAILNAMES;
char mailopts[BUFSIZE] = DSMAILOPTS;
char maillist[BUFSIZE] = DSMAILLISTS;
#else
extern char namelist[];
extern char mailopts[];
extern char maillist[];
#endif

 

