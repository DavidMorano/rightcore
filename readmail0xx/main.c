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

main(argc,argv)
 int argc;  char *argv[];
{
	char startup[LINELEN];
	int failed;


	/* clear umask so creat permissions are used without modification */
	umask(0);
	get_screensize();
	/* initialize the user profile options */
	profinit();

	if (argc == 1)
	{	/* no args. standard startup in new box */
		failed =  setup_mailbox("new");
	}
	else
	{	
		/* first argument is startup mail box */
		full_boxname (startup,argv[1]);
		if ((access(startup, A_READ)) == 0)
		{
			failed =  setup_mailbox(argv[1]);
		}
		else	
		{
			printf(" no access to mailbox \"%s\",  ",argv[1]);
			printf("starting up in the \"new\" box.\n");
			failed =  setup_mailbox("new");
		}
	}

	 /* interactively read and execute the commands */
	if (failed)
		printf("\n *** program aborting *** \n");
	else
		inter();
}
/*printf(fmt, args)
char *fmt;
{
	return(_doprnt(fmt, &args, stdout));
}
*/

