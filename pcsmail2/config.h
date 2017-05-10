/* config */

/* 
	= David A.D. Morano, 94/01/06


*/


#define	VERSION		"3.2-beta"


#ifndef	PCS
#define	PCS	"/usr/add-on/pcs"
#endif

/* default environment variables 					*/
/* In the following define the default options and translation tables	*/
/* that are to be used. Any translation tables defined by the user will */
/* override these defaults.						*/

#define DSMAILOPTS "+standard"
#define DSMAILNAMES "/usr/add-on/pcs/lib/pcsmail/user.names:/usr/add-on/pcs/lib/pcsmail/other.names"
#define DSMAILLISTS "/usr/add-on/pcs/lib/pcsmail/user.lists:/usr/add-on/pcs/lib/pcsmail/other.lists"


/* end of default environment definitions */

#define LOGFILE		"log/pcsmail"
#define USERFILE	"log/pcsmail.users"
#define NUSERFILE	"log/pcsmail.nusers"
#define ERRFILE		"log/pcsmail.err"
#define HELPFILE	"lib/pcsmail/help"


/* sizes of data structures  DO NOT CHANGE LIGHTHEARTEDLY ! */

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	1024
#endif

#define	NAMEDB		18000	/* max num of names in translation directory*/
#define	MAXLIST		20      /* max mailling list directories 	    */
#define	MAXMBAGS	200	/* max number of mailbags		    */
#define	SYSNAMELEN	200	/* max length of system name		    */
#define	LENNAME		200	/* max length of untranslated name 	    */
#define	SYSLEN		4096 	/* max length of translated sysname 	    */
#define	BUFSIZE		8000	/* size of a general buffer 		    */
#define	BUFLEN		BUFSIZE
#define	LINELEN		200	/* typical line length */
#define	HVLEN		BUFSIZE


/* Message ID related stuff						*/

#define		MESS_ID_FIRST
/*#define	MESS_ID_LAST*/



/* definition of mail UID and mail GID */
/* set these to the appropriate numbers by refering to 
/* /etc/passwd & /etc/group on your machines */

#ifndef MAILGROUP
#define	MAILGROUP	6
#endif


/* prefix chars for distribution list names (bboard, standard and user) */
/* DO NOT CHANGE WITHOUT THOROUGHLY UNDERSTANDING THE CONSEQUENCES      */

#define	  NONMAIL	':'
#define   STANLIST      '_'
#define   ALTLIST       '~'
#define   ALIAS         '='



#define	PROG_SENDMAIL	"/usr/lib/sendmail"
#define	PROG_PCSCLEANUP	"pcscleanup"
#define	PROG_PCSCL	"pcscl"
#define	PROG_PAGER	"more"

#define	MAILHOST	"mtgbcs.lucent.com"
#define	MAILNODE	"mtgbcs"
#define LOCALDOMAIN	"mt.lucent.com"
#define COMPANYDOMAIN	"lucent.com"
#define MAILDOMAIN	"mt.lucent.com"
#define	ORGANIZATION	"Lucent Technologies"

#define	DEADFILE	"dead.letter"
#define	DEAD		DEADFILE




