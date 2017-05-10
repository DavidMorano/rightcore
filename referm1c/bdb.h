/* bdb */



#ifndef	BDB_INCLUDE
#define	BDB_INCLUDE	1



#include	<sys/types.h>

#include	<bfile.h>
#include	<hdb.h>



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
	int		magic ;
	HDB_HEAD	lh ;
} ;



typedef struct bdb_head	bdb ;



#define	BDB			struct bdb_head

#define	BDB_HEAD		struct bdb_head
#define	BDB_CUR			struct hdb_cur
#define	BDB_CURSOR		struct hdb_cur
#define	BDB_VALUE		struct hdb_value




#ifndef	BDB_MASTER

extern int	bdb_init(BDB *) ;
extern int	bdb_free(BDB *) ;
extern int	bdb_count(BDB *) ;
extern int	bdb_getname(BDB *,BDB_CUR *,char **) ;
extern int	bdb_delcur(BDB *,BDB_CUR *) ;
extern int	bdb_query(BDB *,char *,VECELEM *)
extern int	bdb_nullcursor(BDB *,BDB_CURSOR *) ;

#endif /* BDB_MASTER */


#endif /* BDB_INCLUDE */



