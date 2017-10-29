static char SCCSID[] = "@(#) crt.c:  6.1 8/25/83";
/* crt.c
**
**	TECO CRT Handling
**
**	David Kristol, May, 1983
**
** This module contains support for TECO-style CRT handling.
** However, unlike standard TECO, the support is based on
** a UNIX termcap/curses mechanism that ties the processing
** less to a specific terminal.
**
** To support the CRT-style rubouts, the following capabilities
** are required:
**
**  1)	Backspace must backspace.  Space must overwrite any
**	character in its way.
**  2)  Carriage Return must move to the left margin without
**	erasing anything or moving down a line.
**  3)  There must be an "erase from here to end of screen" sequence.
**  4)  There must be a "move up one line in the current column"
**	sequence.  Furthermore, if the cursor is at the top of the
**	screen, the cursor should stay in the same place.
**
** These notions are captured in the following functions:
**
**	Crtinit		determines CRT type and determines whether
**			it meets the criteria.  If so, it locates
**			the needed sequences.
**	Crtechar	erases the character under the cursor
**	Crteline	erases the current line (and the rest of
**			the screen)
**	Crtupline	moves the cursor up one line
**
** These functions are optional, in the sense that they will be used
** if they exist:
**
**	Crtstandout	turns "standout" mode on or off for errors
*/


#include "bool.h"
#include "mdconfig.h"

#ifdef	CRTRUB

#include <stdio.h>
#include "chars.h"
#include "tflags.h"
#include "tty.h"
#include "values.h"

#define	CNULL	((char *) 0)		/* a null pointer */
#define	CAPSIZE	1024			/* size of buffer for terminal
					** capabilities
					*/

/* Define handy macro for putting out control strings */

#define	outctrl(s) (void) tputs(s,1,outcchar)
int outcchar();				/* for the above */
extern int tputs();

#endif	/* def CRTRUB */
/* Data */

/* Global data */

BOOL Iscrt = FALSE;			/* true if terminal is certifiably a
					** CRT
					*/

#ifdef	CRTRUB

short ospeed;			/* terminal speed for tputs() */
char PC;				/* pad character for tputs() */

extern TNUMB ET;			/* TECO ET flag */

/* Static data */

static char * cr = "\r";		/* Carriage return sequence */
static char * echar = "\b \b";		/* Erase-character sequence */
static char * eline;			/* Erase line sequence:
					** actually from cursor to end
					** of screen
					*/
static char * upline;			/* Move cursor up a line */
static char * sostart = "";		/* Start standout */
static char * soend = "";		/* End standout */

#endif	/* def CRTRUB */
/* Crtinit -- initialize CRT handling
**
** This routine checks for all the relevant (required) CRT handling
** capabilities.  If they are all present, the terminal is declared
** to be a CRT, and CRT-type handling is enabled.  Otherwise the
** terminal is forever doomed to be treated as a dumb terminal.
** The environment variable TERM must contain the terminal type, as
** is customary with termcap/curses.
*/

void
Crtinit()
{
#ifdef	CRTRUB

    static char area[100];		/* area to dump strings into */
    char capbuf[CAPSIZE];		/* place for raw capabilities */
    char * oldarea = area;		/* pointer to current "area" pos. */
    char * tempcp;			/* temporary char pointer */
    extern int Twidth;			/* terminal width */

    extern int tgetent();
    extern int tgetnum();
    extern int tgetflag();
    extern char * tgetstr();

    extern char * getenv();

#endif	/* def CRTRUB */

    Iscrt = FALSE;			/* not a CRT yet */

#ifdef	CRTRUB

    if (   (tgetent(capbuf,getenv("TERM")) > 0)
	&& tgetflag("bs")		/* can do backspace */
	&& (eline = tgetstr("cd",&oldarea)) != CNULL
					/* has erase to end of screen */
	&& (upline = tgetstr("up",&oldarea)) != CNULL
					/* has up-line capability */
	&& ! tgetflag("nc")		/* has valid CR ability */
	)
	Iscrt = TRUE;
    else
	return;				/* not a CRT.  Bail out */
    
    if ((cr = tgetstr("cr", &oldarea)) == CNULL)
	cr = "\r";			/* set CR string */

    /* Set pad character */

    PC = NUL;				/* assume NUL first */
    if ((tempcp = tgetstr("pc", &oldarea)) != CNULL)
	PC = *tempcp;			/* if other pad, grab it */

    /* Get standout start/end strings, if they exist */

    if (   (sostart = tgetstr("so", &oldarea)) == CNULL
	|| (soend = tgetstr("se", &oldarea)) == CNULL
	)
	sostart = soend = "";
    
    Twidth = tgetnum("co");		/* terminal width is number
					** of columns
					*/

    /* Get terminal speed for padding */

    ospeed = Ttspeed();

    ET |= ET_crt;			/* call terminal a CRT */

#endif	/* def CRTRUB */

    return;
}
/* outcchar -- output control string char
**
** This routine outputs a argument control string character to the
** terminal.  It's called by tputs().
*/

#ifdef	CRTRUB

int
outcchar(c)
char c;
{
    return(putc(c, Ttyout));
}
/* Output the various control sequences */

void
Crtechar()				/* erase one character */
{
    outctrl(echar);			/* output erase character sequence */
    return;
}


void
Crteline()				/* erase current line + */
{
    outctrl(cr);			/* CR-equivalent string */
    outctrl(eline);			/* erase line */
    return;
}


void
Crtupline()				/* move up a line */
{
    outctrl(upline);			/* move up a line */
    return;
}

#endif	/* def CRTRUB */

void
Crtstandout(onoff)
BOOL onoff;				/* true to turn on standout */
{

#ifdef	CRTRUB

    if ((ET & ET_crt) != 0)		/* only in CRT mode */
	outctrl( onoff ? sostart : soend );

#endif	/* def CRTRUB */

    return;
}
