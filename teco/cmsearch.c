static char SCCSID[] = "@(#) cmsearch.c:  4.2 12/19/82";
/* cmsearch.c
**
**	TECO search (and replace) support routines and commands
**
**	David Kristol, April, 1982
**
**	This module contains all of the TECO search-related routines.
**	Search commands in TECO are a mess.  They are being cleaned up
**	by the TECO SIG, but we try to maintain backward compatibility
**	here.  The features that are particularly ugly to support are:
**
**	m,nS	bounded search
**	{:};	implicit value return when a search completes in an iteration
**
**	The search (and replace) commands are:
**
**	search only:	FB  N  EN  S  _  E_
**	search/replace:	FC  FN  FS  F_
**
**	The basic template for search commands is something like this:
**
**	1)  Delimit the search string.
**	2)  Build a "search string" using the "string build" operators.
**	3)  Convert the search string to a command sequence that we can
**		execute quickly.
**	4)  Execute the command sequence as needed on the current and
**		subsequent buffers, as appropriate, until the iteration
**		count or file is exhausted.
**	5)  Report an error or set a value, as appropriate.
**
**	We create various local routines to help with the common parts of
**	the functions outlined above.
*/

/* Search execution list
**
**	The search commands build a "search execution list" which
**	is a pre-interpreted list of directives based on the search
**	string.  Some of the directives take arguments which follow
**	the directive.  The various kinds of directives are:
**
**	M_1char		match single literal character
**	M_alnu		match any single alphabetic or numeric character
**	M_any		match any one character
**	M_dig		match any single numeral
**	M_inv		invert the success or failure of the next directive
**	M_last		end of execution list
**	M_lc		match any single lower case letter
**	M_let		match any single letter, either case
**	M_literal	match a string literally; args:
**				start position of string in Q_search
**				length of string
**	M_lt		match any single line terminator
**	M_nocase	match string without regard for character cases; args:
**				start position of string in Q_search
**				length of string
**	M_oneof		match any of the characters in Q-register; arg:
**				text block # of Q-register
**	M_sep		match any single separator
**	M_space		match any sequence of spaces or TABs
**	M_bspace	match any sequence of spaces or TABs, but look
**			backward, as well as forward
**	M_sym		match any single symbol constituent
**	M_uc		match any singw3le upper case letter
*/


/*	M_bspace is a funny case that only applies as the first directive
**	of a search execution list in a backward search.  The problem is
**	that in a backward search, as we step backwards and try to match,
**	the first space/TAB we see would match.  We actually want to find
**	the left-most space/TAB and call that part of the match.  M_bspace
**	handles this special case.
*/
/* include files */

#include "bool.h"
#include "chars.h"
#include "ctype.h"
#include "cmutil.h"
#include "errors.h"
#include "errmsg.h"
#include "exittype.h"
#include "find.h"
#include "mdconfig.h"
#include "memory.h"
#include "qname.h"
#include "skiptype.h"
#include "tb.h"
#include "tflags.h"
#include "values.h"
#include "xec.h"
#include "xectype.h"


extern TNUMB ED;			/* ED flag */
extern TNUMB ES;			/* ES flag */
extern TNUMB Fl_ctX;			/* ^X flag */

#ifndef	NULL
#define	NULL	((char *) 0)
#endif
/* define local structures and module globals */

/* Allocate space for the search execution list */

static int searchinst [MAXSEARCH];


/* Declare variables that are global to this module.  They are used
** extensively to pass values between functions in contradiction to
** all the best interests of structured programming, but in the interests
** of speed.  Sorry about that.
*/


static char *	Buffirst;		/* pointer to beginning of portion of
					** text buffer to match
					*/
static char *	Buflast;		/* pointer to end of same +1 */
static char *	Bufbound;		/* pointer to last possible place
					** at which to begin match
					*/

static char *	Csptr;			/* pointer to current position in
					** text buffer for searching
					*/
static int *	Cspc;			/* current search "program counter" */

static int	Sminlen;		/* minimum length of a matching search
					** string (bytes)
					*/
static int	Siter;			/* number of desired matches (for
					** example, 'n' in nSfoo$)
					*/
static int	Smatch;			/* length of the matched string */

static int	Replfirst;		/* on replace, position of first char */
static int	Repllen;		/* on replace, length of replacement
					** string
					*/
static BOOL	Replflag;		/* TRUE if search followed by replace */
/* Search directive definitions */

#define	M_last		0	/* last directive in list */
#define	M_1char		1	/* match one character */
#define	M_alnu		2	/* match alphanumeric */
#define	M_any		3	/* match any char */
#define	M_dig		4	/* match digit */
#define	M_inv		5	/* invert sense */
#define	M_lc		6	/* match lower case letter */
#define	M_let		7	/* match letter */
#define	M_literal	8	/* match literal string */
#define	M_lt		9	/* match line terminator */
#define	M_nocase	10	/* match string without regard for case */
#define	M_oneof		11	/* match a character from Q-register */
#define	M_sep		12	/* match any separator */
#define	M_space		13	/* match space and TABs */
#define	M_bspace	14	/* match space and TABs backwards */
#define	M_sym		15	/* match any symbol constituent */
#define	M_uc		16	/* match upper case letter */

#define	M_dummy		17	/* dummy function:  not used in matchonce */
/* srchinit -- initialize for search command
**
** This routine does the common set-up for the search commands.  It
** first delimits the search string.  If this is a search and replace,
** we also delimit the replacement string.  If we are skipping rather than
** executing, the routine exits forthwith.  Otherwise it proceeds to
** build the "search string" and the search command string.
*/

static BOOL				/* TRUE if currently skipping */
srchinit(flag)
BOOL flag;				/* TRUE if search and replace,
					** FALSE for vanilla search.
					*/
{
    void buildsearch();

    int first;				/* initial pos. of search string */
    int len;				/* length of search string */

    if (flag)				/* search and replace? */
	Find2dlm(ESC,&first,&len,&Replfirst,&Repllen);
					/* get two delimited strings */
    else
	Finddlm(ESC,&first,&len);	/* delimit the search string */

    if (Skiptype != SKNONE)		/* if skipping, exit now */
	return(TRUE);
    
    Inslen = 0;				/* length of search match in case
					** search fails
					*/

    Replflag = flag;			/* remember whether replacing */

    if (len > 0)			/* if 0 length, use old string */
	Buildstring(Q_search,first,len,TRUE);
					/* else, build new search string,
					** leave quote chars alone
					*/
    
    buildsearch();			/* build search command string from
					** search string
					*/
    return(FALSE);			/* signal we keep going */
}
/* buildsearch -- build search command string
**
** This routine builds the search execution sequence that speeds searches.
** We do this in a big loop that walks through the current "search string"
** in Q_search.  The usual presumption is that we are working on some
** kind of string (literal or non-cased, depending on the ^X flag).  When
** we encounter some other match construct, we finish up the string
** (implicitly in "putfunc") and add another new function to the search
** execution list.  ^N match constructs are handled specially:
** consecutive ^N constructs are collapsed so that in the search execution
** list there is either no or one M_inv, and it immediately precedes a
** directive.  Apologies for the unwieldy size of the routine....
*/

/* macro to fetch next character from current search string */
#define	NEXTSCHAR()	( *(TBtext(Q_search) + Searpos++) )


/* These static variables are really private to buildsearch, putfunc,
** and putsearch.
*/

static int Searlen;		/* length of Q_search */
static int Searpos;		/* current position in Q_search */
static int Strstart;		/* start of most recent non-directive string */
static int Strlen;		/* current length of that string */
static BOOL Invflag;		/* current sense of ^N (invert sense) flag */
/* Start of buildsearch */

static void
buildsearch()
{
    unsigned char c;			/* current character */

    void putsearch();
    void putfunc();
    extern short Qnum();

    Cspc = searchinst;			/* use Cspc to build execution list */

    Searpos = 0;			/* current position in Q_search */
    Searlen = TBsize(Q_search);		/* total size of search string */
    Strstart = 0;			/* initial literal string starts at 0 */
    Strlen = 0;				/* its current length */
    Sminlen = 0;			/* resultant minimum search length
					** (so far)
					*/
    Invflag = FALSE;			/* not inverting sense yet */

/* Now we loop through all of the characters in Q_search and build an
** execution list.  If the execution list gets too long, "putsearch"
** produces an error message.
*/

    while (Searpos < Searlen)		/* while still chars in Q_search */
    {
	c = NEXTSCHAR();		/* get next search string char */

	switch (c)
	{
	default:			/* default:  non-directive */
	    if (Invflag)		/* if inverting sense, this must be
					** the first character following the
					** ^N.  Handle specially.
					*/
	    {
		putfunc(M_1char,1);	/* just match one character.  (This
					** forces out a preceding M_inv.)
					*/
		putsearch((int) c);	/* match this character */
	    }
	    else			/* normal case */
		Strlen++;		/* count off one more normal char */
	    
	    break;
/*	^X	^S	^N	^Q	^R	*/

	case CTRLX:			/* ^X:  any character */
	    putfunc(M_any,1);
	    break;
	
	case CTRLS:			/* ^S:  any separator */
	    putfunc(M_sep,1);
	    break;
	
	case CTRLN:			/* ^N:  invert match sense */
	    if (Strlen > 0)		/* set up current string first */
		putfunc(M_dummy,0);	/* use dummy function to force out
					** string
					*/

	    Invflag = ! Invflag;	/* just invert current flag state */
	    break;
	
	case CTRLQ:			/* ^Q:  quote next char */
	case CTRLR:			/* ^R:  same as ^Q */
	    if (Searpos >= Searlen)	/* make sure there are more chars */
		terrNUL(&Err_ISS);	 /* there aren't */

	    c = NEXTSCHAR();		/* get next char */

	    putfunc(M_1char,1);		/* match this single character */
	    putsearch((int) c);
	    break;
/*	^E cases */

	case CTRLE:			/* ^E constructs */
	    if (Searpos >= Searlen)	/* must be at least one more char */
		terrNUL(&Err_ISS);
	    
	    c = NEXTSCHAR();		/* get next char */

	    switch (c)			/* dispatch on construct */
	    {
	    case 'a': case 'A':		/* ^EA:  any letter */
		putfunc(M_let,1);
		break;
	    
	    case 'b': case 'B':		/* ^EB:  any separator */
		putfunc(M_sep,1);
		break;
	    
	    case 'c': case 'C':		/* ^EC:  any symbol constituent */
		putfunc(M_sym,1);
		break;
	    
	    case 'd': case 'D':		/* ^ED:  any digit */
		putfunc(M_dig,1);
		break;
	    
	    case 'r': case 'R':		/* ^ER:  any alphanumeric */
		putfunc(M_alnu,1);
		break;
	    
	    case 's': case 'S':		/* ^ES:  any spaces/TABs */
		putfunc(M_space,1);
		break;
	    

	    case 'v': case 'V':		/* ^EV:  any lower case letter */
		putfunc(M_lc,1);
		break;
	    
	    case 'w': case 'W':		/* ^EW:  any upper case letter */
		putfunc(M_uc,1);
		break;
	    
	    case 'x': case 'X':		/* ^EX:  any char */
		putfunc(M_any,1);
		break;
	    
	    case 'g': case 'G':		/* ^EGq:  any char in Q-reg q */
		putfunc(M_oneof,1);

		if (Searpos >= Searlen)	/* must be able to get next char */
		    terrNUL(&Err_ISS);
		
		c = NEXTSCHAR();	/* get Q-reg name */

		putsearch((int) Qnum((int)(unsigned int) c,FALSE));
					/* Qnum gives error if bad Q-reg name.
					** Disallow special names * and _
					*/
		break;
	    
	    default:			/* bad ^E construct */
		terrCHR(&Err_ICE,(int) c);
	    } /* end ^E switch */
	} /* end main switch */
    } /* end while */

    putfunc(M_last,0);			/* mark end of execution list */
    return;
}
/* putsearch -- add word to search execution list
**
** This routine adds another word to the search execution list.  It
** produces an error when the list if full.
*/

static void
putsearch(i)
int i;					/* word to put on list */
{
    if (Cspc >= searchinst + MAXSEARCH - 1)
	terrTB(&Err_STL,Q_search);	/* search string too long */
    
    *Cspc++ = i;			/* save value on list */
    return;
}
/* putfunc -- put search function on execution list
**
** This routine puts a new function on the search execution list.
** It takes care of the messy details of setting up a preceding
** literal string, then adds the new function.  It also takes
** care of the "invert sense" flag, and of re-initializing for
** the next literal string.
*/

static void
putfunc(func,minlen)
int func;				/* search function code */
int minlen;				/* minimum length of this function */
{
    if (Strlen > 0)			/* set up string if non-0 length */
    {
	putsearch(Fl_ctX == 0 ? M_nocase : M_literal);
					/* search for literal or non-cased
					** string, depending on ^X flag
					*/
	putsearch(Strstart);		/* start position of string */
	putsearch(Strlen);		/* length of string */
	Sminlen += Strlen;		/* bump minimum search length */
    }

    if (func != M_dummy)		/* process if not dummy function */
    {
	if (Invflag)			/* put out invert if set */
	    putsearch(M_inv);
	putsearch(func);		/* put out new function */
	Invflag = FALSE;		/* no further inversion */
	Sminlen += minlen;		/* bump minimum length as requested */
    }

    Strstart = Searpos;			/* begin new string */
    Strlen = 0;				/* no chars yet */
    return;
}
/* initglobal -- initialize globals for search
**
** This routine sets some of the search global variables that need to be
** reset for each new buffer load.  We are assuming that the text space
** could be reorganized under us each time the buffer is re-read, so we
** must reset the pointer to the text buffer, for example.
**
** All of the positions are assumed to be within the buffer bounds,
** and start is assumed to be between first and last.
*/

static BOOL				/* return FALSE if match impossible */
initglobal(start,first,last)
int start;				/* starting search position in search
					** buffer
					*/
int first;				/* lowest numbered position of search
					** buffer to examine
					*/
int last;				/* highest numbered position to start
					** search at; if -1, search goes to end
					** of buffer
					*/
{
    extern void interr();
    char * bufbeg = TBtext(Q_text);	/* start of buffer */

    Smatch = 0;				/* amount matched is zero */

    if (last < 0)
	last = TBsize(Q_text) - Sminlen; /* last place to attempt a match */

#ifdef DEBUG
    if (    first < 0 || first > TBsize(Q_text)
	||  (last >= 0 && last > TBsize(Q_text))
	||  start < first || start > TBsize(Q_text)
	)
	interr("Bad search limits in initglobal");
#endif

    Buffirst = bufbeg + first;		/* start of search area */
    Buflast = bufbeg + TBsize(Q_text);	/* point past end of search area */
    Csptr = bufbeg + start;		/* starting search position */

    if ( last < 0 )			/* make sure match would fit */
    {
	Bufbound = Buffirst;		/* it won't; set up dummy position
					** for sanitary reasons
					*/
	return(FALSE);			/* say match fails */
    }

    Bufbound = bufbeg + last;		/* set up last possible starting
					** place for match
					*/
    return(TRUE);
}
/* matchonce -- attempt to match search string against text buffer
**
** This routine attempts to match the search string against the text
** buffer, starting at the current buffer search position.
** It is driven by the search execution list which is built by
** buildsearch.  Words in this list are search pattern directives,
** sometimes followed by arguments.
** The most typical directive matches a literal string
** against the text buffer.
*/

static BOOL				/* TRUE if success */
matchonce()
{
    extern void interr();

    char * start = Csptr;		/* remember current search buffer
					** pointer, since lower level
					** routines change it
					*/
    int func;				/* current directive function */
    unsigned char c;			/* temp. char for general use */
    register BOOL matchflag;		/* current directive result */
    register BOOL wantflag;		/* current desired directive result:
					** normally TRUE, but may be inverted
					*/
    BOOL retval = FALSE;		/* presumed return value */

    wantflag = TRUE;			/* initially want a match */

/* Loop until we get to the end of the search execution list or fail */

    do
    {
	if ( (func = *Cspc++) == M_last )
	{
	    Smatch = Csptr - start;	/* reached end.  Set match length */
	    retval = TRUE;		/* announce success */
	    break;
	}

	if (func == M_inv)		/* if inverting sense... */
	{
	    wantflag = FALSE;		/* change what we want */
	    continue;			/* get another function */
	}

	if (Csptr >= Buflast)		/* must be at least 1 char left */
	    break;			/* there weren't any */
	
	c = *Csptr++;			/* grab a char for those that need it */

	matchflag = FALSE;		/* assume failure */

/* Now we do a giant switch wherein we choose the match directive
** and perform it, leaving the result in "matchflag".
*/

	switch (func)
	{
	case M_1char:			/* match single character */
	    matchflag = (c == (unsigned char) *Cspc++);
	    break;
	
	case M_alnu:			/* match alphanumeric */
	    matchflag = (ISLET(c) || ISDIG(c));
	    break;
	
	case M_any:			/* match any */
	    matchflag = TRUE;		/* always succeed */
	    break;
	
	case M_dig:			/* match digit */
	    matchflag = ISDIG(c);
	    break;
	
	case M_lc:			/* match lower case letter */
	    matchflag = (ISLC(c) && ISLET(c));
	    break;
	
	case M_let:			/* match any letter */
	    matchflag = ISLET(c);
	    break;
	
	case M_lt:			/* match line terminator */
	    matchflag = ISLT(c);
	    break;
	
	case M_oneof:			/* match anything in a Q-register */
	    {
		int tb = *Cspc++;	/* get text block number */

		matchflag =
		    (memchr(TBtext(tb),(char) c,TBsize(tb)) != NULL);
	    }
	    break;
	
	case M_sep:			/* match a separator */
	    matchflag = ! (ISLET(c) || ISDIG(c)); /* (TECO's definition) */
	    break;

	case M_sym:			/* match symbol constituent */
	    matchflag = ISSYM(c);
	    break;
	
	case M_uc:			/* upper case letter */
	    matchflag = (ISUC(c) && ISLET(c));
	    break;

	case M_bspace:			/* match spaces/TABs backward */

	/* The first character must be a space/TAB.  If it is, we push
	** "start" to the left-most space/TAB and match following
	** spaces/TABs.  Remember that initial character has been pre-fetched.
	*/

	    {
		char * cp = Csptr - 1;	/* remember where c came from */

		if ( c != SPACE && c != TAB )
		    break;		/* break with a failure */
		
		while (    cp-- > Buffirst	/* stay within bounds */
			&& ((c = *cp) == SPACE || c == TAB)
		      )
		      ;				/* null loop */
		
		start = cp + 1;		/* reset start position for match
					** (went one too far back
					*/
	    }
	    /* fall through to match remaining characters >= Csptr */

	case M_space:			/* match spaces/TABs */
	    {
		char * first = --Csptr;	/* initial pointer; back up Csptr to
					** simplify logic
					*/
		
		while (    Csptr <= Buflast 
			&& ( (c = *Csptr++) == SPACE ||  c == TAB )
			)
		    ;			/* null loop */
		
		matchflag = (--Csptr != first);
					/* back up over non-space/TAB */
	    }
	    break;
/* Now for the two hard cases:  matching strings */

	case M_literal:			/* literal string */
	    {
		int start = *Cspc++;	/* start position */
		int len = *Cspc++;	/* length of string */

		Csptr--;		/* disregard pre-fetched char */

		if (    Csptr + len <= Buflast
					/* string would fit... */
		    &&  memcmp(TBtext(Q_search)+start, Csptr, len) == 0
		    )
		{
		    Csptr += len;	/* skip over matched string */
		    matchflag = TRUE;
		}
	    }
	    break;
	case M_nocase:			/* match string without regard for 
					** case
					*/
	    {
		int start = *Cspc++;	/* start position in Q_search */
		int len = *Cspc++;	/* length of string */
		char * cp1;		/* pointers to string to match */
		unsigned char c1, c2;	/* chars from both strings */

		--Csptr;		/* back up over char. prefetch */

		if (Csptr + len <= Buflast) /* must fit in buffer */
		{
		    cp1 = TBtext(Q_search) + start;

		    while (len-- > 0)
		    {
			c1 = *cp1++;	/* get chars, force to upper case */
			c2 = *Csptr++;
			if (UPPER(c1) != UPPER(c2))
			    goto failnocase;
		    }
		    matchflag = TRUE;
		    /* Csptr points beyond string */
		}
	    failnocase: ;
	    }
	    break;
	

	default:			/* no directive matched */
	    interr("no directive matched in matchonce");
	} /* end monster switch */

	if (matchflag != wantflag)	/* fail if we didn't get desired
					** result
					*/
	    break;

	wantflag = TRUE;		/* reset what we want */
    } while (TRUE);
    
    Csptr = start;			/* restore original start position */
    return(retval);			/* return chosen value */
}
/* matchfast -- do quick pre-check for search failure
**
** This routine does a quick pre-check for possible search failure.
** The basic idea is, when possible, to find the first character
** of a matching string by a quick buffer search before embarking
** on the full match code.
**
** We can only do this test for a few of the search commands.
** The routine returns FALSE if the match couldn't possible succeed.
** It returns TRUE if we don't know whether the match will succeed,
** or if we succeed in finding the first potential match character.
** In the latter case, Csptr is advanced to that matching character.
**
** Csptr and Cspc must be prepared appropriately.
*/

static BOOL
matchfast()
{
    int len = Buflast - Csptr;		/* remaining chars in buffer */
    int arg1 = Cspc[1];			/* first arg. of first search command */
    char * s;				/* gets future new value of Csptr */
    char * s2;				/* another pointer */
    char c;				/* temporary match char */

    switch (Cspc[0])			/* dispatch on first search command */
    {
    case M_1char:			/* want to match one character */
	if ((s = memchr(Csptr,(char) arg1,len)) == NULL)
	    return(FALSE);		/* didn't find the char in buffer */
	break;				/* s contains new Csptr */

    case M_literal:			/* match literal string */
	if ((s = memchr(Csptr,*(TBtext(Q_search)+arg1),len)) == NULL)
					/* try to find first char of literal
					** string
					*/
	    return(FALSE);		/* failed if not found */
	break;				/* s contains new value for Csptr */
    
    case M_nocase:			/* uncased string match */

/* For this case, we look for both the lower and upper case characters.
** We choose the lower non-NULL pointer as the new starting point.
*/

	c = LOWER(*(TBtext(Q_search)+arg1)); /* get first char of string */
	s = memchr(Csptr,c,len);	/* get pointer for lower case */
	if (s != NULL)
	    len = s - Csptr;		/* try to find earlier upper case */
	s2 = memchr(Csptr,(char) UPPER(c),len); /* check upper case */

/* Now handle the four cases for s, s2 (NULL/non-NULL) */

	if (s == NULL)
	{
	    if ((s = s2) == NULL)	/* both NULL */
		return(FALSE);		/* not found in either case:  fail */
	}
	else if (s2 != NULL && s2 < s)	/* choose lower value */
	    s = s2;
	break;				/* s set to new Csptr value */
    
    default:				/* everything else.... */
	return(TRUE);			/* assume the search should continue */
    }

    Csptr = s;				/* set new value to start search at */
    return(TRUE);			/* continue search */
}
/* matchfwd -- match forward in buffer
**
** This routine searches within the given bounds in the forward
** direction from the designated start position.  Siter must have
** been set already.
*/

static BOOL
matchfwd(start,first,last)
int start;				/* starting search position */
int first;				/* lowest position bound */
int last;				/* highest position bound (or < 0
					** to go to end of buffer)
					*/
{
    if (!initglobal(start,first,last))	/* initialize search globals */
	return(FALSE);			/* couldn't possibly succeed
					** unless matching null string
					*/

    if (Sminlen == 0)			/* check pathological case of
					** matching null string
					*/
	return(TRUE);

    while (Siter > 0)
    {
	Cspc = searchinst;		/* point at search instructions */

	if (! matchfast())		/* try quick match check */
	    return(FALSE);		/* failed:  search fails */

	if (Csptr > Bufbound)		/* make sure match is possible */
	    return(FALSE);		/* can't match */
	
	if (matchonce())		/* try to match at current position */
	{
	    Siter--;			/* count off another match */
	    /* test for ED bit that allows only single char step on match */
	    if ((ED & ED_sstep) != 0)
		Csptr++;		/* just bump by one position */
	    else
		Csptr += Smatch;	/* otherwise,
					** skip over matched string
					*/
	}
	else
	    Csptr++;			/* move one char right and try again */
    }
    return(TRUE);			/* matched required iterations */
}
/* matchfbuf -- match one buffer in forward direction
**
** This routine matches a search string in the forward direction.  It stays
** within the current text buffer.  Siter must be set by caller to number
** of remaining match iterations required.
*/

static BOOL				/* TRUE if successful match */
matchfbuf(start)
int start;				/* position to start search at */
{

    return( matchfwd(start,0,-1) );	/* search within the buffer bounds */
}
/* matchbkwd -- match backward in the text buffer
**
** This routine matches a search string backward within the current
** text buffer from the designated starting point.  The caller
** provides the search bounds within the buffer.  Siter must have been
** initialized.
*/

static BOOL
matchbkwd(start,first,last)
int start;				/* position from which to start */
int first;				/* lowest-numbered position bound */
int last;				/* highest- numbered position bound,
					** or < 0 to use buffer size
					*/
{
    if (!initglobal(start,first,last))	/* initialize search globals */
	return(FALSE);			/* would never succeed unless
					** matching null string
					*/

    if (Sminlen == 0)			/* pathological case:  null string */
	return(TRUE);

    while (Siter > 0)			/* continue until all matches done */
    {
	if (Csptr < Buffirst)		/* quit if beyond start of buffer */
	    return(FALSE);		/* failed */
	
	Cspc = searchinst;		/* point at command string array */

	/* Make special check for M_space as first directive and change to
	** M_bspace, since this is a backward search.
	*/

	if (*Cspc == M_space)
	    *Cspc = M_bspace;

	if (matchonce())		/* on successful match... */
	    Siter--;			/* mark off successful match */
	Csptr--;			/* on success or failure, back up
					** buffer search pointer
					*/
    }
/* correct current pointer to just beyond matched string */

    Csptr += Smatch + 1;		/* +1 to compensate for -1 above */
    return(TRUE);			/* success */
}
/* matchbbuf -- match buffer backwards
**
** This routine matches a search string against the text buffer by going
** backward in the buffer.  Siter must be set with number of required
** matches before calling.
*/

static BOOL				/* TRUE if successful match */
matchbbuf(start)
int start;				/* position to start at */
{
    return( matchbkwd(start,0,-1) );	/* search entire buffer */
}
/* matchbound -- bounded search in buffer
**
** This routine performs a bounded search in the current text buffer.  As
** with the other routines, Siter must be set with the number of matches
** to be found.  The search is confined to begin between the first and last
** buffer positions passed as arguments.  If 'first' > 'last', the search
** proceeds backward.
*/

static BOOL				/* TRUE if Siter matches found */
matchbound(first,last)
int first;				/* first position in buffer */
int last;				/* last position in buffer */
{
    if (first <= last)			/* check whether forward or backward */
	return( matchfwd(first,first,last) );
					/* keep original bound order */
    else
	return( matchbkwd(first,last,first) );
					/* go backward, reverse bound order */
}
/* matchxbuf -- cross-buffer matching
**
** This routine is the nucleus of the search commands that cross
** buffer boundaries:
**
**	N _ E_
**
** It also handles the corresponding search/replace commands.
*/

static short
matchxbuf(write,ytest,replace)
BOOL write;				/* TRUE to write (P) buffer,
					** FALSE for read only (Y)
					*/
BOOL ytest;				/* TRUE to do Yank protection */
BOOL replace;				/* TRUE if called for searc/replace,
					** FALSE for search-only
					*/
{
    TNUMB n;				/* temporary iteration count */
    short matchexit();

    if (srchinit(replace))		/* initialize for search */
	return(SKIPCONT);		/* we're skipping.  Continue */
    
    Set1dfl(1);				/* set default iteration count of 1 */
    (void) Get1val(&n);			/* get the iteration count */
    if ((Siter = n) <= 0)		/* must be positive */
	terrNUL(&Err_ISA);
    
/* We start the search at '.'.  After each buffer reload, '.' will be 0. 
** matchfbuf will return TRUE when the iteration count is exhausted.
*/

    while (! matchfbuf(Dot))		/* match, going forward */
    {
	if ( (write ? Dopcmd(FALSE) : Doyank(ytest)) == 0)
					/* these return 0 on reaching EOF */
	    return(matchexit(FALSE,TRUE)); /* okay to set '.' to 0 */
	/* Dot is now 0 on successful P or Y */
    }
    return(matchexit(TRUE,TRUE));	/* found required number of matches,
					** okay to move '.'
					*/
}
/* matchexit -- handle details of exiting a search command
**
** This routine handles the myriad of messy details related to completing
** a search.  These details include:
**
**	1)  Modifying '.' as defined by the ED mode bits and argument.
**	2)  Setting a search value if : flag set or in iteration and followed
**		by a ; or :; .
**	3)  Performing a replace if successful search and replace command.
**	4)  Reporting a warning if the search failed within an iteration.
**	5)  Reporting an error if search failed outside iteration and no :
**		flag was set.
**	6)  Displaying lines surrounding matching string, according to ES.
**	7) Returning the XECCONT result to the caller otherwise.
*/

static short				/* XECCONT if we ever return at all */
matchexit(success,movedot)
BOOL success;				/* TRUE if search succeeded */
BOOL movedot;				/* TRUE if ok to move '.' on failure */
{
    BOOL valpeek();
    void Dovflag();

    Eat_val();				/* discard values unconditionally */

    if (success)			/* handling a search success? */
    {
/* Temporarily set '.' to start of matched string.  We will set the correct
** value later.
*/
	Dot = Csptr - TBtext(Q_text) - Smatch;
	if (valpeek())			/* if : or in iter. and {:}; */
	    Set1val((TNUMB) -1);	/* report success on search */
	/* valpeek discards flags */

	Inslen = Smatch;		/* length of match for ^S */
	if (Replflag)			/* handle replace operation */
	{
	    if (! TBrepl(Q_text,Dot,Smatch, /* replace matched string with... */
				TBtext(Cmdtb)+Replfirst,Repllen))
					   /* replacement string */
		terrNUL(&Err_MEM);	/* ran out of memory */
	    Inslen = Repllen;		/* set length of replacement for ^S */
	}
	Dot = Dot + Inslen;		/* now correct '.' */
/* output matched region if according to ES flag, if not in macro or
** iteration.
*/
	if (Xectype == XECINPUT)	/* only do this at top level */
	    Dovflag(ES);		/* pass ES flag */

	return(XECCONT);		/* return continue-code */
    }
/* reach here on search failure */

    if (movedot && (ED & ED_predot) == 0) /* reset '.' to 0 if bit not set */
	 Dot = 0;
    
    if (valpeek())			/* if : or in iter. and {:}; */
	Set1val((TNUMB) 0);		/* report failure */
    else if (Xectype == XECITER)	/* if in iteration, print warning */
	terrWRN(&Wrn_SFL);
    else
	terrTB(&Err_SRH,Q_search);	/* search failed */
    
    /* valpeek discards flags */
    return(XECCONT);
}
/* valpeek -- peek ahead if necessary to determine whether to set value
**
** This routine handles the messy details of deciding whether to report
** a value from a search routine.  According to the TECO manual, a search
** in an iteration can be followed by ; or :;, even if it does not have a
** preceding ':' to force it to return a value.  The easiest way to implement
** the desired function is to return a value.  (In fact, it's the only way,
** since ; would complain if there was no preceding value.)
*/

static BOOL				/* TRUE if value should be returned */
valpeek()
{
    
    if (Fl_colon != 0)			/* always return value if : flag set */
    {
	Eat_flags();			/* discard flags */
	return(TRUE);
    }

    Eat_flags();			/* discard flags for other cases now */

    if (Xectype != XECITER)		/* if not in iteration, no value */
	return(FALSE);
    
/* peek ahead in the command stream for an optional : followed by ; */

    if (pCMch() == ':')
    {
	Fl_colon++;			/* saw :.  Set : flag accordingly */
	(void) gCMch();			/* eat the character.  Note that if
					** the next thing is not ;, we still
					** have retained : appropriately
					*/
    }

    return(pCMch() == ';');		/* TRUE if next is ;, else FALSE */
}
/* CMscmd -- S command */

short
CMscmd()
{
    short scmdsub();

    return(scmdsub(FALSE));		/* do bulk of S command, no replace */
}

/* CMfscmd -- FS command */

short
CMfscmd()
{
    short scmdsub();

    return(scmdsub(TRUE));		/* do S command with replace */
}
/* scmdsub -- do bulk of FS, S command
**
** At last!  The guts of a real command!
**
**	(F)S has these variants:
**
**	::S	(0 arguments, >1 :)	"match in place"
**	{:}S	(0 arguments, <= 1 :)	search
**	n{:}S	(1 argument)		search
**	m,n{:}S	(2 arguments)		bounded search
**
** We must catch the ::S case as a special case, since it is irregular.
*/

static short
scmdsub(flag)
BOOL flag;				/* TRUE if FS, FALSE if S */
{
    int m;				/* first search arg */
    int n;				/* second search arg */

    if (srchinit(flag))			/* make sure we should proceed */
	return(SKIPCONT);		/* no:  skipping */
    
/* handle ::S case */

    if (!Get1val(&n) && Fl_colon > 1)
    {
	Siter = 1;			/* just one iteration */
	return(matchexit(matchbound(Dot,Dot),FALSE));
					/* search once at current buffer pos
					** don't move '.'
					*/
    }

    Set1dfl(1);				/* set default value of 1 for current
					** arg
					*/
/* m,nS:  bounded search.  ABS(m) gives number of positions that may be
** moved.  If n > 0, the search is forward.  If n < 0, the search is backward.
** n = 0 is an error.  If m = 0, the buffer pointer doesn't move on failure,
** but the search goes from '.' to the appropriate buffer boundary.
*/

    if (Get2val(&m,&n))
    {
	int first = Dot;		/* assume non-moving search */
	int last = Dot;			/* positions that demarcate search */

	if (n == 0)
	    terrNUL(&Err_ISA);		/* n must be non-0 */
	
	if (m < 0)			/* ABS(m) */
	    m = -m;
	
	if (n > 0)			/* forward search */
	{
	    int temp = TBsize(Q_text);	/* size of text buffer */
	    if (m == 0)			/* search to end of buffer */
		last = temp;
	    else
	    {
		last += m-1;		/* otherwise, move m-1 positions max. */
		if (last > temp)	/* confine within buffer */
		    last = temp;
	    /* first remains '.' */
	    }
	}
	else				/* backward search */
	{
	    n = -n;			/* make number of iterations + */
	    if (m == 0)			/* search to start of buffer */
		last = 0;
	    else
	    {
		last -= m-1;		/* move at most m-1 positions */
		if (last < 0)		/* confine to buffer */
		    last = 0;
	    /* first remains '.' */
	    }
	}
	Siter = n;			/* set number of iterations */
	return(matchexit(matchbound(first,last), FALSE));
					/* preserve '.' */
    }
/* routine case:  single argument */

    (void) Get1val(&n);			/* iteration count */

    if (n == 0)				/* 0 iteration count forbidden */
	terrNUL(&Err_ISA);		/* illegal search arg */
    
    if ((Siter = n) > 0)		/* positive search */
	return(matchexit(matchfbuf(Dot),TRUE));
					/* move '.' on failure */

/* remaining case is negative search */

    Siter = -Siter;			/* abs. value of iteration count */
    return(matchexit(matchbbuf(Dot),TRUE));
					/* move '.' on failure */
}
/* N _ E_ cross-buffer searches,
** FN F_ cross-buffer search/replace.
*/

short
CMncmd()				/* N command */
{
    return(matchxbuf(	TRUE,		/* write buffer before proceeding */
			TRUE,		/* do Y protection */
			FALSE));	/* not a search/replace */
}


short
CMfncmd()				/* FN command */
{
    return(matchxbuf(	TRUE,		/* write buffer before proceeding */
			TRUE,		/* do Y protection */
			TRUE));		/* do search/replace */
}


short
CMundcmd()				/* _ command */
{
    return(matchxbuf(	FALSE,		/* don't write buffer */
			TRUE,		/* do Y protection */
			FALSE));	/* not a search/replace */
}


short
CMeundcmd()				/* E_ command */
{
    return(matchxbuf(	FALSE,		/* don't write buffer */
			FALSE,		/* bypass Y protection */
			FALSE));	/* not a search/replace */
}


short
CMfundcmd()				/* F_ command */
{
    return(matchxbuf(	FALSE,		/* don't write buffer */
			TRUE,		/* do Y protection */
			TRUE));		/* do search/replace */
}
/* FB, FC bounded search commands
**
** Both of these command use a helper routine, below.
*/

short
CMfbcmd()
{
    short fbsub();

    return( fbsub(FALSE) );		/* do bulk of FB without replace */
}


short
CMfccmd()
{
    short fbsub();

    return(  fbsub(TRUE) );		/* do bulk of FB with replace */
}
/* fbsub -- do guts of FB, FC commands
**
** This routine does all of the work of the FB and FC commands,
** which are bounded searches.
*/

static short
fbsub(flag)
BOOL flag;				/* TRUE if replace (FC) */
{
    TNUMB m,n;				/* arg. values to command */
    int max = TBsize(Q_text);		/* maximum position in buffer */
    int cmdstart = Cmddot - 2;		/* position of start of fb/fc */


    if (srchinit(flag))			/* initialize, check whether to proceed
					*/
	return(SKIPCONT);		/* skipping -- just continue same */
    
    Set1dfl((TNUMB) 1);			/* set default value */

    if (! Get2val(&m,&n) )		/* try to get 2 values */
    {
	(void) Get1val(&m);		/* if failed, get the lone value */
	n = Findlt(Q_text,Dot,m);	/* get position of m lines from here */
	if (m > 0 && n != Dot)
	    n--;			/* if going + and we moved, back up
					** over line terminator so it is
					** last position searched
					*/
	m = Dot;			/* limits are now m,n */
    }

    /* confine search limits to buffer */

    if (    m < 0 || m > max
	||  n < 0 || n > max
	)
	terrSTR(&Err_POP, TBtext(Cmdtb) + cmdstart, 2);
					/* if out of bounds, issue POP error
					** (like TECO-11); capture command
					** name as typed
					*/
    
    Siter = 1;				/* just one iteration */

    return(matchexit(matchbound(m,n),FALSE));
					/* don't allow '.' to change */
}
