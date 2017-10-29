static char SCCSID[] = "@(#) cmfile.c:  5.1 2/22/83";
/* cmfile.c
 *
 *	TECO file stream-oriented commands
 *
 *	David Kristol, March, 1982
 *
 * This module contains those TECO commands that open and close files.
 * These include:
 *
 *	EA EB EC EF EI EK EN EP ER EW EX
 *
 * There are also two routines that command processors use to read and
 * write data from the text buffer:
 *
 *	Readlines	reads lines or a buffer of characters
 *	Writebuf	write characters from the text buffer
 *	Doyank		do much of Y command, including "yank test"
 *
 *
 * The strategy for handling output files is something like this.  We want
 * to preserve original files as long as possible.  On many systems and
 * especially UNIX, opening a file for write deletes any existing old file.
 * To get around this, we will open a temporary file for writing and later
 * rename it to be the desired name.  Thus the target name stays around as
 * long as possible in its old form.  This has the additional benefit that
 * the user can access it under its original name during an editing session
 * and before the new file is closed.
 */

#include <stdio.h>
#include <sys/types.h>			/* for mdtypes.h */
#include "bool.h"
#include "chars.h"
#include "cmutil.h"
#include "errors.h"
#include "errmsg.h"
#include "exittype.h"
#include "mdtypes.h"			/* needed by fileio.h: DON'T MOVE! */
#include "fileio.h"
#include "find.h"
#include "mdconfig.h"
#include "memory.h"
#include "qname.h"
#include "skiptype.h"
#include "tb.h"
#include "tflags.h"
#include "tty.h"
#include "values.h"
#include "xec.h"



/* external subroutines */

extern void	interr() ;


/* Define local data structures */

TNUMB Fl_ctN;			/* ^N (end of file) flag:  -1 if current
				 * input stream is at eof, else 0
				 */
TNUMB Fl_ctE;			/* ^E (form feed) flag:  -1 if last read to
				 * text buffer (via Readlines) ended with a
				 * form feed
				 */



#define PRIMARY		0		/* primary input or output */
#define SECONDARY	1		/* secondary input or output */

/* Each stream, whether input or output, is represented by one of these
 * structures:
 */

/* flags for stream */

#define S_OPEN	(1<<0)			/* set if stream is open */
#define S_EOF	(1<<1)			/* set if (input) stream at EOF */
#define S_IMAGE	(1<<2)			/* input stream:  set to accept
					 *   stream exactly; clear to turn
					 *   NL into CR LF
					 * output stream:  set to write text
					 *   buffer exactly; clear to turn
					 *   CR LF into NL
					 */
#define S_BACKUP (1<<3)			/* output file has backup name */
#define S_STDOUT (1<<4)			/* output file connected to stdout
					 * (has no temporary file name)
					 */

struct stream
{
    short flags;			/* contains S_ flags */
    char name[NAMELEN];			/* saved filename */
    FILE * file;			/* pointer to file's FILE structure */
};


#define STREAM 	struct stream

static STREAM inputs[2];		/* declare input streams */
static STREAM outputs[2];		/* declare output streams */



/* At initialization time, if stdin and stdout are selected to remain open,
 * we put dummy names in the stream structures.  The dummy names are
 * defined below.  They must contain ".  The name for standard output
 * is particularly important, as it will be printed in the OFO error if
 * the user tries to open a new output file without first closing this one.
 */

#define STDINNAME	"**stdin**"	/* standard input */
#define STDOUTNAME	"**stdout**"	/* standard output */


static char backname[2][NAMELEN];	/* names of backup files on
					 * primary and secondary streams
					 */
static char tempname[2][NAMELEN];	/* names of temporary files on
					 * both output streams
					 */

/* buffer handling stuff for Readlines */

static char inbufP[MAXLINE];		/* primary input buffer */
static char inbufS[MAXLINE];		/* secondary input buffer */
static char * bufbeg[2] =		/* buffer begin for each input stream */
			{ inbufP, inbufS };
static char * bufcur[2];		/* current buffer pointers in above */
static int bufleft[2];			/* characters left in each buffer */

static short curinp;			/* current input stream */
static short curout;			/* current output stream */

/* These variables support command (EI) files */

static char einame[NAMELEN];		/* filename length */
static BOOL EI;				/* (global) flag:  FALSE if no EI file 
					** in effect, else TRUE
					*/
static BOOL EInl;			/* TRUE if must output LF for new line
					** seen in EI file
					*/
static FILE * eifile;			/* FILE structure for EI file */



/* Fileinit -- initialize file operations */

void
Fileinit(iflag,oflag)
BOOL iflag;				/* TRUE to open stdin */
BOOL oflag;				/* TRUE to open stdout */
{
    STREAM * in = &inputs[PRIMARY];	/* some handy locals */
    STREAM * out = &outputs[PRIMARY];

    char * strcpy();

    curinp = PRIMARY;			/* set up primary input and output */
    curout = PRIMARY;

    in->flags = 0;			/* assume no flags */
    out->flags = 0;
    inputs[SECONDARY].flags = 0;
    outputs[SECONDARY].flags = 0;

    tempname[PRIMARY][0] = (char) 0;	/* turn off temporary file names */
    tempname[SECONDARY][0] = (char) 0;

    backname[PRIMARY][0] = (char) 0;	/* turn off remembered backup names */
    backname[SECONDARY][0] = (char) 0;

    bufleft[PRIMARY] = 0;		/* no characters in buffers */
    bufleft[SECONDARY] = 0;

/* now set up standard input and output */

    if (iflag)				/* TRUE to open stdin */
    {
	in->flags = S_OPEN;		/* primary in is open */
	in->file = stdin;		/* FILE structure */
	(void) strcpy(in->name,STDINNAME); /* dummy name */
	(void) Fopread(NULL);		/* indicate we're opening stdin */
    }

    if (oflag)				/* TRUE to open stdout */
    {
	out->flags = S_OPEN | S_STDOUT;
	out->file = stdout;
	(void) strcpy(out->name,STDOUTNAME);
	/* tempname is null, however */
	(void) Fopwrite(NULL,NOMODE);	/* indicate we're opening stdout */
    }

    return;
}
/* some service routines for the commands */

/* filerr -- output file-oriented error message
 *
 * This routine produces an error message based on the value of errno and
 * the contents of the Q_file text block.  If the : flag is set, we don't
 * bother with the message and just set a value.  Otherwise we print the
 * message, and there is no return from here.
 */

static void
filerr(emsg)
struct TER * emsg;			/* pointer to error message */
{
    char sbuf[200];			/* arbitrary size (big) buffer */
    int len;				/* length of result string */

    extern int errno;
    extern int sys_nerr;		/* number of defined system errors */
    extern char * sys_errlist[];	/* system error messages */

    if (Fl_colon != 0)			/* return if : flag set */
    {
	Set1val((TNUMB) 0);		/* first, set failure value */
	Eat_flags();			/* discard current flags */
	return;				/* then return */
    }

    if (errno <= sys_nerr)		/* if there's a message for this */
	len = sprintf(sbuf,"\"%.*s\":  %s",TBsize(Q_file), TBtext(Q_file),
			sys_errlist[errno] );
					/* build message in buffer */

    else
	len = sprintf(sbuf,"%%.*s\":  System error %d", TBsize(Q_file),
			TBtext(Q_file), errno);

/* message is built into sbuf buffer.  Output it. */

    terrSTR(emsg,sbuf,len);
    /* no return */
}
/* cpyname -- copy filename
 *
 * This routine copies a filename from one place to another and appends
 * a null character, making it an official C string.
 */

void
cpyname(from,to,len)
char * from;				/* "from" string */
char * to;				/* "to" string */
int len;				/* length to copy */
{
    while (len-- > 0)
	*to++ = *from++;
    *to = (char) 0;			/* add null char */
    return;
}
/* CMercmd -- do ER command
 *
 * This routine opens a file for reading on the current stream.
 */

short
CMercmd()
{
    int first;				/* initial position for filename */
    int len;				/* length of filename */
    STREAM * s = inputs + curinp;	 /* current input stream */


    Finddlm(ESC,&first,&len);		/* get delimited filename */

    if (Skiptype != SKNONE)		/* if skipping, exit */
	return(SKIPCONT);

    if (Fl_colon == 0)			/* eat values if no : */
    {
	Eat_val();			/* (could be macro) */
    }

    Buildstring(Q_file,first,len,FALSE);/* build filename string using string
					 * build characters, translate quoted
					 * characters
					 */

    if (TBsize(Q_file) >= NAMELEN)	/* filename too long? */
	terrTB(&Err_PTL,Q_file);	/* yes.  pathname too long */
    else if (TBsize(Q_file) == 0)	/* if no filename, just change to
					 * primary input
					 */
	curinp = PRIMARY;		/* switch to primary input */
    else				/* normal open on current stream */
    {
	if ((s->flags & S_OPEN) != 0)
	    (void) Fclose(s->file);	/* close current file */
	s->flags = 0;			/* mark as closed */
	cpyname(TBtext(Q_file),s->name,TBsize(Q_file));
					/* save filename */
	
	if ((s->file = Fopread(s->name)) == NULL)
	{
	    filerr(&Err_FER);		/* failed to open */
	    return(XECCONT);		/* exit */
	}
	s->flags = S_OPEN;		/* file now open */
	bufleft[curinp] = 0;		/* mark local buffer as empty */

	if (Fl_colon != 0)		/* if : flag set, return success */
	    Set1val((TNUMB) -1);
    }
    Fl_ctN = ((inputs[curinp].flags & S_EOF) != 0 ? -1 : 0);
					/* set eof flag, based on state of
					 * current stream
					 */
    Eat_flags();			/* discard current flags */
    return(XECCONT);			/* continue execution */
}
/* EW command:  open file for writing */

/* Although the documentation doesn't mention it, we support a :EW
** command because TECO.TEC uses one.  Besides returning a value,
** :EW suppresses the "superseding old file" message.
*/

short
CMewcmd()
{
    int first;				/* first position for file name in
					 * command string
					 */
    int len;				/* length of same */
    STREAM * s = outputs + curout;	/* pointer to current output stream */
    char * stemp;			/* temporary filename pointer */

    int strlen();

    Finddlm(ESC,&first,&len);		/* delimit filename string */
    if (Skiptype != SKNONE)		/* if already skipping, continue */
	return(SKIPCONT);
    
    Buildstring(Q_file,first,len,FALSE);/* build "search" string = filename,
					** translate quoted chars
					*/

    if ((s->flags & S_OPEN) != 0  && TBsize(Q_file) != 0)
   					 /* trying to open already open file */
	terrSTR(&Err_OFO,s->name,strlen(s->name));
    if (TBsize(Q_file) >= NAMELEN)	/* path too long */
	terrTB(&Err_PTL,Q_file);
    
    if (TBsize(Q_file) == 0)		/* just changing to primary output */
	curout = PRIMARY;		/* set to primary output */
    else				/* open new output file */
    {
	cpyname(TBtext(Q_file),s->name,TBsize(Q_file)); /* copy name
							 * to stream
							 */

	/* check filename for funny characters */

	for (stemp = s->name; *stemp != '\0'; stemp++)
	    if (*stemp <= ' ' || *stemp > '~')
	    {
		terrWRN(&Wrn_FNM);	/* issue warning */
		break;
	    }

	if (Fmaketemp(s->name,tempname[curout],NAMELEN))
	    terrTB(&Err_PTL,Q_file);	/* can't make temporary file name */
/* If the file already exists, it must not be a directory, and we must
** have write permissions for it.
*/
	if (    Ftestfile(s->name,'X')	/* already exists */
	    &&  (
		    Ftestfile(s->name,'D') /* check directory */
		 || ! Ftestfile(s->name,'W') /* check write permissions */
		 )
	    )
	{
	    filerr(&Err_FER);		/* announce file error */
	    return(XECCONT);		/* if return, just continue */
	}

	if ((s->file = Fopwrite(tempname[curout],NOMODE)) == NULL)
	{
	    filerr(&Err_FER);		/* failed to open */
	    return(XECCONT);		/* if return, just continue */
	}

	if (Fl_colon == 0 && Ftestfile(s->name,'X'))
					/* if file exists already, issue
					 * warning (if not :EW)
					 */
	    terrWRN(&Wrn_SUP);		/* superseding existing file */

	s->flags = S_OPEN;		/* opened successfully */
    }
    if (Fl_colon != 0)			/* return value if :EW */
	Set1val((TNUMB) -1);		/* set success */
    Eat_flags();			/* discard @ and : flags */
    return(XECCONT);
}
/* EB command:  open input, save old as backup, open output
 *
 * This command is a hybrid of ER and EW, but is sufficiently different
 * that it is hard coded.  The : modifier is allowed here, but it only
 * applies to the attempt to open the input file.
 */

short
CMebcmd()
{
    int first;				/* first position of file name in
					 * command string
					 */
    int len;				/* length of file name in string */
    STREAM * in = &inputs[curinp];	/* point at current input stream */
    STREAM * out = &outputs[curout];	/* point at current output stream */
    int tempflags = 0;			/* temporary file flags */
    FPRO oldmode;			/* protection modes of old file */

    char * strcpy();


    Finddlm(ESC,&first,&len);		/* delimit the file name string */

    if (Skiptype != SKNONE)		/* if skipping, continue now to do so */
	return(SKIPCONT);
    
    Buildstring(Q_file,first,len,FALSE);/* build a "search string" for the
					 * file name, translate quoted chars
					 */

/* Make sure string is neither too long nor zero length */

    if (TBsize(Q_file) >= NAMELEN)
	terrTB(&Err_PTL,Q_file);	/* report error */
    
    if (TBsize(Q_file) == 0)
	terrNUL(&Err_EBF);		/* zero length:  bad file name */

    if ((in->flags & S_OPEN) != 0)	/* close an open input file */
	(void) Fclose(in->file);
    
    in->flags = 0;			/* mark file closed */

    cpyname(TBtext(Q_file),in->name,TBsize(Q_file));
					/* copy name into stream */
/* Check that we have write permissions on file.  Fopread will check
** for read permissions and non-directory-ness.
*/

    if (    ! Ftestfile(in->name,'W')
	||  (in->file = Fopread(in->name)) == NULL
	)
    {
	filerr(&Err_FER);		/* error opening file */
	return(XECCONT);		/* continue if got a return */
    }
    in->flags = S_OPEN;			/* mark file open */
    bufleft[curinp] = 0;		/* mark local buffer as empty */

    if (Fl_colon != 0)			/* if :EB, set value now.  If we
					 * fail later, the value won't matter.
					 * Also, kill : flag so filerr below
					 * won't return
					 */
	Set1val((TNUMB) -1);		/* -1 means success */

    Eat_flags();
/* At this point we have succeeded in opening the input file.  This also
 * means that there is a file (the input file) which will need to be
 * renamed when the output file is closed.
 */

    if ((out->flags & S_OPEN) != 0)	/* make sure output is not open */
	terrSTR(&Err_OFO,out->name,strlen(out->name));
					/* else, output message */
    
    if (Fmakebak(in->name,backname[curout]))
    {
	terrWRN(&Wrn_BAK);		/* backup name bad -> no backup */
	/* tempflags initialized to 0 */
    }
    else
	tempflags = S_BACKUP;		/* will set S_BACKUP later */
    
    if (Fmaketemp(in->name,tempname[curout],NAMELEN))
	terrTB(&Err_PTL,Q_file);	/* flag error if can't make temp file
					 * name
					 */
    oldmode = Fgetmode(in->file);	/* get protection mode of old file */

    /* open new file with same mode as old file */

    if ((out->file = Fopwrite(tempname[curout],oldmode)) == NULL)
	filerr(&Err_FER);		/* error opening output file */
	/* won't return because : flag turned off */
    
    (void) strcpy(out->name,in->name);	/* set up output file name */
    out->flags = S_OPEN | tempflags;	/* set appropriate flags */

    return(XECCONT);
}
/* EK command -- kill current output file
 *
 * As outlined above, we delete the current temporary file.  Nothing
 * further is necessary.
 */

short
CMekcmd()
{
    void eksub();

    if (Skiptype == SKNONE)		/* if not skipping, kill current file */
	eksub();
    return(SKIPCONT);
}

static void
eksub()					/* subroutine used by EX */
{
    STREAM * s = &outputs[curout];	/* current stream */

    if ((s->flags & S_OPEN) == 0)	/* exit if not open */
	return;
    
    (void) Fclose(s->file);		/* close the opened file */
    if (tempname[curout][0] != (char) 0) /* if not stdout, delete file */
	(void) Fdelete(tempname[curout]);
    s->flags = 0;			/* file no longer open */
    backname[curout][0] = (char) 0;	/* turn off backup file name, too */
    return;
}
/* EF command -- close current output file where we are now */

short
CMefcmd()
{
    void efsub();

    if (Skiptype == SKNONE)		/* close file if not skipping */
	efsub();
    return(CONTINUE);			/* continue executing/skiping */
}

/* This subroutine manages the mechanics of file closing */

static void
efsub()
{
    STREAM * s = &outputs[curout];	/* current output stream */

    if ((s->flags & S_OPEN) == 0)	/* make sure file is open */
	return;				/* otherwise return */
    
    (void) Fclose(s->file);		/* close the file */

/* If there is a backup file name, we want to rename an existing target
 * file (s->name) to have the backup name.  In any case, we will then
 * rename the current temporary file to have the target name.
 */

    if ((s->flags & S_BACKUP) != 0)	/* if backup name exists */
    {
	(void) Fdelete(backname[curout]); /* delete existing backup, if any,
					   * ignore any errors
					   */
	(void) Frename(s->name,backname[curout]);
    }
    
    if ((s->flags & S_STDOUT) == 0)	/* rename output if not standard out */
    {
	(void) Fdelete(s->name);	/* delete old version of file, if any */
	(void) Frename(tempname[curout],s->name);
    }

    s->flags = 0;			/* file no longer open */
    backname[curout][0] = tempname[curout][0] = '\0';
					/* disable backup and temp file names */

    Eat_flags();			/* discard flags and values for
					 * everyone
					 */
    Eat_val();

    return;
}
/* EC command -- pass current input to output, close output */

short
CMeccmd()
{
    void Ecsub();

    if (Skiptype == SKNONE)		/* do it if not skipping */
	Ecsub(TRUE);			/* do "blind" close */
    return(CONTINUE);			/* continue execution or skipping */
}
/* Ecsub -- do guts of EC command
 * This subroutine does the real work of the EC command and is used by
 * the EX and EG commands as well.  It writes the current buffer,
 * passes the rest of the input file to the output file, then closes
 * the file.  If the argument is FALSE, we check whether any of this
 * makes sense.  If TRUE, we blindly go ahead and do the EC stuff.
 */

void
Ecsub(flag)
BOOL flag;				/* TRUE to do blind EC */
{
    TNUMB Dopcmd();			/* guts of P command */
    STREAM * istrm = &inputs[curinp];	/* point at current input stream */
    BOOL realinput = TRUE;		/* TRUE if input is real, FALSE
					** if we force input to look open
					*/

    if ((! flag) && (outputs[curout].flags & S_OPEN) == 0)
	    return;			/* done if no output file open */

    /* if no input file open, make it look like one is, and that
    ** it is at end of file.
    */

    if ( (istrm->flags & S_OPEN) == 0)
    {
	istrm->flags = S_OPEN | S_EOF;
	realinput = FALSE;		/* we have a fake input stream */
    }

    while ( Dopcmd(FALSE) == -1 )	/* keep looping until done, don't
					 * force FF at end of each buffer
					 */
	Testinterrupt();		/* just check for interrupt after
					 * each buffer
					 */
    efsub();				/* close output file after flushing */

    if (realinput)			/* if there's really an input file */
	(void) Fclose(istrm->file);	/* close the input file */
    
    istrm->flags = 0;			/* in either case, reset flags */

    return;
}
/* EX command -- flush current input to current output, close current
 * output, close other output in place, and exit TECO
 */

short
CMexcmd()
{
    extern void Tecexit();
    TNUMB exitcode;

    if (Skiptype != SKNONE)		/* continue if skipping */
	return(SKIPCONT);
    
/* If text buffer non-empty, make sure file is open */

    if (TBsize(Q_text) != 0 && (outputs[curout].flags & S_OPEN) == 0)
	terrNUL(&Err_NFO);		/* no file open */

    Ecsub(FALSE);			/* flush current output if file open */

/* Close remaining output files by killing them. */
    
    if (( outputs[curout = PRIMARY].flags & S_OPEN) != 0)
	eksub();
    if (( outputs[curout = SECONDARY].flags & S_OPEN) != 0)
	eksub();
    
    Set1dfl((TNUMB) 0);			/* set default exit value */
    (void) Get1val(&exitcode);		/* get the exit code */

/* Assume open input files don't matter */

    Tecexit((int) exitcode);		/* effect exit from TECO */
/*NOTREACHED*/
}
/* EA, EP commands
 *
 * EA switches current output to secondary stream.
 * EP switches current input to secondary stream.
 */

short
CMeacmd()
{
    if (Skiptype == SKNONE)		/* only do it if skipping */
    {
	curout = SECONDARY;		/* change output stream */
	Eat_flags();			/* discard flags and values */
	Eat_val();
    }
    return(CONTINUE);			/* continue, whether skipping or not */
}


short
CMepcmd()
{
    if (Skiptype == SKNONE)		/* analogous to above */
    {
	curinp = SECONDARY;
	Eat_flags();
	Eat_val();
    }
    return(CONTINUE);
}
/* Readlines -- read lines into text buffer
 *
 * This routine reads a designated number of lines, or an entire edit page,
 * into the TECO text buffer.  It also worries about the space management
 * issues that insure that there is adequate space left in memory for
 * useful editing.
 *
 * If the number of lines requested is <= 0, we read an entire text buffer.
 * This behavior corresponds to what TECO-11 does with a value <= 0 for
 * the :A command.
 *
 * The returned value corresponds to the TECO notion that a read command
 * succeeds when it reads 1 or more characters (including FF).
 *
 * We manage the FF and EOF flags here.
 */

#define SUCCESS		((TNUMB) -1)	/* success indicator */
#define FAILURE		((TNUMB) 0)	/* failure indicator */


/* define codes to communicate between fillup and Readlines */

#define	LINE		1		/* read a line with CR/LF */
#define	FULL		2		/* filled buffer without CR/LF */
#define	FORMFEED	3		/* read stuff followed by FF */

/* EOF retains its usually definition */
#define	NONE		0		/* used internal to fillup */


TNUMB					/* returns SUCCESS or FAIL, depending
					 * on whether any characters were read
					 */
Readlines(n,expand)
int n;					/* number of lines to read.  If <= 0,
					 * read full buffer
					 */
BOOL expand;				/* TRUE for unconditional expand */
{
    int fillup();
    extern void SMsetexp();
    extern TNUMB ED;			/* TECO ED flag */

    STREAM * s = &inputs[curinp];	/* current input stream */
    char * bufptr;			/* temp. pointer into text buffer */
    BOOL sawsomething = FALSE;		/* TRUE when we've added chars to
					** buffer
					*/

    if ((s->flags & S_OPEN) == 0)	/* stream must be open */
	terrNUL(&Err_NFI);
    
    Fl_ctE = 0;				/* no FF yet for this read */

    if ((s->flags & S_EOF) != 0)	/* if at eof already, we fail */
    {
	Fl_ctN = -1;			/* should be set already, but
					 * insure it
					 */
	return(FAILURE);		/* operation fails */
    }

    if (n == 0)				/* make 0 minus so end test works */
	n--;
/* We run this loop until we've collected the requested number of lines, we
 * get a FF, or we hit EOF.  To read a whole buffer we set the line count
 * negative so we should never hit a zero value.
 */

    while (n != 0)
    {

/* play around with enabling memory expansion around getting the
** required memory in the text buffer for the read.
*/

	SMsetexp(expand || (ED & ED_expand) != 0);
					/* ON if forced or enabled */
	bufptr = TBneed(Q_text,MAXLINE + CUSHION);
					/* require room for line and
					** extra space
					*/
	SMsetexp((ED & ED_expand) != 0);/* reset memory expansion */

	if (bufptr == NULL)		/* this means we failed */
	    return(sawsomething ? SUCCESS : FAILURE);
					/* note whether we read anything */
	
/* Now fill up the part of the text buffer we allocated, and find out
** what we got.
*/

	switch( fillup(s,bufptr,MAXLINE) )
	{
	case EOF:			/* hit end of file */
	    Fl_ctN = -1;		/* set TECO flag */
	    s->flags |= S_EOF;		/* set flag in stream */
	    return(sawsomething ? SUCCESS : FAILURE);
					/* succeed if we've read anything */
	
	case LINE:			/* read a line to CR/LF */
	    n--;			/* count another line */
	    /* fall through */
	case FULL:			/* filled buffer, no CR/LF */
	    sawsomething = TRUE;	/* say we got some chars */
	    continue;			/* continue loop */

	case FORMFEED:			/* read a something with a FF */
	    Fl_ctE = -1;		/* set TECO FF flag */
	    return(SUCCESS);		/* succeed */
	
#ifdef DEBUG
	default:			/* anything else */
	    interr("Unknown return code in Readlines");
#endif
	}	/* end switch */
    }	/* end while */
    return(SUCCESS);			/* read all requested lines */
}
/* fillup -- fill up part of the text buffer
**
** This routine is a service routine for Readlines.  It fills up a
** designated part of the text buffer and returns how it did.
** Possible results are:
**	EOF was reached.  No data were added to the buffer
**	A line was read up to a CR/LF.
**	A line was read that filled up the available space without hitting
**		a CR/LF.
**	A string of characters was read, followed by a FF.
**
** The processing is complicated by the fact that we keep part of the
** current input stream in temporary input buffers when we need to.
** The basic processing scheme is 3-fold:
**	1.  If there are characters in the temporary input buffer,
**		add them to the text buffer, up to a line terminator
**		or until they are all used up.  If a line terminator
**		is encountered, we're done.
**	2.  Read a line, up to a CR/LF, or until space runs out.
**	3.  Scan the stuff that was read, looking for a line terminator,
**		put any remaining stuff back into the temporary buffer.
**
** One extra complication is that because of the routine's logic
** a FF ends up in the buffer and must be backed up over.
*/

static int
fillup(strm,start,avail)
STREAM * strm;				/* stream to read from */
char * start;				/* where to read into */
int avail;				/* number of bytes available */
{
    void interr();

    int exittype = NONE;		/* no reason to exit yet */
    char * bufptr = start;		/* current buffer pointer */
    char * s;				/* handy pointer */
    int bufnow = bufleft[curinp];	/* remaining chars in temp. buffer */
    int buflinelen;			/* assumed length of current line */
    int remainder;			/* chars remaining in line read */
/* Step 1:  use characters remaining in temp. buffer */

    if (bufnow > 0)
    {
	if (bufnow > avail)
	    bufnow = avail;		/* restrict to available space */

	/* look for line terminator */

	if ((s = Findslt(bufcur[curinp],bufnow)) != NULL)
	{
	    buflinelen = s - bufcur[curinp] + 1;
					/* only consider stuff up to term. */
	    exittype = (*s == FF ? FORMFEED : LINE);
	}
	else
	{
	    buflinelen = bufnow;	/* no line terminator; use everything */
	    if (bufnow >= avail)
		exittype = FULL;	/* full line */
	}

	/* copy temp. buffer chars to text buffer */

	(void) memcpy(bufptr,bufcur[curinp],buflinelen);

	/* reset buffer data */

	bufleft[curinp] -= buflinelen;
	bufcur[curinp] += buflinelen;
	if (exittype != NONE)		/* do we have reason to exit? */
	{
	    if (exittype == FORMFEED)	/* bump down length */
		buflinelen--;

	    TBhave(Q_text,start,buflinelen); /* yes.  declare what we added */
	    return(exittype);
	}

	avail -= buflinelen;		/* reduce available space */
	bufptr += buflinelen;		/* bump current pointer */
    }
    exittype = FULL;		/* assume we will fill line */
/* Step 2.  We have now used up all characters in the temporary buffer.
** Read lines until we use up the available space or hit a line
** terminator.  We must read at least 2 chars.
*/

    while (exittype == FULL && avail > 1)
    {
	buflinelen = Fread1line(strm->file,
			bufptr,avail,((strm->flags & S_IMAGE) != 0));
	
	if (buflinelen == -1)		/* error indication ? */
	{
	    if (Ferror(strm->file))	/* yes.  Check for hard error */
		terrNUL(&Err_INP);	/* Is.  Announce same */

	    if (bufptr == start)	/* EOF.  Did we add to buffer? */
		return(EOF);		/* no.  Say we hit EOF */
	    
	    break;			/* otherwise escape loop */
	}

	/* See if there's a line terminator in the buffer */

	s = Findslt(bufptr,buflinelen);
	if (s == NULL)
	    remainder = 0;		/* no line terminator.  Use all */
	else
	{
	    remainder = buflinelen - (s-bufptr+1);
					/* chars in buffer after line term. */
	    buflinelen -= remainder;	/* reduce actual line length */
	    exittype = (*s == FF ? FORMFEED : LINE);
					/* set future exit type */
	}
	/* adjust text buffer stuff */

	bufptr += buflinelen;		/* bump pointer */
	avail -= buflinelen;		/* reduce what's available */

	/* Step 3.  Prepare to copy remaining stuff to temporary buffer */

	if ((bufleft[curinp] = remainder) != 0)
	{
	    bufcur[curinp] = bufbeg[curinp]; /* reset temp. buffer ptr */
	    (void) memcpy(bufbeg[curinp],bufptr,remainder);
	}

#ifdef DEBUG
	if (avail < 0)
	    interr("avail went negative in fillup");
#endif
    }	/* end while */

    if (exittype == FORMFEED)		/* bump pointer back */
	bufptr--;

    TBhave(Q_text,start,bufptr-start);	/* declare everything we've added */
    return(exittype);
}
/* Writebuf -- write characters from text buffer to current output
 *
 * This routine writes characters from the text buffer to the current
 * output stream after doing the appropriate checks.
 */

void
Writebuf(first,len,ffflag)
int first;				/* first character number to write */
int len;				/* number of characters to write */
BOOL ffflag;				/* TRUE to write FF after text */
{
    STREAM * s = &outputs[curout];	/* current output stream */
    FILE * f = s->file;			/* FILE structure for that stream */
    char c;				/* next char to write */
    char * bufptr;			/* pointer to buffer to write */
    void writec();


/* Error if we will actually write something and no file is open */

    if (   (s->flags & S_OPEN) == 0
	&& ( len > 0 || ffflag)
	)
	terrNUL(&Err_NFO);
    
    bufptr = TBtext(Q_text) + first;	/* point at first character */

    while (len-- > 0)
    {
	c = *bufptr++;			/* next character */

/* check for CR LF and its conversion to NL */

	if(	c == CR &&		/* see CR */
		len > 0 &&		/* more chars to write */
		*bufptr == LF &&	/* next char is LF */
		(s->flags & S_IMAGE) == 0	/* image mode is off */
	    )
	{
	    len--;			/* discard character */
	    bufptr++;			/* skip LF */
	    c = '\n';			/* change char to NL */
	}

	writec(c,f);			/* write the character */
    }

    if (ffflag)				/* check for FF */
	writec(FF,f);			/* write form feed if requested */ 
    return;
}
/* writec -- service routine for Writebuf */

static void
writec(c,f)
char c;					/* character to write */
FILE * f;				/* FILE to write to */
{
    if (Fwritec(c,f))			/* flag error */
	terrNUL(&Err_OUT);
    return;
}
/* Doyank -- do essential parts of Y command
 *
 * This routine does the "yank test", then fills the text buffer.  The
 * "yank test" may be by-passed if the argument flag is FALSE
 */

extern TNUMB ED;			/* ED flag */

TNUMB					/* 0 if at eof, else -1 */
Doyank(flag)
BOOL flag;				/* TRUE to do yank test */
{
    if (	flag &&			/* test requested */
		TBsize(Q_text) != 0 &&	/* buffer non-empty */
		(outputs[curout].flags & S_OPEN) != 0 && /* output file open */
		(ED & ED_yank) == 0	/* yank protection "on" */
	)
	terrNUL(&Err_YCA);		/* yank aborted */
    
    Dot = 0;				/* buffer pointer will be 0 when
					 * we're done
					 */
    TBkill(Q_text);			/* delete text buffer */
    TBreorg();				/* reorganize text space */
    return(Readlines(-1,FALSE));	/* read whole buffer, return value;
					** don't force expansion
					*/
}
/* killei -- service routine to kill EI file */

void
Killei()
{
    if (EI)				/* turn off if now on */
        (void) Fclose(eifile);

    EI = FALSE;				/* mark as off */
    return;
}
/* EI command -- set command file */

short
CMeicmd()
{
    int first;				/* first position of file name */
    int len;				/* length of file name */

    void interr();
    void Killini();
    BOOL eiopen();			/* routine below for Filepath */

    Finddlm(ESC,&first,&len);		/* delimit file name */

    if (Skiptype != SKNONE)		/* exit if skipping */
	return(SKIPCONT);
    
    if (Fl_colon == 0)			/* if no : flag set */
    {	Eat_val(); }			/* ... in case of macro */

    Killei();				/* kill EI file */

    Buildstring(Q_file,first,len,FALSE); /* build filename string in Q*,
					 ** translate quoted chars
					 */

    if (TBsize(Q_file) >= NAMELEN)	/* make sure name not too long */
	terrTB(&Err_PTL,Q_file);
    else if (TBsize(Q_file) == 0)	/* can exit if length is 0 */
    {					/* EI$ command */
	Killini();			/* kill initialization commands */
	Eat_flags();			/* kill flags */
	return(XECCONT);
    }
/* Follow path variable, try to open file */

    if (! Filepath (
			TBtext(Q_file),	/* "base name" for file */
			TBsize(Q_file),	/* its length */
			EIVAR,		/* EI path variable name */
			EIDFL,		/* default path variable value */
			einame,		/* place to store complete path */
			NAMELEN,	/* available length */
			eiopen		/* routine to call */
		    )
	)
    {
	filerr(&Err_FER);		/* report error as last "errno"
					** from eiopen
					*/
	return(XECCONT);		/* continue executing on : flag */
    }
    
    EI = TRUE;				/* signal that we have an EI file */
    EInl = FALSE;			/* new line not pending */

    if (Fl_colon != 0)			/* return value if requested */
	Set1val((TNUMB) -1);		/* success */

    Eat_flags();			/* discard current flags */

    return(XECCONT);
}
/* eiopen -- open EI file
**
** This routine gets called by way of Filepath to open an EI file.
** It returns TRUE on success.
** Note that 'errno' will be set on failure.  The EI command processor
** depends on the last such value for its error message, if one is needed.
*/

static BOOL
eiopen(filename)
char * filename;			/* passed in by Filepath */
{
    return( (eifile = Fopread(filename)) != NULL);
}
/* rEIch -- read EI character
**
** This routine reads characters from the EI command file.  It returns
** each character, or -1 for end of file.
*/

int
rEIch()
{
    int c;				/* temporary character (as int) */
    int Freadc();

    if (! EI )				/* if no file open, return EOF */
	return(EOF);

    if (EInl)				/* new line pending? */
    {
	EInl = FALSE;			/* yes.  Disable and return LF */
	return(LF);
    }

    if ((c = Freadc(eifile)) == EOF)	/* check for EOF, turn off file */
    {
        Killei();
	if (Ferror(eifile))		/* report error if real */
	    terrNUL(&Err_INP);
    }

    if (c == '\n')			/* convert new line to CR */
    {
	EInl = TRUE;			/* mark new line as pending */
	c = CR;
    }

    return(c);
}
/* EN command
**
** Elaborate a bit by allowing strings created by Buildstring
*/

short
CMencmd()
{
    extern void Setwild();
    extern void Nextwild();

    int start;				/* start position of specifier
					** string
					*/
    int len;				/* length of string */


    Finddlm(ESC,&start,&len);		/* delimit the specifier string */

    if (Skiptype != SKNONE)		/* continue skipping if so doing */
	return(SKIPCONT);
    
    Buildstring(Q_entemp,start,len,FALSE); /* expand special characters,
					   ** translate quoted chars
					   */

    if (TBsize(Q_entemp) > 0)		/* start new string if non-0 length */
    {
	Setwild(TBtext(Q_entemp),TBsize(Q_entemp));
	Eat_val();			/* kill any current values */
    }
    else				/* otherwise, get next filename */
    {
	Nextwild();			/* get next into Q_file */

	if (Fl_colon != 0)		/* value requested? */
	    Set1val((TNUMB) (TBsize(Q_file) != 0 ? -1 : 0));
					/* -1 if value present, else 0 */
	else				/* otherwise, kill off values */
	{   Eat_val(); }
    }

    TBkill(Q_entemp);			/* clear out temporary text block */
    Eat_flags();			/* kill off flags */
    return(XECCONT);			/* continue executing */
}
/* Fileexit -- clean up file I/O
**
** This routine closes all lingering open files.
*/

void
Fileexit()
{
    STREAM * strm;			/* temp. stream pointer */

    Killei();				/* close any EI file */

/* Close any open input files.  Assume any NULL file descriptors are
** the result of faked close in EX command if the file is marked
** at end of file.
*/

    strm = &inputs[PRIMARY];		/* point at primary input */
    if ( (strm->flags & S_OPEN) != 0)
    {
	if (! ((strm->flags & S_EOF) != 0 && strm->file == NULL))
	    (void) Fclose(strm->file);
	strm->flags = 0;		/* clean up flags */
    }

    strm = &inputs[SECONDARY];		/* point at secondary input */
    if ( (strm->flags & S_OPEN) != 0)
    {
	if (! ((strm->flags & S_EOF) != 0 && strm->file == NULL))
	    (void) Fclose(strm->file);
	strm->flags = 0;		/* clean up flags */
    }


/* close open output files.  We have to set "curout" to make use
** of eksub.
*/

    if ( (outputs[curout = PRIMARY].flags & S_OPEN) != 0)
	eksub();

    if ( (outputs[curout = SECONDARY].flags & S_OPEN) != 0)
	eksub();
    
    return;
}
