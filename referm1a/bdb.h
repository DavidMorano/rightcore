/* bdb */



#include	<sys/types.h>

#include	<bfile.h>
#include	<hdb.h>



struct bdb_flags {
	uint	open : 1 ;
} ;

struct bdb_value {
	struct bdb_flags	f ;
	bfile			infile, outfile, errfile ;
} ;

struct bdb {
	HDB_HEAD	lh ;
} ;

struct bdb_cur {
	HDB_CUR	c ;
} ;



