/*	static char h_errors[] = "@(#) errors.h:  5.1 2/22/83";	*/
/* errors.h -- define error structures */

struct TER
{
    short type;				/* error type */
    char * first;			/* first part */
    char * second;			/* second part */
};

#define TERPTR struct TER *

/* define error types */

#define	ErrNUL	1	/* string by itself */
#define	ErrCHR	2	/* string with embedded character */
#define	ErrTB	3	/* string with embedded text block */
#define	ErrSTR	4	/* string with embedded string */
#define	ErrWRN	5	/* warning string */
#define	ErrSWRN	6	/* special warning string:  output warning, but do
			** full error processing
			*/
