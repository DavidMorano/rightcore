/*	static char h_mdconfig[] = "@(#) mdconfig.h:  6.1 8/25/83";	*/
/* mdconfig.h -- machine dependent configuration parameters */

#define MAXSTACK 20		/* max. paren depth */
#define MAXXSTACK 200		/* max. execution stack depth:  figure
				 * about 3 words per macro or loop
				 */

/* storage manager granularity is a trade-off of conflicting goals:
 * making it larger results in more unused space;  making it smaller
 * results in higher overhead for extending text blocks.
 * In any case the value must be an integral multiple of the natural
 * byte boundary that 'brk' uses.
 */

#define SMGRAN 64		/* storage granularity */

/* The storage manager increment is the amount of memory that the storage
 * manager allocates at one time.  It is an integral multiple of the
 * granule size.  Because TECO periodically issues informational messages
 * about memory extension, this value should usually be 1024/SMGRAN.  Check
 * the code for the consequences of not doing so.
 */

#define SMINCR (1024/64)	/* storage increment */


/* The storage manager can be constrained not to use more than a maximum
** amount of memory.  The amount is expressed below in increments.
*/

#define SMMAXSIZE (50)		/* 50K text space */


/* The storage manager will allocate an initial amount of space, which
** is expressed below in units of increments.
*/

#define SMINITSIZE (10)		/* 10K initial text space */


/* Filename length (or pathname length for UNIX systems) is an arbitrary
 * length string.  For most DEC operating systems, the maximum size of a
 * filename is known in advance.  Use that value + 1.  For UNIX systems
 * the length of a pathname is arbitrary, but we can allow for very large
 * ones.  Choose something reasonable.
 */

#define NAMELEN	100		/* pathname maximum length */


/* "Search length" is a vague term which has to do with the number of
** search match operations to be performed.  Matching a literal string
** is one operation, but it takes two arguments, for a total of 3 in
** the count here.  Most other operators take 0 or 1 arguments.
** Increasing this number decreases the likelihood of getting STL errors.
** Empirically, though, search strings are not all that complex, so this
** does not have to be enormous.
*/

#define MAXSEARCH	30	/* maximum "search length" */


/* UNIX 5.0 and later systems introduced memory functions that deal with
** contiguous strings of bytes in memory without regard for embedded
** NUL characters.  For earlier systems, comparable routines are needed.
** Define MEMFUNCTION to use standard library routines or undefine it to
** use TECO's version.
*/

#define MEMFUNCTION

/* TECO's version number, to be reported by the EO command */

#define	TECVERSION	(36)		/* compatible with RSX/VMS versions */


/* When TECO reads an input file, it tries to leave some room in the text
** buffer for editing, even if the buffer "fills up".  In this implementation
** we specify a "cushion" of space which is available, but that does
** not contain any characters.  We also need to specify an assumed
** maximum sized line so we can define static input buffers for reading.
** In the UNIX version, at least, this maximum size does not, in fact,
** define the upper bound on the size of any one line that is read.
*/

#define	CUSHION		512		/* comfortable amount of space */
#define	MAXLINE		200		/* ... a large line, indeed */

/* This version of TECO allows for searching several paths for
** certain files.  The variables below define various aspects
** of this feature.
** At present, only the EI command uses this feature.
*/

/* Define environment variable names.  Must be "-quoted string */

#define EIVAR		"TECPATH"	/* path for EI command */

/* Define default value for environment variables if they are not
** present in the environment.  Must be "-quoted string.
*/

#define	EIDFL		""		/* default path for EI command */

/* Define separator character for elements of path variables. */

#define PATHVSEP	':'		/* colon for UNIX */

/* Defining symbol ENVVAR enables the EE command (to look up an
** environment variable), and it enables the $ special Q register
** for the G and ^E commands.  If the symbol is undefined, TECO
** remains compatible (in this respect) with standard TECO.
*/

#define	ENVVAR				/* turn on EE command, etc. */

/* Defining CRTRUB turns on the typical CRT handling of erase
** characters.  If it's undefined, all erase handling is TECO
** standard:  the erased character is repeated.
** Your system must have something like termlib/terminfo/curses
** to support CRTRUB.  You'll have to change the makefile (look
** for CURSES=) to select the proper library of routines.  Also,
** you may have to change the routines that module crt.c uses.
** If you have just one CRT that you want to support, you can
** also define constant strings that do the proper screen
** management.
**
** By the way, at this writing, the curses library requires an
** ungodly extra 35K of code, so using it on PDP-11's is out.
*/

#define	CRTRUB				/* no CRT erase */
