/* config */

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
 ************************************************************************/



#define	VERSION		"3.0g"


#ifndef	PCS
#define	PCS	"/usr/add-on/pcs"
#endif

/* default environment variables 					*/
/* In the following define the default options and translation tables	*/
/* that are to be used. Any translation tables defined by the user will */
/* override these defaults.						*/

#define DSMAILOPTS "+standard"
#define DSMAILNAMES "/usr/add-on/pcs/lib/sendmail/user.names:/usr/add-on/pcs/lib/sendmail/other.names"
#define DSMAILLISTS "/usr/add-on/pcs/lib/sendmail/user.lists:/usr/add-on/pcs/lib/sendmail/other.lists"

/* end of default environment definitions */

#define LOGFILE		"/usr/add-on/pcs/spoolpcs/log/pcsmail"
#define ERRFILE		"/usr/add-on/pcs/spoolpcs/log/pcsmailerr"


/* sizes of data structures  DO NOT CHANGE LIGHTHEARTEDLY ! */

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	1024
#endif

#define	NAMEDB	4000   	/* max num of names in translation directory*/
#define	MAXLIST       20      /* max mailling list directories 	    */
#define	MAXMBAGS	200	/* max number of mailbags		    */
#define	SYSNAME	200	/* max length of system name		    */
#define	LENNAME	200	/* max length of untranslated name 	    */
#define	SYSLEN	500 	/* max length of translated sysname 	    */
#define	BUFSIZE	8000	/* size of a general buffer 		    */
#define	BUFLEN	BUFSIZE
#define	LINELEN		200	/* typical line length */


/* Message ID related stuff						*/

#define		MESS_ID_FIRST
/*#define	MESS_ID_LAST*/



/* definition of mail UID and mail GID */
/* set these to the appropriate numbers by refering to 
/* /etc/passwd & /etc/group on your machines */

#ifndef MAILGROUP
#define	MAILGROUP	5
#endif

#ifndef MAILUSER
#define MAILUSER	5
#endif

/* prefix chars for distribution list names (bboard, standard and user) */
/* DO NOT CHANGE WITHOUT THOROUGHLY UNDERSTANDING THE CONSEQUENCES      */

#define	  NONMAIL	':'
#define   STANLIST      '_'
#define   ALTLIST       '~'
#define   ALIAS         '='



#define	SENDMAIL	"/usr/lib/sendmail"
#define	MAILNAME	"mtgbcs.att.com"
#define	MAILHOST	"mtgbcs"
#define DOMAIN		"mt.att.com"
#define	DEAD		"dead.letter"


#define	TESTING		"SMAIL test"



