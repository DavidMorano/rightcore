/* reviewit */


#define	CF_DEBUG	0


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



#include	<sys/types.h>
#include	<string.h>
#include	<stdio.h>

#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"header.h"



/* external subroutines */


/* external cariables */

struct global		g ;




void reviewit(option)
char	*option ;
{
	char	hv_from[HVLEN + 1] ;


	getfield(tempfile,HS_FROM,hv_from) ;

	if (strcmp(from,hv_from)) {

	    if ( *sentby == '\0' )
	        strcpy( sentby, from ) ;

	    strcpy( from, hv_from) ;

	}

#ifdef	COMMENT
	if ((option = strtok(0,"")) != NULL) {

	    if ((*option == '|') || (*option == '>')) {

	        if (isappend && isforward)
	            sprintf(syscom,"cat %s %s %s %s",
	                tempfile,appfile,forwfile,option) ;

	        else if (isappend)
	            sprintf(syscom,"cat %s %s %s",
	                tempfile,appfile,option) ;

	        else if (isforward)
	            sprintf(syscom,"cat %s %s %s",
	                tempfile,forwfile,option) ;

	        else sprintf(syscom,"cat %s %s", 
	            tempfile,option) ;

	    } else {

	        bprintf(g.ofp,"unknown command -- please try again") ;

	        return ;
	    }

	} else {

	    if (isappend && isforward)
	        sprintf(syscom,"cat %s %s %s",
	            tempfile,appfile,forwfile) ;

	    else if (isappend)
	        sprintf(syscom,"cat %s %s",
	            tempfile,appfile) ;

	    else if (isforward)
	        sprintf(syscom,"cat %s %s",
	            tempfile,forwfile) ;

	    else 
	        sprintf(syscom,"cat %s", tempfile) ;

	    bputc(g.ofp,'\n') ;
	}
#else
	    if (isappend && isforward)
	        sprintf(syscom,"cat %s %s %s",
	            tempfile,appfile,forwfile) ;

	    else if (isappend)
	        sprintf(syscom,"cat %s %s",
	            tempfile,appfile) ;

	    else if (isforward)
	        sprintf(syscom,"cat %s %s",
	            tempfile,forwfile) ;

	    else 
	        sprintf(syscom,"cat %s", tempfile) ;

	if (g.prog_pager != NULL) {

		strcat(syscom," | ") ;

		strcat(syscom,g.prog_pager) ;

	}

	    bputc(g.ofp,'\n') ;

#endif

#if	CF_DEBUG
	logfile_printf(&g.eh,"reviewit: SYSTEM> %s\n",syscom) ;
#endif

	system(syscom) ;

}
/* end subroutine (reviewit) */



