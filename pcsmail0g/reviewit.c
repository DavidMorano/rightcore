/* reviewit */


#define		DEBUG	0


/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
 *		T.S.Kennedy						
 *		J.Mukerji						
 *									
 

 * reviewit() does all the footwork for reviewing the message


****************************************************************************/



#include	<string.h>
#include	<stdio.h>

#include	"config.h"
#include	"smail.h"
#include	"header.h"



reviewit(option)
char	*option ;
{


	getfield(tempfile,HS_FROM,syscom) ;

	if(strcmp(from,syscom)) {

	    if( *sentby == '\0' )
	        strcpy( sentby, from ) ;

	    strcpy( from, syscom ) ;
	}

	if((option = strtok(0,"")) != NULL) {

	    if(*option == '|' || *option == '>') {

	        if(isappend && isforward)
	            sprintf(syscom,"cat %s %s %s %s",
	                tempfile,appfile,forwfile,option) ;

	        else if(isappend)
	            sprintf(syscom,"cat %s %s %s",
	                tempfile,appfile,option) ;

	        else if(isforward)
	            sprintf(syscom,"cat %s %s %s",
	                tempfile,forwfile,option) ;

	        else sprintf(syscom,"cat %s %s", 
	            tempfile,option) ;

	    } else {

	        printf("unknown command -- please try again.") ;

	        return ;
	    }

	} else {

	    if (isappend && isforward)
	        sprintf(syscom,"cat %s %s %s",
	            tempfile,appfile,forwfile) ;

	    else if(isappend)
	        sprintf(syscom,"cat %s %s",
	            tempfile,appfile) ;

	    else if(isforward)
	        sprintf(syscom,"cat %s %s",
	            tempfile,forwfile) ;

	    else 
	        sprintf(syscom,"cat %s", tempfile) ;

	    putc('\n',stdout) ;
	}

	system(syscom) ;

}
/* end subroutine (reviewit) */


