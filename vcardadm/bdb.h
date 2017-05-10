/* bdb */


#ifndef	BDB_INCLUDE
#define	BDB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vecobj.h>
#include	<hdb.h>

#include	"localmisc.h"



#define	BDB		struct bdb_head

#define	BDB_CUR	struct bdb_cur
#define	BDB_VALUE	struct bdb_value
#define	BDB_ENT	struct bdb_ent

#define	BDB_CITEKEYLEN	100
#define	BDB_BUFLEN	(3 * MAXPATHLEN)

/* operational options */
#define	BDB_OUNIQ	0x01		/* queries must be unique */



struct bdb_cur {
	HDB_CUR	cur ;
} ;

/* we return this to the caller upon key-query */
struct bdb_ent {
	char	*(*keyvals)[2] ;
	char	*query ;
	int	nkeys ;
	int	size ;
	int	fi ;			/* file index */
} ;

struct bdb_key {
	char		*citekey ;
	offset_t		offset ;
	int		len ;
	int		fi ;		/* file index */
} ;

struct bdb_file {
	char		*fname ;
	int		f_indexed ;
} ;

struct bdb_head {
	ulong		magic ;
	char		*qkey ;		/* query key name (usually 'Q') */
	VECOBJ		files ;
	HDB		keys ;
	uint		opts ;
	int		unindexed ;	/* number of unindexed files */
	int		qkeylen ;
} ;



#if	(! defined(BDB_MASTER)) || (BDB_MASTER == 0)

extern int	bdb_init(BDB *,const char *,int) ;
extern int	bdb_free(BDB *) ;
extern int	bdb_add(BDB *,const char *) ;
extern int	bdb_count(BDB *) ;
extern int	bdb_query(BDB *,const char *,BDB_ENT *,char *,int) ;
extern int	bdb_curbegin(BDB *,BDB_CUR *) ;
extern int	bdb_curend(BDB *,BDB_CUR *) ;
extern int	bdb_getname(BDB *,BDB_CUR *,char *) ;
extern int	bdb_delcur(BDB *,BDB_CUR *,f_adv) ;

#endif /* BDB_MASTER */


#endif /* BDB_INCLUDE */



