/************************************************************************
 *                                                                      
 * The information contained herein is for use of   AT&T Information    
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   
 *                                                                      
 *     (c) 1984 AT&T Information Systems                                
 *                                                                      
 * Authors of the contents of this file:                                
 *                                                                      
 *                      Jishnu Mukerji                                  
 *									
 ***********************************************************************/



#include "defs.h"
#include "profile.h"



extern char	*getenv() ;


/*
  Function to initialize screen size 
*/

get_screensize()
{
    char *envlines=getenv("LINES");
    char *termcap=getenv("TERMCAP");
    char *term=getenv("TERM");


/* first try the LINES environment variable */

    if (envlines != NULL ) {

	maxlines = atoi(envlines) ;

	return ;
    }

/* next try the termcap variable */

    if (termcap != NULL ) {

	char *c;


	c = termcap;
	while((c = strchr(c,':')) != NULL) {

	    c += 1 ;
	    if (strncmp(c,"li#",3) == 0 ) {

		maxlines = atoi(c+3) ;

		return;
	    }
	}
    }
    /* next try to look up a termcap entry using term */
    /* To be added later */
}



 /* boolean function which determines whether the current user profile 
   has set the specified option (desires certain action).  
   returns 1 if set, 0 if not.
   references the global profile structure "userprof".
 */

profile (opt)
 char opt[];
{
	int i;
	i=0;
	while (strlen(userprof[i].name) > 0)
	{
		if (strcmp(userprof[i].name,opt) == 0)
			return (userprof[i].value);
		i++;
	}
	printf ("\n *** bad argument \"%s\" to \"profile\" ***\n",opt);
	return(0);
}






 /* initializes the various profile options.
   any user-specified ones are contained in the shell variable RDMAILOPTS.
   these options then override the defaults in the global profile structure
    "userprof".  this structure is defined in "defs.c" and declared
    in "defs.h" (which is included into every function file).
   set options (+) have value 1, not set (-) have value 0.
 */


profinit ()
{
	char *getenv(),opts[300],*opt;
	int i,val;


	 /* check for user overrides */
	opt = getenv("RDMAILOPTS");
	if(( opt == NULL ) || ( *opt == NULL ))
		strcpy( opts, DRDMAILOPTS );
	else
		strcpy( opts, opt );

	opt = strtok (opts,":");
	if(opt != NULL ) {
	  do
	    {
		if (*opt == '-')
		  	val = 0;     		/* turn off */
		else if (*opt == '+')
			val = 1;		/* turn on */
		else continue;
		opt++;

		i=0;
		while (strlen(userprof[i].name) > 0)
		{
			if (strcmp(opt,userprof[i].name) == 0)
			{
				userprof[i].value = val;
/*			printf("profile: option %s set to %d\n",opt,val);*/
				goto nextopt;
			}
			i++;
		}
		printf("\n  illegal profile option \"%s\", please",opt);
		printf(" respecify shell variable RDMAILOPTS.\n");
		return(1);
		
		 nextopt:
		opt = strtok (0,":");
	      }  while  (opt != NULL);
	}
}
