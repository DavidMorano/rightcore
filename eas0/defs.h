/* defs */




/* miscellaneous */

#define	BUFLEN		200
#define	MAXARG_SUBCKTS	200



/* data structures used throughout */


/* circuit type descriptor */

struct type {
	struct type	*next ;
	char		*type ;
} ;


/* subcircuit data structures */

struct block {
	struct block	*next ;
	offset_t		start ;
	offset_t		len ;
} ;


struct circuit {
	struct circuit	*next ;
	struct block	*bp ;
	int		type ;
	int		sl ;
	int		lines ;
	char		*name ;
	char		nlen ;
} ;



/* global data structures */

struct flags {
	uint	debug : 1 ;
	uint	verbose : 1 ;
	uint	separate : 1 ;
} ;


struct global {
	bfile		*efp ;
	bfile		*ofp ;
	bfile		*ifp ;
	struct flags	f ;
	struct circuit	*top ;
	struct circuit	*bottom ;
	int	debuglevel ;
	char	*progname ;
	char	*buf ;
	char	*suffix ;
} ;



