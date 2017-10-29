/*	static char h_exittype[] = "@(#) exittype.h:  5.1 2/22/83";	*/
/* exittype.h -- define exit codes for Xec, Skip, and longjmp */

#define XECDONE		(-1)	/* done in Xec */

/* These next codes are deliberately chosen to be an easily testable
 * value.  Furthermore they must have the same value.
 */

#define XECCONT		0	/* continue in Xec */
#define SKIPCONT	0	/* continue in Skip */
#define CONTINUE	0	/* generic continue in both */

#define SKIPDONE	1	/* done skipping.  Exit Skip */
#define SKIPRANOFF	2	/* ran off the end of current text block
				 * while skipping.
				 */

/* These codes are for longjmp's to Reset.  They must be non-zero, since
** the call to setjmp returns 0.
*/

#define TECEXIT		1	/* exit from TECO */
#define ERROREXIT	2	/* exit from error */
#define CTRLCEXIT	3	/* exit due to ^C in command string */

/* These values are UNIX exit codes. */

#define	UEXITOK		0	/* success exit */
#define	UEXITSU		1	/* error in start-up macro (value explicit
				** in teco.tec.c
				*/
#define	UEXITMUNG	2	/* error during MUNGed macro */
