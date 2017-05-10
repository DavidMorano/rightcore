/* config */


/****************************************************************************

	this is the global header file which is included into every 
	source file.   the declarations here are defined in "defs.c".


******************************************************************************/


#define	VERSION		"0"
#define	WHATINFO	"@(#)MOVEMAIL "
#define	BANNER		"Move Mail"

#ifndef	PCS
#define	PCS		"/usr/add-on/pcs"
#endif

#ifndef	LOGFILE
#define	LOGFILE		"log/movemail"
#endif

#ifndef	USERFILE
#define	USERFILE	"log/movemail.users"
#endif

#ifndef	HELPFILE
#define	HELPFILE	"lib/movemail/help"
#endif

#define	MAGIC		"PCS_INDEX"


/* programs */

#define	PROG_GETMAIL	"getmail"
#define	PROG_EDITOR	"vi"
#define	PROG_MAILER	"pcsmail"
#define	PROG_METAMAIL	"metamail"


/* mail spool area stuff */

#define MAILDNAME	"/var/mail"
#define	MAILLOCKAGE	(5 * 60)


/* mail host stuff */

#define	MAILHOST	"mtgbcs.mt.lucent.com"
#define	ORGDOMAIN	"lucent.com"
#define	ORGANIZATION	"Lucent Technologies"


/* local system stuff */

#ifndef MAILGROUP
#define MAILGROUP	6
#endif


#define	DEFMAILBOX	"new"
#define	DEFMBOX		"mail/new"

#define	TMPDIR		"/tmp"

 

