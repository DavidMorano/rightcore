/* header file for the "Multiple Index Data Base (MIDB) package */

/* revision history:

	= 1998-05-01, David A­D­ Morano


*/



#define		MIDB_MAGIC	0x20052616
#define		MIDB_FILEMAGIC	"midb 0"
#define		MIDB_BUFSIZE	512



typedef struct midb_struct {
	struct midb_index	*skey_struct[] ;
	struct midb_index	*mkey_struct[] ;
	long	magic ;
	long	bufoffset ;
	offset_t	offset ;
	int	fd ;
	int	skey_len ;
	int	mkey_len ;
	int	(*skey_fun[])() ;
	int	(*mkey_fun[])() ;
	int	stat ;
	int	len ;
	int	oflag ;
	int	bufsize ;
	char	*buf ;
	char	*bp ;
} midb ;


typedef struct midb_query_struct {
	long	next ;			/* use the next record of this index */
	char	*key ;			/* use this key if record not given */
	int	keylen ;		/* key's length if not string */
	int	index ;			/* searching this index */
} ;


struct midb_index {
	int	fd ;
	int	f_open ;
} ;

struct midb_ent {
	long	offset_data ;
	long	offset_index ;
} ;


/* internal data status fields */

#define		MIDBSM_WRITE	0x01
#define		MIDBSM_NOTSEEK	0x02
#define		MIDBSM_LINEBUF	0x04
#define		MIDBSM_UNBUF	0x08


/* control commands */

#define		MIDBCMD_TELL	1
#define		MIDBCMD_BUF	2
#define		MIDBCMD_FULLBUF	2
#define		MIDBCMD_LINEBUF	3
#define		MIDBCMD_UNBUF	4


/* returns */

#define		MIDBR_NOTOPEN	-300
#define		MIDBR_OPEN	-300
#define		MIDBR_WRONLY	-301
#define		MIDBR_RDONLY	-302
#define		MIDBR_BADNAME	-303
#define		MIDBR_NOTSEEK	-304
#define		MIDBR_EOF	-305
#define		MIDBR_FAULT	-306



