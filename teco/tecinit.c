static char SCCSID[] = "@(#) tecinit.c:  6.1 8/25/83";
/* tecinit.c
**
**	TECO initialization code
**
**	David Kristol, April, 1982
**
** This module contains all of TECO's initialization code.  System
** dependent initializations should be inserted here.
*/

#include <stdio.h>			/* for tty.h */
#include "bool.h"
#include "chars.h"
#include "cmutil.h"
#include "mdconfig.h"
#include "qname.h"
#include "tb.h"
#include "tflags.h"
#include "tty.h"
#include "values.h"			/* for TNUMB */
#include "xec.h"			/* for Radix */

#define	STDIN	0			/* file descriptor for stdin */
#define STDOUT	1			/* file descriptor for stdout */
void
Tecinit(argc,argv)
int argc;				/* number of arguments */
char ** argv;				/* argument string array */
{
    void Fileinit();
    BOOL SMinit();
    void TBinit();
    void Crtinit();
    void arginit();
    void interr();

    extern int isatty();

    extern TNUMB ED;			/* TECO ED flag */
    extern TNUMB ET;			/* TECO ET flag */

    Ttinit();				/* initialize terminal */

    Crtinit();				/* initialize CRT handling, if any */

    /* keep stdin and stdout open if they are not terminals */
    Fileinit(   ! (BOOL) isatty(STDIN),	/* stdin open if not tty */
		! (BOOL) isatty(STDOUT)	/* stdout open if not tty */
	    );

    if (!SMinit(
		SMGRAN,			/* granule size */
		SMINCR,			/* granules per increment */
		SMINITSIZE,		/* initial number of increments */
		SMMAXSIZE,		/* maximum number of increments */
		TRUE			/* permit expansion */
		))
	interr("Can't initialize memory");

    TBinit();				/* initialize text block management */

/* Initialize flags */

    Radix = 10;				/* base 10 initially */
    ED |= (ED_expand | ED_upar);	/* enable expand, since SMinit call
					** enabled it; ^ is literal in
					** searches, as per spec.
					*/

#ifndef DEBUG
    ET |= ET_mung;			/* force exit on start-up error;
					** applies by default to "mung",
					** since it stays in effect until
					** first prompt
					*/
#endif
    /* everything else is implicitly initialized to zero */

    arginit(argc,argv);			/* argument initialization */

    return;
}
/* arginit -- argument initialization
**
** This code does the initialization as prescribed by the TECO manual,
** to wit:
**
**	1.  The arguments are placed in the filename string buffer.
**	2.  The initialization processing is placed in the text buffer.
**	3.  We set up the world so the following command string will
**	    be executed:
**		HXY HK G* HXZ HK :EITECO.TEC$$ MY$$
**
** Note that there are special hooks in rIch (cmdio.c) to handle a
** special input source for initialization commands (rINIch).
** Furthermore, the EI command (cmfile.c) is rigged so a null EI$
** command turns off this initialization source.  This behavior
** is necessary to match the specification that the MY is bypassed
** if an EI$ is done.
**
** One other note:  because of UNIX's more sophisticated argument
** processing, we make one extension to the specification.  All of
** TECO's incoming arguments are placed in the filename string buffer,
** but they are separated by an ARGSEP character.  This allows us to
** deal with embedded spaces within arguments and still note that the
** arguments are separate.  This character is also stored in the number
** portion of Q-register Z (which is also the Q register in which the
** arguments end up in, as well).
*/
/* Data for arginit */

#define	ARGSEP	0			/* numeric value of argument
					** separator
					*/
#define	DIG1	(ARGSEP/100)		/* high order digit of sep. */
#define	DIG2	((ARGSEP-(DIG1*100))/10) /* second digit of sep. */
#define	DIG3	(ARGSEP-(DIG1*100)-(DIG2*10)) /* third digit */


static char cmdstr[] =			/* command string to put separator in
					** Q-register Z
					*/
{	0,0,0,				/* filled with three digits of sep. */
	'U', 'Z', NUL
};
static void
arginit(argc,argv)
int argc;				/* number of arguments */
char ** argv;				/* array of argument strings */
{
    extern char teco_tec[];		/* external initial teco macro */
    char * s;				/* pointer to current argument */

/* Initialize command string because we can't do it at compile time. */

    cmdstr[0] = DIG1 + '0';
    cmdstr[1] = DIG2 + '0';
    cmdstr[2] = DIG3 + '0';

/* Initialize text buffer with teco.tec macro */

    TBstatic(Q_text,teco_tec,strlen(teco_tec));

/* Initialize filename buffer with command line arguments */

    while (argc-- > 0)			/* for each argument */
    {
	s = *argv++;			/* get next argument */

	Addst(Q_file,s,strlen(s));	/* add this argument to filename
					** buffer
					*/
	if (argc != 0)			/* no trailing separator after
					** last argument
					*/
	    Addc(Q_file,ARGSEP);	/* append argument separator */
    }

    Xecimmed(cmdstr);			/* put ARGSEP in Q-register Z */
    return;
}
/* rINIch -- read an initialization character
**
** This routine returns the next character in the initialization
** command stream, or EOF if there are no more.
*/

/* First, here's the initialization command string. */

static char inicmd[] =
{
	'H', 'X', 'Y', 'H', 'K', 'G', '*',
	'H', 'X', 'Z', 'H', 'K',
	':', 'E', 'I', 't', 'e', 'c', 'o', '.', 't', 'e', 'c', ESC, ESC,
	'M', 'Y', ESC, ESC, NUL
};

static char * iniptr = inicmd;		/* pointer to string above */

int
rINIch()
{
    if (*iniptr == NUL)			/* at end of string? */
	return(EOF);			/* yes.  return EOF */
    
    return((unsigned) *iniptr++);	/* else, return the character */
}
/* Killini -- turn off initialization command stream
**
** This routine turns off the initialization command stream so
** rINIch will only return EOF.
*/

void
Killini()
{
    iniptr = "";			/* pretend no further chars */
    return;
}
