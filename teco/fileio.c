static char SCCSID[] = "@(#) fileio.c:  5.1 2/22/83";
/* fileio.c
 *
 *	TECO machine-dependent file i/o routines
 *
 *	David Kristol, March, 1982
 *
 * This module contains routines that attempt to encapsulate the machine-
 * dependent parts of file input/output into machine-independent concepts.
 * Portions of this module may need to be changed for different environ-
 * ments.
 *
 * The fundamental structures are:
 *
 *	FILE	a structure containing everything the operating system
 *		needs to know about a file
 *	string	filename strings that are host-machine acceptable
 *
 * The model of processing we assume is:
 *
 *	1.  A file must be opened before reading or writing.
 *	2.  A file must be closed after reading and writing.
 *	3.  More than one input and/or output file can be open
 *		simultaneously.
 *	4.  The "to" filename of a rename must not already exist.
 *
 * The externally visible routines are:
 *
 *	Fopread		open file for reading
 *	Fopwrite	create a new file for writing
 *	Ftestfile	test file access
 *	Fclose		close file
 *	Frename		rename file
 *	Fdelete		delete file (by name)
 *
 *	Freadc		read character from stream
 *	Ferror		note whether real error occurred
 *	Fwritec		write character to stream
 *	Fread1line	read one line of input
 *
 *	Fmakebak	make backup file filename from original name
 *	Fmaketemp	make temporary file name sharing same directory as
 *			argument file name
 *
 *	Fgetmode	get file protection mode of existing file
 *
 *	Filepath	try different parts of path variable
 */



#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>			/* define system error codes */
#include	<string.h>
#include <stdio.h>

#include "bool.h"
#include "chars.h"
#include "mdconfig.h"
#include "mdtypes.h"
#include "memory.h"



/* external subroutines */

extern int	getpid() ;			/* get process id */

extern char	*getenv() ;

extern void	interr() ;



#define DIRSIZE	14			/* maximum filename size */

#define NBUFS	6			/* number of simultaneous buffers
					 * (= files) required
					 * (2 input, 2 output, 1 EI, 1
					 * wildcard file)
					 */

static char bufs[NBUFS][BUFSIZ];	/* reserve buffer space statically */

static FILE * bufuser[NBUFS];		/* for each buffer, the FILE that uses
					 * it
					 */


/* Buffer allocation and deallocation
 *
 * These routines provide buffers to the various FILEs that get opened
 * and closed during a TECO session.
 */

/* getbuf -- get buffer, do setbuf on it */

static void
getbuf(f)
FILE * f;
{
    short i;				/* current buffer # */

/* scan list of buffer users, looking for one that's free */

    for (i = 0; i < NBUFS; i++)
    {
	if (bufuser[i] == NULL)		/* if NULL, found one */
	{
	    bufuser[i] = f;		/* mark it used now */
	    setbuf(f,bufs[i]);		/* set this as file's buffer */
	    return;
	}
    }
    interr("failed to allocate buffer in getbuf");
}


/* retbuf -- return buffer to buffer pool */

static void
retbuf(f)
FILE * f;				/* file whose buffer we return */
{
    short i;				/* index into used buffer list */

    for (i = 0; i < NBUFS; i++)		/* scan used list for this file */
    {
	if(bufuser[i] == f)		/* found it if = */
	{
	    bufuser[i] = NULL;		/* mark as free */
	    setbuf(f,NULL);		/* unset buffer usage for file */
	    return;			/* done */
	}
    }
    interr("failed to return buffer in retbuf");
}
/* Fopread -- open file for reading
 *
 * This routine opens a file for reading.  The argument is the path name
 * of the file to open.  Returns NULL if the file will not open or the
 * FILE structure for the file.  errno contains the error number on error.
 * If the name string is null, we assume this is for (an already open)
 * standard input.
 */

FILE *
Fopread(s)
char * s;				/* suitable file (path) name */
{
    BOOL Ftestfile();

    FILE * f = NULL;			/* assume failure */

    if (s == (char *) 0)		/* null string? */
	f = stdin;			/* yes.  Set stdin as file */
    else if (! Ftestfile(s,'D'))	/* no.  Open the file if not a
					** directory.
					*/
	f = fopen(s,"r");

    if (f != NULL)
	getbuf(f);			/* allocate input buffer */
    return(f);
}
/* Fopwrite -- create file for writing
 *
 * This routine creates a new file for writing.  In other respects it
 * behaves like Fopread.  It sets the selected permissions, or uses
 * the default if "mode" is NOMODE.
 */

FILE *
Fopwrite(s,mode)
char * s;				/* file/path name */
FPRO mode;				/* new file protection mode */
{
    int creat();

    FILE * f = NULL;
    int newmode = 0666;			/* presumed file mode if none
					** supplied
					*/
    int fdesc;				/* file descriptor number */

    if (s == (char *) 0)		/* check null string */
	f = stdout;			/* set stdout on null string */
    else				/* otherwise, open new file */
    {
	if (mode != NOMODE)
	    newmode = mode;		/* use user-supplied mode */
	if ( (fdesc = creat(s,newmode)) >= 0)
	    f = fdopen(fdesc,"w");	/* open using standard i/o */
    }

    if (f != NULL)
	getbuf(f);			/* allocate output buffer */
    return(f);
}
/* Ftestfile -- test file access
 *
 * This routine tests various types of file access and returns a BOOLean
 * value.  The various types are:
 *	R	read access (errno set if read access denied, FALSE)
 *	W	write access (errno set if write access denied, FALSE)
 *	X	exists (errno set if file does not exist, FALSE)
 *	D	is directory (errno set no EISDIR if file is directory, TRUE)
 */

#define	RACCESS	04		/* read access */
#define	WACCESS	02		/* write access */
#define	EXISTS	00		/* exists */
#define	ISDIRECT	040000	/* is directory (for stat) */


BOOL					/* set according to above table */
Ftestfile(s,type)
char * s;				/* null-terminated file name to test */
int type;				/* test type to apply */
{
    struct ustat buf;			/* buffer for 'stat' */
    int stat();
    int access();
    extern void interr();

    extern int errno;			/* global error number */


    switch (type)			/* varied actions depending on type */
    {
    case 'R':				/* read access */
	return( access(s,RACCESS) == 0); /* TRUE if succeeds */
    
    case 'W':				/* write access */
	return( access(s,WACCESS) == 0); /* TRUE if succeeds */
    
    case 'X':				/* existence */
	return( access(s,EXISTS) == 0);	/* TRUE if succeeds */
    
    case 'D':
	if (stat(s,&buf) < 0)		/* if stat fails, file doesn't
					** exist (and isn't a directory)
					*/
	    return(FALSE);
	
	if ( (buf.st_mode & ISDIRECT) != 0)
	{
	    errno = EISDIR;		/* set "is directory" error */
	    return(TRUE);		/* say it's a directory */
	}
	else
	    return(FALSE);
    default:
	interr("Bad type in Ftestfile");
    }
/*NOTREACHED*/
}
/* Fclose -- close file (input or output)
 *
 * This routine closes the designated file.  Any local buffers are
 * flushed automatically.  The associated input/output buffer is
 * deallocated.
 */

BOOL					/* TRUE if errors */
Fclose(f)
FILE * f;				/* file to close */
{
    BOOL status = (fclose(f) != 0);	/* TRUE if errors */

    retbuf(f);				/* deallocate buffer */
    return(status);			/* return success or failure */
}
/* Fdelete -- delete selected file
 *
 * This routine deletes a named file.
 */

BOOL					/* TRUE if errors */
Fdelete(s)
char * s;				/* name of file to delete */
{
    return( (unlink(s) == 0 ? FALSE : TRUE) ); /* just unlink */
}



/* Frename -- rename selected file
 *
 * This routine renames a designated file from an old to a new name.
 */

BOOL					/* TRUE if errors */
Frename(old,new)
char * old;				/* old filename */
char * new;				/* new filename */
{

/* first link new name, then unlink old one */

    if ( link(old,new) != 0)
	return(TRUE);			/* link failed */
    return(Fdelete(old));		/* delete (unlink) old filename */
}
/* Freadc -- read character from stream
 *
 * This routine returns the next character from an input stream.
 * Characters are trimmed to 7 bits, except that a EOF indicates
 * end-of-file.
 */

int
Freadc(f)
FILE * f;				/* file to read from */
{
    int c = getc(f);			/* get char */

    return( (c == EOF ? EOF : c & 0177));
}


/* Ferror -- note error
**
** This routine distinguishes between real input errors and
** plain end-of-file.
*/

BOOL					/* TRUE if error is real */
Ferror(f)
FILE * f;				/* stream to check */
{
    return( ferror(f) != 0 );
}


/* Fwritec -- write character onto stream
 *
 * This routine adds the argument character to the output stream.
 * On error it returns TRUE.  Characters are truncated to 7 bits.
 */

BOOL
Fwritec(c,f)
char c;					/* character to write */
FILE * f;				/* file to write to */
{
    c &= 0177;				/* truncate here to avoid problem
					** (2/14/83) with expr. in putc
					*/
    if (putc(c,f) == EOF)
	return(TRUE);			/* error */
    return(FALSE);			/* no error */
}
/* Fread1line -- read one line of input
**
** This routine reads one line of input into a caller-supplied buffer.
** A line is defined as text ending with a carriage return and line feed.
** It is permissible to return a full (or nearly) full buffer which does
** not contain a CR/LF pair as well.  The idea here really is to return
** a clump of characters, rather than 1.  A line may conform to an
** operating system's idea of a record, too.
** Characters are truncated to 7 bits.  "Image mode" does not change
** new line or truncate characters to 7 bits.
**
** We return the number of characters actually available.  A return of
** -1 implies that no characters have been read and that there has been
** an error.  Ferror will tell whether the error is really EOF or not.
**
** n must be at least 2 so we can replace a \n with CR LF.
*/

int					/* number of chars actually in buffer */
Fread1line(f,buf,n,image)
FILE * f;				/* file to read from */
char * buf;				/* buffer to read into */
int n;					/* size of buffer */
BOOL image;				/* TRUE if we should read in "image"
					** mode"  (don't modify input stream)
					*/
{
    int i;				/* loop index */
    int newchar;			/* for read loop */
    char * oldbuf = buf;		/* original buffer pointer */

#ifdef DEBUG
    if (n < 2)
	interr("Buffer too small in Fread1line");
#endif	/* def DEBUG */
/* We loop using "getc" to obtain characters.  The loop exits when
** we encounter an error, the buffer is full, or we read a new line.
** In non-image mode we convert new line to CR/LF and truncate
** characters to 7 bits.
*/

    for ( i = n-1;			/* leave room to convert nl to CR/LF */
	  i > 0 && (newchar = getc(f)) != EOF;
					/* room in buffer, not error */
	  i--				/* decr. buffer count */
	)
    {
	if (! image)			/* truncate if not image mode */
	    newchar &= 0177;
	
	*buf++ = newchar;		/* save new char */

	if (newchar == '\n')		/* check for end of line */
	{
	    if (! image)		/* change to CR/LF? */
	    {
		*(buf-1) = CR;		/* convert last char from nl to CR */
		*buf++ = LF;		/* put in LF, too */
	    }
	    break;			/* done loop on nl */
	}
    }

/* At loop exit, "buf" points just past the last character stored.
** buf - oldbuf represents the number of characters stored.
** If zero characters have been stored, we must have encountered
** an error.  Otherwise the buffer contains characters as we promised.
*/

    return ( (buf - oldbuf) == 0 ? -1 : buf - oldbuf );
}


/* Fmakebak -- make backup file name
 *
 * This routine makes a suitable backup file name, given an original
 * filename.  The backup consists of the original name with '.B'
 * appended.  We return FALSE on success.
 */

BOOL
Fmakebak(in,out)
char * in;				/* pointer to original name */
char * out;				/* pointer to place to put
					 * resulting name
					 */
{
    char * last = in + strlen(in);	/* point at last char */

    char * first = strrchr(in,'/');	/* point at last / in path */


    if (first++ == (char *) 0)		/* if no /, point at first char,
					 * else, past last /
					 */
	first = in;

/* we want to make sure the length of the last path element is between
 * 1 and MAX-2 (2 for .B)
 */

    if (last-first <= 0 || last-first > DIRSIZE-2)
	return(TRUE);			/* error:  can't make name */
    
    while ((*out++ = *in++) != 0)	/* copy string */
	;
    
    *--out = '.';			/* append .B over null char */
    *++out = 'B';
    *++out = 0;				/* stick in null */
    return(FALSE);			/* no error */
}
/* Fmaketemp -- make temporary file name
 *
 * This routine returns a unique temporary file name with the property
 * that it shares the same directory with an input file name.  The idea
 * here is to make it possible to create temporary files that can later
 * be renamed to be a result file when closed, or deleted without harm
 * if killed.
 */

#define TEMPSIZE	11		/* temp. name TECpppppsss size */

BOOL					/* FALSE if name fits, TRUE if too
					 * long
					 */
Fmaketemp(inname,outname,maxlen)
char * inname;				/* input file name */
char * outname;				/* result (temporary) file name */
int maxlen;				/* maximum allowed length for name */
{
    static short tempid;		/* cyclic temporary sequence number */

    char * first = strrchr(inname,'/');	/* find last / in path */
    int preflen;			/* length of path prefix string */


/* In UNIX we want to find the path prefix to which we will attach a
 * temporary name.
 */

    if (first++ == (char *) 0)		/* bump past / if there is one */
	first = inname;			/* or point at beginning, if not */
    
    preflen = first - inname;		/* length of prefix */

/* The temporary file name is of the form TECpppppsss, where ppppp is the
 * (presumed 5 or fewer digit) process id and sss is the internal sequence
 * number.  The result must be shorter than DIRSIZE.
 */

#ifdef DEBUG
    if (TEMPSIZE > DIRSIZE)		/* bad size */
	interr("bad temporary file size in Fmaketemp");
#endif

    if (preflen + TEMPSIZE >= maxlen)	/* too big for caller? */
	return(TRUE);			/* yes */
    
    if (++tempid >= 1000)		/* cycle on 3 digits */
	tempid = 0;
    
    (void) sprintf(outname, "%.*sTEC%.5d%.3d",
		preflen,inname,getpid(),tempid);
					/* build temporary file name */
    return(FALSE);
}
/* Fgetmode -- get file protection modes
**
** This routine returns the file protection modes for an existing
** file.
*/

FPRO
Fgetmode(file)
FILE * file;				/* file whose protections we get */
{
    extern int fstat();

    struct ustat buf;			/* statistics buffer */

    if (fstat(fileno(file),&buf) != 0)	/* successful if 0 */
	interr("fstat fails in Fgetmode");
    
    return( buf.st_mode );		/* return mode statistics */
}
/* Filepath -- try elements of path variable
**
** This routine creates full file path names from a base name and
** a set of paths to try.  After assembling each one, Filepath
** calls a supplied routine until the routine announces "success"
** or all paths have been tried.  Filepath returns TRUE if some
** path was "successful", in the sense mentioned above.
** If the supplied "base name" is an absolute path, in some
** (system-dependent) sense, just that one path is tried.
**
** Filepath ignores paths which, when concatenated, would be too
** long for the supplied buffer.
** 
** The routine that gets called must return TRUE when the use
** of the path was successful.  Filepath calls the routine thus:
**
**	routine(name)
**
** where 'name' is a NUL-terminated path string that fits in the
** supplied (to Filepath) buffer.
*/

BOOL
Filepath(curpath,curplen,pathvar,pathdfl,target,targlen,routine)
char * curpath;			/* "base name" to append to path */
int curplen;			/* length of curpath */
char * pathvar;			/* name of path variable (NUL-terminated) */
char * pathdfl;			/* default path variable value if variable
				** not in environment
				*/
char target[];			/* place to store full path name */
int targlen;			/* size of target */
BOOL (* routine)();		/* routine to call with path name */
{
    char * curvar = getenv(pathvar);
					/* get value of environment variable */
    char * varend;		        /* absolute end of variable value */
    char * varnext;		        /* end of current path element */
    int varlen;			        /* length of current path element */
    char * targptr;		        /* helper pointer */

    if (curvar == NULL)		        /* if no such variable in environment */
	curvar = (pathdfl == NULL ? "" : pathdfl);
					/* use default unless it's null, too */


/* Do system-dependent checking for absolute paths in base name */

/* On UNIX systems, absolute paths begin "/", "./" or "../" */

    targptr = curpath;		        /* point at beginning */
    if (*targptr == '.')	        /* . or .. case */
	targptr++;
    if (*targptr == '.')	        /* .. case */
	targptr++;
    if (*targptr == '/')	        /* ./ ../ or / cases */
	curvar = "";		        /* reset path string to empty,
					** continue
					*/
    
    varend = curvar + strlen(curvar);   /* point past last char in path str. */
/* Now loop, picking off successive path elements and calling the
** supplied routine.
*/

    do				        /* always do at least once */
    {
	varnext = strchr(curvar,PATHVSEP);
				        /* find next path variable separator */
	if (varnext == NULL)	        /* if none, reset to end of string */
	    varnext = varend;
	
	varlen = varnext - curvar;      /* length of current path element */

	if (varlen + 1 + curplen + 1 <= targlen)
					/* need room for prefix, /, base name,
					** NUL
					*/
	{
	    targptr = target;
	    if (varlen > 0)		/* only prepend path, /, if non-0 */
	    {
		targptr = ((char *) memcpy(targptr,curvar,varlen)) + varlen;
					/* copy prefix (variable),
					** remember end
					*/
		*targptr++ = '/';		/* append path separator */
	    }
	    targptr = ((char *) memcpy(targptr,curpath,curplen)) + curplen;
					/* append base name, remember end */
	    *targptr = NUL;

	    if (routine(target))	/* see if routine succeeds */
		return(TRUE);		/* yes.  exit */
	}

	if (varnext == varend)
	    break;			/* if reached end of string */

	curvar = varnext + 1;		/* failed.  look at element past
					** separator
					*/
    } while (TRUE);			/* until exceed end of path var. */

    return(FALSE);			/* didn't find (or whatever) file */
}
