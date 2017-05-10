/* doinfo */


#define		DEBUG	1


/************************************************************************
 *									*
 * The information contained herein is for the use of AT&T Information	*
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	*
 *									*
 *	(c) 1984 AT&T Information Systems				*
 *									*
 * Authors of the contents of this file:				*
 *									*
 *		J.Mukerji						*
 *									*

 * doinfo() collects a bunch of names from the command line and runs
 * checkname(name,mode) on each of them. mode = 0 if '-' option is specified
 * mode =2 otherwise. The value returned in exit is 0 if the name is found
 * 1 otherwise.


***************************************************************************/



#include	<stdio.h>

#include	"config.h"

#include	"smail.h"



/* external variables */

extern struct global	g ;



int doinfo(argc, argv)
int  argc;
char **argv;
{
	int i, mode;


/* first get the names of files that contain name translation */

	mode = 2 ;
	if (argc < 2) {

	infousage() ;

	return OK ;
	}

	if (namelist[0] == '\0')
	    errprintf(stdout,
	        "warning: translation table not defined\n");

/* now get the contents of the translation tables */

	gettable(g.nodename) ;

/* do something here */

	argv++;		/* skip the command name */
	if (**argv == '-') {
	    mode = 0; 
	    argv++;
	}

/* check for a command line option */

	if (**argv == '+') {

	    if (! strncmp( "+verbose", *argv, strlen(*argv))) {

	        verbose = 1; 
	        argv++;

	    } else {

		infousage() ;

		return OK ;
	    }

	} /* end if */

/* go through the actual loops */

	while (*argv != NULL) {

#if	DEBUG
	errprintf("before\n") ;
#endif

	    i = checkname(*argv++, mode);

#if	DEBUG
	errprintf("after\n") ;
#endif

	}

	if (i) i = 0 ;

	else i = 1 ;

	return i ;
}
/* end subroutine (doinfo) */


infousage()
{
	printf(
	    "usage: %s [-] [+verbose] name1  .....  namen\n", 
	g.progname) ;

}


