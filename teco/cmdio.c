static char SCCSID[] = "@(#) cmdio.c:  6.2 1/16/84";
/* cmdio.c
 *
 *	TECO command input/output
 *
 *	David Kristol, February, 1982
 *
 *	This module contains input/output code for reading and writing
 *	command characters.  Some of the routines are used elsewhere
 *	for writing text buffer characters, as well.
 *
 *	This module exports the following routines for outside use:
 *
 *	inicmd	-- initialize for command reading
 *	rCch	-- read command character
 *	rIch	-- read input character, either from terminal or EI file
 *	rTch	-- read terminal character
 *	whCch	-- where is next command character coming from?
 *	uCch	-- unread command character
 *	wCch	-- write command character
 *	wCch1	-- write command character, ignore casing, etc.
 *	Erasechar --
 *		   erase one character
 *	wTBst	-- write text block string
 *	Crlf	-- write carriage return, line feed to terminal
 *
 *	An important structure for these routines is the character echo
 *	table.  The echo characteristics of each character are stored
 *	here.  Each character is of (exactly) one of 6 types.  For some
 *	of these types these is a secondary character which is used for
 *	some purposes which are described below, referred to as <s>.
 *
 *	normal	-- character echoes as itself always
 *	ctrl	-- character is control character, echoes as ^<s>
 *	lc	-- character is lower case.  If case flag says flag
 *		   lower case chars, echoes as '<s>
 *	uc	-- character is upper case.  Same as lc, except echo as
 *		   '<s> if flagging upper case
 *	repl	-- character is replaced by <s>
 *	coded	-- echoing is handled by software:  no implicit echo
 *	nul	-- nothing gets echoed at all
 *
 *	Another entry in the echo table is the horizontal position
 *	increment.  This is the amount by which the horizontal position
 *	increases if the character is written to the output device as
 *	a raw character (no further interpretation).
 */


struct et				/* define echo table entry */
{
    unsigned char newchar;		/* secondary character */
    unsigned short type:4;		/* character echo type */
    short posinc:2;			/* natural horiz. position incr. */
};

#define Ec_norm	0			/* echoes as itself */
#define Ec_ctrl	1			/* echoes as ^ (second char) */
#define Ec_lc	2			/* is lower case */
#define Ec_uc	3			/* is upper case */
#define Ec_repl	4			/* gets replaced */
#define Ec_coded 5			/* handled by code */
#define Ec_null	6			/* echoes as nothing */

/* These are the flags to use when lower/upper case are presented as the
 * other case
 */

#define LCFLAG '\''			/* presently same for both */
#define UCFLAG '\''

/* this is the echo table proper */

static struct et Etable[] = {
    '@',	Ec_ctrl,	0,	/*	nul	000	*/
    'A',	Ec_ctrl,	0,	/*	^A	001	*/
    'B',	Ec_ctrl,	0,	/*	^B	002	*/
    'C',	Ec_ctrl,	0,	/*	^C	003	*/
    'D',	Ec_ctrl,	0,	/*	^D	004	*/
    'E',	Ec_ctrl,	0,	/*	^E	005	*/
    'F',	Ec_ctrl,	0,	/*	^F	006	*/
    0,		Ec_coded,	0,	/*	BEL	007	*/
    'H',	Ec_coded,	(unsigned short) 0x03,	/* BS	010	*/
    'I',	Ec_coded,	0,	/*	TAB	011	*/
    0,		Ec_norm,	0,	/*	LF	012	*/
    'K',	Ec_coded,	0,	/*	VT	013	*/
    'L',	Ec_coded,	0,	/*	FF	014	*/
    0,		Ec_norm,	0,	/*	CR	015	*/
    'N',	Ec_ctrl,	0,	/*	^N	016	*/
    'O',	Ec_ctrl,	0,	/*	^O	017	*/
    'P',	Ec_ctrl,	0,	/*	^P	020	*/
    'Q',	Ec_ctrl,	0,	/*	^Q	021	*/
    'R',	Ec_ctrl,	0,	/*	^R	022	*/
    'S',	Ec_ctrl,	0,	/*	^S	023	*/
    'T',	Ec_ctrl,	0,	/*	^T	024	*/
    'U',	Ec_ctrl,	0,	/*	^U	025	*/
    'V',	Ec_ctrl,	0,	/*	^V	026	*/
    'W',	Ec_ctrl,	0,	/*	^W	027	*/
    'X',	Ec_ctrl,	0,	/*	^X	030	*/
    'Y',	Ec_ctrl,	0,	/*	^Y	031	*/
    'Z',	Ec_ctrl,	0,	/*	^Z	032	*/
    '$',	Ec_repl,	0,	/*	ESC	033	*/
    '\\',	Ec_ctrl,	0,	/*	^\	034	*/
    ']',	Ec_ctrl,	0,	/*	^]	035	*/
    '^',	Ec_ctrl,	0,	/*	^^	036	*/
    '_',	Ec_ctrl,	0,	/*	^_	037	*/

    0,		Ec_norm,	1,	/*	SPACE	040	*/
    0,		Ec_norm,	1,	/*	!	041	*/
    0,		Ec_norm,	1,	/*	"	042	*/
    0,		Ec_norm,	1,	/*	#	043	*/
    0,		Ec_norm,	1,	/*	$	044	*/
    0,		Ec_norm,	1,	/*	%	045	*/
    0,		Ec_norm,	1,	/*	&	046	*/
    0,		Ec_norm,	1,	/*	'	047	*/
    0,		Ec_norm,	1,	/*	(	050	*/
    0,		Ec_norm,	1,	/*	)	051	*/
    0,		Ec_norm,	1,	/*	*	052	*/
    0,		Ec_norm,	1,	/*	+	053	*/
    0,		Ec_norm,	1,	/*	,	054	*/
    0,		Ec_norm,	1,	/*	-	055	*/
    0,		Ec_norm,	1,	/*	.	056	*/
    0,		Ec_norm,	1,	/*	/	057	*/
    0,		Ec_norm,	1,	/*	0	060	*/
    0,		Ec_norm,	1,	/*	1	061	*/
    0,		Ec_norm,	1,	/*	2	062	*/
    0,		Ec_norm,	1,	/*	3	063	*/
    0,		Ec_norm,	1,	/*	4	064	*/
    0,		Ec_norm,	1,	/*	5	065	*/
    0,		Ec_norm,	1,	/*	6	066	*/
    0,		Ec_norm,	1,	/*	7	067	*/
    0,		Ec_norm,	1,	/*	8	070	*/
    0,		Ec_norm,	1,	/*	9	071	*/
    0,		Ec_norm,	1,	/*	:	072	*/
    0,		Ec_norm,	1,	/*	;	073	*/
    0,		Ec_norm,	1,	/*	<	074	*/
    0,		Ec_norm,	1,	/*	=	075	*/
    0,		Ec_norm,	1,	/*	>	076	*/
    0,		Ec_norm,	1,	/*	?	077	*/

    '`',	Ec_uc,		1,	/*	@	100	*/
    'A',	Ec_uc,		1,	/*	A	101	*/
    'B',	Ec_uc,		1,	/*	B	102	*/
    'C',	Ec_uc,		1,	/*	C	103	*/
    'D',	Ec_uc,		1,	/*	D	104	*/
    'E',	Ec_uc,		1,	/*	E	105	*/
    'F',	Ec_uc,		1,	/*	F	106	*/
    'G',	Ec_uc,		1,	/*	G	107	*/
    'H',	Ec_uc,		1,	/*	H	110	*/
    'I',	Ec_uc,		1,	/*	I	111	*/
    'J',	Ec_uc,		1,	/*	J	112	*/
    'K',	Ec_uc,		1,	/*	K	113	*/
    'L',	Ec_uc,		1,	/*	L	114	*/
    'M',	Ec_uc,		1,	/*	M	115	*/
    'N',	Ec_uc,		1,	/*	N	116	*/
    'O',	Ec_uc,		1,	/*	O	117	*/
    'P',	Ec_uc,		1,	/*	P	120	*/
    'Q',	Ec_uc,		1,	/*	Q	121	*/
    'R',	Ec_uc,		1,	/*	R	122	*/
    'S',	Ec_uc,		1,	/*	S	123	*/
    'T',	Ec_uc,		1,	/*	T	124	*/
    'U',	Ec_uc,		1,	/*	U	125	*/
    'V',	Ec_uc,		1,	/*	V	126	*/
    'W',	Ec_uc,		1,	/*	W	127	*/
    'X',	Ec_uc,		1,	/*	X	130	*/
    'Y',	Ec_uc,		1,	/*	Y	131	*/
    'Z',	Ec_uc,		1,	/*	Z	132	*/
    0,		Ec_norm,	1,	/*	[	133	*/
    0,		Ec_norm,	1,	/*	\	134	*/
    0,		Ec_norm,	1,	/*	]	135	*/
    0,		Ec_norm,	1,	/*	^	136	*/
    0,		Ec_norm,	1,	/*	_	137	*/

    '@',	Ec_lc,		1,	/*	`	140	*/
    'A',	Ec_lc,		1,	/*	a	141	*/
    'B',	Ec_lc,		1,	/*	b	142	*/
    'C',	Ec_lc,		1,	/*	c	143	*/
    'D',	Ec_lc,		1,	/*	d	144	*/
    'E',	Ec_lc,		1,	/*	e	145	*/
    'F',	Ec_lc,		1,	/*	f	146	*/
    'G',	Ec_lc,		1,	/*	g	147	*/
    'H',	Ec_lc,		1,	/*	h	150	*/
    'I',	Ec_lc,		1,	/*	i	151	*/
    'J',	Ec_lc,		1,	/*	j	152	*/
    'K',	Ec_lc,		1,	/*	k	153	*/
    'L',	Ec_lc,		1,	/*	l	154	*/
    'M',	Ec_lc,		1,	/*	m	155	*/
    'N',	Ec_lc,		1,	/*	n	156	*/
    'O',	Ec_lc,		1,	/*	o	157	*/
    'P',	Ec_lc,		1,	/*	p	160	*/
    'Q',	Ec_lc,		1,	/*	q	161	*/
    'R',	Ec_lc,		1,	/*	r	162	*/
    'S',	Ec_lc,		1,	/*	s	163	*/
    'T',	Ec_lc,		1,	/*	t	164	*/
    'U',	Ec_lc,		1,	/*	u	165	*/
    'V',	Ec_lc,		1,	/*	v	166	*/
    'W',	Ec_lc,		1,	/*	w	167	*/
    'X',	Ec_lc,		1,	/*	x	170	*/
    'Y',	Ec_lc,		1,	/*	y	171	*/
    'Z',	Ec_lc,		1,	/*	z	172	*/
    '[',	Ec_lc,		1,	/*	{	173	*/
    '\\',	Ec_lc,		1,	/*	|	174	*/
    ']',	Ec_lc,		1,	/*	}	175	*/
    '_',	Ec_lc,		1,	/*	~	176	*/
    '?',	Ec_ctrl,	0	/*	DEL	177	*/
};


/* Interesting data */

#include <stdio.h>
#include "bool.h"		/* define booleans for crt.h, tb.h */
#include "chars.h"		/* define character names and codes */
#include "crt.h"		/* define crt interface */
#include "mdconfig.h"		/* configuration information */
#include "qname.h"		/* define Q-register names */
#include "sources.h"		/* input source codes */
#include "tb.h"			/* text block routines */
#include "tflags.h"		/* define TECO flags bits */
#include "tty.h"		/* define terminal interface routines */
#include "values.h"		/* define TNUMB type */




/* external subroutines */

extern void	interr() ;


/* external variables */

extern int Twidth;		/* width of terminal in chars */

extern TNUMB ET;		/* ET (terminal modes) flag */
extern TNUMB EU;		/* EU (case) flag */

extern BOOL Fl_ctO;		/* ^O flag */


/* global variables */

int Inpsrc;			/* contains input source number for last
				** character read by rIch */

/* local variables */

static int pos;			/* current horizontal position */
static int oldsrc;		/* input source of "ungotten" char */


/* inicmd -- initialize for command reading
 *
 * This routine sets up internal structures to prepare for command
 * reading.
 */

static int ungotchar;		/* "ungotten" character */

void
inicmd()
{

    ungotchar = EOF;		/* initialize "ungotten" char */
    return;
}


/* uCch -- unget command character
 *
 * This routine provides for a 1-character push-back of command
 * characters.  The command reader needs at least 1 level.
 */

void
uCch(c)
int c;				/* character to push back */
{
    ungotchar = c;		/* save the char */
    oldsrc = Inpsrc;		/* remember its source */
    return;
}
/* rother -- read character from other than terminal
**
** This routine returns the next character from anywhere but the
** user's terminal (since that could cause a wait...).  It also
** sets "Inpsrc" properly.  If we don't get a character from an
** EI file or the initialization stream, we assume it will come
** from the terminal (eventually), and we set that as the source,
** and return EOF.
*/

static int
rother()
{
    extern int rINIch();

    int c;			/* the character */

    if (ungotchar != EOF)
    {
	c = ungotchar;		/* save character */
	ungotchar = EOF;	/* forget ungotten char */
	Inpsrc = oldsrc;	/* return old source */
	return(c);		/* and the old character */
    }

    if ( (c=rEIch()) != EOF )	/* try EI file */
    {
	Inpsrc = SRC_EI;	/* say char is from EI */
	return(c);		/* return it */
    }

    if ( (c=rINIch()) != EOF )	/* any initialization chars? */
    {
	Inpsrc = SRC_INI;	/* yes.  set source */
	return(c);
    }

    Inpsrc = SRC_TTY;		/* default is from terminal */
    return(EOF);		/* return EOF for this case */
}
/* rCch -- read command character
 *
 * This routine returns the next command character, taking into account
 * any pushed back characters.  It also takes care of case conversions
 * and conversions of ESC equivalents.
 */

int				/* returned char */
rCch()
{
    extern int rIch();
    int c;			/* the character to return */

    c = rIch(TRUE);		/* get input character, convert terminal
				** CR to CR/LF
				*/

/* This is a ****crock****:  to handle older terminals with ALT MODE
 * keys, we translate such character codes to ESCape if the terminal
 * is claimed not to handle lower case input
 */

    if (    (ET & ET_rlc) == 0
	&&  (c == ALTMOD1 || c == ALTMOD2)
	)
	c = ESC;		/* convert */

/* if we have read a lower case character and the ET flag says we should
 * not read lower case, we convert the character to (presumably) upper
 * case
 */

    if (Etable[c].type == Ec_lc && ((ET & ET_rlc) == 0))
	c = Etable[c].newchar;	/* substitute corrected char */

    return(c);			/* return command char */
}
/* rIch -- read input character
**
** This routine returns the next character read from the terminal,
** an EI file, or the initialization stream.  The input from an EI
** file looks identical to terminal input.  If the flag is TRUE,
** carriage returns at the terminal are expanded to CR/LF.
** Inpsrc is set with the input source for the character.
*/

int
rIch(flag)
BOOL flag;			/* TRUE to change terminal CR to CR/LF */
{
    extern int rTch();

    register int i;		/* next character read */

    if ( (i = rother()) == EOF)	/* get character from other than terminal */
	i = rTch(flag);		/* if EOF, get from terminal */

    return (i);			/* return the character */
}
/* rTch -- read terminal character
**
** This routine returns the next character read from the terminal.
** A carriage return gets changed into a carriage return/line feed
** pair if flag is TRUE.
**
** An EOF can be returned if the read is to satisfy a ^T command
** and read-with-no-wait has been set.  We guarantee that -1 is
** returned in that case.  Otherwise we return the character we read.
*/

int
rTch(flag)
BOOL flag;			/* TRUE to convert CR to CR/LF */
{
    static BOOL lfpending = FALSE;
    extern BOOL Xnlcr;		/* translate incoming NL to CR */

    int i;			/* new character */

    if (lfpending)		/* return pending LF */
    {
	lfpending = FALSE;
	return(LF);
    }

    i = getc(Ttyin);		/* get next terminal character */

    if (i == LF && Xnlcr)	/* NL is a line feed; translate NL to CR */
	i = CR;

    if (i == CR && flag)	/* on CR, note LF pending, if requested */
	lfpending = TRUE;
    else if (i == EOF)
	i = -1;			/* -1 specified for end of file */
    
    return(i);			/* return character */
}
/* whCch -- where is next command character from?
**
** This routine returns the source of the next input character,
** rather than the character itself.  The character will next
** be returned by rCch.
*/

int
whCch()
{
    int c;			/* character that gets read */

    if ( (c = rother()) != EOF)	/* if we get a character from non-tty */
	uCch(c);		/* unget it */
    
    return(Inpsrc);		/* Inpsrc set correctly in either case */
}
/* wCch -- write command character
 *
 * Actually, this routine writes more than a command character.  It is
 * the principle character writer to the terminal, taking care of
 * changing cases, flagging control characters, etc.  wCch is sensitive
 * to the settings of the various TECO output control bits.
 */

void
wCch(c,file)
int c;				/* original character to write */
FILE * file;			/* stream to write char to */
{
    void wCch1();		/* declare function */
    void interr();

    int cnew = Etable[c].newchar; /* get secondary character, just in case */

    if ((ET & ET_image) != 0)	/* handle image mode */
    {
	wCch1(c,file);		/* write the character as is */
	return;
    }

    switch (Etable[c].type)	/* dispatch on character type */
    {
    case Ec_ctrl:		/* control char */
	wCch1('^',file);	/* write ^<s> */
	wCch1(cnew,file);
	break;
    
    case Ec_lc:			/* lower case */
	if (EU == 0)		/* if we flag lower case chars */
	{
	    wCch1(LCFLAG,file);	/* write flag */
	    wCch1(cnew,file);	/* write second char */
	}
	else
	    wCch1(c,file);	/* else, write original char */
	break;

    case Ec_uc:			/* upper case */
	if (EU > 0)		/* if flag upper case */
	{
	    wCch1(UCFLAG,file);	/* write flag */
	    wCch1(cnew,file);	/* write second char */
	}
	else
	    wCch1(c,file);	/* else, write original char */
	break;
/* this is the complicated case of coded chars */

    case Ec_coded:
    /* switch on which character */
	switch (c)
	{
	case FF:		/* form feed */
	case VT:		/* vertical tab */
	case BS:		/* back space */
	/* do these only for hard-copy terminals */
	    if ((ET & ET_crt) !=0)
	    {
		wCch1('^',file);/* for crt, treat like ctrl char */
		wCch1(cnew,file);
	    }
	    else
		wCch1(c,file);	/* else, write original char */
	    break;
	
	case TAB:		/* horizontal tab:  expand to spaces */
	    do			/* always do one */
	    {
		wCch1(SPACE,file); /* output space */
	    } while ((pos & 07) != 1);
	    break;

	case BEL:		/* BEL char:  echo as ^G and ring bell */
	    wCch1('^',file);
	    wCch1('G',file);
	    wCch1(BEL,file);
	    break;
	
	default:
	    interr("missed coded char in wCch");
	}
	break;

    case Ec_repl:			/* replace character */
	c = cnew;			/* replace and fall through */
    default:				/* for normal */
	wCch1(c,file);			/* just write character */
    case Ec_null:			/* for null case, do nothing */
	break;
    }
    return;
}
/* wCch1 -- write uncased command char
 *
 * This routine is really a subroutine of wCch.  Its main function is to
 * get characters to the terminal, handle horizontal cursor position,
 * and take care of line truncation
 */

void
wCch1(c,file)
int c;					/* character to write */
FILE * file;				/* stream to write to */
{
    int posinc = Etable[c].posinc;	/* position incr. for char */

    Testinterrupt();			/* check for interrupts first */

    if (pos <= 0 || c == CR)
	pos = 1;			/* reset position */
    if (
	(pos <= Twidth ||		/* if we start out in view... */
        (pos + posinc) <= Twidth ||	/* ... or we end up in view... */
	(ET & ET_trunc) == 0) &&	/* ... or we're not truncating */
	! Fl_ctO			/* ... and ^O isn't set */
	)
	    (void) putc(c,file);	/* write char */

    if (! Fl_ctO)
	pos += posinc;			/* bump position if not ^O */

/* Note that we have monitored the horizontal position, whether or not
 * we actually truncate.  If we manage to backspace back into the field
 * of vision on a device that truncates and does backspace, we will
 * show the movement accordingly
 */
    return;
}
/* Erasechar -- erase character
**
** This routine must do the inverse of what wCch does, in terms
** of knowing what wCch would have printed for the corresponding
** character.  Therefore it uses the same table, but it makes
** certain assumptions about things like CR and LF.
** It must also take care of the nitty-gritty details around
** line-wrap boundaries, if it can.
*/

void
Erasechar(c)
char c;					/* character being erased */
{
    void echoline();			/* routine to echo current command
					** line (1 only) on CRT
					*/
    void crtechar();			/* erase CRT char */
    void crteline();			/* erase CRT line */

#ifdef	CRTRUB

    if ((ET & ET_crt) == 0)		/* if not a CRT */
    {
#endif
	wCch(c,Ttyout);			/* write char to terminal */
	return;
#ifdef	CRTRUB
    }

    if (c == CR)			/* backing up over CR */
    {
	echoline();			/* then repeat current line (cheap(?)
					** way to get cursor to end of line)
					*/
	return;
    }

    if (c == LF)			/* backing up over LF */
    {
	Crtupline();			/* move up a line */
	return;
    }
/* Here's the meat of the routine.  Take care of other chars. */

    switch( Etable[c].type )
    {
    case Ec_ctrl:			/* control char:  erase 2 */
	crtechar();
    default:				/* normal case:  erase 1 */
	crtechar();
    case Ec_null:			/* null case:  do nothing */
	break;

    case Ec_lc:				/* lower case alpha */
	if (EU == 0)			/* one or two, depending of flag */
	    crtechar();
	crtechar();
	break;

    case Ec_uc:				/* upper case alpha */
	if (EU > 0)			/* one or two, depending on flag */
	    crtechar();
	crtechar();
	break;

    case Ec_coded:			/* special coded chars:  invert
					** what wCch does, remembering
					** that we know this is a crt
					*/
	switch (c)
	{
	case BEL:			/* treat these like ctrl chars */
	case FF:
	case VT:
	case BS:
	    crtechar();
	    crtechar();
	    break;
	
	case TAB:
	    echoline();
	    break;

	default:			/* unknown */
	    interr("missed coded char in Erasechar");
	}
    }
    return;

#endif	/* def CRTRUB */

}


/* Eraseline -- erase current command line
**
** This routine does the appropriate processing for erasing
** a command line.  If the terminal is a CRT, we wipe out
** the line.  Otherwise we just output a CRLF.
*/

void
Eraseline()
{
    void Crlf();
    void crteline();

#ifdef	CRTRUB
    if ((ET & ET_crt) == 0)
#endif

	Crlf(Ttyout);			/* to terminal */

#ifdef	CRTRUB
    else
	crteline();			/* otherwise, erase line */
#endif
    return;
}
/* crtechar -- erase CRT character
**
** This routine does that, and it adjusts our line position.
*/

#ifdef	CRTRUB

static void
crtechar()
{
    Crtechar();
    pos--;				/* we backed up one position */
    return;
}
/* crteline -- erase CRT line
**
** This routine erases a CRT line and resets our line position.
*/

static void
crteline()
{
    Crteline();				/* erase the line */
    pos = 0;				/* reset terminal horizontal pos. */
    return;
}
/* echoline -- echo line to CRT
**
** This routine echoes the current line on the screen, CRT-style.
** We assume we need to erase some stuff first.
** We take a slightly different (more correct?) approach than
** standard DEC TECO here.  We re-write the current line from the
** most recent Carriage Return, on the grounds that that's the only
** way we know when the cursor was at the left margin (our only
** point of reference).
*/

static void
echoline()
{
    char * first = TBtext(Q_cmd) + 0;	/* start of command string */
    char * cur = first + TBsize(Q_cmd) - 1;
					/* current command position */
    int vertspace = 0;			/* number of vertical spaces to
					** back up over to get to CR
					*/
    BOOL lastLF = FALSE;		/* remember if prev. char was LF */
    void wTBst();
#define	PROMPT	'*'			/* prompt character */

    /* when command text block is empty, echo prompt on new line */

    if (TBsize(Q_cmd) == 0)
    {
	crteline();
	wCch1(PROMPT, Ttyout);
	return;
    }

    /* walk backwards until we find a CR or start of the command string */

    for ( ; cur >= first; cur-- )
    {
	if (*cur == CR)
	{
	    cur++;			/* don't display CR */
	    if (lastLF)			/* if there's a following LF, don't
					** bother repeating it
					*/
	    {
		vertspace--;
		cur++;
	    }
	    break;			/* out of loop */
	}

	if (lastLF = (*cur == LF))	/* count LF's */
	    vertspace++;
    }
/* Now we've either found a CR or the start of the command string. */

    if (cur < first)
	cur = first;			/* if we've walked past the start */
    
    /* Move up screen over any LF's */

    while (vertspace-- > 0)
	Crtupline();
    
    crteline();				/* Erase from here to end of screen */

    if (cur == first)			/* Display original prompt char again */
	wCch1(PROMPT, Ttyout);
    
    /* Now redisplay the "line" from the last CR */

    wTBst(Q_cmd, cur-first, TBsize(Q_cmd) - (cur-first), Ttyout);

    return;
}

#endif	/* def CRTRUB */
/* wTBst -- write text block string
 *
 * This routine writes the (partial) contents of a text block through
 * wCch.
 */

void
wTBst(tb,first,len,file)
int tb;					/* text block number */
int first;				/* first position to write */
int len;				/* number of chars to write */
FILE * file;				/* file to write to */
{
    char * cp = TBtext(tb) + first;	/* starting position */
    static char buf[BUFSIZ];		/* use this locally only to improve
					** performance
					*/

    (void) fflush(file);		/* flush current file content */
    setbuf(file,buf);			/* set up bigger buffer */

    while (len-- > 0)
	wCch(*cp++,file);

    (void) fflush(file);		/* flush out of the local buffer */
    setbuf(file,NULL);			/* reset to small buffer */
    return;
}
/* Crlf -- write carriage return, line feed to terminal */

void
Crlf(file)
FILE * file;				/* stream to write to */
{
    wCch1(CR,file);
    wCch1(LF,file);
    return;
}
