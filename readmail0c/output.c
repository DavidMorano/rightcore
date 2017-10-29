/* output */

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



#include "defs.h"



/* function to divert a message */

output(messnum,command)
int messnum ;
char command[] ;
{
	FILE *ftmp ;

	char temp[100],last[100] ;
	char tmp[20] ;


/* open temporary file for meesage */

	strcpy(tmp,"/tmp/rdXXXXXX") ;

	mktemp(tmp) ;

	if ((ftmp = fopen(tmp,"w")) == NULL) {

	    fprintf(stderr,"Error: cannot open temporary file (%s)\n",tmp) ;
	    return ;
	}

	messnum = messord[messnum];        /* convert to internal number */
	fseek(curr.fp,messbeg[messnum],0) ;

#ifdef	COMMENT
/* skip to the "FROM:" line (sendmail).  
	     if none (UNIX mail), use last "From" line.
	  */

	fgets(temp,100,curr.fp) ;

	strcpy(last,temp) ;

	while (fgets(temp,100,curr.fp) != NULL) {

	    if ((strncmp(temp,"From",4) != 0)  &&
	        (strncmp(temp,">From",5) != 0) &&
	        (strncmp(temp,"FROM:",5) != 0)) break ;

	    strcpy (last,temp) ;

	}

	if (last[0] == '>')
	    fprintf(ftmp,"%s",&last[1]) ;

	else 
	    fprintf(ftmp,"%s",last) ;

	fprintf(ftmp,"%s",temp);	/* first line of message */
#endif

/* print out the message */

	while (ftell(curr.fp) < messend[messnum]) {

	    fgets(temp,100,curr.fp) ;

	    fprintf(ftmp,"%s",temp) ;

	}

	fclose(ftmp) ;

/* construct command line */

	sprintf(temp,"cat %s %s",tmp,command) ;

	usystem(temp) ;

/* remove temporary file */

	unlink(tmp) ;

}
/* end subroutine (output) */


