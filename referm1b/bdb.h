/* bdb */



#ifndef	BDB_INCLUDE
#define	BDB_INCLUDE	1



#include	<sys/types.h>

#include	<bfile.h>
#include	<hdb.h>



#define	BDB			struct bdb_head

#define	BDB_HEAD		strucy bdb_head
#define	BDB_CUR			HDB_CUR
#define	BDB_CUR		HDB_CUR
#define	BDB_VALUE		struct bdb_value


#define	BDB_MAXOPEN	12		/* maximum databases */
#define	BDB_BUFLEN	(MAXPATHLEN - 1)



struct bdb_flags {
	uint	open : 1 ;
} ;

struct bdb_value {
	struct bdb_flags	f ;
	bfile			infile, outfile, errfile ;
} ;

struct bdb_head {
	long	magic ;
	HDB	lh ;
} ;




#ifndef	BDB_MASTER

extern int	bdbinit(BDB *) ;
extern int	bdbfree(BDB *) ;
extern int	bdbcount(BDB *) ;
extern int	bdbgetkey(BDB *,BDB_CUR *,char **) ;
extern int	bdbdelcursor(BDB *,BDB_CUR *) ;

#endif /* BDB_MASTER */


#endif /* BDB_INCLUDE */



