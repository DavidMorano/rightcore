/************************************************************************
 *                                                                      
 * The information contained herein is for use of   AT&T Information    
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   
 *                                                                      
 *     (c) 1984 AT&T Information Systems                                
 *                                                                      
 * Authors of the contents of this file:                                
 *                                                                      
 *                      Bruce Schatz                                    
 *									

 ***********************************************************************/



#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>

#include	"config.h"
#include	"defs.h"



/* external variables */

extern struct global	g ;



/* this prints out short descriptions of the commands.
   assumes all relevant command names in HELPFILE are preceded
   by INDENT spaces
 */

#define  INDENT 2
#define  CMP(str1,str2)  (strncmp ((str1),(str2),strlen((str1)))  ==  0)



int help(command)
char	command[] ;
{
	FILE	*ftemp ;

	char	helpline[LINELEN + 1], *com ;
	char	helpfname[MAXPATHLEN + 1] ;
	

	sprintf(helpfname,"%s/%s",g.progroot,HELPFILE) ;

	ftemp = fopen(helpfname, "r") ;

	if (ftemp == NULL) {

	    printf("\n *** help file cannot be opened *** \n") ;

	    return 1 ;
	}

	com = strtok(command," ") ;

	if (com ==  NULL) {	/* all commands */

	    while (fgets (helpline,LINELEN,ftemp)  !=  NULL)
	        printf ("%s",helpline) ;

	} else {	/* specific command */

	    while (fgets (helpline,80,ftemp)  !=  NULL)

	        if (CMP (com,&helpline[INDENT])) {

	            printf("\n%s",helpline);   

	            goto closeout;  
	        }

	    printf("\n no help about \"%s\"\n",command) ;

	}

closeout:
	fclose (ftemp) ;

	return 0 ;
}
/* end subroutine (help) */



