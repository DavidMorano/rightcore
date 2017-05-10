/* configfile */



#include	<vecstr.h>



struct confighead {
	vecstr	exports ;		/* environment variables */
	vecstr	defaults ;		/* parameters */
	vecstr	machines ;		/* machines */
	int		badline ;		/* line number of bad thing */
	int		srs ;			/* secondary return status */
	char		*advice ;		/* ADVICE program */
	char		*runadvice_advice ;	/* environment variable */
	char		*tmpdir ;		/* environment variable */
	char		*control ;		/* ADVICE file */
	char		*main ;			/* ADVICE file */
	char		*params ;		/* ADVICE file */
} ;


/* returns */

#define	CFR_OK		0
#define	CFR_BADOPEN	-1001
#define	CFR_BADALLOC	-1002
#define	CFR_BADPROGRAM	-1003
#define	CFR_BADREAD	-1004
#define	CFR_BADCONFIG	-1005
#define	CFR_BADMACHINES	-1006
#define	CFR_BADDEFAULTS	-1007
#define	CFR_BADEXPORTS	-1008





