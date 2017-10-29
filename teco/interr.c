static char SCCSID[] = "@(#) interr.c:  4.1 12/12/82";
/* interr.c
 *
 *	TECO internal error handler
 *
 *	David Kristol, February, 1982
 */


#define ERROUT	2			/* errors to standard error */

static int calls;			/* keep track of number of times
					** we're called (avoid recursion)
					*/

void
interr(cp)
char * cp;				/* pointer to character string */
{
    extern int strlen();
    extern int abort();
    extern int write();
    extern void Ttreset();

    if (calls++ == 0)			/* if first time... */
    {
	Ttreset();			/* reset terminal state */
	(void) write(ERROUT,cp,(unsigned) strlen(cp));
   					 /* panic-write error string */
	(void) write(ERROUT,"\n",1);	/* write new line */
    }
    (void) abort();			/* bail out */
}
