/* mailit */


#define	DEBUG	0


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
		David A.D. Morano
 *									*
 
*									*
 * Change log								*
 *									*
 * 2/7/85 Changed mailerror messages to make them more reasonable (JM)	*
 *									*
 * 6/7/85 Changed the envelope to make it UNIX standard	(JM)		*

 
*
 *	int	mailit( tempfile, recip, dest_host )
 *	char *tempfile ;
 *	struct table *recip ;
 *	char *dest_host ;
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


**************************************************************************/



#include	<stdio.h>
#include	<string.h>
#include	<signal.h>
#include	<setjmp.h>

#include	<baops.h>

#include	"config.h"
#include	"smail.h"
#include	"header.h"
#include	"localmisc.h"



/* external subroutines */

extern struct passwd	*getpwnam() ;

extern int		logfile_printf() ;


/* local subroutines */

static char	*mailcommand() ;


/* external data */

extern struct global	g ;


/* local data */

static char	command[3*BUFSIZE] ;



int mailit(file,recipient,dest_host)
char		*file ;
struct table	*recipient ;
char		*dest_host ;
{
	struct table	*recip ;

	int		localcnt, bufsize ;
	int		lenllast, lenladdr, tlenllast, tlenladdr ;
	int		f_local = FALSE ;

	char	envelope[256] ;
	char	*destsite ;
	char	laddr[BUFSIZE] ;
	char	luser[BUFSIZE] ;
	char	options[20] ;


#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG))
		fprintf(stderr,"mailit was called\n") ;
#endif

	localcnt = 0 ;
	recip = recipient ;
	strcpy( laddr, recip->mailaddress) ;

	bufsize = MIN((BUFSIZE - 15),200) ;
	if (strcmp(dest_host, "LOCAL") != 0) {

	    char	*fromusr ;


/* remote mail */

	    if( *fromaddr == '\0' ) fromusr = "daemon" ;

	    else if (( fromusr = strrchr( fromaddr, '!' )) == NULL)
	        fromusr = fromaddr ;

	    else fromusr++ ;

	    destsite = dest_host ;
	    sprintf(envelope, "From %s %s remote from %s\n", 
	        fromusr,stddatetime,g.nodename) ;

	} else {

/* local mail */

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG))
		errprintf("local mail \n") ;
#endif

	f_local = TRUE ;
	    destsite = "" ;
	    *envelope = '\0' ;
	}

	*options = *luser = *laddr = '\0' ;

	lenladdr = lenllast = 0 ;
	*luser = *laddr = '\0' ;

/* now build the address string from the string of table entries */

	while (recip != NULL) {

	    if (recip->mail != 0) {

		int	lu, ll ;


		lu = strlen(recip->mailaddress) ;

		ll = strlen(recip->realname) ;

	        if (((lenladdr + lu) < (bufsize - 1)) && 
			((lenllast + ll) < (bufsize - 1))) {

	if (! f_local) {

		lenladdr += sprintf(laddr + lenladdr," \"%s\"",
			recip->mailaddress) ;

		lenllast += sprintf(luser + lenllast," \"%s\"",
			recip->realname) ;

	} else {

		lenladdr += sprintf(laddr + lenladdr,"%s",
			recip->mailaddress) ;

		lenllast += sprintf(luser + lenllast,"%s",
			recip->realname) ;

	}

	            localcnt += 1 ;

	        } else {

/* send this bunch out to the world */

	            if (sendit(file,destsite,laddr,luser,options,envelope))
	                return -1 ;

	            lenladdr = lenllast = 0 ;
	            *luser = *laddr = '\0' ;
	            continue ;	/* fix the last addressee drop bug	*/
	        }

	    } /* end if */

	    recip = recip->syslink ;

	} /* end while */

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
/* end subroutine */


int sendit( file, destsite, destuser, destlast, options, envelope )
char	*file, *destsite, *destuser, *destlast, *options, *envelope ;
{
	char	*mailcom ;


	if ( *destlast == '\0' ) return 0 ;

	if (strncmp(destuser, "NO", 2) == 0) {

	    mailerror( destlast, destuser, fromaddr, file ) ;

#ifdef LOGFILE
	logfile_printf("mail_error to address \"%s\"\n", destlast) ;
#endif

	    return 1 ;
	}

	mailcom = mailcommand(destuser) ;

	if (*mailcom == '\0') {

	    mailerror( destlast, "NOLINK", fromaddr, file ) ;

#ifdef LOGFILE
	logfile_printf("mail_error to address \"%s\"\n", destlast) ;
#endif

	    return 1 ;
	}

#ifdef LOGFILE
	logfile_printf("D=[%s]\n", mailcom) ;
#endif

	if (pipeit(mailcom,file, envelope, 0) != 0) {

	    mailerror( destlast, "NORES", fromaddr, file ) ;

#ifdef LOGFILE
	logfile_printf("mail_error to address \"%s\"\n", destlast) ;
#endif

	    return 1 ;
	}

	return 0 ;
}
/* end subroutine (sendit) */


mailerror( addr, addressee, toaddr, file )
char	*addr, *addressee ;
char	*toaddr, *file ;
{
	char	header[512] ;
	char	*mailcom ;

	sprintf(header,"From: MAILER@%s.%s\n", 
		g.nodename,g.domain) ;

	sprintf(header+strlen(header),
	    "%s       %s\n",
	    Date, datetime) ;

	sprintf(header+strlen(header),
	    "Subject:	delivery failure\n") ;

#ifdef	COMMENT
/* 02/07/85 */

	sprintf(header+strlen(header),
	    "Status:	Unable to deliver\n") ;
#endif

	sprintf(header+strlen(header),
	    "Original-recipient: %s\n", addr) ;

	sprintf(header+strlen(header),
	    "Original-Date: %s\n", *date?date:datetime) ;

	if (*subject != '\0' )
	    sprintf(header+strlen(header),
	        "Original-subject: %s\n", subject) ;

#ifdef	COMMENT

#ifdef MESS_ID_FIRST
	if (*mess_id != '\0' )
	    sprintf(header+strlen(header),
	        "%s      %s\n", Confid, mess_id) ;
#endif

#ifdef MESS_ID_LAST
	if (*mess_id != '\0' )
	    sprintf(header+strlen(header),
	        "%s      %s\n", Confid, mess_id) ;
#endif

#endif

	sprintf(header+strlen(header),"\nReason for failure: ") ;

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
	    sprintf(header+strlen(header),"unknown addressee") ;/*2/7/85*/

	sprintf(header+strlen(header),
	    "\na copy of the undelivered message follows:\n") ;

	mailcom = mailcommand(toaddr) ;

	pipeit(mailcom, file, header, 1) ;

}
/* end subroutine (mailerr) */


static char *mailcommand(recipients)
char  *recipients ;
{

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG))
	errprintf("recip \"%s\"\n",recipients) ;
#endif

	if ((strpbrk(recipients," \t/:%&!=") == NULL) &&
		(getpwnam(recipients) != NULL))
		sprintf(command,"/bin/mail %s",
			recipients) ;

	else
	sprintf(command, "%s %s", 
	    SENDMAIL,recipients) ;

	return command ;
}
/* end subroutine (mailcommand) */


