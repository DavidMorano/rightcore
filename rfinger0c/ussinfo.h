/* ussinfo */


#ifndef	USSINFO_INCLUDE
#define	USSINFO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<ids.h>
#include	<vecstr.h>
#include	<expcook.h>
#include	<userinfo.h>
#include	<keyopt.h>
#include	<localmisc.h>

#include	"sysdialer.h"


#define	USSINFO		struct ussinfo_head
#define	USSINFO_FL	struct ussinfo_flags
#define	USSINFO_A	struct ussinfo_allocs


struct ussinfo_flags {
	uint		stores:1 ;
	uint		progdash:1 ;
	uint		ids:1 ;
	uint		userinfo:1 ;
	uint		ignore:1 ;
	uint		log:1 ;
} ;

struct ussinfo_allocs {
	const char	*node ;
	const char	*svc ;
	const char	*pr ;
	const char	*portspec ;
} ;

struct ussinfo_head {
	const char	**argv ;
	const char	**envv ;
	const char	*pr ;
	const char	*prn ;
	const char	*searchname ;
	const char	*version ;
	const char	*afspec ;
	const char	*hostname ;
	const char	*portspec ;
	const char	*svcname ;
	const char	*pvfname ;
	const char	*dfname ;
	const char	*xfname ;
	const char	*efname ;
	const char	*architecture ;		/* machine architecture */
	const char	*umachine ;		/* UNAME machine name */
	const char	*usysname ;		/* UNAME OS system-name */
	const char	*urelease ;		/* UNAME OS release */
	const char	*uversion ;		/* UNAME OS version */
	const char	*hz ;			/* OS HZ */
	const char	*nodename ;		/* USERINFO */
	const char	*domainname ;		/* USERINFO */
	const char	*username ;		/* USERINFO */
	const char	*homedname ;		/* USERINFO */
	const char	*shell ;		/* USERINFO */
	const char	*organization ;		/* USERINFO */
	const char	*gecosname ; 		/* USERINFO */
	const char	*realname ;		/* USERINFO */
	const char	*name ;			/* USERINFO */
	const char	*tz ;			/* USERINFO */
	const char	*groupname ;
	const char	*tmpdname ;
	const char	*maildname ;
	const char	*hfname ;
	const char	*lfname ;
	const char	*paramfname ;
	const char	*logid ;
	const char	*defprog ;
	void		*op ;
	SYSDIALER_ARGS	*ap ;
	IDS		id ;
	VECSTR		aenvs ;
	VECSTR		stores ;
	USERINFO	u ;
	USSINFO_A	a ;
	USSINFO_FL	f, init, open ;
	uid_t		uid ;
	gid_t		gid ;
	int		argc ;
	int		argi ;
	int		ncpu ;
	int		af ;
	int		to ;
} ;


#if	(! defined(USSINFO_MASTER)) || (USSINFO_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ussinfo_start(USSINFO *,void *, SYSDIALER_INFO *,
			SYSDIALER_ARGS *,
			const char *,const char *) ;
extern int	ussinfo_finish(USSINFO *) ;
extern int	ussinfo_procargs(USSINFO *) ;
extern int	ussinfo_procopts(USSINFO *,KEYOPT *) ;
extern int	ussinfo_defaults(USSINFO *) ;
extern int	ussinfo_addrparse(USSINFO *) ;
extern int	ussinfo_logfile(USSINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* USSINFO_MASTER */

#endif /* USSINFO_INCLUDE */


