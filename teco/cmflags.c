static char SCCSID[] = "@(#) cmflags.c:  4.1 12/12/82";
/* cmflags.c
 *
 *	TECO flag command handler
 *
 *	David Kristol, February, 1982
 *
 *	This module handles these TECO flags:
 *
 *	: @ , ?
 */

#include "bool.h"
#include "exittype.h"
#include "skiptype.h"
#include "values.h"
#include "xec.h"
/* Handle : */

short
CMcolon()
{
    if (Skiptype == SKNONE)
	Fl_colon++;			/* just bump the global */
    return(CONTINUE);
}


/* Handle @ */

short
CMatsign()
{
    /* do this even when skipping */
    Fl_at = TRUE;			/* set global to signify we saw one */
    return(CONTINUE);
}


/* Handle , */

short
CMcomma()
{
    if (Skiptype == SKNONE)
        Nextval();			/* just call the appropriate routine */
    return(CONTINUE);
}


/* Handle ? */

short
CMquest()
{
    if (Skiptype == SKNONE)
	Fl_trace = ! Fl_trace;		/* toggle trace */
    return(CONTINUE);
}
