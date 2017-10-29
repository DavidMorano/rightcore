static char SCCSID[] = "@(#) wild.c:  4.1 12/12/82";
/* wild.c
**
**	TECO Wildcard file specification handler
**
**	David Kristol, July, 1982
**
** This module encapsulates the system-specific functions of
** wildcard file specifier handling.  These are the primitives:
**
**	Setwild		set initial wildcard string
**	Nextwild	get next wildcard specifier in Q_file
**	Unsetwild	clean up wildcard handling
**
** The handling here is for UNIX.  The approach is to do an
** 'ls' command to a file when the wildcard string is specified
** and to read the file, one path string at a time, for each
** Nextwild.  Unsetwild can be used as a general clean-up, and
** for us it is used to discard the temporary file containing
** the filenames.
*/



#include <stdio.h>
#include "bool.h"
#include "chars.h"
#include "cmutil.h"
#include "errors.h"
#include "errmsg.h"
#include "fileio.h"
#include "qname.h"
#include "tb.h"



/* forward references */

void	appst0() ;
void	Unsetwild() ;
void	interr() ;


/* local data */

static FILE * wildfile;			/* current stream for temporary
					** file or NULL if none
					*/

static char * wildname;			/* pointer to temporary filename */




/* Setwild -- set up wildcard specifier
**
** This routine initializes the wildcard specifier.  It does a
** "system" call with an ls command and writes the results to a
** temporary file.  To avoid possible syntax errors from the shell
** for bad file names, we do a 'sh 2>/dev/null -c "ls ..."'.
** We do an ls -p to avoid searching directories recursively and to
** give us a '/' at the end of directory names.
** If there was an existing temporary file, it is deleted first.
*/

void
Setwild(s,len)
char * s;				/* pointer to wildcard string */
int len;				/* length of string */
{

    Unsetwild();			/* kill off any current file */

    TBkill(Q_btemp);			/* use temporary text block to
					** build command string
					*/

#ifdef DEBUG
    if (len == 0)			/* shouldn't get null string */
	interr("null string in Setwild");
#endif

    appst0("sh -c 2>/dev/null \"ls -p "); /* start command */
    Addst(Q_btemp,s,len);		/* append caller's specification */
    appst0(" 2>/dev/null >");		/* force "not found" messages not
					** to show up in file
					*/
    appst0(wildname = tmpnam((char*) 0)); /* make temp file name, add to
					** command string
					*/
    Addc(Q_btemp,'"');			/* append " */
    Addc(Q_btemp,NUL);			/* append NUL to string for "system" */

    (void) system(TBtext(Q_btemp));	/* do the ls command */

    /* open file */

    wildfile = Fopread(wildname);	/* if NULL, we probably had an error
					** in the file specifier.  Treat as
					** if EN file closed.
					*/
    
    TBkill(Q_btemp);			/* clean up text block */
    return;
}
/* Nextwild -- get next filename in wildcard search
**
** This routine reads the next filename from the temporary file
** and stores it in Q_file.  The text block is empty if there is
** no next file.
*/

void
Nextwild()
{
    int i;

    TBkill(Q_file);			/* discard current contents of
					** text block
					*/

    if (wildfile == NULL)		/* return none if no current
					** wildcard specifier or all done
					*/
	return;
    
    /* read through trailing stuff of previous name, if any */

    while ((i = Freadc(wildfile)) == LF)
	;				/* do nothing */

    while ( i != LF && i != EOF )	/* read to next LF or EOF */
    {
	Addc(Q_file,(char) i);		/* add to text block */

	i = Freadc(wildfile);		/* get next char and continue */
    }

    if (i == EOF)			/* clean up if reached end */
	Unsetwild();
    
    return;
}


/* Unsetwild -- clean up wildcard handling
**
** This routine cleans up any lingering junk resulting from wildcard
** processing.
*/

void
Unsetwild()
{
    int unlink();

    if (wildfile != NULL)		/* delete file if any */
    {
	(void) Fclose(wildfile);	/* close file first */

	(void) unlink(wildname);	/* then delete it */
	
	wildfile = NULL;		/* turn off wildcard specifier */
    }
    return;
}
/* Utility routine used by the above */


static void
appst0(s)				/* append NUL-terminated string
					** to Q_btemp
					*/
char * s;
{
    extern int strlen();

    Addst(Q_btemp,s,strlen(s));		/* append to temp text block */
    return;
}



