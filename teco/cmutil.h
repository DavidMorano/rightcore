/*	static char h_cmutil[] = "@(#) cmutil.h:  4.2 12/12/82";	*/
/* cmutil.h -- declare command utility functions */

extern void Postest();			/* test position */
extern void Confine();			/* handle 0/1/2 arg commands,
					 * confine result between 0 and Z
					 */
extern void Buildstring();		/* build search string */
extern void Addc();			/* append character to text block */
extern void Addst();			/* append string to text block */
