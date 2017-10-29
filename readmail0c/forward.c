/* forward */


#define	CF_DEBUG	0


/************************************************************************
 *                                                                      
 * The information contained herein is for use of   AT&T Information    
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   
 *                                                                      
 *     (c) 1984 AT&T Information Systems                                
 *                                                                      
 * Authors of the contents of this file:                                
 *                                                                      
 *                      Bruce Schatz, Jishnu Mukerji                    
		David A.D. Morano, 96/06/19
 *									

 ***********************************************************************/



#include	<bfile.h>
#include	<logfile.h>

#include	"config.h"
#include	"defs.h"
#include	"localmisc.h"



/* external variables */

extern struct global	g ;

extern int		errno ;




/* invokes PCS SMAIL appending specified message onto the entered message.
  the forwarded message may be reviewed but not edited.
*/


int forward(messnum, f_edit)
int	messnum, f_edit ;
{
	FILE	*fout ;

	int	rs ;

	char	temp[LINELEN + 1], last[LINELEN + 1] ;
	char	filename[LINELEN + 1], command[(2 * LINELEN) + 1] ;


	messnum = messord[messnum];      /* convert to internal number */

/* makeup filename and delete any previous ones.
	   can't delete after mail send in the program because sendmail
	   forks off the delivery process.

	   Actually the forked off portion of sendmail can indeed delete 
	   the file after cating it into the appropriate UNIX mail program!
	   Indeed that is what it does now!
         */

	strcpy(filename,"/tmp/forwardXXXXXX") ;

	mktemp(filename) ;

	    unlink(filename) ;	/* delete it for fun anyway (if we can) */

	if ((fout = fopen(filename,"w")) == NULL) {

	    unlink(filename) ;

	    printf("readmail: could not open TMP file for FORWARD\n") ;

	    return BAD ;
	}

/*jm*/	chmod( filename, 0660 );		/* Don't let the rest of */
/* the world read the forwarded mail!!   */
/*jm*/	chown( filename, getuid(), MAILGROUP ) ;

	fprintf(fout,"\n-----------------") ;

	if (f_edit)
	    fprintf (fout," begin edited forwarded message ") ;

	else	
	    fprintf (fout,"--- begin forwarded message ----") ;

	fprintf(fout,"----------------\n") ;

	fseek(curr.fp,messbeg[messnum],0) ;

/* skip to the "FROM:" line (sendmail).  
	     if none (UNIX mail), use last "From" line.
	  */

	fgets(temp,LINELEN,curr.fp) ;

	strcpy(last,temp) ;

	while (fgets(temp,LINELEN,curr.fp) != NULL) {

	    if ((strncmp(temp,"From",4) != 0)  &&
	        (strncmp(temp,">From",5) != 0) &&
	        (strncmp(temp,"FROM:",5) != 0)) break ;

	    strcpy(last,temp) ;

	} /* end while */

	if (last[0] == 'F') fprintf(fout,">") ;

	fprintf (fout,"%s",last) ;

	fprintf(fout,"%s",temp);	/* first line of message */

/* write out the message */

	while (ftell(curr.fp) < messend[messnum]) {

	    fgets(temp,LINELEN,curr.fp) ;

	    if(!strncmp(temp,"From",4) || !strncmp(temp,"CC:",3))
	        fprintf(fout,">") ;

	    fprintf(fout,"%s",temp) ;

	} /* end while */

	fprintf(fout,"\n--------------------") ;

	fprintf(fout," end of forwarded message ") ;

	fprintf(fout,"--------------------\n") ;

	fclose(fout) ;

/* invoke sendmail */

#if	CF_DEBUG
	logfile_printf(&g.lh,"forward edit=%d\n",f_edit) ;
#endif

	sprintf(command,"%s %s=%s",
	    g.prog_mailer,f_edit ? "eappend":"dappend",filename) ;

	ssystem(command) ;

	if ((rs = fork()) == 0) {

		sleep(15) ;

		unlink(filename) ;

		exit(0) ;

	} else if (rs < 0) {
	
		logfile_printf(&g.lh,
		"forward fork failed (errno %d)\n",errno) ;

		sleep(5) ;

		unlink(filename) ;

	}

	return OK ;
}
/* end subroutine (forward) */


