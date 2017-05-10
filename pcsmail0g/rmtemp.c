static char sccsid[] = "@(#)rmtemp.c	PCS 3.0";

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
 * Changes:								
 *									
 * 2/14/85 type anomalies in signal types fixed by Paul Sherman		
 *									

 ***********************************************************************/



#include	<sys/types.h>

#include	<signal.h>

#include	<stdio.h>

#include	<setjmp.h>

#include	"config.h"

#include	"smail.h"



	extern	FILE	*fp;

	extern char tempfile[];
	extern char forwfile[];
	extern char uuname[];

void (*setsig())();


void rmtemp(i)
int i;
{
	int	j;

	char	command[200] ;


/* first ignore any further signals */

	setsig(SIG_IGN) ;

/* tempfile exists, so save it and notify the user */

	if ((*tempfile != '\0') && 
		((j = open(tempfile,0)) >= 0) && (i != 0)) {

		if (fp != ((FILE *) 0)) fflush(fp);

		sprintf(command, "cat %s >> %s/%s", 
			tempfile, getenv("HOME"),DEAD);

		system( command );

		close(j);

		printf(
"\nmessage saved in '%s' in your home directory\n",DEAD);

	}

	unlink(tempfile);

	if (*forwfile != '\0') unlink(forwfile);

	if (*uuname != '\0') unlink(uuname);

	if (iswait) return ;

	fclose(stdout) ;

	fclose(stderr) ;

	if (i != 0) exit(1) ;

	exit(0) ;
}


/* setsig set all signals to the type specified by sig_type and returns
 * the previous value of the last signal. NOTE: the returned value is the
 * previous value of the LAST signal and not SIGHUP or SIGQUIT!
*/

void (*setsig( sig_type ))()
void	(*sig_type)();
{
	void	(*j)();


	j = signal( SIGQUIT, sig_type );

	j = signal( SIGPIPE, sig_type );

	j = signal( SIGALRM, sig_type );

	j = signal( SIGTERM, sig_type );

	j = signal( SIGHUP, sig_type );

	j = signal( SIGINT, sig_type );

	return j ;
}


