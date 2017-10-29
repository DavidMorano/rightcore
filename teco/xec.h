/*	static char h_xec[] = "@(#) xec.h:  4.2 12/12/82";	*/
/* xec.h -- declare execution variables that are externally accessible */
/* must be preceded by bool.h */

extern int Dot;				/* current text position */
extern int Cmddot;			/* current command text block
					 * position
					 */
extern short Cmdtb;			/* current command text block */
extern short Cmdchar;			/* current command char */
extern short Xectype;			/* execution types: */

extern BOOL Fl_at;			/* @ flag */
extern short Fl_colon;			/* number of : */
extern BOOL Fl_trace;			/* trace flag */

extern int Inslen;			/* length of last insert */

extern short Radix;			/* current radix:  8, 10, 16 */

extern short Skiptype;			/* current skipping type */
/* define these helpful "functions" */

#define Eat_flags()	Fl_at=FALSE;Fl_colon=0	/* kill flags */
#define Eat_val()	Valinit()		/* kill values */

/* ... and these routines */

extern int pCMch();
extern int gCMch();
extern void Unterm();
extern void Xec();
extern void Xecimmed();
extern BOOL Skip();
extern void Pushx();
extern int Popx();
