static char SCCSID[] = "@(#) cmspec.c:  4.1 12/12/82";
/* cmspec.c
**
**	TECO special flags commands
**
**	David Kristol, April, 1982
**
** This module contains the code to support these TECO special flag
** values:
**
**	^B ^D ^E ^F ^H ^N ^O ^Q ^R ^S ^Y ^^ ^Z
**
*/

#include <time.h>			/* system time related definitions */
#include "bool.h"
#include "errors.h"
#include "errmsg.h"
#include "exittype.h"
#include "qname.h"
#include "skiptype.h"
#include "tb.h"
#include "values.h"
#include "xec.h"
/* numeric value globals and declarations */

extern int Inslen;		/* length of last insert/search/etc. */
extern short Radix;		/* current radix:  8, 10, 16 */
extern TNUMB Fl_ctE;		/* form feed flag */
extern TNUMB Fl_ctN;		/* end of file flag */
/* ^D command */

short
CMctdcmd()
{
    if (Skiptype == SKNONE)		/* change radix if not skipping */
	Radix = 10;			/* set to 10 */
    return(CONTINUE);
}


/* ^E command */

short
CMctecmd()
{
    if (Skiptype == SKNONE)		/* return FF flag if not skipping */
	Set1val(Fl_ctE);
    return(CONTINUE);
}



/* ^F command */

short
CMctfcmd()
{
    if (Skiptype == SKNONE)		/* return 0 as switch reg. value */
	Set1val((TNUMB) 0);
    return(CONTINUE);
}

/* ^N command */

short
CMctncmd()
{
    if (Skiptype == SKNONE)		/* return EOF flag */
	Set1val(Fl_ctN);
    return(CONTINUE);
}
/* ^O command */

short
CMctocmd()
{
    if (Skiptype == SKNONE)		/* set octal radix if not skipping */
	Radix = 8;
    return(CONTINUE);
}


/* ^R command */

short
CMctrcmd()
{
    TNUMB t;				/* temporary numeric value */

    if (Skiptype != SKNONE)		/* exit if skipping */
	return(SKIPCONT);
    
    if (Get1val(&t))			/* if value provided, set radix */
    {
	if (t == 8 || t == 10 || t == 16)
	    Radix = t;			/* set radix if one of allowed values */
	else
	    terrNUL(&Err_IRA);		/* else, illegal value */
	
	Eat_val();			/* discard numeric values */
    }
    else				/* asking us to return value */
	Set1val((TNUMB) Radix);
    
    return(XECCONT);
}
/* ^S command */

short
CMctscmd()
{
    if (Skiptype == SKNONE)		/* if not skipping ... */
	Set1val((TNUMB) -Inslen);	/* set value of -(length of last
					** insert/search/etc.
					*/
    return(CONTINUE);
}


/* ^Y command */

short
CMctycmd()
{
    if (Skiptype == SKNONE)		/* if not skipping ... */
	Set2val((TNUMB) (Dot-Inslen),(TNUMB) Dot);
					/* set .+^S,. */
    return(CONTINUE);
}


/* ^^ command */

short
CMctupcmd()
{
    int c = gCMch();			/* always read next command character */
    void Unterm();

    if (Skiptype == SKNONE)		/* if not skipping... */
    {
	if (c < 0)			/* if there is no new char */
	    Unterm();			/* report unterminated command */
	Set1val((TNUMB) c);		/* set ASCII code as value */
    }
    return(CONTINUE);
}
/* ^Q command */

short
CMctqcmd()
{
    TNUMB n;				/* number of lines */

    if (Skiptype == SKNONE)		/* if not skipping... */
    {
	Set1dfl((TNUMB) 1);		/* default value is 1 */

	(void) Get1val(&n);		/* get number of lines */

	Set1val(Findlt(Q_text,Dot,n) - Dot);
					/* distance from . to 'n' lines away */
    }
    return(CONTINUE);
}
/* gettime -- get current time of day into time structure
**
** This routine gets the current date and time of day and converts
** it to a more useable form in a system-provided structure.
** The time is provided for the user's time zone.
*/

struct tm *
gettime()
{
    long time();			/* system call, returns time of day
					** in seconds since 00:00:00 GMT,
					** 1/1/1970
					*/


    long clock;				/* returned clock value */
    /* struct * tm localtime();		/* declared in time.h */

    (void) time(&clock);		/* get time of day */
    return( localtime(&clock) );	/* return converted version */
}
/* ^B command
**
** Set value corresponding to date.  We set a value in the RSX-11/VMS
** format:  ((year-1970) * 16 + month) * 32 + day_of_month
*/

short
CMctbcmd()
{
    struct tm * time;			/* time system structure */

    if (Skiptype == SKNONE)		/* if not skipping... */
    {
	time = gettime();		/* get current time-of-day */
	Set1val( (TNUMB)
	    ( ((time->tm_year * 16) + time->tm_mon) * 32) + time->tm_mday
		);
    }
    return(CONTINUE);
}
/* ^H command
**
** Set value corresponding to current time-of-day.  We set a value in
** the RSX-11/VMS format:
**	((((hour * 60) + minute) * 60) + second) / 2
** To avoid problems on 16-bit processors, the calculation we do is
** actually:
**	(((hour * 60) + minute) * 30) + second/2
** Note, also, that the time is returned in the user's current time
** zone, not GMT.
*/

short
CMcthcmd()
{
    struct tm * time;			/* system time structure */

    if (Skiptype == SKNONE)		/* if not skipping... */
    {
	time = gettime();		/* get time-of-day */
	Set1val( (TNUMB)
	    ( (((time->tm_hour * 60) + time->tm_min) * 30) + time->tm_sec/2 )
		);
    }
    return(CONTINUE);
}
/* ^Z command
**
** This command returns, as a value, the number of bytes in Q-registers
** and the command buffer.  We also include in this count the number of
** bytes in the search string and file string buffers.
*/

/* define array containing text block numbers of all text blocks whose
** size we want to sum.
*/

static int qregs[] =
{
    Q_A, Q_B, Q_C, Q_D, Q_E,
    Q_F, Q_G, Q_H, Q_I, Q_J,
    Q_K, Q_L, Q_M, Q_N, Q_O,
    Q_P, Q_Q, Q_R, Q_S, Q_T,
    Q_U, Q_V, Q_W, Q_X, Q_Y,
    Q_Z, Q_0, Q_1, Q_2, Q_3,
    Q_4, Q_5, Q_6, Q_7, Q_8,
    Q_9, Q_search, Q_file, Q_cmd
};

short
CMctzcmd()
{
    if (Skiptype == SKNONE)		/* if not skipping... */
    {
	TNUMB sum = 0;			/* current sum */
	int i;				/* index into table above */

	for (i = sizeof(qregs)/sizeof(int) - 1; i >= 0; i--)
	    sum += TBsize(qregs[i]);	/* add in size of current tb */
    
	Set1val(sum);			/* set this as a numeric value */
    }
    return(CONTINUE);
}
