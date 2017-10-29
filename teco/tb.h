/*	static char h_tb[] = "@(#) tb.h:  4.2 12/12/82";	*/
/* tb.h
 *
 *	Define parts of the text block package for callers.
 *	Must be preceded by 'bool.h'.
 */

    extern BOOL TBins();
    extern BOOL TBapp();
    extern void TBdel();
    extern void TBkill();
    extern BOOL TBrepl();
    extern char * TBneed();
    extern void TBhave();
    extern int TBsize();
    extern char * TBtext();
    extern void TBreorg();
    extern void TBrename();
    extern void TBstatic();
    extern void TBinit();
#ifdef DEBUG
    extern BOOL isTB();
#endif

/* Text block structure:  Defined here to permit macros for
** TBsize and TBtext.  NOT FOR EXTERNAL USE!
*/

struct Tblk {
    char * text;		/* pointer to actual text block text */
    int alloc;			/* number of bytes allocated */
    int used;			/* number of bytes actually used
				** (Note that the "position of the last
				** byte is used-1
				*/
};

#ifndef DEBUG			/* for production system.... */
extern struct Tblk TB[];	/* text block array */

#define	TBsize(tb)	(TB[tb].used)
#define TBtext(tb)	(TB[tb].text)

#endif	/* ndef DEBUG */
