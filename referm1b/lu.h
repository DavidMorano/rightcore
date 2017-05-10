/* lu */



#ifndef	LU_INCLUDE
#define	LU_INCLUDE	1



#include	<sys/types.h>

#include	<bfile.h>
#include	<hdb.h>




struct lu_flags {
	uint	open : 1 ;
} ;

struct lu_value {
	struct lu_flags	f ;
	bfile			infile, outfile, errfile ;
} ;

struct lu_head {
	int		magic ;
	HDB_HEAD	lh ;
} ;



typedef struct lu_head	lu ;



#define	LU		struct lu_head

#define	LU_HEAD		struct lu_head
#define	LU_CUR		struct hdb_cur
#define	LU_CUR	struct hdb_cur
#define	LU_VALUE	struct hdb_value




#ifndef	LU_MASTER

extern int	bdbinit(LU *) ;
extern int	bdbfree(LU *) ;
extern int	bdbcount(LU *) ;
extern int	bdbgetkey(LU *,LU_CUR *,char **) ;
extern int	bdbdelcursor(LU *,LU_CUR *) ;

#endif /* LU_MASTER */


#endif /* LU_INCLUDE */



