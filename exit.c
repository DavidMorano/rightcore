/* exit */

/* exit a process */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Remarkably, it appears that newer OSes are now doing what we do here!
	At least Solaris has seemed to come around to this way of handling
	process exit.  Former 'exit(3c)'s used to force the linking of the
	STDIO library just to exit a process!  This wreaked havoc in small
	embedded environments so it was avoided (using this subroutine).  Newer
	OSes seem to now register their stupid library cleanup (including
	stupid STDIO) with 'atexit(3c)' that is called by '_exithandle()'
	below.  So there is no need to force the linking in of the STDIO
	library just to check if there are some stupid STDIO file handles lying
	around.

	Maybe this subroutine is not needed any longer if the OS now does
	pretty much exactly what we are doing here.  We'll watch the situation
	(for how many more years?) and see what settles out.


*******************************************************************************/


#include	<envstandards.h>

#include	<unistd.h>


void exit(int ex)
{

	_exithandle() ;

	(void) _exit(ex) ;
}
/* end subroutine (exit) */


