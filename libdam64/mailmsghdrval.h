/* mailmsghdrval */


/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILMSGHDRVAL_INCLUDE
#define	MAILMSGHDRVAL_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>

#include	<localmisc.h>


#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN		MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN		2048
#endif
#endif


/* object defines */

#define	MAILMSGHDRVAL		struct mailmsghdrval_head
#define	MAILMSGHDRVAL_BUFLEN	LINEBUFLEN


struct mailmsghdrval_head {
	char		vbuf[MAILMSGHDRVAL_BUFLEN + 1] ;
	char		*v ;
	char		vlen ;
	int		i ;
} ;


#if	(! defined(MAILMSGHDRVAL_MASTER)) || (MAILMSGHDRVAL_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailmsghdrval_start(MAILMSGHDRVAL *,int,const char *,int) ;
extern int mailmsghdrval_add(MAILMSGHDRVAL *,const char *,int) ;
extern int mailmsghdrval_get(MAILMSGHDRVAL *,const char **,int *) ;
extern int mailmsghdrval_finish(MAILMSGHDRVAL *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILMSGHDRVAL_MASTER */

#endif /* MAILMSGHDRVAL_INCLUDE */


