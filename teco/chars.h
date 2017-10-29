/*	static char h_chars[] = "@(#) chars.h:  4.2 12/12/82";	*/
/* define character names */

#define CTRLA	('A' & 077)	/* ctrl A */
#define CTRLC	('C' & 077)	/* ctrl C */
#define CTRLE	('E' & 077)	/* ctrl E */
#define CTRLN	('N' & 077)	/* ctrl N */
#define CTRLO	('O' & 077)	/* ctrl O */
#define	CTRLQ	('Q' & 077)	/* ctrl Q */
#define	CTRLR	('R' & 077)	/* ctrl R */
#define	CTRLS	('S' & 077)	/* ctrl S */
#define	CTRLV	('V' & 077)	/* ctrl V */
#define	CTRLW	('W' & 077)	/* ctrl W */
#define	CTRLX	('X' & 077)	/* ctrl X */

#define NUL	00		/* null */
#define BEL	07		/* ctrl G */
#define BS	010		/* ctrl H */
#define TAB	011		/* ctrl I */
#define LF	012		/* ctrl J */
#define VT	013		/* ctrl K */
#define FF	014		/* ctrl L */
#define CR	015		/* ctrl M */
#define ESC	033		/* ESCape */

#define SPACE	040		/* space */

/* old style ALT MODE codes */

#define ALTMOD1 0175	/* first one */
#define ALTMOD2 0176	/* second one */
