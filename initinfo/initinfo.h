/* initinfo */


#ifndef	INITINFO_INCLUDE
#define	INITINFO_INCLUDE	1


#include	<sys/types.h>

#include	<vecstr.h>

#include	"localmisc.h"



#define	INITINFO	struct initinfo_head
#define	INITINFO_CUR	struct initinfo_c




struct initinfo_c {
	int	i ;
} ;

struct initinfo_flags {
	uint	init : 1 ;
} ;

struct initinfo_head {
	unsigned long		magic ;
	const char		*pr ;
	VECSTR			entries ;
	struct initinfo_flags	f ;
	time_t			ti_check ;
	time_t			ti_pread ;
} ;



#if	(! defined(INITINFO_MASTER)) || (INITINFO_MASTER == 0)

extern int	initinfo_open(INITINFO *,const char *) ;
extern int	initinfo_curbegin(INITINFO *,INITINFO_CUR *) ;
extern int	initinfo_curend(INITINFO *,INITINFO_CUR *) ;
extern int	initinfo_query(INITINFO *,const char *,char *,int) ;
extern int	initinfo_enum(INITINFO *,INITINFO_CUR *,
			char *,int,char *,int) ;
extern int	initinfo_close(INITINFO *) ;

#endif /* INITINFO_MASTER */


#endif /* INITINFO_INCLUDE */



