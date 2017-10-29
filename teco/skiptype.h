/*	static char h_skiptype[] = "@(#) skiptype.h:  4.2 12/12/82";	*/
/* skiptype.h -- define skipping types */

#define SKNONE	0	/* not skipping at all */
#define SKITER	1	/* skipping to end of < > loop */
#define SKGOTO	2	/* skipping, looking for label because of goto */
#define SKCOND	3	/* skipping to next part of conditional, either
			 * | or '
			 */
#define SKFI	4	/* skipping to end of conditional:  ' */
