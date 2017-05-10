/************************************************************************
 *                                                                      *
 * The information contained herein is for use of   AT&T Information    *
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   *
 *                                                                      *
 *     (c) 1984 AT&T Information Systems                                *
 *                                                                      *
 * Authors of the contents of this file:                                *
 *                                                                      *
 *                      Bruce Schatz, Jishnu Mukerji                    *
 *									*
 ***********************************************************************/
#include "defs.h"

/* invokes sendmail appending specified message onto the entered message.
  the forwarded message may be reviewed but not edited.
*/

forward (messnum, edit)
  int messnum, edit;
{
   	char temp[LINELEN],last[LINELEN],filename[LINELEN],command[LINELEN];
   	FILE *fout;

	messnum = messord [messnum];      /* convert to internal number */

	 /* makeup filename and delete any previous ones.
	   can't delete after mail send in the program because sendmail
	   forks off the delivery process.

	   Actually the forked off portion of sendmail can indeed delete 
	   the file after cating it into the appropriate UNIX mail program!
	   Indeed that is what it does now!
         */
	strcpy (filename,"/tmp/forwardXXXXXX");
	mktemp (filename);
	unlink (filename);	/* Still just in case!!! */

	fout =  fopen (filename,"w");
/*jm*/	chmod( filename, 0660 );		/* Don't let the rest of */
				/* the world read the forwarded mail!!   */
/*jm*/	chown( filename, getuid(), MAILGROUP );

	fprintf (fout,"\n-----------------");
	if (edit)
		fprintf (fout," begin edited forwarded message ");
	else	fprintf (fout,"--- begin forwarded message ----");
	fprintf (fout,"----------------\n");

	fseek(curr.fp,messbeg[messnum],0);

	  /* skip to the "FROM:" line (sendmail).  
	     if none (UNIX mail), use last "From" line.
	  */
	fgets (temp,LINELEN,curr.fp);
	strcpy (last,temp);
	while (fgets (temp,LINELEN,curr.fp) != NULL)
	{
		if ( (strncmp(temp,"From",4) != 0)  &&
		     (strncmp(temp,">From",5) != 0) &&
		     (strncmp(temp,"FROM:",5) != 0))         break;
		strcpy (last,temp);		
	}
	if( last[0] == 'F' ) fprintf(fout,">");
	fprintf (fout,"%s",last);
	fprintf(fout,"%s",temp);	/* first line of message */

	  /* write out the message */
	while(ftell(curr.fp)< messend[messnum])
	{
		fgets(temp,LINELEN,curr.fp);
		if(!strncmp(temp,"From",4) || !strncmp(temp,"CC:",3))
			fprintf(fout,">");
		fprintf(fout,"%s",temp);
	}

	fprintf (fout,"\n--------------------");
	fprintf (fout," end of forwarded message ");
	fprintf (fout,"--------------------\n");
	fclose (fout);

	 /* invoke sendmail */
	sprintf( command,"%s %s=%s",
		SENDMAIL,edit?"eappend":"dappend",filename);
	ssystem (command);

}
