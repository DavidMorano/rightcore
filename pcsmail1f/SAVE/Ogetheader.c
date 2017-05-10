/* get header */



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
 
*
 * getheader( file )
 *
 * getheader collects the header fields from the named file
 * and puts them in corresponding strings in the global data space


*************************************************************************/



#include	<stdio.h>

#include	"char.h"

#include	"misc.h"
#include	"config.h"
#include	"smail.h"
#include	"header.h"



int getheader(file, from)
char	*file, *from ;
{
	FILE *fp ;

	int htype ;

	char line[BUFSIZE] ;


/* open file for reading */

	if ((fp = fopen(file,"r" )) == '\0') return -1 ;

/* careful on the next line to check for EOF also */

	while ((fgetline(fp,line, BUFSIZE) > 0) && (*line != '\n')) {

	    htype = header_line(line) ;

	    switch (htype) {

	    case TITLE:
	    case SUBJECT:
	        if (*subject == '\0') {

	            strncpy(subject, hvalue,BUFSIZE) ;

	            subject[BUFSIZE-1] = '\0' ;
	        }
	        break ;

	    case KEYWORDS:
	        if (*keys == '\0')  {

	            strncpy(keys, hvalue,BUFSIZE) ;

	            keys[BUFSIZE-1] = '\0' ;
	        }
	        break ;

	    case REFERENCE:
	        if (*reference == '\0')  {

	            strncpy(reference, hvalue,BUFSIZE) ;

	            reference[BUFSIZE-1] = '\0' ;
	        }
	        break ;

	    case MESSAGE_ID:
	        if (*mess_id == '\0' || isfile) {

	            strncpy(mess_id, hvalue,SYSLEN) ;

	            mess_id[SYSLEN-1] = '\0' ;
	        }
	        break ;

	    case OPTIONS:
	        if (*moptions == '\0' || isfile) {

	            strncpy(moptions, hvalue,SYSLEN) ;

	            moptions[SYSLEN-1] = '\0' ;
	        }
	        break ;

	    case TO:
	        strncpy(realto, hvalue,BUFSIZE) ;

	        realto[BUFSIZE-1] = '\0' ;
	        if (*recipient == '\0')  {

	            strncpy( recipient, realto, BUFSIZE ) ;

	            recipient[BUFSIZE-1] = '\0' ;
	        }
	        break ;

	    case BCC:
	        if (*bcopyto == '\0')  {

	            strncpy( bcopyto, hvalue, BUFSIZE ) ;

	            bcopyto[BUFSIZE-1] = '\0' ;

	        }
	        break ;

	    case CC:
	        if (*copyto == '\0')  {

	            strncpy( copyto, hvalue, BUFSIZE ) ;

	            copyto[BUFSIZE-1] = '\0' ;
	        }
	        break ;

	    case FROM:
	        if ( *hvalue != '\0' ) {

	            strncpy( from, hvalue, BUFSIZE ) ;

	            from[BUFSIZE-1] = '\0' ;
	        }
	        break ;

	    case SENDER:
	        strncpy( sentby, hvalue, BUFSIZE ) ;

	        sentby[BUFSIZE-1] = '\0' ;
	        break ;

	    case DATE:
	        strcpy( date, hvalue ) ;

	    }
	}

	fclose(fp) ;
}
/* end subroutine */


