static char SCCSID[] = "@(#) memory.c:  4.1 12/12/82";
/* memory.c
**
**	TECO memory functions
**
**	David Kristol, June, 1982
**
**	These functions serve as surrogates on systems that do no
**	have UNIX 5.0 memory functions.  Supported functions are:
**
**		memcpy		copy memory
**		memchr		look for character in string
**		memcmp		compare strings
**
**	These routines operate on arbitrary strings of characters,
**	ignoring embedded NULs.
**
**	The intention here is to be functionally compatible with the 5.0
**	functions so the standard library routines can be used in their
**	stead, if possible.
*/

#include "mdconfig.h"			/* to see whether to compile this */

#ifndef NULL
#define	NULL	((char *) 0)
#endif


#ifndef	MEMFUNCTION			/* compile if not using std. lib. */
/* memcpy -- copy bytes
 *
 *	This routine is a block copy of bytes from one place to another.
 *	No guarantee is made about the respective alignments
 *	of the from and to data, but from must be > to.
 */

char *
memcpy(to,from,num)
register char * from;			/* copy from here... */
char * to;				/* ... to here */
register int num;			/* this many bytes */
{
    register char * totemp = to;	/* pointer to use for real */

    while (--num >= 0)
	*totemp++ = *from++;
    return(to);				/* return destination pointer */
}
/* memchr -- find character in string
**
** This routine searches for the first instance of a single character
** in a string.
*/

char *					/* pointer to found char, or NULL */
memchr(s,c,n)
register char * s;			/* pointer to string to check */
register char c;			/* character to find */
register int n;				/* length of string to check */
{
    while (--n >= 0)
	if (*s++ == c)
	    return(--s);		/* back up to found character */
    return(NULL);			/* failed */
}
/* memcmp -- compare strings
**
** Compare two strings, based on a number of characters.  Return an integer
** that is greater than, less than, or equal to 0, based on the relative
** comparison of the first to the second string.
*/

int					/* relation of first to second string */
memcmp(s1,s2,n)
register char * s1;			/* first string */
register char * s2;			/* second string */
register int n;				/* number of chars */
{
    while (--n >= 0)
	if ( *s1++ != *s2++ )
	    return(*--s1 - *--s2);	/* if non-equal, return difference */
    return(0);				/* success.  return 0 */
}

#endif	/* ndef MEMFUNCTION */
