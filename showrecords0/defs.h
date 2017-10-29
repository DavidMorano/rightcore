/* config */


#define		BLOCKSIZE	512	/* ALWAYS -- UNIX standard */


struct flags {
	uint	debug : 1 ;
} ;

struct global {
	struct flags	f ;
	char		*progname ;
} ;



