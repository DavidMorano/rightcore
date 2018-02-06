/* comparse */

/* comment-parse (for RFC822) small strings (like for stupid RFC822 date) */


/* revision history:

	= 1998-02-22, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	COMPARSE_INCLUDE
#define	COMPARSE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<netdb.h>

#include	<localmisc.h>


#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif


/* object defines */

#define	COMPARSE_MAGIC		0x77811276
#define	COMPARSE		struct comparse_head
#define	COMPARSE_STR		struct comparse_str
#define	COMPARSE_LVALUE		MAILADDRLEN
#define	COMPARSE_LCOMMENT	LINEBUFLEN


struct comparse_str {
	const char	*sp ;
	int		sl ;
} ;

struct comparse_head {
	uint		magic ;
	COMPARSE_STR	val, com ;
} ;


#if	(! defined(COMPARSE_MASTER)) || (COMPARSE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int comparse_start(COMPARSE *,const char *,int) ;
extern int comparse_getval(COMPARSE *,const char **) ;
extern int comparse_getcom(COMPARSE *,const char **) ;
extern int comparse_finish(COMPARSE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* COMPARSE_MASTER */

#endif /* COMPARSE_INCLUDE */


