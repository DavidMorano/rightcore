/* calday */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	CALDAY_INCLUDE
#define	CALDAY_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>

#include	"caldays.h"


#define	CALDAY_MAGIC	0x99447246
#define	CALDAY		struct calday_head
#define	CALDAY_QUERY	struct calday_q
#define	CALDAY_CITE	struct calday_q
#define	CALDAY_CUR	struct calday_c


struct calday_q {
	ushort	y ;
	uchar	m, d ;
} ;

struct calday_c {
	uint	magic ;
	void	*scp ;
} ;

struct calday_calls {
	int	(*open)(void *,const char *,const char **,const char **) ;
	int	(*count)(void *) ;
	int	(*curbegin)(void *,CALDAYS_CUR *) ;
	int	(*lookcite)(void *,CALDAYS_CUR *,CALDAYS_QUERY *) ;
	int	(*read)(void *,CALDAYS_CUR *,
			CALDAYS_CITE *,char *,int) ;
	int	(*curend)(void *,CALDAYS_CUR *) ;
	int	(*check)(void *,time_t) ;
	int	(*audit)(void *) ;
	int	(*close)(void *) ;
} ;

struct calday_head {
	uint	magic ;
	void	*sop ;			/* shared-object (SO) pointer */
	void	*obj ;			/* object pointer */
	int	objsize ;		/* object size */
	int	cursize ;		/* cursor size */
	struct calday_calls	call ;
} ;


#if	(! defined(CALDAY_MASTER)) || (CALDAY_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int calday_open(CALDAY *,const char *,
			const char **,const char **) ;
extern int calday_count(CALDAY *) ;
extern int calday_curbegin(CALDAY *,CALDAY_CUR *) ;
extern int calday_lookcite(CALDAY *,CALDAY_CUR *,CALDAY_QUERY *) ;
extern int calday_read(CALDAY *,CALDAY_CUR *,
			CALDAY_CITE *,char *,int) ;
extern int calday_curend(CALDAY *,CALDAY_CUR *) ;
extern int calday_check(CALDAY *,time_t) ;
extern int calday_audit(CALDAY *) ;
extern int calday_close(CALDAY *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CALDAY_MASTER */

#endif /* CALDAY_INCLUDE */


