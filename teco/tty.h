/*	static char h_tty[] = "@(#) tty.h:  6.1 8/25/83";	*/
/* tty.h -- declare terminal functions
 *		must be preceded by stdio.h
 */

extern short Charkill;			/* character erase char */
extern short Linekill;			/* line erase char */

extern FILE * Ttyin;			/* terminal input */
extern FILE * Ttyout;			/* terminal output */
extern FILE * Cmdout;			/* command output */

extern void Ttinit();			/* initialize terminal */
extern void Ttenable();			/* enable changes to terminal modes */
extern void Ttscmd();			/* initialize for command reading */
extern void Ttsxec();			/* set terminal for command execution */
extern void Ttsxecnd();			/* set ... for ^T no wait */
extern void Ttreset();			/* reset terminal to initial state */
extern void Ttsave();			/* save current tty status */
extern void Ttrestore();		/* restore saved tty status */
extern void Testinterrupt();		/* test for interrupts */
extern int Ttynum();			/* return user's terminal number */
extern short Ttspeed();			/* return terminal speed */
