/* fappend */


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
 *		J.Mukerji						
 *									
 
*
  fappend(file1,file2) is equivalent to the shell script 
	cat file1 >> file2

  If file1 or file2
  is not accessible then this is a NOP


*************************************************************************/



#include	<sys/types.h>
#include	<stdio.h>

#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



int fappend(file1,file2)
char *file1,*file2;
{
    FILE	*fd1, *fd2;

    char	templine[BUFSIZE];


    if ((fd1 = fopen(file1, "r")) == NULL) return BAD ;

    if ((fd2 = fopen(file2, "a")) == NULL) {

	fclose(fd1);

	return BAD ;
    }

    while (fgetline(fd1,templine, BUFSIZE) > 0) {

	if (! strncmp(templine, "From", 4)) fputs( ">", fd2 );

    	fputs(templine, fd2);

    }

    if (templine[0] != '\n') fprintf(fd2, "\n" );

    fclose(fd1);

    fclose(fd2);

	return OK ;
}
/* end subroutine (fappend) */



