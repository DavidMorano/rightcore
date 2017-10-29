/*	static char h_cmdio[] = "@(#) cmdio.h:  6.1 8/25/83";	*/
/* cmdio.h -- declare command i/o routines */

extern int rCch();		/* read command char */
extern int rTch();		/* read terminal character */
extern int whCch();		/* where is next command char coming from? */
extern void wCch();		/* write command char */
extern void wCch1();		/* write char, ignore cases, etc. */
extern void uCch();		/* unget command char */
extern void wTBst();		/* write text block string */
extern void Crlf();		/* write Carriage Return, Line Feed */
extern void Erasechar();	/* erase designated character */
extern void Eraseline();	/* erase current command line */
