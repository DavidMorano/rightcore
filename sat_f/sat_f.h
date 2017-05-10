/* freeze file entry */


#define		NFILTERS	50
#define		FBUFLEN		4096

#define		FZPATH		"/tmp"
#define		FZPREFIX	"fz"
#define		MAXTMP		100

#define		FZMAGIC		0x01234567
#define		PATHLEN		150
#define		LINELEN		150

#define		MAXLINES	50


struct fzentry {
	int	klen, slen ;
	char	*kp, *sp ;
} ;


/* internally used defines */

#define PR_OK		0		/* general OK return */
#define PR_BAD		-1		/* general bad return */

#define	GC_BADPATH	-1001		/* no full path of filter file */
#define	GC_TOOMANY	-1002		/* too many filter entries */
#define	GC_TOOMUCH	-1003		/* ran out of string space */
#define	GC_MAXTMP	-1004		/* maximum temporary file attempts */

#define	PR_UNKNOWN	-1005		/* unknown key string encountered */
#define	PR_BADFILTER	-1006		/* bad filter file */
#define	PR_BADNOKEY	-1007		/* bad key macro encountered */


