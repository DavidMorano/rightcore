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
#include <signal.h>
#include <setjmp.h>

/* list out names of all mailboxes */
/* Modified 1.17.84 by jm to produce cleaner list */
/* Modified 9/5/84 by jm to handle interrupt during listing properly */

static char filename[LINELEN];
static jmp_buf env;
static	void (*sigint)(), (*sigquit)();
int mask;

static void fr();


void mlist()
{	
	FILE *fl;

	int len, namelen;

	char command[LINELEN], name[LINELEN];
	char blanks[18];


	sprintf(blanks,"                ");
	strcpy (filename,"/tmp/maillistXXXXXX");
	mktemp (filename);

	mask = umask(077);
	sigint = signal(SIGINT, SIG_IGN);
	sigquit = signal(SIGQUIT, SIG_IGN);
	if (setjmp(env) != 0) return;

	signal( SIGINT, fr);
	signal( SIGQUIT,fr);

	sprintf (command, "ls %s > %s", maildir(), filename);
	usystem (command);
	fl = fopen (filename,"r");
	printf ("\nmailboxes: \n\n");
	signal( SIGINT, SIG_IGN );
	signal( SIGQUIT,SIG_IGN );
	len=0;
	while (fgets(name,LINELEN,fl) != NULL)
	{	/* read mailbox names from ls list */
		name[strlen(name)-1] =  NULL;    /* remove newline char */
		namelen = strlen(name) + 1;
		len += 16;	 /* includes blank */
		if (len <= COLS)
		{
			printf ("%s",name);
			if (len < COLS) printf("%s",&blanks[namelen]);
		}
		else
		{	/* next word wouldn't fit on line so start next one */
			printf ("\n%s%s",name,&blanks[namelen]);/*leave newline char*/
			len = 16;
		}
	}
	fclose(fl);
	printf("\n\n");
	fflush(stdout);
	unlink (filename);   /* remove tempfile */

	umask(mask);
	signal( SIGINT, sigint);
	signal( SIGQUIT,sigquit);
}

static void fr(sig)
int	sig ;
{
	unlink(filename);
	longjmp(env, 1);
}


