/* pcsruns */

/* PCS real-username name-server */


#ifndef	PCSRUNS_INCLUDE
#define	PCSRUNS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<vechand.h>
#include	<strpack.h>


#define	PCSRUNS		struct pcsruns_head


struct pcsruns_head {
	vechand		env ;
	strpack		stores ;
} ;


#if	(! defined(PCSRUNS_MASTER)) || (PCSRUNS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pcsruns_start(PCSRUNS *,const char *) ;
extern int pcsruns_lookup(PCSRUNS *,const char *,int,const char **) ;
extern int pcsruns_envset(PCSRUNS *,const char *,const char *,int) ;
extern int pcsruns_sort(PCSRUNS *) ;
extern int pcsruns_getvec(PCSRUNS *,const char ***) ;
extern int pcsruns_finish(PCSRUNS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(PCSRUNS_MASTER)) || (PCSRUNS_MASTER == 0) */

#endif /* PCSRUNS_INCLUDE */


