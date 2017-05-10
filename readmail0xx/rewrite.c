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

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
extern int errno;
static struct ustat errbuf;


 /* removes messages marked for deletion in current mailbox
   then closes the file.   returns 0 if successful, 1 if error.
 */


rewrite()
{
   int i,flag,permit;
   long j;
   FILE *ft;	
   int confirmation;			/* JM 2/19/85 */
   char tempfile[100], old_mailbox[100];

	 /* check whether user actually wants to make the deletions */
	confirmation = profile("confirm");
	if (expunge(confirmation) == 1)
		return(1);	/* no changes (throw away marks) */

	 /* the tempfile can't be in /tmp or it can't be "link"ed to */
	strcpy (tempfile,maildir());
	strcat (tempfile,"/rewriteXXXXXX");
	mktemp(tempfile);
	flag=0;

	 /* create tempfile containing unmarked messages */
	ft = fopen(tempfile,"w");
	chmod (tempfile,0600);
	if (ft == NULL)
	{
		printf ("\n *** can't update mailbox; no deletions made.  ");
		printf ("please check permissions. ***\n\n");
		return(1);
	}
	for(i=1; i<=nummess; i++)
		if (messdel[i] == 0)
		{	/* copy message to tempfile */
			fseek (curr.fp,messbeg[i],0);   /*start of mess*/
			for (j=messbeg[i]; j<=messend[i]; j++)
				if (putc (getc(curr.fp),ft) == EOF)
				{
					printf ("\n *** can't update mailbox; no deletions made.  ");
					printf ("no space in filesystem. ***\n\n");
					return(1);
				}
			flag=1;
		}

	 /* close the old mailbox (open since initial mailbox setup) and 
	   also close the new one */
	fclose (curr.fp);
	fclose (ft);


	  /* create the revised mailbox if there were any messages for it */
	 full_boxname (old_mailbox,curr.mailbox);
	 if (flag == 1)
	 {	/* were messages: save permissions, delete old, make new */
		stat (old_mailbox,&errbuf);
		permit =  errbuf.st_mode & 0777;
		unlink (old_mailbox);
		link (tempfile,old_mailbox);
		chmod (old_mailbox,permit);
	}
	else
	{	/* were none: just delete */
		/* unlink (old_mailbox); No good. Should not uncreate a */
		/* mailbox just because it is empty. Ask what the user  */
		/* wants. If wnats it removed then remove it otherwise  */
		/* just empty it .					*/
		if(remove_mailbox(curr.mailbox, confirmation))
			unlink(old_mailbox);
		else
			fclose(fopen( old_mailbox, "w"));
	}

	 /* cleanup */
	unlink(tempfile);
	return(0);
}





 /* when leave mailbox (change,quit), ask whether should rewrite.
    if no, delete marks disappear. 
    return 0 if should rewrite (some marked and user says yes).
    return 1 otherwise (none marked or user says no).
    if confirmation is not requested and soem marked say yes./* JM 2/19/85 
 */

expunge( confirm )				/* JM 2/19/85 */
int confirm;					/* JM 2/19/85 */
{
	char answer[50];
	int i;


	for (i=1; i<=nummess; i++)
	{	/* check for deleted messages */
		if (messdel[i] == 1)
		{	/* there is at least one message marked for deletion */
		    if( confirm )		/* JM 2/19/85 */
		    {				/* JM 2/19/85 */
			printf ("\n delete marked messages? [yes] ");
			gets (answer);
			if ((strlen(answer) == 0)  ||
			    (strncmp (answer,"yes",strlen(answer))  ==  0))
					return(0);		
			printf("\n");
			return(1);		/* JM 2/19/85 */
		    }				/* JM 2/19/85 */
		    return(0);			/* JM 2/19/85 */
		}
	}

	return(1);	/* no deleted messages */
}

remove_mailbox(box, confirm)
char	*box;
int	confirm;
{
	char answer[50];

	if( confirm )
	{
	    if( strcmp( box, "new" ))
	    {
		printf ("\n the mailbox is empty\n");
		printf ("\n remove the mailbox? [yes] ");
		gets (answer);
	    }
	    printf("\n");
	}
	else
	    return(0);
	if ((strlen(answer) == 0)  ||
	    (strncmp (answer,"yes",strlen(answer))  ==  0))
	{
		return(1);		
	}
	return(0);
}
