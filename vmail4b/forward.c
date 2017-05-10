/* forward */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We invoke SMAIL, appending specified message onto the entered text and
        then forwards the entire composite message.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<signal.h>
#include	<string.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<curses.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* external variables */


/* exported subroutines */


int forward(pip,mn, f_edit)
struct proginfo	*pip ;
int	mn, f_edit ;
{
	FILE	*fout ;

	int	len ;

	char	temp[LINEBUFLEN + 1], last[LINEBUFLEN + 1] ;
	char	filename[LINEBUFLEN + 1], command[LINEBUFLEN + 1] ;
	char	string[LINEBUFLEN + 1], from[LINEBUFLEN + 1] ;


	mn = messord[mn] ;     /* convert to internal number */

/* 
	The idea there is to 
	makeup filename and delete any previous ones.
	   We can't delete the file after mail send in the program because SMAIL
	   forks off the delivery process.

	   Actually the forked off portion of SMAIL can indeed delete 
	   the file after sending it into the appropriate UNIX mail program !
	   Indeed that is what it does now !
*/

	strcpy(filename,"/tmp/forwardXXXXXX") ;

	mktemp(filename) ;

	if ((fout = fopen(filename,"w")) == NULL)
		return BAD ;

/*jm*/	chmod(filename, 0660) ;		/* don't let the rest of */
/* the world read the forwarded mail!!   */

/*jm*/	chown(filename,getuid(), pip->gid_mail) ;

/* prepend the forwarded message text */

/* get the "from" person */

	if (fetchfield(mn,"FROM:",string,LINEBUFLEN) < 0)
		string[0] = '\0' ;

	if ((*string == '\0') || (strncmp(string,"   ",3) == 0))
	    fetchfrom(pip,mn,string,LINEBUFLEN) ;   /* sent by UNIX mail */

	fixfrom(pip,string,from,LINEBUFLEN) ;

/* put it all together */

	fprintf(fout,"%s writes :\n",from) ;

	fprintf(fout,"-------------------") ;

	if (f_edit) 
		fprintf(fout," begin edited_forwarded message ") ;

	else 
		fprintf(fout,"---- begin forwarded message ---") ;

	fprintf(fout,"-------------------\n") ;

/* seek to the start of the current message that was being read */

	fseek(curr.fp,messbeg[mn],0) ;

/* 
	Here we read a line hoping that it is the UNIX envelope header.
	Skip to the "FROM:" line (like from SMAIL).  
	If none (UNIX mail), use last "From" line.
*/

	freadline(curr.fp,temp,LINEBUFLEN) ;

#ifdef	COMMENT
	strcpy (last,temp);

	while (freadline(curr.fp,temp,LINEBUFLEN) > 0) {

		if ( (strncmp(temp,"From",4) != 0)  &&
		     (strncmp(temp,">From",5) != 0) &&
		     (strncmp(temp,"FROM:",5) != 0))         break;
		strcpy (last,temp) ;		
	}

	fprintf(fout,">%s",last);

	fprintf(fout,"%s",temp) ;	/* first line of  */
#endif

/* write out the message */

	while (ftell(curr.fp) < messend[mn]) {

	    if ((len = freadline(curr.fp,temp,LINEBUFLEN)) > 0) {

		if (temp[len - 1] != '\n') temp[len++] = '\n' ;

		temp[len] = '\0' ;

	    	fprintf(fout,">%s",temp) ;

	    }

	} /* end while */

	fprintf(fout,"-------------------") ;

	fprintf(fout,"--- end of forwarded message ---") ;

	fprintf(fout,"-------------------\n") ;

	fclose(fout) ;

/* invoke PCSMAIL */

	sprintf(command,"%s -verify %s=%s",
	    pip->prog_mailer,(f_edit ? "eappend" : "dappend"),filename) ;

	system(command) ;

	u_unlink(filename) ;		/* still just in case!!! */

	return OK ;
}
/* end subroutine (forward) */



