static char SCCSID[] = "@(#) tecexit.c:  5.1 2/22/83";
/* tecexit.c
**
**	TECO exit code
**
**	David Kristol, April, 1982
**
** This module contains the TECO clean-up and exit code.  Any dirty work
** that TECO must do to restore status of the user's terminal and other
** items should be placed here.
*/

#include <stdio.h>			/* for tty.h */
#include "bool.h"
#include "tty.h"
void
Tecexit(exitcode)
int exitcode;				/* value for UNIX exit */
{
    void exit();
    void Unsetwild();
    void Fileexit();

    Unsetwild();			/* reset wildcard handler */

    Fileexit();				/* close lingering files */

    Ttreset();				/* reset terminal to its
					** initial state
					*/
    exit(exitcode);			/* return exit status code */
}
