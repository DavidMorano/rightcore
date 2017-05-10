/* bbnewsrc */


#ifndef	BBNEWSRC_INCLUDE
#define	BBNEWSRC_INCLUDE		1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<bfile.h>
#include	<dater.h>
#include	<localmisc.h>


#define	BBNEWSRC	struct bbnewsrc_head
#define	BBNEWSRC_ENT	struct bbnewsrc_ent
#define	BBNEWSRC_FL	struct bbnewsrc_flags


struct bbnewsrc_flags {
	uint		readtime:1 ;
	uint		wroteheader:1 ;
	uint		fileopen:1 ;
} ;

struct bbnewsrc_head {
	uint		magic ;
	BBNEWSRC_FL	f ;
	bfile		nf ;
	DATER		tmpdate ;
	const char	*ufname ;
	int		line ;
} ;

struct bbnewsrc_ent {
	time_t		mtime ;
	uint		f_subscribed:1 ;
	int		ln ;		/* line-number */
	char		ngname[MAXPATHLEN+1] ;
} ;


#if	(! defined(BBNEWSRC_MASTER)) || (BBNEWSRC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bbnewsrc_open(BBNEWSRC *,const char *,int) ;
extern int	bbnewsrc_close(BBNEWSRC *) ;
extern int	bbnewsrc_rewind(BBNEWSRC *) ;
extern int	bbnewsrc_read(BBNEWSRC *,BBNEWSRC_ENT *) ;
extern int	bbnewsrc_write(BBNEWSRC *,const char *,int,time_t) ;

#ifdef	__cplusplus
}
#endif

#endif /* BBNEWSRC_MASTER */

#endif /* BBNEWSRC_INCLUDE */


