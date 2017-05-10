/* fdb */


#ifndef	FDB_INCLUDE
#define	FDB_INCLUDE	1



#include	<sys/types.h>



#define	FDB_FILEMAGIC		"FDB"
#define	FDB_FILEMAGICLEN	3
#define	FDB_VERSION		0

#define	FDB_DEFESIZE		(4 * 8)
#define	FDB_DEFNHASH		32
#define	FDB_DEFTIMEOUT		30	/* seconds */




struct fdb_head {
	int	magic ;
	int	uflags ;
	int	fd ;
	int	timeout ;
	int	stamp ;
	int	mapsize ;
	char	*buf ;
} ;

struct fdb_c {
	int	i, j, k ;
} ;

struct fdb_datum {
	void	*buf ;
	int	len ;
} ;



typedef	struct fdb_head		fdb ;



#define	FDB			struct fdb_head
#define	FDB_DATUM		struct fdb_datum
#define	FDB_CUR		struct fdb_c



#ifndef	FDB_MASTER

extern int	fdb_open(FDB *,char *,int,int,int,int) ;
extern int	fdb_close(FDB *) ;
extern int	fdb_curbegin(FDB *,struct fdb_cur *) ;
extern int	fdb_curend(FDB *,struct fdb_cur *) ;
extern int	fdb_store(FDB *,FDB_DATUM,FDB_DATUM) ;
extern int	fdb_fetch(FDB *,FDB_DATUM,FDB_CUR *,FDB_DATUM *) ;
extern int	fdb_enum(FDB *,FDB_CUR *,FDB_DATAUM *,FDB_DATUM *) ;
extern int	fdb_delkey(FDB *,FDB_DATUM) ;
extern int	fdb_delcur(FDB *,FDB_CUR *) ;

#endif /* FDB_MASTER */


#endif /* FDB_INCLUDE */



