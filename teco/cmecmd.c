static char SCCSID[] = "@(#) cmecmd.c:  4.1 12/12/82";
/* cmecmd.c
 *
 *	TECO E command dispatcher
 *
 *	David Kristol, March, 1982
 *
 * This module houses the dispatch code for the E commands.  Since they
 * are so diverse, the actual processing code is dispersed here and there.
 */

#include <stdio.h>
#include "bool.h"
#include "errors.h"
#include "errmsg.h"
#include "exittype.h"
#include "mdconfig.h"
#include "skiptype.h"
#include "tty.h"
#include "values.h"
#include "xec.h"
#ifdef DEBUG
#include "qname.h"
#endif



/* define external routines */

extern short
	CMeacmd(), CMebcmd(), CMeccmd(), CMedcmd(), CMeecmd(),
	CMefcmd(), CMegcmd(),
	CMehcmd(), CMeicmd(), CMejcmd(), CMekcmd(), CMencmd(),
	CMepcmd(), CMercmd(),
	CMescmd(), CMetcmd(), CMeucmd(), CMevcmd(),
	CMewcmd(), CMexcmd(), CMeycmd(),
	CMeundcmd();

/* dispatch on next character */

short
CMecmd()
{
    short c = gCMch();			/* char following E */
#ifdef DEBUG
    void memreport();
#endif

    if (c < 0)				/* make sure there is one */
	Unterm();			/* flag as error if not */
    
    switch (c)				/* dispatch on type */
    {
    case 'a':				/* EA */
    case 'A':
	return(CMeacmd());

    case 'b':				/* EB */
    case 'B':
	return(CMebcmd());

    case 'c':				/* EC */
    case 'C':
	return(CMeccmd());

    case 'd':				/* ED */
    case 'D':
	return(CMedcmd());

#ifdef ENVVAR				/* if env. variable support */
    case 'e':				/* EE */
    case 'E':
	return(CMeecmd());
#endif

    case 'f':
    case 'F':
	return(CMefcmd());

    case 'g':				/* EG */
    case 'G':
	return(CMegcmd());

    case 'h':				/* EH */
    case 'H':
	return(CMehcmd());

    case 'i':				/* EI */
    case 'I':
	return(CMeicmd());

    case 'j':				/* EJ */
    case 'J':
	return(CMejcmd());

    case 'k':				/* EK */
    case 'K':
	return(CMekcmd());

    case 'n':				/* EN */
    case 'N':
	return(CMencmd());

    case 'o':				/* EO */
    case 'O':
	if (Skiptype == SKNONE)		/* if not skipping, set current
					** version
					*/
	    Set1val((TNUMB) TECVERSION);
	return(CONTINUE);

    case 'p':				/* EP */
    case 'P':
	return(CMepcmd());

    case 'r':				/* ER */
    case 'R':
	return(CMercmd());

    case 's':				/* ES */
    case 'S':
	return(CMescmd());


    case 't':				/* ET */
    case 'T':
	return(CMetcmd());

    case 'u':				/* EU */
    case 'U':
	return(CMeucmd());

    case 'v':				/* EV */
    case 'V':
	return(CMevcmd());

    case 'w':				/* EW */
    case 'W':
	return(CMewcmd());

    case 'x':				/* EX */
    case 'X':
	return(CMexcmd());

    case 'y':				/* EY */
    case 'Y':
	return(CMeycmd());
    case '_':				/* E_ */
	return(CMeundcmd());

#ifdef DEBUG
    case 'm':				/* EM for debugging */
    case 'M':
	memreport();
	return(CONTINUE);
#endif

    default:				/* flag as error */
	terrCHR(&Err_IEC,(char) c);
	/* no return */
    }
/*NOTREACHED*/
}
/* memreport -- report on content of memory
**
** memreport tries to account for all text memory by looking for it in
** text blocks and free space.  It produces a report of what it
** finds.
*/

#ifdef DEBUG


void
memreport()
{
    char * cur;				/* current pointer to text space */
    char * last;			/* pointer to end of text space */
    char * curstart;			/* beginning of current
					** block of stuff
					*/
    int len;				/* length of current block */
    int minlen;				/* smallest length if in both tb
					** and free space (an error
					** condition)
					*/
    short tb;			/* text block, if pointer is in one */
    void SMlimit();
    BOOL SMisfree();
    BOOL isTb();
    char * qname();
/* loop through all of memory, trying to identify who owns what, reporting
** on what we find as we go.
*/

    SMlimit(&cur,&last);		/* set current (first) pointer, end */
    fprintf(Ttyout,"limits are %d - %d (%d)\r\n",
		(int)cur,(int)last,last-cur);

    while (cur < last)
    {
	minlen = 0;			/* haven't seen free space or tb */

	if (SMisfree(cur,&curstart,&len)) /* a free block? */
	{
	    (void) fprintf(Ttyout,"%d-%d\t(%d)\tfree%s\r\n",
					/* print report line */
			(int) curstart, (int) curstart+len-1,
			len, (cur != curstart ? " overlap" : ""));
	    minlen = len;		/* record length of block */
	}
	if (isTB(cur,&curstart,&len,&tb)) /* check whether in text block */
	{
	    fprintf(Ttyout,"%d-%d\t(%d)\t%s%s\r\n",
		  (int) curstart, (int) curstart+len-1, len,
		  qname(tb), (cur!=curstart ? " overlap" : ""));

/* adjust length appropriately */

	    if (minlen == 0)		/* no previous length */
		minlen = len;		/* set to current length */
	    else if (len < minlen)	/* select smaller */
		minlen = len;
	}
	cur += minlen;			/* bump pointer to next block */
    }
    return;
}
/* qname -- return Q register name
**
** This routine returns the string Q-register name for a
** text block.
*/

static char qstring[] = {'Q', ' ', 'r', 'e', 'g', '.', ' ', 'x', '\0'};
#define QPOS (sizeof(qstring) - 1 -1)	/* position of x */
char *
qname(tb)
short tb;				/* text block number */
{
    switch(tb)
    {
    case Q_text:
	return("text");
    case Q_search:
	return("search");
    case Q_file:
	return("filename");
    case Q_cmd:
	return("cmd");
    case Q_stack:
	return("stack");
    case Q_goto:
	return("goto");
    case Q_btemp:
	return("btemp");
    case Q_entemp:
	return("entemp");
    case Q_syscmd:
	return("system cmd");
    case Q_env:
	return("env");

    case Q_A:  case Q_B:  case Q_C:  case Q_D:  case Q_E:
    case Q_F:  case Q_G:  case Q_H:  case Q_I:  case Q_J:
    case Q_K:  case Q_L:  case Q_M:  case Q_N:  case Q_O:
    case Q_P:  case Q_Q:  case Q_R:  case Q_S:  case Q_T:
    case Q_U:  case Q_V:  case Q_W:  case Q_X:  case Q_Y:  case Q_Z:
	qstring[QPOS] = 'A' + tb - Q_A;
	return(qstring);

    case Q_0:  case Q_1:  case Q_2:  case Q_3:  case Q_4:
    case Q_5:  case Q_6:  case Q_7:  case Q_8:  case Q_9:
	qstring[QPOS] = '0' + tb - Q_0;
	return(qstring);

    default:
	return("unknown");		/* unrecognized */
    }
/*NOTREACHED*/
}


#endif
