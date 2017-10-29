static char SCCSID[] = "@(#) tb.c:  4.1 12/12/82";
/* tb.c
 *
 *	TECO text block management routines
 *
 *	David Kristol, February, 1982
 *
 *	TECO manages all of its text in what are called "text blocks",
 *	which are contiguous clumps of bytes in memory.  Text blocks
 *	include Q registers and the main text buffer.  Outside of this
 *	package, text blocks are referred to by a text block number
 *	(defined in qname.h).  The only externally visible attributes
 *	for a text block are its size (number of characters therein)
 *	and a pointer to the text in memory.  The text pointed to by a
 *	text block must be considered read-only.  Internally we maintain
 *	the number of bytes actually allocated to the text block, which
 *	is always at least as large as the number of bytes used.  The
 *	extra allocation comes about because of the granules used by
 *	the storage manager.
 *
 *	All additions to text blocks go through one of the routines in
 *	this package.  The package automatically allocates additional
 *	space as needed by a text block.  Some of these operations can
 *	fail if, in fact, the space isn't available.
 *
 *	The package also provides for declaring a static character
 *	string to be the contents of a text block.  The allocated size
 *	is declared to be -1 to distinguish this case.  When such a
 *	text block is deleted, the header is merely reset, rather than
 *	deallocating the text space.
 *
 *	For debugging purposes TBsize and TBtext are defined as functions.
 *	Otherwise they are macros to improve performance.
 *
 *	These functions are available outside of this package:
 *
 *		TBins	insert bytes into text block
 *		TBapp	append bytes to text block
 *		TBdel	delete bytes from text block
 *		TBkill	delete entire text block
 *		TBrepl	replace bytes in text block
 *		TBneed	make sure N bytes are available in text block
 *		TBhave	declare that N bytes have been added to text block
 *		TBsize	return size (in bytes) of text block
 *		TBtext	return pointer to bytes comprising text block
 *		TBreorg	reorganize text storage so there are no holes:
 *			the maximum amount of space is available for a
 *			new text block at the end.
 *		TBrename rename text block
 *		TBstatic declare static string as text block
 *		TBinit	initialize text block storage
 *		isTB	is pointer in one of the text blocks? (debug code)
 */



#include "qname.h"			/* Q register names */
#include "bool.h"
#include "memory.h"
#include "tb.h"



/* external subroutines */

extern void	TBfree();


/* static data */


/* See tb.h for text block structure definition */

/* Define here the text block header data structures */

#define TBPTR struct Tblk *	/* define pointer to text block */
#define TBnil (char *) 0;	/* nil pointer for text block data */
#define TBSTATIC -1		/* value of ->alloc if static text block */

/* Allocate space for the text blocks.  The symbols are defined in
 * qname.h .  We are depending on the values of Q_MIN and Q_MAX to
 * be reasonable.
 */

struct Tblk TB[Q_MAX+1];	/* provide space for text block headers */

extern void interr();		/* function definition */


/* Define non-DEBUG "function" */

#ifndef DEBUG

#define TBaddr(tb)	(&TB[tb])	/* address of text block structure */

#else

TBPTR TBaddr();				/* declare for everyone */

#endif
/*
 * This first group of routines constitutes the ones that are
 * externally callable.
 */

/* TBins -- insert character string into text block
 *
 * This routine inserts the argument character string into the
 * designated text block.  The only failure is if we run out of
 * available memory.
 */

BOOL				/* FALSE if we run out of memory */
TBins(tb, pos, len, str)
int tb;				/* text block number */
int pos;			/* position *after* which to insert text */
int len;			/* length of char. string to insert */
char * str;			/* string to insert */
{
    BOOL TBspread();		/* functions defined later */
    void TBcopy();

    if (! TBspread(tb,pos,len))	/* fail if we can't make sufficient room */
	return(FALSE);

    TBcopy(tb,pos,str,len);	/* else, copy string into text block */
    return(TRUE);
}
/* TBapp -- append string to text block
 *
 * This routine is essentially the same as TBins, except the insert
 * takes place implicitly at the end of the text block.
 */

BOOL
TBapp(tb,len,str)
int tb;				/* text block number */
int len;			/* length of char. string to append */
char * str;			/* string to append */
{
    return(
	TBins(tb,TBaddr(tb)->used, len, str)
				/* do the equivalent insert */
	  );
}
/* TBdel -- delete text from text block
 *
 * This routine deletes parts of a text block.  The allocated space of
 * the text block is not reduced, unless the resulting text block is
 * empty.  In that case the text block is deallocated.
 */


void
TBdel(tb,pos,len)
int tb;				/* text block number */
int pos;			/* starting position for deletion */
int len;			/* length of deletion:  delete 'len'
				 * bytes, starting with position 'pos'
				 */
{
    void TBshrink();		/* define function */

    TBshrink(tb,pos,len);	/* just shrink the text block */
    return;
}
/* TBkill -- delete all text in text block
 *
 * This routine deletes all of the text in a text block.  It is
 * simply an externally available call to TBfree.
 */

void
TBkill(tb)
int tb;				/* text block number to delete */
{
    void TBfree();

    TBfree(tb);			/* free up the text */
    return;
}
/* TBrepl -- replace text in text block
 *
 * This routine replaces part (or all) of the text in a text block.
 * The size of the text block will be altered appropriately.  If more
 * text is deleted than inserted, the allocated size of the text block
 * will not change.
 */

BOOL				/* FALSE if expansion required, but it
				 * failed, TRUE on success
				 */
TBrepl(tb,pos,dellen,str,strlen)
int tb;				/* text block number */
int pos;			/* starting position for replacement */
int dellen;			/* number of bytes to replace */
char * str;			/* replacement string */
int strlen;			/* length of replacement string */
{
    BOOL TBspread();		/* define functions */
    void TBcopy();

/* Our strategy is to expand or shrink the text block first by the
 * difference in sizes between the deleted and inserted text strings.
 * We could, of course, delete one string and insert another, but that
 * would be unnecessarily sloppy
 */

    if (dellen > strlen)	/* deleting more than inserting */
	TBdel(tb,pos+strlen,dellen-strlen);
				/* delete the excess characters at the
				 * end
				 */
    else if (strlen > dellen)	/* inserting more than deleting */
	if (! TBspread(tb,pos+dellen,strlen-dellen))
	    return(FALSE);
				/* make room for more characters */

/* The text block size should now be correct.  Just move the text */

    TBcopy(tb,pos,str,strlen);
    return(TRUE);		/* we succeeded */
}
/* TBneed -- request extension of text block
**
** This routine guarantees that N bytes are available in the text block,
** and it returns a pointer to the first such free byte at the end of
** the text block.  TBneed is intended primarily for TECO's I/O, where
** the I/O routines try to assure that there is enough space in advance.
*/

char *				/* return pointer to end or NULL if not enough
				** space available
				*/
TBneed(tb,size)
int tb;				/* text block where space needed */
int size;			/* amount of available space needed */
{
    BOOL TBextend();

    if (TBextend(tb,size))	/* TBextend will extend allocated size */
	return(TBtext(tb)+TBsize(tb));
				/* if success, return pointer to end */
    
    return((char *) 0);		/* denote failure */
}
/* TBhave -- declare bytes added to text block
**
** This routine is a companion to TBneed:  it declares that some bytes
** have been added to the end of a text block.  Disasters can occur
** if bytes are added in the wrong place, or if too many are added.
*/

void
TBhave(tb,ptr,size)
int tb;				/* text block to which stuff was added */
char * ptr;			/* place where addition started */
int size;			/* number of bytes added */
{
    TBPTR tbp = TBaddr(tb);	/* point at text block info */

/* check for disasters */

#ifdef DEBUG
    if (tbp->text + tbp->used != ptr)
	interr("Bad pointer to TBhave"); /* didn't start at old end */
    
    if (tbp->used + size > tbp->alloc)
	interr("Size too big to TBhave"); /* added too many bytes */
#endif

    tbp->used += size;		/* normally, just bump number of used bytes */
    return;
}
/* TBsize -- return text block (used) size
 *
 * This routine returns the number of characters actually used in a
 * text block.
 */

#ifdef DEBUG

int
TBsize(tb)
int tb;				/* text block number */
{
    return(TBaddr(tb)->used);	/* return the size */
}

#endif	/* def DEBUG */


/* TBtext -- return pointer to text block's text
 *
 * This routine returns a pointer to the characters comprising a
 * text block's text.  Outside of this package the text should be
 * considered read-only.
 */

#ifdef DEBUG

char *
TBtext(tb)
int tb;				/* text block number */
{
    return(TBaddr(tb)->text);	/* return the pointer */
}

#endif	/* def DEBUG */


/* TBreorg -- reorganize text block space
 *
 * This routine, with the help of the storage manager, reorganizes the
 * text block storage space.  All text is moved to one end of the space,
 * leaving all free space in one blob at the other end.  TBreorg will
 * potentially invalidate all text block text pointers (as obtained
 * by TBtext).  We assume here that text is moved to **lower** addresses
 * in memory!!
 */

void
TBreorg()
{
    char * lbound;		/* lower bound of changed pointers */
    char * to;			/* address to which text beginning at
				 * lbound is moved to
				 */
    int nbytes;			/* number of bytes moved */
    int i;			/* loop index */
    char * ubound;		/* upper bound of changed pointers */
    int adjust;			/* amount by which to adjust pointers */
    TBPTR tbp;			/* pointer to a text block header */

    BOOL SMmove();		/* declare functions */

/* Our strategy is to make repetitive calls to SMmove.  Each successful
 * call moves a block of text.  All text block text pointers that fall
 * within the moved area must be changed to reflect the new location of
 * the text.
 */

    while (SMmove(&lbound,&to,&nbytes))
    {
/* SMmove has moved stuff.  Adjust all relevant pointers */

	ubound = lbound + nbytes; /* upper bound of changed pointers */
	adjust = lbound - to;	/* amount by which to adjust pointers */

	for (i=Q_MIN; i <= Q_MAX; i++) /* for each text block */
	{
	    tbp = TBaddr(i);	/* pointer to next text block */
	    if ( tbp->text >= lbound &&
		 tbp->text <  ubound )
		tbp->text -= adjust;	/* adjust this pointer */
	}
    }
    return;
}


/* TBrename -- rename text block
**
** This routine effectively renames a text block:  the "to" text block
** is deleted, then the header information for the "from" is copied to
** the "to" header.  Then the "from" header is initialized to empty.
** The "from" text block is thus effectively left empty, and the old
** contents of the "to" text block are deleted.
*/

void
TBrename(from,to)
short from;				/* "from" tb number */
short to;				/* "to" tb number */
{
    TBPTR tbp;				/* pointer to "from" header */

    TBkill(to);				/* kill old "to" contents */

    *TBaddr(to) = *(tbp = TBaddr(from)); /* copy header structure */

    tbp->used = tbp->alloc = 0;		/* reset "from" header */
    tbp->text = TBnil;
    return;
}


/* TBstatic -- declare static string as text block content
**
** This routine assigns the value of a static string as the text
** portion of a text block.  The text block is thereafter treated
** identically to one that whose text is allocated from dynamic
** memory, except that it doesn't get reorganized, and the text
** space is not deallocated when the text block is killed.
*/

void
TBstatic(tb,ptr,len)
short tb;				/* text block number */
char * ptr;				/* pointer to static string */
int len;				/* length of static string */
{

    TBPTR tbp = TBaddr(tb);		/* point at tb header */

    TBfree(tb);				/* delete current contents of tb */

    tbp->used = len;
    tbp->alloc = TBSTATIC;		/* special signal value for static */
    tbp->text = ptr;
    return;
}


/* TBinit -- initialize text block storage
 *
 * This routine initializes all of the text block headers to a nice,
 * healthy state.
 */

void
TBinit()
{
    TBPTR tbp;				/* pointer to text block header */
    int i;				/* loop index */

    for (i = Q_MIN; i <= Q_MAX; i++)
    {
	tbp = TBaddr(i);		/* get header pointer */
	tbp->used = tbp->alloc = 0;	/* no bytes allocated or used */
	tbp->text = TBnil;		/* no text pointer */
    }
    return;
}
/* Second level routines
 *	These routine move stuff around in text blocks but don't
 *	call any routines outside of the TB package.
 */


/* TBshrink -- shrink text block
 *
 * This routine shrinks the effective size of a text block by removing
 * characters from the middle.  The allocated size of the text block
 * is unaffected unless the resulting size is 0, in which case the
 * text block is deallocated.
 */

static void
TBshrink(tb,pos,len)
int tb;					/* text block to shrink */
int pos;				/* position to start shrinking at */
int len;				/* length of shrinking:  number
					 * of characters to remove
					 */
{
    TBPTR tbp = TBaddr(tb);		/* get text block header */

    char * from;
    char * to;				/* pointers to text after, at
					 * point of shrinkage
					 */
    int move;				/* number of chars to move */
    extern int SMshrink();
    extern void interr();
    BOOL statictb = (tbp->alloc == TBSTATIC);
					/* TRUE if static tb */

#ifdef DEBUG
    if (				/* check funny arguments */
	    pos+len > tbp->used
	||  pos < 0
	||  len < 0
	)	
	interr("bad pos or len in TBshrink");
#endif

    if (len <= 0)			/* done if nothing to shrink */
	return;

    to = tbp->text + pos;		/* position to which chars will
					 * be moved
					 */
    from = to + len;			/* position from which chars moved */
    move = tbp->text + tbp->used - from;
					/* number of chars from 'from'
					 * to end
					 */

#ifdef DEBUG
    if (move != 0 && statictb)
	interr("attempt to shrink static text block");
#endif

    (void) memcpy(to,from,move);	/* move chars down */
    if ((tbp->used -=len) == 0)		/* reduce used length */
	TBfree(tb);			/* deallocate if 0 */
    else if (! statictb)		/* still some in use and not static */
	tbp->alloc -= SMshrink(		/* shrink allocated size */
			    tbp->text + tbp->alloc,   /* pointer past end */
			    tbp->alloc - tbp->used ); /* available to return */
#ifdef DEBUG
    if (! statictb && tbp->used > tbp->alloc)
	interr("used > alloc in TBshrink");
#endif

    return;
}
/* TBspread -- open up hole in text block
 *
 * This routine opens up space in a text block to make room for an
 * insertion.  The routine can fail if the text block must be extended
 * and there is no more room available in storage.
 */

static BOOL
TBspread(tb,pos,size)
int tb;					/* text block number */
int pos;				/* position at which to start
					 * moving things
					 */
int size;				/* size of hole to open up */
{
    BOOL TBextend();

    TBPTR tbp = TBaddr(tb);		/* get text block header */
    char * from;			/* pointers to from, to positions */
    char * to;
    int move;				/* number of chars to move */

#ifdef DEBUG
    if (pos > tbp->used)		/* check reasonableness */
	interr("bad pos in TBspread");
#endif

    if (! TBextend(tb,size))		/* make sure enough space exists */
	return(FALSE);

/* Because we are moving chars to the "right", we must move the end of
 * the text block first.  We can't use TBcopy because we are moving
 * chars to higher addresses.  Thus the loop is hard coded here.
 */

    from = tbp->text + tbp->used - 1;	/* pointer to last char in tb */
    to = from + size;			/* move 'size' chars right */
    move = tbp->used - pos;		/* move this many chars */

    while (--move >= 0)
	*to-- = *from-- ;		/* move stuff */
    
    tbp->used += size;			/* bump number of chars used */
    return(TRUE);			/* announce success */
}
 /* Third level routines.  These routines interface directly to the
  * storage manager.
  */


    extern int SMexpand();		/* define functions */
    extern int SMget();
    extern void SMfree();
/* TBextend -- allow for extension of used portion of text block
 *
 * This routine makes sure enough room exists to extend the used portion
 * of a text block.  If there isn't enough room, TBextend attempts to
 * add the necessary space and only fails if there is no more space to
 * be had.  Note that the ->text pointer could be invalid after a call
 * to TBextend.
 */

static BOOL
TBextend(tb,size)
int tb;					/* text block number */
int size;				/* amount of extra space desired */
{
    BOOL TBget();

    TBPTR tbp = TBaddr(tb);		/* get tb header pointer */
    int added;				/* number of bytes added */
    char * new;				/* pointer to new block for tb */
    int alloc;				/* allocated size of same */

/* this is a 4-step process:
 *	1.  If nothing is allocated to the tb, allocate the required
 *	    amount.
 *	2.  If enough space is already allocated, return success.
 *	3.  Try to expand the tb in place.
 *	4.  Allocate a new piece of memory of sufficient size, move the
 *	    old text block into it, delete the old one.
 */

#ifdef DEBUG
    if (tbp->alloc == TBSTATIC)
	interr("attempt to extend static text block");
#endif

    if (tbp->alloc == 0)		/* not allocated */
	return(TBget(tb,size));		/* get a new tb */

    if (tbp->used + size <= tbp->alloc)
	return(TRUE);			/* space already exists */
/* step 3:  try to extend in place */

    if ((added = SMexpand(tbp->text + tbp->alloc,	/* pos. after last */
			size - (tbp->alloc - tbp->used)
			)
	) != 0)
    {
	tbp->alloc += added;		/* success:  bump allocation */

#ifdef DEBUG
/* double check allocation */
	if (tbp->used + size > tbp->alloc)
	    interr("not enough added in TBextend");
#endif

	return(TRUE);			/* success */
    }

/* step 4:  get more space and move everything */

    if ((alloc = SMget(&new,tbp->used + size)) == 0)
	return(FALSE);			/* failed if no more memory */

    (void) memcpy(new,tbp->text,tbp->used); /* move the existing bytes */
    SMfree(tbp->text,tbp->alloc);	/* free up the old tb text */

    tbp->text = new;			/* reset relevant data */
    tbp->alloc = alloc;
    /* tbp->used stays the same */
    return(TRUE);			/* return success */
}
/* TBget -- get new text block
 *
 * This routine gets a text block of the required size.  If a hunk
 * of memory had previously been allocated to this text block, it
 * is freed first.  TBget can fail if there is not enough free space
 * to allocate the required hunk of memory.
 */

static BOOL
TBget(tb,size)
int tb;					/* text block number */
int size;				/* minimum required size */
{
    void TBfree();			/* define function */
    TBPTR tbp;				/* pointer to header of interest */

    TBfree(tb);				/* deallocate any existing text */
    if (size == 0)			/* size 0 is no problem */
	return(TRUE);

    tbp = TBaddr(tb);			/* get ptr to tb header */
    if ((tbp->alloc = SMget(&tbp->text,size)) == 0)
	return(FALSE);			/* if allocation fails */
    tbp->used = 0;			/* else set used portion to 0 */
    return(TRUE);
}
/* TBfree -- free up text block
 *
 * This routine deallocates any dynamic memory allocated to a text block.
 * It also resets all of the internal information.  It is not an error
 * to free up a text block with no memory allocated
 */

static void
TBfree(tb)
int tb;					/* text block number */
{
    void SMfree();

    TBPTR tbp = TBaddr(tb);		/* point at header */

    if (tbp->alloc > 0)
	SMfree(tbp->text,tbp->alloc);	/* deallocate the memory */
    
    tbp->alloc = tbp->used = 0;		/* reset for sanitary reasons */
    tbp->text = TBnil;
    return;
}
/* TBaddr -- get address of text block header
 *
 * This routine returns a pointer to the text block whose number
 * is provided as an argument
 */

/* See definition of TBaddr as macro when not DEBUG */

#ifdef DEBUG

static TBPTR
TBaddr(tb)
int tb;					/* text block number */
{
    TBPTR tbp = &TB[tb];		/* form the pointer */

/* do some validity checks on the world */

    if (tb < Q_MIN || tb > Q_MAX)
	interr("bad text block number in TBaddr");
    if (tbp->used > tbp->alloc && tbp->alloc != TBSTATIC)
	interr("used > alloc in TBaddr");
    return(tbp);			/* things look okay */
}

#endif
/* TBcopy -- copy text string into text block
 *
 * This routine copies a text string into a designated place in a
 * text block.
 */

static void
TBcopy(tb,pos,str,len)
int tb;					/* text block number */
int pos;				/* first position to fill */
char * str;				/* pointer to string to move */
int len;				/* length of string to move */
{
    TBPTR tbp = TBaddr(tb);		/* point at tb header */

#ifdef DEBUG
    if (pos > tbp->used)
	interr("bad pos to TBcopy");
#endif

    (void) memcpy(tbp->text+pos,str,len); /* do actual copy */
    return;
}
/* isTB -- test whether pointer falls in text block
**
** This routine tests whether an arbitrary pointer falls within one
** of the text blocks.  If it does, we return the start address of
** the text block, its length, and its number.
*/

#ifdef DEBUG				/* debug code only */


BOOL
isTB(p,pstart,plen,ptb)
char * p;				/* pointer to test */
char ** pstart;				/* place to store pointer to start
					** of tb
					*/
int * plen;				/* place to store length of tb */
short * ptb;				/* place to store tb number */
{
    int tb;				/* current tb we're checking */
    TBPTR tbp;				/* pointer to tb header */

    for (tb = Q_MIN; tb <= Q_MAX; tb++)	/* loop through text blocks */
    {
	tbp = TBaddr(tb);		/* point at tb header */
	if (
		tbp->alloc != 0		/* tb is in use */
	    &&  tbp->alloc != TBSTATIC	/* not a static array */
	    &&  tbp->text <= p		/* check pointer within limits */
	    &&  p < tbp->text + tbp->alloc
	    )
	{
	    *pstart = tbp->text;	/* return start of tb */
	    *plen = tbp->alloc;		/* length of tb */
	    *ptb = tb;			/* and text block number */
	    return(TRUE);		/* found pointer in text block */
	}
    }
    return(FALSE);			/* pointer not within tb */
}


#endif		/* DEBUG */
