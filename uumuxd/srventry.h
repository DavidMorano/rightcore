/* SRVENTRY */

/* expanded server entry */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	SRVENTRY_INCLUDE
#define	SRVENTRY_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<varsub.h>
#include	<srvtab.h>


/* local object defines */

#define	SRVENTRY	struct srventry_head
#define	SRVENTRY_ARGS	struct srventry_a


struct srventry_a {
	const char	*version ;	/* %V */
	const char	*searchname ;	/* %S */
	const char	*programroot ;	/* %R */
	const char	*nodename ;	/* %N */
	const char	*domainname ;	/* %D */
	const char	*hostname ;	/* %H */
	const char	*username ;	/* %U */
	const char	*service ;
	const char	*subservice ;
	const char	*svcargs ;	/* service arguments! */
	const char	*peername ;
	const char	*ident ;	/* IDENT name if available */
	const char	*nethost ;	/* reverse lookup (what is this ??) */
	const char	*netuser ;	/* network username if available */
	const char	*netpass ;	/* network password (encrypted ?) */
} ;

struct srventry_head {
	const char	*program ;	/* server program path */
	const char	*srvargs ;	/* server program arguments */
	const char	*username ;
	const char	*groupname ;
	const char	*options ;
	const char	*access ;
} ;


#if	(! defined(SRVENTRY_MASTER)) || (SRVENTRY_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int srventry_start(SRVENTRY *) ;
extern int srventry_finish(SRVENTRY *) ;
extern int srventry_process(SRVENTRY *,varsub *,char **,
			SRVTAB_ENTRY *,SRVENTRY_ARGS *) ;
extern int srventry_addprogram(SRVENTRY *,char *) ;
extern int srventry_addsrvargs(SRVENTRY *,char *) ;
extern int srventry_addusername(SRVENTRY *,char *) ;
extern int srventry_addgroupname(SRVENTRY *,char *) ;
extern int srventry_addoptions(SRVENTRY *,char *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SRVENTRY_MASTER */

#endif /* SRVENTRY_INCLUDE */


