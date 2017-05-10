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
#include  "defs.h"

/*  executes  func(arg1,messnum)  for all messnum in current mailbox
  which match the logical expression "exp".
  eg  FROM:schatz&SUBJECT:rdmail.
  errors are handled and announced by the called routines.
*/

logexpr (func,arg1,exp)
 int (*func)();
 char arg1[];
 char exp[];
{
	/* parse into global variables */
	if ( leparse (exp)  ==  0)
		/* invoke function on matched messages */
		search (func,arg1);
} 
