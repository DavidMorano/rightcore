static char SCCSID[] = "@(#) cmfcmd.c:  4.1 12/12/82";
/* cmfcmd.c
**
**	TECO F? commands
**
**	David Kristol, April, 1982
**
** This module simply dispatches to a myriad of other routines in various
** places.
*/

#include "bool.h"
#include "errors.h"
#include "errmsg.h"
#include "xec.h"
/* define external routines */

extern short
	CMfbcmd(), CMfccmd(), CMfncmd(), CMfrcmd(),
	CMfscmd(), CMfundcmd(), Fctrl();


short
CMfcmd()
{
    short c = gCMch();			/* char following F */

    if (c < 0)				/* make sure there is one */
	Unterm();			/* else, unterminated command */
    
    switch (c)				/* dispatch on type */
    {
    case 'b':				/* FB command */
    case 'B':
	return(CMfbcmd());
    
    case 'c':				/* FC command */
    case 'C':
	return(CMfccmd());

    case 'n':				/* FN */
    case 'N':
	return(CMfncmd());

    case 'r':				/* FR */
    case 'R':
	return(CMfrcmd());

    case 's':				/* FS */
    case 'S':
	return(CMfscmd());

    case '_':				/* F_ */
	return(CMfundcmd());

/* The various F control commands are handled as a batch: */

    case '<':				/* F< */
    case '>':				/* F> */
    case '|':				/* F| */
    case '\'':				/* F' */
	return(Fctrl( (int) c ));

    default:
	terrCHR(&Err_IFC,c);		/* didn't find character */
    }
/*NOTREACHED*/
}
