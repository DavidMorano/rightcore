static char SCCSID[] = "@(#) tecmain.c:  6.1 8/25/83";
/* tecmain.c
**
**	TECO main program
**
**	David Kristol, April, 1982
**
** This module is the main code of TECO from which all else stems.
*/

#include <setjmp.h>			/* setjmp/longjmp definitions */
#include "bool.h"			/* for values.h */
#include "exittype.h"			/* exit type codes */
#include "values.h"
/* Here's the main TECO code:
**
**	1.  Initialize
**	2.  If selected, display lines near '.' .
**	3.  Read a command string.
**	4.  Execute command string.
**
** This business continues until we are instructed to exit, such as by ^C
** or other means.
*/

jmp_buf Reset;				/* declare longjmp return point */


int
main(argc,argv)
int argc;				/* number of command line args */
char **argv;				/* array of argument strings */
{
    extern TNUMB EV;			/* TECO EV flag */
    void Rdcmd();
    void Tecexit();
    void Tecinit();
    void Xec();


    Tecinit(argc,argv);			/* initialize the world */

    while (setjmp(Reset) != TECEXIT)	/* continue until directed to exit */
    {
	Rdcmd();			/* read next command string */
	Xec();				/* execute it */
    }

    Tecexit(UEXITOK);			/* now exit TECO, signal success */
    /*NOTREACHED*/
}
