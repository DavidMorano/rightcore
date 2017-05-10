static char sccsid[] = "@(#)mailit.c	PCS 3.1" ;

/************************************************************************
 *									*
 * The information contained herein is for the use of AT&T Information	*
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	*
 *									*
 *	(c) 1984 AT&T Information Systems				*
 *									*
 * Authors of the contents of this file:				*
 *									*
 *		J.Mukerji						*
 *		A.M.Toto						*
 *									*
 
*									*
 * Change log								*
 *									*
 * 2/7/85 Changed mailerror messages to make them more reasonable (JM)	*
 *									*
 * 6/7/85 Changed the envelope to make it UNIX standard	(JM)		*

 
*
 *	int	mailit( tempfile, recip, dest_host )
 *	char *tempfile;
 *	struct table *recip;
 *	char *dest_host;
 *
 *	Mailit mails the contents of the file named in tempfile to
 * each addressee in the string of recipients specified by recips. 
 * It is assumed that the user field of each table entry holds a
 * valid address,
 * 	If it fails to send the mail, it sends mail back to
 * the originator of the message notifying him/her of the failure. It includes
 * a copy of the message that it failed to deliver.
 *	It appropriately updates the mail field of the table entry, goodmail
 *
 * NOTE: The local addresses NOUSER, NODOM are used internally, If any user
 * has a login which happens to be one of these words s/he should be 
 * encouraged to change it!


*****************************************************************************/



#include	<stdio.h>

#include	<string.h>

#include	<signal.h>

#include	<setjmp.h>

#include	"config.h"

#include	"smail.h"

#include	"header.h"



/* external data */

extern struct global	g ;


/* local variables */

static char command[2*BUFSIZE] ;



static char *mailcommand(a)
char	*a ;
{
	FILE	*fp ;

	char	line[BUFSIZE + 1] ;
	char	aa[BUFSIZE + 1] ;
	char	*cp ;


	if (strpbrk(a,":/=.") != NULL) {

	    strcpy(aa,a) ;

/* replace all slashes with colons */

	    while ((cp = strchr(aa,'/')) != NULL) *cp = ':' ;

	    sprintf(command,"uux - mtgzfs3!rmail '(%s)'",aa) ;

	} else if (strpbrk(a,"@%") != NULL)
	    sprintf(command,"uux - mtgzfs3!rmail '(%s)'",a) ;

	else if ((cp = strchr(a,'!')) != NULL) {

	    strcpy(aa,a) ;

	    aa[cp - a] = '\0' ;
	    if ((fp = popen("uuname","r")) == NULL) {

	        sprintf(command,"uux - mtgzfs3!rmail '(%s)'",a) ;

	    } else {

	        while ((l = fgetline(fp,line,BUFSIZE)) > 0) {

		    if (line[l - 1] == '\n') line[l - 1] = '\0' ;

	            if (strcmp(line,aa) == 0) break ;

	        }

	        if (line[0] != '\0') sprintf(command,
	            "uux - %s!rmail '(%s)'",aa,cp + 1) ;

	        else sprintf(command,
	            "uux - att!rmail '(%s)'",a) ;

	        pclose(fp) ;
	    }

	} else {

	    sprintf(command,"/bin/mail %s",a) ;

	}

	return command ;
}


int mailit( file, recipient, dest_host )
char	*file ;
struct table *recipient ;
char	*dest_host ;
{
	struct table *recip ;

	int		local ;
	int		localcnt, bufsize ;
	int		lenluser, lenladdr, tlenluser, tlenladdr ;

	char	envelope[256] ;
	char	*destsite ;
	char	laddr[BUFSIZE] ;
	char	luser[BUFSIZE] ;
	char	options[20] ;


#ifdef DEBUG
	fprintf(errlog, "mailit address - %s, file - %s\n", recipient, file) ;
#endif

	localcnt = 0 ;
	recip = recipient ;
	strcpy( laddr, recip->user ) ;
	bufsize = BUFSIZE - 15 ;

/*PAS-JM 2/8/85*/
/* remote mail */

	if (strcmp(dest_host, "LOC") != 0) {

	    char *fromusr ;

	    if( *fromaddr == '\0' ) fromusr = "daemon" ;

	    else if (( fromusr = strrchr( fromaddr, '!' )) == NULL)
	        fromusr = fromaddr ;

	    else fromusr++ ;

	    local = 0 ;
	    destsite = dest_host ;
	    sprintf(envelope, "From %s %s remote from %s\n", 
	        fromusr,stddatetime,thisys) ;

	} else {

/* local mail */

	    local = 1 ;
	    destsite = "" ;
	    *envelope = '\0' ;
	}

	*options = *luser = *laddr = '\0' ;

	lenladdr = lenluser = 0 ;
	*luser = *laddr = '\0' ;

/* now build the address string from the string of table entries */

	while (recip != NULL) {

	    if (recip->mail != 0) {

	        register char *cc ;

	        cc = recip->user ;
	        tlenladdr = lenladdr + strlen( cc ) ;

	        tlenluser = lenluser + strlen( recip->last ) ;

	        if (( tlenluser < bufsize-1 ) && ( tlenladdr < bufsize-1 )) {

	            strcpy( &laddr[lenladdr], cc ) ;

	            strcpy( &laddr[tlenladdr++], " " ) ;

	            strcpy( &luser[lenluser], recip->last ) ;

	            strcpy( &luser[tlenluser++], " " ) ;

	            localcnt++ ;
	            lenladdr = tlenladdr ;
	            lenluser = tlenluser ;

	        } else {

/* send this bunch out to the world */

	            if (sendit( file, destsite, laddr, luser, options, envelope ))
	                return -1 ;

	            lenladdr = lenluser = 0 ;
	            *luser = *laddr = '\0' ;
	            continue;	/* Fix the last addressee drop bug	*/
	        }

	    }
	    recip = recip->syslink ;
	}

	if (sendit( file, destsite, laddr, luser, options, envelope ))
	    return -1 ;

	goodmail += localcnt ;
	recip = recipient ;
	while (recip) {

	    recip->mail = 0 ;
	    recip = recip->syslink ;
	}

	return 0 ;
}


mailerror( addr, addressee, toaddr, file )
char	*addr, *addressee ;
char *toaddr, *file ;
{
	char	header[512] ;
	char	*mailcom ;

	sprintf(header,	"%s       MAILER@%s.%s\n", From, thisys,g.domain) ;

	sprintf(header+strlen(header),
	    "%s       %s\n",
	    Date, datetime) ;

	sprintf(header+strlen(header),
	    "%s    delivery failure\n", Subject) ;

	sprintf(header+strlen(header),
	    "%s     Unable to deliver\n",	/*2/7/85*/
	Status) ;

	sprintf(header+strlen(header),
	    "%s %s\n", Orecip, addr) ;

	sprintf(header+strlen(header),
	    "%s      %s\n", Odate, *date?date:datetime) ;

	if( *subject != '\0' )
	    sprintf(header+strlen(header),
	        "%s   %s\n", Osubj, subject) ;

#ifdef MESS_ID_FIRST
	if( *mess_id != '\0' )
	    sprintf(header+strlen(header),
	        "%s      %s\n", Confid, mess_id) ;
#endif

#ifdef MESS_ID_LAST
	if( *mess_id != '\0' )
	    sprintf(header+strlen(header),
	        "%s      %s\n", Confid, mess_id) ;
#endif

	sprintf(header+strlen(header),"%s      ", Reason) ;

	if( strncmp( addressee, "NODOM", 5 ) == 0)
	    sprintf(header+strlen(header),"Unknown domain") ;

	else if( strncmp( addressee, "NOMAIL", 6) == 0)
	    sprintf(header+strlen(header),"Unknown host or user") ;

	else if( strncmp( addressee, "NORES", 5 ) == 0)
	    sprintf(header+strlen(header),"System problems") ;

	else if( strncmp( addressee, "NOLINK", 6 ) == 0)
	    sprintf(header+strlen(header),
	        "No link available to addressed host") ;

	else if ( strncmp( addressee, "NOPOST", 6 ) == 0)
	    sprintf(header+strlen(header),
	        "bulletin board inaccessible") ;

	else if ( strncmp( addressee, "NONPOST", 7 ) == 0)
	    sprintf(header+strlen(header),"netnews system error") ;
	else
	    sprintf(header+strlen(header),"unknown addressee") ; /*2/7/85*/

	sprintf(header+strlen(header),
	    "\na copy of the undelivered message follows:\n") ;

	mailcom = mailcommand(toaddr) ;

	pipeit( mailcom, file, header, 1 ) ;

}


int sendit( file, destsite, destuser, destlast, options, envelope )
char *file, *destsite, *destuser, *destlast, *options, *envelope ;
{
	char *mailcom ;


	if ( *destlast == '\0' ) return 0 ;

	if (strncmp(destuser, "NO", 2) == 0) {

	    mailerror( destlast, destuser, fromaddr, file ) ;
	    return(1) ;
	}

	mailcom = mailcommand(destuser) ;

	if (*mailcom == '\0') {

	    mailerror( destlast, "NOLINK", fromaddr, file ) ;

	    return 1 ;
	}

#ifdef LOGFILE
	fprintf(log,"%d : %s\n",pid, mailcom) ;
#endif

	if (pipeit(mailcom,file, envelope, 0)!= 0) {

	    mailerror( destlast, "NORES", fromaddr, file ) ;
	    return(1) ;
	}
	return 0 ;
}


