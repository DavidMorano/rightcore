/* putheader */


#define	CF_DEBUG	0


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
		David A.D. Morano
 *									*


 * putheader( fp, addbcc )
 *
 * putheader outputs a standard header for the message into the given open
 * file. If addbcc is 1 then a BCC line is also added to the header.


*************************************************************************/


#include	<sys/types.h>
#include	<string.h>
#include	<stdio.h>

#include	<baops.h>
#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* external variables */

extern struct global	g ;



#define initheader()		c=s

#define addtoheader( string )	for( d = string;*c++ = *d++;);c--

#define saveheader() 		*c++ = '\n'; *c++ = '\n'; *c = '\0';\
				fputs(s,fp)


void putheader( fp, addbcc )
FILE	*fp ;
char	addbcc ;
{
	char *c, *d;
	char	*cp ;

	initheader();

#ifdef MESS_ID_FIRST
	if (!addbcc && *mess_id != '\0') {

		addtoheader("Message-Id: ");
		addtoheader(mess_id);
		addtoheader("\n");

	} if (!addbcc && *reference != '\0') {

		addtoheader("References:  ");
		addtoheader(reference);
		addtoheader("\n");
	}
#endif

 	if ( *from != '\0' ) {

 		addtoheader("From:       ");
 		addtoheader(from);
 		addtoheader("\n");

 	} else if( *sentby != '\0' ) {

 		addtoheader("From:       ");
 		addtoheader(sentby);
		strcpy(from,sentby);
 		addtoheader("\n");
 	}

	if ((*sentby != '\0') && ( strcmp( from, sentby ) != 0 )) {

		addtoheader("Sender:     ");

		addtoheader(sentby);

		addtoheader("\n");

	}

	addtoheader("To:         ");

	if (*realto != '\0') {

		addtoheader(realto);

	} else {

		addtoheader(recipient);
	}

	if (*copyto != '\0') {

		addtoheader("\nCc:         ");
		addtoheader(copyto);
	}

	if (addbcc && *bcopyto != '\0') {

		addtoheader("\nBcc:        ");
		addtoheader(bcopyto);
	}

	if (*moptions != '\0') {

		addtoheader("\nOptions:    ");
		addtoheader(moptions);
	}

#ifdef MESS_ID_LAST
	if (!addbcc && *mess_id != '\0') {

		addtoheader("\nMessage-Id: ");
		addtoheader(mess_id);
	}
#endif

	if ((g.msgdate != NULL) && (g.msgdate[0] != '\0')) {

		addtoheader("\nDate:       ");
		addtoheader(g.msgdate);

	}

	if (*subject != '\0') {

		addtoheader("\nSubject:    ");

		cp = subject ;
		while (ISWHITE(*cp)) cp += 1 ;

		addtoheader(cp) ;

	}

	if (*keys != '\0') {

		addtoheader("\nKeywords:   ");
		addtoheader(keys);
	}

	if (f_fullname && (g.fullname != ((char *) 0))) {

		addtoheader("\nFull-Name:  ");
		addtoheader(g.fullname) ;
	}

#ifndef MESS_ID_FIRST
	if(!addbcc && *reference != '\0') {

		addtoheader("\nReferences:  ");
		addtoheader(reference);
	}
#endif MESS_ID_FIRST

#ifdef	COMMENT
		addtoheader("\nX-Mailer:   AT&T PCS 3.0a") ;
#endif

	saveheader();

}
/* end subroutine (putheader) */


