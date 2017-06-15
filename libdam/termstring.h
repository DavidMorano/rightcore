/* termstring */

/* terminal display editing control sequences */
/* last modified %G% version %I% */


/* revision history:

	- 1984-04-10, David A­D­ Morano
	File was originally written.

	- 1989-08-01, Dave morano
	File was updated with the codes for "advanced" terminals.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	TERMSTRING_INCLUDE
#define	TERMSTRING_INCLUDE	1


/* saving, restoring */

#define	TERM_SAVE	"\0337"		/* save cursor and mode */
#define	TERM_RESTORE	"\0338"		/* restore cursor and mode */

/* cursor positioning */

#define	TERM_UP		"\033[A"
#define	TERM_DOWN	"\033[B"
#define	TERM_RIGHT	"\033[C"
#define	TERM_LEFT	"\033[D"
#define	TERM_HOME	"\033[H"	/* cursor home */

/* editing */

#define	TERM_ED		"\033[J"	/* erase to end of display */
#define	TERM_EL		"\033[K"	/* erase to end of line */
#define	TERM_EC		"\033[X"	/* erase character */

#define	TERM_EDIS	"\033[J"	/* erase to end of display */
#define	TERM_ECH	"\033[X"	/* erase character */
#define	TERM_X		"\033[X"	/* erase character */

#define	TERM_DL		"\033[M"	/* delete line */
#define	TERM_DC		"\033[P"	/* delete character */
#define	TERM_DCH	"\033[P"	/* delete character */

#define	TERM_IL		"\033[L"	/* insert line */
#define	TERM_IC		"\033[@"	/* insert characters */
#define	TERM_ICH	"\033[@"	/* insert characters */

/* character renditions */

#define	TERM_NORM	"\033[m"	/* no attributes */
#define	TERM_BOLD	"\033[1m"	/* bold */
#define	TERM_UNDER	"\033[4m"	/* under line on */
#define	TERM_BLINK	"\033[5m"	/* blink */
#define	TERM_REVERSE	"\033[7m"	/* reverse */

/* character renditions for advanced terminals */

#define	TERM_NOBOLD	"\033[22m"	/* no bold */
#define	TERM_NOUNDER	"\033[24m"	/* no under line */
#define	TERM_NOBLINK	"\033[27m"	/* no blink */
#define	TERM_REVERSE	"\033[7m"	/* no reverse */

/* terminal modes */

#define	TERM_S_IRM	"\033[4h"	/* set insert/replacement */
#define	TERM_R_IRM	"\033[4l"	/* clear insert/replacement */
#define	TERM_S_CUR	"\033[\07725h"	/* cursor on */
#define	TERM_R_CUR	"\033[\07725l"	/* cursor off */

#define	TERM_S_SD	"\033[1$}"	/* set status line display */
#define	TERM_R_SD	"\033[0$}"	/* reset active display */


#endif /* TERMSTRING_INCLUDE */


