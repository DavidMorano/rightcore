/* job */


#ifndef	JOB_INCLUDE
#define	JOB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vecitem.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"srvtab.h"



#define	JOB		VECITEM




#if	(! defined(JOB_MASTER)) || (JOB_MASTER == 0)

extern int job_init(JOB *) ;
extern int job_add(JOB *,struct jobentry *) ;
extern int job_del(JOB *,int) ;
extern int job_start(JOB *,struct jobentry *,
		struct proginfo *,struct ustat *,SRVTAB *) ;
extern int job_findpid(JOB *,struct proginfo *,int,struct jobentry **) ;
extern int job_search(JOB *,struct proginfo *,char *,struct jobentry **) ;
extern int job_get(JOB *,struct proginfo *,int,struct jobentry **) ;
extern int job_end(JOB *, struct jobentry *, struct proginfo *, int) ;
extern int job_free(JOB *) ;

#endif /* JOB_MASTER */


#endif /* JOB_INCLUDE */



