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

/* moves specified message into specified savebox (complete pathname).
  i.e. copies message into savebox then marks for deletion in current box.
  returns 0 if successful, 1 if error. 
*/


 /* declarations for use of stat */
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
extern int  errno;
static struct ustat errbuf;


save(savebox,messnum)
   char savebox[];
   int messnum;
{
   	int extmessnum;
	long i;
   	char answer[LINELEN],boxname[LINELEN];
   	FILE *fs;

	extmessnum = messnum;	/* delete later will want external number */
	messnum = messord[messnum];     /* convert to internal number */

	 /* don't move it if has been deleted */
	if (messdel[messnum] == 1)     return(1);
	/* don't move it if you're already in that mailbox */

	mail_boxname(boxname,savebox);
	if (strcmp(boxname,curr.mailbox) == 0)
	{
		printf("\nAlready in mailbox \"%s\" ", boxname);
		printf("\n");
		return(1);
	}

	 /* make new savebox if not already extant */
	if (stat(savebox,&errbuf) == -1)
	if (errno == ENOENT)
	{	/* create new mailbox after confirmation */
		if (strcmp(boxname,"new") == 0  ||  strcmp(boxname,"old") == 0)
			creat (savebox,0600);
		else
		{
			printf("\nCreate new mailbox \"%s\" ? [yes] ",boxname);
			gets(answer);
			if ((strlen(answer) == 0)  ||
		            (strncmp (answer,"yes",strlen(answer))  ==  0))
				creat (savebox,0600);
			else	return(1);	/* don't save */
		}
	}
	else
	{
		printf("\n *** can't access \"%s\", please ",savebox);
		printf("check permissions *** \n\n");
		return(1);
	}


	 /* open mailbox and append the new message */
	if ((fs = fopen(savebox,"a"))  ==  NULL)
	{
		printf("\n *** save: could not open %s *** \n",savebox);
		return(1);
	}


	 /* copy message from this box into the savebox (append to end) */
	fseek(curr.fp,messbeg[messnum],0);	/*postion sending file*/
	fseek(fs,0L,2);	     /* position receiving file to end */
	for(i=messbeg[messnum]; i<=messend[messnum]; i++) 
		putc(getc(curr.fp),fs);


	fclose(fs);
	delete(" ",extmessnum);     /* "delete" the moved message */
	return(0);
}
