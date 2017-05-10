/* recsvc */



#ifndef	RECSVC_INCLUDE
#define	RECSVC_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	"localmisc.h"



/* object defines */

#define	RECSVC			struct recsvc_head

#define	RECSVC_STARTNUM	100	/* starting number records */


/* branch types */

#define	RECSVC_BTFWD		1	/* forward */
#define	RECSVC_BTSSH		2	/* Simple Single-sided Hammock */



struct recsvc_ent {
	uint	canon ;		/* cannonical name */
	uint	name ;		/* service name */
	uint	svc ;		/* service code */
} ;

struct recsvc_head {
	unsigned long		magic ;
	struct recsvc_ent	*rectab ;
	int	i ;		/* current length */
	int	e ;		/* current buffer extent */
	int	c ;		/* count */
} ;



#if	(! defined(RECSVC_MASTER)) || (RECSVC_MASTER == 0)

extern int	recsvc_init(RECSVC *,int) ;
extern int	recsvc_free(RECSVC *) ;
extern int	recsvc_add(RECSVC *,uint,int,int) ;
extern int	recsvc_already(RECSVC *,uint,int,int) ;
extern int	recsvc_gettab(RECSVC *,uint **) ;
extern int	recsvc_rtlen(RECSVC *) ;
extern int	recsvc_count(RECSVC *) ;
extern int	recsvc_countindex(RECSVC *) ;
extern int	recsvc_sizeindex(RECSVC *) ;
extern int	recsvc_mkindex(RECSVC *,uint [][2],int) ;

#endif /* RECSVC_MASTER */


#endif /* RECSVC_INCLUDE */



