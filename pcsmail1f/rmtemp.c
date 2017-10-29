/* rmtemp */

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
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<signal.h>
#include	<setjmp.h>
#include	<stdio.h>

#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* external subroutines */


/* extern variables */

extern struct global	g ;

	extern	FILE	*fp;

	extern char tempfile[];
	extern char forwfile[];

void	(*setsig())();





void rmtemp(f_bad,s)
int	f_bad ;
char	*s ;
{
	int	j ;

	char	command[(2 * MAXPATHLEN) + 1] ;


#ifdef	COMMENT
	logfile_printf(&g.lh,"rmtemp: entered from \"%s\"\n",s) ;
#endif

	logfile_printf(&g.eh,"rmtemp: entered from \"%s\"\n",s) ;

/* first ignore any further signals */

	setsig(SIG_IGN) ;

/* tempfile exists, so save it and notify the user */

	if (f_bad && (tempfile[0] != '\0') &&
		(access(tempfile,R_OK) >= 0)) {

		sprintf(command, "cat %s >> %s/%s", 
			tempfile, g.homedir,DEADFILE);

	logfile_printf(&g.eh,"rmtemp: SYSTEM> %s\n",command) ;

		system( command );

		bprintf(g.efp,
	"\nmessage saved in '%s' in your home directory\n",
	DEADFILE) ;

	}

	unlink(tempfile) ;

	if (*forwfile != '\0') unlink(forwfile);

	if (iswait) return ;

	return ;
}
/* end subroutine (rmtemp) */


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



