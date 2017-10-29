static char SCCSID[] = "@(#) sm.c:  5.1 2/22/83";
/*	sm.c
 *
 *	TECO storage manager
 *
 *	David Kristol, February, 1982
 *
 *	TECO thinks it has complete control over an arena of memory
 *	starting with the brk value at initialization time and covering
 *	any memory into which it expands.  Therefore it is important
 *	that malloc either be bypassed or suppressed.
 *
 *	Those areas which are not marked as free are assumed to be under
 *	the control of the text block management routines.  This fact
 *	is important when we consider the SMexpand, TBextend, and
 *	TBreorg functions.
 *
 *	The storage manager exports these capabilities:
 *		SMget		get a block of storage
 *		SMfree		free a block of storage
 *		SMmove		consolidate storage:  move used block
 *				into free area
 *		SMexpand	try to expand used area into adjacent
 *				free area
 *		SMshrink	re-absorb part of a memory hunk as free space
 *		SMbumps		return number of memory increments
 *				since last call
 *		SMsetexp	enable/disable memory expansion
 *		SMisfree	is pointer in free space? (debug code)
 *		SMlimit		return arena limits (debug code)
 *		SMinit		initialize memory management
 *
 *	We define the following terms:
 *		granule		the size of the smallest memory request
 *				granted; all requests are rounded up to
 *				the nearest 'granule' multiple of bytes.
 *				A granule must be at least as large as the
 *				storage manager's free block header.
 *		increment	the amount of memory added to the storage
 *				arena when we request more memory from the
 *				operating system.  Increments are multiples
 *				of granules.
 *		bound		limit, in bytes, on expansion.  Attempts
 *				to expand beyond the limit will fail, even
 *				if the operating system would permit it.
 *
 */

#include "bool.h"
#include "memory.h"

void interr();			/* declare function

 /* These local data define the current state of the SM world */

 /* This is the structure for a free list entry */

 struct SMblock {
    struct SMblock * next;	/* next free block */
    int size;			/* size of this free block */
				/* characters follow implicitly */
};

#define SMPTR	struct SMblock *
#define SMnil	((SMPTR) 0)

/* static data */

/* freelist is really a dummy SMblock which serves to anchor
 * the free list.  The free list is sorted in ascending memory
 * address order.
 */

static struct SMblock freelist;	/* dummy first block on free list */
static char * smfirst;		/* pointer to first byte in arena (forever) */
static char * smend;		/* pointer to first byte beyond end of
				 * current arena */
static int smsize;		/* current arena size in increments */
static int smincr;		/* size of an increment in bytes */
static int smgranule;		/* size of granule in bytes */
static int smbound;		/* maximum amount of expansion permitted
				 * in 'smincr' units */
static int smbumps;		/* number of storage increments since last
				 * check */
static BOOL smexpand;		/* expand flag:  TRUE if expansion allowed */
/* SMinit -- initialize the whole thing
 *
 * This routine initializes the storage manager and grabs an initial hunk of
 * memory.
 */

BOOL				/* returns TRUE if resulting arena is
				 * non-empty */
SMinit(granule, incr, start, bound, expand)
int granule;			/* size of granule in bytes */
int incr;			/* size of increment in 'granules' */
int start;			/* initial number of increments to grab */
int bound;			/* upper limit on expansion, in increments */
BOOL expand;			/* expansion allowed (after initialization)
				 * if TRUE */
{
    BOOL retval;		/* temporary for returned value */

    extern char *sbrk();	/* define some functions we need */
    BOOL addmem();

/* copy some stuff */

    if ((smgranule = granule) < sizeof(struct SMblock))
	return(FALSE);		/* fail on bad granule size */
    smincr = granule * incr;
    smbound = bound;
    smsize = 0;			/* present size before getting memory */
    smexpand = TRUE;		/* allow expansion until done initialization */
    smbumps = 0;

/* initialize free list */

    freelist.size = 0;		/* not really available */
    freelist.next = SMnil;	/* and no memory yet */

    smfirst = smend = sbrk(0);	/* find out what the end of
				 * memory is now */

    retval = addmem(start*smincr);	/* get a hunk of memory, remember
					 * how we did */
    smexpand = expand;		/* set correct expansion flag */
    return(retval);		/* return remembered value */
}
/* SMget -- get a piece of memory
 *
 * This routine finds a piece of memory of the requested size.  If
 * no such piece is available, memory is expanded, if possible and
 * allowed, to provide for a piece of the requested size.
 */

int				/* return size of piece allocated */
SMget(ptr,size)
char ** ptr;			/* pointer to place to put to returned piece */
int size;			/* size of requested piece, in bytes */
{
/* We use a first fit strategy to allocate memory.  The idea here is to walk
 * along with two pointers pointing at adjacent pieces of the free list.
 * 'prev' points at the predecessor to the current block and 'cur' points
 * at the current block.  We start off pointing at the dummy free block
 * which is 'freelist'.
 */

    BOOL addmem();		/* define function type */
    SMPTR prev = &freelist;
    SMPTR cur = freelist.next;

/* round up size to nearest granule boundary */

    size = (size + smgranule - 1) / smgranule * smgranule;

/* find suitable piece or end of list */

    while ( (cur != SMnil) && (size > cur->size) )
    {	prev = cur;		/* move to next piece */
	cur = cur->next;
    };

/* If we failed to find a piece, try to expand the arena.
 * We must be allowed to increase the arena.
 */

    if (cur == SMnil)
	if ( addmem(size) )
		return(SMget(ptr,size)); /* recursive call !! */
	else
	    return(0);			/* no new block */
/* Reaching here, we have enough room for the new piece and we are
 * actually pointing at the piece from which to carve out the new one.
 */

    *ptr = (char *) cur;		/* carve stuff off front of block */
    if (size == cur->size)
	prev->next = cur->next;		/* if using all of piece, remove it */
    else
    {   SMPTR new = (prev->next = (SMPTR)((char *) cur + size));
					/* otherwise the new, remaining free
					 * piece is what's left after carving
					 * off the stuff we're returning.
					 */
	new->size = cur->size - size;	/* adjust size */
	new->next = cur->next;		/* propagate pointer to next block */
    }
    return(size);			/* return adjusted size */
}
/* addmem -- add memory to arena
 *
 * This routine adds a dolup of memory to the storage manager's arena.
 */

static BOOL				/* TRUE if success, else FALSE */
addmem(size)
int size;				/* amount of memory to add */
{
/* We round the requested size up to the nearest increment size and
 * try to add that.
 */

    extern int brk();			/* declare function */
    void SMfree();
    int incr = (size + smincr - 1) / smincr;
					/* number of units of size 'smincr' */

/* make sure we can still add memory */

    if ( ! smexpand || smsize >= smbound)
	return(0);			/* expansion off, or reached limit */

/* make sure we don't add too much */

    if (smsize + incr > smbound)
	incr = smbound - smsize;	/* just allow the remaining size */

    size = smincr * incr;		/* recompute size in bytes */

/* now allocate the new space */

    if (brk(smend+size) == 0)
    {   SMfree(smend,size);		/* add the new piece of free space */
	smend += size;			/* bump known end of memory */
	smsize += incr;			/* bump known size of memory */
	smbumps += incr;		/* added memory since last call */
	return(TRUE);			/* indicate success */
    }
    else
	return(FALSE);			/* else fail */
}
/* SMfree -- return a block of memory to free storage
 *
 * This routine returns to memory a block of storage that SMget provided.
 * Needless to say, things get very tangled up if the returned pointer or
 * size is bad
 */

void
SMfree(ptr,size)
char * ptr;				/* pointer to returned block */
int size;
{
/* Our strategy here is to find the place where the block fits in
 * according to address value.  Then we coalesce the block with any
 * abutting blocks.  As in SMget, we walk the free list with two
 * pointers, 'prev' and 'cur', until we find the right place to drop
 * our piece in.
 */

    void coalesce();			/* define function */
    SMPTR prev;				/* gets set in the code */
    SMPTR cur = &freelist;
    SMPTR smptr = (SMPTR) ptr;		/* a cast-ed version of ptr */

/* round up the size to an even granule size */

    size = (size + smgranule - 1) / smgranule * smgranule;

    smptr->size = size;			/* create a prototype SMblock */
    smptr->next = SMnil;

/* find place to put the block */

    while ( (prev = cur, (cur = cur->next) != SMnil) && (smptr > cur) )
	;
/* note that even if the returned piece goes at the end, we are positioned
 * correctly.  The new piece goes between 'prev' and 'cur'. */

    smptr->next = cur;			/* set new piece's next */
    prev->next = smptr;

/* Now we coalesce the new piece with the piece following it and then the
 * piece before it.  We choose that order because the header of the new
 * piece will still be a real header after the first coalesce, but after
 * the second, the header may fall in the middle of a larger block.
 */

    coalesce(smptr);			/* merge new with following */
    coalesce(prev);			/* merge prev with new */
    return;
}
/* coalesce -- make a piece of memory and its successor coalesce
 *
 * This routine coalesces a piece of memory and its successor if they
 * are adjacent in memory.
 */

static void
coalesce(p)
SMPTR p;				/* pointer to first block */
{
/* We can coalesce p and its successor if p plus the size of a block
 * is equal to the 'next' pointer.
 * We assume that p->next is non-null.
 */

    if ( (char *)p + p->size == (char *)(p->next) )
    {   p->size += (p->next)->size;	/* combine sizes */
	p->next = (p->next)->next;	/* propagate 'next' ptr */
    };
    return;
}
/* SMexpand -- expand a piece of used memory into free memory
 *
 * This routine tries to expand an existing piece of occupied memory
 * into free space above it, if possible.  If the expansion abuts
 * directly on the end of the arena, we try to get a new piece of
 * memory.
 */

int					/* 0 if failed, else amount added */
SMexpand(ptr,size)
char * ptr;				/* pointer to place where expansion
					 * must begin */
int size;				/* size of desired expansion */
{
/* As before we use an algorithm that retains two pointers, one to the
 * piece of interest, one to its predecessor.
 */

    SMPTR prev;				/* pointer to predecessor, gets set
					 * in code */
    SMPTR cur = &freelist;		/* current block of interest */

/* adjust size to be even granule multiple */

    size = (size + smgranule - 1) / smgranule * smgranule;

/* find the hole of interest, if it exists */

    while ( prev=cur, ((cur=cur->next) != SMnil) && (ptr > (char *) cur) )
	;

/* Either we have run off the end of the free list, in which case 'prev'
** points at the last free list entry, we have found a free block at the
** place we want one, or we have walked past any possible free list entry.
** There are these cases:
**
**	1.  'ptr' does not point at a block of memory on the free list.
**	2.  'ptr' points at a block of memory on the free list and:
**	    a) it is large enough.
**	    b) it is not large enough, and it is not at the end of the arena.
**	    c) it is not large enough, but it IS at the end of the arena,
**		so we can try to expand memory
**	3.  'ptr' equals 'smend', meaning we there is no suitable free block,
**		but we want more memory at the end of the arena, so we can
**		expand the arena.
*/

    if (
	    (char *) cur != ptr		/* case 1 */
	&&  ! ( cur == SMnil && ptr == smend ) /* not case 3 */
	)
	return(0);			/* can't extend memory piece here */
/* We will want 'cur' to point at the piece of memory into which we
** will expand.  'prev' will point at its predecessor.
*/

    if (cur == SMnil)			/* expansion not on free list (but
					** we know that ptr == smend)
					*/
    {
	cur = (SMPTR) smend;		/* new piece will begin at current
					** end
					*/
	if ( ! addmem(size) )		/* try to grab more memory */
	    return(0);			/* failed:  couldn't expand */
    }
    /* We know here that cur == ptr.  If the existing piece is big enough,
    ** we're all set.  Otherwise we can try to expand that piece if it's
    ** at the end of the arena.
    */

    else if (size > cur->size)		/* case 2b, 2c */
    {
	if ( (char *) cur + cur->size != smend ) /* not at end of arena */
	    return(0);			/* fail */
	if ( ! addmem(size - cur->size)) /* try to expand free block */
	    return(0);			/* fail */
    }

    /* A piece of sufficient size exists (we believe). */

#ifdef DEBUG
    if (prev->next != cur)		/* make sure we got what we thought */
	interr("pointer didn't match in SMexpand");
    if (cur->size < size)
	interr("size failure in SMextend");
#endif

/* This code extracts 'size' bytes from the available piece.  It looks
 * very much like equivalent code in SMget.
 */

    if (size == cur->size)
	prev->next = cur->next;		/* unlink block completely */
    else
    {   SMPTR new = (prev->next = (SMPTR)((char *)cur + size));
					/* start of truncated free block */
	new->size = cur->size - size;
	new->next = cur->next;
    };
    return(size);
}
/* SMshrink -- grab portion of memory and put back on free list
 *
 * This routine attempts to carve an integral number of granules off the
 * **end** of the supplied memory region and return it to free storage.
 * The idea here is that the caller is offering to us a portion of a
 * previously allocated memory hunk and saying we can have it back.  We
 * assume that the original hunk was granule-aligned, so we only want to
 * grab pieces that are granule-sized.
 */

int					/* number of bytes re-absorbed */
SMshrink(ptr,size)
char * ptr;				/* pointer to first byte just past
					 * end of piece offered back
					 */
int size;				/* number of bytes potentially
					 * returned
					 */
{
    int kept = (size / smgranule) * smgranule;
					/* amount we'll keep */

    if (kept > 0)			/* return to free storage whatever
					 * we kept
					 */
	SMfree(ptr-kept, kept);
    return(kept);			/* tell how much we kept */
}
/* SMmove -- interchange a free block and a used block
 *
 * This routine has a very specialized purpose:  it expects to be
 * called repetitively until all free space has been bubbled to
 * the end of the arena and all used space is consolidated at the
 * beginning.  It is used by TBreorg.  SMmove returns after each
 * free block has been bubbled so that TBreorg can adjust any pointers
 * that lie in the moved region.
 */

/* SMmove returns most of its interesting results via pointers */

BOOL					/* TRUE if anything moved */
SMmove(oldptr,newptr,nbytes)
char ** oldptr;				/* pointer to 'from' pointer */
char ** newptr;				/* pointer to 'to' pointer */
int * nbytes;				/* pointer to # of bytes moved */
{
    char * old;				/* 'from' pointer */
    char * new;				/* 'to' pointer */
    int size;				/* size of block moved */
    SMPTR oldblk;			/* old location of free block */
    SMPTR newblk;			/* new location of free block */
    SMPTR hunk;				/* free piece getting extracted */
    SMPTR next;				/* temporary copy of 'next' ptr */

/* We are done when the free list is empty or the last piece in the
 * free list is at the end of the arena.
 */

    hunk = freelist.next;		/* this is the piece we want to move */
    *newptr = new = (char *) hunk;	/* stuff will get moved here */
    size = hunk->size;			/* size of hole */

    if ( hunk == SMnil ||
	 new + size >= smend )
	     return(FALSE);

    *oldptr = old = new + size;		/* stuff moved from here */

    freelist.next = hunk->next;		/* remove piece from free list */
/* We have two choices now.  If the free list is empty, we must put a new
 * piece on that contains everything from the end of the moved piece to
 * the end of the arena.  Otherwise we add the length of the removed
 * free piece to the following free piece.
 *
 * In either case we must be careful:  we cannot make create the new
 * free block until after we have moved the contents of non-free piece.
 * Otherwise we could clobber some of its data.
 */

    if (freelist.next == SMnil)
    {
	newblk = (SMPTR) (smend-size);
	/* size stays unchanged */
	next = SMnil;			/* this will be 'next' ptr */
	*nbytes = smend-old;		/* the used memory was from 'old'
					 * to the end of the arena */
    }
    else
    {   oldblk = freelist.next;
	newblk = (SMPTR) ((char *)oldblk - size); /* make up a new header */
	next = oldblk->next;		/* future 'next' ptr */
	size += oldblk->size;		/* future size */
	*nbytes = (char *)oldblk - old;	/* amount to move is from end of
					 * original free block (old) to
					 * start of second free block
					 * (oldblk) */
    };
    (void) memcpy(new,old,*nbytes);	/* now copy the bytes we claimed
					 * we moved */
/* set up the new free block header */

    freelist.next = newblk;		/* will be first on free list */
    newblk->next = next;		/* 'next' ptr */
    newblk->size = size;		/* size */

    return(TRUE);
}
/* SMsetexp -- set memory expansion flag
 *
 * This routine turns the memory expansion capability on and off.
 */

void
SMsetexp(flag)
BOOL flag;				/* TRUE to enable expansion */
{
    smexpand = flag;			/* just copy flag */
    return;
}
/* SMisfree -- tell if pointer is in free space
**
** This debugging routine tells the caller whether a pointer resides
** in free space or not.  If it does, the routine returns the start
** of the corresponding free block and its length.
*/


#ifdef DEBUG				/* debug code only */

BOOL
SMisfree(p,pstart,plen)
char * p;				/* pointer to test */
char ** pstart;				/* pointer to place to store start
					** address
					*/
int * plen;				/* pointer to place to put length */
{
    SMPTR cur;				/* current free block */

    for (cur = freelist.next; cur != SMnil; cur = cur->next)
    {
	if ( (char *) cur <= p && p < (char *) cur + cur->size)
					/* test whether pointer within block */
	{
	    *pstart = (char *) cur;	/* return pointer */
	    *plen = cur->size;		/* return length */
	    return(TRUE);		/* pointer is in free block */
	}
    }
    return(FALSE);			/* not found:  must not be free */
}
/* SMlimit -- get current low and high limits of arena
**
** This routine returns the arena bounds.
*/

void
SMlimit(pstart,pend)
char ** pstart;				/* start of arena */
char ** pend;				/* just past end of arena */
{
    *pstart = smfirst;
    *pend = smend;
    return;
}


#endif	/* DEBUG */
/* malloc -- stub to tie it off */

char *
malloc(size)
unsigned size;
{
    interr("Malloc was called");
}

void
free(ptr)
{
    interr("Free was called");
}

char *
realloc(ptr,size)
char * ptr;
unsigned size;
{
    interr("Realloc was called");
}

char *
calloc(nelem,elsize)
unsigned nelem, elsize;
{
    interr("Calloc was called");
}
