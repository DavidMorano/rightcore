static char SCCSID[] = "@(#) xec.c:  5.1 2/22/83";
/* xec.c
 *
 *	TECO command interpreter
 *
 *	David Kristol, February, 1982
 *
 *	This module contains the main command interpreter loop for TECO.
 *	The module also contains other useful routines for command interpreting.
 *	In particular:
 *		Xecimmed	execute literal command string
 *		Xec		execute commands
 *		Skip		skip commands
 *		pCMch		peek at next command char
 *		gCMch		get next command char, trace
 *		Pushx		push on execute stack
 *		Popx		pop from execute stack
 *
 *	Because of the context dependencies of TECO commands, skipping
 *	over command strings for conditionals and other control constructs
 *	is a tricky business.  The way we handle this is to build the smarts
 *	into each command.  Each command knows what to do in a non-skipping
 *	(Skiptype = SKNONE) and non-skipping state.  In general non-control
 *	structure commands eat up the rest of their arguments when in a
 *	skipping state, then exit.  On the other hand, control constructs
 *	check whether they are the construct that's being looked for and
 *	act accordingly.
 *
 *	Command routines return 'short's.  In a non-skipping state they return
 *	zero.  In a skipping state they return SKIPDONE to exit the current
 *	skipping level, and SKIPCONT to continue skipping.
 */

#include <stdio.h>
#include "bool.h"		/* declare booleans */
#include "ctype.h"		/* declare TECO character predicates */
#include "errors.h"		/* declare error structures */
#include "errmsg.h"		/* declare error messages */
#include "exittype.h"		/* declare Xec and Skip exit types */
#include "mdconfig.h"		/* declare machine-dependent configuration */
#include "qname.h"		/* define Q_text */
#include "skiptype.h"		/* define skipping types */
#include "tb.h"			/* declare text block routines */
#include "tflags.h"		/* TECO's flag bit definitions */
#include "tty.h"		/* declare terminal stuff */
#include "values.h"		/* declare value stuff */
#include "xectype.h"		/* declare execute types */
/* Data available globally to the other interpreter routines.
 * Also declared in xec.h.
 */

int Dot;			/* current text buffer position */

int Cmddot;			/* current position in command buffer */
short Cmdtb;			/* text block from which we're executing */

short Xectype;			/* execution type:  execution type, defined
				 * in xectype.h
				 */

short Cmdchar;			/* most recent command character */

BOOL Fl_at;			/* @ flag:  TRUE if seen */
short Fl_colon;			/* : flag:  number of colons seen */
BOOL Fl_trace;			/* trace flag:  TRUE if tracing on */

int Inslen;			/* length of last insert, search, or Q-reg
				 * insertion (value of ^S, that is )
				 */

short Radix;			/* current radix:  8, 10, 16 */

short Skiptype;			/* current type of skipping in progress */

short xstackp;			/* execution stack pointer */
int xstack[MAXXSTACK];		/* execution stack */

extern TNUMB ET;		/* TECO ET flag */
/* Small utility routines */

/* pCMch -- peek at next command character 
 *
 * This routine returns the next command character without tracing or
 * bumping the pointer.  It returns -1 if at end of text block.
 */

int
pCMch()
{
    if (Cmddot >= TBsize(Cmdtb))
	return(-1);			/* past end */
    return ( *(TBtext(Cmdtb) + Cmddot)); /* return current char */
}


/* gCMch -- get next command character
 *
 * gCMch does what pCMch does, but it bumps the command pointer and
 * does a trace
 */

int
gCMch()
{
    void wTRch();
    register int c = pCMch();		/* get next */

    if (c >= 0)
    {
	wTRch(c);			/* trace */
	Cmddot++;			/* bump position */
    }
    return(c);				/* return char */
}
/* wTRch -- write trace character
 *
 * This routine writes a command character as part of the trace function.
 */

void
wTRch(c)
int c;					/* character to write */
{
    void wCch();

    if (Fl_trace)			/* if tracing is on */
	wCch(c,Cmdout);			/* write command char */
    return;
}

/* output proper error on unterminated command */

void
Unterm()
{
    if (Cmdtb == Q_cmd)
	terrCHR(&Err_UTC,Cmdchar);	/* unterminated command */
    else
	terrNUL(&Err_UTM);		/* unterminated macro */
}
/* Executive stack management routines */

/* Pushx -- push onto stack
 *
 * This routine pushes an integer value onto the execution stack after
 * check for overflow.
 */

void
Pushx(x)
{
    if (xstackp > MAXXSTACK)
	terrNUL(&Err_PDO);		/* push down overflow */
    xstack[xstackp++] = x;		/* otherwise stack value */
    return;
}


/* Popx -- pop from stack
 *
 * This routine removes the top value from the execution stack.
 */

int
Popx()
{
    void interr();

    if (xstackp <= 0)
	interr("stack underflow in Popx");
    
    return(xstack[--xstackp]);
}
/* Xecimmed -- execute literal TECO command string
**
** This routine executes a static literal command string.
** The command string is statically assigned to text block Q_cmd.
*/

void
Xecimmed(s)
char * s;				/* TECO command string to execute */
{
    extern int strlen();
    extern void Xec();

    BOOL savetrace = Fl_trace;		/* save old trace flag */

    Fl_trace = FALSE;			/* disable tracing for this */
    TBstatic(Q_cmd,s,strlen(s));	/* set up text block */
    Xec();				/* execute it */
    TBkill(Q_cmd);			/* kill off text block */
    Fl_trace = savetrace;		/* restore trace flag */
    return;
}
/* Xec -- execute TECO command string
 *
 * This is the interpreter main code.  It returns when it runs out of
 * characters.
 */

void
Xec()
{
    extern short (* (CMdisp[]))();	/* declare dispatch array */
    short Exitmacro();

/* initialize important flags and such */

/* prepare terminal for execution according to ET_rnow flag (no wait) */
    if ((ET & ET_rnow) == 0)
	Ttsxec();			/* ^T waits for character */
    else
	Ttsxecnd();			/* ^T does not wait for character */

    Cmdtb = Q_cmd;			/* interpret from command buffer */
    Cmddot = 0;				/* start at beginning */
    Xectype = XECINPUT;			/* interpreting input */

    Pstackp = 0;			/* current paren stack pointer */
    Stackdepth = 0;			/* current local stack depth */
    xstackp = 0;			/* current execution stack pointer */

    Fl_at = FALSE;			/* discard flags and values */
    Fl_colon = 0;
    Valinit();
    Skiptype = SKNONE;			/* not presently skipping */
/* continue until we run off the end or someone tells us to quit or
 * an interrupt occurs
 */

    while (TRUE)
    {
	if ((Cmdchar = gCMch()) < 0)	/* ran off end */
	    if (Exitmacro(FALSE) == XECDONE) /* reach top level after exiting
					     ** macro? (preserve values)
					     */
		break;			/* yes.  Exit */
	    else
		continue;		/* no.  Continue executing in new macro:
					** pick up new character
					*/

	if (CMdisp[Cmdchar]() == XECDONE) /* dispatch and execute */
	    break;			/* exit loop if told to exit */
#ifdef DEBUG
	if (Dot < 0 || Dot > TBsize(Q_text)) /* double-check position */
	    interr("bad Dot after command in xec");
#endif
	Testinterrupt();		/* test for interrupt */
    }
    return;				/* done executing */
}
/* Skip -- skip over commands
 *
 * This routine effects control structure skipping as for goto's, conditionals
 * and F commands.  Although it is recursive, it isn't used that way.
 * Because TECO commands are context sensitive, we must skip over them
 * carefully.  For example, to skip over an insertion ( Iabcde$ ) we must
 * recognize the I and then read its text argument.  Of course, the same
 * insert could be written as ( @I/abcde/ ).  Consequently each command
 * is responsible for knowing how to do skipping over all its parts if
 * some skip state is in effect.
 *
 * The types of skipping are:
 *	SKGOTO	skip to label of goto
 *	SKCOND	skip to next | or ' at current level
 *	SKFI	skip to next ' at current level
 *	SKITER	skip to next > at current level
 *
 * The key to the complications is "at current level".  Iterations and
 * conditionals do not need to be properly nested.  In fact, a common
 * idiom in TECO is
 *	<  ..."c ...>'
 * where failure to execute the insides of the conditional results in
 * exiting the conditional.  This turns out to be messy to implement,
 * because the conditionals and iterations must be dealt with "at the
 * same hierarchical level, in some sense.
 *
 * The general scheme employed is to set a "skipping state" when a construct
 * is encountered that demands that commands be skipped.  Most commands
 * processors skip over all argument characters and return an indication
 * that requires that skipping continue.  However, those commands which
 * play a part in the conditional execution mechanism, such as " | ' ,
 * must be cognizant of the skipping states and do appropriate things.
 * See individual commands for details.
 */
short					/* SKIPDONE if found item,
					 * successfully, SKIPRANOFF if we ran
					 * off the end trying
					 */
Skip(type)
short type;				/* skip type */
{
    short oldtype = Skiptype;		/* save old one */
    short c;				/* command char */

    extern int Condnest;		/* conditional nesting level */
    extern int Iternest;		/* iteration nesting level */
    extern short (* (CMdisp[]))();	/* declare dispatch array */
    void Unterm();

    if (oldtype == SKNONE)		/* if not previously skipping... */
    {
	Condnest = 0;			/* initialize nesting levels of */
	Iternest = 0;			/*   conditionals and interations */
    }

/* This loop is nearly identical to the one in Xec.  Note that we don't
 * change Cmdchar, since we want that around for error messages if we
 * run off the end of some text block
 */

    Skiptype = type;			/* copy skip type */

    while ( (c = gCMch()) >= 0 &&
	    (CMdisp[c]()) == SKIPCONT)
	Testinterrupt();		/* just test for interrupts */

    Skiptype = oldtype;			/* restore type */
    if (c < 0)				/* run off end? */
	return(SKIPRANOFF);		/* yes */
    else
	return(SKIPDONE);		/* otherwise return "done" */
}
/* Some simple catch-all command handlers */

/* this one is for ^ commands */

static short
CMupar()
{
    short c;				/* second char */
    void Unterm();
    extern short (* (CMdisp[]))();	/* dispatch array */

    if ( (c = gCMch()) < 0)		/* check for another char */
	Unterm();			/* unterminated command */
    if ((c = TRCTRL(c)) < 0 &&
	Skiptype != SKNONE)		/* disregard bad char if skipping */
	return(SKIPCONT);
    if (Skiptype == SKNONE)		/* if not skipping, make this new
					 * command char
					 */
	Cmdchar = c;
    return(CMdisp[c]());		/* dispatch to routine */
}

/* this one is for illegal commands */

short
badcmd()
{
    if (Skiptype == SKNONE)		/* only if not skipping */
	terrCHR(&Err_ILL,Cmdchar);	/* output message */
    else
	return(SKIPCONT);		/* otherwise, ignore command */
/*NOTREACHED*/
}
/* this one is for not-yet-implemented ones */

short
nyi()
{
    if (Skiptype == SKNONE)		/* only if not skipping */
	terrNUL(&Err_NYI);		/* output message */
    else
	return(SKIPCONT);		/* otherwise ignore */
/*NOTREACHED*/
}


/* this one is for null commands */

short
nulcmd()
{
    return(CONTINUE);			/* just continue executing/skipping */
}
/* Main command table
 *
 * This table contains a pointer to a function for every valid
 * ASCII character.  Some of these pointers are to routines
 * that generate error messages
 */

/* declare functions mentioned below */

extern short
	CMacmd(), CMamp(), CMapos(), CMatsign(),
	CMbar(), CMbcmd(), CMbksl(), 
	CMccmd(), CMcolon(), CMcomma(),
	CMcta(), CMctbcmd(), CMctccmd(), CMctdcmd(),
	CMctecmd(), CMctfcmd(), CMcthcmd(),
	CMcticmd(), CMctncmd(), CMctocmd(), CMctqcmd(),
	CMctrcmd(), CMctscmd(), CMcttcmd(),
	CMctu(), CMctund(), CMctupcmd(),
	CMctxcmd(), CMctycmd(), CMctzcmd(),
	CMdcmd(), CMdigit(), CMdotcmd(), CMdquot(),
	CMecmd(), CMeq(), CMesc(), CMexcl(),
	CMfcmd(), CMff(), CMgcmd(),
	CMgt(), CMhcmd(), CMicmd(), CMjcmd(),
	CMkcmd(), CMlbrk(),  CMlcmd(), CMlpar(), CMlt(), CMmcmd(),
	CMminus(), CMncmd(), CMocmd(), CMpcmd(), CMper(), CMplus(),
	CMpound(), CMqcmd(), CMquest(), CMrbrk(), CMrcmd(),
	CMrpar(), CMscmd(), CMsemi(), CMslash(), CMtcmd(),
	CMtimes(), CMucmd(), CMundcmd(), CMupar(),
	CMvcmd(), CMxcmd(), CMycmd(), CMzcmd();


static short (* (CMdisp[]))() =
{
    nulcmd,	/*	NUL	000	*/
    CMcta,	/*	^A	001	*/
    CMctbcmd,	/*	^B	002	*/
    CMctccmd,	/*	^C	003	*/
    CMctdcmd,	/*	^D	004	*/
    CMctecmd,	/*	^E	005	*/
    CMctfcmd,	/*	^F	006	*/
    badcmd,	/*	^G	007	*/
    CMcthcmd,	/*	^H	010	*/
    CMcticmd,	/*	^I	011	*/
    nulcmd,	/*	LF	012	*/
    nulcmd,	/*	VT	013	*/
    CMff,	/*	^L	014	*/
    nulcmd,	/*	CR	015	*/
    CMctncmd,	/*	^N	016	*/
    CMctocmd,	/*	^O	017	*/
    badcmd,	/*	^P	020	*/
    CMctqcmd,	/*	^Q	021	*/
    CMctrcmd,	/*	^R	022	*/
    CMctscmd,	/*	^S	023	*/
    CMcttcmd,	/*	^T	024	*/
    CMctu,	/*	^U	025	*/
    badcmd,	/*	^V	026	*/
    badcmd,	/*	^W	027	*/
    CMctxcmd,	/*	^X	030	*/
    CMctycmd,	/*	^Y	031	*/
    CMctzcmd,	/*	^Z	032	*/
    CMesc,	/*	ESC	033	*/
    badcmd,	/*	^\	034	*/
    badcmd,	/*	^]	035	*/
    CMctupcmd,	/*	^^	036	*/
    CMctund,	/*	^_	037	*/
    nulcmd,	/*	SPACE	040	*/
    CMexcl,	/*	!	041	*/
    CMdquot,	/*	"	042	*/
    CMpound,	/*	#	043	*/
    badcmd,	/*	$	044	*/
    CMper,	/*	%	045	*/
    CMamp,	/*	&	046	*/
    CMapos,	/*	'	047	*/
    CMlpar,	/*	(	050	*/
    CMrpar,	/*	)	051	*/
    CMtimes,	/*	*	052	*/
    CMplus,	/*	+	053	*/
    CMcomma,	/*	,	054	*/
    CMminus,	/*	-	055	*/
    CMdotcmd,	/*	.	056	*/
    CMslash,	/*	/	057	*/
    CMdigit,	/*	0	060	*/
    CMdigit,	/*	1	061	*/
    CMdigit,	/*	2	062	*/
    CMdigit,	/*	3	063	*/
    CMdigit,	/*	4	064	*/
    CMdigit,	/*	5	065	*/
    CMdigit,	/*	6	066	*/
    CMdigit,	/*	7	067	*/
    CMdigit,	/*	8	070	*/
    CMdigit,	/*	9	071	*/
    CMcolon,	/*	:	072	*/
    CMsemi,	/*	;	073	*/
    CMlt,	/*	<	074	*/
    CMeq,	/*	=	075	*/
    CMgt,	/*	>	076	*/
    CMquest,	/*	?	077	*/
    CMatsign,	/*	@	100	*/
    CMacmd,	/*	A	101	*/
    CMbcmd,	/*	B	102	*/
    CMccmd,	/*	C	103	*/
    CMdcmd,	/*	D	104	*/
    CMecmd,	/*	E	105	*/
    CMfcmd,	/*	F	106	*/
    CMgcmd,	/*	G	107	*/
    CMhcmd,	/*	H	110	*/
    CMicmd,	/*	I	111	*/
    CMjcmd,	/*	J	112	*/
    CMkcmd,	/*	K	113	*/
    CMlcmd,	/*	L	114	*/
    CMmcmd,	/*	M	115	*/
    CMncmd,	/*	N	116	*/
    CMocmd,	/*	O	117	*/
    CMpcmd,	/*	P	120	*/
    CMqcmd,	/*	Q	121	*/
    CMrcmd,	/*	R	122	*/
    CMscmd,	/*	S	123	*/
    CMtcmd,	/*	T	124	*/
    CMucmd,	/*	U	125	*/
    CMvcmd,	/*	V	126	*/
    nyi,	/*	W	127	*/
    CMxcmd,	/*	X	130	*/
    CMycmd,	/*	Y	131	*/
    CMzcmd,	/*	Z	132	*/
    CMlbrk,	/*	[	133	*/
    CMbksl,	/*	\	134	*/
    CMrbrk,	/*	]	135	*/
    CMupar,	/*	^	136	*/
    CMundcmd,	/*	_	137	*/
    badcmd,	/*	`	140	*/
    CMacmd,	/*	a	141	*/
    CMbcmd,	/*	b	142	*/
    CMccmd,	/*	c	143	*/
    CMdcmd,	/*	d	144	*/
    CMecmd,	/*	e	145	*/
    CMfcmd,	/*	f	146	*/
    CMgcmd,	/*	g	147	*/
    CMhcmd,	/*	h	150	*/
    CMicmd,	/*	i	151	*/
    CMjcmd,	/*	j	152	*/
    CMkcmd,	/*	k	153	*/
    CMlcmd,	/*	l	154	*/
    CMmcmd,	/*	m	155	*/
    CMncmd,	/*	n	156	*/
    CMocmd,	/*	o	157	*/
    CMpcmd,	/*	p	160	*/
    CMqcmd,	/*	q	161	*/
    CMrcmd,	/*	r	162	*/
    CMscmd,	/*	s	163	*/
    CMtcmd,	/*	t	164	*/
    CMucmd,	/*	u	165	*/
    CMvcmd,	/*	v	166	*/
    nyi,	/*	w	167	*/
    CMxcmd,	/*	x	170	*/
    CMycmd,	/*	y	171	*/
    CMzcmd,	/*	z	172	*/
    badcmd,	/*	{	173	*/
    CMbar,	/*	|	174	*/
    badcmd,	/*	}	175	*/
    badcmd,	/*	~	176	*/
    badcmd	/*	DEL	177	*/
};
