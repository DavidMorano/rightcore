/* filewatch */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	FILEWATCH_INCLUDE
#define	FILEWATCH_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"strfilter.h"


/* object defines */
#define	FILEWATCH		struct filewatch_head
#define	FILEWATCH_ARGS		struct filewatch_args
#define	FILEWATCH_FL		struct filewatch_flags


/* options */
#define	FILEWATCH_MCARRIAGE	0x0001
#define	FILEWATCH_MCLEAN	0x0002


/* constants */

#ifdef	LINE_MAX
#define	FILEWATCH_BUFLEN	MAX(LINE_MAX,4096)
#else
#define	FILEWATCH_BUFLEN	4096
#endif


struct filewatch_args {
	int		interval ;
	int		cut ;
	int		columns ;
	int		indent ;
	int		opts ;
} ;

struct filewatch_flags {
	uint		open:1 ;
	uint		carriage:1 ;
	uint		clean:1 ;
} ;

struct filewatch_head {
	const char	*fname ;
	char		*lp ;
	char		*buf ;
	STRFILTER	*sfp ;
	FILEWATCH_FL	f ;
	bfile		wfile ;
	time_t		lastcheck ;
	time_t		lastchange ;
	time_t		opentime ;
	uino_t		ino ;
	dev_t		dev ;
	uint		offset ;
	int		interval ;
	int		cut ;
	int		columns ;
	int		indent ;
	int		opts ;
	int		bi ;
	int		ll ;
} ;


#if	(! defined(FILEWATCH_MASTER)) || (FILEWATCH_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int filewatch_start(FILEWATCH *,FILEWATCH_ARGS *,STRFILTER *,cchar *) ;
extern int filewatch_check(FILEWATCH *,time_t,bfile *) ;
extern int filewatch_readline(FILEWATCH *,time_t,char *,int) ;
extern int filewatch_finish(FILEWATCH *) ;

#ifdef	__cplusplus
}
#endif

#endif /* FILEWATCH_MASTER */


#endif /* FILEWATCH_INCLUDE */



