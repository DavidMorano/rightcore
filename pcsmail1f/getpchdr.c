/* getpchdr */


#define	CF_DEBUGS	0		/* compile-time debugging */


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

 * Function getpcheader() scans a command line assuming it is a 'pc'
 * command line. A 'pc' command line looks like:
 *
 *    pc [+/-not] [+/-del] [+/-del] recipient word1 word2 ... wordn
 *
 * After scanning the line it puts recipient in the recipient string,
 * sets up the options as specified, and puts the string word1 word2 
 * ... wordn in the subject string, sets message="  ", and finally
 * forks off the rest of sendmail in the background


*******************************************************************************/


#include	<envstandards.h>

#include	<string.h>
#include	<signal.h>
#include	<stdio.h>

#include	<logfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"header.h"



VOID getpcheader(argv)
char	*argv[];
{
	int oval;
	int wordcount;

	char *c;


	wordcount = 0;
	*recipient = '\0';
	argv++;
	for ( c = *argv++; c != NULL; c = *argv++ ) {

		oval = 0;
		switch (*c++) {

/* see if it is one of the three options */
		case '-': 
			oval = -1;

		case '+': 
			oval++;
			switch (*c) {

			case 'n': /* notify */
				break;

			case 'v': /* verify */
				verify = oval;
				break;

			case 'd': /* delivery */
				break;

			default:
				printf("unknown option '%s' ignored\n",
				 --c);

			}
			break;

/* The first word is the recipient */
/* all subsequent words go into subject */
		default: 
			c--;
			if (! wordcount++) {

				strcpy( recipient, c );

			} else {

				sprintf( subject+strlen(subject), "%s ", c );
			}
		}
	}

	while( *recipient == '\0' ) {

		prompt(TO);
	}

	while( *subject == '\0' ) {

		prompt(SUBJECT);
	}
	iswait = 1;
	strcpy( message, "  " );

	setsig( SIG_IGN );

	if (fork() != 0)  exit(0) ;

	runcmd = YES ;
	return ;
}
/* end subroutine (getpchdr) */


VOID pcusage()
{
	printf("%s: usage %s [+/-del] [+/-not] [+/-ver] name message\n",
		comm_name, comm_name);
}


