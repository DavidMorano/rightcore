static char SCCSID[] = "@(#) cmenv.c:  4.2 12/13/82";
/* cmenv.c
**
**	TECO environment-related commands
**
**	David Kristol, July, 1982
**
** This module contains these TECO environment-related commands:
**
**	EE EG EJ
*/

#include <stdio.h>			/* for tty.h */
#include <string.h>

#include "bool.h"
#include "chars.h"
#include "cmdio.h"
#include "cmutil.h"
#include "ctype.h"
#include "errors.h"
#include "errmsg.h"
#include "exittype.h"
#include "find.h"
#include "mdconfig.h"
#include "memory.h"
#include "qname.h"
#include "skiptype.h"
#include "tb.h"
#include "tty.h"
#include "values.h"
#include "xec.h"
/* EG command
**
** Our notion of the EG command is somewhat non-standard.  It has these
** features:
**
**	1.  EG may be entered in an @EG/.../ form.
**	2.  :EG returns the exit status for the executed command.
**	3.  We allow the search-string building operations.  This gives
**		us the potential to pick up the current filename with
**		^EQ*.
**	4.  If the EG string is null, we repeat the previous EG command.
**		We echo the previous EG string in that case.
**	5.  We don't exit when EG finishes.
**	6.  We delete all NUL characters from the EG string.
**
** We also allow multi-line commands to be passed, which means we must:
**
**	1.  Delete CR characters from the EG string when we first read it.
**		(The Shell gets upset by CR LF, requiring just LF, which
**		is New Line.
**	2.  Print LF as CR LF for EG$.
**	3.  Prohibit isolated LFs in commands (since the Shell treats them
**		as New Line.
*/

short
CMegcmd()
{
    int first;				/* initial position of EG string */
    int len;				/* length of same */
    TNUMB status = 0;			/* returned status of system call */
    int fl_colon = Fl_colon;		/* save flag which Ecsub can change */

    extern void Ecsub();

    Finddlm(ESC,&first,&len);		/* delimit EG string */

    if (Skiptype != SKNONE)		/* if skipping, continue doing so */
	return(SKIPCONT);
    
    Ecsub(FALSE);			/* pass through current file, if any */
/* What we do depends on whether the user's string is null */

    if (len > 0)			/* if there's a new string... */
    {
	char * found;			/* pointer to character found */

	Buildstring(Q_syscmd,first,len,FALSE);
					/* build string into Q_syscmd,
					** translate quoted chars
					*/

	/* eliminate any embedded NULs */

	while ((found = memchr(TBtext(Q_syscmd),NUL,TBsize(Q_syscmd))) != NULL)
	    TBdel(Q_syscmd,found-TBtext(Q_syscmd),1);

	Addc(Q_syscmd,NUL);		/* append NUL for system call */

	/* eliminate CRs (change CR/LF to New Line) */

	while ((found = strchr(TBtext(Q_syscmd),CR)) != NULL)
	    TBdel(Q_syscmd,found-TBtext(Q_syscmd),1);
    }
    else		/* len == 0 case */
    {
	/* echo EG string */

	wCch('[',Cmdout);		/* bracket with [ ] */

	if (TBtext(Q_syscmd) != 0)	/* if non-empty string */
	{
	    char * firstchar = TBtext(Q_syscmd);
					/* start of string */
	    char * curchar = firstchar;	/* current printing position */
	    char * newchar;		/* end of current phrase (NL) */

	    while ((newchar = strchr(curchar,LF)) != NULL)
	    {
		wTBst(Q_syscmd,curchar-firstchar,newchar-curchar,Cmdout);
					/* write portion up to New Line */
		Crlf(Cmdout);		/* output CR/LF */
		curchar = newchar + 1;	/* skip over New Line */
	    }

	    /* write last phrase to end of string */

	    wTBst(Q_syscmd,
		  curchar - firstchar,
		  firstchar + TBsize(Q_syscmd) - curchar - 1,
					/* portion from current place to end,
					** excluding trailing NUL
					*/
		  Cmdout);
	}
	wCch(']',Cmdout);
	Crlf(Cmdout);			/* follow with cr/lf */
    }
/* now do the actual system call */

    if (TBsize(Q_syscmd) != 0)		/* only when there's something to do */
    {
	Ttsave();			/* save current terminal modes */
	Ttreset();			/* reset to original modes */
	status = system(TBtext(Q_syscmd));
	Ttrestore();			/* restore previous terminal modes */
    }

    if (fl_colon != 0)			/* return status if :EG */
	Set1val(status);
    
    Eat_flags();			/* discard flags */
    return(XECCONT);			/* continue execution */
}
/* EJ command
**
** EJ returns various environment parameters.  We make the following
** assumptions:
**
**	1.  If no preceding value is specified, 0EJ is assumed.
**	2.  If no valid EJ value is specified (e.g., 10EJ), we
**		behave like TECO-11:  for values < -1, choose -1;
**		for values > 2, choose 2.
**	3.  The values returned for UNIX systems are:
**
**		-1EJ	operating system = 10
**		 0EJ	process ID of TECO
**		 1EJ	N of "ttyN", if the terminal is so-named,
**			otherwise -1
**		 2EJ	real user ID number
*/

/* Pick operating system and processor values: */

#ifndef PROC				/* processor value */
#ifdef pdp11
#	define	PROC	0		/* PDP-11 processor value */
#endif	/* def pdp11 */
#ifdef vax
#	define	PROC	4		/* VAX native mode */
#endif	/* def vax */
#ifdef u3b				/* UNIX on 3B20S */
#	define	PROC	10		/* arbitrary value for 3B20S */
#endif	/* def u3b */
#ifdef SUN
#	define	PROC	11		/* arbitrary value for Sun */
#endif	/* def sun */
#endif	/* ndef PROC */

#ifndef OS				/* operating system value */
#if defined (unix)			/* UNIX system */
#	define	OS	10		/* arbitrary value for UNIX */
#endif	/* defines */
#endif	/* ndef OS */


/* EJ command proper */

short
CMejcmd()
{
    extern int getpid();
    extern int getuid();
    extern int atoi();
    extern int strncmp();

    TNUMB value;			/* incoming and outgoing value */

    if (Skiptype != SKNONE)		/* continue skipping if now doing so */
	return(SKIPCONT);
    
    Set1dfl((TNUMB) 0);			/* set default EJ value */

    (void) Get1val(&value);		/* now fetch argument value */

    Eat_flags();			/* discard current flags and values */
    Eat_val();

    if (value < -1)			/* force good value */
	value = -1;
    else if (value > 2)
	value = 2;


    switch (value)			/* select value to return */
    {
    case -1:				/* processor*256 + operating system */
	value = PROC * 256 + OS;
	break;
    
    case 0:				/* job number */
	value = getpid();
	break;
    
    case 1:				/* terminal number */
	value = Ttynum();		/* get user's terminal number */
	break;
    
    case 2:				/* user ID */
	value = getuid();
	break;
    }

    Set1val(value);			/* set new value */
    return(XECCONT);			/* continue execution */
}


/* EE command
**
** This (non-standard) command looks up an UNIX environment
** variable and makes sets its value in Q-register '$' (also
** non-standard).  For the :ee command, test whether the
** variable exists and set a value.
*/

#ifdef ENVVAR				/* if environment var. support */

short
CMeecmd()
{
    int first;				/* position of start of env. var. */
    int len;				/* length of env. var. name */
    char * varval;			/* pointer to variable value */
    extern char * getenv();


    Finddlm(ESC,&first,&len);		/* delimit variable name */

    if (Skiptype != SKNONE)		/* if skipping */
	return(SKIPCONT);		/* continue to do so now */
    
    TBkill(Q_env);			/* discard any current value */
    Addst(Q_env, TBtext(Cmdtb)+first, len);
					/* use Q_env to hold variable
					** name for now
					*/
    Addc(Q_env, NUL);			/* terminate with NUL for getenv */
    varval = getenv(TBtext(Q_env));	/* find variable */
/* Have variable value in "varval" */

    TBkill(Q_env);			/* discard Q_env content again */

    if (Fl_colon == 0)			/* returning a value? */
    {
	if (varval == NUL)		/* no.  check for NULL value */
	    terrSTR(&Err_VNF, TBtext(Cmdtb)+first, len);
					/* NULL:  produce error */
    }
    else				/* set return value */
	Set1val(varval == NULL ? (TNUMB) 0 : (TNUMB) -1);
    

/* Set NULL value if pointed at value is null. */

    if (*varval == NUL)
	varval = NULL;

/* Q_env will be NULL if there is no value or if the pointed-at value is
** NULL.
*/

    TBstatic(Q_env, varval, strlen(varval));
					/* set static value */

    Eat_flags();			/* discard flags */
    return(XECCONT);			/* continue execution */
}

#endif	/* def ENVVAR */
