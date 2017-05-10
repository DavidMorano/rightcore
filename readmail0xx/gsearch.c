/************************************************************************
 *                                                                      *
 * The information contained herein is for use of   AT&T Information    *
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   *
 *                                                                      *
 *     (c) 1984 AT&T Information Systems                                *
 *                                                                      *
 * Authors of the contents of this file:                                *
 *                                                                      *
 *                      Bruce Schatz                                    *
 *									*
 ***********************************************************************/
#include "defs.h"

 /* this searches globally (across all boxes) for messages which
    match the specifed expression and prints the scanline of each.
    setups each mailbox and scans in turn, then resetsup the current one.
 */


gsearch (exp)
   char exp[];
{
	char command[200], box[100], dummy[100];
	char fboxlist[100];
	FILE *fb;
	int tempmessdel[500], tempmsgno;  /* to preserve state after resetup */
	char tempmailbox[100];		  /* preserve old mailbox name */
	int scanmess();
	int i;

	/* place list of mailbox names into temporary file */
	strcpy(fboxlist,"/tmp/gsearchXXXXXX");
	mktemp(fboxlist);
	strcpy(command,"ls ");
	strcat(command,maildir());
	strcat(command," > ");
	strcat(command,fboxlist);
	usystem(command);	    

	/* parse to check for invalid logical expression */
	if (leparse (exp) == 1)
		return(1);

	/* preserve state which will be lost when current mailbox is
	  setup again after gsearch is done.
	*/
	strcpy (tempmailbox,curr.mailbox);	/* name of mailbox */
	tempmsgno = curr.msgno;			/* message pointer */
	for (i=1; i<=nummess; i++)
		tempmessdel[i] = messdel[i];	/* deletion markers */
	fclose (curr.fp);		/* close current mailbox */

	/* search through all mailboxes. 
	  must reparse before each search since presort in setup uses
	  same global logexpr token variables.
	*/
	fb = fopen(fboxlist,"r");
	while (fgets(box,50,fb)  !=  NULL)
	{	/* setup and search each mailbox */
		box[strlen(box)-1] = NULL;  /* remove linefeed */
		setup_mailbox (box);
		leparse (exp);              /* reparse */
		firstmatch = 0;		    /* no matches yet */
		search (scanmess,dummy);    /* scan matched messages */
		fclose (curr.fp);	    /* close box without rewriting */
	}
	fclose(fb);
	unlink(fboxlist);

	/* restore (setup) the original mailbox */		
	setup_mailbox (tempmailbox);
	curr.msgno = tempmsgno;
	for (i=1; i<=nummess; i++)
		messdel[i] = tempmessdel[i];

	printf("\n");
	return(0);
}


/* prints out scanline of message */

scanmess (dummy,messnum)
 char dummy[];
 int messnum;
{
	if (firstmatch == 1)
	{	 /* first match in mailbox so print mailbox name */
		printf ("\n%s:\n",curr.mailbox);
		firstmatch = 2;		/* past firstmatch */
	}
	scan ("all",messnum);
	return(0);
}
