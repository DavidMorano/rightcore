/* job */



#ifndef	JOB_INCLUDE
#define	JOB_INCLUDE	1



#include	<sys/types.h>

#include	<vecelem.h>

#include	"misc.h"
#include	"config.h"
#include	"defs.h"
#include	"srvtab.h"


#define	JOB		VECELEM




extern int job_start(VECELEM *,struct jobentry *,
		struct proginfo *,struct ustat *,SRVTAB *) ;
extern int job_findpid(VECELEM *,struct proginfo *,int,struct jobentry **) ;
extern int job_search(VECELEM *,struct proginfo *,char *,struct jobentry **) ;
extern int job_get(VECELEM *,struct proginfo *,int,struct jobentry **) ;
extern int job_end(VECELEM *, struct jobentry *, struct proginfo *, int) ;


#endif /* JOB_INCLUDE */



