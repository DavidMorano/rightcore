/* dictfiles */


#ifndef	DICTFILES_INCLUDE
#define	DICTFILES_INCLUDE	1


#include	<sys/types.h>

#include	<bfile.h>
#include	<localmisc.h>


/* object defines */

#define	DICTFILES		struct dictfiles_head
#define	DICTFILES_NLETTERS	256


struct dictfiles_ent {
	ulong		len ;
	ulong		usage ;
	int		fi ;
	int		f_open:1 ;
	int		f_trunced:1 ;
} ;

struct dictfiles_file {
	bfile		df ;
	uint		c ;		/* parent leading letter */
	uint		f_open:1 ;
} ;

struct dictfiles_head {
	ulong		magic ;
	struct dictfiles_ent	e[DICTFILES_NLETTERS] ;
	struct dictfiles_file	*files ;
	char		*dictdname ;
	char		*prefix ;
	int		maxopen ;
	int		nopen ;
	int		usage ;		/* counter */
} ;


#if	(! defined(DICTFILES_MASTER)) || (DICTFILES_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int dictfiles_open(DICTFILES *,int,const char *,const char *) ;
extern int dictfiles_write(DICTFILES *,const char *,int) ;
extern int dictfiles_close(DICTFILES *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DICTFILES_MASTER */


#endif /* DICTFILES_INCLUDE */



